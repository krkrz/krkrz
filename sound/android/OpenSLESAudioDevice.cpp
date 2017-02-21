#ifdef ANDROID

#include <assert.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>


class OpenSLESAudioDevice : public iTVPAudioDevice {
	SLObjectItf EngineObject;
	SLEngineItf EngineEngine;
	tjs_int Volume;

	std::vector<OpenSLESAudioStream*> Children;
public:
	OpenSLESAudioDevice() : EngineObject(nullptr), EngineEngine(nullptr), Volume(100000) {}
	virtual ~OpenSLESAudioDevice() override {}

	virtual void Initialize( tTVPAudioInitParam& param ) override;
	virtual void Uninitialize() override;
	virtual iTVPAudioStream* CreateAudioStream( tTVPAudioStreamParam& param ) override;
	virtual void SetMasterVolume(tjs_int vol) override;
	virtual tjs_int GetMasterVolume() const override;

	void AddStream( OpenSLESAudioStream* stream ) {
		Children.push_back( stream );
	}
	void DelStream( OpenSLESAudioStream* stream ) {
		auto i = std::remove( Children.begin(), Children.end(), stream );
		Children.erase( i, Children.end() );
	}

	SLEngineItf GetEngine() { return EngineEngine; }
};

class OpenSLESAudioStream : public iTVPAudioStream {
	OpenSLESAudioDevice* Owner;
	
	tTVPAudioStreamParam Param;

	SLObjectItf						OutputMixObject;	// 出力オブジェクト
	SLObjectItf						PlayerObject;		// プレイヤーオブジェクト
	SLPlayItf						Player;				// インタフェース
	SLAndroidSimpleBufferQueueItf	BufferQueue;		// バッファキューインタフェース
	SLVolumeItf						Volume;				// 音量インタフェース

	StreamQueueCallback CallbackFunc;
	void* UserData;

	tjs_int AudioVolumeValue;
	tjs_int AudioBalanceValue;

public:
	OpenSLESAudioStream( OpenSLESAudioDevice* parent, const tTVPAudioStreamParam& param );
	virtual ~OpenSLESAudioStream();

	static void PlayerCallback(SLAndroidSimpleBufferQueueItf, void* context) {
		OpenSLESAudioStream* stream = reinterpret_cast<OpenSLESAudioStream*>(context);
		strem->Callback();
	}
	void Callback() {
		CallbackFunc( this, UserData );
	}
	virtual void SetCallback( StreamQueueCallback callback, void* user ) override {
		CallbackFunc = callback;
		UserData = user;
	}
	virtual void Enqueue( void *data, size_t size, bool last ) override {
		(*BufferQueue)->Enqueue(BufferQueue, data, (SLuint32)size );
	}
	virtual void ClearQueue() override {
		(*BufferQueue)->Clear( BufferQueue );
	}

	virtual tjs_uint32 GetQueuedCount() const {
		SLAndroidSimpleBufferQueueState state;
		SLresult result = (*BufferQueue)->GetState( BufferQueue );
		if( SL_RESULT_SUCCESS == result ) {
			return state.count;
		} else {
			return 0;
		}
	}
	virtual tjs_uint64 GetSamplesPlayed() const {
		SLmillisecond pos = 0;
		SLresult result = (*Player)->GetPosition( Player, &pos );
		if( SL_RESULT_SUCCESS == result ) {
			return (tjs_uint64)Param.SampleRate * (tjs_uint64)pos / 1000ULL;
		} else {
			return 0;
		}
	}

	virtual void StartStream() override {
		(*Player)->SetPlayState( Player, SL_PLAYSTATE_PLAYING );
	}
	virtual void StopStream() override {
		(*Player)->SetPlayState( Player, SL_PLAYSTATE_STOPPED );
	}
	virtual void AbortStream() override {
		(*Player)->SetPlayState( Player, SL_PLAYSTATE_STOPPED );
	}

	virtual void SetVolume(tjs_int vol) override {
		if( AudioVolumeValue != vol ) {
			SLmillibel maxVol;
			SLresult result = (*Volume)->GetMaxVolumeLevel( Volume, &maxVol );
			if( SL_RESULT_SUCCESS != result ) {
				TVPThrowExceptionMessage( TJS_W("SLVolumeItf::GetMaxVolumeLevel Error : ") + TVPGetOpenSLESErrorMessage(result) );
			}

			SLmillibel vol = static_cast<SLmillibel>( (static_cast<tjs_int>(maxVol)*100000) / vol );
			(*Volume)->SetVolumeLevel( Volume, vol );
			if( SL_RESULT_SUCCESS != result ) {
				TVPThrowExceptionMessage( TJS_W("SLVolumeItf::SetVolumeLevel Error : ") + TVPGetOpenSLESErrorMessage(result) );
			}
			AudioVolumeValue = vol;
		}
	}
	virtual tjs_int GetVolume() const override { return AudioVolumeValue; }
	virtual void SetPan(tjs_int pan) override {
		if( AudioBalanceValue != pan ) {
			if( pan == 0 ) {
				SLresult result = (*Volume)->SetStereoPosition( Volume, 0 );
				if( SL_RESULT_SUCCESS != result ) {
					TVPThrowExceptionMessage( TJS_W("SLVolumeItf::SetStereoPosition Error : ") + TVPGetOpenSLESErrorMessage(result) );
				}

				result = (*Volume)->EnableStereoPosition( Volume, SL_BOOLEAN_FALSE );
				if( SL_RESULT_SUCCESS != result ) {
					TVPThrowExceptionMessage( TJS_W("SLVolumeItf::EnableStereoPosition Error : ") + TVPGetOpenSLESErrorMessage(result) );
				}
			} else {
				SLboolean panning;
				SLresult result = (*Volume)->IsEnabledStereoPosition( Volume, &panning );
				if( SL_RESULT_SUCCESS != result ) {
					TVPThrowExceptionMessage( TJS_W("SLVolumeItf::IsEnabledStereoPosition Error : ") + TVPGetOpenSLESErrorMessage(result) );
				}
				if( panning != SL_BOOLEAN_TRUE ) {
					result = (*Volume)->EnableStereoPosition( Volume, SL_BOOLEAN_TRUE );
					if( SL_RESULT_SUCCESS != result ) {
						TVPThrowExceptionMessage( TJS_W("SLVolumeItf::EnableStereoPosition Error : ") + TVPGetOpenSLESErrorMessage(result) );
					}
				}
				SLpermille pos = static_cast<SLpermille>( pan / 100 );
				result = (*Volume)->SetStereoPosition( Volume, pos );	// -1000 - 0 - 1000
				if( SL_RESULT_SUCCESS != result ) {
					TVPThrowExceptionMessage( TJS_W("SLVolumeItf::SetStereoPosition Error : ") + TVPGetOpenSLESErrorMessage(result) );
				}
			}
			AudioBalanceValue = pan;
		}
	}
	virtual tjs_int GetPan() const override { return AudioBalanceValue; }

	// Android ではサポートされない SLRatePitchItf の取得はエラーとなる
	virtual void SetFrequency(tjs_int freq) {}
	virtual tjs_int GetFrequency() const { return Param.SampleRate; }
};


void OpenSLESAudioDevice::Initialize( tTVPAudioInitParam& param ) {
	// param は無視。環境依存で設定されている
	SLresult result;
	result = slCreateEngine(&EngineObject, 0, nullptr, 0, nullptr, nullptr);
	assert(SL_RESULT_SUCCESS == result);

	result = (*EngineObject)->Realize(EngineObject, SL_BOOLEAN_FALSE);
	assert(SL_RESULT_SUCCESS == result);

	result = (*EngineObject)->GetInterface(EngineObject, SL_IID_ENGINE, &EngineEngine);
	assert(SL_RESULT_SUCCESS == result);
}

void OpenSLESAudioDevice::Uninitialize() {
	if( EngineObject ) {
		(*EngineObject)->Destroy(EngineObject);
		EngineObject = nullptr;
	}
}
void OpenSLESAudioDevice::SetMasterVolume(tjs_int vol) {
	// call to java
}
tjs_int OpenSLESAudioDevice::GetMasterVolume() const {
	// call to java
	return 1;	// TODO implements
}
iTVPAudioStream* OpenSLESAudioDevice::CreateAudioStream( tTVPAudioStreamParam& param ) {
	OpenSLESAudioStream* stream = new OpenSLESAudioStream( this, param );
	AddStream( stream );
	return stream;
}

OpenSLESAudioStream::OpenSLESAudioStream( OpenSLESAudioDevice* parent, const tTVPAudioStreamParam& param ) {
	Owner = parent;
	Param = param;
	SLEngineItf engine = parent->GetEngine();

	// 出力オブジェクト作成
	SLresult result = (*engine)->CreateOutputMix(engine, &OutputMixObject, 0, nullptr, nullptr );
	assert(SL_RESULT_SUCCESS == result);
	result = (*OutputMixObject)->Realize(OutputMixObject, SL_BOOLEAN_FALSE);
	assert(SL_RESULT_SUCCESS == result);

	SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
	SLDataFormat_PCM format_pcm;
	SLDataSource audioSrc = {&loc_bufq, &format_pcm};

	format_pcm.formatType	 = SL_DATAFORMAT_PCM;
	format_pcm.numChannels	 = (SLuint32)param.Channels;
	format_pcm.samplesPerSec = (SLuint32)param.SampleRate*1000;
	format_pcm.bitsPerSample = (SLuint32)param.BitsPerSample;
	format_pcm.containerSize = (SLuint32)param.BitsPerSample;
	format_pcm.channelMask	 = (param.Channels == 1) ? SL_SPEAKER_FRONT_CENTER : (SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT);
	format_pcm.endianness	 = SL_BYTEORDER_LITTLEENDIAN;

	SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, OutputMixObject};
	SLDataSink audioSnk = {&loc_outmix, nullptr};
	const SLInterfaceID ids[3] = {SL_IID_PLAY, SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
	const SLboolean		req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

	// プレイヤーオブジェクト作成
	SLresult result = (*engine)->CreateAudioPlayer(engine, &PlayerObject, &audioSrc, &audioSnk, 3, ids, req);
	if( SL_RESULT_SUCCESS != result ) {
		PlayerObject = nullptr;
		throw;
	}
	result = (*PlayerObject)->Realize(PlayerObject, SL_BOOLEAN_FALSE);
	assert(SL_RESULT_SUCCESS == result);

	// インタフェース取得
	result = (*PlayerObject)->GetInterface(PlayerObject, SL_IID_PLAY, &Player);
	assert(SL_RESULT_SUCCESS == result);

	// バッファキューインタフェース
	result = (*PlayerObject)->GetInterface(PlayerObject, SL_IID_BUFFERQUEUE, &BufferQueue);
	assert(SL_RESULT_SUCCESS == result);

	// 音量インタフェース
	result = (*PlayerObject)->GetInterface(PlayerObject, SL_IID_VOLUME, &Volume);
	assert(SL_RESULT_SUCCESS == result);

	// 再生コールバック設定
	result = (*BufferQueue)->RegisterCallback(BufferQueue, OpenSLESAudioStream::PlayerCallback, this);
	assert(SL_RESULT_SUCCESS == result);
}
OpenSLESAudioStream::~OpenSLESAudioStream() {
	if( PlayerObject ) {
		(*PlayerObject)->Destroy(PlayerObject);
		PlayerObject = nullptr;
	}
	if( OutputMixObject ) {
		(*OutputMixObject)->Destroy(OutputMixObject);
		OutputMixObject = nullptr;
	}
	if( Owner ) {
		Owner->DelStream( this );
		Owner = nullptr;
	}
}


iTVPAudioDevice* TVPCreateAudioDevice() {
	return new OpenSLESAudioDevice();
}


#endif
