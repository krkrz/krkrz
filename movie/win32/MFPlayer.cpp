
#include "tjsCommHead.h"
#include "MsgIntf.h"
#include "SysInitIntf.h"

#include <windows.h>
#include <assert.h>
#include <math.h>
#include <cmath>
#include <propvarutil.h>
#include <string>
#include <vector>

#include <Mfidl.h>
#include <mfapi.h>
#include <mferror.h>
#include <evr.h>

#include <streams.h>
#include <atlbase.h>
#include <atlcom.h>

#include "krmovie.h"
#include "MFPlayer.h"
#include "DShowException.h"
#include "DebugIntf.h"
#include "DShowException.h"

#include "MFByteStream.h"

#ifdef _MSC_VER
#pragma comment( lib, "propsys.lib" )
#pragma comment( lib, "Mfplat.lib" )
#pragma comment( lib, "Mf.lib" )
#pragma comment( lib, "Mfuuid.lib" )
//#pragma comment( lib, "d3d9.lib" )
//#pragma comment( lib, "dxva2.lib" )
//#pragma comment( lib, "evr.lib" )
#endif

//----------------------------------------------------------------------------
//! @brief	  	VideoOverlay MediaFoundationを取得する
//! @param		callbackwin : 
//! @param		stream : 
//! @param		streamname : 
//! @param		type : 
//! @param		size : 
//! @param		out : VideoOverlay Object
//! @return		エラー文字列
//----------------------------------------------------------------------------
void __stdcall GetMFVideoOverlayObject(
	HWND callbackwin, IStream *stream, const tjs_char * streamname,
	const tjs_char *type, unsigned __int64 size, iTVPVideoOverlay **out)
{
	*out = new tTVPMFPlayer;

	if( *out )
		static_cast<tTVPMFPlayer*>(*out)->BuildGraph( callbackwin, stream, streamname, type, size );
}
//----------------------------------------------------------------------------
STDMETHODIMP tTVPPlayerCallback::NonDelegatingQueryInterface(REFIID riid,void **ppv) {
	if(IsEqualIID(riid,IID_IMFAsyncCallback)) return GetInterface(static_cast<IMFAsyncCallback *>(this),ppv);
	return CUnknown::NonDelegatingQueryInterface(riid,ppv);
}
STDMETHODIMP tTVPPlayerCallback::GetParameters( DWORD *pdwFlags, DWORD *pdwQueue ) {
	return E_NOTIMPL;
}
STDMETHODIMP tTVPPlayerCallback::Invoke( IMFAsyncResult *pAsyncResult ) {
	HRESULT hr;
	MediaEventType met = MESessionClosed;
	CComPtr<IMFMediaEvent> pMediaEvent;
	if( SUCCEEDED(hr = owner_->GetMediaSession()->EndGetEvent( pAsyncResult, &pMediaEvent )) ) {
		if( SUCCEEDED(hr = pMediaEvent->GetType(&met)) ) {
			PROPVARIANT pvValue;
			PropVariantInit(&pvValue);
			switch( met ) {
			case MESessionClosed:
				owner_->OnMediaItemCleared();
				break;
			case MESessionPaused:
				owner_->OnPause();
				break;
			case MESessionEnded:
				owner_->OnPlayBackEnded();
				break;
			case MESessionNotifyPresentationTime:
				break;
			case MESessionRateChanged:
				if( SUCCEEDED(pMediaEvent->GetValue( &pvValue )) ) {
					double value;
					if( FAILED(PropVariantToDouble(pvValue,&value)) ) {
						value = 1.0;
					}
					owner_->OnRateSet(value);
				} else {
					owner_->OnRateSet(1.0);
				}
				break;
			case MESessionScrubSampleComplete:
				break;
			case MESessionStarted:
				owner_->OnPlay();
				break;
			case MESessionStopped:
				owner_->OnStop();
				break;
			case MESessionStreamSinkFormatChanged:
				break;
			case MESessionTopologiesCleared:
				break;
			case MESessionTopologySet:
				break;
			case MESessionTopologyStatus: {
				UINT32 status = MF_TOPOSTATUS_INVALID;
				pMediaEvent->GetUINT32( MF_EVENT_TOPOLOGY_STATUS, &status );
				owner_->OnTopologyStatus(status);
				break;
				}
			}
			PropVariantClear(&pvValue);
		}
		owner_->GetMediaSession()->BeginGetEvent( this, NULL );
	}
	return S_OK;
}
//----------------------------------------------------------------------------
tTVPMFPlayer::tTVPMFPlayer() {
	CoInitialize(NULL);
	MFStartup( MF_VERSION );

	BuildWindow = NULL;
	OwnerWindow = NULL;
	CallbackWindow = NULL;
	Visible = false;
	Rect.left = 0; Rect.top = 0; Rect.right = 320; Rect.bottom = 240;
	RefCount = 1;
	Shutdown = false;
	PlayerCallback = new tTVPPlayerCallback(this);
	PlayerCallback->AddRef();
	FPSNumerator = 1;
	FPSDenominator = 1;
	//Stream = NULL;

	HnsDuration = 0;

	
	AudioVolumeValue = 0;
	AudioBalanceValue = 0;
	//StartPositionSpecify = false;
}
tTVPMFPlayer::~tTVPMFPlayer() {
	Shutdown = true;
	ReleaseAll();
	MFShutdown();
	CoUninitialize();
}
void __stdcall tTVPMFPlayer::AddRef(){
	RefCount++;
}
void __stdcall tTVPMFPlayer::Release(){
	if(RefCount == 1)
		delete this;
	else
		RefCount--;
}

void tTVPMFPlayer::OnDestoryWindow() {
	ReleaseAll();
}
/*
void tTVPMFPlayer::InitializeMFDLL() {
	if( MfDLL.IsLoaded() == false ) {
		MfDLL.Load(TJS_W("mf.dll"));
	}
}
*/
#define CUSTOM_BYTE_STREAM
//----------------------------------------------------------------------------
void __stdcall tTVPMFPlayer::BuildGraph( HWND callbackwin, IStream *stream,
	const tjs_char * streamname, const tjs_char *type, unsigned __int64 size )
{
	BuildWindow = callbackwin;
	OwnerWindow = callbackwin;
	PlayWindow::SetOwner( callbackwin );
	CallbackWindow = callbackwin;
	PlayWindow::SetMessageDrainWindow( callbackwin );
	StreamName = tjs_string(streamname);
	//Stream = stream;

	HRESULT hr = S_OK;
	// MFCreateMFByteStreamOnStream は、Windows 7 以降にのみある API なので、動的ロードして Vista での起動に支障がないようにする
	// Vista は非サポートとなったので、直接使用するように変更。
	/*
	if( MfplatDLL.IsLoaded() == false ) MfplatDLL.Load(TJS_W("mfplat.dll"));
	typedef HRESULT (WINAPI *FuncMFCreateMFByteStreamOnStream)(IStream *pStream,IMFByteStream **ppByteStream);
	FuncMFCreateMFByteStreamOnStream pCreateMFByteStream = (FuncMFCreateMFByteStreamOnStream)MfplatDLL.GetProcAddress("MFCreateMFByteStreamOnStream");
	if( pCreateMFByteStream == NULL ) {
		TVPThrowExceptionMessage(TJS_W("Faild to retrieve MFCreateMFByteStreamOnStream from mfplat.dll."));
	}
	*/
#ifdef CUSTOM_BYTE_STREAM
	ByteStream = new tTVPMFByteStream( &hr, stream, streamname, size );
	if( FAILED( hr ) ) {
		//if( FAILED(hr = MFCreateMFByteStreamOnStream( stream, &ByteStream )) ) {
		TVPThrowExceptionMessage( TJS_W( "Faild to create stream." ) );
	}
	if( FAILED( hr = static_cast<tTVPMFByteStream*>( ByteStream.p )->Open() ) ) {
		TVPThrowExceptionMessage( TJS_W( "Faild to open stream." ) );
	}
#else
	if( FAILED(hr = MFCreateMFByteStreamOnStream( stream, &ByteStream )) ) {
		TVPThrowExceptionMessage( TJS_W( "Faild to create stream." ) );
	}
#endif
	ContentType = tjs_string( ParseVideoType( type ) );
/*
	CComPtr<IMFAttributes> pAttribute;
	if( SUCCEEDED( hr = ByteStream.QueryInterface( &pAttribute ) ) ) {
		bool hasContentType = false;
		UINT32 len;
		if( SUCCEEDED( hr = pAttribute->GetString( MF_BYTESTREAM_CONTENT_TYPE, NULL, 0, &len ) ) ) {
			std::unique_ptr<WCHAR[]> contentType( new WCHAR[len + 1] );
			if( SUCCEEDED( hr = pAttribute->GetString( MF_BYTESTREAM_CONTENT_TYPE, (LPWSTR)contentType.get(), len + 1, NULL ) ) ) {
				hasContentType = true;
			}
		}
		if( hasContentType == false ) {
			const tjs_char *ctype = ParseVideoType( type );
			if( FAILED( hr = pAttribute->SetString( MF_BYTESTREAM_CONTENT_TYPE, ctype ) ) ) {
				ThrowDShowException( TJS_W( "Faild to set content type." ), hr );
			}
		}
	}
*/
}

//----------------------------------------------------------------------------
//! @brief	  	拡張子からムービーのタイプを判別します
//! @param		type : ムービーファイルの拡張子
//----------------------------------------------------------------------------
const tjs_char * tTVPMFPlayer::ParseVideoType( const tjs_char *type ) {
	if( _wcsicmp( type, TJS_W( ".mp4" ) ) == 0 )
		return TJS_W( "video/mp4" );
	else if( _wcsicmp( type, TJS_W( ".wmv" ) ) == 0 )
		return TJS_W("video/x-ms-wmv");
	else if( _wcsicmp( type, TJS_W( ".avi" ) ) == 0 )
		return TJS_W( "video/x-msvideo" );
	else
		TVPThrowExceptionMessage( TJS_W( "Unknown video format extension." ) ); // unknown format
	return nullptr;
}
/*
HRESULT tTVPMFPlayer::GetPresentationDescriptorFromTopology( IMFPresentationDescriptor **ppPD ) {
    CComPtr<IMFCollection> pCollection;
    CComPtr<IUnknown> pUnk;
    CComPtr<IMFTopologyNode> pNode;
	
	HRESULT hr = S_OK;
    // Get the collection of source nodes from the topology.
    if( FAILED(hr = Topology->GetSourceNodeCollection(&pCollection)) ) {
		return hr;
	}
    // Any of the source nodes should have the PD, so take the first
    // object in the collection.
    if( FAILED(hr = pCollection->GetElement(0, &pUnk)) ) {
		return hr;
	}
    if( FAILED(hr = pUnk->QueryInterface(IID_PPV_ARGS(&pNode))) ) {
		return hr;
	}
    // Get the PD, which is stored as an attribute.
    if( FAILED(hr = pNode->GetUnknown( MF_TOPONODE_PRESENTATION_DESCRIPTOR, IID_PPV_ARGS(ppPD))) ) {
		return hr;
	}
    return hr;
}
*/
void tTVPMFPlayer::OnTopologyStatus(UINT32 status) {
	HRESULT hr;
	switch( status ) {
	case MF_TOPOSTATUS_INVALID:
		break;
	case MF_TOPOSTATUS_READY: {
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ms695350%28v=vs.85%29.aspx
		CComPtr<IMFGetService> pGetService;
		if( SUCCEEDED(hr = MediaSession->QueryInterface( &pGetService )) ) {
			if( FAILED(hr = pGetService->GetService( MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl, (void**)&VideoDisplayControl )) ) {
				TVPAddLog( TJS_W( "MF : Cannot retrieve IID_IMFVideoDisplayControl." ) );
			}
			if( FAILED(hr = pGetService->GetService( MR_STREAM_VOLUME_SERVICE, IID_IMFAudioStreamVolume, (void**)&AudioVolume )) ) {
				TVPAddLog(TJS_W("MF : Cannot retrieve IID_IMFAudioStreamVolume."));
			}
			if( FAILED(hr = pGetService->GetService( MR_POLICY_VOLUME_SERVICE, IID_IMFSimpleAudioVolume, (void**)&SimpleAudioVolume )) ) {
				TVPAddLog( TJS_W( "MF : Cannot retrieve IID_IMFSimpleAudioVolume." ) );
			}
			pGetService->GetService( MF_RATE_CONTROL_SERVICE, IID_IMFRateControl, (void**)&RateControl );
			pGetService->GetService( MF_RATE_CONTROL_SERVICE, IID_IMFRateSupport, (void**)&RateSupport );
			CComPtr<IMFClock> pClock;
			HRESULT hrTmp = MediaSession->GetClock(&pClock);
			if( SUCCEEDED(hrTmp) ) {
				hr = pClock->QueryInterface(IID_PPV_ARGS(&PresentationClock));
			}
		}
		if( BuildWindow != NULL) ::PostMessage( BuildWindow, WM_STATE_CHANGE, vsReady, 0 );
		break;
		}
	case MF_TOPOSTATUS_STARTED_SOURCE:
		break;
	case MF_TOPOSTATUS_DYNAMIC_CHANGED:
		break;
	case MF_TOPOSTATUS_SINK_SWITCHED:
		break;
	case MF_TOPOSTATUS_ENDED:
		break;
	}
}

HRESULT tTVPMFPlayer::CreateVideoPlayer() {
	if( MediaSession.p ) {
		return S_OK;	// 既に作成済み
	}

	HRESULT hr = CreateChildWindow();
	if( hr != S_OK ) return hr;

	HWND hWnd = GetChildWindow();
	if( hWnd == NULL || hWnd == INVALID_HANDLE_VALUE )
		return E_FAIL;

	if( FAILED(hr = MFCreateMediaSession( NULL, &MediaSession )) ) {
		ThrowDShowException(TJS_W("Faild to create Media session."), hr);
	}
	if( FAILED(hr = MediaSession->BeginGetEvent( PlayerCallback, NULL )) ) {
		ThrowDShowException(TJS_W("Faild to begin get event."), hr);
	}
	CComPtr<IMFAttributes> pAttribute;
	if( SUCCEEDED( hr = ByteStream.QueryInterface( &pAttribute ) ) ) {
		bool hasContentType = false;
		UINT32 len;
		if( SUCCEEDED( hr = pAttribute->GetString( MF_BYTESTREAM_CONTENT_TYPE, NULL, 0, &len ) ) ) {
			std::unique_ptr<WCHAR[]> contentType( new WCHAR[len + 1] );
			if( SUCCEEDED( hr = pAttribute->GetString( MF_BYTESTREAM_CONTENT_TYPE, (LPWSTR)contentType.get(), len + 1, NULL ) ) ) {
				hasContentType = true;
			}
		}
		if( hasContentType == false ) {
			if( FAILED( hr = pAttribute->SetString( MF_BYTESTREAM_CONTENT_TYPE, ContentType.c_str() ) ) ) {
				ThrowDShowException( TJS_W( "Faild to set content type." ), hr );
			}
		}
	}
	CComPtr<IMFSourceResolver> pSourceResolver;
	if( FAILED(hr = MFCreateSourceResolver(&pSourceResolver)) ) {
		ThrowDShowException(TJS_W("Faild to create source resolver."), hr);
	}
	MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;
	CComPtr<IUnknown> pSource;
#ifdef CUSTOM_BYTE_STREAM
	if( FAILED( hr = pSourceResolver->CreateObjectFromByteStream( ByteStream, nullptr, MF_RESOLUTION_MEDIASOURCE, NULL, &ObjectType, (IUnknown**)&pSource ) ) ) {
#else
	if( FAILED(hr = pSourceResolver->CreateObjectFromByteStream( ByteStream, StreamName.c_str(), MF_RESOLUTION_MEDIASOURCE, NULL, &ObjectType, (IUnknown**)&pSource )) ) {
#endif
	//if( FAILED( hr = pSourceResolver->CreateObjectFromByteStream( ByteStream, StreamName.c_str(), MF_RESOLUTION_MEDIASOURCE, NULL, &ObjectType, (IUnknown**)&pSource ) ) ) {
	//if( FAILED(hr = pSourceResolver->CreateObjectFromURL( TJS_W("C:\\krkrz\\bin\\win32\\data\\test.mp4"), MF_RESOLUTION_MEDIASOURCE, NULL, &ObjectType, (IUnknown**)&pSource)) ) {
		ThrowDShowException(TJS_W("Faild to open stream."), hr );
	}
	if( ObjectType != MF_OBJECT_MEDIASOURCE ) {
		TVPThrowExceptionMessage(TJS_W("Invalid media source."));
	}
	//CComPtr<IMFMediaSource> pMediaSource;
	if( FAILED(hr = pSource.QueryInterface(&MediaSource)) ) {
		ThrowDShowException(TJS_W("Faild to query Media source."), hr );
	}
	if( FAILED(hr = MFCreateTopology(&Topology)) ) {
		ThrowDShowException(TJS_W("Faild to create Topology."), hr );
	}
	CComPtr<IMFPresentationDescriptor> pPresentationDescriptor;
	if( FAILED(hr = MediaSource->CreatePresentationDescriptor(&pPresentationDescriptor)) ) {
		ThrowDShowException(TJS_W("Faild to create Presentation Descriptor."), hr );
	}
	DWORD streamCount;
	if( FAILED(hr = pPresentationDescriptor->GetStreamDescriptorCount(&streamCount)) ) {
		ThrowDShowException(TJS_W("Faild to get stream count."), hr );
	}
	if( streamCount < 1 ) {
		TVPThrowExceptionMessage(TJS_W("Not found media stream."));
	}
	for( DWORD i = 0; i < streamCount; i++ ) {
		if( FAILED(hr = AddBranchToPartialTopology(Topology, MediaSource, pPresentationDescriptor, i, hWnd)) ) {
			ThrowDShowException(TJS_W("Faild to add nodes."), hr );
		}
	}
	pPresentationDescriptor->GetUINT64(MF_PD_DURATION, (UINT64*)&HnsDuration);
	
	if( FAILED(hr = MediaSession->SetTopology( 0, Topology )) ) {
		ThrowDShowException(TJS_W("Faild to set topology."), hr );
	}
	return hr;
}

HRESULT tTVPMFPlayer::AddBranchToPartialTopology( IMFTopology *pTopology, IMFMediaSource *pSource, IMFPresentationDescriptor *pPD, DWORD iStream, HWND hVideoWnd ) {
	CComPtr<IMFStreamDescriptor>	pSD;
	HRESULT hr;
	BOOL selected = FALSE;
    if( FAILED(hr = pPD->GetStreamDescriptorByIndex(iStream, &selected, &pSD)) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to get stream desc."));
	}
	if( selected ) {
		// Create the media sink activation object.
		CComPtr<IMFActivate> pSinkActivate;
		if( FAILED(hr = CreateMediaSinkActivate(pSD, hVideoWnd, &pSinkActivate)) ) {
			return S_OK;	// video と audio 以外は無視
		}
		// Add a source node for this stream.
		CComPtr<IMFTopologyNode> pSourceNode;
        if( FAILED(hr = AddSourceNode(pTopology, pSource, pPD, pSD, &pSourceNode) ) ) {
			TVPThrowExceptionMessage(TJS_W("Faild to add source node."));
		}
		// Create the output node for the renderer.
		CComPtr<IMFTopologyNode> pOutputNode;
        if( FAILED(hr = AddOutputNode(pTopology, pSinkActivate, 0, &pOutputNode)) ) {
			TVPThrowExceptionMessage(TJS_W("Faild to add output node."));
		}
		// Connect the source node to the output node.
        if( FAILED(hr = pSourceNode->ConnectOutput(0, pOutputNode, 0)) ) {
			TVPThrowExceptionMessage(TJS_W("Faild to connect node."));
		}
	}
	return hr;
}
HRESULT tTVPMFPlayer::CreateMediaSinkActivate( IMFStreamDescriptor *pSourceSD, HWND hVideoWindow, IMFActivate **ppActivate ) {
	HRESULT hr;
	CComPtr<IMFMediaTypeHandler> pHandler;
	// Get the media type handler for the stream.
    if( FAILED(hr = pSourceSD->GetMediaTypeHandler(&pHandler)) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to get media type handler."));
	}
	// Get the major media type.
    GUID guidMajorType;
    if( FAILED(hr = pHandler->GetMajorType(&guidMajorType)) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to get major type."));
	}
    CComPtr<IMFActivate>		pActivate;
	if( MFMediaType_Audio == guidMajorType ) {
		// Create the audio renderer.
        if( FAILED(hr = MFCreateAudioRendererActivate(&pActivate) )) {
			TVPThrowExceptionMessage(TJS_W("Faild to create audio render."));
		}
	} else if( MFMediaType_Video == guidMajorType ) {
		// Get FPS
		CComPtr<IMFMediaType> pMediaType;
		if( SUCCEEDED(hr = pHandler->GetCurrentMediaType(&pMediaType)) ) {
			hr = MFGetAttributeRatio( pMediaType, MF_MT_FRAME_RATE, &FPSNumerator, &FPSDenominator );
		}

        // Create the video renderer.
        if( FAILED(hr = MFCreateVideoRendererActivate(hVideoWindow, &pActivate) ) ) {
			TVPThrowExceptionMessage(TJS_W("Faild to create video render."));
		}
		// ここでカスタムEVRをつなぐようにすると自前で色々描画できるようになる
		// 現状は標準のものを使っている
#if 0
		tTVPEVRCustomPresenter* my_activate_obj = new tTVPEVRCustomPresenter(hr);
		my_activate_obj->AddRef();
		CComPtr<IUnknown> unk;
		my_activate_obj->QueryInterface( IID_IUnknown, (void**)&unk );
		if( FAILED(hr = pActivate->SetUnknown(MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_ACTIVATE, unk)) ) {
			my_activate_obj->Release();
			TVPThrowExceptionMessage(TJS_W("Faild to add custom EVR presenter video render."));
		}
		my_activate_obj->Release();
#endif
	} else {
		hr = E_FAIL;
	}
	if( SUCCEEDED(hr) ) {
		// Return IMFActivate pointer to caller.
		*ppActivate = pActivate;
		(*ppActivate)->AddRef();
	}
	return hr;
}
HRESULT tTVPMFPlayer::AddSourceNode( IMFTopology *pTopology, IMFMediaSource *pSource, IMFPresentationDescriptor *pPD, IMFStreamDescriptor *pSD, IMFTopologyNode **ppNode ) {
	HRESULT hr;
	// Create the node.
	CComPtr<IMFTopologyNode> pNode;
    if( FAILED(hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode)) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to create source node."));
	}
	// Set the attributes.
    if( FAILED(hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource) ) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to set source node."));
	}
	if( FAILED(hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPD) ) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to set presentation desc."));
	}
	if( FAILED(hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD)) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to set stream desc."));
	}
	// Add the node to the topology.
    if( FAILED(hr = pTopology->AddNode(pNode)) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to add source node to topology."));
	}
	// Return the pointer to the caller.
    *ppNode = pNode;
    (*ppNode)->AddRef();

	return hr;
}
HRESULT tTVPMFPlayer::AddOutputNode( IMFTopology *pTopology, IMFActivate *pActivate, DWORD dwId, IMFTopologyNode **ppNode ) {
	HRESULT hr;
    // Create the node.
    CComPtr<IMFTopologyNode> pNode;
    if( FAILED(hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode)) ){
		TVPThrowExceptionMessage(TJS_W("Faild to create output node."));
	}
    // Set the object pointer.
    if( FAILED(hr = pNode->SetObject(pActivate)) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to set activate."));
	}
    // Set the stream sink ID attribute.
    if( FAILED(hr = pNode->SetUINT32(MF_TOPONODE_STREAMID, dwId)) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to set stream id."));
	}
	if( FAILED(hr = pNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE)) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to set no shutdown on remove."));
	}
    // Add the node to the topology.
    if( FAILED(hr = pTopology->AddNode(pNode)) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to add ouput node to topology."));
	}
    // Return the pointer to the caller.
    *ppNode = pNode;
    (*ppNode)->AddRef();

	return hr;
}
/*
HRESULT tTVPMFPlayer::AddOutputNode( IMFTopology *pTopology, IMFStreamSink *pStreamSink, IMFTopologyNode **ppNode ) {
	HRESULT hr;
	// Create the node.
	CComPtr<IMFTopologyNode> pNode;
    if( FAILED(hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode)) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to create output node."));
	}
	// Set the object pointer.
	if( FAILED(hr = pNode->SetObject(pStreamSink)) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to set stream sink."));
	}
	// Add the node to the topology.
	if( FAILED(hr = pTopology->AddNode(pNode))) {
		TVPThrowExceptionMessage(TJS_W("Faild to add output node."));
	}
	if( FAILED(hr = pNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, TRUE) ) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to set no shutdown on remove."));
	}
	*ppNode = pNode;
    (*ppNode)->AddRef();

	return hr;
}
*/
//----------------------------------------------------------------------------
//! @brief	  	インターフェイスを解放する
//----------------------------------------------------------------------------
void __stdcall tTVPMFPlayer::ReleaseAll()
{
	if( MediaSession.p ) {
		MediaSession->Stop();
		MediaSession->Close();
		if( MediaSource.p ) {
			MediaSource->Shutdown();
		}
		MediaSession->Shutdown();
	}
	if( PlayerCallback ) {
		PlayerCallback->Release();
		PlayerCallback = NULL;
	}
	if( ByteStream.p ) {
		ByteStream->Close();
		ByteStream.Release();
	}
	if( SimpleAudioVolume.p ) {
		SimpleAudioVolume.Release();
	}
	if( AudioVolume.p ) {
		AudioVolume.Release();
	}
	if( VideoDisplayControl.p ) {
		VideoDisplayControl.Release();
	}
	if( RateSupport.p ) {
		RateSupport.Release();
	}
	if( RateControl.p ) {
		RateControl.Release();
	}
	if( PresentationClock.p ) {
		PresentationClock.Release();
	}
	if( MediaSource.p ) {
		MediaSource.Release();
	}
	if( Topology.p ) {
		Topology.Release();
	}
	if( MediaSession.p ) {
		MediaSession.Release();
	}
}
//----------------------------------------------------------------------------
void tTVPMFPlayer::NotifyError( HRESULT hr ) {
	TVPThrowExceptionMessage(TJS_W("MF Operation Error."),hr);
}
void tTVPMFPlayer::OnMediaItemCleared() {
}
void tTVPMFPlayer::OnPause() {
	if( BuildWindow != NULL) ::PostMessage( BuildWindow, WM_STATE_CHANGE, vsPaused, 0 );
}
void tTVPMFPlayer::OnPlayBackEnded() {
	if( BuildWindow != NULL ) ::PostMessage( BuildWindow, WM_STATE_CHANGE, vsEnded, 0 );
}
void tTVPMFPlayer::OnRateSet( double rate ) {
}
void tTVPMFPlayer::OnStop() {
	if( BuildWindow != NULL) ::PostMessage( BuildWindow, WM_STATE_CHANGE, vsStopped, 0 );
}
void tTVPMFPlayer::OnPlay() {
	if( BuildWindow != NULL) ::PostMessage( BuildWindow, WM_STATE_CHANGE, vsPlaying, 0 );
}
//----------------------------------------------------------------------------
void __stdcall tTVPMFPlayer::SetWindow(HWND window) {
	HRESULT hr = E_FAIL;
	// 吉里吉里ZではWindowハンドルが変わらないはずなので、NULLの時は何もしない
	if( window == NULL ) return;

	if( OwnerWindow != window ) {
		OwnerWindow = window;
		PlayWindow::SetOwner( window );
		if( VideoDisplayControl.p ) {
			//hr = VideoDisplayControl->SetVideoWindow( window );
			hr = VideoDisplayControl->SetVideoWindow( GetChildWindow() );
			if( FAILED( hr ) ) {
				TVPThrowExceptionMessage( TJS_W( "Faild to call SetVideoWindow." ) );
			}
		}
		CreateVideoPlayer();
	}
}
void __stdcall tTVPMFPlayer::SetMessageDrainWindow(HWND window) {
	PlayWindow::SetMessageDrainWindow( window );
	CallbackWindow = window;
}
void __stdcall tTVPMFPlayer::SetRect(RECT *rect) {
	PlayWindow::SetRect( rect );
	if( VideoDisplayControl.p ) {
		// ウィンドウ位置で描画位置を制御しているので、内部の動画は位置のオフセットは行わない
		RECT vr;
		vr.left = 0;
		vr.top = 0;
		vr.right = rect->right - rect->left;
		vr.bottom = rect->bottom - rect->top;
		// MF では、ソース矩形も指定可能になっている
		VideoDisplayControl->SetAspectRatioMode( MFVideoARMode_PreservePicture );
		HRESULT hr = VideoDisplayControl->SetVideoPosition( NULL, &vr );
		if( FAILED(hr) ) {
			TVPThrowExceptionMessage(TJS_W("Faild to set rect."));
		}
	}
}
void __stdcall tTVPMFPlayer::SetVisible(bool b) {
	PlayWindow::SetVisible( b );
}
void __stdcall tTVPMFPlayer::Play() {
	HRESULT hr = E_FAIL;
	if( MediaSession.p ) {
		SetVolumeToMF();

		PROPVARIANT varStart;
		PropVariantInit(&varStart);
		/*
		if( StartPositionSpecify ) {
			varStart.vt = VT_I8;
			varStart.hVal.QuadPart = StartPosition;
			StartPositionSpecify = false;
		}
		*/
		hr = MediaSession->Start( &GUID_NULL, &varStart );
		PropVariantClear(&varStart);
	}
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to play."));
	}
}
void __stdcall tTVPMFPlayer::Stop() {
	HRESULT hr = E_FAIL;
	if( MediaSession.p ) {
		hr = MediaSession->Stop();
		//hr = MediaSession->Pause();
		//StartPositionSpecify = false;
	}
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to stop."));
	}
}
void __stdcall tTVPMFPlayer::Pause() {
	HRESULT hr = E_FAIL;
	if( MediaSession.p ) {
		hr = MediaSession->Pause();
		//StartPositionSpecify = false;
	}
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to stop."));
	}
}
MFCLOCK_STATE tTVPMFPlayer::GetClockState() {
	HRESULT hr = S_OK;
	MFCLOCK_STATE state;
	if( PresentationClock.p ) {
		if( SUCCEEDED(hr = PresentationClock->GetState( 0, &state )) ) {
			return state;
		}
	}
	return MFCLOCK_STATE_INVALID;
}
// Seek
// http://msdn.microsoft.com/en-us/library/windows/desktop/ee892373%28v=vs.85%29.aspx
void __stdcall tTVPMFPlayer::SetPosition(unsigned __int64 tick) {
	HRESULT hr = S_OK;
	if( MediaSession.p && GetClockState() == MFCLOCK_STATE_RUNNING) {
		PROPVARIANT var;
		PropVariantInit(&var);
		var.vt = VT_I8;
		var.hVal.QuadPart = tick * (ONE_SECOND/ONE_MSEC);
		MediaSession->Start( &GUID_NULL, &var );
		PropVariantClear(&var);
	}
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to set position."));
	}
}
void __stdcall tTVPMFPlayer::GetPosition(unsigned __int64 *tick) {
	HRESULT hr = S_OK;
	if( PresentationClock.p ) {
		MFTIME mftime;
		if( SUCCEEDED(hr = PresentationClock->GetTime(&mftime)) ) {
			*tick = mftime / (ONE_SECOND / ONE_MSEC);
		} else {
			// 失敗した場合は 0 を返すようにしておく
			hr = S_OK;
			*tick = 0;
		}
	} else {
		*tick = 0;
	}
	if( FAILED(hr) ) {
		//TVPThrowExceptionMessage(TJS_W("Faild to get position."));
		ThrowDShowException(TJS_W("Faild to get position."),hr);
	}
}
void __stdcall tTVPMFPlayer::GetStatus(tTVPVideoStatus *status) {
	if( status ) {
		switch( GetClockState() ) {
		case MFCLOCK_STATE_INVALID:
			*status = vsStopped;
			break;
		case MFCLOCK_STATE_RUNNING:
			*status = vsPlaying;
			break;
		case MFCLOCK_STATE_STOPPED:
			*status = vsStopped;
			break;
		case MFCLOCK_STATE_PAUSED:
			*status = vsPaused;
			break;
		default:
			*status = vsStopped;
			break;
		}
	}
}
void __stdcall tTVPMFPlayer::GetEvent(long *evcode, LONG_PTR *param1, LONG_PTR *param2, bool *got) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::FreeEventParams(long evcode, LONG_PTR param1, LONG_PTR param2) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::Rewind() {
	SetPosition( 0 );
}
void __stdcall tTVPMFPlayer::SetFrame( int f ) {
	UINT64 avgTime;
	HRESULT hr = MFFrameRateToAverageTimePerFrame( FPSNumerator, FPSDenominator, &avgTime );
	if( SUCCEEDED(hr) ) {
		LONGLONG requestTime = avgTime * (LONGLONG)f;
		if( MediaSession.p && GetClockState() == MFCLOCK_STATE_RUNNING ) {
			PROPVARIANT var;
			PropVariantInit(&var);
			var.vt = VT_I8;
			var.hVal.QuadPart = requestTime;
			MediaSession->Start( &GUID_NULL, &var );
			PropVariantClear(&var);
		}
	}
}
void __stdcall tTVPMFPlayer::GetFrame( int *f ) {
	UINT64 avgTime;
	*f = 0;
	HRESULT hr = MFFrameRateToAverageTimePerFrame( FPSNumerator, FPSDenominator, &avgTime );
	if( SUCCEEDED(hr) ) {
		if( PresentationClock.p ) {
			MFTIME mftime;
			if( SUCCEEDED(hr = PresentationClock->GetTime(&mftime)) ) {
				*f = (int)( mftime / avgTime );
			}
		}
	}
}
void __stdcall tTVPMFPlayer::GetFPS( double *f ) {
	*f = (double)FPSNumerator / (double)FPSDenominator;
}
void __stdcall tTVPMFPlayer::GetNumberOfFrame( int *f ) {
	UINT64 avgTime;
	HRESULT hr = MFFrameRateToAverageTimePerFrame( FPSNumerator, FPSDenominator, &avgTime );
	if( SUCCEEDED(hr) ) {
		*f = (int)( HnsDuration / avgTime );
	}
}
/**
 * @brief ムービーの長さ(msec)を取得する
 * @param f ムービーの長さを入れる変数へのポインタ
 * 
 * http://msdn.microsoft.com/en-us/library/windows/desktop/dd979590%28v=vs.85%29.aspx
 */
void __stdcall tTVPMFPlayer::GetTotalTime( __int64 *t ) {
    *t = (HnsDuration / (ONE_SECOND / ONE_MSEC));
}

void __stdcall tTVPMFPlayer::GetVideoSize( long *width, long *height ){
	HRESULT hr = S_OK;
	if( VideoDisplayControl.p ) {
		SIZE vsize;
		if( SUCCEEDED(hr = VideoDisplayControl->GetNativeVideoSize( &vsize, NULL )) ) {
			*width = vsize.cx;
			*height = vsize.cy;
		} else {
			hr = S_OK;
			*width = 320;
			*height = 240;
		}
	} else {
		hr = S_OK;
		*width = 320;
		*height = 240;
	}
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to get video size."));
	}
}
void __stdcall tTVPMFPlayer::GetFrontBuffer( BYTE **buff ){
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::SetVideoBuffer( BYTE *buff1, BYTE *buff2, long size ){
	/* 何もしない */
}

// http://msdn.microsoft.com/en-us/library/windows/desktop/gg583862%28v=vs.85%29.aspx
void __stdcall tTVPMFPlayer::SetStopFrame( int frame ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetStopFrame( int *frame ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::SetDefaultStopFrame() {
	/* 何もしない */
}

void __stdcall tTVPMFPlayer::SetPlayRate( double rate ) {
	HRESULT hr = E_FAIL;
	if( RateSupport.p && RateControl.p ) {
		float playrate = (float)rate;
		float acceptrate = playrate;
		if( SUCCEEDED(hr = RateSupport->IsRateSupported( FALSE, playrate, &acceptrate )) ) {
			hr = RateControl->SetRate( FALSE, acceptrate );
		}
	}
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to set play rate."));
	}
}
void __stdcall tTVPMFPlayer::GetPlayRate( double *rate ) {
	HRESULT hr = E_FAIL;
	if( RateControl.p ) {
		float playrate = 1.0f;
		if( SUCCEEDED(hr = RateControl->GetRate( NULL, &playrate )) ) {
			*rate = playrate;
		}
	}
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to get play rate."));
	}
}
HRESULT tTVPMFPlayer::SetVolumeToMF() {
	HRESULT hr = S_OK;
	if( AudioVolume.p ) {
		UINT32 count;
		if( SUCCEEDED(hr = AudioVolume->GetChannelCount( &count )) ) {
			if( count == 2 ) {
				float channels[2];
				channels[0] = CalcLeftVolume();
				channels[1] = CalcRightVolume();
				hr = AudioVolume->SetAllVolumes( count, &channels[0] );
			} else if( count == 1 ) {
				hr = AudioVolume->SetChannelVolume( 0, CalcVolume() );
			} else {
				float volume = CalcVolume();
				for( UINT32 i = 0; i < count; i++ ) {
					hr = AudioVolume->SetChannelVolume( i, volume );
				}
			}
		}
	}
	return hr;
}
// ステレオの場合のみバランスは有効
void __stdcall tTVPMFPlayer::SetAudioBalance( long balance ) {
	if( AudioBalanceValue != balance ) {
		if( balance < -10000 ) AudioBalanceValue = -10000;
		else if( balance > 10000 ) AudioBalanceValue = 10000;
		else AudioBalanceValue = balance;

		HRESULT hr = SetVolumeToMF();
		if( FAILED(hr) ) {
			TVPThrowExceptionMessage(TJS_W("Faild to set audio balance."));
		} 
	}
}
void __stdcall tTVPMFPlayer::GetAudioBalance( long *balance ) {
	*balance = AudioBalanceValue;
	/*
	HRESULT hr = S_OK;
	*balance = 0;
	if( AudioVolume.p ) {
		UINT32 count;
		if( SUCCEEDED(hr = AudioVolume->GetChannelCount( &count )) ) {
			std::vector<float> channels(count);
			if( SUCCEEDED(hr = AudioVolume->GetAllVolumes( count, &channels[0] )) ) {
				float total = 0.0f;
				if( count == 2 ) {
					float left = channels[0] * (-10000);
					float right = channels[1] * 10000;
					float v = left + right;
					long b = (long)v;
					if( b < (-10000) ) b = (-10000);
					else if( b > 10000 ) b = 10000;
					*balance = b;
				}
			}
		}
	}
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to get audio balance."));
	}
	*/
}
void __stdcall tTVPMFPlayer::SetAudioVolume( long volume ) {
	if( AudioVolumeValue != volume ) {
		if( volume > 0 ) AudioVolumeValue = 0;
		else if( volume < -10000 ) AudioVolumeValue = -10000;
		else AudioVolumeValue = volume;
		
		HRESULT hr = SetVolumeToMF();
		if( FAILED(hr) ) {
			TVPThrowExceptionMessage(TJS_W("Faild to set audio volume."));
		} 
	}
}
void __stdcall tTVPMFPlayer::GetAudioVolume( long *volume ) {
	*volume = AudioVolumeValue;
	/*
	HRESULT hr = E_FAIL;
	float vol = 1.0f;
	if( SimpleAudioVolume.p ) {
		if( FAILED(hr=SimpleAudioVolume->GetMasterVolume( &vol )) ) {
			vol = 1.0f;
		}
	}
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(TJS_W("Faild to get audio volume."));
	} else {
		*volume = (long)((vol - 1.0f) * 10000);
	}
	*/
}

void __stdcall tTVPMFPlayer::GetNumberOfAudioStream( unsigned long *streamCount ){
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::SelectAudioStream( unsigned long num ){
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetEnableAudioStreamNum( long *num ){
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::DisableAudioStream( void ){
	/* 何もしない */
}

void __stdcall tTVPMFPlayer::GetNumberOfVideoStream( unsigned long *streamCount ){
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::SelectVideoStream( unsigned long num ){
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetEnableVideoStreamNum( long *num ){
	/* 何もしない */
}

void __stdcall tTVPMFPlayer::SetMixingBitmap( HDC hdc, RECT *dest, float alpha ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::ResetMixingBitmap() {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::SetMixingMovieAlpha( float a ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetMixingMovieAlpha( float *a ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::SetMixingMovieBGColor( unsigned long col ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetMixingMovieBGColor( unsigned long *col ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::PresentVideoImage() {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetContrastRangeMin( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetContrastRangeMax( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetContrastDefaultValue( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetContrastStepSize( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetContrast( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::SetContrast( float v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetBrightnessRangeMin( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetBrightnessRangeMax( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetBrightnessDefaultValue( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetBrightnessStepSize( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetBrightness( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::SetBrightness( float v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetHueRangeMin( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetHueRangeMax( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetHueDefaultValue( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetHueStepSize( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetHue( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::SetHue( float v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetSaturationRangeMin( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetSaturationRangeMax( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetSaturationDefaultValue( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetSaturationStepSize( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::GetSaturation( float *v ) {
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::SetSaturation( float v ) {
	/* 何もしない */
}



