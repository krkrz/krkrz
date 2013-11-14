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
//#include <dxerr9.h>

extern bool TVPZoomInterpolation;
//---------------------------------------------------------------------------
//! @brief	Direct3D9 によるダブルバッファリングを行うクラス
//! @note	tTVPDrawer_DDDoubleBuffering とよく似ているが別クラスになっている。
//!			修正を行う場合は、互いによく見比べ、似たようなところがあればともに修正を試みること。
//---------------------------------------------------------------------------
//! @brief	コンストラクタ
tTVPDrawer_D3DDoubleBuffering::tTVPDrawer_D3DDoubleBuffering(tTVPPassThroughDrawDevice * device)
 : tTVPDrawer(device), IsInitd3dpp( false )
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
	ZeroMemory( &D3dPP, sizeof(D3dPP) );
}

//---------------------------------------------------------------------------
//! @brief	デストラクタ
tTVPDrawer_D3DDoubleBuffering::~tTVPDrawer_D3DDoubleBuffering()
{
	DestroyOffScreenSurface();
}
//---------------------------------------------------------------------------
ttstr tTVPDrawer_D3DDoubleBuffering::GetName() {
	return TJS_W("Direct3D9 double buffering");
}
//---------------------------------------------------------------------------
void tTVPDrawer_D3DDoubleBuffering::DestroyOffScreenSurface() {
	DestroyTexture();
	if(Direct3DDevice) Direct3DDevice->Release(), Direct3DDevice = NULL;
	if(Direct3D) Direct3D = NULL;
}
//---------------------------------------------------------------------------
void tTVPDrawer_D3DDoubleBuffering::DestroyTexture() {
	if(TextureBuffer && Texture) Texture->UnlockRect(0), TextureBuffer = NULL;
	if(Texture) Texture->Release(), Texture = NULL;
	//if(RenderTarget) RenderTarget->Release(), RenderTarget = NULL;
}
//---------------------------------------------------------------------------
void tTVPDrawer_D3DDoubleBuffering::InvalidateAll()
{
	// レイヤ演算結果をすべてリクエストする
	// サーフェースが lost した際に内容を再構築する目的で用いる
	Device->RequestInvalidation(tTVPRect(0, 0, DestWidth, DestHeight));
}
//---------------------------------------------------------------------------
void tTVPDrawer_D3DDoubleBuffering::GetDirect3D9Device() {
	DestroyOffScreenSurface();

	TVPEnsureDirect3DObject();

	if( NULL == ( Direct3D = TVPGetDirect3DObjectNoAddRef() ) )
		TVPThrowExceptionMessage( TVPFaildToCreateDirect3D );

	HRESULT hr;
	if( IsInitd3dpp == false ) {
		if( FAILED( hr = DecideD3DPresentParameters() ) ) {
			ErrorToLog( hr );
			TVPThrowExceptionMessage( TVPFaildToDecideBackbufferFormat );
		}
		IsInitd3dpp = true;
	}

	UINT iCurrentMonitor = GetMonitorNumber( TargetWindow );
	DWORD	BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED;
	if( D3D_OK != ( hr = Direct3D->CreateDevice( iCurrentMonitor, D3DDEVTYPE_HAL, TargetWindow, BehaviorFlags, &D3dPP, &Direct3DDevice ) ) ) {
		ErrorToLog( hr );
		TVPThrowExceptionMessage( TVPFaildToCreateDirect3DDevice );
	}
	CurrentMonitor = iCurrentMonitor;

	D3DVIEWPORT9 vp;
	vp.X  = DestLeft;
	vp.Y  = DestTop;
	vp.Width = DestWidth != 0 ? DestWidth : D3dPP.BackBufferWidth;
	vp.Height = DestHeight != 0 ? DestHeight : D3dPP.BackBufferHeight;
	vp.MinZ  = 0.0f;
	vp.MaxZ  = 1.0f;
	if( FAILED(hr = Direct3DDevice->SetViewport(&vp)) ) {
		ErrorToLog( hr );
		TVPThrowExceptionMessage( TVPFaildToSetViewport );
	}
	/*
	if( RenderTarget ) RenderTarget->Release(), RenderTarget = NULL;
	if( FAILED( hr = Direct3DDevice->GetRenderTarget( 0, &RenderTarget ) ) )
		TVPThrowExceptionMessage(TJS_W("Faild to get render target."));
	*/

	if( FAILED( hr = InitializeDirect3DState() ) ) {
		ErrorToLog( hr );
 		TVPThrowExceptionMessage( TVPFaildToSetRenderState );
	}

	int refreshrate;
	HDC hdc;

	hdc = ::GetDC(TargetWindow);
	refreshrate = GetDeviceCaps( hdc, VREFRESH );
	::ReleaseDC( TargetWindow, hdc );
	VsyncInterval = 1000 / refreshrate;
}
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
HRESULT tTVPDrawer_D3DDoubleBuffering::DecideD3DPresentParameters() {
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

	ZeroMemory( &D3dPP, sizeof(D3dPP) );
	D3dPP.Windowed = TRUE;
	D3dPP.SwapEffect = D3DSWAPEFFECT_COPY;
	D3dPP.BackBufferFormat = D3DFMT_UNKNOWN;
	D3dPP.BackBufferHeight = height;
	D3dPP.BackBufferWidth = width;
	D3dPP.hDeviceWindow = TargetWindow;
	D3dPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT tTVPDrawer_D3DDoubleBuffering::DecideD3DPresentParameters( UINT monitor, HWND window, bool iswindow, tjs_uint width, tjs_uint height, tjs_uint bpp, tjs_uint color, bool changeresolution )
{
	if( Direct3D == NULL ) return false;

	HRESULT			hr;
	D3DDISPLAYMODE	dm;
	if( FAILED( hr = Direct3D->GetAdapterDisplayMode( monitor, &dm ) ) )
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

	// 解像度変更なしで現在の幅と高さbppが一致するのならwindowモードとする
	if( iswindow == false && changeresolution == false ) {
		if( dm.Width == width && dm.Height == height ) {
			if( bpp == 16 ) {
				if( dm.Format == D3DFMT_X1R5G5B5 || dm.Format == D3DFMT_R5G6B5 ) {
					iswindow = true;
				}
			} else if( dm.Format == D3DFMT_X8R8G8B8 ) {
				iswindow = true;
			}
		}
	}

	ZeroMemory( &D3dPP, sizeof(D3dPP) );
	D3dPP.Windowed = iswindow ? TRUE : FALSE;
	D3dPP.SwapEffect = D3DSWAPEFFECT_COPY;
	D3dPP.BackBufferHeight = height;
	D3dPP.BackBufferWidth = width;
	D3dPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	D3dPP.hDeviceWindow = window;
	if( iswindow ) {
		D3dPP.BackBufferFormat = D3DFMT_UNKNOWN;
	} else {
		if( bpp == 16 ) {
			if( dm.Format == D3DFMT_X1R5G5B5 || dm.Format == D3DFMT_R5G6B5 ) {
				D3dPP.BackBufferFormat = dm.Format;
			} else if( color == 565 ) {
				D3dPP.BackBufferFormat = D3DFMT_R5G6B5;
			} else {
				D3dPP.BackBufferFormat = D3DFMT_X1R5G5B5;
			}
		} else {
			D3dPP.BackBufferFormat = D3DFMT_X8R8G8B8;
		}
		D3dPP.FullScreen_RefreshRateInHz = dm.RefreshRate;
		//D3dPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
		//D3dPP.BackBufferCount = 1;
		//D3dPP.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	}

	return S_OK;
}
//---------------------------------------------------------------------------
void tTVPDrawer_D3DDoubleBuffering::CreateOffScreenSurface()
{
	// Direct3D デバイス、テクスチャなどを作成する
	DestroyOffScreenSurface();
	if(TargetWindow && SrcWidth > 0 && SrcHeight > 0) {
		// get Direct3D9 interface
		GetDirect3D9Device();

		CreateTexture();
	}
}
//---------------------------------------------------------------------------
void tTVPDrawer_D3DDoubleBuffering::CreateTexture() {
	DestroyTexture();
	if(TargetWindow && SrcWidth > 0 && SrcHeight > 0) {
		HRESULT hr = S_OK;

		D3DCAPS9 d3dcaps;
		Direct3DDevice->GetDeviceCaps( &d3dcaps );

		TextureWidth = SrcWidth;
		TextureHeight = SrcHeight;
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
			ErrorToLog( hr );
			TVPThrowExceptionMessage(TVPCannotAllocateD3DOffScreenSurface,TJSInt32ToHex(hr, 8));
		}
	}
}
//---------------------------------------------------------------------------
bool tTVPDrawer_D3DDoubleBuffering::SetDestPos(tjs_int left, tjs_int top)
{
	if(inherited::SetDestPos(left, top))
	{
		if( Direct3DDevice ) {
			D3DVIEWPORT9 vp;
			vp.X  = DestLeft;
			vp.Y  = DestTop;
			vp.Width = DestWidth != 0 ? DestWidth : D3dPP.BackBufferWidth;
			vp.Height = DestHeight != 0 ? DestHeight : D3dPP.BackBufferHeight;
			vp.MinZ  = 0.0f;
			vp.MaxZ  = 1.0f;
			Direct3DDevice->SetViewport(&vp);
		}
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
bool tTVPDrawer_D3DDoubleBuffering::SetDestSize(tjs_int width, tjs_int height)
{
	if(inherited::SetDestSize(width, height))
	{
		try
		{
			CreateOffScreenSurface();

			D3DVIEWPORT9 vp;
			vp.X  = DestLeft;
			vp.Y  = DestTop;
			vp.Width = DestWidth != 0 ? DestWidth : D3dPP.BackBufferWidth;
			vp.Height = DestHeight != 0 ? DestHeight : D3dPP.BackBufferHeight;
			vp.MinZ  = 0.0f;
			vp.MaxZ  = 1.0f;
			Direct3DDevice->SetViewport(&vp);
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
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
void tTVPDrawer_D3DDoubleBuffering::SetTargetWindow(HWND wnd)
{
	inherited::SetTargetWindow(wnd);
	CreateOffScreenSurface();
}
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
void tTVPDrawer_D3DDoubleBuffering::ReportTimings()
{
	TVPAddLog(TJS_W("GetDC / Lock : ") + ttstr((int)GetDCTime));
	TVPAddLog(TJS_W("DrawDibDraw / memcpy : ") + ttstr((int)DrawDibDrawTime));
	TVPAddLog(TJS_W("ReleaseDC / Unlock : ") + ttstr((int)ReleaseDCTime));
	TVPAddLog(TJS_W("DrawPrimitive : ") + ttstr((int)DrawPrimitiveTime));
	TVPAddLog(TJS_W("Blt : ") + ttstr((int)BltTime));
}
#endif
//---------------------------------------------------------------------------
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
			ErrorToLog( hr );
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
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
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
	
	/*
	TVPAddLog(	TJS_W("Dst W:") + ttstr((int)DestWidth) + TJS_W(", Dst H:") + ttstr((int)DestHeight) + 
				TJS_W(", Src W:") + ttstr((int)SrcWidth) + TJS_W(", Src H:") + ttstr((int)SrcHeight) +
				TJS_W(", Tex W:") + ttstr((int)TextureWidth) + TJS_W(", Tex H:") + ttstr((int)TextureHeight)
		);
	*/

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
		hr = Direct3DDevice->Reset(&D3dPP);
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
		} else if( hr == D3D_OK ) {
			if( FAILED( hr = InitializeDirect3DState() ) ) {
				ErrorToLog( hr );
				TVPThrowExceptionMessage( TVPFaildToSetRenderState );
			}
		}
	} else if(hr != D3D_OK) {
		ErrorToLog( hr );
		TVPAddImportantLog( TVPFormatMessage(TVPPassthroughInfPolygonDrawingFailed,TJSInt32ToHex(hr, 8)) );
	}
}
//---------------------------------------------------------------------------
void tTVPDrawer_D3DDoubleBuffering::ErrorToLog( HRESULT hr ) {
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
		ErrorToLog( hr );
		InvalidateAll();  // causes reconstruction of off-screen image
	} else if(hr != D3D_OK) {
		ErrorToLog( hr );
		TVPAddImportantLog( TVPFormatMessage(TVPPassthroughInfPrimarySurfaceDirect3DDevicePresentFailed,TJSInt32ToHex(hr, 8)) );
	}
}
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
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

	UINT iMonitor = GetMonitorNumber( window );
	DestroyTexture();
	hr = DecideD3DPresentParameters( iMonitor, window, false, w, h, bpp, color, changeresolution );

	if(FAILED(hr)) {
		TVPAddLog(TJS_W("TVPDecideD3DPresentParameters failed/hr=") + TJSInt32ToHex(hr) );
		ErrorToLog( hr );
	} else {
		if( iMonitor != CurrentMonitor ) {
			 // 最初に生成された時からモニタが移動している、デバイス破棄して生成からやり直す
			try {
				CreateOffScreenSurface();
				Direct3DDevice->SetDialogBoxMode( TRUE );
				success = true;
			} catch(...) {
				success = false;
			}
		} else {
			hr = Direct3DDevice->Reset(&D3dPP);
			if(FAILED(hr)) {
				// リセットに失敗したら再生成を試みる
				try {
					CreateOffScreenSurface();
					Direct3DDevice->SetDialogBoxMode( TRUE );
					success = true;
				} catch(...) {
					success = false;
				} 
			} else {
				try {
					CreateTexture();
					Direct3DDevice->SetDialogBoxMode( TRUE );
					success = true;
				} catch(...) {
					success = false;
				}
			}
		}
	}
	return success;
}
//---------------------------------------------------------------------------
void tTVPDrawer_D3DDoubleBuffering::RevertFromFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color )
{
	// ウィンドウに戻す時にはモニタ移動は発生していないはず
	bool iswindowed = D3dPP.Windowed != 0;
	UINT iMonitor = GetMonitorNumber( window );
	HRESULT hr = DecideD3DPresentParameters( iMonitor, window, true, w, h, bpp );
	if( SUCCEEDED(hr) ) {
		if( Direct3DDevice ) {
			hr = Direct3DDevice->Reset(&D3dPP);
			if( FAILED(hr) ) {
				// リセットに失敗したら再生成を試みる
				CreateOffScreenSurface();
				hr = S_OK;
			} else {
				if( FAILED( hr = InitializeDirect3DState() ) ) {
					ErrorToLog( hr );
				}
			}
		} else {
			hr = E_FAIL;
		}
	}
	if( iswindowed != true && FAILED(hr) ) {
		::ChangeDisplaySettings(NULL, 0);
	}
}
//---------------------------------------------------------------------------
