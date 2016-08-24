//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Thread base class
//---------------------------------------------------------------------------
#define NOMINMAX
#include "tjsCommHead.h"

//#include <process.h>
//#include <algorithm>
//#include <unistd.h>
#include <errno.h>
#include <cpu-features.h>

#include "ThreadIntf.h"
#include "ThreadImpl.h"
#include "MsgIntf.h"



// pause/resume
// http://stackoverflow.com/questions/1606400/how-to-sleep-or-pause-a-pthread-in-c-on-linux

//---------------------------------------------------------------------------
// tTVPThread : a wrapper class for thread
//---------------------------------------------------------------------------
tTVPThread::tTVPThread(bool suspended)
{
	Terminated = false;
	//Handle = NULL;
	//ThreadId = 0;
	Suspended = suspended;

	if( suspended ) {
		int err = sem_init( &SuspendSemaphore, 0, 0 );
		if( err != 0 ) TVPThrowInternalError;
	}

	pthread_attr_t threadAttr;
	pthread_attr_init(&threadAttr);
	threadAttr.sched_policy = SCHED_RR;
	int maxpri = sched_get_priority_max( SCHED_RR );
	int minpri = sched_get_priority_min( SCHED_RR );
	threadAttr.sched_priority = minpri + (maxpri - minpri) / 2;	// 真ん中にしておく

	int err = pthread_create( &Handle, &threadAttr, tTVPThread::StartProc, (void*)this );
	if( err != 0 ) TVPThrowInternalError;
}
//---------------------------------------------------------------------------
tTVPThread::~tTVPThread()
{
	if( Suspended ) {
		sem_destroy( &SuspendSemaphore );
	}
	pthread_detach(Handle);
}
//---------------------------------------------------------------------------
void* tTVPThread::StartProc(void * arg)
{
	tTVPThread* thread = ((tTVPThread*)arg);
	if( thread->Suspended ) {
		sem_wait( &(thread->SuspendSemaphore) );
	}
	thread->Execute();
	return 0;
}
//---------------------------------------------------------------------------
void tTVPThread::WaitFor()
{	// 終了待ち
	pthread_join( Handle, NULL );
}
//---------------------------------------------------------------------------
tTVPThreadPriority tTVPThread::GetPriority()
{
	int policy;
	struct sched_param param;
	int err = pthread_getschedparam( Handle, &policy, &param );
	int maxpri = sched_get_priority_max( policy );
	int minpri = sched_get_priority_min( policy );
	int range = (maxpri - minpri);
	int half = range / 2;
	int pri = param.sched_priority;
	if( pri == minpri ) {
		return ttpIdle;
	} else if( pri == maxpri ) {
		return ttpTimeCritical;
	} else {
		pri -= minpri;
		if( pri == half ) {
			return ttpNormal;
		}
		if( pri < half ) {
			if( pri <= (half/3) ) {
				return ttpLowest;
			} else {
				return ttpLower;
			}
		} else {
			pri -= half;
			if( pri <= (half/3) ) {
				return ttpHigher;
			} else {
				return ttpHighest;
			}
		}
	}
	return ttpNormal;
}
//---------------------------------------------------------------------------
void tTVPThread::SetPriority(tTVPThreadPriority pri)
{
	int policy;
	struct sched_param param;
	int err = pthread_getschedparam( Handle, &policy, &param );
	int maxpri = sched_get_priority_max( policy );
	int minpri = sched_get_priority_min( policy );
	int range = (maxpri - minpri);
	int half = range / 2;

	param.sched_priority = minpri + half;
	switch(pri)
	{
	case ttpIdle:			param.sched_priority = minpri;						break;
	case ttpLowest:			param.sched_priority = minpri + half/3;				break;
	case ttpLower:			param.sched_priority = minpri + (half*2)/3;			break;
	case ttpNormal:			param.sched_priority = minpri + half;				break;
	case ttpHigher:			param.sched_priority = minpri + half + half/3;		break;
	case ttpHighest:		param.sched_priority = minpri + half+ (half*2)/3;	break;
	case ttpTimeCritical:	param.sched_priority = maxpri;						break;
	}
	err = pthread_setschedparam( Handle, policy, &param );
}
//---------------------------------------------------------------------------
	/*
void tTVPThread::Suspend()
{
	SuspendThread(Handle);
}
	*/
//---------------------------------------------------------------------------
void tTVPThread::Resume()
{
	if( Suspended ) {
		sem_post( &SuspendSemaphore );
	}
}
//---------------------------------------------------------------------------
/*
// krkrz のコア、各種機能はここからすべてたどれるようにする
tTVPEngine {
}
// エンジンを動作させるためのドライバ、ポーリングや各種
tTVPEngineDriver {
}
*/
//
#if 0
// Mutex は cond とともに使う様子
    pthread_cond_t cond;
class PThreadMutex {
	pthread_mutex_t Mutex;

public:
	PThreadMutex() {
		// pthread_mutexattr_t Attr; アトリビュートは汎用的ではないよう
		pthread_mutex_init( &Mutex, NULL );
	}
	~PThreadMutex() {
		pthread_mutex_destroy( &Mutex );
	}
	void Lock() {
		pthread_mutex_lock( &Mutex );
	}
	void Unlock() {
		pthread_mutex_unlock( &Mutex );
	}
};

// Win32 のようにはちょっと使いづらい?
// シグナルを使うので、スレッドで実装されたTJS2用タイマーを共用した方が良さそう。
// タイマーとイベント(メッセージ)は独自実装のものを使った方がよさそうだな
// http://umezawa.dyndns.info/wordpress/?p=3174
class PThreadTimer {
	timer_t TimerId;
	struct timer_t   TimerSpec;
	struct sigaction    SignalAction;

	static void SignalHander( int signo ) {
		/*
		if( event_ ) {
			event_->Handle();
		}
		*/
	}
public:
	PThreadTimer() {
		// シグナル初期化
		SignalAction.sa_handler = SignalHander;
		SignalAction.sa_flags = 0;
		sigemptyset( &SignalAction.sa_mask );
		int err = sigaction( SIGALRM, &SignalAction, NULL );
		if( err != 0 ) TVPThrowInternalError;

		// タイマー生成
		err = timer_create( CLOCK_REALTIME, NULL, &TimerId );
		if( err != 0 ) TVPThrowInternalError;
	}
	~PThreadTimer() {
		timer_delete( TimerId );
	}
	// msec ミリ秒
	int SetInterval( tjs_uint32 msec ) {
		struct itimerspec spec;
		tjs_uint32 sec = msec / 1000;
		msec -= sec*1000;
		spec.it_interval.tv_sec = sec;
		spec.it_interval.tv_nsec = msec*1000*1000;
		spec.it_value.tv_sec = sec;
		spec.it_value.tv_nsec = msec*1000*1000;
		return timer_settime( TimerSpec, 0, &spec, NULL );
	}
	int Oneshot( tjs_uint32 msec ) {
		struct itimerspec spec;
		tjs_uint32 sec = msec / 1000;
		msec -= sec*1000;
		spec.it_interval.tv_sec = sec;
		spec.it_interval.tv_nsec = msec*1000*1000;
		spec.it_value.tv_sec = 0;
		spec.it_value.tv_nsec = 0;
		return timer_settime( TimerSpec, 0, &spec, NULL );
	}
	void Pause() {
		// pause(0); // シグナル待ち？
	}
};
#endif
// http://www.tsoftware.jp/nptl/
// pthread では、イベントはセマフォを使う様子








//---------------------------------------------------------------------------
// tTVPThreadEvent
//---------------------------------------------------------------------------
tTVPThreadEvent::tTVPThreadEvent(bool manualreset)
{
#ifdef WIN32
	Handle = ::CreateEvent(NULL, manualreset?TRUE:FALSE, FALSE, NULL);
	if(!Handle) TVPThrowInternalError;
#else
	if( manualreset ) TVPThrowInternalError; // マニュアルリセットは非サポート
	int err = sem_init( &Handle, 0, 0 );
	if( err != 0 ) TVPThrowInternalError;
#endif
}
//---------------------------------------------------------------------------
tTVPThreadEvent::~tTVPThreadEvent()
{
#ifdef WIN32
	::CloseHandle(Handle);
#else
	sem_destroy( &Handle );
#endif
}
//---------------------------------------------------------------------------
void tTVPThreadEvent::Set()
{
#ifdef WIN32
	SetEvent(Handle);
#else
	int err = sem_post( &Handle );
	if( err != 0 ) TVPThrowInternalError;
#endif
}
//---------------------------------------------------------------------------
void tTVPThreadEvent::Reset()
{
#ifdef WIN32
	ResetEvent(Handle);
#else
	int val = 0;
	int err = sem_getvalue( &Handle, &val );
	if( err == 0 && val > 0 ) {
		for( int i = 0; i < val; i++ ) {
			err = sem_trywait( &Handle );
			if( err != 0 ) break;
		}
	}
#endif
}
//---------------------------------------------------------------------------
bool tTVPThreadEvent::WaitFor(tjs_uint timeout)
{
#ifdef WIN32
	// wait for event;
	// returns true if the event is set, otherwise (when timed out) returns false.
	DWORD state = WaitForSingleObject(Handle, timeout == 0 ? INFINITE : timeout);
	if(state == WAIT_OBJECT_0) return true;
	return false;
#else
	if( timeout == 0 ) {
		sem_wait( &Handle );
	} else {
		struct timespec spec;
		memset(&spec, 0x00, sizeof(spec));
		clock_gettime(CLOCK_REALTIME, &spec); // コンパイルに-lrt必要
		tjs_uint sec = timeout / 1000;
		spec.tv_sec += sec;
		spec.tv_nsec += (timeout - (sec*1000)) * 1000 * 1000;	// nanoseconds
		int err = sem_timedwait( &Handle, &spec );
		return (err != ETIMEDOUT );
	}
	return true;
#endif
}
//---------------------------------------------------------------------------

// シングルスレッドで動作させる
#if 1
tjs_int TVPDrawThreadNum = 1;
tjs_int TVPGetProcessorNum() { return 1; }
tjs_int TVPGetThreadNum() { return 1; }
void TVPBeginThreadTask(tjs_int taskNum) {}
void TVPExecThreadTask(TVP_THREAD_TASK_FUNC func, TVP_THREAD_PARAM param) {
	func( param );
}
void TVPEndThreadTask() {}
#else
// Android NDKで使えないシステムコール・ライブラリ関数一覧 
// http://dsas.blog.klab.org/archives/52155870.html
// sched_setaffinity(2) は使えない
tjs_int TVPDrawThreadNum = 1;
struct ThreadInfo {
	bool readyToExit;
	pthread_t thread;
	pthread_cond_t cond;
	pthread_mutex_t mtx;
	TVP_THREAD_TASK_FUNC  lpStartAddress;
	TVP_THREAD_PARAM lpParameter;
};
tjs_int TVPGetProcessorNum() { return android_getCpuCount(); }
tjs_int TVPGetThreadNum() {
	tjs_int threadNum = TVPDrawThreadNum ? TVPDrawThreadNum : GetProcesserNum();
	threadNum = std::min(threadNum, TVPMaxThreadNum);
	return threadNum;
}
static bool ThreadLoop(void *p) {
	ThreadInfo *threadInfo = (ThreadInfo*)p;

	// http://linuxgcc.sytes.net/sys019.php
	// http://www.principia-m.com/ts/0037/index-jp.html
	pthread_mutex_lock(&p->mtx);
	pthread_cond_wait( &p->cond, &p->mtx );
	pthread_mutex_unlock(&p->mtx);

	for(;;) {
		if( threadInfo->readyToExit ) break;
		(threadInfo->lpStartAddress)(threadInfo->lpParameter);
		InterlockedDecrement(&TVPRunningThreadCount);
		SuspendThread(GetCurrentThread());
	}
	delete threadInfo;
	ExitThread(0);
	return true;
}
void TVPBeginThreadTask(tjs_int taskNum) {
	TVPThreadTaskNum = taskNum;
	TVPThreadTaskCount = 0;
	tjs_int extraThreadNum = TVPGetThreadNum() - 1;
	if( TVPProcesserIdList.empty() ) {
		for( tjs_int i = 0; i < MAXIMUM_PROCESSORS; i++ ) {
			TVPProcesserIdList.push_back(i);
    	}
	}
	while( static_cast<tjs_int>(TVPThreadList.size()) < extraThreadNum) {
		ThreadInfo *threadInfo = new ThreadInfo();
		threadInfo->readyToExit = false;
		if( pthread_mutex_init( &threadInfo->mtx, NULL ) != 0 ) TVPThrowInternalError;
		if( pthread_cond_init( &threadInfo->cond, NULL ) != 0 ) TVPThrowInternalError;
		if( pthread_create( &threadInfo->thread, NULL, ThreadLoop, threadInfo ) != 0 ) TVPThrowInternalError;
		TVPThreadList.push_back(threadInfo);
	}
  while ( static_cast<tjs_int>(TVPThreadList.size()) > extraThreadNum) {
    ThreadInfo *threadInfo = TVPThreadList.back();
    threadInfo->readyToExit = true;
    while (ResumeThread(threadInfo->thread) == 0)
      Sleep(0);
    TVPThreadList.pop_back();
  }
}

#endif
#if 0
//---------------------------------------------------------------------------
tjs_int TVPDrawThreadNum = 1;

struct ThreadInfo {
  bool readyToExit;
   thread;
  TVP_THREAD_TASK_FUNC  lpStartAddress;
  TVP_THREAD_PARAM lpParameter;
};
static std::vector<ThreadInfo*> TVPThreadList;
static std::vector<tjs_int> TVPProcesserIdList;
static LONG TVPRunningThreadCount = 0;
static tjs_int TVPThreadTaskNum, TVPThreadTaskCount;

//---------------------------------------------------------------------------
static tjs_int GetProcesserNum(void)
{
  static tjs_int processor_num = 0;
  if (! processor_num) {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    processor_num = info.dwNumberOfProcessors;
  }
  return processor_num;
}

tjs_int TVPGetProcessorNum(void)
{
  return GetProcesserNum();
}

//---------------------------------------------------------------------------
tjs_int TVPGetThreadNum(void)
{
  tjs_int threadNum = TVPDrawThreadNum ? TVPDrawThreadNum : GetProcesserNum();
  threadNum = std::min(threadNum, TVPMaxThreadNum);
  return threadNum;
}

//---------------------------------------------------------------------------
static DWORD WINAPI ThreadLoop(LPVOID p)
{
  ThreadInfo *threadInfo = (ThreadInfo*)p;
  for(;;) {
    if (threadInfo->readyToExit)
      break;
    (threadInfo->lpStartAddress)(threadInfo->lpParameter);
    InterlockedDecrement(&TVPRunningThreadCount);
    SuspendThread(GetCurrentThread());
  }
  delete threadInfo;
  ExitThread(0);

  return TRUE;
}
//---------------------------------------------------------------------------
void TVPBeginThreadTask(tjs_int taskNum)
{
  TVPThreadTaskNum = taskNum;
  TVPThreadTaskCount = 0;
  tjs_int extraThreadNum = TVPGetThreadNum() - 1;
  if (TVPProcesserIdList.empty()) {
    DWORD processAffinityMask, systemAffinityMask;
    GetProcessAffinityMask(GetCurrentProcess(),
                           &processAffinityMask,
                           &systemAffinityMask);
    for (tjs_int i = 0; i < MAXIMUM_PROCESSORS; i++) {
      if (processAffinityMask & (1 << i))
        TVPProcesserIdList.push_back(i);
    }
    if (TVPProcesserIdList.empty())
      TVPProcesserIdList.push_back(MAXIMUM_PROCESSORS);
  }
  while ( static_cast<tjs_int>(TVPThreadList.size()) < extraThreadNum) {
    ThreadInfo *threadInfo = new ThreadInfo();
    threadInfo->readyToExit = false;
    threadInfo->thread = CreateThread(NULL, 0, ThreadLoop, threadInfo, CREATE_SUSPENDED, NULL);
    SetThreadIdealProcessor(threadInfo->thread, TVPProcesserIdList[TVPThreadList.size() % TVPProcesserIdList.size()]);
    TVPThreadList.push_back(threadInfo);
  }
  while ( static_cast<tjs_int>(TVPThreadList.size()) > extraThreadNum) {
    ThreadInfo *threadInfo = TVPThreadList.back();
    threadInfo->readyToExit = true;
    while (ResumeThread(threadInfo->thread) == 0)
      Sleep(0);
    TVPThreadList.pop_back();
  }
}

//---------------------------------------------------------------------------
void TVPExecThreadTask(TVP_THREAD_TASK_FUNC func, TVP_THREAD_PARAM param)
{
  if (TVPThreadTaskCount >= TVPThreadTaskNum - 1) {
    func(param);
    return;
  }    
  ThreadInfo *threadInfo;
  threadInfo = TVPThreadList[TVPThreadTaskCount++];
  threadInfo->lpStartAddress = func;
  threadInfo->lpParameter = param;
  InterlockedIncrement(&TVPRunningThreadCount);
  while (ResumeThread(threadInfo->thread) == 0)
    Sleep(0);
}
//---------------------------------------------------------------------------
void TVPEndThreadTask(void) 
{
  while ((LONG)InterlockedCompareExchange(&TVPRunningThreadCount, 0, 0) != 0)
    Sleep(0);
}

//---------------------------------------------------------------------------
#endif
	