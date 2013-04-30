//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// "Window" TJS Class implementation (VCL side)
//---------------------------------------------------------------------------
#ifndef WindowFormUnitH
#define WindowFormUnitH
//---------------------------------------------------------------------------
#include <vfw.h>

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <ObjectList.h>

#include "WindowIntf.h"



//---------------------------------------------------------------------------
// Options
//---------------------------------------------------------------------------
extern void TVPInitWindowOptions();
extern int TVPFullScreenBPP; // = 0; // 0 for no-change
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// VCL-based constants to TVP-based constants conversion (and vice versa)
//---------------------------------------------------------------------------
tTVPMouseButton TVP_TMouseButton_To_tTVPMouseButton(TMouseButton button);
tjs_uint32 TVP_TShiftState_To_uint32(TShiftState state);
TShiftState TVP_TShiftState_From_uint32(tjs_uint32 state);
tjs_uint32 TVPGetCurrentShiftKeyState();
//---------------------------------------------------------------------------













//---------------------------------------------------------------------------
// TTVPWindowForm
//---------------------------------------------------------------------------
#define TVP_WM_SHOWVISIBLE (WM_USER + 2)
#define TVP_WM_SHOWTOP     (WM_USER + 3)
#define TVP_WM_RETRIEVEFOCUS     (WM_USER + 4)
#define TVP_WM_ACQUIREIMECONTROL    (WM_USER + 5)
extern void TVPShowModalAtAppActivate();
extern void TVPHideModalAtAppDeactivate();
extern HDWP TVPShowModalAtTimer(HDWP);
extern TTVPWindowForm * TVPFullScreenedWindow;
//---------------------------------------------------------------------------
struct tTVPMessageReceiverRecord
{
	tTVPWindowMessageReceiver Proc;
	const void *UserData;
	bool Deliver(tTVPWindowMessage *Message)
	{ return Proc(const_cast<void*>(UserData), Message); }	
};
class tTJSNI_Window;
class TTVPMenuContainerForm;
class tTVPRect;
class tTVPBaseBitmap;
class tTVPWheelDirectInputDevice; // class for DirectInputDevice management
class tTVPPadDirectInputDevice; // class for DirectInputDevice management
class TTVPWindowForm : public TForm
{
__published:	// IDE 管理のコンポーネント
	TScrollBox *ScrollBox;
	TMainMenu *MainMenu;
	TPopupMenu *PopupMenu;
	TMenuItem *ShowControllerMenuItem;
	TMenuItem *ShowWatchMenuItem;
	TMenuItem *ShowConsoleMenuItem;
	TMenuItem *ShowScriptEditorMenuItem;
	TMenuItem *ShowAboutMenuItem;
	TMenuItem *TabMenuItem;
	TMenuItem *ShitTabMenuItem;
	TMenuItem *UpdateRectDebugMenuItem;
	TMenuItem *DumpLayerStructorMenuItem;
	TMenuItem *CopyImportantLogMenuItem;
	TMenuItem *AltEnterMenuItem;
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall ScrollBoxMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
	void __fastcall FormMouseMove(TObject *Sender, TShiftState Shift, int X,
          int Y);
	void __fastcall ShowControllerMenuItemClick(TObject *Sender);
	void __fastcall ShowScriptEditorMenuItemClick(TObject *Sender);
	void __fastcall ShowWatchMenuItemClick(TObject *Sender);
	void __fastcall ShowConsoleMenuItemClick(TObject *Sender);
	void __fastcall ShowAboutMenuItemClick(TObject *Sender);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
	void __fastcall FormKeyPress(TObject *Sender, char &Key);
	void __fastcall FormKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall TabMenuItemClick(TObject *Sender);
	void __fastcall ShitTabMenuItemClick(TObject *Sender);
	void __fastcall UpdateRectDebugMenuItemClick(TObject *Sender);
	void __fastcall FormDeactivate(TObject *Sender);
	void __fastcall DumpLayerStructorMenuItemClick(TObject *Sender);
	void __fastcall FormMouseWheel(TObject *Sender, TShiftState Shift,
          int WheelDelta, TPoint &MousePos, bool &Handled);
	void __fastcall CopyImportantLogMenuItemClick(TObject *Sender);
	void __fastcall AltEnterMenuItemClick(TObject *Sender);
	void __fastcall FormActivate(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);

public:		// ユーザー宣言
	__fastcall TTVPWindowForm(TComponent* Owner, tTJSNI_Window *ni);
private:	// ユーザー宣言
	bool InMode;
	bool Focusable;

public:
	bool GetWindowActive();

private:
	//-- drawdevice related
	bool NextSetWindowHandleToDrawDevice;
	tTVPRect LastSentDrawDeviceDestRect;

	//-- interface to plugin
	tObjectList<tTVPMessageReceiverRecord> WindowMessageReceivers;

	//-- DirectInput related
	tTVPWheelDirectInputDevice *DIWheelDevice;
	tTVPPadDirectInputDevice *DIPadDevice;
	bool ReloadDevice;
	DWORD ReloadDeviceTick;

	//-- TJS object related
	tTJSNI_Window * TJSNativeInstance;
	int LastMouseDownX, LastMouseDownY; // in Layer coodinates

	//-- full screen managemant related
	TPaintBox * PaintBox;
	int InnerWidthSave;
	int InnerHeightSave;
	DWORD OrgStyle;
	DWORD OrgExStyle;
	TBorderStyle OrgScrollBoxBorderStyle;
	int OrgLeft;
	int OrgTop;
	int OrgWidth;
	int OrgHeight;
	bool OrgInnerSunken;
	TTVPMenuContainerForm * MenuContainer;
	DWORD ResetStayOnTopStateTick;

	//-- keyboard input
	AnsiString PendingKeyCodes;

	TImeMode LastSetImeMode;
	bool TrapKeys;
	bool CanReceiveTrappedKeys;
	bool InReceivingTrappedKeys;
	bool InMenuLoop;
	bool IsRepeatMessage;
	bool UseMouseKey; // whether using mouse key emulation
	tjs_int MouseKeyXAccel;
	tjs_int MouseKeyYAccel;
	DWORD LastMouseKeyTick;
	bool LastMouseMoved;
	tTVPImeMode DefaultImeMode;
	bool MouseLeftButtonEmulatedPushed;
	bool MouseRightButtonEmulatedPushed;
	TPoint LastMouseMovedPos;  // in Layer coodinates

	bool AttentionPointEnabled;
	TPoint AttentionPoint;
	TFont *AttentionFont;
public:
	void InternalKeyUp(WORD key, tjs_uint32 shift);
	void InternalKeyDown(WORD key, tjs_uint32 shift);
private:
	void UnacquireImeControl();
	void AcquireImeControl();
	TTVPWindowForm * GetKeyTrapperWindow();
	static bool FindKeyTrapper(LRESULT &result, UINT msg, WPARAM wparam,
		LPARAM lparam);
	bool ProcessTrappedKeyMessage(LRESULT &result, UINT msg, WPARAM wparam,
		LPARAM lparam);

	//-- mouse cursor
	tTVPMouseCursorState MouseCursorState;
	bool ForceMouseCursorVisible; // true in menu select
	TCursor CurrentMouseCursor;
	tjs_int LastMouseScreenX; // managed by RestoreMouseCursor
	tjs_int LastMouseScreenY;

	//-- layer position / size
	tjs_int LayerLeft;
	tjs_int LayerTop;
	tjs_int LayerWidth;
	tjs_int LayerHeight;
	tjs_int ZoomDenom; // Zooming factor denominator (setting)
	tjs_int ZoomNumer; // Zooming factor numerator (setting)
	tjs_int ActualZoomDenom; // Zooming factor denominator (actual)
	tjs_int ActualZoomNumer; // Zooming factor numerator (actual)

	//-- menu related
	bool MenuBarVisible;


private:

public:		// ユーザー宣言
	//-- interface to plugin
private:
	void CallWindowDetach(bool close);
	void CallWindowAttach();
public:
	void RegisterWindowMessageReceiver(tTVPWMRRegMode mode,
		void * proc, const void *userdata);
private:
	bool InternalDeliverMessageToReceiver(tTVPWindowMessage &msg);
	bool DeliverMessageToReceiver(tTVPWindowMessage &msg)
		{ if(WindowMessageReceivers.GetCount())
			return InternalDeliverMessageToReceiver(msg);
			else return false; }
public:

	//-- DirectInput related
	void __fastcall CreateDirectInputDevice();
	void __fastcall FreeDirectInputDevice();

	//-- close action related
	bool Closing;
	bool ProgramClosing;
	bool CanCloseWork;
	void __fastcall Close(void);
	void __fastcall InvalidateClose();
	void __fastcall OnCloseQueryCalled(bool b);
	void __fastcall SendCloseMessage();

	static void __fastcall DeliverPopupHide();
private:
	void __fastcall FirePopupHide();
	bool __fastcall CanSendPopupHide() const
		{ return !Focusable && GetVisible() && GetStayOnTop(); }

	//-- form mode
public:
	bool __fastcall GetFormEnabled();

	//-- draw device
	void __fastcall ResetDrawDevice();

	//-- interface to layer
private:
	DWORD LastRecheckInputStateSent;

public:
	void __fastcall ZoomRectangle(
		tjs_int & left, tjs_int & top,
		tjs_int & right, tjs_int & bottom);
private:
	void __fastcall SetDrawDeviceDestRect();
	void __fastcall InternalSetPaintBoxSize();
public:
	void __fastcall SetPaintBoxSize(tjs_int w, tjs_int h);

	void __fastcall SetMouseCursorToWindow(TCursor cursor);

	void __fastcall SetDefaultMouseCursor();
	void __fastcall SetMouseCursor(tjs_int handle);

	void __fastcall GetCursorPos(tjs_int &x, tjs_int &y);
	void __fastcall SetCursorPos(tjs_int x, tjs_int y);

	void __fastcall SetHintText(const ttstr &text);

	void __fastcall SetLayerLeft(tjs_int left);
	tjs_int __fastcall GetLayerLeft() const { return LayerLeft; }
	void __fastcall SetLayerTop(tjs_int top);
	tjs_int __fastcall GetLayerTop() const { return LayerTop; }
	void __fastcall SetLayerPosition(tjs_int left, tjs_int top);

	void __fastcall AdjustNumerAndDenom(tjs_int &n, tjs_int &d);
	void __fastcall SetZoom(tjs_int numer, tjs_int denom, bool set_logical = true);
	void __fastcall SetZoomNumer(tjs_int n)
		{ SetZoom(n, ZoomDenom); }
	tjs_int __fastcall GetZoomNumer() const { return ZoomNumer; }
	void __fastcall SetZoomDenom(tjs_int d)
		{ SetZoom(ZoomNumer, d); }
	tjs_int __fastcall GetZoomDenom() const { return ZoomDenom; }

	void __fastcall SetImeMode(tTVPImeMode mode);
	void __fastcall SetDefaultImeMode(tTVPImeMode mode, bool reset);
	tTVPImeMode GetDefaultImeMode() const { return  DefaultImeMode; }
	void __fastcall ResetImeMode();

	void __fastcall SetAttentionPoint(tjs_int left, tjs_int top, TFont *font);
	void __fastcall DisableAttentionPoint();

	void __fastcall SetMaskRegion(HRGN rgn);
	void __fastcall RemoveMaskRegion();

	void __fastcall HideMouseCursor();
	void __fastcall SetMouseCursorState(tTVPMouseCursorState mcs);
	tTVPMouseCursorState __fastcall GetMouseCursorState() const { return MouseCursorState; }

	void __fastcall SetFocusable(bool b);
	bool __fastcall GetFocusable() const { return Focusable; }

private:
	void __fastcall RestoreMouseCursor();
	void __fastcall SetMouseCursorVisibleState(bool b);
	void __fastcall SetForceMouseCursorVisible(bool s);

public:

	//-- full screen management
	void __fastcall SetFullScreenMode(bool b);
	bool __fastcall GetFullScreenMode() const;

private:
	void __fastcall CleanupFullScreen(); // called at destruction
	void __fastcall DestroyMenuContainer();
	void __fastcall CreateMenuContainer();
	void __fastcall CheckMenuBarDrop();

public:
	//-- methods/properties
	void __fastcall BeginMove();
	void __fastcall BringToFront();
	void __fastcall UpdateWindow(tTVPUpdateType type = utNormal);
	void __fastcall ShowWindowAsModal();

	void __fastcall SetVisible(bool b);
	bool __fastcall GetVisible() const { return Visible; }

	void __fastcall SetInnerSunken(bool b);
	bool __fastcall GetInnerSunken() const;

	void __fastcall SetInnerWidth(tjs_int w);
	tjs_int __fastcall GetInnerWidth() const;
	void __fastcall SetInnerHeight(tjs_int h);
	tjs_int __fastcall GetInnerHeight() const;
	void __fastcall SetInnerSize(tjs_int w, tjs_int h);

	void __fastcall SetBorderStyle(tTVPBorderStyle st);
	tTVPBorderStyle __fastcall GetBorderStyle() const;

	void __fastcall SetMenuBarVisible(bool b);
	bool __fastcall GetMenuBarVisible() const;
	void __fastcall RevertMenuBarVisible();

	void __fastcall SetStayOnTop(bool b);
	bool __fastcall GetStayOnTop() const;

	void __fastcall SetShowScrollBars(bool b);
	bool __fastcall GetShowScrollBars() const;

	void __fastcall SetUseMouseKey(bool b);
	bool __fastcall GetUseMouseKey() const;

	void __fastcall SetTrapKey(bool b);
	bool __fastcall GetTrapKey() const;

protected:
	//-- paint box management
	void __fastcall CreatePaintBox(TWinControl *owner);

	// note that also window mouse events are to be sent to these functions
	void __fastcall PaintBoxClick(TObject *Sender);
	void __fastcall PaintBoxMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall PaintBoxPaint(TObject *Sender);
	void __fastcall PaintBoxDblClick(TObject *Sender);
	void __fastcall PaintBoxMouseMove(TObject *Sender, TShiftState Shift,
		  int X, int Y);
	void __fastcall PaintBoxMouseUp(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);

	//-- window coodinate translation
	void __fastcall TranslateWindowToPaintBox(int &x, int &y);

protected:
	//-- Windows message mapping
BEGIN_MESSAGE_MAP
	VCL_MESSAGE_HANDLER( WM_MOVE, TWMMove, WMMove);
	VCL_MESSAGE_HANDLER( WM_DROPFILES,  TMessage, WMDropFiles);
	VCL_MESSAGE_HANDLER( CM_MOUSEENTER, TMessage, CMMouseEnter)
	VCL_MESSAGE_HANDLER( CM_MOUSELEAVE, TMessage, CMMouseLeave)
	VCL_MESSAGE_HANDLER( WM_MOUSEACTIVATE, TWMMouseActivate, WMMouseActivate);
	VCL_MESSAGE_HANDLER( TVP_WM_SHOWVISIBLE, TMessage, WMShowVisible)
	VCL_MESSAGE_HANDLER( TVP_WM_SHOWTOP, TMessage, WMShowTop)
	VCL_MESSAGE_HANDLER( TVP_WM_RETRIEVEFOCUS, TMessage, WMRetrieveFocus);
	VCL_MESSAGE_HANDLER( TVP_WM_ACQUIREIMECONTROL, TMessage, WMAcquireImeControl);
	VCL_MESSAGE_HANDLER( WM_ENABLE, TMessage, WMEnable);
	VCL_MESSAGE_HANDLER( WM_SETFOCUS, TWMSetFocus, WMSetFocus);
	VCL_MESSAGE_HANDLER( WM_KILLFOCUS, TWMKillFocus, WMKillFocus);
	VCL_MESSAGE_HANDLER( WM_ENTERMENULOOP, TWMEnterMenuLoop, WMEnterMenuLoop);
	VCL_MESSAGE_HANDLER( WM_EXITMENULOOP, TWMExitMenuLoop, WMExitMenuLoop);
	VCL_MESSAGE_HANDLER( WM_KEYDOWN, TWMKeyDown, WMKeyDown);
	VCL_MESSAGE_HANDLER( WM_DEVICECHANGE, TMessage, WMDeviceChange);
	VCL_MESSAGE_HANDLER( WM_NCLBUTTONDOWN, TWMNCLButtonDown , WMNCLButtonDown)
	VCL_MESSAGE_HANDLER( WM_NCRBUTTONDOWN, TWMNCRButtonDown , WMNCRButtonDown)
END_MESSAGE_MAP(TForm)
	void __fastcall CMMouseEnter(TMessage &Msg);
	void __fastcall CMMouseLeave(TMessage &Msg);
	void __fastcall WMMouseActivate(TWMMouseActivate &msg);
	void __fastcall WMMove(TWMMove &Msg);
	void __fastcall WMDropFiles(TMessage &Msg);
	void __fastcall WMShowVisible(TMessage &Msg);
	void __fastcall WMShowTop(TMessage &Msg);
	void __fastcall WMRetrieveFocus(TMessage &Msg);
	void __fastcall WMAcquireImeControl(TMessage &Msg);
	void __fastcall WMEnable(TMessage &Msg);
	void __fastcall WMSetFocus(TWMSetFocus &Msg);
	void __fastcall WMKillFocus(TWMKillFocus &Msg);

	void __fastcall WMEnterMenuLoop(TWMEnterMenuLoop &Msg);
	void __fastcall WMExitMenuLoop(TWMExitMenuLoop &Msg);

	void __fastcall WMKeyDown(TWMKeyDown &Msg);

	void __fastcall WMDeviceChange(TMessage &Msg);

	void __fastcall WMNCLButtonDown(TWMNCLButtonDown &msg);
	void __fastcall WMNCRButtonDown(TWMNCRButtonDown &msg);

public:
	void __fastcall InvokeShowVisible();
	void __fastcall InvokeShowTop(bool activate = true);
	HDWP __fastcall ShowTop(HDWP hdwp);

protected:
	void __fastcall WndProc(TMessage &Message);

	DYNAMIC bool __fastcall IsShortCut(Messages::TWMKey &Message);

public:
	HWND __fastcall GetMenuOwnerWindowHandle();

	HWND __fastcall GetSurfaceWindowHandle();

	HWND __fastcall GetWindowHandle(tjs_int &ofsx, tjs_int &ofsy);
	HWND __fastcall GetWindowHandleForPlugin();

	void __fastcall TickBeat(); // called every 50ms intervally

private:
	void __fastcall GenerateMouseEvent(bool fl, bool fr, bool fu, bool fd);

};
//---------------------------------------------------------------------------
extern PACKAGE TTVPWindowForm *TVPWindowForm;
//---------------------------------------------------------------------------


#endif
