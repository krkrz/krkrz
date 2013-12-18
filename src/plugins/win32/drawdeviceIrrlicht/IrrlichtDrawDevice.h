#ifndef IRRLICHTDRAWDEVICE_H
#define IRRLICHTDRAWDEVICE_H

#include "IrrlichtBase.h"
#include "BasicDrawDevice.h"

/**
 * Irrlicht ベースの DrawDevice
 */
class IrrlichtDrawDevice : public IrrlichtBase, public tTVPDrawDevice
{
	typedef tTVPDrawDevice inherited;

protected:
	bool zoomMode;
	tjs_int width;        //< ユーザ指定の画面横幅
	tjs_int height;       //< ユーザ指定の画面縦幅
	tjs_int destWidth;    //< 実描画領域の横幅
	tjs_int destHeight;   //< 実描画領域の縦幅

	tjs_int screenWidth;  //< Irrlicht 実画面の画面横幅
	tjs_int screenHeight; //< Irrlicht 実画面の画面縦幅
	irr::core::rect<irr::s32> screenRect;
	irr::core::rect<irr::s32> destRect;

public:
	// コンストラクタ
	IrrlichtDrawDevice(iTJSDispatch2 *objthis, int width, int height);
	// デストラクタ
	virtual ~IrrlichtDrawDevice();

	// -----------------------------------------------------------------------
	// 生成ファクトリ
	// -----------------------------------------------------------------------

	static tjs_error Factory(IrrlichtDrawDevice **obj, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);
	
	// -----------------------------------------------------------------------
	// continuous handler
	// -----------------------------------------------------------------------
public:
	/**
	 * Continuous コールバック
	 */
	virtual void TJS_INTF_METHOD OnContinuousCallback(tjs_uint64 tick);
	
protected:
	// デバイス割り当て後処理
	virtual void onAttach();

	// デバイス解放前処理
	virtual void onDetach();
	
	/**
	 * Device→Irrlicht方向の座標の変換を行う
	 * @param		x		X位置
	 * @param		y		Y位置
	 * @note		x, y は DestRectの (0,0) を原点とする座標として渡されると見なす
	 */
	void transformToIrrlicht(tjs_int &x, tjs_int &y);

	/** Irrlicht→Device方向の座標の変換を行う
	 * @param		x		X位置
	 * @param		y		Y位置
	 * @note		x, y は レイヤの (0,0) を原点とする座標として渡されると見なす
	 */
	void transformFromIrrlicht(tjs_int &x, tjs_int &y);

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
	
	// ------------------------------------------------------------
	// 更新処理
	// ------------------------------------------------------------
protected:
	/**
	 * クラス固有更新処理
	 * シーンマネージャの処理後、GUIの処理前に呼ばれる
	 */
	void update(irr::video::IVideoDriver *driver);
	
public:
	//---- LayerManager の管理関連
	virtual void TJS_INTF_METHOD AddLayerManager(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD RemoveLayerManager(iTVPLayerManager * manager);

	//---- 描画位置・サイズ関連
	virtual void TJS_INTF_METHOD SetTargetWindow(HWND wnd, bool is_main);
	virtual void TJS_INTF_METHOD SetDestRectangle(const tTVPRect & rect);
	virtual void TJS_INTF_METHOD GetSrcSize(tjs_int &w, tjs_int &h);
	virtual void TJS_INTF_METHOD NotifyLayerResize(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD NotifyLayerImageChange(iTVPLayerManager * manager) {}

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
	
	//---- LayerManager からの画像受け渡し関連
	virtual void TJS_INTF_METHOD StartBitmapCompletion(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD NotifyBitmapCompleted(iTVPLayerManager * manager,
		tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
		const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity);
	virtual void TJS_INTF_METHOD EndBitmapCompletion(iTVPLayerManager * manager);

	// -----------------------------------------------------------------------
	// 共通メソッド呼び出し用
	// -----------------------------------------------------------------------

public:
	void setEventMask(int mask) {
		IrrlichtBase::setEventMask(mask);
	}

	int getEventMask() {
		return IrrlichtBase::getEventMask();
	}

	irr::video::IVideoDriver *getVideoDriver() {
		return IrrlichtBase::getVideoDriver();
	}

	irr::scene::ISceneManager *getSceneManager() {
		return IrrlichtBase::getSceneManager();
	}

	irr::gui::IGUIEnvironment *getGUIEnvironment() {
		return IrrlichtBase::getGUIEnvironment();
	}

	irr::ILogger *getLogger() {
		return IrrlichtBase::getLogger();
	}

	irr::io::IFileSystem *getFileSystem() {
		return IrrlichtBase::getFileSystem();
	}
	
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

	bool getZoomMode() {
		return zoomMode;
	}
	
	void setZoomMode(bool zoomMode) {
		this->zoomMode = zoomMode;
		Window->NotifySrcResize();
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
