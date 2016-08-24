
#include "tjsCommHead.h"
#include "TVPTimer.h"
#include "WindowsUtil.h"

TVPTimer::TVPTimer() : event_(NULL), interval_(1000), enabled_(true) {
	CreateUtilWindow();

	// Application->addEventHandler しつつ、TimerでApplicationにpostEvent
	// 定期的にハンドリングする

	// http://www.ops.dti.ne.jp/~allergy/thread/thread.html
	// http://smdn.jp/programming/tips/posix_timer/
	
	// http://yamanetoshi.github.io/blog/2014/09/25/do-it-after-a-few-seconds/
	timer_create( CLOCK_MONOTONIC, 
}

TVPTimer::~TVPTimer() {
	Destroy();
	if( event_ ) {
		delete event_;
	}
}

void TVPTimer::Destroy() {
	if( window_handle_ != NULL ) {
		::SetWindowLongPtr( window_handle_, GWLP_USERDATA, (LONG_PTR)NULL );
		::DestroyWindow( window_handle_ );
		window_handle_ = NULL;
	}
}
LRESULT WINAPI TVPTimer::WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	if( msg == WM_TIMER ) {
		TVPTimer *win = reinterpret_cast<TVPTimer*>(::GetWindowLongPtr(hWnd,GWLP_USERDATA));
		if( win != NULL ) {
			win->FireEvent();
			return 1;
		}
	}
	return ::DefWindowProc(hWnd,msg,wParam,lParam);
}
void TVPTimer::UpdateTimer() {
	if( ::KillTimer( window_handle_, 1 ) == 0 ) {
#ifdef _DEBUG
		TVP_WINDOWS_ERROR_LOG;
#endif
	}
	if( interval_ > 0 && enabled_ && event_ != NULL ) {
		if( ::SetTimer( window_handle_, 1, interval_, NULL ) == 0 ) {
			TVPThrowWindowsErrorException();
		}
	}
}

