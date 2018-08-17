#ifndef X264ENCODE_H_
#define X264ENCODE_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "x264.h"
#include "x264_config.h"
#ifdef __cplusplus
}
#endif

//YUYV:X264_CSP_I422
//I420:X264_CSP_I420

class X264Encode {
public:
	class Callback {
	public:
		virtual void onPacket(uint8_t * packet, int len,int keyFrame, int64_t timestamp) = 0;
	};

	int init(int width, int height, int fps, int bitrate,uint32_t pixelFormat,bool bConstantsBitrate);
	void start(Callback *callback) {
		callback_ = callback;
	}
	void close();

	int encode(x264_picture_t *image, int64_t timestamp);
	x264_picture_t * LockInput();
	void UnlockInput(x264_picture_t* image);

	int spsLen() {
		return m_sps_len;
	}
	uint8_t *getSPS() {
		return m_sps;
	}
	int ppsLen() {
		return m_pps_len;
	}
	uint8_t * getPPS() {
		return m_pps;
	}
private:
	int m_width;	 //宽度
	int m_height;	 // 高度
	int	m_frameRate; //帧率fps
	int m_bitRate;	 //比特率
	int m_byteFrameSize;

	bool m_bHasExtraData;
	uint8_t * m_sps = NULL;
	int m_sps_len;
	uint8_t * m_pps = NULL;
	int m_pps_len;

	x264_t * h = NULL;

	x264_param_t param;
	x264_nal_t* nal_t = NULL;

	int available_count;
	int max_input_count;
	x264_picture_t **m_pPool = NULL;
	x264_picture_t *m_picInput = NULL;
	x264_picture_t m_picOutput;

	Callback * callback_ = NULL;
};

#endif


