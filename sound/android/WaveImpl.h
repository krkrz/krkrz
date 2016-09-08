//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Wave Player implementation
//---------------------------------------------------------------------------
#ifndef WaveImplH
#define WaveImplH


#include "WaveIntf.h"
#include "WaveLoopManager.h"


//---------------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------------

#define TVP_WSB_ACCESS_FREQ (8)  // wave sound buffer access frequency (hz)

#define TVP_TIMEOFS_INVALID_VALUE ((tjs_int)(- 2147483648i64)) // invalid value for 32bit time offset

//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// tTJSNI_WaveSoundBuffer : Wave Native Instance
//---------------------------------------------------------------------------
class tTVPWaveLoopManager;
class tTVPWaveSoundBufferDecodeThread;
class tTJSNI_WaveSoundBuffer : public tTJSNI_BaseWaveSoundBuffer
{
	typedef  tTJSNI_BaseWaveSoundBuffer inherited;
	tTJSCriticalSection BufferCS;

public:
	tTJSNI_WaveSoundBuffer();
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param,
		iTJSDispatch2 *tjs_obj);
	void TJS_INTF_METHOD Invalidate();

public:
	void FreeDirectSoundBuffer(bool disableevent = true)
	{
	}

public:
	tTJSCriticalSection & GetBufferCS() { return BufferCS; }

public:
	bool ThreadCallbackEnabled;

public:
	bool FillL2Buffer(bool firstwrite, bool fromdecodethread);

public:
	bool FillBuffer(bool firstwrite = false, bool allowpause = true);

public:
	tjs_int FireLabelEventsAndGetNearestLabelEventStep(tjs_int64 tick);
	tjs_int GetNearestEventStep();
	void FlushAllLabelEvents();

public:
	void Play();
	void Stop();

	bool GetPaused() const { return false; }
	void SetPaused(bool b);

	tjs_int GetBitsPerSample() const { return 0; }
	tjs_int GetChannels() const { return 0; }

public:
	void Open(const ttstr & storagename);

public:
	void SetLooping(bool b);
	bool GetLooping() const { return false; }

    tjs_uint64 GetSamplePosition();
	void SetSamplePosition(tjs_uint64 pos);

    tjs_uint64 GetPosition();
	void SetPosition(tjs_uint64 pos);

	tjs_uint64 GetTotalTime();

public:
	void SetVolumeToSoundBuffer();

public:
	void SetVolume(tjs_int v);
	tjs_int GetVolume() const { return 0; }
	void SetVolume2(tjs_int v);
	tjs_int GetVolume2() const { return 0; }
	void SetPan(tjs_int v);
	tjs_int GetPan() const { return 0; }
	static void SetGlobalVolume(tjs_int v);
	static tjs_int GetGlobalVolume() { return 0; }
	static void SetGlobalFocusMode(tTVPSoundGlobalFocusMode b);
	static tTVPSoundGlobalFocusMode GetGlobalFocusMode() { return sgfmNeverMute; }

public:
	void SetPos(float x, float y, float z);
	void SetPosX(float v);
	float GetPosX() const {return 0;}
	void SetPosY(float v);
	float GetPosY() const {return 0;}
	void SetPosZ(float v);
	float GetPosZ() const {return 0;}

public:
	tjs_int GetFrequency() const { return 0; }
	void SetFrequency(tjs_int freq);

	//-- visualization stuff ----------------------------------------------
public:
	void SetUseVisBuffer(bool b);
	bool GetUseVisBuffer() const { return false; }

protected:
	void TimerBeatHandler(); // override

	void ResetVisBuffer(); // reset or recreate visualication buffer
	void DeallocateVisBuffer();

	void CopyVisBuffer(tjs_int16 *dest, const tjs_uint8 *src,
		tjs_int numsamples, tjs_int channels);
public:
	tjs_int GetVisBuffer(tjs_int16 *dest, tjs_int numsamples, tjs_int channels,
		tjs_int aheadsamples);
};
//---------------------------------------------------------------------------

extern void TVPWaveSoundBufferCommitSettings();
#endif
