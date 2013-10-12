
#include "TVPTimer.h"


TVPTimer::TVPTimer() : event_(NULL), interval_(1000), enabled_(true) {
	CreateUtilWindow();
}

TVPTimer::~TVPTimer() {
	Destroy();
	if( event_ ) {
		delete event_;
	}
}
int TVPTimer::CreateUtilWindow() {
	::ZeroMemory( &wc_, sizeof(wc_) );
	wc_.cbSize = sizeof(WNDCLASSEX);
	//wc_.style = 0;
	//wc_.lpfnWndProc = TVPTimer::WndProc;
	wc_.lpfnWndProc = ::DefWindowProc;
	//wc_.cbClsExtra = 0;
	//wc_.cbWndExtra = 0;
	wc_.hInstance = ::GetModuleHandle(NULL);
	//wc_.hIcon = 0;
	//wc_.hCursor = 0;
	//wc_.hbrBackground = 0;
	//wc_.lpszMenuName = NULL;
	wc_.lpszClassName = L"TVPUtilWindow";//L"TVPTimerWindow";

	BOOL ClassRegistered = ::GetClassInfoEx( wc_.hInstance, wc_.lpszClassName, &wc_ );
	if( ClassRegistered == 0 ) {
		if( ::RegisterClassEx( &wc_ ) == 0 ) {
#ifdef _DEBUG
			LPVOID lpMsgBuf;
			::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL );
			::OutputDebugString( (LPCWSTR)lpMsgBuf );
			::LocalFree(lpMsgBuf);
#endif
			return HRESULT_FROM_WIN32(::GetLastError());
		}
	}
	window_handle_ = ::CreateWindowEx( WS_EX_TOOLWINDOW, wc_.lpszClassName, L"",
						WS_POPUP, 0, 0, 0, 0, NULL, NULL, wc_.hInstance, NULL );
	
	if( window_handle_ == NULL ) {
#ifdef _DEBUG
		LPVOID lpMsgBuf;
		::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL );
		::OutputDebugString( (LPCWSTR)lpMsgBuf );
		::LocalFree(lpMsgBuf);
#endif
		return HRESULT_FROM_WIN32(::GetLastError());
	}
    ::SetWindowLongPtr( window_handle_, GWLP_WNDPROC, (LONG_PTR)TVPTimer::WndProc );
	::SetWindowLongPtr( window_handle_, GWLP_USERDATA, (LONG_PTR)this );
	return S_OK;
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
	::KillTimer( window_handle_, 1 );
	if( interval_ > 0 && enabled_ && event_ != NULL ) {
		if( ::SetTimer( window_handle_, 1, interval_, NULL ) == 0 ) {
			// Exception
		}
	}
	
}

