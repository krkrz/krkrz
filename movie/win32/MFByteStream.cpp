/****************************************************************************/
/*! @file
@brief Media Foundation 用の ByteStream クラス

-----------------------------------------------------------------------------
	Copyright (C) 2019 T.Imoto ( http://www.kaede-software.com/ )
-----------------------------------------------------------------------------
@author		T.Imoto
@date		2019/07/07
@note
MediaFoundationを使う (8) IMFByteStreamを実装してみる 後編
https://www.timbreofprogram.info/blog/archives/460
を参考に吉里吉里Zに合うように調整して実装。

ただし、読み込み処理周りはかなり異なる実装となっている。
理由はヘッダーのコメントを参照。
*****************************************************************************/


#include "tjsCommHead.h"
#include "MsgIntf.h"
#include "tjsUtils.h"
#include "tjsString.h"
#include "DebugIntf.h"

#include <windows.h>

#include <Mfidl.h>
#include <mfapi.h>
#include <mferror.h>
#include <streams.h>
#include <atlbase.h>
#include <atlcom.h>
#include "MFByteStream.h"


//#define MF_D_LOG

tTVPMFByteStream::tTVPMFByteStream( HRESULT *lphr, IStream* lpIStream, const tjs_char *lpFileName, tjs_uint64 size )
 : CUnknown( TJS_W( "tTVPMFByteStream" ), nullptr ), FileName(lpFileName), FileStream(lpIStream), StreamSize(size),
	WorkQueueId(0), WorkQueueCB(this,&tTVPMFByteStream::OnDispatchRead)
{
	HRESULT hr;
	hr = MFCreateAttributes( &MFAttributes, 0 );
	NonDelegatingAddRef();
	if( FileStream != nullptr ) FileStream->AddRef();
	if( lphr != nullptr) { *lphr = hr; }
}

tTVPMFByteStream::~tTVPMFByteStream()
{
	Close();
	if( MFAttributes.p ) {
		MFAttributes.Release();
	}
}

STDMETHODIMP tTVPMFByteStream::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
	if(IsEqualIID(riid,IID_IMFByteStream)) return GetInterface(static_cast<IMFByteStream *>(this),ppv);
	else if(IsEqualIID(riid,IID_IMFAttributes)) return GetInterface(MFAttributes.p,ppv);
	return CUnknown::NonDelegatingQueryInterface(riid,ppv);
}
// ストリームを開く
HRESULT tTVPMFByteStream::Open()
{
	HRESULT hr = MFAllocateWorkQueue( &WorkQueueId );
	return hr;
}
// ストリームを閉じる
STDMETHODIMP tTVPMFByteStream::Close()
{
	MFUnlockWorkQueue( WorkQueueId );

	CAutoLock holder( &CSStream );
	if( IsValidStream() ){
		if( FileStream ) {
			FileStream->Release();
			FileStream = nullptr;
		}
		StreamSize = 0;
	}
	return S_OK;
}
// ストリームの能力を取得する
STDMETHODIMP tTVPMFByteStream::GetCapabilities(DWORD *pdwCapabilities)
{
	CheckPointer(pdwCapabilities,E_POINTER);
	if(!IsValidStream()) return E_HANDLE;
	*pdwCapabilities = MFBYTESTREAM_IS_READABLE | MFBYTESTREAM_IS_SEEKABLE;
	return S_OK;
}
// ストリームのサイズを取得する
STDMETHODIMP tTVPMFByteStream::GetLength(QWORD *pqwLength)
{
	CheckPointer(pqwLength,E_POINTER);
	*pqwLength = StreamSize;
#ifdef MF_D_LOG
	TVPAddLog( TJS_W( "File size : " ) + to_tjs_string( StreamSize ) );
#endif
	return S_OK;
}
// ストリームのサイズを設定する
STDMETHODIMP tTVPMFByteStream::SetLength(QWORD qwLength)
{
	return E_NOTIMPL;
}
// ストリームの位置を取得する
STDMETHODIMP tTVPMFByteStream::GetCurrentPosition( QWORD *pqwPosition ) {
#ifdef MF_D_LOG
	TVPAddLog( TJS_W( "call GetCurrentPosition." ) );
#endif
	CheckPointer( pqwPosition, E_POINTER );
	if( !IsValidStream() ) return E_HANDLE;

	CAutoLock holder( &CSStream );
	*pqwPosition = StreamPos;
	return S_OK;
}
// ストリームの位置を設定する
STDMETHODIMP tTVPMFByteStream::SetCurrentPosition(QWORD qwPosition)
{
	CAutoLock holder( &CSStream );
	StreamPos = qwPosition;
	if( StreamPos > StreamSize ) StreamPos = StreamSize;
	return S_OK;
}

// ストリームの終端かどうか
STDMETHODIMP tTVPMFByteStream::IsEndOfStream(BOOL *pfEndOfStream)
{
#ifdef MF_D_LOG
	TVPAddLog( TJS_W( "call IsEndOfStream." ) );
#endif
	CheckPointer( pfEndOfStream, E_POINTER );
	CAutoLock holder( &CSStream );
	*pfEndOfStream = StreamPos >= StreamSize ? TRUE : FALSE;
	return S_OK;
}

// 同期書き込みを行う
STDMETHODIMP tTVPMFByteStream::Write(const BYTE *pb,ULONG cb,ULONG *pcbWritten) {
	return E_NOTIMPL;
}
// 非同期書き込みを開始する
STDMETHODIMP tTVPMFByteStream::BeginWrite(const BYTE *pb,ULONG cb,IMFAsyncCallback *pCallback,IUnknown *punkState)
{
	return E_NOTIMPL;
}
// 非同期書き込みを終了する
STDMETHODIMP tTVPMFByteStream::EndWrite(IMFAsyncResult *pResult,ULONG *pcbWritten)
{
	return E_NOTIMPL;
}
//書き込みバッファのフラッシュを行う
STDMETHODIMP tTVPMFByteStream::Flush()
{
	return E_NOTIMPL;
}
STDMETHODIMP tTVPMFByteStream::Seek(MFBYTESTREAM_SEEK_ORIGIN SeekOrigin,LONGLONG llSeekOffset,DWORD dwSeekFlags,QWORD *pqwCurrentPosition)
{
	int64_t llNewPos;
	CAutoLock holder( &CSStream );
	switch( SeekOrigin ) {
		case msoBegin:
			llNewPos = llSeekOffset;
#ifdef MF_D_LOG
			TVPAddLog( TJS_W( "Seek to : " ) + to_tjs_string( llSeekOffset ) );
#endif
			break;
		case msoCurrent:
			llNewPos = (int64_t)StreamPos + llSeekOffset;
#ifdef MF_D_LOG
			TVPAddLog( TJS_W( "Seek offset : " ) + to_tjs_string( llSeekOffset ) );
#endif
			break;
		default:
			return E_INVALIDARG;
	}
	if( llNewPos < 0 ) { llNewPos = 0; } else if( (uint64_t)llNewPos > StreamSize ) { llNewPos = StreamSize; }
	StreamPos = llNewPos;
	if( pqwCurrentPosition != nullptr ) { *pqwCurrentPosition = StreamPos; }
	return S_OK;
}

// 同期読み取りを行う
STDMETHODIMP tTVPMFByteStream::Read(BYTE *pb,ULONG cb,ULONG *pcbRead)
{
	CheckPointer(pb,E_POINTER);
	CheckPointer(pcbRead,E_POINTER);
	CAutoLock holder( &CSStream );
#ifdef MF_D_LOG
	TVPAddLog( TJS_W( "Read size : " ) + to_tjs_string( cb ) );
#endif
	LARGE_INTEGER offset;
	offset.QuadPart = StreamPos;
	if( FAILED( FileStream->Seek( offset, STREAM_SEEK_SET, nullptr ) ) ) return E_FAIL;
	if( FAILED( FileStream->Read( pb, cb, pcbRead ) ) ) return E_FAIL;
	StreamPos += *pcbRead;
	return S_OK;
}

// 非同期読み取りを開始する
STDMETHODIMP tTVPMFByteStream::BeginRead( BYTE *pb, ULONG cb, IMFAsyncCallback *pCallback, IUnknown *punkState ) {
	// TVPAddLog( TJS_W( "BeginRead." ) );
	CheckPointer( pb, E_POINTER );
	CheckPointer( pCallback, E_POINTER );
	if( !IsValidStream() ) return E_HANDLE;

	HRESULT hr;
	CAutoLock holder( &CSStream );
	AsyncReader* reader = new AsyncReader( this, pb, cb, StreamPos );
	IMFAsyncResult* pResult = nullptr;
	if( FAILED( hr = MFCreateAsyncResult( reader, pCallback, punkState, &pResult ) ) ) return hr;
	reader->Release();
	// 読み込みを行っていなくてもこのメソッド内で読み込み位置を進める必要がある
	StreamPos += cb;
	if( StreamPos > StreamSize ) StreamPos = StreamSize;
	if( FAILED( hr = MFPutWorkItem( WorkQueueId, &WorkQueueCB, pResult ) ) ) return hr;
	pResult->Release();

	return S_OK;
}

// 非同期読み取りを終了する
STDMETHODIMP tTVPMFByteStream::EndRead(IMFAsyncResult *pResult,ULONG *pcbRead)
{
	// TVPAddLog( TJS_W( "EndRead." ) );
	CheckPointer(pResult,E_POINTER);
	CheckPointer(pcbRead,E_POINTER);

	*pcbRead = 0;
	HRESULT hr;

	CAutoLock holder( &CSStream );
	if( FAILED(hr = pResult->GetStatus()) ) return hr;

	IUnknown* pObj = nullptr;
	if( FAILED(hr = pResult->GetObject(&pObj )) ) return hr;
	AsyncReader* pReader = static_cast<AsyncReader*>( pObj );
	*pcbRead = pReader->GetReadSize();
	if( pObj ) pObj->Release(), pObj = nullptr;
	return hr;
}

// 内部読み込み処理
HRESULT tTVPMFByteStream::ReadForAsync(BYTE *pb,ULONG cb,ULONG *pcbRead,uint64_t nReadPos)
{
	if( !IsValidStream() ) return E_HANDLE;

	CAutoLock holder(&CSStream);
	LARGE_INTEGER offset;
	offset.QuadPart = nReadPos;
	if( FAILED( FileStream->Seek( offset, STREAM_SEEK_SET, nullptr ) ) ) return E_FAIL;
	if( FAILED( FileStream->Read( pb, cb, pcbRead ) ) ) return E_FAIL;
	// StreamPos の更新は行わない BeginRead で読み込めなくても位置の移動は行ってしまう
	return S_OK;
}

HRESULT tTVPMFByteStream::OnDispatchRead(IMFAsyncResult* pAsyncResult)
{
	// TVPAddLog( TJS_W( "OnDispatchRead." ) );
	HRESULT hr = S_OK;

	IUnknown* pState = nullptr;
	if( FAILED( hr = pAsyncResult->GetState(&pState))) return hr;

	IMFAsyncResult *pResult = nullptr;
	if( FAILED( hr = pState->QueryInterface(IID_PPV_ARGS(&pResult))) ) {
		if( pState ) pState->Release(), pState = nullptr;
		return hr;
	}

	IUnknown *pObj = nullptr;
	if( SUCCEEDED(hr = pResult->GetObject(&pObj)) ) {
		AsyncReader *pReader = static_cast<AsyncReader*>( pObj );
		hr = pReader->DoRead();
		if( pResult ) {
			HRESULT lhr;
			lhr = pResult->SetStatus( hr );
			lhr = MFInvokeCallback( pResult );
		}
	}

	if( pState ) pState->Release(), pState = nullptr;
	if( pObj ) pObj->Release(), pObj = nullptr;
	if( pResult ) pResult->Release(), pResult = nullptr;

	return hr;
}

HRESULT tTVPMFByteStream::AsyncReader::DoRead() {
	return Owner->ReadForAsync( DestBuffer, ByteCount, &ReadSize, ReadOffset );
}
tTVPMFByteStream::AsyncReader::~AsyncReader() {
	// TVPAddLog( TJS_W("delete AsyncReader"));
}

