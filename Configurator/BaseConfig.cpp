#include "stdafx.h"

#include "ConstValues.h"
#include "Record.h"
#include "BaseConfig.h"
#include <cstdio>

namespace Config
{
	BaseConfig::BaseConfig(void)
	{
		records[L"Audio"][L"SampleRate"] = DwordRecord(L"", L"Audio", L"SampleRate",to_wstring(defaultSampleRate));
		records[L"Audio"][L"ByteRate"] = ByteRecord(L"", L"Audio", L"ByteRate", to_wstring(defaultByteRate));
		records[L"Audio"][L"AudioChanels"] = ByteRecord(L"", L"Audio", L"AudioChanels",to_wstring(defaultAudioChanels));
		records[L"Audio"][L"Device"] = DwordRecord(L"", L"Audio", L"Device",  to_wstring(defaultDevice));
		records[L"Audio"][L"BufferLength"] = DwordRecord(L"", L"Audio", L"BufferLength",  to_wstring(defaultBufferLength));
		records[L"Video"][L"FPS"] = DwordRecord(L"", L"Video", L"FPS", to_wstring(defaultFps));
		records[L"Video"][L"BitRate"] = DwordRecord(L"", L"Video", L"BitRate", to_wstring(defaultBitRate));
	}


	BaseConfig::~BaseConfig(void)
	{
	}


}