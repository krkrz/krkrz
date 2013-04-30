//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Watch window
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "WatchFormUnit.h"
#include "MainFormUnit.h"
#include "tjsUtils.h"
#include "ScriptMgnIntf.h"
#include "DebugIntf.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TTVPWatchForm *TVPWatchForm;
//---------------------------------------------------------------------------
__fastcall TTVPWatchForm::TTVPWatchForm(TComponent* Owner)
	: TForm(Owner)
{
	// adjust component size
	StatusBar->Left = ClientWidth - StatusBar->Width;
	StatusBar->Top = ClientHeight - StatusBar->Height;

	ToolBar->Left = 0;
	ToolBar->Top = ClientHeight - ToolBar->Height;

	ListView->Width = ClientWidth;
	ListView->Height = ClientHeight - StatusBar->Height;

	// set hot keys
	ShowControllerMenuItem->ShortCut = TVPMainForm->ShowControllerMenuItem->ShortCut;
	ShowScriptEditorMenuItem->ShortCut = TVPMainForm->ShowScriptEditorMenuItem->ShortCut;
	ShowWatchMenuItem->ShortCut = TVPMainForm->ShowWatchMenuItem->ShortCut;
	ShowConsoleMenuItem->ShortCut = TVPMainForm->ShowConsoleMenuItem->ShortCut;
	ShowAboutMenuItem->ShortCut = TVPMainForm->ShowAboutMenuItem->ShortCut;
	CopyImportantLogMenuItem->ShortCut = TVPMainForm->CopyImportantLogMenuItem->ShortCut;

	// read profile
	tTVPProfileHolder * prof = TVPGetEnvironProfile();
	TVPEnvironProfileAddRef();
	static AnsiString section("watch");
	int n;
	n = prof->ReadInteger(section, "left", -1);
	if(n != -1) Left = n;
	n = prof->ReadInteger(section, "top", -1);
	if(n != -1) Top = n;
	n = prof->ReadInteger(section, "width", -1);
	if(n != -1) Width = n;
	n = prof->ReadInteger(section, "height", -1);
	if(n != -1) Height = n;
	n = prof->ReadInteger(section, "col0width", -1);
	if(n != -1) ListView->Column[0]->Width = n;
	n = prof->ReadInteger(section, "col1width", -1);
	if(n != -1) ListView->Column[1]->Width = n;
	n = prof->ReadInteger(section, "stayontop", 0);
	FormStyle = n ? fsStayOnTop : fsNormal;


	if(Left > Screen->Width) Left = Screen->Width - Width;
	if(Top > Screen->Height) Top = Screen->Height - Height;

	TStringList *list = new TStringList;

	try
	{
		prof->ReadStrings(section, "exprlist", list);

		// add to expr list
		for(int i = 0; i<list->Count; i++)
		{
			TListItem *newitem = ListView->Items->Add();
			newitem->Caption = list->Strings[i];
		}
	}
	catch(...)
	{
		delete list;
		throw;
	}

	delete list;

	n = prof->ReadInteger(section, "interval", -1);
	if(n != -1)
	{
		// set interval
		TMenuItem *parent = AutoUpdateIntervalMenuItem;
		for(int i = 0; i < parent->Count; i++)
		{
			TMenuItem *child = parent->Items[i];
			if(child->Tag == n) child->Checked = true;
		}
        UpdateTimer->Interval = n;
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWatchForm::FormDestroy(TObject *Sender)
{
	WriteProfile();
	TVPEnvironProfileRelease();
}
//---------------------------------------------------------------------------

void __fastcall TTVPWatchForm::FormClose(TObject *Sender,
	  TCloseAction &Action)
{
	// write profile to disk
	WriteProfile();
	TVPWriteEnvironProfile();

	// hide
	Action = caHide;
	TVPMainForm->NotifyWatchHiding();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWatchForm::WriteProfile()
{
	tTVPProfileHolder * prof = TVPGetEnvironProfile();
	static AnsiString section("watch");
	prof->WriteInteger(section, "left", Left);
	prof->WriteInteger(section, "top", Top);
	prof->WriteInteger(section, "width", Width);
	prof->WriteInteger(section, "height", Height);
	prof->WriteInteger(section, "col0width", ListView->Column[0]->Width);
	prof->WriteInteger(section, "col1width", ListView->Column[1]->Width);
	prof->WriteInteger(section, "stayontop", FormStyle == fsStayOnTop);

	TStringList *list = new TStringList;
	try
	{
		for(int i = 0; i<ListView->Items->Count; i++)
		{
			list->Add(ListView->Items->Item[i]->Caption);
		}
		prof->WriteStrings(section, "exprlist", list);
	}
	catch(...)
	{
		delete list;
		throw;
	}
	delete list;

	prof->WriteInteger(section, "interval", UpdateTimer->Interval);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWatchForm::FormShow(TObject *Sender)
{
	TVPMainForm->SmallIconImageList->GetIcon(14, Icon);
}
//---------------------------------------------------------------------------

void __fastcall TTVPWatchForm::ShowScriptEditorMenuItemClick(
	  TObject *Sender)
{
	TVPMainForm->ShowScriptEditorButtonClick(Sender);

}
//---------------------------------------------------------------------------

void __fastcall TTVPWatchForm::ShowWatchMenuItemClick(TObject *Sender)
{
	TVPMainForm->ShowWatchButtonClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWatchForm::ShowConsoleMenuItemClick(TObject *Sender)
{
	TVPMainForm->ShowConsoleButtonClick(Sender);
}
//---------------------------------------------------------------------------

void __fastcall TTVPWatchForm::ShowControllerMenuItemClick(TObject *Sender)
{
	TVPMainForm->ShowController();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWatchForm::ShowAboutMenuItemClick(TObject *Sender)
{
	TVPMainForm->ShowAboutMenuItemClick(Sender);

}
//---------------------------------------------------------------------------

void __fastcall TTVPWatchForm::CopyImportantLogMenuItemClick(
	  TObject *Sender)
{
	TVPMainForm->CopyImportantLogMenuItemClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWatchForm::NewExprButtonClick(TObject *Sender)
{
	// add new expression
	TListItem *newitem = ListView->Items->Add();
	newitem->Caption="void"; // initialy void
	newitem->EditCaption();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWatchForm::ListViewMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
	LastMouseX = X;
	LastMouseY = Y;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWatchForm::ListViewDblClick(TObject *Sender)
{
	// on double click
	TListItem *item;
	item = ListView->GetItemAt(LastMouseX, LastMouseY);

	if(item)
		item->EditCaption(); // edit
	else
		NewExprButtonClick(this); // new
}
//---------------------------------------------------------------------------

void __fastcall TTVPWatchForm::DeleteButtonClick(TObject *Sender)
{
	// delete expression(s)
	// delete marked item(s)
	ListView->Items->BeginUpdate();
	TCursor def = ListView->Cursor;
	ListView->Cursor = crHourGlass;
	try
	{
		for(int i = 0; i<ListView->Items->Count; i++)
		{
			if(ListView->Items->Item[i]->Selected)
				ListView->Items->Delete(i--);
		}
	}
	catch(...)
	{
		ListView->Cursor = def;
		ListView->Items->EndUpdate();
		throw;
	}
	ListView->Cursor = def;
	ListView->Items->EndUpdate();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWatchForm::EditExpressionMenuItemClick(TObject *Sender)
{
	if(!ListView->IsEditing())
	{
		if(ListView->Items->Count == 0)
		{
		 NewExprButtonClick(this);
		}
		else
		{
			if(ListView->Selected) ListView->Selected->EditCaption();
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TTVPWatchForm::ListViewKeyPress(TObject *Sender, char &Key)
{
	if(Key == 13) // enter
	{
		EditExpressionMenuItemClick(this);
	}
}
//---------------------------------------------------------------------------

void __fastcall TTVPWatchForm::ListViewEdited(TObject *Sender,
	  TListItem *Item, AnsiString &S)
{
	EliminateTimer->Enabled = true; // ensure "EliminateTimerTimer" is called
	if(S!="") EvalExpression(Item, &S);
}
//---------------------------------------------------------------------------
void __fastcall TTVPWatchForm::EliminateTimerTimer(TObject *Sender)
{
	EliminateTimer->Enabled = false;

	// eliminate all empty item(s)
	ListView->Items->BeginUpdate();
	TCursor def = ListView->Cursor;
	ListView->Cursor = crHourGlass;
	try
	{
		for(int i = 0; i<ListView->Items->Count; i++)
		{
			if(ListView->Items->Item[i]->Caption == "")
				ListView->Items->Delete(i--);
		}
	}
	catch(...)
	{
		ListView->Cursor = def;
		ListView->Items->EndUpdate();
		throw;
	}
	ListView->Cursor = def;
	ListView->Items->EndUpdate();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWatchForm::EvalExpression(TListItem *item, AnsiString *exp)
{
	// evaluate an item
	if(item->SubItems->Count < 1) item->SubItems->Add("");

	tTJSVariant result;

	ttstr result_str;

	if(exp)
		result_str = *exp;
	else
		result_str = ttstr(item->Caption);

	try
	{
		TVPExecuteExpression(result_str, &result);
	}
	catch(eTJS &e)
	{
		static AnsiString error("(error) ");
		item->SubItems->Strings[0] = error + e.GetMessage().c_str();
		return;
	}
	catch(...)
	{
		throw;
	}

	item->SubItems->Strings[0] = TJSVariantToReadableString(result).c_str();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWatchForm::EvalAll()
{
	// evaluate all expressions
	ListView->Items->BeginUpdate();
	TCursor def = ListView->Cursor;
	ListView->Cursor = crHourGlass;
	try
	{
		for(int i = 0; i<ListView->Items->Count; i++)
		{
			EvalExpression(ListView->Items->Item[i], NULL);
		}
	}
	catch(...)
	{
		ListView->Cursor = def;
		ListView->Items->EndUpdate();
		throw;
	}
	ListView->Cursor = def;
	ListView->Items->EndUpdate();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWatchForm::UpdateButtonClick(TObject *Sender)
{
	// update
	EvalAll();
}
//---------------------------------------------------------------------------
void __fastcall TTVPWatchForm::UpdateTimerTimer(TObject *Sender)
{
	// timer function

	// check for mouse button's down ( to avoid update in item selecting )
	// check for mouse position ( is mouse on the view ? )
	TPoint ul = ClientToScreen(TPoint(ListView->Left, ListView->Top));
	TPoint dr = ClientToScreen(TPoint(ListView->Width+ListView->Left,
		ListView->Height+ListView->Top));
	POINT cr;
	GetCursorPos(&cr);
	if(!( (ul.x <= cr.x && ul.y <= cr.y && dr.x > cr.x && dr.y > cr.y) && (
		(GetAsyncKeyState(VK_LBUTTON)&0x8000) ||
		(GetAsyncKeyState(VK_RBUTTON)&0x8000) ||
		(GetAsyncKeyState(VK_MBUTTON)&0x8000))))
	{
		EvalAll();  // evaluate all
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWatchForm::UIRealTimeMenuItemClick(TObject *Sender)
{
	TMenuItem *item = dynamic_cast<TMenuItem*>(Sender);
	if(item)
	{
		item->Checked = true;
		UpdateTimer->Interval = item->Tag;
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPWatchForm::AutoUpdateButtonClick(TObject *Sender)
{
	UpdateTimer->Enabled = AutoUpdateButton->Down;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWatchForm::AutoUpdateMenuItemClick(TObject *Sender)
{
	AutoUpdateButton->Down = !AutoUpdateButton->Down;
	UpdateTimer->Enabled = AutoUpdateButton->Down;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWatchForm::PopupMenuPopup(TObject *Sender)
{
	ShowOnTopMenuItem->Checked = FormStyle == fsStayOnTop;

	ShowControllerMenuItem->Checked = TVPMainForm->Visible;
	ShowScriptEditorMenuItem->Checked = TVPMainForm->GetScriptEditorVisible();
	ShowConsoleMenuItem->Checked = TVPMainForm->GetConsoleVisible();

	AutoUpdateMenuItem->Checked = AutoUpdateButton->Down;
}
//---------------------------------------------------------------------------
void __fastcall TTVPWatchForm::ShowOnTopMenuItemClick(TObject *Sender)
{
	FormStyle = FormStyle == fsStayOnTop ? fsNormal : fsStayOnTop;
}
//---------------------------------------------------------------------------


