#include <stdlib.h>
#include <WinSock2.h>
#include <assert.h>

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

//单位微秒
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

static int readAble(int fd, uint32_t usec)
{
	struct timeval timeout;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	timeout.tv_sec = usec / 1000000;
	timeout.tv_usec = usec % 1000000;
	int ret = select(0, &fds, NULL, &fds, &timeout);
	return ret;
}

#include <time.h>      //添加头文件
int64_t getCurrentTime()
{
	struct timeval tv;
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;

	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	tv.tv_sec = clock;
	tv.tv_usec = wtm.wMilliseconds * 1000;
	return ((unsigned long long)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000);
}

typedef struct {
	int width;
	int height;
	int fps;
	int rate;
	uint32_t pixelformat;
	int index;

	X264Encode * encode;
	RTMPPush * publish;

	uint8_t * cache;
	uint8_t * cache_y;
	uint8_t * cache_u;
	uint8_t * cache_v;
}StreamParam;

class CaptureFrameData
	: public webrtc::DesktopCapturer::Callback,
	public X264Encode::Callback
{
public:
	void SetParam(StreamParam *param) {
		param_ = param;
	}

	virtual void OnCaptureCompleted(webrtc::DesktopFrame* frame) {
		if (param_->encode != NULL) {
			OnCaptureCompletedEncode(frame);
		}
	}

	void OnCaptureCompletedEncode(webrtc::DesktopFrame* frame) {
		time_used = getCurrentTime();

		int width = frame->size().width();
		int height = frame->size().height();

		x264_picture_t * input = param_->encode->LockInput();

		if (input->img.i_csp == X264_CSP_I420)
		{
			libyuv::ARGBToI420(frame->data(), frame->stride(),
				input->img.plane[0], input->img.i_stride[0],
				input->img.plane[1], input->img.i_stride[1],
				input->img.plane[2], input->img.i_stride[2],
				width, height);
		}

		param_->encode->encode(input, getCurrentTime());

		param_->encode->UnlockInput(input);
	}

	virtual void onPacket(uint8_t * packet, int len, int keyFrame, int64_t timestamp)
	{
		int64_t begin = getCurrentTime();
		int ret = param_->publish->send(packet + 4, len - 4, keyFrame == PICTURE_TYPE_I, begin);
		if (ret <= 0) {
			printf("rtmp error:%d\n", ret);
		}
		int64_t send = getCurrentTime() - begin;

		//printf("encode:%lld send:%lld t:%lld\n", getCurrentTime() - timestamp, send, begin);
		printf("t:%d\n", uint32_t(begin));
	}
private:
	StreamParam *param_;
	int64_t time_used;
};

void pushAsync(char * url)
{
	StreamParam param_ = {
		1920,1080,
		30,
		1024*1024,
		X264_CSP_I420,
		0,
		NULL,
		NULL
	};
	StreamParam * param = &param_;

	RTMPPush * publish = new RTMPPush();
	int bret = publish->connect(url);
	assert(bret == 1);

	int ChunkSize = 1024;
	bret = publish->publish(ChunkSize);
	assert(bret == 1);

	X264Encode * encode = new X264Encode();
	bret = encode->init(param->width, param->height, param->fps, param->rate, param->pixelformat, false);
	assert(bret == 1);

	publish->setMetaData(encode->getPPS() + 4, encode->ppsLen() - 4, encode->getSPS() + 4, encode->spsLen() - 4);

	param->encode = encode;
	param->publish = publish;

	const webrtc::DesktopCaptureOptions options;
	webrtc::ScreenCapturer * capturer = webrtc::ScreenCapturer::Create(options);
	capturer->SelectScreen(0);

	CaptureFrameData frame;
	frame.SetParam(param);

	param->encode->start(&frame);

	capturer->Start(&frame);

	webrtc::DesktopRegion region;
	region.AddRect(webrtc::DesktopRect::MakeWH(1920, 1080));

	uint32 frameInt = 1000 / param->fps;
	while (true) {
		int64_t lastRun = getCurrentTime();
		capturer->Capture(region);
		int64_t nowRun = getCurrentTime();
		int64_t Int = frameInt - (nowRun - lastRun);
		if (Int < 0) Int = 0;
		usleep(Int*1000);
	}
}
