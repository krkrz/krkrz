#include "IrrlichtBase.h"
#include "ncbind/ncbind.hpp"

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
IrrlichtBase::IrrlichtBase(iTJSDispatch2 *objthis)
	: objthis(objthis), device(NULL), attached(false),
	  eventMask(EMASK_ATTACH | EMASK_DETACH)
{
}

/**
 * デストラクタ
 */
IrrlichtBase::~IrrlichtBase()
{
	stop();
	detach();
}

/**
 * ドライバの情報表示
 */
void
IrrlichtBase::showDriverInfo()
{
	IVideoDriver *driver = device->getVideoDriver();
	if (driver) {
		dimension2d<s32> size = driver->getScreenSize();
		message_log("デバイス生成後のスクリーンサイズ:%d, %d", size.Width, size.Height);
		size = driver->getCurrentRenderTargetSize();
		message_log("デバイス生成後のRenderTargetの:%d, %d", size.Width, size.Height);
	}
}

/**
 * TJSイベント呼び出し。自己オブジェクトの該当メソッドを呼び出す。
 * @param eventName イベント名
 */
void
IrrlichtBase::sendTJSEvent(const tjs_char *eventName)
{
	tTJSVariant method;
	if (TJS_SUCCEEDED(objthis->PropGet(0, eventName, NULL, &method, objthis))) { // イベントメソッドを取得
		if (method.Type() == tvtObject) {
			iTJSDispatch2 *m = method.AsObjectNoAddRef();
			if (TJS_SUCCEEDED(m->IsInstanceOf(0, NULL, NULL, L"Function", m))) { // ファンクションかどうか
				tTJSVariant self(objthis, objthis);
				tTJSVariant *params[] = {&self};
				m->FuncCall(0, NULL, NULL, NULL, 1, params, method.AsObjectThisNoAddRef());
			}
		}
	}
}

/**
 * デバイスの割り当て
 * @param hwnd 親ウインドウハンドル
 * @param width バックバッファサイズ横幅
 * @param height バックバッファサイズ縦幅
 */
void
IrrlichtBase::attach(HWND hwnd, int width, int height)
{
	if (!attached && hwnd) {
		// デバイス生成
		SIrrlichtCreationParameters params;
		params.WindowId     = reinterpret_cast<void*>(hwnd);
		params.DriverType    = EDT_DIRECT3D9;
		params.Stencilbuffer = true;
		params.Vsync = true;
		params.EventReceiver = this;
		params.AntiAlias = true;
		if (width != 0 && height != 0) {
			params.WindowSize = core::dimension2d<s32>(width, height);
		}
		if ((device = irr::createDeviceEx(params))) {
			TVPAddLog(L"Irrlichtデバイス初期化");
			// テクスチャのα合成時にも常にZテストを行うように。
			// device->getSceneManager()->getParameters()->setAttribute(scene::ALLOW_ZWRITE_ON_TRANSPARENT, true);
			showDriverInfo();
			onAttach();
			if ((eventMask & EMASK_ATTACH)) {
				sendTJSEvent(L"onAttach");
			}
		} else {
			TVPThrowExceptionMessage(L"Irrlicht デバイスの初期化に失敗しました");
		}
		attached = true;
	}
}

/**
 * デバイスの破棄
 */
void
IrrlichtBase::detach()
{
	if (device) {
		if ((eventMask & EMASK_DETACH)) {
			sendTJSEvent(L"onDetach");
		}
		onDetach();
		device->drop();
		device = NULL;
	}
	attached = false;
}

/**
 * Irrlicht描画処理
 * @param destRect 描画先領域
 * @param srcRect 描画元領域
 * @param destDC 描画先DC
 * @return 描画された
 */
bool
IrrlichtBase::show(irr::core::rect<irr::s32> *destRect, irr::core::rect<irr::s32> *srcRect, HDC destDC)
{
	if (device) {
		// 時間を進める XXX tick を外部から与えられないか？
		device->getTimer()->tick();
		
		IVideoDriver *driver = device->getVideoDriver();
		// 描画開始
		if (driver && driver->beginScene(true, true, irr::video::SColor(0,0,0,0))) {
			
			if ((eventMask & EMASK_BEFORE_SCENE)) {
				sendTJSEvent(L"onBeforeScene");
			}
			
			/// シーンマネージャの描画
			ISceneManager *smgr = device->getSceneManager();
			if (smgr) {
				smgr->drawAll();
			}

			if ((eventMask & EMASK_AFTER_SCENE)) {
				sendTJSEvent(L"onAfterScene");
			}
			
			// 固有処理
			update(driver);

			if ((eventMask & EMASK_BEFORE_GUI)) {
				sendTJSEvent(L"onBeforeGUI");
			}
			
			// GUIの描画
			IGUIEnvironment *gui = device->getGUIEnvironment();
			if (gui) {
				gui->drawAll();
			}

			if ((eventMask & EMASK_AFTER_GUI)) {
				sendTJSEvent(L"onAfterGUI");
			}
			
			// 描画完了
			driver->endScene(0, srcRect, destRect, destDC);
			return true;
		}
	}
	return false;
};

/**
 * Irrlicht へのイベント送信
 */
bool
IrrlichtBase::postEvent(SEvent &ev)
{
	if (device) {
		if (device->getGUIEnvironment()->postEventFromUser(ev) ||
			device->getSceneManager()->postEventFromUser(ev)) {
			return true;
		}
	}
	return false;
}

/**
 * イベント受理
 * HWND を指定して生成している関係で Irrlicht 自身はウインドウから
 * イベントを取得することはない。ので GUI Environment からのイベント
 * だけがここにくることになる。自分の適当なメソッドを呼び出すように要修正 XXX
 * @return 処理したら true
 */
bool
IrrlichtBase::OnEvent(const irr::SEvent &event)
{
	bool ret = false;
	if ((eventMask & EMASK_EVENT)) {
		tTJSVariant method;
		if (TJS_SUCCEEDED(objthis->PropGet(0, L"onEvent", NULL, &method, objthis))) { // イベントメソッドを取得
			if (method.Type() == tvtObject) {
				iTJSDispatch2 *m = method.AsObjectNoAddRef();
				if (TJS_SUCCEEDED(m->IsInstanceOf(0, NULL, NULL, L"Function", m))) { // ファンクションかどうか
					tTJSVariant self(objthis, objthis);
					tTJSVariant ev;

					// SEvent を変換
					typedef ncbInstanceAdaptor<SEvent> AdaptorT;
					iTJSDispatch2 *adpobj = AdaptorT::CreateAdaptor(new SEvent(event));
					if (adpobj) {
						ev = tTJSVariant(adpobj, adpobj);
						adpobj->Release();
					}

					tTJSVariant *params[] = {&self, &ev};
					tTJSVariant result;
					m->FuncCall(0, NULL, NULL, &result, 2, params, method.AsObjectThisNoAddRef());
					ret = (tjs_int)result != 0;
				}
			}
		}
	}
	return ret;
}

// --------------------------------------------------------------------------------

void
IrrlichtBase::start()
{
	stop();
	TVPAddContinuousEventHook(this);
}

void
IrrlichtBase::stop()
{
	TVPRemoveContinuousEventHook(this);
}
