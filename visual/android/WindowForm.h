
#ifndef __WINDOW_FORM_H__
#define __WINDOW_FORM_H__

#include "TouchPoint.h"
#include "VelocityTracker.h"
#include "tvpinputdefs.h"
#include "WindowIntf.h"
#include "NativeEventQueue.h"

enum {
	 ssShift = TVP_SS_SHIFT,
	 ssAlt = TVP_SS_ALT,
	 ssCtrl = TVP_SS_CTRL,
	 ssLeft = TVP_SS_LEFT,
	 ssRight = TVP_SS_RIGHT,
	 ssMiddle = TVP_SS_MIDDLE,
	 ssDouble = TVP_SS_DOUBLE,
	 ssRepeat = TVP_SS_REPEAT,
};

enum {
	orientUnknown,
	orientPortrait,
	orientLandscape,
};

typedef unsigned long TShiftState;
extern tjs_uint32 TVP_TShiftState_To_uint32(TShiftState state);
extern TShiftState TVP_TShiftState_From_uint32(tjs_uint32 state);

/*
 * tTJSNI_Window が持つ Form クラスに要求されるメソッドを列挙したもの。
 * Form = new TTVPWindowForm(Application, this) のように実体化される。
 * このファイル自体は使われず、各環境で各メソッドを実装するためのベースとして使用する
 *
 * イベント配信周りは書かれていないので、別途実装の必要あり
 */
class TTVPWindowForm : public TouchHandler {
	class tTVPApplication* app_;

	TouchPointList touch_points_;
	VelocityTrackers TouchVelocityTracker;
	VelocityTracker MouseVelocityTracker;

	NativeEventQueue<TTVPWindowForm> EventQueue;

	//-- TJS object related
	tTJSNI_Window * TJSNativeInstance;
	int LastMouseDownX, LastMouseDownY; // in Layer coodinates

private:
	static int MulDiv( int nNumber, int nNumerator, int nDenominator ) {
		return (int)(((int64_t)nNumber * (int64_t)nNumerator) / nDenominator);
	}

	void PostVideoOverlayEvent( tjs_uint ev );
protected:
	void WndProc(NativeEvent& ev);

public:
	TTVPWindowForm( class tTVPApplication* app, class tTJSNI_Window* ni );
	virtual ~TTVPWindowForm();

	// Windowが有効かどうか、無効だとイベントが配信されない
	bool GetFormEnabled() { return true; }

	// 閉じる
	void InvalidateClose();
	void Close();

	// Androidで閉じるのを抑止することはできない
	void OnCloseQueryCalled(bool b) {}

	// 定期的に呼び出されるので、定期処理があれば実行する
	void TickBeat();

	// アクティブ/デアクティブ化された時に、Windowがアクティブかどうかチェックされる
	bool GetWindowActive() { return true; }

	// キー入力
	void OnKeyDown( tjs_int vk, int shift );
	void InternalKeyDown(tjs_uint16 key, tjs_uint32 shift);
	void OnKeyUp( tjs_int vk, int shift );
	void InternalKeyUp( tjs_uint16 key, tjs_uint32 shift );
	void OnKeyPress( tjs_int vk, int repeat, bool prevkeystate, bool convertkey );

	// マウスカーソル
	void SetDefaultMouseCursor() {}
	void SetMouseCursor(tjs_int handle) {}
	void HideMouseCursor() {}
	void SetMouseCursorState(tTVPMouseCursorState mcs) {}
	tTVPMouseCursorState GetMouseCursorState() const { return mcsHidden; }

	// マウスカーソル座標はLayer.cursorX/cursorYで得られるようになっているが、タッチスクリーンだと無意味なので使えなくなる(最後のタッチ座標を記憶して置いた方がよいか？)
	void GetCursorPos(tjs_int &x, tjs_int &y) { x = 0; y = 0;}
	void SetCursorPos(tjs_int x, tjs_int y) {}

	// ヒント表示(マウスオーバー出来ないのでAndroidでは無効)
	void SetHintText(iTJSDispatch2* sender, const ttstr &text) {}
	void SetHintDelay( tjs_int delay ) {}
	tjs_int GetHintDelay() const { return 0; }

	// IME 入力関係、EditViewを最前面に貼り付けて、そこで入力させるのが現実的かな、好きな位置に表示は画面狭いとあまり現実的じゃないかも
	// 入力タイトルを指定して、入力受付、確定文字が返ってくるスタイルの方がいいか、モーダルにはならないから、確定後イベント通知かな
	void SetAttentionPoint(tjs_int left, tjs_int top, const struct tTVPFont * font ) {}
	void DisableAttentionPoint() {}
	void SetImeMode(tTVPImeMode mode) {}
	tTVPImeMode GetDefaultImeMode() const { return imDisable; }
	void ResetImeMode() {}

	// Windowハンドル系メソッドはすべて不要
	// VideoOverlay で SetMessageDrainWindow に渡す Window ハンドル
	void* GetSurfaceWindowHandle() { return nullptr; }
	// VideoOverlay の OwnerWindow として設定される Window ハンドル
	void* GetWindowHandle() { return nullptr; }
	// Window::HWND プロパティ値として使用される。非Windows環境ではプラグインに渡すこと出来ないから不要か。ダミーでNULL返せばいい
	void* GetWindowHandleForPlugin() { return nullptr; }

	// VideoOverlayで表示サイズを決めるためにズーム値を用いて引数値を拡大縮小する
	void ZoomRectangle( tjs_int & left, tjs_int & top, tjs_int & right, tjs_int & bottom) {}
	// VideoOverlayで表示位置を決めるため、フルスクリーン時の黒ふちを考慮したleft/top位置を得る
	void GetVideoOffset(tjs_int &ofsx, tjs_int &ofsy) {}

	// 内容更新
	void UpdateWindow(tTVPUpdateType type = utNormal);

	// 表示/非表示
	bool GetVisible() const { return true; }
	void SetVisibleFromScript(bool b) { /* 非表示には出来ない */}
	void ShowWindowAsModal();

	// タイトル、Activityのタイトルに設定できるが、無意味かな
	tjs_string GetCaption();
	void GetCaption( tjs_string& v ) const;
	void SetCaption( const tjs_string& v );

	// サイズや位置など
	// 位置はAndroidでは無効か、常に0を返し、設定もスルーなど
	void SetLeft( int l ) {}
	int GetLeft() const { return 0; }
	void SetTop( int t ) {}
	int GetTop() const { return 0; }
	void SetPosition( int l, int t ) {}
	// サイズ
	void SetWidth( int w ){ /* Activityのサイズを変更することはできない */ }
	int GetWidth() const;
	void SetHeight( int h ){ /* Activityのサイズを変更することはできない */ }
	int GetHeight() const;
	void SetSize( int w, int h ){ /* Activityのサイズを変更することはできない */ }
	// 最小、最大サイズ関係、Androidなどリサイズがないとしたら無効か
	void SetMinWidth( int v ) {}
	int GetMinWidth() const { return 0; }
	void SetMinHeight( int v ) {}
	int GetMinHeight() { return 0; }
	void SetMinSize( int w, int h ) {}
	void SetMaxWidth( int v ) {}
	int GetMaxWidth() { return 0; }
	void SetMaxHeight( int v ) {}
	int GetMaxHeight() { return 0; }
	void SetMaxSize( int w, int h ) {}

	// 内部のサイズ、実質的にこれが表示領域サイズ
	void SetInnerWidth( int w ) { /* 表示領域のサイズを変更することはできない */ }
	int GetInnerWidth() const;
	void SetInnerHeight( int h ) { /* 表示領域のサイズを変更することはできない */ }
	int GetInnerHeight() const;
	void SetInnerSize( int w, int h ) { /* 表示領域のサイズを変更することはできない */ }

	// 境界サイズ、無効
	void SetBorderStyle( enum tTVPBorderStyle st) {}
	enum tTVPBorderStyle GetBorderStyle() const { return bsNone; }

	// 常に最前面表示、無効
	void SetStayOnTop( bool b ) {}
	bool GetStayOnTop() const { return true; }
	// 最前面へ移動
	void BringToFront() {}

	// フルスクリーン、無効と言うか常に真
	void SetFullScreenMode(bool b) {}
	bool GetFullScreenMode() const { return true; }

	//マウスキー(キーボードでのマウスカーソル操作)は無効
	void SetUseMouseKey(bool b) {}
	bool GetUseMouseKey() const { return false; }

	// 他ウィンドウのキー入力をトラップするか、無効
	void SetTrapKey(bool b) {}
	bool GetTrapKey() const { return false; }

	// ウィンドウマスクリージョンは無効
	//void SetMaskRegion(HRGN threshold);
	void SetMaskRegion(void* threshold) {}
	void RemoveMaskRegion() {}

	// フォースは常に真
	void SetFocusable(bool b) {}
	bool GetFocusable() const { return true; }

	// 表示ズーム関係(非サポート)
	void SetZoom(tjs_int numer, tjs_int denom, bool set_logical = true) {}
	void SetZoomNumer( tjs_int n ) {}
	tjs_int GetZoomNumer() const { return 1; }
	void SetZoomDenom(tjs_int d) {}
	tjs_int GetZoomDenom() const { return 1; }

	// タッチ入力関係 ( TouchPointList によって管理されている )
	void SetTouchScaleThreshold( double threshold ) { touch_points_.SetScaleThreshold( threshold ); }
	double GetTouchScaleThreshold() const { return touch_points_.GetScaleThreshold(); }
	void SetTouchRotateThreshold( double threshold ) { touch_points_.SetRotateThreshold( threshold ); }
	double GetTouchRotateThreshold() const { return touch_points_.GetRotateThreshold(); }
	tjs_real GetTouchPointStartX( tjs_int index ) const { return touch_points_.GetStartX(index); }
	tjs_real GetTouchPointStartY( tjs_int index ) const { return touch_points_.GetStartY(index); }
	tjs_real GetTouchPointX( tjs_int index ) const { return touch_points_.GetX(index); }
	tjs_real GetTouchPointY( tjs_int index ) const { return touch_points_.GetY(index); }
	tjs_int GetTouchPointID( tjs_int index ) const { return touch_points_.GetID(index); }
	tjs_int GetTouchPointCount() const { return touch_points_.CountUsePoint(); }
	void ResetTouchVelocity( tjs_int id ) {
		TouchVelocityTracker.end( id );
	}

	// タッチ入力のマウスエミュレートON/OFF
	void SetEnableTouch( bool b ) {}
	bool GetEnableTouch() const { return false; }

	// タッチ、マウス加速度
	bool GetTouchVelocity( tjs_int id, float& x, float& y, float& speed ) const {
		return TouchVelocityTracker.getVelocity( id, x, y, speed );
	}
	bool GetMouseVelocity( float& x, float& y, float& speed ) const {
		if( MouseVelocityTracker.getVelocity( x, y ) ) {
			speed = hypotf(x, y);
			return true;
		}
		return false;
	}
	void ResetMouseVelocity() {
		MouseVelocityTracker.clear();
	}

	// 画面表示向き取得
	int GetDisplayOrientation();
	int GetDisplayRotate();

	void OnTouchDown( float x, float y, float cx, float cy, tjs_int id, tjs_int64 tick );
	void OnTouchMove( float x, float y, float cx, float cy, tjs_int id, tjs_int64 tick );
	void OnTouchUp( float x, float y, float cx, float cy, tjs_int id, tjs_int64 tick );
	void OnTouchScaling( double startdist, double currentdist, double cx, double cy, int flag );
	void OnTouchRotate( double startangle, double currentangle, double distance, double cx, double cy, int flag );
	void OnMultiTouch();

	void OnStart() {}
	void OnRestart() {}
	void OnResume();
	void OnPause();
	void OnStop() {}
	void OnDestory() {}

	void OnDisplayRotate( tjs_int orientation, tjs_int density );

	NativeEventQueueIntarface* GetEventHandler() { return &EventQueue; }
};

#endif
