#include "rtmp_pull.h"

int RTMPPull::connect(char * url)
{
	bool bLiveStream = true;

	rtmp = RTMP_Alloc();
	RTMP_Init(rtmp);

	int ret = RTMP_SetupURL(rtmp, url);
	if (ret == FALSE) {
		RTMP_Free(rtmp);
		rtmp = NULL;
		return -1;
	}

	if (bLiveStream) {
		rtmp->Link.lFlags |= RTMP_LF_LIVE;
	}

	RTMP_SetBufferMS(rtmp, 3600 * 1000);

	ret = RTMP_Connect(rtmp, NULL);
	if (!ret) {
		RTMP_Free(rtmp);
		rtmp = NULL;
		return -1;
	}
	ret = RTMP_ConnectStream(rtmp, 0);
	if (!ret) {
		RTMP_Close(rtmp);
		RTMP_Free(rtmp);
		rtmp = NULL;
		return -1;
	}
	return TRUE;
}

int RTMPPull::Read()
{
	int nRead = 0;
	int bufsize = 1024 * 1024 * 10;
	char *buf = (char*)malloc(bufsize);
	memset(buf, 0, bufsize);

	while ((nRead = RTMP_Read(rtmp, buf, bufsize)) > 0){
		if (!RTMP_IsConnected(rtmp)) {
			break;
		}
		if (callback_ != NULL) {
			callback_->onPacket((uint8_t*)buf,bufsize);
		}
	}

	free(buf);

	return nRead;
}

int RTMPPull::ReadPacket()
{
	int nRead = 0;
	RTMPPacket packet = { 0 };

	while ((nRead = RTMP_ReadPacket(rtmp, &packet)) > 0) {
		if (RTMPPacket_IsReady(&packet))
		{
			if (callback_ != NULL) {
				callback_->onPacket(&packet);
			}
		}
		RTMPPacket_Free(&packet);

		if (bstop) break;
		if (!RTMP_IsConnected(rtmp)) {
			break;
		}
	}

	return nRead;
}

void RTMPPull::stop(bool b)
{
	bstop = b;
}

void RTMPPull::close()
{
	if (rtmp != NULL)
	{
		RTMP_Close(rtmp);
		RTMP_Free(rtmp);
		rtmp = NULL;
	}
}