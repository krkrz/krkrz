
#include "tjsCommHead.h"

#define DIRECTDRAW_VERSION 0x0300
#include <ddraw.h>

#include <dbt.h> // for WM_DEVICECHANGE

#include <algorithm>
#include "WindowFormUnit.h"
#include "WindowImpl.h"
#include "EventIntf.h"
#include "ComplexRect.h"
#include "LayerBitmapIntf.h"
#include "tjsArray.h"
#include "StorageIntf.h"
#include "MsgIntf.h"
#include "MenuContainerForm.h"
#include "MainFormUnit.h"
#include "SysInitIntf.h"
#include "PluginImpl.h"
#include "Random.h"
#include "SystemImpl.h"
#include "DInputMgn.h"
#include "tvpinputdefs.h"

#include "Application.h"
#include "TFont.h"
#include "TickCount.h"

tjs_uint32 TVP_TShiftState_To_uint32(TShiftState state) {
	tjs_uint32 result = 0;
	if( state & MK_SHIFT ) {
		result |= ssShift;
	}
	if( state & MK_CONTROL ) {
		result |= ssCtrl;
	}
	if( state & MK_ALT ) {
		result |= ssAlt;
	}
	return result;
}
TShiftState TVP_TShiftState_From_uint32(tjs_uint32 state){
	TShiftState result = 0;
	if( state & ssShift ) {
		result |= MK_SHIFT;
	}
	if( state & ssCtrl ) {
		result |= MK_CONTROL;
	}
	if( state & ssAlt ) {
		result |= MK_ALT;
	}
	return result;
}
tTVPMouseButton TVP_TMouseButton_To_tTVPMouseButton(int button) {
	return (tTVPMouseButton)button;
}


//---------------------------------------------------------------------------
// Modal Window List
//---------------------------------------------------------------------------
// modal window list is used to ensure modal window accessibility when
// the system is full-screened,
std::vector<TTVPWindowForm *> TVPModalWindowList;
//---------------------------------------------------------------------------
static void TVPAddModalWindow(TTVPWindowForm * window)
{
	std::vector<TTVPWindowForm *>::iterator i;
	i = std::find(TVPModalWindowList.begin(), TVPModalWindowList.end(), window);
	if(i == TVPModalWindowList.end())
		TVPModalWindowList.push_back(window);
}
//---------------------------------------------------------------------------
static void TVPRemoveModalWindow(TTVPWindowForm *window)
{
	std::vector<TTVPWindowForm *>::iterator i;
	i = std::find(TVPModalWindowList.begin(), TVPModalWindowList.end(), window);
	if(i != TVPModalWindowList.end())
		TVPModalWindowList.erase(i);
}
//---------------------------------------------------------------------------
#include "DebugIntf.h"
void TVPShowModalAtAppActivate()
{
	// called when the application is activated
	if(TVPFullScreenedWindow != NULL)
	{
		// any window is full-screened
		::ShowWindow(TVPFullScreenedWindow->GetHandle(), SW_RESTORE); // restore the window

		// send message which brings modal windows to front
		std::vector<TTVPWindowForm *>::iterator i;
		for(i = TVPModalWindowList.begin(); i != TVPModalWindowList.end(); i++)
			(*i)->InvokeShowVisible();
		for(i = TVPModalWindowList.begin(); i != TVPModalWindowList.end(); i++)
			(*i)->InvokeShowTop();
	}
}
//---------------------------------------------------------------------------
HDWP TVPShowModalAtTimer(HDWP hdwp)
{
	// called every 4 seconds, to ensure the modal window visible
	if(TVPFullScreenedWindow != NULL)
	{
		// send message which brings modal windows to front
		std::vector<TTVPWindowForm *>::iterator i;
		for(i = TVPModalWindowList.begin(); i != TVPModalWindowList.end(); i++)
			hdwp = (*i)->ShowTop(hdwp);
	}
	return hdwp;
}
//---------------------------------------------------------------------------
void TVPHideModalAtAppDeactivate()
{
	// called when the application is deactivated
	if(TVPFullScreenedWindow != NULL)
	{
		// any window is full-screened

		// hide modal windows
		std::vector<TTVPWindowForm *>::iterator i;
		for(i = TVPModalWindowList.begin(); i != TVPModalWindowList.end(); i++)
			(*i)->SetVisible( false );
	}

	// hide also popups
	TTVPWindowForm::DeliverPopupHide();
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Window/Bitmap options
//---------------------------------------------------------------------------
static bool TVPWindowOptionsInit = false;
static bool TVPControlImeState = true;
//---------------------------------------------------------------------------
void TVPInitWindowOptions()
{
	// initialize various options around window/graphics

	if(TVPWindowOptionsInit) return;

	bool initddraw = false;

	tTJSVariant val;

	if(TVPGetCommandLine(TJS_W("-wheel"), &val) )
	{
		ttstr str(val);
		if(str == TJS_W("dinput"))
			TVPWheelDetectionType = wdtDirectInput;
		else if(str == TJS_W("message"))
			TVPWheelDetectionType = wdtWindowMessage;
		else
			TVPWheelDetectionType = wdtNone;
	}

	if(TVPGetCommandLine(TJS_W("-joypad"), &val) )
	{
		ttstr str(val);
		if(str == TJS_W("dinput"))
			TVPJoyPadDetectionType = jdtDirectInput;
/*		else if(str == TJS_W("message"))
			TVPJoyPadDetectionType = wdtWindowMessage; */
		else
			TVPJoyPadDetectionType = jdtNone;
	}


	if(TVPGetCommandLine(TJS_W("-controlime"), &val) )
	{
		ttstr str(val);
		if(str == TJS_W("no"))
			TVPControlImeState = false;
	}

	if(initddraw) TVPEnsureDirectDrawObject();

	TVPWindowOptionsInit = true;
}
//---------------------------------------------------------------------------


TTVPWindowForm::TTVPWindowForm( TApplication* app, tTJSNI_Window* ni ) : TML::Window(), CurrentMouseCursor(crDefault) {
	CreateWnd( "TVPMainWindow", Application->GetTitle(), 10, 10 );
	TVPInitWindowOptions();
	
	// initialize members
	TJSNativeInstance = ni;
	app->AddWindow(this);
	
	NextSetWindowHandleToDrawDevice = true;
	LastSentDrawDeviceDestRect.clear();
	
	InMode = false;
	Closing = false;
	ProgramClosing = false;
	InnerWidthSave = GetInnerWidth();
	InnerHeightSave = GetInnerHeight();
	MenuContainer = NULL;
	MenuBarVisible = true;

	AttentionFont = new TFont();
	
	ZoomDenom = ActualZoomDenom = 1;
	ZoomNumer = ActualZoomNumer = 1;

	DefaultImeMode = imClose;
	LastSetImeMode = imClose;
	::PostMessage( GetHandle(), TVP_WM_ACQUIREIMECONTROL, 0, 0);


	UseMouseKey = false;
	TrapKeys = false;
	CanReceiveTrappedKeys = false;
	InReceivingTrappedKeys = false;
	MouseKeyXAccel = 0;
	MouseKeyYAccel = 0;
	LastMouseMoved = false;
	MouseLeftButtonEmulatedPushed = false;
	MouseRightButtonEmulatedPushed = false;
	LastMouseMovedPos.x = 0;
	LastMouseMovedPos.y = 0;

	MouseCursorState = mcsVisible;
	ForceMouseCursorVisible = false;
	CurrentMouseCursor = crDefault;

	DIWheelDevice = NULL;
	DIPadDevice = NULL;
	ReloadDevice = false;
	ReloadDeviceTick = 0;
	
	LastRecheckInputStateSent = 0;

	MainMenu = NULL;
}
TTVPWindowForm::~TTVPWindowForm() {
	Application->RemoveWindow(this);
}
tjs_uint32 TVPGetCurrentShiftKeyState() {
	tjs_uint32 f = 0;

	if(TVPGetAsyncKeyState(VK_SHIFT)) f += TVP_SS_SHIFT;
	if(TVPGetAsyncKeyState(VK_MENU)) f += TVP_SS_ALT;
	if(TVPGetAsyncKeyState(VK_CONTROL)) f += TVP_SS_CTRL;
	if(TVPGetAsyncKeyState(VK_LBUTTON)) f += TVP_SS_LEFT;
	if(TVPGetAsyncKeyState(VK_RBUTTON)) f += TVP_SS_RIGHT;
	if(TVPGetAsyncKeyState(VK_MBUTTON)) f += TVP_SS_MIDDLE;

	return f;
}
TTVPWindowForm * TVPFullScreenedWindow;

void TVPGetFontList(std::vector<std::string> & list, tjs_uint32 flags, TFont * refcanvas) {
}
enum {
	CM_BASE = 0xB000,
	CM_MOUSEENTER = CM_BASE + 19,
	CM_MOUSELEAVE = CM_BASE + 20,
};

LRESULT WINAPI TTVPWindowForm::Proc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	tTVPWindowMessage Message;
	Message.LParam = lParam;
	Message.WParam = wParam;
	Message.Msg = msg;
	Message.Result = 0;
	if( DeliverMessageToReceiver( Message) ) return Message.Result;
	//switch( msg )
	{
		/*
	case WM_MOVE:
		return WMMove();
	case WM_DROPFILES:
		return WMDropFiles();
	case CM_MOUSEENTER:
		return CMMouseEnter();
	case CM_MOUSELEAVE:
		return CMMouseLeave();
	case WM_MOUSEACTIVATE:
		return WMMouseActivate();
	case TVP_WM_SHOWVISIBLE:
		return WMShowVisible();
	case TVP_WM_SHOWTOP:
		return WMShowTop();
	case TVP_WM_RETRIEVEFOCUS:
		return WMRetrieveFocus();
	case TVP_WM_ACQUIREIMECONTROL:
		return WMAcquireImeControl();
	case WM_ENABLE:
		return WMEnable();
	case WM_SETFOCUS:
		return WMSetFocus();
	case WM_KILLFOCUS:
		return WMKillFocus();
	case WM_ENTERMENULOOP:
		return WMEnterMenuLoop();
	case WM_EXITMENULOOP:
		return WMExitMenuLoop();
	case WM_KEYDOWN:
		return WMKeyDown();
	case WM_DEVICECHANGE:
		return WMDeviceChange();
	case WM_NCLBUTTONDOWN:
		return WMNCLButtonDown();
	case WM_NCRBUTTONDOWN:
		return WMNCRButtonDown();
	default:
		*/
		return TML::Window::Proc( hWnd, msg, wParam, lParam );
	}
}
bool TTVPWindowForm::GetFormEnabled() { 
	return TRUE == ::IsWindowEnabled(GetHandle());
}

void TTVPWindowForm::TickBeat(){
	// called every 50ms intervally
	DWORD tickcount = static_cast<DWORD>(TVPGetTickCount());
	bool focused = HasFocus();
	/*
	bool showingmenu = InMenuLoop;
	if( MenuContainer ) {
		focused = focused || MenuContainer->Focused();
		showingmenu = showingmenu || MenuContainer->GetShowingMenu();
	}
	*/

	// set mouse cursor state
	//SetForceMouseCursorVisible(showingmenu);

	// watch menu bar drop
	//if(focused && !showingmenu) CheckMenuBarDrop();

	// mouse key
	if(UseMouseKey && /*PaintBox && !showingmenu &&*/ focused) {
		GenerateMouseEvent(false, false, false, false);
	}

	// device reload
	if( ReloadDevice && (int)(tickcount - ReloadDeviceTick) > 0) {
		ReloadDevice = false;
		FreeDirectInputDevice();
		CreateDirectInputDevice();
	}

	// wheel rotation detection
	tjs_uint32 shift = TVPGetCurrentShiftKeyState();
	if( TVPWheelDetectionType == wdtDirectInput ) {
		CreateDirectInputDevice();
		if( /*!showingmenu && */ focused && TJSNativeInstance && DIWheelDevice /*&& PaintBox*/ ) {
			tjs_int delta = DIWheelDevice->GetWheelDelta();
			if( delta ) {
				POINT origin = {0,0};
				::ClientToScreen( GetHandle(), &origin );
				//TPoint origin;
				//origin = PaintBox->ClientToScreen(TPoint(0, 0));

				POINT mp = {0, 0};
				::GetCursorPos(&mp);
				tjs_int x = mp.x - origin.x;
				tjs_int y = mp.y - origin.y;
				TVPPostInputEvent(new tTVPOnMouseWheelInputEvent(TJSNativeInstance, shift, delta, x, y));
			}
		}
	}

	// pad detection
	if( TVPJoyPadDetectionType == jdtDirectInput ) {
		CreateDirectInputDevice();
		if( DIPadDevice && TJSNativeInstance /*&& PaintBox*/ ) {
			if( /*!showingmenu &&*/ focused )
				DIPadDevice->UpdateWithCurrentState();
			else
				DIPadDevice->UpdateWithSuspendedState();

			const std::vector<WORD> & uppedkeys  = DIPadDevice->GetUppedKeys();
			const std::vector<WORD> & downedkeys = DIPadDevice->GetDownedKeys();
			const std::vector<WORD> & repeatkeys = DIPadDevice->GetRepeatKeys();
			std::vector<WORD>::const_iterator i;

			// for upped pad buttons
			for(i = uppedkeys.begin(); i != uppedkeys.end(); i++) {
				InternalKeyUp(*i, shift);
			}
			// for downed pad buttons
			for(i = downedkeys.begin(); i != downedkeys.end(); i++) {
				InternalKeyDown(*i, shift);
			}
			// for repeated pad buttons
			for(i = repeatkeys.begin(); i != repeatkeys.end(); i++) {
				InternalKeyDown(*i, shift|TVP_SS_REPEAT);
			}
		}
	}


	// check RecheckInputState
	if( tickcount - LastRecheckInputStateSent > 1000 ) {
		LastRecheckInputStateSent = tickcount;
		if(TJSNativeInstance) TJSNativeInstance->RecheckInputState();
	}
}
bool TTVPWindowForm::GetWindowActive() {
	return ::GetForegroundWindow() == GetHandle();
}
void TTVPWindowForm::SetDrawDeviceDestRect()
{
	tjs_int x_ofs = 0;
	tjs_int y_ofs = 0;
	tTVPRect rt;
	GetClientRect( rt );

	tTVPRect destrect( rt.left + x_ofs, rt.top + y_ofs, rt.right + x_ofs, rt.bottom + y_ofs);

	if( LastSentDrawDeviceDestRect != destrect ) {
		if( TJSNativeInstance ) TJSNativeInstance->GetDrawDevice()->SetDestRectangle(destrect);
		LastSentDrawDeviceDestRect = destrect;
	}
}
void TTVPWindowForm::OnPaint() {
	// a painting event
	if( NextSetWindowHandleToDrawDevice ) {
		bool ismain = false;
		if( TJSNativeInstance ) ismain = TJSNativeInstance->IsMainWindow();
		if( TJSNativeInstance ) TJSNativeInstance->GetDrawDevice()->SetTargetWindow( GetHandle(), ismain);
		NextSetWindowHandleToDrawDevice = false;
	}

	SetDrawDeviceDestRect();

	if( TJSNativeInstance ) {
		tTVPRect r;
		GetClientRect( r );
		TJSNativeInstance->NotifyWindowExposureToLayer(r);
	}
}

void TTVPWindowForm::SetUseMouseKey( bool b ) {
	UseMouseKey = b;
	if( b ) {
		MouseLeftButtonEmulatedPushed = false;
		MouseRightButtonEmulatedPushed = false;
		LastMouseKeyTick = GetTickCount();
	} else {
		if(MouseLeftButtonEmulatedPushed) {
			MouseLeftButtonEmulatedPushed = false;
			OnMouseUp( mbLeft, 0, LastMouseMovedPos.x, LastMouseMovedPos.y);
		}
		if(MouseRightButtonEmulatedPushed) {
			MouseRightButtonEmulatedPushed = false;
			OnMouseUp( mbRight, 0, LastMouseMovedPos.x, LastMouseMovedPos.y);
		}
	}
}
bool TTVPWindowForm::GetUseMouseKey() const {
	return UseMouseKey;
}

void TTVPWindowForm::SetTrapKey(bool b) {
	TrapKeys = b;
	if( TrapKeys ) {
		// reset CanReceiveTrappedKeys and InReceivingTrappedKeys
		CanReceiveTrappedKeys = false;
		InReceivingTrappedKeys = false;
		// note that SetTrapKey can be called while the key trapping is processing.
	}
}
bool TTVPWindowForm::GetTrapKey() const {
	return TrapKeys;
}


void TTVPWindowForm::HideMouseCursor() {
	// hide mouse cursor temporarily
    SetMouseCursorState(mcsTempHidden);
}
void TTVPWindowForm::SetMouseCursorState(tTVPMouseCursorState mcs) {
	if(MouseCursorState == mcsVisible && mcs != mcsVisible) {
		// formerly visible and newly invisible
		if(!ForceMouseCursorVisible) SetMouseCursorVisibleState(false);
	} else if(MouseCursorState != mcsVisible && mcs == mcsVisible) {
		// formerly invisible and newly visible
		if(!ForceMouseCursorVisible) SetMouseCursorVisibleState(true);
	}

	if(MouseCursorState != mcs && mcs == mcsTempHidden) {
		POINT pt;
		::GetCursorPos(&pt);
		LastMouseScreenX = pt.x;
		LastMouseScreenY = pt.y;
	}
	MouseCursorState = mcs;
}

void TTVPWindowForm::RestoreMouseCursor() {
	// restore mouse cursor hidden by HideMouseCursor
	if( MouseCursorState == mcsTempHidden ) {
		POINT pt;
		::GetCursorPos(&pt);
		if( LastMouseScreenX != pt.x || LastMouseScreenY != pt.y ) {
			SetMouseCursorState(mcsVisible);
		}
	}
}
void TTVPWindowForm::SetMouseCursorVisibleState(bool b) {
	// set mouse cursor visible state
	// this does not look MouseCursorState
	if(b)
		SetMouseCursorToWindow( CurrentMouseCursor );
	else
		SetMouseCursorToWindow( MouseCursor(crNone) );
}
void TTVPWindowForm::SetForceMouseCursorVisible(bool s) {
	if(ForceMouseCursorVisible != s) {
		if(s) {
			// force visible mode
			// the cursor is to be fixed in crDefault
			SetMouseCursorToWindow( MouseCursor(crDefault) );
		} else {
			// normal mode
			// restore normal cursor
			SetMouseCursorVisibleState(MouseCursorState == mcsVisible);
		}
		ForceMouseCursorVisible = s;
	}
}
void TTVPWindowForm::SetMouseCursorToWindow( MouseCursor& cursor ) {
	cursor.SetCursor();
}
void TTVPWindowForm::InternalSetPaintBoxSize() {
	//tjs_int l = MulDiv(LayerLeft,   ActualZoomNumer, ActualZoomDenom);
	//tjs_int t = MulDiv(LayerTop,    ActualZoomNumer, ActualZoomDenom);
	tjs_int w = MulDiv(LayerWidth,  ActualZoomNumer, ActualZoomDenom);
	tjs_int h = MulDiv(LayerHeight, ActualZoomNumer, ActualZoomDenom);
	//PaintBox->SetBounds(l, t, w, h);
	SetInnerSize( w, h );
	SetDrawDeviceDestRect();
}
void TTVPWindowForm::AdjustNumerAndDenom(tjs_int &n, tjs_int &d){
	tjs_int a = n;
	tjs_int b = d;
	while( b ) {
		tjs_int t = b;
		b = a % b;
		a = t;
	}
	n = n / a;
	d = d / a;
}
void TTVPWindowForm::SetZoom( tjs_int numer, tjs_int denom, bool set_logical ) {
	// set layer zooming factor;
	// the zooming factor is passed in numerator/denoiminator style.
	// we must find GCM to optimize numer/denium via Euclidean algorithm.
	AdjustNumerAndDenom(numer, denom);
	if( set_logical ) {
		ZoomNumer = numer;
		ZoomDenom = denom;
	}
	if( !GetFullScreenMode() ) {
		// in fullscreen mode, zooming factor is controlled by the system
		ActualZoomDenom = denom;
		ActualZoomNumer = numer;
	}
	InternalSetPaintBoxSize();
}
void TTVPWindowForm::SetFullScreenMode( bool b ) {
	// note that we should not change the display mode when showing overlay videos.
	CallWindowDetach(false); // notify to plugin
	try {
		if(TJSNativeInstance) TJSNativeInstance->DetachVideoOverlay();
		FreeDirectInputDevice();
		// due to re-create window (but current implementation may not re-create the window)

		if( b ) {
			if(TVPFullScreenedWindow == this) return;
			if(TVPFullScreenedWindow) TVPFullScreenedWindow->SetFullScreenMode(false);

			// save position and size
			OrgLeft = GetLeft();
			OrgTop = GetTop();
			OrgWidth = GetWidth();
			OrgHeight = GetHeight();

			// determin desired full screen size
			tjs_int desired_fs_w = InnerWidthSave;
			tjs_int desired_fs_h = InnerHeightSave;

			// set ScrollBox' border invisible
			//OrgInnerSunken = GetInnerSunken();
			//ScrollBox->BorderStyle = Forms::bsNone;

			// set BorderStyle
			OrgStyle = GetWindowLong(GetHandle(), GWL_STYLE);
			OrgExStyle = GetWindowLong(GetHandle(), GWL_EXSTYLE);
			//OrgScrollBoxBorderStyle = ScrollBox->BorderStyle;
			SetWindowLong( GetHandle(), GWL_STYLE, WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPED );
			//ScrollBox->BorderStyle = Forms::bsNone;

			// try to switch to fullscreen
			try {
				TVPSwitchToFullScreen( GetHandle(), desired_fs_w, desired_fs_h );
			} catch(...) {
				SetFullScreenMode(false);
				return;
			}

			// get resulted screen size
			tjs_int fs_w = TVPFullScreenMode.Width;
			tjs_int fs_h = TVPFullScreenMode.Height;

			// determine fullscreen zoom factor and ScrollBox size
			int sb_w, sb_h, zoom_d, zoom_n;
			zoom_d = TVPFullScreenMode.ZoomDenom;
			zoom_n = TVPFullScreenMode.ZoomNumer;
			sb_w = desired_fs_w * zoom_n / zoom_d;
			sb_h = desired_fs_h * zoom_n / zoom_d;

			SetZoom(zoom_n, zoom_d, false);

			// indicate fullscreen state
			TVPFullScreenedWindow = this;

			// reset window size
			SetLeft( 0 );
			SetTop( 0 );
			SetWidth( fs_w );
			SetHeight( fs_h );

			// reset ScrollBox size
			/*
			ScrollBox->Align = alNone;
			ScrollBox->Left = (fs_w - sb_w)/2;
			ScrollBox->Top = (fs_h - sb_h)/2;
			ScrollBox->Width = sb_w;
			ScrollBox->Height = sb_h;
			*/

			// float menu bar
			CreateMenuContainer();

			// re-adjust video rect
			if(TJSNativeInstance) TJSNativeInstance->ReadjustVideoRect();

			// activate self
			//Application->BringToFront();
			BringToFront();
			::SetFocus(GetHandle());

			// activate self (again) // Added by W.Dee 2003/11/02
			Sleep(200);
			SetWindowPos( GetHandle(), HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOREPOSITION|SWP_NOSIZE|SWP_SHOWWINDOW );
		} else {
			if(TVPFullScreenedWindow != this) return;

			// dock menu bar
			DestroyMenuContainer();

			// revert from fullscreen
			TVPRevertFromFullScreen( GetHandle() );
			TVPFullScreenedWindow = NULL;

			// revert zooming factor
			ActualZoomDenom = ZoomDenom;
			ActualZoomNumer = ZoomNumer;

			SetZoom(ZoomNumer, ZoomDenom);  // reset zoom factor

			// set BorderStyle
			SetWindowLong(GetHandle(), GWL_STYLE, OrgStyle);
			SetWindowLong(GetHandle(), GWL_EXSTYLE, OrgExStyle);
			//ScrollBox->BorderStyle = OrgScrollBoxBorderStyle;

			// set ScrollBox visible
			//SetInnerSunken(OrgInnerSunken);
			//ScrollBox->Align = alClient;
			// SetWindowPos(ScrollBox->Handle, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);

			// revert the position and size
			SetBounds(OrgLeft, OrgTop, OrgWidth, OrgHeight);

			// re-adjust video rect
			if(TJSNativeInstance) TJSNativeInstance->ReadjustVideoRect();

			// restore
			// WindowState = wsNormal;
		}
	} catch(...) {
		CallWindowAttach();
		throw;
	}
	CallWindowAttach();

	//SetMouseCursor(CurrentMouseCursor);
	CurrentMouseCursor.SetCursor();
}
bool TTVPWindowForm::GetFullScreenMode() const { 
	return false;
}
void TTVPWindowForm::CreateMenuContainer() {
	if( !MenuContainer ) {
		// create MenuContainer
#pragma message( __LOC__ "TODO" )
		// MenuContainer = new TTVPMenuContainerForm(this);
	}
}
void TTVPWindowForm::DestroyMenuContainer() {
	if( MenuContainer ) {
		// delete MeunContainer
		delete MenuContainer;
		MenuContainer = NULL;
	}
}
void TTVPWindowForm::CallWindowDetach(bool close) {
	if( TJSNativeInstance ) TJSNativeInstance->GetDrawDevice()->SetTargetWindow( NULL, false );

	tTVPWindowMessage msg;
	msg.Msg = TVP_WM_DETACH;
	msg.LParam = 0;
	msg.WParam = close ? 1 : 0;
	msg.Result = 0;
	DeliverMessageToReceiver(msg);
}
//---------------------------------------------------------------------------
void TTVPWindowForm::CallWindowAttach() {
	NextSetWindowHandleToDrawDevice = true;
	LastSentDrawDeviceDestRect.clear();

	tTVPWindowMessage msg;
	msg.Msg = TVP_WM_ATTACH;
	msg.LParam = reinterpret_cast<int>(GetWindowHandleForPlugin());
	msg.WParam = 0;
	msg.Result = 0;

	DeliverMessageToReceiver(msg);
}

bool TTVPWindowForm::InternalDeliverMessageToReceiver(tTVPWindowMessage &msg) {
	if( !TJSNativeInstance ) return false;
	if( TVPPluginUnloadedAtSystemExit ) return false;

	tObjectListSafeLockHolder<tTVPMessageReceiverRecord> holder(WindowMessageReceivers);
	tjs_int count = WindowMessageReceivers.GetSafeLockedObjectCount();

	bool block = false;
	for( tjs_int i = 0; i < count; i++ ) {
		tTVPMessageReceiverRecord *item = WindowMessageReceivers.GetSafeLockedObjectAt(i);
		if(!item) continue;
		bool b = item->Deliver(&msg);
		block = block || b;
	}
	return block;
}

void TTVPWindowForm::UpdateWindow(tTVPUpdateType type ) {
	if( TJSNativeInstance ) {
		tTVPRect r;
		r.left = 0;
		r.top = 0;
		r.right = LayerWidth;
		r.bottom = LayerHeight;
		TJSNativeInstance->NotifyWindowExposureToLayer(r);
		TVPDeliverWindowUpdateEvents();
	}
}

void TTVPWindowForm::ShowWindowAsModal() {
	// TODO: what's modalwindowlist ?
	ModalResult = 0;
	InMode = true;
	TVPAddModalWindow(this); // add to modal window list
	try {
		ShowModal();
	} catch(...) {
		TVPRemoveModalWindow(this);
		InMode = false;
		throw;
	}
	TVPRemoveModalWindow(this);
	InMode = false;
}


void TTVPWindowForm::RegisterWindowMessageReceiver(tTVPWMRRegMode mode, void * proc, const void *userdata) {
	if( mode == wrmRegister ) {
		// register
		tjs_int count = WindowMessageReceivers.GetCount();
		tjs_int i;
		for(i = 0 ; i < count; i++) {
			tTVPMessageReceiverRecord *item = WindowMessageReceivers[i];
			if(!item) continue;
			if((void*)item->Proc == proc) break; // have already registered
		}
		if(i == count) {
			// not have registered
			tTVPMessageReceiverRecord *item = new tTVPMessageReceiverRecord();
			item->Proc = (tTVPWindowMessageReceiver)proc;
			item->UserData = userdata;
			WindowMessageReceivers.Add(item);
		}
	} else if(mode == wrmUnregister) {
		// unregister
		tjs_int count = WindowMessageReceivers.GetCount();
		for(tjs_int i = 0 ; i < count; i++) {
			tTVPMessageReceiverRecord *item = WindowMessageReceivers[i];
			if(!item) continue;
			if((void*)item->Proc == proc) {
				// found
				WindowMessageReceivers.Remove(i);
				delete item;
			}
		}
		WindowMessageReceivers.Compact();
	}
}
void TTVPWindowForm::OnClose( CloseAction& action ) {
	//if(ModalResult == 0) Action = caNone; else Action = caHide;
	if( ProgramClosing ) {
		if( TJSNativeInstance ) {
			if( TJSNativeInstance->IsMainWindow() ) {
				// this is the main window
			} else 			{
				// not the main window
				action = caFree;
			}
			if( TVPFullScreenedWindow != this ) {
				// if this is not a fullscreened window
				SetVisible( false );
			}
			iTJSDispatch2 * obj = TJSNativeInstance->GetOwnerNoAddRef();
			TJSNativeInstance->NotifyWindowClose();
			obj->Invalidate(0, NULL, NULL, obj);
			TJSNativeInstance = NULL;
		}
	}
}
void TTVPWindowForm::Close() {
	// closing action by "close" method
	if( Closing ) return; // already waiting closing...

	ProgramClosing = true;
	try {
		//::DestroyWindow( GetHandle() );
		SendCloseMessage();
	} catch(...) {
		ProgramClosing = false;
		throw;
	}
	ProgramClosing = false;
}
void TTVPWindowForm::InvalidateClose() {
	// closing action by object invalidation;
	// this will not cause any user confirmation of closing the window.
	TJSNativeInstance = NULL;
	SetVisible( false );
	delete this;
}
void TTVPWindowForm::OnCloseQueryCalled( bool b ) {
	// closing is allowed by onCloseQuery event handler
	if( !ProgramClosing ) {
		// closing action by the user
		if( b ) {
			if( InMode )
				ModalResult = 1; // when modal
			else
				SetVisible( false );  // just hide

			Closing = false;
			if( TJSNativeInstance ) {
				if( TJSNativeInstance->IsMainWindow() ) {
					// this is the main window
					iTJSDispatch2 * obj = TJSNativeInstance->GetOwnerNoAddRef();
					obj->Invalidate(0, NULL, NULL, obj);
					TJSNativeInstance = NULL;
				}
			} else {
				delete this;
			}
		} else {
			Closing = false;
		}
	} else {
		// closing action by the program
		CanCloseWork = b;
	}
}
void TTVPWindowForm::SendCloseMessage() {
	::PostMessage(GetHandle(), WM_CLOSE, 0, 0);
}

void TTVPWindowForm::ZoomRectangle( tjs_int& left, tjs_int& top, tjs_int& right, tjs_int& bottom) {
	left =   MulDiv(left  ,  ActualZoomNumer, ActualZoomDenom);
	top =    MulDiv(top   ,  ActualZoomNumer, ActualZoomDenom);
	right =  MulDiv(right ,  ActualZoomNumer, ActualZoomDenom);
	bottom = MulDiv(bottom,  ActualZoomNumer, ActualZoomDenom);
}

HWND TTVPWindowForm::GetMenuOwnerWindowHandle() {
	if(MenuContainer) {
		// this is a quick hack which let MenuContainer be visible,
		// because the menu owner window must be visible to receive menu command.
		MenuContainer->PrepareToReceiveMenuCommand();
		return MenuContainer->GetHandle();
	}
	return GetHandle();
}
HWND TTVPWindowForm::GetSurfaceWindowHandle() {
	return GetHandle();
}
HWND TTVPWindowForm::GetWindowHandle(tjs_int &ofsx, tjs_int &ofsy) {
	/*
	if( ScrollBox ) {
		SetWindowPos(ScrollBox->Handle, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
	}
	ofsx = ofsy = GetInnerSunken()?2:0;
	ofsx += ScrollBox->Align == alClient ? 0 : ScrollBox->Left;
	ofsy += ScrollBox->Align == alClient ? 0 : ScrollBox->Top;
	*/
	RECT rt;
	::GetWindowRect( GetHandle(), &rt );
	POINT pt = { rt.left, rt.top };
	ScreenToClient( GetHandle(), &pt );
	ofsx = -pt.x;
	ofsy = -pt.y;
	return GetHandle();
}
HWND TTVPWindowForm::GetWindowHandleForPlugin() {
	return GetHandle();
}

void TTVPWindowForm::SetMenuBarVisible( bool b ) { 
	MenuBarVisible = b;
	SetMenu( b ? MainMenu : NULL );
}
bool TTVPWindowForm::GetMenuBarVisible() const {
	return MenuBarVisible;
}
/**
 メニューバー表示を復元
 */
void TTVPWindowForm::RevertMenuBarVisible() {
	SetMenu( MenuBarVisible ? MainMenu : NULL );
}
void TTVPWindowForm::ResetDrawDevice() {
	NextSetWindowHandleToDrawDevice = true;
	LastSentDrawDeviceDestRect.clear();
	// if(PaintBox) PaintBox->Invalidate();
	::InvalidateRect( GetHandle(), NULL, FALSE );
}

void TTVPWindowForm::InternalKeyUp(WORD key, tjs_uint32 shift) {
	DWORD tick = GetTickCount();
	TVPPushEnvironNoise(&tick, sizeof(tick));
	TVPPushEnvironNoise(&key, sizeof(key));
	TVPPushEnvironNoise(&shift, sizeof(shift));
	if( TJSNativeInstance ) {
		if( UseMouseKey /*&& PaintBox*/ ) {
			if( key == VK_RETURN || key == VK_SPACE || key == VK_ESCAPE || key == VK_PAD1 || key == VK_PAD2) {
				POINT p;
				::GetCursorPos(&p);
				//TPoint tp;
				//tp.x = p.x; tp.y = p.y;
				//tp = ScrollBox->ScreenToClient(tp);
				//if(tp.x >= 0 && tp.y >= 0 && tp.x < ScrollBox->Width && tp.y < ScrollBox->Height) {
				::ScreenToClient( GetHandle(), &p );
				if( p.x >= 0 && p.y >= 0 && p.x < GetInnerWidth() && p.y < GetInnerHeight() ) {
					if( key == VK_RETURN || key == VK_SPACE || key == VK_PAD1 ) {
						//PaintBoxClick(PaintBox);
						OnMouseClick( mbLeft, 0, p.x, p.y );
						MouseLeftButtonEmulatedPushed = false;
						//PaintBoxMouseUp(PaintBox, Controls::mbLeft, TShiftState(), tp.x, tp.y);
						OnMouseUp( mbLeft, 0, p.x, p.y );
					}

					if( key == VK_ESCAPE || key == VK_PAD2 ) {
						MouseRightButtonEmulatedPushed = false;
						//PaintBoxMouseUp(PaintBox, Controls::mbRight, TShiftState(), tp.x, tp.y);
						OnMouseUp( mbRight, 0, p.x, p.y );
					}
				}
				return;
			}
		}

		TVPPostInputEvent(new tTVPOnKeyUpInputEvent(TJSNativeInstance, key, shift));
	}
}
void TTVPWindowForm::InternalKeyDown(WORD key, tjs_uint32 shift) {
	DWORD tick = GetTickCount();
	TVPPushEnvironNoise(&tick, sizeof(tick));
	TVPPushEnvironNoise(&key, sizeof(key));
	TVPPushEnvironNoise(&shift, sizeof(shift));

	if( TJSNativeInstance ) {
		if(UseMouseKey /*&& PaintBox*/ ) {
			if(key == VK_RETURN || key == VK_SPACE || key == VK_ESCAPE || key == VK_PAD1 || key == VK_PAD2) {
				POINT p;
				::GetCursorPos(&p);
				//TPoint tp;
				//tp.x = p.x; tp.y = p.y;
				//tp = ScrollBox->ScreenToClient(tp);
				//if( tp.x >= 0 && tp.y >= 0 && tp.x < ScrollBox->Width && tp.y < ScrollBox->Height) {
				::ScreenToClient( GetHandle(), &p );
				if( p.x >= 0 && p.y >= 0 && p.x < GetInnerWidth() && p.y < GetInnerHeight() ) {
					if( key == VK_RETURN || key == VK_SPACE || key == VK_PAD1 ) {
						MouseLeftButtonEmulatedPushed = true;
						//PaintBoxMouseDown(PaintBox, Controls::mbLeft, TShiftState(), tp.x, tp.y);
						OnMouseDown( mbLeft, 0, p.x, p.y );
					}

					if(key == VK_ESCAPE || key == VK_PAD2) {
						MouseRightButtonEmulatedPushed = true;
						//PaintBoxMouseDown(PaintBox, Controls::mbRight, TShiftState(), tp.x, tp.y);
						OnMouseDown( mbLeft, 0, p.x, p.y );
					}
				}
				return;
			}

			switch(key) {
			case VK_LEFT:
			case VK_PADLEFT:
				if( MouseKeyXAccel == 0 && MouseKeyYAccel == 0 ) {
					GenerateMouseEvent(true, false, false, false);
					LastMouseKeyTick = GetTickCount() + 100;
				}
				return;
			case VK_RIGHT:
			case VK_PADRIGHT:
				if(MouseKeyXAccel == 0 && MouseKeyYAccel == 0)
				{
					GenerateMouseEvent(false, true, false, false);
					LastMouseKeyTick = GetTickCount() + 100;
				}
				return;
			case VK_UP:
			case VK_PADUP:
				if(MouseKeyXAccel == 0 && MouseKeyYAccel == 0)
				{
					GenerateMouseEvent(false, false, true, false);
					LastMouseKeyTick = GetTickCount() + 100;
				}
				return;
			case VK_DOWN:
			case VK_PADDOWN:
				if(MouseKeyXAccel == 0 && MouseKeyYAccel == 0)
				{
					GenerateMouseEvent(false, false, false, true);
					LastMouseKeyTick = GetTickCount() + 100;
				}
				return;
			}
		}
		TVPPostInputEvent(new tTVPOnKeyDownInputEvent(TJSNativeInstance, key, shift));
	}
}
void TTVPWindowForm::GenerateMouseEvent(bool fl, bool fr, bool fu, bool fd) {
	//if(!PaintBox) return;

	if( !fl && !fr && !fu && !fd ) {
		if(GetTickCount() - 45 < LastMouseKeyTick) return;
	}

	bool shift = 0!=(GetAsyncKeyState(VK_SHIFT) & 0x8000);
	bool left = fl || GetAsyncKeyState(VK_LEFT) & 0x8000 || TVPGetJoyPadAsyncState(VK_PADLEFT, true);
	bool right = fr || GetAsyncKeyState(VK_RIGHT) & 0x8000 || TVPGetJoyPadAsyncState(VK_PADRIGHT, true);
	bool up = fu || GetAsyncKeyState(VK_UP) & 0x8000 || TVPGetJoyPadAsyncState(VK_PADUP, true);
	bool down = fd || GetAsyncKeyState(VK_DOWN) & 0x8000 || TVPGetJoyPadAsyncState(VK_PADDOWN, true);

	DWORD flags = 0;
	if(left || right || up || down) flags |= MOUSEEVENTF_MOVE;

	if(!right && !left && !up && !down) {
		LastMouseMoved = false;
		MouseKeyXAccel = MouseKeyYAccel = 0;
	}

	if(!shift) {
		if(!right && left && MouseKeyXAccel > 0) MouseKeyXAccel = -0;
		if(!left && right && MouseKeyXAccel < 0) MouseKeyXAccel = 0;
		if(!down && up && MouseKeyYAccel > 0) MouseKeyYAccel = -0;
		if(!up && down && MouseKeyYAccel < 0) MouseKeyYAccel = 0;
	} else {
		if(left) MouseKeyXAccel = -TVP_MOUSE_SHIFT_ACCEL;
		if(right) MouseKeyXAccel = TVP_MOUSE_SHIFT_ACCEL;
		if(up) MouseKeyYAccel = -TVP_MOUSE_SHIFT_ACCEL;
		if(down) MouseKeyYAccel = TVP_MOUSE_SHIFT_ACCEL;
	}

	if(right || left || up || down) {
		if(left) if(MouseKeyXAccel > -TVP_MOUSE_MAX_ACCEL)
			MouseKeyXAccel = MouseKeyXAccel?MouseKeyXAccel - 2:-2;
		if(right) if(MouseKeyXAccel < TVP_MOUSE_MAX_ACCEL)
			MouseKeyXAccel = MouseKeyXAccel?MouseKeyXAccel + 2:+2;
		if(!left && !right) {
			if(MouseKeyXAccel > 0) MouseKeyXAccel--;
			else if(MouseKeyXAccel < 0) MouseKeyXAccel++;
		}

		if(up) if(MouseKeyYAccel > -TVP_MOUSE_MAX_ACCEL)
			MouseKeyYAccel = MouseKeyYAccel?MouseKeyYAccel - 2:-2;
		if(down) if(MouseKeyYAccel < TVP_MOUSE_MAX_ACCEL)
			MouseKeyYAccel = MouseKeyYAccel?MouseKeyYAccel + 2:+2;
		if(!up && !down) {
			if(MouseKeyYAccel > 0) MouseKeyYAccel--;
			else if(MouseKeyYAccel < 0) MouseKeyYAccel++;
		}

	}

	if(flags) {
		POINT pt;
		if(::GetCursorPos(&pt)) {
			::SetCursorPos( pt.x + (MouseKeyXAccel>>1), pt.y + (MouseKeyYAccel>>1)); 
		}
		LastMouseMoved = true;
	}
	LastMouseKeyTick = GetTickCount();
}

void TTVPWindowForm::SetPaintBoxSize(tjs_int w, tjs_int h) {
	LayerWidth  = w;
	LayerHeight = h;
	InternalSetPaintBoxSize();
}

void TTVPWindowForm::SetDefaultMouseCursor() {
	if( !CurrentMouseCursor.IsCurrentCursor(crDefault ) ) {
		if( MouseCursorState == mcsVisible && !ForceMouseCursorVisible ) {
			SetMouseCursorToWindow(MouseCursor(crDefault));
		}
	}
	CurrentMouseCursor.SetCursorIndex(crDefault);
}
void TTVPWindowForm::SetMouseCursor( tjs_int handle ) {
	if( !CurrentMouseCursor.IsCurrentCursor(handle ) ) {
		if(MouseCursorState == mcsVisible && !ForceMouseCursorVisible) {
			SetMouseCursorToWindow( MouseCursor(handle) );
		}
	}
	CurrentMouseCursor.SetCursorIndex(handle);
}
/**
 * クライアント領域座標からウィンドウ領域座標へ変換する
 */
void TTVPWindowForm::OffsetClientPoint( int &x, int &y ) {
	POINT origin = {0,0};
	::ClientToScreen( GetHandle(), &origin );
	x = -origin.x;
	y = -origin.y;
}
void TTVPWindowForm::GetCursorPos(tjs_int &x, tjs_int &y) {
	// get mouse cursor position in client
	POINT origin = {0,0};
	::ClientToScreen( GetHandle(), &origin );
	POINT mp = {0, 0};
	::GetCursorPos(&mp);
	x = mp.x - origin.x;
	y = mp.y - origin.y;
}
void TTVPWindowForm::SetCursorPos(tjs_int x, tjs_int y) {
	POINT pt = {x,y};
	::ClientToScreen( GetHandle(), &pt );
	::SetCursorPos(pt.x, pt.y);
	LastMouseScreenX = LastMouseScreenY = -1; // force to display mouse cursor
	RestoreMouseCursor();
}
void TTVPWindowForm::UnacquireImeControl() {
	if( TVPControlImeState ) {
		GetIME()->Reset();
	}
}

TTVPWindowForm * TTVPWindowForm::GetKeyTrapperWindow() {
	// find most recent "trapKeys = true" window and return it.
	// returnts "this" window if there is no trapping window.
	tjs_int count = TVPGetWindowCount();
	for( tjs_int i = count - 1; i >= 0; i-- ) {
		tTJSNI_Window * win = TVPGetWindowListAt(i);
		if( win ) {
			TTVPWindowForm * form = win->GetForm();
			if( form && form != this ) {
				if( form->TrapKeys && form->GetVisible() ) {
					// found
					return form;
				}
			}
		}
	}
	return this;
}

int TTVPWindowForm::ConvertImeMode( tTVPImeMode mode ) {
	switch( mode ) {
	case ::imDisable   : return ImeControl::ModeClose   ; // (*)
	case ::imClose     : return ImeControl::ModeClose   ;
	case ::imOpen      : return ImeControl::ModeOpen    ;
	case ::imDontCare  : return ImeControl::ModeDontCare;
	case ::imSAlpha    : return ImeControl::ModeSAlpha  ;
	case ::imAlpha     : return ImeControl::ModeAlpha   ;
	case ::imHira      : return ImeControl::ModeHira    ;
	case ::imSKata     : return ImeControl::ModeSKata   ;
	case ::imKata      : return ImeControl::ModeKata    ;
	case ::imChinese   : return ImeControl::ModeChinese ;
	case ::imSHanguel  : return ImeControl::ModeSHanguel;
	case ::imHanguel   : return ImeControl::ModeHanguel ;
	}
	return ImeControl::ModeDontCare;
}
void TTVPWindowForm::AcquireImeControl() {
	if( HasFocus() ) {
		// find key trapper window ...
		TTVPWindowForm * trapper = GetKeyTrapperWindow();

		// force to access protected some methods.
		// much nasty way ...
		if( TVPControlImeState ) {
			GetIME()->Reset();
			tTVPImeMode newmode = trapper->LastSetImeMode;
			if( GetIME()->IsEnableThisLocale() )
				GetIME()->Enable();
			GetIME()->SetIme(ConvertImeMode(newmode));
		}

		if( trapper->AttentionPointEnabled ) {
			::SetCaretPos( trapper->AttentionPoint.x, trapper->AttentionPoint.y );
			if( trapper == this ) {
				GetIME()->SetCompositionWindow( AttentionPoint.x, AttentionPoint.y );
				GetIME()->SetCompositionFont( AttentionFont);
			} else 			{
				// disable IMM composition window
				GetIME()->Disable();
			}
		}
	}
}
void TTVPWindowForm::SetImeMode(tTVPImeMode mode) {
	LastSetImeMode = mode;
	AcquireImeControl();
}
void TTVPWindowForm::SetDefaultImeMode(tTVPImeMode mode, bool reset) {
	DefaultImeMode = mode;
	if(reset) ResetImeMode();
}
void TTVPWindowForm::ResetImeMode() {
	SetImeMode(DefaultImeMode);
}

void TTVPWindowForm::SetAttentionPoint(tjs_int left, tjs_int top, TFont *font) {
	OffsetClientPoint( left, top );

	// set attention point information
	AttentionPoint.x = left;
	AttentionPoint.y = top;
	AttentionPointEnabled = true;
	if( font ) {
		AttentionFont->Assign(font);
	} else {
		TFont * default_font = new TFont();
		AttentionFont->Assign(default_font);
		delete default_font;
	}
	AcquireImeControl();
}
void TTVPWindowForm::DisableAttentionPoint() {
	AttentionPointEnabled = false;
}
void TTVPWindowForm::CreateDirectInputDevice() {
	if( !DIWheelDevice ) DIWheelDevice = new tTVPWheelDirectInputDevice(GetHandle());
	if( !DIPadDevice ) DIPadDevice = new tTVPPadDirectInputDevice(GetHandle());
}
void TTVPWindowForm::FreeDirectInputDevice() {
	if( DIWheelDevice ) {
		delete DIWheelDevice;
		DIWheelDevice = NULL;
	}
	if( DIPadDevice ) {
		delete DIPadDevice;
		DIPadDevice = NULL;
	}
}
LRESULT TTVPWindowForm::CMMouseEnter() {
	return 0;
}
LRESULT TTVPWindowForm::CMMouseLeave() {
	return 0;
}
LRESULT TTVPWindowForm::WMMouseActivate() {
	return 0;
}
LRESULT TTVPWindowForm::WMMove() {
	return 0;
}
LRESULT TTVPWindowForm::WMDropFiles() {
	return 0;
}
LRESULT TTVPWindowForm::WMShowVisible() {
	return 0;
}
LRESULT TTVPWindowForm::WMShowTop() {
	return 0;
}
LRESULT TTVPWindowForm::WMRetrieveFocus() {
	return 0;
}
LRESULT TTVPWindowForm::WMAcquireImeControl() {
	return 0;
}
LRESULT TTVPWindowForm::WMEnable() {
	return 0;
}
LRESULT TTVPWindowForm::WMSetFocus() {
	return 0;
}
LRESULT TTVPWindowForm::WMKillFocus() {
	return 0;
}

LRESULT TTVPWindowForm::WMEnterMenuLoop() {
	return 0;
}
LRESULT TTVPWindowForm::WMExitMenuLoop() {
	return 0;
}

LRESULT TTVPWindowForm::WMKeyDown() {
	return 0;
}

LRESULT TTVPWindowForm::WMDeviceChange() {
	return 0;
}

LRESULT TTVPWindowForm::WMNCLButtonDown() {
	return 0;
}
LRESULT TTVPWindowForm::WMNCRButtonDown() {
	return 0;
}

void TTVPWindowForm::OnKeyDown( WORD vk, int shift, int repreat, bool prevkeystate ) {
	if(TJSNativeInstance) {
		tjs_uint32 s = TVP_TShiftState_To_uint32( shift );
		InternalKeyDown( vk, repreat > 0 ? s|TVP_SS_REPEAT : s );
	}
}
void TTVPWindowForm::OnKeyUp( WORD vk, int shift ) {
	tjs_uint32 s = TVP_TShiftState_To_uint32(shift);
	InternalKeyUp(vk, s );
}
void TTVPWindowForm::OnKeyPress( WORD vk, int repreat, bool prevkeystate, bool convertkey ) {
	if( TJSNativeInstance && vk ) {
		if(UseMouseKey && (vk == 0x1b || vk == 13 || vk == 32)) return;
		if( PendingKeyCodes.empty() != true ) {
			// pending keycode
			PendingKeyCodes += static_cast<char>(vk);
			wchar_t dest;
			int res = TJS_mbtowc(&dest, PendingKeyCodes.c_str(), PendingKeyCodes.length());
			if( res > 0 ) {
				// convertion succeeded
				TVPPostInputEvent(new tTVPOnKeyPressInputEvent(TJSNativeInstance, dest));
				PendingKeyCodes.clear();
			}
		} else 		{
			char key[2];
			key[0] = static_cast<char>(vk);
			key[1] = 0;
			wchar_t dest;
			int res = TJS_mbtowc(&dest, key, 1);
			if( res > 0 ) {
				// convertion succeeded
				TVPPostInputEvent(new tTVPOnKeyPressInputEvent(TJSNativeInstance, dest));
			} else {
				PendingKeyCodes = std::string(key);
			}
		}
	}
}
void TTVPWindowForm::TranslateWindowToPaintBox(int &x, int &y) {
	/*
	POINT pt = {x,y};
	::ClientToScreen( GetHandle(), &pt );
	::ScreenToClient( GetHandle(), &pt );
	x = pt.x;
	y = pt.y;
	*/
}

void TTVPWindowForm::FirePopupHide() {
	// fire "onPopupHide" event
	if(!CanSendPopupHide()) return;
	if(!GetVisible()) return;
	TVPPostInputEvent( new tTVPOnPopupHideInputEvent(TJSNativeInstance) );
}

void TTVPWindowForm::DeliverPopupHide() {
	// deliver onPopupHide event to unfocusable windows
	tjs_int count = TVPGetWindowCount();
	for( tjs_int i = count - 1; i >= 0; i-- ) {
		tTJSNI_Window * win = TVPGetWindowListAt(i);
		if( win ){
			TTVPWindowForm * form = win->GetForm();
			if( form ) {
				form->FirePopupHide();
			}
		}
	}
}

void TTVPWindowForm::CheckMenuBarDrop() {
	if( MenuContainer && MenuBarVisible && GetWindowActive() ) {
		POINT pos = {0, GetHeight()};
		::GetCursorPos(&pos);
		if( pos.y <= 0 ) {
#pragma message(__LOC__ "TODO" )
			//MenuContainer->StartDropWatch();
		}
	}
}
void TTVPWindowForm::InvokeShowVisible() {
	// this posts window message which invokes WMShowVisible
	::PostMessage( GetHandle(), TVP_WM_SHOWVISIBLE, 0, 0);
}
//---------------------------------------------------------------------------
void TTVPWindowForm::InvokeShowTop(bool activate) {
	// this posts window message which invokes WMShowTop
	::PostMessage( GetHandle(), TVP_WM_SHOWTOP, activate ? 1:0, 0);
}
//---------------------------------------------------------------------------
HDWP TTVPWindowForm::ShowTop(HDWP hdwp) {
	if( GetVisible() ) {
		hdwp = ::DeferWindowPos(hdwp, GetHandle(), HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOREPOSITION|
			SWP_NOSIZE|WM_SHOWWINDOW);
	}
	return hdwp;
}
void TTVPWindowForm::OnMouseMove( int shift, int x, int y ) {
	TranslateWindowToPaintBox(x, y);
	if( TJSNativeInstance ) {
		tjs_uint32 s = TVP_TShiftState_To_uint32(shift);
		TVPPostInputEvent( new tTVPOnMouseMoveInputEvent(TJSNativeInstance, x, y, s), TVP_EPT_DISCARDABLE );
	}

	CheckMenuBarDrop();
	RestoreMouseCursor();

	int pos = (y << 16) + x;
	TVPPushEnvironNoise(&pos, sizeof(pos));

	LastMouseMovedPos.x = x;
	LastMouseMovedPos.y = y;
}
void TTVPWindowForm::OnMouseDown( int button, int shift, int x, int y ) {
	if( !CanSendPopupHide() ) DeliverPopupHide();

	TranslateWindowToPaintBox( x, y);
	SetMouseCapture();

	LastMouseDownX = x;
	LastMouseDownY = y;

	if(TJSNativeInstance) {
		tjs_uint32 s = TVP_TShiftState_To_uint32(shift);
		tTVPMouseButton b = TVP_TMouseButton_To_tTVPMouseButton(button);
		TVPPostInputEvent( new tTVPOnMouseDownInputEvent(TJSNativeInstance, x, y, b, s));
	}
}
void TTVPWindowForm::OnMouseUp( int button, int shift, int x, int y ) {
	TranslateWindowToPaintBox(x, y);
	ReleaseMouseCapture();
	if(TJSNativeInstance) {
		tjs_uint32 s = TVP_TShiftState_To_uint32(shift);
		tTVPMouseButton b = TVP_TMouseButton_To_tTVPMouseButton(button);
		TVPPostInputEvent( new tTVPOnMouseUpInputEvent(TJSNativeInstance, x, y, b, s));
	}
}
void TTVPWindowForm::OnMouseDoubleClick( int button, int x, int y ) {
	// fire double click event
	if( TJSNativeInstance ) {
		TVPPostInputEvent( new tTVPOnDoubleClickInputEvent(TJSNativeInstance, x, y));
	}
}
void TTVPWindowForm::OnMouseClick( int button, int shift, int x, int y ) {
	// fire click event
	if( TJSNativeInstance ) {
		TVPPostInputEvent( new tTVPOnClickInputEvent(TJSNativeInstance, x, y));
	}
}
void TTVPWindowForm::OnMouseWheel( int delta, int shift, int x, int y ) {
	if( TVPWheelDetectionType == wdtWindowMessage ) {
		// wheel
		if( TJSNativeInstance ) {
			tjs_uint32 s = TVP_TShiftState_To_uint32(shift);
			TVPPostInputEvent(new tTVPOnMouseWheelInputEvent(TJSNativeInstance, shift, delta, x, y));
		}
	}
}

void TTVPWindowForm::OnActive( HWND preactive ) {
	if( TVPFullScreenedWindow == this )
		TVPShowModalAtAppActivate();
}
void TTVPWindowForm::OnDeactive( HWND postactive ) {
	if( TJSNativeInstance ) {
		TVPPostInputEvent( new tTVPOnReleaseCaptureInputEvent(TJSNativeInstance) );
	}
}
