//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Watch window
//---------------------------------------------------------------------------
#ifndef WatchFormUnitH
#define WatchFormUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ToolWin.hpp>
#include <Menus.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TTVPWatchForm : public TForm
{
__published:	// IDE 管理のコンポーネント
	TToolBar *ToolBar;
	TStatusBar *StatusBar;
	TToolButton *UpdateButton;
	TToolButton *AutoUpdateButton;
	TPopupMenu *PopupMenu;
	TMenuItem *ShowScriptEditorMenuItem;
	TMenuItem *ShowConsoleMenuItem;
	TMenuItem *NewExprMenuItem;
	TMenuItem *DeleteExprMenuItem;
	TMenuItem *N1;
	TMenuItem *N2;
	TMenuItem *AutoUpdateIntervalMenuItem;
	TMenuItem *UIRealTimeMenuItem;
	TMenuItem *UI0_2SecMenuItem;
	TMenuItem *UI0_5SecMenuItem;
	TMenuItem *N071;
	TMenuItem *UI1SecMenuItem;
	TMenuItem *N221;
	TMenuItem *UI3SecMenuItem;
	TMenuItem *UI5SecMenuItem;
	TMenuItem *UI9SecMenuItem;
	TListView *ListView;
	TToolButton *NewExprButton;
	TToolButton *DeleteButton;
	TToolButton *ToolButton3;
	TMenuItem *UpdateMenuItem;
	TMenuItem *AutoUpdateMenuItem;
	TTimer *EliminateTimer;
	TTimer *UpdateTimer;
	TMenuItem *ShowControllerMenuItem;
	TMenuItem *ShowAboutMenuItem;
	TMenuItem *EditExpressionMenuItem;
	TMenuItem *CopyImportantLogMenuItem;
	TMenuItem *N3;
	TMenuItem *ShowOnTopMenuItem;
	TMenuItem *ShowWatchMenuItem;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall NewExprButtonClick(TObject *Sender);
	void __fastcall DeleteButtonClick(TObject *Sender);
	void __fastcall ListViewEdited(TObject *Sender, TListItem *Item,
          AnsiString &S);
	void __fastcall EliminateTimerTimer(TObject *Sender);
	void __fastcall UpdateButtonClick(TObject *Sender);
	void __fastcall ListViewDblClick(TObject *Sender);
	void __fastcall ListViewMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
	void __fastcall ShowScriptEditorMenuItemClick(TObject *Sender);
	void __fastcall ShowConsoleMenuItemClick(TObject *Sender);
	void __fastcall UIRealTimeMenuItemClick(TObject *Sender);
	void __fastcall AutoUpdateButtonClick(TObject *Sender);
	void __fastcall AutoUpdateMenuItemClick(TObject *Sender);
	void __fastcall PopupMenuPopup(TObject *Sender);
	void __fastcall UpdateTimerTimer(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall ShowControllerMenuItemClick(TObject *Sender);
	void __fastcall ShowAboutMenuItemClick(TObject *Sender);
	void __fastcall EditExpressionMenuItemClick(TObject *Sender);
	void __fastcall ListViewKeyPress(TObject *Sender, char &Key);
	void __fastcall CopyImportantLogMenuItemClick(TObject *Sender);
	void __fastcall ShowOnTopMenuItemClick(TObject *Sender);
	void __fastcall ShowWatchMenuItemClick(TObject *Sender);
private:	// ユーザー宣言

public:		// ユーザー宣言
	__fastcall TTVPWatchForm(TComponent* Owner);

private:
	void __fastcall WriteProfile();

	int LastMouseX, LastMouseY;
	void __fastcall EvalExpression(TListItem *item, AnsiString *exp);
	void __fastcall EvalAll();
};
//---------------------------------------------------------------------------
extern PACKAGE TTVPWatchForm *TVPWatchForm;
//---------------------------------------------------------------------------
#endif
