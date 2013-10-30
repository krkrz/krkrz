
#ifndef __TVP_WINDOW_H__
#define __TVP_WINDOW_H__

#include <string>
#include <shellapi.h>
#include <oleidl.h> // for MK_ALT
#include "tvpinputdefs.h"

#include "ImeControl.h"

#ifndef MK_ALT 
#define MK_ALT (0x20)
#endif

enum {
	 ssShift = (1<<0),
	 ssAlt=(1<<1),
	 ssCtrl=(1<<2),
};

class tTVPWindow {
	WNDCLASSEX			wc_;
	bool	created_;

protected:
	enum CloseAction {
	  caNone,
	  caHide,
	  caFree,
	  caMinimize
	};

	HWND				window_handle_;

	std::wstring	window_class_name_;
	std::wstring	window_title_;
	SIZE		window_client_size_;
	SIZE		min_size_;
	SIZE		max_size_;
	int			border_style_;
	bool		in_window_;
	bool		ignore_touch_mouse_;

	bool InMode;
	int ModalResult;

	static const UINT SIZE_CHANGE_FLAGS;
	static const UINT POS_CHANGE_FLAGS;
	static const DWORD DEFAULT_EX_STYLE;

	bool LeftDoubleClick;

	ImeControl* ime_control_;

	enum ModeFlag {
		FALG_FULLSCREEN = 0x01,
	};
	
	unsigned long flags_;
	void SetFlag( unsigned long f ) {
		flags_ |= f;
	}
	void ResetFlag( unsigned long f ) {
		flags_ &= ~f;
	}
	bool GetFlag( unsigned long f ) {
		return 0 != (flags_ & f);
	}
	
	void UnregisterWindow();

	// window procedure
	static LRESULT WINAPI WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

	virtual LRESULT WINAPI Proc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

	HRESULT CreateWnd( const std::wstring& classname, const std::wstring& title, int width, int height );

	virtual void OnDestroy();
	virtual void OnPaint();

	inline int GetAltKeyState() const {
		if( ::GetKeyState( VK_MENU  ) ) {
			return MK_ALT;
		} else {
			return 0;
		}
	}
	inline int GetShiftState( int wParam ) const {
		int shift = GET_KEYSTATE_WPARAM(wParam) & (MK_SHIFT|MK_CONTROL);
		shift |= GetAltKeyState();
		return shift;
	}
	inline int GetShiftState() const {
		int shift = 0;
		if( ::GetKeyState( VK_MENU  ) ) shift |= MK_ALT;
		if( ::GetKeyState( VK_SHIFT ) ) shift |= MK_SHIFT;
		if( ::GetKeyState( VK_CONTROL ) ) shift |= MK_CONTROL;
		return shift;
	}
	
	void SetMouseCapture() {
		::SetCapture( GetHandle() );
	}
	void ReleaseMouseCapture() {
		::ReleaseCapture();
	}
	HICON GetBigIcon();
public:
	tTVPWindow()
	: window_handle_(NULL), created_(false), LeftDoubleClick(false), ime_control_(NULL), border_style_(0), ModalResult(0),
		in_window_(false), ignore_touch_mouse_(false) {
		min_size_.cx = min_size_.cy = 0;
		max_size_.cx = max_size_.cy = 0;
	}
	virtual ~tTVPWindow();

	bool HasFocus() const {
		return window_handle_ == ::GetFocus();
	}
	bool IsValidHandle() const {
		return ( window_handle_ != NULL && window_handle_ != INVALID_HANDLE_VALUE && ::IsWindow(window_handle_) );
	}

	virtual bool Initialize();

	void SetWidnowTitle( const std::wstring& title );
	void SetScreenSize( int width, int height );

	HWND GetHandle() { return window_handle_; }
	HWND GetHandle() const { return window_handle_; }

	ImeControl* GetIME() { return ime_control_; }
	const ImeControl* GetIME() const { return ime_control_; }

	static void SetClientSize( HWND hWnd, SIZE& size );

//-- properties
	bool GetVisible() const;
	void SetVisible(bool s);
	void Show() { SetVisible( true ); }
	void Hide() { SetVisible( false ); }

	bool GetEnable() const;
	void SetEnable( bool s );

	void GetCaption( std::wstring& v ) const;
	void SetCaption( const std::wstring& v );
	
	void SetBorderStyle( enum tTVPBorderStyle st);
	enum tTVPBorderStyle GetBorderStyle() const;

	void SetWidth( int w );
	int GetWidth() const;
	void SetHeight( int h );
	int GetHeight() const;
	void SetSize( int w, int h );
	void GetSize( int &w, int &h );

	void SetLeft( int l );
	int GetLeft() const;
	void SetTop( int t );
	int GetTop() const;
	void SetPosition( int l, int t );
	
	void SetBounds( int x, int y, int width, int height );

	void SetMinWidth( int v ) {
		if( GetWidth() < v ) {
			SetWidth( v );
		}
		min_size_.cx = v;
	}
	int GetMinWidth() const{ return min_size_.cx; }
	void SetMinHeight( int v ) {
		if( GetHeight() < v ) {
			SetHeight( v );
		}
		min_size_.cy = v;
	}
	int GetMinHeight() const { return min_size_.cy; }
	void SetMinSize( int w, int h ) {
		int dw, dh;
		GetSize( dw, dh );
		if( dw < w || dh < h ) {
			if( dw < w ) dw = w;
			if( dh < h ) dh = h;
			SetSize( dw, dh );
		}
		min_size_.cx = w;
		min_size_.cy = h;
	}

	void SetMaxWidth( int v ) {
		if( GetWidth() > v ) {
			SetWidth( v );
		}
		max_size_.cx = v;
	}
	int GetMaxWidth() const{ return max_size_.cx; }
	void SetMaxHeight( int v ) {
		if( GetHeight() > v ) {
			SetHeight( v );
		}
		max_size_.cy = v;
	}
	int GetMaxHeight() const{ return max_size_.cy; }
	void SetMaxSize( int w, int h ) {
		int dw, dh;
		GetSize( dw, dh );
		if( dw < w || dh < h ) {
			if( dw > w ) dw = w;
			if( dh > h ) dh = h;
			SetSize( dw, dh );
		}
		max_size_.cx = w;
		max_size_.cy = h;
	}

	void SetInnerWidth( int w );
	int GetInnerWidth() const;
	void SetInnerHeight( int h );
	int GetInnerHeight() const;
	void SetInnerSize( int w, int h );
	
	void BringToFront();
	void SetStayOnTop( bool b );
	bool GetStayOnTop() const;

	/*
	void SetFullScreenMode(bool b);
	inline bool GetFullScreenMode() const {
		return 0 != (flags_&FALG_FULLSCREEN);
	}
	*/

	int ShowModal();
	void Close();

	void GetClientRect( struct tTVPRect& rt );

	// メッセージハンドラ
	virtual void OnActive( HWND preactive ) {}
	virtual void OnDeactive( HWND postactive ) {}
	virtual void OnClose( CloseAction& action ){}
	virtual bool OnCloseQuery() { return true; }
	virtual void OnFocus(HWND hFocusLostWnd) {}
	virtual void OnFocusLost(HWND hFocusingWnd) {}
	virtual void OnMouseDown( int button, int shift, int x, int y ){}
	virtual void OnMouseUp( int button, int shift, int x, int y ){}
	virtual void OnMouseMove( int shift, int x, int y ){}
	virtual void OnMouseDoubleClick( int button, int x, int y ) {}
	virtual void OnMouseClick( int button, int shift, int x, int y ){}
	virtual void OnMouseWheel( int delta, int shift, int x, int y ){}
	virtual void OnKeyUp( WORD vk, int shift ){}
	virtual void OnKeyDown( WORD vk, int shift, int repreat, bool prevkeystate ){}
	virtual void OnKeyPress( WORD vk, int repreat, bool prevkeystate, bool convertkey ){}
	virtual void OnMove( int x, int y ) {}
	virtual void OnDropFile( HDROP hDrop ) {}
	virtual int OnMouseActivate( HWND hTopLevelParentWnd, WORD hitTestCode, WORD MouseMsg ) { return MA_ACTIVATE; }
	virtual void OnEnable( bool enabled ) {}
	virtual void OnEnterMenuLoop( bool entered ) {}
	virtual void OnExitMenuLoop( bool isShortcutMenu ) {}
	virtual void OnDeviceChange( int event, void *data ) {}
	virtual void OnNonClientMouseDown( int button, int hittest, int x, int y ){}
	virtual void OnMouseEnter() {}
	virtual void OnMouseLeave() {}
	virtual void OnShow( int status ) {}
	virtual void OnHide( int status ) {}

	virtual void OnTouchDown( double x, double y, double cx, double cy, DWORD id ) {}
	virtual void OnTouchMove( double x, double y, double cx, double cy, DWORD id ) {}
	virtual void OnTouchUp( double x, double y, double cx, double cy, DWORD id ) {}
};

#endif // __TVP_WINDOW_H__
