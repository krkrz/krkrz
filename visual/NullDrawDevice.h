/**
 * 何も描画を行わないDrawDevice
 * Canvas の時はこのDrawDeviceが設定される(ただし使用はされない)
 */
#ifndef NullDrawDeviceH
#define NullDrawDeviceH

#include "DrawDevice.h"

class tTVPNullDrawDevice : public iTVPDrawDevice
{
public:
	~tTVPNullDrawDevice() {}
	void TJS_INTF_METHOD Destruct() override { delete this; }
	void TJS_INTF_METHOD SetWindowInterface(iTVPWindow * window) override {}
	void TJS_INTF_METHOD AddLayerManager(iTVPLayerManager * manager) override {}
	void TJS_INTF_METHOD RemoveLayerManager(iTVPLayerManager * manager) override {}
	void TJS_INTF_METHOD SetTargetWindow(HWND wnd, bool is_main) override {}
	void TJS_INTF_METHOD SetDestRectangle(const tTVPRect & rect) override {}
	void TJS_INTF_METHOD SetClipRectangle(const tTVPRect & rect) override {}
	void TJS_INTF_METHOD GetSrcSize(tjs_int &w, tjs_int &h) override {}
	void TJS_INTF_METHOD NotifyLayerResize(iTVPLayerManager * manager) override {}
	void TJS_INTF_METHOD NotifyLayerImageChange(iTVPLayerManager * manager) override {}
	void TJS_INTF_METHOD OnClick(tjs_int x, tjs_int y) override {}
	void TJS_INTF_METHOD OnDoubleClick(tjs_int x, tjs_int y) override {}
	void TJS_INTF_METHOD OnMouseDown(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags) override {}
	void TJS_INTF_METHOD OnMouseUp(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags) override {}
	void TJS_INTF_METHOD OnMouseMove(tjs_int x, tjs_int y, tjs_uint32 flags) override {}
	void TJS_INTF_METHOD OnReleaseCapture() override {}
	void TJS_INTF_METHOD OnMouseOutOfWindow() override {}
	void TJS_INTF_METHOD OnKeyDown(tjs_uint key, tjs_uint32 shift) override {}
	void TJS_INTF_METHOD OnKeyUp(tjs_uint key, tjs_uint32 shift) override {}
	void TJS_INTF_METHOD OnKeyPress(tjs_char key) override {}
	void TJS_INTF_METHOD OnMouseWheel(tjs_uint32 shift, tjs_int delta, tjs_int x, tjs_int y) override {}
	void TJS_INTF_METHOD OnTouchDown( tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id ) override {}
	void TJS_INTF_METHOD OnTouchUp( tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id ) override {}
	void TJS_INTF_METHOD OnTouchMove( tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id ) override {}
	void TJS_INTF_METHOD OnTouchScaling( tjs_real startdist, tjs_real curdist, tjs_real cx, tjs_real cy, tjs_int flag ) override {}
	void TJS_INTF_METHOD OnTouchRotate( tjs_real startangle, tjs_real curangle, tjs_real dist, tjs_real cx, tjs_real cy, tjs_int flag ) override {}
	void TJS_INTF_METHOD OnMultiTouch() override {}
	void TJS_INTF_METHOD OnDisplayRotate( tjs_int orientation, tjs_int rotate, tjs_int bpp, tjs_int width, tjs_int height ) override {}
	void TJS_INTF_METHOD RecheckInputState() override {}
	void TJS_INTF_METHOD SetDefaultMouseCursor(iTVPLayerManager * manager) override {}
	void TJS_INTF_METHOD SetMouseCursor(iTVPLayerManager * manager, tjs_int cursor) override {}
	void TJS_INTF_METHOD GetCursorPos(iTVPLayerManager * manager, tjs_int &x, tjs_int &y) override {}
	void TJS_INTF_METHOD SetCursorPos(iTVPLayerManager * manager, tjs_int x, tjs_int y) override {}
	void TJS_INTF_METHOD WindowReleaseCapture(iTVPLayerManager * manager) override {}
	void TJS_INTF_METHOD SetHintText(iTVPLayerManager * manager, iTJSDispatch2* sender, const ttstr & text) override {}
	void TJS_INTF_METHOD SetAttentionPoint(iTVPLayerManager * manager, tTJSNI_BaseLayer *layer, tjs_int l, tjs_int t ) override {}
	void TJS_INTF_METHOD DisableAttentionPoint(iTVPLayerManager * manager) override {}
	void TJS_INTF_METHOD SetImeMode(iTVPLayerManager * manager, tTVPImeMode mode) override {}
	void TJS_INTF_METHOD ResetImeMode(iTVPLayerManager * manager) override {}
	tTJSNI_BaseLayer * TJS_INTF_METHOD GetPrimaryLayer() override { return nullptr; }
	tTJSNI_BaseLayer * TJS_INTF_METHOD GetFocusedLayer() override { return nullptr; }
	void TJS_INTF_METHOD SetFocusedLayer(tTJSNI_BaseLayer * layer) override {}
	void TJS_INTF_METHOD RequestInvalidation(const tTVPRect & rect) override {}
	void TJS_INTF_METHOD Update() override {}
	void TJS_INTF_METHOD Show() override {}
	void TJS_INTF_METHOD StartBitmapCompletion(iTVPLayerManager * manager) override {}
	void TJS_INTF_METHOD NotifyBitmapCompleted(iTVPLayerManager * manager,
		tjs_int x, tjs_int y, const void * bits, const class BitmapInfomation * bitmapinfo,
		const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity) override {}
	void TJS_INTF_METHOD EndBitmapCompletion(iTVPLayerManager * manager) override {}
	void TJS_INTF_METHOD DumpLayerStructure() override {}
	void TJS_INTF_METHOD SetShowUpdateRect(bool b) override {}
	bool TJS_INTF_METHOD SwitchToFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color, bool changeresolution ) override { return true; }
	void TJS_INTF_METHOD RevertFromFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color ) override {}
	bool TJS_INTF_METHOD WaitForVBlank( tjs_int* in_vblank, tjs_int* delayed ) { return false; }
};

//---------------------------------------------------------------------------
// tTJSNI_NullDrawDevice
//---------------------------------------------------------------------------
class tTJSNI_NullDrawDevice : public tTJSNativeInstance
{
	typedef tTJSNativeInstance inherited;

	tTVPNullDrawDevice * Device;

public:
	tTJSNI_NullDrawDevice();
	~tTJSNI_NullDrawDevice() override;
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) override;
	void TJS_INTF_METHOD Invalidate() override;

public:
	tTVPNullDrawDevice * GetDevice() const { return Device; }

};
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// tTJSNC_NullDrawDevice
//---------------------------------------------------------------------------
class tTJSNC_NullDrawDevice : public tTJSNativeClass
{
public:
	tTJSNC_NullDrawDevice();

	static tjs_uint32 ClassID;

private:
	iTJSNativeInstance *CreateNativeInstance();
};
//---------------------------------------------------------------------------


#endif
