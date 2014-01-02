
#define NOMINMAX

#include <windows.h>
#include <stdio.h>
#include <vfw.h>
#include "tp_stub.h"
#include "GDIDrawDevice.h"

//---------------------------------------------------------------------------
// オプション
//---------------------------------------------------------------------------
static tjs_int TVPPassThroughOptionsGeneration = 0;
bool TVPZoomInterpolation = true;
//---------------------------------------------------------------------------
static void TVPInitGDIOptions()
{
	if(TVPPassThroughOptionsGeneration == TVPGetCommandLineArgumentGeneration()) return;
	TVPPassThroughOptionsGeneration = TVPGetCommandLineArgumentGeneration();

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
tTVPGDIDrawDevice::tTVPGDIDrawDevice()
{
	TVPInitGDIOptions(); // read and initialize options
	TargetWindow = NULL;

	DrawDibHandle = NULL;
	DrawUpdateRectangle = false;
	TargetDC = NULL;
	OffScreenBitmap = NULL;
	OffScreenDC = NULL;
	OldOffScreenBitmap = NULL;
	ShouldShow = false;
}
//---------------------------------------------------------------------------
tTVPGDIDrawDevice::~tTVPGDIDrawDevice()
{
	DestroyBitmap();
	if(TargetDC && TargetWindow) ::ReleaseDC(TargetWindow, TargetDC);
	if(DrawDibHandle) ::DrawDibClose(DrawDibHandle), DrawDibHandle = NULL;
}
//---------------------------------------------------------------------------
void tTVPGDIDrawDevice::DestroyBitmap()
{
	if(OffScreenBitmap && OffScreenDC) {
		::SelectObject(OffScreenDC, OldOffScreenBitmap), OldOffScreenBitmap = NULL;
		if(OffScreenBitmap) ::DeleteObject(OffScreenBitmap), OffScreenBitmap = NULL;
		if(OffScreenDC)     ::DeleteDC(OffScreenDC), OffScreenDC = NULL;
	}
}
//---------------------------------------------------------------------------
void tTVPGDIDrawDevice::CreateBitmap()
{
	// スクリーン互換の DDB を作成する。
	// これはたいていの場合、ビデオメモリ上に作成される。
	DestroyBitmap();
	tjs_int w, h;
	GetSrcSize( w, h );
	if(TargetWindow && w > 0 && h > 0) {
		try {
			HDC screendc = ::GetDC(TargetWindow);
			if(!screendc) TVPThrowExceptionMessage(TJS_W("Failed to create screen DC"));
			OffScreenBitmap = ::CreateCompatibleBitmap(screendc, w, h);
			if(!OffScreenBitmap) TVPThrowExceptionMessage(TJS_W("Failed to create offscreen bitmap"));
			OffScreenDC     = ::CreateCompatibleDC(screendc);
			if(!OffScreenDC) TVPThrowExceptionMessage(TJS_W("Failed to create offscreen DC"));
			::ReleaseDC(TargetWindow, screendc);
			OldOffScreenBitmap = (HBITMAP)::SelectObject(OffScreenDC, OffScreenBitmap);
		} catch(...) {
			DestroyBitmap();
			throw;
		}
	}
}
//---------------------------------------------------------------------------
bool tTVPGDIDrawDevice::IsTargetWindowActive() const {
	if( TargetWindow == NULL ) return false;
	return ::GetForegroundWindow() == TargetWindow;
}
//---------------------------------------------------------------------------
void tTVPGDIDrawDevice::EnsureDrawer()
{
	TVPInitGDIOptions();
	if(!OffScreenBitmap || !OffScreenDC){
		CreateBitmap();
	}
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPGDIDrawDevice::AddLayerManager(iTVPLayerManager * manager)
{
	if(inherited::Managers.size() > 0)
	{
		// "GDI" デバイスでは２つ以上のLayer Managerを登録できない
		TVPThrowExceptionMessage(TJS_W("\"GDI\" device does not support layer manager more than 1"));
	}
	inherited::AddLayerManager(manager);

	manager->SetDesiredLayerType(ltOpaque); // ltOpaque な出力を受け取りたい
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPGDIDrawDevice::SetTargetWindow(HWND wnd, bool is_main)
{
	TVPInitGDIOptions();
	if( wnd ) {
		// 描画用 DC を取得する
		TargetDC = ::GetDC(wnd);
	} else {
		// 描画用 DC を開放する
		if(TargetDC) ::ReleaseDC(TargetWindow, TargetDC), TargetDC = NULL;
	}

	if(DrawDibHandle) ::DrawDibClose(DrawDibHandle), DrawDibHandle = NULL;
	TargetWindow = wnd;
	IsMainWindow = is_main;
	DrawDibHandle = DrawDibOpen();

	CreateBitmap();
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPGDIDrawDevice::SetDestRectangle(const tTVPRect & rect)
{
	// 位置だけの変更の場合かどうかをチェックする
	if(rect.get_width() == DestRect.get_width() && rect.get_height() == DestRect.get_height()) {
		// 位置だけの変更だ
		inherited::SetDestRectangle(rect);
	} else {
		// サイズも違う
		inherited::SetDestRectangle(rect);
		DestroyBitmap();
		EnsureDrawer();
	}
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPGDIDrawDevice::NotifyLayerResize(iTVPLayerManager * manager)
{
	inherited::NotifyLayerResize(manager);
	CreateBitmap();
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPGDIDrawDevice::Show()
{
	// TODO ClipRect の考慮が欠けている
	if(TargetDC && OffScreenDC && ShouldShow) {
		// オフスクリーンビットマップを TargetDC に転送する
		tjs_int srcw, srch;
		GetSrcSize( srcw, srch );
		tjs_int dstw = DestRect.get_width();
		tjs_int dsth = DestRect.get_height();
		if(dstw == srcw && dsth == srch) {
			// 拡大・縮小は必要ない
			::BitBlt(TargetDC,
				DestRect.left,
				DestRect.top,
				dstw,
				dsth,
				OffScreenDC,
				0,
				0,
				SRCCOPY);
		} else {
			// 拡大・縮小が必要
			if(TVPZoomInterpolation)
				SetStretchBltMode(TargetDC, HALFTONE);
			else
				SetStretchBltMode(TargetDC, COLORONCOLOR);
			::SetBrushOrgEx(TargetDC, 0, 0, NULL);

			::StretchBlt(TargetDC,
				DestRect.left,
				DestRect.top,
				dstw,
				dsth,
				OffScreenDC,
				0,
				0,
				srcw,
				srch,
				SRCCOPY);
		}
		ShouldShow = false;
	}
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPGDIDrawDevice::StartBitmapCompletion(iTVPLayerManager * manager)
{
	EnsureDrawer();
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPGDIDrawDevice::NotifyBitmapCompleted(iTVPLayerManager * manager,
	tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
	const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity)
{
	TVPInitGDIOptions();
	// DrawDibDraw にて OffScreenDC に描画を行う
	if(DrawDibHandle && OffScreenDC) {
		ShouldShow = true;
		::DrawDibDraw(DrawDibHandle,
			OffScreenDC,
			x,
			y,
			cliprect.get_width(),
			cliprect.get_height(),
			const_cast<BITMAPINFOHEADER*>(reinterpret_cast<const BITMAPINFOHEADER*>(bitmapinfo)),
			const_cast<void*>(bits),
			cliprect.left,
			cliprect.top,
			cliprect.get_width(),
			cliprect.get_height(),
			0);
	}
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPGDIDrawDevice::EndBitmapCompletion(iTVPLayerManager * manager)
{
	// やることなし
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPGDIDrawDevice::SetShowUpdateRect(bool b)
{
	DrawUpdateRectangle = b;
}
//---------------------------------------------------------------------------

