
#ifndef __UTIL_WINDOW_H__
#define __UTIL_WINDOW_H__

#include "stdafx.h"


class UtilWindow {
	WNDCLASSEX			wc_;

	static LRESULT WINAPI WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

protected:
	HWND				window_handle_;

	HRESULT CreateWnd();
	void UnregisterWindow();

	virtual LRESULT WINAPI Proc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

public:
	UtilWindow()
	: window_handle_(NULL) {
		ZeroMemory( &wc_, sizeof(wc_) );
	}
	virtual ~UtilWindow() {
		DeallocateUtilWnd();
	}

	void AllocateUtilWnd();
	void DeallocateUtilWnd();

	void PostMessage( UINT msg, WPARAM wParam, LPARAM lParam ) {
		::PostMessage( window_handle_, msg, wParam, lParam );
	}
};



#endif // __UTIL_WINDOW_H__
