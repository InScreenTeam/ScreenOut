#include "Recorder.h"

#include <boost\bind.hpp>
#include <windows.h>

#pragma comment(lib, "lib\\avcodec.lib")
#pragma comment(lib, "lib\\avformat.lib")
#pragma comment(lib, "lib\\swscale.lib")
#pragma comment(lib, "lib\\avutil.lib")

#define RGB_PLANES 4
#define CORRECTION 0.4
#define CAPTURE_DELAY 1000 / FPS

using namespace std;
using namespace boost;

namespace ScreenOut
{
	Recorder::Recorder(void)		
		: ioService(),
		logger("recorder.log"),
		captureDelay(CAPTURE_DELAY),
		captureTimer(ioService, posix_time::milliseconds(captureDelay))		
	{		
		buffer = new queue<AVPicture*>();
		GetScreenResolution();		
		frameNumber = 0;		
		muxer = new Muxer(width, height);
		capture = new Capture(width, height, 32);
		averageDelay = 0;
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
		currentCaptureTime = previousCaptureTime = previousCaptureTime.zero();		
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
		AdjustCaptureDelay();
		uint8_t *input = reinterpret_cast<uint8_t *>(rgbBuffer);
		AVPicture* yuvPicture = new AVPicture;
		avpicture_alloc(yuvPicture, AV_PIX_FMT_YUV420P, width, height);
		sws_scale(swsContext, &input, rgbLinesize, 0, height, yuvPicture->data, yuvPicture->linesize);
		buffer->push(yuvPicture);		
		++frameNumber;				
		captureTimer.expires_from_now(posix_time::milliseconds(captureDelay));
		captureTimer.async_wait(boost::bind(&Recorder::CaptureScreen, this));
	}

	void Recorder::Multiplex()
	{
		muxer->Initialize("test.mp4");
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
		startCapture = captureClock.now();
		captureThread = thread(&Recorder::CaptureTask, this);	
		//SetThreadPriority(captureThread.native_handle(), THREAD_PRIORITY_TIME_CRITICAL);
		muxerThread = thread(&Recorder::Multiplex, this);	
		//SetThreadPriority(muxerThread.native_handle(), THREAD_PRIORITY_TIME_CRITICAL);
	}

	void Recorder::Stop()
	{				
		isRecording = false;	
		logger << Logger::Level::LOG_INFO << "Must be recorded:" << currentCaptureTime.count()
			<< "; Average delay: " << averageDelay / frameNumber;
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

	void Recorder::AdjustCaptureDelay()
	{		
		currentCaptureTime = captureClock.now() - startCapture;
		double realDelay = (currentCaptureTime - previousCaptureTime).count() * 1000;
		int correction = (int)ceil(abs(CAPTURE_DELAY - realDelay));
		captureDelay += realDelay > CAPTURE_DELAY ? -correction : correction ;
		averageDelay +=realDelay;
		logger<< Logger::Level::LOG_INFO << "Frame #" << frameNumber 
			<< " at" << currentCaptureTime.count() 
			<< "s. Real delay:" << realDelay
			<<"; Correction:" << correction
			<< "; Adjusted delay:" << captureDelay;
		previousCaptureTime = currentCaptureTime;
	}

	Recorder::~Recorder(void)
	{
		delete buffer;		
	}
}
