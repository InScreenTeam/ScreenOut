#pragma once
#include "stdafx.h"
#include <queue>

using namespace std;

#define _USE_MATH_DEFINES
#define AUDIO_SAMPLE_RATE 48000
#define AUDIO_CHANELS 2
#define AUDIO_BYTES_PER_SAMPLE 2
#define DEFAULT_NAME "test.wav"
#define DEFAULT_DEVICE_NUMBER -1
#define BASS_RECORD_BUFFER_SIZE 1024


typedef struct {
	float fInGain;
	float fReverbMix;
	float fReverbTime;
	float fHighFreqRTRatio;
} BASS_FX_DX8_REVERB_STRUCT;

class Sound
{
	
	bool recording;
	
	HRECORD currentRecord;
	vector<BASS_DEVICEINFO*> recordDeviceVector;
	FILE* currentFile;
	DWORD currentRecordSize;
public:
	Sound(void);
	bool InitBass(DWORD device, DWORD freq, DWORD flags, HWND win, const GUID *dsguid);
	~Sound(void);
	
	string GetRecordDeviceName (DWORD index) const;
	
	void SetRecordDeviceVector();
	int GetRecordDeviceCount();
	bool SetRecordDevice(DWORD index);
	void RecordFree();
	bool RecordStart(DWORD dwSeconds, int deviceNumber, string fileName);
	static BOOL CALLBACK RecordHandler(HRECORD handle,  const void *buffer,  DWORD length, void *user);
	bool RecordStop();
	void WriteWav( DWORD dwSeconds, DWORD sampleRate, WORD chanelsCount, WORD bytesPerSample, string fileName, short *data );
	void Test();
	void WriteWavHeader( DWORD dwSeconds, DWORD sampleRate, WORD chanelsCount, WORD bytesPerSample, FILE* wav_file);
	LPVOID GetSample(DWORD time);
};

