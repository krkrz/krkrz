

#ifndef __GDI_DRAWER_H__
#define __GDI_DRAWER_H__

#include "Drawer.h"
#include <windows.h>

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
	tTVPDrawer_GDI(tTVPPassThroughDrawDevice * device);
	~tTVPDrawer_GDI();
	void SetTargetWindow(HWND wnd);
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
	tTVPDrawer_DrawDibNoBuffering(tTVPPassThroughDrawDevice * device);
	~tTVPDrawer_DrawDibNoBuffering();
	virtual ttstr GetName();
	bool SetDestSize(tjs_int width, tjs_int height);
	bool NotifyLayerResize(tjs_int w, tjs_int h);
	void SetTargetWindow(HWND wnd);
	void StartBitmapCompletion();
	void NotifyBitmapCompleted(tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo, const tTVPRect &cliprect);
	void EndBitmapCompletion();
	int GetInterpolationCapability();

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
	tTVPDrawer_GDIDoubleBuffering(tTVPPassThroughDrawDevice * device);
	~tTVPDrawer_GDIDoubleBuffering();
	ttstr GetName();
	void DestroyBitmap();
	void CreateBitmap();
	bool SetDestSize(tjs_int width, tjs_int height);
	bool NotifyLayerResize(tjs_int w, tjs_int h);
	void SetTargetWindow(HWND wnd);
	void StartBitmapCompletion();
	void NotifyBitmapCompleted(tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo, const tTVPRect &cliprect);
	void EndBitmapCompletion();
	void Show();
	virtual int GetInterpolationCapability();
	virtual void InitTimings();
	virtual void ReportTimings();
};
//---------------------------------------------------------------------------

#endif // __GDI_DRAWER_H__
