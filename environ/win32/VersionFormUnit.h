//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Version information dialog box
//---------------------------------------------------------------------------

#ifndef VersionFormUnitH
#define VersionFormUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
//---------------------------------------------------------------------------
class TTVPVersionForm : public TForm
{
__published:	// IDE 管理のコンポーネント
    TButton *OKButton;
    TRichEdit *Memo;
    TPopupMenu *PopupMenu;
    TMenuItem *CopyMenuItem;
    TButton *CopyEnvInfoButton;
    void __fastcall PopupMenuPopup(TObject *Sender);
    void __fastcall CopyMenuItemClick(TObject *Sender);
    void __fastcall CopyEnvInfoButtonClick(TObject *Sender);
private:	// ユーザー宣言
public:		// ユーザー宣言
    __fastcall TTVPVersionForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TTVPVersionForm *TVPVersionForm;
//---------------------------------------------------------------------------
extern void TVPShowVersionForm();
//---------------------------------------------------------------------------

#endif
