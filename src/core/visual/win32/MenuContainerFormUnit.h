//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// floating menu bar implementation ( used by WindowForm when fullscreened )
//---------------------------------------------------------------------------

#ifndef MenuContainerFormUnitH
#define MenuContainerFormUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TTVPWindowForm;
class TTVPMenuContainerForm : public TForm
{
__published:	// IDE 管理のコンポーネント
	TTimer *Timer;
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall TimerTimer(TObject *Sender);
private:	// ユーザー宣言
	TTVPWindowForm *OwnerForm;

	bool DroppedByKey;
	bool LastOutOfWindow;
	bool WaitingShowing;
	bool ShowingMenu;

public:		// ユーザー宣言
	__fastcall TTVPMenuContainerForm(TComponent* Owner);

private:
	void __fastcall Popup();
	void __fastcall Conceal();
	void __fastcall Drop();

public:
	void __fastcall StartDropWatch();
	void __fastcall DropByKey();

	bool __fastcall GetShowingMenu() const { return ShowingMenu; }

	void __fastcall PrepareToReceiveMenuCommand();

protected:

BEGIN_MESSAGE_MAP
	VCL_MESSAGE_HANDLER( WM_ENTERMENULOOP,  TMessage, WMEnterMenuLoop)
	VCL_MESSAGE_HANDLER( WM_EXITMENULOOP,   TMessage, WMExitMenuLoop)
	VCL_MESSAGE_HANDLER( WM_SETFOCUS,	TWMSetFocus, WMSetFocus)
END_MESSAGE_MAP(TForm)
	void __fastcall WMEnterMenuLoop(TMessage &Msg);
	void __fastcall WMExitMenuLoop(TMessage &Msg);
	void __fastcall WMSetFocus(TWMSetFocus &Msg);
};
//---------------------------------------------------------------------------
extern PACKAGE TTVPMenuContainerForm *TVPMenuContainerForm;
//---------------------------------------------------------------------------
#endif
