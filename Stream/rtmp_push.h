#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "librtmp/rtmp.h"
#include "librtmp/rtmp_sys.h"
#include "librtmp/amf.h"
#include "librtmp/log.h"

#ifdef __cplusplus
}
#endif

class RTMPPush {
public:
	int connect(char * url);
	int publish(int outChunkSize);
	int send(uint8_t *data, uint32_t size, bool bIsKeyFrame, uint32_t timestamp);
	void close();

	void setMetaData(unsigned char *pps, int pps_len, unsigned char * sps, int sps_len);
private:
	RTMP *m_rtmp = NULL;
	RTMPPacket* packet_ = NULL;
	RTMPPacket* packet_meta = NULL;

	uint8_t * cache_buffer = NULL;
	uint32_t cache_length = 0;
};