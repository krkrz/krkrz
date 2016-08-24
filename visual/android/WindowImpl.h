//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// "Window" TJS Class implementation
//---------------------------------------------------------------------------


#ifndef WindowImplH
#define WindowImplH

#include "WindowIntf.h"

//---------------------------------------------------------------------------
// tTJSNI_Window : Window Native Instance
//---------------------------------------------------------------------------
//class TTVPWindowForm;
class iTVPDrawDevice;
class tTJSNI_BaseLayer;
class tTJSNI_Window : public tTJSNI_BaseWindow
{
#if 0
	TTVPWindowForm *Form;
	class tTVPVSyncTimingThread *VSyncTimingThread;
#endif

public:
	tTJSNI_Window();
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param,
		iTJSDispatch2 *tjs_obj);
	void TJS_INTF_METHOD Invalidate();

#if 0
	bool CloseFlag;

public:
	bool CanDeliverEvents() const; // tTJSNI_BaseWindow::CanDeliverEvents override

public:
	TTVPWindowForm * GetForm() const { return Form; }
	void NotifyWindowClose();

	void SendCloseMessage();
#endif
	void TickBeat() {}
#if 0
private:
	bool GetWindowActive();
	void UpdateVSyncThread();

public:
//-- draw device
	virtual void ResetDrawDevice();

//-- event control
	virtual void PostInputEvent(const ttstr &name, iTJSDispatch2 * params);  // override

//-- interface to layer manager
	void TJS_INTF_METHOD NotifySrcResize(); // is called from primary layer

	void TJS_INTF_METHOD SetDefaultMouseCursor(); // set window mouse cursor to default
	void TJS_INTF_METHOD SetMouseCursor(tjs_int cursor); // set window mouse cursor
	void TJS_INTF_METHOD GetCursorPos(tjs_int &x, tjs_int &y);
	void TJS_INTF_METHOD SetCursorPos(tjs_int x, tjs_int y);
	void TJS_INTF_METHOD WindowReleaseCapture();
	void TJS_INTF_METHOD SetHintText(iTJSDispatch2* sender, const ttstr & text);
	void TJS_INTF_METHOD SetAttentionPoint(tTJSNI_BaseLayer *layer,
		tjs_int l, tjs_int t);
	void TJS_INTF_METHOD DisableAttentionPoint();
	void TJS_INTF_METHOD SetImeMode(tTVPImeMode mode);
	void SetDefaultImeMode(tTVPImeMode mode);
	tTVPImeMode GetDefaultImeMode() const;
	void TJS_INTF_METHOD ResetImeMode();

//-- update managment
	void BeginUpdate(const tTVPComplexRect &rects);
	void EndUpdate();

//-- interface to VideoOverlay object
public:
	HWND GetSurfaceWindowHandle();
	HWND GetWindowHandle();
	void GetVideoOffset(tjs_int &ofsx, tjs_int &ofsy);

	void ReadjustVideoRect();
	void WindowMoved();
	void DetachVideoOverlay();

//-- interface to plugin
	void ZoomRectangle(
		tjs_int & left, tjs_int & top,
		tjs_int & right, tjs_int & bottom);
	HWND GetWindowHandleForPlugin();
	void RegisterWindowMessageReceiver(tTVPWMRRegMode mode,
		void * proc, const void *userdata);

//-- methods
	void Close();
	void OnCloseQueryCalled(bool b);

	void BringToFront();
	void Update(tTVPUpdateType);

	void ShowModal();

	void HideMouseCursor();

//-- properties
	bool GetVisible() const;
	void SetVisible(bool s);

	void GetCaption(ttstr & v) const;
	void SetCaption(const ttstr & v);

	void SetWidth(tjs_int w);
	tjs_int GetWidth() const;
	void SetHeight(tjs_int h);
	tjs_int GetHeight() const;
	void SetSize(tjs_int w, tjs_int h);

	void SetMinWidth(int v);
	int GetMinWidth() const;
	void SetMinHeight(int v);
	int GetMinHeight() const;
	void SetMinSize(int w, int h);

	void SetMaxWidth(int v);
	int GetMaxWidth() const;
	void SetMaxHeight(int v);
	int GetMaxHeight() const;
	void SetMaxSize(int w, int h);

	void SetLeft(tjs_int l);
	tjs_int GetLeft() const;
	void SetTop(tjs_int t);
	tjs_int GetTop() const;
	void SetPosition(tjs_int l, tjs_int t);

	void SetInnerWidth(tjs_int w);
	tjs_int GetInnerWidth() const;
	void SetInnerHeight(tjs_int h);
	tjs_int GetInnerHeight() const;

	void SetInnerSize(tjs_int w, tjs_int h);

	void SetBorderStyle(tTVPBorderStyle st);
	tTVPBorderStyle GetBorderStyle() const;

	void SetStayOnTop(bool b);
	bool GetStayOnTop() const;

	void SetFullScreen(bool b);
	bool GetFullScreen() const;

	void SetUseMouseKey(bool b);
	bool GetUseMouseKey() const;

	void SetTrapKey(bool b);
	bool GetTrapKey() const;

	void SetMaskRegion(tjs_int threshold);
	void RemoveMaskRegion();

	void SetMouseCursorState(tTVPMouseCursorState mcs);
    tTVPMouseCursorState GetMouseCursorState() const;

	void SetFocusable(bool b);
	bool GetFocusable();

	void SetZoom(tjs_int numer, tjs_int denom);
	void SetZoomNumer(tjs_int n);
	tjs_int GetZoomNumer() const;
	void SetZoomDenom(tjs_int n);
	tjs_int GetZoomDenom() const;
	
	void SetTouchScaleThreshold( tjs_real threshold );
	tjs_real GetTouchScaleThreshold() const;
	void SetTouchRotateThreshold( tjs_real threshold );
	tjs_real GetTouchRotateThreshold() const;

	tjs_real GetTouchPointStartX( tjs_int index );
	tjs_real GetTouchPointStartY( tjs_int index );
	tjs_real GetTouchPointX( tjs_int index );
	tjs_real GetTouchPointY( tjs_int index );
	tjs_real GetTouchPointID( tjs_int index );
	tjs_int GetTouchPointCount();
	
	void SetHintDelay( tjs_int delay );
	tjs_int GetHintDelay() const;

	void SetEnableTouch( bool b );
	bool GetEnableTouch() const;

	int GetDisplayOrientation();
	int GetDisplayRotate();
	
	bool WaitForVBlank( tjs_int* in_vblank, tjs_int* delayed );

public: // for iTVPLayerTreeOwner
	// LayerManager -> LTO
	/*
	implements on tTJSNI_BaseWindow
	virtual void TJS_INTF_METHOD RegisterLayerManager( class iTVPLayerManager* manager );
	virtual void TJS_INTF_METHOD UnregisterLayerManager( class iTVPLayerManager* manager );
	*/

	virtual void TJS_INTF_METHOD StartBitmapCompletion(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD NotifyBitmapCompleted(class iTVPLayerManager * manager,
		tjs_int x, tjs_int y, const void * bits, const class BitmapInfomation * bitmapinfo,
		const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity);
	virtual void TJS_INTF_METHOD EndBitmapCompletion(iTVPLayerManager * manager);

	virtual void TJS_INTF_METHOD SetMouseCursor(class iTVPLayerManager* manager, tjs_int cursor);
	virtual void TJS_INTF_METHOD GetCursorPos(class iTVPLayerManager* manager, tjs_int &x, tjs_int &y);
	virtual void TJS_INTF_METHOD SetCursorPos(class iTVPLayerManager* manager, tjs_int x, tjs_int y);
	virtual void TJS_INTF_METHOD ReleaseMouseCapture(class iTVPLayerManager* manager);

	virtual void TJS_INTF_METHOD SetHint(class iTVPLayerManager* manager, iTJSDispatch2* sender, const ttstr &hint);

	virtual void TJS_INTF_METHOD NotifyLayerResize(class iTVPLayerManager* manager);
	virtual void TJS_INTF_METHOD NotifyLayerImageChange(class iTVPLayerManager* manager);

	virtual void TJS_INTF_METHOD SetAttentionPoint(class iTVPLayerManager* manager, tTJSNI_BaseLayer *layer, tjs_int x, tjs_int y);
	virtual void TJS_INTF_METHOD DisableAttentionPoint(class iTVPLayerManager* manager);

	virtual void TJS_INTF_METHOD SetImeMode( class iTVPLayerManager* manager, tjs_int mode ); // mode == tTVPImeMode
	virtual void TJS_INTF_METHOD ResetImeMode( class iTVPLayerManager* manager );

protected:
#endif
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#endif
