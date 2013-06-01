#pragma once

#include <queue>
#include <boost\thread.hpp>
#include <boost\asio.hpp>

#include "Capture.h"
#include "Muxer.h"
#include "Logger.h"
#include "inttypes.h"

namespace ScreenOut
{
	class Recorder
	{
	public:
		Recorder(void);
		void Start();
		void Stop();
		bool IsRecording();
		bool IsDone();
		~Recorder(void);
	private:
		void Initialize();
		void CaptureScreen();
		void CaptureTask();
		void Multiplex();
		void GetScreenResolution();
	private:
		boost::asio::io_service ioService;
		boost::asio::deadline_timer captureTimer;	
		boost::thread muxerThread;
		boost::thread captureThread;

		std::queue<AVPicture*>* buffer;
		Muxer* muxer;
		Capture* capture;
		Logger logger;
		bool isRecording;
		bool isDone;
	public:
		boost::chrono::high_resolution_clock captureClock;
		boost::chrono::high_resolution_clock::time_point startPoint;
		boost::chrono::duration<double> prevDuration;
	private:
		int width;		
		int height;
		int frameNumber;
		LPVOID rgbBuffer, rgb24Buffer;			
		SwsContext* swsContext;		
		int rgbLinesize[8];
		int yuvLinesize[8];					
	};
}

