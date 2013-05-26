#include "Recorder.h"

#include <boost\bind.hpp>
#include <windows.h>

#pragma comment(lib, "lib\\avcodec.lib")
#pragma comment(lib, "lib\\avformat.lib")
#pragma comment(lib, "lib\\swscale.lib")
#pragma comment(lib, "lib\\avutil.lib")

#define RGB_PLANES 4
#define FPS 10
#define CORRECTION 0.5
#define CAPTURE_DELAY 1000 / FPS * CORRECTION

using namespace std;
using namespace boost;

namespace ScreenOut
{
	Recorder::Recorder(void)		
		: ioService(),
		logger("recorder.log"),
		captureTimer(ioService, posix_time::milliseconds(CAPTURE_DELAY))		
	{		
		buffer = new queue<AVPicture*>();
		GetScreenResolution();		
		frameNumber = 0;		
		muxer = new Muxer(width, height);
		capture = new Capture(width, height, 32);
		Initialize();			
		isDone = false;
	}

	void Recorder::Initialize()
	{				
		rgbBuffer = malloc(height * width * RGB_PLANES);		
		if (!rgbBuffer) 
		{
			throw;
		}		
		swsContext = sws_getContext(width, height, AV_PIX_FMT_RGB32,
			width, height, AV_PIX_FMT_YUV420P,SWS_FAST_BILINEAR , NULL,  NULL, NULL);		
		capture->TakePic(0, 0, height, width, NULL);
		capture->SetBitmapInfo();				
		memset(rgbLinesize, 0, 8 * sizeof(int));		
		rgbLinesize[0] = capture->bitmapWidth;		
		prevDuration.zero();
	}

	void Recorder::CaptureTask()
	{
		ioService.run();	
	}

	void Recorder::CaptureScreen()
	{		
		if(!isRecording)
			return;		
		capture->TakePic(0, 0, height, width, rgbBuffer);
		uint8_t *input = reinterpret_cast<uint8_t *>(rgbBuffer);
		AVPicture* yuvPicture = new AVPicture;
		avpicture_alloc(yuvPicture, AV_PIX_FMT_YUV420P, width, height);
		sws_scale(swsContext, &input, rgbLinesize, 0, height, yuvPicture->data, yuvPicture->linesize);
		buffer->push(yuvPicture);
		++frameNumber;
		chrono::duration<double> recorded = captureClock.now() - startPoint;
		logger<< Logger::Level::LOG_INFO << "Captured frame #" << frameNumber 
			<< " at " << recorded.count() << "s. Diff: " << (recorded - prevDuration).count();
		prevDuration = recorded;
		captureTimer.expires_from_now(posix_time::milliseconds(CAPTURE_DELAY));
		captureTimer.async_wait(boost::bind(&Recorder::CaptureScreen, this));
	}

	void Recorder::Multiplex()
	{
		muxer->Initialize();
		int i = 0;
		AVPicture* lastPicture;
		while(true)
		{
			if(buffer->empty())
			{
				if(isRecording)
					continue;				
				break;
			}
			AVPicture* tmpBuffer =  buffer->front();
			buffer->pop();
			muxer->WriteVideoFrame(tmpBuffer);
			++i;
			avpicture_free(tmpBuffer);						
		}			
		logger << Logger::Level::LOG_INFO << "frames count:" << i;
		muxer->Flush();
		isDone = true;
	}

	void Recorder::Start()
	{		
		isRecording = true;
		captureTimer.async_wait(boost::bind(&Recorder::CaptureScreen, this));
		captureThread = thread(&Recorder::CaptureTask, this);	
		SetThreadPriority(captureThread.native_handle(), THREAD_PRIORITY_TIME_CRITICAL);
		muxerThread = thread(&Recorder::Multiplex, this);			
		startPoint = captureClock.now();
	}

	void Recorder::Stop()
	{
		chrono::duration<double> recorded = captureClock.now() - startPoint;
		logger << Logger::Level::LOG_INFO << "Must be recorded:" << recorded.count();
		isRecording = false;	
	}

	bool Recorder::IsRecording()
	{
		return isRecording;
	}
	
	bool Recorder::IsDone()
	{
		return isDone;
	}

	void Recorder::GetScreenResolution()
	{
		RECT desktop;   
		HWND hDesktop = GetDesktopWindow();   
		GetWindowRect(hDesktop, &desktop);	   
		width = desktop.right;
		height = desktop.bottom;		
	}

	Recorder::~Recorder(void)
	{
		delete buffer;		
	}
}
