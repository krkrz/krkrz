//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Timer Object Implementation
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include <algorithm>
#include "EventIntf.h"
#include "TickCount.h"
#include "SysInitIntf.h"
#include "ThreadIntf.h"
#include "MsgIntf.h"
#include "DebugIntf.h"

#include "UserEvent.h"

#include "TimerThread.h"
#include "TimerIntf.h"

//---------------------------------------------------------------------------

// TVP Timer class gives ability of triggering event on punctual interval.
// a large quantity of event at once may easily cause freeze to system,
// so we must trigger only porocess-able quantity of the event.
#define TVP_LEAST_TIMER_INTERVAL 3

#define TVP_TIME_INFINITE 0

static tTVPTimerThread * TVPTimerThread = nullptr;
//---------------------------------------------------------------------------
tTVPTimerThread::tTVPTimerThread() : EventQueue(this,&tTVPTimerThread::Proc)
{
	PendingEventsAvailable = false;
	SetPriority(TVPLimitTimerCapacity ? ttpNormal : ttpHighest);
	EventQueue.Allocate();
	StartTread();
}
//---------------------------------------------------------------------------
tTVPTimerThread::~tTVPTimerThread()
{
	Terminate();
	Event.Set();
	WaitFor();
	EventQueue.Deallocate();
}
//---------------------------------------------------------------------------
void tTVPTimerThread::Execute()
{
	while(!GetTerminated())
	{
		tjs_uint64 step_next = (tjs_uint64)(tjs_int64)-1L; // invalid value
		tjs_uint64 curtick = TVPGetTickCount() << TVP_SUBMILLI_FRAC_BITS;
		tjs_uint sleeptime;

		{	// thread-protected
			tTJSCriticalSectionHolder holder(TVPTimerCS);

			bool any_triggered = false;

			std::vector<tTVPTimerBase*>::iterator i;
			for(i = List.begin(); i!=List.end(); i ++)
			{
				tTVPTimerBase * item = *i;

				if(!item->GetEnabled() || item->GetInterval() == 0) continue;

				if(item->GetNextTick() < curtick)
				{
					tjs_uint n = static_cast<tjs_uint>( (curtick - item->GetNextTick()) / item->GetInterval() );
					n++;
					if(n > 40)
					{
						// too large amount of event at once; discard rest
						item->Trigger(1);
						any_triggered = true;
						item->SetNextTick(curtick + item->GetInterval());
					}
					else
					{
						item->Trigger(n);
						any_triggered = true;
						item->SetNextTick(item->GetNextTick() +
							n * item->GetInterval());
					}
				}


				tjs_uint64 to_next = item->GetNextTick() - curtick;

				if(step_next == (tjs_uint64)(tjs_int64)-1L)
				{
					step_next = to_next;
				}
				else
				{
					if(step_next > to_next) step_next = to_next;
				}
			}


			if(step_next != (tjs_uint64)(tjs_int64)-1L)
			{
				// too large step_next must be diminished to size of tjs_uint.
				if(step_next >= 0x80000000)
					sleeptime = 0x7fffffff; // smaller value than step_next is OK
				else {
					if( step_next == 0 ) {
						step_next = 1;
					}
					sleeptime = static_cast<tjs_uint>( step_next );
				}
			}
			else
			{
				sleeptime = TVP_TIME_INFINITE;
			}

			if( List.size() == 0 ) {
				sleeptime = TVP_TIME_INFINITE;
			}

			if(any_triggered)
			{
				// triggered; post notification message to the UtilWindow
				if(!PendingEventsAvailable)
				{
					PendingEventsAvailable = true;
					EventQueue.PostEvent( NativeEvent(TVP_EV_TIMER_THREAD) );
				}
			}

		}	// end-of-thread-protected

		// now, sleeptime has sub-milliseconds precision but we need millisecond
		// precision time.
		if(sleeptime != TVP_TIME_INFINITE)
			sleeptime = (sleeptime >> TVP_SUBMILLI_FRAC_BITS) + (sleeptime & ((1<<TVP_SUBMILLI_FRAC_BITS)-1) ? 1: 0); // round up

		// clamp to TVP_LEAST_TIMER_INTERVAL ...
		if(sleeptime != TVP_TIME_INFINITE && sleeptime < TVP_LEAST_TIMER_INTERVAL)
			sleeptime = TVP_LEAST_TIMER_INTERVAL;

		Event.WaitFor(sleeptime); // wait until sleeptime is elapsed or
									// Event->SetEvent() is executed.
	}
}
//---------------------------------------------------------------------------
void tTVPTimerThread::Proc( NativeEvent& ev )
{
	// Window procedure of UtilWindow
	if( ev.Message == TVP_EV_TIMER_THREAD && !GetTerminated())
	{
		// pending events occur
		tTJSCriticalSectionHolder holder(TVPTimerCS); // protect the object

		ProcWork.reserve( Pending.size() );
		ProcWork = Pending;
		Pending.clear();
		for( auto i = ProcWork.begin(); i != ProcWork.end(); i++ ) {
			if( std::find( List.begin(), List.end(), ( *i ) ) != List.end() )
				(*i)->FirePendingEventsAndClear();	// この呼び出しによってList/Peinding内から削除されるケースがありうるので注意。
		}
		ProcWork.clear();
		PendingEventsAvailable = false;
	}
	else
	{
		EventQueue.HandlerDefault(ev);
	}
}
//---------------------------------------------------------------------------
void tTVPTimerThread::AddItem(tTVPTimerBase * item)
{
	tTJSCriticalSectionHolder holder(TVPTimerCS);

	if(std::find(List.begin(), List.end(), item) == List.end())
		List.push_back(item);
}
//---------------------------------------------------------------------------
bool tTVPTimerThread::RemoveItem(tTVPTimerBase *item)
{
	tTJSCriticalSectionHolder holder(TVPTimerCS);

	// remove from the List
	for( auto i = List.begin(); i != List.end(); /**/)
	{
		if(*i == item) i = List.erase(i); else i++;
	}

	// also remove from the Pending list
	RemoveFromPendingItem(item);

	return List.size() != 0;
}
//---------------------------------------------------------------------------
void tTVPTimerThread::RemoveFromPendingItem(tTVPTimerBase *item)
{
	// remove item from pending list
	for( auto i = Pending.begin(); i != Pending.end(); /**/)
	{
		if(*i == item) i = Pending.erase(i); else i++;
	}
	item->ZeroPendingCount();
}
//---------------------------------------------------------------------------
void tTVPTimerThread::RegisterToPendingItem(tTVPTimerBase *item)
{
	// register item to the pending list
	Pending.push_back(item);
}
//---------------------------------------------------------------------------
void tTVPTimerThread::SetEnabled(tTVPTimerBase *item, bool enabled)
{
	{ // thread-protected
		tTJSCriticalSectionHolder holder(TVPTimerCS);

		item->InternalSetEnabled(enabled);
		if(enabled)
		{
			item->SetNextTick((TVPGetTickCount()  << TVP_SUBMILLI_FRAC_BITS) + item->GetInterval());
		}
		else
		{
			item->CancelEvents();
			item->ZeroPendingCount();
		}
	} // end-of-thread-protected

	if(enabled) Event.Set();
}
//---------------------------------------------------------------------------
void tTVPTimerThread::SetInterval(tTVPTimerBase *item, tjs_uint64 interval)
{
	{ // thread-protected
		tTJSCriticalSectionHolder holder(TVPTimerCS);

		item->InternalSetInterval(interval);
		if(item->GetEnabled())
		{
			item->CancelEvents();
			item->ZeroPendingCount();
			item->SetNextTick((TVPGetTickCount()  << TVP_SUBMILLI_FRAC_BITS) + item->GetInterval());
		}
	} // end-of-thread-protected

	if(item->GetEnabled()) Event.Set();

}
//---------------------------------------------------------------------------
void tTVPTimerThread::Init()
{
	if(!TVPTimerThread)
	{
		TVPStartTickCount(); // in TickCount.cpp
		TVPTimerThread = new tTVPTimerThread();
	}
}
//---------------------------------------------------------------------------
void tTVPTimerThread::Uninit()
{
	if(TVPTimerThread)
	{
		delete TVPTimerThread;
		TVPTimerThread = nullptr;
	}
}
//---------------------------------------------------------------------------
static tTVPAtExit TVPTimerThreadUninitAtExit(TVP_ATEXIT_PRI_SHUTDOWN,
	tTVPTimerThread::Uninit);
//---------------------------------------------------------------------------
void tTVPTimerThread::Add(tTVPTimerBase * item)
{
	// at this point, item->GetEnebled() must be false.

	Init();

	TVPTimerThread->AddItem(item);
}
//---------------------------------------------------------------------------
void tTVPTimerThread::Remove(tTVPTimerBase *item)
{
	if(TVPTimerThread)
	{
		if(!TVPTimerThread->RemoveItem(item)) Uninit();
	}
}
//---------------------------------------------------------------------------
void tTVPTimerThread::RemoveFromPending(tTVPTimerBase *item)
{
	if(TVPTimerThread)
	{
		TVPTimerThread->RemoveFromPendingItem(item);
	}
}
//---------------------------------------------------------------------------
void tTVPTimerThread::RegisterToPending(tTVPTimerBase *item)
{
	if(TVPTimerThread)
	{
		TVPTimerThread->RegisterToPendingItem(item);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tTVPTimerBase::tTVPTimerBase()
 : NextTick(0), Interval(1000 << TVP_SUBMILLI_FRAC_BITS ), PendingCount(0), Enabled(false)
{
}
//---------------------------------------------------------------------------
void tTVPTimerBase::SetEnabled(bool b)
{
	TVPTimerThread->SetEnabled(this, b);
}
//---------------------------------------------------------------------------
void tTVPTimerBase::SetInterval(tjs_uint64 n)
{
	TVPTimerThread->SetInterval(this, n);
}
//---------------------------------------------------------------------------
void tTVPTimerBase::Trigger(tjs_uint n)
{
	// this function is called by sub-thread.
	if(PendingCount == 0) tTVPTimerThread::RegisterToPending(this);
	PendingCount += n;
}
//---------------------------------------------------------------------------
void tTVPTimerBase::FirePendingEventsAndClear()
{
	// fire all pending events and clear the pending event count
	if(PendingCount)
	{
		Fire(PendingCount);
		ZeroPendingCount();
	}
}

