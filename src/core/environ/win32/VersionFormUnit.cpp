//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Version information dialog box
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "MsgIntf.h"
#include "VersionFormUnit.h"
#include "ConsoleFormUnit.h"
#include "MainFormUnit.h"
#include "WindowImpl.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TTVPVersionForm *TVPVersionForm;
//---------------------------------------------------------------------------
__fastcall TTVPVersionForm::TTVPVersionForm(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TTVPVersionForm::PopupMenuPopup(TObject *Sender)
{
	CopyMenuItem->Enabled = Memo->SelLength != 0;
}
//---------------------------------------------------------------------------
void __fastcall TTVPVersionForm::CopyMenuItemClick(TObject *Sender)
{
	Memo->CopyToClipboard();
}
//---------------------------------------------------------------------------
void __fastcall TTVPVersionForm::CopyEnvInfoButtonClick(TObject *Sender)
{
	TVPCopyImportantLogToClipboard();
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void TVPShowVersionForm()
{
	// get DirectDraw driver information
	TVPEnsureDirectDrawObject();
	TVPDumpDirectDrawDriverInformation();

	// show version information
	TTVPVersionForm *form = new TTVPVersionForm(Application);
	TStringList *list = new TStringList();
	list->Text = TVPGetAboutString().AsAnsiString();
	form->Memo->Lines->Assign(list);
	form->Memo->SelStart = 0;
	delete list;
	form->ShowModal();
	delete form;
}
//---------------------------------------------------------------------------




