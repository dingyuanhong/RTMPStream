#pragma once

#include "EvHeade.h"

class Encode
{
public:
	Encode();
	int Open(const char * file);
	void Close();
	virtual int NewVideoStream(AVStream * stream, int width, int height, AVPixelFormat pixFormat);
	virtual int NewAudioStream(AVStream * stream);

	int WriteHeader();
	int WriteVideo(AVPacket *packet);
	int WriteAudio(AVPacket *packet);
	int WriteTrailer();
protected:
	AVFormatContext* formatCtx_ = NULL;
	AVOutputFormat* output_ = NULL;

	AVStream* videoStream_ = NULL;
	AVCodecContext* videoCodecCtx_ = NULL;
	AVCodec* videoCodec_ = NULL;

	AVStream* audioStream_ = NULL;
	AVCodecContext* audioCodecCtx_ = NULL;
	AVCodec* audioCodec_ = NULL;
};