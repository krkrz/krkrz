//---------------------------------------------------------------------------

#ifndef CheckDebuggeeUnitH
#define CheckDebuggeeUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <map>
#include <string>
#include <windows.h>
#include "CommandBuffer.h"
//---------------------------------------------------------------------------
class DebuggeeCheckThread : public TThread
{
  typedef struct tagTHREADNAME_INFO
  {
    DWORD dwType;     // 0x1000 になること
    LPCSTR szName;    // 名前へのポインタ（ユーザーアドレス空間上）
    DWORD dwThreadID; // スレッド ID (-1=呼び出しスレッド)
    DWORD dwFlags;    // 将来のために予約，0 にすること
  } THREADNAME_INFO;
private:
	void SetName();
	PROCESS_INFORMATION		proc_info_;
	AnsiString	command_line_;
	AnsiString	work_folder_;
	AnsiString	debug_string_;
	std::map<LPVOID,std::string>	dll_info_;
	bool		is_first_break_;
	bool		is_request_break_;
	CommandBuffer	command_;
	void*	debuggee_comm_area_addr_;
	int		debuggee_comm_area_size_;
	DWORD	debug_continue_status_;

	void __fastcall GetParameters();
	void __fastcall SetProcInfo();
	void __fastcall ShowLastError();
	void __fastcall WakeupDebugee();
	void __fastcall SetDebugString();
	void __fastcall OnBreak();
	void __fastcall GetCommand();
	void __fastcall CheckBreakRequest();

	int __fastcall HandleDebugEvent( DEBUG_EVENT& debug );
	int __fastcall HandleDebugString( DEBUG_EVENT& debug );
	int __fastcall HandleDllLoad( DEBUG_EVENT& debug );
	int __fastcall HandleDllUnload( DEBUG_EVENT& debug );
	int __fastcall HandleDebugException( DEBUG_EVENT& debug );

	void __fastcall PushDllInfo( LPVOID baseaddr, const std::string& filename );
	bool __fastcall GetDllInfo( LPVOID baseaddr, std::string& filename );

	void __fastcall DebuggeeCheckThreadTerminate(TObject *Sender);
protected:
	void __fastcall Execute();
public:
	__fastcall DebuggeeCheckThread(bool CreateSuspended);
};
//---------------------------------------------------------------------------
#endif
