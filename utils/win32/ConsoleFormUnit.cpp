//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Console
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "ConsoleFormUnit.h"

#include "DebugIntf.h"
#include "MainFormUnit.h"
#include "ScriptMgnIntf.h"
#include "SysInitImpl.h"
#include "MsgIntf.h"
#include "TLogViewer.h"
#include "WindowImpl.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TTVPConsoleForm *TVPConsoleForm;
//---------------------------------------------------------------------------
#define TVP_STREAM_DISP_INITIAL_LINES 1024
#define TVP_STREAM_DISP_TRIM_CHARS 65536
#define TVP_STREAM_DISP_MAX_CHARS 98394
#define TVP_COMBO_HIST_MAX 64
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPConsoleFormOnLog
//---------------------------------------------------------------------------
void TVPConsoleFormOnLog(const ttstr & log)
{
	// add "log" to the ConsoleForm window
	TVPDumpHWException(); // dump cached hw exceptoin
	if(TVPConsoleForm) TVPConsoleForm->AddLine(log);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// TVPCopyImportantLogToClipboard
//---------------------------------------------------------------------------
void TVPCopyImportantLogToClipboard()
{
	// get DirectDraw driver information
	TVPEnsureDirectDrawObject();
	TVPDumpDirectDrawDriverInformation();

	// copy
	TVPCopyToClipboard(TVPGetImportantLog());
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TConsoleForm
//---------------------------------------------------------------------------
__fastcall TTVPConsoleForm::TTVPConsoleForm(TComponent* Owner)
	: TForm(Owner)
{
	TVPConsoleForm = this;
	TVPSetOnLog(TVPConsoleFormOnLog);

//	MemoLineCount = 0;

	// create log viewer
	LogViewer = new TLogViewer(this);

	// adjust components
	StatusBar->Left = ClientWidth - StatusBar->Width;
	StatusBar->Top = ClientHeight - StatusBar->Height;
	ToolBar->Left = 0;
	ToolBar->Top = ClientHeight - ToolBar->Height;
	ExprComboBox->Top = ClientHeight - ExprComboBox->Height;
	ExprComboBox->Left = ToolBar->Width;
	ExprComboBox->Width = ClientWidth - ToolBar->Width - StatusBar->Width;
	LogPanel->Left = 0;
	LogPanel->Top = 0;
	LogPanel->Width = ClientWidth;
	LogPanel->Height = ClientHeight - ExprComboBox->Height;
	LogViewer->Parent = LogPanel;
	LogViewer->Align = alClient;
	LogViewer->PopupMenu = PopupMenu;

	// set hot keys
	ShowControllerMenuItem->ShortCut = TVPMainForm->ShowControllerMenuItem->ShortCut;
	ShowScriptEditorMenuItem->ShortCut = TVPMainForm->ShowScriptEditorMenuItem->ShortCut;
	ShowWatchMenuItem->ShortCut = TVPMainForm->ShowWatchMenuItem->ShortCut;
	ShowConsoleMenuItem->ShortCut = TVPMainForm->ShowConsoleMenuItem->ShortCut;
	ShowAboutMenuItem->ShortCut = TVPMainForm->ShowAboutMenuItem->ShortCut;
	CopyImportantLogMenuItem->ShortCut = TVPMainForm->CopyImportantLogMenuItem->ShortCut;

	// load proflie
	tTVPProfileHolder * prof = TVPGetEnvironProfile();
	TVPEnvironProfileAddRef();
	static AnsiString section("console");
	int n;
	n = prof->ReadInteger(section, "left", -1);
	if(n != -1) Left = n;
	n = prof->ReadInteger(section, "top", -1);
	if(n != -1) Top = n;
	n = prof->ReadInteger(section, "width", -1);
	if(n != -1) Width = n;
	n = prof->ReadInteger(section, "height", -1);
	if(n != -1) Height = n;
	n = prof->ReadInteger(section, "stayontop", 0);
	FormStyle = n ? fsStayOnTop : fsNormal;

	AnsiString fontinfo = prof->ReadString(section, "font", "");
	if(fontinfo != "")
	{
		LOGFONT lf;
		ZeroMemory(&lf, sizeof(lf));
		const char *p = fontinfo.c_str();
		unsigned char *d = (unsigned char *)&lf;
		unsigned char *dlim = (unsigned char *)&lf + sizeof(lf);
		while(*p && d < dlim)
		{
			if(!p[1]) break;
			*d = ((p[0] - 'a') << 4) + (p[1] - 'A');
			d ++;
			p += 2;
		}
		HFONT font = CreateFontIndirect(&lf);
		if(font != NULL) LogPanel->Font->Handle = font;
	}


	LogViewer->SetFont(LogPanel->Font);

	if(Left > Screen->Width) Left = Screen->Width - Width;
	if(Top > Screen->Height) Top = Screen->Height - Height;

	prof->ReadStrings(section, "exprhist", ExprComboBox->Items);

	ExprComboBox->ItemIndex = LastSelectedItem = ExprComboBox->Items->Count - 1;

	// load show-on-error state
	AutoShowOnErrorMenuItem->Checked = TVPMainForm->GetAutoShowConsoleOnError();
}
//---------------------------------------------------------------------------
__fastcall TTVPConsoleForm::~TTVPConsoleForm()
{
	delete LogViewer, LogViewer = NULL;
	TVPSetOnLog(NULL);
	TVPConsoleForm = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TTVPConsoleForm::FormDestroy(TObject *Sender)
{
	WriteProfile();
	TVPEnvironProfileRelease();
}
//---------------------------------------------------------------------------
void __fastcall TTVPConsoleForm::WriteProfile()
{
	tTVPProfileHolder * prof = TVPGetEnvironProfile();
	static AnsiString section("console");
	prof->WriteInteger(section, "left", Left);
	prof->WriteInteger(section, "top", Top);
	prof->WriteInteger(section, "width", Width);
	prof->WriteInteger(section, "height", Height);
	prof->WriteStrings(section, "exprhist", ExprComboBox->Items);
	prof->WriteInteger(section, "stayontop", FormStyle == fsStayOnTop);

	LOGFONT lf;
	ZeroMemory(&lf, sizeof(lf));
	GetObject(LogPanel->Font->Handle, sizeof(lf), &lf);

	unsigned char *d = (unsigned char *)&lf;
	unsigned char *dlim = (unsigned char *)&lf + sizeof(lf);

	AnsiString str;
	while(d < dlim)
	{
		char ch[3];
		ch[2] = 0;
		ch[0] = (char)((*d >> 4) + 'a');
		ch[1] = (char)((*d & 0x0f) + 'A');
		str += ch;
		d ++;
	}

	prof->WriteString(section, "font", str);
}
//---------------------------------------------------------------------------
void __fastcall TTVPConsoleForm::AddLine(const ttstr & line)
{
	if(LogViewer)
	{
		LogViewer->Append(line, true);
		LogViewer->Trim(TVP_STREAM_DISP_MAX_CHARS, TVP_STREAM_DISP_TRIM_CHARS);
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPConsoleForm::GetLastLog()
{
	if(LogViewer)
	{
		LogViewer->SetText(TVPGetLastLog(TVP_STREAM_DISP_INITIAL_LINES));
		LogViewer->ShowLast();
	}
}
//---------------------------------------------------------------------------
void __fastcall TTVPConsoleForm::FormClose(TObject *Sender,
	  TCloseAction &Action)
{
	// write profile to disk
	WriteProfile();
	TVPWriteEnvironProfile();

	// hide
	Action = caHide;
	TVPMainForm->NotifyConsoleHiding();
}
//---------------------------------------------------------------------------
void __fastcall TTVPConsoleForm::FormHide(TObject *Sender)
{
	TVPSetOnLog(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TTVPConsoleForm::FormShow(TObject *Sender)
{
	TVPMainForm->SmallIconImageList->GetIcon(13, Icon);
	GetLastLog();
	TVPSetOnLog(TVPConsoleFormOnLog);
}
//---------------------------------------------------------------------------
void __fastcall TTVPConsoleForm::PopupMenuPopup(TObject *Sender)
{
	ShowOnTopMenuItem->Checked = FormStyle == fsStayOnTop;
	ShowControllerMenuItem->Checked = TVPMainForm->Visible;
	ShowScriptEditorMenuItem->Checked = TVPMainForm->GetScriptEditorVisible();
	ShowWatchMenuItem->Checked = TVPMainForm->GetWatchVisible();
	CopyMenuItem->Enabled = LogViewer->CanCopy();
}
//---------------------------------------------------------------------------
void __fastcall TTVPConsoleForm::CopyMenuItemClick(TObject *Sender)
{
	LogViewer->CopyToClipboard();	
}
//---------------------------------------------------------------------------
void __fastcall TTVPConsoleForm::SelectAllMenuItemClick(TObject *Sender)
{
	LogViewer->SelectAll();
}
//---------------------------------------------------------------------------
void __fastcall TTVPConsoleForm::ShowScriptEditorMenuItemClick(TObject *Sender)
{
	TVPMainForm->ShowScriptEditorButtonClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TTVPConsoleForm::ShowWatchMenuItemClick(TObject *Sender)
{
	TVPMainForm->ShowWatchButtonClick(Sender);
}
//---------------------------------------------------------------------------

void __fastcall TTVPConsoleForm::ShowConsoleMenuItemClick(TObject *Sender)
{
	TVPMainForm->ShowConsoleButtonClick(Sender);
}
//---------------------------------------------------------------------------

void __fastcall TTVPConsoleForm::ShowAboutMenuItemClick(TObject *Sender)
{
	TVPMainForm->ShowAboutMenuItemClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TTVPConsoleForm::CopyImportantLogMenuItemClick(
	  TObject *Sender)
{
	TVPMainForm->CopyImportantLogMenuItemClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TTVPConsoleForm::ShowControllerMenuItemClick(TObject *Sender)
{
	TVPMainForm->ShowController();
}
//---------------------------------------------------------------------------
void __fastcall TTVPConsoleForm::ExecButtonClick(TObject *Sender)
{
	if(ExprComboBox->Text != "")
	{
		ttstr result_str = ttstr(ExprComboBox->Text.c_str());

		// enter to the history list
		if(ExprComboBox->Items->Count != 0)
		{
			int last = ExprComboBox->Items->Count - 1;
			int idx = -1;
			for(tjs_int i = 0; i<ExprComboBox->Items->Count; i++)
			{
				if(ExprComboBox->Items->Strings[i] == ExprComboBox->Text)
				{
					idx = i;
					break;
				}
			}
			if(idx != -1)
			{
				// already in the list
				ExprComboBox->Items->Move(idx, last - 1);
			}
			else
			{
				// replace last
				ExprComboBox->Items->Strings[last] = ExprComboBox->Text;
				ExprComboBox->Items->Add("");
				if(ExprComboBox->Items->Count > TVP_COMBO_HIST_MAX)
					ExprComboBox->Items->Delete(0);
			}
		}
		else
		{
			ExprComboBox->Items->Add(ExprComboBox->Text);
			ExprComboBox->Items->Add("");
		}

		ExprComboBox->ItemIndex = LastSelectedItem = ExprComboBox->Items->Count - 1;

		// execute the expression
		tTJSVariant result;
		try
		{
			TVPExecuteExpression(result_str, &result);
		}
		catch(eTJS &e)
		{
			result_str = TVPConsoleResult + result_str +
				TVPExceptionHadBeenOccured + e.GetMessage();
			TVPAddLog(result_str);
//			TVPShowScriptException(e);
			return;
		}
		catch(...)
		{
			throw;
		}

		result_str = TVPConsoleResult + result_str;
		result_str += TJS_W(" = ");
		result_str += TJSVariantToReadableString(result);
		TVPAddLog(result_str);
	}
}
//---------------------------------------------------------------------------

void __fastcall TTVPConsoleForm::ExprComboBoxKeyPress(TObject *Sender,
      char &Key)
{
	if(Key == 13 ) // enter key
	{
		ExecButtonClick(Sender);
		Key = 0;
	}
}
//---------------------------------------------------------------------------
bool __fastcall TTVPConsoleForm::CheckMenuShortCuts(WORD key, TShiftState shift)
{
	for(int i = 0; i < PopupMenu->Items->Count; i++)
	{
		TMenuItem *item = PopupMenu->Items->Items[i];
		if(item->ShortCut != 0)
		{
			WORD item_key;
			TShiftState item_shift;
			ShortCutToKey(item->ShortCut, item_key, item_shift);
			if(item_key == key && item_shift == shift)
			{
				item->Click();
				return true;
			}
		}
	}
	return false;
}
//---------------------------------------------------------------------------
void __fastcall TTVPConsoleForm::ExprComboBoxKeyDown(TObject *Sender,
	  WORD &Key, TShiftState Shift)
{
	if(Key == VK_UP || Key == VK_DOWN)
	{
		if(!Shift.Contains(ssShift))
		{
			::PostMessage(Handle, WM_USER+0x31, 0, 0);
		}
		else if(Shift.Contains(ssShift))
		{
			LogViewer->ScrollBy(Key == VK_UP ? -1 : 1);
			Key = 0;
		}
	}
	else if(Shift.Contains(ssCtrl) && (Key == 'C' || Key == VK_INSERT))
	{
		if(ExprComboBox->SelLength == 0 &&
			LogViewer->CanCopy())
		{
			LogViewer->CopyToClipboard();
			Key = 0;
		}
	}
	else if(Shift.Contains(ssShift) && Key == VK_F4)
	{
		TVPMainForm->ShowConsoleButtonClick(this);
		Key = 0;
	}

	if(CheckMenuShortCuts(Key, Shift)) Key = 0;
}
//---------------------------------------------------------------------------
void __fastcall TTVPConsoleForm::FormMouseWheelDown(TObject *Sender,
	  TShiftState Shift, TPoint &MousePos, bool &Handled)
{
	LogViewer->ScrollBy(3);
	Handled = true;
}
//---------------------------------------------------------------------------

void __fastcall TTVPConsoleForm::FormMouseWheelUp(TObject *Sender,
	  TShiftState Shift, TPoint &MousePos, bool &Handled)
{
	LogViewer->ScrollBy(-3);
	Handled = true;
}
//---------------------------------------------------------------------------
void __fastcall TTVPConsoleForm::WMClearSelection(TMessage &Msg)
{
	ExprComboBox->SelStart = ExprComboBox->Text.Length();
	ExprComboBox->SelLength = 0;
}
//---------------------------------------------------------------------------
void __fastcall TTVPConsoleForm::AutoShowOnErrorMenuItemClick(
	  TObject *Sender)
{
	AutoShowOnErrorMenuItem->Checked = !AutoShowOnErrorMenuItem->Checked;
	TVPMainForm->SetAutoShowConsoleOnError(AutoShowOnErrorMenuItem->Checked);
}
//---------------------------------------------------------------------------

void __fastcall TTVPConsoleForm::ShowOnTopMenuItemClick(TObject *Sender)
{
	FormStyle = FormStyle == fsStayOnTop ? fsNormal : fsStayOnTop;
}
//---------------------------------------------------------------------------

void __fastcall TTVPConsoleForm::SelectFontMenuItemClick(TObject *Sender)
{
	FontDialog->Font->Assign(LogPanel->Font);
	if(FontDialog->Execute())
	{
		LogPanel->Font->Assign(FontDialog->Font);
		LogViewer->SetFont(LogPanel->Font);
	}
}
//---------------------------------------------------------------------------

