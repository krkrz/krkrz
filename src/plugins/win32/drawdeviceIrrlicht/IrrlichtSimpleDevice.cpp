#include "ncbind/ncbind.hpp"
#include "IrrlichtSimpleDevice.h"

extern void message_log(const char* format, ...);
extern void error_log(const char *format, ...);

using namespace irr;
using namespace core;
using namespace video;
using namespace scene;
using namespace io;
using namespace gui;

// イベント処理
bool __stdcall
IrrlichtSimpleDevice::messageHandler(void *userdata, tTVPWindowMessage *Message)
{
	IrrlichtSimpleDevice *self = (IrrlichtSimpleDevice*)userdata;
	switch (Message->Msg) {
	case TVP_WM_DETACH:
		self->destroyWindow();
		break;
	case TVP_WM_ATTACH:
		self->createWindow((HWND)Message->LParam);
		break;
	default:
		break;
	}
	return false;
}

// ユーザメッセージレシーバの登録/解除
void
IrrlichtSimpleDevice::setReceiver(tTVPWindowMessageReceiver receiver, bool enable)
{
	tTJSVariant mode     = enable ? (tTVInteger)(tjs_int)wrmRegister : (tTVInteger)(tjs_int)wrmUnregister;
	tTJSVariant proc     = (tTVInteger)(tjs_int)receiver;
	tTJSVariant userdata = (tTVInteger)(tjs_int)this;
	tTJSVariant *p[] = {&mode, &proc, &userdata};
	if (window->FuncCall(0, L"registerMessageReceiver", NULL, NULL, 4, p, window) != TJS_S_OK) {
		if (enable) {
			TVPThrowExceptionMessage(L"can't regist user message receiver");
		}
	}
}

// デバイス割り当て後処理
void
IrrlichtSimpleDevice::onAttach()
{
	if (useRender) {
		if (device) {
			IVideoDriver *driver = device->getVideoDriver();
			// 描画開始
			if (driver) {
				target = driver->createRenderTargetTexture(driver->getScreenSize());
				if (target) {
					driver->setRenderTarget(target);
				} else {
					error_log("failed to create rendertarget");
				}
			}
		}
	}
}

	/// デバイス破棄前処理
void
IrrlichtSimpleDevice::onDetach()
{
	if (target) {
		target->drop();
		target = NULL;
	}
}

/**
 * ウインドウを生成
 * @param krkr 親ウインドウ
 */
void
IrrlichtSimpleDevice::createWindow(HWND krkr)
{
	if (krkr) {
		hwnd = krkr;
		start();
	}
}

/**
 * ウインドウを破棄
 */
void
IrrlichtSimpleDevice::destroyWindow()
{
	stop();
	detach();
	hwnd = 0;
}

/**
 * コンストラクタ
 * @param widow ウインドウ
 * @param width 横幅
 * @param height 縦幅
 * @param useRender レンダーターゲットを使う(αが確実に有効)
 */
IrrlichtSimpleDevice::IrrlichtSimpleDevice(iTJSDispatch2 *objthis, iTJSDispatch2 *window, int width, int height, bool useRender)
	: IrrlichtBase(objthis), window(window), hwnd(0), width(width), height(height),
	  useRender(useRender), dwidth(-1), dheight(-1), hbmp(0), oldbmp(0), destDC(0), bmpbuffer(NULL), target(NULL)
{
	window->AddRef();
	setReceiver(messageHandler, true);
	
	tTJSVariant krkrHwnd; // 親のハンドル
	if (window->PropGet(0, TJS_W("HWND"), NULL, &krkrHwnd, window) == TJS_S_OK) {
		createWindow((HWND)(tjs_int)krkrHwnd);
	}
}

/**
 * デストラクタ
 */
IrrlichtSimpleDevice::~IrrlichtSimpleDevice()
{
	clearDC();
	destroyWindow();
	if (window) {
		setReceiver(messageHandler, false);
		window->Release();
		window = NULL;
	}
}


//! returns the size of a texture which would be the optimize size for rendering it
static s32 getTextureSizeFromImageSize(s32 size)
{
	s32 ts = 0x01;
	while(ts < size)
		ts <<= 1;
	return ts;
}

/**
 * 生成ファクトリ
 */
tjs_error
IrrlichtSimpleDevice::Factory(IrrlichtSimpleDevice **obj, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	if (numparams < 3) {
		return TJS_E_BADPARAMCOUNT;
	}
	iTJSDispatch2 *window = param[0]->AsObjectNoAddRef();
	if (window == NULL || window->IsInstanceOf(0, NULL, NULL, L"Window", window) != TJS_S_TRUE) {
		TVPThrowExceptionMessage(L"must set Window object");
	}
	int width  = (tjs_int)*param[1];
	int height = (tjs_int)*param[2];
	bool useRender = numparams >= 4 ? (tjs_int)*param[3]!=0 : false;

	if (useRender) {
		if (getTextureSizeFromImageSize(width) != width ||
			getTextureSizeFromImageSize(height) != height) {
			TVPThrowExceptionMessage(L"width/height must be power of 2 when render mode");
		}
	}

	*obj = new IrrlichtSimpleDevice(objthis, window, width, height, useRender);
	return TJS_S_OK;
}

// -----------------------------------------------------------------------
// Continuous
// -----------------------------------------------------------------------

/**
 * Continuous コールバック
 * 吉里吉里が暇なときに常に呼ばれる
 * これが事実上のメインループになる
 */
void TJS_INTF_METHOD
IrrlichtSimpleDevice::OnContinuousCallback(tjs_uint64 tick)
{
	if (hwnd) {
		attach(hwnd, width, height);
	}
	stop();
}

// -----------------------------------------------------------------------
// 固有機能
// -----------------------------------------------------------------------

void
IrrlichtSimpleDevice::_setSize()
{
	if (useRender) {
		TVPThrowExceptionMessage(L"can't change width/height when render mode");
	}
	if (device) {
		IVideoDriver *driver = device->getVideoDriver();
		if (driver) {
			driver->OnResize(dimension2d<s32>(width, height));
		}
	}
}

/**
 * DCを破棄
 */
void
IrrlichtSimpleDevice::clearDC()
{
	if (destDC) {
		SelectObject(destDC, oldbmp);
		DeleteDC(destDC);
		DeleteObject(hbmp);
		oldbmp = 0;
		hbmp = 0;
		destDC = 0;
	}
}

/**
 * DCを更新
 */
void
IrrlichtSimpleDevice::updateDC(tjs_int dwidth, tjs_int dheight)
{
	if (this->dwidth != dwidth || this->dheight != dheight) {
		// 一度破棄
		clearDC();
		// 描画先のDIBを作る
		BITMAPINFO biBMP;
		ZeroMemory(&biBMP, sizeof biBMP);
		biBMP.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		biBMP.bmiHeader.biBitCount = 32;
		biBMP.bmiHeader.biPlanes = 1;
		biBMP.bmiHeader.biWidth  = dwidth;
		biBMP.bmiHeader.biHeight = -dheight;
		hbmp = CreateDIBSection(NULL, &biBMP, DIB_RGB_COLORS, &bmpbuffer, NULL, 0);
		if (hbmp) {
			// 描画用にDCを作る
			destDC = CreateCompatibleDC(NULL);
			if (destDC) {
				oldbmp = (HBITMAP)SelectObject(destDC, hbmp);
			} else {
				DeleteObject(hbmp);
				hbmp = 0;
			}
		}
		this->dwidth = dwidth;
		this->dheight = dheight;
	}
}

/**
 * レイヤに対して更新描画
 * バックバッファからコピーします。
 * @param layer レイヤ
 * @param srcRect ソース領域
 */
void
IrrlichtSimpleDevice::_updateToLayer(iTJSDispatch2 *layer, irr::core::rect<s32> *srcRect)
{
	if (device) {
		// レイヤ情報取得
		ncbPropAccessor obj(layer);
		tjs_int dwidth  = obj.GetValue(L"imageWidth", ncbTypedefs::Tag<tjs_int>());
		tjs_int dheight = obj.GetValue(L"imageHeight", ncbTypedefs::Tag<tjs_int>());
		tjs_int dPitch  = obj.GetValue(L"mainImageBufferPitch", ncbTypedefs::Tag<tjs_int>());
		unsigned char *dbuffer = (unsigned char *)obj.GetValue(L"mainImageBufferForWrite", ncbTypedefs::Tag<tjs_int>());
		
		// 描画先DCの更新
		updateDC(dwidth, dheight);
		
		if (destDC) {
			irr::core::rect<s32> destRect(0,0,dwidth,dheight);
			if (show(&destRect, srcRect, destDC)) {
				// ビットマップからコピー
				for (tjs_int y = 0; y < dheight; y++) {
					unsigned char *src = (unsigned char *)(bmpbuffer) + dwidth * y * 4;
					CopyMemory(dbuffer, src, dwidth*4);
					dbuffer += dPitch;
				}
				// レイヤを更新
				layer->FuncCall(0, L"update", NULL, NULL, 0, NULL, layer);
			} else {
				error_log("failed to show");
			}
		}
	}
}

tjs_error
IrrlichtSimpleDevice::updateToLayer(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	IrrlichtSimpleDevice *self = ncbInstanceAdaptor<IrrlichtSimpleDevice>::GetNativeInstance(objthis);
	if (!self) {
		return TJS_E_NATIVECLASSCRASH;
	}
	if (numparams < 1) {
		return TJS_E_BADPARAMCOUNT;
	}
	// レイヤオブジェクトかどうか
	iTJSDispatch2 *layer = param[0]->AsObjectNoAddRef();
	if (layer == NULL || layer->IsInstanceOf(0, NULL, NULL, L"Layer", layer) != TJS_S_TRUE) {
		TVPThrowExceptionMessage(L"must set Layer object");
	}
	// ソース領域判定
	irr::core::rect<s32> *srcRect = NULL, _srcRect;
	if (numparams >= 5) {
		_srcRect.setLeft((tjs_int)*param[1]);
		_srcRect.setTop((tjs_int)*param[2]);
		_srcRect.setWidth((tjs_int)*param[3]);
		_srcRect.setHeight((tjs_int)*param[4]);
		srcRect = &_srcRect;
	}
	self->_updateToLayer(layer, srcRect);
	return TJS_S_OK;
}
