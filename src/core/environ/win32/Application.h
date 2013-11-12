
#ifndef __T_APPLICATION_H__
#define __T_APPLICATION_H__

#include <vector>
#include <map>

std::wstring ExePath();

// 後で見通しのよう方法に変更する
extern int _argc;
extern char ** _argv;

enum {
	mrOk,
	mrAbort,
	mrCancel,
};

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
class tTVPApplication {
	std::vector<class TTVPWindowForm*> windows_list_;
	std::wstring title_;

	bool is_attach_console_;
	FILE* newstdin_;
	FILE* newstdout_;
	FILE* newstderr_;
	std::wstring console_title_;
	AcceleratorKeyTable accel_key_;
	bool tarminate_;
	bool ApplicationActivating;

	class tTVPAsyncImageLoader* ImageLoadThread;

private:
	void CheckConsole();
	void CloseConsole();
	void CheckDigitizer();
	void ShowException( const wchar_t* e );
	void Initialize() {}
	void Run();

public:
	tTVPApplication();
	~tTVPApplication();
	bool StartApplication( int argc, char* argv[] );

	void PrintConsole( const wchar_t* mes, unsigned long len );
	bool IsAttachConsole() { return is_attach_console_; }

	bool IsTarminate() const { return tarminate_; }

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
	unsigned int GetWindowCount() const {
		return (unsigned int)windows_list_.size();
	}

	void FreeDirectInputDeviceForWindows();

	bool ProcessMessage( MSG &msg );
	void ProcessMessages();
	void HandleMessage();
	void HandleIdle(MSG &msg);

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


	HWND GetMainWindowHandle() const;

	int ArgC;
	char ** ArgV;
	std::vector<std::string> CommandLines;

	void PostMessageToMainWindow(UINT message, WPARAM wParam, LPARAM lParam);


	void ModalStarted() {}
	void ModalFinished() {}
	void DisableWindows();
	void EnableWindows( const std::vector<class TTVPWindowForm*>& win );
	void GetDisableWindowList( std::vector<class TTVPWindowForm*>& win );
	void GetEnableWindowList( std::vector<class TTVPWindowForm*>& win, class TTVPWindowForm* activeWindow );

	
	void RegisterAcceleratorKey(HWND hWnd, char virt, short key, short cmd);
	void UnregisterAcceleratorKey(HWND hWnd, short cmd);
	void DeleteAcceleratorKeyTable( HWND hWnd );

	void OnActivate( HWND hWnd );
	void OnDeactivate( HWND hWnd );
	bool GetActivating() const { return ApplicationActivating; }
	bool GetNotMinimizing() const;

	/**
	 * 画像の非同期読込み要求
	 */
	void LoadImageRequest( class iTJSDispatch2 *owner, class tTJSNI_Bitmap* bmp, const ttstr &name );
};
std::vector<std::string>* LoadLinesFromFile( const std::wstring& path );

inline HINSTANCE GetHInstance() { return ((HINSTANCE)GetModuleHandle(0)); }
extern class tTVPApplication* Application;


#endif // __T_APPLICATION_H__
