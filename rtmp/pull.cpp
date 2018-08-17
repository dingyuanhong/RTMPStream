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

class RawData
	: public RTMPPull::Callback
{
public:
	RawData(const char * file)
	{
		fp = fopen(file, "wb+");
	}
	~RawData() {
		fclose(fp);
	}
private:
	void onPacket(uint8_t * packet, int len) {
		if (fp != NULL) fwrite(packet, 1, len, fp);
	}

	FILE * fp = NULL;
};

class H264Decode
	: public RTMPPull::Callback
{
public:
	H264Decode() {
		time = timeGetTime();
		first = true;
		firstPacket = true;
	}
	virtual void onPacket(RTMPPacket *packet) {
		uint32_t c = timeGetTime();

		if (firstPacket) {
			printf("c:%d t:%d diff:%d\n", c, time, c - time);
			firstPacket = false;
		}

		if (packet->m_packetType == RTMP_PACKET_TYPE_VIDEO)
		{
			if (first) {
				printf("c:%d t:%d diff:%d\n", c, time, c - time);
				first = false;
			}


			uint8_t * body = (uint8_t*)packet->m_body;
			if (body != NULL)
			{
				//printf("%02x %02x %02x %02x %02x\n", body[0], body[1], body[2], body[3], body[4]);

				//printf(" c:%d t:%d diff:%d\n", c, packet->m_nTimeStamp, c - packet->m_nTimeStamp);

				//printf("t:%d\n", packet->m_nTimeStamp);
			}
			else {
				//printf("Nc:%d t:%d diff:%d\n", c, packet->m_nTimeStamp, c - packet->m_nTimeStamp);
			}
		}
	}
private:
	uint32_t time;
	bool first;
	bool firstPacket;
};

void pull(char * url) 
{
	//RawData data("./raw.h264");
	H264Decode data;
	RTMPPull pull;
	pull.start(&data);
	pull.connect(url);
	while (true) {
		int n = pull.ReadPacket();
		readAble(RTMP_Socket(pull.Handle()), 1000);
	}
}