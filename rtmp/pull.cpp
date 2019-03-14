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

#include "util.h"

class RawData
	: public RTMPPull::Callback
{
public:
	RawData(const char * file)
	{
		m_file = (char*)file;
		//fp = fopen(file, "wb+");
	}
	~RawData() {
		if(fp != NULL) fclose(fp);
	}
private:
	void onPacket(uint8_t * packet, int len) {
		
		if (fp != NULL) fwrite(packet, 1, len, fp);
	}

	void onPacket(RTMPPacket *packet) {
		char name[255];
		sprintf(name, "%s/%d.bit", m_file,index++);
		fp = fopen(name,"wb+");
		if (fp != NULL) {
			fwrite(packet->m_body, 1, packet->m_nBodySize, fp);
			fclose(fp);
			fp = NULL;
		}
	};

	char * m_file;
	FILE * fp = NULL;
	int index = 0;
};

class H264Decode
	: public RTMPPull::Callback
{
public:
	H264Decode() {
		time = getCurrentTime();
		first = true;
		firstPacket = true;
	}
	virtual void onPacket(RTMPPacket *packet) {
		int64_t c = getCurrentTime();

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
				//FrameType|CodecID PacketType CompositionTime 
				printf("%02x %02x %02x %02x %02x : %02X %02X %02X %02X %02X\n", body[0], body[1], body[2], body[3], body[4],   body[5], body[6], body[7], body[8], body[9]);

				printf(" c:%lld t:%d diff:%d absolute:%d\n", c, packet->m_nTimeStamp, (uint32_t)(c - packet->m_nTimeStamp) ,packet->m_hasAbsTimestamp);

				//printf("t:%d\n", packet->m_nTimeStamp);
			}
			else {
				//printf("Nc:%d t:%d diff:%d\n", c, packet->m_nTimeStamp, c - packet->m_nTimeStamp);
			}
		}
	}
private:
	int64_t time;
	bool first;
	bool firstPacket;
};

void pullRaw(char * url,char * output)
{
	RawData data(output);
	RTMPPull pull;
	pull.start(&data);
	int ret = pull.connect(url);
	if (ret < 0) {
		return;
	}
	int failedCount = 0;
	while (true) {
		int n = pull.ReadPacket();
		if (n == FALSE) {
			failedCount++;
			if (failedCount >= 5) {
				int ret = RTMP_ReconnectStream(pull.Handle(), -1);
				if (ret == 0) {
					break;
				}
				continue;
			}
		}
		else {
			failedCount = 0;
		}
		int ret = readAble(RTMP_Socket(pull.Handle()), 1000);

		if (ret == -1) {
			if (!pull.IsConnected()) break;
		}
	}
}

void pullH264(char * url) 
{
	//RawData data("./raw.h264");
	H264Decode data;
	RTMPPull pull;
	pull.start(&data);
	int ret = pull.connect(url);
	if (ret < 0) {
		return;
	}
	while (true) {
		int n = pull.ReadPacket();
		int ret = readAble(RTMP_Socket(pull.Handle()), 1000);
		if (ret == -1) {
			if (!pull.IsConnected()) break;
		}
	}
}