#ifndef FXStructs_H
#define FXStructs_H

typedef struct {
	float fInGain;
	float fReverbMix;
	float fReverbTime;
	float fHighFreqRTRatio;
} BASS_FX_DX8_REVERB_STRUCT;

typedef struct {
	float fWetDryMix;
	float fDepth;
	float fFeedback;
	float fFrequency;
	DWORD lWaveform;
	float fDelay;
	DWORD lPhase;
} BASS_DX8_CHORUS_STRUCT;

typedef struct {
	float fGain;
	float fAttack;
	float fRelease;
	float fThreshold;
	float fRatio;
	float fPredelay;
} BASS_DX8_COMPRESSOR_STRUCT;

typedef struct {
	float fGain;
	float fEdge;
	float fPostEQCenterFrequency;
	float fPostEQBandwidth;
	float fPreLowpassCutoff;
} BASS_DX8_DISTORTION_STRUCT;

typedef struct {
	float fWetDryMix;
	float fFeedback;
	float fLeftDelay;
	float fRightDelay;
	BOOL lPanDelay;
} BASS_DX8_ECHO_STRUCT;

typedef struct {
	float fWetDryMix;
	float fDepth;
	float fFeedback;
	float fFrequency;
	DWORD lWaveform;
	float fDelay;
	DWORD lPhase;
} BASS_DX8_FLANGER_STRUCT;

typedef struct {
	DWORD dwRateHz;
	DWORD dwWaveShape;
} BASS_DX8_GARGLE_STRUCT;

typedef struct {
	int lRoom;
	int lRoomHF;
	float flRoomRolloffFactor;
	float flDecayTime;
	float flDecayHFRatio;
	int lReflections;
	float flReflectionsDelay;
	int lReverb;
	float flReverbDelay;
	float flDiffusion;
	float flDensity;
	float flHFReference;
} BASS_DX8_I3DL2REVERB_STRUCT;

//EQ
typedef struct {
	float fCenter;
	float fBandwidth;
	float fGain;
} BASS_DX8_PARAMEQ_STRUCT;

#endif