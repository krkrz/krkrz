
#include "NativeEventQueue.h"

int NativeEventQueueImplement::CreateUtilWindow() {
	wc_.cbSize = sizeof(WNDCLASSEX);
	wc_.style = 0;
	wc_.lpfnWndProc = NativeEventQueueImplement::WndProc;
	wc_.cbClsExtra = 0;
	wc_.cbWndExtra = 0;
	wc_.hInstance = ::GetModuleHandle(NULL);
	wc_.hIcon = 0;
	wc_.hCursor = 0;
	wc_.hbrBackground = 0;
	wc_.lpszMenuName = NULL;
	wc_.lpszClassName = L"TPUtilWindow";

	BOOL ClassRegistered = ::GetClassInfoEx( wc_.hInstance, wc_.lpszClassName, &wc_ );
	if( ClassRegistered == 0 ) {
		::RegisterClassEx( &wc_ );
	}
	window_handle_ = ::CreateWindowEx( WS_EX_TOOLWINDOW, wc_.lpszClassName, L"",
						WS_POPUP, 0, 0, 0, 0, NULL, NULL, wc_.hInstance, NULL );

	if( window_handle_ == NULL )
		return HRESULT_FROM_WIN32(::GetLastError());
	::SetWindowLongPtr( window_handle_, GWLP_USERDATA, (LONG_PTR)this );
	return S_OK;
}

LRESULT WINAPI NativeEventQueueImplement::WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	NativeEvent event;
	event.Result = 0;
	event.HWnd = hWnd;
	event.Message = msg;
	event.WParam = wParam;
	event.LParam = lParam;
	NativeEventQueueIntarface *win = reinterpret_cast<NativeEventQueueIntarface*>(::GetWindowLongPtr(hWnd,GWLP_USERDATA));
	if( win != NULL ) {
		win->Dispatch( event );
		return event.Result;
	} else {
		return ::DefWindowProc(event.HWnd,event.Message,event.WParam,event.LParam);
	}
}

// デフォルトハンドラ
void NativeEventQueueImplement::HandlerDefault( NativeEvent& event ) {
	event.Result = ::DefWindowProc(event.HWnd,event.Message,event.WParam,event.LParam);
}
void NativeEventQueueImplement::Allocate() {
	CreateUtilWindow();
}
void NativeEventQueueImplement::Deallocate() {
	if( window_handle_ != NULL ) {
		::SetWindowLongPtr( window_handle_, GWLP_USERDATA, (LONG_PTR)NULL );
		::DestroyWindow( window_handle_ );
		window_handle_ = NULL;
	}
}
void NativeEventQueueImplement::PostEvent( const NativeEvent& event ) {
	::PostMessage( window_handle_, event.Message, event.WParam, event.LParam );
}

