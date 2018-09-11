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

#define PICTURE_TYPE_N 0
#define PICTURE_TYPE_I 1
#define PICTURE_TYPE_B 2
#define PICTURE_TYPE_P 3
#define PICTURE_TYPE_O 4

class X264Encode {
public:
	class Callback {
	public:
		virtual void onPacket(uint8_t * packet, int len,int pictureType, int64_t timestamp) = 0;
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

	uint8_t *getSEI() {
		return m_sei;
	}
	int seiLen() {
		return m_sei_len;
	}
private:
	int m_width;	 //宽度
	int m_height;	 // 高度
	int	m_frameRate; //帧率fps
	int m_bitRate;	 //比特率
	int m_byteFrameSize;

	bool m_bHasExtraData;
	uint8_t * m_sps = NULL;
	int m_sps_len = 0;
	uint8_t * m_pps = NULL;
	int m_pps_len = 0;
	uint8_t *m_sei = NULL;
	int m_sei_len = 0;

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


