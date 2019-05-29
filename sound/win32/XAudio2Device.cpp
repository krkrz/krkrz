#ifdef WIN32

#include "tjsCommHead.h"

#include <string.h>
#include <assert.h>
#include <vector>
#include <algorithm>
#include "AudioDevice.h"
#include "MsgIntf.h"
#include "DebugIntf.h"

// 暫定的に対処 TODO 後でDirectX SDK入れること。
#undef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WIN8
#include <Xaudio2.h>

#ifdef _MSC_VER
//#pragma comment(lib,"XAudio2.lib")
#endif
bool TVPHasXAudio2DLL() {
	HMODULE hModule = nullptr;
	hModule = ::LoadLibrary( TJS_W("XAudio2_9.dll") );
	if( hModule == nullptr ) {
		hModule = ::LoadLibrary( TJS_W("XAudio2_8.dll") );
	}
	bool result = hModule != nullptr;
	if( hModule ) {
		::FreeLibrary(hModule);
	}
	return result;
}
class XAudio2Stream;
class XAudio2Device : public iTVPAudioDevice {
	typedef HRESULT ( __stdcall *FuncXAudio2Create)( IXAudio2 **ppXAudio2, UINT32 Flags, XAUDIO2_PROCESSOR XAudio2Processor);

	HMODULE XAudio2DLL;
	FuncXAudio2Create procXAudio2Create;
	IXAudio2* XAudio2;
	IXAudio2MasteringVoice* MasteringVoice;
	tjs_int Volume;

	std::vector<XAudio2Stream*> Children;

	float CalcVolume() const {
		float level = (float)Volume / 100000.0f;
		if( level < 0.0f ) level = 0.0f;
		if( level > 1.0f ) level = 1.0f;
		return level;
	}
public:
	XAudio2Device() : XAudio2DLL(nullptr), procXAudio2Create(nullptr), XAudio2(nullptr), MasteringVoice(nullptr), Volume(100000) {}
	virtual ~XAudio2Device() override;

	virtual void Initialize( tTVPAudioInitParam& param ) override;
	virtual void Uninitialize() override;
	virtual iTVPAudioStream* CreateAudioStream( tTVPAudioStreamParam& param ) override;
	virtual void SetMasterVolume(tjs_int vol) override {
		if( Volume != vol ) {
			if( MasteringVoice ) {
				Volume = vol;
				MasteringVoice->SetVolume( CalcVolume() );
			}
		}
	}
	virtual tjs_int GetMasterVolume() const override { return Volume; }

	void AddStream( XAudio2Stream* stream ) {
		Children.push_back( stream );
	}
	void DelStream( XAudio2Stream* stream ) {
		auto i = std::remove( Children.begin(), Children.end(), stream );
		Children.erase( i, Children.end() );
	}

	IXAudio2* GetXAudio2() { return XAudio2; }
};

class XAudio2Stream : public iTVPAudioStream, IXAudio2VoiceCallback {
	XAudio2Device* Owner;
	IXAudio2SourceVoice* SourceVoice;

	tTVPAudioStreamParam StreamParam;
	tjs_int AudioVolumeValue;
	tjs_int AudioBalanceValue;

	StreamQueueCallback QueueCallback;
	void* UserData;

private:
	float CalcLeftVolume() const {
		float level = (float)AudioVolumeValue / 100000.0f;
		if( AudioBalanceValue > 0 ) {
			float balance = (float)(100000-AudioBalanceValue) / 100000.0f;
			level *= balance;
		}
		if( level < 0.0f ) level = 0.0f;
		if( level > 1.0f ) level = 1.0f;
		return level;
	}

	float CalcRightVolume() const {
		float level = (float)AudioVolumeValue / 100000.0f;
		if( AudioBalanceValue < 0 ) {
			float balance = (float)(100000+AudioBalanceValue) / 100000.0f;
			level *= balance;
		}
		if( level < 0.0f ) level = 0.0f;
		if( level > 1.0f ) level = 1.0f;
		return level;
	}
	float CalcVolume() const {
		float level = (float)AudioVolumeValue / 100000.0f;
		if( level < 0.0f ) level = 0.0f;
		if( level > 1.0f ) level = 1.0f;
		return level;
	}

public:
	XAudio2Stream( XAudio2Device* parent, const tTVPAudioStreamParam& param );
	virtual ~XAudio2Stream() override {
		if( SourceVoice ) {
			SourceVoice->DestroyVoice();
			SourceVoice = nullptr;
		}
		if( Owner ) {
			Owner->DelStream( this );
			Owner = nullptr;
		}
	}

	virtual void SetCallback( StreamQueueCallback callback, void* user ) override {
		QueueCallback = callback;
		UserData = user;
	}
	virtual void Enqueue( void *data, size_t size, bool last ) override {
		if( SourceVoice ) {
			XAUDIO2_BUFFER bufferDesc = { 0 };
			bufferDesc.Flags      = last ? XAUDIO2_END_OF_STREAM : 0;
			bufferDesc.AudioBytes = size;
			bufferDesc.pAudioData = reinterpret_cast< BYTE * >( data );
			SourceVoice->SubmitSourceBuffer( &bufferDesc );
		}
	}
	virtual void ClearQueue() override {
		if( SourceVoice ) SourceVoice->FlushSourceBuffers();
	}
	virtual tjs_uint32 GetQueuedCount() const override {
		if( SourceVoice ) {
			XAUDIO2_VOICE_STATE state;
			SourceVoice->GetState( &state );
			return state.BuffersQueued;
		}
		return 0;
	}
	virtual tjs_uint64 GetSamplesPlayed() const override {
		if( SourceVoice ) {
			XAUDIO2_VOICE_STATE state;
			SourceVoice->GetState( &state );
			return state.SamplesPlayed;
		}
		return 0;
	}

	virtual void StartStream() override {
		if( SourceVoice ) SourceVoice->Start();
	}
	virtual void StopStream() override {
		if( SourceVoice ) {
			SourceVoice->Stop();
			SourceVoice->FlushSourceBuffers();
		}
	}
	virtual void AbortStream() override {
		if( SourceVoice ) SourceVoice->Stop();
	}

	virtual void SetVolume(tjs_int vol) override {
		if( AudioVolumeValue != vol ) {
			if( vol > iTVPAudioDevice::VOLUME_MAX ) AudioVolumeValue = iTVPAudioDevice::VOLUME_MAX;
			else if( vol < 0 ) AudioVolumeValue = 0;
			else AudioVolumeValue = vol;

			SetVolumeToXAudio();
		}
	}
	virtual tjs_int GetVolume() const override { return AudioVolumeValue; }
	virtual void SetPan(tjs_int pan) override {
		if( AudioBalanceValue != pan ) {
			if( pan < -100000 ) AudioBalanceValue = -100000;
			else if( pan > 100000 ) AudioBalanceValue = 100000;
			else AudioBalanceValue = pan;

			SetVolumeToXAudio();
		}
	}
	virtual tjs_int GetPan() const override { return AudioBalanceValue; }
	virtual void SetFrequency(tjs_int freq) override {
		if( SourceVoice ) {
			float rate = (float)freq / (float)StreamParam.SampleRate;
			SourceVoice->SetFrequencyRatio( rate );
		}
	}
	virtual tjs_int GetFrequency() const override {
		tjs_int freq = 0;
		if( SourceVoice ) {
			float rate = 1.0f;
			SourceVoice->GetFrequencyRatio( &rate );
			freq = (tjs_int)( StreamParam.SampleRate * rate );
		}
		return freq;
	}

	void SetVolumeToXAudio() {
		if( SourceVoice ) {
			UINT32 count = StreamParam.Channels;
			if( count == 2 ) {
				float channels[2];
				channels[0] = CalcLeftVolume();
				channels[1] = CalcRightVolume();
				SourceVoice->SetChannelVolumes( count, &channels[0] );
			} else if( count == 1 ) {
				SourceVoice->SetVolume( CalcVolume() );
			} else {
				float channels[1];
				channels[0] = CalcVolume();
				for( UINT32 i = 0; i < count; i++ ) {
					SourceVoice->SetChannelVolumes( i, channels );
				}
			}
		}
	}

	virtual void STDMETHODCALLTYPE OnStreamEnd() override {}
	virtual void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() override {}
	virtual void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32 SamplesRequired) override {}
	virtual void STDMETHODCALLTYPE OnBufferEnd(void * pBufferContext) override { if( QueueCallback ) QueueCallback( this, UserData ); }
	virtual void STDMETHODCALLTYPE OnBufferStart(void * pBufferContext) override {}
	virtual void STDMETHODCALLTYPE OnLoopEnd(void * pBufferContext) override {}
	virtual void STDMETHODCALLTYPE OnVoiceError(void * pBufferContext, HRESULT Error) override {}
};
XAudio2Device::~XAudio2Device() {
	if( XAudio2DLL ) {
		::FreeLibrary(XAudio2DLL);
		XAudio2DLL = nullptr;
	}
	procXAudio2Create = nullptr;
}
void XAudio2Device::Initialize( tTVPAudioInitParam& param ) {
	if( param.SampleRate < XAUDIO2_MIN_SAMPLE_RATE || param.SampleRate > XAUDIO2_MAX_SAMPLE_RATE ) {
		TVPThrowExceptionMessage(TJS_W("Invalid parameter."));
	}
	TVPAddLog( TJS_W("XAudio2 initializing...") );

	if( XAudio2DLL == nullptr ) {
		XAudio2DLL = ::LoadLibrary( TJS_W("XAudio2_9.dll") );
		if( XAudio2DLL == nullptr ) {
			XAudio2DLL = ::LoadLibrary( TJS_W("XAudio2_8.dll") );
		}
	}
	if( XAudio2DLL == nullptr ) {
		TVPThrowExceptionMessage(TJS_W("Cannot load XAudio2 dll."));
	}
	procXAudio2Create = (HRESULT ( __stdcall *)(IXAudio2 **,UINT32,XAUDIO2_PROCESSOR))GetProcAddress(XAudio2DLL, "XAudio2Create");
	if( !procXAudio2Create ) {
		TVPThrowExceptionMessage(TJS_W("Missing XAudio2Create in XAudio2 dll."));
	}

	HRESULT hr;
	if( FAILED( hr = ::CoInitializeEx( nullptr, COINIT_MULTITHREADED ) ) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to call CoInitializeEx."));
	}

	UINT32 flags = 0;
	//if( FAILED( hr = ::XAudio2Create( &XAudio2, flags ) ) ) {
	if( FAILED( hr = procXAudio2Create( &XAudio2, flags, XAUDIO2_DEFAULT_PROCESSOR ) ) ) {
		::CoUninitialize();
		TVPThrowExceptionMessage(TJS_W("Faild to call XAudio2Create."));
	}
	if( FAILED( hr = XAudio2->CreateMasteringVoice( &MasteringVoice, param.Channels, param.SampleRate ) ) ) {
		::CoUninitialize();
		TVPThrowExceptionMessage(TJS_W("Faild to call CreateMasteringVoice."));
	}

	/*
	XAUDIO2_VOICE_DETAILS details; // 2.7 と 2.8 で違う
	MasteringVoice->GetVoiceDetails( &details );
	TVPAddLog( ttstr(TJS_W( "Number of Channels : " )) + ttstr( to_tjs_string(details.InputChannels)) );
	TVPAddLog( ttstr(TJS_W( "Sample rate : " )) + ttstr( to_tjs_string(details.InputSampleRate)) );
	*/
}

void XAudio2Device::Uninitialize() {
	while( Children.size() ) {
		auto i = Children.begin();
		delete *i;
	}

	if( MasteringVoice ) {
		MasteringVoice->DestroyVoice();
		MasteringVoice = nullptr;
	}
	if( XAudio2 ) {
		XAudio2->Release();
		XAudio2 = nullptr;
	}
	::CoUninitialize();
}
iTVPAudioStream* XAudio2Device::CreateAudioStream( tTVPAudioStreamParam& param ) {
	XAudio2Stream* stream = new XAudio2Stream( this, param );
	AddStream( stream );
	return stream;
}

XAudio2Stream::XAudio2Stream( XAudio2Device* parent, const tTVPAudioStreamParam& param )
: Owner( parent ), SourceVoice(nullptr), AudioVolumeValue(iTVPAudioDevice::VOLUME_MAX),
  AudioBalanceValue(0), QueueCallback(nullptr), UserData(nullptr) {
	memset( &StreamParam, 0, sizeof(StreamParam) );
	WORD  tag = WAVE_FORMAT_PCM;
	if( param.SampleType == astFloat32 ) {
		tag = WAVE_FORMAT_IEEE_FLOAT;
	}
	WAVEFORMATEX sourceWaveFormat;
	sourceWaveFormat.wFormatTag      = tag;
	sourceWaveFormat.nChannels       = param.Channels;
	sourceWaveFormat.nSamplesPerSec  = param.SampleRate;
	sourceWaveFormat.wBitsPerSample  = param.BitsPerSample;
	sourceWaveFormat.nBlockAlign     = ( sourceWaveFormat.wBitsPerSample / 8 ) * sourceWaveFormat.nChannels;
	sourceWaveFormat.nAvgBytesPerSec = sourceWaveFormat.nSamplesPerSec * sourceWaveFormat.nBlockAlign;
	sourceWaveFormat.cbSize = 0;
	HRESULT hr;
	if( FAILED( hr = parent->GetXAudio2()->CreateSourceVoice( &SourceVoice, &sourceWaveFormat,0, XAUDIO2_DEFAULT_FREQ_RATIO, this ) ) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to create source voice."));
	}
	StreamParam = param;
}

iTVPAudioDevice* TVPCreateAudioDevice() {
	return new XAudio2Device();
}


#endif
