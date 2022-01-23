//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// "System" class interface
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "tjsMessage.h"
#include "SystemIntf.h"
#include "SysInitIntf.h"
#include "SysInitImpl.h"
#include "MsgIntf.h"
#include "GraphicsLoaderIntf.h"
#include "EventIntf.h"
#include "LayerIntf.h"
#include "LayerBitmapIntf.h"
#include "Random.h"
#include "ScriptMgnIntf.h"
#include "DebugIntf.h"
#include "tjsGlobalStringMap.h"

extern int TVPGetOpenGLESVersion();

//---------------------------------------------------------------------------
// TVPFireOnApplicationActivateEvent
//---------------------------------------------------------------------------
void TVPFireOnApplicationActivateEvent(bool activate_or_deactivate)
{
	// get the script engine
	tTJS *engine = TVPGetScriptEngine();
	if(!engine)
		return; // the script engine had been shutdown

	// get System.onActivate or System.onDeactivate
	// and call it.
	iTJSDispatch2 * global = TVPGetScriptEngine()->GetGlobalNoAddRef();
	if(!global) return;

	tTJSVariant val;
	tTJSVariant val2;
	tTJSVariantClosure clo;
	tTJSVariantClosure func;

	try
	{
		tjs_error er;
		static ttstr System_name(TJSMapGlobalStringMap(TJS_W("System")));
		er = global->PropGet(TJS_MEMBERMUSTEXIST, System_name.c_str(), System_name.GetHint(), &val, global);
		if(TJS_FAILED(er)) return;

		if(val.Type() != tvtObject) return;

		clo = val.AsObjectClosureNoAddRef();

		if(clo.Object == NULL) return;

		static ttstr onActivate_name(TJSMapGlobalStringMap(TJS_W("onActivate")));
		static ttstr onDeactivate_name(TJSMapGlobalStringMap(TJS_W("onDeactivate")));
		clo.PropGet(TJS_MEMBERMUSTEXIST,
				activate_or_deactivate?
					onActivate_name.c_str():
					onDeactivate_name.c_str(),
				activate_or_deactivate?
					onActivate_name.GetHint():
					onDeactivate_name.GetHint(),
			&val2, NULL);

		if(val2.Type() != tvtObject) return;

		func = val2.AsObjectClosureNoAddRef();
	}
	catch(const eTJS &e)
	{
		// the system should not throw exceptions during retrieving the function
		TVPAddLog( TVPFormatMessage( TVPErrorInRetrievingSystemOnActivateOnDeactivate, e.GetMessage() ) );
		return;
	}

	if(func.Object != NULL) func.FuncCall(0, NULL, NULL, NULL, 0, NULL, NULL);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// tTJSNC_System
//---------------------------------------------------------------------------
tjs_uint32 tTJSNC_System::ClassID = -1;
tTJSNC_System::tTJSNC_System() : inherited(TJS_W("System"))
{
	// registration of native members

	TJS_BEGIN_NATIVE_MEMBERS(System)
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL_NO_INSTANCE(/*TJS class name*/System)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/System)
//----------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/terminate)
{
	int code = numparams > 0 ? static_cast<int>(*param[0]) : 0;
	TVPTerminateAsync(code);

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/terminate)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/exit)
{
	// this method does not return

	int code = numparams > 0 ? static_cast<int>(*param[0]) : 0;
	TVPTerminateSync(code);

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/exit)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/addContinuousHandler)
{
	// add function to continus handler list

	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();

	TVPAddContinuousHandler(clo);

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/addContinuousHandler)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/removeContinuousHandler)
{
	// remove function from continuous handler list

	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();

	TVPRemoveContinuousHandler(clo);

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/removeContinuousHandler)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/toActualColor)
{
	// convert color codes to 0xRRGGBB format.

	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	if(result)
	{
		tjs_uint32 color = (tjs_int)(*param[0]);
		color = TVPToActualColor(color);
		*result = (tjs_int)color;
	}

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/toActualColor)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/clearGraphicCache)
{
	// clear graphic cache
	TVPClearGraphicCache();

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/clearGraphicCache)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/touchImages)
{
	// try to cache graphics

	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	std::vector<ttstr> storages;
	tTJSVariantClosure array = param[0]->AsObjectClosureNoAddRef();

	tjs_int count = 0;
	while(true)
	{
		tTJSVariant val;
		if(TJS_FAILED(array.Object->PropGetByNum(0, count, &val, array.ObjThis)))
			break;
		if(val.Type() == tvtVoid) break;
		storages.push_back(ttstr(val));
		count++;
	}

	tjs_int64 limit = 0;
	tjs_uint64 timeout = 0;

	if(numparams >= 2 && param[1]->Type() != tvtVoid) limit = (tjs_int64)*param[1];
	if(numparams >= 3 && param[2]->Type() != tvtVoid) timeout = (tjs_int64)*param[2];

	TVPTouchImages(storages, limit, timeout);

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/touchImages)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/createUUID)
{
	// create UUID
	// return UUID string in form of "43abda37-c597-4646-a279-c27a1373af90"

	tjs_uint8 uuid[16];

	TVPGetRandomBits128(uuid);

	uuid[8] &= 0x3f;
	uuid[8] |= 0x80; // override clock_seq

	uuid[6] &= 0x0f;
	uuid[6] |= 0x40; // override version

	tjs_char buf[40];
	TJS_snprintf(buf, sizeof(buf)/sizeof(tjs_char),
TJS_W("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x"),
		uuid[ 0], uuid[ 1], uuid[ 2], uuid[ 3],
		uuid[ 4], uuid[ 5], uuid[ 6], uuid[ 7],
		uuid[ 8], uuid[ 9], uuid[10], uuid[11],
		uuid[12], uuid[13], uuid[14], uuid[15]);

	if(result) *result = tTJSVariant(buf);

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/createUUID)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/assignMessage)
{
	// assign system message

	if(numparams < 2) return TJS_E_BADPARAMCOUNT;

	ttstr id(*param[0]);
	ttstr msg(*param[1]);

	bool res = TJSAssignMessage(id.c_str(), msg.c_str());

	if(result) *result = tTJSVariant((tjs_int)res);

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/assignMessage)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/doCompact)
{
	// compact memory usage

	tjs_int level = TVP_COMPACT_LEVEL_MAX;

	if(numparams >= 1 && param[0]->Type() != tvtVoid)
		level = (tjs_int)*param[0];

	TVPDeliverCompactEvent(level);

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/doCompact)
//----------------------------------------------------------------------

//--properties

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(versionString)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = TVPGetVersionString();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL(versionString)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(versionInformation)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = TVPGetVersionInformation();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL(versionInformation)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(eventDisabled)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = TVPGetSystemEventDisabledState();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TVPSetSystemEventDisabledState(param->operator bool());
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL(eventDisabled)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(graphicCacheLimit)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = (tjs_int)TVPGetGraphicCacheLimit();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TVPSetGraphicCacheLimit((tjs_int)*param);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL(graphicCacheLimit)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(platformName)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = TVPGetPlatformName();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL(platformName)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(osName)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = TVPGetOSName();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL(osName)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(exitOnWindowClose)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = TVPTerminateOnWindowClose;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TVPTerminateOnWindowClose = 0!=(tjs_int)*param;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL(exitOnWindowClose)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(drawThreadNum)
{
        TJS_BEGIN_NATIVE_PROP_GETTER
          {
            *result = TVPDrawThreadNum;
            return TJS_S_OK;
          }
        TJS_END_NATIVE_PROP_GETTER
        TJS_BEGIN_NATIVE_PROP_SETTER
          {
            TVPDrawThreadNum = (tjs_int)*param;
            return TJS_S_OK;
          }
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL(drawThreadNum)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(processorNum)
{
        TJS_BEGIN_NATIVE_PROP_GETTER
          {
            *result = TVPGetProcessorNum();
            return TJS_S_OK;
          }
        TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL(processorNum)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(exeBits)
{
        TJS_BEGIN_NATIVE_PROP_GETTER
          {
#ifdef TJS_64BIT_OS
            *result = 64;
#else
            *result = 32;
#endif
            return TJS_S_OK;
          }
        TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL(exeBits)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(osBits)
{
        TJS_BEGIN_NATIVE_PROP_GETTER
          {
          	*result = TVPGetOSBits();
            return TJS_S_OK;
          }
        TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL(osBits)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(exitOnNoWindowStartup)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = TVPTerminateOnNoWindowStartup;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER
	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TVPTerminateOnNoWindowStartup = 0!=(tjs_int)*param;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL(exitOnNoWindowStartup)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(isWindows) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
#ifdef WIN32
		*result = (tjs_int)1;
#else
		*result = (tjs_int)0;
#endif
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL(isWindows)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(isAndroid) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
#ifdef ANDROID
		*result = (tjs_int)1;
#else
		*result = (tjs_int)0;
#endif
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL(isAndroid)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(openGLESVersion) {
	TJS_BEGIN_NATIVE_PROP_GETTER {
		*result = (tjs_int)TVPGetOpenGLESVersion();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL(openGLESVersion)
//----------------------------------------------------------------------
	TJS_END_NATIVE_MEMBERS


	// register default "exceptionHandler" member
	tTJSVariant val((iTJSDispatch2*)NULL, (iTJSDispatch2*)NULL);
	static ttstr exceptionHandler_name(TJSMapGlobalStringMap(TJS_W("exceptionHandler")));
	PropSet(TJS_MEMBERENSURE, exceptionHandler_name.c_str(), exceptionHandler_name.GetHint(), &val, this);

	// and onActivate, onDeactivate
	static ttstr onActivate_name(TJSMapGlobalStringMap(TJS_W("onActivate")));
	PropSet(TJS_MEMBERENSURE, onActivate_name.c_str(), onActivate_name.GetHint(), &val, this);
	static ttstr onDeactivate_name(TJSMapGlobalStringMap(TJS_W("onDeactivate")));
	PropSet(TJS_MEMBERENSURE, onDeactivate_name.c_str(), onDeactivate_name.GetHint(), &val, this);
}
//---------------------------------------------------------------------------
tTJSNativeInstance * tTJSNC_System::CreateNativeInstance()
{
	// this class cannot create an instance
	TVPThrowExceptionMessage(TVPCannotCreateInstance);

	return NULL;
}
//---------------------------------------------------------------------------




