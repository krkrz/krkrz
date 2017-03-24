//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Wave Player implementation
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "SystemControl.h"
#include "DebugIntf.h"
#include "MsgIntf.h"
#include "StorageIntf.h"
#include "WaveIntf.h"
#include "QueueSoundBufferImpl.h"
#include "PluginImpl.h"
#include "SysInitIntf.h"
#include "ThreadIntf.h"
#include "Random.h"
#include "UtilStreams.h"
#include "TickCount.h"
#include "TVPTimer.h"
#include "Application.h"
#include "UserEvent.h"
#include "NativeEventQueue.h"

#include "SoundEventThread.h"
#include "SoundDecodeThread.h"
#include <algorithm>
#include "SoundSamples.h"

//---------------------------------------------------------------------------
// Options management
//---------------------------------------------------------------------------
static bool TVPSoundOptionsInit = false;

static tTVPSoundGlobalFocusMode TVPSoundGlobalFocusModeByOption = sgfmNeverMute;
static tjs_int TVPSoundGlobalFocusMuteVolume = 0;
static tjs_int TVPSoundFrequency = 48000;
static tjs_int TVPSoundChannels = 2;
//---------------------------------------------------------------------------
static void TVPInitSoundOptions()
{
	if( TVPSoundOptionsInit) return;

	tTJSVariant val;
	if(TVPGetCommandLine(TJS_W("-wsfreq"), &val)) {
		TVPSoundFrequency = val;
	}

	TVPSoundOptionsInit = true;
}
//---------------------------------------------------------------------------
static bool TVPDeferedSettingAvailable = false;
//---------------------------------------------------------------------------
static void TVPWaveSoundBufferCommitSettings()
{
	// commit all defered sound buffer settings
	if(TVPDeferedSettingAvailable)
	{
		TVPDeferedSettingAvailable = false;
	}
}
//---------------------------------------------------------------------------
static iTVPAudioDevice* TVPAudioDevice = nullptr;
static void TVPInitAudioDevice()
{
	TVPInitSoundOptions();

	if( TVPAudioDevice ) return;

	TVPAudioDevice = TVPCreateAudioDevice();

	tTVPAudioInitParam param;
	param.Channels = TVPSoundChannels;
	param.SampleRate = TVPSoundFrequency;
	TVPAudioDevice->Initialize( param );
}
//---------------------------------------------------------------------------
static void TVPUninitAudioDevice()
{
	if( TVPAudioDevice ) {
		TVPAudioDevice->Uninitialize();
		delete TVPAudioDevice;
		TVPAudioDevice = nullptr;
	}
}
static tTVPAtExit TVPUninitAudioDeviceAtExit
( TVP_ATEXIT_PRI_RELEASE, TVPUninitAudioDevice );
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Buffer management
//---------------------------------------------------------------------------
static tTVPSoundBuffers TVPSoundBuffers;

static void TVPShutdownSoundBuffers() {
	TVPSoundBuffers.Shutdown();
}
static tTVPAtExit TVPShutdownWaveSoundBuffersAtExit( TVP_ATEXIT_PRI_PREPARE, TVPShutdownSoundBuffers );
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// tTJSNI_QueueSoundBuffer
//---------------------------------------------------------------------------
extern void TVPRegisterTSSWaveDecoderCreator();
tjs_int tTJSNI_QueueSoundBuffer::GlobalVolume = 100000;
tTVPSoundGlobalFocusMode tTJSNI_QueueSoundBuffer::GlobalFocusMode = sgfmNeverMute;
//---------------------------------------------------------------------------
tTJSNI_QueueSoundBuffer::tTJSNI_QueueSoundBuffer() : Player(this)
{
	TVPInitSoundOptions();
	// TVPRegisterTSSWaveDecoderCreator();
	Decoder = nullptr;
	LoopManager = nullptr;
	Thread = nullptr;
	UseVisBuffer = false;
	ThreadCallbackEnabled = false;
	Volume = 100000;
	Volume2 = 100000;
	Pan = 0;
	for( tjs_uint i = 0; i < BufferCount; i++ ) {
		Buffer[i] = nullptr;
	}

	TVPSoundBuffers.AddBuffer( this );
	Thread = new tTVPSoundDecodeThread( this );
	memset( &InputFormat, 0, sizeof( InputFormat ) );
	Looping = false;
	BufferPlaying = false;
	LastCheckedDecodePos = -1;
	LastCheckedTick = 0;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD
tTJSNI_QueueSoundBuffer::Construct(tjs_int numparams, tTJSVariant **param,
		iTJSDispatch2 *tjs_obj)
{
	tjs_error hr = inherited::Construct(numparams, param, tjs_obj);
	if(TJS_FAILED(hr)) return hr;

	return TJS_S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_QueueSoundBuffer::Invalidate()
{
	inherited::Invalidate();

	Clear();

	DestroySoundBuffer();

	if( Thread ) delete Thread, Thread = nullptr;

	TVPSoundBuffers.RemoveBuffer( this );
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::ReleaseSoundBuffer( bool disableevent ) {
	// called at exit ( system uninitialization )
	bool b = CanDeliverEvents;
	if( disableevent )
		CanDeliverEvents = false; // temporarily disables event derivering
	Stop();
	DestroySoundBuffer();
	CanDeliverEvents = b;
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::DestroySoundBuffer() {
	Player.Destroy();

	BufferPlaying = false;

	LabelEventQueue.clear();

	for( tjs_uint i = 0; i < BufferCount; i++ ) {
		if( Buffer[i] ) delete Buffer[i];
		Buffer[i] = nullptr;
	}
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::ResetSamplePositions() {
	for( tjs_uint i = 0; i < BufferCount; i++ ) {
		if( Buffer[i] ) Buffer[i]->Reset();
	}
	LabelEventQueue.clear();
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::Clear()
{
	// clear all status and unload current decoder
	Stop();
	ThreadCallbackEnabled = false;
	TVPSoundBuffers.CheckAllSleep();
	Thread->Interrupt();
	if(LoopManager) delete LoopManager, LoopManager = nullptr;
	ClearFilterChain();
	if(Decoder) delete Decoder, Decoder = nullptr;
	BufferPlaying = false;
	Player.Clear();

	ResetSamplePositions();

	SetStatus(ssUnload);
}
//---------------------------------------------------------------------------
tjs_uint tTJSNI_QueueSoundBuffer::Decode( void *buffer, tjs_uint bufsamplelen, tTVPWaveSegmentQueue & segments ) {
	// decode one buffer unit
	tjs_uint w = 0;
	try {
		// decode
		FilterOutput->Decode( (tjs_uint8*)buffer, bufsamplelen, w, segments );
	} catch( ... ) {
		// ignore errors
		w = 0;
	}
	return w;
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::PushPlayStream( tTVPSoundSamplesBuffer* buffer ) {
	if( !BufferPlaying ) return;

	ResetLastCheckedDecodePos();

	// push play stream
	Player.PushSamplesBuffer( buffer );
#if 0
	tjs_int64 pos = buffer->GetDecodePosition();
	tjs_int64 ppos = Player.GetCurrentPlayingPosition();
	TVPAddLog( TJS_W( "Sample Pos : " ) + ttstr( (tjs_int)ppos ) + TJS_W( "/" ) + ttstr( (tjs_int)pos ) );
#endif

	tjs_int64 decodePos = buffer->GetDecodePosition();
	const std::deque<tTVPWaveLabel> & labels = buffer->GetSegmentQueue().GetLabels();
	if(labels.size() != 0) {
		// add DecodePos offset to each item->Offset
		// and insert into LabelEventQueue
		for( std::deque<tTVPWaveLabel>::const_iterator i = labels.begin(); i != labels.end(); i++) {
			LabelEventQueue.push_back( tTVPWaveLabel(i->Position, i->Name, static_cast<tjs_int>(i->Offset + decodePos)));
		}

		// sort
		std::sort(LabelEventQueue.begin(), LabelEventQueue.end(), tTVPWaveLabel::tSortByOffsetFuncObj());

		// re-schedule label events
		TVPSoundBuffers.ReschedulePendingLabelEvent(GetNearestEventStep());
	}
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::Update() {
	tTJSCriticalSectionHolder holder(BufferCS);
	if(!Decoder) return;
	if(!BufferPlaying) return;

	bool continued = Player.Update();
	if( continued == false ) {
		FlushAllLabelEvents();
		ResetSamplePositions();
		BufferPlaying = false;
		if( LoopManager ) LoopManager->SetPosition( 0 );
	}
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::CreateSoundBuffer() {
	// ensure Audio Device object
	TVPInitAudioDevice();
	if( TVPAudioDevice == nullptr ) {
		TVPThrowExceptionMessage(TJS_W("Uninitialize audio device."));
	}

	if( !Player.IsSameFormat( InputFormat ) ) {
		for( tjs_uint i = 0; i < BufferCount; i++ ) {
			if( Buffer[i] == nullptr ) {
				Buffer[i] = new tTVPSoundSamplesBuffer( this, i );
			}
			Buffer[i]->Create( &InputFormat, UseVisBuffer );
		}
		//Player.CreateStream( TVPAudioDevice, InputFormat, Buffer[0]->GetSamplesCount() );
	}
	Player.CreateStream( TVPAudioDevice, InputFormat, Buffer[0]->GetSamplesCount() );

	// reset volume, sound position and frequency
	SetVolumeToStream();
	SetFrequencyToStream();
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::ResetLastCheckedDecodePos() {
	if( !Player.HasStream() ) return;

	// set LastCheckedDecodePos and  LastCheckedTick
	// we shoud reset these values because the clock sources are usually
	// not identical.
	tTJSCriticalSectionHolder holder(BufferCS);
	LastCheckedDecodePos = Player.GetCurrentPlayingPosition();
	LastCheckedTick = TVPGetTickCount();
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_QueueSoundBuffer::FireLabelEventsAndGetNearestLabelEventStep( tjs_int64 tick ) {
	// fire events, event.EventTick <= tick, and return relative time to
	// next nearest event (return TVP_TIMEOFS_INVALID_VALUE for no events).

	// the vector LabelEventQueue must be sorted by the position.
	tTJSCriticalSectionHolder holder(BufferCS);

	if(!BufferPlaying) return TVP_TIMEOFS_INVALID_VALUE; // buffer is not currently playing
	if(!Player.IsPlaying()) return TVP_TIMEOFS_INVALID_VALUE; // direct sound buffer is not currently playing

	if(LabelEventQueue.size() == 0) return TVP_TIMEOFS_INVALID_VALUE; // no more events

	// calculate current playing decodepos
	// at this point, LastCheckedDecodePos must not be -1
	if(LastCheckedDecodePos == -1) ResetLastCheckedDecodePos();
	tjs_int64 decodepos = (tick - LastCheckedTick) * Frequency / 1000 + LastCheckedDecodePos;

	while(true)
	{
		if(LabelEventQueue.size() == 0) break;
		auto i = LabelEventQueue.begin();
		int diff = (tjs_int32)i->Offset - (tjs_int32)decodepos;
		if(diff <= 0)
			InvokeLabelEvent(i->Name);
		else
			break;
		LabelEventQueue.erase(i);
	}

	if(LabelEventQueue.size() == 0) return TVP_TIMEOFS_INVALID_VALUE; // no more events

	return (tjs_int)((LabelEventQueue[0].Offset - (tjs_int32)decodepos) * 1000 / Frequency);
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_QueueSoundBuffer::GetNearestEventStep() {
	// get nearest event stop from current tick
	// (current tick is taken from TVPGetTickCount)
	tTJSCriticalSectionHolder holder(BufferCS);

	if(LabelEventQueue.size() == 0) return TVP_TIMEOFS_INVALID_VALUE; // no more events

	// calculate current playing decodepos
	// at this point, LastCheckedDecodePos must not be -1
	if(LastCheckedDecodePos == -1) ResetLastCheckedDecodePos();
	tjs_int64 decodepos = (TVPGetTickCount() - LastCheckedTick) * Frequency / 1000 + LastCheckedDecodePos;
	return (tjs_int)((LabelEventQueue[0].Offset - (tjs_int32)decodepos) * 1000 / Frequency);
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::FlushAllLabelEvents() {
	// called at the end of the decode.
	// flush all undelivered events.
	tTJSCriticalSectionHolder holder(BufferCS);

	for( auto i = LabelEventQueue.begin(); i != LabelEventQueue.end(); i++)
		InvokeLabelEvent(i->Name);

	LabelEventQueue.clear();
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::StartPlay()
{
	if(!Decoder) return;

	// let primary buffer to start running
	// TVPEnsurePrimaryBufferPlay();

	// ensure playing thread
	TVPSoundBuffers.EnsureBufferWorking();

	// play from first
	tjs_int64 predecodedSamples = 0;
	{	// thread protected block
		tTJSCriticalSectionHolder holder(BufferCS);
		Thread->ClearQueue();
		Player.ClearSampleQueue();

		CreateSoundBuffer();

		// reset filter chain
		ResetFilterChain();

		// fill sound buffer with some first samples
		BufferPlaying = true;
		Player.Reset();
		for( tjs_int i = 0; i < BufferCount; i++ ) {
			Buffer[i]->Reset();
			Buffer[i]->Decode();
			Buffer[i]->SetDecodePosition( predecodedSamples );
			predecodedSamples += Buffer[i]->GetInSamples();
			PushPlayStream( Buffer[i] );
		}

		// start playing
		Player.Start();

		// re-schedule label events
		ResetLastCheckedDecodePos();
		TVPSoundBuffers.ReschedulePendingLabelEvent(GetNearestEventStep());
	}	// end of thread protected block

	// ensure thread
	TVPSoundBuffers.EnsureBufferWorking(); // wake the playing thread up again
	ThreadCallbackEnabled = true;
	Thread->StartDecoding( predecodedSamples );
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::StopPlay()
{
	if(!Decoder) return;

	tTJSCriticalSectionHolder holder(BufferCS);

	Player.Stop();

	BufferPlaying = false;
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::Play() {
	// play from first or current position
	if(!Decoder) return;
	if(BufferPlaying) return;

	StopPlay();

	tTJSCriticalSectionHolder holder(BufferCS);

	StartPlay();
	SetStatus(ssPlay);
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::Stop() {
	// stop playing
	StopPlay();

	// delete thread
	ThreadCallbackEnabled = false;
	TVPSoundBuffers.CheckAllSleep();
	Thread->Interrupt();

	// set status
	if(Status != ssUnload) SetStatus(ssStop);

	// rewind
	if(LoopManager) LoopManager->SetPosition(0);
}
//---------------------------------------------------------------------------
bool tTJSNI_QueueSoundBuffer::GetPaused() const {
	return Player.IsPaused();
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::SetPaused(bool b) {
	Player.SetPsused( b );
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::Open(const ttstr & storagename) {
	// open a storage and prepare to play
	//TVPEnsurePrimaryBufferPlay(); // let primary buffer to start running

	Clear();

	Decoder = TVPCreateWaveDecoder(storagename);

	try
	{
		// make manager
		LoopManager = new tTVPWaveLoopManager();
		LoopManager->SetDecoder(Decoder);
		LoopManager->SetLooping(Looping);

		// build filter chain
		RebuildFilterChain();

		// retrieve format
		InputFormat = FilterOutput->GetFormat();
		Frequency = InputFormat.SamplesPerSec;
	}
	catch(...)
	{
		Clear();
		throw;
	}

	// open loop information file
	ttstr sliname = storagename + TJS_W(".sli");
	if(TVPIsExistentStorage(sliname))
	{
		tTVPStreamHolder slistream(sliname);
		char *buffer;
		tjs_uint size;
		buffer = new char [ (size = static_cast<tjs_uint>(slistream->GetSize())) +1];
		try
		{
			slistream->ReadBuffer(buffer, size);
			buffer[size] = 0;

			if(!LoopManager->ReadInformation(buffer))
				TVPThrowExceptionMessage(TVPInvalidLoopInformation, sliname);
			RecreateWaveLabelsObject();
		}
		catch(...)
		{
			delete [] buffer;
			Clear();
			throw;
		}
		delete [] buffer;
	}

	// set status to stop
	SetStatus(ssStop);
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::ReleasePlayedSamples( tTVPSoundSamplesBuffer* buffer, bool continued ) {
	// 再生が終了したバッファ。まだ再生するのなら Decoder へ入れる
	if( continued ) {
		if( Thread ) Thread->PushSamplesBuffer( buffer );
	}
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::SetLooping(bool b) {
	Looping = b;
	if( LoopManager ) LoopManager->SetLooping( Looping );
}
//---------------------------------------------------------------------------
tjs_uint64 tTJSNI_QueueSoundBuffer::GetSamplePosition() {
	if(!Decoder) return 0L;
	return Player.GetSamplePosition();
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::SetSamplePosition(tjs_uint64 pos) {
	tjs_uint64 possamples = pos; // in samples

	if(InputFormat.TotalSamples && InputFormat.TotalSamples <= possamples) return;

	if(BufferPlaying && Player.IsPlaying() ) {
		StopPlay();
		LoopManager->SetPosition(possamples);
		StartPlay();
	} else {
		LoopManager->SetPosition(possamples);
	}
}
//---------------------------------------------------------------------------
tjs_uint64 tTJSNI_QueueSoundBuffer::GetPosition() {
	if(!Decoder) return 0L;
	if(!Player.HasStream()) return 0L;
	return GetSamplePosition() * 1000 / InputFormat.SamplesPerSec;
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::SetPosition(tjs_uint64 pos) {
	SetSamplePosition(pos * InputFormat.SamplesPerSec / 1000); // in samples
}
//---------------------------------------------------------------------------
tjs_uint64 tTJSNI_QueueSoundBuffer::GetTotalTime() {
	return InputFormat.TotalSamples * 1000ULL / InputFormat.SamplesPerSec;
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::SetVolumeToStream() {
	// set current volume/pan to Stream
	if( Player.HasStream() ) {
		tjs_int v;
		tjs_int mutevol = 100000;
		if(TVPSoundGlobalFocusModeByOption >= sgfmMuteOnDeactivate &&
			TVPSoundGlobalFocusMuteVolume == 0)
		{
			// no mute needed here;
			// muting will be processed in DirectSound framework.
			;
		}
		else
		{
			// mute mode is choosen from GlobalFocusMode or
			// TVPSoundGlobalFocusModeByOption which is more restrictive.
			tTVPSoundGlobalFocusMode mode =
				GlobalFocusMode > TVPSoundGlobalFocusModeByOption ?
				GlobalFocusMode : TVPSoundGlobalFocusModeByOption;

			switch(mode)
			{
			case sgfmNeverMute:
				;
				break;
			case sgfmMuteOnMinimize:
				if(!  Application->GetNotMinimizing())
					mutevol = TVPSoundGlobalFocusMuteVolume;
				break;
			case sgfmMuteOnDeactivate:
				if(! (  Application->GetActivating() && Application->GetNotMinimizing()))
					mutevol = TVPSoundGlobalFocusMuteVolume;
				break;
			}
		}

		// compute volume for each buffer
		v = (Volume / 10) * (Volume2 / 10) / 1000;
		v = (v / 10) * (GlobalVolume / 10) / 1000;
		v = (v / 10) * (mutevol / 10) / 1000;
		Player.SetVolume( v );

		// set pan
		Player.SetPan( Pan );
	}
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::SetVolume(tjs_int v) {
	if(v < 0) v = 0;
	if(v > 100000) v = 100000;

	if( Volume != v ) {
		Volume = v;
		SetVolumeToStream();
	}
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::SetVolume2(tjs_int v) {
	if(v < 0) v = 0;
	if(v > 100000) v = 100000;

	if( Volume2 != v ) {
		Volume2 = v;
		SetVolumeToStream();
	}
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::SetPan(tjs_int v) {
	if(v < -100000) v = -100000;
	if(v > 100000) v = 100000;
	if( Pan != v ) {
		SetVolumeToStream();
	}
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::SetGlobalVolume(tjs_int v) {
	if(v < 0) v = 0;
	if(v > 100000) v = 100000;

	if( GlobalVolume != v ) {
		GlobalVolume = v;
		TVPSoundBuffers.ResetVolumeToAllSoundBuffer();
	}
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::SetGlobalFocusMode(tTVPSoundGlobalFocusMode b) {
	if( GlobalFocusMode != b ) {
		GlobalFocusMode = b;
		TVPSoundBuffers.ResetVolumeToAllSoundBuffer();
	}
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::SetFrequencyToStream() {
	Player.SetFrequency(Frequency);
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::SetFrequency(tjs_int freq) {
	Frequency = freq;
	SetFrequencyToStream();
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::SetUseVisBuffer(bool b) {
	tTJSCriticalSectionHolder holder(BufferCS);
	if(b) {
		UseVisBuffer = true;
		if(Player.HasStream()) ResetVisBuffer();
	} else {
		DeallocateVisBuffer();
		UseVisBuffer = false;
	}
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::TimerBeatHandler() {
	inherited::TimerBeatHandler();

	// check buffer stopping
	if(Status == ssPlay && !BufferPlaying)
	{
		// buffer was stopped
		ThreadCallbackEnabled = false;
		TVPSoundBuffers.CheckAllSleep();
		Thread->Interrupt();
		SetStatusAsync(ssStop);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::ResetVisBuffer() {
	// reset or recreate visualication buffer
	tTJSCriticalSectionHolder holder(BufferCS);
	for( tjs_uint i = 0; i < BufferCount; i++ ) {
		if( Buffer[i] ) Buffer[i]->ResetVisBuffer();
	}
	UseVisBuffer = true;
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::DeallocateVisBuffer() {
	tTJSCriticalSectionHolder holder(BufferCS);
	for( tjs_uint i = 0; i < BufferCount; i++ ) {
		if( Buffer[i] ) Buffer[i]->DeallocateVisBuffer();
	}
	UseVisBuffer = false;
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::CopyVisBuffer(tjs_int16 *dest, const tjs_uint8 *src,
	tjs_int numsamples, tjs_int channels) {

	if(channels == 1)
	{
		TVPConvertPCMTo16bits(dest, (const void*)src, InputFormat.Channels,
			InputFormat.BytesPerSample / 8, InputFormat.BitsPerSample,
			InputFormat.IsFloat, numsamples, true);
	}
	else if(channels == InputFormat.Channels)
	{
		TVPConvertPCMTo16bits(dest, (const void*)src, InputFormat.Channels,
			InputFormat.BytesPerSample / 8, InputFormat.BitsPerSample,
			InputFormat.IsFloat, numsamples, false);
	}
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_QueueSoundBuffer::GetVisBuffer(tjs_int16 *dest, tjs_int numsamples, tjs_int channels, tjs_int aheadsamples ) {
	// read visualization buffer samples
	if(!UseVisBuffer) return 0;
	if(!Decoder) return 0;
	if(!Player.HasStream()) return 0;
	if(!Player.IsPlaying() || !BufferPlaying) return 0;

	if(channels != InputFormat.Channels && channels != 1) return 0;

	return Player.GetVisBuffer( dest, numsamples, channels, aheadsamples );
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// tTJSNC_WaveSoundBuffer
//---------------------------------------------------------------------------
static tTJSNativeInstance *CreateNativeInstance()
{
	return new tTJSNI_QueueSoundBuffer();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPCreateNativeClass_WaveSoundBuffer
//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_QueueSoundBuffer()
{
	tTJSNativeClass *cls = new tTJSNC_WaveSoundBuffer();
	((tTJSNC_WaveSoundBuffer*)cls)->Factory = CreateNativeInstance;
	static tjs_uint32 TJS_NCM_CLASSID;
	TJS_NCM_CLASSID = tTJSNC_WaveSoundBuffer::ClassID;

//----------------------------------------------------------------------
// methods
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/freeDirectSound)  /* static */
{
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL_OUTER(/*object to register*/cls,
	/*func. name*/freeDirectSound)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/getVisBuffer)
{
	// get samples for visualization 
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
		/*var. type*/tTJSNI_QueueSoundBuffer);

	if(numparams < 3) return TJS_E_BADPARAMCOUNT;
	tjs_int16 *dest = (tjs_int16*)(tjs_int)(*param[0]);

	tjs_int ahead = 0;
	if(numparams >= 4) ahead = (tjs_int)*param[3];

	tjs_int res = _this->GetVisBuffer(dest, *param[1], *param[2], ahead);

	if(result) *result = res;

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL_OUTER(/*object to register*/cls,
	/*func. name*/getVisBuffer)
//----------------------------------------------------------------------



//----------------------------------------------------------------------
// properties
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(useVisBuffer)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
			/*var. type*/tTJSNI_QueueSoundBuffer);

		*result = _this->GetUseVisBuffer();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
			/*var. type*/tTJSNI_QueueSoundBuffer);

		_this->SetUseVisBuffer(0!=(tjs_int)*param);

		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL_OUTER(cls, useVisBuffer)
//----------------------------------------------------------------------
	return cls;
}
//---------------------------------------------------------------------------

