//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// "Plugins" class implementation / Service for plug-ins
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include <algorithm>
#include <functional>
#include "ScriptMgnIntf.h"
#include "PluginImpl.h"
#include "StorageImpl.h"

#include "MsgImpl.h"
#include "SysInitIntf.h"

#include "tjsHashSearch.h"
#include "EventIntf.h"
#include "TransIntf.h"
#include "tjsArray.h"
#include "tjsDictionary.h"
#include "DebugIntf.h"
#include "FuncStubs.h"
#include "tjs.h"

#pragma pack(push, 8)
	///  tvpsnd.h needs packing size of 8
	#include "tvpsnd.h"
#pragma pack(pop)

#include "FilePathUtil.h"
#include "Application.h"


//---------------------------------------------------------------------------
// export table
//---------------------------------------------------------------------------
static tTJSHashTable<ttstr, void *> TVPExportFuncs;
static bool TVPExportFuncsInit = false;
void TVPAddExportFunction(const char *name, void *ptr)
{
	TVPExportFuncs.Add(name, ptr);
}
void TVPAddExportFunction(const tjs_char *name, void *ptr)
{
	TVPExportFuncs.Add(name, ptr);
}
static void TVPInitExportFuncs()
{
	if(TVPExportFuncsInit) return;
	TVPExportFuncsInit = true;


	// Export functions
	TVPExportFunctions();
}
//---------------------------------------------------------------------------
struct tTVPFunctionExporter : iTVPFunctionExporter
{
	bool TJS_INTF_METHOD QueryFunctions(const tjs_char **name, void **function,
		tjs_uint count);
	bool TJS_INTF_METHOD QueryFunctionsByNarrowString(const char **name,
		void **function, tjs_uint count);
} static TVPFunctionExporter;
//---------------------------------------------------------------------------
bool TJS_INTF_METHOD tTVPFunctionExporter::QueryFunctions(const tjs_char **name, void **function,
		tjs_uint count)
{
	// retrieve function table by given name table.
	// return false if any function is missing.
	bool ret = true;
	ttstr tname;
	for(tjs_uint i = 0; i<count; i++)
	{
		tname = name[i];
		void ** ptr = TVPExportFuncs.Find(tname);
		if(ptr)
			function[i] = *ptr;
		else
			function[i] = NULL, ret= false;
	}
	return ret;
}
//---------------------------------------------------------------------------
bool TJS_INTF_METHOD tTVPFunctionExporter::QueryFunctionsByNarrowString(
	const char **name, void **function, tjs_uint count)
{
	// retrieve function table by given name table.
	// return false if any function is missing.
	bool ret = true;
	ttstr tname;
	for(tjs_uint i = 0; i<count; i++)
	{
		tname = name[i];
		void ** ptr = TVPExportFuncs.Find(tname);
		if(ptr)
			function[i] = *ptr;
		else
			function[i] = NULL, ret= false;
	}
	return ret;
}
//---------------------------------------------------------------------------
extern "C" iTVPFunctionExporter * __stdcall TVPGetFunctionExporter()
{
	// for external applications
	TVPInitExportFuncs();
    return &TVPFunctionExporter;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TVPThrowPluginUnboundFunctionError(const char *funcname)
{
	TVPThrowExceptionMessage(TVPPluginUnboundFunctionError, funcname);
}
//---------------------------------------------------------------------------
void TVPThrowPluginUnboundFunctionError(const tjs_char *funcname)
{
	TVPThrowExceptionMessage(TVPPluginUnboundFunctionError, funcname);
}
//---------------------------------------------------------------------------
#if 0
// .dll Windows
// .dylib MAC
// .so Linux/Android
// 拡張子があったら環境ごとに書き換え、なかったら追加。
// .tmp は書き換えない
// 環境によってはプレフィックスにlibが必要か。
#ifndef _WIN32
#include <dlfcn.h>
#endif
class tTVPSharedLibrary {
#ifdef _WIN32
	static inline HINSTANCE LoadLibrary( const char* path ) { return ::LoadLibrary( path ); }
	static inline void* GetProcAddress( HINSTANCE handle, const char* func_name ) { return ::GetProcAddress(handle,func_name); }
	static inline void FreeLibrary( HINSTANCE handle ) { ::FreeLibrary(handle); }
	HINSTANCE handle_;
#else
	static inline void* LoadLibrary( const char* path ) { return dlopen( path, RTLD_LAZY|RTLD_GLOBAL ); } // or RTLD_LAZY|RTLD_LOCAL
	static inline void* GetProcAddress( void* handle, const char* func_name ) { return dlsym(handle,func_name); }
	static inline void FreeLibrary( void* handle ) { dlclose(handle); }
	void* handle_;
	// const char* err = dlerror();
#endif
public:
	tTVPSharedLibrary() : handle_(NULL) {}
	~tTVPSharedLibrary() {
		Close();
	}

	void Open(const char* path) {
		handle_ = LoadLibrary(path);
	}
	void Close() {
		if( handle_ ) {
			FreeLibrary( handle_ );
			handle_ = NULL;
		}
	}
	void* GetFunction( const char* name ) {
		if( handle_ ) {
			return GetProcAddress( handle_, name );
		}
		return NULL;
	}
};
#endif
//---------------------------------------------------------------------------
// Plug-ins management
//---------------------------------------------------------------------------
struct tTVPPlugin
{
	ttstr Name;
	//HINSTANCE Instance;

	tTVPPluginHolder *Holder;

	//ITSSModule *TSSModule;

	tTVPV2LinkProc V2Link;
	tTVPV2UnlinkProc V2Unlink;

	/*
	tTVPGetModuleInstanceProc GetModuleInstance;
	tTVPGetModuleThreadModelProc GetModuleThreadModel;
	tTVPShowConfigWindowProc ShowConfigWindow;
	tTVPCanUnloadNowProc CanUnloadNow;
	*/

	std::vector<ttstr> SupportedExts;

	tTVPPlugin(const ttstr & name, ITSSStorageProvider *storageprovider);
	~tTVPPlugin();

	bool Uninit();
};
//---------------------------------------------------------------------------
tTVPPlugin::tTVPPlugin(const ttstr & name, ITSSStorageProvider *storageprovider)
{
	Name = name;

	//Instance = NULL;
	Holder = NULL;
	//TSSModule = NULL;


	V2Link = NULL;
	V2Unlink = NULL;

	/*
	GetModuleInstance = NULL;
	GetModuleThreadModel = NULL;
	ShowConfigWindow = NULL;
	CanUnloadNow = NULL;
	*/

	// load DLL
	Holder = new tTVPPluginHolder(name);
#if 0
	Instance = LoadLibrary(Holder->GetLocalName().AsStdString().c_str());
	if(!Instance)
	{
		delete Holder;
		TVPThrowExceptionMessage(TVPCannotLoadPlugin, name);
	}

	try
	{
		// retrieve each functions
		V2Link = (tTVPV2LinkProc)
			GetProcAddress(Instance, "V2Link");
		V2Unlink = (tTVPV2UnlinkProc)
			GetProcAddress(Instance, "V2Unlink");

		//GetModuleInstance = (tTVPGetModuleInstanceProc)
		//	GetProcAddress(Instance, "GetModuleInstance");
		//GetModuleThreadModel = (tTVPGetModuleThreadModelProc)
		//	GetProcAddress(Instance, "GetModuleThreadModel");
		//ShowConfigWindow = (tTVPShowConfigWindowProc)
		//	GetProcAddress(Instance, "ShowConfigWindow");
		CanUnloadNow = (tTVPCanUnloadNowProc)
			GetProcAddress(Instance, "CanUnloadNow");

		// link
		if(V2Link)
		{
			V2Link(TVPGetFunctionExporter());
		}

		/*
		if(GetModuleInstance)
		{
			HRESULT hr = GetModuleInstance(&TSSModule, storageprovider,
				 NULL, Application->GetHandle());
			if(FAILED(hr) || TSSModule == NULL)
				TVPThrowExceptionMessage(TVPCannotLoadPlugin, name);

			// get supported extensions
			unsigned long index = 0;
			while(true)
			{
				tjs_char mediashortname[33];
				tjs_char buf[256];
				HRESULT hr = TSSModule->GetSupportExts(index,
					mediashortname, buf, 255);
				if(hr == S_OK)
					SupportedExts.push_back(ttstr(buf).AsLowerCase());
				else
					break;
				index ++;
			}
		}
		*/
	}
	catch(...)
	{
		FreeLibrary(Instance);
		delete Holder;
		throw;
	}
#endif
}
//---------------------------------------------------------------------------
tTVPPlugin::~tTVPPlugin()
{
}
//---------------------------------------------------------------------------
bool tTVPPlugin::Uninit()
{
	tTJS *tjs = TVPGetScriptEngine();
	if(tjs) tjs->DoGarbageCollection(); // to release unused objects

	if(V2Unlink)
	{
 		if(FAILED(V2Unlink())) return false;
	}
#if 0
	//if(TSSModule) TSSModule->Release();

	FreeLibrary(Instance);
#endif
	delete Holder;
	return true;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
bool TVPPluginUnloadedAtSystemExit = false;
typedef std::vector<tTVPPlugin*> tTVPPluginVectorType;
struct tTVPPluginVectorStruc
{
	tTVPPluginVectorType Vector;
	tTVPStorageProvider StorageProvider;
} static TVPPluginVector;
static void TVPDestroyPluginVector(void)
{
	// state all plugins are to be released
	TVPPluginUnloadedAtSystemExit = true;

	// delete all objects
	tTVPPluginVectorType::iterator i;
	while(TVPPluginVector.Vector.size())
	{
		i = TVPPluginVector.Vector.end() - 1;
		try
		{
			(*i)->Uninit();
			delete *i;
		}
		catch(...)
		{
		}
		TVPPluginVector.Vector.pop_back();
	}
}
tTVPAtExit TVPDestroyPluginVectorAtExit
	(TVP_ATEXIT_PRI_RELEASE, TVPDestroyPluginVector);
//---------------------------------------------------------------------------
static bool TVPPluginLoading = false;
void TVPLoadPlugin(const ttstr & name)
{
	// load plugin
	if(TVPPluginLoading)
		TVPThrowExceptionMessage(TVPCannnotLinkPluginWhilePluginLinking);
			// linking plugin while other plugin is linking, is prohibited
			// by data security reason.

	// check whether the same plugin was already loaded
	tTVPPluginVectorType::iterator i;
	for(i = TVPPluginVector.Vector.begin();
		i != TVPPluginVector.Vector.end(); i++)
	{
		if((*i)->Name == name) return;
	}

	tTVPPlugin * p;

	try
	{
		TVPPluginLoading = true;
		p = new tTVPPlugin(name, &TVPPluginVector.StorageProvider);
		TVPPluginLoading = false;
	}
	catch(...)
	{
		TVPPluginLoading = false;
		throw;
	}

	TVPPluginVector.Vector.push_back(p);
}
//---------------------------------------------------------------------------
bool TVPUnloadPlugin(const ttstr & name)
{
	// unload plugin

	tTVPPluginVectorType::iterator i;
	for(i = TVPPluginVector.Vector.begin();
		i != TVPPluginVector.Vector.end(); i++)
	{
		if((*i)->Name == name)
		{
			if(!(*i)->Uninit()) return false;
			delete *i;
			TVPPluginVector.Vector.erase(i);
			return true;
		}
	}
	TVPThrowExceptionMessage(TVPNotLoadedPlugin, name);
	return false;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// plug-in autoload support
//---------------------------------------------------------------------------
static tjs_int TVPAutoLoadPluginCount = 0;
void TVPLoadPluigins(void)
{
	// This function searches plugins which have an extension of ".tpm"
	// in the default path: 
	//    1. a folder which holds kirikiri executable
	//    2. "plugin" folder of it
	// Plugin load order is to be decided using its name;
	// aaa.tpm is to be loaded before aab.tpm (sorted by ASCII order)

	// search plugins from path: (exepath), (exepath)\system, (exepath)\plugin
}
//---------------------------------------------------------------------------
tjs_int TVPGetAutoLoadPluginCount() { return TVPAutoLoadPluginCount; }
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// some service functions for plugin
//---------------------------------------------------------------------------
#include "zlib/zlib.h"
int ZLIB_uncompress(unsigned char *dest, unsigned long *destlen,
	const unsigned char *source, unsigned long sourcelen)
{
	return uncompress(dest, destlen, source, sourcelen);
}
//---------------------------------------------------------------------------
int ZLIB_compress(unsigned char *dest, unsigned long *destlen,
	const unsigned char *source, unsigned long sourcelen)
{
	return compress(dest, destlen, source, sourcelen);
}
//---------------------------------------------------------------------------
int ZLIB_compress2(unsigned char *dest, unsigned long *destlen,
	const unsigned char *source, unsigned long sourcelen, int level)
{
	return compress2(dest, destlen, source, sourcelen, level);
}
//---------------------------------------------------------------------------
#include "md5.h"
static char TVP_assert_md5_state_t_size[
	 (sizeof(TVP_md5_state_t) >= sizeof(md5_state_t))];
	// if this errors, sizeof(TVP_md5_state_t) is not equal to sizeof(md5_state_t).
	// sizeof(TVP_md5_state_t) must be equal to sizeof(md5_state_t).
//---------------------------------------------------------------------------
void TVP_md5_init(TVP_md5_state_t *pms)
{
	md5_init((md5_state_t*)pms);
}
//---------------------------------------------------------------------------
void TVP_md5_append(TVP_md5_state_t *pms, const tjs_uint8 *data, int nbytes)
{
	md5_append((md5_state_t*)pms, (const md5_byte_t*)data, nbytes);
}
//---------------------------------------------------------------------------
void TVP_md5_finish(TVP_md5_state_t *pms, tjs_uint8 *digest)
{
	md5_finish((md5_state_t*)pms, digest);
}
//---------------------------------------------------------------------------
HWND TVPGetApplicationWindowHandle()
{
	return Application->GetHandle();
}
//---------------------------------------------------------------------------
void TVPProcessApplicationMessages()
{
	Application->ProcessMessages();
}
//---------------------------------------------------------------------------
void TVPHandleApplicationMessage()
{
	Application->HandleMessage();
}
//---------------------------------------------------------------------------
bool TVPRegisterGlobalObject(const tjs_char *name, iTJSDispatch2 * dsp)
{
	// register given object to global object
	tTJSVariant val(dsp);
	iTJSDispatch2 *global = TVPGetScriptDispatch();
	tjs_error er;
	try
	{
		er = global->PropSet(TJS_MEMBERENSURE, name, NULL, &val, global);
	}
	catch(...)
	{
		global->Release();
		return false;
	}
	global->Release();
	return TJS_SUCCEEDED(er);
}
//---------------------------------------------------------------------------
bool TVPRemoveGlobalObject(const tjs_char *name)
{
	// remove registration of global object
	iTJSDispatch2 *global = TVPGetScriptDispatch();
	if(!global) return false;
	tjs_error er;
	try
	{
		er = global->DeleteMember(0, name, NULL, global);
	}
	catch(...)
	{
		global->Release();
		return false;
	}
	global->Release();
	return TJS_SUCCEEDED(er);
}
//---------------------------------------------------------------------------
void TVPDoTryBlock(
	tTVPTryBlockFunction tryblock,
	tTVPCatchBlockFunction catchblock,
	tTVPFinallyBlockFunction finallyblock,
	void *data)
{
	try
	{
		tryblock(data);
	}
	catch(const eTJS & e)
	{
		if(finallyblock) finallyblock(data);
		tTVPExceptionDesc desc;
		desc.type = TJS_W("eTJS");
		desc.message = e.GetMessage();
		if(catchblock(data, desc)) throw;
		return;
	}
	catch(...)
	{
		if(finallyblock) finallyblock(data);
		tTVPExceptionDesc desc;
		desc.type = TJS_W("unknown");
		if(catchblock(data, desc)) throw;
		return;
	}
	if(finallyblock) finallyblock(data);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPGetFileVersionOf
// 実行ファイル等のバージョン情報を取得する
// 本体のバージョン取得にも使われていて、ここで得たバージョンナンバーを表示する
//---------------------------------------------------------------------------
bool TVPGetFileVersionOf(const tjs_char* module_filename, tjs_int &major, tjs_int &minor, tjs_int &release, tjs_int &build)
{
	major = 1;
	minor = release = 0;
	build = 1;
	return true;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPCreateNativeClass_Plugins
//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_Plugins()
{
	tTJSNC_Plugins *cls = new tTJSNC_Plugins();


	// setup some platform-specific members
//---------------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/link)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr name = *param[0];

	TVPLoadPlugin(name);

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL_OUTER(/*object to register*/cls,
	/*func. name*/link)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/unlink)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr name = *param[0];

	bool res = TVPUnloadPlugin(name);

	if(result) *result = (tjs_int)res;

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL_OUTER(/*object to register*/cls,
	/*func. name*/unlink)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(getList)
{
	iTJSDispatch2 * array = TJSCreateArrayObject();
	try
	{
		tTVPPluginVectorType::iterator i;
		tjs_int idx = 0;
		for(i = TVPPluginVector.Vector.begin(); i != TVPPluginVector.Vector.end(); i++)
		{
			tTJSVariant val = (*i)->Name.c_str();
			array->PropSetByNum(TJS_MEMBERENSURE, idx++, &val, array);
		}
	
		if (result) *result = tTJSVariant(array, array);
	}
	catch(...)
	{
		array->Release();
		throw;
	}
	array->Release();
	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL_OUTER(cls, getList)
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
	return cls;
}
//---------------------------------------------------------------------------




