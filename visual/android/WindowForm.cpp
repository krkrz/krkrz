
#include "tjsCommHead.h"

#include "WindowForm.h"
#include "ActivityEvents.h"

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
 : touch_points_(this), EventQueue(this,&TTVPWindowForm::WndProc) {
	EventQueue.Allocate();
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
		break;
	case AM_SURFACE_CREATED:
		break;
	case AM_SURFACE_DESTORYED:
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

// プライマリーレイヤーのサイズに合うように呼び出される。w/hはLayerWidth/LayerHeightに当たり、ズームを考慮して表示サイズを設定する
void TTVPWindowForm::SetPaintBoxSize(tjs_int w, tjs_int h) {
}

// VideoOverlayで表示サイズを決めるためにズーム値を用いて引数値を拡大縮小する
void TTVPWindowForm::ZoomRectangle( tjs_int & left, tjs_int & top, tjs_int & right, tjs_int & bottom) {
}
// VideoOverlayで表示位置を決めるため、フルスクリーン時の黒ふちを考慮したleft/top位置を得る
void TTVPWindowForm::GetVideoOffset(tjs_int &ofsx, tjs_int &ofsy) {
}

// 内容更新
void TTVPWindowForm::UpdateWindow(tTVPUpdateType type) {
}

// 表示/非表示
void TTVPWindowForm::SetVisibleFromScript(bool b) {
}
void TTVPWindowForm::ShowWindowAsModal() {
	// modal は対応しないので、例外出す
}

// タイトル、Activityのタイトルに設定できるが、無意味かな
std::wstring TTVPWindowForm::GetCaption() {
	return std::wstring(L"");
}
void TTVPWindowForm::GetCaption( std::wstring& v ) const {
}
void TTVPWindowForm::SetCaption( const std::wstring& v ) {
}

// サイズや位置など
// 位置はAndroidでは無効か、常に0を返し、設定もスルーなど
void TTVPWindowForm::SetLeft( int l ) {}
int TTVPWindowForm::GetLeft() const { return 0; };
void TTVPWindowForm::SetTop( int t ) {}
int TTVPWindowForm::GetTop() const { return 0; }
void TTVPWindowForm::SetPosition( int l, int t ) {}
// サイズ
void TTVPWindowForm::SetWidth( int w ) {}
int TTVPWindowForm::GetWidth() const { return 0; }
void TTVPWindowForm::SetHeight( int h ) {}
int TTVPWindowForm::GetHeight() const { return 0; }
void TTVPWindowForm::SetSize( int w, int h ) {}

// 内部のサイズ、実質的にこれが表示領域サイズ
void TTVPWindowForm::SetInnerWidth( int w ) {}
int TTVPWindowForm::GetInnerWidth() const { return 0; }
void TTVPWindowForm::SetInnerHeight( int h ) {}
int TTVPWindowForm::GetInnerHeight() const { return 0; }
void TTVPWindowForm::SetInnerSize( int w, int h ) {}

// 表示ズーム関係
void TTVPWindowForm::SetZoom(tjs_int numer, tjs_int denom, bool set_logical ) {}
void TTVPWindowForm::SetZoomNumer( tjs_int n ) {}
tjs_int TTVPWindowForm::GetZoomNumer() const { return 1; }
void TTVPWindowForm::SetZoomDenom(tjs_int d) {}
tjs_int TTVPWindowForm::GetZoomDenom() const { return 1; }

// 画面表示向き取得
int TTVPWindowForm::GetDisplayOrientation() { return orientUnknown; }
int TTVPWindowForm::GetDisplayRotate() { return 0; }

void TTVPWindowForm::OnTouchScaling( double startdist, double currentdist, double cx, double cy, int flag ) {}
void TTVPWindowForm::OnTouchRotate( double startangle, double currentangle, double distance, double cx, double cy, int flag ) {}
void TTVPWindowForm::OnMultiTouch() {}

