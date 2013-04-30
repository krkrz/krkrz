//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// "System" class implementation
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include <shellapi.h>
#include <shlobj.h>

#include "GraphicsLoaderImpl.h"

#include "SystemImpl.h"
#include "SystemIntf.h"
#include "SysInitIntf.h"
#include "StorageIntf.h"
#include "StorageImpl.h"
#include "TickCount.h"
#include "WideNativeFuncs.h"
#include "ComplexRect.h"
#include "WindowImpl.h"
#include "MainFormUnit.h"
#include "DInputMgn.h"



//---------------------------------------------------------------------------
static ttstr TVPAppTitle;
static bool TVPAppTitleInit = false;
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPShowSimpleMessageBox
//---------------------------------------------------------------------------
static void TVPShowSimpleMessageBox(const ttstr & text, const ttstr & caption)
{
	::MessageBox(TVPGetModalWindowOwnerHandle(),
		text.AsAnsiString().c_str(),
		caption.AsAnsiString().c_str(), MB_OK|MB_ICONINFORMATION);
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPInputQuery
//---------------------------------------------------------------------------
bool TVPInputQuery(const ttstr & caption, const ttstr &prompt,
	ttstr &value)
{
	AnsiString v = value.AsAnsiString();
	bool res = InputQuery(caption.AsAnsiString(), prompt.AsAnsiString(),
		v);
	if(res)
		value = v.c_str();
	return res;
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// TVPGetAsyncKeyState
//---------------------------------------------------------------------------
bool TVPGetAsyncKeyState(tjs_uint keycode, bool getcurrent)
{
	// get keyboard state asynchronously.
	// return current key state if getcurrent is true.
	// otherwise, return whether the key is pushed during previous call of
	// TVPGetAsyncKeyState at the same keycode.

	if(keycode >= VK_PAD_FIRST  && keycode <= VK_PAD_LAST)
	{
		// JoyPad related keys are treated in DInputMgn.cpp
		return TVPGetJoyPadAsyncState(keycode, getcurrent);
	}

	if(keycode == VK_LBUTTON || keycode == VK_RBUTTON)
	{
		// check whether the mouse button is swapped
		if(GetSystemMetrics(SM_SWAPBUTTON))
		{
			// mouse button had been swapped; swap key code
			if(keycode == VK_LBUTTON)
				keycode = VK_RBUTTON;
			else
				keycode = VK_LBUTTON;
		}
	}

	return
		(bool)(GetAsyncKeyState(keycode)
			 & ( getcurrent?0x8000:0x0001) ) ;
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// TVPGetPlatformName
//---------------------------------------------------------------------------
ttstr TVPGetPlatformName()
{
	static ttstr platform(TJS_W("Win32"));
	return platform;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPGetOSName
//---------------------------------------------------------------------------
ttstr TVPGetOSName()
{
	OSVERSIONINFO ovi;
	ovi.dwOSVersionInfoSize = sizeof(ovi);
	GetVersionEx(&ovi);
	tjs_char buf[256];
	const tjs_char *osname;

	switch(ovi.dwPlatformId)
	{
	case VER_PLATFORM_WIN32s:
		osname = TJS_W("Win32s"); break;
	case VER_PLATFORM_WIN32_WINDOWS:
		switch((ovi.dwBuildNumber&0xffff ))
		{
		case 1998:
			osname = TJS_W("Windows 98"); break;
		case 95:
			osname = TJS_W("Windows 95"); break;
		default:
			osname = TJS_W("Win9x"); break;
		}
		break;
	case VER_PLATFORM_WIN32_NT:
		osname = TJS_W("Windows NT"); break;
	default:
		osname = TJS_W("Unknown"); break;
	}

	TJS_sprintf(buf, TJS_W("%ls %d.%d.%d "), osname, ovi.dwMajorVersion,
		ovi.dwMinorVersion, ovi.dwBuildNumber&0xfff);

	ttstr str(buf);
	str += ttstr(ovi.szCSDVersion);

	return str;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPShellExecute
//---------------------------------------------------------------------------
static bool TVPShellExecute(const ttstr &target, const ttstr &param)
{
	// open or execute target file
//	ttstr file = TVPGetNativeName(TVPNormalizeStorageName(target));

	if(procShellExecuteW)
	{
		if(ShellExecuteW(NULL, NULL,
			target.c_str(),
			param.IsEmpty() ? NULL : param.c_str(),
			L"",
			SW_SHOWNORMAL)
			<=(void *)32)
		{
			return false;
		}
		else
		{
			return true;
		}

	}
	else
	{
		AnsiString a_target(target.AsAnsiString());
		AnsiString a_param(param.AsAnsiString());
		if(ShellExecute(NULL, NULL,
			target.AsAnsiString().c_str(),
			param.IsEmpty() ? NULL : a_param.c_str(),
			"",
			SW_SHOWNORMAL)
			<=(void *)32)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPReadRegValue
//---------------------------------------------------------------------------
static void TVPReadRegValue(tTJSVariant &result, const ttstr & key)
{
	// open specified registry key
	if(key.IsEmpty()) { result.Clear(); return; }

	// check whether the key contains root key name
	HKEY root = HKEY_CURRENT_USER;
	const tjs_char *key_p = key.c_str();

	if(key.StartsWith(TJS_W("HKEY_CLASSES_ROOT")))
	{
		key_p += 17;
		root = HKEY_CLASSES_ROOT;
	}
	else if(key.StartsWith(TJS_W("HKEY_CURRENT_CONFIG")))
	{
		key_p += 19;
		root = HKEY_CURRENT_CONFIG;
	}
	else if(key.StartsWith(TJS_W("HKEY_CURRENT_USER")))
	{
		key_p += 17;
		root = HKEY_CURRENT_USER;
	}
	else if(key.StartsWith(TJS_W("HKEY_LOCAL_MACHINE")))
	{
		key_p += 18;
		root = HKEY_LOCAL_MACHINE;
	}
	else if(key.StartsWith(TJS_W("HKEY_USERS")))
	{
		key_p += 10;
		root = HKEY_USERS;
	}
	else if(key.StartsWith(TJS_W("HKEY_PERFORMANCE_DATA")))
	{
		key_p += 21;
		root = HKEY_PERFORMANCE_DATA;
	}
	else if(key.StartsWith(TJS_W("HKEY_DYN_DATA")))
	{
		key_p += 13;
		root = HKEY_DYN_DATA;
	}

	if(*key_p == TJS_W('\\')) key_p ++;

	// search value name
	const tjs_char *start = key_p;
	key_p += TJS_strlen(key_p);
	key_p--;
	while(start <= key_p && *key_p != TJS_W('\\')) key_p--;
	ttstr valuename(key_p+1);
	if(key_p < start || *key_p != TJS_W('\\')) key_p++;

	ttstr keyname(start, key_p - start);

	// open key
	HKEY handle;
	LONG res;

	if(procRegOpenKeyExW)
		res = procRegOpenKeyExW(root, keyname.c_str(), 0, KEY_READ, &handle);
	else
		res = RegOpenKeyExA(root, keyname.AsAnsiString().c_str(), 0, KEY_READ, &handle);
	if(res != ERROR_SUCCESS) { result.Clear(); return; }

	// try query value size and read key
	DWORD size;
	DWORD type;


	AnsiString a_valuename;

	if(!procRegQueryValueExW)
		a_valuename = valuename.AsAnsiString();

	// query size
	if(procRegQueryValueExW)
		res = procRegQueryValueExW(handle, valuename.c_str(), 0,
			&type, NULL, &size);
	else
		res = RegQueryValueExA(handle, a_valuename.c_str(), 0,
			&type, NULL, &size);


	if(res != ERROR_SUCCESS)
	{
		RegCloseKey(handle);
		result.Clear();
		return;
	}


	switch(type)
	{
	case REG_DWORD:
//	case REG_DWORD_LITTLE_ENDIAN: // is actually the same as REG_DWORD
	case REG_DWORD_BIG_ENDIAN:
	case REG_EXPAND_SZ:
	case REG_SZ:
		break; // these should be OK

	case REG_MULTI_SZ: // sorry not yet implemented
	case REG_BINARY:
	case REG_LINK:
	case REG_NONE:
	case REG_RESOURCE_LIST:
	default:
		// not capable types
		RegCloseKey(handle);
		result.Clear();
		return;
	}

	while(true)
	{
		tjs_uint8 * data = new tjs_uint8[size];

		try
		{
			DWORD size2 = size;
			if(procRegQueryValueExW)
				res = procRegQueryValueExW(handle, valuename.c_str(), 0,
					NULL, data, &size2);
			else
				res = RegQueryValueExA(handle, a_valuename.c_str(), 0,
					NULL, data, &size2);

			if(res == ERROR_MORE_DATA)
			{
				// more data required
				delete [] data;
				size += 1024;
				continue;
			}
			else if(res != ERROR_SUCCESS)
			{
				RegCloseKey(handle);
				result.Clear();
				return;
			}

			// query succeeded


			// store data into result
			switch(type)
			{
			case REG_DWORD:
//			case REG_DWORD_LITTLE_ENDIAN:
				result = (tTVInteger)*(DWORD*)data;
				break;

			case REG_DWORD_BIG_ENDIAN:
				{
					DWORD val = *(DWORD*)data;
					val = (val >> 24) + ((val >> 8) & 0x0000ff00) +
						((val << 8) & 0x00ff0000) + (val << 24);
					result = (tTVInteger)val;
			  	}
				break;

			case REG_EXPAND_SZ:
			case REG_SZ:
				if(procRegQueryValueExW)
				{
					// data is stored in unicode
					result = ttstr((const tjs_char*)data);
				}
				else
				{
					// data is stored in ANSI
					result = ttstr((const char*)data);
				}
				break;
			}
		}
		catch(...)
		{
			RegCloseKey(handle);
			delete [] data;
			throw;
		}
		RegCloseKey(handle);
		delete [] data;

		break;
	}
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// Static function for retrieving special folder path
//---------------------------------------------------------------------------
static ttstr TVPGetSpecialFolderPath(int csidl)
{
	if(procSHGetSpecialFolderPathW)
	{
		WCHAR path[MAX_PATH+1];
		if(!procSHGetSpecialFolderPathW(NULL, path, csidl, false))
			return ttstr();
		return ttstr(path);
	}
	else
	{
		char path[MAX_PATH+1];
		if(!SHGetSpecialFolderPathA(NULL, path, csidl, false))
			return ttstr();
		return ttstr(path);
	}
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPGetPersonalPath
//---------------------------------------------------------------------------
ttstr TVPGetPersonalPath()
{
	// Retrieve personal directory;
	// This usually refers "My Documents".
	// If this is not exist, returns application data path, then exe path.
	// for windows vista, this refers application data path.
	ttstr path;
	path = TVPGetSpecialFolderPath(CSIDL_PERSONAL);
	if(path.IsEmpty())
		path = TVPGetSpecialFolderPath(CSIDL_APPDATA);
	
	if(!path.IsEmpty())
	{
		path = TVPNormalizeStorageName(path);
		if(path.GetLastChar() != TJS_W('/')) path += TJS_W('/');
		return path;
	}

	return TVPGetAppPath();
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPGetAppDataPath
//---------------------------------------------------------------------------
ttstr TVPGetAppDataPath()
{
	// Retrieve application data directory;
	// If this is not exist, returns application exe path.

	ttstr path = TVPGetSpecialFolderPath(CSIDL_APPDATA);
	
	if(!path.IsEmpty())
	{
		path = TVPNormalizeStorageName(path);
		if(path.GetLastChar() != TJS_W('/')) path += TJS_W('/');
		return path;
	}

	return TVPGetAppPath();
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPCreateAppLock
//---------------------------------------------------------------------------
bool TVPCreateAppLock(const ttstr &lockname)
{
	// lock application using mutex
	if(procCreateMutexW)
	{
		procCreateMutexW(NULL, TRUE, lockname.c_str());
	}
	else
	{
		AnsiString a_lockname(lockname.AsAnsiString());
		CreateMutexA(NULL, TRUE, a_lockname.c_str());
	}

	if(GetLastError())
	{
		return false; // already running
	}


	// No need to release the mutex object because the mutex is automatically
	// released when the calling thread exits.

	return true;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPGetDesktopRect
//---------------------------------------------------------------------------
static void TVPGetDesktopRect(tTVPRect &dest)
{
	RECT r;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
	dest.left = r.left;
	dest.top = r.top;
	dest.right = r.right;
	dest.bottom = r.bottom;
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// System.onActivate and System.onDeactivate related
//---------------------------------------------------------------------------
static void TVPOnApplicationActivate(bool activate_or_deactivate);
//---------------------------------------------------------------------------
class tTVPOnApplicationActivateEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	bool ActivateOrDeactivate; // true for activate; otherwise deactivate
public:
	tTVPOnApplicationActivateEvent(bool activate_or_deactivate) :
		tTVPBaseInputEvent(Application, Tag),
		ActivateOrDeactivate(activate_or_deactivate) {};
	void Deliver() const
	{ TVPOnApplicationActivate(ActivateOrDeactivate); }
};
tTVPUniqueTagForInputEvent tTVPOnApplicationActivateEvent              ::Tag;
//---------------------------------------------------------------------------
void TVPPostApplicationActivateEvent()
{
	TVPPostInputEvent(new tTVPOnApplicationActivateEvent(true), TVP_EPT_REMOVE_POST);
}
//---------------------------------------------------------------------------
void TVPPostApplicationDeactivateEvent()
{
	TVPPostInputEvent(new tTVPOnApplicationActivateEvent(false), TVP_EPT_REMOVE_POST);
}
//---------------------------------------------------------------------------
static void TVPOnApplicationActivate(bool activate_or_deactivate)
{
	// called by event system, to fire System.onActivate or
	// System.onDeactivate event
	if(!TVPMainFormAlive) return;

	// check the state again (because the state may change during the event delivering).
	// but note that this implementation might fire activate events even in the application
	// is already activated (the same as deactivation).
	if(activate_or_deactivate != TVPMainForm->GetApplicationActivating()) return;

	// fire the event
	TVPFireOnApplicationActivateEvent(activate_or_deactivate);
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPCreateNativeClass_System
//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_System()
{
	tTJSNC_System *cls = new tTJSNC_System();


	// setup some platform-specific members
//----------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/inform)
{
	// show simple message box
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr text = *param[0];

	ttstr caption;
	if(numparams >= 2 && param[1]->Type() != tvtVoid)
		caption = *param[1];
	else
		caption = TJS_W("Information");

	TVPShowSimpleMessageBox(text, caption);

	if(result) result->Clear();

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL_OUTER(/*object to register*/cls,
	/*func. name*/inform)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/getTickCount)
{
	if(result)
	{
		TVPStartTickCount();

		*result = (tjs_int64) TVPGetTickCount();
	}
	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL_OUTER(/*object to register*/cls,
	/*func. name*/getTickCount)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/getKeyState)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	tjs_uint code = (tjs_int)*param[0];

	bool getcurrent = true;
	if(numparams >= 2) getcurrent = *param[1];

	bool res = TVPGetAsyncKeyState(code, getcurrent);

	if(result) *result = (tjs_int)res;
	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL_OUTER(/*object to register*/cls,
	/*func. name*/getKeyState)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/shellExecute)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr target = *param[0];
	ttstr execparam;

	if(numparams >= 2) execparam = *param[1];

	bool res = TVPShellExecute(target, execparam);

	if(result) *result = (tjs_int)res;
	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL_OUTER(/*object to register*/cls,
	/*func. name*/shellExecute)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/system)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr target = *param[0];

	int ret = system(target.AsAnsiString().c_str());

	TVPDeliverCompactEvent(TVP_COMPACT_LEVEL_MAX); // this should clear all caches

	if(result) *result = (tjs_int)ret;
	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL_OUTER(/*object to register*/cls,
	/*func. name*/system)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/readRegValue)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;
	if(!result) return TJS_S_OK;

	ttstr key = *param[0];


	TVPReadRegValue(*result, key);

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL_OUTER(/*object to register*/cls,
	/*func. name*/readRegValue)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/getArgument)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;
	if(!result) return TJS_S_OK;

	ttstr name = *param[0];

	bool res = TVPGetCommandLine(name.c_str(), result);

	if(!res) result->Clear();

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL_OUTER(/*object to register*/cls,
	/*func. name*/getArgument)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setArgument)
{
	if(numparams < 2) return TJS_E_BADPARAMCOUNT;

	ttstr name = *param[0];
	ttstr value = *param[1];

	TVPSetCommandLine(name.c_str(), value);

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL_OUTER(/*object to register*/cls,
	/*func. name*/setArgument)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/createAppLock)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;
	if(!result) return TJS_S_OK;

	ttstr lockname = *param[0];

	bool res = TVPCreateAppLock(lockname);

	if(result) *result = (tjs_int)res;

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL_OUTER(/*object to register*/cls,
	/*func. name*/createAppLock)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/nullpo)
{
	// force make a null-po
	*(int *)0  = 0;

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL_OUTER(/*object to register*/cls,
	/*func. name*/nullpo)
//---------------------------------------------------------------------------

//----------------------------------------------------------------------

//-- properties

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(exePath)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = TVPGetAppPath();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, exePath)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(personalPath)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = TVPGetPersonalPath();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, personalPath)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(appDataPath)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = TVPGetAppDataPath();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, appDataPath)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(dataPath)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = TVPDataPath;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, dataPath)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(exeName)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		static ttstr exename(TVPNormalizeStorageName(ParamStr(0)));
		*result = exename;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, exeName)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(title)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		if(!TVPAppTitleInit)
		{
			TVPAppTitleInit = true;
			TVPAppTitle = Application->Title;
		}
		*result = TVPAppTitle;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TVPAppTitle = *param;
		Application->Title = TVPAppTitle.AsAnsiString();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, title)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(screenWidth)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = Screen->Width;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, screenWidth)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(screenHeight)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = Screen->Height;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, screenHeight)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(desktopLeft)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		tTVPRect r;
		TVPGetDesktopRect(r);
		*result = r.left;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, desktopLeft)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(desktopTop)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		tTVPRect r;
		TVPGetDesktopRect(r);
		*result = r.top;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, desktopTop)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(desktopWidth)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		tTVPRect r;
		TVPGetDesktopRect(r);
		*result = r.get_width();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, desktopWidth)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(desktopHeight)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		tTVPRect r;
		TVPGetDesktopRect(r);
		*result = r.get_height();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, desktopHeight)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(stayOnTop)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		if(TVPMainForm) *result = TVPMainForm->GetApplicationStayOnTop();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TVPMainForm->SetApplicationStayOnTop(*param);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, stayOnTop)
//----------------------------------------------------------------------


	return cls;

}
//---------------------------------------------------------------------------


