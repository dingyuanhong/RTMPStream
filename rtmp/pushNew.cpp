#include <stdlib.h>
#include <WinSock2.h>
#include <assert.h>
#include <list>

#include "../capture/screen_capturer.h"
#include "../capture/desktop_capture_options.h"
#include "../capture/desktop_geometry.h"
#include "../capture/desktop_region.h"
#include "../capture/desktop_frame.h"

#include "../Stream/x264_encoder.h"
#include "../Stream/librtmp_send264.h"

#include "../Stream/x264Encoder.h"
#include "../Stream/rtmp_push.h"
#include "../Stream/rtmp_pull.h"

extern "C" {
#include "../librtmp/log.h"
#include "../librtmp/rtmp.h"
}

#include "libyuv.h"

static void usleep(uint32_t usec)
{
	struct timeval timeout;
	fd_set fds;
	FD_ZERO(&fds);
	int32_t fd = socket(AF_INET, SOCK_DGRAM, 0);
	FD_SET(fd, &fds);
	timeout.tv_sec = usec / 1000000;
	timeout.tv_usec = usec % 1000000;
	int ret = select(0, NULL, NULL, &fds, &timeout);
	if (0 > ret)
	{
	}
	closesocket(fd);
}

class Element {
public:
	virtual void start() {
	}
	virtual void stop() {
		stop_ = true;
	}
	virtual void Run() {
	}

	virtual int onInput(void*data) { return -1; };
	virtual int onOutput(void *data) {
		if(output_ != NULL)
			return output_->onInput(data);
		return 0;
	}

	void attach(Element * output) {
		output_ = output;
	}
protected:
	Element * output_ = NULL;
	bool stop_ = true;
};

DWORD WINAPI ThreadHandleCallback(
	LPVOID lp
	)
{
	Element * thiz = (Element*)lp;
	thiz->Run();
	return 0;
}

class NoThreadElement
	:public Element
{
public:
	virtual void start() {
		stop_ = false;
		Run();
	}
};

class ThreadElement
	:public Element
{
public:
	~ThreadElement() {
		if (hThread != NULL)
		{
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
			hThread = NULL;
		}
	}

	void start() {
		stop_ = false;
		hThread = CreateThread(NULL,0, ThreadHandleCallback,this,0,NULL);
		assert(hThread != NULL);
	}
private:
	HANDLE hThread = NULL;
};

class OutputElement
	: public NoThreadElement
{
public:
	virtual int onOutput(void *data) { return -1;};
};

class CaptureElement
	: public NoThreadElement
	, public webrtc::DesktopCapturer::Callback
{
public:
	void setCapture(webrtc::DesktopCapturer * capturer) {
		capturer_ = capturer;
		if (capturer_ != NULL) {
			capturer_->Start(this);
		}

		region.AddRect(webrtc::DesktopRect::MakeWH(1920, 1080));
	}

	virtual void Run() {
		while (!stop_)
		{
			capturer_->Capture(region);

			usleep(1000000 / 30);
		}
	}

	void OnCaptureCompleted(webrtc::DesktopFrame* frame) {
		onOutput(frame);
	}
private:
	webrtc::DesktopCapturer * capturer_ = NULL;
	webrtc::DesktopRegion region;
};

typedef struct Packet {
	uint8_t * data;
	int len; 
	int keyFrame;
	int64_t timestamp;
}Packet;

class EncodeElement 
	: public ThreadElement
	, X264Encode::Callback
{
public:
	void setEncode(X264Encode * encode) {
		encode_ = encode;
		encode_->start(this);
	}

	virtual int onInput(void*data) {
		webrtc::DesktopFrame* frame = (webrtc::DesktopFrame*)data;

		int width = frame->size().width();
		int height = frame->size().height();

		x264_picture_t * input = encode_->LockInput();
		if (input == NULL) return 0;
		
		if (input->img.i_csp == X264_CSP_I420)
		{
			libyuv::ARGBToI420(frame->data(), frame->stride(),
				input->img.plane[0], input->img.i_stride[0],
				input->img.plane[1], input->img.i_stride[1],
				input->img.plane[2], input->img.i_stride[2],
				width, height);

			cache_.push_back(input);
			return 1;
		}
		else {
			encode_->UnlockInput(input);
		}
		return 0;
	}

	virtual void Run() {
		while (!stop_)
		{
			x264_picture_t * data = cache_.front();
			if (data != NULL)
			{
				cache_.pop_front();
				encode_->encode(data, timeGetTime());
			}
			else {
				usleep(1);
			}
		}
		
	}

	virtual void onPacket(uint8_t * data, int len, int keyFrame, int64_t timestamp) {
		Packet packet = { data,len,keyFrame,timestamp };
		onOutput(&packet);
	}
private:
	X264Encode * encode_;
	std::list<x264_picture_t *> cache_;
};

class RTMPOutput
: public OutputElement
{
public:
	virtual int onInput(void*data) {
		Packet * packet = (Packet*)data;
		
		int ret = push_->send(packet->data + 4, packet->len - 1, packet->keyFrame == PICTURE_TYPE_I, packet->timestamp);
		if (ret <= 0) {
			printf("rtmp error:%d\n", ret);
		}

		return 1;
	}

	void setRTMP(RTMPPush * publish) {
		push_ = publish;
	}

private:
	RTMPPush * push_;
};

typedef struct {
	int width;
	int height;
	int fps;
	int rate;
	uint32_t pixelformat;
}StreamParam;

void pushNew(char * url)
{
	StreamParam param_ = {
		1920,1080,
		30,
		1024,
		X264_CSP_I420
	};
	StreamParam * param = &param_;

	X264Encode * encode = new X264Encode();
	int bret = encode->init(param->width, param->height, param->fps, param->rate, param->pixelformat, false);
	assert(bret == 1);

	RTMPPush * publish = new RTMPPush();
	bret = publish->connect(url);
	assert(bret == 1);

	int ChunkSize = 1024;
	bret = publish->publish(ChunkSize);
	assert(bret == 1);

	publish->setMetaData(encode->getPPS()+4, encode->ppsLen()-4, encode->getSPS()+4, encode->spsLen()-4);

	const webrtc::DesktopCaptureOptions options;
	webrtc::ScreenCapturer * capturer = webrtc::ScreenCapturer::Create(options);
	capturer->SelectScreen(0);

	CaptureElement source;
	EncodeElement eelement;
	RTMPOutput output;

	source.setCapture(capturer);
	source.attach(&eelement);

	eelement.setEncode(encode);

	output.setRTMP(publish);

	output.start();
	eelement.start();
	source.start();
}
