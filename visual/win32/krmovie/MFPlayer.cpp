
#include <windows.h>
#include <assert.h>
#include <math.h>
#include <cmath>
#include <propvarutil.h>
#include <string>
#include <vector>

// for MFCreateMFByteStreamOnStream, MFCreateVideoRendererActivate
#include <Mfidl.h>	// Mfplat.lib, Mfplat.dll
#include <mfapi.h>
#include <mferror.h>
#include <evr.h>

#include <streams.h>
#include <atlbase.h>
#include <atlcom.h>

#include "../krmovie.h"
#include "MFPlayer.h"
#include "DShowException.h"
#include "tp_stub.h"

#pragma comment( lib, "propsys.lib" )
#pragma comment( lib, "Mfplat.lib" )
#pragma comment( lib, "Mf.lib" )
#pragma comment( lib, "Mfuuid.lib" )
//#pragma comment( lib, "d3d9.lib" )
//#pragma comment( lib, "dxva2.lib" )
//#pragma comment( lib, "evr.lib" )

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
	HWND callbackwin, IStream *stream, const wchar_t * streamname,
	const wchar_t *type, unsigned __int64 size, iTVPVideoOverlay **out)
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
			// OutputDebugString( std::to_wstring(met).c_str() ); OutputDebugString( L"\n" );
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

	OwnerWindow = NULL;
	CallbackWindow = NULL;
	Visible = false;
	Rect.left = 0; Rect.top = 0; Rect.right = 320; Rect.bottom = 240;
	RefCount = 1;
	Shutdown = false;
	PlayerCallback = new tTVPPlayerCallback(this);
	ByteStream = NULL;
	AudioVolume = NULL;
	VideoDisplayControl = NULL;
	VideoStatue = vsStopped;
	FPSNumerator = 1;
	FPSDenominator = 1;
	
	MediaSession = NULL;
	Topology = NULL;
	Stream = NULL;

	HnsDuration = 0;
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
//----------------------------------------------------------------------------
void __stdcall tTVPMFPlayer::BuildGraph( HWND callbackwin, IStream *stream,
	const wchar_t * streamname, const wchar_t *type, unsigned __int64 size )
{
	VideoStatue = vsProcessing;

	OwnerWindow = callbackwin;
	PlayWindow::SetOwner( callbackwin );
	CallbackWindow = callbackwin;
	PlayWindow::SetMessageDrainWindow( callbackwin );
	StreamName = std::wstring(streamname);
	Stream = stream;
	Stream->AddRef();

	HRESULT hr = S_OK;
	if( FAILED(hr = MFCreateMFByteStreamOnStream( stream, &ByteStream )) ) {
		TVPThrowExceptionMessage(L"Faild to create stream.");
		VideoStatue = vsStopped;
	}
}

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
				VideoDisplayControl = NULL;
			}
			if( FAILED(hr = pGetService->GetService( MR_STREAM_VOLUME_SERVICE, IID_IMFAudioStreamVolume, (void**)&AudioVolume )) ) {
				AudioVolume = NULL;
			}
			pGetService->GetService( MF_RATE_CONTROL_SERVICE, IID_IMFRateControl, (void**)&RateControl.p );
			pGetService->GetService( MF_RATE_CONTROL_SERVICE, IID_IMFRateSupport, (void**)&RateSupport.p );
			
			CComPtr<IMFPresentationDescriptor> pPD;
			if( SUCCEEDED(hr = GetPresentationDescriptorFromTopology( &pPD )) ) {
				(void)pPD->GetUINT64(MF_PD_DURATION, (UINT64*)&HnsDuration);
			}
		}
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
	if( MediaSession ) {
		return S_OK;	// 既に作成済み
	}

	HRESULT hr = CreateChildWindow();
	if( hr != S_OK ) return hr;

	HWND hWnd = GetChildWindow();
	if( hWnd == NULL || hWnd == INVALID_HANDLE_VALUE )
		return E_FAIL;

	if( FAILED(hr = MFCreateMediaSession( NULL, &MediaSession )) ) {
		TVPThrowExceptionMessage(L"Faild to create Media session.");
	}
	if( FAILED(hr = MediaSession->BeginGetEvent( PlayerCallback, NULL )) ) {
		TVPThrowExceptionMessage(L"Faild to begin get event.");
	}
	CComPtr<IMFSourceResolver> pSourceResolver;
	if( FAILED(hr = MFCreateSourceResolver(&pSourceResolver)) ) {
		TVPThrowExceptionMessage(L"Faild to create source resolver.");
	}
	MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;
	CComPtr<IUnknown> pSource;
	if( FAILED(hr = pSourceResolver->CreateObjectFromByteStream( ByteStream, StreamName.c_str(), MF_RESOLUTION_MEDIASOURCE, NULL, &ObjectType, (IUnknown**)&pSource )) ) {
	//if( FAILED(hr = pSourceResolver->CreateObjectFromURL( L"C:\\ToolDev\\kirikiri_vc\\master\\krkrz\\bin\\win32\\data\\test.mp4",
	//	MF_RESOLUTION_MEDIASOURCE, NULL, &ObjectType, (IUnknown**)&pSource)) ) {
		TVPThrowExceptionMessage(L"Faild to open stream.");
	}
	if( ObjectType != MF_OBJECT_MEDIASOURCE ) {
		TVPThrowExceptionMessage(L"Invalid media source.");
	}
	CComPtr<IMFMediaSource> pMediaSource;
	if( FAILED(hr = pSource.QueryInterface(&pMediaSource)) ) {
		TVPThrowExceptionMessage(L"Faild to query Media source.");
	}
	if( FAILED(hr = MFCreateTopology(&Topology)) ) {
		TVPThrowExceptionMessage(L"Faild to create Topology.");
	}
	CComPtr<IMFPresentationDescriptor> pPresentationDescriptor;
	if( FAILED(hr = pMediaSource->CreatePresentationDescriptor(&pPresentationDescriptor)) ) {
		TVPThrowExceptionMessage(L"Faild to create Presentation Descriptor.");
	}
	DWORD streamCount;
	if( FAILED(hr = pPresentationDescriptor->GetStreamDescriptorCount(&streamCount)) ) {
		TVPThrowExceptionMessage(L"Faild to get stream count.");
	}
	if( streamCount < 1 ) {
		TVPThrowExceptionMessage(L"Not found media stream.");
	}
	for( DWORD i = 0; i < streamCount; i++ ) {
		if( FAILED(hr = AddBranchToPartialTopology(Topology, pMediaSource, pPresentationDescriptor, i, hWnd)) ) {
			TVPThrowExceptionMessage(L"Faild to add nodes.");
		}
	}
	
	if( FAILED(hr = MediaSession->SetTopology( 0, Topology )) ) {
		TVPThrowExceptionMessage(L"Faild to set topology.");
	}
#if 0
	hr = pSource.QueryInterface(&MediaItem);
	if( SUCCEEDED(hr) ) {
		BOOL hasVideo, selected;
		hr = MediaItem->HasVideo( &hasVideo, &selected );
		if( FAILED(hr) ) {
			TVPThrowExceptionMessage(L"Faild to call HasVide.");
		}
		if( hasVideo && selected ) {
			DWORD streamCount = 0;
			hr = MediaItem->GetNumberOfStreams( &streamCount );
			if( FAILED(hr) ) {
				TVPThrowExceptionMessage(L"Faild to call GetNumberOfStreams.");
			}
			DWORD videoStreamIndex = 0;
			PROPVARIANT var;
			PropVariantInit(&var);
			for( DWORD sidx = 0; sidx < streamCount; sidx++ ) {
				PropVariantInit(&var);
				hr = MediaItem->GetStreamAttribute( sidx, MF_MT_MAJOR_TYPE, &var);
				if( SUCCEEDED(hr) && var.vt == VT_CLSID ) {
					if( IsEqualGUID( *var.puuid, MFMediaType_Video ) ) {
						// OutputDebugString( L"Video" );
						PropVariantClear(&var);
						videoStreamIndex = sidx;
						PropVariantInit(&var);

						// Get FPS
						if( SUCCEEDED(hr = MediaItem->GetStreamAttribute( sidx, MF_MT_FRAME_RATE, &var)) ) {
							ULONGLONG val;
							hr = PropVariantToUInt64( var, &val );
							if( SUCCEEDED(hr) ) {
								FPSNumerator = (unsigned long)((val>>32ULL)&0xffffffff);
								FPSDenominator = (unsigned long)(val&0xffffffff);
							}
							PropVariantClear(&var);
						}
						break;
					} else if( IsEqualGUID( *var.puuid, MFMediaType_Audio ) ) {
						// OutputDebugString( L"Audio" );
					}
				}
				PropVariantClear(&var);
			}
		}
	}
#endif

#if 0
	//hr = MFPCreateMediaPlayer( NULL, FALSE, 0, PlayerCallback, NULL, &MediaPlayer );
	hr = MFPCreateMediaPlayer( NULL, FALSE, 0, PlayerCallback, NULL, &MediaPlayer );
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(L"Faild to create Media Player.");
	}


	//hr = MediaPlayer->CreateMediaItemFromObject( ByteStream, FALSE, this, &MediaItem );	// 非同期処理
	hr = MediaPlayer->CreateMediaItemFromObject( ByteStream, TRUE, (DWORD_PTR)this, &MediaItem );	// 同期処理
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(L"Faild to create media item.");
	}

	// 画像ストリームに自前の描画用Sinkを設定する
	// 非同期でメディアを開く場合、この処理は、MFP_EVENT_TYPE_MEDIAITEM_SET イベント時に行う必要がある
	// 取得は IMFPMediaPlayer::GetMediaItem で行う
	BOOL hasVideo, selected;
	hr = MediaItem->HasVideo( &hasVideo, &selected );
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(L"Faild to call HasVide.");
	}
	if( hasVideo && selected ) {
		CComPtr<IMFActivate> pActivate;
		if( FAILED(hr = MFCreateVideoRendererActivate(OwnerWindow, &pActivate)) ) {
			TVPThrowExceptionMessage(L"Faild to call MFCreateVideoRendererActivate.");
		}
		//CComObject<EVRActiveObject>* pEVRActiveObj;
		//CComObject<EVRActiveObject>::CreateInstance(&pEVRActiveObj);
		//CComPtr<IUnknown> pUnknown(pEVRActiveObj);
		CComPtr<IUnknown> pUnknown;
		if( FAILED(hr = tTVPEVRCustomPresenter::CreateInstance( NULL, IID_IUnknown, (void**)&pUnknown ) ) ) {
			TVPThrowExceptionMessage(L"Faild to create Activate.");
		}
		if( FAILED(hr = pActivate->SetUnknown(MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_ACTIVATE, pUnknown) ) ) {
			TVPThrowExceptionMessage(L"Faild to call IMFActivate::SetUnknown.");
		}
		CComPtr<IMFVideoPresenter> pVideoPresenter;
		//if( FAILED(hr = pUnknown.ActivateObject( IID_IMFVideoPresenter, (void**)&pVideoPresenter ) ) ) {
		if( FAILED(hr = pUnknown.QueryInterface( &pVideoPresenter ) ) ) {
			TVPThrowExceptionMessage(L"Faild to get VideoPresenter.");
		}
		//if( FAILED( hr = pVideoPresenter.p->QueryInterface( IID_IMFVideoDisplayControl, (void**)&VideoDisplayControl ) ) ) {
		if( FAILED( hr = pVideoPresenter.QueryInterface( &VideoDisplayControl ) ) ) {
			TVPThrowExceptionMessage(L"Faild to get VideoDisplayControl.");
		}

		DWORD streamCount = 0;
		hr = MediaItem->GetNumberOfStreams( &streamCount );
		if( FAILED(hr) ) {
			TVPThrowExceptionMessage(L"Faild to call GetNumberOfStreams.");
		}
		DWORD videoStreamIndex = 0;
		PROPVARIANT var;
		PropVariantInit(&var);
		for( DWORD sidx = 0; sidx < streamCount; sidx++ ) {
			PropVariantInit(&var);
			hr = MediaItem->GetStreamAttribute( sidx, MF_MT_MAJOR_TYPE, &var);
			if( SUCCEEDED(hr) && var.vt == VT_CLSID ) {
				if( IsEqualGUID( *var.puuid, MFMediaType_Video ) ) {
					// OutputDebugString( L"Video" );
					PropVariantClear(&var);
					videoStreamIndex = sidx;
					PropVariantInit(&var);

					// Get FPS
					if( SUCCEEDED(hr = MediaItem->GetStreamAttribute( sidx, MF_MT_FRAME_RATE, &var)) ) {
						ULONGLONG val;
						hr = PropVariantToUInt64( var, &val );
						if( SUCCEEDED(hr) ) {
							FPSNumerator = (unsigned long)((val>>32ULL)&0xffffffff);
							FPSDenominator = (unsigned long)(val&0xffffffff);
						}
						PropVariantClear(&var);
					}
					break;
				} else if( IsEqualGUID( *var.puuid, MFMediaType_Audio ) ) {
					// OutputDebugString( L"Audio" );
				}
			}
			PropVariantClear(&var);
		}
		hr = MediaItem->SetStreamSink( videoStreamIndex, pActivate );
		if( FAILED(hr) ) {
			TVPThrowExceptionMessage(L"Faild to set Video Sink.");
		}
	}
#endif
	/*
	HRESULT hr = tTVPAudioSessionVolume::CreateInstance( WM_AUDIO_EVENT, callbackwin, &AudioVolume );
	if( SUCCEEDED(hr) ) {
		// Ask for audio session events.
		hr = AudioVolume->EnableNotifications(TRUE);
	}

	if( FAILED(hr) ) {
		if( AudioVolume ) {
			AudioVolume->Release();
			AudioVolume = NULL;
		}
	}
	*/
	return hr;
}

HRESULT tTVPMFPlayer::AddBranchToPartialTopology( IMFTopology *pTopology, IMFMediaSource *pSource, IMFPresentationDescriptor *pPD, DWORD iStream, HWND hVideoWnd ) {
	CComPtr<IMFStreamDescriptor>	pSD;
	HRESULT hr;
	BOOL selected = FALSE;
    if( FAILED(hr = pPD->GetStreamDescriptorByIndex(iStream, &selected, &pSD)) ) {
		TVPThrowExceptionMessage(L"Faild to get stream desc.");
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
			TVPThrowExceptionMessage(L"Faild to add source node.");
		}
		// Create the output node for the renderer.
		CComPtr<IMFTopologyNode> pOutputNode;
        if( FAILED(hr = AddOutputNode(pTopology, pSinkActivate, 0, &pOutputNode)) ) {
			TVPThrowExceptionMessage(L"Faild to add output node.");
		}
		// Connect the source node to the output node.
        if( FAILED(hr = pSourceNode->ConnectOutput(0, pOutputNode, 0)) ) {
			TVPThrowExceptionMessage(L"Faild to connect node.");
		}
	}
	return hr;
}
HRESULT tTVPMFPlayer::CreateMediaSinkActivate( IMFStreamDescriptor *pSourceSD, HWND hVideoWindow, IMFActivate **ppActivate ) {
	HRESULT hr;
	CComPtr<IMFMediaTypeHandler> pHandler;
	// Get the media type handler for the stream.
    if( FAILED(hr = pSourceSD->GetMediaTypeHandler(&pHandler)) ) {
		TVPThrowExceptionMessage(L"Faild to get media type handler.");
	}
	// Get the major media type.
    GUID guidMajorType;
    if( FAILED(hr = pHandler->GetMajorType(&guidMajorType)) ) {
		TVPThrowExceptionMessage(L"Faild to get major type.");
	}
    CComPtr<IMFActivate>		pActivate;
	if( MFMediaType_Audio == guidMajorType ) {
		// Create the audio renderer.
        if( FAILED(hr = MFCreateAudioRendererActivate(&pActivate) )) {
			TVPThrowExceptionMessage(L"Faild to create audio render.");
		}
	} else if( MFMediaType_Video == guidMajorType ) {
		// Get FPS
		CComPtr<IMFMediaType> pMediaType;
		if( SUCCEEDED(hr = pHandler->GetCurrentMediaType(&pMediaType)) ) {
			hr = MFGetAttributeRatio( pMediaType, MF_MT_FRAME_RATE, &FPSNumerator, &FPSDenominator );
		}

        // Create the video renderer.
        if( FAILED(hr = MFCreateVideoRendererActivate(hVideoWindow, &pActivate) ) ) {
			TVPThrowExceptionMessage(L"Faild to create video render.");
		}
		// TODO ここでカスタムEVRをつなぐようにすると自前で色々描画できるようになる
		// 現状は標準のものを使っている
#if 0
		tTVPEVRCustomPresenter* my_activate_obj = new tTVPEVRCustomPresenter(hr);
		my_activate_obj->AddRef();
		CComPtr<IUnknown> unk;
		my_activate_obj->QueryInterface( IID_IUnknown, (void**)&unk );
		if( FAILED(hr = pActivate->SetUnknown(MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_ACTIVATE, unk)) ) {
			my_activate_obj->Release();
			TVPThrowExceptionMessage(L"Faild to add custom EVR presenter video render.");
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
		TVPThrowExceptionMessage(L"Faild to create source node.");
	}
	// Set the attributes.
    if( FAILED(hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource) ) ) {
		TVPThrowExceptionMessage(L"Faild to set source node.");
	}
	if( FAILED(hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPD) ) ) {
		TVPThrowExceptionMessage(L"Faild to set presentation desc.");
	}
	if( FAILED(hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD)) ) {
		TVPThrowExceptionMessage(L"Faild to set stream desc.");
	}
	// Add the node to the topology.
    if( FAILED(hr = pTopology->AddNode(pNode)) ) {
		TVPThrowExceptionMessage(L"Faild to add source node to topology.");
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
		TVPThrowExceptionMessage(L"Faild to create output node.");
	}
    // Set the object pointer.
    if( FAILED(hr = pNode->SetObject(pActivate)) ) {
		TVPThrowExceptionMessage(L"Faild to set activate.");
	}
    // Set the stream sink ID attribute.
    if( FAILED(hr = pNode->SetUINT32(MF_TOPONODE_STREAMID, dwId)) ) {
		TVPThrowExceptionMessage(L"Faild to set stream id.");
	}
	if( FAILED(hr = pNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE)) ) {
		TVPThrowExceptionMessage(L"Faild to set no shutdown on remove.");
	}
    // Add the node to the topology.
    if( FAILED(hr = pTopology->AddNode(pNode)) ) {
		TVPThrowExceptionMessage(L"Faild to add ouput node to topology.");
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
		TVPThrowExceptionMessage(L"Faild to create output node.");
	}
	// Set the object pointer.
	if( FAILED(hr = pNode->SetObject(pStreamSink)) ) {
		TVPThrowExceptionMessage(L"Faild to set stream sink.");
	}
	// Add the node to the topology.
	if( FAILED(hr = pTopology->AddNode(pNode))) {
		TVPThrowExceptionMessage(L"Faild to add output node.");
	}
	if( FAILED(hr = pNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, TRUE) ) ) {
		TVPThrowExceptionMessage(L"Faild to set no shutdown on remove.");
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
	if( PlayerCallback ) {
		delete PlayerCallback;
		PlayerCallback = NULL;
	}
	if( ByteStream ) {
		ByteStream->Release();
		ByteStream = NULL;
	}
	if( AudioVolume ) {
		AudioVolume->Release();
		AudioVolume = NULL;
	}
	if( VideoDisplayControl ) {
		VideoDisplayControl->Release();
		VideoDisplayControl = NULL;
	}
	RateSupport.Release();
	RateControl.Release();

	if( Topology ) {
		Topology->Release();
		Topology = NULL;
	}
	if( MediaSession ) {
		MediaSession->Release();
		MediaSession = NULL;
	}
	if( Stream ) {
		Stream->Release();
		Stream = NULL;
	}
}
//----------------------------------------------------------------------------
void tTVPMFPlayer::NotifyError( HRESULT hr ) {
	TVPThrowExceptionMessage(L"MF Operation Error.",hr);
}
void tTVPMFPlayer::OnMediaItemCleared() {
}
void tTVPMFPlayer::OnPause() {
	VideoStatue = vsPaused;
}
void tTVPMFPlayer::OnPlayBackEnded() {
}
void tTVPMFPlayer::OnRateSet( double rate ) {
}
void tTVPMFPlayer::OnStop() {
	VideoStatue = vsStopped;
}
void tTVPMFPlayer::OnPlay() {
	VideoStatue = vsPlaying;
}
//----------------------------------------------------------------------------
void __stdcall tTVPMFPlayer::SetWindow(HWND window) {
	HRESULT hr = E_FAIL;
	OwnerWindow = window;
	PlayWindow::SetOwner( window );
	if( VideoDisplayControl ) {
		hr = VideoDisplayControl->SetVideoWindow( window );
		if( FAILED(hr) ) {
			TVPThrowExceptionMessage(L"Faild to call SetVideoWindow.");
		}
	}
	CreateVideoPlayer();
}
void __stdcall tTVPMFPlayer::SetMessageDrainWindow(HWND window) {
	PlayWindow::SetMessageDrainWindow( window );
	CallbackWindow = window;
}
void __stdcall tTVPMFPlayer::SetRect(RECT *rect) {
	PlayWindow::SetRect( rect );
	if( VideoDisplayControl ) {
		// MF では、ソース矩形を指定可能になっている
		HRESULT hr = VideoDisplayControl->SetVideoPosition( NULL, rect );
		if( FAILED(hr) ) {
			TVPThrowExceptionMessage(L"Faild to set rect.");
		}
	}
}
void __stdcall tTVPMFPlayer::SetVisible(bool b) {
	PlayWindow::SetVisible( b );
}
void __stdcall tTVPMFPlayer::Play() {
	HRESULT hr = E_FAIL;
	if( MediaSession ) {
		PROPVARIANT varStart;
		PropVariantInit(&varStart);
		hr = MediaSession->Start( &GUID_NULL, &varStart );
		PropVariantClear(&varStart);
	}
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(L"Faild to play.");
	}
}
void __stdcall tTVPMFPlayer::Stop() {
	HRESULT hr = E_FAIL;
	if( MediaSession ) {
		hr = MediaSession->Stop();
	}
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(L"Faild to stop.");
	}
}
void __stdcall tTVPMFPlayer::Pause() {
	HRESULT hr = E_FAIL;
	if( MediaSession ) {
		hr = MediaSession->Pause();
	}
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(L"Faild to stop.");
	}
}
// Seek
// http://msdn.microsoft.com/en-us/library/windows/desktop/ee892373%28v=vs.85%29.aspx
void __stdcall tTVPMFPlayer::SetPosition(unsigned __int64 tick) {
	HRESULT hr = E_FAIL;
	PROPVARIANT var;
	PropVariantInit(&var);
	/*
	if( MediaPlayer ) {
		var.vt = VT_I8;
		var.hVal.QuadPart = tick;
		HRESULT hr = MediaPlayer->SetPosition( MFP_POSITIONTYPE_100NS, &var );
	}
	*/
	PropVariantClear(&var);
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(L"Faild to set position.");
	}
}
void __stdcall tTVPMFPlayer::GetPosition(unsigned __int64 *tick) {
	HRESULT hr = E_FAIL;
	PROPVARIANT var;
	PropVariantInit(&var);
	/*
	if( MediaPlayer ) {
		hr = MediaPlayer->GetPosition( MFP_POSITIONTYPE_100NS, &var );
		if( SUCCEEDED(hr) ) {
			*tick = var.hVal.QuadPart;
		}
	}
	*/
	PropVariantClear(&var);
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(L"Faild to get position.");
	}
}
void __stdcall tTVPMFPlayer::GetStatus(tTVPVideoStatus *status) {
	if( status ) {
		*status = VideoStatue;
	}
}
void __stdcall tTVPMFPlayer::GetEvent(long *evcode, long *param1, long *param2, bool *got) {
	*got = false;
}
void __stdcall tTVPMFPlayer::FreeEventParams(long evcode, long param1, long param2) {
}
void __stdcall tTVPMFPlayer::Rewind() {
	SetPosition( 0 );
}
void __stdcall tTVPMFPlayer::SetFrame( int f ) {
	UINT64 avgTime;
	HRESULT hr = MFFrameRateToAverageTimePerFrame( FPSNumerator, FPSDenominator, &avgTime );
	if( SUCCEEDED(hr) ) {
		LONGLONG requestTime = avgTime * (LONGLONG)f;
		SetPosition( requestTime );
	}
}
void __stdcall tTVPMFPlayer::GetFrame( int *f ) {
	UINT64 avgTime;
	HRESULT hr = MFFrameRateToAverageTimePerFrame( FPSNumerator, FPSDenominator, &avgTime );
	if( SUCCEEDED(hr) ) {
		unsigned __int64 tick;
		GetPosition( &tick );
		*f = (int)( tick / avgTime );
	}
}
void __stdcall tTVPMFPlayer::GetFPS( double *f ) {
	*f = (double)FPSNumerator / (double)FPSDenominator;
}
void __stdcall tTVPMFPlayer::GetNumberOfFrame( int *f ) {
	UINT64 avgTime;
	HRESULT hr = MFFrameRateToAverageTimePerFrame( FPSNumerator, FPSDenominator, &avgTime );
	if( SUCCEEDED(hr) ) {
		long long t;
		GetTotalTime( &t );
		*f = (int)( t / avgTime );
	}
}
/**
 * @brief ムービーの長さ(msec)を取得する
 * @param f ムービーの長さを入れる変数へのポインタ
 */
void __stdcall tTVPMFPlayer::GetTotalTime( __int64 *t ) {
	// http://msdn.microsoft.com/en-us/library/windows/desktop/dd979590%28v=vs.85%29.aspx
    *t = (HnsDuration / (ONE_SECOND / ONE_MSEC));
}

void __stdcall tTVPMFPlayer::GetVideoSize( long *width, long *height ){
	HRESULT hr = E_FAIL;
	if( VideoDisplayControl ) {
		SIZE vsize;
		hr = VideoDisplayControl->GetNativeVideoSize( &vsize, NULL );
		if( FAILED(hr) ) {
			TVPThrowExceptionMessage(L"Faild to get video size.");
		}
		*width = vsize.cx;
		*height = vsize.cy;
	}
}
void __stdcall tTVPMFPlayer::GetFrontBuffer( BYTE **buff ){
	/* 何もしない */
}
void __stdcall tTVPMFPlayer::SetVideoBuffer( BYTE *buff1, BYTE *buff2, long size ){
	/* 何もしない */
}

void __stdcall tTVPMFPlayer::SetStopFrame( int frame ) {
}
void __stdcall tTVPMFPlayer::GetStopFrame( int *frame ) {
}
void __stdcall tTVPMFPlayer::SetDefaultStopFrame() {
}

void __stdcall tTVPMFPlayer::SetPlayRate( double rate ) {
	HRESULT hr = E_FAIL;
	if( RateSupport.p && RateControl.p ) {
		float playrate = (float)rate;
		float acceptrate = playrate;
		if( SUCCEEDED(hr = RateSupport->IsRateSupported( TRUE, playrate, &acceptrate )) ) {
			hr = RateControl->SetRate( TRUE, acceptrate );
		}
	}
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(L"Faild to set play rate.");
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
		TVPThrowExceptionMessage(L"Faild to get play rate.");
	}
}

void __stdcall tTVPMFPlayer::SetAudioBalance( long balance ) {
	HRESULT hr = E_FAIL;
	// AudioVolume でチャンネルに応じたバランス設定していかないといけないので非サポートにしておく
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(L"Faild to set audio balance.");
	}
}
void __stdcall tTVPMFPlayer::GetAudioBalance( long *balance ) {
	HRESULT hr = E_FAIL;
	// AudioVolume でチャンネルに応じたバランス設定していかないといけないので非サポートにしておく
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(L"Faild to get audio balance.");
	}
}
void __stdcall tTVPMFPlayer::SetAudioVolume( long volume ) {
	HRESULT hr = E_FAIL;
	if( AudioVolume ) {
		UINT32 count;
		if( SUCCEEDED(hr = AudioVolume->GetChannelCount( &count )) ) {
			std::vector<float> channels(count);
			float v = 0.0f;
			if( volume > -10000  ) {
				v = (10000 + volume)/10000.0f;
			} 
			for( UINT32 i = 0; i < count; i++ ) {
				channels[i] = v;
			}
			hr = AudioVolume->SetAllVolumes( count, &channels[0] );
		}
	}
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(L"Faild to set audio volume.");
	}
}
void __stdcall tTVPMFPlayer::GetAudioVolume( long *volume ) {
	HRESULT hr = E_FAIL;
	float vol = 0.0f;
	if( AudioVolume ) {
		UINT32 count;
		if( SUCCEEDED(hr = AudioVolume->GetChannelCount( &count )) ) {
			std::vector<float> channels(count);
			if( SUCCEEDED(hr = AudioVolume->GetAllVolumes( count, &channels[0] )) ) {
				float total = 0.0f;
				for( UINT32 i = 0; i < count; i++ ) {
					total += channels[i];
				}
				vol = total / count;
			}
		}
	}
	if( FAILED(hr) ) {
		TVPThrowExceptionMessage(L"Faild to get audio volume.");
	} else {
		*volume = 10000 - (long)(vol*10000);
	}
}

void __stdcall tTVPMFPlayer::GetNumberOfAudioStream( unsigned long *streamCount ){
}
void __stdcall tTVPMFPlayer::SelectAudioStream( unsigned long num ){
}
void __stdcall tTVPMFPlayer::GetEnableAudioStreamNum( long *num ){
}
void __stdcall tTVPMFPlayer::DisableAudioStream( void ){
}

void __stdcall tTVPMFPlayer::GetNumberOfVideoStream( unsigned long *streamCount ){
}
void __stdcall tTVPMFPlayer::SelectVideoStream( unsigned long num ){
}
void __stdcall tTVPMFPlayer::GetEnableVideoStreamNum( long *num ){
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



