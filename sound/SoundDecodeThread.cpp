#include "tjsCommHead.h"

#include "SoundDecodeThread.h"
#include "TickCount.h"
#include "Random.h"
#include "QueueSoundBufferImpl.h"


static tTVPThreadPriority TVPDecodeThreadHighPriority = ttpHigher;
static tTVPThreadPriority TVPDecodeThreadLowPriority = ttpLowest;
//---------------------------------------------------------------------------
tTVPSoundDecodeThread::tTVPSoundDecodeThread( tTJSNI_QueueSoundBuffer * owner)
{
	Owner = owner;
	SetPriority(TVPDecodeThreadHighPriority);
	Running = false;
	StartTread();
}
//---------------------------------------------------------------------------
tTVPSoundDecodeThread::~tTVPSoundDecodeThread()
{
	SetPriority(TVPDecodeThreadHighPriority);
	Running = false;
	Terminate();
	Event.Set();
	WaitFor();
}
//---------------------------------------------------------------------------
#define TVP_WSB_DECODE_THREAD_SLEEP_TIME 110
void tTVPSoundDecodeThread::Execute(void)
{
	while(!GetTerminated())
	{
		// decoder thread main loop
		tjs_uint32 st = TVPGetRoughTickCount32();
		while(Running)
		{
			bool wait;
			tjs_uint32 et;

			if(Running)
			{
				volatile tTJSCriticalSectionHolder cs_holder(OneLoopCS);
				wait = !Owner->DoDecode();	// FillL2Buffer(false, true); // fill
			}

			if(GetTerminated()) break;

			if(Running)
			{
				et = TVPGetRoughTickCount32();
				TVPPushEnvironNoise(&et, sizeof(et));
				if(wait)
				{
					// buffer is full; sleep longer
					tjs_uint32 elapsed = et -st;
					if(elapsed < TVP_WSB_DECODE_THREAD_SLEEP_TIME)
					{
						Event.WaitFor(
							TVP_WSB_DECODE_THREAD_SLEEP_TIME - elapsed);
					}
				}
				else
				{
					// buffer is not full; sleep shorter
					std::this_thread::yield();
					if(!GetTerminated()) SetPriority(TVPDecodeThreadLowPriority);
				}
				st = et;
			}
		}
		if(GetTerminated()) break;
		// sleep while running
		Event.WaitFor(0);	// INFINITE
	}
}
//---------------------------------------------------------------------------
void tTVPSoundDecodeThread::Interrupt()
{
	// interrupt the thread
	if(!Running) return;
	SetPriority(TVPDecodeThreadHighPriority);
	Event.Set();
	tTJSCriticalSectionHolder cs_holder(OneLoopCS);
		// this ensures that this function stops the decoding
	Running = false;
}
//---------------------------------------------------------------------------
void tTVPSoundDecodeThread::Continue()
{
	SetPriority(TVPDecodeThreadHighPriority);
	Running = true;
	Event.Set();
}
//---------------------------------------------------------------------------

