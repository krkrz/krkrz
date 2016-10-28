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

//---------------------------------------------------------------------------
// tTVPThread
//---------------------------------------------------------------------------
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>

class tTVPThread
{
protected:
	std::thread* Thread;
private:
	bool Terminated;

	std::mutex Mtx;
	std::condition_variable Cond;
	bool ThreadStarting;

	void StartProc();

public:
	tTVPThread() {}
	tTVPThread( bool suspended );
	virtual ~tTVPThread();

	bool GetTerminated() const { return Terminated; }
	void SetTerminated(bool s) { Terminated = s; }
	void Terminate() { Terminated = true; }

protected:
	virtual void Execute() {}

public:
	void StartTread();
	void WaitFor() { if (Thread && Thread->joinable()) { Thread->join(); } }

	tTVPThreadPriority GetPriority();
	void SetPriority(tTVPThreadPriority pri);

	std::thread::native_handle_type GetHandle() { if(Thread) return Thread->native_handle(); else return reinterpret_cast<std::thread::native_handle_type>(nullptr); }
	std::thread::id GetThreadId() { if(Thread) return Thread->get_id(); else return std::thread::id(); }
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// tTVPThreadEvent
//---------------------------------------------------------------------------
class tTVPThreadEvent
{
	std::mutex Mtx;
	std::condition_variable Cond;
	bool IsReady;

public:
	tTVPThreadEvent() : IsReady(false) {}
	virtual ~tTVPThreadEvent() {}

	void Set() {
		{
			std::lock_guard<std::mutex> lock(Mtx);
			IsReady = true;
		}
		Cond.notify_all();
	}
	/*
	void Reset() {
		std::lock_guard<std::mutex> lock(Mtx);
		IsReady = false;
	}
	*/
	bool WaitFor( tjs_uint timeout ) {
		std::unique_lock<std::mutex> lk( Mtx );
		if( timeout == 0 ) {
			Cond.wait( lk, [this]{ return IsReady;} );
			IsReady = false;
			return true;
		} else {
			//std::cv_status result = Cond.wait_for( lk, std::chrono::milliseconds( timeout ) );
			//return result == std::cv_status::no_timeout;
			bool result = Cond.wait_for( lk, std::chrono::milliseconds( timeout ), [this]{ return IsReady;} );
			IsReady = false;
			return result;
		}
	}
};
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
#endif
