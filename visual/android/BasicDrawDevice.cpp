

#include "tjsCommHead.h"
#include "DrawDevice.h"
#include "BasicDrawDevice.h"
#include "LayerIntf.h"
#include "MsgIntf.h"
#include "SysInitIntf.h"
#include "WindowIntf.h"
#include "DebugIntf.h"
#include "ThreadIntf.h"
#include "ComplexRect.h"
#include "EventIntf.h"
#include "WindowImpl.h"
#include "Application.h"

#include <algorithm>

//---------------------------------------------------------------------------
// オプション
//---------------------------------------------------------------------------
static tjs_int TVPBasicDrawDeviceOptionsGeneration = 0;
bool TVPZoomInterpolation = true;
//---------------------------------------------------------------------------
static void TVPInitBasicDrawDeviceOptions()
{
	if(TVPBasicDrawDeviceOptionsGeneration == TVPGetCommandLineArgumentGeneration()) return;
	TVPBasicDrawDeviceOptionsGeneration = TVPGetCommandLineArgumentGeneration();

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
tTVPBasicDrawDevice::tTVPBasicDrawDevice()
: DrawUpdateRectangle(false), ShouldShow(false)
{
	TVPInitBasicDrawDeviceOptions(); // read and initialize options
}
//---------------------------------------------------------------------------
tTVPBasicDrawDevice::~tTVPBasicDrawDevice()
{
}
//---------------------------------------------------------------------------
void tTVPBasicDrawDevice::InvalidateAll()
{
	// レイヤ演算結果をすべてリクエストする
	// サーフェースが lost した際に内容を再構築する目的で用いる
	RequestInvalidation(tTVPRect(0, 0, DestRect.get_width(), DestRect.get_height()));
}
//---------------------------------------------------------------------------
void tTVPBasicDrawDevice::EnsureDevice()
{
	TVPInitBasicDrawDeviceOptions();
	// 何もしない
	// SurfaceView は既に作られている前提
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::AddLayerManager(iTVPLayerManager * manager)
{
	if(inherited::Managers.size() > 0)
	{
		// "Basic" デバイスでは２つ以上のLayer Managerを登録できない
		TVPThrowExceptionMessage(TVPBasicDrawDeviceDoesNotSupporteLayerManagerMoreThanOne);
	}
	inherited::AddLayerManager(manager);

	manager->SetDesiredLayerType(ltOpaque); // ltOpaque な出力を受け取りたい
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::SetTargetWindow(HWND wnd, bool is_main)
{
	TVPInitBasicDrawDeviceOptions();
	IsMainWindow = is_main;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::SetDestRectangle(const tTVPRect & rect)
{
	// 位置だけの変更の場合かどうかをチェックする
	if(rect.get_width() == DestRect.get_width() && rect.get_height() == DestRect.get_height()) {
		// 位置だけの変更だ
		inherited::SetDestRectangle(rect);
	} else {
		// サイズも違う
		// Application->changeSurfaceSize( w, h );	// サーフェイスサイズを変更する(非同期)
		ANativeWindow* win = Application->getWindow();
		if( win != nullptr ) {
			int32_t result = ANativeWindow_setBuffersGeometry( win, rect.get_width(), rect.get_height(), WINDOW_FORMAT_RGBX_8888 );
		}
		inherited::SetDestRectangle(rect);
	}
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::NotifyLayerResize(iTVPLayerManager * manager)
{
	inherited::NotifyLayerResize(manager);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::Show()
{
	if(!ShouldShow) return;

	ShouldShow = false;
	if( IsLockBuffer ) {
		ANativeWindow* win = Application->getWindow();
		if( win != nullptr ) {
			ANativeWindow_unlockAndPost( win );
			IsLockBuffer = false;
		}
	}
/*
	ANativeWindow* win = Application->getWindow();
	if( win != nullptr ) {
		int w = ANativeWindow_getWidth( win );
		int h = ANativeWindow_getHeight( win );
		ARect dirty;
		dirty.left = 0;
		dirty.top = 0;
		dirty.right = w;
		dirty.bottom = h;
		ANativeWindow_Buffer buffer;
		ANativeWindow_lock( win, &buffer, &dirty );
		tjs_uint8* bits = (tjs_uint8*)buffer.bits;
		const tjs_uint8* src = WorkBuffer;
		for( int32_t y = 0; y < buffer.height; y++ ) {
			unsigned char* lines = bits;
			for( int32_t x = 0; x < buffer.width; x++ ) {
				// src を書き込む
				lines[0] = 0xff; lines[1] = 0xff; lines[2] = 0; lines[3] = 0xff;
				lines += 4;
			}
			bits += buffer.stride*sizeof(int32_t);
		}
		ANativeWindow_unlockAndPost( win  );
	}
*/
}
//---------------------------------------------------------------------------
bool TJS_INTF_METHOD tTVPBasicDrawDevice::WaitForVBlank( tjs_int* in_vblank, tjs_int* delayed )
{
	return false;	// 非サポート
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::StartBitmapCompletion(iTVPLayerManager * manager)
{
	EnsureDevice();

	// lock window(surface)
	if( IsLockBuffer == false ) {
		ANativeWindow* win = Application->getWindow();
		if( win != nullptr ) {
			ARect dirty;
			dirty.left = 0;
			dirty.top = 0;
			dirty.right = ANativeWindow_getWidth( win );
			dirty.bottom = ANativeWindow_getHeight( win );
			ANativeWindow_lock( win, &BackBuffer, &dirty );
			IsLockBuffer = true;
		}
	}
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::NotifyBitmapCompleted(iTVPLayerManager * manager,
	tjs_int x, tjs_int y, const void * bits, const class BitmapInfomation * bmpinfo,
	const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity)
{
	// bits, bitmapinfo で表されるビットマップの cliprect の領域を、x, y に描画する。
	// opacity と type は無視するしかないので無視する
	tjs_int w, h;
	GetSrcSize( w, h );
	if( IsLockBuffer &&
		!(x < 0 || y < 0 ||
			x + cliprect.get_width() > w ||
			y + cliprect.get_height() > h) &&
		!(cliprect.left < 0 || cliprect.top < 0 ||
			cliprect.right > bmpinfo->GetWidth() ||
			cliprect.bottom > bmpinfo->GetHeight()))
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

		if(bmpinfo->GetHeight() < 0)
		{
			// bottom-down
			src_pitch = bmpinfo->GetPitchBytes();
		}
		else
		{
			// bottom-up
			src_pitch = bmpinfo->GetPitchBytes();
			src_p += bmpinfo->GetWidth() * 4 * (bmpinfo->GetHeight() - 1);
		}

		for(; src_y < src_y_limit; src_y ++, dest_y ++)
		{
			const void *srcp = src_p + src_pitch * src_y + src_x * 4;
			void *destp = (tjs_uint8*)BackBuffer.bits + BackBuffer.stride*sizeof(int32_t) * dest_y + dest_x * 4;
			memcpy(destp, srcp, width_bytes);
		}
	}
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::EndBitmapCompletion(iTVPLayerManager * manager)
{
	// 何もしない、Show 呼ばれ待ち
	// Android の場合、unlockとshowが一体化しているので、show で unlock する
	// ClipRect の考慮がない
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::SetShowUpdateRect(bool b)
{
	DrawUpdateRectangle = b;
}
//---------------------------------------------------------------------------
bool TJS_INTF_METHOD tTVPBasicDrawDevice::SwitchToFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color, bool changeresolution )
{
	// フルスクリーン化の処理はなにも行わない
	ShouldShow = true;
	return true;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPBasicDrawDevice::RevertFromFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color )
{
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// tTJSNI_BasicDrawDevice : BasicDrawDevice TJS native class
//---------------------------------------------------------------------------
tjs_uint32 tTJSNC_BasicDrawDevice::ClassID = (tjs_uint32)-1;
tTJSNC_BasicDrawDevice::tTJSNC_BasicDrawDevice() :
	tTJSNativeClass(TJS_W("BasicDrawDevice"))
{
	// register native methods/properties

	TJS_BEGIN_NATIVE_MEMBERS(BasicDrawDevice)
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
// constructor/methods
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_BasicDrawDevice,
	/*TJS class name*/BasicDrawDevice)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/BasicDrawDevice)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/recreate)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_BasicDrawDevice);
	_this->GetDevice()->SetToRecreateDrawer();
	_this->GetDevice()->EnsureDevice();
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/recreate)
//----------------------------------------------------------------------


//---------------------------------------------------------------------------
//----------------------------------------------------------------------
// properties
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(interface)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_BasicDrawDevice);
		*result = reinterpret_cast<tjs_int64>(_this->GetDevice());
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(interface)
//----------------------------------------------------------------------
	TJS_END_NATIVE_MEMBERS
}
//---------------------------------------------------------------------------
iTJSNativeInstance *tTJSNC_BasicDrawDevice::CreateNativeInstance()
{
	return new tTJSNI_BasicDrawDevice();
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
tTJSNI_BasicDrawDevice::tTJSNI_BasicDrawDevice()
{
	Device = new tTVPBasicDrawDevice();
}
//---------------------------------------------------------------------------
tTJSNI_BasicDrawDevice::~tTJSNI_BasicDrawDevice()
{
	if(Device) Device->Destruct(), Device = NULL;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD
	tTJSNI_BasicDrawDevice::Construct(tjs_int numparams, tTJSVariant **param,
		iTJSDispatch2 *tjs_obj)
{
	return TJS_S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_BasicDrawDevice::Invalidate()
{
	if(Device) Device->Destruct(), Device = NULL;
}
//---------------------------------------------------------------------------

