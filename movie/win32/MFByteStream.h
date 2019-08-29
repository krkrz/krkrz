/****************************************************************************/
/*! @file
@brief Media Foundation 用の ByteStream クラス

-----------------------------------------------------------------------------
	Copyright (C) 2019 T.Imoto ( http://www.kaede-software.com/ )
-----------------------------------------------------------------------------
@author		T.Imoto
@date		2019/07/07
@note
MediaFoundationを使う (7) IMFByteStreamを実装してみる 前編
https://www.timbreofprogram.info/blog/archives/458
を参考に吉里吉里Zに合うように調整して実装。

リンク先では、自前でスレッドとキューを持った実装となっているが、
ここでは Media Foundation の MFPutWorkItem を用いた実装とした。
また BeginRead 内でファイルポインタを進めないと Win7 で上手く動かない ( 
ドキュメントに進めるように記述がある ) ので、進めている。
読み込みは非同期処理にする必要があるため、失敗しているかもしれないが、
このように実装しないと Win7/8.1/10 でうまく動くようにならない。
*****************************************************************************/

#ifndef __MF_BYTE_STREAM__H__
#define __MF_BYTE_STREAM__H__

#include <list>
#include <mfobjects.h>
#include "AsyncCB.h"

class tTVPMFByteStream : public CUnknown, public IMFByteStream {
public:
	tTVPMFByteStream( HRESULT *lphr, IStream* lpIStream, const tjs_char *lpFileName, tjs_uint64 size );
	virtual ~tTVPMFByteStream();

	//IUnknownの機能を実装する
	DECLARE_IUNKNOWN;

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

	//IMFByteStreamの実装
	// ストリームの能力を取得する
	STDMETHODIMP GetCapabilities(DWORD *pdwCapabilities);

	// ストリームのサイズを取得する
	STDMETHODIMP GetLength(QWORD *pqwLength);

	// ストリームのサイズを設定する
	STDMETHODIMP SetLength(QWORD qwLength);

	// ストリームの位置を取得する
	STDMETHODIMP GetCurrentPosition(QWORD *pqwPosition);

	// ストリームの位置を設定する
	STDMETHODIMP SetCurrentPosition(QWORD qwPosition);

	// ストリームの終端かどうか
	STDMETHODIMP IsEndOfStream(BOOL *pfEndOfStream);

	// 同期読み取りを行う
	STDMETHODIMP Read(BYTE *pb,ULONG cb,ULONG *pcbRead);

	// 非同期読み取りを開始する
	STDMETHODIMP BeginRead(BYTE *pb,ULONG cb,IMFAsyncCallback *pCallback,IUnknown *punkState);

	// 非同期読み取りを終了する
	STDMETHODIMP EndRead(IMFAsyncResult *pResult,ULONG *pcbRead);

	// 同期書き込みを行う
	STDMETHODIMP Write(const BYTE *pb,ULONG cb,ULONG *pcbWritten);

	// 非同期書き込みを開始する
	STDMETHODIMP BeginWrite(const BYTE *pb,ULONG cb,IMFAsyncCallback *pCallback,IUnknown *punkState);

	// 非同期書き込みを終了する
	STDMETHODIMP EndWrite(IMFAsyncResult *pResult,ULONG *pcbWritten);

	// ストリーム位置を移動する
	STDMETHODIMP Seek(MFBYTESTREAM_SEEK_ORIGIN SeekOrigin,LONGLONG llSeekOffset,DWORD dwSeekFlags,QWORD *pqwCurrentPosition);

	// 書き込みバッファのフラッシュを行う
	STDMETHODIMP Flush();

	// ストリームを閉じる
	STDMETHODIMP Close();

	// ストリームを開く
	HRESULT Open();

	// 非同期用の読み込み処理
	HRESULT ReadForAsync(BYTE *pb,ULONG cb,ULONG *pcbRead,uint64_t nReadPos);
protected:
	// ストリームが有効かどうか
	inline bool IsValidStream() const { return FileStream != nullptr; }

private:
	tjs_string FileName;
	IStream* FileStream;

	tjs_uint64 StreamPos;	// ストリーム位置
	tjs_uint64 StreamSize;	// ストリームサイズ

	CCritSec  CSStream;

	CComPtr<IMFAttributes> MFAttributes;


	class AsyncReader : public IUnknown {
	public:
		AsyncReader( tTVPMFByteStream* owner, BYTE* dest=nullptr, ULONG size=0, tjs_uint64 offset=0 ) : RefCount(1), Owner(owner), DestBuffer(dest), ByteCount( size ), ReadSize(0), ReadOffset(offset) {}
		STDMETHODIMP QueryInterface( REFIID iid, void** ppv ) {
			if( !ppv ) return E_POINTER;
			if( iid == IID_IUnknown ) *ppv = static_cast<IUnknown*>(this);
			else {
				*ppv = nullptr;
				return E_NOINTERFACE;
			}
			AddRef();
			return S_OK;
		}
		STDMETHODIMP_( ULONG ) AddRef() { return InterlockedIncrement(&RefCount); }
		STDMETHODIMP_( ULONG ) Release() {
			ULONG uCount = InterlockedDecrement(&RefCount);
			if( uCount == 0 ) delete this;
			return uCount;
		}

		HRESULT DoRead();
		ULONG GetReadSize() const { return ReadSize; }
		void SetDestAndSize( BYTE* dest, ULONG size, ULONG offset ) {
			DestBuffer = dest;
			ByteCount = size;
			ReadSize = 0;
			ReadOffset = 0;
		}

	private:
		long	RefCount;

		tTVPMFByteStream* Owner;
		BYTE*	DestBuffer;
		ULONG	ByteCount;
		ULONG	ReadSize;
		tjs_uint64	ReadOffset;

		virtual	~AsyncReader();
	};

	DWORD							WorkQueueId;
	AsyncCallback<tTVPMFByteStream>	WorkQueueCB;

	HRESULT OnDispatchRead(IMFAsyncResult* pAsyncResult);
};


#endif // __MF_BYTE_STREAM__H__
