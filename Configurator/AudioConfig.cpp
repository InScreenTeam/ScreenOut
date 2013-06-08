#include "stdafx.h"

#include "BaseConfig.h"
#include "AudioConfig.h"

using namespace std;

namespace Config
{

	AudioConfig::AudioConfig()
	{
		Set(defaultSampleRate, defaultByteRate, defaultAudioChanels, defaultDevice, defaultBufferLength);
	}

	void AudioConfig::Set( DWORD sampleRate, BYTE byteRate, BYTE audioChanels, int device, DWORD bufferLength )
	{
		SetSampleRate(sampleRate);
		SetByteRate(byteRate);
		SetAudioChanels(audioChanels);
		SetDevice(device);
		SetBufferLength(bufferLength);
	}


	void AudioConfig::SetSampleRate( DWORD value )
	{
		if (value == 44100 || value == 48000 || value == 96000 || value == 192000)
			this->sampleRate = value;
		else 
			this->sampleRate = defaultSampleRate;
	}

	void AudioConfig::SetByteRate( BYTE value )
	{
		if (value > 0)
			this->byteRate = value;
		else
			this->byteRate = defaultByteRate;
	}

	void AudioConfig::SetAudioChanels( BYTE value )
	{
		if (value > 0)
			this->audioChanels = value;
		else
			this->bufferLength = defaultAudioChanels;
	}

	void AudioConfig::SetDevice( int value )
	{
		if (value >= -1)
			this->device = value;
		else
			this->device = defaultDevice;

	}

	void AudioConfig::SetBufferLength( DWORD value )
	{
		if (value > 0)
			this->bufferLength = value;
		else
			this->bufferLength = defaultBufferLength;
	}
}