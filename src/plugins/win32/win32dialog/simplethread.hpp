#ifndef _simplethread_hpp_
#define _simplethread_hpp_

#include <process.h>
#include <vector>

template <class T>
class SimpleThreadBase {
public:
	typedef std::vector<HANDLE> EventArray;

	struct ThreadStartParam {
		SimpleThreadBase *self;
		HANDLE prepare, stop;
		T param;
	};

	SimpleThreadBase(DWORD tout)
		: threadHandle(0), prepareEvent(0), stopEvent(0), threadTimeout(tout)
	{}
	virtual ~SimpleThreadBase() {
		threadStop(threadTimeout);
		closeAllEvent();
	}

	void threadStart(T t) {
		if (!prepareEvent) prepareEvent = createEvent();
		if (!stopEvent)    stopEvent    = createEvent(true);

		threadStop(threadTimeout);
		::ResetEvent(stopEvent);

		ThreadStartParam param = { this, prepareEvent, stopEvent, t };
		threadHandle = (HANDLE)_beginthreadex(NULL, 0, &threadFunc, &param, 0, NULL);
		if (threadHandle) {
			HANDLE handles[] = { prepareEvent, threadHandle };
			::WaitForMultipleObjects(2, handles, FALSE, INFINITE);
			if (::WaitForSingleObject(threadHandle, 0) == WAIT_OBJECT_0) {
				::CloseHandle(threadHandle);
				threadHandle = 0;
			}
		}
	}
	void threadStop(DWORD timeout = INFINITE) {
		if (threadHandle) {
			::SetEvent(stopEvent);
			::WaitForSingleObject(threadHandle, timeout);
			::CloseHandle(threadHandle);
		}
		threadHandle = 0;
	}

protected:
	HANDLE threadHandle, prepareEvent, stopEvent;
	DWORD threadTimeout;
	EventArray events;

	virtual unsigned threadMain(HANDLE prepare, HANDLE stop, T param) = 0;

	HANDLE createEvent(bool b = false) {
		HANDLE ev = ::CreateEvent(NULL, b ? TRUE : FALSE, FALSE, NULL);
		events.push_back(ev);
		return ev;
	}
	void closeEvent(HANDLE ev) {
		if (ev) {
			::CloseHandle(ev);
			events.remove(ev);
		}
	}


private:
	static unsigned __stdcall threadFunc(void *vparam) {
		ThreadStartParam *param = (ThreadStartParam*)vparam;
		unsigned retval = param->self->threadMain(param->prepare, param->stop, param->param);
		_endthreadex(retval);
		return retval;
	}

	void closeAllEvent() {
		for (EventArray::iterator i = events.begin(), end = events.end(); i != end; i++) {
			::CloseHandle(*i);
		}
		events.clear();
	}
};

template <class T>
class SimpleThreadWithMessageWindow : public SimpleThreadBase<T> {
public:
	typedef SimpleThreadBase<T> InheritedClass;
	
	SimpleThreadWithMessageWindow(ttstr const &cname, ttstr const &wname, DWORD tout)
		: InheritedClass(tout), messageWindow(0)
	{
		messageWindow = createMessageWindow(cname.c_str(), wname.c_str());
	}

	virtual ~SimpleThreadWithMessageWindow()
	{
		messageWindow = destroyMessageWindow(messageWindow);
		//UnregisterMessageWindowClass();
	}

	static void UnregisterMessageWindowClass() {
		ATOM atom = MessageWindowClass();
		if (atom != 0) ::UnregisterClassW((LPCWSTR)MAKELONG(atom, 0), ::GetModuleHandle(NULL));
		MessageWindowClass(0, true);
	}

protected:
	HWND messageWindow;

	virtual LRESULT onMessage(UINT msg, WPARAM wp, LPARAM lp) = 0;

	void postMessage(UINT msg, WPARAM wp, LPARAM lp) {
		if (messageWindow) ::PostMessage(messageWindow, msg, wp, lp);
	}
	void sendMessage(UINT msg, WPARAM wp, LPARAM lp) {
		if (messageWindow) ::SendMessage(messageWindow, msg, wp, lp);
	}

private:
	static ATOM MessageWindowClass(ATOM set = 0, bool force = false) {
		static ATOM atom = 0;
		if (set || force) atom = set;
		return atom;
	}

	static LRESULT WINAPI MsgWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
		if (msg >= WM_APP && msg < 0xC000) {
			SimpleThreadBase *self = (SimpleThreadBase*)(::GetWindowLong(hwnd, GWL_USERDATA));
			if (self) return self->onMessage(msg, wp, lp);
		}
		return DefWindowProc(hwnd, msg, wp, lp);
	}

	HWND createMessageWindow(LPCWSTR className, LPCWSTR windowName) {
		HINSTANCE hinst = ::GetModuleHandle(NULL);
		if (!MessageWindowClass()) {
			WNDCLASSEXW wcex = {
				/*size*/sizeof(WNDCLASSEX), /*style*/0, /*proc*/&MsgWndProc, /*extra*/0L,0L, /*hinst*/hinst,
				/*icon*/NULL, /*cursor*/NULL, /*brush*/NULL, /*menu*/NULL, /*class*/className, /*smicon*/NULL };
			
			if (!(MessageWindowClass(false, ::RegisterClassExW(&wcex))))
				TVPThrowExceptionMessage((ttstr(TJS_W("register window class failed: "))+className).c_str());
		}
		HWND hwnd = ::CreateWindowExW(0, (LPCWSTR)MAKELONG(MessageWindowClass(), 0), windowName,
									  0, 0, 0, 1, 1, HWND_MESSAGE, NULL, hinst, NULL);
		if (!hwnd) TVPThrowExceptionMessage((ttstr(TJS_W("create message window failed: "))+windowName).c_str());
		::SetWindowLong(hwnd, GWL_USERDATA, (LONG)this);
		return hwnd;
	}

	HWND destroyMessageWindow(HWND hwnd) {
		if (hwnd) {
			::SetWindowLong(hwnd, GWL_USERDATA, 0);
			::DestroyWindow(hwnd);
		}
		return NULL;
	}
};

#endif
