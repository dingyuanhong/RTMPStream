#include "rtmp_push.h"

//定义包头长度，RTMP_MAX_HEADER_SIZE=18
#define RTMP_HEAD_SIZE   (sizeof(RTMPPacket)+RTMP_MAX_HEADER_SIZE)
//存储Nal单元数据的buffer大小
#define BUFFER_SIZE 32768
//搜寻Nal单元时的一些标志
#define GOT_A_NAL_CROSS_BUFFER 			BUFFER_SIZE+1
#define GOT_A_NAL_INCLUDE_A_BUFFER 		BUFFER_SIZE+2
#define NO_MORE_BUFFER_TO_READ 			BUFFER_SIZE+3

#define BODY_HEADER_SIZE 9

int RTMPPush::connect(char * url)
{
	RTMP *rtmp = RTMP_Alloc();
	RTMP_Init(rtmp);
	rtmp->Link.lFlags |= RTMP_LF_LIVE;

	/*设置URL*/
	if (RTMP_SetupURL(rtmp, (char*)url) == FALSE)
	{
		RTMP_Free(rtmp);
		return 0;
	}
	/*设置可写,即发布流,这个函数必须在连接前使用,否则无效*/
	RTMP_EnableWrite(rtmp);
	/*连接服务器*/
	if (RTMP_Connect(rtmp, NULL) == FALSE)
	{
		RTMP_Free(rtmp);
		return 0;
	}

	/*连接流*/
	if (RTMP_ConnectStream(rtmp, 0) == FALSE)
	{
		RTMP_Close(rtmp);
		RTMP_Free(rtmp);
		return 0;
	}
	m_rtmp = rtmp;
	return 1;
}

void RTMPPush::close()
{
	if (m_rtmp != NULL) {
		//RTMP_Close(rtmp);
		RTMP_Free(m_rtmp);
		m_rtmp = NULL;
	}

	if (cache_buffer != NULL)
	{
		free(cache_buffer); cache_buffer = NULL;
	}
	cache_length = NULL;

	if (packet_ != NULL)
	{
		free(packet_);
		packet_ = NULL;
	}
}

void RTMPPush::setMetaData(unsigned char *pps, int pps_len, unsigned char * sps, int sps_len)
{
	RTMPPacket * packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE + 1024);
	memset(packet, 0, RTMP_HEAD_SIZE + 1024);
	packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
	unsigned char * body = (unsigned char *)packet->m_body;
	int i = 0;
	body[i++] = 0x17;
	body[i++] = 0x00;

	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;

	/*AVCDecoderConfigurationRecord*/
	body[i++] = 0x01;
	body[i++] = sps[1];
	body[i++] = sps[2];
	body[i++] = sps[3];
	body[i++] = 0xff;

	/*sps*/
	body[i++] = 0xe1;
	body[i++] = (sps_len >> 8) & 0xff;
	body[i++] = sps_len & 0xff;
	memcpy(&body[i], sps, sps_len);
	i += sps_len;

	/*pps*/
	body[i++] = 0x01;
	body[i++] = (pps_len >> 8) & 0xff;
	body[i++] = (pps_len)& 0xff;
	memcpy(&body[i], pps, pps_len);
	i += pps_len;

	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet->m_nBodySize = i;
	packet->m_nChannel = 0x04;
	packet->m_nTimeStamp = 0;
	packet->m_hasAbsTimestamp = 0;
	packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	packet->m_nInfoField2 = m_rtmp->m_stream_id;

	if (packet_meta != NULL) {
		free(packet_meta);
	}
	packet_meta = packet;
}

void setPacket(RTMP *rtmp,RTMPPacket *packet, unsigned int nPacketType, unsigned int size, uint32_t nTimestamp)
{
	memset(packet, 0, RTMP_HEAD_SIZE);

	packet->m_hasAbsTimestamp = 0;
	packet->m_packetType = nPacketType; 				/*此处为类型有两种一种是音频,一种是视频*/
	packet->m_nInfoField2 = rtmp->m_stream_id;
	packet->m_nChannel = 0x04;

	packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
	if (RTMP_PACKET_TYPE_AUDIO == nPacketType && size != 4) {
		packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	}
	packet->m_nTimeStamp = nTimestamp;

	packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
	packet->m_nBodySize = size;
}

int RTMPPush::publish(int outChunkSize)
{
	//修改发送分包的大小  默认128字节
	RTMPPacket pack;
	RTMPPacket_Alloc(&pack, 4);
	pack.m_packetType = RTMP_PACKET_TYPE_CHUNK_SIZE;
	pack.m_nChannel = 0x02;
	pack.m_headerType = RTMP_PACKET_SIZE_LARGE;
	pack.m_nTimeStamp = 0;
	pack.m_nInfoField2 = 0;
	pack.m_nBodySize = 4;
	pack.m_body[3] = outChunkSize & 0xff; //大字节序
	pack.m_body[2] = outChunkSize >> 8;
	pack.m_body[1] = outChunkSize >> 16;
	pack.m_body[0] = outChunkSize >> 24;

	m_rtmp->m_outChunkSize = outChunkSize;

	int ret = RTMP_SendPacket(m_rtmp, &pack, TRUE);
	RTMPPacket_Free(&pack);

	return ret;
}

int RTMPPush::send(uint8_t *data, uint32_t size, bool bIsKeyFrame, uint32_t timestamp)
{
	if (data == NULL && size < 11) {
		return -1;
	}

	uint32_t nBodySize = BODY_HEADER_SIZE + size;
	uint32_t max_size = RTMP_HEAD_SIZE + nBodySize;

	if (max_size > cache_length) {
		if (cache_buffer != NULL)
		{
			free(cache_buffer); cache_buffer = NULL;
		}

		cache_length = max_size;
		cache_buffer = (uint8_t*)malloc(cache_length);
		packet_ = (RTMPPacket*)cache_buffer;
		packet_->m_body = (char *)packet_ + RTMP_HEAD_SIZE;
	}
	
	char *body = packet_->m_body;
	int i = 0;
	int nRet = 0;

	if (bIsKeyFrame) {
		int bRetMeta = 1;
		if (packet_meta != NULL)
		{
			packet_meta->m_nTimeStamp = timestamp;
			bRetMeta = RTMP_SendPacket(m_rtmp, packet_meta, TRUE);

			//printf("t:%d\n",packet_meta->m_nTimeStamp);
		}

		if (bRetMeta > 0) {
			setPacket(m_rtmp, packet_, RTMP_PACKET_TYPE_VIDEO, nBodySize, timestamp);

			body[i++] = 0x17;// 1:Iframe  7:AVC
			body[i++] = 0x01;// AVC NALU
			body[i++] = 0x00;
			body[i++] = 0x00;
			body[i++] = 0x00;

			// NALU size
			body[i++] = size >> 24 & 0xff;
			body[i++] = size >> 16 & 0xff;
			body[i++] = size >> 8 & 0xff;
			body[i++] = size & 0xff;
			// NALU data
			memcpy(&body[i], data, size);

			if (RTMP_IsConnected(m_rtmp)) {
				nRet = RTMP_SendPacket(m_rtmp, packet_, TRUE); /*TRUE为放进发送队列,FALSE是不放进发送队列,直接发送*/

				//printf("t:%d\n", packet_->m_nTimeStamp);
			}
		}
		return nRet;
	}
	else {
		setPacket(m_rtmp, packet_, RTMP_PACKET_TYPE_VIDEO, nBodySize, timestamp);

		body[i++] = 0x27;// 2:Pframe  7:AVC
		body[i++] = 0x01;// AVC NALU
		body[i++] = 0x00;
		body[i++] = 0x00;
		body[i++] = 0x00;

		// NALU size
		body[i++] = size >> 24 & 0xff;
		body[i++] = size >> 16 & 0xff;
		body[i++] = size >> 8 & 0xff;
		body[i++] = size & 0xff;
		// NALU data
		memcpy(&body[i], data, size);

		if (RTMP_IsConnected(m_rtmp)) {
			nRet = RTMP_SendPacket(m_rtmp, packet_, TRUE); /*TRUE为放进发送队列,FALSE是不放进发送队列,直接发送*/

			//printf("t:%d\n", packet_->m_nTimeStamp);
		}
		return nRet;
	}
}