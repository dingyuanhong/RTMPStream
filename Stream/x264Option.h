#ifndef X264OPTION_H
#define X264OPTION_H
#pragma once
/*
* https://www.cnblogs.com/lihaiping/p/4167844.html
*/

#include<stdint.h>
#include <string>
#include "jsoncpp\json\json.h"

enum SSType {
	AV_OPT_TYPE_FLAGS,
	AV_OPT_TYPE_UINT,
	AV_OPT_TYPE_INT,
	AV_OPT_TYPE_INT64,
	AV_OPT_TYPE_FLOAT,
	AV_OPT_TYPE_DOUBLE,
	AV_OPT_TYPE_CONST = 128,
};

typedef struct SSOption {
	char* name;
	char *help;
	int offset;
	enum SSType type;
}SSOption;

void ss_set_option(uint8_t* obj, SSOption * option, std::string &name, Json::Value &value) {
	if (strcmp(option->name , name.c_str()) == 0)
	{
		try{
			if (option->type == AV_OPT_TYPE_FLAGS)
			{
				*(bool*)(obj + option->offset) = value.asBool();
			}
			else if (option->type == AV_OPT_TYPE_UINT)
			{
				*(Json::Value::UInt*)(obj + option->offset) = value.asUInt();
			}
			else if (option->type == AV_OPT_TYPE_INT)
			{
				*(int*)(obj + option->offset) = value.asInt();
			}
			else if (option->type == AV_OPT_TYPE_INT64)
			{
				*(Json::Value::Int64*)(obj + option->offset) = value.asInt64();
			}
			else if (option->type == AV_OPT_TYPE_FLOAT)
			{
				*(float*)(obj + option->offset) = value.asFloat();
			}
			else if (option->type == AV_OPT_TYPE_DOUBLE)
			{
				*(double*)(obj + option->offset) = value.asDouble();
			}
			else {

			}
		}
		catch (...) {
		}
	}
}

#define OFFSET(x) offsetof(x264_param_t, x)
SSOption x264Option[] = {
	/* CPU flags */
	{ "cpu","cpu±êÖ¾Î»",OFFSET(cpu),AV_OPT_TYPE_UINT },
	{ "i_threads","",OFFSET(i_threads),AV_OPT_TYPE_INT },
	{ "i_lookahead_threads","",OFFSET(i_lookahead_threads),AV_OPT_TYPE_INT },
	{ "b_sliced_threads","",OFFSET(b_sliced_threads),AV_OPT_TYPE_INT },
	{ "b_deterministic","",OFFSET(b_deterministic),AV_OPT_TYPE_INT },
	{ "b_cpu_independent","",OFFSET(b_cpu_independent),AV_OPT_TYPE_INT },
	{ "i_sync_lookahead","",OFFSET(i_sync_lookahead),AV_OPT_TYPE_INT },
	/* Video Properties */
	{ "i_width","",OFFSET(i_width),AV_OPT_TYPE_INT },
	{ "i_height","",OFFSET(i_height),AV_OPT_TYPE_INT },
	{ "i_csp","",OFFSET(i_csp),AV_OPT_TYPE_INT },
	{ "i_level_idc","",OFFSET(i_level_idc),AV_OPT_TYPE_INT },
	{ "i_frame_total","",OFFSET(i_frame_total),AV_OPT_TYPE_INT },
	/* NAL HRD*/
	{ "i_nal_hrd","",OFFSET(i_nal_hrd),AV_OPT_TYPE_INT },

	{ "vui.i_sar_height","",OFFSET(vui.i_sar_height),AV_OPT_TYPE_INT },
	{ "vui.i_sar_width","",OFFSET(vui.i_sar_width),AV_OPT_TYPE_INT },
	{ "vui.i_sar_height","",OFFSET(vui.i_sar_height),AV_OPT_TYPE_INT },
	{ "vui.i_overscan","",OFFSET(vui.i_overscan),AV_OPT_TYPE_INT },
	{ "vui.b_fullrange","",OFFSET(vui.b_fullrange),AV_OPT_TYPE_INT },
	{ "vui.i_colorprim","",OFFSET(vui.i_colorprim),AV_OPT_TYPE_INT },
	{ "vui.i_transfer","",OFFSET(vui.i_transfer),AV_OPT_TYPE_INT },
	{ "vui.i_colmatrix","",OFFSET(vui.i_colmatrix),AV_OPT_TYPE_INT },
	{ "vui.i_chroma_loc","",OFFSET(vui.i_chroma_loc),AV_OPT_TYPE_INT },

	{ "i_frame_reference","",OFFSET(i_frame_reference),AV_OPT_TYPE_INT },
	{ "i_dpb_size","",OFFSET(i_dpb_size),AV_OPT_TYPE_INT },

	{ "i_keyint_max","",OFFSET(i_keyint_max),AV_OPT_TYPE_INT },
	{ "i_keyint_min","",OFFSET(i_keyint_min),AV_OPT_TYPE_INT },
	{ "i_scenecut_threshold","",OFFSET(i_scenecut_threshold),AV_OPT_TYPE_INT },
	{ "b_intra_refresh","",OFFSET(b_intra_refresh),AV_OPT_TYPE_INT },

	{ "i_bframe","",OFFSET(i_bframe),AV_OPT_TYPE_INT },
	{ "i_bframe_adaptive","",OFFSET(i_bframe_adaptive),AV_OPT_TYPE_INT },
	{ "i_bframe_bias","",OFFSET(i_bframe_bias),AV_OPT_TYPE_INT },
	{ "i_bframe_pyramid","",OFFSET(i_bframe_pyramid),AV_OPT_TYPE_INT },
	{ "b_open_gop","",OFFSET(b_open_gop),AV_OPT_TYPE_INT },
	{ "b_bluray_compat","",OFFSET(b_bluray_compat),AV_OPT_TYPE_INT },
	{ "i_avcintra_class","",OFFSET(i_avcintra_class),AV_OPT_TYPE_INT },

	{ "b_deblocking_filter","",OFFSET(b_deblocking_filter),AV_OPT_TYPE_INT },
	{ "i_deblocking_filter_alphac0","",OFFSET(i_deblocking_filter_alphac0),AV_OPT_TYPE_INT },
	{ "i_deblocking_filter_beta","",OFFSET(i_deblocking_filter_beta),AV_OPT_TYPE_INT },

	{ "b_cabac","",OFFSET(b_cabac),AV_OPT_TYPE_INT },
	{ "i_cabac_init_idc","",OFFSET(i_cabac_init_idc),AV_OPT_TYPE_INT },
	{ "b_interlaced","",OFFSET(b_interlaced),AV_OPT_TYPE_INT },
	{ "b_constrained_intra","",OFFSET(b_constrained_intra),AV_OPT_TYPE_INT },

	{ "i_cqm_preset","",OFFSET(i_cqm_preset),AV_OPT_TYPE_INT },

	/* Log */
	{ "i_log_level","",OFFSET(i_log_level),AV_OPT_TYPE_INT },
	{ "b_full_recon","",OFFSET(b_full_recon),AV_OPT_TYPE_INT },

	/*analyse*/
	{ "analyse.intra","",OFFSET(analyse.intra),AV_OPT_TYPE_UINT },
	{ "analyse.inter","",OFFSET(analyse.inter),AV_OPT_TYPE_UINT },
	{ "analyse.b_transform_8x8","",OFFSET(analyse.b_transform_8x8),AV_OPT_TYPE_INT },
	{ "analyse.i_weighted_pred","",OFFSET(analyse.i_weighted_pred),AV_OPT_TYPE_INT },
	{ "analyse.b_weighted_bipred","",OFFSET(analyse.b_weighted_bipred),AV_OPT_TYPE_INT },
	{ "analyse.i_direct_mv_pred","",OFFSET(analyse.i_direct_mv_pred),AV_OPT_TYPE_INT },
	{ "analyse.i_chroma_qp_offset","",OFFSET(analyse.i_chroma_qp_offset),AV_OPT_TYPE_INT },
	{ "analyse.i_me_method","",OFFSET(analyse.i_me_method),AV_OPT_TYPE_INT },
	{ "analyse.i_me_range","",OFFSET(analyse.i_me_range),AV_OPT_TYPE_INT },
	{ "analyse.i_mv_range","",OFFSET(analyse.i_mv_range),AV_OPT_TYPE_INT },
	{ "analyse.i_mv_range_thread","",OFFSET(analyse.i_mv_range_thread),AV_OPT_TYPE_INT },
	{ "analyse.i_subpel_refine","",OFFSET(analyse.i_subpel_refine),AV_OPT_TYPE_INT },
	{ "analyse.b_chroma_me","",OFFSET(analyse.b_chroma_me),AV_OPT_TYPE_INT },
	{ "analyse.b_mixed_references","",OFFSET(analyse.b_mixed_references),AV_OPT_TYPE_INT },
	{ "analyse.i_trellis","",OFFSET(analyse.i_trellis),AV_OPT_TYPE_INT },
	{ "analyse.b_fast_pskip","",OFFSET(analyse.b_fast_pskip),AV_OPT_TYPE_INT },
	{ "analyse.b_dct_decimate","",OFFSET(analyse.b_dct_decimate),AV_OPT_TYPE_INT },
	{ "analyse.i_noise_reduction","",OFFSET(analyse.i_noise_reduction),AV_OPT_TYPE_INT },
	{ "analyse.f_psy_rd","",OFFSET(analyse.f_psy_rd),AV_OPT_TYPE_FLOAT },
	{ "analyse.f_psy_trellis","",OFFSET(analyse.f_psy_trellis),AV_OPT_TYPE_FLOAT },
	{ "analyse.b_psy","",OFFSET(analyse.b_psy),AV_OPT_TYPE_INT },
	{ "analyse.b_mb_info","",OFFSET(analyse.b_mb_info),AV_OPT_TYPE_INT },
	{ "analyse.b_mb_info_update","",OFFSET(analyse.b_mb_info_update),AV_OPT_TYPE_INT },
	{ "analyse.b_psnr","",OFFSET(analyse.b_psnr),AV_OPT_TYPE_INT },
	{ "analyse.b_ssim","",OFFSET(analyse.b_ssim),AV_OPT_TYPE_INT },

	/*rc*/
	{ "rc.i_rc_method","",OFFSET(rc.i_rc_method),AV_OPT_TYPE_INT },
	{ "rc.i_qp_constant","",OFFSET(rc.i_qp_constant),AV_OPT_TYPE_INT },
	{ "rc.i_qp_min","",OFFSET(rc.i_qp_min),AV_OPT_TYPE_INT },
	{ "rc.i_qp_max","",OFFSET(rc.i_qp_max),AV_OPT_TYPE_INT },
	{ "rc.i_qp_step","",OFFSET(rc.i_qp_step),AV_OPT_TYPE_INT },
	{ "rc.i_bitrate","",OFFSET(rc.i_bitrate),AV_OPT_TYPE_INT },
	{ "rc.f_rf_constant","",OFFSET(rc.f_rf_constant),AV_OPT_TYPE_INT },
	{ "rc.f_rf_constant_max","",OFFSET(rc.f_rf_constant_max),AV_OPT_TYPE_FLOAT },
	{ "rc.f_rate_tolerance","",OFFSET(rc.f_rate_tolerance),AV_OPT_TYPE_FLOAT },
	{ "rc.i_vbv_max_bitrate","",OFFSET(rc.i_vbv_max_bitrate),AV_OPT_TYPE_INT },
	{ "rc.i_vbv_buffer_size","",OFFSET(rc.i_vbv_buffer_size),AV_OPT_TYPE_INT },
	{ "rc.f_vbv_buffer_init","",OFFSET(rc.f_vbv_buffer_init),AV_OPT_TYPE_FLOAT },
	{ "rc.f_ip_factor","",OFFSET(rc.f_ip_factor),AV_OPT_TYPE_FLOAT },
	{ "rc.f_pb_factor","",OFFSET(rc.f_pb_factor),AV_OPT_TYPE_FLOAT },
	{ "rc.b_filler","",OFFSET(rc.b_filler),AV_OPT_TYPE_INT },
	{ "rc.i_aq_mode","",OFFSET(rc.i_aq_mode),AV_OPT_TYPE_INT },
	{ "rc.f_aq_strength","",OFFSET(rc.f_aq_strength),AV_OPT_TYPE_FLOAT },
	{ "rc.b_mb_tree","",OFFSET(rc.b_mb_tree),AV_OPT_TYPE_INT },
	{ "rc.i_lookahead","",OFFSET(rc.i_lookahead),AV_OPT_TYPE_INT },
	{ "rc.b_stat_write","",OFFSET(rc.b_stat_write),AV_OPT_TYPE_INT },

	{ "rc.b_stat_read","",OFFSET(rc.b_stat_read),AV_OPT_TYPE_INT },
	{ "rc.f_qcompress","",OFFSET(rc.f_qcompress),AV_OPT_TYPE_FLOAT },
	{ "rc.f_qblur","",OFFSET(rc.f_qblur),AV_OPT_TYPE_FLOAT },
	{ "rc.f_complexity_blur","",OFFSET(rc.f_complexity_blur),AV_OPT_TYPE_FLOAT },
	{ "rc.i_zones","",OFFSET(rc.i_zones),AV_OPT_TYPE_INT },
	/*crop_rect*/


	{ "i_frame_packing","",OFFSET(i_frame_packing),AV_OPT_TYPE_INT },
	{ "b_aud","",OFFSET(b_aud),AV_OPT_TYPE_INT },
	{ "b_repeat_headers","",OFFSET(b_repeat_headers),AV_OPT_TYPE_INT },
	{ "b_annexb","",OFFSET(b_annexb),AV_OPT_TYPE_INT },
	{ "i_sps_id","",OFFSET(i_sps_id),AV_OPT_TYPE_INT },
	{ "b_vfr_input","",OFFSET(b_vfr_input),AV_OPT_TYPE_INT },

	{ "b_pulldown","",OFFSET(b_pulldown),AV_OPT_TYPE_INT },
	{ "i_fps_num","",OFFSET(i_fps_num),AV_OPT_TYPE_UINT },
	{ "i_fps_den","",OFFSET(i_fps_den),AV_OPT_TYPE_UINT },
	{ "i_timebase_num","",OFFSET(i_timebase_num),AV_OPT_TYPE_UINT },
	{ "i_timebase_den","",OFFSET(i_timebase_den),AV_OPT_TYPE_UINT },

	{ "b_tff","",OFFSET(b_tff),AV_OPT_TYPE_INT },
	{ "b_pic_struct","",OFFSET(b_pic_struct),AV_OPT_TYPE_INT },
	{ "b_fake_interlaced","",OFFSET(b_fake_interlaced),AV_OPT_TYPE_INT },
	{ "b_stitchable","",OFFSET(b_stitchable),AV_OPT_TYPE_INT },

	{ "b_opencl","",OFFSET(b_opencl),AV_OPT_TYPE_INT },
	{ "i_opencl_device","",OFFSET(i_opencl_device),AV_OPT_TYPE_INT },
	/* Slicing parameters */
	{ "i_slice_max_size","",OFFSET(i_slice_max_size),AV_OPT_TYPE_INT },
	{ "i_slice_max_mbs","",OFFSET(i_slice_max_mbs),AV_OPT_TYPE_INT },
	{ "i_slice_min_mbs","",OFFSET(i_slice_min_mbs),AV_OPT_TYPE_INT },
	{ "i_slice_count","",OFFSET(i_slice_count),AV_OPT_TYPE_INT },
	{ "i_slice_count_max","",OFFSET(i_slice_count_max),AV_OPT_TYPE_INT }
};
#undef OFFSET

#endif
