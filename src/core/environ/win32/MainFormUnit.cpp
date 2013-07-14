#include "tjsCommHead.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MainFormUnit.h"
#include "EventIntf.h"
#include "MsgIntf.h"
#include "WindowFormUnit.h"
#include "SysInitIntf.h"
#include "SysInitImpl.h"
#include "ScriptMgnIntf.h"
#include "WindowIntf.h"
#include "WindowImpl.h"
#include "StorageIntf.h"
#include "EmergencyExit.h" // for TVPCPUClock
#include "DebugIntf.h"
#include "VersionFormUnit.h"
#include "WaveImpl.h"
#include "SystemImpl.h"
#include "UserEvent.h"
#include "Application.h"
#include "TickCount.h"
#include "Random.h"

TTVPMainForm *TVPMainForm;
bool TVPMainFormAlive = false;

//---------------------------------------------------------------------------
// Get whether to control main thread priority or to insert wait
//---------------------------------------------------------------------------
static bool TVPMainThreadPriorityControlInit = false;
static bool TVPMainThreadPriorityControl = false;
static bool TVPGetMainThreadPriorityControl()
{
	if(TVPMainThreadPriorityControlInit) return TVPMainThreadPriorityControl;
	tTJSVariant val;
	if( TVPGetCommandLine(TJS_W("-lowpri"), &val) )
	{
		ttstr str(val);
		if(str == TJS_W("yes"))
			TVPMainThreadPriorityControl = true;
	}

	TVPMainThreadPriorityControlInit = true;

	return TVPMainThreadPriorityControl;
}


TTVPMainForm::TTVPMainForm() {
	ContinuousEventCalling = false;
	AutoShowConsoleOnError = false;
	ApplicationStayOnTop = false;
	ApplicationActivating = true;
	ApplicationNotMinimizing = true;

	LastCompactedTick = 0;
	LastCloseClickedTick = 0;
	LastShowModalWindowSentTick = 0;
	LastRehashedTick = 0;

	TVPMainFormAlive = true;

	SystemWatchTimer.SetInterval(50);
	SystemWatchTimer.SetOnTimerHandler( this, &TTVPMainForm::SystemWatchTimerTimer );
	SystemWatchTimer.SetEnabled( true );
}
void TTVPMainForm::InvokeEvents() {
	CallDeliverAllEventsOnIdle();
}
void TTVPMainForm::CallDeliverAllEventsOnIdle() {
	Application->PostMessageToMainWindow( TVP_EV_DELIVER_EVENTS_DUMMY, 0, 0 );
}

void TTVPMainForm::BeginContinuousEvent() {
	if(!ContinuousEventCalling)
	{
		ContinuousEventCalling = true;
		InvokeEvents();
		if(TVPGetMainThreadPriorityControl())
		{
			// make main thread priority lower
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
		}
	}
}
void TTVPMainForm::EndContinuousEvent() {
	if(ContinuousEventCalling)
	{
		ContinuousEventCalling = false;
		if(TVPGetMainThreadPriorityControl())
		{
			// make main thread priority normal
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
		}
	}
}

void TTVPMainForm::NotifyEventDelivered() {
	// called from event system, notifying the event is delivered.
	LastCloseClickedTick = 0;
	// if(TVPHaltWarnForm) delete TVPHaltWarnForm, TVPHaltWarnForm = NULL;
}
/*
bool TTVPMainForm::GetApplicationStayOnTop() { return false; }
void TTVPMainForm::SetApplicationStayOnTop( bool ) {}
*/
bool TTVPMainForm::ApplicationIdel() {
	DeliverEvents();
	bool cont = !ContinuousEventCalling;
	MixedIdleTick += TVPGetRoughTickCount32();
	return cont;
}

void TTVPMainForm::DeliverEvents() {
	if(ContinuousEventCalling)
		TVPProcessContinuousHandlerEventFlag = true; // set flag

	if(EventEnable) TVPDeliverAllEvents();
}

void TTVPMainForm::SystemWatchTimerTimer() {
	if( TVPTerminated ) {
		// this will ensure terminating the application.
		// the WM_QUIT message disappears in some unknown situations...
		Application->PostMessageToMainWindow( TVP_EV_DELIVER_EVENTS_DUMMY, 0, 0 );
		Application->Terminate();
		Application->PostMessageToMainWindow( TVP_EV_DELIVER_EVENTS_DUMMY, 0, 0 );
	}

	// call events
	DWORD tick = TVPGetRoughTickCount32();

	// push environ noise
	TVPPushEnvironNoise(&tick, sizeof(tick));
	TVPPushEnvironNoise(&LastCompactedTick, sizeof(LastCompactedTick));
	TVPPushEnvironNoise(&LastShowModalWindowSentTick, sizeof(LastShowModalWindowSentTick));
	TVPPushEnvironNoise(&MixedIdleTick, sizeof(MixedIdleTick));
	POINT pt;
	::GetCursorPos(&pt);
	TVPPushEnvironNoise(&pt, sizeof(pt));

	// CPU clock monitoring
	{
		static bool clock_rough_printed = false;
		if( !clock_rough_printed && TVPCPUClockAccuracy == ccaRough ) {
			tjs_char msg[80];
			TJS_sprintf(msg, TJS_W("(info) CPU clock (roughly) : %dMHz"), (int)TVPCPUClock);
			TVPAddImportantLog(msg);
			clock_rough_printed = true;
		}
		static bool clock_printed = false;
		if( !clock_printed && TVPCPUClockAccuracy == ccaAccurate ) {
			tjs_char msg[80];
			TJS_sprintf(msg, TJS_W("(info) CPU clock : %.1fMHz"), (float)TVPCPUClock);
			TVPAddImportantLog(msg);
			clock_printed = true;
		}
	}

	// check status and deliver events
	DeliverEvents();

	// call TickBeat
	tjs_int count = TVPGetWindowCount();
	for( tjs_int i = 0; i<count; i++ ) {
		tTJSNI_Window *win = TVPGetWindowListAt(i);
		win->TickBeat();
	}

	if( !ContinuousEventCalling && tick - LastCompactedTick > 4000 ) {
		// idle state over 4 sec.
		LastCompactedTick = tick;

		// fire compact event
		TVPDeliverCompactEvent(TVP_COMPACT_LEVEL_IDLE);
	}

	if( !ContinuousEventCalling && tick > LastRehashedTick + 1500 ) {
		// TJS2 object rehash
		LastRehashedTick = tick;
		TJSDoRehash();
	}

	// ensure modal window visible
	if( tick > LastShowModalWindowSentTick + 4100 ) {
		//	::PostMessage(Handle, WM_USER+0x32, 0, 0);
		// This is currently disabled because IME composition window
		// hides behind the window which is bringed top by the
		// window-rearrangement.
		LastShowModalWindowSentTick = tick;
	}
}

#if 0
bool InputQuery( const std::string& caption, const std::string& prompt, std::string& value ) {
}
#endif
