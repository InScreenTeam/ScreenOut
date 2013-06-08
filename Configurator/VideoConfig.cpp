#include "stdafx.h"

#include "BaseConfig.h"
#include "VideoConfig.h"

using namespace std;

namespace Config
{
	VideoConfig::VideoConfig()
	{
		Set(defaultFps, defaultBitRate);
	}

	void VideoConfig::Set( DWORD fps, DWORD bitRate )
	{
		SetFPS(fps);
		SetBitRate(bitRate);
	}

	

	void VideoConfig::SetFPS( DWORD value )
	{
		if (value > 0 && value <= MAX_FPS)
			this->fps = value;
		else
			this->fps = defaultFps;
	}

	void VideoConfig::SetBitRate( DWORD value )
	{
		if (value > 0 && value <= MAX_VIDEO_BITRATE)
			this->bitRate = value;
		else
			this->bitRate = defaultBitRate;
	}

}