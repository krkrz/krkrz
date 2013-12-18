//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//!@file "PassThrough" 描画デバイス管理
//---------------------------------------------------------------------------

#include <windows.h>
#include <stdio.h>
#include "tp_stub.h"
#include "PassThroughDrawDevice.h"
#include <vfw.h>
#include <ddraw.h>
#include <d3d.h>

/*
	PassThroughDrawDevice クラスには、Window.PassThroughDrawDevice として
	アクセスできる。通常、Window クラスを生成すると、その drawDevice プロパ
	ティには自動的にこのクラスのインスタンスが設定されるので、(ほかのDrawDevice
	を使わない限りは) 特に意識する必要はない。

	PassThroughDrawDevice は以下のメソッドとプロパティを持つ。

	recreate()
		Drawer (内部で使用している描画方式) を切り替える。preferredDrawer プロパティ
		が dtNone 以外であればそれに従うが、必ず指定された drawer が使用される保証はない。

	preferredDrawer
		使用したい drawer を表すプロパティ。以下のいずれかの値をとる。
		値を設定することも可能。new 直後の値は コマンドラインオプションの dbstyle で
		設定した値になる。
		drawerがこの値になる保証はない (たとえば dtDBD3D を指定していても何らかの
		原因で Direct3D の初期化に失敗した場合は DirectDraw が使用される可能性がある)。
		ウィンドウ作成直後、最初にプライマリレイヤを作成するよりも前にこのプロパティを
		設定する事により、recreate() をわざわざ実行しなくても指定の drawer を使用
		させることができる。
		Window.PassThroughDrawDevice.dtNone			指定しない
		Window.PassThroughDrawDevice.dtDrawDib		拡大縮小が必要な場合はGDI、
													そうでなければDBなし
		Window.PassThroughDrawDevice.dtDBGDI		GDIによるDB
		Window.PassThroughDrawDevice.dtDBDD			DirectDrawによるDB
		Window.PassThroughDrawDevice.dtDBD3D		Direct3DによるDB

	drawer
		現在使用されている drawer を表すプロパティ。以下のいずれかの値をとる。
		読み取り専用。
		Window.PassThroughDrawDevice.dtNone			普通はこれはない
		Window.PassThroughDrawDevice.dtDrawDib		ダブルバッファリング(DB)なし
		Window.PassThroughDrawDevice.dtDBGDI		GDIによるDB
		Window.PassThroughDrawDevice.dtDBDD			DirectDrawによるDB
		Window.PassThroughDrawDevice.dtDBD3D		Direct3DによるDB
*/

//---------------------------------------------------------------------------
// オプション
//---------------------------------------------------------------------------
static tjs_int TVPPassThroughOptionsGeneration = 0;
static bool TVPZoomInterpolation = true;
static bool TVPForceDoublebuffer = false;
static tTVPPassThroughDrawDevice::tDrawerType TVPPreferredDrawType = tTVPPassThroughDrawDevice::dtNone;
//---------------------------------------------------------------------------
static void TVPInitPassThroughOptions()
{
	if(TVPPassThroughOptionsGeneration == TVPGetCommandLineArgumentGeneration()) return;
	TVPPassThroughOptionsGeneration = TVPGetCommandLineArgumentGeneration();

	bool initddraw = false;
	tTJSVariant val;

	TVPForceDoublebuffer = false;
	if(TVPGetCommandLine(TJS_W("-usedb"), &val) )
	{
		ttstr str(val);
		if(str == TJS_W("yes")) TVPForceDoublebuffer = true;
	}

	TVPPreferredDrawType = tTVPPassThroughDrawDevice::dtNone;
	if(TVPGetCommandLine(TJS_W("-dbstyle"), &val) )
	{
		ttstr str(val);
		if(str == TJS_W("none") || str == TJS_W("no") || str == TJS_W("auto"))
			TVPPreferredDrawType = tTVPPassThroughDrawDevice::dtNone;
		if(str == TJS_W("gdi"))
			TVPPreferredDrawType = tTVPPassThroughDrawDevice::dtDBGDI;
		if(str == TJS_W("ddraw"))
			TVPPreferredDrawType = tTVPPassThroughDrawDevice::dtDBDD;
		if(str == TJS_W("d3d"))
			TVPPreferredDrawType = tTVPPassThroughDrawDevice::dtDBD3D;
	}

	TVPZoomInterpolation = true;
	if(TVPGetCommandLine(TJS_W("-smoothzoom"), &val))
	{
		ttstr str(val);
		if(str == TJS_W("no"))
			TVPZoomInterpolation = false;
		else
			TVPZoomInterpolation = true;
	}

	if(initddraw) TVPEnsureDirectDrawObject();
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
//! @brief	PassThrough で用いる描画方法用インターフェース
//---------------------------------------------------------------------------
class tTVPDrawer
{
protected:
	tTVPPassThroughDrawDevice * Device;
	tjs_int DestLeft;
	tjs_int DestTop;
	tjs_int DestWidth;
	tjs_int DestHeight;
	tjs_int SrcWidth;
	tjs_int SrcHeight;
	HWND TargetWindow;
	HDRAWDIB DrawDibHandle;
	bool DrawUpdateRectangle;
public:
	tTVPDrawer(tTVPPassThroughDrawDevice * device)
	{
		Device = device;
		SrcWidth = 0;
		SrcHeight = 0;
		DestLeft = DestTop = DestWidth = DestHeight = 0;
		TargetWindow = NULL;
		DrawDibHandle = NULL;
		DrawUpdateRectangle = NULL;
	} 
	virtual ~tTVPDrawer()
	{
		if(DrawDibHandle) DrawDibClose(DrawDibHandle), DrawDibHandle = NULL;
	}
	virtual ttstr GetName() = 0;

	virtual bool SetDestPos(tjs_int left, tjs_int top)
		{ DestLeft = left; DestTop = top; return true; }
	virtual bool SetDestSize(tjs_int width, tjs_int height)
		{ DestWidth = width; DestHeight = height; return true; }
	void GetDestSize(tjs_int &width, tjs_int &height) const
		{ width = DestWidth; height = DestHeight; }
	virtual bool NotifyLayerResize(tjs_int w, tjs_int h)
		{ SrcWidth = w; SrcHeight = h; return true; }
	void GetSrcSize(tjs_int &w, tjs_int &h) const
		{ w = SrcWidth; h = SrcHeight; }
	virtual bool SetDestSizeAndNotifyLayerResize(tjs_int width, tjs_int height, tjs_int w, tjs_int h)
	{
		if(!SetDestSize(width, height)) return false;
		if(!NotifyLayerResize(w, h)) return false;
		return true;
	}
	virtual void SetTargetWindow(HWND wnd)
	{
		if(DrawDibHandle) DrawDibClose(DrawDibHandle), DrawDibHandle = NULL;
		TargetWindow = wnd;
		DrawDibHandle = DrawDibOpen();
	}
	virtual void StartBitmapCompletion() = 0;
	virtual void NotifyBitmapCompleted(tjs_int x, tjs_int y,
		const void * bits, const BITMAPINFO * bitmapinfo,
		const tTVPRect &cliprect) = 0;
	virtual void EndBitmapCompletion() = 0;
	virtual void Show() {;}
	virtual void SetShowUpdateRect(bool b)  { DrawUpdateRectangle = b; }
	virtual int GetInterpolationCapability() { return 3; }
		// bit 0 for point-on-point, bit 1 for bilinear interpolation

	virtual void InitTimings() {;} // notifies begining of benchmark
	virtual void ReportTimings() {;} // notifies end of benchmark
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//! @brief	GDIによる描画を必要とする基本クラス
//---------------------------------------------------------------------------
class tTVPDrawer_GDI : public tTVPDrawer
{
	typedef tTVPDrawer inherited;
protected:
	HDC TargetDC;

public:
	//! @brief	コンストラクタ
	tTVPDrawer_GDI(tTVPPassThroughDrawDevice * device) : tTVPDrawer(device)
	{
		TargetDC = NULL;
	}

	//! @brief	デストラクタ
	~tTVPDrawer_GDI()
	{
		if(TargetDC && TargetWindow) ReleaseDC(TargetWindow, TargetDC);
	}

	void SetTargetWindow(HWND wnd)
	{
		if(wnd)
		{
			// 描画用 DC を取得する
			TargetDC = GetDC(wnd);
		}
		else
		{
			// 描画用 DC を開放する
			if(TargetDC) ReleaseDC(TargetWindow, TargetDC), TargetDC = NULL;
		}

		inherited::SetTargetWindow(wnd);

	}
};
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
//! @brief	DrawDibによるバッファ無し描画を行う基本クラス
//---------------------------------------------------------------------------
class tTVPDrawer_DrawDibNoBuffering : public tTVPDrawer_GDI
{
	typedef tTVPDrawer_GDI inherited;

	HPEN BluePen;
	HPEN YellowPen;

public:
	//! @brief	コンストラクタ
	tTVPDrawer_DrawDibNoBuffering(tTVPPassThroughDrawDevice * device) : tTVPDrawer_GDI(device)
	{
		BluePen = NULL;
		YellowPen = NULL;
	}

	//! @brief	デストラクタ
	~tTVPDrawer_DrawDibNoBuffering()
	{
		if(BluePen)   DeleteObject(BluePen);
		if(YellowPen) DeleteObject(YellowPen);
	}

	virtual ttstr GetName() { return TJS_W("DrawDIB (no buffering)"); }

	bool SetDestSize(tjs_int width, tjs_int height)
	{
		// このデバイスでは拡大縮小はできないので
		// 拡大縮小が必要な場合は false を返す
		tjs_int w, h;
		Device->GetSrcSize(w, h);
		if(width != w || height != h)
			return false;

		return inherited::SetDestSize(width, height);
	}

	bool NotifyLayerResize(tjs_int w, tjs_int h)
	{
		return inherited::NotifyLayerResize(w, h);
	}

	void SetTargetWindow(HWND wnd)
	{
		inherited::SetTargetWindow(wnd);
	}

	void StartBitmapCompletion()
	{
		// やることなし
	}

	void NotifyBitmapCompleted(tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
		const tTVPRect &cliprect)
	{
		// DrawDibDraw にて TargetDC に描画を行う
		if(DrawDibHandle && TargetDC)
			DrawDibDraw(DrawDibHandle,
				TargetDC,
				x + DestLeft,
				y + DestTop,
				cliprect.get_width(),
				cliprect.get_height(),
				const_cast<BITMAPINFOHEADER*>(reinterpret_cast<const BITMAPINFOHEADER*>(bitmapinfo)),
				const_cast<void*>(bits),
				cliprect.left,
				cliprect.top,
				cliprect.get_width(),
				cliprect.get_height(),
				0);

		// 更新矩形の表示
		if(DrawUpdateRectangle)
		{
			if(!BluePen) BluePen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
			if(!YellowPen) YellowPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 0));

			HPEN oldpen;

			int ROP_save = GetROP2(TargetDC);

			tjs_int rleft   = x + DestLeft;
			tjs_int rtop    = y + DestTop;
			tjs_int rright  = rleft + cliprect.get_width();
			tjs_int rbottom = rtop  + cliprect.get_height();

			POINT points[5];
			points[0].x = rleft;
			points[0].y = rtop;
			points[1].x = rright -1;
			points[1].y = rtop;
			points[2].x = rright -1;
			points[2].y = rbottom -1;
			points[3].x = rleft;
			points[3].y = rbottom -1;
			points[4] = points[0];

			oldpen = SelectObject(TargetDC, BluePen);
			SetROP2(TargetDC, R2_NOTMASKPEN);
			Polyline(TargetDC, points, 4);

			SelectObject(TargetDC, YellowPen);
			SetROP2(TargetDC, R2_MERGEPEN);
			Polyline(TargetDC, points, 5);

			SelectObject(TargetDC, oldpen);
		}
	}

	void EndBitmapCompletion()
	{
		// やることなし
	}

	virtual int GetInterpolationCapability() { return 1; }
		// bit 0 for point-on-point, bit 1 for bilinear interpolation

};
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
//! @brief	GDIによるダブルバッファリングを行うクラス
//---------------------------------------------------------------------------
class tTVPDrawer_GDIDoubleBuffering : public tTVPDrawer_GDI
{
	typedef tTVPDrawer_GDI inherited;
	HBITMAP OffScreenBitmap; //!< オフスクリーンビットマップ
	HDC OffScreenDC; //!< オフスクリーン DC
	HBITMAP OldOffScreenBitmap; //!< OffScreenDC に以前選択されていた ビットマップ
	bool ShouldShow; //!< show で実際に画面に画像を転送すべきか
	bool InBenchMark; //!< ベンチマーク中かどうか

public:
	//! @brief	コンストラクタ
	tTVPDrawer_GDIDoubleBuffering(tTVPPassThroughDrawDevice * device) : tTVPDrawer_GDI(device)
	{
		OffScreenBitmap = NULL;
		OffScreenDC = NULL;
		OldOffScreenBitmap = NULL;
		ShouldShow = false;
		InBenchMark = false;
	}

	//! @brief	デストラクタ
	~tTVPDrawer_GDIDoubleBuffering()
	{
		DestroyBitmap();
	}

	virtual ttstr GetName() { return TJS_W("GDI double buffering"); }

	void DestroyBitmap()
	{
		if(OffScreenBitmap && OffScreenDC)
		{
			SelectObject(OffScreenDC, OldOffScreenBitmap), OldOffScreenBitmap = NULL;
			if(OffScreenBitmap) DeleteObject(OffScreenBitmap), OffScreenBitmap = NULL;
			if(OffScreenDC)     DeleteDC(OffScreenDC), OffScreenDC = NULL;
		}
	}

	void CreateBitmap()
	{
		// スクリーン互換の DDB を作成する。
		// これはたいていの場合、ビデオメモリ上に作成される。
		DestroyBitmap();
		if(TargetWindow && SrcWidth > 0 && SrcHeight > 0)
		{
			try
			{
				HDC screendc = GetDC(TargetWindow);
				if(!screendc) TVPThrowExceptionMessage(TJS_W("Failed to create screen DC"));
				OffScreenBitmap = CreateCompatibleBitmap(screendc, SrcWidth, SrcHeight);
				if(!OffScreenBitmap) TVPThrowExceptionMessage(TJS_W("Failed to create offscreen bitmap"));
				OffScreenDC     = CreateCompatibleDC(screendc);
				if(!OffScreenDC) TVPThrowExceptionMessage(TJS_W("Failed to create offscreen DC"));
				ReleaseDC(TargetWindow, screendc);
				OldOffScreenBitmap = SelectObject(OffScreenDC, OffScreenBitmap);
			}
			catch(...)
			{
				DestroyBitmap();
				throw;
			}
		}
	}

	bool SetDestSize(tjs_int width, tjs_int height)
	{
		return inherited::SetDestSize(width, height);
	}

	bool NotifyLayerResize(tjs_int w, tjs_int h)
	{
		if(inherited::NotifyLayerResize(w, h))
		{
			CreateBitmap();
			return true;
		}
		return false;
	}

	void SetTargetWindow(HWND wnd)
	{
		inherited::SetTargetWindow(wnd);
		CreateBitmap();
	}

	void StartBitmapCompletion()
	{
		// やることなし
	}

	void NotifyBitmapCompleted(tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
		const tTVPRect &cliprect)
	{
		// DrawDibDraw にて OffScreenDC に描画を行う
		if(DrawDibHandle && OffScreenDC)
		{
			ShouldShow = true;
			DrawDibDraw(DrawDibHandle,
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

	void EndBitmapCompletion()
	{
	}

	void Show()
	{
		if(TargetDC && OffScreenDC && ShouldShow)
		{
			// オフスクリーンビットマップを TargetDC に転送する
			if(DestWidth == SrcWidth && DestHeight == SrcHeight)
			{
				// 拡大・縮小は必要ない
				BitBlt(TargetDC,
					DestLeft,
					DestTop,
					DestWidth,
					DestHeight,
					OffScreenDC,
					0,
					0,
					SRCCOPY);
			}
			else
			{
				// 拡大・縮小が必要
				if(TVPZoomInterpolation)
					SetStretchBltMode(TargetDC, HALFTONE);
				else
					SetStretchBltMode(TargetDC, COLORONCOLOR);
				SetBrushOrgEx(TargetDC, 0, 0, NULL);

				StretchBlt(TargetDC,
					DestLeft,
					DestTop,
					DestWidth,
					DestHeight,
					OffScreenDC,
					0,
					0,
					SrcWidth,
					SrcHeight,
					SRCCOPY);
			}

			if(InBenchMark)
			{
				// 画面からの読み出しを行う関数を実行する
				// こうしないと StrechBlt などはコマンドキューにたたき込まれる
				// だけで、実際の描画を待たずに帰る可能性がある。
				(void)GetPixel(TargetDC, DestLeft + DestWidth / 2, DestTop + DestHeight / 2);
			}

			ShouldShow = false;
		}
	}

	virtual int GetInterpolationCapability() { return 1+2; }
		// bit 0 for point-on-point, bit 1 for bilinear interpolation

	virtual void InitTimings() { InBenchMark = true; }
	virtual void ReportTimings() { InBenchMark = false; }


};
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
//! @brief	DirectDrawによるダブルバッファリングを行うクラス
//---------------------------------------------------------------------------
class tTVPDrawer_DDDoubleBuffering : public tTVPDrawer
{
	typedef tTVPDrawer inherited;

	HDC OffScreenDC;
	IDirectDrawSurface * Surface;
	IDirectDrawClipper * Clipper;

	bool LastOffScreenDCGot;
	bool ShouldShow; //!< show で実際に画面に画像を転送すべきか

public:
	//! @brief	コンストラクタ
	tTVPDrawer_DDDoubleBuffering(tTVPPassThroughDrawDevice * device) : tTVPDrawer(device)
	{
		TVPEnsureDirectDrawObject();
		OffScreenDC = NULL;
		Surface = NULL;
		Clipper = NULL;
		LastOffScreenDCGot = true;
		ShouldShow = false;
	}

	//! @brief	デストラクタ
	~tTVPDrawer_DDDoubleBuffering()
	{
		DestroyOffScreenSurface();
	}

	virtual ttstr GetName() { return TJS_W("DirectDraw double buffering"); }

	void DestroyOffScreenSurface()
	{
		if(OffScreenDC && Surface) Surface->ReleaseDC(OffScreenDC);
		if(Clipper) Clipper->Release(), Clipper = NULL;
		if(Surface) Surface->Release(), Surface = NULL;
		TVPReleaseDDPrimarySurface();
	}

	void InvalidateAll()
	{
		// レイヤ演算結果をすべてリクエストする
		// サーフェースが lost した際に内容を再構築する目的で用いる
		Device->RequestInvalidation(tTVPRect(0, 0, DestWidth, DestHeight));
	}

	void CreateOffScreenSurface()
	{
		// オフスクリーンサーフェースを設定する
		DestroyOffScreenSurface();
		if(TargetWindow && SrcWidth > 0 && SrcHeight > 0)
		{
			IDirectDraw2 *object = TVPGetDirectDrawObjectNoAddRef();
			if(!object) TVPThrowExceptionMessage(TJS_W("DirectDraw not available"));

			// allocate secondary off-screen buffer
			DDSURFACEDESC ddsd;
			ZeroMemory(&ddsd, sizeof(ddsd));
			ddsd.dwSize = sizeof(ddsd);
			ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
			ddsd.dwWidth = SrcWidth;
			ddsd.dwHeight = SrcHeight;
			ddsd.ddsCaps.dwCaps =
				DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
			HRESULT hr;

			hr = object->CreateSurface(&ddsd, &Surface, NULL);

			if(hr != DD_OK)
				TVPThrowExceptionMessage(TJS_W("Cannot allocate off-screen surface/HR=%1"),
					TJSInt32ToHex(hr, 8));

			// check whether the surface is on video memory
			ZeroMemory(&ddsd, sizeof(ddsd));
			ddsd.dwSize = sizeof(ddsd);

			hr = Surface->GetSurfaceDesc(&ddsd);

			if(hr != DD_OK)
			{
				TVPThrowExceptionMessage(TJS_W("Cannot get surface description/HR=%1"),
					TJSInt32ToHex(hr, 8));
			}

			if(ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY &&
				ddsd.ddsCaps.dwCaps & DDSCAPS_LOCALVIDMEM)
			{
				// ok
			}
			else
			{
				TVPThrowExceptionMessage(TJS_W("Cannot allocate the surface on the local video memory"),
					TJSInt32ToHex(hr, 8));
			}


			// create clipper object
			hr = object->CreateClipper(0, &Clipper, NULL);
			if(hr != DD_OK)
			{
				TVPThrowExceptionMessage(TJS_W("Cannot create a clipper object/HR=%1"),
					TJSInt32ToHex(hr, 8));
			}
			hr = Clipper->SetHWnd(0, TargetWindow);
			if(hr != DD_OK)
			{
				TVPThrowExceptionMessage(TJS_W("Cannot set the window handle to the clipper object/HR=%1"),
					TJSInt32ToHex(hr, 8));
			}
		}
	}

	bool SetDestSize(tjs_int width, tjs_int height)
	{
		return inherited::SetDestSize(width, height);
	}

	bool NotifyLayerResize(tjs_int w, tjs_int h)
	{
		if(inherited::NotifyLayerResize(w, h))
		{
			try
			{
				CreateOffScreenSurface();
			}
/*			catch(const eTJS & e)
			{
				TVPAddImportantLog(TJS_W("Passthrough: Failed to create DirectDraw off-screen buffer: ") + e.GetMessage());
				return false;
			}
*/			catch(...)
			{
				TVPAddImportantLog(TJS_W("Passthrough: Failed to create DirectDraw off-screen buffer: unknown reason"));
				return false;
			}
			return true;
		}
		return false;
	}

	void SetTargetWindow(HWND wnd)
	{
		inherited::SetTargetWindow(wnd);
		CreateOffScreenSurface();
	}

	void StartBitmapCompletion()
	{
		// retrieve DC
		if(Surface && OffScreenDC) Surface->ReleaseDC(OffScreenDC), OffScreenDC = NULL;

		if(Surface && TargetWindow)
		{
			HDC dc = NULL;
			HRESULT hr = Surface->GetDC(&dc);
			if(hr == DDERR_SURFACELOST || hr == DDERR_SURFACEBUSY)
			{
				Surface->Restore();
				InvalidateAll();  // causes reconstruction of off-screen image
				hr = Surface->GetDC(&dc);
			}

			if(hr != DD_OK)
			{
				dc = NULL;
				InvalidateAll();  // causes reconstruction of off-screen image

				if(LastOffScreenDCGot)
				{
					// display this message only once since last success
					TVPAddImportantLog(
						TJS_W("Passthrough: (inf) Off-screen surface, IDirectDrawSurface::GetDC failed/HR=") +
						TJSInt32ToHex(hr, 8) + TJS_W(", recreating drawer ..."));
					Device->DestroyDrawer(); // destroy self
					return; // return immediately
				}
			}

			OffScreenDC = dc;

			if(OffScreenDC) LastOffScreenDCGot = true; else LastOffScreenDCGot = false;
		}
	}

	void NotifyBitmapCompleted(tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
		const tTVPRect &cliprect)
	{
		// DrawDibDraw にて OffScreenDC に描画を行う
		if(DrawDibHandle && OffScreenDC && TargetWindow)
		{
			ShouldShow = true;
			DrawDibDraw(DrawDibHandle,
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

	void EndBitmapCompletion()
	{
		if(!TargetWindow) return;
		if(!Surface) return;
		if(!OffScreenDC) return;

		Surface->ReleaseDC(OffScreenDC), OffScreenDC = NULL;
	}

	void Show()
	{
		if(!TargetWindow) return;
		if(!Surface) return;
		if(!ShouldShow) return;

		ShouldShow = false;

		// Blt to the primary surface
		IDirectDrawSurface *pri = TVPGetDDPrimarySurfaceNoAddRef();
		if(!pri)
			TVPThrowExceptionMessage(TJS_W("Cannot retrieve primary surface object"));

		// set clipper
		TVPSetDDPrimaryClipper(Clipper);

		// get PaintBox's origin
		POINT origin; origin.x = DestLeft, origin.y = DestTop;
		ClientToScreen(TargetWindow, &origin);

		// entire of the bitmap is to be transfered (this is not optimal. FIX ME!)
		RECT drect;
		drect.left   = origin.x;
		drect.top    = origin.y;
		drect.right  = origin.x + DestWidth;
		drect.bottom = origin.y + DestHeight;

		RECT srect;
		srect.left = 0;
		srect.top = 0;
		srect.right  = SrcWidth;
		srect.bottom = SrcHeight;

		HRESULT hr = pri->Blt(&drect, Surface, &srect, DDBLT_WAIT, NULL);
		if(hr == DDERR_SURFACELOST || hr == DDERR_SURFACEBUSY)
		{
			pri->Restore();
			Surface->Restore();
			InvalidateAll();  // causes reconstruction of off-screen image
		}
		else if(hr == DDERR_INVALIDRECT)
		{
			// ignore this error
		}
		else if(hr != DD_OK)
		{
			TVPAddImportantLog(
				TJS_W("Passthrough: (inf) Primary surface, IDirectDrawSurface::Blt failed/HR=") +
				TJSInt32ToHex(hr, 8));
		}
	}

	virtual int GetInterpolationCapability()
	{
		// bit 0 for point-on-point, bit 1 for bilinear interpolation
		// さて、DirectDraw の blt が補間を行うかどうかを確認するのはちょっと
		// やっかいである。
		// GetCaps などのメソッドがあって、そこから得られる値に補間を行うかどうかの
		// 情報があるならば話は早いが、そんなもんは探した限りではみつからない。
		// プライマリサーフェースへ実際に画像を転送して確かめてみるという手はあるが
		// 画面を汚す上に描画してから確認するまでの間に他のアプリがそこの画像を
		// 消してしまうかもしれない (やっかいなもんだいですなあ)
		// しょうがないので、２個ちっこいオフスクリーンサーフェースを作ってみて、
		// そこの間同士での転送を行ってみることにする。オフスクリーンサーフェースでは
		// あるが、プライマリサーフェースと同じくビデオメモリ上に配置されるので
		// 同じような補間の仕方をしてくれると期待する。
		// ちっこいといっても、ある程度の大きさがないと補間を行ってくれない
		// デバイスが存在するかもしれないので、100x100と200x200を確保してみることにする。
		IDirectDraw2 *object = TVPGetDirectDrawObjectNoAddRef();
		if(!object) return 0;

		int caps = 0;

		IDirectDrawSurface * s1 = NULL;
		IDirectDrawSurface * s2 = NULL;
		HDC s1dc = NULL;
		HDC s2dc = NULL;

		// サーフェースの確保
		for(int i = 0; i < 2; i++)
		{
			IDirectDrawSurface * & surface = (i == 0) ? s1 : s2;

			// allocate secondary off-screen buffer
			DDSURFACEDESC ddsd;
			ZeroMemory(&ddsd, sizeof(ddsd));
			ddsd.dwSize = sizeof(ddsd);
			ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
			ddsd.dwWidth  = (i == 0) ? 100 : 200;
			ddsd.dwHeight = (i == 0) ? 100 : 200;
			ddsd.ddsCaps.dwCaps =
				DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
			HRESULT hr;

			hr = object->CreateSurface(&ddsd, &surface, NULL);

			if(hr != DD_OK) goto got_error;

			// check whether the surface is on video memory
			ZeroMemory(&ddsd, sizeof(ddsd));
			ddsd.dwSize = sizeof(ddsd);

			hr = surface->GetSurfaceDesc(&ddsd);

			if(hr != DD_OK) goto got_error;

			if(ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY &&
				ddsd.ddsCaps.dwCaps & DDSCAPS_LOCALVIDMEM)
			{
				// ok
			}
			else
			{
				goto got_error;
			}
		}

		// s1 に しろ と くろ の細かい縦のストライプを書く
		while(true)
		{
			HDC dc = NULL;
			HRESULT hr = s1->GetDC(&s1dc);
			if(hr == DDERR_SURFACELOST || hr == DDERR_SURFACEBUSY)
			{
				s1->Restore();
				continue;
			}
			else if(FAILED(hr))
				goto got_error;

			// s1 のサイズ(100x100) にストライプを書く
			HPEN white_pen   = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
			HPEN black_pen   = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

			HPEN oldpen;

			oldpen = SelectObject(s1dc, white_pen);
			SetROP2(s1dc, R2_COPYPEN);
			for(int i = 0; i < 100; i += 2)
			{
				MoveToEx(s1dc, i, 0, NULL);
				LineTo(s1dc, i, 100);
			}

			SelectObject(s1dc, black_pen);
			for(int i = 1; i < 100; i += 2)
			{
				MoveToEx(s1dc, i, 0, NULL);
				LineTo(s1dc, i, 100);
			}
			SelectObject(s1dc, oldpen);

			s1->ReleaseDC(s1dc), s1dc = NULL;
			break;
		}

		// s1 を s2 に拡大 Blt する
		RECT drect;
		drect.left   = 0;
		drect.top    = 0;
		drect.right  = 200;
		drect.bottom = 200;

		RECT srect;
		srect.left   = 0;
		srect.top    = 0;
		srect.right  = 100;
		srect.bottom = 100;

		if(FAILED(s2->Blt(&drect, s1, &srect, DDBLT_WAIT, NULL)))
			goto got_error;

		// s2 がどう拡大されたかを調査する
		while(true)
		{
			HDC dc = NULL;
			HRESULT hr = s2->GetDC(&s2dc);
			if(hr == DDERR_SURFACELOST || hr == DDERR_SURFACEBUSY)
			{
				s2->Restore();
				continue;
			}
			else if(FAILED(hr))
				goto got_error;

			// まんなかへんの画素を調べる
			bool halftone_detected = false;
			for(int i = 90; i < 110; i++)
			{
				// 色をget
				COLORREF color = GetPixel(s2dc, i, 100);
				// もし、補間が行われていれば、しろとくろ以外の色が
				// 出てきているはず
				halftone_detected = halftone_detected ||
					(color != 0xffffff && color != 0x000000);
			}
			caps = halftone_detected ? 2 : 1;

			s2->ReleaseDC(s2dc), s2dc = NULL;
			break;
		}

		// 解放する
	got_error:
		if(s1dc && s1)
			s1->ReleaseDC(s1dc), s1dc = NULL;
		if(s1) s1->Release(), s1 = NULL;
		if(s2dc && s2)
			s2->ReleaseDC(s2dc), s2dc = NULL;
		if(s2) s2->Release(), s2 = NULL;

		switch(caps)
		{
		case 0:
			TVPAddImportantLog(TJS_W("Passthrough: Could not get IDirectDraw::Blt working."));
			break;
		case 1:
			TVPAddImportantLog(TJS_W("Passthrough: IDirectDraw::Blt seems to filter by nearest neighbor method."));
			break;
		case 2:
			TVPAddImportantLog(TJS_W("Passthrough: IDirectDraw::Blt seems to filter by some kind of interpolation method."));
			break;
		}

		return caps;
	}
};
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
//! @brief	Direct3D7 によるダブルバッファリングを行うクラス
//! @note	tTVPDrawer_DDDoubleBuffering とよく似ているが別クラスになっている。
//!			修正を行う場合は、互いによく見比べ、似たようなところがあればともに修正を試みること。
//---------------------------------------------------------------------------
class tTVPDrawer_D3DDoubleBuffering : public tTVPDrawer
{
	typedef tTVPDrawer inherited;

/*
	note: Texture に対していったん描画された内容は Surface に転送され、
			さらにそこからプライマリサーフェースにコピーされる。
*/

	HDC OffScreenDC;
	IDirectDraw7 * DirectDraw7;
	IDirect3D7 * Direct3D7;
	IDirect3DDevice7 * Direct3DDevice7;
	IDirectDrawSurface7 * Surface;
	IDirectDrawSurface7 * Texture;
	IDirectDrawClipper * Clipper;

	void * TextureBuffer; //!< テクスチャのサーフェースへのメモリポインタ
	long TexturePitch; //!< テクスチャのピッチ

	tjs_uint TextureWidth; //!< テクスチャの横幅
	tjs_uint TextureHeight; //!< テクスチャの縦幅

	bool LastOffScreenDCGot;
	bool ShouldShow; //!< show で実際に画面に画像を転送すべきか
	bool UseDirectTransfer; //!< メモリ直接転送を行うかどうか

public:
	//! @brief	コンストラクタ
	tTVPDrawer_D3DDoubleBuffering(tTVPPassThroughDrawDevice * device) : tTVPDrawer(device)
	{
		TVPEnsureDirectDrawObject();
		OffScreenDC = NULL;
		DirectDraw7 = NULL;
		Direct3D7 = NULL;
		Direct3DDevice7 = NULL;
		Surface = NULL;
		Texture = NULL;
		Clipper = NULL;
		LastOffScreenDCGot = true;
		ShouldShow = false;
		UseDirectTransfer = false;
		TextureBuffer = NULL;
		TextureWidth = TextureHeight = 0;
	}

	//! @brief	デストラクタ
	~tTVPDrawer_D3DDoubleBuffering()
	{
		DestroyOffScreenSurface();
	}

	virtual ttstr GetName() { return TJS_W("Direct3D double buffering"); }

	void DestroyOffScreenSurface()
	{
		if(TextureBuffer && Texture) Texture->Unlock(NULL), TextureBuffer = NULL;
		if(OffScreenDC && Surface) Surface->ReleaseDC(OffScreenDC), OffScreenDC = NULL;
		if(Texture) Texture->Release(), Texture = NULL;
		if(Direct3DDevice7) Direct3DDevice7->Release(), Direct3DDevice7 = NULL;
		if(Surface) Surface->Release(), Surface = NULL;
		if(Direct3D7) Direct3D7->Release(), Direct3D7 = NULL;
		if(DirectDraw7) DirectDraw7->Release(), DirectDraw7 = NULL;
		TVPReleaseDDPrimarySurface();
		if(Clipper) Clipper->Release(), Clipper = NULL;
	}

	void InvalidateAll()
	{
		// レイヤ演算結果をすべてリクエストする
		// サーフェースが lost した際に内容を再構築する目的で用いる
		Device->RequestInvalidation(tTVPRect(0, 0, DestWidth, DestHeight));
	}

	void GetDirect3D7Device()
	{
		// get DirectDraw7/Direct3D7 interface
		if(DirectDraw7) DirectDraw7->Release(), DirectDraw7 = NULL;
		DirectDraw7 = TVPGetDirectDraw7ObjectNoAddRef();
		if(!DirectDraw7) TVPThrowExceptionMessage(TJS_W("DirectDraw7 not available"));

		DirectDraw7->AddRef();

		if(Direct3D7) Direct3D7->Release(), Direct3D7 = NULL;
		HRESULT hr = DirectDraw7->QueryInterface(IID_IDirect3D7, (void**)&Direct3D7);
		if(FAILED(hr))
			TVPThrowExceptionMessage(TJS_W("Direct3D7 not available"));
	}

	void CreateOffScreenSurface()
	{
		// Direct3D デバイス、テクスチャなどを作成する
		DestroyOffScreenSurface();
		if(TargetWindow && SrcWidth > 0 && SrcHeight > 0)
		{
			HRESULT hr;

			// get DirectDraw7/Direct3D7 interface
			GetDirect3D7Device();

			// check display mode
			DDSURFACEDESC2 ddsd;
			ZeroMemory(&ddsd, sizeof(ddsd));
			ddsd.dwSize = sizeof(ddsd);
			hr = DirectDraw7->GetDisplayMode(&ddsd);
			if(FAILED(hr) || ddsd.ddpfPixelFormat.dwRGBBitCount <= 8)
				TVPThrowExceptionMessage(TJS_W("Too less display color depth"));

			// create clipper object
			hr = DirectDraw7->CreateClipper(0, &Clipper, NULL);
			if(hr != DD_OK)
			{
				TVPThrowExceptionMessage(TJS_W("Cannot create a clipper object/HR=%1"),
					TJSInt32ToHex(hr, 8));
			}
			hr = Clipper->SetHWnd(0, TargetWindow);
			if(hr != DD_OK)
			{
				TVPThrowExceptionMessage(TJS_W("Cannot set the window handle to the clipper object/HR=%1"),
					TJSInt32ToHex(hr, 8));
			}

			// allocate secondary off-screen buffer
			ZeroMemory(&ddsd, sizeof(ddsd));
			ddsd.dwSize = sizeof(ddsd);
			ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
			ddsd.dwWidth  = DestWidth;
			ddsd.dwHeight = DestHeight;
			ddsd.ddsCaps.dwCaps =
				/*DDSCAPS_OFFSCREENPLAIN |*/ DDSCAPS_VIDEOMEMORY /*| DDSCAPS_LOCALVIDMEM*/ | DDSCAPS_3DDEVICE;

			hr = DirectDraw7->CreateSurface(&ddsd, &Surface, NULL);

			if(hr != DD_OK)
				TVPThrowExceptionMessage(TJS_W("Cannot allocate D3D off-screen surface/HR=%1"),
					TJSInt32ToHex(hr, 8));

			// check whether the surface is on video memory
			ZeroMemory(&ddsd, sizeof(ddsd));
			ddsd.dwSize = sizeof(ddsd);

			hr = Surface->GetSurfaceDesc(&ddsd);

			if(hr != DD_OK)
			{
				TVPThrowExceptionMessage(TJS_W("Cannot get D3D surface description/HR=%1"),
					TJSInt32ToHex(hr, 8));
			}

			if(ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY &&
				ddsd.ddsCaps.dwCaps & DDSCAPS_LOCALVIDMEM)
			{
				// ok
			}
			else
			{
				TVPThrowExceptionMessage(TJS_W("Cannot allocate the D3D surface on the local video memory"),
					TJSInt32ToHex(hr, 8));
			}

			// create Direct3D Device
			hr = Direct3D7->CreateDevice(IID_IDirect3DHALDevice, Surface, &Direct3DDevice7);
			if(FAILED(hr))
				TVPThrowExceptionMessage(TJS_W("Cannot create Direct3D device/HR=%1"),
					TJSInt32ToHex(hr, 8));

			// retrieve device caps
			D3DDEVICEDESC7 caps;
			ZeroMemory(&caps, sizeof(caps));
			if(FAILED(Direct3DDevice7->GetCaps(&caps)))
				TVPThrowExceptionMessage(TJS_W("Failed to retrieve Direct3D device caps/HR=%1"),
					TJSInt32ToHex(hr, 8));

			// decide texture size
			TextureWidth = SrcWidth;
			TextureHeight = SrcHeight;
			if(caps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
			{
				// only square textures are supported
				TextureWidth = std::max(TextureHeight, TextureWidth);
				TextureHeight = TextureWidth;
			}

			if(caps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2)
			{
				// power of 2 size of texture dimentions are required
				tjs_uint sz;

				sz = 1; while(sz < TextureWidth) sz <<= 1;
				TextureWidth = sz;

				sz = 1; while(sz < TextureHeight) sz <<= 1;
				TextureHeight = sz;
			}

			if(caps.dwMinTextureWidth  > TextureWidth) TextureWidth = caps.dwMinTextureWidth;
			if(caps.dwMinTextureHeight > TextureHeight) TextureHeight = caps.dwMinTextureHeight;
			if(	caps.dwMaxTextureWidth  < TextureWidth ||
				caps.dwMaxTextureHeight < TextureHeight)
			{
				TVPThrowExceptionMessage(TJS_W("Could not allocate texture size of %1x%2"),
					ttstr((int)TextureWidth), ttstr((int)TextureHeight));
			}

			// create Direct3D Texture
			ZeroMemory(&ddsd, sizeof(ddsd));
			ddsd.dwSize = sizeof(ddsd);
			ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
			ddsd.dwWidth  = TextureWidth;
			ddsd.dwHeight = TextureHeight;
			ddsd.ddsCaps.dwCaps =
				/*DDSCAPS_OFFSCREENPLAIN |*/ DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE | DDSCAPS_LOCALVIDMEM;

			ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
			ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
			ddsd.ddpfPixelFormat.dwRGBBitCount	= 32;
			ddsd.ddpfPixelFormat.dwRBitMask		= 0x00FF0000;
			ddsd.ddpfPixelFormat.dwGBitMask		= 0x0000FF00;
			ddsd.ddpfPixelFormat.dwBBitMask		= 0x000000FF;

			hr = DirectDraw7->CreateSurface(&ddsd, &Texture, NULL);

			if(hr == DD_OK)
			{
				UseDirectTransfer = true; // 直接のメモリ転送を有効にする
			}
			else /*if(hr != DD_OK) */
			{
				// ピクセルフォーマットを指定せずに生成を試みる

				ZeroMemory(&ddsd, sizeof(ddsd));
				ddsd.dwSize = sizeof(ddsd);
				ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS/* | DDSD_PIXELFORMAT*/;
				ddsd.dwWidth = SrcWidth;
				ddsd.dwHeight = SrcHeight;
				ddsd.ddsCaps.dwCaps =
					/*DDSCAPS_OFFSCREENPLAIN |*/ DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE | DDSCAPS_LOCALVIDMEM;

				hr = DirectDraw7->CreateSurface(&ddsd, &Texture, NULL);

				if(FAILED(hr))
					TVPThrowExceptionMessage(TJS_W("Cannot allocate D3D texture/HR=%1"),
						TJSInt32ToHex(hr, 8));

				TVPAddImportantLog("Passthrough: Using non 32bit ARGB texture format");
			}
		}
	}

	bool SetDestSize(tjs_int width, tjs_int height)
	{
		if(inherited::SetDestSize(width, height))
		{
			try
			{
				CreateOffScreenSurface();
			}
/*			catch(const eTJS & e)
			{
				TVPAddImportantLog(TJS_W("Passthrough: Failed to create Direct3D devices: ") + e.GetMessage());
				return false;
			}
*/			catch(...)
			{
				TVPAddImportantLog(TJS_W("Passthrough: Failed to create Direct3D devices: unknown reason"));
				return false;
			}
			return true;
		}
		return false;
	}

	bool NotifyLayerResize(tjs_int w, tjs_int h)
	{
		if(inherited::NotifyLayerResize(w, h))
		{
			try
			{
				CreateOffScreenSurface();
			}
/*			catch(const eTJS & e)
			{
				TVPAddImportantLog(TJS_W("Passthrough: Failed to create Direct3D devices: ") + e.GetMessage());
				return false;
			}
*/			catch(...)
			{
				TVPAddImportantLog(TJS_W("Passthrough: Failed to create Direct3D devices: unknown reason"));
				return false;
			}
			return true;
		}
		return false;
	}

	bool SetDestSizeAndNotifyLayerResize(tjs_int width, tjs_int height, tjs_int w, tjs_int h)
	{
		if(inherited::SetDestSize(width, height) && inherited::NotifyLayerResize(w, h))
		{
			try
			{
				CreateOffScreenSurface();
			}
/*			catch(const eTJS & e)
			{
				TVPAddImportantLog(TJS_W("Passthrough: Failed to create Direct3D devices: ") + e.GetMessage());
				return false;
			}
*/			catch(...)
			{
				TVPAddImportantLog(TJS_W("Passthrough: Failed to create Direct3D devices: unknown reason"));
				return false;
			}
			return true;
		}
		return false;
	}

	void SetTargetWindow(HWND wnd)
	{
		inherited::SetTargetWindow(wnd);
		CreateOffScreenSurface();
	}
//#define TVPD3DTIMING
#ifdef TVPD3DTIMING
	DWORD StartTick;

	DWORD GetDCTime;
	DWORD DrawDibDrawTime;
	DWORD ReleaseDCTime;
	DWORD DrawPrimitiveTime;
	DWORD BltTime;
	void InitTimings()
	{
		GetDCTime = 0;
		DrawDibDrawTime = 0;
		ReleaseDCTime = 0;
		DrawPrimitiveTime = 0;
		BltTime = 0;
	}

	void ReportTimings()
	{
		TVPAddLog(TJS_W("GetDC / Lock : ") + ttstr((int)GetDCTime));
		TVPAddLog(TJS_W("DrawDibDraw / memcpy : ") + ttstr((int)DrawDibDrawTime));
		TVPAddLog(TJS_W("ReleaseDC / Unlock : ") + ttstr((int)ReleaseDCTime));
		TVPAddLog(TJS_W("DrawPrimitive : ") + ttstr((int)DrawPrimitiveTime));
		TVPAddLog(TJS_W("Blt : ") + ttstr((int)BltTime));
	}
#endif

	void StartBitmapCompletion()
	{
		// retrieve DC
		if(Texture && TargetWindow)
		{
			if(UseDirectTransfer)
			{
#ifdef TVPD3DTIMING
StartTick = timeGetTime();
#endif
				if(TextureBuffer)
				{
					TVPAddImportantLog(TJS_W("Passthrough: Texture has already been locked (StartBitmapCompletion() has been called twice without EndBitmapCompletion()), unlocking the texture."));
					Texture->Unlock(NULL), TextureBuffer = NULL;
				}


				DDSURFACEDESC2 ddsd;
				ZeroMemory(&ddsd, sizeof(ddsd));
				ddsd.dwSize = sizeof(ddsd);
				HRESULT hr = Texture->Lock(NULL, &ddsd, DDLOCK_WAIT|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY, NULL);

				if(hr == DDERR_SURFACELOST || hr == DDERR_SURFACEBUSY)
				{
					Texture->Restore();
					InvalidateAll();  // causes reconstruction of off-screen image
					hr = Texture->Lock(NULL, &ddsd, DDLOCK_WAIT|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY, NULL);
				}

				if(hr != DD_OK)
				{
					TextureBuffer = NULL;
					InvalidateAll();  // causes reconstruction of off-screen image

					if(LastOffScreenDCGot)
					{
						// display this message only once since last success
						TVPAddImportantLog(
							TJS_W("Passthrough: (inf) Texture, IDirectDrawSurface::Lock failed/HR=") +
							TJSInt32ToHex(hr, 8) + TJS_W(", recreating drawer ..."));
						Device->DestroyDrawer(); // destroy self
						return;
					}
				}
				else /*if(hr == DD_OK) */
				{
					TextureBuffer = ddsd.lpSurface;
					TexturePitch = ddsd.lPitch;
				}


#ifdef TVPD3DTIMING
GetDCTime += timeGetTime() - StartTick;
#endif
			}
			else
			{
				HDC dc = NULL;
#ifdef TVPD3DTIMING
StartTick = timeGetTime();
#endif
				HRESULT hr = Texture->GetDC(&dc);
#ifdef TVPD3DTIMING
GetDCTime += timeGetTime() - StartTick;
#endif
				if(hr == DDERR_SURFACELOST || hr == DDERR_SURFACEBUSY)
				{
					Texture->Restore();
					InvalidateAll();  // causes reconstruction of off-screen image
					hr = Texture->GetDC(&dc);
				}

				if(hr != DD_OK)
				{
					dc = NULL;
					InvalidateAll();  // causes reconstruction of off-screen image

					if(LastOffScreenDCGot)
					{
						// display this message only once since last success
						TVPAddImportantLog(
							TJS_W("Passthrough: (inf) Texture, IDirectDrawSurface::GetDC failed/HR=") +
							TJSInt32ToHex(hr, 8) + TJS_W(", recreating drawer ..."));
						Device->DestroyDrawer(); // destroy self
						return;
					}
				}

				OffScreenDC = dc;

				if(OffScreenDC) LastOffScreenDCGot = true; else LastOffScreenDCGot = false;
			}
		}
	}

	void NotifyBitmapCompleted(tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
		const tTVPRect &cliprect)
	{
		if(UseDirectTransfer)
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
		else
		{
			// DrawDibDraw にて OffScreenDC に描画を行う
#ifdef TVPD3DTIMING
StartTick = timeGetTime();
#endif
			if(DrawDibHandle && OffScreenDC && TargetWindow)
			{
				ShouldShow = true;
				DrawDibDraw(DrawDibHandle,
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
#ifdef TVPD3DTIMING
DrawDibDrawTime += timeGetTime() - StartTick;
#endif
		}
	}

	void EndBitmapCompletion()
	{
		if(!TargetWindow) return;
		if(!Texture) return;
		if(!Surface) return;
		if(!Direct3DDevice7) return;

		if(UseDirectTransfer)
		{
#ifdef TVPD3DTIMING
StartTick = timeGetTime();
#endif
			if(!TextureBuffer) return;
			Texture->Unlock(NULL), TextureBuffer = NULL;
#ifdef TVPD3DTIMING
ReleaseDCTime += timeGetTime() - StartTick;
#endif
		}
		else
		{
#ifdef TVPD3DTIMING
StartTick = timeGetTime();
#endif
			if(!OffScreenDC) return;
			Texture->ReleaseDC(OffScreenDC), OffScreenDC = NULL;
#ifdef TVPD3DTIMING
ReleaseDCTime += timeGetTime() - StartTick;
#endif
		}

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
		//- draw as triangles
		hr = Direct3DDevice7->SetTexture(0, Texture);
		if(FAILED(hr)) goto got_error;

		Direct3DDevice7->SetRenderState(D3DRENDERSTATE_LIGHTING			, FALSE);
		Direct3DDevice7->SetRenderState(D3DRENDERSTATE_BLENDENABLE		, FALSE);
		Direct3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE	, FALSE); 
		Direct3DDevice7->SetRenderState(D3DRENDERSTATE_CULLMODE			, D3DCULL_NONE);

		Direct3DDevice7->SetTextureStageState(0, D3DTSS_MAGFILTER,
			TVPZoomInterpolation ?  D3DTFG_LINEAR : D3DTFG_POINT);
		Direct3DDevice7->SetTextureStageState(0, D3DTSS_MINFILTER,
			TVPZoomInterpolation ?  D3DTFN_LINEAR : D3DTFN_POINT);
		Direct3DDevice7->SetTextureStageState(0, D3DTSS_MIPFILTER,
			TVPZoomInterpolation ?  D3DTFP_LINEAR : D3DTFP_POINT);
		Direct3DDevice7->SetTextureStageState(0, D3DTSS_ADDRESS  , D3DTADDRESS_CLAMP);

		hr = Direct3DDevice7->BeginScene();
		if(FAILED(hr)) goto got_error;

		hr = Direct3DDevice7->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_XYZRHW | D3DFVF_TEX1,
												vertices, 4, D3DDP_WAIT);
		if(FAILED(hr)) goto got_error;

		Direct3DDevice7->EndScene();
		Direct3DDevice7->SetTexture(0, NULL);

#ifdef TVPD3DTIMING
DrawPrimitiveTime += timeGetTime() - StartTick;
#endif
	got_error:
		if(hr == DDERR_SURFACELOST || hr == DDERR_SURFACEBUSY)
		{
			Surface->Restore();
			Texture->Restore();
			InvalidateAll();  // causes reconstruction of off-screen image
		}
		else if(hr == DDERR_INVALIDRECT)
		{
			// ignore this error
		}
		else if(hr != D3D_OK)
		{
			TVPAddImportantLog(
				TJS_W("Passthrough: (inf) Polygon drawing failed/HR=") +
				TJSInt32ToHex(hr, 8));
		}
	}


	void Show()
	{
		if(!TargetWindow) return;
		if(!Texture) return;
		if(!Surface) return;
		if(!Direct3DDevice7) return;
		if(!ShouldShow) return;

		ShouldShow = false;

		HRESULT hr;

		// retrieve the primary surface
		IDirectDrawSurface *pri = TVPGetDDPrimarySurfaceNoAddRef();
		if(!pri)
			TVPThrowExceptionMessage(TJS_W("Cannot retrieve primary surface object"));

		// set clipper
		TVPSetDDPrimaryClipper(Clipper);

#ifdef TVPD3DTIMING
StartTick = timeGetTime();
#endif
		// get PaintBox's origin
		POINT origin; origin.x = DestLeft, origin.y = DestTop;
		ClientToScreen(TargetWindow, &origin);

		// entire of the bitmap is to be transfered (this is not optimal. FIX ME!)
		RECT drect;
		drect.left   = origin.x;
		drect.top    = origin.y;
		drect.right  = origin.x + DestWidth;
		drect.bottom = origin.y + DestHeight;

		RECT srect;
		srect.left   = 0;
		srect.top    = 0;
		srect.right  = DestWidth;
		srect.bottom = DestHeight;

		hr = pri->Blt(&drect, (IDirectDrawSurface*)Surface, &srect, DDBLT_WAIT, NULL);

#ifdef TVPD3DTIMING
BltTime += timeGetTime() - StartTick;
#endif

	got_error:
		if(hr == DDERR_SURFACELOST || hr == DDERR_SURFACEBUSY)
		{
			pri->Restore();
			Surface->Restore();
			Texture->Restore();
			InvalidateAll();  // causes reconstruction of off-screen image
		}
		else if(hr == DDERR_INVALIDRECT)
		{
			// ignore this error
		}
		else if(hr != DD_OK)
		{
			TVPAddImportantLog(
				TJS_W("Passthrough: (inf) Primary surface, IDirectDrawSurface::Blt failed/HR=") +
				TJSInt32ToHex(hr, 8));
		}
	}

	virtual int GetInterpolationCapability()
	{
		// bit 0 for point-on-point, bit 1 for bilinear interpolation
		GetDirect3D7Device();
		if(Direct3DDevice7)
		{
			HRESULT hr;
			D3DDEVICEDESC7 desc;
			ZeroMemory(&desc, sizeof(desc));
			if(SUCCEEDED(Direct3DDevice7->GetCaps(&desc)))
			{
				int caps = 0;
				if(desc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR)
					caps += 2;
				if(desc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFPOINT)
					caps += 1;
				return caps;
			}
			return 3;
		}
		else
		{
			return 3;
		}
	}
};
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
tTVPPassThroughDrawDevice::tTVPPassThroughDrawDevice()
{
	TVPInitPassThroughOptions(); // read and initialize options
	PreferredDrawerType = TVPPreferredDrawType;
	TargetWindow = NULL;
	Drawer = NULL;
	DrawerType = dtNone;
	DestSizeChanged = false;
	SrcSizeChanged = false;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tTVPPassThroughDrawDevice::~tTVPPassThroughDrawDevice()
{
	DestroyDrawer();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tTVPPassThroughDrawDevice::DestroyDrawer()
{
	if(Drawer) delete Drawer, Drawer = NULL;
	DrawerType = dtNone;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tTVPPassThroughDrawDevice::CreateDrawer(tDrawerType type)
{
	if(Drawer) delete Drawer, Drawer = NULL;

	switch(type)
	{
	case dtNone:
		break;
	case dtDrawDib:
		Drawer = new tTVPDrawer_DrawDibNoBuffering(this);
		break;
	case dtDBGDI:
		Drawer = new tTVPDrawer_GDIDoubleBuffering(this);
		break;
	case dtDBDD:
		Drawer = new tTVPDrawer_DDDoubleBuffering(this);
		break;
	case dtDBD3D:
		Drawer = new tTVPDrawer_D3DDoubleBuffering(this);
		break;
	}

	try
	{

		if(Drawer)
			Drawer->SetTargetWindow(TargetWindow);

		if(Drawer)
		{
			if(!Drawer->SetDestPos(DestRect.left, DestRect.top))
			{
				TVPAddImportantLog(
					TJS_W("Passthrough: Failed to set destination position to draw device drawer"));
				delete Drawer, Drawer = NULL;
			}
		}

		if(Drawer)
		{
			tjs_int srcw, srch;
			GetSrcSize(srcw, srch);
			if(!Drawer->SetDestSizeAndNotifyLayerResize(DestRect.get_width(), DestRect.get_height(), srcw, srch))
			{
				TVPAddImportantLog(
					TJS_W("Passthrough: Failed to set destination size and source layer size to draw device drawer"));
				delete Drawer, Drawer = NULL;
			}
		}

		if(Drawer) DrawerType = type; else DrawerType = dtNone;

		RequestInvalidation(tTVPRect(0, 0, DestRect.get_width(), DestRect.get_height()));
	}
/*	catch(const eTJS & e)
	{
		TVPAddImportantLog(TJS_W("Passthrough: Failed to create drawer: ") + e.GetMessage());
		DestroyDrawer();
	}
*/	catch(...)
	{
		TVPAddImportantLog(TJS_W("Passthrough: Failed to create drawer: unknown reason"));
		DestroyDrawer();
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tTVPPassThroughDrawDevice::CreateDrawer(bool zoom_required, bool should_benchmark)
{
	// プライマリレイヤのサイズを取得
	tjs_int srcw, srch;
	GetSrcSize(srcw, srch);

	// いったん Drawer を削除
	tDrawerType last_type = DrawerType;
	DestroyDrawer();

	// プライマリレイヤがないならば DrawDevice は作成しない
	if(srcw == 0 || srch == 0) return;

	// should_benchmark が偽で、前回 Drawer を作成していれば、それと同じタイプの
	// Drawer を用いる
	if(!Drawer && !should_benchmark && last_type != dtNone)
		CreateDrawer(last_type);

	// PreferredDrawerType が指定されていればそれを使う
	if(!Drawer)
	{
		// PreferredDrawerType が dtDrawDib の場合は、ズームが必要な場合は
		// dtGDI を用いる
		if (PreferredDrawerType == dtDrawDib)
			CreateDrawer(zoom_required ? dtDBGDI : dtDrawDib);
		else if(PreferredDrawerType != dtNone)
			CreateDrawer(PreferredDrawerType);
	}

	// もしズームが必要なく、ダブルバッファリングも必要ないならば
	// 一番基本的な DrawDib のやつを使う
	if(!Drawer && !zoom_required && !TVPForceDoublebuffer)
		CreateDrawer(dtDrawDib);

	if(!Drawer)
	{
		// メインウィンドウ以外の場合はズームが必要なければ基本的なメソッドを使う
		if(!IsMainWindow && !zoom_required)
			CreateDrawer(dtDrawDib);
	}

	if(!Drawer)
	{
		// まだ Drawer が作成されてないぜ

		// ベンチマークしますかね
		static tDrawerType bench_types[] = { dtDBDD, dtDBGDI, dtDBD3D };
		const static tjs_char * type_names[] = { TJS_W("DirectDraw"), TJS_W("GDI"), TJS_W("Direct3D") };
		static const int num_types = sizeof(bench_types) / sizeof(bench_types[0]);
		struct tBenchmarkResult
		{
			float score;
			tDrawerType type;
		} results[num_types];

		// ベンチマーク用の元画像を確保
		BITMAPINFOHEADER bmi;
		bmi.biSize = sizeof(BITMAPINFOHEADER);
		bmi.biWidth = srcw;
		bmi.biHeight = srch;
		bmi.biPlanes = 1;
		bmi.biBitCount = 32;
		bmi.biCompression = BI_RGB;
		bmi.biSizeImage = srcw * 4 * srch; // 32bpp の場合はこれでいい
		bmi.biXPelsPerMeter = 0;
		bmi.biYPelsPerMeter = 0;
		bmi.biClrUsed = 0;
		bmi.biClrImportant = 0;

		void * memblk = GlobalAlloc(GMEM_FIXED, bmi.biSizeImage + 64); // 64 = 余裕(無くてもいいかもしれない)
		ZeroMemory(memblk, bmi.biSizeImage);

		tTVPRect cliprect;
		cliprect.left = 0;
		cliprect.top = 0;
		cliprect.right = srcw;
		cliprect.bottom = srch;

		// ベンチマークを行う
		for(int i = 0; i < num_types; i++)
		{
			results[i].type = bench_types[i];
			results[i].score = -1.0f;

			try
			{
				// drawer を作成
				CreateDrawer(results[i].type);
				if(!Drawer)
				{
					TVPAddImportantLog(TJS_W("Passthrough: Could not create drawer object ") + ttstr(type_names[i]));
					continue;
				}

				// ズーム補間の設定は受け入れられるか？
				int caps = Drawer->GetInterpolationCapability();
				if(TVPZoomInterpolation && !(caps & 2))
				{
					TVPAddImportantLog(TJS_W("Passthrough: Drawer object ") + ttstr(type_names[i]) +
						TJS_W(" does not have smooth zooming functionality"));
					continue;
				}
				else if(!TVPZoomInterpolation && !(caps & 1))
				{
					TVPAddImportantLog(TJS_W("Passthrough: Drawer object ") + ttstr(type_names[i]) +
						TJS_W(" does not have point-on-point zooming functionality"));
					continue;
				}

				// ベンチマークを行う
				// 持ち時間約333msで、その間に何回転送を行えるかを見る
				Drawer->InitTimings();
				static const DWORD timeout = 333;
				DWORD start_tick = timeGetTime();
				int count = 0;
				while(timeGetTime() - start_tick < timeout)
				{
					Drawer->StartBitmapCompletion();
					Drawer->NotifyBitmapCompleted(0, 0, memblk, (const BITMAPINFO *)&bmi, cliprect);
					Drawer->EndBitmapCompletion();
					Drawer->Show();
					count ++;
				}
				DWORD end_tick = timeGetTime();
				Drawer->ReportTimings();

				// 結果を格納、それとデバッグ用に表示
				results[i].score = count * 1000 / (float)(end_tick - start_tick);
				char msg[80];
				sprintf(msg, "%.2f fps", (float)results[i].score);
				TVPAddImportantLog(TJS_W("Passthrough: benchmark result: ") + ttstr(type_names[i]) + TJS_W(" : ") +
					msg);

				// GDI は最後の手段
				// 結果だけは計っておくが、これが候補になるのは
				// ほかのdrawerに失敗したときのみ
				if(results[i].type == dtDBGDI)
					results[i].score = 0.0f;

			}
			catch(...)
			{
				DestroyDrawer();
			}
			DestroyDrawer();
		}

		// ベンチマークに使った画像を解放
		GlobalFree((HGLOBAL)memblk);


		// 結果をスコア順にソート
		// そんなに数は多くないので原始的にバブルソート
		while(true)
		{
			bool swapped = false;
			for(int i = 0; i < num_types - 1; i++)
			{
				if(results[i].score < results[i+1].score)
				{
					tBenchmarkResult tmp = results[i];
					results[i] = results[i+1];
					results[i+1] = tmp;
					swapped = true;
				}
			}
			if(!swapped) break;
		}
	
		// スコアの高い順から作成を試みる
		for(int i = 0; i < num_types; i++)
		{
			CreateDrawer(results[i].type);
			if(Drawer) break;
		}

	}

	if(!Drawer)
	{
		// Drawer を全く作成できなかった
		// これはヤバい
		// まずあり得ないが致命的。
		TVPThrowExceptionMessage(TJS_W("Fatal: Could not create any drawer objects."));
	}

	if(Drawer)
	{
		if(IsMainWindow)
			TVPAddImportantLog(TJS_W("Passthrough: Using passthrough draw device: ") + Drawer->GetName());
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tTVPPassThroughDrawDevice::EnsureDrawer()
{
	// このメソッドでは、以下の条件の際に drawer を作る(作り直す)。
	// 1. Drawer が NULL の場合
	// 2. 現在の Drawer のタイプが適切でなくなったとき
	// 3. 元のレイヤのサイズが変更されたとき
	TVPInitPassThroughOptions();

	if(TargetWindow)
	{
		// ズームは必要だったか？
		bool zoom_was_required = false;
		if(Drawer)
		{
			tjs_int srcw, srch;
			Drawer->GetSrcSize(srcw, srch);
			tjs_int destw, desth;
			Drawer->GetDestSize(destw, desth);
			if(destw != srcw || desth != srch)
				zoom_was_required = true;
		}

		// ズームは(今回は)必要か？
		bool zoom_is_required = false;
		tjs_int srcw, srch;
		GetSrcSize(srcw, srch);
		if(DestRect.get_width() != srcw || DestRect.get_height() != srch)
			zoom_is_required = true;


		bool need_recreate = false;
		bool should_benchmark = false;
		if(!Drawer) need_recreate = true;
		if(zoom_was_required != zoom_is_required) need_recreate = true;
		if(need_recreate) should_benchmark = true;
		if(SrcSizeChanged) { SrcSizeChanged = false; need_recreate = true; }
			// SrcSizeChanged という理由だけでは should_benchmark は真には
			// 設定しない

		if(need_recreate)
		{
			// Drawer の再作成が必要
			CreateDrawer(zoom_is_required, should_benchmark);
		}
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPPassThroughDrawDevice::AddLayerManager(iTVPLayerManager * manager)
{
	if(inherited::Managers.size() > 0)
	{
		// "Pass Through" デバイスでは２つ以上のLayer Managerを登録できない
		TVPThrowExceptionMessage(TJS_W("\"passthrough\" device does not support layer manager more than 1"));
			// TODO: i18n
	}
	inherited::AddLayerManager(manager);

	manager->SetDesiredLayerType(ltOpaque); // ltOpaque な出力を受け取りたい
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPPassThroughDrawDevice::SetTargetWindow(HWND wnd, bool is_main)
{
	TVPInitPassThroughOptions();
	DestroyDrawer();
	TargetWindow = wnd;
	IsMainWindow = is_main;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPPassThroughDrawDevice::SetDestRectangle(const tTVPRect & rect)
{
	// 位置だけの変更の場合かどうかをチェックする
	if(rect.get_width() == DestRect.get_width() && rect.get_height() == DestRect.get_height())
	{
		// 位置だけの変更だ
		if(Drawer) Drawer->SetDestPos(rect.left, rect.top);
		inherited::SetDestRectangle(rect);
	}
	else
	{
		// サイズも違う
		DestSizeChanged = true;
		inherited::SetDestRectangle(rect);
		EnsureDrawer();
		if(Drawer)
		{
			if(!Drawer->SetDestSize(rect.get_width(), rect.get_height()))
				DestroyDrawer(); // エラーが起こったのでその drawer を破棄する
		}
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPPassThroughDrawDevice::NotifyLayerResize(iTVPLayerManager * manager)
{
	inherited::NotifyLayerResize(manager);
	SrcSizeChanged = true;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPPassThroughDrawDevice::Show()
{
	if(Drawer) Drawer->Show();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPPassThroughDrawDevice::StartBitmapCompletion(iTVPLayerManager * manager)
{
	EnsureDrawer();

	// この中で DestroyDrawer が呼ばれる可能性に注意すること
	if(Drawer) Drawer->StartBitmapCompletion();

	if(!Drawer)
	{
		// リトライする
		EnsureDrawer();
		if(Drawer) Drawer->StartBitmapCompletion();
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPPassThroughDrawDevice::NotifyBitmapCompleted(iTVPLayerManager * manager,
	tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
	const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity)
{
	// bits, bitmapinfo で表されるビットマップの cliprect の領域を、x, y に描画
	// する。
	// opacity と type は無視するしかないので無視する
	if(Drawer)
	{
		TVPInitPassThroughOptions();
		Drawer->NotifyBitmapCompleted(x, y, bits, bitmapinfo, cliprect);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPPassThroughDrawDevice::EndBitmapCompletion(iTVPLayerManager * manager)
{
	if(Drawer) Drawer->EndBitmapCompletion();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPPassThroughDrawDevice::SetShowUpdateRect(bool b)
{
	if(Drawer) Drawer->SetShowUpdateRect(b);
}
//---------------------------------------------------------------------------



