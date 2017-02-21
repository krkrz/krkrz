

#ifndef __SOUND_DECODE_THREAD_H__
#define __SOUND_DECODE_THREAD_H__

#include "ThreadIntf.h"

class tTJSNI_QueueSoundBuffer;
class tTVPSoundDecodeThread : public tTVPThread
{
	tTJSNI_QueueSoundBuffer * Owner;
	tTVPThreadEvent Event;
	tTJSCriticalSection OneLoopCS;
	volatile bool Running;

public:
	tTVPSoundDecodeThread(tTJSNI_QueueSoundBuffer * owner);
	~tTVPSoundDecodeThread();

	void Execute(void);

	void Interrupt();
	void Continue();

	bool GetRunning() const { return Running; }
};


#endif // __SOUND_DECODE_THREAD_H__