
#include "tjsCommHead.h"

#include "WindowForm.h"
#include "ActivityEvents.h"
#include "Application.h"

tjs_uint32 TVP_TShiftState_To_uint32(TShiftState state) {
	tjs_uint32 result = 0;
/* Android用のシフトキー定義に変える
	if( state & MK_SHIFT ) {
		result |= ssShift;
	}
	if( state & MK_CONTROL ) {
		result |= ssCtrl;
	}
	if( state & MK_ALT ) {
		result |= ssAlt;
	}
*/
	return result;
}
TShiftState TVP_TShiftState_From_uint32(tjs_uint32 state){
	TShiftState result = 0;
/* Android用のシフトキー定義に変える
	if( state & ssShift ) {
		result |= MK_SHIFT;
	}
	if( state & ssCtrl ) {
		result |= MK_CONTROL;
	}
	if( state & ssAlt ) {
		result |= MK_ALT;
	}
*/
	return result;
}

TTVPWindowForm::TTVPWindowForm( class tTVPApplication* app, class tTJSNI_Window* ni )
 : app_(app), touch_points_(this), EventQueue(this,&TTVPWindowForm::WndProc), TJSNativeInstance(ni), FullScreenDestRect(0,0,0,0),
	LayerLeft(0), LayerTop(0), LayerWidth(32), LayerHeight(32), ZoomDenom(1), ActualZoomDenom(1), ZoomNumer(1), ActualZoomNumer(1) {
	EventQueue.Allocate();

	app->AddWindow(this);

	NextSetWindowHandleToDrawDevice = true;
	LastSentDrawDeviceDestRect.clear();
}
TTVPWindowForm::~TTVPWindowForm() {
	EventQueue.Deallocate();
}
void TTVPWindowForm::WndProc(NativeEvent& ev) {
	switch( ev.Message ) {
	case AM_START:
		break;
	case AM_RESTART:
		break;
	case AM_RESUME:
		break;
	case AM_PAUSE:
		break;
	case AM_STOP:
		break;
	case AM_DESTROY:
		break;
	case AM_SURFACE_CHANGED:
		// Surfaceが切り替わったので、DrawDevice 準備
		break;
	case AM_SURFACE_CREATED:
		break;
	case AM_SURFACE_DESTORYED:
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
	default:
		EventQueue.HandlerDefault( ev );
		break;
	}
}
	// Windowが有効かどうか、無効だとイベントが配信されない
bool TTVPWindowForm::GetFormEnabled() {
	return true;
}

// 閉じる
void TTVPWindowForm::InvalidateClose() {
}
void TTVPWindowForm::SendCloseMessage() {
}
void TTVPWindowForm::Close() {
}
void TTVPWindowForm::OnCloseQueryCalled(bool b) {
}

// 定期的に呼び出されるので、定期処理があれば実行する
void TTVPWindowForm::TickBeat() {
}

// アクティブ/デアクティブ化された時に、Windowがアクティブかどうかチェックされる
bool TTVPWindowForm::GetWindowActive() {
	return true;
}

// DrawDevice
void TTVPWindowForm::ResetDrawDevice() {
}

// キー入力
void TTVPWindowForm::InternalKeyDown(tjs_uint16 key, tjs_uint32 shift) {
}
void TTVPWindowForm::OnKeyUp( tjs_uint16 vk, int shift ) {
}
void TTVPWindowForm::OnKeyPress( tjs_uint16 vk, int repeat, bool prevkeystate, bool convertkey ) {
}

void TTVPWindowForm::SetDrawDeviceDestRect()
{
	tTVPRect destrect;
	tjs_int w = MulDiv(LayerWidth,  ActualZoomNumer, ActualZoomDenom);
	tjs_int h = MulDiv(LayerHeight, ActualZoomNumer, ActualZoomDenom);
	if( w < 1 ) w = 1;
	if( h < 1 ) h = 1;

	destrect.left = destrect.top = 0;
	destrect.right = w;
	destrect.bottom = h;

	if( LastSentDrawDeviceDestRect != destrect ) {
		if( TJSNativeInstance ) {
			if( GetFullScreenMode() ) {
				TJSNativeInstance->GetDrawDevice()->SetClipRectangle(FullScreenDestRect);
			} else {
				TJSNativeInstance->GetDrawDevice()->SetClipRectangle(destrect);
			}
			TJSNativeInstance->GetDrawDevice()->SetDestRectangle(destrect);
		}
		LastSentDrawDeviceDestRect = destrect;
	}
}
void TTVPWindowForm::InternalSetPaintBoxSize() {
	SetDrawDeviceDestRect();
}
// プライマリーレイヤーのサイズに合うように呼び出される。w/hはLayerWidth/LayerHeightに当たり、ズームを考慮して表示サイズを設定する
void TTVPWindowForm::SetPaintBoxSize(tjs_int w, tjs_int h) {
	LayerWidth  = w;
	LayerHeight = h;
	InternalSetPaintBoxSize();
}

// VideoOverlayで表示サイズを決めるためにズーム値を用いて引数値を拡大縮小する
void TTVPWindowForm::ZoomRectangle( tjs_int & left, tjs_int & top, tjs_int & right, tjs_int & bottom) {
}
// VideoOverlayで表示位置を決めるため、フルスクリーン時の黒ふちを考慮したleft/top位置を得る
void TTVPWindowForm::GetVideoOffset(tjs_int &ofsx, tjs_int &ofsy) {
}

// 内容更新
void TTVPWindowForm::UpdateWindow(tTVPUpdateType type) {
	if( TJSNativeInstance ) {
		tTVPRect r;
		r.left = 0;
		r.top = 0;
		r.right = LayerWidth;
		r.bottom = LayerHeight;
		TJSNativeInstance->NotifyWindowExposureToLayer(r);
		TVPDeliverWindowUpdateEvents();
	}
}

// 表示/非表示
void TTVPWindowForm::SetVisibleFromScript(bool b) {
}
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
// 位置はAndroidでは無効か、常に0を返し、設定もスルーなど
void TTVPWindowForm::SetLeft( int l ) {}
int TTVPWindowForm::GetLeft() const { return 0; };
void TTVPWindowForm::SetTop( int t ) {}
int TTVPWindowForm::GetTop() const { return 0; }
void TTVPWindowForm::SetPosition( int l, int t ) {}
// サイズ
void TTVPWindowForm::SetWidth( int w ) { /* Activityのサイズを変更することはできない */ }
int TTVPWindowForm::GetWidth() const {
	return Application->GetActivityWidth();
}
void TTVPWindowForm::SetHeight( int h ) { /* Activityのサイズを変更することはできない */ }
int TTVPWindowForm::GetHeight() const {
	return Application->GetActivityHeight();
}
void TTVPWindowForm::SetSize( int w, int h ) { /* Activityのサイズを変更することはできない */ }

// 内部のサイズ、実質的にこれが表示領域サイズ
void TTVPWindowForm::SetInnerWidth( int w ) { /* 表示領域のサイズを変更することはできない */ }
int TTVPWindowForm::GetInnerWidth() const {
	return Application->GetMainViewWidth();
}
void TTVPWindowForm::SetInnerHeight( int h ) { /* 表示領域のサイズを変更することはできない */ }
int TTVPWindowForm::GetInnerHeight() const {
	return Application->GetMainViewHeight();
}
void TTVPWindowForm::SetInnerSize( int w, int h ) { /* 表示領域のサイズを変更することはできない */ }

// 表示ズーム関係
void TTVPWindowForm::SetZoom(tjs_int numer, tjs_int denom, bool set_logical ) {}
void TTVPWindowForm::SetZoomNumer( tjs_int n ) {}
tjs_int TTVPWindowForm::GetZoomNumer() const { return 1; }
void TTVPWindowForm::SetZoomDenom(tjs_int d) {}
tjs_int TTVPWindowForm::GetZoomDenom() const { return 1; }

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

void TTVPWindowForm::TranslateWindowToDrawArea(float&x, float &y) {
	if( GetFullScreenMode() ) {
		x -= FullScreenDestRect.left;
		y -= FullScreenDestRect.top;
	}
}
void TTVPWindowForm::OnTouchDown( float x, float y, float cx, float cy, tjs_int id, tjs_int64 tick ) {
	TranslateWindowToDrawArea(x, y);

	TouchVelocityTracker.start( id );
	TouchVelocityTracker.update( id, tick, (float)x, (float)y );

	if(TJSNativeInstance) {
		TVPPostInputEvent( new tTVPOnTouchDownInputEvent(TJSNativeInstance, x, y, cx, cy, id));
	}
	touch_points_.TouchDown( x, y ,cx, cy, id, static_cast<tjs_uint>(tick&0xffffffff) );
}
void TTVPWindowForm::OnTouchMove( float x, float y, float cx, float cy, tjs_int id, tjs_int64 tick ) {
	TranslateWindowToDrawArea( x, y);

	TouchVelocityTracker.update( id, tick, (float)x, (float)y );

	if(TJSNativeInstance) {
		TVPPostInputEvent( new tTVPOnTouchMoveInputEvent(TJSNativeInstance, x, y, cx, cy, id));
	}
	touch_points_.TouchMove( x, y, cx, cy, id, static_cast<tjs_uint>(tick&0xffffffff) );
}
void TTVPWindowForm::OnTouchUp( float x, float y, float cx, float cy, tjs_int id, tjs_int64 tick ) {
	TranslateWindowToDrawArea( x, y);

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

