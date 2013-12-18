//---------------------------------------------------------------------------

#include <windows.h>
#include "tvpsnd.h"

#include <stdio.h>
//---------------------------------------------------------------------------
#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
	return 1;
}

//---------------------------------------------------------------------------
#pragma pack(push,1)
struct adpcmcoef
{
	short int iCoef1;
	short int iCoef2;
};
struct adpcmwaveformat
{
     WAVEFORMATEX wfxx;
     WORD wSamplesPerBlock;
     WORD wNumCoef;
};
#pragma pack(pop)

static int AdaptationTable[]= 
{
	230, 230, 230, 230, 307, 409, 512, 614,
	768, 614, 512, 409, 307, 230, 230, 230 
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void strcpy_limit(LPWSTR dest, LPWSTR  src, int n)
{
	wcsncpy(dest, src, n-1);
	dest[n-1] = '\0';
}
//---------------------------------------------------------------------------
ITSSStorageProvider *StorageProvider = NULL;
//---------------------------------------------------------------------------
class MSADPCMModule : public ITSSModule
{
	ULONG RefCount;

public:
	MSADPCMModule();
	~MSADPCMModule();

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
class MSADPCMDecoder : public ITSSWaveDecoder
{
	ULONG RefCount;
	adpcmwaveformat Format; // Wave のフォーマット
	adpcmcoef *Coefs; // Coefのセット
	IStream *InputStream;
	__int64 StreamPos;
	short int *SamplePos;
	long DataStart;
	long DataSize;
	__int64 Pos;
	long BufferRemain;
	BYTE *BlockBuffer;
	short int *Samples;
	int Predictor[2];
	int Delta[2];
	TSSWaveFormat TSSFormat;

public:
	MSADPCMDecoder();
	~MSADPCMDecoder();

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

	// そのほか
	HRESULT Open(wchar_t * url);
	bool ReadBlock(void);
};
//---------------------------------------------------------------------------
// MSADPCMModule インプリメンテーション #####################################
//---------------------------------------------------------------------------
MSADPCMModule::MSADPCMModule()
{
	RefCount = 1;
}
//---------------------------------------------------------------------------
MSADPCMModule::~MSADPCMModule()
{
}
//---------------------------------------------------------------------------
HRESULT __stdcall MSADPCMModule::QueryInterface(REFIID iid, void ** ppvObject)
{
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
ULONG __stdcall MSADPCMModule::AddRef()
{
	return ++RefCount;
}
//---------------------------------------------------------------------------
ULONG __stdcall MSADPCMModule::Release()
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
HRESULT __stdcall MSADPCMModule::GetModuleCopyright(LPWSTR buffer, unsigned long buflen)
{
	strcpy_limit(buffer, L"Microsft ADPCM decoder for TVP Sound System (C) 2000 W.Dee <dee@kikyou.info> and contributors",
			buflen);
	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT __stdcall MSADPCMModule::GetModuleDescription(LPWSTR buffer, unsigned long buflen )
{
	strcpy_limit(buffer, L"Microsoft ADPCM decoder (*.wav)", buflen);
	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT __stdcall MSADPCMModule::GetSupportExts(unsigned long index, LPWSTR mediashortname,
												LPWSTR buf, unsigned long buflen )
{
	if(index >= 1) return S_FALSE;
	wcscpy(mediashortname, L"MS-ADPCM Wave ファイル");
	strcpy_limit(buf, L".wav", buflen);
	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT __stdcall MSADPCMModule::GetMediaInfo(LPWSTR url, ITSSMediaBaseInfo ** info )
{
	return E_NOTIMPL;
}
//---------------------------------------------------------------------------
HRESULT __stdcall MSADPCMModule::GetMediaSupport(LPWSTR url )
{
	return E_NOTIMPL;
}
//---------------------------------------------------------------------------
HRESULT __stdcall MSADPCMModule::GetMediaInstance(LPWSTR url, IUnknown ** instance )
{
	HRESULT hr;
	MSADPCMDecoder * decoder = new MSADPCMDecoder();
	hr = decoder->Open(url);
	if(FAILED(hr))
	{
		delete decoder;
		return hr;
	}

	*instance = (IUnknown*)decoder;

	return S_OK;
}
//---------------------------------------------------------------------------
// MSADPCMDecoder インプリメンテーション ####################################
//---------------------------------------------------------------------------
MSADPCMDecoder::MSADPCMDecoder()
{
	RefCount = 1;
	InputStream = NULL;
	Coefs=NULL;
	BlockBuffer=NULL;
	Samples=NULL;
}
//---------------------------------------------------------------------------
MSADPCMDecoder::~MSADPCMDecoder()
{
	if(InputStream)
	{
		InputStream->Release();
		InputStream = NULL;
	}
	if(Coefs) delete [] Coefs;
	if(BlockBuffer) delete [] BlockBuffer;
	if(Samples) delete [] Samples;
}
//---------------------------------------------------------------------------
HRESULT __stdcall MSADPCMDecoder::QueryInterface(REFIID iid, void ** ppvObject)
{
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
ULONG __stdcall MSADPCMDecoder::AddRef()
{
	return ++RefCount;
}
//---------------------------------------------------------------------------
ULONG __stdcall MSADPCMDecoder::Release()
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
HRESULT __stdcall MSADPCMDecoder::GetFormat(TSSWaveFormat *format)
{
	*format = TSSFormat;

	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT __stdcall MSADPCMDecoder::Render(void *buf, unsigned long bufsamplelen,
            unsigned long *rendered, unsigned long *status)
{
	// 展開
	unsigned long n;
	short int *pbuf=(short int*)buf;

	for(n=0;n<bufsamplelen;n++)
	{
		if(BufferRemain<=0)
		{
			if(!ReadBlock()) // デコード
			{
				// デコード終了
				if(rendered) *rendered = n;
				if(status) *status = 0;
				Pos += n;
				return S_OK;
			}
		}

		if(Format.wfxx.nChannels == 1)
		{
			*(pbuf++)=*(SamplePos++);
		}
		else
		{
			*(pbuf++)=*(SamplePos++);
			*(pbuf++)=*(SamplePos++);
		}

		BufferRemain--;
	}

	if(rendered) *rendered = n;
	if(status) *status = 1;
	Pos+=n;
	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT __stdcall MSADPCMDecoder::SetPosition(unsigned __int64 samplepos)
{
	// pos (ms単位) に移動する

	// 現在位置を保存
	LARGE_INTEGER newpos;
	ULARGE_INTEGER result;
	newpos.QuadPart=0;
	InputStream->Seek(newpos,1,&result);

	__int64 bytepossave=(long)newpos.QuadPart;
	__int64 samplepossave=Pos;

	// 新しい位置を特定
	long newbytepos= samplepos / Format.wSamplesPerBlock;
		// この時点で newbytepos = ブロック数
	long remnant = samplepos - newbytepos * Format.wSamplesPerBlock;
	Pos = samplepos;
	newbytepos *= Format.wfxx.nBlockAlign;

	// シーク
	newpos.QuadPart=DataStart+newbytepos;
	InputStream->Seek(newpos,0,&result);
	if(result.QuadPart != (unsigned __int64) newpos.QuadPart)
	{
		// シーク失敗
		newpos.QuadPart=bytepossave;
		InputStream->Seek(newpos,0,&result);
		Pos=samplepossave;
		return E_FAIL;
	}

	StreamPos=DataStart+newbytepos;

	ReadBlock();
	BufferRemain = Format.wSamplesPerBlock - remnant;
	SamplePos += remnant * Format.wfxx.nChannels;

	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT MSADPCMDecoder::Open(wchar_t * url)
{
	// url で指定された URL を開きます
	InputStream = NULL;

	HRESULT hr;

	hr = StorageProvider->GetStreamForRead(url, (IUnknown**)&InputStream);
	if(FAILED(hr))
	{
		InputStream = NULL;
		return hr;
	}

	ULONG read;
	LARGE_INTEGER p;
	p.QuadPart=0;
	InputStream->Seek(p,0,NULL);

	char buf[16];

	LARGE_INTEGER newpos;
	newpos.QuadPart=0;
	ULARGE_INTEGER result;
	InputStream->Seek(newpos,0,&result);
	if(result.QuadPart != (unsigned __int64) newpos.QuadPart)
		return E_FAIL;

	// RIFF チェック
	InputStream->Read(buf,16,&read);
	if(read!=16) return E_FAIL;
	if(memcmp(buf,"RIFF",4)) return E_FAIL; // riff
	if(memcmp(buf+8,"WAVEfmt ",8)) return E_FAIL; // wave+fmt // ここら辺は適当

	InputStream->Read(buf,4,&read); // dummy

	// ヘッダ読み込み
	InputStream->Read(&Format,sizeof(adpcmwaveformat),&read);
	if(read!=sizeof(adpcmwaveformat)) return E_FAIL;
	if(Format.wfxx.wFormatTag!=0x0002) return E_FAIL;

	// coef 読み込み
	Coefs=new adpcmcoef[Format.wNumCoef];
	InputStream->Read(Coefs,sizeof(adpcmcoef)*Format.wNumCoef,&read);
	if(read!=sizeof(adpcmcoef)*Format.wNumCoef) return E_FAIL;

	// data チャンクを探す
	for(;;)
	{
		InputStream->Read(buf,4,&read);
		if(read!=4) return E_FAIL;
		if(!memcmp(buf,"data",4)) break;
		int size;
		InputStream->Read(&size,4,&read);
		if(read!=4) return E_FAIL;
		newpos.QuadPart=size;
		InputStream->Seek(newpos,1,&result);
	}

	// DataSize
	InputStream->Read(&DataSize,4,&read);
	if(read!=4) return E_FAIL;

	// 現在位置を取得
	newpos.QuadPart=0;
	InputStream->Seek(newpos,1,&result);
	StreamPos=(long)result.QuadPart;
	DataStart=StreamPos;

	// その他、初期化
	BufferRemain=0;
	Pos=0;

	ZeroMemory(&TSSFormat, sizeof(TSSFormat));
	TSSFormat.dwSamplesPerSec = Format.wfxx.nSamplesPerSec;
	TSSFormat.dwChannels = Format.wfxx.nChannels;
	TSSFormat.dwBitsPerSample = 16;
	TSSFormat.dwSeekable = 2;
	TSSFormat.ui64TotalSamples = 0;
	TSSFormat.dwTotalTime = 0;

	return S_OK;
}
//---------------------------------------------------------------------------
bool MSADPCMDecoder::ReadBlock(void)
{

	// メモリ確保
	if(!BlockBuffer)
		BlockBuffer=new BYTE[Format.wfxx.nBlockAlign];
	if(!Samples)
		Samples=new short int[Format.wSamplesPerBlock  *  Format.wfxx.nChannels];

	// シーク
	LARGE_INTEGER newpos;
	ULARGE_INTEGER result;
	newpos.QuadPart=0;
	InputStream->Seek(newpos,1,&result);
	if((long)result.QuadPart!=StreamPos)
	{
		newpos.QuadPart=StreamPos;
		InputStream->Seek(newpos,0,&result);
		if(result.QuadPart != (unsigned __int64) newpos.QuadPart) return false;
	}

	// ブロックヘッダ読み込み
	ULONG read;
	InputStream->Read(BlockBuffer,Format.wfxx.nBlockAlign,&read);
	StreamPos+=read;
	if(Format.wfxx.nBlockAlign != (long)read) return false;
	
	if(Format.wfxx.nChannels==1)
	{
		Predictor[0]=BlockBuffer[0];
		Predictor[1]=0;

		Delta[0]=*(short int *)(BlockBuffer+1);
		Delta[1]=0;

		Samples[1]=*(short int*)(BlockBuffer+3);
		Samples[0]=*(short int*)(BlockBuffer+5);
	}
	else
	{
		Predictor[0]=BlockBuffer[0];
		Predictor[1]=BlockBuffer[1];
		
		Delta[0]=*(short int *)(BlockBuffer+2);
		Delta[1]=*(short int *)(BlockBuffer+4);

		Samples[2]=*(short int*)(BlockBuffer+6);
		Samples[3]=*(short int*)(BlockBuffer+8);
		Samples[0]=*(short int*)(BlockBuffer+10);
		Samples[1]=*(short int*)(BlockBuffer+12);
	}

	// サンプル置き場にnibbleデータを配分
	int p=7 * Format.wfxx.nChannels;
	int i=2* Format.wfxx.nChannels;
	while(i< Format.wSamplesPerBlock * Format.wfxx.nChannels)
	{
		BYTE b=BlockBuffer[p++];
		Samples[i++]=b>>4;
		Samples[i++]=b&0x0f;
	}

	// デコード
	int channels=Format.wfxx.nChannels;
	p=2*channels;
	while(p<Format.wSamplesPerBlock*channels)
	{
		int ch;
		for(ch=0;ch<channels;ch++)
		{
			int b=Samples[p]&0xf;

			int idelta=Delta[ch] ;
			Delta[ch]=(AdaptationTable[b]*idelta)>>8;
			if(Delta[ch]<16) Delta[ch] = 16 ;
		
    		int pred=((Samples[p-  channels]*Coefs[Predictor[ch]].iCoef1)+ 
					  (Samples[p-2*channels]*Coefs[Predictor[ch]].iCoef2));
			pred>>=8;
			int current=(  (b>=8?(b-16):b)  * idelta)+pred;
    
		    if(current>32767) current=32767;
			else if (current<-32768) current=-32768 ;

			Samples[p++]=current ;
		}
	}

	BufferRemain = Format.wSamplesPerBlock;
	SamplePos = Samples;

	return true;
}
//---------------------------------------------------------------------------
// ##########################################################################
//---------------------------------------------------------------------------
extern "C" HRESULT _export _stdcall GetModuleInstance(ITSSModule **out,
	ITSSStorageProvider *provider,
	IStream * config, HWND mainwin)
{
	StorageProvider = provider;
	*out = new MSADPCMModule();
	return S_OK;
}
//---------------------------------------------------------------------------

