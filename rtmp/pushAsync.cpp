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
		time_used = timeGetTime();

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

		param_->encode->encode(input, timeGetTime());

		param_->encode->UnlockInput(input);


	}

	virtual void onPacket(uint8_t * packet, int len, int keyFrame, int64_t timestamp)
	{
		int64_t encode = timeGetTime() - time_used;
		int ret = param_->publish->send(packet, len, keyFrame == 1, timeGetTime());
		if (ret <= 0) {
			printf("rtmp error:%d\n", ret);
		}
		int64_t send = timeGetTime() - time_used;

		printf("c:%d t:%lld e:%lld s:%lld\n", timeGetTime(), timestamp, encode, send);
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
		1024,
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

	publish->setMetaData(encode->getPPS(), encode->ppsLen(), encode->getSPS(), encode->spsLen());

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

	while (true) {
		capturer->Capture(region);

		usleep(1000000 / param->fps);
	}
}
