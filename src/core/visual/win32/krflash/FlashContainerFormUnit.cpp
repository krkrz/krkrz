//---------------------------------------------------------------------------
// FlashContainerFormUnit.cpp ( part of KRFLASH.DLL )
// (c)2001-2009, W.Dee <dee@kikyou.info> and contributors
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "FlashContainerFormUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "ShockwaveFlashObjects_OCX"
#pragma resource "*.dfm"
TFlashContainerForm *FlashContainerForm;

/*
//---------------------------------------------------------------------------
static int HookRefCount = 0;
static HHOOK HookHandle = NULL;
static LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode == HC_ACTION || nCode >= 0)
	{
		// process
		MSG * msg = (MSG *)lParam;

		if(msg->message == WM_CONTEXTMENU)
		{
			// prevent context menu
			HWND wnd = msg->hwnd;
			char classname[256];
			do
			{
				classname[0] = 0;
				GetClassName(wnd, classname, 255);
				if(!strcmp(classname, "FlashContainerForm"))
				{
					msg->message == WM_NULL;
					break;
				}

				wnd = GetParent(wnd);
			} while(wnd);
		}
	}

	return CallNextHookEx(HookHandle, nCode, wParam, lParam);
}
//---------------------------------------------------------------------------
static void HookWindow()
{
	if(HookRefCount == 0)
	{
		// hook
		HookHandle = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)GetMsgProc,
			NULL, GetCurrentThreadId());
	}
	HookRefCount ++;
}
//---------------------------------------------------------------------------
static void UnhookWindow()
{
	HookRefCount --;
	if(HookRefCount == 0)
	{
		// unhook
		UnhookWindowsHookEx(HookHandle);
	}
}
//---------------------------------------------------------------------------
*/

//---------------------------------------------------------------------------
__fastcall TFlashContainerForm::TFlashContainerForm(TComponent* Owner,
	tTVPFlashOverlay *overlay, HWND ownerwin,
	const RECT &rect)
	:  TForm((Overlay = overlay, OwnerWindow = ownerwin, Rect = rect, Owner))
{
	Invisible = false;
	VisibleState = true;
}
//---------------------------------------------------------------------------
void __fastcall TFlashContainerForm::CreateParams(Controls::TCreateParams &Params)
{
	TForm::CreateParams(Params);

	Params.WndParent = OwnerWindow;

	Params.Style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD;
	Params.X = Rect.left;
	Params.Y = Rect.top;
	Params.Width = Rect.right - Rect.left;
	Params.Height = Rect.bottom - Rect.top;
}
//---------------------------------------------------------------------------
void __fastcall TFlashContainerForm::SetFormParent(HWND parent)
{
	OwnerWindow = parent;
	if(!OwnerWindow)
	{
		Invisible = true;
		Visible = false;
		::SetParent(Handle, NULL);
	}
	else
	{
		::SetParent(Handle, parent);
		Invisible = false;
		if(VisibleState) SetFlashVisible(true);
	}
}
//---------------------------------------------------------------------------
void __fastcall TFlashContainerForm::SetMovie(wchar_t *filename)
{
	Flash->Movie = filename;
}
//---------------------------------------------------------------------------
void __fastcall TFlashContainerForm::Play()
{
	Flash->Play();
}
//---------------------------------------------------------------------------
void __fastcall TFlashContainerForm::Stop()
{
	Flash->Stop();
}
//---------------------------------------------------------------------------
void __fastcall TFlashContainerForm::Pause()
{
	// not implemented
}
//---------------------------------------------------------------------------
void __fastcall TFlashContainerForm::SetFlashVisible(bool b)
{
	VisibleState= b;
	if(Invisible) b = false;
	Visible = b;
	if(b)
	{
		SetWindowPos(Handle, HWND_TOP, 0, 0, 0, 0,
				SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
	}
}
//---------------------------------------------------------------------------
tTVPVideoStatus __fastcall TFlashContainerForm::GetStatus()
{
	return Flash->Playing ? vsPlaying : vsStopped;
}
//---------------------------------------------------------------------------
void __fastcall TFlashContainerForm::TimerTimer(TObject *Sender)
{
	if(::GetFocus() == Flash->Handle)
		::SetFocus(OwnerWindow);
}
//---------------------------------------------------------------------------
void __fastcall TFlashContainerForm::FlashFSCommand(TObject *Sender,
      BSTR command, BSTR args)
{
	Overlay->SendCommand(command, args);	
}
//---------------------------------------------------------------------------


