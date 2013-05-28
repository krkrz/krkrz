
#ifndef __T_APPLICATION_H__
#define __T_APPLICATION_H__

#include <vector>
#include <map>
#include "tstring.h"

//tstring ParamStr( int index );
tstring ExePath();

// 後で見通しのよう方法に変更する
extern int _argc;
extern char ** _argv;

enum {
	mrOk,
	mrAbort,
	mrCancel,
};
// イベントハンドラについては要検討
class SystemEvent {
public:
	LRESULT Result;
	HWND HWnd;
	UINT Message;
	WPARAM WParam;
	LPARAM LParam;
};

class IMessageHandler {
public:
	virtual void Handle( SystemEvent& message ) = 0;
};

class EventDispatcher {
	HWND window_handle_;
public:
	IMessageHandler* mHandler;
	static LRESULT CALLBACK WindowsProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		EventDispatcher	*dispatcher = reinterpret_cast<EventDispatcher*>(GetWindowLong(hWnd,GWL_USERDATA));
		if( dispatcher != NULL ) {
			SystemEvent ev;
			ev.HWnd = hWnd;
			ev.Message = message;
			ev.WParam = wParam;
			ev.LParam = lParam;
			ev.Result = 0;
			dispatcher->Handle( ev );
			return ev.Result;
		}
		return DefWindowProc( hWnd, message, wParam, lParam );
	}
	HWND AllocateHWnd() {
		const TCHAR* classname = _T("TPUtilWindow");
		WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowsProcedure, 0L, 0L,
							GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
							classname, NULL };
		::RegisterClassEx( &wc );
		window_handle_ = ::CreateWindow( classname, _T(""),
							WS_OVERLAPPEDWINDOW, 0, 0, 0, 0,
							NULL, NULL, wc.hInstance, NULL );
		if( window_handle_ == NULL )
			return NULL;
		//::SetWindowLong(window_handle_,GWL_USERDATA,(LONG)this);
		::SetWindowLongPtr( window_handle_, GWLP_USERDATA, (LONG_PTR)this ); // for x64
		return window_handle_;
	} 
	virtual void Handle( SystemEvent& message ) = 0;
};
class AcceleratorKey {
	HACCEL hAccel_;
	ACCEL* keys_;
	int key_count_;

public:
	AcceleratorKey();
	~AcceleratorKey();
	void AddKey( WORD id, WORD key, BYTE virt );
	void DelKey( WORD id );
	HACCEL GetHandle() { return hAccel_; }
};
class AcceleratorKeyTable {
	std::map<HWND,AcceleratorKey*> keys_;
	HACCEL hAccel_;

public:
	AcceleratorKeyTable();
	~AcceleratorKeyTable();
	void AddKey( HWND hWnd, WORD id, WORD key, BYTE virt );
	void DelKey( HWND hWnd, WORD id );
	void DelTable( HWND hWnd );
	HACCEL GetHandle(HWND hWnd) {
		std::map<HWND,AcceleratorKey*>::iterator i = keys_.find(hWnd);
		if( i != keys_.end() ) {
			return i->second->GetHandle();
		}
		return hAccel_;
	}
};
class TApplication {
	std::vector<EventDispatcher*> dispatcher_;
	std::vector<class TTVPWindowForm*> windows_list_;
	tstring title_;

	bool is_attach_console_;
	FILE* oldstdin_;
	FILE* oldstdout_;
	tstring console_title_;
	AcceleratorKeyTable accel_key_;

public:
	TApplication() : is_attach_console_(false) {}
	void CheckConsole();
	void CloseConsole();
	bool IsAttachConsole() { return is_attach_console_; }
	void CheckDigitizer();

	HWND GetHandle();
	bool IsIconic() {
		HWND hWnd = GetHandle();
		if( hWnd != INVALID_HANDLE_VALUE ) {
			return 0 != ::IsIconic(hWnd);
		}
		return true; // そもそもウィンドウがない
	}
	void Minimize();
	void Restore();
	void BringToFront();

	void AddWindow( class TTVPWindowForm* win ) {
		windows_list_.push_back( win );
	}
	void RemoveWindow( class TTVPWindowForm* win );

	void ProcessMessages() {
#if 0
		int count = dispatcher_.size();
		for( int i = 0; i < count; i++ ) {
			if( dispatcher_[i]->HWnd != INVALID_HANDLE_VALUE ) {
				MSG msg;
				HWND hWnd = dispatcher_[i]->HWnd;
				while(true) {
					if( PeekMessage( &msg,NULL,0,0,PM_NOREMOVE) ) {
						SystemEvent event={};
						if( !GetMessage( &msg,NULL,0,0) ) break;
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					} else {
						break;
					}
				}
			}
		}
#endif
	}
	void HandleMessage() {}
	tstring GetTitle() const { return title_; }
	void SetTitle( const tstring& caption );

	static inline int MessageDlg( const tstring& string, const tstring& caption, int type ) {
		::MessageBox( NULL, string.c_str(), caption.c_str(), type  );
	}
	void Terminate() {
		::PostQuitMessage(0);
	}
	void SetHintHidePause( int v ) {}
	void SetShowHint( bool b ) {}
	void SetShowMainForm( bool b ) {}

	void Initialize() {}
	void ShowException( class Exception* e );
	void Run();

	HWND GetMainWindowHandle();

	int ArgC;
	char ** ArgV;
	std::vector<std::string> CommandLines;

	void PostMessageToMainWindow(UINT message, WPARAM wParam, LPARAM lParam);


	void ModalStarted() {}
	void ModalFinished() {}
	void DisableWindows();
	void EnableWindows( const std::vector<class TTVPWindowForm*>& ignores );
	void GetDisableWindowList( std::vector<class TTVPWindowForm*>& win );

	
	void RegisterAcceleratorKey(HWND hWnd, char virt, short key, short cmd);
	void UnregisterAcceleratorKey(HWND hWnd, short cmd);
	void DeleteAcceleratorKeyTable( HWND hWnd );
};
std::vector<std::string>* LoadLinesFromFile( const tstring& path );

// スタブ、正しくは動作しないはず。
inline HWND AllocateHWnd( LRESULT (CALLBACK *UtilWndProc)(HWND,UINT,WPARAM,LPARAM) ) {
	
	const TCHAR* classname = _T("TPUtilWindow");
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, UtilWndProc, 0L, 0L,
						GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
						classname, NULL };
	::RegisterClassEx( &wc );
	HWND hWnd = ::CreateWindow( classname, _T(""),
						WS_OVERLAPPEDWINDOW, 0, 0, 0, 0,
						NULL, NULL, wc.hInstance, NULL );
	return hWnd;
}
inline bool DeallocateHWnd( HWND hWnd ) {
	return 0 != ::DestroyWindow( hWnd );
}


inline HINSTANCE GetHInstance() { return ((HINSTANCE)GetModuleHandle(0)); }
extern class TApplication* Application;


#endif // __T_APPLICATION_H__
