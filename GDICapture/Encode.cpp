#include "Encode.h"



/**
* 最简单的基于FFmpeg的视频编码器
* Simplest FFmpeg Video Encoder
*
* 雷霄骅 Lei Xiaohua
* leixiaohua1020@126.com
* 中国传媒大学/数字电视技术
* Communication University of China / Digital TV Technology
* http://blog.csdn.net/leixiaohua1020
*
* 本程序实现了YUV像素数据编码为视频码流（H264，MPEG2，VP8等等）。
* 是最简单的FFmpeg视频编码方面的教程。
* 通过学习本例子可以了解FFmpeg的编码流程。
* This software encode YUV420P data to H.264 bitstream.
* It's the simplest video encoding software based on FFmpeg.
* Suitable for beginner of FFmpeg
*/

#include <stdio.h>  

#define __STDC_CONSTANT_MACROS  

#include "Encode.h"

#ifdef USE_NEW_API
static AVCodecContext *CreateCodecContent(AVCodecParameters *codecpar)
{
	AVCodecContext *codecContext = avcodec_alloc_context3(NULL);
	avcodec_parameters_to_context(codecContext, codecpar);
	return codecContext;
}
#endif

Encode::Encode()
{
	formatCtx_ = NULL;
	output_ = NULL;

	videoStream_ = NULL;
	videoCodecCtx_ = NULL;
	videoCodec_ = NULL;

	audioStream_ = NULL;
	audioCodecCtx_ = NULL;
	audioCodec_ = NULL;
}

int Encode::Open(const char * file)
{
	av_register_all();
	//Method1.  
	formatCtx_ = avformat_alloc_context();
	//Guess Format  
	output_ = av_guess_format(NULL, file, NULL);
	formatCtx_->oformat = output_;

	//Method 2.
	//avformat_alloc_output_context2(&formatCtx_, NULL, NULL, file);
	//fmt = formatCtx_->oformat;

	//Open output URL  

	if (!(output_->flags & AVFMT_NOFILE)) {
		if (avio_open(&formatCtx_->pb, file, AVIO_FLAG_WRITE) < 0) {
			printf("Could not open output file '%s'", file);
			return -1;
		}
	}
	return 0;
}

int Encode::NewVideoStream(AVStream * stream,int width,int height, AVPixelFormat pixFormat)
{
	if (stream == NULL)
	{
		return -1;
	}

	videoStream_ = avformat_new_stream(formatCtx_, avcodec_find_decoder(AV_CODEC_ID_H264));

	if (videoStream_ == NULL) {
		return -1;
	}
	videoStream_->time_base = stream->time_base;
	//帧率
	videoStream_->r_frame_rate = stream->r_frame_rate;
	videoStream_->start_time = stream->start_time;
	videoStream_->avg_frame_rate = stream->avg_frame_rate;

	if (videoCodecCtx_) {
		avcodec_close(videoCodecCtx_);
#ifdef USE_NEW_API
		avcodec_free_context(&videoCodecCtx_);
#endif
		videoCodecCtx_ = NULL;
	}
#ifdef USE_NEW_API
	videoCodecCtx_ = CreateCodecContent(videoStream_->codecpar);
#else
	//Param that must set  
	videoCodecCtx_ = videoStream_->codec;
#endif
	AVCodecContext* codecCtx = videoCodecCtx_;
	
#ifdef USE_NEW_API
	avcodec_parameters_to_context(codecCtx, stream->codecpar);
#else
	if(stream->codec != NULL) avcodec_copy_context(codecCtx, stream->codec);
#endif
	codecCtx->time_base = videoStream_->time_base;
	codecCtx->width = width;
	codecCtx->height = height;
	codecCtx->pix_fmt = pixFormat;

	codecCtx->codec_id = output_->video_codec;
	codecCtx->codec_tag = 0;
	codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;

	if ((codecCtx->flags & CODEC_FLAG_GLOBAL_HEADER ) != CODEC_FLAG_GLOBAL_HEADER)
	{
		AVOutputFormat *of = formatCtx_->oformat;
		of->flags &= ~AVFMT_GLOBALHEADER;
	}

	return 0;
}

int Encode::NewAudioStream(AVStream * stream)
{
	if (stream == NULL) return -1;

	audioStream_ = avformat_new_stream(formatCtx_, NULL);

	if (audioStream_ == NULL) {
		return -1;
	}
	audioStream_->time_base = stream->time_base;
	//帧率
	audioStream_->r_frame_rate = stream->r_frame_rate;
	audioStream_->start_time = stream->start_time;
	audioStream_->avg_frame_rate = stream->avg_frame_rate;

	if (audioCodecCtx_) {
		avcodec_close(audioCodecCtx_);
#ifdef USE_NEW_API
		avcodec_free_context(&audioCodecCtx_);
#endif
		audioCodecCtx_ = NULL;
	}
#ifdef USE_NEW_API
	audioCodecCtx_ = CreateCodecContent(audioStream_->codecpar);
#else
	//Param that must set  
	audioCodecCtx_ = audioStream_->codec;
#endif
	AVCodecContext* codecCtx = audioCodecCtx_;

#ifdef USE_NEW_API
	avcodec_parameters_to_context(codecCtx, stream->codecpar);
#else
	avcodec_copy_context(codecCtx, stream->codec);
#endif

	codecCtx->codec_id = output_->audio_codec;
	codecCtx->codec_tag = 0;
	codecCtx->codec_type = AVMEDIA_TYPE_AUDIO;

	if ((codecCtx->flags & CODEC_FLAG_GLOBAL_HEADER) != CODEC_FLAG_GLOBAL_HEADER)
	{
		AVOutputFormat *of = formatCtx_->oformat;
		of->flags &= ~AVFMT_GLOBALHEADER;
	}

	return 0;
}

#ifdef USE_NEW_API
AVCodecParameters *Encode::GetCodecContext(AVMediaType type)
#else
AVCodecContext *Encode::GetCodecContext(AVMediaType type)
#endif
{
	if (formatCtx_ == nullptr) return nullptr;
	for (int i = 0; i < formatCtx_->nb_streams; i++) {
		AVStream * stream = formatCtx_->streams[i];
		if (stream == nullptr) continue;
#ifdef USE_NEW_API
		if (stream->codecpar->codec_type == type) {
			return stream->codecpar;
		}
		else 
#endif
		if (stream->codec->codec_type == type) {
			return stream->codec;
		}
	}
	return nullptr;
}

int Encode::WriteHeader()
{
	if (formatCtx_ == NULL) return -1;
	//Write File Header  
	return avformat_write_header(formatCtx_, NULL);
}

int Encode::WriteVideo(AVPacket *packet)
{
	if (formatCtx_ == NULL) return -1;
	if (videoStream_ == NULL) return -1;
	packet->stream_index = videoStream_->index;
	//int ret = av_interleaved_write_frame(formatCtx_, packet);
	int ret = av_write_frame(formatCtx_, packet);
	return ret;
}

int Encode::WriteAudio(AVPacket *packet)
{
	if (formatCtx_ == NULL) return -1;
	if (audioStream_ == NULL) return -1;
	packet->stream_index = audioStream_->index;
	//int ret = av_interleaved_write_frame(formatCtx_, packet);
	int ret = av_write_frame(formatCtx_, packet);
	return ret;
}

int Encode::WriteTrailer()
{
	if (formatCtx_ == NULL) return -1;
	av_write_trailer(formatCtx_);
	return 0;
}

void Encode::Close()
{
	if (videoCodecCtx_) {
		avcodec_close(videoCodecCtx_);
#ifdef USE_NEW_API
		avcodec_free_context(&videoCodecCtx_);
#endif
		videoCodecCtx_ = NULL;
	}
	if (audioCodecCtx_) {
		avcodec_close(audioCodecCtx_);
#ifdef USE_NEW_API
		avcodec_free_context(&audioCodecCtx_);
#endif
		audioCodecCtx_ = NULL;
	}
	if (formatCtx_) {
		avio_closep(&formatCtx_->pb);
		avformat_free_context(formatCtx_);
	}

	videoStream_ = NULL;
	videoCodecCtx_ = NULL;
	videoCodec_ = NULL;

	audioStream_ = NULL;
	audioCodecCtx_ = NULL;
	audioCodec_ = NULL;

	output_ = NULL;
	formatCtx_ = NULL;
}