#pragma comment (lib, "Urlmon.lib")

#include <windows.h>
#include "tp_stub.h"

#define PROTOCOL L"krkr"

/**
 * Asynchronous Pluggable Protocols による IE/URL Moniker からの
 * アーカイブへのアクセス処理
 */
class CArchive : public IInternetProtocol, public IClassFactory {

protected:
	int refCount;
	IStream *stream;
public:
	CArchive() {
		refCount = 0;
		stream = NULL;
		AddRef();
	}

	~CArchive() {
		if (stream) {
			stream->Release();
		}
	}
	
	//----------------------------------------------------------------------------
	// IUnknown 実装
public:
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject) {
		
		if (riid == IID_IUnknown ||
			riid == IID_IInternetProtocol ||
			riid == IID_IInternetProtocolRoot) {
			if (ppvObject == NULL)
				return E_POINTER;
			*ppvObject = this;
			AddRef();
			return S_OK;
		} else {
			*ppvObject = 0;
			return E_NOINTERFACE;
		}
	}

	ULONG STDMETHODCALLTYPE AddRef() {
		return ++refCount;
	}

	ULONG STDMETHODCALLTYPE Release() {
		int ret = --refCount;
		if (ret <= 0) {
			delete this;
			ret = 0;
		}
		return ret;
	}
	
	//----------------------------------------------------------------------------
	// IInternetProtocolRoot 実装

public:
	HRESULT STDMETHODCALLTYPE Start( 
		/* [in] */ LPCWSTR szUrl,
        /* [in] */ IInternetProtocolSink __RPC_FAR *pOIProtSink,
		/* [in] */ IInternetBindInfo __RPC_FAR *pOIBindInfo,
        /* [in] */ DWORD grfPI,
		/* [in] */ DWORD dwReserved) {

		LPCWSTR p = wcsstr(szUrl, PROTOCOL L":");

		//TVPAddLog(ttstr(p));

		if (p) {
			p += wcslen(PROTOCOL L":");

			//TVPAddLog(ttstr(p));
			
			// パス名の切り出し
			int i=0;
			while (p[i] && p[i] != '#' && p[i] != '?') i++;
			ttstr name(p, i);

			//TVPAddLog(name);

			stream = TVPCreateIStream(name, TJS_BS_READ);
			if (stream) {

				//TVPAddLog(TJS_W("get stream!"));

				STATSTG stat;
				memset(&stat, 0, sizeof(stat));
				stream->Stat(&stat, STATFLAG_NONAME);
				ULONG size = stat.cbSize.LowPart; // XXX 32bitの壁こえるとはまるコード
				pOIProtSink->ReportData(BSCF_DATAFULLYAVAILABLE,size,size);
				pOIProtSink->ReportResult(S_OK,0,NULL);
			} else {
				return INET_E_OBJECT_NOT_FOUND;
			}
			return S_OK;
		}
		return INET_E_USE_DEFAULT_PROTOCOLHANDLER;
	}
	
	HRESULT STDMETHODCALLTYPE Continue( 
		/* [in] */ PROTOCOLDATA __RPC_FAR *pProtocolData) {
		return S_OK;
	}
    
	HRESULT STDMETHODCALLTYPE Abort( 
		/* [in] */ HRESULT hrReason,
        /* [in] */ DWORD dwOptions) {
		if (stream) {
			stream->Release();
			stream = NULL;
		}
		return S_OK;
	}
	
    HRESULT STDMETHODCALLTYPE Terminate( 
		/* [in] */ DWORD dwOptions) {
		return S_OK;
	}
	
	HRESULT STDMETHODCALLTYPE Suspend( void) {
		return E_NOTIMPL;
	}		// Not implemented

    HRESULT STDMETHODCALLTYPE Resume( void) {
		return E_NOTIMPL;
	}		// Not implemented

	//----------------------------------------------------------------------------
	// IInternetProtocol 実装
public:
    HRESULT STDMETHODCALLTYPE Read( 
        /* [length_is][size_is][out][in] */ void __RPC_FAR *pv,
        /* [in] */ ULONG cb,
		/* [out] */ ULONG __RPC_FAR *pcbRead) {
		if (stream) {
			stream->Read(pv, cb, pcbRead);
			if (*pcbRead == cb) {
				return S_OK;
			}
			return S_FALSE;
		} else {
			return INET_E_DOWNLOAD_FAILURE;
		}
	}
    
    HRESULT STDMETHODCALLTYPE Seek( 
        /* [in] */ LARGE_INTEGER dlibMove,
        /* [in] */ DWORD dwOrigin,
		/* [out] */ ULARGE_INTEGER __RPC_FAR *plibNewPosition) {
		return S_OK;
	}
    
    HRESULT STDMETHODCALLTYPE LockRequest(/* [in] */ DWORD dwOptions) { 
		return S_OK; 
	}
    
	HRESULT STDMETHODCALLTYPE UnlockRequest( void) {
		if (stream) {
			stream->Release();
			stream = NULL;
		}
		return S_OK;
	}

	//----------------------------------------------------------------------------
	// IClassFactory 実装
	
	HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown * pUnkOuter, REFIID riid,void ** ppvObject) {
		*ppvObject = NULL;
		if (pUnkOuter) {
			return CLASS_E_NOAGGREGATION;
		} 
		CArchive *p = new CArchive();
		if (p == NULL) {
			return E_OUTOFMEMORY;
		}
		HRESULT hr = p->QueryInterface(riid, ppvObject);
		p->Release();
		return hr;
	}

	HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock) {
		return S_OK;
	}
};

static IInternetSession *pIInternetSession = NULL;
static IClassFactory *pcf = NULL;

/**
 * アーカイブ処理をプロセスのインターネットセッションに登録する
 */
void registerArchive()
{
	if (pIInternetSession == NULL && CoInternetGetSession(0, &pIInternetSession, 0) == S_OK) {
		pcf = new CArchive();
		GUID dummy;
		pIInternetSession->RegisterNameSpace(pcf, (REFCLSID)dummy, PROTOCOL, 0, NULL, 0);
	}
}

/**
 * アーカイブ処理をプロセスのインターネットセッションから解除する
 */
void unregisterArchive()
{
	if (pIInternetSession) {
		pIInternetSession->UnregisterNameSpace(pcf, PROTOCOL);
		pcf->Release();
		pIInternetSession->Release();
		pIInternetSession = NULL;
	}
}
