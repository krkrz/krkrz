//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Thread base class
//---------------------------------------------------------------------------
#ifndef ThreadImplH
#define ThreadImplH
#include "tjsNative.h"
#include "ThreadIntf.h"

#include <pthread.h>
#include <semaphore.h>

//---------------------------------------------------------------------------
// tTVPThread
//---------------------------------------------------------------------------
class tTVPThread
{
	bool Terminated;
	pthread_t Handle;
	//uint32_t threadId;
	bool Suspended;
	sem_t SuspendSemaphore;

	static void* StartProc(void * arg);

public:
	tTVPThread(bool suspended);
	virtual ~tTVPThread();

	bool GetTerminated() const { return Terminated; }
	void SetTerminated(bool s) { Terminated = s; }
	void Terminate() { Terminated = true; }

protected:
	virtual void Execute() = 0;

public:
	void WaitFor();

	tTVPThreadPriority GetPriority();
	void SetPriority(tTVPThreadPriority pri);

	//void Suspend();
	void Resume();
	// Suspend はサポートしないので、Event(semaphore)を使って待ち合わせする

#ifdef WIN32
	HANDLE GetHandle() const { return Handle; } 	/* win32 specific */
	DWORD GetThreadId() const { return ThreadId; }  /* win32 specific */
#endif
};
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// tTVPThreadEvent
//---------------------------------------------------------------------------
class tTVPThreadEvent
{
#ifdef WIN32
	HANDLE Handle;
#else	// pthread
	sem_t Handle;
#endif

public:
	tTVPThreadEvent(bool manualreset = false);
	virtual ~tTVPThreadEvent();

	void Set();
	void Reset();
	bool WaitFor(tjs_uint timeout);
};
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
#endif
