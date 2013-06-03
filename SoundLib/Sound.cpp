#include "stdafx.h"
#include <assert.h>
#include <math.h>
#include <commctrl.h>

#pragma warning(disable: 4996)

#include "Sound.h"

using namespace std;

DWORD Sound::recordBufferLength;
DWORD Sound::currentLength;
queue<LPVOID> Sound::recordQueue;
LPBYTE Sound::recordBuffer;
DWORD Sound::curRecordBufferLength;

void WriteLittleEndian(unsigned int word, int num_bytes, FILE *wav_file)
{
	unsigned buf;
	while(num_bytes>0)
	{   buf = word & 0xff;
	fwrite(&buf, 1,1, wav_file);
	num_bytes--;
	word >>= 8;
	}
}


Sound::Sound(void)
{
	SetRecordDeviceVector();
	if (GetRecordDeviceCount() == 0)
	{
		std::cout << "device count = 0\n";
		return;
	}
	if (!InitBass(-1, AUDIO_SAMPLE_RATE, NULL, NULL, NULL))
	{
		std::cout << "init device 0 failed\n";
		return;
	}
	std::cout<<"WOOOOOORKS!";
	recording = false;
	recordBufferLength = BUFFER_LENGTH;
	recordBuffer = new BYTE[recordBufferLength];
	
	
}
Sound::~Sound(void)
{
	delete[] recordBuffer;	
}
void Sound::SetRecordDeviceVector()
{
	BASS_DEVICEINFO *info = new BASS_DEVICEINFO();
	for (UINT i = 0; i < recordDeviceVector.size(); ++i)
		delete recordDeviceVector[i];
	int n = 0;
	while (BASS_RecordGetDeviceInfo(n, info))
	{
		recordDeviceVector.push_back(info);
		info = new BASS_DEVICEINFO();
		++n;
	}	
}
bool Sound::InitBass( DWORD device, DWORD freq, DWORD flags, HWND win, const GUID *dsguid )
{
	if (!BASS_Init(device, freq, flags, win, dsguid))
		return false;
	return true;
}
	
int Sound::GetRecordDeviceCount()
{
	return recordDeviceVector.size();
}
std::string Sound::GetRecordDeviceName( DWORD index ) const
{
	return string(recordDeviceVector[index]->name);
}
bool Sound::SetRecordDevice( DWORD index )
{
	return (BASS_RecordSetDevice(index) > -1);
}
void Sound::RecordFree()
{
	BASS_RecordFree();
}

BOOL CALLBACK Sound::QueueRecordHandler( HRECORD handle, const void *buffer, DWORD length, void *user )
{
	LPBYTE p = (LPBYTE)buffer;
	for (DWORD i = 0; i < length; ++i)
	{
		recordBuffer[curRecordBufferLength++] = p[i];
		if (curRecordBufferLength == recordBufferLength)
		{
			RecordQueuePush(recordBuffer, recordBufferLength);
			curRecordBufferLength = 0;
		}
	}
	return true;
}
BOOL CALLBACK Sound::WavRecordHandler( HRECORD handle, const void *buffer, DWORD length, void *user )
{
	fwrite(buffer, 1, length, (FILE*)user);
	return 1;
}

//information about the WAV file format from http://ccrma.stanford.edu/courses/422/projects/WaveFormat/
void Sound::WriteWavHeader( DWORD dwSeconds, DWORD sampleRate, WORD chanelsCount, WORD bytesPerSample, FILE* wav_file )
{
	if (sampleRate<=0) 
		sampleRate = AUDIO_SAMPLE_RATE;
	DWORD samplesNum = sampleRate*dwSeconds;
	DWORD byteRate = sampleRate*chanelsCount*bytesPerSample;
	DWORD dataSize = samplesNum*chanelsCount*bytesPerSample;
// write RIFF header
	fwrite("RIFF", 1, 4, wav_file);
	WriteLittleEndian(36 + dataSize, 4, wav_file);
	fwrite("WAVE", 1, 4, wav_file);
// write fmt  subchunk 
	fwrite("fmt ", 1, 4, wav_file);
	WriteLittleEndian(16, 4, wav_file);   /* SubChunk1Size is 16 */
	WriteLittleEndian(1, 2, wav_file);    /* PCM is format 1 */
	WriteLittleEndian(chanelsCount, 2, wav_file);
	WriteLittleEndian(sampleRate, 4, wav_file);
	WriteLittleEndian(byteRate, 4, wav_file);
	WriteLittleEndian(chanelsCount*bytesPerSample, 2, wav_file);  /* block align */
	WriteLittleEndian(8*bytesPerSample, 2, wav_file);  /* bits/sample */
// write data subchunk
	fwrite("data", 1, 4, wav_file);
	WriteLittleEndian(dataSize, 4, wav_file);
}

bool Sound::RecordStart( int deviceNumber )
{
	if (recording)
		BASS_ChannelStop(currentRecord);
	if (!BASS_RecordInit(deviceNumber) && !BASS_RecordInit(DEFAULT_DEVICE_NUMBER))
			return false;
	currentLength = 0;
	recording = true;
    currentRecord =  BASS_RecordStart(AUDIO_SAMPLE_RATE, AUDIO_CHANELS, BASS_RECORD_PAUSE , (RECORDPROC*)Sound::QueueRecordHandler, currentFile);
	curRecordBufferLength = 0;
	BASS_ChannelPlay(currentRecord, false);
	return true;
}
bool Sound::RecordStop()
{
	recording = false;
	return ((BASS_ChannelStop(currentRecord) ) > -1);
}
void Sound::RecordQueuePush( LPVOID buffer, DWORD length )
{
	PBYTE temp = new BYTE[recordBufferLength];
	memcpy(temp, buffer, recordBufferLength);
	recordQueue.push(temp);
}
//call after RecordStart & in the end of record call RecordStop
bool Sound::GetSample(void * const outputBuffer)
{
	if (recordQueue.size() > 0)
	{
		LPVOID temp = recordQueue.front();
		memcpy(outputBuffer, temp, recordBufferLength);
		recordQueue.pop();
		delete [] temp;
		return true;
	}
	return false;
}





//not working!needed for future
void Sound::WriteWav( DWORD dwSeconds, DWORD sampleRate, WORD chanelsCount, WORD bytesPerSample, string fileName )
{
	FILE *wav_file = fopen(fileName.c_str(), "wb");
    WriteWavHeader(dwSeconds, sampleRate, chanelsCount, bytesPerSample, wav_file);
	DWORD size = sampleRate*dwSeconds*chanelsCount;
    fclose(wav_file);
}
//not working! needed for future
void SetEffects(DWORD record)
{
	HFX rever = BASS_ChannelSetFX(record,BASS_FX_DX8_REVERB, 0 );
	BASS_FX_DX8_REVERB_STRUCT reverStruct;
	reverStruct.fInGain = 80;
	reverStruct.fReverbMix = 0;
	reverStruct.fReverbTime = 500;
	reverStruct.fHighFreqRTRatio = 0.5;
	BASS_FXSetParameters(rever, &reverStruct);
}
//no working
void Sound::Test()
{
	const DWORD dwSeconds = 2;
	const DWORD bufferSize = AUDIO_SAMPLE_RATE*AUDIO_CHANELS*AUDIO_BYTES_PER_SAMPLE*dwSeconds;
	short buffer[bufferSize];
	double amplitude = 3200;
	double freq_Hz = 100;
	double phase=0;
	double freq_radians_per_sample = freq_Hz*2*M_PI/AUDIO_SAMPLE_RATE;
}		







