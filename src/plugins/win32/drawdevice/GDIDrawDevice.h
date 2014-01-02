
#ifndef GDI_DRAW_DEVICE_H
#define GDI_DRAW_DEVICE_H

#include "tp_stub.h"
#include "BasicDrawDevice.h"

//---------------------------------------------------------------------------
//! @brief		「GDI」デバイス
//---------------------------------------------------------------------------
class tTVPGDIDrawDevice : public tTVPDrawDevice
{
	typedef tTVPDrawDevice inherited;

	HWND TargetWindow;
	bool IsMainWindow;
	HDRAWDIB DrawDibHandle;
	bool DrawUpdateRectangle;
	HDC TargetDC;
	HBITMAP OffScreenBitmap; //!< オフスクリーンビットマップ
	HDC OffScreenDC; //!< オフスクリーン DC
	HBITMAP OldOffScreenBitmap; //!< OffScreenDC に以前選択されていた ビットマップ
	bool ShouldShow; //!< show で実際に画面に画像を転送すべきか


public:
	tTVPGDIDrawDevice(); //!< コンストラクタ

private:
	~tTVPGDIDrawDevice(); //!< デストラクタ

	bool IsTargetWindowActive() const;

	void EnsureDrawer();
	void DestroyBitmap();
	void CreateBitmap();

public:
	void SetToRecreateDrawer() { DestroyBitmap(); }

public:
	void EnsureDevice();

//---- LayerManager の管理関連
	virtual void TJS_INTF_METHOD AddLayerManager(iTVPLayerManager * manager);

//---- 描画位置・サイズ関連
	virtual void TJS_INTF_METHOD SetTargetWindow(HWND wnd, bool is_main);
	virtual void TJS_INTF_METHOD SetDestRectangle(const tTVPRect & rect);
	virtual void TJS_INTF_METHOD NotifyLayerResize(iTVPLayerManager * manager);

//---- 再描画関連
	virtual void TJS_INTF_METHOD Show();

//---- LayerManager からの画像受け渡し関連
	virtual void TJS_INTF_METHOD StartBitmapCompletion(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD NotifyBitmapCompleted(iTVPLayerManager * manager,
		tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
		const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity);
	virtual void TJS_INTF_METHOD EndBitmapCompletion(iTVPLayerManager * manager);

//---- デバッグ支援
	virtual void TJS_INTF_METHOD SetShowUpdateRect(bool b);
};
//---------------------------------------------------------------------------



#endif // GDI_DRAW_DEVICE_H