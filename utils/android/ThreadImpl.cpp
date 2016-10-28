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

#ifdef _WIN32
#include <process.h>
#endif
#include <algorithm>
#include <assert.h>

#include "ThreadIntf.h"
#include "ThreadImpl.h"
#include "MsgIntf.h"

#ifdef ANDROID
#include <pthread.h>
#include <semaphore.h>
#endif

//---------------------------------------------------------------------------
// tTVPThread : a wrapper class for thread
//---------------------------------------------------------------------------
tTVPThread::tTVPThread( bool suspended )
 : Thread(nullptr), Terminated(false), ThreadStarting(false)
{
	assert( suspended );
}
//---------------------------------------------------------------------------
tTVPThread::~tTVPThread()
{
	if( Thread != nullptr ) {
		Thread->detach();
		delete Thread;
	}
}
//---------------------------------------------------------------------------
void tTVPThread::StartProc()
{
	{	// スレッドが開始されたのでフラグON
		std::lock_guard<std::mutex> lock( Mtx );
		ThreadStarting = true;
	}
	Cond.notify_all();
	Execute();
	// return 0;
}
//---------------------------------------------------------------------------
void tTVPThread::StartTread()
{
	if( Thread == nullptr ) {
		try {
			Thread = new std::thread( &tTVPThread::StartProc, this );

			// スレッドが開始されるのを待つ
			std::unique_lock<std::mutex> lock( Mtx );
			Cond.wait( lock, [this] { return ThreadStarting; } );
		} catch( std::system_error &e ) {
			TVPThrowInternalError;
		}
	}
}
//---------------------------------------------------------------------------
tTVPThreadPriority tTVPThread::GetPriority()
{
#ifdef _WIN32
	int n = ::GetThreadPriority( GetHandle());
	switch(n)
	{
	case THREAD_PRIORITY_IDLE:			return ttpIdle;
	case THREAD_PRIORITY_LOWEST:		return ttpLowest;
	case THREAD_PRIORITY_BELOW_NORMAL:	return ttpLower;
	case THREAD_PRIORITY_NORMAL:		return ttpNormal;
	case THREAD_PRIORITY_ABOVE_NORMAL:	return ttpHigher;
	case THREAD_PRIORITY_HIGHEST:		return ttpHighest;
	case THREAD_PRIORITY_TIME_CRITICAL:	return ttpTimeCritical;
	}

	return ttpNormal;
#else
	int policy;
	struct sched_param param;
	int err = pthread_getschedparam( GetHandle(), &policy, &param );
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
#endif
}
//---------------------------------------------------------------------------
void tTVPThread::SetPriority(tTVPThreadPriority pri)
{
#ifdef _WIN32
	int npri = THREAD_PRIORITY_NORMAL;
	switch(pri)
	{
	case ttpIdle:			npri = THREAD_PRIORITY_IDLE;			break;
	case ttpLowest:			npri = THREAD_PRIORITY_LOWEST;			break;
	case ttpLower:			npri = THREAD_PRIORITY_BELOW_NORMAL;	break;
	case ttpNormal:			npri = THREAD_PRIORITY_NORMAL;			break;
	case ttpHigher:			npri = THREAD_PRIORITY_ABOVE_NORMAL;	break;
	case ttpHighest:		npri = THREAD_PRIORITY_HIGHEST;			break;
	case ttpTimeCritical:	npri = THREAD_PRIORITY_TIME_CRITICAL;	break;
	}

	::SetThreadPriority( GetHandle(), npri);
#else
	int policy;
	struct sched_param param;
	int err = pthread_getschedparam( GetHandle(), &policy, &param );
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
	err = pthread_setschedparam( GetHandle(), policy, &param );
#endif
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
#ifndef _WIN32
tjs_int TVPDrawThreadNum = 1;
//---------------------------------------------------------------------------
tjs_int TVPGetProcessorNum( void )
{
	return std::thread::hardware_concurrency();
}
//---------------------------------------------------------------------------
tjs_int TVPGetThreadNum( void )
{
	tjs_int threadNum = TVPDrawThreadNum ? TVPDrawThreadNum : std::thread::hardware_concurrency();
	threadNum = std::min( threadNum, TVPMaxThreadNum );
	return threadNum;
}
//---------------------------------------------------------------------------
static void TJS_USERENTRY DummyThreadTask(void *) {}
//---------------------------------------------------------------------------
class DrawThreadPool;
class DrawThread : public tTVPThread {
	std::mutex mtx;
	std::condition_variable cv;
	TVP_THREAD_TASK_FUNC  lpStartAddress;
	TVP_THREAD_PARAM lpParameter;
	DrawThreadPool* parent;
protected:
	virtual void Execute();

public:
	DrawThread( DrawThreadPool* p ) : parent(p), lpStartAddress(nullptr), lpParameter(nullptr)  {}
	void SetTask( TVP_THREAD_TASK_FUNC func, TVP_THREAD_PARAM param ) {
		std::lock_guard<std::mutex> lock(mtx);
		lpStartAddress = func;
		lpParameter = param;
		cv.notify_one();
	}
};
//---------------------------------------------------------------------------
class DrawThreadPool {
	std::vector<DrawThread*> workers;
	std::atomic<int> running_thread_count;
	tjs_int task_num;
	tjs_int task_count;
private:
	void PoolThread( tjs_int taskNum );

public:
	DrawThreadPool() : running_thread_count(0), task_num(0), task_count(0) {}
	inline void DecCount() { running_thread_count--; }
	void BeginTask( tjs_int taskNum ) {
		task_num = taskNum;
		task_count = 0;
		PoolThread( taskNum );
	}
	void ExecTask(TVP_THREAD_TASK_FUNC func, TVP_THREAD_PARAM param) {
		if( task_count >= task_num - 1 ) {
			func( param );
			return;
		}
		running_thread_count++;
		workers[task_count]->SetTask( func, param );
		std::this_thread::yield();
	}
	void WaitForTask() {
		int expected = 0;
		while( false == std::atomic_compare_exchange_weak( &running_thread_count, &expected, 0 ) ) {
			expected = 0;
			std::this_thread::yield();	// スレッド切り替え(再スケジューリング)
		}
	}
};
//---------------------------------------------------------------------------
void DrawThread::Execute() {
	while(!GetTerminated()) {
		{
			std::unique_lock<std::mutex> uniq_lk(mtx);
			cv.wait(uniq_lk, [this]{ return lpStartAddress != nullptr;});
		}
		if( lpStartAddress != nullptr ) (lpStartAddress)( lpParameter );
		lpStartAddress = nullptr;
		parent->DecCount();
	}
}
//---------------------------------------------------------------------------
void DrawThreadPool::PoolThread( tjs_int taskNum ) {
	tjs_int extraThreadNum = TVPGetThreadNum() - 1;

/*
	// for pthread(!android)
	cpu_set_t cpuset;
	CPU_ZERO( &cpuset );
	CPU_SET( i, &cpuset );
	int rc = pthread_setaffinity_np( workers[i].native_handle(), sizeof( cpu_set_t ), &cpuset );
*/
	// スレッド数がextraThreadNumに達していないので(suspend状態で)生成する
	while( workers.size() < extraThreadNum ) {
		DrawThread* th = new DrawThread(this);
		th->StartTread();
		workers.push_back( th );
	}
	// スレッド数が多い場合終了させる
	while( workers.size() > extraThreadNum ) {
		DrawThread *th = workers.back();
		th->Terminate();
		th->SetTask( DummyThreadTask, nullptr );
		th->WaitFor();
		workers.pop_back();
	}
}
//---------------------------------------------------------------------------
static DrawThreadPool TVPTheadPool;
//---------------------------------------------------------------------------
void TVPBeginThreadTask(tjs_int taskNum) {
	TVPTheadPool.BeginTask( taskNum );
}
//---------------------------------------------------------------------------
void TVPExecThreadTask(TVP_THREAD_TASK_FUNC func, TVP_THREAD_PARAM param) {
	TVPTheadPool.ExecTask( func, param );
}
//---------------------------------------------------------------------------
void TVPEndThreadTask(void) {
	TVPTheadPool.WaitForTask();
}
//---------------------------------------------------------------------------

#if 0
tjs_int TVPDrawThreadNum = 1;
struct ThreadInfo {
	bool readyToExit;
	std::thread* thread;
	TVP_THREAD_TASK_FUNC  lpStartAddress;
	TVP_THREAD_PARAM lpParameter;
};
static std::vector<ThreadInfo*> TVPThreadList;
static std::vector<tjs_int> TVPProcesserIdList;
//static tjs_int TVPRunningThreadCount = 0;
static std::atomic_long TVPRunningThreadCount{0};
static tjs_int TVPThreadTaskNum, TVPThreadTaskCount;
//---------------------------------------------------------------------------
tjs_int TVPGetProcessorNum( void )
{
	return std::thread::hardware_concurrency();
}
//---------------------------------------------------------------------------
tjs_int TVPGetThreadNum( void )
{
	tjs_int threadNum = TVPDrawThreadNum ? TVPDrawThreadNum : std::thread::hardware_concurrency();
	threadNum = std::min( threadNum, TVPMaxThreadNum );
	return threadNum;
}
//---------------------------------------------------------------------------
static void ThreadLoop( void* p )
{
	ThreadInfo *threadInfo = (ThreadInfo*)p;
	for( ;;) {
		if( threadInfo->readyToExit )
			break;
		( threadInfo->lpStartAddress )( threadInfo->lpParameter );
		TVPRunningThreadCount--;
		SuspendThread( GetCurrentThread() );
	}
	delete threadInfo;
	ExitThread( 0 );
}
//---------------------------------------------------------------------------
void TVPBeginThreadTask( tjs_int taskNum )
{
	TVPThreadTaskNum = taskNum;
	TVPThreadTaskCount = 0;
	tjs_int extraThreadNum = TVPGetThreadNum() - 1;
	if( TVPProcesserIdList.empty() ) {
#ifdef _WIN32
		// for windows thread
#ifndef TJS_64BIT_OS
		DWORD processAffinityMask, systemAffinityMask;
		GetProcessAffinityMask( GetCurrentProcess(),
			&processAffinityMask,
			&systemAffinityMask );
		for( tjs_int i = 0; i < MAXIMUM_PROCESSORS; i++ ) {
			if( processAffinityMask & ( 1 << i ) )
				TVPProcesserIdList.push_back( i );
		}
#else
		ULONGLONG processAffinityMask, systemAffinityMask;
		GetProcessAffinityMask( GetCurrentProcess(),
			(PDWORD_PTR)&processAffinityMask,
			(PDWORD_PTR)&systemAffinityMask );
		for( tjs_int i = 0; i < MAXIMUM_PROCESSORS; i++ ) {
			if( processAffinityMask & ( 1ULL << i ) )
				TVPProcesserIdList.push_back( i );
		}
#endif
#else defined(!ANDROID)
		// for pthread(!android)
		cpu_set_t cpuset;
		CPU_ZERO( &cpuset );
		CPU_SET( i, &cpuset );
		int rc = pthread_setaffinity_np( threads[i].native_handle(),
			sizeof( cpu_set_t ), &cpuset );
#endif
		if( TVPProcesserIdList.empty() )
			TVPProcesserIdList.push_back( MAXIMUM_PROCESSORS );
	}
	// スレッド数がextraThreadNumに達していないので(suspend状態で)生成する
	while( static_cast<tjs_int>( TVPThreadList.size() ) < extraThreadNum ) {
		ThreadInfo *threadInfo = new ThreadInfo();
		threadInfo->readyToExit = false;
		threadInfo->thread = CreateThread( NULL, 0, ThreadLoop, threadInfo, CREATE_SUSPENDED, NULL );
		SetThreadIdealProcessor( threadInfo->thread, TVPProcesserIdList[TVPThreadList.size() % TVPProcesserIdList.size()] );
		TVPThreadList.push_back( threadInfo );
	}
	// extraThreadNum よりスレッド数が多い場合終了させる
	while( static_cast<tjs_int>( TVPThreadList.size() ) > extraThreadNum ) {
		ThreadInfo *threadInfo = TVPThreadList.back();
		threadInfo->readyToExit = true;
		while( ResumeThread( threadInfo->thread ) == 0 )
			Sleep( 0 );
		TVPThreadList.pop_back();
	}
}

//---------------------------------------------------------------------------
void TVPExecThreadTask( TVP_THREAD_TASK_FUNC func, TVP_THREAD_PARAM param )
{
	if( TVPThreadTaskCount >= TVPThreadTaskNum - 1 ) {
		func( param );
		return;
	}
	ThreadInfo *threadInfo;
	threadInfo = TVPThreadList[TVPThreadTaskCount++];
	threadInfo->lpStartAddress = func;
	threadInfo->lpParameter = param;
	TVPRunningThreadCount++;
	while( ResumeThread( threadInfo->thread ) == 0 )
		Sleep( 0 );
}
//---------------------------------------------------------------------------
void TVPEndThreadTask( void )
{
	long expected = 0;
	while( false == std::atomic_compare_exchange_weak( &TVPRunningThreadCount, &expected, 0 ) ) {
		expected = 0;
		std::this_thread::yield();	// スレッド切り替え(再スケジューリング)
	}
}

//---------------------------------------------------------------------------
#endif
#else
tjs_int TVPDrawThreadNum = 1;

struct ThreadInfo {
  bool readyToExit;
  HANDLE thread;
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
#ifndef TJS_64BIT_OS
    DWORD processAffinityMask, systemAffinityMask;
    GetProcessAffinityMask(GetCurrentProcess(),
                           &processAffinityMask,
                           &systemAffinityMask);
    for (tjs_int i = 0; i < MAXIMUM_PROCESSORS; i++) {
      if (processAffinityMask & (1 << i))
        TVPProcesserIdList.push_back(i);
    }
#else
    ULONGLONG processAffinityMask, systemAffinityMask;
    GetProcessAffinityMask(GetCurrentProcess(),
                           (PDWORD_PTR)&processAffinityMask,
                           (PDWORD_PTR)&systemAffinityMask);
    for (tjs_int i = 0; i < MAXIMUM_PROCESSORS; i++) {
      if (processAffinityMask & (1ULL << i))
        TVPProcesserIdList.push_back(i);
    }
#endif
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
