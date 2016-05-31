#include <windows.h>
#include "tp_stub.h"
#include <stdio.h>
#include <string>
#include "EditResource.h"


static void addMember(iTJSDispatch2 *dispatch, const tjs_char *name, iTJSDispatch2 *member) {
	tTJSVariant var = tTJSVariant(member);
	member->Release();
	dispatch->PropSet( TJS_MEMBERENSURE, name, NULL, &var, dispatch );
}

static void delMember(iTJSDispatch2 *dispatch, const tjs_char *name) {
	dispatch->DeleteMember( 0, name, NULL, dispatch );
}

#define TJS_NATIVE_CLASSID_NAME ClassID_ResourceEdit
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;
class NI_ResourceEdit : public tTJSNativeInstance {
public:
	NI_ResourceEdit() {}
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {return S_OK;}
	void TJS_INTF_METHOD Invalidate() {}
};
static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_ResourceEdit() {
	return new NI_ResourceEdit();
}
static iTJSDispatch2 * Create_NC_ResourceEdit() {
	tTJSNativeClassForPlugin * classobj = TJSCreateNativeClassForPlugin(TJS_W("ResourceEdit"), Create_NI_ResourceEdit);

	TJS_BEGIN_NATIVE_MEMBERS(/*TJS class name*/ResourceEdit)

		TJS_DECL_EMPTY_FINALIZE_METHOD

		TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
			/*var.name*/_this,
			/*var.type*/NI_ResourceEdit,
			/*TJS class name*/ResourceEdit)
		{
			return TJS_S_OK;
		}
		TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/ResourceEdit)

		// exeFileName, iconFileName
		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/updateIcon)
		{
			if (numparams < 2) return TJS_E_BADPARAMCOUNT;
			int ret = ReplaceIcon( (const tjs_char *)*(param[0]->AsStringNoAddRef()), (const tjs_char *)*(param[1]->AsStringNoAddRef()) );
			if( result ) *result = ret;
			return TJS_S_OK;
		}
		TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/updateIcon)
			
		// 
		// exeFileName, binaryFileName, resourceType, resourceName
		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/updateBinaryResource)
		{
			if (numparams < 4) return TJS_E_BADPARAMCOUNT;
			int ret = UpdateBinaryResource( (const tjs_char *)*(param[0]->AsStringNoAddRef()), (const tjs_char *)*(param[1]->AsStringNoAddRef()),
				(const tjs_char *)*(param[2]->AsStringNoAddRef()), (const tjs_char *)*(param[3]->AsStringNoAddRef()));
			if( result ) *result = ret;
			return TJS_S_OK;
		}
		TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/updateBinaryResource)

	TJS_END_NATIVE_MEMBERS

	return classobj;
}
#undef TJS_NATIVE_CLASSID_NAME
//---------------------------------------------------------------------------
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved) {
	return 1;
}
//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
extern "C" HRESULT _stdcall V2Link(iTVPFunctionExporter *exporter) {
	TVPInitImportStub(exporter);
	iTJSDispatch2 * global = TVPGetScriptDispatch();
	if (global) {
		addMember(global, L"ResourceEdit", Create_NC_ResourceEdit());
		global->Release();
	}
	GlobalRefCountAtInit = TVPPluginGlobalRefCount;
	return S_OK;
}
//---------------------------------------------------------------------------
extern "C" HRESULT _stdcall V2Unlink() {
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;
	iTJSDispatch2 * global = TVPGetScriptDispatch();
	if (global)	{
		delMember(global, L"ResourceEdit");
		global->Release();
	}
	TVPUninitImportStub();
	return S_OK;
}
//---------------------------------------------------------------------------
