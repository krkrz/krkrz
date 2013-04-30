//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Wide(UNICODE) native functions support
//---------------------------------------------------------------------------
#ifndef WideNativeFuncsH
#define WideNativeFuncsH

#include <windows.h>

//---------------------------------------------------------------------------
// macros
//---------------------------------------------------------------------------


#if !defined(TVP_WNF_A) && !defined(TVP_WNF_B)
	#define TVP_WIDE_NATIVE_FUNC_REG(type, calltype, name, args, module) \
		extern type (calltype * proc##name)args;
#endif

#if defined(TVP_WNF_A)
	#define TVP_WIDE_NATIVE_FUNC_REG(type, calltype, name, args, module) \
		type (calltype * proc##name)args = NULL;
#endif

#if defined(TVP_WNF_B)
	#define TVP_WIDE_NATIVE_FUNC_REG(type, calltype, name, args, module) \
		{ (void**)&proc##name, #name, module },
struct tTVPWideNativeFunc
{
	void ** Ptr;
	const char * Name;
	const char * Module;
} static TVPWideNativeFuncs[] = {
#endif
//---------------------------------------------------------------------------
	

TVP_WIDE_NATIVE_FUNC_REG(
	DWORD, WINAPI, GetGlyphOutlineW,
	( HDC, UINT, UINT, LPGLYPHMETRICS, DWORD, LPVOID, CONST MAT2 *),
	"GDI32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	BOOL, APIENTRY, GetTextExtentPoint32W,
	(HDC, LPCWSTR, int, LPSIZE),
	"GDI32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	BOOL, APIENTRY, GetTextExtentPointW,
	(HDC, LPCWSTR, int, LPSIZE),
	"GDI32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	BOOL, APIENTRY, TextOutW,
	(HDC, int, int, LPCWSTR, int),
	"GDI32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	DWORD, WINAPI, GetFileAttributesW,
	(LPCWSTR lpFileName),
	"KERNEL32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	BOOL, WINAPI, CreateDirectoryW,
	(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes),
	"KERNEL32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	HANDLE, WINAPI, CreateFileW,
	(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes, HANDLE hTemplateFile),
	"KERNEL32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	DWORD, WINAPI, GetTempPathW,
	(DWORD nBufferLength, LPWSTR lpBuffer),
	"KERNEL32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	BOOL, WINAPI, DeleteFileW,
	(LPCWSTR lpFileName),
	"KERNEL32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	BOOL, WINAPI, RemoveDirectoryW,
	(LPCWSTR lpPathName),
	"KERNEL32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	HANDLE, WINAPI, CreateMutexW,
	(LPSECURITY_ATTRIBUTES lpMutexAttributes,
	BOOL bInitialOwner,
	LPCWSTR lpName),
	"KERNEL32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	HANDLE, WINAPI, FindFirstFileW,
	(LPCWSTR lpFileName,
	LPWIN32_FIND_DATAW lpFindFileData),
	"KERNEL32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	BOOL, WINAPI, FindNextFileW,
	(HANDLE hFindFile,
	LPWIN32_FIND_DATAW lpFindFileData),
	"KERNEL32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	HCURSOR, WINAPI, LoadCursorFromFileW,
	(LPCWSTR lpFileName),
	"USER32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	HINSTANCE, WINAPI, ShellExecuteW,
	(HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile, LPCWSTR lpParameters,
	LPCWSTR lpDirectory, INT nShowCmd),
	"SHELL32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	LONG, WINAPI, RegQueryValueExW,
	(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType,
	 LPBYTE lpData, LPDWORD lpcbData),
	"ADVAPI32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	LONG, WINAPI, RegOpenKeyExW,
	(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired,
		::PHKEY phkResult),
	"ADVAPI32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	BOOL, WINAPI, SHGetSpecialFolderPathW,
	(HWND hwnd, LPWSTR pszPath, int csidl, BOOL fCreate),
	"shell32.dll")
	
///// wide Ç∂Ç·Ç»Ç¢ÇØÇ«ÅAÇ∆ÇËÇ†Ç¶Ç∏Ç±Ç±Ç÷
TVP_WIDE_NATIVE_FUNC_REG(
	BOOL, WINAPI, GetTouchInputInfo,
	(HTOUCHINPUT hTouchInput,UINT cInputs,PTOUCHINPUT pInputs,int cbSize),
	"USER32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	BOOL, WINAPI, CloseTouchInputHandle,
	(HTOUCHINPUT hTouchInput),
	"USER32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	BOOL, WINAPI, GetGestureInfo,
	(HGESTUREINFO hGestureInfo,PGESTUREINFO pGestureInfo),
	"USER32.DLL")

TVP_WIDE_NATIVE_FUNC_REG(
	BOOL, WINAPI, CloseGestureInfoHandle,
	(HGESTUREINFO hGestureInfo),
	"USER32.DLL")
/////
//---------------------------------------------------------------------------
#if defined(TVP_WNF_B)
};
#endif

#undef TVP_WIDE_NATIVE_FUNC_REG
//---------------------------------------------------------------------------
#endif
