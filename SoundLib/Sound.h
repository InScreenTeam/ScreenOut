#ifndef SOUND_H
#define SOUND_H
#include "stdafx.h"

#include "FXStructs.h"

#define AUDIO_SAMPLE_RATE 48000
#define AUDIO_CHANELS 2
#define AUDIO_BYTES_PER_SAMPLE 2
#define DEFAULT_NAME "test1.wav"
#define DEFAULT_DEVICE_NUMBER -1
#define BUFFER_LENGTH 4096

using namespace std;


class Sound
{
	static DWORD currentLength;
	static DWORD curRecordBufferLength;
	static DWORD recordBufferLength;
	static LPBYTE recordBuffer;
	static queue<LPVOID> recordQueue;

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
	bool RecordStart(int deviceNumber);
	static BOOL CALLBACK QueueRecordHandler(HRECORD handle, const void *buffer, DWORD length, void *user);
	static BOOL CALLBACK WavRecordHandler(HRECORD handle, const void *buffer, DWORD length, void *user);
	bool RecordStop();
	void WriteWav( DWORD dwSeconds, DWORD sampleRate, WORD chanelsCount, WORD bytesPerSample, string fileName );
	void Test();
	void WriteWavHeader( DWORD dwSeconds, DWORD sampleRate, WORD chanelsCount, WORD bytesPerSample, FILE* wav_file);
	static bool GetSample(void *const outputBuffer);
	static void RecordQueuePush(LPVOID buffer, DWORD length);
};
#endif
