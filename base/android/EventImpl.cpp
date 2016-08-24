//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Script Event Handling and Dispatching
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "EventImpl.h"
#include "SystemControl.h"
#include "ThreadIntf.h"
#include "TickCount.h"
#include "TimerIntf.h"
#include "SysInitIntf.h"
#include "DebugIntf.h"
#include "WindowImpl.h"
#include <mmsystem.h>

#include "Application.h"
#include "NativeEventQueue.h"
#include "UserEvent.h"

//---------------------------------------------------------------------------
// TVPInvokeEvents
//---------------------------------------------------------------------------
void TVPInvokeEvents()
{
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// TVPEventReceived
//---------------------------------------------------------------------------
void TVPEventReceived()
{
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPCallDeliverAllEventsOnIdle
//---------------------------------------------------------------------------
void TVPCallDeliverAllEventsOnIdle()
{
	if(TVPSystemControl)
	{
		TVPSystemControl->CallDeliverAllEventsOnIdle();
	}
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPBreathe
//---------------------------------------------------------------------------
static bool TVPBreathing = false;
void TVPBreathe()
{
	TVPEventDisabled = true; // not to call TVP events...
	TVPBreathing = true;
	try
	{
		Application->ProcessMessages(); // do Windows message pumping
	}
	catch(...)
	{
		TVPBreathing = false;
		TVPEventDisabled = false;
		throw;
	}

	TVPBreathing = false;
	TVPEventDisabled = false;
}
//---------------------------------------------------------------------------
bool TVPGetBreathing()
{
	// return whether now is in event breathing
	return TVPBreathing;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPSystemEventDisabledState
//---------------------------------------------------------------------------
void TVPSetSystemEventDisabledState(bool en)
{
}
//---------------------------------------------------------------------------
bool TVPGetSystemEventDisabledState()
{
	return false;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void TVPBeginContinuousEvent()
{
}
//---------------------------------------------------------------------------
void TVPEndContinuousEvent()
{
}
//---------------------------------------------------------------------------
// TVPDeliverWindowUpdateEvents
// TVPDumpHWException
// TVPGetCommandLine(
// TVPCreateNativeClass_System
// TVPCreateNativeClass_Storages
// TVPCreateNativeClass_Plugins
// TVPCreateNativeClass_Clipboard
// TVPTerminateSync
// TVPShellExecute
// TVPPreNormalizeStorageName

