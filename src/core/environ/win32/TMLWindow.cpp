
#include "tjsCommHead.h"
#include "WindowIntf.h"
#include "ComplexRect.h"
#include "TMLWindow.h"
#include <vector>
#include <windowsx.h>
#include "MainFormUnit.h"
#include "WindowMenu.h"
#include "Exception.h"
#include "Application.h"
#include "Resource.h"

namespace TML {

LRESULT WINAPI Window::WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	Window	*win = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd,GWLP_USERDATA));
	if( win != NULL ) {
		return win->Proc( hWnd, msg, wParam, lParam );
	}
	return ::DefWindowProc(hWnd,msg,wParam,lParam);
}

LRESULT WINAPI Window::Proc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	switch( msg ) {
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC	hDC = BeginPaint( hWnd, &ps );
		EndPaint( hWnd, &ps );
		if( created_ ) OnPaint();
		return 0;
	}
	case WM_CLOSE: {
		CloseAction action = caFree;
		OnClose(action);
		switch( action ) {
		case caNone:
			break;
		case caHide:
			::ShowWindow( GetHandle(), SW_HIDE );
			break;
		case caFree:
			::DestroyWindow( GetHandle() );
			break;
		case caMinimize:
			::ShowWindow( GetHandle(), SW_MINIMIZE );
			break;
		}
		return 0;
	}

// Mouse ハンドラ
		/* wParam
		0x0001 (MK_LBUTTON) 	マウスの左ボタンが押されています。
		0x0002 (MK_RBUTTON) 	マウスの右ボタンが押されています。
		0x0004 (MK_SHIFT) 		[Shift] キーが押されています。
		0x0008 (MK_CONTROL) 	[Ctrl] キーが押されています。
		0x0010 (MK_MBUTTON) 	マウスの中央ボタンが押されています。
		0x0020 (MK_XBUTTON1) 	Windows 2000/XP： 1番目の X ボタンが押されています。
		0x0040 (MK_XBUTTON2)	Windows 2000/XP： 2番目の X ボタンが押されています。
		*/
	case WM_MOUSEWHEEL:
		OnMouseWheel( GET_WHEEL_DELTA_WPARAM(wParam), GetShiftState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
		return 0;

	case WM_MOUSEMOVE:
		OnMouseMove( GetShiftState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
		return 0;

	case WM_LBUTTONDOWN:
		// ::SetTimer( GetHandle(), msg, ::GetDoubleClickTime(), NULL );
		LeftDoubleClick = false;
		OnMouseDown( mbLeft, GetShiftState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
		return 0;
	case WM_LBUTTONUP:
		if( LeftDoubleClick == false ) {
			OnMouseClick( mbLeft, GetShiftState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
		}
		LeftDoubleClick = false;
		OnMouseUp( mbLeft, GetShiftState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
		return 0;
	case WM_LBUTTONDBLCLK:
		LeftDoubleClick = true;
		OnMouseDoubleClick( mbLeft, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
		OnMouseDown( mbLeft, GetShiftState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
		return 0;

	case WM_RBUTTONDOWN:
		OnMouseDown( mbRight, GetShiftState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
		return 0;
	case WM_RBUTTONUP:
		OnMouseUp( mbRight, GetShiftState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
		return 0;
	case WM_RBUTTONDBLCLK: // 右ダブルクリックは無視
		return ::DefWindowProc(hWnd,msg,wParam,lParam);

	case WM_MBUTTONDOWN:
		OnMouseDown( mbMiddle, GetShiftState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
		return 0;
	case WM_MBUTTONUP:
		OnMouseUp( mbMiddle, GetShiftState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
		return 0;
	case WM_MBUTTONDBLCLK: // 中ダブルクリックは無視
		return ::DefWindowProc(hWnd,msg,wParam,lParam);

	case WM_XBUTTONDBLCLK: // Xダブルクリックは無視
		return ::DefWindowProc(hWnd,msg,wParam,lParam);
	case WM_XBUTTONUP:
	    switch(GET_XBUTTON_WPARAM(wParam)){
        case XBUTTON1: // サイドキーが第1Xボタン
            OnMouseUp( mbX1, GetShiftState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
			return 0;
        case XBUTTON2: // サイドキーが第2Xボタン
            OnMouseUp( mbX2, GetShiftState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
			return 0;
		}
		return ::DefWindowProc(hWnd,msg,wParam,lParam);
	case WM_XBUTTONDOWN: // 戻るや進むが割り当てられる
	    switch(GET_XBUTTON_WPARAM(wParam)){
        case XBUTTON1: // サイドキーが第1Xボタン
            OnMouseDown( mbX1, GetShiftState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
			return 0;
        case XBUTTON2: // サイドキーが第2Xボタン
            OnMouseDown( mbX2, GetShiftState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
			return 0;
		}
		return ::DefWindowProc(hWnd,msg,wParam,lParam);
#if 0
	case WM_TOUCH: {
		// user32.dll から GetTouchInputInfo など読み込む
		UINT cInputs = LOWORD(wParam);
		PTOUCHINPUT pInputs = new TOUCHINPUT[cInputs];
		if( NULL != pInputs ) {
			if( GetTouchInputInfo( (HTOUCHINPUT)lParam, cInputs, pInputs, sizeof(TOUCHINPUT)) ) {
				// process pInputs
				if( !CloseTouchInputHandle((HTOUCHINPUT)lParam) ) {
					// error handling
				}
			} else {
				// GetLastError() and error handling
			}
			delete[] pInputs;
		} else {
			// error handling, presumably out of memory
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	case WM_GESTURE: {
		GESTUREINFO gi;
		ZeroMemory(&gi, sizeof(GESTUREINFO));
		gi.cbSize = sizeof(gi);
		BOOL bResult = GetGestureInfo((HGESTUREINFO)lParam, &gi);
		BOOL bHandled = FALSE;
		switch (gi.dwID){
        case GID_ZOOM:
            // Code for zooming goes here
			bHandled = TRUE;
            break;
        case GID_PAN:
			bHandled = TRUE;
            break;
        case GID_ROTATE:
			bHandled = TRUE;
            break;
        case GID_TWOFINGERTAP:
			bHandled = TRUE;
            break;
        case GID_PRESSANDTAP:
			bHandled = TRUE;
            break;
        default:
            // You have encountered an unknown gesture
            break;
		}
		CloseGestureInfoHandle((HGESTUREINFO)lParam);
		if( bHandled ) return 0;
		else return DefWindowProc(hWnd, msg, wParam, lParam);
	}
#endif
	 /*
	case WM_IDLE:
		if( TVPMainForm ) {
			return TVPMainForm->ApplicationIdel();
		}
		return ::DefWindowProc(hWnd,msg,wParam,lParam);	
		*/
	case WM_KEYDOWN:
		OnKeyDown( (WORD)wParam, GetShiftState(), lParam&0xffff, (lParam&(1<<30))?true:false );
		return ::DefWindowProc(hWnd,msg,wParam,lParam);
	case WM_KEYUP:
		OnKeyUp( (WORD)wParam, GetShiftState() );
		return ::DefWindowProc(hWnd,msg,wParam,lParam);
	case WM_CHAR:
		OnKeyPress( (WORD)wParam, lParam&0xffff, (lParam&(1<<30))?true:false, (lParam&(1<<31))?true:false );
		return 0;
		
	case WM_SETFOCUS:
		OnFocus();
		return ::DefWindowProc(hWnd,msg,wParam,lParam);
	case WM_KILLFOCUS:
		OnFocusLost();
		return ::DefWindowProc(hWnd,msg,wParam,lParam);
	case WM_GETMINMAXINFO:
		if( min_size_.cx != 0 ||  min_size_.cy != 0 || max_size_.cx != 0 || max_size_.cy != 0 ) {
			MINMAXINFO* lpmmi = (LPMINMAXINFO)lParam;
			// lpmmi->ptMaxPosition 最大化時の位置
			// 最小サイズ
			if( min_size_.cx > 0 ) {
				lpmmi->ptMinTrackSize.x = min_size_.cx;
			}
			if( min_size_.cy > 0 ) {
				lpmmi->ptMinTrackSize.y = min_size_.cy;
			}
			if( max_size_.cx > 0 ) {
				lpmmi->ptMaxTrackSize.x = max_size_.cx; // サイズ変更時の最大サイズ
				lpmmi->ptMaxSize.x = max_size_.cx; // 最大化時のサイズ
			}
			if( max_size_.cy > 0 ) {
				lpmmi->ptMaxTrackSize.y = max_size_.cy; // サイズ変更時の最大サイズ
				lpmmi->ptMaxSize.y = max_size_.cy; // 最大化時のサイズ
			}
			return 0;
		} else {
			return ::DefWindowProc(hWnd,msg,wParam,lParam);
		}
	case WM_SYSCOMMAND:
		if( wParam == SC_CLOSE ) {
			ModalResult = mrCancel;
		}
		return ::DefWindowProc(hWnd,msg,wParam,lParam);
	case WM_DESTROY:
		OnDestroy();
		PostQuitMessage( 0 );
		return 0;
	default:
		return ::DefWindowProc(hWnd,msg,wParam,lParam);
	}
	return ::DefWindowProc(hWnd,msg,wParam,lParam);
}
void Window::OnPaint() {
}
const DWORD Window::DEFAULT_EX_STYLE = WS_EX_ACCEPTFILES | WS_EX_APPWINDOW;
HRESULT Window::CreateWnd( const std::string& classname, const std::string& title, int width, int height )
{
	window_class_name_ = classname;
	window_title_ = title;
	window_client_size_.cx = width;
	window_client_size_.cy = height;

	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC | CS_DBLCLKS, Window::WndProc, 0L, 0L,
						GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
						window_class_name_.c_str(), NULL };
	BOOL ClassRegistered = ::GetClassInfoEx( wc.hInstance, wc.lpszClassName, &wc );
	if( ClassRegistered == 0 ) {
		::RegisterClassEx( &wc );
	}
	wc_ = wc;

	RECT	winRc = { 0, 0, window_client_size_.cx, window_client_size_.cy };
	::AdjustWindowRectEx( &winRc, WS_OVERLAPPEDWINDOW, NULL, DEFAULT_EX_STYLE );
	window_handle_ = ::CreateWindowEx( DEFAULT_EX_STYLE, window_class_name_.c_str(), window_title_.c_str(),
						WS_OVERLAPPEDWINDOW, 0, 0, winRc.right-winRc.left, winRc.bottom-winRc.top,
						NULL, NULL, wc.hInstance, NULL );

	if( window_handle_ == NULL )
		return HRESULT_FROM_WIN32(::GetLastError());
	created_ = true;
	ime_control_ = new ImeControl(window_handle_);
	border_style_ = bsSizeable;

	::SetWindowLongPtr( window_handle_, GWLP_USERDATA, (LONG_PTR)this );

	//::ShowWindow(window_handle_,SW_SHOWDEFAULT);
	::ShowWindow(window_handle_,SW_HIDE);
	if( ::UpdateWindow(window_handle_) == 0 )
		return HRESULT_FROM_WIN32(::GetLastError());

	return S_OK;
}
void Window::UnregisterWindow() {
	::UnregisterClass( window_class_name_.c_str(), wc_.hInstance );
}

void Window::SetWidnowTitle( const std::string& title ) {
	if( window_title_ != title ) {
		window_title_ = title;
		if( window_handle_ ) {
			::SetWindowText( window_handle_, window_title_.c_str() );
		}
	}
}

void Window::SetScreenSize( int width, int height ) {
	if( window_client_size_.cx != width || window_client_size_.cy != height ) {
		window_client_size_.cx = width;
		window_client_size_.cy = height;
		if( window_handle_ ) {
			RECT	winRc = { 0, 0, window_client_size_.cx, window_client_size_.cy };
			::AdjustWindowRect( &winRc, WS_OVERLAPPEDWINDOW, NULL );
			::SetWindowPos( window_handle_, HWND_TOP, 0, 0, winRc.right-winRc.left, winRc.bottom-winRc.top,
				SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
		}
	}
}
/*
int Window::MainLoop() {
	MSG msg;
	ZeroMemory( &msg, sizeof(msg) );
	// 最初に生成関係のたまっているメッセージを処理する
	while( msg.message != WM_QUIT && PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) ) {
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	// アプリスレッドを始動する
	GetApplication()->Wakeup();

	// メイン処理
	while( msg.message != WM_QUIT ) {
		if( GetMessage( &msg, NULL, 0, 0 ) ) {
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}
	return 0;
}
*/
bool Window::Initialize() {
	return true;
}
void Window::OnDestroy() {
	::SetWindowLongPtr( window_handle_, GWLP_USERDATA, (LONG_PTR)NULL );
	delete this;
}

void Window::SetClientSize( HWND hWnd, SIZE& size ) {
	HMENU hMenu = ::GetMenu( hWnd );
	DWORD style = ::GetWindowLong( hWnd, GWL_STYLE );
	DWORD exStyle = ::GetWindowLong( hWnd, GWL_EXSTYLE );
	RECT rect;
	::SetRect( &rect, 0, 0, size.cx, size.cy );
	if( ::AdjustWindowRectEx( &rect, style, hMenu ? TRUE : FALSE, exStyle ) ) {
		RECT rect2;
		if( ::GetWindowRect( hWnd, &rect2 ) ) {
			if( ::SetWindowPos( hWnd, NULL, rect2.left, rect2.top, rect.right-rect.left, rect.bottom-rect.top, SIZE_CHANGE_FLAGS ) == 0 ) {
				// error
			}
		} else {
			// error
		}
	} else {
		// error
	}
}


// 表示状態
bool Window::GetVisible() const {
/*
	WINDOWPLACEMENT wndpl;
//	::ZeroMemory( &wndpl, sizeof(wndpl) );
	wndpl.length = sizeof(WINDOWPLACEMENT);
	if( GetWindowPlacement( GetHandle(), &wndpl ) ) {
		// 非表示以外の時は、表示状態
		return wndpl.showCmd != SW_HIDE;
	} else {
		// error, とりあえず false を返しておく
		return false;
	}
*/
	return ::IsWindowVisible( GetHandle() ) ? true : false;
}
void Window::SetVisible( bool s ) {
	::ShowWindow( GetHandle(), s ? SW_SHOW : SW_HIDE );
}

bool Window::GetEnable() const {
	return ::IsWindowEnabled( GetHandle() ) ? true : false;
}
void Window::SetEnable( bool s ) {
	::EnableWindow( GetHandle(), s ? TRUE : FALSE );
}

void Window::GetCaption( std:: string& v ) const {
	v.clear();
	int len = ::GetWindowTextLength( GetHandle() );
	if( len > 0 ) {
		std::vector<char> caption(len+1,0);
		int readlen = ::GetWindowText( GetHandle(), &(caption[0]), len+1 );
		if( readlen > 0 ) {
			v.assign( &(caption[0]) );
		}
	}

// 内部で持っているのよりも、ちゃんと毎回取得した方がいいか
//	v = window_title_;
}
void Window::SetCaption( const std::string& v ) {
	if( window_title_ != v ) {
		window_title_ = v;
		::SetWindowText( GetHandle(), window_title_.c_str() );
	}
}
/*
bsDialog : サイズ変更不可の、ダイアログボックスと同様の外見を持ちます。
WS_DLGFRAME | WS_POPUP | WS_CAPTION
WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE;
//WS_DLGFRAME 
//-WS_THICKFRAME 

bsSingle : サイズ変更不可のウィンドウです。
WS_CAPTION | WS_BORDER
//WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_BORDER

bsNone : ボーダーのないウィンドウです。
WS_POPUP
// WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX

bsSizeable : サイズ変更可の一般的なウィンドウです。デフォルトです。
WS_CAPTION | WS_THICKFRAME
//WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_BORDER


bsToolWindow : サイズ変更不可のツールウィンドウ(キャプションの小さいウィンドウ) です。
WS_CAPTION | WS_BORDER
WS_EX_TOOLWINDOW

bsSizeToolWin : bsToolWindow と似ていますが、サイズ変更が可能です。
WS_CAPTION | WS_THICKFRAME
WS_EX_TOOLWINDOW
*/
void Window::SetBorderStyle(tTVPBorderStyle st) {
	const DWORD notStyle = WS_POPUP | WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_DLGFRAME;
	DWORD style = ::GetWindowLong( GetHandle(), GWL_STYLE);
	DWORD exStyle = ::GetWindowLong( GetHandle(), GWL_EXSTYLE);
	border_style_ = static_cast<int>(st);
	switch( st ) {
	case bsDialog:
		style = ~notStyle;
		style = WS_DLGFRAME | WS_POPUP | WS_CAPTION;
		exStyle &= ~DEFAULT_EX_STYLE;
		exStyle |= WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE;
		break;
	case bsSingle:
		style = ~notStyle;
		style = WS_CAPTION | WS_BORDER;
		exStyle &= ~DEFAULT_EX_STYLE;
		break;
	case bsNone:
		style = ~notStyle;
		style = WS_POPUP;
		exStyle &= ~DEFAULT_EX_STYLE;
		break;
	case bsSizeable:
		style = ~notStyle;
		style = WS_OVERLAPPED;
		exStyle &= ~DEFAULT_EX_STYLE;
		break;
	case bsToolWindow:
		style = ~notStyle;
		style = WS_CAPTION | WS_BORDER;
		exStyle &= ~DEFAULT_EX_STYLE;
		exStyle |= WS_EX_TOOLWINDOW;
		break;
	case bsSizeToolWin:
		style = ~notStyle;
		style = WS_CAPTION | WS_THICKFRAME;
		exStyle &= ~DEFAULT_EX_STYLE;
		exStyle |= WS_EX_TOOLWINDOW;
		break;
	}
	::SetWindowLong( GetHandle(), GWL_STYLE, style );
	::SetWindowLong( GetHandle(), GWL_EXSTYLE, exStyle );
	if( st == bsDialog ) {
		::SendMessage( GetHandle(), WM_SETICON, ICON_BIG, LPARAM(GetBigIcon()));
	} else {
		::SendMessage( GetHandle(), WM_SETICON, ICON_BIG, 0 );
	}
	::GetSystemMenu( GetHandle(), TRUE );
	::SendMessage( GetHandle(), WM_NCCREATE, 0, 0 );
	::SetWindowPos( GetHandle(), 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE );
	::InvalidateRect( GetHandle(), NULL, TRUE );
}
tTVPBorderStyle Window::GetBorderStyle() const {
	return static_cast<tTVPBorderStyle>(border_style_);
}
HICON Window::GetBigIcon() {
	return (HICON)SendMessage( GetHandle(), WM_GETICON, ICON_BIG, 0 );
}

const UINT Window::SIZE_CHANGE_FLAGS = SWP_DEFERERASE | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;

void Window::SetWidth( int w ) {
	RECT rect;
	if( ::GetWindowRect( GetHandle(), &rect ) ) {
		if( ::SetWindowPos( GetHandle(), NULL, rect.left, rect.top, w, rect.bottom-rect.top, SIZE_CHANGE_FLAGS ) == 0 ) {
			// error
		}
	}
}

int Window::GetWidth() const {
	RECT rect;
	if( ::GetWindowRect( GetHandle(), &rect ) ) {
		return rect.right - rect.left;
	} else {
		// error
		return 0;
	}
}

void Window::SetHeight( int h ) {
	RECT rect;
	if( ::GetWindowRect( GetHandle(), &rect ) ) {
		if( ::SetWindowPos( GetHandle(), NULL, rect.left, rect.top, rect.right-rect.left, h, SIZE_CHANGE_FLAGS ) == 0 ) {
			// error
		}
	}
}

int Window::GetHeight() const {
	RECT rect;
	if( ::GetWindowRect( GetHandle(), &rect ) ) {
		return rect.bottom - rect.top;
	} else {
		// error
		return 0;
	}
}

void Window::SetSize( int w, int h ) {
	RECT rect;
	if( ::GetWindowRect( GetHandle(), &rect ) ) {
		if( ::SetWindowPos( GetHandle(), NULL, rect.left, rect.top, w, h, SIZE_CHANGE_FLAGS ) == 0 ) {
			// error
		}
	}
}
void Window::GetSize( int &w, int &h ) {
	RECT rect;
	if( ::GetWindowRect( GetHandle(), &rect ) ) {
		h = rect.bottom - rect.top;
		w = rect.right - rect.left;
	} else {
		h = w = 0;
	}
}
const UINT Window::POS_CHANGE_FLAGS = SWP_DEFERERASE | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER;

void Window::SetLeft( int l ) {
	RECT rect;
	if( ::GetWindowRect( GetHandle(), &rect ) ) {
		if( ::SetWindowPos( GetHandle(), NULL, l, rect.top, rect.right-rect.left, rect.bottom-rect.top, POS_CHANGE_FLAGS ) == 0 ) {
			// error
		}
	}
}
int Window::GetLeft() const {
	RECT rect;
	if( ::GetWindowRect( GetHandle(), &rect ) ) {
		return rect.left;
	} else {
		// error
		return 0;
	}
}
void Window::SetTop( int t ) {
	RECT rect;
	if( ::GetWindowRect( GetHandle(), &rect ) ) {
		if( ::SetWindowPos( GetHandle(), NULL, rect.left, t, rect.right-rect.left, rect.bottom-rect.top, POS_CHANGE_FLAGS ) == 0 ) {
			// error
		}
	}
}
int Window::GetTop() const {
	RECT rect;
	if( ::GetWindowRect( GetHandle(), &rect ) ) {
		return rect.top;
	} else {
		// error
		return 0;
	}
}
void Window::SetPosition( int l, int t ) {
	RECT rect;
	if( ::GetWindowRect( GetHandle(), &rect ) ) {
		if( ::SetWindowPos( GetHandle(), NULL, l, t, rect.right-rect.left, rect.bottom-rect.top, POS_CHANGE_FLAGS ) == 0 ) {
			// error
		}
	}
}
void Window::SetBounds( int x, int y, int width, int height ) {
	::SetWindowPos( GetHandle(), NULL, x, y, width, height, POS_CHANGE_FLAGS );
}
void Window::SetInnerWidth( int w ) {
	RECT rect;
	if( ::GetClientRect( GetHandle(), &rect ) ) {
		SIZE size;
		size.cx = w;
		size.cy = rect.bottom - rect.top;
		SetClientSize( GetHandle(), size );
	} else {
		// error
	}
}

int Window::GetInnerWidth() const {
	RECT rect;
	if( ::GetClientRect( GetHandle(), &rect ) ) {
		return rect.right - rect.left;
	} else {
		// error
		return 0;
	}
}
void Window::SetInnerHeight( int h ) {
	RECT rect;
	if( ::GetClientRect( GetHandle(), &rect ) ) {
		SIZE size;
		size.cx = rect.right - rect.left;
		size.cy = h;
		SetClientSize( GetHandle(), size );
	} else {
		// error
	}
}

int Window::GetInnerHeight() const {
	RECT rect;
	if( ::GetClientRect( GetHandle(), &rect ) ) {
		return rect.bottom - rect.top;
	} else {
		// error
		return 0;
	}
}

void Window::SetInnerSize( int w, int h ) {
	SIZE size;
	size.cx = w;
	size.cy = h;
	SetClientSize( GetHandle(), size );
}

void Window::BringToFront() {
	::SetWindowPos( GetHandle(), HWND_TOP, 0, 0, 0, 0, (SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE) );
}
void Window::SetStayOnTop( bool b ) {
	static const UINT flags = SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOSENDCHANGING;
	if( ::SetWindowPos( GetHandle(), b ? HWND_TOPMOST : HWND_NOTOPMOST, 0,0,0,0, flags ) == 0 ) {
		// error
	}
}
bool Window::GetStayOnTop() const {
	DWORD exStyle = ::GetWindowLong( GetHandle(), GWL_EXSTYLE );
	if( exStyle == 0 ) {
		// error
		return false;
	}
	return (exStyle & WS_EX_TOPMOST) ? true : false;
}
void Window::SetFullScreenMode(bool b) {
}

void Window::GetClientRect( struct tTVPRect& rt ) {
	RECT r;
	::GetClientRect( GetHandle(), &r );
	rt.top = r.top;
	rt.left = r.left;
	rt.bottom = r.bottom;
	rt.right = r.right;
}
void Window::SetMenu( class WindowMenu* menu ) {
	if( menu ) {
		::SetMenu( GetHandle(), menu->GetHandle() );
	} else {
		::SetMenu( GetHandle(), NULL );
	}
}
int Window::ShowModal() {
	if( GetVisible() || !GetEnable() ) {
		throw Exception("Cannot Show Modal.");
	}
	if( ::GetCapture() != 0 ) {
		::SendMessage( ::GetCapture(), WM_CANCELMODE, 0, 0 );
	}
	::ReleaseCapture();
	Application->ModalStarted();
	std::vector<class TTVPWindowForm*> disablewins;
	try {
		//HWND hActiveWnd = ::GetActiveWindow();
		Application->GetDisableWindowList( disablewins );
		Application->DisableWindows();
		SetEnable( true );
		Show();
		// ::SendMessage( GetHandle(), CM_ACTIVATE, 0, 0 );
		ModalResult = 0;
		
		MSG msg;
		HACCEL hAccelTable = ::LoadAccelerators( (HINSTANCE)GetModuleHandle(0), MAKEINTRESOURCE(IDC_TVPWIN32));

		HWND hSelfWnd = GetHandle();
		BOOL ret = TRUE;
		while( ModalResult == 0 && ::PeekMessage( &msg, hSelfWnd, 0, 0, PM_NOREMOVE) ) {
			ret = ::GetMessage( &msg, hSelfWnd, 0, 0);
			if( ret && !TranslateAccelerator(msg.hwnd, hAccelTable, &msg) ) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		Application->EnableWindows( disablewins );
	} catch(...) {
		Application->ModalFinished();
		throw;
	}
	Application->ModalFinished();
	
/*
function TCustomForm.ShowModal: Integer;
var
  WindowList: Pointer;
  SaveFocusState: TFocusState;
  SaveCursor: TCursor;
  SaveCount: Integer;
  ActiveWindow: HWnd;
begin
  CancelDrag;
  if Visible or not Enabled or (fsModal in FFormState) or
    (FormStyle = fsMDIChild) then
    raise EInvalidOperation.Create(SCannotShowModal);
  if GetCapture <> 0 then SendMessage(GetCapture, WM_CANCELMODE, 0, 0);
  ReleaseCapture;
  Application.ModalStarted;
  try
    Include(FFormState, fsModal);
    if (PopupMode = pmNone) and (Application.ModalPopupMode <> pmNone) then
    begin
      RecreateWnd;
      HandleNeeded;
    end;
    ActiveWindow := GetActiveWindow;
    SaveFocusState := Forms.SaveFocusState;
    Screen.SaveFocusedList.Insert(0, Screen.FocusedForm);
    Screen.FocusedForm := Self;
    SaveCursor := Screen.Cursor;
    Screen.Cursor := crDefault;
    SaveCount := Screen.CursorCount;
    WindowList := DisableTaskWindows(0);
    try
      Show;
      try
        SendMessage(Handle, CM_ACTIVATE, 0, 0);
        ModalResult := 0;
        repeat
          Application.HandleMessage;
          if Application.Terminated then ModalResult := mrCancel else
            if ModalResult <> 0 then CloseModal;
        until ModalResult <> 0;
        Result := ModalResult;
        SendMessage(Handle, CM_DEACTIVATE, 0, 0);
        if GetActiveWindow <> Handle then ActiveWindow := 0;
      finally
        Hide;
      end;
    finally
      if Screen.CursorCount = SaveCount then
        Screen.Cursor := SaveCursor
      else Screen.Cursor := crDefault;
      EnableTaskWindows(WindowList);
      if Screen.SaveFocusedList.Count > 0 then
      begin
        Screen.FocusedForm := Screen.SaveFocusedList.First;
        Screen.SaveFocusedList.Remove(Screen.FocusedForm);
      end else Screen.FocusedForm := nil;
      if ActiveWindow <> 0 then SetActiveWindow(ActiveWindow);
      RestoreFocusState(SaveFocusState);
      Exclude(FFormState, fsModal);
    end;
  finally
    Application.ModalFinished;
  end;
end;
*/
	return ModalResult;
}
}; // namespace
