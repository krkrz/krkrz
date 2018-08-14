/**
 * まずはスレッドとして実装したが、実際はタイマー動作か……
 * タイマーを継承して実装で十分かな描画待ちをするわけじゃないし
 * ただ、負荷が高くなるとイベントが詰まる
 */

#include "tjsCommHead.h"

#include "DrawCycleTimer.h"
#include "WindowIntf.h"
#include "DebugIntf.h"
#include "MsgIntf.h"
#include "TickCount.h"
#include "TimerIntf.h"

#include "UserEvent.h"

#if 0
//---------------------------------------------------------------------------
tTVPDrawCycleTimer::tTVPDrawCycleTimer(tTJSNI_BaseWindow* owner) : OwnerWindow(owner) {
	tTVPTimerThread::Add(this);
#ifdef ANDROID
	Application->addActivityEventHandler( this );
#endif
}
//---------------------------------------------------------------------------
tTVPDrawCycleTimer::~tTVPDrawCycleTimer() {
	tTVPTimerThread::Remove(this);
	ZeroPendingCount();
#ifdef ANDROID
	Application->removeActivityEventHandler( this );
#endif
}
//---------------------------------------------------------------------------
#ifdef ANDROID
void tTVPDrawCycleTimer::onResume() {
	ForceDisable = false;
	ResetDrawCycle();
}
//---------------------------------------------------------------------------
void tTVPDrawCycleTimer::onPause() {
	ForceDisable = true;
	SetEnabled( false );
}
#endif
//---------------------------------------------------------------------------
void tTVPDrawCycleTimer::Fire(tjs_uint n) {
	if( OwnerWindow == nullptr ) return;

//	tjs_uint64 start = TVPGetTickCount();
	//if( !ForceDisable ) OwnerWindow->StartDrawingInternal();
	if( !ForceDisable ) OwnerWindow->OnDraw();
//	tjs_uint64 duration = TVPGetTickCount() - start;
//	TVPAddLog( to_tjs_string( duration ) );

	tjs_uint64 tick = TVPGetTickCount();
	tjs_uint64 lasttick = LastDrawTick;
	if( lasttick != 0 ) {
		tjs_uint32 drawInterval = 1000/DrawCycle;
		tjs_uint32 interval;
		if( lasttick < tick ) {
			interval = tick - lasttick;
		} else {
			interval = (tjs_uint32)( ((((tjs_uint64)lasttick)+0xFFFFFFFFULL) - ((tjs_uint64)tick))&0xFFFFFFFF );
		}
		TVPAddLog( to_tjs_string( interval ) );
		if( interval > (drawInterval*4) ) {
			// 描画サイクルの4倍より遅れている、混みあっているので、タイマー再設定して遅らせる
			ResetDrawCycle();
			LastDrawTick = 0;
		}
	} else {
		LastDrawTick = tick;
	}
}
//---------------------------------------------------------------------------
void tTVPDrawCycleTimer::ResetDrawCycle()
{
	if( DrawCycle != 0 ) {
		SetEnabled( false );
		SetEnabled( true );
	}
}
//---------------------------------------------------------------------------
void tTVPDrawCycleTimer::SetDrawCycle( tjs_uint32 cycle ) {
	if( DrawCycle != cycle ) {
		bool enable = cycle != 0;
		bool prevEnable = DrawCycle != 0;
		if( prevEnable ) SetEnabled( false );
		DrawCycle = cycle;
		if( cycle != 0 ) {
			SetInterval( (tjs_uint64)((1ULL << TVP_SUBMILLI_FRAC_BITS)*1000) / cycle );
		}
		if( !prevEnable || !enable ) {
			SetEnabled( enable );
		}
		LastDrawTick = 0;
	}
}
//---------------------------------------------------------------------------
#else
 //---------------------------------------------------------------------------
tTVPDrawCycleTimer::tTVPDrawCycleTimer( tTJSNI_BaseWindow* owner ) : OwnerWindow( owner ), EventQueue( this, &tTVPDrawCycleTimer::Proc ) {
#ifdef ANDROID
	Application->addActivityEventHandler( this );
#endif
	EventQueue.Allocate();
	StartTread();
}
//---------------------------------------------------------------------------
tTVPDrawCycleTimer::~tTVPDrawCycleTimer() {
#ifdef ANDROID
	Application->removeActivityEventHandler( this );
#endif
	Terminate();
	Event.Set();
	WaitFor();
	EventQueue.Deallocate();
}
//---------------------------------------------------------------------------
#ifdef ANDROID
void tTVPDrawCycleTimer::onResume() {
	ForceDisable = false;
	ResetDrawCycle();
}
//---------------------------------------------------------------------------
void tTVPDrawCycleTimer::onPause() {
	ForceDisable = true;
	//SetEnabled( false );
	ResetDrawCycle();
}
#endif
//---------------------------------------------------------------------------
void tTVPDrawCycleTimer::Execute() {
	if( !OwnerWindow ) return;
	tjs_uint sleeptime = Interval;
	while( !GetTerminated() ) {
		Event.WaitFor( sleeptime );
		if( !ForceDisable && !GetTerminated() ) {
			tjs_uint64 start = GetTickCount();
			EventQueue.PostEvent( NativeEvent( TVP_EV_DRAW_TIMING_THREAD ) );
			//OwnerWindow->StartDrawingInternal();
			Event.WaitFor( 0 );	// 描画処理待ち

			tjs_uint64 duration = GetTickCount() - start;
			tjs_uint64 interval = Interval;
			if( duration >= interval )
				sleeptime = 1;
			else
				sleeptime = (tjs_uint)( interval - duration );
		} else {
			sleeptime = 0; // infinity
		}
	}
}
//---------------------------------------------------------------------------
void tTVPDrawCycleTimer::Proc( NativeEvent& ev ) {
	if( ev.Message == TVP_EV_DRAW_TIMING_THREAD && !GetTerminated() ) {
		OwnerWindow->StartDrawingInternal();
	} else {
		EventQueue.HandlerDefault( ev );
	}
}
//---------------------------------------------------------------------------
void tTVPDrawCycleTimer::ResetDrawCycle() {
	Event.Set();
}
//---------------------------------------------------------------------------
void tTVPDrawCycleTimer::SetDrawCycle( tjs_uint32 cycle ) {
	if( DrawCycle != cycle ) {
		DrawCycle = cycle;
		Interval = 1000 / cycle;
		LastDrawTick = 0;
		Event.Set();
	}
}
//---------------------------------------------------------------------------
void tTVPDrawCycleTimer::FinishDrawing() {
	Event.Set();
}
#endif
