
#include "tjsCommHead.h"

#include "WindowForm.h"
#include "ActivityEvents.h"
#include "Application.h"
#include "TickCount.h"
#include "Random.h"

extern tjs_uint16 TVPTranslateAndroidKeyToVirtualKey( tjs_int androidKey );

// Androidでは変換しない, ssShiftなどで統一的に扱っている。
tjs_uint32 TVP_TShiftState_To_uint32(TShiftState state) { return (tjs_uint32)state; }
TShiftState TVP_TShiftState_From_uint32(tjs_uint32 state){ return (TShiftState)state; }

TTVPWindowForm::TTVPWindowForm( class tTVPApplication* app, class tTJSNI_Window* ni )
 : app_(app), touch_points_(this), EventQueue(this,&TTVPWindowForm::WndProc), TJSNativeInstance(ni) {
	EventQueue.Allocate();

	app->AddWindow(this);
}
TTVPWindowForm::~TTVPWindowForm() {
	EventQueue.Deallocate();
}
void TTVPWindowForm::WndProc(NativeEvent& ev) {
	switch( ev.Message ) {
	case AM_START:
		OnStart();
		break;
	case AM_RESTART:
		OnRestart();
		break;
	case AM_RESUME:
		OnResume();
		break;
	case AM_PAUSE:
		OnPause();
		break;
	case AM_STOP:
		OnStop();
		break;
	case AM_DESTROY:
		OnDestory();
		break;
	case AM_SURFACE_CHANGED:
		// Surfaceが切り替わった
		if( TJSNativeInstance ) TJSNativeInstance->UpdateCanvasSurface();
		UpdateWindow();
		break;
	case AM_SURFACE_CREATED:
		break;
	case AM_SURFACE_DESTORYED:
		if( TJSNativeInstance ) TJSNativeInstance->ReleaseCanvasSurface();
		break;
	case AM_SURFACE_PAINT_REQUEST:
		UpdateWindow();
		break;
	case AM_TOUCH_DOWN:
		OnTouchDown( ev.WParamf0, ev.WParamf1, ev.LParamf0, ev.LParamf0, ev.LParam1, ev.Result );
		break;
	case AM_TOUCH_MOVE:
		OnTouchMove( ev.WParamf0, ev.WParamf1, ev.LParamf0, ev.LParamf0, ev.LParam1, ev.Result );
		break;
	case AM_TOUCH_UP:
		OnTouchUp( ev.WParamf0, ev.WParamf1, ev.LParamf0, ev.LParamf0, ev.LParam1, ev.Result );
		break;
	case AM_KEY_DOWN:
		OnKeyDown( (tjs_int)ev.WParam, (int)ev.LParam );
		break;
	case AM_KEY_UP:
		OnKeyUp( (tjs_int)ev.WParam, (int)ev.LParam );
		break;
	case AM_MOVIE_ENDED:
	case AM_MOVIE_PLAYER_ERROR:
	case AM_MOVIE_LOAD_ERROR:
	case AM_MOVIE_BUFFERING:
	case AM_MOVIE_IDLE:
	case AM_MOVIE_READY:
	case AM_MOVIE_PLAY:
		PostVideoOverlayEvent( ev.Message );
		break;
	case AM_DISPLAY_ROTATE:
		OnDisplayRotate( (tjs_int)ev.WParam, (tjs_int)ev.LParam );
		break;
	default:
		EventQueue.HandlerDefault( ev );
		break;
	}
}
void TTVPWindowForm::PostVideoOverlayEvent( tjs_uint ev ) {
	if( TJSNativeInstance ) {
		TJSNativeInstance->VideoOverlayEvent( ev );
	}
}


// 閉じる
void TTVPWindowForm::InvalidateClose() {
	// closing action by object invalidation;
	// this will not cause any user confirmation of closing the window.
	TJSNativeInstance = nullptr;
	Application->finishActivity();
}
void TTVPWindowForm::Close() {
	Application->finishActivity();
}

// 定期的に呼び出されるので、定期処理があれば実行する
void TTVPWindowForm::TickBeat() {
}


// キー入力
void TTVPWindowForm::OnKeyDown( tjs_int vk, int shift ) {
	InternalKeyDown( TVPTranslateAndroidKeyToVirtualKey(vk), shift );
}
void TTVPWindowForm::InternalKeyDown(tjs_uint16 key, tjs_uint32 shift) {
	tjs_uint32 tick = TVPGetRoughTickCount32();
	TVPPushEnvironNoise(&tick, sizeof(tick));
	TVPPushEnvironNoise(&key, sizeof(key));
	TVPPushEnvironNoise(&shift, sizeof(shift));
	if( TJSNativeInstance ) {
		TVPPostInputEvent(new tTVPOnKeyDownInputEvent(TJSNativeInstance, key, shift));
	}
}
void TTVPWindowForm::OnKeyUp( tjs_int vk, int shift ) {
	InternalKeyUp( TVPTranslateAndroidKeyToVirtualKey(vk), shift );
}
void TTVPWindowForm::InternalKeyUp( tjs_uint16 key, tjs_uint32 shift ) {
	tjs_uint32 tick = TVPGetRoughTickCount32();
	TVPPushEnvironNoise(&tick, sizeof(tick));
	TVPPushEnvironNoise(&key, sizeof(key));
	TVPPushEnvironNoise(&shift, sizeof(shift));
	if( TJSNativeInstance ) {
		TVPPostInputEvent(new tTVPOnKeyUpInputEvent(TJSNativeInstance, key, shift));
	}
}
void TTVPWindowForm::OnKeyPress( tjs_int vk, int repeat, bool prevkeystate, bool convertkey ) {
}

// 内容更新
void TTVPWindowForm::UpdateWindow(tTVPUpdateType type) {
	if( TJSNativeInstance ) {
		tTVPRect r; // dummy
		TJSNativeInstance->NotifyWindowExposureToLayer(r);
		TVPDeliverWindowUpdateEvents();
	}
}

// 表示/非表示
void TTVPWindowForm::ShowWindowAsModal() {
	// modal は対応しないので、例外出す
	TVPThrowExceptionMessage(TJS_W("Modal window is not supported."));
}

// タイトル、Activityのタイトルに設定できるが、無意味かな
tjs_string TTVPWindowForm::GetCaption() {
	return Application->GetActivityCaption();
}
void TTVPWindowForm::GetCaption( tjs_string& v ) const {
	v = Application->GetActivityCaption();
}
void TTVPWindowForm::SetCaption( const tjs_string& v ) {
	Application->SetActivityCaption( v );
}

// サイズや位置など
// サイズ
int TTVPWindowForm::GetWidth() const {
	return Application->GetActivityWidth();
}
int TTVPWindowForm::GetHeight() const {
	return Application->GetActivityHeight();
}
// 内部のサイズ、実質的にこれが表示領域サイズ
int TTVPWindowForm::GetInnerWidth() const {
	return Application->GetMainViewWidth();
}
int TTVPWindowForm::GetInnerHeight() const {
	return Application->GetMainViewHeight();
}
// 画面表示向き取得
int TTVPWindowForm::GetDisplayOrientation() {
	tjs_int orient = Application->getOrientation();
	switch( orient ) {
	case tTVPApplication::orientUnknown:
		return orientUnknown;
	case tTVPApplication::orientPortrait:
		return orientPortrait;
	case tTVPApplication::orientLandscape:
		return orientLandscape;
	case tTVPApplication::orientSquare:	// not used
	default:
		return  orientUnknown;
	}
}
int TTVPWindowForm::GetDisplayRotate() {
	return Application->GetDisplayRotate();
}

void TTVPWindowForm::OnTouchDown( float x, float y, float cx, float cy, tjs_int id, tjs_int64 tick ) {
	TouchVelocityTracker.start( id );
	TouchVelocityTracker.update( id, tick, (float)x, (float)y );

	if(TJSNativeInstance) {
		TVPPostInputEvent( new tTVPOnTouchDownInputEvent(TJSNativeInstance, x, y, cx, cy, id));
	}
	touch_points_.TouchDown( x, y ,cx, cy, id, static_cast<tjs_uint>(tick&0xffffffff) );
}
void TTVPWindowForm::OnTouchMove( float x, float y, float cx, float cy, tjs_int id, tjs_int64 tick ) {
	TouchVelocityTracker.update( id, tick, (float)x, (float)y );

	if(TJSNativeInstance) {
		TVPPostInputEvent( new tTVPOnTouchMoveInputEvent(TJSNativeInstance, x, y, cx, cy, id));
	}
	touch_points_.TouchMove( x, y, cx, cy, id, static_cast<tjs_uint>(tick&0xffffffff) );
}
void TTVPWindowForm::OnTouchUp( float x, float y, float cx, float cy, tjs_int id, tjs_int64 tick ) {
	TouchVelocityTracker.update( id, tick, (float)x, (float)y );

	if(TJSNativeInstance) {
		TVPPostInputEvent( new tTVPOnTouchUpInputEvent(TJSNativeInstance, x, y, cx, cy, id));
	}
	touch_points_.TouchUp( x, y, cx, cy, id, static_cast<tjs_uint>(tick&0xffffffff) );
}

void TTVPWindowForm::OnTouchScaling( double startdist, double currentdist, double cx, double cy, int flag ) {
	if(TJSNativeInstance) {
		TVPPostInputEvent( new tTVPOnTouchScalingInputEvent(TJSNativeInstance, startdist, currentdist, cx, cy, flag ));
	}
}
void TTVPWindowForm::OnTouchRotate( double startangle, double currentangle, double distance, double cx, double cy, int flag ) {
	if(TJSNativeInstance) {
		TVPPostInputEvent( new tTVPOnTouchRotateInputEvent(TJSNativeInstance, startangle, currentangle, distance, cx, cy, flag));
	}
}
void TTVPWindowForm::OnMultiTouch() {
	if( TJSNativeInstance ) {
		TVPPostInputEvent( new tTVPOnMultiTouchInputEvent(TJSNativeInstance) );
	}
}
void TTVPWindowForm::OnResume() {
	if(TJSNativeInstance) TJSNativeInstance->FireOnActivate(true);
}
void TTVPWindowForm::OnPause() {
	if(TJSNativeInstance) TJSNativeInstance->FireOnActivate(false);
}
void TTVPWindowForm::OnDisplayRotate( tjs_int orientation, tjs_int density ) {
	if(TJSNativeInstance) {
		TVPPostInputEvent( new tTVPOnDisplayRotateInputEvent(TJSNativeInstance, orientation, -1, density, 0, 0));
	}
}


