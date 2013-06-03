#pragma once

#include <queue>
#include <boost\thread.hpp>
#include <boost\asio.hpp>

#define FPS 10

#include "Capture.h"
#include "Muxer.h"
#include "Logger.h"
#include "SoundLib\Sound.h"
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
		void AdjustCaptureDelay();
	private:
		boost::asio::io_service ioService;
		boost::asio::deadline_timer captureTimer;	
		boost::thread muxerThread;
		boost::thread captureThread;
		int captureDelay;		

		std::queue<AVPicture*>* buffer;
		Muxer* muxer;
		Capture* capture;
		Sound* sound;
		Logger logger;
		bool isRecording;
		bool isDone;
	private:
		boost::chrono::high_resolution_clock captureClock;
		boost::chrono::high_resolution_clock::time_point startCapture;
		boost::chrono::duration<double> currentCaptureTime;
		boost::chrono::duration<double> previousCaptureTime;
	private:
		int width;		
		int height;
		int frameNumber;
		double averageDelay;
		LPVOID rgbBuffer;
		LPVOID rgb24Buffer;			
		SwsContext* swsContext;		
		int rgbLinesize[8];
		int yuvLinesize[8];					
	};
}

