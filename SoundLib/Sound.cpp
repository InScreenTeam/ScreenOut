#include "stdafx.h"
#include <assert.h>
#include <math.h>
#include <commctrl.h>

#pragma warning(disable: 4996)

#include "Sound.h"

using namespace std;

DWORD Sound::recordBufferLength;
DWORD Sound::currentLength;
DWORD Sound::tailLength;
queue<LPVOID> Sound::recordQueue;
LPBYTE Sound::recordBuffer;
DWORD Sound::curRecordBufferLength;

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

bool Sound::InitBass( DWORD device, DWORD freq, DWORD flags, HWND win, const GUID *dsguid )
{
	if (!BASS_Init(device, freq, flags, win, dsguid))
		return false;
	return true;
}
	
Sound::~Sound(void)
{
	delete[] recordBuffer;	
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

void write_little_endian(unsigned int word, int num_bytes, FILE *wav_file)
{
	unsigned buf;
	while(num_bytes>0)
	{   buf = word & 0xff;
	fwrite(&buf, 1,1, wav_file);
	num_bytes--;
	word >>= 8;
	}
}

void *tempBuffer;


BOOL CALLBACK Sound::RecordHandler(HRECORD handle,  const void *buffer,  DWORD length, void *user)
{
	LPBYTE p = (LPBYTE)buffer;
	BYTE* temp ;
	for (DWORD i = 0; i < length; ++i)
	{
		recordBuffer[curRecordBufferLength++] = p[i];
		if (curRecordBufferLength == recordBufferLength)
		{
			temp = new BYTE[recordBufferLength];
			memcpy(temp, recordBuffer, recordBufferLength);
			recordQueue.push(temp);
			curRecordBufferLength = 0;
		}
	}
	//fwrite(buffer, 1,length, (FILE*)user);
	return true;
}

void Sound::WriteWavHeader( DWORD dwSeconds, DWORD sampleRate, WORD chanelsCount, WORD bytesPerSample, FILE* wav_file )
{

	if (sampleRate<=0) 
		sampleRate = 44100;
	DWORD samplesNum = sampleRate*dwSeconds;
	DWORD byteRate = sampleRate*chanelsCount*bytesPerSample;
	DWORD dataSize = samplesNum*chanelsCount*bytesPerSample;
// write RIFF header
	fwrite("RIFF", 1, 4, wav_file);
	write_little_endian(36 + dataSize, 4, wav_file);
	fwrite("WAVE", 1, 4, wav_file);
// write fmt  subchunk 
	fwrite("fmt ", 1, 4, wav_file);
	write_little_endian(16, 4, wav_file);   /* SubChunk1Size is 16 */
	write_little_endian(1, 2, wav_file);    /* PCM is format 1 */
	write_little_endian(chanelsCount, 2, wav_file);
	write_little_endian(sampleRate, 4, wav_file);
	write_little_endian(byteRate, 4, wav_file);
	write_little_endian(chanelsCount*bytesPerSample, 2, wav_file);  /* block align */
	write_little_endian(8*bytesPerSample, 2, wav_file);  /* bits/sample */
// write data subchunk
	fwrite("data", 1, 4, wav_file);
	write_little_endian(dataSize, 4, wav_file);
}

bool Sound::RecordStart( DWORD dwSeconds, int deviceNumber, string fileName )
{
	if (recording)
		BASS_ChannelStop(currentRecord);
	currentFile = fopen(DEFAULT_NAME, "wb");
	//WriteWavHeader(dwSeconds, AUDIO_SAMPLE_RATE, AUDIO_CHANELS, AUDIO_BYTES_PER_SAMPLE, currentFile);
	if (!BASS_RecordInit(deviceNumber))
		if(!BASS_RecordInit(DEFAULT_DEVICE_NUMBER))
			return false;
	currentLength = 0;
	recording = true;
	//BASS_SetConfig(BASS_CONFIG_REC_BUFFER, BASS_RECORD_BUFFER_SIZE);
    currentRecord =  BASS_RecordStart(AUDIO_SAMPLE_RATE, AUDIO_CHANELS, BASS_RECORD_PAUSE , (RECORDPROC*)Sound::RecordHandler, currentFile);
	HFX rever = BASS_ChannelSetFX(currentRecord,BASS_FX_DX8_REVERB, 0 );

	BASS_FX_DX8_REVERB_STRUCT reverStruct;
	reverStruct.fInGain = 80;
	reverStruct.fReverbMix = 0;
	reverStruct.fReverbTime = 500;
	reverStruct.fHighFreqRTRatio = 0.5;

	curRecordBufferLength = 0;
	tailLength = 0;
	BASS_FXSetParameters(rever, &reverStruct);
	BASS_ChannelPlay(currentRecord, false);
	return true;
}

bool Sound::RecordStop()
{
	recording = false;
	return ((BASS_ChannelStop(currentRecord) & fclose(currentFile)) > -1);
}

 
/* information about the WAV file format from
 http://ccrma.stanford.edu/courses/422/projects/WaveFormat/ */
void Sound::WriteWav( DWORD dwSeconds, DWORD sampleRate, WORD chanelsCount, WORD bytesPerSample, string fileName, short *data )
{
	FILE *wav_file = fopen(fileName.c_str(), "wb");
    WriteWavHeader(dwSeconds, sampleRate, chanelsCount, bytesPerSample, wav_file);
	DWORD size = sampleRate*dwSeconds*chanelsCount;
    for (DWORD i = 0; i < size	; i++)
	{
       write_little_endian((UINT)(data[i]),bytesPerSample, wav_file);
	}
    fclose(wav_file);
}

void Sound::Test()
{
	const DWORD dwSeconds = 2;
	const DWORD bufferSize = AUDIO_SAMPLE_RATE*AUDIO_CHANELS*AUDIO_BYTES_PER_SAMPLE*dwSeconds;
	short buffer[bufferSize];
	double amplitude = 3200;
	double freq_Hz = 100;
	double phase=0;
	double freq_radians_per_sample = freq_Hz*2*M_PI/AUDIO_SAMPLE_RATE;
//fill buffer with a sine wave 
	for (int i = 0; i < bufferSize; i++)
	{
		phase += freq_radians_per_sample;
		buffer[i] = (int)(amplitude * sin(phase));
	}	
	WriteWav(dwSeconds, AUDIO_SAMPLE_RATE, AUDIO_CHANELS, AUDIO_BYTES_PER_SAMPLE, "test.wav", buffer);
}

LPVOID Sound::GetSample( DWORD time )
{
	
	if (recordQueue.size() > 0)
	{
		LPVOID temp = recordQueue.front();
		recordQueue.pop();
		return temp;
	}
	return NULL;
}

void Sound::QueuePush( LPVOID buffer, DWORD length )
{
	
}


