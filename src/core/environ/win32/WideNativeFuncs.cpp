//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Wide(UNICODE) native functions support
//---------------------------------------------------------------------------
#include "tjsCommHead.h"


#define TVP_WNF_A
#include "WideNativeFuncs.h"
#undef TVP_WNF_A

#undef WideNativeFuncsH

#define TVP_WNF_B
#include "WideNativeFuncs.h"
#undef TVP_WNF_B

#define TVP_USE_WIDE_NATIVE_FUNCS

//---------------------------------------------------------------------------
// TVPInitWideNativeFunctions
//---------------------------------------------------------------------------
static void TVPInitWideNativeFunctions()
{
#ifdef TVP_USE_WIDE_NATIVE_FUNCS
	// retrieve function pointer from each module

	// Note that GetGlyphOutlieW on Win9x seems not to work properly.

	OSVERSIONINFO osinfo;
	osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osinfo);
	bool nt = osinfo.dwPlatformId == VER_PLATFORM_WIN32_NT;

	if(!nt) return; // windows NT preferable.

	const tjs_int n = sizeof(TVPWideNativeFuncs)/sizeof(tTVPWideNativeFunc);
	for(tjs_int i = 0; i<n; i++)
	{
		tTVPWideNativeFunc * p = TVPWideNativeFuncs + i;
		HMODULE module = GetModuleHandle(p->Module);
		if(module) *(p->Ptr) = (void*)GetProcAddress(module, p->Name);
	}
#endif
}
//---------------------------------------------------------------------------
#pragma startup TVPInitWideNativeFunctions 64
