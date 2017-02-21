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

//---------------------------------------------------------------------------
// Options management
//---------------------------------------------------------------------------
static bool TVPSoundOptionsInit = false;
static tTVPThreadPriority TVPDecodeThreadHighPriority = ttpHigher;
static tTVPThreadPriority TVPDecodeThreadLowPriority = ttpLowest;

static tTVPSoundGlobalFocusMode TVPSoundGlobalFocusModeByOption = sgfmNeverMute;
static tjs_int TVPSoundGlobalFocusMuteVolume = 0;
//---------------------------------------------------------------------------
static void TVPInitSoundOptions()
{
	if( TVPSoundOptionsInit) return;

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

	// tmp. TODO : オプションから読んで値を適時設定する
	tTVPAudioInitParam param;
	param.Channels = 2;
	param.SampleRate = 48000;
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
tTJSNI_QueueSoundBuffer::tTJSNI_QueueSoundBuffer()
{
	TVPInitSoundOptions();
	// TVPRegisterTSSWaveDecoderCreator();
	Decoder = nullptr;
	Stream = nullptr;
	LoopManager = nullptr;
	Thread = nullptr;
	UseVisBuffer = false;
	VisBuffer = nullptr;
	ThreadCallbackEnabled = false;
	//Level2Buffer = NULL;
	//Level2BufferSize = 0;
	Volume = 100000;
	Volume2 = 100000;
	//BufferCanControlPan = false;
	Pan = 0;
	/*PosX = PosY = PosZ = ( D3DVALUE )0.0;
	SoundBuffer = NULL;
	Sound3DBuffer = NULL;
	L2BufferDecodedSamplesInUnit = NULL;*/

	for( tjs_uint i = 0; i < BufferCount; i++ ) {
		Buffer[i] = nullptr;
	}
	//L1BufferSegmentQueues = nullptr;
	//L2BufferSegmentQueues = NULL;
	//L1BufferDecodeSamplePos = nullptr;
	DecodePos = 0;
	//L1BufferUnits = 0;
	//L2BufferUnits = 0;
	TVPSoundBuffers.AddBuffer( this );
	Thread = new tTVPSoundDecodeThread( this );
	ZeroMemory( &C_InputFormat, sizeof( C_InputFormat ) );
	ZeroMemory( &InputFormat, sizeof( InputFormat ) );
	//ZeroMemory( &Format, sizeof( Format ) );
	Looping = false;
	StreamPlaying = false;
	BufferPlaying = false;
	Paused = false;
	//BufferBytes = 0;
	AccessUnitBytes = 0;
	AccessUnitSamples = 0;
	//L2AccessUnitBytes = 0;
	//SoundBufferPrevReadPos = 0;
	//SoundBufferWritePos = 0;
	PlayStopPos = 0;
	//L2BufferReadPos = 0;
	//L2BufferWritePos = 0;
	//L2BufferRemain = 0;
	//L2BufferEnded = false;
	BufferEnded = false;
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
void tTJSNI_QueueSoundBuffer::StreamCallback( iTVPAudioStream* stream, void* user ) {
	tTJSNI_QueueSoundBuffer* sb = reinterpret_cast<tTJSNI_QueueSoundBuffer*>(user);
	sb->EnqueueBuffer( stream );
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::MakeSilentWave( void *dest, tjs_int bytes, tjs_int bitsPerSample ) {
	if( bitsPerSample == 8 ) {
		// 0x80
		memset( dest, 0x80, bytes );
	} else {
		// 0x00
		memset( dest, 0x00, bytes );
	}
}
//---------------------------------------------------------------------------
bool tTJSNI_QueueSoundBuffer::EnqueueBuffer( iTVPAudioStream* stream ) {
	tTJSCriticalSectionHolder holder(BufferCS);

	if(!Stream) return false;
	if(!Decoder) return false;
	if(!BufferPlaying) return false;

	ResetLastCheckedDecodePos();

	if( PlayStopPos != -1 ) {
		// check whether the buffer playing position passes over PlayStopPos
		tjs_uint64 samplesPlayed = stream->GetSamplesPlayed();
		if( PlayStopPos <= (tjs_int64)(samplesPlayed*InputFormat.BytesPerSample*InputFormat.Channels) ) {
			FlushAllLabelEvents();
			Stream->StopStream();
			ResetSamplePositions();
			StreamPlaying = false;
			BufferPlaying = false;
			if( LoopManager ) LoopManager->SetPosition( 0 );
			return true;
		}
	}

	bool result = false;
	tjs_uint32 count = stream->GetQueuedCount();
	if( count < 2 ) {
  		if( ReadBufferIndex != WriteBufferIndex ) {
			tjs_uint decoded = BufferDecodedSamplesInUnit[ReadBufferIndex];
			if( decoded == 0 ) return false;
			if( decoded < (tjs_uint)AccessUnitSamples ) {
				// fill rest with silence
				tjs_uint blockAlign = InputFormat.BytesPerSample*InputFormat.Channels;
				MakeSilentWave( (tjs_uint8*)Buffer[ReadBufferIndex] + decoded*blockAlign, (AccessUnitSamples - decoded)*blockAlign, InputFormat.BitsPerSample );

				if( PlayStopPos == -1 ) {
					// decoding was finished
					PlayStopPos = QueuedSize + decoded*blockAlign;
					// set stop position
				}
			}
			stream->Enqueue( Buffer[ReadBufferIndex], BufferSize, BufferEnded );
  			result = true;
  			if( UseVisBuffer ) {
  				memcpy( VisBuffer + ReadBufferIndex*BufferSize, Buffer[ReadBufferIndex], BufferSize );
  			}

			// insert labels into LabelEventQueue and sort
			const std::deque<tTVPWaveLabel> & labels = segment->GetLabels();
			if(labels.size() != 0) {
				// add DecodePos offset to each item->Offset
				// and insert into LabelEventQueue
				for( std::deque<tTVPWaveLabel>::const_iterator i = labels.begin(); i != labels.end(); i++) {
					LabelEventQueue.push_back( tTVPWaveLabel(i->Position, i->Name, static_cast<tjs_int>(i->Offset + DecodePos)));
				}

				// sort
				std::sort(LabelEventQueue.begin(), LabelEventQueue.end(), tTVPWaveLabel::tSortByOffsetFuncObj());

				// re-schedule label events
				TVPSoundBuffers.ReschedulePendingLabelEvent(GetNearestEventStep());
			}
			QueuedSize += BufferSize;
  			DecodeSamplePos[ReadBufferIndex] = DecodePos;
  			DecodePos += AccessUnitSamples;
			ReadBufferIndex++;
			ReadBufferIndex %= BufferCount;
 		}
	}
	return result;
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
	if( Stream ) delete Stream, Stream = nullptr;

	StreamPlaying = false;
	BufferPlaying = false;

	LabelEventQueue.clear();

	for( tjs_uint i = 0; i < BufferCount; i++ ) {
		if( Buffer[i] ) delete[] Buffer[i];
		Buffer[i] = nullptr;
	}

	ZeroMemory( &C_InputFormat, sizeof( C_InputFormat ) );
	
	DeallocateVisBuffer();
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::ResetSamplePositions() {
	for( tjs_uint i = 0; i < BufferCount; i++ ) {
		BufferSegmentQueues[i].Clear();
		DecodeSamplePos[i] = -1;
	}
	LabelEventQueue.clear();
	WriteBufferIndex = ReadBufferIndex = 0;
	DecodePos = 0;
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::Clear()
{
	// clear all status and unload current decoder
	Stop();
	ThreadCallbackEnabled = false;
	TVPSoundBuffers.CheckAllSleep();
	Thread->Interrupt();
	if(LoopManager) delete LoopManager, LoopManager = NULL;
	ClearFilterChain();
	if(Decoder) delete Decoder, Decoder = NULL;
	BufferPlaying = false;
	StreamPlaying = false;
	Paused = false;

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
bool tTJSNI_QueueSoundBuffer::DoDecode() {
	if( Stream ) {
		tTJSCriticalSectionHolder holder(BufferCS);

		tjs_uint32 count = Stream->GetQueuedCount();
		if( count < 2 && ReadBufferIndex == WriteBufferIndex ) {
			BufferSegmentQueues[WriteBufferIndex].Clear();
			if( BufferEnded ) {
				BufferDecodedSamplesInUnit[WriteBufferIndex] = 0;
				WriteBufferIndex++;
				WriteBufferIndex %= BufferCount;
			} else {
				tjs_uint decoded = Decode( Buffer[WriteBufferIndex], AccessUnitSamples, BufferSegmentQueues[WriteBufferIndex] );
				BufferDecodedSamplesInUnit[WriteBufferIndex] = decoded;
				WriteBufferIndex++;
				WriteBufferIndex %= BufferCount;

				if( decoded < (tjs_uint)AccessUnitSamples ) BufferEnded = true;
			}
			return true;
		}
	}
	return false;
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::Update() {
	tTJSCriticalSectionHolder holder(BufferCS);
	if(!SoundBuffer) return;
	if(!Decoder) return;
	if(!BufferPlaying) return;

	if( Paused ) {
		if( StreamPlaying ) {
			Stream->StopStream();
			StreamPlaying = false;
		}
		return;
	} else {
		if( !StreamPlaying ) {
			Stream->StartStream();
			StreamPlaying = true;
		}
	}
	EnqueueBuffer( Stream );
}
//---------------------------------------------------------------------------
#if 0
bool tTJSNI_QueueSoundBuffer::FillL2Buffer(bool firstwrite, bool fromdecodethread) {
	if(!fromdecodethread && Thread->GetRunning())
		Thread->SetPriority(ttpHighest);
			// make decoder thread priority high, before entering critical section

	tTJSCriticalSectionHolder holder(L2BufferCS);

	if(firstwrite)
	{
		// only main thread runs here
		L2BufferReadPos = L2BufferWritePos = L2BufferRemain = 0;
		L2BufferEnded = false;
		for(tjs_int i = 0; i<L2BufferUnits; i++)
			L2BufferDecodedSamplesInUnit[i] = 0;
	}

	{
		tTVPThreadPriority ttpbefore = TVPDecodeThreadHighPriority;
		bool retflag = false;
		if(Thread->GetRunning())
		{
			ttpbefore = Thread->GetPriority();
			Thread->SetPriority(TVPDecodeThreadHighPriority);
		}
		{
			tTJSCriticalSectionHolder holder(L2BufferRemainCS);
			if(L2BufferRemain == L2BufferUnits) retflag = true;
		}
		if(!retflag) UpdateFilterChain(); // if the buffer is not full, update filter internal state
		if(Thread->GetRunning()) Thread->SetPriority(ttpbefore);
		if(retflag) return false; // buffer is full
	}

	if(L2BufferEnded)
	{
		L2BufferSegmentQueues[L2BufferWritePos].Clear();
		L2BufferDecodedSamplesInUnit[L2BufferWritePos] = 0;
	}
	else
	{
		L2BufferSegmentQueues[L2BufferWritePos].Clear();
		tjs_uint decoded = Decode(
			L2BufferWritePos * L2AccessUnitBytes + Level2Buffer,
			AccessUnitSamples,
			L2BufferSegmentQueues[L2BufferWritePos]);

		if(decoded < (tjs_uint) AccessUnitSamples) L2BufferEnded = true;

		L2BufferDecodedSamplesInUnit[L2BufferWritePos] = decoded;
	}

	L2BufferWritePos++;
	if(L2BufferWritePos >= L2BufferUnits) L2BufferWritePos = 0;

	{
		tTVPThreadPriority ttpbefore = TVPDecodeThreadHighPriority;
		if(Thread->GetRunning())
		{
			ttpbefore = Thread->GetPriority();
			Thread->SetPriority(TVPDecodeThreadHighPriority);
		}
		{
			tTJSCriticalSectionHolder holder(L2BufferRemainCS);
			L2BufferRemain++;
		}
		if(Thread->GetRunning()) Thread->SetPriority(ttpbefore);
	}
	return true;
}
#endif
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::CreateSoundBuffer() {
	TVPInitAudioDevice(); // ensure Audio Device object

	//bool format_is_not_identical = TVPAlwaysRecreateSoundBuffer ||
	bool format_is_not_identical = 
		C_InputFormat.SamplesPerSec		!= InputFormat.SamplesPerSec ||
		C_InputFormat.Channels			!= InputFormat.Channels ||
		C_InputFormat.BitsPerSample		!= InputFormat.BitsPerSample ||
		C_InputFormat.BytesPerSample	!= InputFormat.BytesPerSample ||
		C_InputFormat.SpeakerConfig		!= InputFormat.SpeakerConfig ||
		C_InputFormat.IsFloat			!= InputFormat.IsFloat;

	if( format_is_not_identical ) {
		TryCreateSoundBuffer();
	}

	// reset volume, sound position and frequency
	SetVolumeToSoundBuffer();
	SetFrequencyToBuffer();

	C_InputFormat = InputFormat;
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::TryCreateSoundBuffer()
{
	// compute buffer bytes
	AccessUnitSamples = InputFormat.SamplesPerSec;	// 1sec
#ifdef ANDROID
	if( InputFormat.SamplesPerSec == TVPSoundNativeFrameRate ) {
		// サンプリングレートがHW出力レートと一致する時、バッファサイズの調整を行いレイテンシを低減させる
		tjs_uint unit = AccessUnitSamples / TVPSoundNativeFramesPerBuffer;
		if( AccessUnitSamples != (TVPSoundNativeFramesPerBuffer*unit) ) {
			AccessUnitSamples = TVPSoundNativeFramesPerBuffer*(unit+1);
		}
	}
#endif
	AccessUnitBytes = AccessUnitSamples * InputFormat.BytesPerSample * InputFormat.Channels;
	BufferSize = AccessUnitBytes;
	if(BufferSize <= 0)
		TVPThrowExceptionMessage(TJS_W("Invalid format."));

	for( tjs_uint i = 0; i < BufferCount; i++ ) {
		if( Buffer[i] ) {
			delete[] Buffer[i];
		}
		Buffer[i] = new tjs_uint8[BufferSize];
	}

	if( TVPAudioDevice == nullptr ) {
		TVPThrowExceptionMessage(TJS_W("Uninitialize audio device."));
	}
	if( Stream ) {
		delete Stream;
	}
	tTVPAudioStreamParam param;
	param.Channels = InputFormat.Channels;		// チャンネル数
	param.SampleRate = InputFormat.SamplesPerSec;		// サンプリングレート
	param.BitsPerSample = InputFormat.BitsPerSample;	// サンプル当たりのビット数
	param.SampleType = astUInt8;
	if( InputFormat.IsFloat ) {
		param.SampleType = astFloat32;	// サンプルの形式
	} else if( param.BitsPerSample == 8 ) {
		param.SampleType = astUInt8;
	} else if( param.BitsPerSample == 16 ) {
		param.SampleType = astInt16;
	} else {
		TVPThrowExceptionMessage(TJS_W("Invalid format(BitsPerSample)."));
	}
	param.FramesPerBuffer = InputFormat.SamplesPerSec;		// 1回のキューイングで入れるサンプル数
	Stream = TVPAudioDevice->CreateAudioStream( param );
	if( Stream == nullptr ) {
		TVPThrowExceptionMessage(TJS_W("Faild to create audio stream."));
	}

	// allocate visualization buffer
	if(UseVisBuffer) ResetVisBuffer();

}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::ResetLastCheckedDecodePos() {
	if( !Stream ) return;

	// set LastCheckedDecodePos and  LastCheckedTick
	// we shoud reset these values because the clock sources are usually
	// not identical.
	tTJSCriticalSectionHolder holder(BufferCS);

	tjs_uint32 count = Stream->GetQueuedCount();
	tjs_uint32 index = (ReadBufferIndex + count) % BufferCount;

	tjs_uint64 pos = Stream->GetSamplesPlayed();
	tjs_int offset = (tjs_int)( pos % AccessUnitSamples );

	if(DecodeSamplePos[index] != -1)
	{
		LastCheckedDecodePos = DecodeSamplePos[index] + offset;
		LastCheckedTick = TVPGetTickCount();
	}
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_QueueSoundBuffer::FireLabelEventsAndGetNearestLabelEventStep( tjs_int64 tick ) {
	// fire events, event.EventTick <= tick, and return relative time to
	// next nearest event (return TVP_TIMEOFS_INVALID_VALUE for no events).

	// the vector LabelEventQueue must be sorted by the position.
	tTJSCriticalSectionHolder holder(BufferCS);

	if(!BufferPlaying) return TVP_TIMEOFS_INVALID_VALUE; // buffer is not currently playing
	if(!StreamPlaying) return TVP_TIMEOFS_INVALID_VALUE; // direct sound buffer is not currently playing

	if(LabelEventQueue.size() == 0) return TVP_TIMEOFS_INVALID_VALUE; // no more events

	// calculate current playing decodepos
	// at this point, LastCheckedDecodePos must not be -1
	if(LastCheckedDecodePos == -1) ResetLastCheckedDecodePos();
	tjs_int64 decodepos = (tick - LastCheckedTick) * Frequency / 1000 + LastCheckedDecodePos;

	while(true)
	{
		if(LabelEventQueue.size() == 0) break;
		std::vector<tTVPWaveLabel>::iterator i = LabelEventQueue.begin();
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
	tjs_int64 decodepos = (TVPGetTickCount() - LastCheckedTick) * Frequency / 1000 +
		LastCheckedDecodePos;

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

	{	// thread protected block
		if(Thread->GetRunning()) { Thread->SetPriority(TVPDecodeThreadHighPriority); }
		tTJSCriticalSectionHolder holder(BufferCS);

		CreateSoundBuffer();

		// reset filter chain
		ResetFilterChain();

		// fill sound buffer with some first samples
		BufferPlaying = true;
		PlayStopPos = -1;
		QueuedSize = 0;
		BufferEnded = false;
		DoDecode();
		EnqueueBuffer( Stream );
		DoDecode();
		EnqueueBuffer( Stream );

		// start playing
		if(!Paused)
		{
			Stream->StartStream();
			StreamPlaying = true;
		}

		// re-schedule label events
		ResetLastCheckedDecodePos();
		//TVPReschedulePendingLabelEvent(GetNearestEventStep());
	}	// end of thread protected block

	// ensure thread
	TVPSoundBuffers.EnsureBufferWorking(); // wake the playing thread up again
	ThreadCallbackEnabled = true;
	Thread->Continue();

}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::StopPlay()
{
	if(!Decoder) return;

	if(Thread->GetRunning()) { Thread->SetPriority(TVPDecodeThreadHighPriority);}
	tTJSCriticalSectionHolder holder(BufferCS);

	if( Stream ) Stream->StopStream();
	StreamPlaying = false;
	BufferPlaying = false;
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::Play() {
	// play from first or current position
	if(!Decoder) return;
	if(BufferPlaying) return;

	StopPlay();

	//TVPEnsurePrimaryBufferPlay(); // let primary buffer to start running

	if(Thread->GetRunning()) { Thread->SetPriority(TVPDecodeThreadHighPriority);}
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
void tTJSNI_QueueSoundBuffer::SetPaused(bool b) {
	if( Thread->GetRunning() ) {
		Thread->SetPriority(TVPDecodeThreadHighPriority);
	}
	tTJSCriticalSectionHolder holder(BufferCS);
	Paused = b;
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
void tTJSNI_QueueSoundBuffer::SetLooping(bool b) {
	Looping = b;
	if( LoopManager ) LoopManager->SetLooping( Looping );
}
//---------------------------------------------------------------------------
tjs_uint64 tTJSNI_QueueSoundBuffer::GetSamplePosition() {
	if(!Decoder) return 0L;
	if(!Stream) return 0L;

	tTJSCriticalSectionHolder holder(BufferCS);

	// 現在再生している側のバッファを求める
	tjs_uint32 count = Stream->GetQueuedCount();
	tjs_uint32 index = (ReadBufferIndex + count) % BufferCount;
	tTVPWaveSegmentQueue & segs = BufferSegmentQueues[index];

	// 全再生サンプル数をバッファサイズで剰余してオフセットを求める
	tjs_uint64 pos = Stream->GetSamplesPlayed();
	tjs_int offset = (tjs_int)( pos % AccessUnitSamples );
	return segs.FilteredPositionToDecodePosition(offset);
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::SetSamplePosition(tjs_uint64 pos) {
	tjs_uint64 possamples = pos; // in samples

	if(InputFormat.TotalSamples && InputFormat.TotalSamples <= possamples) return;

	if(BufferPlaying && StreamPlaying) {
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
	if(!Stream) return 0L;
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
void tTJSNI_QueueSoundBuffer::SetVolumeToSoundBuffer() {
	// set current volume/pan to Stream
	if( Stream ) {
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
		Stream->SetVolume( v );

		// set pan
		Stream->SetPan( Pan );
	}
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::SetVolume(tjs_int v) {
	if(v < 0) v = 0;
	if(v > 100000) v = 100000;

	if( Volume != v ) {
		Volume = v;
		SetVolumeToSoundBuffer();
	}
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::SetVolume2(tjs_int v) {
	if(v < 0) v = 0;
	if(v > 100000) v = 100000;

	if( Volume2 != v ) {
		Volume2 = v;
		SetVolumeToSoundBuffer();
	}
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::SetPan(tjs_int v) {
	if(v < -100000) v = -100000;
	if(v > 100000) v = 100000;
	if( Pan != v ) {
		SetVolumeToSoundBuffer();
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
void tTJSNI_QueueSoundBuffer::SetFrequencyToBuffer() {
	if(Stream) Stream->SetFrequency(Frequency);
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::SetFrequency(tjs_int freq) {
	Frequency = freq;
	SetFrequencyToBuffer();
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::SetUseVisBuffer(bool b) {
	tTJSCriticalSectionHolder holder(BufferCS);
	if(b) {
		UseVisBuffer = true;
		if(Stream) ResetVisBuffer();
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
	DeallocateVisBuffer();
	VisBuffer = new tjs_uint8[BufferSize*BufferCount];
	UseVisBuffer = true;
}
//---------------------------------------------------------------------------
void tTJSNI_QueueSoundBuffer::DeallocateVisBuffer() {
	tTJSCriticalSectionHolder holder(BufferCS);
	if(VisBuffer) delete [] VisBuffer, VisBuffer = nullptr;
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
	if(!VisBuffer) return 0;
	if(!Decoder) return 0;
	if(!Stream) return 0;
	if(!StreamPlaying || !BufferPlaying) return 0;

	if(channels != InputFormat.Channels && channels != 1) return 0;
	tjs_uint64 samples = 0;
	{
		tTJSCriticalSectionHolder holder(BufferCS);
		samples = Stream->GetSamplesPlayed() + aheadsamples;
		samples %= AccessUnitSamples;
		if(BufferSegmentQueues[pp/AccessUnitBytes].GetFilteredLength() == 0)
			return 0;
	}
	return 0; 
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

