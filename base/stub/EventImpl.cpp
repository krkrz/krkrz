//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Script Event Handling and Dispatching
// TVPSystemControl と Application、各種ユーティリティクラスによって抽象化され
// ているので、このソースコードはプラットフォーム共通として扱える。
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "EventImpl.h"

#include "SystemControl.h"
#include "ThreadIntf.h"
#include "TickCount.h"
#include "TimerIntf.h"
#include "SysInitIntf.h"
#include "DebugIntf.h"
#include "Application.h"
#include "NativeEventQueue.h"
#include "UserEvent.h"

//---------------------------------------------------------------------------
// TVPInvokeEvents for EventIntf.cpp
// 各プラットフォームで実装する。
// アプリケーションの準備が出来ている時、イベントの配信を確実にするために必要
// 吉里吉里のイベントがPostされた後にコールされる。
// Windowsではメッセージディスパッチャでイベントをハンドリングするために
// ダミーメッセージをメインウィンドウへ送っている(::PostMessage)
//---------------------------------------------------------------------------
bool TVPEventInvoked = false;
void TVPInvokeEvents()
{
	if(TVPEventInvoked) return;
	TVPEventInvoked = true;
	// Windowsではダミーメッセージをメインウィンドウへ送っている
	if(TVPSystemControl) TVPSystemControl->InvokeEvents();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// TVPEventReceived for EventIntf.cpp
// 各プラットフォームで実装する。
// 吉里吉里のイベント配信後、次のイベントの準備のために呼び出される。
// Windowsでは特に何もしていない
//---------------------------------------------------------------------------
void TVPEventReceived()
{
	TVPEventInvoked = false;
	// Windowsではカウンタを初期化しているが、その値は未使用(将来削除)
	if( TVPSystemControl ) TVPSystemControl->NotifyEventDelivered();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// TVPCallDeliverAllEventsOnIdle for EventIntf.cpp
// 各プラットフォームで実装する。
// 一度OSに制御を戻し、その後TVPInvokeEvents（）を呼び出すように設定。
// Windowsではダミーメッセージをメインウィンドウへ送っている(::PostMessage)
//---------------------------------------------------------------------------
void TVPCallDeliverAllEventsOnIdle()
{
	// Windowsではダミーメッセージをメインウィンドウへ送っている
	if( TVPSystemControl ) TVPSystemControl->CallDeliverAllEventsOnIdle();
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPBreathe
// implement this in each platform
// to handle OS's message events
// this should be called when in a long time processing, something like a
// network access, to reply to user's Windowing message, repainting, etc...
// in TVPBreathe, TVP events must not be invoked. ( happened events over the
// long time processing are pending until next timing of message delivering. )
// 各プラットフォームで実装する。
// 長時間メインスレッドを止めてしまう時にGUI等が固まらないように定期的に呼び出す
// プラグイン用に提供されている
//---------------------------------------------------------------------------
static bool TVPBreathing = false;
void TVPBreathe()
{
	TVPEventDisabled = true; // not to call TVP events...
	TVPBreathing = true;
	try
	{
		// Windowsではメッセージポンプを回す
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
// breathing処理中かどうか
//---------------------------------------------------------------------------
bool TVPGetBreathing()
{
	// return whether now is in event breathing
	return TVPBreathing;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPSystemEventDisabledState
// sets whether system overall event handling is enabled.
// this works distinctly from TVPEventDisabled.
// イベントハンドリングの有効/無効を設定する
//---------------------------------------------------------------------------
void TVPSetSystemEventDisabledState(bool en)
{
	TVPSystemControl->SetEventEnabled( !en );
	if(!en) TVPDeliverAllEvents();
}
//---------------------------------------------------------------------------
bool TVPGetSystemEventDisabledState()
{
	return !TVPSystemControl->GetEventEnabled();
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// tTVPContinuousHandlerCallLimitThread
//---------------------------------------------------------------------------
class tTVPContinuousHandlerCallLimitThread : public tTVPThread
{
	tjs_uint64 NextEventTick;
	tjs_uint64 Interval;
	tTVPThreadEvent Event;
	tTJSCriticalSection CS;

	bool Enabled;

	NativeEventQueue<tTVPContinuousHandlerCallLimitThread> EventQueue;

public:
	tTVPContinuousHandlerCallLimitThread();
	~tTVPContinuousHandlerCallLimitThread();

protected:
	void Execute();

	void WndProc(NativeEvent& ev) {
		EventQueue.HandlerDefault(ev);
	}

public:
	void SetEnabled(bool enabled);

	void SetInterval(tjs_uint64 interval) { Interval = interval; }


} static * TVPContinuousHandlerCallLimitThread = NULL;
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tTVPContinuousHandlerCallLimitThread::tTVPContinuousHandlerCallLimitThread()
	 : tTVPThread(true), EventQueue(this,&tTVPContinuousHandlerCallLimitThread::WndProc)
{
	NextEventTick = 0;
	Interval = (1<<TVP_SUBMILLI_FRAC_BITS)*1000/60; // default 60Hz
	Enabled = false;
	EventQueue.Allocate();
	Resume();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tTVPContinuousHandlerCallLimitThread::~tTVPContinuousHandlerCallLimitThread()
{
	Terminate();
	Resume();
	Event.Set();
	WaitFor();
	EventQueue.Deallocate();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tTVPContinuousHandlerCallLimitThread::Execute()
{
	while(!GetTerminated())
	{
		tjs_uint64 curtick = TVPGetTickCount() << TVP_SUBMILLI_FRAC_BITS;
		DWORD sleeptime;

		{	// thread-protected
			tTJSCriticalSectionHolder holder(CS);

			if(Enabled)
			{
				if(NextEventTick <= curtick)
				{
					TVPProcessContinuousHandlerEventFlag = true; // set flag to process event on next idle
					EventQueue.PostEvent( NativeEvent(TVP_EV_CONTINUE_LIMIT_THREAD) );
					while(NextEventTick <= curtick) NextEventTick += Interval;
				}
				tjs_uint64 sleeptime_64 = NextEventTick - curtick;
				sleeptime = (DWORD)(sleeptime_64 >> TVP_SUBMILLI_FRAC_BITS) +
						((sleeptime_64 & ((1<<TVP_SUBMILLI_FRAC_BITS)-1))?1:0);
							// add 1 if fraction exists
			}
			else
			{
				sleeptime = 10000; // how long to sleep when disabled does not matter
			}


		}	// end-of-thread-protected

		if(sleeptime == 0) sleeptime = 1; // 0 will let thread sleeping forever ...
		Event.WaitFor(sleeptime);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tTVPContinuousHandlerCallLimitThread::SetEnabled(bool enabled)
{
	tTJSCriticalSectionHolder holder(CS);

	Enabled = enabled;
	if(enabled)
	{
		tjs_uint64 curtick = TVPGetTickCount() << TVP_SUBMILLI_FRAC_BITS;
		NextEventTick = ((curtick + 1) / Interval) * Interval;
		Event.Set();
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
static tjs_int TVPContinousHandlerLimitFrequency = 0;
//---------------------------------------------------------------------------
void TVPBeginContinuousEvent()
{
	// read commandline options
	static tjs_int ArgumentGeneration = 0;
	if(ArgumentGeneration != TVPGetCommandLineArgumentGeneration())
	{
		ArgumentGeneration = TVPGetCommandLineArgumentGeneration();

		tTJSVariant val;
		if( TVPGetCommandLine(TJS_W("-contfreq"), &val) )
		{
			TVPContinousHandlerLimitFrequency = (tjs_int)val;
		}
	}

	if(!TVPIsWaitVSync())
	{
		if(TVPContinousHandlerLimitFrequency == 0)
		{
			// no limit
			// this notifies continuous calling of TVPDeliverAllEvents.
			if(TVPSystemControl) TVPSystemControl->BeginContinuousEvent();
		}
		else
		{
			// has limit
			if(!TVPContinuousHandlerCallLimitThread)
				TVPContinuousHandlerCallLimitThread = new tTVPContinuousHandlerCallLimitThread();
			TVPContinuousHandlerCallLimitThread->SetInterval(
				 (1<<TVP_SUBMILLI_FRAC_BITS)*1000/TVPContinousHandlerLimitFrequency);
			TVPContinuousHandlerCallLimitThread->SetEnabled(true);
		}
	}


	// TVPEnsureVSyncTimingThread();
	// if we wait vsync, the continuous handler will be executed at the every timing of
	// vsync.
}
//---------------------------------------------------------------------------
void TVPEndContinuousEvent()
{
	// anyway
	if(TVPContinuousHandlerCallLimitThread)
		TVPContinuousHandlerCallLimitThread->SetEnabled(false);

	// anyway
	if(TVPSystemControl) TVPSystemControl->EndContinuousEvent();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
static void TVPReleaseContinuousHandlerCallLimitThread()
{
	if(TVPContinuousHandlerCallLimitThread)
		delete TVPContinuousHandlerCallLimitThread,
			TVPContinuousHandlerCallLimitThread = NULL;
}
// to release TVPContinuousHandlerCallLimitThread at exit
static tTVPAtExit TVPTimerThreadUninitAtExit(TVP_ATEXIT_PRI_SHUTDOWN,
	TVPReleaseContinuousHandlerCallLimitThread);
//---------------------------------------------------------------------------
