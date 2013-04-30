//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// "Window" TJS Class implementation
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#define DIRECTDRAW_VERSION 0x0300
#include <ddraw.h>

#include <algorithm>
#include "MsgIntf.h"
#include "WindowIntf.h"
#include "LayerIntf.h"
#include "WindowFormUnit.h"
#include "SysInitIntf.h"
#include "tjsHashSearch.h"
#include "StorageIntf.h"
#include "WideNativeFuncs.h"
#include "VideoOvlIntf.h"
#include "DebugIntf.h"
#include "PluginImpl.h"
#include "LayerManager.h"
#include "PassThroughDrawDevice.h"
#include "EventImpl.h"


//---------------------------------------------------------------------------
// Mouse Cursor management
//---------------------------------------------------------------------------
static tTJSHashTable<ttstr, tjs_int> TVPCursorTable;
static TVPCursorCount = 1;
tjs_int TVPGetCursor(const ttstr & name)
{
	// get placed path
	ttstr place(TVPSearchPlacedPath(name));

	// search in cache
	tjs_int * in_hash = TVPCursorTable.Find(place);
	if(in_hash) return *in_hash;

	// not found
	tTVPLocalTempStorageHolder file(place);

	HCURSOR handle;

	if(procLoadCursorFromFileW)
	{
		handle = procLoadCursorFromFileW(file.GetLocalName().c_str());
	}
	else
	{
		tTJSNarrowStringHolder holder(file.GetLocalName().c_str());
		handle = LoadCursorFromFileA(holder);
	}

	if(!handle) TVPThrowExceptionMessage(TVPCannotLoadCursor, place);

	TVPCursorCount++;
	Screen->Cursors[TVPCursorCount] = handle; // using VCL

	TVPCursorTable.Add(place, TVPCursorCount);

	return TVPCursorCount;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Color Format Detection
//---------------------------------------------------------------------------
tjs_int TVPDisplayColorFormat = 0;
static tjs_int TVPGetDisplayColorFormat()
{
	// detect current 16bpp display color format
	// return value:
	// 555 : 16bit 555 mode
	// 565 : 16bit 565 mode
	// 0   : other modes

	// create temporary bitmap and device contexts
	HDC desktopdc = GetDC(0);
	HDC bitmapdc = CreateCompatibleDC(desktopdc);
	HBITMAP bmp = CreateCompatibleBitmap(desktopdc, 1, 1);
	HBITMAP oldbmp = SelectObject(bitmapdc, bmp);

	int count;
	int r, g, b;
	COLORREF lastcolor;

	// red
	count = 0;
	lastcolor = 0xffffff;
	for(int i = 0; i < 256; i++)
	{
		SetPixel(bitmapdc, 0, 0, RGB(i, 0, 0));
		COLORREF rgb = GetPixel(bitmapdc, 0, 0);
		if(rgb != lastcolor) count ++;
		lastcolor = rgb;
	}
	r = count;

	// green
	count = 0;
	lastcolor = 0xffffff;
	for(int i = 0; i < 256; i++)
	{
		SetPixel(bitmapdc, 0, 0, RGB(0, i, 0));
		COLORREF rgb = GetPixel(bitmapdc, 0, 0);
		if(rgb != lastcolor) count ++;
		lastcolor = rgb;
	}
	g = count;

	// blue
	count = 0;
	lastcolor = 0xffffff;
	for(int i = 0; i < 256; i++)
	{
		SetPixel(bitmapdc, 0, 0, RGB(0, 0, i));
		COLORREF rgb = GetPixel(bitmapdc, 0, 0);
		if(rgb != lastcolor) count ++;
		lastcolor = rgb;
	}
	b = count;

	// free bitmap and device contexts
	SelectObject(bitmapdc, oldbmp);
	DeleteObject(bmp);
	DeleteDC(bitmapdc);
	ReleaseDC(0, desktopdc);

	// determine type
	if(r == 32 && g == 64 && b == 32)
	{
		TVPDisplayColorFormat = 565;
		return 565;
	}
	else if(r == 32 && g == 32 && b == 32)
	{
		TVPDisplayColorFormat = 555;
		return 555;
	}
	else
	{
		TVPDisplayColorFormat = 0;
		return 0;
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// DirectDraw/Full Screen and priamary surface management
//---------------------------------------------------------------------------

//! @brief		Display resolution mode for full screen
enum tTVPFullScreenResolutionMode
{
	fsrAuto, //!< auto negotiation
	fsrProportional, //!< let screen resolution fitting neaest to the preferred resolution,
								//!< preserving the original aspect ratio
	fsrNearest, //!< let screen resolution fitting neaest to the preferred resolution.
				//!< There is no guarantee that the aspect ratio is preserved
	fsrNoChange //!< no change resolution
};
static IDirectDraw *TVPDirectDraw=NULL;
static IDirectDraw2 *TVPDirectDraw2=NULL;
static IDirectDraw7 *TVPDirectDraw7=NULL;
static HRESULT WINAPI (*TVPDirectDrawCreate)
	( GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter ) = NULL;
static HRESULT WINAPI (*TVPDirectDrawCreateEx)
	( GUID FAR * lpGuid, LPVOID  *lplpDD, REFIID  iid,IUnknown FAR *pUnkOuter ) = NULL;

static HRESULT WINAPI (*TVPDirectDrawEnumerateA)
	( LPDDENUMCALLBACKA lpCallback, LPVOID lpContext ) = NULL;
static HRESULT WINAPI (*TVPDirectDrawEnumerateExA)
	( LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags) = NULL;

static HMODULE TVPDirectDrawDLLHandle=NULL;
static bool TVPUseChangeDisplaySettings = false;
static tTVPScreenMode TVPDefaultScreenMode;

static bool TVPInFullScreen = false;
static HWND TVPFullScreenWindow = NULL;
tTVPScreenModeCandidate TVPFullScreenMode;
static IDirectDrawSurface * TVPDDPrimarySurface = NULL;
bool TVPDDPrimarySurfaceFailed = false;

static tjs_int TVPPreferredFullScreenBPP = 0;
static tTVPFullScreenResolutionMode TVPPreferredFullScreenResolutionMode = fsrAuto;
enum tTVPFullScreenUsingEngineZoomMode
{
	fszmNone, //!< no zoom by the engine
	fszmInner, //!< inner fit on the monitor (uncovered areas may be filled with black)
	fszmOuter //!< outer fit on the monitor (primary layer may jut out of the monitor)
};
static tTVPFullScreenUsingEngineZoomMode TVPPreferredFullScreenUsingEngineZoomMode = fszmInner;


//---------------------------------------------------------------------------
static void TVPInitFullScreenOptions()
{
	tTJSVariant val;

	if(TVPGetCommandLine(TJS_W("-fsbpp"), &val) )
	{
		ttstr str(val);
		if(str == TJS_W("16"))
			TVPPreferredFullScreenBPP = 16;
		else if(str == TJS_W("24"))
			TVPPreferredFullScreenBPP = 24;
		else if(str == TJS_W("32"))
			TVPPreferredFullScreenBPP = 32;
		else
			TVPPreferredFullScreenBPP = 0; // means nochange
	}

	if(TVPGetCommandLine(TJS_W("-fsres"), &val) )
	{
		ttstr str(val);
		if(str == TJS_W("auto"))
			TVPPreferredFullScreenResolutionMode = fsrAuto;
		else if(str == TJS_W("prop") || str == TJS_W("proportional"))
			TVPPreferredFullScreenResolutionMode = fsrProportional;
		else if(str == TJS_W("nearest"))
			TVPPreferredFullScreenResolutionMode = fsrNearest;
		else if(str == TJS_W("nochange"))
			TVPPreferredFullScreenResolutionMode = fsrNoChange;
	}

	if(TVPGetCommandLine(TJS_W("-fszoom"), &val) )
	{
		ttstr str(val);
		if(str == TJS_W("yes") || str == TJS_W("inner"))
			TVPPreferredFullScreenUsingEngineZoomMode = fszmInner;
		if(str == TJS_W("outer"))
			TVPPreferredFullScreenUsingEngineZoomMode = fszmOuter;
		else if(str == TJS_W("no"))
			TVPPreferredFullScreenUsingEngineZoomMode = fszmNone;
	}

	if(TVPGetCommandLine(TJS_W("-fsmethod"), &val) )
	{
		ttstr str(val);
		if(str == TJS_W("cds"))
			TVPUseChangeDisplaySettings = true;
		else
			TVPUseChangeDisplaySettings = false;
	}
}
//---------------------------------------------------------------------------
static BOOL WINAPI DDEnumCallbackEx( GUID *pGUID, LPSTR pDescription, LPSTR strName,
							  LPVOID pContext, HMONITOR hm )
{
	ttstr log(TJS_W("(info) DirectDraw Driver/Device found : "));
	if(pDescription)
		log += ttstr(pDescription);
	if(strName)
		log += TJS_W(" [") + ttstr(strName) + TJS_W("]");
	char tmp[60];
	sprintf(tmp, "0x%p", hm);
	log += TJS_W(" (monitor: ") + ttstr(tmp) + TJS_W(")");
	TVPAddImportantLog(log);

	return  DDENUMRET_OK;
}
//---------------------------------------------------------------------------
static BOOL WINAPI DDEnumCallback( GUID *pGUID, LPSTR pDescription,
							LPSTR strName, LPVOID pContext )
{
	return ( DDEnumCallbackEx( pGUID, pDescription, strName, pContext, NULL ) );
}
//---------------------------------------------------------------------------
void TVPDumpDirectDrawDriverInformation()
{
	if(TVPDirectDraw7)
	{
		IDirectDraw7 *dd7 = TVPDirectDraw7;
		static bool dumped = false;
		if(dumped) return;
		dumped = true;

		TVPAddImportantLog(TJS_W("(info) DirectDraw7 or higher detected. Retrieving current DirectDraw driver information..."));

		try
		{
			// dump directdraw information
			DDDEVICEIDENTIFIER2 DDID = {0};
			if(SUCCEEDED(dd7->GetDeviceIdentifier(&DDID, 0)))
			{
				ttstr infostart(TJS_W("(info)  "));
				ttstr log;

				// driver string
				log = infostart + ttstr(DDID.szDescription) + TJS_W(" [") + ttstr(DDID.szDriver) + TJS_W("]");
				TVPAddImportantLog(log);

				// driver version(reported)
				log = infostart + TJS_W("Driver version (reported) : ");
				char tmp[256];
				wsprintf( tmp, "%d.%02d.%02d.%04d ",
						  HIWORD( DDID.liDriverVersion.u.HighPart ),
						  LOWORD( DDID.liDriverVersion.u.HighPart ),
						  HIWORD( DDID.liDriverVersion.u.LowPart  ),
						  LOWORD( DDID.liDriverVersion.u.LowPart  ) );
				log += tmp;
				TVPAddImportantLog(log);

				// driver version(actual)
				char driverpath[1024];
				char *driverpath_filename = NULL;
				bool success = SearchPath(NULL, DDID.szDriver, NULL, 1023, driverpath, &driverpath_filename);

				if(!success)
				{
					char syspath[1024];
					GetSystemDirectory(syspath, 1023);
					strcat(syspath, "\\drivers"); // SystemDir\drivers
					success = SearchPath(syspath, DDID.szDriver, NULL, 1023, driverpath, &driverpath_filename);
				}

				if(!success)
				{
					char syspath[1024];
					GetWindowsDirectory(syspath, 1023);
					strcat(syspath, "\\system32"); // WinDir\system32
					success = SearchPath(syspath, DDID.szDriver, NULL, 1023, driverpath, &driverpath_filename);
				}

				if(!success)
				{
					char syspath[1024];
					GetWindowsDirectory(syspath, 1023);
					strcat(syspath, "\\system32\\drivers"); // WinDir\system32\drivers
					success = SearchPath(syspath, DDID.szDriver, NULL, 1023, driverpath, &driverpath_filename);
				}

				if(success)
				{
					log = infostart + TJS_W("Driver version (") + ttstr(driverpath) + TJS_W(") : ");
					tjs_int major, minor, release, build;
					if(TVPGetFileVersionOf(driverpath, major, minor, release, build))
					{
						wsprintf(tmp, "%d.%d.%d.%d", (int)major, (int)minor, (int)release, (int)build);
						log += tmp;
					}
					else
					{
						log += TJS_W("unknown");
					}
				}
				else
				{
					log = infostart + TJS_W("Driver ") + ttstr(DDID.szDriver) +
						TJS_W(" is not found in search path.");
				}
				TVPAddImportantLog(log);

				// device id
				wsprintf(tmp, "VendorId:%08X  DeviceId:%08X  SubSysId:%08X  Revision:%08X",
					DDID.dwVendorId, DDID.dwDeviceId, DDID.dwSubSysId, DDID.dwRevision);
				log = infostart + TJS_W("Device ids : ") + tmp;
				TVPAddImportantLog(log);

				// Device GUID
				GUID *pguid = &DDID.guidDeviceIdentifier;
				wsprintf( tmp, "%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X",
						  pguid->Data1,
						  pguid->Data2,
						  pguid->Data3,
						  pguid->Data4[0], pguid->Data4[1], pguid->Data4[2], pguid->Data4[3],
						  pguid->Data4[4], pguid->Data4[5], pguid->Data4[6], pguid->Data4[7] );
				log = infostart + TJS_W("Unique driver/device id : ") + tmp;
				TVPAddImportantLog(log);

				// WHQL level
				wsprintf(tmp, "%08x", DDID.dwWHQLLevel);
				log = infostart + TJS_W("WHQL level : ")  + tmp;
				TVPAddImportantLog(log);
			}
			else
			{
				TVPAddImportantLog(TJS_W("(info) Failed."));
			}
		}
		catch(...)
		{
		}
	}

}
//---------------------------------------------------------------------------
static void TVPUnloadDirectDraw();
static void TVPInitDirectDraw()
{
	if(!TVPDirectDrawDLLHandle)
	{
		// load ddraw.dll
		TVPAddLog(TJS_W("(info) Loading DirectDraw ..."));
		TVPDirectDrawDLLHandle = LoadLibrary("ddraw.dll");
		if(!TVPDirectDrawDLLHandle)
			TVPThrowExceptionMessage(TVPCannotInitDirectDraw,
				TJS_W("Cannot load ddraw.dll"));

		// Enumerate display drivers, for debugging information
		try
		{
			TVPDirectDrawEnumerateExA = (HRESULT WINAPI (*)
				( LPDDENUMCALLBACKEXA , LPVOID , DWORD )	)
					GetProcAddress(TVPDirectDrawDLLHandle, "DirectDrawEnumerateExA");
			if(TVPDirectDrawEnumerateExA)
			{
				TVPDirectDrawEnumerateExA( DDEnumCallbackEx, NULL,
										  DDENUM_ATTACHEDSECONDARYDEVICES |
										  DDENUM_DETACHEDSECONDARYDEVICES |
										  DDENUM_NONDISPLAYDEVICES );
			}
			else
			{
				TVPDirectDrawEnumerateA = (HRESULT WINAPI (*)
					( LPDDENUMCALLBACKA , LPVOID  ))
					GetProcAddress(TVPDirectDrawDLLHandle, "DirectDrawEnumerateA");
				if(TVPDirectDrawEnumerateA)
				{
			        TVPDirectDrawEnumerateA( DDEnumCallback, NULL );
				}
			}
		}
		catch(...)
		{
			// Ignore errors
		}
	}

	if(!TVPDirectDraw2)
	{
		try
		{
			// get DirectDrawCreaet function
			TVPDirectDrawCreate = (HRESULT(WINAPI*)(_GUID*,IDirectDraw**,IUnknown*))
				GetProcAddress(TVPDirectDrawDLLHandle, "DirectDrawCreate");
			if(!TVPDirectDrawCreate)
				TVPThrowExceptionMessage(TVPCannotInitDirectDraw,
					TJS_W("Missing DirectDrawCreate in ddraw.dll"));

			TVPDirectDrawCreateEx = (HRESULT(WINAPI*)( GUID FAR *, LPVOID  *, REFIID,IUnknown FAR *))
				GetProcAddress(TVPDirectDrawDLLHandle, "DirectDrawCreateEx");

			// create IDirectDraw object
			if(TVPDirectDrawCreateEx)
			{
				HRESULT hr;
				hr = TVPDirectDrawCreateEx(NULL, (void**)&TVPDirectDraw7, IID_IDirectDraw7, NULL);
 				if(FAILED(hr))
					TVPThrowExceptionMessage(TVPCannotInitDirectDraw,
						ttstr(TJS_W("DirectDrawCreateEx failed./HR="))+
							TJSInt32ToHex((tjs_uint32)hr));

				// retrieve IDirecDraw2 interface
				hr = TVPDirectDraw7->QueryInterface(IID_IDirectDraw2,
					(void **)&TVPDirectDraw2);
				if(FAILED(hr))
					TVPThrowExceptionMessage(TVPCannotInitDirectDraw,
						ttstr(TJS_W("Querying of IID_IDirectDraw2 failed."
							"/HR="))+
							TJSInt32ToHex((tjs_uint32)hr));
			}
			else
			{
				HRESULT hr;

				hr = TVPDirectDrawCreate(NULL, &TVPDirectDraw, NULL);
				if(FAILED(hr))
					TVPThrowExceptionMessage(TVPCannotInitDirectDraw,
						ttstr(TJS_W("DirectDrawCreate failed./HR="))+
							TJSInt32ToHex((tjs_uint32)hr));

				// retrieve IDirecDraw2 interface
				hr = TVPDirectDraw->QueryInterface(IID_IDirectDraw2,
					(void **)&TVPDirectDraw2);
				if(FAILED(hr))
					TVPThrowExceptionMessage(TVPCannotInitDirectDraw,
						ttstr(TJS_W("Querying of IID_IDirectDraw2 failed."
							" (DirectX on this system may be too old)/HR="))+
							TJSInt32ToHex((tjs_uint32)hr));

				TVPDirectDraw->Release(), TVPDirectDraw = NULL;

				// retrieve IDirectDraw7 interface
				hr = TVPDirectDraw2->QueryInterface(IID_IDirectDraw7, (void**)&TVPDirectDraw7);
				if(FAILED(hr)) TVPDirectDraw7 = NULL;
			}


			if(TVPLoggingToFile)
			{
				TVPDumpDirectDrawDriverInformation();
			}

			// set cooperative level
			if(TVPDirectDraw7)
				TVPDirectDraw7->SetCooperativeLevel(NULL, DDSCL_NORMAL);
			else
				TVPDirectDraw2->SetCooperativeLevel(NULL, DDSCL_NORMAL);
		}
		catch(...)
		{
			TVPUnloadDirectDraw();
			throw;
		}
	}

	TVPGetDisplayColorFormat();
}
//---------------------------------------------------------------------------
static void TVPUninitDirectDraw()
{
	// release DirectDraw object ( DLL will not be released )
}
//---------------------------------------------------------------------------
static void TVPUnloadDirectDraw()
{
	// release DirectDraw object and /*release it's DLL */
	TVPUninitDirectDraw();
	if(TVPDDPrimarySurface) TVPDDPrimarySurface->Release(), TVPDDPrimarySurface = NULL;
	if(TVPDirectDraw7) TVPDirectDraw7->Release(), TVPDirectDraw7 = NULL;
	if(TVPDirectDraw2) TVPDirectDraw2->Release(), TVPDirectDraw2 = NULL;
	if(TVPDirectDraw) TVPDirectDraw -> Release(), TVPDirectDraw = NULL;
//	if(TVPDirectDrawDLLHandle)
//		FreeLibrary(TVPDirectDrawDLLHandle), TVPDirectDrawDLLHandle = NULL;

	TVPGetDisplayColorFormat();
}
//---------------------------------------------------------------------------
void TVPEnsureDirectDrawObject()
{
	try
	{
		TVPInitDirectDraw();
	}
	catch(...)
	{
	}
}
//---------------------------------------------------------------------------
IDirectDraw2 * TVPGetDirectDrawObjectNoAddRef()
{
	// retrieves DirectDraw2 interface
	return TVPDirectDraw2;
}
//---------------------------------------------------------------------------
IDirectDraw7 * TVPGetDirectDraw7ObjectNoAddRef()
{
	// retrieves DirectDraw7 interface
	return TVPDirectDraw7;
}
//---------------------------------------------------------------------------
IDirectDrawSurface * TVPGetDDPrimarySurfaceNoAddRef()
{
	if(TVPDDPrimarySurfaceFailed) return NULL;
	if(TVPDDPrimarySurface) return TVPDDPrimarySurface;

	TVPEnsureDirectDrawObject();

	if(!TVPDirectDraw2)
	{
		// DirectDraw not available
		TVPDDPrimarySurfaceFailed = true;
		return NULL;
	}

	DDSURFACEDESC ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	HRESULT hr;
	hr = TVPDirectDraw2->CreateSurface(&ddsd, &TVPDDPrimarySurface, NULL);
	if(hr != DD_OK)
	{
		// failed to create DirectDraw primary surface
		TVPDDPrimarySurface = NULL;
		TVPDDPrimarySurfaceFailed = true;
		return NULL;
	}

	return TVPDDPrimarySurface;
}
//---------------------------------------------------------------------------
void TVPSetDDPrimaryClipper(IDirectDrawClipper *clipper)
{
	// set clipper object

	IDirectDrawSurface * pri = TVPGetDDPrimarySurfaceNoAddRef();

	// set current clipper object
	if(pri) pri->SetClipper(clipper);
}
//---------------------------------------------------------------------------
void TVPReleaseDDPrimarySurface()
{
	if(TVPDDPrimarySurface) TVPDDPrimarySurface->Release(), TVPDDPrimarySurface = NULL;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
static tTVPAtExit
	TVPUnloadDirectDrawAtExit(TVP_ATEXIT_PRI_RELEASE, TVPUnloadDirectDraw);
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
//! @brief		Get tTVPFullScreenResolutionMode enumeration string
static ttstr TVPGetGetFullScreenResolutionModeString(tTVPFullScreenResolutionMode mode)
{
	switch(mode)
	{
	case fsrAuto:				return TJS_W("fsrAuto");
	case fsrProportional:		return TJS_W("fsrProportional");
	case fsrNearest:			return TJS_W("fsrNearest");
	case fsrNoChange:			return TJS_W("fsrNoChange");
	}
	return ttstr();
}
//---------------------------------------------------------------------------
//! @brief	do reduction for numer over denom
static void TVPDoReductionNumerAndDenom(tjs_int &n, tjs_int &d)
{
	tjs_int a = n;
	tjs_int b = d;
	while(b)
	{
		tjs_int t = b;
		b = a % b;
		a = t;
	}
	n = n / a;
	d = d / a;
}
//---------------------------------------------------------------------------
static void TVPGetOriginalScreenMetrics()
{
	// retrieve original (un-fullscreened) information
	TVPDefaultScreenMode.Width = Screen->Width;
	TVPDefaultScreenMode.Height = Screen->Height;
	HDC dc = GetDC(0);
	TVPDefaultScreenMode.BitsPerPixel = GetDeviceCaps(dc, BITSPIXEL);
	ReleaseDC(0, dc);
}
//---------------------------------------------------------------------------
//! @brief	callback function of TVPEnumerateAllDisplayModes
static HRESULT WINAPI TVPEnumerateAllDisplayModesCallback(
	LPDDSURFACEDESC lpDDSufaceDesc, LPVOID lpContext)
{
	std::vector<tTVPScreenMode> *modes = reinterpret_cast<std::vector<tTVPScreenMode> *>(lpContext);

	tTVPScreenMode mode;
	mode.Width  = lpDDSufaceDesc->dwWidth;
	mode.Height = lpDDSufaceDesc->dwHeight;
	mode.BitsPerPixel = lpDDSufaceDesc->ddpfPixelFormat.dwRGBBitCount;
	modes->push_back(mode);

	return DDENUMRET_OK;
}
//---------------------------------------------------------------------------
//! @brief	enumerate all display modes
void TVPEnumerateAllDisplayModes(std::vector<tTVPScreenMode> & modes)
{
	modes.clear();

	if(!TVPUseChangeDisplaySettings)
	{
		// if DisplaySettings APIs is not preferred
		// use DirectDraw
		TVPEnsureDirectDrawObject();
		IDirectDraw2 * dd2 = TVPGetDirectDrawObjectNoAddRef();
		if(dd2)
		{
			dd2->EnumDisplayModes(0, NULL, (void*)&modes,
					TVPEnumerateAllDisplayModesCallback);
		}
	}

	if(modes.size() == 0)
	{
		// try another API to retrieve screen sizes
		TVPUseChangeDisplaySettings = true;

		// attempt to use EnumDisplaySettings
		DWORD num = 0;
		do
		{
			tTVP_devicemodeA dm;
			ZeroMemory(&dm, sizeof(tTVP_devicemodeA));
			dm.dmSize = sizeof(tTVP_devicemodeA);
			dm.dmDriverExtra = 0;
			if(EnumDisplaySettings(NULL, num, reinterpret_cast<DEVMODE*>(&dm)) == 0) break;
			tTVPScreenMode mode;
			mode.Width  = dm.dmPelsWidth;
			mode.Height = dm.dmPelsHeight;
			mode.BitsPerPixel = dm.dmBitsPerPel;
			modes.push_back(mode);
			num ++;
		} while(true);
	}

	TVPAddLog(ttstr(TJS_W("(info) environment: using ")) +
		(TVPUseChangeDisplaySettings?TJS_W("ChangeDisplaySettings API"):TJS_W("DirectDraw")));
}
//---------------------------------------------------------------------------
//! @brief		make full screen mode candidates
//! @note		Always call this function *before* entering full screen mode
void TVPMakeFullScreenModeCandidates(
	const tTVPScreenMode & preferred,
	tTVPFullScreenResolutionMode mode,
	tTVPFullScreenUsingEngineZoomMode zoom_mode,
	std::vector<tTVPScreenModeCandidate> & candidates)
{
	// adjust give parameter
	if(mode == fsrAuto && zoom_mode == fszmNone) zoom_mode = fszmInner;
		// fszmInner is ignored (as always be fszmInner) if mode == fsrAuto && zoom_mode == fszmNone

	// print debug information
	TVPAddLog(TJS_W("(info) Searching best fullscreen resolution ..."));
	TVPAddLog(TJS_W("(info) condition: preferred screen mode: ") + preferred.Dump());
	TVPAddLog(TJS_W("(info) condition: mode: " ) + TVPGetGetFullScreenResolutionModeString(mode));
	TVPAddLog(TJS_W("(info) condition: zoom mode: ") + ttstr(
		zoom_mode == fszmInner ? TJS_W("inner") :
		zoom_mode == fszmOuter ? TJS_W("outer") :
			TJS_W("none")));

	// get original screen metrics
	TVPGetOriginalScreenMetrics();

	// decide preferred bpp
	tjs_int preferred_bpp = preferred.BitsPerPixel == 0 ?
			TVPDefaultScreenMode.BitsPerPixel : preferred.BitsPerPixel;

	// get original screen aspect ratio
	tjs_int screen_aspect_numer = TVPDefaultScreenMode.Width;
	tjs_int screen_aspect_denom = TVPDefaultScreenMode.Height;
	TVPDoReductionNumerAndDenom(screen_aspect_numer, screen_aspect_denom); // do reduction
	TVPAddLog(TJS_W("(info) environment: default screen mode: ") + TVPDefaultScreenMode.Dump());
	TVPAddLog(TJS_W("(info) environment: default screen aspect ratio: ") +
		ttstr(screen_aspect_numer) + TJS_W(":") + ttstr(screen_aspect_denom));

	// clear destination array
	candidates.clear();

	// enumerate all display modes
	std::vector<tTVPScreenMode> modes;
	TVPEnumerateAllDisplayModes(modes);
	std::sort(modes.begin(), modes.end()); // sort by area, and bpp

	{
		tjs_int last_width = -1, last_height = -1;
		ttstr last_line;
		TVPAddLog(TJS_W("(info) environment: available display modes:"));
		for(std::vector<tTVPScreenMode>::iterator i = modes.begin(); i != modes.end(); i++)
		{
			if(last_width != i->Width || last_height != i->Height)
			{
				if(!last_line.IsEmpty()) TVPAddLog(last_line);
				tjs_int w = i->Width, h = i->Height;
				TVPDoReductionNumerAndDenom(w, h);
				last_line = TJS_W("(info)  ") + i->DumpHeightAndWidth() +
					TJS_W(", AspectRatio=") + ttstr(w) + TJS_W(":") + ttstr(h) +
					TJS_W(", BitsPerPixel=") + ttstr(i->BitsPerPixel);
			}
			else
			{
				last_line += TJS_W("/") + ttstr(i->BitsPerPixel);
			}
			last_width = i->Width; last_height = i->Height;
		}
		if(!last_line.IsEmpty()) TVPAddLog(last_line);
	}

	if(mode != fsrNoChange)
	{

		if(mode != fsrNearest)
		{
			// for fstAuto and fsrProportional, we need to see screen aspect ratio

			// reject screen mode which does not match the original screen aspect ratio
			for(std::vector<tTVPScreenMode>::iterator i = modes.begin();
				i != modes.end(); /**/)
			{
				tjs_int aspect_numer = i->Width;
				tjs_int aspect_denom = i->Height;
				TVPDoReductionNumerAndDenom(aspect_numer, aspect_denom);
				if(aspect_numer != screen_aspect_numer || aspect_denom != screen_aspect_denom)
					i = modes.erase(i);
				else
					i++;
			}
		}

		if(zoom_mode == fszmNone)
		{
			// we cannot use resolution less than preferred resotution when
			// we do not use zooming, so reject them.
			for(std::vector<tTVPScreenMode>::iterator i = modes.begin();
				i != modes.end(); /**/)
			{
				if(i->Width < preferred.Width || i->Height < preferred.Height)
					i = modes.erase(i);
				else
					i++;
			}
		}
	}
	else
	{
		// reject resolutions other than the original size
		for(std::vector<tTVPScreenMode>::iterator i = modes.begin();
			i != modes.end(); /**/)
		{
			if(	i->Width  != TVPDefaultScreenMode.Width ||
				i->Height != TVPDefaultScreenMode.Height)
				i = modes.erase(i);
			else
				i++;
		}
	}

	// reject resolutions larger than the default screen mode
	for(std::vector<tTVPScreenMode>::iterator i = modes.begin();
		i != modes.end(); /**/)
	{
		if(i->Width > TVPDefaultScreenMode.Width || i->Height > TVPDefaultScreenMode.Height)
			i = modes.erase(i);
		else
			i++;
	}

	// reject resolutions less than 16
	for(std::vector<tTVPScreenMode>::iterator i = modes.begin();
		i != modes.end(); /**/)
	{
		if(i->BitsPerPixel < 16)
			i = modes.erase(i);
		else
			i++;
	}

	// check there is at least one candidate mode
	if(modes.size() == 0)
	{
		// panic! no candidates
		// this could be if the driver does not provide the screen
		// mode which is the same size as the default screen...
		// push the default screen mode
		TVPAddImportantLog(TJS_W("(info) Panic! There is no reasonable candidate screen mode provided from the driver ... trying to use the default screen size and color depth ..."));
		tTVPScreenMode mode;
		mode.Width  = TVPDefaultScreenMode.Width;
		mode.Height = TVPDefaultScreenMode.Height;
		mode.BitsPerPixel = TVPDefaultScreenMode.BitsPerPixel;
		modes.push_back(mode);
	}

	// copy modes to candidation, with making zoom ratio and resolution rank
	for(std::vector<tTVPScreenMode>::iterator i = modes.begin();
		i != modes.end(); i++)
	{
		tTVPScreenModeCandidate candidate;
		candidate.Width = i->Width;
		candidate.Height = i->Height;
		candidate.BitsPerPixel = i->BitsPerPixel;
		if(zoom_mode != fszmNone)
		{
			double width_r  = (double)candidate.Width /  (double)preferred.Width;
			double height_r = (double)candidate.Height / (double)preferred.Height;

			// select width or height, to fit to target screen from preferred size
			if(zoom_mode == fszmInner ? (width_r < height_r) : (width_r > height_r))
			{
				candidate.ZoomNumer = candidate.Width;
				candidate.ZoomDenom = preferred.Width;
			}
			else
			{
				candidate.ZoomNumer = candidate.Height;
				candidate.ZoomDenom = preferred.Height;
			}

			// if the zooming range is between 1.00 and 1.034 we treat this as 1.00
			double zoom_r = (double)candidate.ZoomNumer / (double)candidate.ZoomDenom;
			if(zoom_r > 1.000 && zoom_r < 1.034)
				candidate.ZoomDenom = candidate.ZoomNumer = 1;
		}
		else
		{
			// zooming disabled
			candidate.ZoomDenom = candidate.ZoomNumer = 1;
		}
		TVPDoReductionNumerAndDenom(candidate.ZoomNumer, candidate.ZoomDenom);

		// make rank on each candidate

		// BPP
		// take absolute difference of preferred and candidate.
		// lesser bpp has less priority, so add 1000 to lesser bpp.
		candidate.RankBPP = std::abs(preferred_bpp - candidate.BitsPerPixel);
		if(candidate.BitsPerPixel < preferred_bpp) candidate.RankBPP += 1000;

		// Zoom-in
		// we usually use zoom-in, zooming out (this situation will occur if
		// the screen resolution is lesser than expected) has lesser priority.
		if(candidate.ZoomNumer < candidate.ZoomDenom)
			candidate.RankZoomIn = 1;
		else
			candidate.RankZoomIn = 0;

		// Zoom-Beauty
		if(mode == fsrAuto)
		{
			// 0: no zooming is the best.
			// 1: zooming using monitor's function is fastest and most preferable.
			// 2: zooming using kirikiri's zooming functions is somewhat slower but not so bad.
			// 3: zooming using monitor's function and kirikiri's function tends to be dirty
			//   because the zooming is applied twice. this is not preferable.
			tjs_int zoom_rank = 0;
			if(candidate.Width != TVPDefaultScreenMode.Width ||
				candidate.Height != TVPDefaultScreenMode.Height)
				zoom_rank += 1; // zoom by monitor

			if(candidate.ZoomNumer != 1 || candidate.ZoomDenom != 1)
				zoom_rank += 2; // zoom by the engine

			candidate.RankZoomBeauty = zoom_rank;
		}
		else
		{
			// Zoom-Beauty is not considered
			candidate.RankZoomBeauty = 0;
		}

		// Size
		// size rank is a absolute difference between area size of candidate and preferred.
		candidate.RankSize = std::abs(
			candidate.Width * candidate.Height - preferred.Width * preferred.Height);

		// push candidate into candidates array
		candidates.push_back(candidate);
	}

	// sort candidate by its rank
	std::sort(candidates.begin(), candidates.end());

	// dump all candidates to log
	TVPAddLog(TJS_W("(info) result: candidates:"));
	for(std::vector<tTVPScreenModeCandidate>::iterator i = candidates.begin();
		i != candidates.end(); i++)
	{
		TVPAddLog(TJS_W("(info)  ") + i->Dump());
	}
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void TVPSwitchToFullScreen(HWND window, tjs_int w, tjs_int h)
{
	if(TVPInFullScreen) return;

	TVPInitFullScreenOptions();

	TVPReleaseVSyncTimingThread();

	TVPReleaseDDPrimarySurface();

	if(!TVPUseChangeDisplaySettings)
	{
		try
		{
			TVPInitDirectDraw();
		}
		catch(eTJS &e)
		{
			TVPAddLog(e.GetMessage());
			TVPUseChangeDisplaySettings = true;
		}
		catch(...)
		{
			TVPUseChangeDisplaySettings = true;
		}
	}


	// get fullscreen mode candidates
	std::vector<tTVPScreenModeCandidate> candidates;
	tTVPScreenMode preferred;
	preferred.Width = w;
	preferred.Height = h;
	preferred.BitsPerPixel = TVPPreferredFullScreenBPP;
	TVPMakeFullScreenModeCandidates(
		preferred,
		TVPPreferredFullScreenResolutionMode,
		TVPPreferredFullScreenUsingEngineZoomMode,
		candidates);

	// try changing display mode
	bool success = false;
	for(std::vector<tTVPScreenModeCandidate>::iterator i = candidates.begin();
		i != candidates.end(); i++)
	{
		TVPAddLog(TJS_W("(info) Trying screen mode: ") + i->Dump());
		if(TVPUseChangeDisplaySettings)
		{
			DEVMODE dm;
			ZeroMemory(&dm, sizeof(DEVMODE));
			dm.dmSize = sizeof(DEVMODE);
			dm.dmPelsWidth = i->Width;
			dm.dmPelsHeight = i->Height;
			dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
			dm.dmBitsPerPel = i->BitsPerPixel;
			LONG ret = ChangeDisplaySettings((DEVMODE*)&dm, CDS_FULLSCREEN);
			switch(ret)
			{
			case DISP_CHANGE_SUCCESSFUL:
				SetWindowPos(window, HWND_TOP, 0, 0, i->Width, i->Height, SWP_SHOWWINDOW);
				success = true;
				break;
			case DISP_CHANGE_RESTART:
				TVPAddLog(TJS_W("ChangeDisplaySettings failed: DISP_CHANGE_RESTART"));
				break;
			case DISP_CHANGE_BADFLAGS:
				TVPAddLog(TJS_W("ChangeDisplaySettings failed: DISP_CHANGE_BADFLAGS"));
				break;
			case DISP_CHANGE_BADPARAM:
				TVPAddLog(TJS_W("ChangeDisplaySettings failed: DISP_CHANGE_BADPARAM"));
				break;
			case DISP_CHANGE_FAILED:
				TVPAddLog(TJS_W("ChangeDisplaySettings failed: DISP_CHANGE_FAILED"));
				break;
			case DISP_CHANGE_BADMODE:
				TVPAddLog(TJS_W("ChangeDisplaySettings failed: DISP_CHANGE_BADMODE"));
				break;
			case DISP_CHANGE_NOTUPDATED:
				TVPAddLog(TJS_W("ChangeDisplaySettings failed: DISP_CHANGE_NOTUPDATED"));
				break;
			default:
				TVPAddLog(TJS_W("ChangeDisplaySettings failed: unknown reason (") +
								ttstr((tjs_int)ret) + TJS_W(")"));
				break;
			}
			if(success)
			{
				TVPFullScreenMode = *i;
				break;
			}
		}
		else
		{
			HRESULT hr;
			hr = TVPDirectDraw2->SetCooperativeLevel(window,
				DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN|DDSCL_ALLOWREBOOT);

			if(FAILED(hr))
			{
				TVPAddLog(TJS_W("IDirectDraw2::SetCooperativeLevel failed/hr=") +
								TJSInt32ToHex(hr) );
			}
			else
			{
				hr =TVPDirectDraw2->SetDisplayMode(i->Width, i->Height, i->BitsPerPixel, 0, 0);
				if(FAILED(hr))
				{
					TVPAddLog(
						ttstr(TJS_W("IDirectDraw2::SetDisplayMode failed/hr=")) + TJSInt32ToHex(hr));
				}
				else
				{
					success = true;
					TVPFullScreenMode = *i;
					break;
				}
			}
		}
	}

	if(!success)
	{
		TVPThrowExceptionMessage(TVPCannotSwitchToFullScreen,
			TJS_W("All screen mode has been tested, but no modes available at all."));
	}

	TVPAddLog(TJS_W("(info) Changing screen mode succeeded"));

	TVPInFullScreen = true;

	TVPGetDisplayColorFormat();
	TVPEnsureVSyncTimingThread();
}
//---------------------------------------------------------------------------
void TVPRevertFromFullScreen(HWND window)
{
	if(!TVPInFullScreen) return;

	TVPReleaseVSyncTimingThread();
	TVPReleaseDDPrimarySurface();

	if(TVPUseChangeDisplaySettings)
	{
		ChangeDisplaySettings(NULL, 0);
	}
	else
	{
		if(TVPDirectDraw2)
		{
			TVPDirectDraw2->RestoreDisplayMode();
			TVPDirectDraw2->SetCooperativeLevel(window, DDSCL_NORMAL);
			ChangeDisplaySettings(NULL, 0);
		}
	}
	TVPUninitDirectDraw();

	TVPInFullScreen = false;

	TVPGetDisplayColorFormat();
	TVPEnsureVSyncTimingThread();
}
//---------------------------------------------------------------------------





































//---------------------------------------------------------------------------
void TVPMinimizeFullScreenWindowAtInactivation()
{
	// only works when TVPUseChangeDisplaySettings == true
	// (DirectDraw framework does this)

	if(!TVPInFullScreen) return;
	if(!TVPUseChangeDisplaySettings) return;

	ChangeDisplaySettings(NULL, 0);

	ShowWindow(TVPFullScreenWindow, SW_MINIMIZE);
}
//---------------------------------------------------------------------------
void TVPRestoreFullScreenWindowAtActivation()
{
	// only works when TVPUseChangeDisplaySettings == true
	// (DirectDraw framework does this)

	if(!TVPInFullScreen) return;
	if(!TVPUseChangeDisplaySettings) return;

	DEVMODE dm;
	ZeroMemory(&dm, sizeof(DEVMODE));
	dm.dmSize = sizeof(DEVMODE);
	dm.dmPelsWidth = TVPFullScreenMode.Width;
	dm.dmPelsHeight = TVPFullScreenMode.Height;
	dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
	dm.dmBitsPerPel = TVPFullScreenMode.BitsPerPixel;
	ChangeDisplaySettings((DEVMODE*)&dm, CDS_FULLSCREEN);

	ShowWindow(TVPFullScreenWindow, SW_RESTORE);
	SetWindowPos(TVPFullScreenWindow, HWND_TOP,
		0, 0, TVPFullScreenMode.Width, TVPFullScreenMode.Height, SWP_SHOWWINDOW);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
static void TVPRestoreDisplayMode()
{
	// only works when TVPUseChangeDisplaySettings == true
	if(!TVPUseChangeDisplaySettings) return;
	if(!TVPInFullScreen) return;
	ChangeDisplaySettings(NULL, 0);
}
//---------------------------------------------------------------------------
static tTVPAtExit
	TVPRestoreDisplayModeAtExit(TVP_ATEXIT_PRI_CLEANUP, TVPUnloadDirectDraw);
//---------------------------------------------------------------------------
















//---------------------------------------------------------------------------
// TVPGetModalWindowOwner
//---------------------------------------------------------------------------
HWND TVPGetModalWindowOwnerHandle()
{
	if(TVPFullScreenedWindow)
		return TVPFullScreenedWindow->Handle;
	else
		return Application->Handle;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// tTJSNI_Window
//---------------------------------------------------------------------------
tTJSNI_Window::tTJSNI_Window()
{
	TVPEnsureVSyncTimingThread();
	Form = NULL;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD
tTJSNI_Window::Construct(tjs_int numparams, tTJSVariant **param,
		iTJSDispatch2 *tjs_obj)
{
	tjs_error hr = tTJSNI_BaseWindow::Construct(numparams, param, tjs_obj);
	if(TJS_FAILED(hr)) return hr;
	Form = new TTVPWindowForm(Application, this);
	return TJS_S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::Invalidate()
{
	tTJSNI_BaseWindow::Invalidate();
	if(Form)
	{
		Form->InvalidateClose();
		Form = NULL;
	}

	// remove all events
	TVPCancelSourceEvents(Owner);
	TVPCancelInputEvents(this);

	// Set Owner null
	Owner = NULL;
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::CanDeliverEvents() const
{
	if(!Form) return false;
	return GetVisible() && Form->GetFormEnabled();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::NotifyWindowClose()
{
	Form = NULL;
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SendCloseMessage()
{
	if(Form) Form->SendCloseMessage();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::TickBeat()
{
	if(Form) Form->TickBeat();
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetWindowActive()
{
	if(Form) return Form->GetWindowActive();
	return false;
}
//---------------------------------------------------------------------------
void tTJSNI_Window::ResetDrawDevice()
{
	if(Form) Form->ResetDrawDevice();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::PostInputEvent(const ttstr &name, iTJSDispatch2 * params)
{
	// posts input event
	if(!Form) return;

	static ttstr key_name(TJS_W("key"));
	static ttstr shift_name(TJS_W("shift"));

	// check input event name
	enum tEventType
	{
		etUnknown, etOnKeyDown, etOnKeyUp, etOnKeyPress
	} type;

	if(name == TJS_W("onKeyDown"))
		type = etOnKeyDown;
	else if(name == TJS_W("onKeyUp"))
		type = etOnKeyUp;
	else if(name == TJS_W("onKeyPress"))
		type = etOnKeyPress;
	else
		type = etUnknown;

	if(type == etUnknown)
		TVPThrowExceptionMessage(TVPSpecifiedEventNameIsUnknown, name);


	if(type == etOnKeyDown || type == etOnKeyUp)
	{
		// this needs params, "key" and "shift"
		if(params == NULL)
			TVPThrowExceptionMessage(
				TVPSpecifiedEventNeedsParameter, name);


		tjs_uint key;
		tjs_uint32 shift = 0;

		tTJSVariant val;
		if(TJS_SUCCEEDED(params->PropGet(0, key_name.c_str(), key_name.GetHint(),
			&val, params)))
			key = (tjs_int)val;
		else
			TVPThrowExceptionMessage(TVPSpecifiedEventNeedsParameter2,
				name, TJS_W("key"));

		if(TJS_SUCCEEDED(params->PropGet(0, shift_name.c_str(), shift_name.GetHint(),
			&val, params)))
			shift = (tjs_int)val;
		else
			TVPThrowExceptionMessage(TVPSpecifiedEventNeedsParameter2,
				name, TJS_W("shift"));

		Word vcl_key = key;
		if(type == etOnKeyDown)
			Form->InternalKeyDown(key, shift);
		else if(type == etOnKeyUp)
			Form->OnKeyUp(Form, vcl_key, TVP_TShiftState_From_uint32(shift));
	}
	else if(type == etOnKeyPress)
	{
		// this needs param, "key"
		if(params == NULL)
			TVPThrowExceptionMessage(
				TVPSpecifiedEventNeedsParameter, name);


		tjs_uint key;

		tTJSVariant val;
		if(TJS_SUCCEEDED(params->PropGet(0, key_name.c_str(), key_name.GetHint(),
			&val, params)))
			key = (tjs_int)val;
		else
			TVPThrowExceptionMessage(TVPSpecifiedEventNeedsParameter2,
				name, TJS_W("key"));

		char vcl_key = key;
		Form->OnKeyPress(Form, vcl_key);
	}
}

//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::NotifySrcResize()
{
	tTJSNI_BaseWindow::NotifySrcResize();

	// is called from primary layer
	// ( or from TWindowForm to reset paint box's size )
	tjs_int w, h;
	DrawDevice->GetSrcSize(w, h);
	if(Form)
		Form->SetPaintBoxSize(w, h);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::SetDefaultMouseCursor()
{
	// set window mouse cursor to default
	if(Form) Form->SetDefaultMouseCursor();
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::SetMouseCursor(tjs_int handle)
{
	// set window mouse cursor
	if(Form) Form->SetMouseCursor(handle);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::GetCursorPos(tjs_int &x, tjs_int &y)
{
	// get cursor pos in primary layer's coordinates
	if(Form) Form->GetCursorPos(x, y);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::SetCursorPos(tjs_int x, tjs_int y)
{
	// set cursor pos in primar layer's coordinates
	if(Form) Form->SetCursorPos(x, y);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::WindowReleaseCapture()
{
	::ReleaseCapture(); // Windows API
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::SetHintText(const ttstr & text)
{
	// set hint text to window
	if(Form) Form->SetHintText(text);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::SetAttentionPoint(tTJSNI_BaseLayer *layer,
	tjs_int l, tjs_int t)
{
	// set attention point to window
	if(Form)
	{
		TFont * font = NULL;
		if(layer)
		{
			tTVPBaseBitmap *bmp = layer->GetMainImage();
			if(bmp)
				font = bmp->GetFontCanvas()->Font;
		}

		Form->SetAttentionPoint(l, t, font);
	}
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::DisableAttentionPoint()
{
	// disable attention point
	if(Form) Form->DisableAttentionPoint();
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::SetImeMode(tTVPImeMode mode)
{
	// set ime mode
	if(Form) Form->SetImeMode(mode);
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetDefaultImeMode(tTVPImeMode mode)
{
	// set default ime mode
	if(Form)
	{
//		Form->SetDefaultImeMode(mode, LayerManager->GetFocusedLayer() == NULL);
	}
}
//---------------------------------------------------------------------------
tTVPImeMode tTJSNI_Window::GetDefaultImeMode() const
{
	if(Form) return Form->GetDefaultImeMode();
	return ::imDisable;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::ResetImeMode()
{
	// set default ime mode ( default mode is imDisable; IME is disabled )
	if(Form) Form->ResetImeMode();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::BeginUpdate(const tTVPComplexRect &rects)
{
	tTJSNI_BaseWindow::BeginUpdate(rects);
}
//---------------------------------------------------------------------------
void tTJSNI_Window::EndUpdate()
{
	tTJSNI_BaseWindow::EndUpdate();
}
//---------------------------------------------------------------------------
TMenuItem * tTJSNI_Window::GetRootMenuItem()
{
	if(!Form) return NULL;
	return Form->MainMenu->Items;
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetMenuBarVisible(bool b)
{
	if(!Form) return;
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidPropertyInFullScreen);

	Form->SetMenuBarVisible(b);
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetMenuBarVisible() const
{
	if(!Form) return false;
	return Form->GetMenuBarVisible();
}
//---------------------------------------------------------------------------
HWND tTJSNI_Window::GetMenuOwnerWindowHandle()
{
	if(!Form) return NULL;
	return Form->GetMenuOwnerWindowHandle();
}
//---------------------------------------------------------------------------
HWND tTJSNI_Window::GetSurfaceWindowHandle()
{
	if(!Form) return NULL;
	return Form->GetSurfaceWindowHandle();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::ZoomRectangle(
	tjs_int & left, tjs_int & top,
	tjs_int & right, tjs_int & bottom)
{
	if(!Form) return;
	Form->ZoomRectangle(left, top, right, bottom);
}
//---------------------------------------------------------------------------
HWND tTJSNI_Window::GetWindowHandle(tjs_int &ofsx, tjs_int &ofsy)
{
	if(!Form) return NULL;
	return Form->GetWindowHandle(ofsx, ofsy);
}
//---------------------------------------------------------------------------
void tTJSNI_Window::ReadjustVideoRect()
{
	if(!Form) return;

	// re-adjust video rectangle.
	// this reconnects owner window and video offsets.

	tObjectListSafeLockHolder<tTJSNI_BaseVideoOverlay> holder(VideoOverlay);
	tjs_int count = VideoOverlay.GetSafeLockedObjectCount();

	for(tjs_int i = 0; i < count; i++)
	{
		tTJSNI_VideoOverlay * item = (tTJSNI_VideoOverlay*)
			VideoOverlay.GetSafeLockedObjectAt(i);
		if(!item) continue;
		item->ResetOverlayParams();
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Window::WindowMoved()
{
	// inform video overlays that the window has moved.
	// video overlays typically owns DirectDraw surface which is not a part of
	// normal window systems and does not matter where the owner window is.
	// so we must inform window moving to overlay window.

	tObjectListSafeLockHolder<tTJSNI_BaseVideoOverlay> holder(VideoOverlay);
	tjs_int count = VideoOverlay.GetSafeLockedObjectCount();
	for(tjs_int i = 0; i < count; i++)
	{
		tTJSNI_VideoOverlay * item = (tTJSNI_VideoOverlay*)
			VideoOverlay.GetSafeLockedObjectAt(i);
		if(!item) continue;
		item->SetRectangleToVideoOverlay();
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Window::DetachVideoOverlay()
{
	// detach video overlay window
	// this is done before the window is being fullscreened or un-fullscreened.
	tObjectListSafeLockHolder<tTJSNI_BaseVideoOverlay> holder(VideoOverlay);
	tjs_int count = VideoOverlay.GetSafeLockedObjectCount();
	for(tjs_int i = 0; i < count; i++)
	{
		tTJSNI_VideoOverlay * item = (tTJSNI_VideoOverlay*)
			VideoOverlay.GetSafeLockedObjectAt(i);
		if(!item) continue;
		item->DetachVideoOverlay();
	}
}
//---------------------------------------------------------------------------
HWND tTJSNI_Window::GetWindowHandleForPlugin()
{
	if(!Form) return NULL;
	return Form->GetWindowHandleForPlugin();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::RegisterWindowMessageReceiver(tTVPWMRRegMode mode,
		void * proc, const void *userdata)
{
	if(!Form) return;
	Form->RegisterWindowMessageReceiver(mode, proc, userdata);
}
//---------------------------------------------------------------------------
void tTJSNI_Window::Close()
{
	if(Form) Form->Close();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::OnCloseQueryCalled(bool b)
{
	if(Form) Form->OnCloseQueryCalled(b);
}
//---------------------------------------------------------------------------
void tTJSNI_Window::BeginMove()
{
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidMethodInFullScreen);
	if(Form) Form->BeginMove();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::BringToFront()
{
	if(Form) Form->BringToFront();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::Update(tTVPUpdateType type)
{
	if(Form) Form->UpdateWindow(type);
}
//---------------------------------------------------------------------------
void tTJSNI_Window::ShowModal()
{
	if(Form && Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidMethodInFullScreen);
	if(Form)
	{
		TVPClearAllWindowInputEvents();
			// cancel all input events that can cause delayed operation
		Form->ShowWindowAsModal();
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Window::HideMouseCursor()
{
	if(Form) Form->HideMouseCursor();
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetVisible() const
{
	if(!Form) return false;
	return Form->GetVisible();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetVisible(bool s)
{
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidPropertyInFullScreen);
	if(Form) Form->SetVisible(s);
}
//---------------------------------------------------------------------------
void tTJSNI_Window::GetCaption(ttstr & v) const
{
	if(Form) v = Form->Caption; else v.Clear();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetCaption(const ttstr & v)
{
	if(Form) Form->Caption = v.AsAnsiString();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetWidth(tjs_int w)
{
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidPropertyInFullScreen);
	if(Form) Form->Width = w;
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetWidth() const
{
	if(!Form) return 0;
	return Form->Width;
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetHeight(tjs_int h)
{
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidPropertyInFullScreen);
	if(Form) Form->Height = h;
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetHeight() const
{
	if(!Form) return 0;
	return Form->Height;
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetLeft(tjs_int l)
{
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidPropertyInFullScreen);
	if(Form) Form->Left = l;
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetLeft() const
{
	if(!Form) return 0;
	return Form->Left;
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetTop(tjs_int t)
{
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidPropertyInFullScreen);
	if(Form) Form->Top = t;
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetTop() const
{
	if(!Form) return 0;
	return Form->Top;
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetSize(tjs_int w, tjs_int h)
{
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidPropertyInFullScreen);
	if(Form) Form->SetBounds(Form->Left, Form->Top, w, h);
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetMinWidth(int v)
{
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidPropertyInFullScreen);
	if(Form) Form->Constraints->MinWidth = v;
}
//---------------------------------------------------------------------------
int  tTJSNI_Window::GetMinWidth() const
{
	if(Form) return Form->Constraints->MinWidth; else return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetMinHeight(int v)
{
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidPropertyInFullScreen);
	if(Form) Form->Constraints->MinHeight = v;
}
//---------------------------------------------------------------------------
int  tTJSNI_Window::GetMinHeight() const
{
	if(Form) return Form->Constraints->MinHeight; else return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetMinSize(int w, int h)
{
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidPropertyInFullScreen);
	if(Form)
	{
		Form->Constraints->MinWidth = w;
		Form->Constraints->MinHeight = h;
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetMaxWidth(int v)
{
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidPropertyInFullScreen);
	if(Form) Form->Constraints->MaxWidth = v;
}
//---------------------------------------------------------------------------
int  tTJSNI_Window::GetMaxWidth() const
{
	if(Form) return Form->Constraints->MaxWidth; else return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetMaxHeight(int v)
{
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidPropertyInFullScreen);
	if(Form) Form->Constraints->MaxHeight = v;
}
//---------------------------------------------------------------------------
int  tTJSNI_Window::GetMaxHeight() const
{
	if(Form) return Form->Constraints->MaxHeight; else return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetMaxSize(int w, int h)
{
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidPropertyInFullScreen);
	if(Form)
	{
		Form->Constraints->MaxWidth = w;
		Form->Constraints->MaxHeight = h;
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetPosition(tjs_int l, tjs_int t)
{
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidPropertyInFullScreen);
	if(Form) Form->SetBounds(l, t, Form->Width, Form->Height);
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetLayerLeft(tjs_int l)
{
	if(Form) Form->SetLayerLeft(l);
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetLayerLeft() const
{
	if(!Form) return 0;
	return Form->GetLayerLeft();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetLayerTop(tjs_int t)
{
	if(Form) Form->SetLayerTop(t);
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetLayerTop() const
{
	if(!Form) return 0;
	return Form->GetLayerTop();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetLayerPosition(tjs_int l, tjs_int t)
{
	if(Form) Form->SetLayerPosition(l, t);
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetInnerSunken(bool b)
{
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidPropertyInFullScreen);
	if(Form) Form->SetInnerSunken(b);
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetInnerSunken() const
{
	if(!Form) return true;
	return Form->GetInnerSunken();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetInnerWidth(tjs_int w)
{
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidPropertyInFullScreen);
	if(Form) Form->SetInnerWidth(w);
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetInnerWidth() const
{
	if(!Form) return 0;
	return Form->GetInnerWidth();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetInnerHeight(tjs_int h)
{
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidPropertyInFullScreen);
	if(Form) Form->SetInnerHeight(h);
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetInnerHeight() const
{
	if(!Form) return 0;
	return Form->GetInnerHeight();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetInnerSize(tjs_int w, tjs_int h)
{
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidPropertyInFullScreen);
	if(Form) Form->SetInnerSize(w, h);
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetBorderStyle(tTVPBorderStyle st)
{
	if(Form->GetFullScreenMode())
		TVPThrowExceptionMessage(TVPInvalidPropertyInFullScreen);
	if(Form) Form->SetBorderStyle(st);
}
//---------------------------------------------------------------------------
tTVPBorderStyle tTJSNI_Window::GetBorderStyle() const
{
	if(!Form) return (tTVPBorderStyle)0;
	return Form->GetBorderStyle();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetStayOnTop(bool b)
{
	if(!Form) return;
	Form->SetStayOnTop(b);
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetStayOnTop() const
{
	if(!Form) return false;
	return Form->GetStayOnTop();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetShowScrollBars(bool b)
{
	if(Form) Form->SetShowScrollBars(b);
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetShowScrollBars() const
{
	if(!Form) return true;
	return Form->GetShowScrollBars();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetFullScreen(bool b)
{
	if(!Form) return;
	Form->SetFullScreenMode(b);
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetFullScreen() const
{
	if(!Form) return false;
	return Form->GetFullScreenMode();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetUseMouseKey(bool b)
{
	if(!Form) return;
	Form->SetUseMouseKey(b);
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetUseMouseKey() const
{
	if(!Form) return false;
	return Form->GetUseMouseKey();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetTrapKey(bool b)
{
	if(!Form) return;
	Form->SetTrapKey(b);
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetTrapKey() const
{
	if(!Form) return false;
	return Form->GetTrapKey();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetMaskRegion(tjs_int threshold)
{
	if(!Form) return;

	if(!DrawDevice) TVPThrowExceptionMessage(TVPWindowHasNoLayer);
	tTJSNI_BaseLayer *lay = DrawDevice->GetPrimaryLayer();
	if(!lay) TVPThrowExceptionMessage(TVPWindowHasNoLayer);
	Form->SetMaskRegion(((tTJSNI_Layer*)lay)->CreateMaskRgn((tjs_uint)threshold));

}
//---------------------------------------------------------------------------
void tTJSNI_Window::RemoveMaskRegion()
{
	if(!Form) return;
	Form->RemoveMaskRegion();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetMouseCursorState(tTVPMouseCursorState mcs)
{
	if(!Form) return;
	Form->SetMouseCursorState(mcs);
}
//---------------------------------------------------------------------------
tTVPMouseCursorState tTJSNI_Window::GetMouseCursorState() const
{
	if(!Form) return mcsVisible;
	return Form->GetMouseCursorState();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetFocusable(bool b)
{
	if(!Form) return;
	Form->SetFocusable(b);
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetFocusable()
{
	if(!Form) return true;
	return Form->GetFocusable();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetZoom(tjs_int numer, tjs_int denom)
{
	if(!Form) return;
	Form->SetZoom(numer, denom);
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetZoomNumer(tjs_int n)
{
	if(!Form) return;
	Form->SetZoomNumer(n);
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetZoomNumer() const
{
	if(!Form) return 1;
	return Form->GetZoomNumer();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetZoomDenom(tjs_int n)
{
	if(!Form) return;
	Form->SetZoomDenom(n);
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetZoomDenom() const
{
	if(!Form) return 1;
	return Form->GetZoomDenom();
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
// tTJSNC_Window::CreateNativeInstance : returns proper instance object
//---------------------------------------------------------------------------
tTJSNativeInstance *tTJSNC_Window::CreateNativeInstance()
{
	return new tTJSNI_Window();
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPCreateNativeClass_Window
//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_Window()
{
	tTJSNativeClass *cls = new tTJSNC_Window();
	static tjs_uint32 TJS_NCM_CLASSID;
	TJS_NCM_CLASSID = tTJSNC_Window::ClassID;
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(findFullScreenCandidates)
{
	if(numparams < 5) return TJS_E_BADPARAMCOUNT;

	std::vector<tTVPScreenModeCandidate> candidates;

	tTVPScreenMode preferred;
	preferred.Width = *param[0];
	preferred.Height = *param[1];
	preferred.BitsPerPixel = *param[2];
	tjs_int mode = *param[3];
	tjs_int zoom_mode = *param[4];

	TVPMakeFullScreenModeCandidates(preferred, (tTVPFullScreenResolutionMode)mode,
		(tTVPFullScreenUsingEngineZoomMode)zoom_mode, candidates);


	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL_OUTER(cls, findFullScreenCandidates)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(registerMessageReceiver)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
	if(numparams < 3) return TJS_E_BADPARAMCOUNT;

	_this->RegisterWindowMessageReceiver((tTVPWMRRegMode)((tjs_int)*param[0]),
		reinterpret_cast<void *>((tjs_int)(*param[1])),
		reinterpret_cast<const void *>((tjs_int)(*param[2])));

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL_OUTER(cls, registerMessageReceiver)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(HWND)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
		*result = (tTVInteger)(tjs_uint)_this->GetWindowHandleForPlugin();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL_OUTER(cls, HWND)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(drawDevice)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
		*result = _this->GetDrawDeviceObject();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
		_this->SetDrawDeviceObject(*param);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL_OUTER(cls, drawDevice)
//---------------------------------------------------------------------------

	TVPGetDisplayColorFormat(); // this will be ran only once here

	return cls;
}
//---------------------------------------------------------------------------


