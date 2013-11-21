
#define NOMINMAX
#include "tjsCommHead.h"
#include "DrawDevice.h"
#include "BasicDrawDevice.h"
#include "LayerIntf.h"
#include "MsgIntf.h"
#include "SysInitIntf.h"
#include "WindowIntf.h"
#include "DebugIntf.h"
#include "ThreadIntf.h"
#include "ComplexRect.h"
#include "EventImpl.h"
#include "WindowImpl.h"

#include <d3d9.h>
#include <mmsystem.h>
#include <algorithm>

//---------------------------------------------------------------------------
// オプション
//---------------------------------------------------------------------------
static tjs_int TVPBasicDrawDeviceOptionsGeneration = 0;
bool TVPZoomInterpolation = true;
//---------------------------------------------------------------------------
static void TVPInitBasicDrawDeviceOptions()
{
	if(TVPBasicDrawDeviceOptionsGeneration == TVPGetCommandLineArgumentGeneration()) return;
	TVPBasicDrawDeviceOptionsGeneration = TVPGetCommandLineArgumentGeneration();

	tTJSVariant val;
	TVPZoomInterpolation = true;
	if(TVPGetCommandLine(TJS_W("-smoothzoom"), &val))
	{
		ttstr str(val);
		if(str == TJS_W("no"))
			TVPZoomInterpolation = false;
		else
			TVPZoomInterpolation = true;
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
tTVPBasicDrawDevice::tTVPBasicDrawDevice()
{
	TVPInitBasicDrawDeviceOptions(); // read and initialize options
	TargetWindow = NULL;
	DrawUpdateRectangle = false;

	Direct3D = NULL;
	Direct3DDevice = NULL;
	Texture = NULL;
	ShouldShow = false;
	TextureBuffer = NULL;
	TextureWidth = TextureHeight = 0;
	VsyncInterval = 16;
	ZeroMemory( &D3dPP, sizeof(D3dPP) );
	ZeroMemory( &DispMode, sizeof(DispMode) );
}
//---------------------------------------------------------------------------
tTVPBasicDrawDevice::~tTVPBasicDrawDevice()
{
	DestroyD3DDevice();
}
//---------------------------------------------------------------------------
void tTVPBasicDrawDevice::DestroyD3DDevice() {
	DestroyTexture();
	if(Direct3DDevice) Direct3DDevice->Release(), Direct3DDevice = NULL;
	if(Direct3D) Direct3D = NULL;
}
//---------------------------------------------------------------------------
void tTVPBasicDrawDevice::DestroyTexture() {
	if(TextureBuffer && Texture) Texture->UnlockRect(0), TextureBuffer = NULL;
	if(Texture) Texture->Release(), Texture = NULL;
}
//---------------------------------------------------------------------------
void tTVPBasicDrawDevice::InvalidateAll()
{
	// レイヤ演算結果をすべてリクエストする
	// サーフェースが lost した際に内容を再構築する目的で用いる
	RequestInvalidation(tTVPRect(0, 0, DestRect.get_width(), DestRect.get_height()));
}
//---------------------------------------------------------------------------
void tTVPBasicDrawDevice::CheckMonitorMoved() {
	UINT iCurrentMonitor = GetMonitorNumber( TargetWindow );
	if( CurrentMonitor != iCurrentMonitor ) {
		// モニタ移動が発生しているので、デバイスを再生成する
		CreateD3DDevice();
	}
}
//---------------------------------------------------------------------------
bool tTVPBasicDrawDevice::IsTargetWindowActive() const {
	if( TargetWindow == NULL ) return false;
	return ::GetForegroundWindow() == TargetWindow;
}
//---------------------------------------------------------------------------
bool tTVPBasicDrawDevice::GetDirect3D9Device() {
	DestroyD3DDevice();

	TVPEnsureDirect3DObject();

	if( NULL == ( Direct3D = TVPGetDirect3DObjectNoAddRef() ) )
		TVPThrowExceptionMessage( TVPFaildToCreateDirect3D );

	HRESULT hr;
	if( FAILED( hr = DecideD3DPresentParameters() ) ) {
		if( IsTargetWindowActive() ) {
			ErrorToLog( hr );
			TVPThrowExceptionMessage( TVPFaildToDecideBackbufferFormat );
		}
		return false;
	}

	UINT iCurrentMonitor = GetMonitorNumber( TargetWindow );
	DWORD	BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED;
	if( D3D_OK != ( hr = Direct3D->CreateDevice( iCurrentMonitor, D3DDEVTYPE_HAL, TargetWindow, BehaviorFlags, &D3dPP, &Direct3DDevice ) ) ) {
		if( IsTargetWindowActive() ) {
			ErrorToLog( hr );
			TVPThrowExceptionMessage( TVPFaildToCreateDirect3DDevice );
		}
		return false;
	}
	CurrentMonitor = iCurrentMonitor;

	/*
	D3DVIEWPORT9 vp;
	vp.X  = DestLeft;
	vp.Y  = DestTop;
	vp.Width = DestWidth != 0 ? DestWidth : D3dPP.BackBufferWidth;
	vp.Height = DestHeight != 0 ? DestHeight : D3dPP.BackBufferHeight;
	*/
	
	D3DVIEWPORT9 vp;
	vp.X  = 0;
	vp.Y  = 0;
	vp.Width = D3dPP.BackBufferWidth;
	vp.Height = D3dPP.BackBufferHeight;
	vp.MinZ  = 0.0f;
	vp.MaxZ  = 1.0f;
	if( FAILED(hr = Direct3DDevice->SetViewport(&vp)) ) {
		if( IsTargetWindowActive() ) {
			ErrorToLog( hr );
			TVPThrowExceptionMessage( TVPFaildToSetViewport );
		}
		return false;
	}

	if( FAILED( hr = InitializeDirect3DState() ) ) {
		if( IsTargetWindowActive() ) {
			ErrorToLog( hr );
 			TVPThrowExceptionMessage( TVPFaildToSetRenderState );
		}
		return false;
	}

	int refreshrate = DispMode.RefreshRate;
	if( refreshrate == 0 ) {
		HDC hdc;
		hdc = ::GetDC(TargetWindow);
		refreshrate = GetDeviceCaps( hdc, VREFRESH );
		::ReleaseDC( TargetWindow, hdc );
	}
	VsyncInterval = 1000 / refreshrate;
	return true;
}
//---------------------------------------------------------------------------
HRESULT tTVPBasicDrawDevice::InitializeDirect3DState() {
	HRESULT	hr;
	D3DCAPS9	d3dcaps;
	if( FAILED( hr = Direct3DDevice->GetDeviceCaps( &d3dcaps ) ) )
		return hr;

	if( d3dcaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR ) {
		if( FAILED( hr = Direct3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, TVPZoomInterpolation?D3DTEXF_LINEAR:D3DTEXF_POINT ) ) )
			return hr;
	} else {
		if( FAILED( hr = Direct3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT ) ) )
			return hr;
	}

	if( d3dcaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR ) {
		if( FAILED( hr = Direct3DDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, TVPZoomInterpolation?D3DTEXF_LINEAR:D3DTEXF_POINT ) ) )
			return hr;
	} else {
		if( FAILED( hr = Direct3DDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT ) ) )
		return hr;
	}

	if( FAILED( hr = Direct3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP ) ) )
		return hr;
	if( FAILED( hr = Direct3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP ) ) )
		return hr;
	if( FAILED( hr = Direct3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE ) ) )
		return hr;
	if( FAILED( hr = Direct3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE ) ) )
		return hr;
	if( FAILED( hr = Direct3DDevice->SetRenderState( D3DRS_ZENABLE, FALSE ) ) )
		return hr;
	if( FAILED( hr = Direct3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE ) ) )
		return hr;
	if( FAILED( hr = Direct3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 ) ) )
		return hr;
	if( FAILED( hr = Direct3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ) ) )
		return hr;
	if( FAILED( hr = Direct3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE ) ) )
		return hr;

	return S_OK;
}
//---------------------------------------------------------------------------
UINT tTVPBasicDrawDevice::GetMonitorNumber( HWND window )
{
	if( Direct3D == NULL || window == NULL ) return D3DADAPTER_DEFAULT;
	HMONITOR windowMonitor = ::MonitorFromWindow( window, MONITOR_DEFAULTTOPRIMARY );
	UINT iCurrentMonitor = 0;
	UINT numOfMonitor = Direct3D->GetAdapterCount();
	for( ; iCurrentMonitor < numOfMonitor; ++iCurrentMonitor ) 	{
		if( Direct3D->GetAdapterMonitor(iCurrentMonitor) == windowMonitor )
			break;
	}
	if( iCurrentMonitor == numOfMonitor )
		iCurrentMonitor = D3DADAPTER_DEFAULT;
	return iCurrentMonitor;
}
//---------------------------------------------------------------------------
HRESULT tTVPBasicDrawDevice::DecideD3DPresentParameters() {
	HRESULT			hr;
	UINT iCurrentMonitor = GetMonitorNumber(TargetWindow);
	if( FAILED( hr = Direct3D->GetAdapterDisplayMode( iCurrentMonitor, &DispMode ) ) )
		return hr;

	ZeroMemory( &D3dPP, sizeof(D3dPP) );
	D3dPP.Windowed = TRUE;
	D3dPP.SwapEffect = D3DSWAPEFFECT_COPY;
	D3dPP.BackBufferFormat = D3DFMT_UNKNOWN;
	D3dPP.BackBufferHeight = DispMode.Height;
	D3dPP.BackBufferWidth = DispMode.Width;
	D3dPP.hDeviceWindow = TargetWindow;
	D3dPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	return S_OK;
}
//---------------------------------------------------------------------------
bool tTVPBasicDrawDevice::CreateD3DDevice()
{
	// Direct3D デバイス、テクスチャなどを作成する
	DestroyD3DDevice();
	if( TargetWindow ) {
		tjs_int w, h;
		GetSrcSize( w, h );
		if( w > 0 && h > 0 ) {
			// get Direct3D9 interface
			if( GetDirect3D9Device() ) {
				return CreateTexture();
			}
		}
	}
	return false;
}
//---------------------------------------------------------------------------
bool tTVPBasicDrawDevice::CreateTexture() {
	DestroyTexture();
	tjs_int w, h;
	GetSrcSize( w, h );
	if(TargetWindow && w > 0 && h > 0) {
		HRESULT hr = S_OK;

		D3DCAPS9 d3dcaps;
		Direct3DDevice->GetDeviceCaps( &d3dcaps );

		TextureWidth = w;
		TextureHeight = h;
		if( d3dcaps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) {
			// only square textures are supported
			TextureWidth = std::max(TextureHeight, TextureWidth);
			TextureHeight = TextureWidth;
		}

		DWORD dwWidth = 64;
		DWORD dwHeight = 64;
		if( d3dcaps.TextureCaps & D3DPTEXTURECAPS_POW2 ) {
			// 2の累乗のみ許可するかどうか判定
			while( dwWidth < TextureWidth ) dwWidth = dwWidth << 1;
			while( dwHeight < TextureHeight ) dwHeight = dwHeight << 1;
			TextureWidth = dwWidth;
			TextureHeight = dwHeight;

			if( dwWidth > d3dcaps.MaxTextureWidth || dwHeight > d3dcaps.MaxTextureHeight ) {
				TVPAddLog( (const tjs_char*)TVPWarningImageSizeTooLargeMayBeCannotCreateTexture );
			}
			TVPAddLog( (const tjs_char*)TVPUsePowerOfTwoSurface );
		} else {
			dwWidth = TextureWidth;
			dwHeight = TextureHeight;
		}

		if( D3D_OK != ( hr = Direct3DDevice->CreateTexture( dwWidth, dwHeight, 1, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &Texture, NULL) ) ) {
			if( IsTargetWindowActive() ) {
				ErrorToLog( hr );
				TVPThrowExceptionMessage(TVPCannotAllocateD3DOffScreenSurface,TJSInt32ToHex(hr, 8));
			}
			return false;
		}
	}
	return true;
}
//---------------------------------------------------------------------------
void tTVPBasicDrawDevice::EnsureDevice()
{
	TVPInitBasicDrawDeviceOptions();
	if( TargetWindow ) {
		try {
			bool recreate = false;
			if( Direct3D == NULL || Direct3DDevice == NULL ) {
				if( GetDirect3D9Device() == false ) {
					return;
				}
				recreate = true;
			}
			if( Texture == NULL ) {
				if( CreateTexture() == false ) {
					return;
				}
				recreate = true;
			}
			if( recreate ) {
				InvalidateAll();
			}
		} catch(const eTJS & e) {
			TVPAddImportantLog( TVPFormatMessage(TVPPassthroughFailedToCreateDrawer,e.GetMessage()) );
			DestroyD3DDevice();
		} 	catch(...) 	{
			TVPAddImportantLog( (const tjs_char*)TVPPassthroughFailedToCreateDrawerUnknownReason );
			DestroyD3DDevice();
		}
	}
}
//---------------------------------------------------------------------------
void tTVPBasicDrawDevice::TryRecreateWhenDeviceLost()
{
	bool success = false;
	if( Direct3DDevice ) {
		DestroyTexture();
		HRESULT hr = Direct3DDevice->TestCooperativeLevel();
		if( hr == D3DERR_DEVICENOTRESET ) {
			hr = Direct3DDevice->Reset(&D3dPP);
		}
		if( FAILED(hr) ) {
			success = CreateD3DDevice();
		} else {
			if( D3D_OK == InitializeDirect3DState() ) {
				success = true;
			}
		}
	} else {
		success = CreateD3DDevice();
	}
	if( success ) {
		InvalidateAll();	// 画像の再描画(Layer Update)を要求する
	}
}
//---------------------------------------------------------------------------
void tTVPBasicDrawDevice::ErrorToLog( HRESULT hr ) {
	switch( hr ) {
	case D3DERR_DEVICELOST:
		TVPAddLog( (const tjs_char*)TVPErrorDeviceLostCannotResetDevice );
		break;
	case D3DERR_DRIVERINTERNALERROR:
		TVPAddLog( (const tjs_char*)TVPErrorDeviceInternalFatalError );
		break;
	case D3DERR_INVALIDCALL:
		TVPAddLog( (const tjs_char*)TVPErrorInvalidCall );
		break;
	case D3DERR_OUTOFVIDEOMEMORY:
		TVPAddLog( (const tjs_char*)TVPErrorCannotAllocateVideoMemory );
		break;
	case E_OUTOFMEMORY:
		TVPAddLog( (const tjs_char*)TVPErrorCannotAllocateMemory );
		break;
	case D3DERR_WRONGTEXTUREFORMAT:
		TVPAddLog( TJS_W("D3D : テクスチャ サーフェイスのピクセル フォーマットが無効です") );
		break;
	case D3DERR_UNSUPPORTEDCOLOROPERATION:
		TVPAddLog( TJS_W("D3D : 色値に対して指定されているテクスチャ ブレンディング処理を、デバイスがサポートしていません") );
		break;
	case D3DERR_UNSUPPORTEDCOLORARG:
		TVPAddLog( TJS_W("D3D : 色値に対して指定されているテクスチャ ブレンディング引数を、デバイスがサポートしていません") );
		break;
	case D3DERR_UNSUPPORTEDALPHAOPERATION:
		TVPAddLog( TJS_W("D3D : アルファ チャンネルに対して指定されているテクスチャ ブレンディング処理を、デバイスがサポートしていません") );
		break;
	case D3DERR_UNSUPPORTEDALPHAARG:
		TVPAddLog( TJS_W("D3D : アルファ チャンネルに対して指定されているテクスチャ ブレンディング引数を、デバイスがサポートしていません") );
		break;
	case D3DERR_TOOMANYOPERATIONS:
		TVPAddLog( TJS_W("D3D : デバイスがサポートしている数より多くのテクスチャ フィルタリング処理を、アプリケーションが要求しています") );
		break;
	case D3DERR_CONFLICTINGTEXTUREFILTER:
		TVPAddLog( TJS_W("D3D : 現在のテクスチャ フィルタは同時には使えません") );
		break;
	case D3DERR_UNSUPPORTEDFACTORVALUE:
		TVPAddLog( TJS_W("D3D : デバイスが指定されたテクスチャ係数値をサポートしていません") );
		break;
	case D3DERR_CONFLICTINGRENDERSTATE:
		TVPAddLog( TJS_W("D3D : 現在設定されているレンダリング ステートは同時には使えません") );
		break;
	case D3DERR_UNSUPPORTEDTEXTUREFILTER:
		TVPAddLog( TJS_W("D3D : デバイスが指定されたテクスチャ フィルタをサポートしていません") );
		break;
	case D3DERR_CONFLICTINGTEXTUREPALETTE:
		TVPAddLog( TJS_W("D3D : 現在のテクスチャは同時には使えません") );
		break;
	case D3DERR_NOTFOUND:
		TVPAddLog( TJS_W("D3D : 要求された項目が見つかりませんでした") );
		break;
	case D3DERR_MOREDATA:
		TVPAddLog( TJS_W("D3D : 指定されたバッファ サイズに保持できる以上のデータが存在します") );
		break;
	case D3DERR_DEVICENOTRESET:
		TVPAddLog( TJS_W("D3D : デバイスは、消失していますが、現在リセットできます") );
		break;
	case D3DERR_NOTAVAILABLE:
		TVPAddLog( TJS_W("D3D : このデバイスは、照会されたテクニックをサポートしていません") );
		break;
	case D3DERR_INVALIDDEVICE:
		TVPAddLog( TJS_W("D3D : 要求されたデバイスの種類が無効です") );
		break;
	case D3DERR_DRIVERINVALIDCALL:
		TVPAddLog( TJS_W("D3D : 使用されません") );
		break;
	case D3DERR_WASSTILLDRAWING:
		TVPAddLog( TJS_W("D3D : このサーフェスとの間で情報を転送している以前のビット演算が不完全です") );
		break;
	case D3DERR_DEVICEHUNG:
		TVPAddLog( TJS_W("D3D : このコードを返したデバイスが原因で、ハードウェア アダプターが OS によってリセットされました") );
		break;
	case D3DERR_UNSUPPORTEDOVERLAY:
		TVPAddLog( TJS_W("D3D : D3DERR_UNSUPPORTEDOVERLAY") );
		break;
	case D3DERR_UNSUPPORTEDOVERLAYFORMAT:
		TVPAddLog( TJS_W("D3D : D3DERR_UNSUPPORTEDOVERLAYFORMAT") );
		break;
	case D3DERR_CANNOTPROTECTCONTENT:
		TVPAddLog( TJS_W("D3D : D3DERR_CANNOTPROTECTCONTENT") );
		break;
	case D3DERR_UNSUPPORTEDCRYPTO:
		TVPAddLog( TJS_W("D3D : D3DERR_UNSUPPORTEDCRYPTO") );
		break;
	case D3DERR_PRESENT_STATISTICS_DISJOINT:
		TVPAddLog( TJS_W("D3D : D3DERR_PRESENT_STATISTICS_DISJOINT") );
		break;
	case D3DERR_DEVICEREMOVED:
		TVPAddLog( TJS_W("D3D : ハードウェア アダプターが削除されています") );
		break;
	case D3D_OK:
		break;
	case D3DOK_NOAUTOGEN:
		TVPAddLog( TJS_W("D3D : 成功しましたが、このフォーマットに対するミップマップの自動生成はサポートされていません") );
		break;
	case E_FAIL:
		TVPAddLog( TJS_W("D3D : Direct3D サブシステム内で原因不明のエラーが発生しました") );
		break;
	case E_INVALIDARG:
		TVPAddLog( TJS_W("D3D : 無効なパラメータが関数に渡されました") );
		break;
	default:
		TVPAddLog( TJS_W("D3D : Unknown Error") );
		break;
	}
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::AddLayerManager(iTVPLayerManager * manager)
{
	if(inherited::Managers.size() > 0)
	{
		// "Basic" デバイスでは２つ以上のLayer Managerを登録できない
		TVPThrowExceptionMessage(TVPPassThroughDeviceDoesNotSupporteLayerManagerMoreThanOne);
	}
	inherited::AddLayerManager(manager);

	manager->SetDesiredLayerType(ltOpaque); // ltOpaque な出力を受け取りたい
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::SetTargetWindow(HWND wnd, bool is_main)
{
	TVPInitBasicDrawDeviceOptions();
	DestroyD3DDevice();
	TargetWindow = wnd;
	IsMainWindow = is_main;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::SetDestRectangle(const tTVPRect & rect)
{
	// 位置だけの変更の場合かどうかをチェックする
	if(rect.get_width() == DestRect.get_width() && rect.get_height() == DestRect.get_height()) {
		// 位置だけの変更だ
		inherited::SetDestRectangle(rect);
	} else {
		// サイズも違う
		bool success = true;
		inherited::SetDestRectangle(rect);

		try {
			EnsureDevice();
		} catch(const eTJS & e) {
			TVPAddImportantLog( TVPFormatMessage(TVPPassthroughFailedToCreateDirect3DDevices,e.GetMessage() ) );
			success = false;
		} catch(...) {
			TVPAddImportantLog( (const tjs_char*)TVPPassthroughFailedToCreateDirect3DDevicesUnknownReason );
			success = false;
		}
		if( success == false ) {
			DestroyD3DDevice();
		}
	}
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::NotifyLayerResize(iTVPLayerManager * manager)
{
	inherited::NotifyLayerResize(manager);

	// テクスチャを捨てて作り直す。
	CreateTexture();
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::Show()
{
	if(!TargetWindow) return;
	if(!Texture) return;
	if(!Direct3DDevice) return;
	if(!ShouldShow) return;

	ShouldShow = false;

	HRESULT hr;
	if( DestRect.left != 0 || DestRect.top != 0 ) {
		hr = Direct3DDevice->Present( NULL, NULL, TargetWindow, NULL );
	} else {
		RECT drect;
		drect.left   = 0;
		drect.top    = 0;
		drect.right  = DestRect.right;
		drect.bottom = DestRect.bottom;

		RECT srect;
		srect.left   = 0;
		srect.top    = 0;
		srect.right  = DestRect.right;
		srect.bottom = DestRect.bottom;

		hr = Direct3DDevice->Present( &srect, &drect, TargetWindow, NULL );
	}

	if(hr == D3DERR_DEVICELOST) {
		if( IsTargetWindowActive() ) ErrorToLog( hr );
		TryRecreateWhenDeviceLost();
	} else if(hr != D3D_OK) {
		ErrorToLog( hr );
		TVPAddImportantLog( TVPFormatMessage(TVPPassthroughInfPrimarySurfaceDirect3DDevicePresentFailed,TJSInt32ToHex(hr, 8)) );
	}
}
//---------------------------------------------------------------------------
bool TJS_INTF_METHOD tTVPBasicDrawDevice::WaitForVBlank( tjs_int* in_vblank, tjs_int* delayed )
{
	if( Direct3DDevice == NULL ) return false;

	bool inVsync = false;
	D3DRASTER_STATUS rs;
	if( D3D_OK == Direct3DDevice->GetRasterStatus(0,&rs) ) {
		inVsync = rs.InVBlank == TRUE;
	}

	// VSync 待ちを行う
	bool isdelayed = false;
	if(!inVsync) {
		// vblank から抜けるまで待つ
		DWORD timeout_target_tick = ::timeGetTime() + 1;
		rs.InVBlank = FALSE;
		do {
			Direct3DDevice->GetRasterStatus(0,&rs);
		} while(rs.InVBlank == TRUE && (long)(::timeGetTime() - timeout_target_tick) <= 0);

		// vblank に入るまで待つ
		rs.InVBlank = TRUE;
		do {
			Direct3DDevice->GetRasterStatus(0,&rs);
		} while(rs.InVBlank == FALSE && (long)(::timeGetTime() - timeout_target_tick) <= 0);

		if((int)(::timeGetTime() - timeout_target_tick) > 0) {
			// フレームスキップが発生したと考えてよい
			isdelayed  = true;
		}
	}
	*delayed = isdelayed ? 1 : 0;
	*in_vblank = inVsync ? 1 : 0;
	return true;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::StartBitmapCompletion(iTVPLayerManager * manager)
{
	EnsureDevice();

	if( Texture && TargetWindow ) {
		if(TextureBuffer) {
			TVPAddImportantLog( (const tjs_char*)TVPPassthroughTextureHasAlreadyBeenLocked );
			Texture->UnlockRect(0), TextureBuffer = NULL;
		}

		D3DLOCKED_RECT rt;
		HRESULT hr = Texture->LockRect( 0, &rt, NULL, D3DLOCK_NO_DIRTY_UPDATE );

		if(hr == D3DERR_INVALIDCALL && IsTargetWindowActive() ) {
			TVPThrowExceptionMessage( TVPInternalErrorResult, TJSInt32ToHex(hr, 8));
		}

		if(hr != D3D_OK) {
			ErrorToLog( hr );
			TextureBuffer = NULL;
			TryRecreateWhenDeviceLost();
		} else /*if(hr == DD_OK) */ {
			TextureBuffer = rt.pBits;
			TexturePitch = rt.Pitch;
		}
	}
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::NotifyBitmapCompleted(iTVPLayerManager * manager,
	tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
	const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity)
{
	// bits, bitmapinfo で表されるビットマップの cliprect の領域を、x, y に描画
	// する。
	// opacity と type は無視するしかないので無視する
	tjs_int w, h;
	GetSrcSize( w, h );
	if( TextureBuffer && TargetWindow &&
		!(x < 0 || y < 0 ||
			x + cliprect.get_width() > w ||
			y + cliprect.get_height() > h) &&
		!(cliprect.left < 0 || cliprect.top < 0 ||
			cliprect.right > bitmapinfo->bmiHeader.biWidth ||
			cliprect.bottom > bitmapinfo->bmiHeader.biHeight))
	{
		// 範囲外の転送は(一部だけ転送するのではなくて)無視してよい
		ShouldShow = true;

		// bitmapinfo で表された cliprect の領域を x,y にコピーする
		long src_y       = cliprect.top;
		long src_y_limit = cliprect.bottom;
		long src_x       = cliprect.left;
		long width_bytes   = cliprect.get_width() * 4; // 32bit
		long dest_y      = y;
		long dest_x      = x;
		const tjs_uint8 * src_p = (const tjs_uint8 *)bits;
		long src_pitch;

		if(bitmapinfo->bmiHeader.biHeight < 0)
		{
			// bottom-down
			src_pitch = bitmapinfo->bmiHeader.biWidth * 4;
		}
		else
		{
			// bottom-up
			src_pitch = -bitmapinfo->bmiHeader.biWidth * 4;
			src_p += bitmapinfo->bmiHeader.biWidth * 4 * (bitmapinfo->bmiHeader.biHeight - 1);
		}

		for(; src_y < src_y_limit; src_y ++, dest_y ++)
		{
			const void *srcp = src_p + src_pitch * src_y + src_x * 4;
			void *destp = (tjs_uint8*)TextureBuffer + TexturePitch * dest_y + dest_x * 4;
			memcpy(destp, srcp, width_bytes);
		}
	}
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::EndBitmapCompletion(iTVPLayerManager * manager)
{
	if(!TargetWindow) return;
	if(!Texture) return;
	if(!Direct3DDevice) return;

	if(!TextureBuffer) return;
	Texture->UnlockRect(0);
	TextureBuffer = NULL;


	//- build vertex list
	struct tVertices
	{
		float x, y, z, rhw;
		float tu, tv;
	};
#if 0
	float dw = (float)DestWidth;
	float dh = (float)DestHeight;

	float sw = (float)SrcWidth  / (float)TextureWidth;
	float sh = (float)SrcHeight / (float)TextureHeight;

	tVertices vertices[] =
	{
		{0.0f - 0.5f, 0.0f - 0.5f, 1.0f, 1.0f, 0.0f, 0.0f},
		{dw   - 0.5f, 0.0f - 0.5f, 1.0f, 1.0f, sw  , 0.0f},
		{0.0f - 0.5f, dh   - 0.5f, 1.0f, 1.0f, 0.0f, sh  },
		{dw   - 0.5f, dh   - 0.5f, 1.0f, 1.0f, sw  , sh  }
	};
#else
	float dl = (float)DestRect.left;
	float dt = (float)DestRect.top;
	float dr = (float)DestRect.right;
	float db = (float)DestRect.bottom;

	tjs_int w, h;
	GetSrcSize( w, h );
	float sw = (float)w  / (float)TextureWidth;
	float sh = (float)h / (float)TextureHeight;

	tVertices vertices[] =
	{
		{dl - 0.5f, dt - 0.5f, 1.0f, 1.0f, 0.0f, 0.0f},
		{dr - 0.5f, dt - 0.5f, 1.0f, 1.0f, sw  , 0.0f},
		{dl - 0.5f, db - 0.5f, 1.0f, 1.0f, 0.0f, sh  },
		{dr - 0.5f, db - 0.5f, 1.0f, 1.0f, sw  , sh  }
	};
#endif

	HRESULT hr;

// フルスクリーン化後、1回は全体消去、それ以降はウィンドウの範囲内のみにした方が効率的。
	D3DVIEWPORT9 vp;
	vp.X  = 0;
	vp.Y  = 0;
	vp.Width = D3dPP.BackBufferWidth;
	vp.Height = D3dPP.BackBufferHeight;
	vp.MinZ  = 0.0f;
	vp.MaxZ  = 1.0f;
	Direct3DDevice->SetViewport(&vp);

	if( SUCCEEDED(Direct3DDevice->BeginScene()) ) {
		struct CAutoEndSceneCall {
			IDirect3DDevice9*	m_Device;
			CAutoEndSceneCall( IDirect3DDevice9* device ) : m_Device(device) {}
			~CAutoEndSceneCall() { m_Device->EndScene(); }
		} autoEnd(Direct3DDevice);

		Direct3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0, 0 );

		//- draw as triangles
		if( FAILED(hr = Direct3DDevice->SetTexture(0, Texture)) )
			goto got_error;

		if( FAILED( hr = Direct3DDevice->SetFVF( D3DFVF_XYZRHW|D3DFVF_TEX1 ) ) )
			goto got_error;

		if( FAILED( hr = Direct3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(tVertices) ) ) )
			goto got_error;

		if( FAILED( hr = Direct3DDevice->SetTexture( 0, NULL) ) )
			goto got_error;
	}

got_error:
	if( hr == D3DERR_DEVICELOST ) {
		TryRecreateWhenDeviceLost();
	} else if(hr == D3DERR_DEVICENOTRESET ) {
		hr = Direct3DDevice->Reset(&D3dPP);
		if( hr == D3DERR_DEVICELOST ) {
			TVPAddLog( (const tjs_char*)TVPErrorDeviceLostCannotResetDevice );
			TryRecreateWhenDeviceLost();
		} else if( hr == D3DERR_DRIVERINTERNALERROR ) {
			TVPAddLog( (const tjs_char*)TVPErrorDeviceInternalFatalError );
			TryRecreateWhenDeviceLost();
		} else if( hr == D3DERR_INVALIDCALL ) {
			TVPAddLog( (const tjs_char*)TVPErrorInvalidCall );
			TryRecreateWhenDeviceLost();
		} else if( hr  == D3DERR_OUTOFVIDEOMEMORY ) {
			TVPAddLog( (const tjs_char*)TVPErrorCannotAllocateVideoMemory );
			TryRecreateWhenDeviceLost();
		} else if( hr == E_OUTOFMEMORY  ) {
			TVPAddLog( (const tjs_char*)TVPErrorCannotAllocateMemory );
			TryRecreateWhenDeviceLost();
		} else if( hr == D3D_OK ) {
			if( FAILED( hr = InitializeDirect3DState() ) ) {
				if( IsTargetWindowActive() ) {
					ErrorToLog( hr );
					TVPThrowExceptionMessage( TVPFaildToSetRenderState );
				} else {
					DestroyD3DDevice();
				}
			}
		}
	} else if(hr != D3D_OK) {
		ErrorToLog( hr );
		TVPAddImportantLog( TVPFormatMessage(TVPPassthroughInfPolygonDrawingFailed,TJSInt32ToHex(hr, 8)) );
	}
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::SetShowUpdateRect(bool b)
{
	DrawUpdateRectangle = b;
}
//---------------------------------------------------------------------------
bool TJS_INTF_METHOD tTVPBasicDrawDevice::SwitchToFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color, bool changeresolution )
{
	// フルスクリーン化の処理はなにも行わない、互換性のためにウィンドウを全画面化するのみで処理する
	// Direct3D9 でフルスクリーン化するとフォーカスを失うとデバイスをロストするので、そのたびにリセットor作り直しが必要になる。
	// モーダルウィンドウを使用するシステムでは、これは困るので常にウィンドウモードで行う。
	// モーダルウィンドウを使用しないシステムにするのなら、フルスクリーンを使用するDrawDeviceを作ると良い。
	ShouldShow = true;
	CheckMonitorMoved();
	return true;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::RevertFromFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color )
{
	ShouldShow = true;
	CheckMonitorMoved();
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// tTJSNI_BasicDrawDevice : BasicDrawDevice TJS native class
//---------------------------------------------------------------------------
tjs_uint32 tTJSNC_BasicDrawDevice::ClassID = (tjs_uint32)-1;
tTJSNC_BasicDrawDevice::tTJSNC_BasicDrawDevice() :
	tTJSNativeClass(TJS_W("BasicDrawDevice"))
{
	// register native methods/properties

	TJS_BEGIN_NATIVE_MEMBERS(BasicDrawDevice)
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
// constructor/methods
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_BasicDrawDevice,
	/*TJS class name*/BasicDrawDevice)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/BasicDrawDevice)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/recreate)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_BasicDrawDevice);
	_this->GetDevice()->SetToRecreateDrawer();
	_this->GetDevice()->EnsureDevice();
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/recreate)
//----------------------------------------------------------------------


//---------------------------------------------------------------------------
//----------------------------------------------------------------------
// properties
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(interface)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_BasicDrawDevice);
		*result = reinterpret_cast<tjs_int64>(_this->GetDevice());
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(interface)
//----------------------------------------------------------------------
	TJS_END_NATIVE_MEMBERS
}
//---------------------------------------------------------------------------
iTJSNativeInstance *tTJSNC_BasicDrawDevice::CreateNativeInstance()
{
	return new tTJSNI_BasicDrawDevice();
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
tTJSNI_BasicDrawDevice::tTJSNI_BasicDrawDevice()
{
	Device = new tTVPBasicDrawDevice();
}
//---------------------------------------------------------------------------
tTJSNI_BasicDrawDevice::~tTJSNI_BasicDrawDevice()
{
	if(Device) Device->Destruct(), Device = NULL;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD
	tTJSNI_BasicDrawDevice::Construct(tjs_int numparams, tTJSVariant **param,
		iTJSDispatch2 *tjs_obj)
{
	return TJS_S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_BasicDrawDevice::Invalidate()
{
	if(Device) Device->Destruct(), Device = NULL;
}
//---------------------------------------------------------------------------

