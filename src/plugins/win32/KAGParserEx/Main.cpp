#include "KAGParserEx.hpp"

#define EXPORT(hr) extern "C" __declspec(dllexport) hr __stdcall

#ifdef _MSC_VER
#pragma comment(linker, "/EXPORT:V2Link=_V2Link@4")
#pragma comment(linker, "/EXPORT:V2Unlink=_V2Unlink@0")
#endif

int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
	return 1;
}

static iTJSDispatch2 *origKAGParser = NULL;

//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
EXPORT(HRESULT) V2Link(iTVPFunctionExporter *exporter)
{
	TVPInitImportStub(exporter);

	tTJSNI_KAGParser::initMethod();
	
	iTJSDispatch2 * global = TVPGetScriptDispatch();
	if (global) {
		tTJSVariant val;
		if (TJS_SUCCEEDED(global->PropGet(0, TVP_KAGPARSER_EX_CLASSNAME, NULL, &val, global))) {
			origKAGParser = val.AsObject();
			val.Clear();
		}
		iTJSDispatch2 * tjsclass = tTJSNC_KAGParser::CreateNativeClass();
		val = tTJSVariant(tjsclass);
		tjsclass->Release();
		global->PropSet(TJS_MEMBERENSURE, TVP_KAGPARSER_EX_CLASSNAME, NULL, &val, global);
		global->Release();
	}
	GlobalRefCountAtInit = TVPPluginGlobalRefCount;
	return S_OK;
}
//---------------------------------------------------------------------------
EXPORT(HRESULT) V2Unlink()
{
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;

	iTJSDispatch2 * global = TVPGetScriptDispatch();
	if (global)	{
		global->DeleteMember(0, TVP_KAGPARSER_EX_CLASSNAME, NULL, global);
		if (origKAGParser) {
			tTJSVariant val(origKAGParser);
			origKAGParser->Release();
			origKAGParser = NULL;
			global->PropSet(TJS_MEMBERENSURE, TVP_KAGPARSER_EX_CLASSNAME, NULL, &val, global);
		}
		global->Release();
	}

	tTJSNI_KAGParser::doneMethod();
	
	TVPUninitImportStub();
	return S_OK;
}
