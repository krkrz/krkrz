#include "tjsCommHead.h"
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
#include "MainFormUnit.h"
//#include "PadFormUnit.h"
//#include "ConsoleFormUnit.h"
#include "EventIntf.h"
#include "MsgIntf.h"
#include "WindowFormUnit.h"
#include "SysInitIntf.h"
#include "SysInitImpl.h"
#include "ScriptMgnIntf.h"
//#include "WatchFormUnit.h"
#include "WindowIntf.h"
#include "WindowImpl.h"
//#include "HaltWarnFormUnit.h"
#include "StorageIntf.h"
#include "Random.h"
#include "EmergencyExit.h" // for TVPCPUClock
#include "DebugIntf.h"
//#include "FontSelectFormUnit.h"
//#include "HintWindow.h"
#include "VersionFormUnit.h"
#include "WaveImpl.h"
#include "SystemImpl.h"

#include "Application.h"

TTVPMainForm *TVPMainForm;
bool TVPMainFormAlive = false;

#define DELIVER_EVENTS_DUMMY_MSG (WM_USER+0x31)

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
	LastCloseClickedTick = 0;
	LastShowModalWindowSentTick = 0;
	LastRehashedTick = 0;
	
	// read previous state from environ profile
	/*
	tTVPProfileHolder *prof = TVPGetEnvironProfile();
	TVPEnvironProfileAddRef();
	*/
	
	TVPMainFormAlive = true;
}
void TTVPMainForm::InvokeEvents() {
	CallDeliverAllEventsOnIdle();
}
void TTVPMainForm::CallDeliverAllEventsOnIdle() {
	Application->PostMessageToMainWindow( DELIVER_EVENTS_DUMMY_MSG, 0, 0 );
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

void TTVPMainForm::SetVisible( bool b ) {}
bool TTVPMainForm::GetVisible() const { return false; }

bool TTVPMainForm::GetApplicationStayOnTop() { return false; }
void TTVPMainForm::SetApplicationStayOnTop( bool ) {}

void TTVPMainForm::NotifySystemError() {
	// if(AutoShowConsoleOnError) SetConsoleVisible(true);
}

bool TTVPMainForm::GetConsoleVisible() { return false; }
void TTVPMainForm::SetConsoleVisible( bool ) {}


bool TTVPMainForm::ApplicationIdel() {
	DeliverEvents();
	bool cont = !ContinuousEventCalling;
	MixedIdleTick += GetTickCount();
	return cont;
}

void TTVPMainForm::DeliverEvents() {
	if(ContinuousEventCalling)
		TVPProcessContinuousHandlerEventFlag = true; // set flag

	if(EventEnable) TVPDeliverAllEvents();
}

#if 0
bool InputQuery( const std::string& caption, const std::string& prompt, std::string& value ) {
}
#endif
