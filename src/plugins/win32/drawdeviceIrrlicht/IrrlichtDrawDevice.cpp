#include "IrrlichtDrawDevice.h"
#include "LayerManagerInfo.h"

extern void message_log(const char* format, ...);
extern void error_log(const char *format, ...);

using namespace irr;
using namespace core;
using namespace video;
using namespace scene;
using namespace io;
using namespace gui;

/**
 * コンストラクタ
 */
IrrlichtDrawDevice::IrrlichtDrawDevice(iTJSDispatch2 *objthis, int width, int height)
	: IrrlichtBase(objthis), width(width), height(height), destWidth(0), destHeight(0), zoomMode(true), defaultVisible(true)
{
	// Irrlicht的画面サイズ
	screenWidth = width;
	screenHeight = height;
	screenRect = rect<s32>(0,0,screenWidth,screenHeight);
}

/**
 * デストラクタ
 */
IrrlichtDrawDevice::~IrrlichtDrawDevice()
{
	stop();
	detach();
}

/**
 * 生成ファクトリ
 */
tjs_error
IrrlichtDrawDevice::Factory(IrrlichtDrawDevice **obj, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	if (numparams < 2) {
		return TJS_E_BADPARAMCOUNT;
	}
	*obj = new IrrlichtDrawDevice(objthis, (tjs_int)*param[0], (tjs_int)*param[1]);
	return TJS_S_OK;
}

// -----------------------------------------------------------------------
// Continuous
// -----------------------------------------------------------------------

/**
 * Continuous コールバック
 * 吉里吉里が暇なときに常に呼ばれる
 */
void TJS_INTF_METHOD
IrrlichtDrawDevice::OnContinuousCallback(tjs_uint64 tick)
{
	Window->RequestUpdate();
}

void
IrrlichtDrawDevice::onAttach()
{
	if (device) {
		Window->NotifySrcResize(); // これを呼ぶことで GetSrcSize(), SetDestRectangle() の呼び返しが来る
		// マネージャに対するテクスチャの割り当て
		IVideoDriver *driver = device->getVideoDriver();
		if (driver) {
			for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
				iTVPLayerManager *manager = *i;
				LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
				if (info != NULL) {
					info->alloc(manager, driver);
				}
			}
		}
	}
}

void
IrrlichtDrawDevice::onDetach()
{
	for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
		LayerManagerInfo *info = (LayerManagerInfo*)(*i)->GetDrawDeviceData();
		if (info) {
			info->free();
		}
	}
}

/**
 * Device→Irrlicht方向の座標の変換を行う
 * @param		x		X位置
 * @param		y		Y位置
 * @note		x, y は DestRectの (0,0) を原点とする座標として渡されると見なす
 */
void
IrrlichtDrawDevice::transformToIrrlicht(tjs_int &x, tjs_int &y)
{
	x = screenWidth  ? (x * destWidth  / screenWidth) : 0;
	y = screenHeight ? (y * destHeight / screenHeight) : 0;
}

/** Irrlicht→Device方向の座標の変換を行う
 * @param		x		X位置
 * @param		y		Y位置
 * @note		x, y は レイヤの (0,0) を原点とする座標として渡されると見なす
 */
void
IrrlichtDrawDevice::transformFromIrrlicht(tjs_int &x, tjs_int &y)
{
	x = destWidth  ? (x * screenWidth  / destWidth) : 0;
	y = destHeight ? (y * screenHeight / destHeight) : 0;
}

/**
 * Device→プライマリレイヤの座標の変換を行う
 * @param		x		X位置
 * @param		y		Y位置
 * @note		x, y は DestRectの (0,0) を原点とする座標として渡されると見なす
 */
void
IrrlichtDrawDevice::transformToManager(iTVPLayerManager * manager, tjs_int &x, tjs_int &y)
{
	// プライマリレイヤマネージャのプライマリレイヤのサイズを得る
	tjs_int pl_w, pl_h;
	manager->GetPrimaryLayerSize(pl_w, pl_h);
	x = destWidth  ? (x * pl_w / destWidth) : 0;
	y = destHeight ? (y * pl_h / destHeight) : 0;
}

/** プライマリレイヤ→Device方向の座標の変換を行う
 * @param		x		X位置
 * @param		y		Y位置
 * @note		x, y は レイヤの (0,0) を原点とする座標として渡されると見なす
 */
void
IrrlichtDrawDevice::transformFromManager(iTVPLayerManager * manager, tjs_int &x, tjs_int &y)
{
	// プライマリレイヤマネージャのプライマリレイヤのサイズを得る
	tjs_int pl_w, pl_h;
	manager->GetPrimaryLayerSize(pl_w, pl_h);
	x = pl_w ? (x * destWidth  / pl_w) : 0;
	y = pl_h ? (y * destHeight / pl_h) : 0;
}

/**
 * Device→標準画面の座標の変換を行う
 * @param		x		X位置
 * @param		y		Y位置
 * @note		x, y は DestRectの (0,0) を原点とする座標として渡されると見なす
 */
void
IrrlichtDrawDevice::transformTo(tjs_int &x, tjs_int &y)
{
	x = destWidth  ? (x * width / destWidth) : 0;
	y = destHeight ? (y * height / destHeight) : 0;
}

/** 標準画面→Device方向の座標の変換を行う
 * @param		x		X位置
 * @param		y		Y位置
 * @note		x, y は レイヤの (0,0) を原点とする座標として渡されると見なす
 */
void
IrrlichtDrawDevice::transformFrom(tjs_int &x, tjs_int &y)
{
	// プライマリレイヤマネージャのプライマリレイヤのサイズを得る
	x = width ? (x * destWidth  / width) : 0;
	y = height ? (y * destHeight / height) : 0;
}

/**
 * 固有更新処理
 */
void
IrrlichtDrawDevice::update(irr::video::IVideoDriver *driver)
{
	// 個別レイヤマネージャの描画
	for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
		LayerManagerInfo *info = (LayerManagerInfo*)(*i)->GetDrawDeviceData();
		if (info) {
			info->draw(driver, screenRect);
		}
	}
}


/**
 * レイヤマネージャの登録
 * @param manager レイヤマネージャ
 */
void TJS_INTF_METHOD
IrrlichtDrawDevice::AddLayerManager(iTVPLayerManager * manager)
{
	int id = (int)Managers.size();
	tTVPDrawDevice::AddLayerManager(manager);
	LayerManagerInfo *info = new LayerManagerInfo(id, defaultVisible);
	manager->SetDrawDeviceData((void*)info);
}

/**
 * レイヤマネージャの削除
 * @param manager レイヤマネージャ
 */
void TJS_INTF_METHOD
IrrlichtDrawDevice::RemoveLayerManager(iTVPLayerManager * manager)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info != NULL) {
		manager->SetDrawDeviceData(NULL);
		delete info;
	}
	tTVPDrawDevice::RemoveLayerManager(manager);
}

/***
 * ウインドウの指定
 * @param wnd ウインドウハンドラ
 */
void TJS_INTF_METHOD
IrrlichtDrawDevice::SetTargetWindow(HWND wnd, bool is_main)
{
	stop();
	detach();
	if (wnd != NULL) {
		attach(wnd, screenWidth, screenHeight);
		start();
	}
}

void TJS_INTF_METHOD
IrrlichtDrawDevice::SetDestRectangle(const tTVPRect &dest)
{
	destRect.setLeft(dest.left);
	destRect.setTop(dest.top);
	destRect.setWidth((destWidth = dest.get_width()));
	destRect.setHeight((destHeight = dest.get_height()));
	if (device) {
		IVideoDriver *driver = device->getVideoDriver();
		if (driver) {
			tjs_int w, h;
			if (zoomMode) {
				w = width;
				h = height;
			} else {
				w = destWidth;
				h = destHeight;
			}
			if (screenWidth != w ||	screenHeight != h) {
				screenWidth = w;
				screenHeight = h;
				screenRect = rect<s32>(0,0,screenWidth,screenHeight);
				driver->OnResize(dimension2d<s32>(w, h));
			}
		}
	}
}

void TJS_INTF_METHOD
IrrlichtDrawDevice::GetSrcSize(tjs_int &w, tjs_int &h)
{
	w = width;
	h = height;
}

void TJS_INTF_METHOD
IrrlichtDrawDevice::NotifyLayerResize(iTVPLayerManager * manager)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info != NULL) {
		info->free();
		if (device) {
			IVideoDriver *driver = device->getVideoDriver();
			if (driver) {
				info->alloc(manager, driver);
			}
		}
	}
}

// -------------------------------------------------------------------------------------
// 入力イベント処理用
// -------------------------------------------------------------------------------------

void TJS_INTF_METHOD
IrrlichtDrawDevice::OnMouseDown(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags)
{
	// Irrlicht に送る
	if (device) {
		tjs_int dx = x;
		tjs_int dy = y;
		transformToIrrlicht(dx, dy);
		SEvent ev;
		ev.EventType = EET_MOUSE_INPUT_EVENT;
		ev.MouseInput.X = dx;
		ev.MouseInput.Y = dy;
		ev.MouseInput.Wheel = 0;
		switch ((mb & 0xff)) {
		case mbLeft:
			ev.MouseInput.Event = EMIE_LMOUSE_PRESSED_DOWN;
			break;
		case mbMiddle:
			ev.MouseInput.Event = EMIE_MMOUSE_PRESSED_DOWN;
			break;
		case mbRight:
			ev.MouseInput.Event = EMIE_RMOUSE_PRESSED_DOWN;
			break;
		}
		postEvent(ev);
	}
	// 吉里吉里のプライマリレイヤに送る
	iTVPLayerManager * manager = GetLayerManagerAt(PrimaryLayerManagerIndex);
	if (manager) {
		transformToManager(manager, x, y);
		manager->NotifyMouseDown(x, y, mb, flags);
	}
}

void TJS_INTF_METHOD
IrrlichtDrawDevice::OnMouseUp(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags)
{
	// Irrlicht に送る
	if (device) {
		tjs_int dx = x;
		tjs_int dy = y;
		transformToIrrlicht(dx, dy);
		SEvent ev;
		ev.EventType = EET_MOUSE_INPUT_EVENT;
		ev.MouseInput.X = dx;
		ev.MouseInput.Y = dy;
		ev.MouseInput.Wheel = 0;
		switch ((mb & 0xff)) {
		case mbLeft:
			ev.MouseInput.Event = EMIE_LMOUSE_LEFT_UP;
			break;
		case mbMiddle:
			ev.MouseInput.Event = EMIE_MMOUSE_LEFT_UP;
			break;
		case mbRight:
			ev.MouseInput.Event = EMIE_RMOUSE_LEFT_UP;
			break;
		}
		postEvent(ev);
	}
	// 吉里吉里のプライマリレイヤに送る
	iTVPLayerManager * manager = GetLayerManagerAt(PrimaryLayerManagerIndex);
	if (manager) {
		transformToManager(manager, x, y);
		manager->NotifyMouseUp(x, y, mb, flags);
	}
}

void TJS_INTF_METHOD
IrrlichtDrawDevice::OnMouseMove(tjs_int x, tjs_int y, tjs_uint32 flags)
{
	// Irrlicht に送る
	if (device) {
		tjs_int dx = x;
		tjs_int dy = y;
		transformToIrrlicht(dx, dy);
		SEvent ev;
		ev.EventType = EET_MOUSE_INPUT_EVENT;
		ev.MouseInput.X = dx;
		ev.MouseInput.Y = dy;
		ev.MouseInput.Wheel = 0;
		ev.MouseInput.Event = EMIE_MOUSE_MOVED;
		postEvent(ev);
	}
	// 吉里吉里のプライマリレイヤに送る
	iTVPLayerManager * manager = GetLayerManagerAt(PrimaryLayerManagerIndex);
	if (manager) {
		transformToManager(manager, x, y);
		manager->NotifyMouseMove(x, y, flags);
	}
}

void TJS_INTF_METHOD
IrrlichtDrawDevice::OnMouseWheel(tjs_uint32 shift, tjs_int delta, tjs_int x, tjs_int y)
{
	// Irrlicht に送る
	if (device) {
		tjs_int dx = x;
		tjs_int dy = y;
		transformToIrrlicht(dx, dy);
		SEvent ev;
		ev.EventType = EET_MOUSE_INPUT_EVENT;
		ev.MouseInput.X = dx;
		ev.MouseInput.Y = dy;
		ev.MouseInput.Wheel = (f32)delta;
		ev.MouseInput.Event = EMIE_MOUSE_WHEEL;
		postEvent(ev);
	}
	// 吉里吉里のプライマリレイヤに送る
	iTVPLayerManager * manager = GetLayerManagerAt(PrimaryLayerManagerIndex);
	if (manager) {
		transformToManager(manager, x, y);
		manager->NotifyMouseWheel(shift, delta, x, y);
	}
}

void TJS_INTF_METHOD
IrrlichtDrawDevice::GetCursorPos(iTVPLayerManager * manager, tjs_int &x, tjs_int &y)
{
	Window->GetCursorPos(x, y);
	transformToManager(manager, x, y);
}

void TJS_INTF_METHOD
IrrlichtDrawDevice::SetCursorPos(iTVPLayerManager * manager, tjs_int x, tjs_int y)
{
	transformFromManager(manager, x, y);
	Window->SetCursorPos(x, y);
}

void TJS_INTF_METHOD
IrrlichtDrawDevice::RequestInvalidation(const tTVPRect & rect)
{
	for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
		iTVPLayerManager *manager = *i;
		LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
		if (info && info->visible) {
			tjs_int l = rect.left, t = rect.top, r = rect.right, b = rect.bottom;
			transformToManager(manager, l, t);
			transformToManager(manager, r, b);
			r ++; // 誤差の吸収(本当はもうちょっと厳密にやらないとならないがそれが問題になることはない)
			b ++;
			manager->RequestInvalidation(tTVPRect(l, t, r, b));
		}
	}
}


// -------------------------------------------------------------------------------------
// 再描画処理用
// -------------------------------------------------------------------------------------

void
IrrlichtDrawDevice::Show()
{
	show(&destRect);
}

// -------------------------------------------------------------------------------------
// LayerManagerからの画像うけわたし
// -------------------------------------------------------------------------------------

/**
 * ビットマップコピー処理開始
 */
void TJS_INTF_METHOD
IrrlichtDrawDevice::StartBitmapCompletion(iTVPLayerManager * manager)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info) {
		info->lock();
	}
}

/**
 * ビットマップコピー処理
 */
void TJS_INTF_METHOD
IrrlichtDrawDevice::NotifyBitmapCompleted(iTVPLayerManager * manager,
	tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
	const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info) {
		info->copy(x, y, bits, bitmapinfo, cliprect, type, opacity);
	}
}

/**
 * ビットマップコピー処理終了
 */
void TJS_INTF_METHOD
IrrlichtDrawDevice::EndBitmapCompletion(iTVPLayerManager * manager)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info) {
		info->unlock();
	}
}

/**
 * プライマリレイヤの表示状態の指定
 * @param id プライマリレイヤの登録ID
 * @param visible 表示状態
 */
void
IrrlichtDrawDevice::setVisible(int id, bool visible)
{
	if (id >= 0 && id < (int)Managers.size()) {
		LayerManagerInfo *info = (LayerManagerInfo*)Managers[id]->GetDrawDeviceData();
		if (info) {
			info->visible = visible;
		}
	}
}

/**
 * プライマリレイヤの表示状態の指定
 * @param id プライマリレイヤの登録ID
 * @return visible 表示状態
 */
bool
IrrlichtDrawDevice::getVisible(int id)
{
	if (id >= 0 && id < (int)Managers.size()) {
		LayerManagerInfo *info = (LayerManagerInfo*)Managers[id]->GetDrawDeviceData();
		if (info) {
			return info->visible;
		}
	}
	return false;
}
