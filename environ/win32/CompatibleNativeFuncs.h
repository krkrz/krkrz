//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// native functions support
//---------------------------------------------------------------------------
#ifndef CompatibleNativeFuncsH
#define CompatibleNativeFuncsH

//---------------------------------------------------------------------------
// macros
//---------------------------------------------------------------------------
#if !defined(TVP_WNF_A) && !defined(TVP_WNF_B)
	#define TVP_NATIVE_FUNC_REG(type, calltype, name, args, module) \
		extern type (calltype * proc##name)args;
#endif

#if defined(TVP_WNF_A)
	#define TVP_NATIVE_FUNC_REG(type, calltype, name, args, module) \
		type (calltype * proc##name)args = NULL;
#endif

#if defined(TVP_WNF_B)
	#define TVP_NATIVE_FUNC_REG(type, calltype, name, args, module) \
		{ (void**)&proc##name, #name, module },

struct tTVPCompatibleNativeFunc
{
	void ** Ptr;
	const char * Name;
	const tjs_char* Module;
} static TVPCompatibleNativeFuncs[] = {
#endif
//---------------------------------------------------------------------------

TVP_NATIVE_FUNC_REG(
	BOOL, WINAPI, GetTouchInputInfo,
	(HTOUCHINPUT hTouchInput,UINT cInputs,PTOUCHINPUT pInputs,int cbSize),
	TJS_W("USER32.DLL"))

TVP_NATIVE_FUNC_REG(
	BOOL, WINAPI, CloseTouchInputHandle,
	(HTOUCHINPUT hTouchInput),
	TJS_W("USER32.DLL"))

TVP_NATIVE_FUNC_REG(
	BOOL, WINAPI, GetGestureInfo,
	(HGESTUREINFO hGestureInfo,PGESTUREINFO pGestureInfo),
	TJS_W("USER32.DLL"))

TVP_NATIVE_FUNC_REG(
	BOOL, WINAPI, CloseGestureInfoHandle,
	(HGESTUREINFO hGestureInfo),
	TJS_W("USER32.DLL"))

TVP_NATIVE_FUNC_REG(
	BOOL, WINAPI, RegisterTouchWindow,
	(HWND hWnd, ULONG ulFlags),
	TJS_W("USER32.DLL"))

TVP_NATIVE_FUNC_REG(
	BOOL, WINAPI, UnregisterTouchWindow,
	(HWND hWnd),
	TJS_W("USER32.DLL"))

TVP_NATIVE_FUNC_REG(
	BOOL, WINAPI, IsTouchWindow,
	(HWND hWnd, PULONG pulFlags),
	TJS_W("USER32.DLL"))
/////
//---------------------------------------------------------------------------
#if defined(TVP_WNF_B)
};
#endif

#undef TVP_NATIVE_FUNC_REG
//---------------------------------------------------------------------------
#endif
