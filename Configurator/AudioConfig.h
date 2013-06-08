#pragma once

#include "BaseConfig.h"

#define MAX_SAMPLE_RATE (192000)

using namespace std;

namespace Config
{
	class AudioConfig: BaseConfig
	{
		DWORD sampleRate;
		BYTE byteRate;
		BYTE audioChanels;
		int device;
		DWORD bufferLength;
	public:
		static const DWORD defaultSampleRate = 48000;
		static const BYTE defaultByteRate = 2;
		static const BYTE defaultAudioChanels = 2;
		static const int defaultDevice = -1;
		static const DWORD defaultBufferLength = 4096;

		AudioConfig();

		void Set(DWORD sampleRate, BYTE byteRate, BYTE audioChanels, int device, DWORD bufferLength);
		void SetSampleRate(DWORD value);
		void SetByteRate(BYTE value);
		void SetAudioChanels(BYTE value);
		void SetDevice(int value);
		void SetBufferLength(DWORD value);
		bool ToVector(vector<Record>& v);
	};
}