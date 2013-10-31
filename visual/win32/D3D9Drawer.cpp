#define NOMINMAX
#include "tjsCommHead.h"
#include "D3D9Drawer.h"
#include "ComplexRect.h"
#include "MsgIntf.h"
#include "DebugIntf.h"
#include "PassThroughDrawDevice.h"
#include "EventImpl.h"
#include "WindowImpl.h"

#include <d3d9.h>

extern bool TVPZoomInterpolation;
//---------------------------------------------------------------------------
//! @brief	Direct3D9 によるダブルバッファリングを行うクラス
//! @note	tTVPDrawer_DDDoubleBuffering とよく似ているが別クラスになっている。
//!			修正を行う場合は、互いによく見比べ、似たようなところがあればともに修正を試みること。
//---------------------------------------------------------------------------
//! @brief	コンストラクタ
tTVPDrawer_D3DDoubleBuffering::tTVPDrawer_D3DDoubleBuffering(tTVPPassThroughDrawDevice * device) : tTVPDrawer(device)
{
	Direct3D = NULL;
	Direct3DDevice = NULL;
	//Surface = NULL;
	Texture = NULL;
	//RenderTarget = NULL;
	ShouldShow = false;
	TextureBuffer = NULL;
	TextureWidth = TextureHeight = 0;
	VsyncInterval = 16;
}

//! @brief	デストラクタ
tTVPDrawer_D3DDoubleBuffering::~tTVPDrawer_D3DDoubleBuffering()
{
	DestroyOffScreenSurface();
}

ttstr tTVPDrawer_D3DDoubleBuffering::GetName() { return TJS_W("Direct3D9 double buffering"); }

void tTVPDrawer_D3DDoubleBuffering::DestroyOffScreenSurface()
{
	if(TextureBuffer && Texture) Texture->UnlockRect(0), TextureBuffer = NULL;
	if(Texture) Texture->Release(), Texture = NULL;
	//if(RenderTarget) RenderTarget->Release(), RenderTarget = NULL;
	if(Direct3DDevice) Direct3DDevice->Release(), Direct3DDevice = NULL;
	if(Direct3D) Direct3D = NULL;
}

void tTVPDrawer_D3DDoubleBuffering::InvalidateAll()
{
	// レイヤ演算結果をすべてリクエストする
	// サーフェースが lost した際に内容を再構築する目的で用いる
	Device->RequestInvalidation(tTVPRect(0, 0, DestWidth, DestHeight));
}

void tTVPDrawer_D3DDoubleBuffering::GetDirect3D9Device() {
	DestroyOffScreenSurface();

	TVPEnsureDirect3DObject();

	if( NULL == ( Direct3D = TVPGetDirect3DObjectNoAddRef() ) )
		TVPThrowExceptionMessage( TVPFaildToCreateDirect3D );

	HRESULT hr;
	D3DPRESENT_PARAMETERS	d3dpp;
	if( FAILED( hr = DecideD3DPresentParameters( d3dpp ) ) )
		TVPThrowExceptionMessage( TVPFaildToDecideBackbufferFormat );

	UINT iCurrentMonitor = GetMonitorNumber( TargetWindow );
	DWORD	BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED;
	if( D3D_OK != ( hr = Direct3D->CreateDevice( iCurrentMonitor, D3DDEVTYPE_HAL, NULL, BehaviorFlags, &d3dpp, &Direct3DDevice ) ) )
		TVPThrowExceptionMessage( TVPFaildToCreateDirect3DDevice );

	D3DVIEWPORT9 vp;
	vp.X  = 0;
	vp.Y  = 0;
	vp.Width = d3dpp.BackBufferWidth;
	vp.Height = d3dpp.BackBufferHeight;
	vp.MinZ  = 0.0f;
	vp.MaxZ  = 1.0f;
	if( FAILED(hr = Direct3DDevice->SetViewport(&vp)) )
		TVPThrowExceptionMessage( TVPFaildToSetViewport );
	/*
	if( RenderTarget ) RenderTarget->Release(), RenderTarget = NULL;
	if( FAILED( hr = Direct3DDevice->GetRenderTarget( 0, &RenderTarget ) ) )
		TVPThrowExceptionMessage(TJS_W("Faild to get render target."));
	*/

	if( FAILED( hr = InitializeDirect3DState() ) )
 		TVPThrowExceptionMessage( TVPFaildToSetRenderState );

	int refreshrate;
	HDC hdc;

	hdc = ::GetDC(TargetWindow);
	refreshrate = GetDeviceCaps( hdc, VREFRESH );
	::ReleaseDC( TargetWindow, hdc );
	VsyncInterval = 1000 / refreshrate;
}
HRESULT tTVPDrawer_D3DDoubleBuffering::InitializeDirect3DState() {
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
UINT tTVPDrawer_D3DDoubleBuffering::GetMonitorNumber( HWND window )
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
HRESULT tTVPDrawer_D3DDoubleBuffering::DecideD3DPresentParameters( D3DPRESENT_PARAMETERS& d3dpp ) {
	HRESULT			hr;
	D3DDISPLAYMODE	dm;
	UINT iCurrentMonitor = GetMonitorNumber(TargetWindow);
	if( FAILED( hr = Direct3D->GetAdapterDisplayMode( iCurrentMonitor, &dm ) ) )
		return hr;

	UINT	width = dm.Width;
	UINT	height = dm.Height;
	RECT	clientRect;
	if( ::GetClientRect( TargetWindow, &clientRect ) ) {
		width = clientRect.right - clientRect.left;
		height = clientRect.bottom - clientRect.top;
	}

	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
	d3dpp.BackBufferFormat = dm.Format;
	d3dpp.BackBufferHeight = height;
	d3dpp.BackBufferWidth = width;
	d3dpp.hDeviceWindow = TargetWindow;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	return S_OK;
}

//---------------------------------------------------------------------------
HRESULT tTVPDrawer_D3DDoubleBuffering::DecideD3DPresentParameters( D3DPRESENT_PARAMETERS& d3dpp, HWND window, bool iswindow, tjs_uint width, tjs_uint height, tjs_uint bpp, tjs_uint color )
{
	if( Direct3D == NULL ) return false;

	HRESULT			hr;
	D3DDISPLAYMODE	dm;
	tjs_uint iCurrentMonitor = GetMonitorNumber(window);
	if( FAILED( hr = Direct3D->GetAdapterDisplayMode( iCurrentMonitor, &dm ) ) )
		return hr;

	if( width == 0 || height == 0 ) {
		width = dm.Width;
		height = dm.Height;
		RECT clientRect;
		if( ::GetClientRect( window, &clientRect ) ) {
			width = clientRect.right - clientRect.left;
			height = clientRect.bottom - clientRect.top;
		}
	}

	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed = iswindow ? TRUE : FALSE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
	if( iswindow ) {
		//d3dpp.BackBufferFormat = dm.Format;
		d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	} else {
		if( bpp == 16 ) {
			if( dm.Format == D3DFMT_X1R5G5B5 || dm.Format == D3DFMT_R5G6B5 ) {
				d3dpp.BackBufferFormat = dm.Format;
			} else if( color == 565 ) {
				d3dpp.BackBufferFormat = D3DFMT_R5G6B5;
			} else {
				d3dpp.BackBufferFormat = D3DFMT_X1R5G5B5;
			}
		} else {
			d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		}
	}
	d3dpp.BackBufferHeight = height;
	d3dpp.BackBufferWidth = width;
	d3dpp.hDeviceWindow = window;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT tTVPDrawer_D3DDoubleBuffering::DecideD3DPresentParametersForFullScreen( D3DPRESENT_PARAMETERS& d3dpp, HWND window )
{
	if( Direct3D == NULL ) return false;

	HRESULT			hr;
	D3DDISPLAYMODE	dm;
	tjs_uint iCurrentMonitor = GetMonitorNumber(window);
	if( FAILED( hr = Direct3D->GetAdapterDisplayMode( iCurrentMonitor, &dm ) ) )
		return hr;

	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
	d3dpp.BackBufferFormat = dm.Format;
	d3dpp.BackBufferHeight = dm.Width;
	d3dpp.BackBufferWidth = dm.Height;
	d3dpp.hDeviceWindow = window;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	return S_OK;
}
void tTVPDrawer_D3DDoubleBuffering::CreateOffScreenSurface()
{
	// Direct3D デバイス、テクスチャなどを作成する
	DestroyOffScreenSurface();
	if(TargetWindow && SrcWidth > 0 && SrcHeight > 0)
	{
		HRESULT hr = S_OK;

		// get Direct3D9 interface
		GetDirect3D9Device();

		D3DCAPS9	d3dcaps;
		Direct3DDevice->GetDeviceCaps( &d3dcaps );

		TextureWidth = SrcWidth;
		TextureHeight = SrcHeight;
		if( d3dcaps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) {
			// only square textures are supported
			TextureWidth = std::max(TextureHeight, TextureWidth);
			TextureHeight = TextureWidth;
		}
		DWORD		dwWidth = 64;
		DWORD		dwHeight = 64;
		if( d3dcaps.TextureCaps & D3DPTEXTURECAPS_POW2 ) {
			// 2の累乗のみ許可するかどうか判定
			while( dwWidth < TextureWidth ) dwWidth = dwWidth << 1;
			while( dwHeight < TextureHeight ) dwHeight = dwHeight << 1;

			if( dwWidth > d3dcaps.MaxTextureWidth || dwHeight > d3dcaps.MaxTextureHeight ) {
				TVPAddLog( (const tjs_char*)TVPWarningImageSizeTooLargeMayBeCannotCreateTexture );
			}

			TVPAddLog( (const tjs_char*)TVPUsePowerOfTwoSurface );
		} else {
			dwWidth = TextureWidth;
			dwHeight = TextureHeight;
		}

		//if( D3D_OK != ( hr = Direct3DDevice->CreateTexture( dwWidth, dwHeight, 1, D3DUSAGE_DYNAMIC, D3DFMT_X8B8G8R8, D3DPOOL_DEFAULT, &Texture, NULL) ) )
		if( D3D_OK != ( hr = Direct3DDevice->CreateTexture( dwWidth, dwHeight, 1, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &Texture, NULL) ) )
			TVPThrowExceptionMessage(TVPCannotAllocateD3DOffScreenSurface,TJSInt32ToHex(hr, 8));
		//Texture->GetSurfaceLevel(0, &Surface);
		//Texture->GetLevelDesc(0, &SurfaceDesc);
	}
}

bool tTVPDrawer_D3DDoubleBuffering::SetDestSize(tjs_int width, tjs_int height)
{
	if(inherited::SetDestSize(width, height))
	{
		try
		{
			CreateOffScreenSurface();
		}
		catch(const eTJS & e)
		{
			TVPAddImportantLog( TVPFormatMessage(TVPPassthroughFailedToCreateDirect3DDevices,e.GetMessage() ) );
			return false;
		}
		catch(...)
		{
			TVPAddImportantLog( (const tjs_char*)TVPPassthroughFailedToCreateDirect3DDevicesUnknownReason );
			return false;
		}
		return true;
	}
	return false;
}

bool tTVPDrawer_D3DDoubleBuffering::NotifyLayerResize(tjs_int w, tjs_int h)
{
	if(inherited::NotifyLayerResize(w, h))
	{
		try
		{
			CreateOffScreenSurface();
		}
		catch(const eTJS & e)
		{
			TVPAddImportantLog( TVPFormatMessage(TVPPassthroughFailedToCreateDirect3DDevices,e.GetMessage() ) );
			return false;
		}
		catch(...)
		{
			TVPAddImportantLog( (const tjs_char*)TVPPassthroughFailedToCreateDirect3DDevicesUnknownReason );
			return false;
		}
		return true;
	}
	return false;
}

bool tTVPDrawer_D3DDoubleBuffering::SetDestSizeAndNotifyLayerResize(tjs_int width, tjs_int height, tjs_int w, tjs_int h)
{
	if(inherited::SetDestSize(width, height) && inherited::NotifyLayerResize(w, h))
	{
		try
		{
			CreateOffScreenSurface();
		}
		catch(const eTJS & e)
		{
			TVPAddImportantLog( TVPFormatMessage(TVPPassthroughFailedToCreateDirect3DDevices,e.GetMessage() ) );
			return false;
		}
		catch(...)
		{
			TVPAddImportantLog( (const tjs_char*)TVPPassthroughFailedToCreateDirect3DDevicesUnknownReason );
			return false;
		}
		return true;
	}
	return false;
}

void tTVPDrawer_D3DDoubleBuffering::SetTargetWindow(HWND wnd)
{
	inherited::SetTargetWindow(wnd);
	CreateOffScreenSurface();
}
//#define TVPD3DTIMING
#ifdef TVPD3DTIMING
void tTVPDrawer_D3DDoubleBuffering::InitTimings()
{
	GetDCTime = 0;
	DrawDibDrawTime = 0;
	ReleaseDCTime = 0;
	DrawPrimitiveTime = 0;
	BltTime = 0;
}

void tTVPDrawer_D3DDoubleBuffering::ReportTimings()
{
	TVPAddLog(TJS_W("GetDC / Lock : ") + ttstr((int)GetDCTime));
	TVPAddLog(TJS_W("DrawDibDraw / memcpy : ") + ttstr((int)DrawDibDrawTime));
	TVPAddLog(TJS_W("ReleaseDC / Unlock : ") + ttstr((int)ReleaseDCTime));
	TVPAddLog(TJS_W("DrawPrimitive : ") + ttstr((int)DrawPrimitiveTime));
	TVPAddLog(TJS_W("Blt : ") + ttstr((int)BltTime));
}
#endif

void tTVPDrawer_D3DDoubleBuffering::StartBitmapCompletion()
{
	// retrieve DC
	if(Texture && TargetWindow)
	{
#ifdef TVPD3DTIMING
StartTick = timeGetTime();
#endif
		if(TextureBuffer)
		{
			TVPAddImportantLog( (const tjs_char*)TVPPassthroughTextureHasAlreadyBeenLocked );
			Texture->UnlockRect(0), TextureBuffer = NULL;
		}

		D3DLOCKED_RECT rt;
		HRESULT hr = Texture->LockRect( 0, &rt, NULL, D3DLOCK_NO_DIRTY_UPDATE );

		if(hr == D3DERR_INVALIDCALL ) {
			TVPThrowExceptionMessage( TVPInternalErrorResult, TJSInt32ToHex(hr, 8));
		}

		if(hr != D3D_OK) {
			TextureBuffer = NULL;
			InvalidateAll();  // causes reconstruction of off-screen image
		} else /*if(hr == DD_OK) */ {
			TextureBuffer = rt.pBits;
			TexturePitch = rt.Pitch;
		}

#ifdef TVPD3DTIMING
GetDCTime += timeGetTime() - StartTick;
#endif
	}
}

void tTVPDrawer_D3DDoubleBuffering::NotifyBitmapCompleted(tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
	const tTVPRect &cliprect)
{
	// 直接メモリ転送を用いて描画を行う
#ifdef TVPD3DTIMING
StartTick = timeGetTime();
#endif
	if(DrawDibHandle && TextureBuffer && TargetWindow &&
		!(x < 0 || y < 0 ||
			x + cliprect.get_width() > SrcWidth ||
			y + cliprect.get_height() > SrcHeight) &&
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
#ifdef TVPD3DTIMING
DrawDibDrawTime += timeGetTime() - StartTick;
#endif
}

void tTVPDrawer_D3DDoubleBuffering::EndBitmapCompletion()
{
	if(!TargetWindow) return;
	if(!Texture) return;
	//if(!Surface) return;
	if(!Direct3DDevice) return;

#ifdef TVPD3DTIMING
StartTick = timeGetTime();
#endif
	if(!TextureBuffer) return;
	Texture->UnlockRect(0);
	TextureBuffer = NULL;
#ifdef TVPD3DTIMING
ReleaseDCTime += timeGetTime() - StartTick;
#endif

	// Blt to the primary surface

	// Blt texture to surface

	//- build vertex list
	struct tVertices
	{
		float x, y, z, rhw;
		float tu, tv;
	};

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

	HRESULT hr;

#ifdef TVPD3DTIMING
StartTick = timeGetTime();
#endif
	//if( FAILED(hr = Direct3DDevice->SetRenderTarget( 0, RenderTarget )) )
	//	goto got_error;

	if( SUCCEEDED(Direct3DDevice->BeginScene()) ) {
		struct CAutoEndSceneCall {
			IDirect3DDevice9*	m_Device;
			CAutoEndSceneCall( IDirect3DDevice9* device ) : m_Device(device) {}
			~CAutoEndSceneCall() { m_Device->EndScene(); }
		} autoEnd(Direct3DDevice);

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

#ifdef TVPD3DTIMING
DrawPrimitiveTime += timeGetTime() - StartTick;
#endif
got_error:
	if( hr == D3DERR_DEVICELOST ) {
		InvalidateAll();  // causes reconstruction of off-screen image
	} else if(hr == D3DERR_DEVICENOTRESET ) {
		D3DPRESENT_PARAMETERS	d3dpp;
		if( FAILED( hr = DecideD3DPresentParameters( d3dpp ) ) ) {
			InvalidateAll();  // causes reconstruction of off-screen image
		} else {
			hr = Direct3DDevice->Reset(&d3dpp);
			if( hr == D3DERR_DEVICELOST ) {
				TVPAddLog( (const tjs_char*)TVPErrorDeviceLostCannotResetDevice );
				InvalidateAll();
			} else if( hr == D3DERR_DRIVERINTERNALERROR ) {
				TVPAddLog( (const tjs_char*)TVPErrorDeviceInternalFatalError );
				InvalidateAll();
			} else if( hr == D3DERR_INVALIDCALL ) {
				TVPAddLog( (const tjs_char*)TVPErrorInvalidCall );
				InvalidateAll();
			} else if( hr  == D3DERR_OUTOFVIDEOMEMORY ) {
				TVPAddLog( (const tjs_char*)TVPErrorCannotAllocateVideoMemory );
				InvalidateAll();
			} else if( hr == E_OUTOFMEMORY  ) {
				TVPAddLog( (const tjs_char*)TVPErrorCannotAllocateMemory );
				InvalidateAll();
			}
		}
	} else if(hr != D3D_OK) {
		TVPAddImportantLog( TVPFormatMessage(TVPPassthroughInfPolygonDrawingFailed,TJSInt32ToHex(hr, 8)) );
	}
}


void tTVPDrawer_D3DDoubleBuffering::Show()
{
	if(!TargetWindow) return;
	if(!Texture) return;
	//if(!Surface) return;
	if(!Direct3DDevice) return;
	if(!ShouldShow) return;

	ShouldShow = false;

	HRESULT hr;


#ifdef TVPD3DTIMING
StartTick = timeGetTime();
#endif
	// get PaintBox's origin
	POINT origin; origin.x = DestLeft, origin.y = DestTop;
	::ClientToScreen(TargetWindow, &origin);

	// entire of the bitmap is to be transfered (this is not optimal. FIX ME!)
	RECT drect;
	/*
	drect.left   = origin.x;
	drect.top    = origin.y;
	drect.right  = origin.x + DestWidth;
	drect.bottom = origin.y + DestHeight;
	*/
	drect.left   = 0;
	drect.top    = 0;
	drect.right  = DestWidth;
	drect.bottom = DestHeight;

	RECT srect;
	srect.left   = 0;
	srect.top    = 0;
	srect.right  = DestWidth;
	srect.bottom = DestHeight;

	hr = Direct3DDevice->Present( &srect, &drect, TargetWindow, NULL );

#ifdef TVPD3DTIMING
BltTime += timeGetTime() - StartTick;
#endif

//	got_error:
	if(hr == D3DERR_DEVICELOST) {
		InvalidateAll();  // causes reconstruction of off-screen image
	} else if(hr != D3D_OK) {
		TVPAddImportantLog( TVPFormatMessage(TVPPassthroughInfPrimarySurfaceDirect3DDevicePresentFailed,TJSInt32ToHex(hr, 8)) );
	}
}

bool tTVPDrawer_D3DDoubleBuffering::WaitForVBlank( tjs_int* in_vblank, tjs_int* delayed )
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
int tTVPDrawer_D3DDoubleBuffering::GetInterpolationCapability() {
	// bit 0 for point-on-point, bit 1 for bilinear interpolation
	GetDirect3D9Device();
	if( Direct3DDevice ) {
		D3DCAPS9 desc;
		ZeroMemory(&desc, sizeof(desc));
		if( SUCCEEDED(Direct3DDevice->GetDeviceCaps(&desc)) ) {
			int caps = 0;
			if(desc.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR)
				caps += 2;
			if(desc.TextureFilterCaps & D3DPTFILTERCAPS_MAGFPOINT)
				caps += 1;
			return caps;
		}
		return 3;
	} else {
		return 3;
	}
}
//---------------------------------------------------------------------------
bool tTVPDrawer_D3DDoubleBuffering::SwitchToFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color, bool changeresolution )
{
	bool success = false;
	HRESULT hr;
	D3DPRESENT_PARAMETERS d3dpp = {0};
	if( changeresolution ) {
		hr = DecideD3DPresentParametersForFullScreen( d3dpp, window );
	} else {
		hr = DecideD3DPresentParameters( d3dpp, window, false, w, h, bpp, color );
	}

	if(FAILED(hr))
	{
		TVPAddLog(TJS_W("TVPDecideD3DPresentParameters failed/hr=") + TJSInt32ToHex(hr) );
	}
	else
	{
		hr = Direct3DDevice->Reset(&d3dpp);
		if(FAILED(hr))
		{
			TVPAddLog(
				ttstr(TJS_W("IDirect3DDevice::Reset failed/hr=")) + TJSInt32ToHex(hr));
		}
		else
		{
			success = true;
		}
	}
	return success;
}
//---------------------------------------------------------------------------
void tTVPDrawer_D3DDoubleBuffering::RevertFromFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color )
{
	D3DPRESENT_PARAMETERS	d3dpp = {0};
	HRESULT hr = DecideD3DPresentParameters( d3dpp, window, true, w, h, bpp );
	if( SUCCEEDED(hr) )
	{
		if( Direct3DDevice ) hr = Direct3DDevice->Reset(&d3dpp);
		else hr = E_FAIL;
	}
	if( FAILED(hr) ) ::ChangeDisplaySettings(NULL, 0);
}
//---------------------------------------------------------------------------
