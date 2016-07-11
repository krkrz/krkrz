
#ifndef __T_APPLICATION_H__
#define __T_APPLICATION_H__

#include <vector>
#include <map>
#include <stack>

std::wstring ExePath();

// ���ʂ��̂悢���@�ɕύX���������ǂ�
extern int _argc;
extern wchar_t ** _wargv;

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
	std::wstring console_title_;
	AcceleratorKeyTable accel_key_;
	bool tarminate_;
	bool application_activating_;
	bool has_map_report_process_;

	class tTVPAsyncImageLoader* image_load_thread_;

	std::stack<class tTVPWindow*> modal_window_stack_;

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
	bool StartApplication( int argc, wchar_t* argv[] );

	void PrintConsole( const wchar_t* mes, unsigned long len, bool iserror = false );
	bool IsAttachConsole() { return is_attach_console_; }

	bool IsTarminate() const { return tarminate_; }

	HWND GetHandle();
	bool IsIconic() {
		HWND hWnd = GetHandle();
		if( hWnd != INVALID_HANDLE_VALUE ) {
			return 0 != ::IsIconic(hWnd);
		}
		return true; // ���������E�B���h�E���Ȃ�
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
	wchar_t ** ArgV;

	void PostMessageToMainWindow(UINT message, WPARAM wParam, LPARAM lParam);


	void ModalStarted( class tTVPWindow* form ) {
		modal_window_stack_.push(form);
	}
	void ModalFinished();
	void OnActiveAnyWindow();
	void DisableWindows();
	void EnableWindows( const std::vector<class TTVPWindowForm*>& win );
	void GetDisableWindowList( std::vector<class TTVPWindowForm*>& win );
	void GetEnableWindowList( std::vector<class TTVPWindowForm*>& win, class TTVPWindowForm* activeWindow );

	
	void RegisterAcceleratorKey(HWND hWnd, char virt, short key, short cmd);
	void UnregisterAcceleratorKey(HWND hWnd, short cmd);
	void DeleteAcceleratorKeyTable( HWND hWnd );

	void OnActivate( HWND hWnd );
	void OnDeactivate( HWND hWnd );
	bool GetActivating() const { return application_activating_; }
	bool GetNotMinimizing() const;

	/**
	 * �摜�̔񓯊��Ǎ��ݗv��
	 */
	void LoadImageRequest( class iTJSDispatch2 *owner, class tTJSNI_Bitmap* bmp, const ttstr &name );
};
std::vector<std::string>* LoadLinesFromFile( const std::wstring& path );

inline HINSTANCE GetHInstance() { return ((HINSTANCE)GetModuleHandle(0)); }
extern class tTVPApplication* Application;


#endif // __T_APPLICATION_H__
