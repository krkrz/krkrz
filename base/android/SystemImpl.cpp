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

//#include "GraphicsLoaderImpl.h"

#include "SystemImpl.h"
#include "SystemIntf.h"
#include "SysInitIntf.h"
#include "StorageIntf.h"
//#include "StorageImpl.h"
#include "TickCount.h"
#include "ComplexRect.h"
//#include "WindowImpl.h"
#include "EventIntf.h"
#include "SystemControl.h"
//#include "DInputMgn.h"

#include "Application.h"
#include "TVPScreen.h"
//#include "CompatibleNativeFuncs.h"
#include "DebugIntf.h"
#include "CharacterSet.h"


#define ANDROID_BUILDING_DISABLE

//---------------------------------------------------------------------------
static ttstr TVPAppTitle;
static bool TVPAppTitleInit = false;
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPShowSimpleMessageBox
//---------------------------------------------------------------------------
static void TVPShowSimpleMessageBox(const ttstr & text, const ttstr & caption)
{
	// ignore caption
	Application->ShowToast( text.AsStdString().c_str() );
}
//---------------------------------------------------------------------------



#ifndef ANDROID_BUILDING_DISABLE
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

	return 0!=( GetAsyncKeyState(keycode) & ( getcurrent?0x8000:0x0001) );
}
//---------------------------------------------------------------------------
#endif // ANDROID_BUILDING_DISABLE






//---------------------------------------------------------------------------
// TVPGetPlatformName
//---------------------------------------------------------------------------
ttstr TVPGetPlatformName()
{
	static ttstr platform(TJS_W("Android"));
	return platform;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPGetOSName
//---------------------------------------------------------------------------
ttstr TVPGetOSName()
{
	const tjs_char *osname = TJS_W("Android");

	std::string lang = Application->getLanguage();
	tjs_string wlang;
	TVPUtf8ToUtf16( wlang, lang );
	std::string country = Application->getCountry();
	tjs_string wcountry;
	TVPUtf8ToUtf16( wcountry, country );
	tjs_int ver = Application->getSdkVersion();
	const tjs_char *sysVer = Application->getSystemVersion().c_str();

	tjs_char buf[256];
	TJS_snprintf(buf, sizeof(buf)/sizeof(tjs_char), TJS_W("%ls %ls(API Level %d) Country %ls : Language %ls"),
		osname, sysVer, ver, wcountry.c_str(), wlang.c_str() );

	return ttstr(buf);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// TVPGetOSBits
//---------------------------------------------------------------------------
tjs_int TVPGetOSBits()
{
	// TODO arm の定義で調べているが、OSから取得するAPIで得るのが好ましい。すぐには見つからなかったが方法を調べること
#if defined(__ANDROID__) 
#	if __ARM_ARCH >= 7
#		if __ARM_32BIT_STATE
			return 32;
#		else
			return 64;
#		endif
#	else
		return 32;
#	endif
#else
	return 32;
#endif
}
//---------------------------------------------------------------------------

#ifndef ANDROID_BUILDING_DISABLE
//---------------------------------------------------------------------------
// TVPShellExecute
//---------------------------------------------------------------------------
bool TVPShellExecute(const ttstr &target, const ttstr &param)
{
	// open or execute target file
	// Android 版は Intent の方がいいな。
	/*
	if(::ShellExecute(NULL, NULL,
		target.c_str(),
		param.IsEmpty() ? NULL : param.c_str(),
		TJS_W(""),
		SW_SHOWNORMAL)
		<=(void *)32)
	{
		return false;
	}
	else
	{
		return true;
	}
	*/
}
//---------------------------------------------------------------------------
#endif // ANDROID_BUILDING_DISABLE


//---------------------------------------------------------------------------
// TVPGetPersonalPath
//---------------------------------------------------------------------------
ttstr TVPGetPersonalPath()
{
	ttstr path( Application->GetExternalDataPath().c_str() );
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
	return ttstr( Application->GetInternalDataPath().c_str() );
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPCreateAppLock
//---------------------------------------------------------------------------
bool TVPCreateAppLock(const ttstr &lockname)
{
	// Androidの場合二重起動などはOSでチェックされているはず？
	return true;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
enum tTVPTouchDevice {
	tdNone				= 0,
	tdIntegratedTouch	= 0x00000001,
	tdExternalTouch		= 0x00000002,
	tdIntegratedPen		= 0x00000004,
	tdExternalPen		= 0x00000008,
	tdMultiInput		= 0x00000040,
	tdDigitizerReady	= 0x00000080,
	tdMouse				= 0x00000100,
	tdMouseWheel		= 0x00000200
};
/**
 * タッチデバイス(とマウス)の接続状態を取得する
 **/
static int TVPGetSupportTouchDevice()
{
	// 常に組み込みタッチパネルを返す
	return tdIntegratedTouch | tdMultiInput;
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
	if(!TVPSystemControlAlive) return;

	// check the state again (because the state may change during the event delivering).
	// but note that this implementation might fire activate events even in the application
	// is already activated (the same as deactivation).
	if(activate_or_deactivate != Application->GetActivating()) return;

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
	// always 0.
	if(result) *result = (tjs_int)0;
	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL_OUTER(/*object to register*/cls,
	/*func. name*/getKeyState)
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
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/nullpo)
{
	// force make a null-po
#ifdef __GNUC__
	__builtin_trap();
#else
	*(int *)0  = 0;
#endif

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
		static ttstr exename(TVPNormalizeStorageName(Application->GetPackageCodePath()));
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
			TVPAppTitle = Application->GetTitle();
		}
		*result = TVPAppTitle;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TVPAppTitle = *param;
		Application->SetTitle( TVPAppTitle.AsStdString() );
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
		*result = tTVPScreen::GetWidth();
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
		*result = tTVPScreen::GetHeight();
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
		*result = tTVPScreen::GetDesktopLeft();
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
		*result = tTVPScreen::GetDesktopTop();
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
		*result = tTVPScreen::GetDesktopWidth();
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
		*result = tTVPScreen::GetDesktopHeight();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, desktopHeight)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(touchDevice)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = TVPGetSupportTouchDevice();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, touchDevice)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(externalCacheDir)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = ttstr(*Application->GetExternalCachePath());
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, externalCacheDir)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(cacheDir)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = ttstr(*Application->GetCachePath());
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, cacheDir)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(filesDir)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = ttstr(Application->GetInternalDataPath());
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, filesDir)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(externalFilesDir)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = ttstr(Application->GetExternalDataPath());
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, externalFilesDir)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(obbDir)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(*Application->GetObbPath());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, obbDir)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(sharedObjectPath)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(Application->GetSoPath());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, sharedObjectPath)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(packageName)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(Application->GetPackageName());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, packageName)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(packageCodePath)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(Application->GetPackageCodePath());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, packageCodePath)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(packageResourcePath)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(Application->GetPackageResourcePath());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, packageResourcePath)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(dataDirectory)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(*Application->GetDataDirectory());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, dataDirectory)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(downloadCacheDirectory)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(*Application->GetDownloadCacheDirectory());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, downloadCacheDirectory)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(externalStorageDirectory)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(*Application->GetExternalStorageDirectory());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, externalStorageDirectory)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(externalPublicMusicPath)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(*Application->GetExternalPublicMusicPath());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, externalPublicMusicPath)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(externalPublicPodcastsPath)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(*Application->GetExternalPublicPodcastsPath());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, externalPublicPodcastsPath)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(externalPublicRingtonesPath)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(*Application->GetExternalPublicRingtonesPath());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, externalPublicRingtonesPath)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(externalPublicAlaramsPath)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(*Application->GetExternalPublicAlaramsPath());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, externalPublicAlaramsPath)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(externalPublicNotificationsPath)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(*Application->GetExternalPublicNotificationsPath());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, externalPublicNotificationsPath)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(externalPublicPicturesPath)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(*Application->GetExternalPublicPicturesPath());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, externalPublicPicturesPath)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(externalPublicMoviesPath)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(*Application->GetExternalPublicMoviesPath());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, externalPublicMoviesPath)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(externalPublicDownloadsPath)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(*Application->GetExternalPublicDownloadsPath());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, externalPublicDownloadsPath)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(externalPublicDCIMPath)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(*Application->GetExternalPublicDCIMPath());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, externalPublicDCIMPath)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(externalPublicDocumentsPath)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(*Application->GetExternalPublicDocumentsPath());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, externalPublicDocumentsPath)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(rootDirectory)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(*Application->GetRootDirectory());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, rootDirectory)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(externalStorageState)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = ttstr(Application->GetExternalStorageState());
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, externalStorageState)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(isExternalStorageEmulated)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = Application->IsExternalStorageEmulated() ? (tjs_int)1 : (tjs_int)0;
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, isExternalStorageEmulated)
//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_PROP_DECL(isExternalStorageRemovable)
	{
		TJS_BEGIN_NATIVE_PROP_GETTER
			{
				*result = Application->IsExternalStorageRemovable() ? (tjs_int)1 : (tjs_int)0;
				return TJS_S_OK;
			}
		TJS_END_NATIVE_PROP_GETTER

		TJS_DENY_NATIVE_PROP_SETTER
	}
	TJS_END_NATIVE_STATIC_PROP_DECL_OUTER(cls, isExternalStorageRemovable)
//----------------------------------------------------------------------



//----------------------------------------------------------------------


	return cls;

}
//---------------------------------------------------------------------------


