#pragma once
namespace ScreenOut
{
/*
extern "C" 
{    
	#include <libavformat\avformat.h>
	#include <libavutil\mathematics.h>
	#include <libswscale\swscale.h>
}*/


	class Muxer
	{
	public:
		Muxer(void);
		void Initialize();
		~Muxer(void);
	private:
		void OpenVideo();
		void OpenAudio();
		void AddStream();
		void WriteVideoFrame();
		void WriteAudioFrame();
		void CloseVideo();
		void CloseAudio();
	};
}