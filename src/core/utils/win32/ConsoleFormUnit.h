//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Console
//---------------------------------------------------------------------------
#ifndef ConsoleFormUnitH
#define ConsoleFormUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <ToolWin.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
extern void TVPCopyImportantLogToClipboard();
//---------------------------------------------------------------------------
class TLogViewer;
class TTVPConsoleForm : public TForm
{
__published:	// IDE 管理のコンポーネント
	TStatusBar *StatusBar;
	TPopupMenu *PopupMenu;
	TMenuItem *CopyMenuItem;
	TMenuItem *N1;
	TMenuItem *ShowScriptEditorMenuItem;
	TMenuItem *ShowAboutMenuItem;
	TMenuItem *ShowControllerMenuItem;
	TMenuItem *ShowWatchMenuItem;
	TComboBox *ExprComboBox;
	TToolBar *ToolBar;
	TToolButton *ExecButton;
	TPanel *LogPanel;
	TMenuItem *SelectAllMenuItem;
	TMenuItem *CopyImportantLogMenuItem;
	TMenuItem *N2;
	TMenuItem *AutoShowOnErrorMenuItem;
	TMenuItem *N3;
	TMenuItem *ShowOnTopMenuItem;
	TFontDialog *FontDialog;
	TMenuItem *SelectFontMenuItem;
	TMenuItem *ShowConsoleMenuItem;
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall FormHide(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall PopupMenuPopup(TObject *Sender);
	void __fastcall CopyMenuItemClick(TObject *Sender);
	void __fastcall ShowScriptEditorMenuItemClick(TObject *Sender);
	void __fastcall ShowAboutMenuItemClick(TObject *Sender);
	void __fastcall ShowControllerMenuItemClick(TObject *Sender);
	void __fastcall ShowWatchMenuItemClick(TObject *Sender);
	void __fastcall ExecButtonClick(TObject *Sender);
	void __fastcall ExprComboBoxKeyPress(TObject *Sender, char &Key);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall FormMouseWheelDown(TObject *Sender, TShiftState Shift,
          TPoint &MousePos, bool &Handled);
	void __fastcall FormMouseWheelUp(TObject *Sender, TShiftState Shift,
          TPoint &MousePos, bool &Handled);
	void __fastcall SelectAllMenuItemClick(TObject *Sender);
	void __fastcall ExprComboBoxKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
	void __fastcall CopyImportantLogMenuItemClick(TObject *Sender);
	void __fastcall AutoShowOnErrorMenuItemClick(TObject *Sender);
	void __fastcall ShowOnTopMenuItemClick(TObject *Sender);
	void __fastcall SelectFontMenuItemClick(TObject *Sender);
	void __fastcall ShowConsoleMenuItemClick(TObject *Sender);
private:	// ユーザー宣言
	TLogViewer *LogViewer;
	tjs_int LastSelectedItem;

protected:
BEGIN_MESSAGE_MAP
	VCL_MESSAGE_HANDLER( WM_USER+0x31, TMessage, WMClearSelection)
END_MESSAGE_MAP(TForm)
	void __fastcall WMClearSelection(TMessage &Msg);

public:		// ユーザー宣言
	__fastcall TTVPConsoleForm(TComponent* Owner);
	__fastcall ~TTVPConsoleForm();

	void __fastcall AddLine(const ttstr & line);
private:
	void __fastcall GetLastLog();
	void __fastcall WriteProfile();

	bool __fastcall CheckMenuShortCuts(WORD key, TShiftState shift);
};
//---------------------------------------------------------------------------
extern PACKAGE TTVPConsoleForm *TVPConsoleForm;
//---------------------------------------------------------------------------
#endif
