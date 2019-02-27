/**
 * 組み込み opus sound decocer
 */
#include "tjsCommHead.h"
#include "DebugIntf.h"
#include "SysInitIntf.h"
#include "StorageIntf.h"
#include "WaveIntf.h"

extern "C" {
#include "opusfile.h"
}

#include <cmath>
#include <memory>

static bool FloatExtraction = false; // true if output format is IEEE 32-bit float
static bool TVPOpusOptionsInit = false;
static void TVPInitOpusOptions() {
	if(TVPOpusOptionsInit) return;

	// retrieve options from commandline
	ttstr debug_str(L"opus:");
	tTJSVariant val;
	if( TVPGetCommandLine(TJS_W("-opus_gain"), &val) ) {
		double db = (tTVReal)val;
		double fac = std::pow(10.0, db / 20);
		debug_str = TJS_W("opus: Setting global gain to ");
		val = (tTVReal)db;
		debug_str += ttstr(val);
		debug_str += TJS_W("dB (");
		val = (tTVReal)(fac * 100);
		debug_str += ttstr(val);
		debug_str += TJS_W("%)");
		TVPAddLog(debug_str);
		// op_set_gain_offset を呼び出した方が良さそう
	}

	if( TVPGetCommandLine(TJS_W("-opus_pcm_format"), &val) ) {
		ttstr sval(val);
		if( sval == TJS_W("f32") ) {
			FloatExtraction = true;
			TVPAddLog(TJS_W("opus: IEEE 32bit float output enabled."));
		}
	}

	TVPOpusOptionsInit = true;
}
//---------------------------------------------------------------------------
// tTVPWD_RIFFWave
//---------------------------------------------------------------------------
class tTVPWD_Opus : public tTVPWaveDecoder
{
	std::unique_ptr<tTJSBinaryStream> Stream;
	bool InputFileInit; // whether InputFile is inited
	std::unique_ptr<OggOpusFile, decltype(&op_free)> InputFile; // OggOpusFile instance
	tTVPWaveFormat Format; // output PCM format
	int CurrentSection; // current section in opus stream

public:
	tTVPWD_Opus( std::unique_ptr<tTJSBinaryStream>&& stream ) : Stream(std::move(stream)), InputFileInit(false), InputFile(nullptr, op_free), CurrentSection(-1) {
		TVPInitOpusOptions();
	}
	virtual ~tTVPWD_Opus() {};
	bool CheckFormat() {
		// open input stream via op_open_callbacks
		int err = 0;
		OpusFileCallbacks callbacks = { read_func, seek_func, tell_func, close_func };
		InputFile.reset( op_open_callbacks( this, &callbacks, NULL, 0, &err) );
		if( err != 0 ) {
			// error!
			return false;
		}
		InputFileInit = true;

		// retrieve PCM information
		const OpusHead *oh;
		oh = op_head( InputFile.get(), -1 );
		if( !oh ) {
			return false;
		}

		// set Format up
		memset( &Format, 0, sizeof(Format) );
		Format.SamplesPerSec = 48000; // Opus always output 48kHz
		Format.Channels = oh->channel_count;
		Format.BitsPerSample = FloatExtraction ? (0x10000 + 32) :  16;
		Format.BytesPerSample = Format.BitsPerSample / 8;
		Format.SpeakerConfig = 0;
		Format.IsFloat = FloatExtraction;
		Format.Seekable = true;

		ogg_int64_t pcmtotal = op_pcm_total(InputFile.get(), -1); // PCM total samples
		if( pcmtotal < 0 ) pcmtotal = 0;
		Format.TotalSamples = pcmtotal;

		double timetotal = (double)pcmtotal / 48000.0;
		if( timetotal < 0 ) {
			Format.TotalTime = 0;
		} else {
			Format.TotalTime = (tjs_uint64)( timetotal * 1000.0 );
		}

		return true;
	}

	/** Retrieve PCM format, etc. */
	void GetFormat(tTVPWaveFormat & format) override { format = Format; }

	/**
		Render PCM from current position.
		where "buf" is a destination buffer, "bufsamplelen" is the buffer's
		length in sample granule, "rendered" is to be an actual number of
		written sample granule.
		returns whether the decoding is to be continued.
		because "redered" can be lesser than "bufsamplelen", the player
		should not end until the returned value becomes false.
	*/
	bool Render(void *buf, tjs_uint bufsamplelen, tjs_uint& rendered)  override {
		// render output PCM
		if(!InputFileInit) return false; // InputFile is yet not inited

		int pcmsize = FloatExtraction ? 4 : 2;
		int res;
		int pos = 0; // decoded PCM (in bytes)
		const int ch = Format.Channels;
		int remain = bufsamplelen * ch;
		if( FloatExtraction ) {
			while( remain ) {
				do {
					res = op_read_float( InputFile.get(), (float*)((char*)buf + pos), remain, &CurrentSection );
				} while( res < 0 );
				if( res == 0 ) break;
				pos += res * ch * pcmsize;
				remain -= res * ch;
			}
		} else {
			while( remain ) {
				do {
					res = op_read( InputFile.get(), (opus_int16*)((char*)buf + pos), remain, &CurrentSection );
				} while( res < 0 ); // ov_read would return a negative number
								// if the decoding is not ready
				if( res == 0 ) break;
				pos += res * ch * pcmsize;
				remain -= res * ch;
			}
		}

		pos /= (ch * pcmsize); // convert to PCM position
		rendered = pos; // return renderd PCM samples
		if((unsigned int)pos < bufsamplelen)
			return false;	// end of stream

		return true;
	}

	/*
		Seek to "samplepos". "samplepos" must be given in unit of sample granule.
		returns whether the seeking is succeeded.
	*/
	bool SetPosition(tjs_uint64 samplepos)  override {
		// set PCM position (seek)
		if(!InputFileInit) return false;

		if( 0 != op_pcm_seek(InputFile.get(), samplepos) ) {
			return false;
		}
		return true;
	}

private:
	int static read_func(void *stream, unsigned char *ptr, int nbytes) {
		// read function (wrapper for IStream)
		tTVPWD_Opus * decoder = (tTVPWD_Opus*)stream;
		if( !decoder->Stream ) return 0;
		int bytesread = static_cast<int>(decoder->Stream->Read(ptr, static_cast<tjs_uint>(nbytes)));
		if( bytesread >= 0 ) return bytesread;
		return -1; // failed
	}
	int static seek_func(void *stream, opus_int64 offset, int whence) {
		// seek function (wrapper for IStream)
		tTVPWD_Opus * decoder = (tTVPWD_Opus*)stream;
		if( !decoder->Stream ) return -1;

		int seek_type = TJS_BS_SEEK_SET;
		switch(whence)
		{
		case SEEK_SET:
			seek_type = TJS_BS_SEEK_SET;
			break;
		case SEEK_CUR:
			seek_type = TJS_BS_SEEK_CUR;
			break;
		case SEEK_END:
			seek_type = TJS_BS_SEEK_END;
			break;
		}
		tjs_uint64 curpos = decoder->Stream->GetPosition();
		tjs_uint64 newpos = decoder->Stream->Seek(static_cast<tjs_int64>(offset), seek_type);
		return curpos != newpos ? 0 : 1;
	}
	int static close_func(void *stream) {
		tTVPWD_Opus * decoder = (tTVPWD_Opus*)stream;
		if( !decoder->Stream ) return EOF;
		decoder->Stream.reset();
		return 0;
	}
	opus_int64 static tell_func(void *stream) {
		tTVPWD_Opus * decoder = (tTVPWD_Opus*)stream;
		if( !decoder->Stream ) return EOF;
		return static_cast<opus_int64>(decoder->Stream->GetPosition());
	}
};

//---------------------------------------------------------------------------
// Opus Decoder creator
//---------------------------------------------------------------------------
class tTVPWDC_Opus : public tTVPWaveDecoderCreator
{
public:
	tTVPWaveDecoder * Create(const ttstr & storagename, const ttstr & extension);
};
//---------------------------------------------------------------------------
tTVPWaveDecoder * tTVPWDC_Opus::Create(const ttstr & storagename, const ttstr &extension)
{
	if(extension != TJS_W(".opus")) return nullptr;

	try {
		std::unique_ptr<tTJSBinaryStream> stream( TVPCreateStream(storagename) );
		if( stream ) {
			std::unique_ptr<tTVPWD_Opus> decoder( new tTVPWD_Opus( std::move(stream) ) );
			if( decoder->CheckFormat() == false ) {
				return nullptr;
			}
			return decoder.release();
		}
	} catch(...) {
		return nullptr;
	}
	return nullptr;
}
//---------------------------------------------------------------------------
tTVPWDC_Opus OpusDecoderCreator;
//---------------------------------------------------------------------------
void TVPRegisterOpusDecoderCreator()
{
	TVPRegisterWaveDecoderCreator(&OpusDecoderCreator);
}
//---------------------------------------------------------------------------

