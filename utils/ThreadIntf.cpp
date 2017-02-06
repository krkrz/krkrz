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

#include "ThreadIntf.h"
#include "ThreadImpl.h"
#include "MsgIntf.h"

#ifdef _WIN32
#include <process.h>
#endif
#include <algorithm>
#include <assert.h>

#ifdef ANDROID
#include <pthread.h>
#include <semaphore.h>
#endif


//---------------------------------------------------------------------------
// tTVPThread : a wrapper class for thread
//---------------------------------------------------------------------------
tTVPThread::tTVPThread()
 : Thread(nullptr), Terminated(false), ThreadStarting(false)
{
}
//---------------------------------------------------------------------------
tTVPThread::~tTVPThread()
{
	if( Thread != nullptr ) {
		if( Thread->joinable() ) Thread->detach();
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
		} catch( std::system_error & ) {
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
static void TJS_USERENTRY DummyThreadTask( void * ) {}
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
	DrawThread( DrawThreadPool* p ) : parent( p ), lpStartAddress( nullptr ), lpParameter( nullptr ) {}
	void SetTask( TVP_THREAD_TASK_FUNC func, TVP_THREAD_PARAM param ) {
		std::lock_guard<std::mutex> lock( mtx );
		lpStartAddress = func;
		lpParameter = param;
		cv.notify_one();
	}
};
//---------------------------------------------------------------------------
class DrawThreadPool {
	std::vector<DrawThread*> workers;
#ifdef _WIN32
	std::vector<tjs_int> processor_ids;
#endif
	std::atomic<int> running_thread_count;
	tjs_int task_num;
	tjs_int task_count;
private:
	void PoolThread( tjs_int taskNum );

public:
	DrawThreadPool() : running_thread_count( 0 ), task_num( 0 ), task_count( 0 ) {}
	~DrawThreadPool() {
		for( auto i = workers.begin(); i != workers.end(); ++i ) {
			DrawThread *th = *i;
			th->Terminate();
			th->SetTask( DummyThreadTask, nullptr );
			th->WaitFor();
			delete th;
		}
	}
	inline void DecCount() { running_thread_count--; }
	void BeginTask( tjs_int taskNum ) {
		task_num = taskNum;
		task_count = 0;
		PoolThread( taskNum );
	}
	void ExecTask( TVP_THREAD_TASK_FUNC func, TVP_THREAD_PARAM param ) {
		if( task_count >= task_num - 1 ) {
			func( param );
			return;
		}
		running_thread_count++;
		DrawThread* thread = workers[task_count];
		task_count++;
		thread->SetTask( func, param );
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
	while( !GetTerminated() ) {
		{
			std::unique_lock<std::mutex> uniq_lk( mtx );
			cv.wait( uniq_lk, [this] { return lpStartAddress != nullptr; } );
		}
		if( lpStartAddress != nullptr ) ( lpStartAddress )( lpParameter );
		lpStartAddress = nullptr;
		parent->DecCount();
	}
}
//---------------------------------------------------------------------------
void DrawThreadPool::PoolThread( tjs_int taskNum ) {
	tjs_int extraThreadNum = TVPGetThreadNum() - 1;

#ifdef _WIN32
	if( processor_ids.empty() ) {
#ifndef TJS_64BIT_OS
		DWORD processAffinityMask, systemAffinityMask;
		::GetProcessAffinityMask( GetCurrentProcess(), &processAffinityMask, &systemAffinityMask );
		for( tjs_int i = 0; i < MAXIMUM_PROCESSORS; i++ ) {
			if( processAffinityMask & ( 1 << i ) )
				processor_ids.push_back( i );
		}
#else
		ULONGLONG processAffinityMask, systemAffinityMask;
		::GetProcessAffinityMask( GetCurrentProcess(), (PDWORD_PTR)&processAffinityMask, (PDWORD_PTR)&systemAffinityMask );
		for( tjs_int i = 0; i < MAXIMUM_PROCESSORS; i++ ) {
			if( processAffinityMask & ( 1ULL << i ) )
				processor_ids.push_back( i );
		}
#endif
		if( processor_ids.empty() )
			processor_ids.push_back( MAXIMUM_PROCESSORS );
	}
#endif

	// スレッド数がextraThreadNumに達していないので(suspend状態で)生成する
	while( (tjs_int)workers.size() < extraThreadNum ) {
		DrawThread* th = new DrawThread( this );
		th->StartTread();
#ifdef _WIN32
		::SetThreadIdealProcessor( th->GetHandle(), processor_ids[workers.size() % processor_ids.size()] );
#else !defined( ANDROID )
		// for pthread(!android)
		cpu_set_t cpuset;
		CPU_ZERO( &cpuset );
		CPU_SET( workers.size(), &cpuset );
		int rc = pthread_setaffinity_np( th->GetHandle(), sizeof( cpu_set_t ), &cpuset );
#endif
		workers.push_back( th );
	}
	// スレッド数が多い場合終了させる
	while( (tjs_int)workers.size() > extraThreadNum ) {
		DrawThread *th = workers.back();
		th->Terminate();
		running_thread_count++;
		th->SetTask( DummyThreadTask, nullptr );
		th->WaitFor();
		workers.pop_back();
		delete th;
	}
}
//---------------------------------------------------------------------------
static DrawThreadPool TVPTheadPool;
//---------------------------------------------------------------------------
void TVPBeginThreadTask( tjs_int taskNum ) {
	TVPTheadPool.BeginTask( taskNum );
}
//---------------------------------------------------------------------------
void TVPExecThreadTask( TVP_THREAD_TASK_FUNC func, TVP_THREAD_PARAM param ) {
	TVPTheadPool.ExecTask( func, param );
}
//---------------------------------------------------------------------------
void TVPEndThreadTask( void ) {
	TVPTheadPool.WaitForTask();
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------


