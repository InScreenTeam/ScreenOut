#pragma once

namespace Config
{
	//Audio
		static const DWORD defaultSampleRate = 48000;
		static const BYTE defaultByteRate = 2;
		static const BYTE defaultAudioChanels = 2;
		static const int defaultDevice = -1;
		static const DWORD defaultBufferLength = 4096;
	//Video
		static const DWORD defaultFps = 10;
		static const DWORD defaultBitRate = 400000;
}