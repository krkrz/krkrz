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
#include "TVPTimer.h"
//---------------------------------------------------------------------------
class TTVPMainForm
{
private:	// ÉÜÅ[ÉUÅ[êÈåæ
	bool ContinuousEventCalling;
	bool AutoShowConsoleOnError;
	bool ApplicationStayOnTop;
	bool ApplicationActivating;
	bool ApplicationNotMinimizing;

	bool EventEnable;

	DWORD LastCompactedTick;
	DWORD LastCloseClickedTick;
	DWORD LastShowModalWindowSentTick;
	DWORD LastRehashedTick;

	DWORD MixedIdleTick;

	TVPTimer SystemWatchTimer;
public:
	TTVPMainForm();

	void InvokeEvents();
	void CallDeliverAllEventsOnIdle();

	void BeginContinuousEvent();
	void EndContinuousEvent();

	void NotifyEventDelivered();

	void SetEventEnabled( bool b ) {
		EventEnable = b;
	}
	bool GetEventEnabled() const { return EventEnable; }

	//bool GetApplicationStayOnTop();
	//void SetApplicationStayOnTop( bool );

	//bool GetApplicationActivating() const { return ApplicationActivating; }
	//bool GetApplicationNotMinimizing() const { return ApplicationNotMinimizing; }

	bool ApplicationIdel();

private:
	void DeliverEvents();
	void SystemWatchTimerTimer();
};
extern TTVPMainForm *TVPMainForm;
extern bool TVPMainFormAlive;

#endif
