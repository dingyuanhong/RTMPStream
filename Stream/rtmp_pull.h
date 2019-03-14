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

class RTMPPull {
public:
	class Callback {
	public:
		virtual void onPacket(uint8_t * packet, int len) {};
		virtual void onPacket(RTMPPacket *packet) {};
	};

	int connect(char * url);
	void start(Callback *callback) {
		callback_ = callback;
	}
	void stop(bool b);
	void close();

	int Read();
	int ReadPacket();

	bool IsConnected();

	RTMP * Handle() {
		return rtmp;
	}
private:
	bool bstop = false;
	RTMP *rtmp = NULL;
	Callback * callback_ = NULL;
};