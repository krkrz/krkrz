//---------------------------------------------------------------------------
// Opus plugin for TSS ( stands for TVP Sound System )
// This is released under TVP(KirikiriZ)'s license.
// See details for TVP source distribution.
//---------------------------------------------------------------------------

#define _CRT_SECURE_NO_WARNINGS

extern "C" {
#include "opusfile.h"
}

#include <windows.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "tp_stub.h"
#include "tvpsnd.h" // TSS sound system interface definitions

static bool FloatExtraction = false; // true if output format is IEEE 32-bit float

//---------------------------------------------------------------------------
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
	return 1;
}
//---------------------------------------------------------------------------
void strcpy_limit(LPWSTR dest, LPWSTR  src, int n)
{
	// string copy with limitation
	// this will add a null terminater at destination buffer
	wcsncpy(dest, src, n-1);
	dest[n-1] = '\0';
}
//---------------------------------------------------------------------------
ITSSStorageProvider *StorageProvider = NULL;
//---------------------------------------------------------------------------
class OpusModule : public ITSSModule // module interface
{
	ULONG RefCount; // reference count

public:
	OpusModule();
	~OpusModule();

public:
	// IUnknown
	HRESULT __stdcall QueryInterface(REFIID iid, void ** ppvObject);
	ULONG __stdcall AddRef(void);
	ULONG __stdcall Release(void);
	
	// ITSSModule
	HRESULT __stdcall GetModuleCopyright(LPWSTR buffer, unsigned long buflen );
	HRESULT __stdcall GetModuleDescription(LPWSTR buffer, unsigned long buflen );
	HRESULT __stdcall GetSupportExts(unsigned long index, LPWSTR mediashortname, LPWSTR buf, unsigned long buflen );
	HRESULT __stdcall GetMediaInfo(LPWSTR url, ITSSMediaBaseInfo ** info );
	HRESULT __stdcall GetMediaSupport(LPWSTR url );
	HRESULT __stdcall GetMediaInstance(LPWSTR url, IUnknown ** instance );
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
class OpusWaveDecoder : public ITSSWaveDecoder // decoder interface
{
	ULONG RefCount; // refernce count
	bool InputFileInit; // whether InputFile is inited
	OggOpusFile* InputFile; // OggOpusFile instance
	IStream *InputStream; // input stream
	TSSWaveFormat Format; // output PCM format
	int CurrentSection; // current section in opus stream

public:
	OpusWaveDecoder();
	~OpusWaveDecoder();

public:
	// IUnkown
	HRESULT __stdcall QueryInterface(REFIID iid, void ** ppvObject);
	ULONG __stdcall AddRef(void);
	ULONG __stdcall Release(void);

	// ITSSWaveDecoder
	HRESULT __stdcall GetFormat(TSSWaveFormat *format);
	HRESULT __stdcall Render(void *buf, unsigned long bufsamplelen,
            unsigned long *rendered, unsigned long *status);
	HRESULT __stdcall SetPosition(unsigned __int64 samplepos);

	// others
	HRESULT SetStream(IStream *stream, LPWSTR url);

private:
	int static _cdecl read_func(void *stream, unsigned char *ptr, int nbytes);
	int static _cdecl seek_func(void *stream, opus_int64 offset, int whence);
	int static _cdecl close_func(void *stream);
	opus_int64 static _cdecl tell_func(void *stream);
};
//---------------------------------------------------------------------------
// OpusModule implementation ##############################################
//---------------------------------------------------------------------------
OpusModule::OpusModule()
{
	// OpusModule constructor
	RefCount = 1;
}
//---------------------------------------------------------------------------
OpusModule::~OpusModule()
{
	// OpusModule destructor
}
//---------------------------------------------------------------------------
HRESULT __stdcall OpusModule::QueryInterface(REFIID iid, void ** ppvObject)
{
	// IUnknown::QueryInterface

	if(!ppvObject) return E_INVALIDARG;

	*ppvObject=NULL;
	if(!memcmp(&iid,&IID_IUnknown,16))
		*ppvObject=(IUnknown*)this;
	else if(!memcmp(&iid,&IID_ITSSModule,16))
		*ppvObject=(ITSSModule*)this;

	if(*ppvObject)
	{
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}
//---------------------------------------------------------------------------
ULONG __stdcall OpusModule::AddRef()
{
	return ++RefCount;
}
//---------------------------------------------------------------------------
ULONG __stdcall OpusModule::Release()
{
	if(RefCount == 1)
	{
		delete this;
		return 0;
	}
	else
	{
		return --RefCount;
	}
}
//---------------------------------------------------------------------------
HRESULT __stdcall OpusModule::GetModuleCopyright(LPWSTR buffer, unsigned long buflen)
{
	// return module copyright information
	strcpy_limit(buffer, L"Opus Plug-in for TVP Sound System (C) 2013 T.Imoto", buflen);
	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT __stdcall OpusModule::GetModuleDescription(LPWSTR buffer, unsigned long buflen )
{
	// return module description
	strcpy_limit(buffer, L"Opus (*.opus) decoder", buflen);
	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT __stdcall OpusModule::GetSupportExts(unsigned long index, LPWSTR mediashortname,
												LPWSTR buf, unsigned long buflen )
{
	// return supported file extensios
	if(index >= 1) return S_FALSE;
	wcscpy(mediashortname, L"Opus Stream Format");
	strcpy_limit(buf, L".opus", buflen);
	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT __stdcall OpusModule::GetMediaInfo(LPWSTR url, ITSSMediaBaseInfo ** info )
{
	// return media information interface
	return E_NOTIMPL; // not implemented
}
//---------------------------------------------------------------------------
HRESULT __stdcall OpusModule::GetMediaSupport(LPWSTR url )
{
	// return media support interface
	return E_NOTIMPL; // not implemented
}
//---------------------------------------------------------------------------
HRESULT __stdcall OpusModule::GetMediaInstance(LPWSTR url, IUnknown ** instance )
{
	// create a new media interface
	HRESULT hr;

	// retrieve input stream interface
	IStream *stream;
	hr = StorageProvider->GetStreamForRead(url, (IUnknown**)&stream);
	if(FAILED(hr)) return hr;

	// create Opus decoder
	OpusWaveDecoder * decoder = new OpusWaveDecoder();
	hr = decoder->SetStream(stream, url);
	if(FAILED(hr))
	{
		// error; stream may not be a Opus stream
		delete decoder;
		stream->Release();
		return hr;
	}

	*instance = (IUnknown*)decoder; // return as IUnknown
	stream->Release(); // release stream because the decoder already holds it

	return S_OK;
}
//---------------------------------------------------------------------------
// OpusWaveDecoder implementation #########################################
//---------------------------------------------------------------------------
OpusWaveDecoder::OpusWaveDecoder()
{
	// OpusWaveDecoder constructor
	RefCount = 1;
	InputFileInit = false;
	InputFile = NULL;
	InputStream = NULL;
	CurrentSection = -1;
}
//---------------------------------------------------------------------------
OpusWaveDecoder::~OpusWaveDecoder()
{
	// OpusWaveDecoder destructor
	if(InputFileInit)
	{
		op_free(InputFile);
		InputFileInit = false;
		InputFile = NULL;
	}
	if(InputStream)
	{
		InputStream->Release();
		InputStream = NULL;
	}
}
//---------------------------------------------------------------------------
HRESULT OpusWaveDecoder::QueryInterface(REFIID iid, void ** ppvObject)
{
	// IUnknown::QueryInterface

	if(!ppvObject) return E_INVALIDARG;

	*ppvObject=NULL;
	if(!memcmp(&iid,&IID_IUnknown,16))
		*ppvObject=(IUnknown*)this;
	else if(!memcmp(&iid,&IID_ITSSWaveDecoder,16))
		*ppvObject=(ITSSWaveDecoder*)this;

	if(*ppvObject)
	{
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}
//---------------------------------------------------------------------------
ULONG __stdcall OpusWaveDecoder::AddRef()
{
	return ++RefCount;
}
//---------------------------------------------------------------------------
ULONG __stdcall OpusWaveDecoder::Release()
{
	if(RefCount == 1)
	{
		delete this;
		return 0;
	}
	else
	{
		return --RefCount;
	}
}
//---------------------------------------------------------------------------
HRESULT __stdcall OpusWaveDecoder::GetFormat(TSSWaveFormat *format)
{
	// return PCM format
	if(!InputFileInit)
	{
		return E_FAIL;
	}

	*format = Format;

	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT __stdcall OpusWaveDecoder::Render(void *buf, unsigned long bufsamplelen,
            unsigned long *rendered, unsigned long *status)
{
	// render output PCM
	if(!InputFileInit) return E_FAIL; // InputFile is yet not inited

	int pcmsize = FloatExtraction?4:2;

	int res;
	int pos = 0; // decoded PCM (in bytes)
	const int ch = Format.dwChannels;
	int remain = bufsamplelen*ch;
	if( FloatExtraction ) {
		while( remain ) {
			do {
				res = op_read_float( InputFile, (float*)((char*)buf + pos), remain, &CurrentSection );
			} while(res<0);
			if(res==0) break;
			pos += res * ch * pcmsize;
			remain -= res * ch;
		}
	} else {
		while( remain ) {
			do {
				res = op_read( InputFile, (opus_int16*)((char*)buf + pos), remain, &CurrentSection );
			} while(res<0); // ov_read would return a negative number
							// if the decoding is not ready
			if(res==0) break;
			pos += res * ch * pcmsize;
			remain -= res * ch;
		}
	}

	pos /= (Format.dwChannels * pcmsize); // convert to PCM position
	
	if(status)
	{
		*status = ((unsigned int)pos < bufsamplelen)?0:1;
			// *status will be 0 if the decoding is ended
	}

	if(rendered)
	{
		*rendered = pos; // return renderd PCM samples
	}

	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT __stdcall OpusWaveDecoder::SetPosition(unsigned __int64 samplepos)
{
	// set PCM position (seek)
	if(!InputFileInit) return E_FAIL;

	if(0 != op_pcm_seek(InputFile, samplepos))
	{
		return E_FAIL;
	}

	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT OpusWaveDecoder::SetStream(IStream *stream, LPWSTR url)
{
	// set input stream
	InputStream = stream;
	InputStream->AddRef(); // add-ref

	OpusFileCallbacks callbacks = {read_func, seek_func, tell_func, close_func };
		// callback functions

	// open input stream via op_open_callbacks
	int err = 0;
	InputFile = op_open_callbacks( this, &callbacks, NULL, 0, &err);
	if( err != 0 )
	{
		// error!
		InputStream->Release();
		InputStream = NULL;
		return E_FAIL;
	}

	InputFileInit = true;

	// retrieve PCM information
	const OpusHead *oh;
	oh = op_head( InputFile, -1 );
	if(!oh)
	{
		return E_FAIL;
	}

	// set Format up
	ZeroMemory(&Format, sizeof(Format));
	Format.dwSamplesPerSec = 48000; // Opus always output 48kHz
	Format.dwChannels = oh->channel_count;
	Format.dwBitsPerSample = FloatExtraction ? (0x10000 + 32) :  16;
	Format.dwSeekable = 2;

	ogg_int64_t pcmtotal = op_pcm_total(InputFile, -1); // PCM total samples
	if(pcmtotal<0) pcmtotal = 0;
	Format.ui64TotalSamples = pcmtotal;

	double timetotal = (double)pcmtotal / 48000.0;
	if(timetotal<0) Format.dwTotalTime = 0; else Format.dwTotalTime = (unsigned long)( timetotal * 1000.0 );

	return S_OK;
}
//---------------------------------------------------------------------------
int _cdecl OpusWaveDecoder::read_func(void *stream, unsigned char *ptr, int nbytes)
{
	// read function (wrapper for IStream)

	OpusWaveDecoder * decoder = (OpusWaveDecoder*)stream;
	if(!decoder->InputStream) return 0;

	ULONG bytesread;
	if(FAILED(decoder->InputStream->Read(ptr, (ULONG)nbytes, &bytesread)))
	{
		return -1; // failed
	}

	return bytesread;
}
//---------------------------------------------------------------------------
int _cdecl OpusWaveDecoder::seek_func(void *stream, opus_int64 offset, int whence)
{
	// seek function (wrapper for IStream)

	OpusWaveDecoder * decoder = (OpusWaveDecoder*)stream;
	if(!decoder->InputStream) return -1;

	LARGE_INTEGER newpos;
	ULARGE_INTEGER result;
	newpos.QuadPart = offset;
	int seek_type = STREAM_SEEK_SET;
	
	switch(whence)
	{
	case SEEK_SET:
		seek_type = STREAM_SEEK_SET;
		break;
	case SEEK_CUR:
		seek_type = STREAM_SEEK_CUR;
		break;
	case SEEK_END:
		seek_type = STREAM_SEEK_END;
		break;
	}

	if(FAILED(decoder->InputStream->Seek(newpos, seek_type, &result)))
	{
		return -1;
	}

	return 0;
}
//---------------------------------------------------------------------------
int _cdecl OpusWaveDecoder::close_func(void *stream)
{
	// close function (wrapper for IStream)

	OpusWaveDecoder * decoder = (OpusWaveDecoder*)stream;
	if(!decoder->InputStream) return EOF;
	
	decoder->InputStream->Release();
	decoder->InputStream = NULL;

	return 0;
}
//---------------------------------------------------------------------------
opus_int64 _cdecl OpusWaveDecoder::tell_func(void *stream)
{
	// tell function (wrapper for IStream)

	OpusWaveDecoder * decoder = (OpusWaveDecoder*)stream;
	if(!decoder->InputStream) return EOF;

	LARGE_INTEGER newpos;
	ULARGE_INTEGER result;
	newpos.QuadPart = 0;

	if(FAILED(decoder->InputStream->Seek(newpos, STREAM_SEEK_CUR, &result)))
	{
		return EOF;
	}
	return result.QuadPart;
}
//---------------------------------------------------------------------------
// ##########################################################################
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HRESULT _stdcall GetModuleInstance(ITSSModule **out, ITSSStorageProvider *provider,
	IStream * config, HWND mainwin)
{
	// GetModuleInstance function (exported)
	StorageProvider = provider;
	*out = new OpusModule();
	return S_OK;
}
//---------------------------------------------------------------------------
extern "C" __declspec(dllexport) HRESULT _stdcall V2Link(iTVPFunctionExporter *exporter)
{
	TVPInitImportStub(exporter);
	
	ttstr debug_str(L"opus:");

	tTJSVariant val;
	if(TVPGetCommandLine(TJS_W("-opus_gain"), &val))
	{
		double db = (tTVReal)val;
		double fac = pow(10.0, db / 20);
		debug_str = TJS_W("opus: Setting global gain to ");
		val = (tTVReal)db;
		debug_str += ttstr(val);
		debug_str += TJS_W("dB (");
		val = (tTVReal)(fac * 100);
		debug_str += ttstr(val);
		debug_str += TJS_W("%)");
		TVPAddLog(debug_str);
		// op_set_gain_offset ÇåƒÇ—èoÇµÇΩï˚Ç™ó«Ç≥ÇªÇ§
	}

	if(TVPGetCommandLine(TJS_W("-opus_pcm_format"), &val))
	{
		ttstr sval(val);
		if(sval == TJS_W("f32"))
		{
			FloatExtraction = true;
			TVPAddLog(TJS_W("krobus: IEEE 32bit float output enabled."));
		}
	}
	return S_OK;
}
//---------------------------------------------------------------------------
extern "C" __declspec(dllexport) HRESULT _stdcall V2Unlink()
{
	TVPUninitImportStub();
	return S_OK;
}
//---------------------------------------------------------------------------
