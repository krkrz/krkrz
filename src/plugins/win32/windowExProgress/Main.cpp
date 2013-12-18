#include <windows.h>
#include <tchar.h>
#include <process.h>
#include <commctrl.h>
#include <vector>
#include "ncbind/ncbind.hpp"

#define CLASSNAME _T("WindowExProgress")
#define KRKRDISPWINDOWCLASS _T("TScrollBox")

#ifndef ID_CANCEL
#define ID_CANCEL 3
#endif

/**
 * 表示画像情報
 */
class ImageInfo {

public:
	ImageInfo() : bitmap(0), bmpdc(0), color(0), left(0), top(0), width(0), height(0) {
	}

	~ImageInfo() {
		removeBitmap();
	}

	// 描画処理
	void show(HDC dc, PAINTSTRUCT &ps) {
		if (bitmap) {
			// ビットマップ描画
			::BitBlt(dc, left, top, width, height, bmpdc, 0, 0, SRCCOPY);
		} else {
			// 矩形描画
			if (width > 0 && height > 0) {
				SelectObject(dc, CreateSolidBrush(color));
				::Rectangle(dc, left, top, width, height);
				DeleteObject(SelectObject(dc, GetStockObject(WHITE_BRUSH)));
			}
		}
	}
	
	/**
	 * ビットマップを設定
	 */
	void setColor(int left, int top, int width, int height, int color) {
		removeBitmap();
		this->left   = left;
		this->top    = top;
		this->width  = width;
		this->height = height;
		this->color  = color;
	}
	
	/**
	 * ビットマップを設定
	 */
	bool setBitmap(int left, int top, iTJSDispatch2 *lay) {
		
		typedef unsigned char PIX;
		if (!lay || !lay->IsInstanceOf(0, 0, 0, TJS_W("Layer"), lay)) return false;
		
		this->left = left;
		this->top  = top;
		ncbPropAccessor obj(lay);
		width  = obj.getIntValue(TJS_W("imageWidth"));
		height = obj.getIntValue(TJS_W("imageHeight"));
		tjs_int ln = obj.getIntValue(TJS_W("mainImageBufferPitch"));
		PIX *pw, *pr = reinterpret_cast<unsigned char *>(obj.getIntValue(TJS_W("mainImageBuffer")));
		
		BITMAPINFO info;
		ZeroMemory(&info, sizeof(info));
		info.bmiHeader.biSize = sizeof(BITMAPINFO);
		info.bmiHeader.biWidth  = width;
		info.bmiHeader.biHeight = height;
		info.bmiHeader.biPlanes = 1;
		info.bmiHeader.biBitCount = 24;
		
		removeBitmap();
		bmpdc  = ::CreateCompatibleDC(NULL);
		bitmap = ::CreateDIBSection(bmpdc, (LPBITMAPINFO)&info, DIB_RGB_COLORS, (LPVOID*)&pw, NULL, 0);
		
		if (!bitmap || !bmpdc) return false;
		for (int y = height-1; y >= 0; y--) {
			PIX *src = pr + (y * ln);
			PIX *dst = pw + ((height-1 - y) * ((width*3+3) & ~3L));
			for (int n = width-1; n >= 0; n--, src+=4, dst+=3) {
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
			}
		}
		::SelectObject(bmpdc, bitmap);
		return true;
	}

protected:
	void removeBitmap() {
		if (bmpdc) {
			::DeleteDC(bmpdc);
			bmpdc  = NULL;
		}
		if (bitmap) {
			::DeleteObject(bitmap);
			bitmap = NULL;
		}
	}
	
	HBITMAP bitmap;
	HDC bmpdc;
	HBRUSH brush;
	tjs_int left, top, width, height;
	int color;
};


/**
 * 表示メッセージ情報
 */
class MessageInfo {
	
public:

	MessageInfo() : left(0), top(0), size(0), color(0xffffff), useShadow(false), shadowColor(0), shadowDistanceX(1), shadowDistanceY(1) {
	}

	~MessageInfo() {
	}

	bool setText(iTJSDispatch2 *init) {
		ncbPropAccessor info(init);
#define GETINTVALUE(a,def) a = info.getIntValue(L#a, def)
#define GETBOOLVALUE(a,def) a = info.getIntValue(L#a,def?1:0) != 0
#define GETSTRVALUE(a,def) a = info.getStrValue(L#a,def)

		GETINTVALUE(left, 0);
		GETINTVALUE(top, 0);
		GETINTVALUE(size, 12);
		GETINTVALUE(color, 0xffffff);
		GETINTVALUE(shadowColor,-1);
		GETINTVALUE(shadowDistanceX,1);
		GETINTVALUE(shadowDistanceY,1);
		useShadow = shadowColor > 0;
	}

	void show(HDC dc, PAINTSTRUCT &ps) {
		if (useShadow) {
//			OutputText(left+shadowDistanceX, top+shadowDistanceY, text.c_str());
		}
//		OutputText(left, top, text.c_str());
	}

protected:
	int left;
	int top;
	ttstr text;
	int size;
	int color;
	bool useShadow;
	int shadowColor;
	int shadowDistanceX;
	int shadowDistanceY;
	HFONT font;
};

/**
 * セーブ処理スレッド用情報
 * プログレス処理を実行するウインドウ
 */
class ProgressWindow {

public:
	/**
	 * ウインドウクラスの登録
	 */
	static void registerWindowClass() {
		WNDCLASSEX wcex;
		ZeroMemory(&wcex, sizeof wcex);
		wcex.cbSize		= sizeof(WNDCLASSEX);
		wcex.style		= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= (WNDPROC)WndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= GetModuleHandle(NULL);
		wcex.hIcon		    = NULL;
		wcex.hCursor		= LoadCursor(NULL, IDC_WAIT);
		wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszMenuName	= 0;
		wcex.lpszClassName	= CLASSNAME;
		wcex.hIconSm		= 0;
		RegisterClassEx(&wcex);
	}

	/**
	 * ウインドウクラスの削除
	 */
	static void unregisterWindowClass() {
		UnregisterClass(CLASSNAME, GetModuleHandle(NULL));
	}

	/**
	 * コンストラクタ
	 */
	ProgressWindow(iTJSDispatch2 *window, iTJSDispatch2 *init) : window(window), hParent(0), hWnd(0), thread(0), doneflag(false), cancelflag(false), percent(0),
	progressBarEnable(true), progressBarHandle(0), progressBarStyle(0),
	progressBarLeft(-1), progressBarTop(-1), progressBarWidth(-1), progressBarHeight(-1),
	progressBarColor(0xff000000), progressBarBackColor(0xff000000),
	cancelButtonEnable(true), cancelButtonHandle(0), cancelButtonCaption(L"Cancel"),
	cancelButtonLeft(-1), cancelButtonTop(-1), cancelButtonWidth(-1), cancelButtonHeight(-1) {
		prepare = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (init) {
			ncbPropAccessor info(init);
			GETINTVALUE(progressBarStyle, 0);
			GETINTVALUE(progressBarTop, -1);
			GETINTVALUE(progressBarLeft, -1);
			GETINTVALUE(progressBarWidth, -1);
			GETINTVALUE(progressBarHeight, -1);
			GETINTVALUE(progressBarColor, 0xff000000);
			GETINTVALUE(progressBarBackColor, 0xff000000);
			GETBOOLVALUE(progressBarEnable, true);
			GETSTRVALUE(cancelButtonCaption, ttstr("Cancel"));
			GETINTVALUE(cancelButtonLeft, -1);
			GETINTVALUE(cancelButtonTop, -1);
			GETINTVALUE(cancelButtonWidth, -1);
			GETINTVALUE(cancelButtonHeight, -1);
			GETBOOLVALUE(cancelButtonEnable, true);
		}
		setReceiver(true);
		start();
	}

	/**
	 * デストラクタ
	 */
	~ProgressWindow() {
		CloseHandle(prepare);
		setReceiver(false);
		end();
	}
	
	/**
	 * プログレス通知
	 * @return キャンセルされてたら true
	 */
	bool doProgress(int percent) {
		if (percent != this->percent) {
			this->percent = percent;
			if (progressBarHandle) {
				SendMessage(progressBarHandle, PBM_SETPOS, (WPARAM)percent, 0 );
			}
		}
		return !hWnd || cancelflag;
	}

	/**
	 * プログレス処理のテキストを差し替える
	 * @param name 識別名
	 * @param text 表示テキスト
	 */
	void setProgressMessage(const tjs_char *name, const tjs_char *text) {
	}
	
protected:
	iTJSDispatch2 *window; //< 親ウインドウ
	HWND hParent; //< 親ハンドル
	HWND hWnd; //< 自分のハンドル
	HANDLE thread; //< プログレス処理のスレッド
	HANDLE prepare; //< 準備待ちイベント
	bool doneflag;   // 終了フラグ
	bool cancelflag; // キャンセルフラグ
	int percent; // パーセント指定

	bool progressBarEnable;
	HWND progressBarHandle; //< プログレスバーのハンドラ
	int progressBarLeft;
	int progressBarStyle;
	int progressBarTop;
	int progressBarWidth;
	int progressBarHeight;
	int progressBarColor;
	int progressBarBackColor;

	bool cancelButtonEnable;
	HWND cancelButtonHandle;
	ttstr cancelButtonCaption;
	int cancelButtonLeft;
	int cancelButtonTop;
	int cancelButtonWidth;
	int cancelButtonHeight;

	ImageInfo *backGround;
	
	/**
	 * ウインドウプロシージャ
	 */
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		ProgressWindow *self = (ProgressWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (self) {
			switch (message) {
			case WM_PAINT: // 画面更新
				{
					PAINTSTRUCT ps;
					HDC dc = BeginPaint(hWnd, &ps);
					self->show(dc, ps);
					EndPaint(hWnd, &ps);
				}
				return 0;
			case WM_COMMAND: // キャンセル通知
				switch (wParam) {
				case ID_CANCEL:
					self->cancel();
					break;
				}
				break;
			}
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	
	/*
	 * ウインドウイベント処理レシーバ
	 */
	static bool __stdcall receiver(void *userdata, tTVPWindowMessage *Message) {
		ProgressWindow *self = (ProgressWindow*)userdata;
		switch (Message->Msg) {
		case TVP_WM_ATTACH:
			self->start();
			break;
		case TVP_WM_DETACH:
			self->end();
			break;
		default:
			break;
		}
		return false;
	}

	// ユーザメッセージレシーバの登録/解除
	void setReceiver(bool enable) {
		tTJSVariant mode     = enable ? (tTVInteger)(tjs_int)wrmRegister : (tTVInteger)(tjs_int)wrmUnregister;
		tTJSVariant proc     = (tTVInteger)(tjs_int)receiver;
		tTJSVariant userdata = (tTVInteger)(tjs_int)this;
		tTJSVariant *p[] = {&mode, &proc, &userdata};
		if (window->FuncCall(0, L"registerMessageReceiver", NULL, NULL, 4, p, window) != TJS_S_OK) {
			TVPThrowExceptionMessage(L"can't regist user message receiver");
		}
	}
	
	// 実行スレッド
	static unsigned __stdcall threadFunc(void *data) {
		((ProgressWindow*)data)->main();
		_endthreadex(0);
		return 0;
	}

	/**
	 * 処理開始
	 */
	void start() {
		end();
		doneflag = false;
		tTJSVariant krkrHwnd;
		if (TJS_SUCCEEDED(window->PropGet(0, TJS_W("HWND"), NULL, &krkrHwnd, window))) {
			hParent = ::FindWindowEx((HWND)(tjs_int)krkrHwnd, NULL, KRKRDISPWINDOWCLASS, NULL);
			if (hParent) {
				thread = (HANDLE)_beginthreadex(NULL, 0, threadFunc, this, 0, NULL);
				if (thread) {
					WaitForSingleObject(prepare, 1000 * 3);
				}
			}
		}
	}
	
	/**
	 * 処理終了
	 */
	void end() {
		doneflag = true;
		if (thread) {
			WaitForSingleObject(thread, INFINITE);
			CloseHandle(thread);
			thread = 0;
		}
		hParent = 0;
	}

	/**
	 * 実行メイン処理
	 * ウインドウの生成から破棄までを独立したスレッドで行う
	 */
	void main() {
		// ウインドウ生成
		if (hParent && !hWnd) {
			RECT rect;
			POINT point;
			point.x = 0;
			point.y = 0;
			::GetClientRect(hParent, &rect);
			::ClientToScreen(hParent, &point);
			int left   = point.x;
			int top    = point.y;
			int width  = rect.right  - rect.left;
			int height = rect.bottom - rect.top;
			hWnd = ::CreateWindowEx(0, CLASSNAME, _T(""), WS_POPUP, left, top, width, height, 0, 0, GetModuleHandle(NULL), NULL);
			if (hWnd && !doneflag) {
				::SetWindowLong(hWnd, GWL_USERDATA, (LONG)this);
				::ShowWindow(hWnd,TRUE);
				create();
				// 待ち合わせ完了
				SetEvent(prepare);
				// メッセージループの実行
				MSG msg;
				while (!doneflag) {
					if (::PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
						if (GetMessage(&msg, NULL, 0, 0)) {
							::TranslateMessage (&msg);
							::DispatchMessage (&msg);
						} else {
							break;
						}
					} else {
					    Sleep(0);
					}
				}
				// ウインドウの破棄
				::DestroyWindow(hWnd);
				hWnd = 0;
			}
		}
	}

	// -------------------------------------------------------------

	static int RGBTOBGR(int c) {
		unsigned int a = (unsigned int)c & 0xff000000;
		unsigned int r = (unsigned int)c & 0x00ff0000;
		unsigned int g = (unsigned int)c & 0x0000ff00;
		unsigned int b = (unsigned int)c & 0x000000ff;
		return (int)(a | r>>16 | g | b<<16);
	}
	
	/**
	 * 描画内容生成
	 */
	void create() {

		RECT rect;
		GetClientRect(hWnd, &rect);
		int swidth  = rect.right  - rect.left;
		int sheight = rect.bottom - rect.top;
		
		if (progressBarEnable) {
			// プログレスバーの配置決定
			if (progressBarWidth < 0) {
				progressBarWidth = swidth / 3;
			}
			if (progressBarHeight < 0) {
				progressBarHeight = sheight/10;
			}
			if (progressBarLeft < 0) {
				progressBarLeft = (swidth - progressBarWidth)/2;
			}
			if (progressBarTop < 0) {
				progressBarTop = (sheight - progressBarHeight)/2;
			}
			// プログレスバーを作成
			progressBarHandle = CreateWindowEx(0, PROGRESS_CLASS, _T(""),
											   WS_VISIBLE | WS_CHILD | (progressBarStyle & (PBS_SMOOTH|PBS_VERTICAL)),
											   progressBarLeft, progressBarTop, progressBarWidth, progressBarHeight,
											   hWnd, (HMENU)1, GetModuleHandle(NULL), NULL);
			SendMessage(progressBarHandle, PBM_SETBARCOLOR, 0, RGBTOBGR(progressBarColor));
			SendMessage(progressBarHandle, PBM_SETBKCOLOR, 0, RGBTOBGR(progressBarBackColor));
			SendMessage(progressBarHandle, PBM_SETRANGE , 0, MAKELPARAM(0, 100));
			SendMessage(progressBarHandle, PBM_SETSTEP, 1, 0 );
			SendMessage(progressBarHandle, PBM_SETPOS, percent, 0);
		}

		if (cancelButtonEnable) {
			// キャンセルボタンの配置決定
			if (cancelButtonWidth < 0) {
				cancelButtonWidth = cancelButtonCaption.length() * 16 + 8;
			}
			if (cancelButtonHeight < 0) {
				cancelButtonHeight = 24;
			}
			if (cancelButtonLeft < 0) {
				cancelButtonLeft = (swidth - cancelButtonWidth)/2;
			}
			if (cancelButtonTop < 0) {
				cancelButtonTop = sheight - cancelButtonHeight * 3;
			}
			// キャンセルボタンを作成
			cancelButtonHandle = CreateWindow(_T("BUTTON"), cancelButtonCaption.c_str(),
											  WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
											  cancelButtonLeft, cancelButtonTop, cancelButtonWidth, cancelButtonHeight,
											  hWnd, (HMENU)ID_CANCEL, GetModuleHandle(NULL), NULL);
		}
	}
	
	/**
	 * 画面更新処理
	 */
	void show(HDC dc, PAINTSTRUCT &ps) {
		// 背景で塗りつぶし

		// アニメパターンを描画

		// テキストを表示
	}

	/**
	 * キャンセル通知
	 */
	void cancel() {
		cancelflag = true;
	}
};

/**
 * ウインドウにレイヤセーブ機能を拡張
 */
class WindowExProgress {

protected:
	iTJSDispatch2 *objthis; //< オブジェクト情報の参照
	ProgressWindow *progressWindow; //< プログレス表示用

public:
	/**
	 * コンストラクタ
	 */
	WindowExProgress(iTJSDispatch2 *objthis) : objthis(objthis), progressWindow(NULL) {}

	/**
	 * デストラクタ
	 */
	~WindowExProgress() {
		delete progressWindow;
	}

	/**
	 * プログレス処理を開始する。
	 * 吉里吉里が実行ブロック中でも正常に表示継続します。
	 * @param init 初期化データ(辞書)
	 */
	void startProgress(iTJSDispatch2 *init) {
		if (progressWindow) {
			TVPThrowExceptionMessage(L"already running progress");
		}
		progressWindow = new ProgressWindow(objthis, init);
	}
	
	/**
	 * プログレス処理の経過状態を通知する。
	 * @param percent 経過状態をパーセント指定
	 * @return キャンセル要求があれば true
	 */
	bool doProgress(int percent) {
		if (!progressWindow) {
			TVPThrowExceptionMessage(L"not running progress");
		}
		return progressWindow->doProgress(percent);
	}

	/**
	 * プログレス処理を終了する。
	 */
	void endProgress() {
		if (!progressWindow) {
			TVPThrowExceptionMessage(L"not running progress");
		}
		delete progressWindow;
		progressWindow = NULL;
	}
};

//---------------------------------------------------------------------------

// インスタンスゲッタ
NCB_GET_INSTANCE_HOOK(WindowExProgress)
{
	NCB_INSTANCE_GETTER(objthis) { // objthis を iTJSDispatch2* 型の引数とする
		ClassT* obj = GetNativeInstance(objthis);	// ネイティブインスタンスポインタ取得
		if (!obj) {
			obj = new ClassT(objthis);				// ない場合は生成する
			SetNativeInstance(objthis, obj);		// objthis に obj をネイティブインスタンスとして登録する
		}
		return obj;
	}
};

#define ENUM(n) Variant(#n, (int)n)

NCB_ATTACH_CLASS_WITH_HOOK(WindowExProgress, Window) {
	ENUM(PBS_SMOOTH);
	ENUM(PBS_VERTICAL);
	NCB_METHOD(startProgress);
	NCB_METHOD(doProgress);
	NCB_METHOD(endProgress);
};

/**
 * 登録処理後
 */
static void PreRegistCallback()
{
	ProgressWindow::registerWindowClass();
}

/**
 * 開放処理前
 */
static void PostUnregistCallback()
{
	ProgressWindow::unregisterWindowClass();
}

NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
