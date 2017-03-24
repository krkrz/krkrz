
#ifndef __SOUND_SAMPLES_BUFFER_H__
#define __SOUND_SAMPLES_BUFFER_H__


#include "WaveSegmentQueue.h"

class tTVPSoundSamplesBuffer {
	static const int BUFFER_DIVIDER = 2;

	tTJSNI_QueueSoundBuffer* Owner;
	tTVPWaveFormat* Format;	// フォーマット形式
	tjs_uint8* Buffer;		// 実サンプル
	tjs_uint8* VisBuffer;	// ビジュアルバッファ
	tjs_uint Index;			// バッファの通し番号
	tjs_uint SampleSize;	// このバッファに格納可能なサンプル数
	tjs_uint ByteSize;		// このバッファのバイトサイズ
	tjs_uint InSamples;		// このバッファ内の実サンプル数
	tjs_int64 DecodePos;	// 再生が開始されてからのこのバッファの位置
	tTVPWaveSegmentQueue Segment;	// このバッファ範囲のWaveのセグメント・ラベル
	bool UseVisBuffer;

private:
	static void MakeSilentWave( void *dest, tjs_int bytes, tjs_int bitsPerSample ) {
		if( bitsPerSample == 8 ) {
			// 0x80
			memset( dest, 0x80, bytes );
		} else {
			// 0x00
			memset( dest, 0x00, bytes );
		}
	}

public:
	tTVPSoundSamplesBuffer( tTJSNI_QueueSoundBuffer* owner, tjs_uint index ) : Owner(owner), Format(nullptr),
		Buffer(nullptr), VisBuffer(nullptr), Index(index), SampleSize(0), ByteSize(0), InSamples(0),
		DecodePos(-1), UseVisBuffer(false) {
		Segment.Clear();
	}
	~tTVPSoundSamplesBuffer() {
		if( Buffer ) {
			delete[] Buffer;
			Buffer = nullptr;
		}
		if( VisBuffer ) {
			delete[] VisBuffer;
			VisBuffer = nullptr;
		}
		Format = nullptr;
		Owner = nullptr;
	}
	void Create( tTVPWaveFormat* format, bool visBuffer = false ) {
		Format = format;
		SampleSize = format->SamplesPerSec / BUFFER_DIVIDER;	// 1sec / div
		ByteSize = SampleSize * format->BytesPerSample * format->Channels;
		if(ByteSize <= 0)
			TVPThrowExceptionMessage(TJS_W("Invalid format."));
		if( Buffer ) delete[] Buffer;
		Buffer = new tjs_uint8[ByteSize];
		if( VisBuffer ) delete[] VisBuffer, VisBuffer = nullptr;
		UseVisBuffer = visBuffer;
		if( visBuffer ) {
			VisBuffer = new tjs_uint8[ByteSize];
		}
		InSamples = SampleSize;
		DecodePos = -1;
		Segment.Clear();
	}
	void Reset() {
		Segment.Clear();
		DecodePos = -1;
		InSamples = SampleSize;
	}
	tjs_uint8* GetBuffer() { return Buffer; }
	const tjs_uint8* GetBuffer() const { return Buffer; }
	tjs_uint GetSamplesCount() const { return SampleSize; }

	void Decode() {
		tTJSCriticalSectionHolder holder(Owner->GetBufferCS());

		Segment.Clear();
		if( InSamples < SampleSize ) {
			InSamples = 0;
		} else {
			InSamples = Owner->Decode( Buffer, SampleSize, Segment );
		}
		if( InSamples < SampleSize ) {
			tjs_uint blockAlign = Format->BytesPerSample * Format->Channels;
			MakeSilentWave( Buffer + InSamples*blockAlign, (SampleSize - InSamples)*blockAlign, Format->BitsPerSample );
		}
		if( UseVisBuffer ) {
			memcpy( VisBuffer, Buffer, ByteSize );
		}
	}
	void Enqueue( iTVPAudioStream* stream ) {
		stream->Enqueue( Buffer, ByteSize, InSamples != SampleSize );
	}
	void SetDecodePosition( tjs_int64 pos ) { DecodePos = pos; }
	tjs_int64 GetDecodePosition() const { return DecodePos; }

	bool IsEnded() const { return ( InSamples < SampleSize ); }
	tjs_uint GetInSamples() const { return InSamples; }

	void ResetVisBuffer() {
		DeallocateVisBuffer();
		VisBuffer = new tjs_uint8[ByteSize];
		UseVisBuffer = true;
	}
	void DeallocateVisBuffer() {
		if( VisBuffer ) delete[] VisBuffer, VisBuffer = nullptr;
		UseVisBuffer = false;
	}
	const tjs_uint8* GetVisBuffer() const { return VisBuffer; }
	const tTVPWaveSegmentQueue& GetSegmentQueue() const { return Segment; }
};

#endif // __SOUND_SAMPLES_BUFFER_H__
