#include "stdafx.h"
#include <windows.h>
#include "BaseConfig.h"
#include "ConfigClasses.h"

using namespace std;

namespace Config
{

	AudioConfig::AudioConfig()
	{
		Set(defaultSampleRate, defaultByteRate, defaultAudioChanels, defaultDevice, defaultBufferLength);
	}

	void AudioConfig::Set( DWORD sampleRate, BYTE byteRate, BYTE audioChanels, int device, DWORD bufferLength )
	{
		if (sampleRate != 0 && sampleRate <= 192000)
			this->sampleRate = sampleRate;
		else sampleRate = defaultSampleRate;
		if (byteRate != 0)
			this->byteRate = byteRate;
		else
			this->byteRate = defaultByteRate;
		if (audioChanels != 0)
			this->audioChanels = audioChanels;
		else
			this->bufferLength = defaultAudioChanels;
		if (device > -1)
			this->device = device;
		else
			this->device = defaultDevice;
		if (bufferLength > 0)
			this->bufferLength = bufferLength;
		else
			this->bufferLength = defaultBufferLength;
	}

	bool AudioConfig::ToVector(vector<Record>& v )
	{
		Record section  = {"#Audio preferences;",'\n' ,"Audio", "" ,""};
		Record sR = {"", 0, "Audio", "SampleRate", to_string(sampleRate), to_string(defaultSampleRate)};
		Record bR = {"", 0, "Audio", "ByteRate", to_string(byteRate), to_string(defaultByteRate)};
		Record aC = {"", 0, "Audio", "AudioChanels", to_string(audioChanels), to_string(defaultAudioChanels)};
		Record d =  {"", 0, "Audio", "Device", to_string(device), to_string(defaultDevice)};
		Record bL = {"", 0, "Audio", "BufferLength", to_string(bufferLength), to_string(defaultBufferLength)};
		v.push_back(section);
		v.push_back(sR);
		v.push_back(bR);
		v.push_back(aC);
		v.push_back(d);
		v.push_back(bL);
		return true;
	}


	VideoConfig::VideoConfig()
	{
		Set(defaultFps, defaultBitRate);
	}

	void VideoConfig::Set( DWORD fps, DWORD bitRate )
	{
		if (fps > 0 && fps <= MAX_FPS)
			this->fps = fps;
		else
			this->fps = defaultFps;
		if (bitRate > 0 && bitRate <= MAX_VIDEO_BITRATE)
			this->bitRate = fps;
		else
			this->bitRate = bitRate;
	}

	bool VideoConfig::ToVector( vector<Record>& v )
	{
		Record section  = {"#Video preferences;",'\n' ,"Video", "" ,""};
		Record f =  {"", 0, "Video", "FPS", to_string(fps), to_string(defaultFps)};
		Record bR = {"", 0, "Video", "BitRate", to_string(bitRate), to_string(defaultBitRate)};
		v.push_back(section);
		v.push_back(f);
		v.push_back(bR);
		return true;
	};

}