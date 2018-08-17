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

	uint8_t * cache;
	uint8_t * cache_y;
	uint8_t * cache_u;
	uint8_t * cache_v;
}StreamParam;

static void init(char * url, StreamParam *param) {
	int ChunkSize = 1024;
	RTMP_CreatePublish(url, ChunkSize);

	param->cache = (uint8_t*)malloc(param->width * param->height * 3 / 2);
	param->cache_y = param->cache;
	param->cache_u = param->cache + param->width;
	param->cache_v = param->cache + param->width + param->width / 4;

	param->pixelformat = V4L2_PIX_FMT_YUV420;
	RTMP_InitVideoParams(param->width, param->height, param->fps, param->rate, param->pixelformat, false);
}

static void sendcapture(uint8_t* data, int size, StreamParam *param) {
	unsigned long timestamp = timeGetTime();
	if (param->index != 0) {
		int ret = RTMP_SendScreenCapture(data, param->height, timestamp, param->pixelformat, param->width, param->height);
		if (ret <= 0) {
			printf("error:%d -- %d\n", timestamp, param->index);
		}
		else {
			printf("%d -- %d\n", timestamp, param->index);
		}
	}
	param->index++;
}

#ifdef DUMP
FILE * fp = fopen("tmp.rgba", "wb");
int w_count = 20;
#endif

class CaptureFrameDataDirect
	: public webrtc::DesktopCapturer::Callback
{
public:
	void SetParam(StreamParam *param) {
		param_ = param;
	}

	virtual void OnCaptureCompleted(webrtc::DesktopFrame* frame) {
		OnCaptureCompletedFrame(frame);
	}

	void OnCaptureCompletedFrame(webrtc::DesktopFrame* frame) {
		int width = frame->size().width();
		int height = frame->size().height();
		uint8_t * y = param_->cache_y;
		uint8_t * u = param_->cache_u;
		uint8_t * v = param_->cache_v;

		libyuv::ARGBToI420(frame->data(), frame->stride(),
			y, width,
			u, width / 2,
			v, width / 2,
			width, height);

		sendcapture(param_->cache, width*height * 3 / 2, param_);

#ifdef DUMP
		//写出部分数据
		if (fp != NULL)
		{
			uint8 * buffer = frame->data();
			int len = width * height * 4;
			buffer = param_->cache;
			len = width * height * 3 / 2;
			fwrite(buffer, len, 1, fp);
			w_count--;
		}
		if (w_count <= 0) {
			if (fp != NULL) {
				fclose(fp);
				fp = NULL;
			}
		}
#endif
	}
private:
	StreamParam *param_;
};

void pushDirect(char * url)
{
	StreamParam param_ = {
		1920,1080,
		30,
		1024,
		X264_CSP_I420,
		0,

		NULL,
		NULL,
		NULL,
		NULL,
	};
	StreamParam * param = &param_;

	init(url, param);

	const webrtc::DesktopCaptureOptions options;
	webrtc::ScreenCapturer * capturer = webrtc::ScreenCapturer::Create(options);
	capturer->SelectScreen(0);

	CaptureFrameDataDirect frame;
	frame.SetParam(param);

	capturer->Start(&frame);

	webrtc::DesktopRegion region;
	region.AddRect(webrtc::DesktopRect::MakeWH(1920, 1080));

	while (true) {
		capturer->Capture(region);

		usleep(1000000 / param->fps);
	}
}