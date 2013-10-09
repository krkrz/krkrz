
#ifndef __T_APPLICATION_H__
#define __T_APPLICATION_H__

#include <vector>
#include <map>

//tstring ParamStr( int index );
std::wstring ExePath();

// 後で見通しのよう方法に変更する
extern int _argc;
extern char ** _argv;

enum {
	mrOk,
	mrAbort,
	mrCancel,
};

/*
MB_ABORTRETRYIGNORE	メッセージボックスに［中止］、［再試行］、［無視］の各プッシュボタンを表示します。
MB_CANCELTRYCONTINUE	Windows 2000：メッセージボックスに［キャンセル］、［再実行］、［続行］の各プッシュボタンを表示します。MB_ABORTRETRYIGNORE の代わりに、このメッセージボックスタイプを使ってください。
MB_HELP	Windows 95/98、Windows NT 4.0 以降：メッセージボックスに［ヘルプ］ボタンを追加します。ユーザーが［ヘルプ］ボタンをクリックするか F1 キーを押すと、システムはオーナーへ メッセージを送信します。
MB_OK	メッセージボックスに［OK］プッシュボタンだけを表示します。これは既定のメッセージボックスタイプです。
MB_OKCANCEL	メッセージボックスに［OK］、［キャンセル］の各プッシュボタンを表示します。
MB_RETRYCANCEL	メッセージボックスに［再試行］、［キャンセル］の各プッシュボタンを表示します。
MB_YESNO	メッセージボックスに［はい］、［いいえ］の各プッシュボタンを表示します。
MB_YESNOCANCEL	メッセージボックスに［はい］、［いいえ］、［キャンセル］の各プッシュボタンを表示します。
*/

enum {
  mtWarning = MB_ICONWARNING,
  mtError = MB_ICONERROR,
  mtInformation = MB_ICONINFORMATION,
  mtConfirmation = MB_ICONQUESTION,
  mtStop = MB_ICONSTOP,
  mtCustom = 0
};
enum {
	mbOK = MB_OK,
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
	std::vector<class TTVPWindowForm*> windows_list_;
	std::wstring title_;

	bool is_attach_console_;
	FILE* oldstdin_;
	FILE* oldstdout_;
	std::wstring console_title_;
	AcceleratorKeyTable accel_key_;
	bool tarminate_;

public:
	TApplication() : is_attach_console_(false), tarminate_(false) {}
	void CheckConsole();
	void CloseConsole();
	void PrintConsole( const wchar_t* mes, unsigned long len );
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

	bool ProcessMessage( MSG &msg );
	void ProcessMessages();
	void HandleMessage();

	std::wstring GetTitle() const { return title_; }
	void SetTitle( const std::wstring& caption );

	static inline int MessageDlg( const std::wstring& string, const std::wstring& caption, int type, int button ) {
		return ::MessageBox( NULL, string.c_str(), caption.c_str(), type|button  );
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
std::vector<std::string>* LoadLinesFromFile( const std::wstring& path );
/*
// スタブ、正しくは動作しないはず。
inline HWND AllocateHWnd( LRESULT (CALLBACK *UtilWndProc)(HWND,UINT,WPARAM,LPARAM) ) {
	
	const wchar_t* classname = TJS_W("TPUtilWindow");
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
*/

inline HINSTANCE GetHInstance() { return ((HINSTANCE)GetModuleHandle(0)); }
extern class TApplication* Application;


#endif // __T_APPLICATION_H__
