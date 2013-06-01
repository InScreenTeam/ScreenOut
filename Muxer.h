#pragma once
extern "C" 
{    
	#include <libavformat\avformat.h>
	#include <libavutil\mathematics.h>
	#include <libswscale\swscale.h>
}

#include "Logger.h"

namespace ScreenOut
{


	class Muxer
	{
	public:
		Muxer(int width, int height);
		void Initialize(char* filename);
		void WriteVideoFrame(AVPicture* buffer);
		void WriteAudioFrame(int16_t* samples);
		double CurrentVideoTimeStamp();
		double CurrentAudioTimeStamp();
		void Flush();
		void Reset();
		~Muxer(void);
	private:
		void OpenVideo();
		void OpenAudio();
		AVStream* AddStream(AVCodec **codec, enum AVCodecID codec_id);		
		void CloseVideo();
		void CloseAudio();
		

	private:
		Logger logger;

		int width;
		int height;

		AVOutputFormat* format;
		AVFormatContext* formatContext;
		AVStream* audioStream; 
		AVStream* videoStream;
		AVCodec* audioCodec;
		AVCodec* videoCodec;		
		AVFrame* frame;
		int audioFrameSize;
		int16_t* samplesBuffer;
		double audioTimestamp; 
		double videoTimestamp;
	};
}