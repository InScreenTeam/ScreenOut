#pragma once

#include <queue>

extern "C"
{    
	#include <libavformat\avformat.h>
	#include <libavutil\mathematics.h>
	#include <libswscale\swscale.h>
}

namespace ScreenOut
{
	class Recorder
	{
	public:
		Recorder(void);
		void Start();
		void Stop();
		~Recorder(void);
	private:
		void ConvertBitmap();
	private:
		std::queue<AVPicture> buffer;		
	};
}

