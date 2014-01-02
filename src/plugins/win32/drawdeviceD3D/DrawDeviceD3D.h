#ifndef DrawDeviceD3D_H
#define DrawDeviceD3D_H

#include <windows.h>
#include <stdio.h>
#include <ddraw.h>
#include <d3d9.h>
#include "BasicDrawDevice.h"

/**
 * D3D ベースの DrawDevice
 */
class DrawDeviceD3D : public tTVPDrawDevice
{
	typedef tTVPDrawDevice inherited;

protected:
	tjs_int width;        //< ユーザ指定の画面横幅
	tjs_int height;       //< ユーザ指定の画面縦幅
	tjs_int destTop;      //< 実描画領域の横幅
	tjs_int destLeft;     //< 実描画領域の縦幅
	tjs_int destWidth;    //< 実描画領域の横幅
	tjs_int destHeight;   //< 実描画領域の縦幅
	
	HWND targetWindow;
	bool isMainWindow;
	bool drawUpdateRectangle;
	bool backBufferDirty;

	HWND hWnd;
	
	IDirect3D9*				direct3D;
	IDirect3DDevice9*		direct3DDevice;
	D3DPRESENT_PARAMETERS	d3dPP;
	D3DDISPLAYMODE			dispMode;
	
	UINT		currentMonitor;
	bool		shouldShow; //!< show で実際に画面に画像を転送すべきか
	tjs_uint	vsyncInterval;
public:
	// コンストラクタ
	DrawDeviceD3D(int width, int height);
	// デストラクタ
	virtual ~DrawDeviceD3D();

protected:
	
	void InvalidateAll();
	UINT GetMonitorNumber( HWND window );

	bool IsTargetWindowActive() const;

	HRESULT DecideD3DPresentParameters();

	void DestroyD3DDevice();
	void TryRecreateWhenDeviceLost();

	// デバイス割り当て
	void attach(HWND hwnd);

	// デバイス解放処理
	void detach();
	
	/**
	 * Device→レイヤマネージャの座標の変換を行う
	 * @param		x		X位置
	 * @param		y		Y位置
	 * @note		x, y は DestRectの (0,0) を原点とする座標として渡されると見なす
	 */
	void transformToManager(iTVPLayerManager * manager, tjs_int &x, tjs_int &y);

	/** レイヤマネージャ→Device方向の座標の変換を行う
	 * @param		x		X位置
	 * @param		y		Y位置
	 * @note		x, y は レイヤの (0,0) を原点とする座標として渡されると見なす
	 */
	void transformFromManager(iTVPLayerManager * manager, tjs_int &x, tjs_int &y);

	/**
	 * Device→標準座標の変換を行う
	 * @param		x		X位置
	 * @param		y		Y位置
	 * @note		x, y は DestRectの (0,0) を原点とする座標として渡されると見なす
	 */
	void transformTo(tjs_int &x, tjs_int &y);
	
	/** 標準座標→Device方向の座標の変換を行う
	 * @param		x		X位置
	 * @param		y		Y位置
	 * @note		x, y は レイヤの (0,0) を原点とする座標として渡されると見なす
	 */
	void transformFrom(tjs_int &x, tjs_int &y);
	
public:
	//---- LayerManager の管理関連
	virtual void TJS_INTF_METHOD AddLayerManager(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD RemoveLayerManager(iTVPLayerManager * manager);

	//---- 描画位置・サイズ関連
	virtual void TJS_INTF_METHOD SetTargetWindow(HWND wnd, bool is_main);
	virtual void TJS_INTF_METHOD SetDestRectangle(const tTVPRect & rect);
	virtual void TJS_INTF_METHOD GetSrcSize(tjs_int &w, tjs_int &h);
	virtual void TJS_INTF_METHOD NotifyLayerResize(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD NotifyLayerImageChange(iTVPLayerManager * manager);

	//---- ユーザーインターフェース関連
	// window → drawdevice
	virtual void TJS_INTF_METHOD OnMouseDown(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags);
	virtual void TJS_INTF_METHOD OnMouseUp(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags);
	virtual void TJS_INTF_METHOD OnMouseMove(tjs_int x, tjs_int y, tjs_uint32 flags);
	virtual void TJS_INTF_METHOD OnMouseWheel(tjs_uint32 shift, tjs_int delta, tjs_int x, tjs_int y);

	virtual void TJS_INTF_METHOD GetCursorPos(iTVPLayerManager * manager, tjs_int &x, tjs_int &y);
	virtual void TJS_INTF_METHOD SetCursorPos(iTVPLayerManager * manager, tjs_int x, tjs_int y);
	virtual void TJS_INTF_METHOD RequestInvalidation(const tTVPRect & rect);
	
	//---- 再描画関連
	virtual void TJS_INTF_METHOD Show();
	virtual bool TJS_INTF_METHOD WaitForVBlank( tjs_int* in_vblank, tjs_int* delayed );
	
	//---- LayerManager からの画像受け渡し関連
	virtual void TJS_INTF_METHOD StartBitmapCompletion(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD NotifyBitmapCompleted(iTVPLayerManager * manager,
		tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
		const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity);
	virtual void TJS_INTF_METHOD EndBitmapCompletion(iTVPLayerManager * manager);
	
//---- フルスクリーン
	virtual bool TJS_INTF_METHOD SwitchToFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color, bool changeresolution );
	virtual void TJS_INTF_METHOD RevertFromFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color );
	// -----------------------------------------------------------------------
	// 固有メソッド
	// -----------------------------------------------------------------------
	
public:
	/**
	 * @return デバイス情報
	 */
	tjs_int64 getDevice() {
		return reinterpret_cast<tjs_int64>((tTVPDrawDevice*)this);
	}

	tjs_int getWidth() {
		return width;
	}
	
	void setWidth(tjs_int width) {
		this->width = width;
		Window->NotifySrcResize();
	}

	tjs_int getHeight() {
		return height;
	}
	
	void setHeight(tjs_int height) {
		this->height = height;
		Window->NotifySrcResize();
	}

	void setSize(tjs_int width, tjs_int height) {
		this->width = width;
		this->height = height;
		Window->NotifySrcResize();
	}

	tjs_int getDestWidth() {
		return destWidth;
	}

	tjs_int getDestHeight() {
		return destHeight;
	}
	
protected:
	/*
	 * プライマリレイヤの標準の visible
	 */
	bool defaultVisible;

public:
	void setDefaultVisible(bool visible) {
		defaultVisible = visible;
	}
	
	bool getDefaultVisible() {
		return defaultVisible;
	}

	/**
	 * プライマリレイヤの表示状態の指定
	 * @param id プライマリレイヤの登録ID
	 * @param visible 表示状態
	 */
	void setVisible(int id, bool visible);

	/**
	 * プライマリレイヤの表示状態の指定
	 * @param id プライマリレイヤの登録ID
	 * @return visible 表示状態
	 */
	bool getVisible(int id);
	
};

#endif
