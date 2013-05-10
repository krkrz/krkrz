
#include "UtilWindow.h"
#include <tchar.h>

LRESULT WINAPI UtilWindow::WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	UtilWindow	*win = reinterpret_cast<UtilWindow*>(GetWindowLongPtr(hWnd,GWLP_USERDATA));
	if( win != NULL ) {
		return win->Proc( hWnd, msg, wParam, lParam );
	}
	return ::DefWindowProc(hWnd,msg,wParam,lParam);
}

LRESULT WINAPI UtilWindow::Proc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	return ::DefWindowProc(hWnd,msg,wParam,lParam);
}
HRESULT UtilWindow::CreateWnd() {
	wc_.cbSize = sizeof(WNDCLASSEX);
	wc_.style = 0;
	wc_.lpfnWndProc = UtilWindow::WndProc;
	wc_.cbClsExtra = 0;
	wc_.cbWndExtra = 0;
	wc_.hInstance = ::GetModuleHandle(NULL);
	wc_.hIcon = 0;
	wc_.hCursor = 0;
	wc_.hbrBackground = 0;
	wc_.lpszMenuName = NULL;
	wc_.lpszClassName = _T("TPUtilWindow");

	BOOL ClassRegistered = ::GetClassInfoEx( wc_.hInstance, wc_.lpszClassName, &wc_ );
	if( ClassRegistered == 0 ) {
		::RegisterClassEx( &wc_ );
	}
	window_handle_ = ::CreateWindowEx( WS_EX_TOOLWINDOW, wc_.lpszClassName, _T(""),
						WS_POPUP, 0, 0, 0, 0, NULL, NULL, wc_.hInstance, NULL );

	if( window_handle_ == NULL )
		return HRESULT_FROM_WIN32(::GetLastError());
	::SetWindowLongPtr( window_handle_, GWLP_USERDATA, (LONG_PTR)this );
	return S_OK;
}
void UtilWindow::UnregisterWindow() {
	::UnregisterClass( wc_.lpszClassName, wc_.hInstance );
}


void UtilWindow::AllocateUtilWnd() {
	CreateWnd();
}
void UtilWindow::DeallocateUtilWnd() {
	if( window_handle_ != NULL ) {
		::SetWindowLongPtr( window_handle_, GWLP_USERDATA, (LONG_PTR)NULL );
		::DestroyWindow( window_handle_ );
		window_handle_ = NULL;
	}
}

