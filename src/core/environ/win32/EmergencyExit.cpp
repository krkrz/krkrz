//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Emergency Exit Hotkey Handler
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "ThreadIntf.h"

#include "EmergencyExit.h"
#include "Random.h"
#include "MainFormUnit.h"

#include "tvpgl_ia32_intf.h"

#include <memory>


double TVPCPUClock = 0; // CPU clock, in MHz
tTVPCPUClockAccuracy TVPCPUClockAccuracy = ccaNotSet;

/*
	a pushing of Ctrl + Alt + F12 over than two seconds can
	cause TVP's force suicide.

	this thread also measures CPU clock.
*/

//---------------------------------------------------------------------------
// TTVPEmergencyExitThread
//---------------------------------------------------------------------------
class tTVPEmergencyExitThread : public tTVPThread
{
	tTVPThreadEvent Event;

public:
	tTVPEmergencyExitThread() : tTVPThread(true)
	{

		// get pam
		DWORD pam = 1;// process affinity mask
		HANDLE hp = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
		if(hp)
		{
			DWORD sam = 1;
			GetProcessAffinityMask(hp, &pam, &sam);
			CloseHandle(hp);

			// set tam to run only upon one processor (for proper working with rdtsc)
			DWORD tam = pam;
			tjs_int bit;
			for(bit = 0; bit <= 31; bit++)
			{
				if(tam & (1<<bit)) break;
			}
			bit ++;
			for(; bit <= 31; bit++)
			{
				tam &= ~(1<<bit);
			}

			SetThreadAffinityMask(GetHandle(), tam);
		}

		Resume();
	}

	~tTVPEmergencyExitThread()
	{
		Terminate();
		Event.Set();
		WaitFor();
	}

	void Execute(void);
};
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void tTVPEmergencyExitThread::Execute(void)
{
	DWORD pushstarttime;

	DWORD prevtime = GetTickCount();
	tjs_uint64 prevtsc = TVPGetTSC();

	while(!GetTerminated())
	{
		bool status =
			(GetAsyncKeyState(VK_CONTROL)&0x8000)&&
			(GetAsyncKeyState(VK_MENU)&0x8000)&&
			(GetAsyncKeyState(VK_F12)&0x8000);

		DWORD curtime = GetTickCount();
		tjs_uint64 curtsc = TVPGetTSC();
		TVPPushEnvironNoise(&curtime, sizeof(curtime));
		TVPPushEnvironNoise(&curtsc, sizeof(curtsc));

		if(TVPCPUClockAccuracy == ccaNotSet)
		{
			if(curtime - prevtime > 200)
			{
				if(prevtsc != curtsc && prevtsc && curtsc)
				{
					TVPCPUClock = (double)((curtsc - prevtsc) / 100) / (double)(curtime - prevtime) *
						0.1f;
					TVPCPUClockAccuracy = ccaRough;
				}

				prevtime = curtime;
				prevtsc = curtsc;
			}
		}
		else if(TVPCPUClockAccuracy == ccaRough)
		{
			if(curtime - prevtime > 20000)
			{
				if(prevtsc != curtsc && prevtsc && curtsc)
				{
					TVPCPUClock = (double)((curtsc - prevtsc) / 10000) /
						(double)(curtime - prevtime) * 10.0f;
					TVPCPUClockAccuracy = ccaAccurate;
				}

				prevtime = curtime;
				prevtsc = curtsc;
			}
		}



		if(!status)
		{
			pushstarttime = curtime;
		}
		else
		{
			if(curtime  - pushstarttime > 3000)
			{
				// force suicide
				DWORD pid;
				GetWindowThreadProcessId(Application->Handle, &pid);
				HANDLE hp=OpenProcess(PROCESS_TERMINATE, FALSE, pid);
				if(hp)
				{
					TerminateProcess(hp, 0);
					CloseHandle(hp);
				}
			}
		}

		{
			MEMORYSTATUS status;
			status.dwLength = sizeof(status);
			GlobalMemoryStatus(&status);
			TVPPushEnvironNoise(&status, sizeof(status));
		}

		if(TVPCPUClockAccuracy == ccaNotSet)
			Event.WaitFor(200);
		else
			Event.WaitFor(500);

		if(TVPMainFormAlive && Application != NULL && Application->Handle != NULL)
		{
			PostMessage(Application->Handle, WM_USER+0x35/*WM_KEEPALIVE*/, 0, 0);
			// Send wakeup message to the main window.
			// VCL sometimes waits message that never come (so hangs up).
			// This message will wake the VCL up.
		}
	}
}
//---------------------------------------------------------------------------
static std::auto_ptr<tTVPEmergencyExitThread>
	TVPEmergencyExitThread(new tTVPEmergencyExitThread);
//---------------------------------------------------------------------------


