#pragma once
#include "stdafx.h"
#include <windows.h>
#include "BaseConfig.h"
#include "..\..\..\..\Study\boost_1_53_0\boost\thread\win32\thread_primitives.hpp"

using namespace std;

namespace Config
{

#define MAX_FPS (25)
#define MAX_SAMPLE_RATE (192000)
#define MAX_VIDEO_BITRATE (40000000)

	struct 
	{
		string Comments;
		char Commented;
		string Section;
		string Key;
		string Value;
		string Default;
	} typedef Record;

	class AudioConfig: BaseConfig
	{
	public:
		DWORD sampleRate;
		BYTE byteRate;
		BYTE audioChanels;
		int device;
		DWORD bufferLength;
		static const DWORD defaultSampleRate = 48000;
		static const BYTE defaultByteRate = 2;
		static const BYTE defaultAudioChanels = 2;
		static const int defaultDevice = -1;
		static const DWORD defaultBufferLength = 4096;

		AudioConfig();

		void Set(DWORD sampleRate, BYTE byteRate, BYTE audioChanels, int device, DWORD bufferLength);
		bool ToVector(vector<Record>& v);
	};

	class VideoConfig: BaseConfig
	{
	public:
		DWORD fps;
		static const DWORD defaultFps = 10;
		DWORD bitRate;
		static const DWORD defaultBitRate = 400000;

		VideoConfig();

		void Set(DWORD fps, DWORD bitRate);
		bool ToVector(vector<Record>& v);
	};
}