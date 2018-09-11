/*=============================================================================  
 *     FileName: x264_encoder.cpp
 *         Desc:  
 *       Author: licaibiao  
 *   LastChange: 2017-05-4   
 * https://www.cnblogs.com/lihaiping/p/4167844.html
 * =============================================================================*/  
#include "x264Encoder.h"
#include "jsoncpp\json\json.h"
#include <fstream>
#include "x264Option.h"


void setX264EncodeParam(x264_param_t *param) {
	
	std::ifstream in;
	in.open("x264.config.json");
	if(!in) return;

	Json::CharReaderBuilder builder;
	Json::Value value;
	std::string err;
	bool ret = Json::parseFromStream(builder, in,&value,&err);
	if (ret == false) return;

	if (value.isObject())
	{
		Json::Value::iterator it = value.begin();
		for (; it != value.end(); it++)
		{
			for (int j = 0; j < sizeof(x264Option) / sizeof(x264Option[0]); j++)
			{
				ss_set_option((uint8_t*)param, &x264Option[j],it.name(),it.key());
			}
		}
	}
}

int X264Encode::init(int width,int height,int fps,int bitrate, uint32_t pixelFormat, bool bConstantsBitrate)
{
	m_width = width;
	m_height = height;
	m_frameRate = fps;
	m_bitRate = bitrate;
	m_byteFrameSize = m_width*m_height;

	x264_param_default(&param);//设置默认参数具体见common/common.c
	//* 使用默认参数，在这里因为是实时网络传输，所以使用了zerolatency的选项，使用这个选项之后就不会有delayed_frames，如果使用的不是这样的话，还需要在编码完成之后得到缓存的编码帧
	x264_param_default_preset(&param, "veryfast", "zerolatency");
	x264_param_apply_profile(&param, "baseline");

	//* cpuFlags
	param.i_threads = X264_SYNC_LOOKAHEAD_AUTO; /* 取空缓冲区继续使用不死锁的保证 */
	param.i_sync_lookahead = X264_SYNC_LOOKAHEAD_AUTO;

	//* 视频选项
	param.i_width = m_width;
	param.i_height = m_height;
	//图片格式
	param.i_csp = pixelFormat;
	//码率
	param.rc.i_bitrate = (int)m_bitRate; //* 码率(比特率,单位Kbps)

	//帧设置
	param.i_frame_total = 0; 			//* 编码总帧数.不知道用0.
	param.i_keyint_min = 0;				//关键帧最小间隔
	param.i_keyint_max = (int)fps * 2;	//关键帧最大间隔
	param.b_annexb = 1;					//1前面为0x00000001,0为nal长度
	param.b_repeat_headers = 1;			//关键帧前面是否放sps跟pps帧，0 否 1，放
	
	//* B帧参数
	param.i_bframe = 0;					//B帧
	param.b_open_gop = 0;
	param.i_bframe_pyramid = 0;
	param.i_bframe_adaptive = X264_B_ADAPT_FAST;

	//* 速率控制参数
	param.rc.i_lookahead = 0;
	
	if (!bConstantsBitrate) {
		param.rc.i_rc_method = X264_RC_ABR;			   //参数i_rc_method表示码率控制，CQP(恒定质量)，CRF(恒定码率)，ABR(平均码率)
		param.rc.i_vbv_max_bitrate = (int)m_bitRate * 1; // 平均码率模式下，最大瞬时码率，默认0(与-B设置相同)
	}
	else {
		param.rc.b_filler = 1;
		param.rc.i_rc_method = X264_RC_ABR;		 //参数i_rc_method表示码率控制，CQP(恒定质量)，CRF(恒定码率)，ABR(平均码率)
		param.rc.i_vbv_max_bitrate = m_bitRate;  // 平均码率模式下，最大瞬时码率，默认0(与-B设置相同)
		param.rc.i_vbv_buffer_size = m_bitRate; //vbv-bufsize
	}

	//* muxing parameters
	param.i_fps_den = 1; 			// 帧率分母
	param.i_fps_num = fps;			// 帧率分子
	param.i_timebase_num = 1;
	param.i_timebase_den = 1000;

	setX264EncodeParam(&param);

	h = x264_encoder_open(&param);	//根据参数初始化X264级别

	x264_picture_init(&m_picOutput);//初始化图片信息

	max_input_count = 10;
	available_count = max_input_count;
	m_pPool = new x264_picture_t*[max_input_count];
	m_picInput = new x264_picture_t[max_input_count];
	for (int i = 0; i < max_input_count; i++) {
		x264_picture_alloc(&m_picInput[i], param.i_csp, m_width, m_height);//图片按I422格式分配空间，最后要x264_picture_clean
		m_picInput[i].i_pts = 0;

		m_pPool[i] = &m_picInput[i];
	}

	int i_nal = 0;
	x264_encoder_headers(h, &nal_t, &i_nal);

	m_bHasExtraData = false;
	if (i_nal > 0) {
		m_bHasExtraData = true;
		for (int i = 0; i < i_nal; i++) {
			//获取SPS数据，PPS数据
			if (nal_t[i].i_type == NAL_SPS) {
				m_sps = new unsigned char[nal_t[i].i_payload];
				m_sps_len = nal_t[i].i_payload;
				memcpy(m_sps, nal_t[i].p_payload, nal_t[i].i_payload);
			}
			else if (nal_t[i].i_type == NAL_PPS) {
				m_pps = new unsigned char[nal_t[i].i_payload];;
				m_pps_len = nal_t[i].i_payload;
				memcpy(m_pps, nal_t[i].p_payload, nal_t[i].i_payload);
			}
			else if (nal_t[i].i_type == NAL_SEI) {
				m_sei = new unsigned char[nal_t[i].i_payload];;
				m_sei_len = nal_t[i].i_payload;
				memcpy(m_sei, nal_t[i].p_payload, nal_t[i].i_payload);
			}
		}
		return 1;
	}

	return 0;
}

x264_picture_t * X264Encode::LockInput()
{
	if (available_count <= 0) {
		return NULL;
	}
	x264_picture_t * ret = m_pPool[0];
	available_count--;
	if (available_count > 0) {
		m_pPool[0] = m_pPool[available_count];
	}
	return ret;
}

void X264Encode::UnlockInput(x264_picture_t* image)
{
	m_pPool[available_count] = image;
	available_count++;
}

int X264Encode::encode(x264_picture_t *image,int64_t timestamp)
{
	int i_nal = 0;
	image->i_pts = timestamp;
	int ret = x264_encoder_encode(h, &nal_t, &i_nal, image, &m_picOutput);
	//image->i_pts++;//少这句的话会出现 x264 [warning]: non-strictly-monotonic PTS

	int rResult = 0;
	for (int i = 0; i < i_nal; i++) {
		uint8_t * packet = nal_t[i].p_payload;
		int len = nal_t[i].i_payload;
		int picture_type = 0;

		switch (nal_t[i].i_type) {
		case X264_TYPE_IDR:
		case X264_TYPE_I:
			picture_type = PICTURE_TYPE_I;
			break;
		case X264_TYPE_P:
			picture_type = PICTURE_TYPE_P;
			break;
		case X264_TYPE_B:
		case X264_TYPE_BREF:
			picture_type = PICTURE_TYPE_B;
			break;
		default:
			break;
		}
		if (picture_type == 0) continue;
		if (callback_ != NULL) {
			callback_->onPacket(packet, len, picture_type, timestamp);
		}
	}
	return rResult;
}

void X264Encode::close()
{
	if (m_sps) {
		delete[] m_sps;
		m_sps = NULL;
	}
	m_sps_len = 0;

	if (m_pps) {
		delete[] m_pps;
		m_pps = NULL;
	}
	m_pps_len = 0;

	if (m_sei) {
		delete[]m_sei;
		m_sei = NULL;
	}
	m_sei_len = 0;

	if (h) {
		x264_encoder_close(h);
		h = NULL;
	}

	if (m_picInput != NULL) {
		for (int i = 0; i < max_input_count; i++) {
			x264_picture_clean(&m_picInput[i]);
		}
		delete[]m_picInput;
		m_picInput = NULL;
	}

	if (m_pPool != NULL) {
		memset(m_pPool,NULL, sizeof(x264_picture_t*)*max_input_count);
		delete[] m_pPool;
		m_pPool = NULL;
	}
}

