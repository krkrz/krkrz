#include "IrrlichtWindow.h"

#define CLASSNAME "IrrlichtWindow"
#define KRKRDISPWINDOWCLASS "TScrollBox"

extern void message_log(const char* format, ...);
extern void error_log(const char *format, ...);

using namespace irr;
using namespace core;
using namespace video;
using namespace scene;
using namespace io;
using namespace gui;

/**
 * ウインドウプロシージャ
 */
LRESULT CALLBACK
IrrlichtWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// 参考: irrlicht/source/Irrlicht/CIrrDeviceWin32.cpp
	// ここに処理をいろいろ書く必要あり

	#ifndef WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL 0x020A
	#endif
	#ifndef WHEEL_DELTA
	#define WHEEL_DELTA 120
	#endif

	
	IrrlichtWindow *self = (IrrlichtWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	irr::SEvent event;
	static irr::s32 ClickCount=0;
	if (GetCapture() != hWnd && ClickCount > 0) {
		ClickCount = 0;
	}

	
	switch (message) {
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			if (self) {
				self->show();
			}
			EndPaint(hWnd, &ps);
		}
		return 0;
	case WM_ERASEBKGND:
		return 0;

		// キーイベントはフォーカスがあってないかどうかでこない模様
	case WM_KEYDOWN:
	case WM_KEYUP:
		if (self) {
			if (self->transparentEvent) {
				self->sendMessage(message, wParam, lParam);
			} else {

				event.EventType = irr::EET_KEY_INPUT_EVENT;
				event.KeyInput.Key = (irr::EKEY_CODE)wParam;
				event.KeyInput.PressedDown = (message==WM_KEYDOWN);
				
				WORD KeyAsc=0;
				BYTE allKeys[256];
				GetKeyboardState(allKeys);
				ToAscii((UINT)wParam,(UINT)lParam,allKeys,&KeyAsc,0);
				
				event.KeyInput.Shift = ((allKeys[VK_SHIFT] & 0x80)!=0);
				event.KeyInput.Control = ((allKeys[VK_CONTROL] & 0x80)!=0);
				event.KeyInput.Char = (KeyAsc & 0x00ff); //KeyAsc >= 0 ? KeyAsc : 0;
				
				self->postEvent(event);
			}
			return 0;
		}
		break;
		
	case WM_MOUSEWHEEL:
		if (self) {
			if (self->transparentEvent) {
				self->sendMessage(message, wParam, lParam);
			} else {
				event.EventType = irr::EET_MOUSE_INPUT_EVENT;
				event.MouseInput.Wheel = ((irr::f32)((short)HIWORD(wParam))) / (irr::f32)WHEEL_DELTA;
				event.MouseInput.Event = irr::EMIE_MOUSE_WHEEL;
				POINT p;
				p.x = 0; p.y = 0;
				ClientToScreen(hWnd, &p);
				event.MouseInput.X = LOWORD(lParam) - p.x;
				event.MouseInput.Y = HIWORD(lParam) - p.y;
				self->postEvent(event);
			}
			return 0;
		}
		break;

	case WM_LBUTTONDOWN:
		if (self) {
			if (self->transparentEvent) {
				self->sendMessage(message, wParam, lParam, true);
			} else {
				ClickCount++;
				SetCapture(hWnd);
				event.EventType = irr::EET_MOUSE_INPUT_EVENT;
				event.MouseInput.Event = irr::EMIE_LMOUSE_PRESSED_DOWN;
				event.MouseInput.X = (short)LOWORD(lParam);
				event.MouseInput.Y = (short)HIWORD(lParam);
				self->postEvent(event);
			}
			return 0;
		}
		break;

	case WM_LBUTTONUP:
		if (self) {
			if (self->transparentEvent) {
				self->sendMessage(message, wParam, lParam, true);
			} else {
				ClickCount--;
				if (ClickCount<1) {
					ClickCount=0;
					ReleaseCapture();
				}
				event.EventType = irr::EET_MOUSE_INPUT_EVENT;
				event.MouseInput.Event = irr::EMIE_LMOUSE_LEFT_UP;
				event.MouseInput.X = (short)LOWORD(lParam);
				event.MouseInput.Y = (short)HIWORD(lParam);
				self->postEvent(event);
			}
			return 0;
		}
		break;

	case WM_RBUTTONDOWN:
		if (self) {
			if (self->transparentEvent) {
				self->sendMessage(message, wParam, lParam, true);
			} else {
				ClickCount++;
				SetCapture(hWnd);
				event.EventType = irr::EET_MOUSE_INPUT_EVENT;
				event.MouseInput.Event = irr::EMIE_RMOUSE_PRESSED_DOWN;
				event.MouseInput.X = (short)LOWORD(lParam);
				event.MouseInput.Y = (short)HIWORD(lParam);
				self->postEvent(event);
			}
			return 0;
		}
		break;

	case WM_RBUTTONUP:
		if (self) {
			if (self->transparentEvent) {
				self->sendMessage(message, wParam, lParam, true);
			} else {
				ClickCount--;
				if (ClickCount<1) {
					ClickCount=0;
					ReleaseCapture();
				}
				event.EventType = irr::EET_MOUSE_INPUT_EVENT;
				event.MouseInput.Event = irr::EMIE_RMOUSE_LEFT_UP;
				event.MouseInput.X = (short)LOWORD(lParam);
				event.MouseInput.Y = (short)HIWORD(lParam);
				self->postEvent(event);
			}
			return 0;
		}
		break;

	case WM_MBUTTONDOWN:
		if (self) {
			if (self->transparentEvent) {
				self->sendMessage(message, wParam, lParam, true);
			} else {
				ClickCount++;
				SetCapture(hWnd);
				event.EventType = irr::EET_MOUSE_INPUT_EVENT;
				event.MouseInput.Event = irr::EMIE_MMOUSE_PRESSED_DOWN;
				event.MouseInput.X = (short)LOWORD(lParam);
				event.MouseInput.Y = (short)HIWORD(lParam);
				self->postEvent(event);
			}
			return 0;
		}
		break;

	case WM_MBUTTONUP:
		if (self) {
			if (self->transparentEvent) {
				self->sendMessage(message, wParam, lParam, true);
			} else {
				ClickCount--;
				if (ClickCount<1) {
					ClickCount=0;
					ReleaseCapture();
				}
				event.EventType = irr::EET_MOUSE_INPUT_EVENT;
				event.MouseInput.Event = irr::EMIE_MMOUSE_LEFT_UP;
				event.MouseInput.X = (short)LOWORD(lParam);
				event.MouseInput.Y = (short)HIWORD(lParam);
				self->postEvent(event);
			}
			return 0;
		}
		break;

	case WM_MOUSEMOVE:
		if (self) {
			if (self->transparentEvent) {
				self->sendMessage(message, wParam, lParam, true);
			} else {
				event.EventType = irr::EET_MOUSE_INPUT_EVENT;
				event.MouseInput.Event = irr::EMIE_MOUSE_MOVED;
				event.MouseInput.X = (short)LOWORD(lParam);
				event.MouseInput.Y = (short)HIWORD(lParam);
				self->postEvent(event);
			}
			return 0;
		}
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void registerWindowClass()
{
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof wcex);
	wcex.cbSize		= sizeof(WNDCLASSEX);
	wcex.style		= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)IrrlichtWindow::WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= GetModuleHandle(NULL);
	wcex.hIcon		    = NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= CLASSNAME;
	wcex.hIconSm		= 0;
	RegisterClassEx(&wcex);
}

void unregisterWindowClass()
{
	UnregisterClass(CLASSNAME, GetModuleHandle(NULL));
}



// イベント処理
bool __stdcall
IrrlichtWindow::messageHandler(void *userdata, tTVPWindowMessage *Message)
{
	IrrlichtWindow *self = (IrrlichtWindow*)userdata;
	switch (Message->Msg) {
	case TVP_WM_DETACH:
		self->destroyWindow();
		break;
	case TVP_WM_ATTACH:
		self->createWindow((HWND)Message->LParam);
		break;
	default:
		break;
	}
	return false;
}

// ユーザメッセージレシーバの登録/解除
void
IrrlichtWindow::setReceiver(tTVPWindowMessageReceiver receiver, bool enable)
{
	tTJSVariant mode     = enable ? (tTVInteger)(tjs_int)wrmRegister : (tTVInteger)(tjs_int)wrmUnregister;
	tTJSVariant proc     = (tTVInteger)(tjs_int)receiver;
	tTJSVariant userdata = (tTVInteger)(tjs_int)this;
	tTJSVariant *p[] = {&mode, &proc, &userdata};
	if (window->FuncCall(0, L"registerMessageReceiver", NULL, NULL, 4, p, window) != TJS_S_OK) {
		if (enable) {
			TVPThrowExceptionMessage(L"can't regist user message receiver");
		}
	}
}

/**
 * ウインドウを生成
 * @param krkr 親ウインドウ
 */
void
IrrlichtWindow::createWindow(HWND krkr)
{
	if (krkr && (parent = ::FindWindowEx(krkr, NULL, KRKRDISPWINDOWCLASS, NULL))) {
		hwnd = CreateWindow(CLASSNAME, "",
							WS_CHILD|WS_CLIPCHILDREN,
							left, top, width, height,
							parent, NULL, GetModuleHandle(NULL), NULL);
		if (hwnd) {
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
			_setPos();
			start();
		}
	}
}

/**
 * ウインドウを破棄
 */
void
IrrlichtWindow::destroyWindow()
{
	stop();
	detach();
	if (hwnd) {
		DestroyWindow(hwnd);
		hwnd = 0;
	}
	parent = 0;
}

/**
 * 親窓にメッセージを送付
 * @param message メッセージ
 * @param wParam WPARAM
 * @param lParam LPARAM
 * @param convPosition lParam のマウス座標値を親のものに変換
 */
void
IrrlichtWindow::sendMessage(UINT message, WPARAM wParam, LPARAM lParam, bool convPosition)
{
	if (parent) {
		if (convPosition) {
			POINT ps = {0,0}, pp = {0,0};
			ClientToScreen(hwnd, &ps);
			ClientToScreen(parent, &pp);
			DWORD x = LOWORD(lParam) + ps.x - pp.x;
			DWORD y = HIWORD(lParam) + ps.y - pp.y;
			lParam = (LPARAM)((DWORD)y << 16 | (DWORD)x);
		}
		SendMessage(parent, message, wParam, lParam);
	}
}

/**
 * コンストラクタ
 */
IrrlichtWindow::IrrlichtWindow(iTJSDispatch2 *objthis, iTJSDispatch2 *win, int left, int top, int width, int height)
	: IrrlichtBase(objthis), window(NULL), parent(0), hwnd(0), visible(false), transparentEvent(true)
{
	window = win;
	window->AddRef();
	setReceiver(messageHandler, true);
	
	tTJSVariant krkrHwnd; // 親のハンドル
	if (window->PropGet(0, TJS_W("HWND"), NULL, &krkrHwnd, window) == TJS_S_OK) {
		HWND parent = ::FindWindowEx((HWND)(tjs_int)krkrHwnd, NULL, KRKRDISPWINDOWCLASS, NULL);
		if (parent) {
			RECT rect;
			GetClientRect(parent, &rect);
			this->left   = 0;
			this->top    = 0;
			this->width  = rect.right - rect.left;
			this->height = rect.bottom - rect.top;
		}
	}
	this->left   = left;
	this->top    = top;
	if (width != 0) {
		this->width  = width;
	}
	if (height != 0) {
		this->height = height;
	}
	createWindow((HWND)(tjs_int)krkrHwnd);
}

/**
 * デストラクタ
 */
IrrlichtWindow::~IrrlichtWindow()
{
	destroyWindow();
	if (window) {
		setReceiver(messageHandler, false);
		window->Release();
		window = NULL;
	}
}

/**
 * 生成ファクトリ
 */
tjs_error
IrrlichtWindow::Factory(IrrlichtWindow **obj, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	if (numparams < 5) {
		return TJS_E_BADPARAMCOUNT;
	}
	iTJSDispatch2 *window = param[0]->AsObjectNoAddRef();
	if (window == NULL || window->IsInstanceOf(0, NULL, NULL, L"Window", window) != TJS_S_TRUE) {
		TVPThrowExceptionMessage(L"must set Window object");
	}
	int left   = (tjs_int)*param[1];
	int top    = (tjs_int)*param[2];
	int width  = (tjs_int)*param[3];
	int height = (tjs_int)*param[4];

	*obj = new IrrlichtWindow(objthis, window, left, top, width, height);
	return TJS_S_OK;
}

// -----------------------------------------------------------------------
// Continuous
// -----------------------------------------------------------------------

/**
 * Continuous コールバック
 * 吉里吉里が暇なときに常に呼ばれる
 * これが事実上のメインループになる
 */
void TJS_INTF_METHOD
IrrlichtWindow::OnContinuousCallback(tjs_uint64 tick)
{
	if (hwnd) {
		attach(hwnd);
		InvalidateRect(hwnd, NULL, false);
	}
}

// -----------------------------------------------------------------------
// 表示指定
// -----------------------------------------------------------------------

void
IrrlichtWindow::_setPos()
{
	if (hwnd) {
		MoveWindow(hwnd, left, top, width, height, FALSE);
		ShowWindow(hwnd, visible);
	}
}
	
void
IrrlichtWindow::setVisible(bool v)
{
	visible = v;
	_setPos();
}

bool
IrrlichtWindow::getVisible()
{
	return visible;
}

void
IrrlichtWindow::setLeft(int l)
{
	left = l;
	_setPos();
}

int
IrrlichtWindow::getLeft()
{
	return left;
}

void
IrrlichtWindow::setTop(int t)
{
	top = t;
	_setPos();
}

int
IrrlichtWindow::getTop()
{
	return top;
}

void
IrrlichtWindow::setWidth(int w)
{
	width = w;
	_setPos();
}

int
IrrlichtWindow::getWidth()
{
	return width;
}

void
IrrlichtWindow::setHeight(int h)
{
	height = h;
	_setPos();
}

int
IrrlichtWindow::getHeight()
{
	return height;
}
	
/**
 * 窓場所指定
 */	
void
IrrlichtWindow::setPos(int l, int t)
{
	left = l;
	top  = t;
	_setPos();
}

/**
 * 窓サイズ指定
 */	
void
IrrlichtWindow::setSize(int w, int h)
{
	width = w;
	height = h;
	_setPos();
}

