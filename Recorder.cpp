#include "Recorder.h"
#include <boost\bind.hpp>


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
		rgbBuffer = _aligned_malloc(height * width * RGB_PLANES, 16);		
		rgb24Buffer = _aligned_malloc(height * width * 3, 16);
		if (!rgbBuffer) 
		{
			throw;
		}		
		swsContext = sws_getContext(width, height, AV_PIX_FMT_RGB32,
			width, height, AV_PIX_FMT_YUV420P,SWS_FAST_BILINEAR , NULL,  NULL, NULL);		
		capture->TakePic(height, width, NULL);
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
		capture->TakePic(height, width, rgbBuffer);
		AdjustCaptureDelay();
		uint8_t *input = reinterpret_cast<uint8_t *>(rgbBuffer);
		AVPicture* yuvPicture = new AVPicture;
		avpicture_alloc(yuvPicture, AV_PIX_FMT_YUV420P, width, height);
		int i = 0;
		int j = 0;
		int ui = width*height;
		int vi = ui + ui/4;
		int r,g,b;
		/*char maskTemp[] = {0,1,2,4, 5,6,8,9, 10,12,13,14, -1,-1,-1,-1};
		char *mask = (char*)_aligned_malloc(16,16);
		memcpy(mask, maskTemp, 16);
		BYTE *src = (BYTE*)rgbBuffer;
		BYTE *dst = (BYTE*)rgb24Buffer;
		__asm{
			mov eax, src;
			movdqa xmm0, [eax];
			movdqa xmm1, [eax+16];  
			for (UINT i = 0; i < width*height; i += 16) 
			{
				 __m128i sa = _mm_shuffle_epi8(_mm_load_si128(src), mask);
				 __m128i sb = _mm_shuffle_epi8(_mm_load_si128(src + 1), mask);
				 __m128i sc = _mm_shuffle_epi8(_mm_load_si128(src + 2), mask);
				 __m128i sd = _mm_shuffle_epi8(_mm_load_si128(src + 3), mask);
				_mm_store_si128(dst, _mm_or_si128(sa, _mm_slli_si128(sb, 12)));
				_mm_store_si128(dst + 1, _mm_or_si128(_mm_srli_si128(sb, 4), _mm_slli_si128(sc, 8)));
				_mm_store_si128(dst + 2, _mm_or_si128(_mm_srli_si128(sc, 8), _mm_slli_si128(sd, 4)));
				src += 4;
				dst += 3;
			}
			
		}*/

		for (int h = 0; h < height; ++h)
		{
			for (int w = 0; w < width; ++w)
			{
				r = input[i];
				g = input[i+1];
				b = input[i+2];
				yuvPicture->data[0][i/4] = ((  66*r + 129*g +  25*b + 128) >> 8)  + 16;
				if (h%2 == 0 && w%2 == 0)
				{
					yuvPicture->data[1][j] = ( (-38*r - 74*g + 112*b + 128) >> 8 ) + 128;
					yuvPicture->data[2][j] = ( (112*r - 94*g - 18*b + 128) >> 8 ) + 128;
					j++;

				}
				i+=4;	
			}
			
		}
		//sws_scale(swsContext, &input, rgbLinesize, 0, height, yuvPicture->data, yuvPicture->linesize);
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
