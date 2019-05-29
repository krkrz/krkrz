//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Wave Player implementation
//---------------------------------------------------------------------------
#ifndef PortAudioImplH
#define PortAudioImplH


#include "WaveIntf.h"
#include "WaveLoopManager.h"
#include "AudioDevice.h"
#include "SoundPlayer.h"

//---------------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------------
#define TVP_TIMEOFS_INVALID_VALUE ((tjs_int)(0x80000000)) // invalid value for 32bit time offset

//---------------------------------------------------------------------------
/*
struct WaveFormat
{
    int8_t       RIFF[4];
    uint32_t     TotalSize;
    int8_t       Fmt[8];
    uint32_t     FmtSize;
    uint16_t     Format;
    uint16_t     Channel;
    uint32_t     Rate;
    uint32_t     AvgByte;
    uint16_t     Block;
    uint16_t     BitPerSample;
    int8_t       Data[4];
    uint32_t     DataSize;
};
struct tTVPWaveFormat
{
	tjs_uint SamplesPerSec; // sample granule per sec
	tjs_uint Channels;
	tjs_uint BitsPerSample; // per one sample
	tjs_uint BytesPerSample; // per one sample
	tjs_uint64 TotalSamples; // in sample granule; unknown for zero
	tjs_uint64 TotalTime; // in ms; unknown for zero
	tjs_uint32 SpeakerConfig; // bitwise OR of SPEAKER_* constants
	bool IsFloat; // true if the data is IEEE floating point
	bool Seekable;
};
*/
//---------------------------------------------------------------------------
// tTJSNI_PortAudioSoundBuffer : Wave Native Instance
//---------------------------------------------------------------------------
class tTVPWaveLoopManager;
class tTJSNI_QueueSoundBuffer : public tTJSNI_BaseWaveSoundBuffer
{
	typedef  tTJSNI_BaseWaveSoundBuffer inherited;
	tTJSCriticalSection BufferCS;

	tTVPWaveDecoder * Decoder;
	class tTVPSoundDecodeThread * Thread;
	tTVPSoundPlayer Player;

	tTVPWaveFormat InputFormat;
	bool Looping;

	std::vector<tTVPWaveLabel> LabelEventQueue;

	bool BufferPlaying;	// decode threadが走って、queueに入れていってる状態
	bool UseVisBuffer;

	// double buffering
	static const tjs_uint BufferCount = 2;
	class tTVPSoundSamplesBuffer* Buffer[BufferCount];

	tjs_uint BufferSize;

	tjs_int64 LastCheckedDecodePos; // last sured position (-1 for not checked) and 
	tjs_uint64 LastCheckedTick; // last sured tick time

	tjs_int Volume;
	tjs_int Volume2;
	tjs_int Frequency;
	static tjs_int GlobalVolume;
	static tTVPSoundGlobalFocusMode GlobalFocusMode;
	tjs_int Pan; // -100000 .. 0 .. 100000

	void ResetSamplePositions();
	void Clear();

	void StartPlay();
	void StopPlay();

	void CreateSoundBuffer();
	void ResetLastCheckedDecodePos();
public:
	bool ThreadCallbackEnabled;

	tTJSNI_QueueSoundBuffer();
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param,
		iTJSDispatch2 *tjs_obj);
	void TJS_INTF_METHOD Invalidate();

	void DestroySoundBuffer();
	void ReleaseSoundBuffer( bool disableevent = true );

	tTJSCriticalSection & GetBufferCS() { return BufferCS; }

	void PushPlayStream( class tTVPSoundSamplesBuffer* buffer );
	void ReleasePlayedSamples( class tTVPSoundSamplesBuffer* buffer, bool continued );

	tjs_int FireLabelEventsAndGetNearestLabelEventStep(tjs_int64 tick);
	tjs_int GetNearestEventStep();
	void FlushAllLabelEvents();

//	bool DoDecode(); // for tTVPSoundDecodeThread
	void Update();	// for tTVPSoundEventThread(FillBuffer)

	tjs_uint Decode( void *buffer, tjs_uint bufsamplelen, tTVPWaveSegmentQueue & segments );

	virtual void Open(const ttstr & storagename) override;
	virtual void Play() override;
	virtual void Stop() override;

	virtual bool GetPaused() const override;
	virtual void SetPaused(bool b) override;

	virtual tjs_int GetBitsPerSample() const override { return InputFormat.BitsPerSample; }
	virtual tjs_int GetChannels() const override { return InputFormat.Channels; }

	virtual void SetLooping(bool b) override;
	virtual bool GetLooping() const override { return Looping; }

    virtual tjs_uint64 GetSamplePosition() override;
	virtual void SetSamplePosition(tjs_uint64 pos) override;

    virtual tjs_uint64 GetPosition() override;
	virtual void SetPosition(tjs_uint64 pos) override;

	virtual tjs_uint64 GetTotalTime() override;

	virtual void SetVolume(tjs_int v) override;
	virtual tjs_int GetVolume() const override { return Volume; }
	virtual void SetVolume2(tjs_int v) override;
	virtual tjs_int GetVolume2() const override { return Volume2; }
	virtual void SetPan(tjs_int v) override;
	virtual tjs_int GetPan() const override { return Pan; }

	// 3D sound is not supported
	virtual void SetPos(float x, float y, float z) override {}
	virtual void SetPosX(float v) override {}
	virtual float GetPosX() const override {return 0;}
	virtual void SetPosY(float v) override {}
	virtual float GetPosY() const override {return 0;}
	virtual void SetPosZ(float v) override {}
	virtual float GetPosZ() const override {return 0;}

	virtual tjs_int GetFrequency() const override { return Frequency; }
	virtual void SetFrequency(tjs_int freq) override;


	void SetVolumeToStream();
	void SetFrequencyToStream();

	static void SetGlobalVolume(tjs_int v);
	static tjs_int GetGlobalVolume() { return GlobalVolume; }
	static void SetGlobalFocusMode(tTVPSoundGlobalFocusMode b);
	static tTVPSoundGlobalFocusMode GetGlobalFocusMode() { return sgfmNeverMute; }

	//-- visualization stuff ----------------------------------------------
	void SetUseVisBuffer(bool b);
	bool GetUseVisBuffer() const { return UseVisBuffer; }

	tjs_int GetVisBuffer(tjs_int16 *dest, tjs_int numsamples, tjs_int channels, tjs_int aheadsamples);

protected:
	virtual void TimerBeatHandler() override; // tTJSNI_BaseSoundBuffer::TimerBeatHandler

	void ResetVisBuffer(); // reset or recreate visualication buffer
	void DeallocateVisBuffer();

	void CopyVisBuffer(tjs_int16 *dest, const tjs_uint8 *src, tjs_int numsamples, tjs_int channels);
};
//---------------------------------------------------------------------------

#endif
