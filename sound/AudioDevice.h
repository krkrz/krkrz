
#ifndef _AUDIO_DEVICE_H__
#define _AUDIO_DEVICE_H__

struct tTVPAudioInitParam {
	tjs_uint32 Channels;
	tjs_uint32 SampleRate;
};
enum TVPAudioSampleType {
	astUInt8,
	astInt16,
	astInt32,
	astFloat32,
};
struct tTVPAudioStreamParam {
	tjs_uint32 Channels;		// チャンネル数
	tjs_uint32 SampleRate;		// サンプリングレート
	tjs_uint32 BitsPerSample;	// サンプル当たりのビット数
	TVPAudioSampleType SampleType;	// サンプルの形式
	tjs_uint32 FramesPerBuffer;		// 1回のキューイングで入れるサンプル数
};
/**
 * インスタンスは1個のみ作られる。
 */
class iTVPAudioDevice
{
public:
	static const int VOLUME_MAX = 100000;
	virtual ~iTVPAudioDevice(){}
	virtual void Initialize( tTVPAudioInitParam& param ) = 0;
	virtual void Uninitialize() = 0;
	virtual class iTVPAudioStream* CreateAudioStream( tTVPAudioStreamParam& param ) = 0;
	virtual void SetMasterVolume(tjs_int vol) = 0;
	virtual tjs_int GetMasterVolume() const = 0;
};

typedef void (*StreamQueueCallback)( class iTVPAudioStream*, void* user );

class iTVPAudioStream
{
public:
	virtual ~iTVPAudioStream(){}
	virtual void SetCallback( StreamQueueCallback callback, void* user ) = 0;
	virtual void Enqueue( void *data, size_t size, bool last ) = 0;
	virtual void ClearQueue() = 0;

	virtual void StartStream() = 0;
	virtual void StopStream() = 0;
	virtual void AbortStream() = 0;

	virtual tjs_uint32 GetQueuedCount() const = 0;
	virtual tjs_uint64 GetSamplesPlayed() const = 0;

	virtual void SetVolume(tjs_int vol) = 0;
	virtual tjs_int GetVolume() const = 0;
	virtual void SetPan(tjs_int pan) = 0;
	virtual tjs_int GetPan() const = 0;
	virtual void SetFrequency(tjs_int freq) = 0;
	virtual tjs_int GetFrequency() const = 0;
};

extern iTVPAudioDevice* TVPCreateAudioDevice();

#endif // _AUDIO_DEVICE_H__
