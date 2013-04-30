//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Script Event Handling and Dispatching
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "EventImpl.h"
#include "MainFormUnit.h"
#include "ThreadIntf.h"
#include "TickCount.h"
#include "TimerIntf.h"
#include "SysInitIntf.h"
#include "DebugIntf.h"
#include "WindowImpl.h"
#include <mmsystem.h>
#include <ddraw.h>


//---------------------------------------------------------------------------
// TVPInvokeEvents
//---------------------------------------------------------------------------
bool TVPEventInvoked = false;
void TVPInvokeEvents()
{
	if(TVPEventInvoked) return;
	TVPEventInvoked = true;
	if(TVPMainForm)
	{
		TVPMainForm->InvokeEvents();
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// TVPEventReceived
//---------------------------------------------------------------------------
void TVPEventReceived()
{
	TVPEventInvoked = false;
	TVPMainForm->NotifyEventDelivered();
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPCallDeliverAllEventsOnIdle
//---------------------------------------------------------------------------
void TVPCallDeliverAllEventsOnIdle()
{
	if(TVPMainForm)
	{
		TVPMainForm->CallDeliverAllEventsOnIdle();
	}
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPBreathe
//---------------------------------------------------------------------------
static bool TVPBreathing = false;
void TVPBreathe()
{
	TVPEventDisabled = true; // not to call TVP events...
	TVPBreathing = true;
	try
	{
		Application->ProcessMessages(); // do Windows message pumping
	}
	catch(...)
	{
		TVPBreathing = false;
		TVPEventDisabled = false;
		throw;
	}

	TVPBreathing = false;
	TVPEventDisabled = false;
}
//---------------------------------------------------------------------------
bool TVPGetBreathing()
{
	// return whether now is in event breathing
	return TVPBreathing;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPSystemEventDisabledState
//---------------------------------------------------------------------------
void TVPSetSystemEventDisabledState(bool en)
{
	TVPMainForm->EventButton->Down = !en;
	if(!en) TVPDeliverAllEvents();
}
//---------------------------------------------------------------------------
bool TVPGetSystemEventDisabledState()
{
	return !TVPMainForm->EventButton->Down;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// tTVPContinuousHandlerCallLimitThread
//---------------------------------------------------------------------------
class tTVPContinuousHandlerCallLimitThread : public tTVPThread
{
	tjs_uint64 NextEventTick;
	tjs_uint64 Interval;
	tTVPThreadEvent Event;
	tTJSCriticalSection CS;

	bool Enabled;
	HWND UtilWindow;

public:
	tTVPContinuousHandlerCallLimitThread();
	~tTVPContinuousHandlerCallLimitThread();

protected:
	void Execute();

	void __fastcall UtilWndProc(Messages::TMessage &Msg)
	{
		Msg.Result =  DefWindowProc(UtilWindow, Msg.Msg, Msg.WParam, Msg.LParam);
		return;
	}

public:
	void SetEnabled(bool enabled);

	void SetInterval(tjs_uint64 interval) { Interval = interval; }

} static * TVPContinuousHandlerCallLimitThread = NULL;
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tTVPContinuousHandlerCallLimitThread::tTVPContinuousHandlerCallLimitThread()
	 : tTVPThread(true)
{
	NextEventTick = 0;
	Interval = (1<<TVP_SUBMILLI_FRAC_BITS)*1000/60; // default 60Hz
	Enabled = false;
	UtilWindow = AllocateHWnd(UtilWndProc);
	Resume();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tTVPContinuousHandlerCallLimitThread::~tTVPContinuousHandlerCallLimitThread()
{
	Terminate();
	Resume();
	Event.Set();
	WaitFor();
	DeallocateHWnd(UtilWindow);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tTVPContinuousHandlerCallLimitThread::Execute()
{
	while(!GetTerminated())
	{
		tjs_uint64 curtick = TVPGetTickCount() << TVP_SUBMILLI_FRAC_BITS;
		DWORD sleeptime;

		{	// thread-protected
			tTJSCriticalSectionHolder holder(CS);

			if(Enabled)
			{
				if(NextEventTick <= curtick)
				{
					TVPProcessContinuousHandlerEventFlag = true; // set flag to process event on next idle
					::PostMessage(UtilWindow, WM_APP+2, 0, 0);
					while(NextEventTick <= curtick) NextEventTick += Interval;
				}
				tjs_uint64 sleeptime_64 = NextEventTick - curtick;
				sleeptime = (DWORD)(sleeptime_64 >> TVP_SUBMILLI_FRAC_BITS) +
						((sleeptime_64 & ((1<<TVP_SUBMILLI_FRAC_BITS)-1))?1:0);
							// add 1 if fraction exists
			}
			else
			{
				sleeptime = 10000; // how long to sleep when disabled does not matter
			}


		}	// end-of-thread-protected

		if(sleeptime == 0) sleeptime = 1; // 0 will let thread sleeping forever ...
		Event.WaitFor(sleeptime);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tTVPContinuousHandlerCallLimitThread::SetEnabled(bool enabled)
{
	tTJSCriticalSectionHolder holder(CS);

	Enabled = enabled;
	if(enabled)
	{
		tjs_uint64 curtick = TVPGetTickCount() << TVP_SUBMILLI_FRAC_BITS;
		NextEventTick = ((curtick + 1) / Interval) * Interval;
		Event.Set();
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
static tjs_int TVPContinousHandlerLimitFrequency = 0;
//---------------------------------------------------------------------------
void TVPBeginContinuousEvent()
{
	// read commandline options
	static tjs_int ArgumentGeneration = 0;
	if(ArgumentGeneration != TVPGetCommandLineArgumentGeneration())
	{
		ArgumentGeneration = TVPGetCommandLineArgumentGeneration();

		tTJSVariant val;
		if( TVPGetCommandLine(TJS_W("-contfreq"), &val) )
		{
			TVPContinousHandlerLimitFrequency = (tjs_int)val;
		}
	}

	if(!TVPGetWaitVSync())
	{
		if(TVPContinousHandlerLimitFrequency == 0)
		{
			// no limit
			// this notifies continuous calling of TVPDeliverAllEvents.
			if(TVPMainForm) TVPMainForm->BeginContinuousEvent();
		}
		else
		{
			// has limit
			if(!TVPContinuousHandlerCallLimitThread)
				TVPContinuousHandlerCallLimitThread = new tTVPContinuousHandlerCallLimitThread();
			TVPContinuousHandlerCallLimitThread->SetInterval(
				 (1<<TVP_SUBMILLI_FRAC_BITS)*1000/TVPContinousHandlerLimitFrequency);
			TVPContinuousHandlerCallLimitThread->SetEnabled(true);
		}
	}


	TVPEnsureVSyncTimingThread();
	// if we wait vsync, the continuous handler will be executed at the every timing of
	// vsync.
}
//---------------------------------------------------------------------------
void TVPEndContinuousEvent()
{
	// anyway
	if(TVPContinuousHandlerCallLimitThread)
		TVPContinuousHandlerCallLimitThread->SetEnabled(false);

	// anyway
	if(TVPMainForm) TVPMainForm->EndContinuousEvent();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
static void TVPReleaseContinuousHandlerCallLimitThread()
{
	if(TVPContinuousHandlerCallLimitThread)
		delete TVPContinuousHandlerCallLimitThread,
			TVPContinuousHandlerCallLimitThread = NULL;
}
// to release TVPContinuousHandlerCallLimitThread at exit
static tTVPAtExit TVPTimerThreadUninitAtExit(TVP_ATEXIT_PRI_SHUTDOWN,
	TVPReleaseContinuousHandlerCallLimitThread);
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
static bool TVPWaitVSync = 0;
//---------------------------------------------------------------------------
bool TVPGetWaitVSync()
{
	static tjs_int ArgumentGeneration = 0;
	if(ArgumentGeneration != TVPGetCommandLineArgumentGeneration())
	{
		ArgumentGeneration = TVPGetCommandLineArgumentGeneration();
		TVPWaitVSync = false;

		tTJSVariant val;
		if(TVPGetCommandLine(TJS_W("-waitvsync"), &val))
		{
			ttstr str(val);
			if(str == TJS_W("yes"))
			{
				TVPWaitVSync = true;
				TVPEnsureDirectDrawObject();
			}
		}
	}
	return TVPWaitVSync;
}
//---------------------------------------------------------------------------e











//---------------------------------------------------------------------------
// VSync用のタイミングを発生させるためのスレッド
//---------------------------------------------------------------------------
class tTVPVSyncTimingThread : public tTVPThread
{
	DWORD SleepTime;
	tTVPThreadEvent Event;
	tTJSCriticalSection CS;
	DWORD VSyncInterval; //!< VSync の間隔(参考値)
	DWORD LastVBlankTick; //!< 最後の vblank の時間

	bool Enabled;
	HWND UtilWindow;

public:
	tTVPVSyncTimingThread();
	~tTVPVSyncTimingThread();

protected:
	void Execute();

	void __fastcall UtilWndProc(Messages::TMessage &Msg);

public:
	void MeasureVSyncInterval(); // VSyncInterval を計測する
} static * TVPVSyncTimingThread = NULL;
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tTVPVSyncTimingThread::tTVPVSyncTimingThread()
	 : tTVPThread(true)
{
	SleepTime = 1;
	LastVBlankTick = 0;
	VSyncInterval = 16; // 初期値。
	Enabled = false;
	UtilWindow = AllocateHWnd(UtilWndProc);
	MeasureVSyncInterval();
	Resume();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tTVPVSyncTimingThread::~tTVPVSyncTimingThread()
{
	Terminate();
	Resume();
	Event.Set();
	WaitFor();
	DeallocateHWnd(UtilWindow);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tTVPVSyncTimingThread::Execute()
{
	while(!GetTerminated())
	{
		// SleepTime と LastVBlankTick を得る
		DWORD sleep_time, last_vblank_tick;
		{	// thread-protected
			tTJSCriticalSectionHolder holder(CS);
			sleep_time = SleepTime;
			last_vblank_tick = LastVBlankTick;
		}

		// SleepTime 分眠る
		// LastVBlankTick から起算し、SleepTime 分眠る
		DWORD sleep_start_tick = timeGetTime();

		DWORD sleep_time_adj = sleep_start_tick - last_vblank_tick;

		if(sleep_time_adj < sleep_time)
		{
			Sleep(sleep_time - sleep_time_adj);
		}
		else
		{
			// 普通、メインスレッド内で Event.Set() したならば、
			// タイムスライス(長くて10ms) が終わる頃は
			// ここに来ているはずである。
			// sleep_time は通常 10ms より長いので、
			// ここに来るってのは異常。
			// よほどシステムが重たい状態になってると考えられる。
			// そこで立て続けに イベントをポストするわけにはいかないので
			// 適当な時間(本当に適当) 眠る。
			Sleep(5);
		}

		// イベントをポストする
		::PostMessage(UtilWindow, WM_APP+2, 0, (LPARAM)sleep_start_tick);

		Event.WaitFor(0x7fffffff); // vsync まで待つ
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void __fastcall tTVPVSyncTimingThread::UtilWndProc(Messages::TMessage &Msg)
{
	if(Msg.Msg != WM_APP+2)
	{
		Msg.Result =  DefWindowProc(UtilWindow, Msg.Msg, Msg.WParam, Msg.LParam);
		return;
	}

	// tTVPVSyncTimingThread から投げられたメッセージ

	// いま vblank 中？
	IDirectDraw2 * DirectDraw2 = TVPGetDirectDrawObjectNoAddRef();
	BOOL in_vblank = false;
	if(DirectDraw2)
		DirectDraw2->GetVerticalBlankStatus(&in_vblank);

	// 時間をチェック
	bool drawn = false;
//	DWORD vblank_wait_start = timeGetTime();

	// VSync 待ちを行う
	bool delayed = false;
	if(!drawn)
	{
		if(!in_vblank)
		{
			// vblank から抜けるまで待つ
			DWORD timeout_target_tick = timeGetTime() + 1;

			BOOL in_vblank = false;
			do
			{
				DirectDraw2->GetVerticalBlankStatus(&in_vblank);
			} while(in_vblank && (long)(timeGetTime() - timeout_target_tick) <= 0);

			// vblank に入るまで待つ
			in_vblank = true;
			do
			{
				DirectDraw2->GetVerticalBlankStatus(&in_vblank);
			} while(!in_vblank && (long)(timeGetTime() - timeout_target_tick) <= 0);

			if((int)(timeGetTime() - timeout_target_tick) > 0)
			{
				// フレームスキップが発生したと考えてよい
				delayed  =true;
			}
		}
	}

//	DWORD vblank_wait_end = timeGetTime();

	// タイマの時間原点を設定する
	if(!delayed)
	{
		tTJSCriticalSectionHolder holder(CS);
		LastVBlankTick = timeGetTime(); // これが次に眠る時間の起算点になる
	}
	else
	{
		tTJSCriticalSectionHolder holder(CS);
		LastVBlankTick += VSyncInterval; // これが次に眠る時間の起算点になる(おおざっぱ)
		if((long) (timeGetTime() - (LastVBlankTick + SleepTime)) <= 0)
		{
			// 眠った後、次に起きようとする時間がすでに過去なので眠れません
			LastVBlankTick = timeGetTime(); // 強制的に今の時刻にします
		}
	}

	// 画面の更新を行う (DrawDeviceのShowメソッドを呼ぶ)
	if(!drawn) TVPDeliverDrawDeviceShow();

	// もし vsync 待ちを行う直前、すでに vblank に入っていた場合は、
	// 待つ時間が長すぎたと言うことである
	if(in_vblank)
	{
		// その場合は SleepTime を減らす
		tTJSCriticalSectionHolder holder(CS);
		if(SleepTime > 8) SleepTime --;
	}
	else
	{
		// vblank で無かった場合は二つの場合が考えられる
		// 1. vblank 前だった
		// 2. vblank 後だった
		// どっちかは分からないが
		// SleepTime を増やす。ただしこれが VSyncInterval を超えるはずはない。
		tTJSCriticalSectionHolder holder(CS);
		SleepTime ++;
		if(SleepTime > VSyncInterval) SleepTime = VSyncInterval;
	}

	// タイマを起動する
	Event.Set();

	// ContinuousHandler を呼ぶ
	// これは十分な時間をとれるよう、vsync 待ちの直後に呼ばれる
	TVPProcessContinuousHandlerEventFlag = true; // set flag to invoke continuous handler on next idle

/*
static DWORD last_report_tick;

if(timeGetTime() > last_report_tick + 5000)
{
	last_report_tick = timeGetTime();
	TVPAddLog(TJS_W("SleepTime : ") + ttstr((int)SleepTime));
	TVPAddLog(TJS_W("VSyncInterval : ") + ttstr((int)VSyncInterval));
	TVPAddLog(TJS_W("VSync wait time : ") + ttstr((int)(vblank_wait_end - vblank_wait_start)));
}
*/
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tTVPVSyncTimingThread::MeasureVSyncInterval()
{
	TVPEnsureDirectDrawObject();

	DWORD vsync_interval = 10000;

	// vsync 周期を ms で得る。
	// ms 単位なのであまり正確な値は得られないが、まぁ特に問題ないこととする。

	// まず、DirectDraw が使用可能な場合、 WaitForVerticalBlank あるいは
	// GetScanLine を busy loop で監視して周期を得ることを試す。
	IDirectDraw2 * dd2 = TVPGetDirectDrawObjectNoAddRef();
	if(dd2)
	{
		// 参考: http://hpcgi1.nifty.com/MADIA/Vcbbs/wwwlng.cgi?print+200605/06050028.txt

		// それにしても GetScanLine は信用ならない
		// これで正常に周期を得られない環境が多すぎて断念
		DWORD start_tick;
		DWORD timeout;

		DWORD last_sync_tick;
		int repeat_count;
/*
		// まず、GetScanLine による周期の取得を試みる
		DWORD last_scanline = 65536;

		// 走査線が元に戻るまで空ループ
		// ここからが本来の計測。
		last_sync_tick = timeGetTime();
		timeout = 250;
		start_tick = timeGetTime();
		repeat_count = 0;
		while(timeGetTime() - start_tick < timeout)
		{
			DWORD scanline = 65536;
			if(FAILED(dd2->GetScanLine(&scanline))) scanline = 65536;
			if(scanline < last_scanline && last_scanline - scanline > 100)
			{
				// 走査線が元に戻った
				// 前回チェックした位置よりも前に値が戻った場合は
				// 復帰したとみなす
				// 前回と比べて100ライン以上戻ってることを確認する。
				// これは W.Dee の環境 (GeForce 7600 GT) で、なぜか
				// まれにスキャンラインが1だけ戻ることがあるという現象が
				// あったため。
				// しかしこの対策をとってもまともに周期を取得できない環境がある……
				DWORD tick = timeGetTime();
				if(repeat_count > 2)
				{
					// 最初の数回の結果は捨てる
					// 最小の間隔を記録する
					if(tick - last_sync_tick < vsync_interval)
						vsync_interval = tick - last_sync_tick;
				}
				last_sync_tick = tick;
				repeat_count ++;
			}
			last_scanline = scanline;
		}

		TVPAddLog(TJS_W("Rough VSync interval measured by GetScanLine() : " + ttstr((int)vsync_interval)));

		// vsync 周期は適切っぽい？
		if(vsync_interval < 6 || vsync_interval > 66)
		{
			TVPAddLog(TJS_W("VSync interval by GetScanLine() seems to be strange, trying WaitForVerticalBlank() ..."));
*/
			// どうも変
			vsync_interval = 10000;
			// WaitForVerticalBlank による測定を試みる
			// 最初のvblankを待つ
			dd2->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
			// 何回かループを回して間隔を測定する
			timeout = 250;
			last_sync_tick = timeGetTime();
			start_tick = timeGetTime();
			repeat_count = 0;
			while(timeGetTime() - start_tick < timeout)
			{
				// vblank から抜けるまで待つ
				BOOL in_vblank = false;
				do
				{
					dd2->GetVerticalBlankStatus(&in_vblank);
				} while(in_vblank && timeGetTime() - start_tick < timeout);

				DWORD aux_wait = timeGetTime();
				while(timeGetTime() - aux_wait < 2) ;
					// 1ms～2msほどまつ
					// どうも、短期間に vblank に入ったり vblank から抜けたりするような
					// 結果が得られることがある。
					// 詳しい原因は分からないが、ここに適当なウェイトを入れることで
					// なんとか対処を試みる。

				// vblank に入るまで待つ
				in_vblank = true;
				do
				{
					dd2->GetVerticalBlankStatus(&in_vblank);
				} while(!in_vblank && timeGetTime() - start_tick < timeout);

				DWORD tick = timeGetTime();
				if(repeat_count > 2)
				{
					// 最初の数回の結果は捨てる
					// 最小の間隔を記録する
					// 最小の間隔を記録する
					if(tick - last_sync_tick < vsync_interval)
						vsync_interval = tick - last_sync_tick;
				}
				last_sync_tick = tick;
				repeat_count ++;
			}

			TVPAddLog(TJS_W("Rough VSync interval measured by GetVerticalBlankStatus() : " + ttstr((int)vsync_interval)));
/*
		}
*/
	}


	// vsync 周期は適切っぽい？
	if(!dd2 || vsync_interval < 6 || vsync_interval > 66)
	{
		// どうもこれでも vsync 周期をうまくとれていないっぽい
		// そうなると、次は API による取得。
		// 参考: http://www.interq.or.jp/moonstone/person/del/zenact01.htm
		DWORD vsync_rate = 0;

		OSVERSIONINFO osvi;
		ZeroMemory(&osvi, sizeof(osvi));
		osvi.dwOSVersionInfoSize = sizeof(osvi);
		GetVersionEx(&osvi);

		if(osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			HDC dc = GetDC(0);
			vsync_rate = GetDeviceCaps(dc, VREFRESH);
			ReleaseDC(0, dc);
		}
		else if(osvi.dwMajorVersion == 4 && osvi.dwMinorVersion >= 10) // Windows 98 or lator
		{
			tTVP_devicemodeA dm;
			ZeroMemory(&dm, sizeof(tTVP_devicemodeA));
			dm.dmSize = sizeof(tTVP_devicemodeA);
			dm.dmDriverExtra = 0;
			EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, reinterpret_cast<DEVMODE*>(&dm));
			vsync_rate = dm.dmDisplayFrequency;
		}

		if(vsync_rate != 0)
			vsync_interval = 1000 / vsync_rate;
		else
			vsync_interval = 0;

		TVPAddLog(TJS_W("Rough VSync interval read from API : " + ttstr((int)vsync_interval)));
	}

	// vsync 周期は適切っぽい？
	if(vsync_interval < 6 || vsync_interval > 66)
	{
		TVPAddLog(TJS_W("Rough VSync interval still seems wrong, assuming default value (16)"));
		vsync_interval = 16;
	}

	VSyncInterval = vsync_interval;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TVPEnsureVSyncTimingThread()
{
	// (もし必要ならば) VSyncTimingThread を作成する
	if(TVPGetWaitVSync())
	{
		if(!TVPVSyncTimingThread)
			TVPVSyncTimingThread = new tTVPVSyncTimingThread();
	}
	else
	{
		TVPReleaseVSyncTimingThread();
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TVPReleaseVSyncTimingThread()
{
	if(TVPVSyncTimingThread)
		delete TVPVSyncTimingThread,
			TVPVSyncTimingThread = NULL;
}
//---------------------------------------------------------------------------
// to release TVPContinuousHandlerCallLimitThread at exit
static tTVPAtExit TVPVSyncTimingThreadUninitAtExit(TVP_ATEXIT_PRI_SHUTDOWN,
	TVPReleaseVSyncTimingThread);
//---------------------------------------------------------------------------





