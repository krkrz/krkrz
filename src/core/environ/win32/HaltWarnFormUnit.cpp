//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Force Suicide User Confirmation Form
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "HaltWarnFormUnit.h"
#include "MainFormUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TTVPHaltWarnForm *TVPHaltWarnForm;
//---------------------------------------------------------------------------
__fastcall TTVPHaltWarnForm::TTVPHaltWarnForm(TComponent* Owner)
	: TForm(Owner)
{
	Caption = Application->Title;
}
//---------------------------------------------------------------------------
void __fastcall TTVPHaltWarnForm::IconPaintBoxPaint(TObject *Sender)
{
	IconPaintBox->Canvas->Draw(
		IconPaintBox->Width/2 - 16, IconPaintBox->Height /2 - 16,
		Application->Icon);
}
//---------------------------------------------------------------------------
void __fastcall TTVPHaltWarnForm::ContinueButtonClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TTVPHaltWarnForm::ExitButtonClick(TObject *Sender)
{
	// force suicide
	DWORD pid;
	GetWindowThreadProcessId(Application->Handle, &pid);
	HANDLE hp=OpenProcess(PROCESS_TERMINATE, FALSE, pid);
	if(hp)
	{
		TerminateProcess(hp, 0);
		CloseHandle(hp);
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPHaltWarnForm::FormClose(TObject *Sender,
	  TCloseAction &Action)
{
	Action = caFree;
	TVPHaltWarnForm = NULL;
	TVPMainForm->NotifyHaltWarnFormClosed();
}
//---------------------------------------------------------------------------

