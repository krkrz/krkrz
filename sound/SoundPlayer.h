//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Sound Player for QueueSoundBuffer
//---------------------------------------------------------------------------
#ifndef __SOUND_PLAYER_H__
#define __SOUND_PLAYER_H__

#include "WaveIntf.h"

class tTJSNI_QueueSoundBuffer;
class tTVPSoundSamplesBuffer;
class iTVPAudioStream;
class tTVPSoundPlayer {
	tTJSNI_QueueSoundBuffer* Owner;
	std::vector<tTVPSoundSamplesBuffer*> Samples;
	iTVPAudioStream *Stream;
	tTVPWaveFormat StreamFormat;	// 現在のStreamのSoundフォーマット。再生成を回避する

	bool Paused;
	bool Playing;
	bool BufferEnded;
	tjs_int64 PlayStopPos;

	void CopyVisBuffer(tjs_int16 *dest, const tjs_uint8 *src, tjs_int numsamples, tjs_int channels);
public:
	tTVPSoundPlayer( tTJSNI_QueueSoundBuffer* owner );
	~tTVPSoundPlayer();

	// サンプルバッファを追加する
	void PushSamplesBuffer( tTVPSoundSamplesBuffer* buf );
	static void StreamCallback( class iTVPAudioStream* stream, void* user ) {
		tTVPSoundPlayer* player = reinterpret_cast<tTVPSoundPlayer*>(user);
		player->Callback( stream );
	}
	void Callback( class iTVPAudioStream* stream );
	void CreateStream( class iTVPAudioDevice* device, tTVPWaveFormat& format, tjs_uint samplesCount );
	void Start();
	void Stop();
	void Reset();
	void Clear();
	void ClearSampleQueue();
	void Destroy();
	bool IsSameFormat( tTVPWaveFormat& format ) const {
		return( StreamFormat.SamplesPerSec	== format.SamplesPerSec &&
				StreamFormat.Channels		== format.Channels &&
				StreamFormat.BitsPerSample	== format.BitsPerSample &&
				StreamFormat.BytesPerSample	== format.BytesPerSample &&
				StreamFormat.SpeakerConfig	== format.SpeakerConfig &&
				StreamFormat.IsFloat		== format.IsFloat );
	}
	void SetVolume(tjs_int v);
	void SetPan(tjs_int v);
	void SetFrequency(tjs_int freq);
	bool HasStream() const { return Stream != nullptr; }
	bool IsPaused() const { return Paused; }
	void SetPsused( bool paused );
	bool IsPlaying() const { return Playing; }
	bool Update();
	tjs_int64 GetCurrentPlayingPosition();
	tjs_uint64 GetSamplePosition();
	tjs_int GetVisBuffer(tjs_int16 *dest, tjs_int numsamples, tjs_int channels, tjs_int aheadsamples );
	bool IsBufferEnded() const { return BufferEnded; }
};

#endif // __SOUND_PLAYER_H__
