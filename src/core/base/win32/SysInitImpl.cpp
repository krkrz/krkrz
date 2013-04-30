//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// System Initialization and Uninitialization
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include <dir.h>
#include <FileCtrl.hpp>
#include <delayimp.h>
#include <mmsystem.h>
#include <objbase.h>

#include "SysInitImpl.h"
#include "StorageIntf.h"
#include "StorageImpl.h"
#include "MsgIntf.h"
#include "GraphicsLoaderIntf.h"
#include "MainFormUnit.h"
#include "DebugIntf.h"
#include "tjsLex.h"
#include "LayerIntf.h"
#include "Random.h"
#include "DetectCPU.h"
#include "OptionsDesc.h"
#include "XP3Archive.h"
#include "ConfSettingsUnit.h"
#include "ScriptMgnIntf.h"
#include "XP3Archive.h"
#include "VersionFormUnit.h"
#include "EmergencyExit.h"

#include "tvpgl_ia32_intf.h"

#define TVP_NEED_UI_VERSION (((0x0001)<<16)+ 4) // needed development UI DLL version


//---------------------------------------------------------------------------
// global data
//---------------------------------------------------------------------------
AnsiString TVPNativeProjectDir;
AnsiString TVPNativeDataPath;
bool TVPProjectDirSelected = false;
bool TVPSystemIsBasedOnNT = false; // is system NT based ?
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// System security options
//---------------------------------------------------------------------------
// system security options are held inside the executable, where
// signature checker will refer. This enables the signature checker
// (or other security modules like XP3 encryption module) to check
// the changes which is not intended by the contents author.
const static char TVPSystemSecurityOptions[] =
"-- TVPSystemSecurityOptions disablemsgmap(0):forcedataxp3(0):acceptfilenameargument(0) --";
//---------------------------------------------------------------------------
int GetSystemSecurityOption(const char *name)
{
	size_t namelen = strlen(name);
	const char *p = strstr(TVPSystemSecurityOptions, name);
	if(!p) return 0;
	if(p[namelen] == '(' && p[namelen + 2] == ')')
		return p[namelen+1] - '0';
	return 0;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// delayed DLL load procedure hook
//---------------------------------------------------------------------------
// for supporting of "_inmm.dll" (C) irori
// http://www.geocities.co.jp/Playtown-Domino/8282/
//---------------------------------------------------------------------------
/*
note:
	_inmm.dll is a replacement of winmm.dll ( windows multimedia system dll ).
	_inmm.dll enables "MCI CD-DA supporting applications" to play musics using
	various way, including midi, mp3, wave or digital CD-DA, by applying a
	patch on those applications.

	TVP(kirikiri) system has a special structure of executable file --
	delayed loading of winmm.dll, in addition to compressed code/data area
	by the UPX executable packer.
	_inmm.dll's patcher can not recognize TVP's import area.

	So we must implement supporting of _inmm.dll alternatively.

	This function only works when -_inmm=yes or -inmm=yes option is specified at
	command line or embeded options area.
*/
//---------------------------------------------------------------------------
static HMODULE _inmm = NULL;
static FARPROC WINAPI DllLoadHook(dliNotification dliNotify,  DelayLoadInfo * pdli)
{
	if(dliNotify == dliNotePreLoadLibrary)
	{
		if(!stricmp(pdli->szDll, "winmm.dll"))
		{
			HMODULE mod = LoadLibrary("_inmm.dll");
			if(mod) _inmm = mod;
			return (FARPROC)mod;
		}
	}
	else if(dliNotify == dliNotePreGetProcAddress)
	{
		if(_inmm == pdli->hmodCur)
		{
			char buf[256];
			buf[1] = 0;
			strcpy(buf, pdli->dlp.szProcName);
			buf[0] = '_';
			return (FARPROC)GetProcAddress(_inmm, buf);
		}
	}

	return 0;
}
//---------------------------------------------------------------------------
static void RegisterDllLoadHook(void)
{
	bool flag = false;
	tTJSVariant val;
	if( TVPGetCommandLine(TJS_W("-_inmm"), &val) ||
		TVPGetCommandLine(TJS_W("-inmm" ), &val) )
	{
		// _inmm support
		ttstr str(val);
		if(str == TJS_W("yes"))
			flag = true;
	}
	if(flag) __pfnDliNotifyHook = DllLoadHook;
}
//---------------------------------------------------------------------------





#ifdef TVP_REPORT_HW_EXCEPTION
//---------------------------------------------------------------------------
// Hardware Exception Report Related
//---------------------------------------------------------------------------
// TVP's Hardware Exception Report comes with hacking RTL source.
// insert following code into rtl/soruce/except/xx.cpp
/*
typedef void __cdecl (*__dee_hacked_getExceptionObjectHook_type)(int ErrorCode,
		EXCEPTION_RECORD *P, unsigned long osEsp, unsigned long osERR, PCONTEXT ctx);
static __dee_hacked_getExceptionObjectHook_type __dee_hacked_getExceptionObjectHook = NULL;

extern "C"
{
	__dee_hacked_getExceptionObjectHook_type
		__cdecl __dee_hacked_set_getExceptionObjectHook(
		__dee_hacked_getExceptionObjectHook_type handler)
	{
		__dee_hacked_getExceptionObjectHook_type oldhandler;
		oldhandler = __dee_hacked_getExceptionObjectHook;
		__dee_hacked_getExceptionObjectHook = handler;
		return oldhandler;
	}
}
*/
// and insert following code into getExceptionObject
/*
	if(__dee_hacked_getExceptionObjectHook)
		__dee_hacked_getExceptionObjectHook(ErrorCode, P, osEsp, osERR, ctx);
*/
//---------------------------------------------------------------------------
typedef void __cdecl (*__dee_hacked_getExceptionObjectHook_type)(int ErrorCode,
		EXCEPTION_RECORD *P, unsigned long osEsp, unsigned long osERR, PCONTEXT ctx);
extern "C"
{
	extern __dee_hacked_getExceptionObjectHook_type
		__cdecl __dee_hacked_set_getExceptionObjectHook(
		__dee_hacked_getExceptionObjectHook_type handler);
}


//---------------------------------------------------------------------------
// data
#define TVP_HWE_MAX_CODES_AT_EIP 96
#define TVP_HWE_MAX_STACK_AT_ESP 80
#define TVP_HWE_MAX_STACK_DATA_DUMP  16
#define TVP_HWE_MAX_CALL_TRACE 32
#define TVP_HWE_MAX_CALL_CODE_DUMP 26
static bool TVPHWExcRaised = false;
struct tTVPHWExceptionData
{
	tjs_int Code;
	tjs_uint8 *EIP;
	tjs_uint32 *ESP;
	int AccessFlag; // for EAccessViolation (0=read, 1=write, 8=execute)
	void *AccessTarget; // for EAccessViolation
	CONTEXT Context; // OS exception context
	char Module[MAX_PATH]; // module name which caused the exception

	tjs_uint8 CodesAtEIP[TVP_HWE_MAX_CODES_AT_EIP];
	tjs_int CodesAtEIPLen;
	void * StackAtESP[TVP_HWE_MAX_STACK_AT_ESP];
	tjs_int StackAtESPLen;
	tjs_uint8 StackDumps[TVP_HWE_MAX_STACK_AT_ESP][TVP_HWE_MAX_STACK_DATA_DUMP];
	tjs_int StackDumpsLen[TVP_HWE_MAX_STACK_AT_ESP];

	void * CallTrace[TVP_HWE_MAX_CALL_TRACE];
	tjs_int CallTraceLen;
	tjs_uint8 CallTraceDumps[TVP_HWE_MAX_CALL_TRACE][TVP_HWE_MAX_CALL_CODE_DUMP];
	tjs_int CallTraceDumpsLen[TVP_HWE_MAX_CALL_TRACE];
};
static tTVPHWExceptionData TVPLastHWExceptionData;

HANDLE TVPHWExceptionLogHandle = NULL;
//---------------------------------------------------------------------------
static char TVPHWExceptionLogFilename[MAX_PATH];

static void TVPWriteHWELogFile()
{
	TVPEnsureDataPathDirectory();
	TJS_nstrcpy(TVPHWExceptionLogFilename, TVPNativeDataPath.c_str());
	TJS_nstrcat(TVPHWExceptionLogFilename, "hwexcept.log");
	TVPHWExceptionLogHandle = CreateFile(TVPHWExceptionLogFilename, GENERIC_WRITE,
		FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(TVPHWExceptionLogHandle == INVALID_HANDLE_VALUE) return;
	DWORD filesize;
	filesize = GetFileSize(TVPHWExceptionLogHandle, NULL);
	SetFilePointer(TVPHWExceptionLogHandle, filesize, NULL, FILE_BEGIN);

	// write header
	const char headercomment[] =
		"THIS IS A HARDWARE EXCEPTION LOG FILE OF KIRIKIRI. "
		"PLEASE SEND THIS FILE TO THE AUTHOR WITH *.console.log FILE. ";
	DWORD written = 0;
	for(int i = 0; i < 4; i++)
		WriteFile(TVPHWExceptionLogHandle, "----", 4, &written, NULL);
	WriteFile(TVPHWExceptionLogHandle, headercomment, sizeof(headercomment)-1,
		&written, NULL);
	for(int i = 0; i < 4; i++)
		WriteFile(TVPHWExceptionLogHandle, "----", 4, &written, NULL);
		

	// write version
	WriteFile(TVPHWExceptionLogHandle, &TVPVersionMajor,
		sizeof(TVPVersionMajor), &written, NULL);
	WriteFile(TVPHWExceptionLogHandle, &TVPVersionMinor,
		sizeof(TVPVersionMinor), &written, NULL);
	WriteFile(TVPHWExceptionLogHandle, &TVPVersionRelease,
		sizeof(TVPVersionRelease), &written, NULL);
	WriteFile(TVPHWExceptionLogHandle, &TVPVersionBuild,
		sizeof(TVPVersionBuild), &written, NULL);

	// write tTVPHWExceptionData
	WriteFile(TVPHWExceptionLogHandle, &TVPLastHWExceptionData,
		sizeof(TVPLastHWExceptionData), &written, NULL);


	// close the handle
	if(TVPHWExceptionLogHandle != INVALID_HANDLE_VALUE)
		CloseHandle(TVPHWExceptionLogHandle);

}
//---------------------------------------------------------------------------
void __cdecl TVP__dee_hacked_getExceptionObjectHook(int ErrorCode,
		EXCEPTION_RECORD *P, unsigned long osEsp, unsigned long osERR, PCONTEXT ctx)
{
	// exception hook function
	int len;
	tTVPHWExceptionData * d = &TVPLastHWExceptionData;

	d->Code = ErrorCode;

	// get AccessFlag and AccessTarget
	if(d->Code == 11) // EAccessViolation
	{
		d->AccessFlag = P->ExceptionInformation[0];
		d->AccessTarget = (void*)P->ExceptionInformation[1];
	}

	// get OS context
	if(!IsBadReadPtr(ctx, sizeof(*ctx)))
	{
		memcpy(&(d->Context), ctx, sizeof(*ctx));
	}
	else
	{
		memset(&(d->Context), 0, sizeof(*ctx));
	}

	// get codes at eip
	d->EIP = (tjs_uint8*)P->ExceptionAddress;
	len = TVP_HWE_MAX_CODES_AT_EIP;

	while(len)
	{
		if(!IsBadReadPtr(d->EIP, len))
		{
			memcpy(d->CodesAtEIP, d->EIP, len);
			d->CodesAtEIPLen = len;
			break;
		}
		len--;
	}

	// get module name
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery(d->EIP, &mbi, sizeof(mbi));
	if(mbi.State == MEM_COMMIT)
	{
		if(!GetModuleFileName((HMODULE)mbi.AllocationBase, d->Module,
			MAX_PATH))
		{
			d->Module[0] = 0;
		}
	}
	else
	{
		d->Module[0] = 0;
	}


	// get stack at esp
	d->ESP = (tjs_uint32*)osEsp;
	len = TVP_HWE_MAX_STACK_AT_ESP;

	while(len)
	{
		if(!IsBadReadPtr(d->ESP, len * sizeof(tjs_uint32)))
		{
			memcpy(d->StackAtESP, d->ESP, len * sizeof(tjs_uint32));
			d->StackAtESPLen = len;
			break;
		}
		len--;
	}

	// get data pointed by each stack data
	for(tjs_int i = 0; i<d->StackAtESPLen; i++)
	{
		void * base = d->StackAtESP[i];
		len = TVP_HWE_MAX_STACK_DATA_DUMP;
		while(len)
		{
			if(!IsBadReadPtr(base, len))
			{
				memcpy(d->StackDumps[i], base, len);
				d->StackDumpsLen[i] = len;
				break;
			}
			len--;
		}
	}

	// get call trace at esp
	d->CallTraceLen = 0;
	tjs_int p = 0;
	while(d->CallTraceLen < TVP_HWE_MAX_CALL_TRACE)
	{
		if(IsBadReadPtr(d->ESP + p, sizeof(tjs_uint32)))
			break;

		if(!IsBadReadPtr((void*)d->ESP[p], 4))
		{
			VirtualQuery((void*)d->ESP[p], &mbi, sizeof(mbi));
			if(mbi.State == MEM_COMMIT)
			{
				char module[MAX_PATH];
				if(GetModuleFileName((HMODULE)mbi.AllocationBase, module,
					MAX_PATH))
				{
					tjs_uint8 buf[16];
					if((DWORD)d->ESP[p] >= 16 &&
						!IsBadReadPtr((void*)((DWORD)d->ESP[p] - 16), 16))
					{
						memcpy(buf, (void*)((DWORD)d->ESP[p] - 16), 16);
						bool flag = false;
						if(buf[11] == 0xe8) flag = true;
						if(!flag)
						{
							for(tjs_int i = 0; i<15; i++)
							{
								if(buf[i] == 0xff && (buf[i+1] & 0x38) == 0x10)
								{
									flag = true;
									break;
								}
							}
						}
						if(flag)
						{
							// this seems to be a call code
							d->CallTrace[d->CallTraceLen] = (void *) d->ESP[p];
							d->CallTraceLen ++;

						}
					}
				}
			}
		}

		p ++;
	}

	// get data pointed by each call trace data
	for(tjs_int i = 0; i<d->CallTraceLen; i++)
	{
		void * base = d->CallTrace[i];
		len = TVP_HWE_MAX_STACK_DATA_DUMP;
		while(len)
		{
			if(!IsBadReadPtr(base, len))
			{
				memcpy(d->CallTraceDumps[i], base, len);
				d->CallTraceDumpsLen[i] = len;
				break;
			}
			len--;
		}
	}

	TVPHWExcRaised = true;

	TVPWriteHWELogFile();
}
//---------------------------------------------------------------------------
static void TVPDumpCPUFlags(ttstr &line, DWORD flags, DWORD bit, tjs_char *name)
{
	line += name;
	if(flags & bit)
		line += TJS_W("+ ");
	else
		line += TJS_W("- ");
}
//---------------------------------------------------------------------------
void TVPDumpOSContext(const CONTEXT &ctx)
{
	// dump OS context block
	tjs_char buf[256];

	// mask FP exception
	TJSSetFPUE();

	// - context flags
	ttstr line;
	TJS_sprintf(buf, TJS_W("Context Flags : 0x%08X [ "), ctx.ContextFlags);
	line += buf;
	if(ctx.ContextFlags & CONTEXT_DEBUG_REGISTERS)
		line += TJS_W("CONTEXT_DEBUG_REGISTERS ");
	if(ctx.ContextFlags & CONTEXT_FLOATING_POINT)
		line += TJS_W("CONTEXT_FLOATING_POINT ");
	if(ctx.ContextFlags & CONTEXT_SEGMENTS)
		line += TJS_W("CONTEXT_SEGMENTS ");
	if(ctx.ContextFlags & CONTEXT_INTEGER)
		line += TJS_W("CONTEXT_INTEGER ");
	if(ctx.ContextFlags & CONTEXT_CONTROL)
		line += TJS_W("CONTEXT_CONTROL ");
	if(ctx.ContextFlags & CONTEXT_EXTENDED_REGISTERS)
		line += TJS_W("CONTEXT_EXTENDED_REGISTERS ");
	line += TJS_W("]");

	TVPAddLog(line);


	// - debug registers
	TJS_sprintf(buf, TJS_W(
		"Debug Registers   : "
		"0:0x%08X  "
		"1:0x%08X  "
		"2:0x%08X  "
		"3:0x%08X  "
		"6:0x%08X  "
		"7:0x%08X  "),
			ctx.Dr0, ctx.Dr1, ctx.Dr2, ctx.Dr3, ctx.Dr6, ctx.Dr7);
	TVPAddLog(buf);


	// - Segment registers
	TJS_sprintf(buf, TJS_W("Segment Registers : GS:0x%04X  FS:0x%04X  ES:0x%04X  DS:0x%04X  CS:0x%04X  SS:0x%04X"),
		ctx.SegGs, ctx.SegFs, ctx.SegEs, ctx.SegDs, ctx.SegCs, ctx.SegSs);
	TVPAddLog(buf);

	// - Generic Integer Registers
	TJS_sprintf(buf, TJS_W("Integer Registers : EAX:0x%08X  EBX:0x%08X  ECX:0x%08X  EDX:0x%08X"),
		ctx.Eax, ctx.Ebx, ctx.Ecx, ctx.Edx);
	TVPAddLog(buf);

	// - Index Registers
	TJS_sprintf(buf, TJS_W("Index Registers   : ESI:0x%08X  EDI:0x%08X"),
		ctx.Esi, ctx.Edi);
	TVPAddLog(buf);

	// - Pointer Registers
	TJS_sprintf(buf, TJS_W("Pointer Registers : EBP:0x%08X  ESP:0x%08X  EIP:0x%08X"),
		ctx.Ebp, ctx.Esp, ctx.Eip);
	TVPAddLog(buf);

	// - Flag Register
	TJS_sprintf(buf, TJS_W("Flag Register     : 0x%08X [ "),
		ctx.EFlags);
	line = buf;
	TVPDumpCPUFlags(line, ctx.EFlags, (1<< 0), TJS_W("CF"));
	TVPDumpCPUFlags(line, ctx.EFlags, (1<< 2), TJS_W("PF"));
	TVPDumpCPUFlags(line, ctx.EFlags, (1<< 4), TJS_W("AF"));
	TVPDumpCPUFlags(line, ctx.EFlags, (1<< 6), TJS_W("ZF"));
	TVPDumpCPUFlags(line, ctx.EFlags, (1<< 7), TJS_W("SF"));
	TVPDumpCPUFlags(line, ctx.EFlags, (1<< 8), TJS_W("TF"));
	TVPDumpCPUFlags(line, ctx.EFlags, (1<< 9), TJS_W("IF"));
	TVPDumpCPUFlags(line, ctx.EFlags, (1<<10), TJS_W("DF"));
	TVPDumpCPUFlags(line, ctx.EFlags, (1<<11), TJS_W("OF"));
	TJS_sprintf(buf, TJS_W("IO%d "), (ctx.EFlags >> 12) & 0x03);
	line += buf;
	TVPDumpCPUFlags(line, ctx.EFlags, (1<<14), TJS_W("NF"));
	TVPDumpCPUFlags(line, ctx.EFlags, (1<<16), TJS_W("RF"));
	TVPDumpCPUFlags(line, ctx.EFlags, (1<<17), TJS_W("VM"));
	TVPDumpCPUFlags(line, ctx.EFlags, (1<<18), TJS_W("AC"));
	TVPDumpCPUFlags(line, ctx.EFlags, (1<<19), TJS_W("VF"));
	TVPDumpCPUFlags(line, ctx.EFlags, (1<<20), TJS_W("VP"));
	TVPDumpCPUFlags(line, ctx.EFlags, (1<<21), TJS_W("ID"));
	line += TJS_W("]");
	TVPAddLog(line);

	// - FP registers

	// -- control words
	TJS_sprintf(buf, TJS_W("FP Control Word : 0x%08X   FP Status Word : 0x%08X   FP Tag Word : 0x%08X"),
		ctx.FloatSave.ControlWord, ctx.FloatSave.StatusWord, ctx.FloatSave.TagWord);
	TVPAddLog(buf);

	// -- offsets/selectors
	TJS_sprintf(buf, TJS_W("FP Error Offset : 0x%08X   FP Error Selector : 0x%08X"),
		ctx.FloatSave.ErrorOffset, ctx.FloatSave.ErrorSelector);
	TJS_sprintf(buf, TJS_W("FP Data Offset  : 0x%08X   FP Data Selector  : 0x%08X"),
		ctx.FloatSave.DataOffset, ctx.FloatSave.DataSelector);

	// -- registers
	long double *ptr = (long double *)&(ctx.FloatSave.RegisterArea[0]);
	for(tjs_int i = 0; i < 8; i++)
	{
		TJS_sprintf(buf, TJS_W("FP ST(%d) : %28.20Lg 0x%04X%016I64X"), i,
			ptr[i], (unsigned int)*(tjs_uint16*)(((tjs_uint8*)(ptr + i)) + 8),
			*(tjs_uint64*)(ptr + i));
		TVPAddLog(buf);
	}

	// -- Cr0NpxState
	TJS_sprintf(buf, TJS_W("FP CR0 NPX State  : 0x%08X"), ctx.FloatSave.Cr0NpxState);
	TVPAddLog(buf);

	// -- SSE/SSE2 registers
	if(ctx.ContextFlags & CONTEXT_EXTENDED_REGISTERS)
	{
		// ExtendedRegisters is a area which meets fxsave and fxrstor instruction?
		#pragma pack(push,1)
		union xmm_t
		{
			struct
			{
				float sA;
				float sB;
				float sC;
				float sD;
			};
			struct
			{
				double dA;
				double dB;
			};
			struct
			{
				tjs_uint64 i64A;
				tjs_uint64 i64B;
			};
		};
		#pragma pack(pop)
		for(tjs_int i = 0; i < 8; i++)
		{
			xmm_t * xmm = (xmm_t *)(ctx.ExtendedRegisters + i * 16+ 0xa0);
			TJS_sprintf(buf,
				TJS_W("XMM %d : [ %15.8g %15.8g %15.8g %15.8g ] [ %24.16lg %24.16lg ] [ 0x%016I64X-0x%016I64X ]"),
				i,
				xmm->sD, xmm->sC, xmm->sB, xmm->sA,
				xmm->dB, xmm->dA,
				xmm->i64B, xmm->i64A);
			TVPAddLog(buf);
		}
		TJS_sprintf(buf, TJS_W("MXCSR : 0x%08X"),
			*(DWORD*)(ctx.ExtendedRegisters + 0x18));
		TVPAddLog(buf);
	}
}
//---------------------------------------------------------------------------
void TVPDumpHWException()
{
	// dump latest hardware exception if it exists

	if(!TVPHWExcRaised) return;
	TVPHWExcRaised = false;

	TVPOnError();

	tjs_char buf[256];
	tTVPHWExceptionData * d = &TVPLastHWExceptionData;

	TVPAddLog(ttstr(TVPHardwareExceptionRaised));

	ttstr line;

	line = TJS_W("Exception : ");

	tjs_char *p = NULL;
	switch(d->Code)
	{
	case  3:	p = TJS_W("Divide By Zero"); break;
	case  4:	p = TJS_W("Range Error"); break;
	case  5:	p = TJS_W("Integer Overflow"); break;
	case  6:	p = TJS_W("Invalid Operation"); break;
	case  7:	p = TJS_W("Zero Divide"); break;
	case  8:	p = TJS_W("Overflow"); break;
	case  9:	p = TJS_W("Underflow"); break;
	case 10:	p = TJS_W("Invalid Cast"); break;
	case 11:	p = TJS_W("Access Violation"); break;
	case 12:	p = TJS_W("Privilege Violation"); break;
	case 13:	p = TJS_W("Control C"); break;
	case 14:	p = TJS_W("Stack Overflow"); break;
	}

	if(p) line += p;

	if(d->Code == 11)
	{
		// EAccessViolation
		const tjs_char *mode = TJS_W("unknown");
		if(d->AccessFlag == 0)
			mode = TJS_W("read");
		else if(d->AccessFlag == 1)
			mode = TJS_W("write");
		else if(d->AccessFlag == 8)
			mode = TJS_W("execute");
		TJS_sprintf(buf, TJS_W("(%ls access to 0x%p)"), mode, d->AccessTarget);
		line += buf;
	}

	TJS_sprintf(buf, TJS_W("  at  EIP = 0x%p   ESP = 0x%p"), d->EIP, d->ESP);
	line += buf;
	if(d->Module[0])
	{
		line += TJS_W("   in ") + ttstr(d->Module);
	}

	TVPAddLog(line);

	// dump OS context
	TVPDumpOSContext(d->Context);

	// dump codes at EIP
	line = TJS_W("Codes at EIP : ");
	for(tjs_int i = 0; i<d->CodesAtEIPLen; i++)
	{
		TJS_sprintf(buf, TJS_W("0x%02X "), d->CodesAtEIP[i]);
		line += buf;
	}
	TVPAddLog(line);

	TVPAddLog(TJS_W("Stack data and data pointed by each stack data :"));

	// dump stack and data
	for(tjs_int s = 0; s<d->StackAtESPLen; s++)
	{
		TJS_sprintf(buf, TJS_W("0x%p (ESP+%3d) : 0x%p : "),
			(DWORD)d->ESP + s*sizeof(tjs_uint32),
			s*sizeof(tjs_uint32), d->StackAtESP[s]);
		line = buf;

		for(tjs_int i = 0; i<d->StackDumpsLen[s]; i++)
		{
			TJS_sprintf(buf, TJS_W("0x%02X "), d->StackDumps[s][i]);
			line += buf;
		}
		TVPAddLog(line);
	}

	// dump call trace
	TVPAddLog(TJS_W("Call Trace :"));
	for(tjs_int s = 0; s<d->CallTraceLen; s++)
	{
		TJS_sprintf(buf, TJS_W("0x%p : "),
			d->CallTrace[s]);
		line = buf;

		for(tjs_int i = 0; i<d->CallTraceDumpsLen[s]; i++)
		{
			TJS_sprintf(buf, TJS_W("0x%02X "), d->CallTraceDumps[s][i]);
			line += buf;
		}
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQuery((void*)d->CallTrace[s], &mbi, sizeof(mbi));
		if(mbi.State == MEM_COMMIT)
		{
			char module[MAX_PATH];
			if(GetModuleFileName((HMODULE)mbi.AllocationBase, module,
				MAX_PATH))
			{
				line += ttstr(ExtractFileName(module).c_str());
				TJS_sprintf(buf, TJS_W(" base 0x%p"), mbi.AllocationBase);
				line += buf;
			}
		}
		TVPAddLog(line);
	}
}
//---------------------------------------------------------------------------
#else
void TVPDumpHWException(void)
{
	// dummy
}
#endif
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// random generator initializer
//---------------------------------------------------------------------------
static BOOL CALLBACK TVPInitRandomEnumWinProc(HWND hwnd, LPARAM lparam)
{
	RECT r;
	GetWindowRect(hwnd, &r);
	TVPPushEnvironNoise(&hwnd, sizeof(hwnd));
	TVPPushEnvironNoise(&r, sizeof(r));
	DWORD procid, threadid;
	threadid = GetWindowThreadProcessId(hwnd, &procid);
	TVPPushEnvironNoise(&procid, sizeof(procid));
	TVPPushEnvironNoise(&threadid, sizeof(threadid));
	return TRUE;
}
//---------------------------------------------------------------------------
static void TVPInitRandomGenerator()
{
	// initialize random generator
	DWORD tick = GetTickCount();
	TVPPushEnvironNoise(&tick, sizeof(DWORD));
	GUID guid;
	CoCreateGuid(&guid);
	TVPPushEnvironNoise(&guid, sizeof(guid));
	DWORD id = GetCurrentProcessId();
	TVPPushEnvironNoise(&id, sizeof(id));
	id = GetCurrentThreadId();
	TVPPushEnvironNoise(&id, sizeof(id));
	SYSTEMTIME systime;
	GetSystemTime(&systime);
	TVPPushEnvironNoise(&systime, sizeof(systime));
	POINT pt;
	GetCursorPos(&pt);
	TVPPushEnvironNoise(&pt, sizeof(pt));

	EnumWindows((WNDENUMPROC)TVPInitRandomEnumWinProc, 0);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPInitializeBaseSystems
//---------------------------------------------------------------------------
void TVPInitializeBaseSystems()
{
	// set system archive delimiter
	tTJSVariant v;
	if(TVPGetCommandLine(TJS_W("-arcdelim"), &v))
		TVPArchiveDelimiter = ttstr(v)[0];

	// set default current directory
	{
		char drive[MAXDRIVE];
		char dir[MAXDIR];
		fnsplit(_argv[0], drive, dir, NULL, NULL);
		ttstr curdir(ttstr(drive)  + ttstr(dir));
		if(curdir.GetLastChar() != TJS_W('\\')) curdir += TJS_W('\\');
		TVPSetCurrentDirectory(curdir);
	}

	// load message map file
	bool load_msgmap = GetSystemSecurityOption("disablemsgmap") == 0;

	if(load_msgmap)
	{
		const tjs_char name_msgmap [] = TJS_W("msgmap.tjs");
		if(TVPIsExistentStorage(name_msgmap))
			TVPExecuteStorage(name_msgmap, NULL, false, TJS_W(""));
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// system initializer / uninitializer
//---------------------------------------------------------------------------
static tjs_uint TVPTotalPhysMemory = 0;
static void TVPInitProgramArgumentsAndDataPath(bool stop_after_datapath_got);
void TVPBeforeSystemInit()
{
	RegisterDllLoadHook();
		// register DLL delayed import hook to support _inmm.dll

	TVPInitProgramArgumentsAndDataPath(false); // ensure command line

#ifdef TVP_REPORT_HW_EXCEPTION
	__dee_hacked_set_getExceptionObjectHook(TVP__dee_hacked_getExceptionObjectHook);
		// register hook function for hardware exceptions
#endif

	Application->HintHidePause = 24*60*60*1000;
		// not to hide tool tip hint immediately
	Application->ShowHint = false;
	Application->ShowHint = true;
		// to ensure assigning new HintWindow Class defined in HintWindow.cpp 


	// randomize
	TVPInitRandomGenerator();

	// memory usage
	{
		MEMORYSTATUS status;
		status.dwLength = sizeof(status);
		GlobalMemoryStatus(&status);

		TVPPushEnvironNoise(&status, sizeof(status));

		TVPTotalPhysMemory = status.dwTotalPhys;

		TVPAddImportantLog(TJS_W("(info) Total physical memory : ") +
			ttstr((int)TVPTotalPhysMemory) );

		tTJSVariant opt;
		if(TVPGetCommandLine(TJS_W("-memusage"), &opt))
		{
			ttstr str(opt);
			if(str == TJS_W("low"))
				TVPTotalPhysMemory = 0; // assumes zero
		}

		if(TVPTotalPhysMemory <= 36*1024*1024)
		{
			// very very low memory, forcing to assume zero memory
			TVPTotalPhysMemory = 0;
		}

		if(TVPTotalPhysMemory < 48*1024*1024)
		{
			// extra low memory
			if(TJSObjectHashBitsLimit > 0)
				TJSObjectHashBitsLimit = 0;
			TVPSegmentCacheLimit = 0;
			TVPFreeUnusedLayerCache = true; // in LayerIntf.cpp
		}
		else if(TVPTotalPhysMemory < 64*1024*1024)
		{
			// low memory
			if(TJSObjectHashBitsLimit > 4)
				TJSObjectHashBitsLimit = 4;
		}
	}


	char buf[MAX_PATH];
	bool bufset = false;
	bool nosel = false;
	bool forcesel = false;

	bool forcedataxp3 = GetSystemSecurityOption("forcedataxp3") != 0;
	bool acceptfilenameargument = GetSystemSecurityOption("acceptfilenameargument") != 0;

	if(!forcedataxp3 && !acceptfilenameargument)
	{
		if(TVPGetCommandLine(TJS_W("-nosel")) || TVPGetCommandLine(TJS_W("-about")))
		{
			nosel = true;
		}
		else
		{
			for(tjs_int i = 1; i<_argc; i++)
			{
				if(_argv[i][0] == '-' &&
					_argv[i][1] == '-' && _argv[i][2] == 0)
					break;

				if(_argv[i][0] != '-')
				{
					// TODO: set the current directory
					strncpy(buf, _argv[i], MAX_PATH-1);
					buf[MAX_PATH-1] = '\0';
					if(DirectoryExists(buf)) // is directory?
						strcat(buf, "\\");

					TVPProjectDirSelected = true;
					bufset = true;
					nosel = true;
				}
			}
		}
	}

	// check "-sel" option, to force show folder selection window
	if(!forcedataxp3 && TVPGetCommandLine(TJS_W("-sel")))
	{
		// sel option was set
		if(bufset)
		{
			char path[MAX_PATH];
			char *dum = 0;
			GetFullPathName(buf, MAX_PATH-1, path, &dum);
			strcpy(buf, path);
			TVPProjectDirSelected = false;
			bufset = true;
		}
		nosel = true;
		forcesel = true;
	}

	// check "content-data" directory
	if(!forcedataxp3 && !nosel)
	{
		char tmp[MAX_PATH];
		strcpy(tmp, IncludeTrailingBackslash(ExtractFileDir(ParamStr(0))).c_str());
		strcat(tmp, "content-data");
		if(DirectoryExists(tmp))
		{
			strcat(tmp, "\\");
			strcpy(buf, tmp);
			TVPProjectDirSelected = true;
			bufset = true;
			nosel = true;
		}
	}

	// check "data.xp3" archive
 	if(!nosel)
	{
		char tmp[MAX_PATH];
		strcpy(tmp, IncludeTrailingBackslash(ExtractFileDir(ParamStr(0))).c_str());
		strcat(tmp, "data.xp3");
		if(FileExists(tmp))
		{
			strcpy(buf, tmp);
			TVPProjectDirSelected = true;
			bufset = true;
			nosel = true;
		}
	}

	// check "data.exe" archive
 	if(!nosel)
	{
		char tmp[MAX_PATH];
		strcpy(tmp, IncludeTrailingBackslash(ExtractFileDir(ParamStr(0))).c_str());
		strcat(tmp, "data.exe");
		if(FileExists(tmp))
		{
			strcpy(buf, tmp);
			TVPProjectDirSelected = true;
			bufset = true;
			nosel = true;
		}
	}

	// check self combined xpk archive
	if(!nosel)
	{
		if(TVPIsXP3Archive(TVPNormalizeStorageName(ParamStr(0))))
		{
			strcpy(buf, ParamStr(0).c_str());
			TVPProjectDirSelected = true;
			bufset = true;
			nosel = true;
		}
	}


	// check "data" directory
	if(!forcedataxp3 && !nosel)
	{
		char tmp[MAX_PATH];
		strcpy(tmp, IncludeTrailingBackslash(ExtractFileDir(ParamStr(0))).c_str());
		strcat(tmp, "data");
		if(DirectoryExists(tmp))
		{
			strcat(tmp, "\\");
			strcpy(buf, tmp);
			TVPProjectDirSelected = true;
			bufset = true;
			nosel = true;
		}
	}

	// decide a directory to execute or to show folder selection
	if(!bufset)
	{
		if(forcedataxp3) throw EAbort("Aborted");
		strcpy(buf, ExtractFileDir(ParamStr(0)).c_str());
		int curdirlen = strlen(buf);
		if(buf[curdirlen-1] != '\\') buf[curdirlen] = '\\', buf[curdirlen+1] = 0;
	}

	if(!forcedataxp3 && (!nosel || forcesel))
	{
		// load krdevui.dll ( TVP[KiRikiri] Development User Interface )
		HMODULE krdevui = LoadLibrary("krdevui.dll");
		if(!krdevui)
		{
			AnsiString toolspath = (IncludeTrailingBackslash(
					ExtractFilePath(ParamStr(0))) + "tools\\krdevui.dll");
			krdevui = LoadLibrary(toolspath.c_str());
		}

		if(!krdevui)
		{
			// cannot locate the dll
			throw Exception(
				ttstr(TVPCannnotLocateUIDLLForFolderSelection).AsAnsiString());
		}

		typedef int PASCAL (*UIShowFolderSelectorForm_t)(void *reserved, char *buf);
		typedef void PASCAL (*UIGetVersion_t)(DWORD *hi, DWORD *low);

		UIShowFolderSelectorForm_t	UIShowFolderSelectorForm;
		UIGetVersion_t				UIGetVersion;

		UIShowFolderSelectorForm =
			(UIShowFolderSelectorForm_t)GetProcAddress(krdevui, "UIShowFolderSelectorForm");
		UIGetVersion =
			(UIGetVersion_t)GetProcAddress(krdevui, "UIGetVersion");

		if(!UIShowFolderSelectorForm || !UIGetVersion)
		{
			FreeLibrary(krdevui);
			throw Exception(ttstr(TVPInvalidUIDLL).AsAnsiString());
		}

		DWORD h, l;
		UIGetVersion(&h, &l);
		if(h != TVP_NEED_UI_VERSION)
		{
			FreeLibrary(krdevui);
			throw Exception(ttstr(TVPInvalidUIDLL).AsAnsiString());
		}


		int result = UIShowFolderSelectorForm(Application->Handle, buf);

//		FreeLibrary(krdevui);
		// FIXME: the library should be freed as soon as finishing to use it.

		if(result == mrAbort)
		{
			// display the main window
		}
		else
		if(result == mrCancel)
		{
			// cancel
			throw EAbort("Canceled");
		}
		else
		if(result == mrOk)
		{
			// ok, prepare to execute the script
			TVPProjectDirSelected = true;
		}
	}

	// check project dir and store some environmental variables
	if(TVPProjectDirSelected)
	{
		Application->ShowMainForm=false;
	}

	tjs_int buflen = strlen(buf);
	if(buflen >= 1)
	{
		if(buf[buflen-1] != '\\') buf[buflen] = TVPArchiveDelimiter, buf[buflen+1] = 0;
	}

	TVPProjectDir = TVPNormalizeStorageName(buf);
	TVPSetCurrentDirectory(TVPProjectDir);
	TVPNativeProjectDir = buf;

	if(TVPProjectDirSelected)
	{
		TVPAddImportantLog(TJS_W("(info) Selected project directory : ") +
			TVPProjectDir);
	}
}
//---------------------------------------------------------------------------
static void TVPDumpOptions();
//---------------------------------------------------------------------------
static bool TVPHighTimerPeriod = false;
static UINT TVPTimeBeginPeriodRes = 0;
//---------------------------------------------------------------------------
void TVPAfterSystemInit()
{
	// ensure datapath directory
	TVPEnsureDataPathDirectory();

	// check CPU type
	TVPDetectCPU();

	// determine OS type
	OSVERSIONINFO osinfo;
	osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osinfo);
	TVPPushEnvironNoise(&osinfo, sizeof(osinfo));


	bool nt = osinfo.dwPlatformId == VER_PLATFORM_WIN32_NT;

	TVPSystemIsBasedOnNT = nt;


	TVPAllocGraphicCacheOnHeap = false; // always false since beta 20

	// determine maximum graphic cache limit
	tTJSVariant opt;
	tjs_int limitmb = -1;
	if(TVPGetCommandLine(TJS_W("-gclim"), &opt))
	{
		ttstr str(opt);
		if(str == TJS_W("auto"))
			limitmb = -1;
		else
			limitmb = (tjs_int)opt;
	}


	if(limitmb == -1)
	{
		if(TVPTotalPhysMemory <= 32*1024*1024)
			TVPGraphicCacheSystemLimit = 0;
		else if(TVPTotalPhysMemory <= 48*1024*1024)
			TVPGraphicCacheSystemLimit = 0;
		else if(TVPTotalPhysMemory <= 64*1024*1024)
			TVPGraphicCacheSystemLimit = 0;
		else if(TVPTotalPhysMemory <= 96*1024*1024)
			TVPGraphicCacheSystemLimit = 4;
		else if(TVPTotalPhysMemory <= 128*1024*1024)
			TVPGraphicCacheSystemLimit = 8;
		else if(TVPTotalPhysMemory <= 192*1024*1024)
			TVPGraphicCacheSystemLimit = 12;
		else if(TVPTotalPhysMemory <= 256*1024*1024)
			TVPGraphicCacheSystemLimit = 20;
		else if(TVPTotalPhysMemory <= 512*1024*1024)
			TVPGraphicCacheSystemLimit = 40;
		else
			TVPGraphicCacheSystemLimit = int(TVPTotalPhysMemory / (1024*1024*10));	// cachemem = physmem / 10
		TVPGraphicCacheSystemLimit *= 1024*1024;
	}
	else
	{
		TVPGraphicCacheSystemLimit = limitmb * 1024*1024;
	}


	if(TVPTotalPhysMemory <= 64*1024*1024)
		TVPSetFontCacheForLowMem();

//	TVPGraphicCacheSystemLimit = 1*1024*1024; // DEBUG 


	// check TVPGraphicSplitOperation option
	if(TVPGetCommandLine(TJS_W("-gsplit"), &opt))
	{
		ttstr str(opt);
		if(str == TJS_W("no"))
			TVPGraphicSplitOperationType = gsotNone;
		else if(str == TJS_W("int"))
			TVPGraphicSplitOperationType = gsotInterlace;
		else if(str == TJS_W("yes") || str == TJS_W("simple"))
			TVPGraphicSplitOperationType = gsotSimple;
		else if(str == TJS_W("bidi"))
			TVPGraphicSplitOperationType = gsotBiDirection;

	}

	// check TVPDefaultHoldAlpha option
	if(TVPGetCommandLine(TJS_W("-holdalpha"), &opt))
	{
		ttstr str(opt);
		if(str == TJS_W("yes") || str == TJS_W("true"))
			TVPDefaultHoldAlpha = true;
		else
			TVPDefaultHoldAlpha = false;
	}

	// check TVPJPEGFastLoad option
	if(TVPGetCommandLine(TJS_W("-jpegdec"), &opt)) // this specifies precision for JPEG decoding
	{
		ttstr str(opt);
		if(str == TJS_W("normal"))
			TVPJPEGLoadPrecision = jlpMedium;
		else if(str == TJS_W("low"))
			TVPJPEGLoadPrecision = jlpLow;
		else if(str == TJS_W("high"))
			TVPJPEGLoadPrecision = jlpHigh;

	}

	// dump option
	TVPDumpOptions();

	// initilaize x86 graphic routines
	TVPGL_IA32_Init();

	// load HBeam cursor
	Screen->Cursors[1] = LoadCursor(HInstance, "HBEAM");

	// timer precision
	UINT prectick = 1;
	if(TVPGetCommandLine(TJS_W("-timerprec"), &opt))
	{
		ttstr str(opt);
		if(str == TJS_W("high")) prectick = 1;
		if(str == TJS_W("higher")) prectick = 5;
		if(str == TJS_W("normal")) prectick = 10;
	}

        // draw thread num
        tjs_int drawThreadNum = 1;
        if (TVPGetCommandLine(TJS_W("-drawthread"), &opt)) {
          ttstr str(opt);
          if (str == TJS_W("auto"))
            drawThreadNum = 0;
          else
            drawThreadNum = (tjs_int)opt;
        }
        TVPDrawThreadNum = drawThreadNum;

	if(prectick)
	{
		// retrieve minimum timer resolution
		TIMECAPS tc;
		timeGetDevCaps(&tc, sizeof(tc));
		if(prectick < tc.wPeriodMin)
			TVPTimeBeginPeriodRes = tc.wPeriodMin;
		else
			TVPTimeBeginPeriodRes = prectick;
		if(TVPTimeBeginPeriodRes > tc.wPeriodMax)
			TVPTimeBeginPeriodRes = tc.wPeriodMax;
		// set timer resolution
		timeBeginPeriod(TVPTimeBeginPeriodRes);
		TVPHighTimerPeriod = true;
	}

	TVPPushEnvironNoise(&TVPCPUType, sizeof(TVPCPUType));

}
//---------------------------------------------------------------------------
void TVPBeforeSystemUninit()
{
	TVPDumpHWException(); // dump cached hw exceptoin
}
//---------------------------------------------------------------------------
void TVPAfterSystemUninit()
{
	// restore timer precision
	if(TVPHighTimerPeriod)
	{
		timeEndPeriod(TVPTimeBeginPeriodRes);
	}
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
bool TVPTerminated = false;
bool TVPTerminateOnWindowClose = true;
bool TVPTerminateOnNoWindowStartup = true;
int TVPTerminateCode = 0;
//---------------------------------------------------------------------------
void TVPTerminateAsync(int code)
{
	// do "A"synchronous temination of application
	TVPTerminated = true;
	TVPTerminateCode = code;

	// posting dummy message will prevent "missing WM_QUIT bug" in DirectDraw framework.
	if(TVPMainForm)
		::PostMessage(TVPMainForm->Handle, WM_USER+0x31/*dummy msg*/, 0, 0);

	Application->Terminate();

	if(TVPMainForm)
		::PostMessage(TVPMainForm->Handle, WM_USER+0x31/*dummy msg*/, 0, 0);
}
//---------------------------------------------------------------------------
void TVPTerminateSync(int code)
{
	// do synchronous temination of application (never return)
	TVPSystemUninit();
	exit(code);
}
//---------------------------------------------------------------------------
void TVPMainWindowClosed()
{
	// called from WindowIntf.cpp, caused by closing all window.
	if(TVPMainForm && !TVPMainForm->Visible && TVPTerminateOnWindowClose) TVPTerminateAsync();
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// GetCommandLine
//---------------------------------------------------------------------------
#define TVP_FIND_OPTION_MIN_OFS 512*1024  // min 512KB
#define TVP_FIND_OPTION_MAX_OFS 5*1024*1024 // max 5MB
//---------------------------------------------------------------------------
static TStringList * TVPGetEmbeddedOptions()
{
	AnsiString filename = ParamStr(0);
	TStream *stream = new TFileStream(filename, fmOpenRead|fmShareDenyWrite);

	char *buf = NULL;
	TStringList *ret = NULL;

	const char * errmsg = NULL;

	tjs_uint offset;
	try
	{
		ret = new TStringList();
		unsigned int size = stream->Size;
		if(size < TVP_FIND_OPTION_MIN_OFS)
			errmsg = "too small executable size."; // too small

		if(!errmsg)
		{
			if(size > TVP_FIND_OPTION_MAX_OFS) size = TVP_FIND_OPTION_MAX_OFS;
			buf = new char [size - TVP_FIND_OPTION_MIN_OFS];
			stream->Position = TVP_FIND_OPTION_MIN_OFS;
			stream->Read(buf, size - TVP_FIND_OPTION_MIN_OFS);

			// search "XOPT_EMBED_AREA_" ( aligned by 16 )
			static char XPT_1[] = "XOPT_EMBED\0\0";
			static char XPT_2[] = "_AREA_";
			char mark[17];
			strcpy(mark, XPT_1);
			strcat(mark, XPT_2); // combine string to avoid search the mark in code itself
			bool found = false;
			for(offset = TVP_FIND_OPTION_MIN_OFS; offset < size; offset += 16)
			{
				if(buf[offset - TVP_FIND_OPTION_MIN_OFS] == 'X')
				{
					if(!memcmp(buf+offset - TVP_FIND_OPTION_MIN_OFS,
						mark, 16))
					{
						ret->Text = buf + offset + 16 + 4 - TVP_FIND_OPTION_MIN_OFS;
						found = true;
						break;
					}
				}
			}

			if(!found) errmsg = "not found in executable.";
		}
	}
	catch(...)
	{
		if(buf) delete [] buf;
		if(ret) delete ret;
		delete stream;
		throw;
	}
	delete [] buf;
	delete stream;

	if(errmsg)
		TVPAddImportantLog(ttstr("(info) Loading executable embedded options failed (ignoring) : ") +
			errmsg);
	else
		TVPAddImportantLog("(info) Loading executable embedded options succeeded.");
	return ret;
}
//---------------------------------------------------------------------------
static TStringList * TVPGetConfigFileOptions(AnsiString filename)
{
	// load .cf file
	AnsiString errmsg;
	if(!FileExists(filename))
		errmsg = "file not found.";

	TStringList * ret = new TStringList();
	if(errmsg == "")
	{
		try
		{
			ret->LoadFromFile(filename);
		}
		catch(Exception & e)
		{
			errmsg = e.Message;
		}
		catch(...)
		{
			delete ret;
			throw;
		}
	}

	if(errmsg != "")
		TVPAddImportantLog(ttstr("(info) Loading configuration file \"") + filename.c_str() +
			"\" failed (ignoring) : " + errmsg.c_str());
	else
		TVPAddImportantLog(ttstr("(info) Loading configuration file \"") + filename.c_str() +
			"\" succeeded.");

	return ret;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

static ttstr TVPParseCommandLineOne(const ttstr &i)
{
	// value is specified
	const tjs_char *p, *o;
	p = o = i.c_str();
	p = TJS_strchr(p, '=');

	if(p == NULL) { return i + TJS_W("=yes"); }

	p++;

	ttstr optname(o, p - o);

	if(*p == TJS_W('\'') || *p == TJS_W('\"'))
	{
		// as an escaped string
		tTJSVariant v;
		TJSParseString(v, &p);

		return optname + ttstr(v);
	}
	else
	{
		// as a string
		return optname + p;
	}
}
//---------------------------------------------------------------------------
std::vector <ttstr> TVPProgramArguments;
static bool TVPProgramArgumentsInit = false;
static tjs_int TVPCommandLineArgumentGeneration = 0;
static bool TVPDataPathDirectoryEnsured = false;
//---------------------------------------------------------------------------
tjs_int TVPGetCommandLineArgumentGeneration() { return TVPCommandLineArgumentGeneration; }
//---------------------------------------------------------------------------
void TVPEnsureDataPathDirectory()
{
	if(!TVPDataPathDirectoryEnsured)
	{
		TVPDataPathDirectoryEnsured = true;
		// ensure data path existence
		if(!TVPCheckExistentLocalFolder(TVPNativeDataPath.c_str()))
		{
			ttstr msg("(info) Data path does not exist, trying to make it ... ");
			if(TVPCreateFolders(TVPNativeDataPath.c_str()))
				TVPAddImportantLog(msg + "ok.");
			else
				TVPAddImportantLog(msg + "failed.");
		}
	}
}
//---------------------------------------------------------------------------
static void PushAllCommandlineArguments()
{
	// store arguments given by commandline to "TVPProgramArguments"
	bool acceptfilenameargument = GetSystemSecurityOption("acceptfilenameargument") != 0;

	bool argument_stopped = false;
	if(acceptfilenameargument) argument_stopped = true;
	int file_argument_count = 0;
	for(tjs_int i = 1; i<_argc; i++)
	{
		if(argument_stopped)
		{
			ttstr arg_name_and_value = TJS_W("-arg") + ttstr(file_argument_count) + TJS_W("=")
				+ ttstr(_argv[i]);
			file_argument_count++;
			TVPProgramArguments.push_back(arg_name_and_value);
		}
		else
		{
			if(_argv[i][0] == '-')
			{
				if(_argv[i][1] == '-' && _argv[i][2] == 0)
				{
					// argument stopper
					argument_stopped = true;
				}
				else
				{
					ttstr value(_argv[i]);
					if(!TJS_strchr(value.c_str(), TJS_W('=')))
						value += TJS_W("=yes");
					TVPProgramArguments.push_back(TVPParseCommandLineOne(value));
				}
			}
		}
	}
}
//---------------------------------------------------------------------------
static void PushConfigFileOptions(TStringList * options)
{
	if(!options) return;
	for(int j = 0; j < options->Count; j++)
	{
		if(options->Strings[j].c_str()[0] != ';') // unless comment
			TVPProgramArguments.push_back(
				TVPParseCommandLineOne(TJS_W("-") + ttstr(options->Strings[j])));
	}
}
//---------------------------------------------------------------------------
static void TVPInitProgramArgumentsAndDataPath(bool stop_after_datapath_got)
{
	if(!TVPProgramArgumentsInit)
	{
		TVPProgramArgumentsInit = true;


		// find options from self executable image
		const int num_option_layers = 3;
		TStringList * options[num_option_layers];
		for(int i = 0; i < num_option_layers; i++) options[i] = NULL;
		try
		{
			// read embedded options and default configuration file
			options[0] = TVPGetEmbeddedOptions();
			options[1] = TVPGetConfigFileOptions(TConfMainFrame::GetConfigFileName(ParamStr(0)));

			// at this point, we need to push all exsting known options
			// to be able to see datapath
			PushAllCommandlineArguments();
			PushConfigFileOptions(options[1]); // has more priority
			PushConfigFileOptions(options[0]); // has lesser priority

			// read datapath
			tTJSVariant val;
			AnsiString config_datapath;
			if(TVPGetCommandLine(TJS_W("-datapath"), &val))
				config_datapath = ((ttstr)val).AsAnsiString();
			TVPNativeDataPath = TConfMainFrame::GetDataPathDirectory(config_datapath, ParamStr(0));

			if(stop_after_datapath_got) return;

			// read per-user configuration file
			options[2] = TVPGetConfigFileOptions(TConfMainFrame::GetUserConfigFileName(config_datapath, ParamStr(0)));

			// push each options into option stock
			// we need to clear TVPProgramArguments first because of the
			// option priority order.
			TVPProgramArguments.clear();
			PushAllCommandlineArguments();
			PushConfigFileOptions(options[2]); // has more priority
			PushConfigFileOptions(options[1]); // has more priority
			PushConfigFileOptions(options[0]); // has lesser priority
		}
		__finally
		{
			for(int i = 0; i < num_option_layers; i++)
				if(options[i]) delete options[i];
		}


		// set data path
		TVPDataPath = TVPNormalizeStorageName(TVPNativeDataPath);
		TVPAddImportantLog(ttstr("(info) Data path : ") + TVPDataPath);

		// set log output directory
		TVPSetLogLocation(TVPNativeDataPath);

		// increment TVPCommandLineArgumentGeneration
		TVPCommandLineArgumentGeneration++;
	}
}
//---------------------------------------------------------------------------
static void TVPDumpOptions()
{
	std::vector<ttstr>::const_iterator i;
	ttstr options(TJS_W("(info) Specified option(s) (earlier item has more priority) :"));
	if(TVPProgramArguments.size())
	{
		for(i = TVPProgramArguments.begin(); i != TVPProgramArguments.end(); i++)
		{
			options += TJS_W(" ");
			options += *i;
		}
	}
	else
	{
		options += TJS_W(" (none)");
	}
	TVPAddImportantLog(options);
}
//---------------------------------------------------------------------------
bool TVPGetCommandLine(const tjs_char * name, tTJSVariant *value)
{
	TVPInitProgramArgumentsAndDataPath(false);

	tjs_int namelen = TJS_strlen(name);
	std::vector<ttstr>::const_iterator i;
	for(i = TVPProgramArguments.begin(); i != TVPProgramArguments.end(); i++)
	{
		if(!TJS_strncmp(i->c_str(), name, namelen))
		{
			if(i->c_str()[namelen] == TJS_W('='))
			{
				// value is specified
				const tjs_char *p = i->c_str() + namelen + 1;
				if(value) *value = p;
				return true;
			}
			else if(i->c_str()[namelen] == 0)
			{
				// value is not specified
				if(value) *value = TJS_W("yes");
				return true;
			}
		}
	}
	return false;
}
//---------------------------------------------------------------------------
void TVPSetCommandLine(const tjs_char * name, const ttstr & value)
{
	TVPInitProgramArgumentsAndDataPath(false);

	tjs_int namelen = TJS_strlen(name);
	std::vector<ttstr>::iterator i;
	for(i = TVPProgramArguments.begin(); i != TVPProgramArguments.end(); i++)
	{
		if(!TJS_strncmp(i->c_str(), name, namelen))
		{
			if(i->c_str()[namelen] == TJS_W('=') || i->c_str()[namelen] == 0)
			{
				// value found
				*i = ttstr(i->c_str(), namelen) + TJS_W("=") + value;
				TVPCommandLineArgumentGeneration ++;
				if(TVPCommandLineArgumentGeneration == 0) TVPCommandLineArgumentGeneration = 1;
				return;
			}
		}
	}

	// value not found; insert argument into front
	TVPProgramArguments.insert(TVPProgramArguments.begin(), ttstr(name) + TJS_W("=") + value);
	TVPCommandLineArgumentGeneration ++;
	if(TVPCommandLineArgumentGeneration == 0) TVPCommandLineArgumentGeneration = 1;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPCheckPrintDataPath
//---------------------------------------------------------------------------
bool TVPCheckPrintDataPath()
{
	// print current datapath to stdout, then exit
	for(int i=1; i<_argc; i++)
	{
		if(!strcmp(_argv[i], "-printdatapath")) // this does not refer TVPGetCommandLine
		{
			TVPInitProgramArgumentsAndDataPath(true);
			printf("%s\n", TVPNativeDataPath.c_str());

			return true; // processed
		}
	}

	return false;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPCheckCmdDescription
//---------------------------------------------------------------------------
bool TVPCheckCmdDescription(void)
{
	// send a command description string when commandline option is "-cmddesc"
	tjs_int i;
	for(i=1; i<_argc; i++)
	{
		if(!strcmp(_argv[i], "-@cmddesc")) // this does not refer TVPGetCommandLine
		{
			AnsiString fn = _argv[i+2];
			TStream * stream = new TFileStream(fn, fmCreate|fmShareDenyWrite);
			try
			{
				ttstr str = TVPGetCommandDesc();
				stream->WriteBuffer(str.c_str(),
					str.GetLen()*sizeof(tjs_char));
			}
			catch(...)
			{
				delete stream;
				throw;
			}
			delete stream;

			HANDLE handle = OpenEvent(EVENT_ALL_ACCESS, FALSE, _argv[i+3]);
			if(!handle) return true; // processed but errored
			SetEvent(handle);
			CloseHandle(handle);

			return true; // processed
		}
	}

	return false;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPCheckAbout
//---------------------------------------------------------------------------
bool TVPCheckAbout(void)
{
	if(TVPGetCommandLine(TJS_W("-about")))
	{
		Sleep(600);
		tjs_char msg[80];
		TJS_sprintf(msg, TJS_W("(info) CPU clock (roughly) : %dMHz"), (int)TVPCPUClock);
		TVPAddImportantLog(msg);

		TVPShowVersionForm();
		return true;
	}

	return false;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPExecuteAsync
//---------------------------------------------------------------------------
static void TVPExecuteAsync(AnsiString progname)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNORMAL;

	BOOL ret =
		CreateProcess(
			NULL,
			progname.c_str(),
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			NULL,
			&si,
			&pi);

	if(ret)
	{
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		return;
	}

	throw Exception(ttstr(TVPExecutionFail).AsAnsiString());
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPWaitWritePermit
//---------------------------------------------------------------------------
static bool TVPWaitWritePermit(AnsiString fn)
{
	tjs_int timeout = 10; // 10/1 = 5 seconds
	while(true)
	{
		HANDLE h = CreateFile(fn.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(h != INVALID_HANDLE_VALUE)
		{
			CloseHandle(h);
			return true;
		}

		Sleep(500);
		timeout--;
		if(timeout == 0) return false;
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPShowUserConfig
//---------------------------------------------------------------------------
static void TVPShowUserConfig(AnsiString orgexe)
{
	TVPEnsureDataPathDirectory();

	Application->Title = ChangeFileExt(ExtractFileName(orgexe), "");
	TConfSettingsForm *form = new TConfSettingsForm(Application, true);
	form->InitializeConfig(orgexe);
	form->ShowModal();
	delete form;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPExecuteUserConfig
//---------------------------------------------------------------------------
bool TVPExecuteUserConfig()
{
	// check command line argument

	tjs_int i;
	bool process = false;
	for(i=1; i<_argc; i++)
	{
		if(!strcmp(_argv[i], "-userconf")) // this does not refer TVPGetCommandLine
			process = true;
	}

	if(!process) return false;

	// execute user config mode
	TVPShowUserConfig(ParamStr(0));

	// exit
	return true;
}
//---------------------------------------------------------------------------




