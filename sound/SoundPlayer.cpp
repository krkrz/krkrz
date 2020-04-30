//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Sound Player for QueueSoundBuffer
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "MsgIntf.h"
#include "SoundPlayer.h"
#include "QueueSoundBufferImpl.h"
#include "SoundSamples.h"

//---------------------------------------------------------------------------
tTVPSoundPlayer::tTVPSoundPlayer( tTJSNI_QueueSoundBuffer* owner )
 : Owner( owner ), Stream(nullptr), Paused(false), Playing(false), PlayStopPos(-1) {
}
//---------------------------------------------------------------------------
tTVPSoundPlayer::~tTVPSoundPlayer() {
}
//---------------------------------------------------------------------------
void tTVPSoundPlayer::PushSamplesBuffer( tTVPSoundSamplesBuffer* buf ) {
	tTJSCriticalSectionHolder holder(Owner->GetBufferCS());
	Samples.push_back( buf );
	if( !BufferEnded ) {
		BufferEnded = buf->IsEnded();
		if( BufferEnded ) {
			PlayStopPos = buf->GetDecodePosition() + buf->GetInSamples();
		}
	}
	if( Stream ) buf->Enqueue( Stream );
}
//---------------------------------------------------------------------------
void tTVPSoundPlayer::Callback( class iTVPAudioStream* stream ) {
	tTVPSoundSamplesBuffer* sample = nullptr;
	{
		tTJSCriticalSectionHolder holder(Owner->GetBufferCS());
		if( Samples.size() > 0 ) {
			auto itr = Samples.begin();
			sample = *itr;
			Samples.erase( itr );
		}
	}
	if( sample ) Owner->ReleasePlayedSamples( sample, !BufferEnded );
}
//---------------------------------------------------------------------------
void tTVPSoundPlayer::CreateStream( iTVPAudioDevice* device, tTVPWaveFormat& format, tjs_uint samplesCount ) {
	if( Stream ) delete Stream;

	tTVPAudioStreamParam param;
	param.Channels = format.Channels;		// チャンネル数
	param.SampleRate = format.SamplesPerSec;		// サンプリングレート
	param.BitsPerSample = format.BitsPerSample;	// サンプル当たりのビット数
	param.SampleType = astUInt8;
	if( format.IsFloat ) {
		param.SampleType = astFloat32;	// サンプルの形式
	} else if( param.BitsPerSample == 8 ) {
		param.SampleType = astUInt8;
	} else if( param.BitsPerSample == 16 ) {
		param.SampleType = astInt16;
	} else {
		TVPThrowExceptionMessage(TJS_W("Invalid format(BitsPerSample)."));
	}
	param.FramesPerBuffer = samplesCount;		// 1回のキューイングで入れるサンプル数
	Stream = device->CreateAudioStream( param );
	if( Stream == nullptr ) {
		TVPThrowExceptionMessage(TJS_W("Faild to create audio stream."));
	}
	StreamFormat = format;
	Stream->SetCallback( StreamCallback, this );
}
//---------------------------------------------------------------------------
tjs_int64 tTVPSoundPlayer::GetCurrentPlayingPosition() {
	tjs_int64 result = -1;
	if( Stream ) {
		tTJSCriticalSectionHolder holder(Owner->GetBufferCS());
		tjs_uint64 pos = Stream->GetSamplesPlayed();
		if( Samples.size() > 0 ) {
			auto itr = Samples.begin();
			tTVPSoundSamplesBuffer* sample = *itr;
			tjs_uint count = sample->GetSamplesCount();
			tjs_int offset = (tjs_int)( pos % count );
			result = sample->GetDecodePosition() + offset;
		}
	}
	return result;
}
//---------------------------------------------------------------------------
tjs_uint64 tTVPSoundPlayer::GetSamplePosition() {
	tjs_uint64 result = 0;
	if( Stream ) {
		tTJSCriticalSectionHolder holder(Owner->GetBufferCS());
		tjs_uint64 pos = Stream->GetSamplesPlayed();
		if( Samples.size() > 0 ) {
			auto itr = Samples.begin();
			tTVPSoundSamplesBuffer* sample = *itr;
			tjs_uint count = sample->GetSamplesCount();
			tjs_int offset = (tjs_int)( pos % count );
			result = sample->GetSegmentQueue().FilteredPositionToDecodePosition( offset );
		}
	}
	return result;
}
//---------------------------------------------------------------------------
void tTVPSoundPlayer::CopyVisBuffer(tjs_int16 *dest, const tjs_uint8 *src,
	tjs_int numsamples, tjs_int channels)
{
	if(channels == 1) {
		TVPConvertPCMTo16bits(dest, (const void*)src, StreamFormat.Channels,
			StreamFormat.BytesPerSample, StreamFormat.BitsPerSample,
			StreamFormat.IsFloat, numsamples, true);
	} else if(channels == StreamFormat.Channels) {
		TVPConvertPCMTo16bits(dest, (const void*)src, StreamFormat.Channels,
			StreamFormat.BytesPerSample, StreamFormat.BitsPerSample,
			StreamFormat.IsFloat, numsamples, false);
	}
}
//---------------------------------------------------------------------------
tjs_int tTVPSoundPlayer::GetVisBuffer(tjs_int16 *dest, tjs_int numsamples, tjs_int channels, tjs_int aheadsamples ) {
	tjs_int writtensamples = 0;
	tjs_uint blockAlign = StreamFormat.BytesPerSample * StreamFormat.Channels;
	if( Stream ) {
		tTJSCriticalSectionHolder holder(Owner->GetBufferCS());
		tjs_uint64 pos = Stream->GetSamplesPlayed();
		if( Samples.size() > 0 ) {
			auto itr = Samples.begin();
			if( (*itr)->GetSegmentQueue().GetFilteredLength() == 0 ) return 0;
			tTVPSoundSamplesBuffer* sample = *itr;
			tjs_int count = static_cast<tjs_int>(sample->GetSamplesCount());
			tjs_int offset = (tjs_int)( pos % count ) + aheadsamples;
			for( auto i = Samples.begin(); i != Samples.end(); i++ ) {
				if( offset >= count ) {
					offset -= count;
					continue;
				}
				tjs_int bufrest = count - offset;
				tjs_int copysamples = (bufrest > numsamples ? numsamples : bufrest);
				CopyVisBuffer(dest, (*i)->GetVisBuffer() + offset * blockAlign, copysamples, channels);
				numsamples -= copysamples;
				writtensamples += copysamples;
				if(numsamples <= 0) break;

				dest += channels * copysamples;
				offset = 0;
			}
		}
	}
	return writtensamples;
}
//---------------------------------------------------------------------------
void tTVPSoundPlayer::Start() {
	if(!Paused) {
		Stream->StartStream();
		Playing = true;
	}
}
//---------------------------------------------------------------------------
void tTVPSoundPlayer::Stop() {
	if( Stream ) Stream->StopStream();
	Playing = false;
}
//---------------------------------------------------------------------------
void tTVPSoundPlayer::Reset() {
	BufferEnded = false;
	PlayStopPos = -1;
}
//---------------------------------------------------------------------------
void tTVPSoundPlayer::Clear() {
	tTJSCriticalSectionHolder holder(Owner->GetBufferCS());
	Paused = false;
	Playing = false;
	PlayStopPos = -1;
	Samples.clear();
}
//---------------------------------------------------------------------------
void tTVPSoundPlayer::ClearSampleQueue() {
	tTJSCriticalSectionHolder holder(Owner->GetBufferCS());
	if( Stream ) Stream->ClearQueue();
	Samples.clear();
}
//---------------------------------------------------------------------------
void tTVPSoundPlayer::Destroy() {
	if( Stream ) delete Stream, Stream = nullptr;
	Playing = false;

	memset( &StreamFormat, 0, sizeof( StreamFormat ) );
}
//---------------------------------------------------------------------------
void tTVPSoundPlayer::SetVolume(tjs_int v) {
	if( Stream ) Stream->SetVolume( v );
}
//---------------------------------------------------------------------------
void tTVPSoundPlayer::SetPan(tjs_int v) {
	if( Stream ) Stream->SetPan( v );
}
//---------------------------------------------------------------------------
void tTVPSoundPlayer::SetFrequency(tjs_int freq) {
	if(Stream) Stream->SetFrequency( freq );
}
//---------------------------------------------------------------------------
void tTVPSoundPlayer::SetPsused( bool paused ) {
	tTJSCriticalSectionHolder holder(Owner->GetBufferCS());
	Playing = Paused;
}
//---------------------------------------------------------------------------
bool tTVPSoundPlayer::Update() {
	bool continued = true;
	if( Paused ) {
		if( Playing ) {
			Stream->StopStream();
			Playing = false;
		}
		return false;
	} else {
		if( !Playing ) {
			Stream->StartStream();
			Playing = true;
		}
	}
	if( PlayStopPos != -1 ) {
		tjs_uint64 samplesPlayed = Stream->GetSamplesPlayed();
		if( PlayStopPos <= (tjs_int64)(samplesPlayed) || samplesPlayed == 0 ) {	// Sound API の種類によって再生終了後にサンプル位置が取得できず、GetSamplesPlayed が0を返すケースもありうる
			Stream->StopStream();
			Playing = false;
			continued = false;
		}
	}
	return continued;
}
//---------------------------------------------------------------------------
