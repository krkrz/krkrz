//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// System Main Window (Controller)
//---------------------------------------------------------------------------
#ifndef MainFormUnitH
#define MainFormUnitH
//---------------------------------------------------------------------------
#include <windows.h>
#include <string>
#include "tstring.h"
//---------------------------------------------------------------------------
class TTVPMainForm
{
private:	// ユーザー宣言
	bool ContinuousEventCalling;
	bool AutoShowConsoleOnError;
	bool ApplicationStayOnTop;
	bool ApplicationActivating;
	bool ApplicationNotMinimizing;

	bool EventEnable;

	DWORD LastCompactedTick;
	DWORD LastContinuousTick;
	DWORD LastCloseClickedTick;
	DWORD LastShowModalWindowSentTick;
	DWORD LastRehashedTick;

	DWORD MixedIdleTick;
public:
	TTVPMainForm();

	void InvokeEvents();
	void CallDeliverAllEventsOnIdle();

	void BeginContinuousEvent();
	void EndContinuousEvent();

	void NotifyEventDelivered();

	void SetVisible( bool b );
	bool GetVisible() const;

	void SetEventButtonDown( bool b ) {
		EventEnable = b;
	}
	bool GetEventButtonDown() const { return EventEnable; }

	bool GetApplicationStayOnTop();
	void SetApplicationStayOnTop( bool );

	void NotifySystemError();

	bool GetConsoleVisible();
	void SetConsoleVisible( bool );
	
	bool GetApplicationActivating() const { return ApplicationActivating; }
	bool GetApplicationNotMinimizing() const { return ApplicationNotMinimizing; }

	HWND GetHandle() { return NULL; }

	bool ApplicationIdel();
private:
	void DeliverEvents();
};
enum {
  mtWarning = MB_ICONWARNING,
  mtError = MB_ICONERROR,
  mtInformation = MB_ICONINFORMATION,
  mtConfirmation = MB_ICONQUESTION,
  mtCustom = 0
};
/*
MB_ABORTRETRYIGNORE	メッセージボックスに［中止］、［再試行］、［無視］の各プッシュボタンを表示します。
MB_CANCELTRYCONTINUE	Windows 2000：メッセージボックスに［キャンセル］、［再実行］、［続行］の各プッシュボタンを表示します。MB_ABORTRETRYIGNORE の代わりに、このメッセージボックスタイプを使ってください。
MB_HELP	Windows 95/98、Windows NT 4.0 以降：メッセージボックスに［ヘルプ］ボタンを追加します。ユーザーが［ヘルプ］ボタンをクリックするか F1 キーを押すと、システムはオーナーへ メッセージを送信します。
MB_OK	メッセージボックスに［OK］プッシュボタンだけを表示します。これは既定のメッセージボックスタイプです。
MB_OKCANCEL	メッセージボックスに［OK］、［キャンセル］の各プッシュボタンを表示します。
MB_RETRYCANCEL	メッセージボックスに［再試行］、［キャンセル］の各プッシュボタンを表示します。
MB_YESNO	メッセージボックスに［はい］、［いいえ］の各プッシュボタンを表示します。
MB_YESNOCANCEL	メッセージボックスに［はい］、［いいえ］、［キャンセル］の各プッシュボタンを表示します。
*/
inline int MessageDlg( const tstring& string, int type, int buttons, void* helpCtx ) {
	return ::MessageBox( NULL, string.c_str(), _T(""), type | buttons );
}

extern TTVPMainForm *TVPMainForm;

extern bool TVPMainFormAlive;

#endif
