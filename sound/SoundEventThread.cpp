
#include "tjsCommHead.h"

#include <algorithm>
#include "SoundEventThread.h"
#include "QueueSoundBufferImpl.h"
#include "TickCount.h"
#include "Random.h"
#include "UserEvent.h"

//---------------------------------------------------------------------------
tTVPSoundEventThread::tTVPSoundEventThread( tTVPSoundBuffers* parent )
	: EventQueue(this,&tTVPSoundEventThread::UtilWndProc),
	Buffers( parent ),
	SuspendThread( false ), PendingLabelEventExists( false ),
	NextLabelEventTick( 0 ), LastFilledTick( 0 ), WndProcToBeCalled( false )
{
	EventQueue.Allocate();
	SetPriority(ttpHighest);
	StartTread();
}
//---------------------------------------------------------------------------
tTVPSoundEventThread::~tTVPSoundEventThread()
{
	SetPriority(ttpNormal);
	Terminate();
	ResetSuspend();
	Event.Set();
	WaitFor();
	EventQueue.Deallocate();
}
//---------------------------------------------------------------------------
void tTVPSoundEventThread::UtilWndProc( NativeEvent& ev )
{
	// Window procedure of UtilWindow
	if( ev.Message == TVP_EV_WAVE_SND_BUF_THREAD && !GetTerminated())
	{
		// pending events occur
		tTJSCriticalSectionHolder holder(Buffers->GetCriticalSection()); // protect the object

		WndProcToBeCalled = false;

		tjs_int64 tick = TVPGetTickCount();

		int nearest_next = TVP_TIMEOFS_INVALID_VALUE;

		for( auto i = Buffers->Begin(); i != Buffers->End(); i++)
		{
			int next = (*i)->FireLabelEventsAndGetNearestLabelEventStep(tick);
				// fire label events and get nearest label event step
			if(next != TVP_TIMEOFS_INVALID_VALUE)
			{
				if(nearest_next == TVP_TIMEOFS_INVALID_VALUE || nearest_next > next)
					nearest_next = next;
			}
		}

		if(nearest_next != TVP_TIMEOFS_INVALID_VALUE)
		{
			PendingLabelEventExists = true;
			NextLabelEventTick = TVPGetRoughTickCount32() + nearest_next;
		}
		else
		{
			PendingLabelEventExists = false;
		}
	}
	else
	{
		EventQueue.HandlerDefault(ev);
	}
}
//---------------------------------------------------------------------------
void tTVPSoundEventThread::ReschedulePendingLabelEvent(tjs_int tick)
{
	if(tick == TVP_TIMEOFS_INVALID_VALUE) return; // no need to reschedule
	tjs_uint32 eventtick = TVPGetRoughTickCount32() + tick;

	tTJSCriticalSectionHolder holder(Buffers->GetCriticalSection());

	if(PendingLabelEventExists)
	{
		if((tjs_int32)NextLabelEventTick - (tjs_int32)eventtick > 0)
			NextLabelEventTick = eventtick;
	}
	else
	{
		PendingLabelEventExists = true;
		NextLabelEventTick = eventtick;
	}
}
//---------------------------------------------------------------------------
#define TVP_WSB_THREAD_SLEEP_TIME 60
void tTVPSoundEventThread::Execute(void)
{
	while(!GetTerminated())
	{
		// thread loop for playing thread
		tjs_uint32 time = TVPGetRoughTickCount32();
		TVPPushEnvironNoise(&time, sizeof(time));

		{	// thread-protected
			tTJSCriticalSectionHolder holder(Buffers->GetCriticalSection());

			// check PendingLabelEventExists
			if(PendingLabelEventExists)
			{
				if(!WndProcToBeCalled)
				{
					WndProcToBeCalled = true;
					EventQueue.PostEvent( NativeEvent(TVP_EV_WAVE_SND_BUF_THREAD) );
				}
			}

			if(time - LastFilledTick >= TVP_WSB_THREAD_SLEEP_TIME)
			{
				for(auto i = Buffers->Begin(); i != Buffers->End(); i++)
				{
					if((*i)->ThreadCallbackEnabled)
						(*i)->Update();
				}
				LastFilledTick = time;
			}
		}	// end-of-thread-protected

		tjs_uint32 time2;
		time2 = TVPGetRoughTickCount32();
		time = time2 - time;

		if(time < TVP_WSB_THREAD_SLEEP_TIME)
		{
			tjs_int sleep_time = TVP_WSB_THREAD_SLEEP_TIME - time;
			if(PendingLabelEventExists)
			{
				tjs_int step_to_next = (tjs_int32)NextLabelEventTick - (tjs_int32)time2;
				if(step_to_next < sleep_time)
					sleep_time = step_to_next;
				if(sleep_time < 1) sleep_time = 1;
			}
			Event.WaitFor(sleep_time);
		}
		else
		{
			Event.WaitFor(1);
		}
		if( !GetTerminated() ) {
			bool suspendrequest = false;
			{
				std::lock_guard<std::mutex> lock( SuspendMutex );
				suspendrequest = SuspendThread;
			}
			if( suspendrequest ) {
				Event.WaitFor( 0 );	// infinity
				ResetSuspend();
			}
		}
	}
}
//---------------------------------------------------------------------------
void tTVPSoundEventThread::Start()
{
	Event.Set();
}
//---------------------------------------------------------------------------
void tTVPSoundEventThread::CheckBufferSleep()
{
	tTJSCriticalSectionHolder holder(Buffers->GetCriticalSection());
	tjs_uint size, nonwork_count;
	nonwork_count = 0;
	size = (tjs_uint)Buffers->Size();
	for(auto i = Buffers->Begin(); i != Buffers->End(); i++)
	{
		if(!(*i)->ThreadCallbackEnabled)
			nonwork_count ++;
	}
	if(nonwork_count == size)
	{
		SetSuspend(); // all buffers are sleeping...
		// TVPStopPrimaryBuffer();
	}
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

void tTVPSoundBuffers::ReleaseBuffers(bool disableevent) {
	tTJSCriticalSectionHolder holder(BufferCS);
	for( auto i = Buffers.begin(); i != Buffers.end(); ++i ) {
		(*i)->ReleaseSoundBuffer(disableevent);
	}
}
//---------------------------------------------------------------------------
void tTVPSoundBuffers::Shutdown() {
	if( EventThread )
		delete EventThread, EventThread = nullptr;
	ReleaseBuffers();
}
//---------------------------------------------------------------------------
void tTVPSoundBuffers::EnsureBufferWorking() {
	if( EventThread == nullptr ) {
		EventThread = new tTVPSoundEventThread(this);
	}
	EventThread->Start();
}
//---------------------------------------------------------------------------
void tTVPSoundBuffers::CheckAllSleep() {
	if( EventThread ) EventThread->CheckBufferSleep();
}
//---------------------------------------------------------------------------
void tTVPSoundBuffers::AddBuffer( tTJSNI_QueueSoundBuffer * buffer ) {
	tTJSCriticalSectionHolder holder(BufferCS);
	Buffers.push_back(buffer);
}
//---------------------------------------------------------------------------
void tTVPSoundBuffers::RemoveBuffer( tTJSNI_QueueSoundBuffer * buffer) {
	bool bufferempty;
	{
		tTJSCriticalSectionHolder holder(BufferCS);
		auto i = std::find(Buffers.begin(), Buffers.end(), buffer);
		if(i != Buffers.end())
			Buffers.erase(i);
		bufferempty = Buffers.size() == 0;
	}
	if(bufferempty) {
		if(EventThread)
			delete EventThread, EventThread = nullptr;
	}
}
//---------------------------------------------------------------------------
void tTVPSoundBuffers::ReschedulePendingLabelEvent(tjs_int tick) {
	if(EventThread)
		EventThread->ReschedulePendingLabelEvent(tick);
}
//---------------------------------------------------------------------------
void tTVPSoundBuffers::ResetVolumeToAllSoundBuffer() {
	// call each SoundBuffer's SetVolumeToSoundBuffer
	tTJSCriticalSectionHolder holder(BufferCS);
	for( auto i = Buffers.begin(); i != Buffers.end(); i++) {
		(*i)->SetVolumeToStream();
	}
}
//---------------------------------------------------------------------------

