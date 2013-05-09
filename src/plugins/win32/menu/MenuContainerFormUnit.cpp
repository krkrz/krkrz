//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// floating menu bar implementation ( used by WindowForm when fullscreened )
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "WindowFormUnit.h"
#include "MenuContainerFormUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TTVPMenuContainerForm *TVPMenuContainerForm;
//---------------------------------------------------------------------------
__fastcall TTVPMenuContainerForm::TTVPMenuContainerForm(TComponent* Owner)
	: TForm(Owner)
{
	OwnerForm = (TTVPWindowForm*)Owner;

	// attach OwnerFowm's main menu to this window
	OwnerForm->Menu = NULL;
	Menu = OwnerForm->MainMenu;
	WaitingShowing = false;
	ShowingMenu = false;
}
//---------------------------------------------------------------------------
void __fastcall TTVPMenuContainerForm::FormDestroy(TObject *Sender)
{
	// detach menu
	Menu = NULL;
	OwnerForm->RevertMenuBarVisible();
}
//---------------------------------------------------------------------------
void __fastcall TTVPMenuContainerForm::Popup()
{
	// show menu bar and stay topmost
	if(IsWindowEnabled(OwnerForm->Handle))
	{
		WaitingShowing = false;
		Timer->Interval = 250/2;
		Left = 0;
		Top = 0;
		Width = OwnerForm->Width;
		ClientHeight = 1;
		if(Height == 1) return;
		if(!ShowingMenu)
		{
			Visible = true;
			BringToFront();
			SetWindowPos(Handle, HWND_TOPMOST, 0, 0, 0, 0,
					SWP_NOMOVE|SWP_NOREPOSITION|SWP_NOSIZE|SWP_SHOWWINDOW);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPMenuContainerForm::Conceal()
{
	Visible = false;
	Top = -Height-100; // bring this to out of the screen
	PostMessage(OwnerForm->Handle, TVP_WM_RETRIEVEFOCUS, 0, 0);
}
//---------------------------------------------------------------------------
void __fastcall TTVPMenuContainerForm::Drop()
{
	DroppedByKey = false;
	LastOutOfWindow = false;
	Popup();
}
//---------------------------------------------------------------------------
void __fastcall TTVPMenuContainerForm::StartDropWatch()
{
	if(WaitingShowing) return;
	WaitingShowing = true;
	Timer->Interval = 500/2;
	Timer->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TTVPMenuContainerForm::DropByKey()
{
	DroppedByKey = true;
	Popup();
}
//---------------------------------------------------------------------------
void __fastcall TTVPMenuContainerForm::PrepareToReceiveMenuCommand()
{
	// to receive menu command, this form must be shown
	// (no matter if the window is within the screen)
	Top = -Height - 100;
	Visible = true;
}
//---------------------------------------------------------------------------
void __fastcall TTVPMenuContainerForm::TimerTimer(TObject *Sender)
{
	// check mouse cursor position
	POINT pos = {0, OwnerForm->Height};
	GetCursorPos(&pos);
	bool outofwindow = pos.y > Height;

	// check popup opportunity
	if(pos.y == 0 && WaitingShowing)
	{
		Drop();
		return;
	}

	// check hiding oppertunity
	if(ShowingMenu)
	{
		LastOutOfWindow = false;
		return;
	}

	if(Visible && outofwindow && LastOutOfWindow) Conceal();

	LastOutOfWindow = outofwindow;
}
//---------------------------------------------------------------------------
void __fastcall TTVPMenuContainerForm::WMEnterMenuLoop(TMessage &Msg)
{
	ShowingMenu = true;
}
//---------------------------------------------------------------------------
void __fastcall TTVPMenuContainerForm::WMExitMenuLoop(TMessage &Msg)
{
	ShowingMenu = false;
}
//---------------------------------------------------------------------------
void __fastcall TTVPMenuContainerForm::WMSetFocus(TWMSetFocus &Msg)
{
	if(Msg.FocusedWnd != OwnerForm->Handle)
	{
		PostMessage(OwnerForm->Handle, TVP_WM_RETRIEVEFOCUS, 0, 0);
	}
}
//---------------------------------------------------------------------------

