//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// "Window" TJS Class implementation (VCL side)
//---------------------------------------------------------------------------
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
#include "MenuContainerFormUnit.h"
#include "MainFormUnit.h"
#include "SysInitIntf.h"
#include "PluginImpl.h"
#include "Random.h"
#include "SystemImpl.h"
#include "DInputMgn.h"
#include "tvpinputdefs.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TTVPWindowForm *TVPWindowForm;



//---------------------------------------------------------------------------
// VCL-based constants to TVP-based constants conversion (and vice versa)
//---------------------------------------------------------------------------
tTVPMouseButton TVP_TMouseButton_To_tTVPMouseButton(TMouseButton button)
{
	if(button == Controls::mbLeft) return ::mbLeft;
	if(button == Controls::mbRight) return ::mbRight;
	if(button == Controls::mbMiddle) return ::mbMiddle;

	return (tTVPMouseButton)(-1);
}
//---------------------------------------------------------------------------
tjs_uint32 TVP_TShiftState_To_uint32(TShiftState state)
{
	tjs_uint32 f = 0;
	if(state.Contains(ssShift)) f += TVP_SS_SHIFT;
	if(state.Contains(ssAlt)) f += TVP_SS_ALT;
	if(state.Contains(ssCtrl)) f += TVP_SS_CTRL;
	if(state.Contains(ssLeft)) f += TVP_SS_LEFT;
	if(state.Contains(ssRight)) f += TVP_SS_RIGHT;
	if(state.Contains(ssMiddle)) f += TVP_SS_MIDDLE;
	if(state.Contains(ssDouble)) f += TVP_SS_DOUBLE;

	return f;
}
//---------------------------------------------------------------------------
TShiftState TVP_TShiftState_From_uint32(tjs_uint32 state)
{
	TShiftState ret;
	if(state & TVP_SS_SHIFT) ret << ssShift;
	if(state & TVP_SS_ALT) ret << ssAlt;
	if(state & TVP_SS_CTRL) ret << ssCtrl;
	if(state & TVP_SS_LEFT) ret << ssLeft;
	if(state & TVP_SS_RIGHT) ret << ssRight;
	if(state & TVP_SS_MIDDLE) ret << ssMiddle;
	if(state & TVP_SS_DOUBLE) ret << ssDouble;

	return ret;
}
//---------------------------------------------------------------------------
tjs_uint32 TVPGetCurrentShiftKeyState()
{
	tjs_uint32 f = 0;

	if(TVPGetAsyncKeyState(VK_SHIFT)) f += TVP_SS_SHIFT;
	if(TVPGetAsyncKeyState(VK_MENU)) f += TVP_SS_ALT;
	if(TVPGetAsyncKeyState(VK_CONTROL)) f += TVP_SS_CTRL;
	if(TVPGetAsyncKeyState(VK_LBUTTON)) f += TVP_SS_LEFT;
	if(TVPGetAsyncKeyState(VK_RBUTTON)) f += TVP_SS_RIGHT;
	if(TVPGetAsyncKeyState(VK_MBUTTON)) f += TVP_SS_MIDDLE;

	return f;
}
//---------------------------------------------------------------------------
static TImeMode TVP_tTVPImeMode_To_TImeMode(tTVPImeMode mode)
{
	switch(mode)
	{
	// Note: imDisable will cause undesired behaviour on ATOK/Win8.
	// This(*) is a quickhack making kirikiri not to use imDisable,
	// which may cause calling troublesome Win32NLSEnableIME.
	// This bug is live AFAIK Windows8 32/64, no SP.
	case ::imDisable   : return Controls::imClose   ; // (*)
	case ::imClose     : return Controls::imClose   ;
	case ::imOpen      : return Controls::imOpen    ;
	case ::imDontCare  : return Controls::imDontCare;
	case ::imSAlpha    : return Controls::imSAlpha  ;
	case ::imAlpha     : return Controls::imAlpha   ;
	case ::imHira      : return Controls::imHira    ;
	case ::imSKata     : return Controls::imSKata   ;
	case ::imKata      : return Controls::imKata    ;
	case ::imChinese   : return Controls::imChinese ;
	case ::imSHanguel  : return Controls::imSHanguel;
	case ::imHanguel   : return Controls::imHanguel ;
	}
	return Controls::imClose; // (*)
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TWinControlEx: proxy class for accessing TWinControl's protected methods
//---------------------------------------------------------------------------
class TWinControlEx : public TWinControl
{
public:
	void __fastcall ResetIme(void);
	void __fastcall SetIme(void);
	bool __fastcall
	SetImeCompositionWindow(Graphics::TFont* Font, int XPos, int YPos);
	void __fastcall _SetImeMode(TImeMode mode);
};
void __fastcall TWinControlEx::ResetIme(void)
{
	TWinControl::ResetIme();
}
void __fastcall TWinControlEx::SetIme(void)
{
	TWinControl::SetIme();
	if(SysLocale.FarEast && ImeMode == Controls::imDontCare)
	Win32NLSEnableIME(Handle, TRUE);
}
bool __fastcall
	TWinControlEx::SetImeCompositionWindow(Graphics::TFont* Font, int XPos, int YPos)
{
	return TWinControl::SetImeCompositionWindow(Font, XPos, YPos);
}
void __fastcall TWinControlEx::_SetImeMode(TImeMode mode)
{
	ImeMode = mode;
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
TTVPWindowForm * TVPFullScreenedWindow = NULL;
//---------------------------------------------------------------------------




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
		::ShowWindow(TVPFullScreenedWindow->Handle, SW_RESTORE); // restore the window

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
			(*i)->Visible = false;
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



//---------------------------------------------------------------------------
// TTVPWindowForm
//---------------------------------------------------------------------------
__fastcall TTVPWindowForm::TTVPWindowForm(TComponent* Owner, tTJSNI_Window *ni)
	: TForm(Owner)
{
	TVPInitWindowOptions();

	// set hot keys
	ShowControllerMenuItem->ShortCut = TVPMainForm->ShowControllerMenuItem->ShortCut;
	ShowScriptEditorMenuItem->ShortCut = TVPMainForm->ShowScriptEditorMenuItem->ShortCut;
	ShowWatchMenuItem->ShortCut = TVPMainForm->ShowWatchMenuItem->ShortCut;
	ShowConsoleMenuItem->ShortCut = TVPMainForm->ShowConsoleMenuItem->ShortCut;
	ShowAboutMenuItem->ShortCut = TVPMainForm->ShowAboutMenuItem->ShortCut;
	CopyImportantLogMenuItem->ShortCut = TVPMainForm->CopyImportantLogMenuItem->ShortCut;

	// retrieve hot keys
	TTVPMainForm::SetHotKey(UpdateRectDebugMenuItem, TJS_W("-hkupdaterect"));
	TTVPMainForm::SetHotKey(DumpLayerStructorMenuItem, TJS_W("-hkdumplayer"));

	// initialize members
	TJSNativeInstance = ni;

	NextSetWindowHandleToDrawDevice = true;
	LastSentDrawDeviceDestRect.clear();

	InMode = false;
	ResetStayOnTopStateTick = 0;
	Focusable = true;
	Closing = false;
	ProgramClosing = false;
	ModalResult = 0;
	InnerWidthSave = ClientWidth;
	InnerHeightSave = ClientHeight;
	MenuContainer = NULL;
	MenuBarVisible = true;

	AttentionFont = new TFont();

	PaintBox = NULL;

	ZoomDenom = ActualZoomDenom = 1;
	ZoomNumer = ActualZoomNumer = 1;

	DefaultImeMode = ::imClose;
	LastSetImeMode = Controls::imDontCare;

	CreatePaintBox(ScrollBox);

	LastSetImeMode = Controls::imClose;
	::PostMessage(Handle, TVP_WM_ACQUIREIMECONTROL, 0, 0);


	UseMouseKey = false;
	InMenuLoop = false;
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

	// redirect window mouse events to paintbox's mouse event
	this->OnClick = PaintBoxClick;
	this->OnMouseDown = PaintBoxMouseDown;
	this->OnDblClick = PaintBoxDblClick;
	this->OnMouseMove = PaintBoxMouseMove;
	this->OnMouseUp = PaintBoxMouseUp;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::FormDestroy(TObject *Sender)
{
	CallWindowDetach(true);

	TJSNativeInstance = NULL;
	DestroyMenuContainer();
	CleanupFullScreen();
	TVPRemoveModalWindow(this);

	FreeDirectInputDevice();

	delete AttentionFont, AttentionFont = NULL;

	tjs_int count = WindowMessageReceivers.GetCount();
	for(tjs_int i = 0 ; i < count; i++)
	{
		tTVPMessageReceiverRecord * item = WindowMessageReceivers[i];
		if(!item) continue;
		delete item;
		WindowMessageReceivers.Remove(i);
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::FormCloseQuery(TObject *Sender,
	  bool &CanClose)
{
	// closing actions are 3 patterns;
	// 1. closing action by the user
	// 2. "close" method
	// 3. object invalidation


	if(TVPGetBreathing())
	{
		CanClose = false;
		return;
	}

	// the default event handler will invalidate this object when an onCloseQuery
	// event reaches the handler.
	if(TJSNativeInstance &&
		(ModalResult == 0 ||
		ModalResult == mrCancel/* mrCancel=when close button is pushed in modal window */  ))
	{
		iTJSDispatch2 * obj = TJSNativeInstance->GetOwnerNoAddRef();
		if(obj)
		{
			tTJSVariant arg[1] = {true};
			static ttstr eventname(TJS_W("onCloseQuery"));

			if(!ProgramClosing)
			{
				// close action does not happen immediately
//				TVPPostEvent(obj, obj, eventname, 0, TVP_EPT_POST, 1, arg);
				if(TJSNativeInstance)
				{
					TVPPostInputEvent(
						new tTVPOnCloseInputEvent(TJSNativeInstance));
				}

				CanClose = false;
				Closing = true; // waiting closing...
				TVPMainForm->NotifyCloseClicked();
			}
			else
			{
				CanCloseWork = true;
				TVPPostEvent(obj, obj, eventname, 0, TVP_EPT_IMMEDIATE, 1, arg);
					// this event happens immediately
					// and does not return until done
				CanClose = CanCloseWork; // CanCloseWork is set by the event handler
			}
		}
		else
		{
			CanClose = true;
		}
	}
	else
	{
		CanClose = true;
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::FormClose(TObject *Sender,
	  TCloseAction &Action)
{
	if(ModalResult == 0) Action = caNone; else Action = caHide;

	if(ProgramClosing)
	{
		if(TJSNativeInstance)
		{
			if(TJSNativeInstance->IsMainWindow())
			{
				// this is the main window
			}
			else
			{
				// not the main window
				Action = caFree;
			}
			if(TVPFullScreenedWindow != this)
			{
				// if this is not a fullscreened window
				Visible = false;
			}
			iTJSDispatch2 * obj = TJSNativeInstance->GetOwnerNoAddRef();
			TJSNativeInstance->NotifyWindowClose();
			obj->Invalidate(0, NULL, NULL, obj);
			TJSNativeInstance = NULL;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::Close(void)
{
	// closing action by "close" method
	if(Closing) return; // already waiting closing...

	ProgramClosing = true;
	try
	{
		TForm::Close();
	}
	catch(...)
	{
		ProgramClosing = false;
		throw;
	}
	ProgramClosing = false;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::InvalidateClose(void)
{
	// closing action by object invalidation;
	// this will not cause any user confirmation of closing the window.
	TJSNativeInstance = NULL;
	Visible = false;
	delete this;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::OnCloseQueryCalled(bool b)
{
	// closing is allowed by onCloseQuery event handler
	if(!ProgramClosing)
	{
		// closing action by the user
		if(b)
		{
			if(InMode)
				ModalResult = 1; // when modal
			else
				Visible = false;  // just hide

			Closing = false;
			if(TJSNativeInstance)
			{
				if(TJSNativeInstance->IsMainWindow())
				{
					// this is the main window
					iTJSDispatch2 * obj = TJSNativeInstance->GetOwnerNoAddRef();
					obj->Invalidate(0, NULL, NULL, obj);
//					TJSNativeInstance->NotifyWindowClose();
					TJSNativeInstance = NULL;
				}
			}
			else
			{
				delete this;
			}
		}
		else
		{
			Closing = false;
		}
	}
	else
	{
		// closing action by the program
		CanCloseWork = b;
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SendCloseMessage()
{
	::PostMessage(Handle, WM_CLOSE, 0, 0);
}
//---------------------------------------------------------------------------
bool TTVPWindowForm::GetWindowActive()
{
	return GetForegroundWindow() == Handle;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::DeliverPopupHide()
{
	// deliver onPopupHide event to unfocusable windows

	tjs_int count = TVPGetWindowCount();
	for(tjs_int i = count - 1; i >= 0; i--)
	{
		tTJSNI_Window * win = TVPGetWindowListAt(i);
		if(win)
		{
			TTVPWindowForm * form = win->GetForm();
			if(form)
			{
				form->FirePopupHide();
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::FirePopupHide()
{
	// fire "onPopupHide" event
	if(!CanSendPopupHide()) return;
	if(!GetVisible()) return;

	TVPPostInputEvent(
		new tTVPOnPopupHideInputEvent(TJSNativeInstance));
}
//---------------------------------------------------------------------------
void TTVPWindowForm::CallWindowDetach(bool close)
{
	if(TJSNativeInstance) TJSNativeInstance->GetDrawDevice()->SetTargetWindow(NULL, false);

	tTVPWindowMessage msg;
	msg.Msg = TVP_WM_DETACH;
	msg.LParam = 0;
	msg.WParam = close ? 1 : 0;
	msg.Result = 0;

	DeliverMessageToReceiver(msg);
}
//---------------------------------------------------------------------------
void TTVPWindowForm::CallWindowAttach()
{
	NextSetWindowHandleToDrawDevice = true;
	LastSentDrawDeviceDestRect.clear();

	tTVPWindowMessage msg;
	msg.Msg = TVP_WM_ATTACH;
	msg.LParam = reinterpret_cast<int>(GetWindowHandleForPlugin());
	msg.WParam = 0;
	msg.Result = 0;

	DeliverMessageToReceiver(msg);
}
//---------------------------------------------------------------------------
void TTVPWindowForm::RegisterWindowMessageReceiver(tTVPWMRRegMode mode,
		void * proc, const void *userdata)
{
	if(mode == wrmRegister)
	{
		// register
		tjs_int count = WindowMessageReceivers.GetCount();
		tjs_int i;
		for(i = 0 ; i < count; i++)
		{
			tTVPMessageReceiverRecord *item = WindowMessageReceivers[i];
			if(!item) continue;
			if((void*)item->Proc == proc) break; // have already registered
		}
		if(i == count)
		{
			// not have registered
			tTVPMessageReceiverRecord *item = new tTVPMessageReceiverRecord();
			item->Proc = (tTVPWindowMessageReceiver)proc;
			item->UserData = userdata;
			WindowMessageReceivers.Add(item);
		}
	}
	else if(mode == wrmUnregister)
	{
		// unregister
		tjs_int count = WindowMessageReceivers.GetCount();
		for(tjs_int i = 0 ; i < count; i++)
		{
			tTVPMessageReceiverRecord *item = WindowMessageReceivers[i];
			if(!item) continue;
			if((void*)item->Proc == proc)
			{
				// found
				WindowMessageReceivers.Remove(i);
				delete item;
			}
		}
		WindowMessageReceivers.Compact();
	}
}
//---------------------------------------------------------------------------
bool TTVPWindowForm::InternalDeliverMessageToReceiver(tTVPWindowMessage &msg)
{
	if(!TJSNativeInstance)
		return false;
	if(TVPPluginUnloadedAtSystemExit)
		return false;

	tObjectListSafeLockHolder<tTVPMessageReceiverRecord> holder(WindowMessageReceivers);
	tjs_int count = WindowMessageReceivers.GetSafeLockedObjectCount();

	bool block = false;
	for(tjs_int i = 0; i < count; i++)
	{
		tTVPMessageReceiverRecord *item = WindowMessageReceivers.GetSafeLockedObjectAt(i);
		if(!item) continue;
		bool b = item->Deliver(&msg);
		block = block || b;
	}

	return block;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::CreateDirectInputDevice()
{
	if(!DIWheelDevice)
		DIWheelDevice = new tTVPWheelDirectInputDevice(Handle);
	if(!DIPadDevice)
		DIPadDevice = new tTVPPadDirectInputDevice(Handle);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::FreeDirectInputDevice()
{
	if(DIWheelDevice)
	{
		delete DIWheelDevice;
		DIWheelDevice = NULL;
	}

	if(DIPadDevice)
	{
		delete DIPadDevice;
		DIPadDevice = NULL;
	}
}
//---------------------------------------------------------------------------
bool __fastcall TTVPWindowForm::GetFormEnabled()
{
	if(!Enabled) return false;
	bool en = IsWindowEnabled(Handle);
	return en;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::ResetDrawDevice()
{
	NextSetWindowHandleToDrawDevice = true;
	LastSentDrawDeviceDestRect.clear();

	if(PaintBox) PaintBox->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::ShowControllerMenuItemClick(
	  TObject *Sender)
{
	TVPMainForm->ShowController();
}
//---------------------------------------------------------------------------

void __fastcall TTVPWindowForm::ShowScriptEditorMenuItemClick(
	  TObject *Sender)
{
	TVPMainForm->ShowScriptEditorButtonClick(Sender);
}
//---------------------------------------------------------------------------

void __fastcall TTVPWindowForm::ShowWatchMenuItemClick(TObject *Sender)
{
	TVPMainForm->ShowWatchButtonClick(Sender);
}
//---------------------------------------------------------------------------

void __fastcall TTVPWindowForm::ShowConsoleMenuItemClick(TObject *Sender)
{
	TVPMainForm->ShowConsoleButtonClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::ShowAboutMenuItemClick(TObject *Sender)
{
	TVPMainForm->ShowAboutMenuItemClick(Sender);
}
//---------------------------------------------------------------------------

void __fastcall TTVPWindowForm::CopyImportantLogMenuItemClick(
	  TObject *Sender)
{
	TVPMainForm->CopyImportantLogMenuItemClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::UpdateRectDebugMenuItemClick(
	  TObject *Sender)
{
	UpdateRectDebugMenuItem->Checked = ! UpdateRectDebugMenuItem->Checked;
	if(TJSNativeInstance) TJSNativeInstance->SetShowUpdateRect(UpdateRectDebugMenuItem->Checked);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::DumpLayerStructorMenuItemClick(TObject *Sender)
{
	if(TJSNativeInstance) TJSNativeInstance->DumpPrimaryLayerStructure();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::FormShow(TObject *Sender)
{
	DragAcceptFiles(Handle, true);
	::PostMessage(Handle, TVP_WM_ACQUIREIMECONTROL, 0, 0);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::ScrollBoxMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
	RestoreMouseCursor();
	CheckMenuBarDrop();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::FormMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
	RestoreMouseCursor();
	CheckMenuBarDrop();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::ZoomRectangle(
	tjs_int & left, tjs_int & top,
	tjs_int & right, tjs_int & bottom)
{
	left =   MulDiv(left  ,  ActualZoomNumer, ActualZoomDenom);
	top =    MulDiv(top   ,  ActualZoomNumer, ActualZoomDenom);
	right =  MulDiv(right ,  ActualZoomNumer, ActualZoomDenom);
	bottom = MulDiv(bottom,  ActualZoomNumer, ActualZoomDenom);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetDrawDeviceDestRect()
{
	tjs_int x_ofs = 0;
	tjs_int y_ofs = 0;

	tTVPRect destrect(PaintBox->Left + x_ofs, PaintBox->Top + y_ofs,
					PaintBox->Width + PaintBox->Left + x_ofs, PaintBox->Height + PaintBox->Top + y_ofs);

	if(LastSentDrawDeviceDestRect != destrect)
	{
		if(TJSNativeInstance)
			TJSNativeInstance->GetDrawDevice()->SetDestRectangle(destrect);
		LastSentDrawDeviceDestRect = destrect;
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::InternalSetPaintBoxSize()
{
	tjs_int l = MulDiv(LayerLeft,   ActualZoomNumer, ActualZoomDenom);
	tjs_int t = MulDiv(LayerTop,    ActualZoomNumer, ActualZoomDenom);
	tjs_int w = MulDiv(LayerWidth,  ActualZoomNumer, ActualZoomDenom);
	tjs_int h = MulDiv(LayerHeight, ActualZoomNumer, ActualZoomDenom);
	PaintBox->SetBounds(l, t, w, h);
	SetDrawDeviceDestRect();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetPaintBoxSize(tjs_int w, tjs_int h)
{
	ScrollBox->HorzScrollBar->Position = 0;
	ScrollBox->VertScrollBar->Position = 0;
	LayerWidth  = w;
	LayerHeight = h;
	InternalSetPaintBoxSize();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetMouseCursorToWindow(TCursor cursor)
{
	TCursor bk = Screen->Cursor;
	Screen->Cursor = cursor;
	if(PaintBox) PaintBox->Cursor = cursor;
	Screen->Cursor = bk;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetDefaultMouseCursor()
{
	if(!PaintBox || PaintBox->Cursor != crDefault)
	{
		if(MouseCursorState == mcsVisible && !ForceMouseCursorVisible)
		{
			SetMouseCursorToWindow(crDefault);
		}
	}
	CurrentMouseCursor = crDefault;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetMouseCursor(tjs_int handle)
{
	if(!PaintBox || PaintBox->Cursor != handle)
	{
		if(MouseCursorState == mcsVisible && !ForceMouseCursorVisible)
		{
			SetMouseCursorToWindow((TCursor)handle);
		}
	}
	CurrentMouseCursor = (TCursor)handle;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::GetCursorPos(tjs_int &x, tjs_int &y)
{
	// get mouse cursor position in paintbox coordinates
	if(PaintBox)
	{
		TPoint origin;
		origin = PaintBox->ClientToScreen(TPoint(0, 0));

		POINT mp = {0, 0};
		::GetCursorPos(&mp);

		x = mp.x - origin.x;
		y = mp.y - origin.y;
	}
	else
	{
		x = 0;
		y = 0;
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetCursorPos(tjs_int x, tjs_int y)
{
	// set mouse cursor position in paintbox coordinates
	if(PaintBox)
	{
		TPoint pt;
		pt = PaintBox->ClientToScreen(TPoint(x, y));
		::SetCursorPos(pt.x, pt.y);

		LastMouseScreenX = LastMouseScreenY = -1; // force to display mouse cursor
		RestoreMouseCursor();
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetHintText(const ttstr &text)
{
	// set window hint
	if(!PaintBox) return;
	Application->CancelHint();
	PaintBox->ShowHint = false;
	PaintBox->Hint = text.AsAnsiString();
	PaintBox->ShowHint = true;

	// ensure hint
	POINT p;
	::GetCursorPos(&p);
	TPoint pt = PaintBox->ScreenToClient(TPoint(p));
	TWMMouse msg;
	msg.Msg = WM_MOUSEMOVE;
	msg.XPos = pt.x;
	msg.YPos = pt.y;
	Application->HintMouseMessage(PaintBox, *(TMessage*)&msg);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetLayerLeft(tjs_int left)
{
	if(LayerLeft != left)
	{
		LayerLeft = left;
		if(PaintBox) InternalSetPaintBoxSize();
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetLayerTop(tjs_int top)
{
	if(LayerTop != top)
	{
		LayerTop = top;
		if(PaintBox) InternalSetPaintBoxSize();
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetLayerPosition(tjs_int left, tjs_int top)
{
	if(LayerLeft != left || LayerTop != top)
	{
		LayerLeft = left;
		LayerTop = top;
		if(PaintBox) InternalSetPaintBoxSize();
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::AdjustNumerAndDenom(tjs_int &n, tjs_int &d)
{
	tjs_int a = n;
	tjs_int b = d;
	while(b)
	{
		tjs_int t = b;
		b = a % b;
		a = t;
	}
	n = n / a;
	d = d / a;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetZoom(tjs_int numer, tjs_int denom, bool set_logical)
{
	// set layer zooming factor;
	// the zooming factor is passed in numerator/denoiminator style.
	// we must find GCM to optimize numer/denium via Euclidean algorithm.
	AdjustNumerAndDenom(numer, denom);
	if(set_logical)
	{
		ZoomNumer = numer;
		ZoomDenom = denom;
	}
	if(!GetFullScreenMode())
	{
		// in fullscreen mode, zooming factor is controlled by the system
		ActualZoomDenom = denom;
		ActualZoomNumer = numer;
	}
	InternalSetPaintBoxSize();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetImeMode(tTVPImeMode mode)
{
	if(PaintBox)
	{
		LastSetImeMode = TVP_tTVPImeMode_To_TImeMode(mode);
		AcquireImeControl();
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetDefaultImeMode(tTVPImeMode mode, bool reset)
{
	DefaultImeMode = mode;
	if(reset) ResetImeMode();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::ResetImeMode()
{
	if(PaintBox)
	{
		SetImeMode(DefaultImeMode);
	}
}
//---------------------------------------------------------------------------
void TTVPWindowForm::UnacquireImeControl()
{
	if(PaintBox)
	{
		if(TVPControlImeState)
		{
			((TWinControlEx*)(PaintBox->Parent))->ResetIme();
			ResetIme();
		}
	}
}
//---------------------------------------------------------------------------
void TTVPWindowForm::AcquireImeControl()
{

	if(PaintBox && Focused())
	{
		// find key trapper window ...
		TTVPWindowForm * trapper = GetKeyTrapperWindow();

		// force to access protected some methods.
		// much nasty way ...
		if(TVPControlImeState)
		{
			ResetIme();
			TImeMode newmode = trapper->LastSetImeMode;
			ImeMode = newmode;
			if(SysLocale.FarEast && ImeMode == Controls::imDontCare)
				Win32NLSEnableIME(Handle, TRUE);
			SetIme();

			((TWinControlEx*)(PaintBox->Parent))->ResetIme();
			((TWinControlEx*)(PaintBox->Parent))->_SetImeMode(newmode);
			((TWinControlEx*)(PaintBox->Parent))->SetIme();
		}

		if(trapper->AttentionPointEnabled)
		{
			SetCaretPos(trapper->AttentionPoint.x, trapper->AttentionPoint.y);

			if(trapper == this)
			{
				((TWinControlEx*)(PaintBox->Parent))->SetImeCompositionWindow(
					AttentionFont, AttentionPoint.x, AttentionPoint.y);
				SetImeCompositionWindow(
					AttentionFont, AttentionPoint.x, AttentionPoint.y);
			}
			else
			{
				// disable IMM composition window
				COMPOSITIONFORM form;
				memset(&form, 0, sizeof(form));
				form.dwStyle = CFS_DEFAULT;
				HIMC imc;
				imc = ImmGetContext(Handle);
				ImmSetCompositionWindow(imc, &form);
				ImmReleaseContext(Handle, imc);
				imc = ImmGetContext(PaintBox->Parent->Handle);
				ImmSetCompositionWindow(imc, &form);
				ImmReleaseContext(PaintBox->Parent->Handle, imc);
			}
		}
	}


}
//---------------------------------------------------------------------------
TTVPWindowForm * TTVPWindowForm::GetKeyTrapperWindow()
{
	// find most recent "trapKeys = true" window and return it.
	// returnts "this" window if there is no trapping window.

	tjs_int count = TVPGetWindowCount();
	for(tjs_int i = count - 1; i >= 0; i--)
	{
		tTJSNI_Window * win = TVPGetWindowListAt(i);
		if(win)
		{
			TTVPWindowForm * form = win->GetForm();
			if(form && form != this)
			{
				if(form->TrapKeys && form->GetVisible())
				{
					// found
					return form;
				}
			}
		}
	}

	return this;
}
//---------------------------------------------------------------------------
bool TTVPWindowForm::FindKeyTrapper(LRESULT &result, UINT msg,
	WPARAM wparam, LPARAM lparam)
{
	// find most recent "trapKeys = true" window.
	tjs_int count = TVPGetWindowCount();
	for(tjs_int i = count - 1; i >= 0; i--)
	{
		tTJSNI_Window * win = TVPGetWindowListAt(i);
		if(win)
		{
			TTVPWindowForm * form = win->GetForm();
			if(form)
			{
				if(form->TrapKeys && form->GetVisible())
				{
					// found
					return form->ProcessTrappedKeyMessage(result, msg, wparam, lparam);
				}
			}
		}
	}

	// not found
	return false;
}
//---------------------------------------------------------------------------
bool TTVPWindowForm::ProcessTrappedKeyMessage(LRESULT &result, UINT msg, WPARAM wparam,
		LPARAM lparam)
{
	// perform key message
	if(msg == WM_KEYDOWN)
	{
		CanReceiveTrappedKeys = true;
			// to prevent receiving a key, which is pushed when the window is just created
	}

	if(CanReceiveTrappedKeys)
	{
		InReceivingTrappedKeys = true;
		result = Perform(msg, wparam, lparam);
		InReceivingTrappedKeys = false;
	}

	if(msg == WM_KEYUP)
	{
		CanReceiveTrappedKeys = true;
	}

	return true;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetAttentionPoint(tjs_int left, tjs_int top,
	TFont *font)
{
	if(ScrollBox->BorderStyle == Forms::bsSingle)
	{
		// sunken style
		left += 2;
		top += 2;
	}

	// add scrollbox offset
	if(ScrollBox)
	{
		left += ScrollBox->Left;
		top += ScrollBox->Top;
	}

	// set attention point information
	AttentionPoint.x = left;
	AttentionPoint.y = top;
	AttentionPointEnabled = true;
	if(font)
	{
		AttentionFont->Assign(font);
	}
	else
	{
		TFont * default_font = new TFont();
		AttentionFont->Assign(default_font);
		delete default_font;
	}
	AcquireImeControl();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::DisableAttentionPoint()
{
	AttentionPointEnabled = false;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetMaskRegion(HRGN rgn)
{
	SetWindowRgn(Handle, rgn, (BOOL)Visible);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::RemoveMaskRegion()
{
	SetWindowRgn(Handle, NULL, (BOOL)Visible);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::HideMouseCursor()
{
	// hide mouse cursor temporarily
    SetMouseCursorState(mcsTempHidden);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetMouseCursorState(tTVPMouseCursorState mcs)
{
	if(MouseCursorState == mcsVisible && mcs != mcsVisible)
	{
		// formerly visible and newly invisible
		if(!ForceMouseCursorVisible) SetMouseCursorVisibleState(false);
	}
	else if(MouseCursorState != mcsVisible && mcs == mcsVisible)
	{
		// formerly invisible and newly visible
		if(!ForceMouseCursorVisible) SetMouseCursorVisibleState(true);
	}

	if(MouseCursorState != mcs && mcs == mcsTempHidden)
	{
		POINT pt;
		::GetCursorPos(&pt);
		LastMouseScreenX = pt.x;
		LastMouseScreenY = pt.y;
	}

	MouseCursorState = mcs;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::RestoreMouseCursor()
{
	// restore mouse cursor hidden by HideMouseCursor
	if(MouseCursorState == mcsTempHidden)
	{
		POINT pt;
		::GetCursorPos(&pt);
		if(LastMouseScreenX != pt.x || LastMouseScreenY != pt.y)
		{
			SetMouseCursorState(mcsVisible);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetMouseCursorVisibleState(bool b)
{
	// set mouse cursor visible state
	// this does not look MouseCursorState
	if(b)
		SetMouseCursorToWindow(CurrentMouseCursor);
	else
		SetMouseCursorToWindow(crNone);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetForceMouseCursorVisible(bool s)
{
	if(ForceMouseCursorVisible != s)
	{
		if(s)
		{
			// force visible mode
			// the cursor is to be fixed in crDefault
			SetMouseCursorToWindow(crDefault);
		}
		else
		{
			// normal mode
			// restore normal cursor
			SetMouseCursorVisibleState(MouseCursorState == mcsVisible);
		}
		ForceMouseCursorVisible = s;
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetFocusable(bool b)
{
	// set focusable state to 'b'.
	// note that currently focused window does not automatically unfocus by
	// setting false to this flag.
	Focusable = b;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetFullScreenMode(bool b)
{
	// note that we should not change the display mode when showing overlay
	// videos.

	CallWindowDetach(false); // notify to plugin
	try
	{
		if(TJSNativeInstance) TJSNativeInstance->DetachVideoOverlay();
		FreeDirectInputDevice();
			// due to re-create window (but current implementation may not re-create the window)

		if(b)
		{
			if(TVPFullScreenedWindow == this) return;
			if(TVPFullScreenedWindow) TVPFullScreenedWindow->SetFullScreenMode(false);

			// save position and size
			OrgLeft = Left;
			OrgTop = Top;
			OrgWidth = Width;
			OrgHeight = Height;

			// determin desired full screen size
			tjs_int desired_fs_w = InnerWidthSave;
			tjs_int desired_fs_h = InnerHeightSave;

			// set ScrollBox' border invisible
			OrgInnerSunken = GetInnerSunken();
			ScrollBox->BorderStyle = Forms::bsNone;

			// change PaintBox's Owner to directly the Form
//			CreatePaintBox(this);

			// set BorderStyle
			OrgStyle = GetWindowLong(Handle, GWL_STYLE);
			OrgExStyle = GetWindowLong(Handle, GWL_EXSTYLE);
			OrgScrollBoxBorderStyle = ScrollBox->BorderStyle;
			SetWindowLong(Handle, GWL_STYLE,
				WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPED);
			ScrollBox->BorderStyle = Forms::bsNone;


			// try to switch to fullscreen
			try
			{
				TVPSwitchToFullScreen(Handle, desired_fs_w, desired_fs_h);
			}
			catch(...)
			{
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
			Left = 0;
			Top = 0;
			Width = fs_w;
			Height = fs_h;

			// reset ScrollBox size
			ScrollBox->Align = alNone;
			ScrollBox->Left = (fs_w - sb_w)/2;
			ScrollBox->Top = (fs_h - sb_h)/2;
			ScrollBox->Width = sb_w;
			ScrollBox->Height = sb_h;

			// float menu bar
			CreateMenuContainer();

			// re-adjust video rect
			if(TJSNativeInstance) TJSNativeInstance->ReadjustVideoRect();

			// activate self
			Application->BringToFront();
			BringToFront();
			SetFocus();

			// activate self (again) // Added by W.Dee 2003/11/02
			Sleep(200);
			SetWindowPos(Handle, HWND_TOP, 0, 0, 0, 0,
				SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOREPOSITION|SWP_NOSIZE|SWP_SHOWWINDOW);
		}
		else
		{
			if(TVPFullScreenedWindow != this) return;

			// dock menu bar
			DestroyMenuContainer();

			// revert from fullscreen
			TVPRevertFromFullScreen(Handle);
			TVPFullScreenedWindow = NULL;

			// revert zooming factor
			ActualZoomDenom = ZoomDenom;
			ActualZoomNumer = ZoomNumer;

			SetZoom(ZoomNumer, ZoomDenom);  // reset zoom factor

			// set BorderStyle
			SetWindowLong(Handle, GWL_STYLE, OrgStyle);
			SetWindowLong(Handle, GWL_EXSTYLE, OrgExStyle);
			ScrollBox->BorderStyle = OrgScrollBoxBorderStyle;

			// change PaintBox's Owner to the ScrollBox
//			CreatePaintBox(ScrollBox);

			// set ScrollBox visible
			SetInnerSunken(OrgInnerSunken);
			ScrollBox->Align = alClient;
			SetWindowPos(ScrollBox->Handle, HWND_BOTTOM, 0, 0, 0, 0,
				SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);

			// revert the position and size
			SetBounds(OrgLeft, OrgTop, OrgWidth, OrgHeight);

			// re-adjust video rect
			if(TJSNativeInstance) TJSNativeInstance->ReadjustVideoRect();

			// restore
			WindowState = wsNormal;
		}
	}
	catch(...)
	{
		CallWindowAttach();
		throw;
	}
	CallWindowAttach();

	SetMouseCursor(CurrentMouseCursor);
}
//---------------------------------------------------------------------------
bool __fastcall TTVPWindowForm::GetFullScreenMode() const
{
	return TVPFullScreenedWindow == this;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::CleanupFullScreen()
{
	// called at destruction
	if(TVPFullScreenedWindow != this) return
	DestroyMenuContainer();
	TVPRevertFromFullScreen(Handle);
	TVPFullScreenedWindow = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::DestroyMenuContainer()
{
	if(MenuContainer)
	{
		// delete MeunContainer
		delete MenuContainer;
		MenuContainer = NULL;
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::CreateMenuContainer()
{
	if(!MenuContainer)
	{
		// create MenuContainer
		MenuContainer = new TTVPMenuContainerForm(this);
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::CheckMenuBarDrop()
{
	if(MenuContainer && MenuBarVisible && GetWindowActive()) 
	{
		POINT pos = {0, Height};
		::GetCursorPos(&pos);
		if(pos.y <= 0) MenuContainer->StartDropWatch();
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::BeginMove()
{
	// begin window moving
	ReleaseCapture(); // release mouse events which captured by VCL
	Perform(WM_SYSCOMMAND, SC_MOVE+2, 0); // start moving
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::BringToFront()
{
	Application->BringToFront();
	TForm::BringToFront();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::UpdateWindow(tTVPUpdateType)
{
	if(TJSNativeInstance)
	{
		tTVPRect r;
		r.left = 0;
		r.top = 0;
		r.right = LayerWidth;
		r.bottom = LayerHeight;
		TJSNativeInstance->NotifyWindowExposureToLayer(r);

		TVPDeliverWindowUpdateEvents();
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::ShowWindowAsModal()
{
	// TODO: what's modalwindowlist ?
	ModalResult = 0;
	InMode = true;
	TVPAddModalWindow(this); // add to modal window list
	try
	{
		TForm::ShowModal();
	}
	catch(...)
	{
		TVPRemoveModalWindow(this);
		InMode = false;
		throw;
	}
	TVPRemoveModalWindow(this);
	InMode = false;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetVisible(bool b)
{
	if(Focusable)
	{
		Visible = b;
	}
	else
	{
		if(!Visible)
		{
			// just show window, not activate
			SetWindowPos(Handle, GetStayOnTop()?HWND_TOPMOST:HWND_TOP, 0, 0, 0, 0,
				SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
			Visible = true;
		}
		else
		{
			Visible = false;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetInnerSunken(bool b)
{
	// window inner appearance style
	ScrollBox->BorderStyle = b ? Forms::bsSingle : Forms::bsNone;
}
//---------------------------------------------------------------------------
bool __fastcall TTVPWindowForm::GetInnerSunken() const
{
	return ScrollBox->BorderStyle == Forms::bsSingle;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetInnerWidth(tjs_int w)
{
	tjs_int bwidth = GetInnerSunken()?4:0;
	ClientWidth = w + bwidth;
	InnerWidthSave = w;
	if(ClientWidth != w + bwidth) ClientWidth = w + bwidth;
		// set ClientWidth twice ( first substitution may not set the size
		// properly in some occasions )
}
//---------------------------------------------------------------------------
tjs_int __fastcall TTVPWindowForm::GetInnerWidth() const
{
	tjs_int bwidth = GetInnerSunken()?4:0;
	return ((TTVPWindowForm*)this)->ClientWidth - bwidth;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetInnerHeight(tjs_int h)
{
	tjs_int bwidth = GetInnerSunken()?4:0;
	ClientHeight = h + bwidth;
	InnerHeightSave = h;
	if(ClientHeight != h + bwidth) ClientHeight = h + bwidth;
}
//---------------------------------------------------------------------------
tjs_int __fastcall TTVPWindowForm::GetInnerHeight() const
{
	tjs_int bwidth = GetInnerSunken()?4:0;
	return ((TTVPWindowForm*)this)->ClientHeight - bwidth;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetInnerSize(tjs_int w, tjs_int h)
{
	tjs_int bwidth = GetInnerSunken()?4:0;
	ClientWidth = w + bwidth;
	ClientHeight = h + bwidth;
	InnerWidthSave = w;
	InnerHeightSave = h;
	if(ClientWidth != w + bwidth ||
		ClientHeight != h + bwidth)
	{
		ClientWidth = w + bwidth;
		ClientHeight = h + bwidth;
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetBorderStyle(tTVPBorderStyle st)
{
	CallWindowDetach(false);
	FreeDirectInputDevice(); // due to re-create window
	if(TJSNativeInstance) TJSNativeInstance->DetachVideoOverlay();

	if(st == ::bsSingle)
		BorderIcons = BorderIcons>>biMaximize; // remove maximize button
	else
		BorderIcons = BorderIcons<<biMaximize;
	switch(st)
	{
	case ::bsDialog:		BorderStyle = Forms::bsDialog;		break;
	case ::bsSingle:		BorderStyle = Forms::bsSingle;		break;
	case ::bsNone:			BorderStyle = Forms::bsNone;		break;
	case ::bsSizeable:		BorderStyle = Forms::bsSizeable;	break;
	case ::bsToolWindow:	BorderStyle = Forms::bsToolWindow;	break;
	case ::bsSizeToolWin:	BorderStyle = Forms::bsSizeToolWin;	break;
	}

	if(TJSNativeInstance) TJSNativeInstance->ReadjustVideoRect();
	CallWindowAttach();
}
//---------------------------------------------------------------------------
tTVPBorderStyle __fastcall TTVPWindowForm::GetBorderStyle() const
{
	switch(BorderStyle)
	{
	case Forms::bsDialog:		return ::bsDialog;
	case Forms::bsSingle:		return ::bsSingle;
	case Forms::bsNone:			return ::bsNone;
	case Forms::bsSizeable:		return ::bsSizeable;
	case Forms::bsToolWindow:	return ::bsToolWindow;
	case Forms::bsSizeToolWin:	return ::bsSizeToolWin;
	}

	return ::bsDialog;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetMenuBarVisible(bool b)
{
	MenuBarVisible = b;
	Menu = b?MainMenu:NULL;
}
//---------------------------------------------------------------------------
bool __fastcall TTVPWindowForm::GetMenuBarVisible() const
{
	return MenuBarVisible;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::RevertMenuBarVisible()
{
	Menu = MenuBarVisible?MainMenu:NULL;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetStayOnTop(bool b)
{
	CallWindowDetach(false);
	FreeDirectInputDevice(); // due to re-create window
	FormStyle = b ? fsStayOnTop:fsNormal;
	CallWindowAttach();
}
//---------------------------------------------------------------------------
bool __fastcall TTVPWindowForm::GetStayOnTop() const
{
	return FormStyle == fsStayOnTop;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetShowScrollBars(bool b)
{
	ScrollBox->HorzScrollBar->Visible = b;
	ScrollBox->VertScrollBar->Visible = b;
	ScrollBox->Color = clBlack;
	ScrollBox->Brush->Style = b?bsSolid:bsClear;
	this->Color = clBlack;
	this->Brush->Style = bsSolid;
}
//---------------------------------------------------------------------------
bool __fastcall TTVPWindowForm::GetShowScrollBars() const
{
	return
		ScrollBox->HorzScrollBar->Visible ||
		ScrollBox->VertScrollBar->Visible;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetUseMouseKey(bool b)
{
	UseMouseKey = b;
	if(b)
	{
		MouseLeftButtonEmulatedPushed = false;
		MouseRightButtonEmulatedPushed = false;
		LastMouseKeyTick = GetTickCount();
	}
	else
	{
		if(MouseLeftButtonEmulatedPushed)
		{
			MouseLeftButtonEmulatedPushed = false;
			PaintBoxMouseUp(PaintBox, Controls::mbLeft,
				TShiftState(), LastMouseMovedPos.x, LastMouseMovedPos.y);
		}
		if(MouseRightButtonEmulatedPushed)
		{
			MouseRightButtonEmulatedPushed = false;
			PaintBoxMouseUp(PaintBox, Controls::mbRight,
				TShiftState(), LastMouseMovedPos.x, LastMouseMovedPos.y);
		}

	}
}
//---------------------------------------------------------------------------
bool __fastcall TTVPWindowForm::GetUseMouseKey() const
{
	return UseMouseKey;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::SetTrapKey(bool b)
{
	TrapKeys = b;
	if(TrapKeys)
	{
		// reset CanReceiveTrappedKeys and InReceivingTrappedKeys
		CanReceiveTrappedKeys = false;
		InReceivingTrappedKeys = false;
		// note that SetTrapKey can be called while the key trapping is
		// processing.
	}
}
//---------------------------------------------------------------------------
bool __fastcall TTVPWindowForm::GetTrapKey() const
{
	return TrapKeys;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::CreatePaintBox(TWinControl *owner)
{
	ReleaseCapture(); // release mouse events which captured by VCL
	if(TJSNativeInstance)
	{
		// send release capture input event
		TVPPostInputEvent(
			new tTVPOnReleaseCaptureInputEvent(TJSNativeInstance));
	}


	if(PaintBox) delete PaintBox;

	PaintBox = new TPaintBox(owner);
	PaintBox->Parent = owner;
	PaintBox->Left = LayerLeft;
	PaintBox->Top = LayerTop;
	if(TJSNativeInstance) TJSNativeInstance->NotifySrcResize(); // to reset size

	PaintBox->Visible = true;

	PaintBox->OnClick = PaintBoxClick;
	PaintBox->OnMouseDown = PaintBoxMouseDown;
	PaintBox->OnPaint = PaintBoxPaint;
	PaintBox->OnDblClick = PaintBoxDblClick;
	PaintBox->OnMouseMove = PaintBoxMouseMove;
	PaintBox->OnMouseUp = PaintBoxMouseUp;

	SetStretchBltMode(PaintBox->Canvas->Handle, HALFTONE);
	SetBrushOrgEx(PaintBox->Canvas->Handle, 0, 0, NULL);
	SetMapMode(PaintBox->Canvas->Handle, MM_TEXT);

	AcquireImeControl();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::PaintBoxClick(TObject *Sender)
{
	// fire click event
	if(TJSNativeInstance)
	{
		TVPPostInputEvent(
			new tTVPOnClickInputEvent(TJSNativeInstance,
				LastMouseDownX, LastMouseDownY));
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::PaintBoxDblClick(TObject *Sender)
{
	// fire double click event
	if(TJSNativeInstance)
	{
		TVPPostInputEvent(
			new tTVPOnDoubleClickInputEvent(TJSNativeInstance,
				LastMouseDownX, LastMouseDownY));
	}
}
//---------------------------------------------------------------------------

void __fastcall TTVPWindowForm::PaintBoxMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
	if(Sender == this)
		TranslateWindowToPaintBox(X, Y);

	if(TJSNativeInstance)
	{
		tjs_uint32 shift = TVP_TShiftState_To_uint32(Shift);
		TVPPostInputEvent(
			new tTVPOnMouseMoveInputEvent(TJSNativeInstance,
				X, Y, shift), TVP_EPT_DISCARDABLE);
	}

	CheckMenuBarDrop();
	RestoreMouseCursor();

	int pos = (Y << 16) + X;
	TVPPushEnvironNoise(&pos, sizeof(pos));

	LastMouseMovedPos.x = X;
	LastMouseMovedPos.y = Y;
}
//---------------------------------------------------------------------------

void __fastcall TTVPWindowForm::PaintBoxMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if(!CanSendPopupHide()) DeliverPopupHide();

	if(Sender == this)
		TranslateWindowToPaintBox(X, Y);

	if(Sender == PaintBox)
		::SetCaptureControl(PaintBox);

	LastMouseDownX = X;
	LastMouseDownY = Y;

	if(TJSNativeInstance)
	{
		tjs_uint32 shift = TVP_TShiftState_To_uint32(Shift);
		tTVPMouseButton button = TVP_TMouseButton_To_tTVPMouseButton(Button);
		TVPPostInputEvent(
			new tTVPOnMouseDownInputEvent(TJSNativeInstance,
				X, Y, button, shift));
	}
}
//---------------------------------------------------------------------------

void __fastcall TTVPWindowForm::PaintBoxMouseUp(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if(Sender == this)
		TranslateWindowToPaintBox(X, Y);

	::SetCaptureControl(NULL);

	if(TJSNativeInstance)
	{
		tjs_uint32 shift = TVP_TShiftState_To_uint32(Shift);
		tTVPMouseButton button = TVP_TMouseButton_To_tTVPMouseButton(Button);
		TVPPostInputEvent(
			new tTVPOnMouseUpInputEvent(TJSNativeInstance,
				X, Y, button, shift));
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::TranslateWindowToPaintBox(int &x, int &y)
{
	if(!PaintBox) return;
	TPoint pt;
	pt = ClientToScreen(TPoint(x, y));
	pt = PaintBox->ScreenToClient(pt);
	x = pt.x;
	y = pt.y;
}
//---------------------------------------------------------------------------
void TTVPWindowForm::InternalKeyDown(WORD key, tjs_uint32 shift)
{
	DWORD tick = GetTickCount();
	TVPPushEnvironNoise(&tick, sizeof(tick));
	TVPPushEnvironNoise(&key, sizeof(key));
	TVPPushEnvironNoise(&shift, sizeof(shift));

	if(TJSNativeInstance)
	{
		if(UseMouseKey && PaintBox)
		{
			if(key == VK_RETURN || key == VK_SPACE || key == VK_ESCAPE ||
				key == VK_PAD1 || key == VK_PAD2)
			{
				POINT p;
				::GetCursorPos(&p);
				TPoint tp;
				tp.x = p.x; tp.y = p.y;
				tp = ScrollBox->ScreenToClient(tp);

				if(tp.x >= 0 && tp.y >= 0 &&
					tp.x < ScrollBox->Width && tp.y < ScrollBox->Height)
				{
					if(key == VK_RETURN || key == VK_SPACE || key == VK_PAD1)
					{
						MouseLeftButtonEmulatedPushed = true;
						PaintBoxMouseDown(PaintBox, Controls::mbLeft,
							TShiftState(), tp.x, tp.y);
					}

					if(key == VK_ESCAPE || key == VK_PAD2)
					{
						MouseRightButtonEmulatedPushed = true;
						PaintBoxMouseDown(PaintBox, Controls::mbRight,
							TShiftState(), tp.x, tp.y);
					}
				}
				return;
			}

			switch(key)
			{
			case VK_LEFT:
			case VK_PADLEFT:
				if(MouseKeyXAccel == 0 && MouseKeyYAccel == 0)
				{
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
		TVPPostInputEvent(new tTVPOnKeyDownInputEvent(TJSNativeInstance,
			key, shift));
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::FormKeyDown(TObject *Sender, WORD &Key,
	  TShiftState Shift)
{
	if(TJSNativeInstance)
		InternalKeyDown(Key, IsRepeatMessage?
			(TVP_TShiftState_To_uint32(Shift)|TVP_SS_REPEAT):
			(TVP_TShiftState_To_uint32(Shift)));
}
//---------------------------------------------------------------------------

void __fastcall TTVPWindowForm::FormKeyPress(TObject *Sender, char &Key)
{
	if(TJSNativeInstance && Key)
	{
		if(UseMouseKey && (Key == 0x1b || Key == 13 || Key == 32)) return;
		if(PendingKeyCodes != "")
		{
			// pending keycode
			PendingKeyCodes += Key;
			wchar_t dest;
			int res = TJS_mbtowc(&dest, PendingKeyCodes.c_str(), PendingKeyCodes.Length());
			if(res > 0)
			{
				// convertion succeeded
				TVPPostInputEvent(new tTVPOnKeyPressInputEvent(TJSNativeInstance,
					dest));
				PendingKeyCodes = "";
			}
		}
		else
		{
			char key[2];
			key[0] = Key;
			key[1] = 0;
			wchar_t dest;
			int res = TJS_mbtowc(&dest, key, 1);
			if(res > 0)
			{
				// convertion succeeded
				TVPPostInputEvent(new tTVPOnKeyPressInputEvent(TJSNativeInstance,
					dest));
			}
			else
			{
				PendingKeyCodes = key;
			}
		}
	}
}
//---------------------------------------------------------------------------
void TTVPWindowForm::InternalKeyUp(WORD key, tjs_uint32 shift)
{
	DWORD tick = GetTickCount();
	TVPPushEnvironNoise(&tick, sizeof(tick));
	TVPPushEnvironNoise(&key, sizeof(key));
	TVPPushEnvironNoise(&shift, sizeof(shift));

	if(TJSNativeInstance)
	{
		if(UseMouseKey && PaintBox)
		{
			if(key == VK_RETURN || key == VK_SPACE || key == VK_ESCAPE ||
				key == VK_PAD1 || key == VK_PAD2)
			{
				POINT p;
				::GetCursorPos(&p);
				TPoint tp;
				tp.x = p.x; tp.y = p.y;
				tp = ScrollBox->ScreenToClient(tp);
				if(tp.x >= 0 && tp.y >= 0 &&
					tp.x < ScrollBox->Width && tp.y < ScrollBox->Height)
				{
					if(key == VK_RETURN || key == VK_SPACE || key == VK_PAD1)
					{
						PaintBoxClick(PaintBox);
						MouseLeftButtonEmulatedPushed = false;
						PaintBoxMouseUp(PaintBox, Controls::mbLeft,
							TShiftState(), tp.x, tp.y);
					}

					if(key == VK_ESCAPE || key == VK_PAD2)
					{
						MouseRightButtonEmulatedPushed = false;
						PaintBoxMouseUp(PaintBox, Controls::mbRight,
							TShiftState(), tp.x, tp.y);
					}
				}
				return;
			}
		}

		TVPPostInputEvent(new tTVPOnKeyUpInputEvent(TJSNativeInstance,
			key, shift));
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::FormKeyUp(TObject *Sender, WORD &Key,
	  TShiftState Shift)
{
	tjs_uint32 shift = TVP_TShiftState_To_uint32(Shift);
	InternalKeyUp(Key, shift);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::FormMouseWheel(TObject *Sender,
	  TShiftState Shift, int WheelDelta, TPoint &MousePos, bool &Handled)
{
	if(TVPWheelDetectionType == wdtWindowMessage)
	{
		// wheel
		if(TJSNativeInstance)
		{
			tjs_uint32 shift = TVP_TShiftState_To_uint32(Shift);
			TVPPostInputEvent(new tTVPOnMouseWheelInputEvent(TJSNativeInstance,
				shift, WheelDelta, MousePos.x, MousePos.y));
			Handled = true;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::TabMenuItemClick(TObject *Sender)
{
	// tab pressed
	WORD key = VK_TAB;
	FormKeyDown(this, key, TShiftState());
	char keychar = '\t';
	FormKeyPress(this, keychar);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::ShitTabMenuItemClick(TObject *Sender)
{
	WORD key = VK_TAB;
	FormKeyDown(this, key, TShiftState()<<ssShift);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::AltEnterMenuItemClick(TObject *Sender)
{
	WORD key = VK_RETURN;
	FormKeyDown(this, key, TShiftState()<<ssAlt);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::FormResize(TObject *Sender)
{
	// on resize
	if(TJSNativeInstance)
	{
		// here specifies TVP_EPT_REMOVE_POST, to remove redundant onResize
		// events.
		TVPPostInputEvent(
			new tTVPOnResizeInputEvent(TJSNativeInstance), TVP_EPT_REMOVE_POST);
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::PaintBoxPaint(TObject *Sender)
{
	// a painting event
	if(NextSetWindowHandleToDrawDevice)
	{
		bool ismain = false;
		if(TJSNativeInstance) ismain = TJSNativeInstance->IsMainWindow();
		if(TJSNativeInstance) TJSNativeInstance->GetDrawDevice()->SetTargetWindow(PaintBox->Parent->Handle, ismain);
		NextSetWindowHandleToDrawDevice = false;
	}

	SetDrawDeviceDestRect();

	if(TJSNativeInstance)
	{
		tTVPRect r;
		TRect tr = PaintBox->Canvas->ClipRect;

		r.left   = tr.left;
		r.top    = tr.top;
		r.right  = tr.right;
		r.bottom = tr.bottom;

		TJSNativeInstance->NotifyWindowExposureToLayer(r);

//		TVPDeliverWindowUpdateEvents();
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::CMMouseEnter(TMessage &Msg)
{
	// mouse entered in client area
	DWORD tick = GetTickCount();
	TVPPushEnvironNoise(&tick, sizeof(tick));


	if(TJSNativeInstance)
	{
		TVPPostInputEvent(
			new tTVPOnMouseEnterInputEvent(TJSNativeInstance));
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::CMMouseLeave(TMessage &Msg)
{
	// mouse leaved from client area
	DWORD tick = GetTickCount();
	TVPPushEnvironNoise(&tick, sizeof(tick));

	if(TJSNativeInstance)
	{
		TVPPostInputEvent(
			new tTVPOnMouseOutOfWindowInputEvent(TJSNativeInstance));
		TVPPostInputEvent(
			new tTVPOnMouseLeaveInputEvent(TJSNativeInstance));
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::WMMouseActivate(TWMMouseActivate &msg)
{
	if(!Focusable)
	{
		// override default action (which activates the window)
		if(msg.HitTestCode == HTCLIENT)
			msg.Result = MA_NOACTIVATE;
		else
			msg.Result = MA_NOACTIVATEANDEAT;
	}
	else
	{
		// do default action
    	TForm::Dispatch((void*)&msg);
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::WMMove(TWMMove &Msg)
{
	if(TJSNativeInstance)
	{
		TJSNativeInstance->WindowMoved();
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::FormActivate(TObject *Sender)
{
	if(TVPFullScreenedWindow == this)
		TVPShowModalAtAppActivate();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::FormDeactivate(TObject *Sender)
{
	if(TJSNativeInstance)
	{
		TVPPostInputEvent(
			new tTVPOnReleaseCaptureInputEvent(TJSNativeInstance));
	}

}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::WMDropFiles(TMessage &Msg)
{
	if(!TJSNativeInstance) return;

	HDROP hd = (HDROP)Msg.WParam;

	char filename[ MAX_PATH ];
	tjs_int filecount=
		DragQueryFile(hd, 0xFFFFFFFF, NULL, MAX_PATH);

	iTJSDispatch2 * array = TJSCreateArrayObject();

	try
	{
		tjs_int count = 0;
		for(tjs_int i = filecount-1; i>=0; i--)
		{
			DragQueryFile(hd, i, filename, MAX_PATH);

			WIN32_FIND_DATA fd;
			HANDLE h;

			// existence checking
			if((h = FindFirstFile(filename, &fd)) != INVALID_HANDLE_VALUE)
			{
				FindClose(h);

				tTJSVariant val = TVPNormalizeStorageName(ttstr(filename));

				// push into array
				array->PropSetByNum(TJS_MEMBERENSURE|TJS_IGNOREPROP,
					count++, &val, array);
			}

		}
		DragFinish(hd);

		tTJSVariant arg(array, array);
		TVPPostInputEvent(
			new tTVPOnFileDropInputEvent(TJSNativeInstance, arg));
	}
	catch(...)
	{
		array->Release();
		throw;
	}

	array->Release();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::WMShowVisible(TMessage &Msg)
{
	Visible = true;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::WMShowTop(TMessage &Msg)
{
	if(Visible)
	{
		if(Msg.WParam) SetZOrder(true);
		SetWindowPos(Handle, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOREPOSITION|SWP_NOSIZE|SWP_SHOWWINDOW);
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::WMRetrieveFocus(TMessage &Msg)
{
	Application->BringToFront();
	SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::WMAcquireImeControl(TMessage &Msg)
{
	AcquireImeControl();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::WMEnable(TMessage &Msg)
{
	// enabled status has changed
	if(TJSNativeInstance)
	{
		TVPPostInputEvent(
			new tTVPOnReleaseCaptureInputEvent(TJSNativeInstance));
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::WMSetFocus(TWMSetFocus &Msg)
{
	::PostMessage(Handle, TVP_WM_ACQUIREIMECONTROL, 0, 0);

	if(PaintBox)
		CreateCaret(PaintBox->Parent->Handle, NULL, 1, 1);

	if(DIPadDevice && TJSNativeInstance && PaintBox)
		DIPadDevice->WindowActivated();

	if(TJSNativeInstance) TJSNativeInstance->FireOnActivate(true);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::WMKillFocus(TWMKillFocus &Msg)
{
	DestroyCaret();
	UnacquireImeControl();

	if(DIPadDevice && TJSNativeInstance && PaintBox)
		DIPadDevice->WindowDeactivated();

	if(TJSNativeInstance) TJSNativeInstance->FireOnActivate(false);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::WMEnterMenuLoop(TWMEnterMenuLoop &Msg)
{
	InMenuLoop = true;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::WMExitMenuLoop(TWMExitMenuLoop &Msg)
{
	InMenuLoop = false;

	// bug workaround for directdraw which does not follow update event
	if(TJSNativeInstance && PaintBox)
		TJSNativeInstance->GetDrawDevice()->RequestInvalidation(
			tTVPRect(0, 0, PaintBox->Width, PaintBox->Height));

}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::WMKeyDown(TWMKeyDown &Msg)
{
	IsRepeatMessage = (Msg.KeyData & 0x40000000);
		// retrieve whether the key is repeating,
		// while VCL onKeyDown cannot handle the state.
	TForm::Dispatch(&Msg);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::WMDeviceChange(TMessage &Msg)
{
	if(Msg.WParam == DBT_DEVNODES_CHANGED)
	{
		// reload DInput device

		ReloadDevice = true; // to reload device
		ReloadDeviceTick = GetTickCount() + 4000; // reload at 4secs later
	}
	TForm::Dispatch(&Msg);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::WMNCLButtonDown(TWMNCLButtonDown &msg)
{
	if(!CanSendPopupHide())
	{
		DeliverPopupHide();
	}

	TForm::Dispatch((void*)&msg);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::WMNCRButtonDown(TWMNCRButtonDown &msg)
{
	if(!CanSendPopupHide())
	{
		DeliverPopupHide();
	}

	TForm::Dispatch((void*)&msg);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::InvokeShowVisible()
{
	// this posts window message which invokes WMShowVisible
	::PostMessage(Handle, TVP_WM_SHOWVISIBLE, 0, 0);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::InvokeShowTop(bool activate)
{
	// this posts window message which invokes WMShowTop
	::PostMessage(Handle, TVP_WM_SHOWTOP, activate ? 1:0, 0);
}
//---------------------------------------------------------------------------
HDWP __fastcall TTVPWindowForm::ShowTop(HDWP hdwp)
{
	if(Visible)
	{
		hdwp = DeferWindowPos(hdwp, Handle, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOREPOSITION|
			SWP_NOSIZE|WM_SHOWWINDOW);
	}
	return hdwp;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::WndProc(TMessage &Message)
{
	// WndProc ( window procedure ) override.
	if(DeliverMessageToReceiver(
		*(tTVPWindowMessage *)(&Message))) return;

	if(Message.Msg == WM_SYSCOMMAND)
	{
		long subcom = Message.WParam & 0xfff0;
		bool ismain = false;
		if(TJSNativeInstance) ismain = TJSNativeInstance->IsMainWindow();
		if(ismain)
		{
			if(subcom == SC_MINIMIZE && !IsIconic(Application->Handle))
			{
				Application->Minimize();
				return;
			}

			if(subcom == SC_RESTORE && IsIconic(Application->Handle))
			{
				Application->Restore();
				return;
			}
		}

		if(subcom == SC_KEYMENU && MenuContainer && MenuBarVisible)
		{
			MenuContainer->DropByKey();
		}
	}
	else 	if(!InReceivingTrappedKeys // to prevent infinite recursive call
		&& Message.Msg >= WM_KEYFIRST && Message.Msg <= WM_KEYLAST)
	{
		// hide popups when alt key is pressed
		if(Message.Msg == WM_SYSKEYDOWN && !CanSendPopupHide())
			DeliverPopupHide();

		// drain message to key trapping window
		LRESULT res;
		if(FindKeyTrapper(res, Message.Msg, Message.WParam, Message.LParam))
		{
			Message.Result = res;
			return;
		}
	}

	TForm::WndProc(Message);
}
//---------------------------------------------------------------------------
bool __fastcall TTVPWindowForm::IsShortCut(Messages::TWMKey &Message)
{
	if(MenuContainer)
	{
		// Menu bar is separated from main window, so we must drain the
		// shortcut query.
		if(MainMenu->IsShortCut(Message)) return true;
	}
	return TForm::IsShortCut(Message);
}
//---------------------------------------------------------------------------
HWND __fastcall TTVPWindowForm::GetMenuOwnerWindowHandle()
{
	if(MenuContainer)
	{
		// this is a quick hack which let MenuContainer be visible,
		// because the menu owner window must be visible to receive menu command.
		MenuContainer->PrepareToReceiveMenuCommand();
		return MenuContainer->Handle;
	}
	return Handle;
}
//---------------------------------------------------------------------------
HWND __fastcall TTVPWindowForm::GetSurfaceWindowHandle()
{
	if(ScrollBox)
	{
		SetWindowPos(ScrollBox->Handle, HWND_BOTTOM, 0, 0, 0, 0,
				SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
	}
	if(PaintBox) return PaintBox->Parent->Handle;
	return NULL;
}
//---------------------------------------------------------------------------
HWND __fastcall TTVPWindowForm::GetWindowHandle(tjs_int &ofsx, tjs_int &ofsy)
{
	if(ScrollBox)
	{
		SetWindowPos(ScrollBox->Handle, HWND_BOTTOM, 0, 0, 0, 0,
				SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
	}
	ofsx = ofsy = GetInnerSunken()?2:0;
	ofsx += ScrollBox->Align == alClient ? 0 : ScrollBox->Left;
	ofsy += ScrollBox->Align == alClient ? 0 : ScrollBox->Top;
	return Handle;
}
//---------------------------------------------------------------------------
HWND __fastcall TTVPWindowForm::GetWindowHandleForPlugin()
{
	if(ScrollBox)
	{
		SetWindowPos(ScrollBox->Handle, HWND_BOTTOM, 0, 0, 0, 0,
				SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
	}
	return Handle;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWindowForm::TickBeat()
{
	// called every 50ms intervally
	DWORD tickcount = GetTickCount();
	bool focused = Focused();
	bool showingmenu = InMenuLoop;
	if(MenuContainer)
	{
		focused = focused || MenuContainer->Focused();
		showingmenu = showingmenu || MenuContainer->GetShowingMenu();
	}

	// set mouse cursor state
	SetForceMouseCursorVisible(showingmenu);

	// watch menu bar drop
	if(focused && !showingmenu) CheckMenuBarDrop();

	// mouse key
	if(UseMouseKey && PaintBox && !showingmenu && focused)
	{
		GenerateMouseEvent(false, false, false, false);
	}

	// device reload
	if(ReloadDevice && (int)(tickcount - ReloadDeviceTick) > 0)
	{
		ReloadDevice = false;
		FreeDirectInputDevice();
		CreateDirectInputDevice();
	}

	// wheel rotation detection
	tjs_uint32 shift = TVPGetCurrentShiftKeyState();
	if(TVPWheelDetectionType == wdtDirectInput)
	{
		CreateDirectInputDevice();
		if(!showingmenu && focused && TJSNativeInstance && DIWheelDevice && PaintBox)
		{
			tjs_int delta = DIWheelDevice->GetWheelDelta();
			if(delta)
			{
				TPoint origin;
				origin = PaintBox->ClientToScreen(TPoint(0, 0));

				POINT mp = {0, 0};
				::GetCursorPos(&mp);

				tjs_int x = mp.x - origin.x;
				tjs_int y = mp.y - origin.y;

				TVPPostInputEvent(new tTVPOnMouseWheelInputEvent(TJSNativeInstance,
						shift, delta, x, y));
			}
		}
	}

	// pad detection
	if(TVPJoyPadDetectionType == jdtDirectInput)
	{
		CreateDirectInputDevice();
		if(DIPadDevice && TJSNativeInstance && PaintBox)
		{
			if(!showingmenu && focused )
				DIPadDevice->UpdateWithCurrentState();
			else
				DIPadDevice->UpdateWithSuspendedState();

			const std::vector<WORD> & uppedkeys  = DIPadDevice->GetUppedKeys();
			const std::vector<WORD> & downedkeys = DIPadDevice->GetDownedKeys();
			const std::vector<WORD> & repeatkeys = DIPadDevice->GetRepeatKeys();
			std::vector<WORD>::const_iterator i;

			// for upped pad buttons
			for(i = uppedkeys.begin(); i != uppedkeys.end(); i++)
			{
				InternalKeyUp(*i, shift);
			}
			// for downed pad buttons
			for(i = downedkeys.begin(); i != downedkeys.end(); i++)
			{
				InternalKeyDown(*i, shift);
			}
			// for repeated pad buttons
			for(i = repeatkeys.begin(); i != repeatkeys.end(); i++)
			{
				InternalKeyDown(*i, shift|TVP_SS_REPEAT);
			}
		}
	}


	// check RecheckInputState
	if(tickcount - LastRecheckInputStateSent > 1000)
	{
		LastRecheckInputStateSent = tickcount;
		if(TJSNativeInstance) TJSNativeInstance->RecheckInputState();
	}
}
//---------------------------------------------------------------------------
#define TVP_MOUSE_MAX_ACCEL 30
#define TVP_MOUSE_SHIFT_ACCEL 40
void __fastcall TTVPWindowForm::GenerateMouseEvent(bool fl, bool fr, bool fu, bool fd)
{
	if(!PaintBox) return;

	if(!fl && !fr && !fu && !fd)
	{
		if(GetTickCount() - 45 < LastMouseKeyTick) return;
	}

	bool shift = GetAsyncKeyState(VK_SHIFT) & 0x8000;
	bool left = fl || GetAsyncKeyState(VK_LEFT) & 0x8000 ||
		TVPGetJoyPadAsyncState(VK_PADLEFT, true);
	bool right = fr || GetAsyncKeyState(VK_RIGHT) & 0x8000 ||
		TVPGetJoyPadAsyncState(VK_PADRIGHT, true);
	bool up = fu || GetAsyncKeyState(VK_UP) & 0x8000 ||
		TVPGetJoyPadAsyncState(VK_PADUP, true);
	bool down = fd || GetAsyncKeyState(VK_DOWN) & 0x8000 ||
		TVPGetJoyPadAsyncState(VK_PADDOWN, true);

	DWORD flags = 0;
	if(left || right || up || down) flags |= MOUSEEVENTF_MOVE;

	if(!right && !left && !up && !down)
	{
		LastMouseMoved = false;
		MouseKeyXAccel = MouseKeyYAccel = 0;
	}

	if(!shift)
	{
		if(!right && left && MouseKeyXAccel > 0) MouseKeyXAccel = -0;
		if(!left && right && MouseKeyXAccel < 0) MouseKeyXAccel = 0;
		if(!down && up && MouseKeyYAccel > 0) MouseKeyYAccel = -0;
		if(!up && down && MouseKeyYAccel < 0) MouseKeyYAccel = 0;
	}
	else
	{
		if(left) MouseKeyXAccel = -TVP_MOUSE_SHIFT_ACCEL;
		if(right) MouseKeyXAccel = TVP_MOUSE_SHIFT_ACCEL;
		if(up) MouseKeyYAccel = -TVP_MOUSE_SHIFT_ACCEL;
		if(down) MouseKeyYAccel = TVP_MOUSE_SHIFT_ACCEL;
	}

	if(right || left || up || down)
	{
		if(left) if(MouseKeyXAccel > -TVP_MOUSE_MAX_ACCEL)
			MouseKeyXAccel = MouseKeyXAccel?MouseKeyXAccel - 2:-2;
		if(right) if(MouseKeyXAccel < TVP_MOUSE_MAX_ACCEL)
			MouseKeyXAccel = MouseKeyXAccel?MouseKeyXAccel + 2:+2;
		if(!left && !right)
		{
			if(MouseKeyXAccel > 0) MouseKeyXAccel--;
			else if(MouseKeyXAccel < 0) MouseKeyXAccel++;
		}

		if(up) if(MouseKeyYAccel > -TVP_MOUSE_MAX_ACCEL)
			MouseKeyYAccel = MouseKeyYAccel?MouseKeyYAccel - 2:-2;
		if(down) if(MouseKeyYAccel < TVP_MOUSE_MAX_ACCEL)
			MouseKeyYAccel = MouseKeyYAccel?MouseKeyYAccel + 2:+2;
		if(!up && !down)
		{
			if(MouseKeyYAccel > 0) MouseKeyYAccel--;
			else if(MouseKeyYAccel < 0) MouseKeyYAccel++;
		}

	}

	if(flags)
	{
		POINT pt;
		if(::GetCursorPos(&pt))
		{
			::SetCursorPos(
					pt.x + (MouseKeyXAccel>>1),
					pt.y + (MouseKeyYAccel>>1));
		}

		LastMouseMoved = true;
	}


	LastMouseKeyTick = GetTickCount();
}
//---------------------------------------------------------------------------











