// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"

#ifdef _IRR_USE_WINDOWS_DEVICE_

#include "CIrrDeviceWin32.h"
#include "IEventReceiver.h"
#include "irrList.h"
#include "os.h"

#include "CTimer.h"
#include "irrString.h"
#include "COSOperator.h"
#include "dimension2d.h"
#include <winuser.h>
#include "irrlicht.h"


namespace irr
{
	namespace video
	{
		#ifdef _IRR_COMPILE_WITH_DIRECT3D_8_
		IVideoDriver* createDirectX8Driver(const core::dimension2d<s32>& screenSize, HWND window,
			u32 bits, bool fullscreen, bool stencilbuffer, io::IFileSystem* io,
			bool pureSoftware, bool highPrecisionFPU, bool vsync, bool antiAlias);
		#endif

		#ifdef _IRR_COMPILE_WITH_DIRECT3D_9_
		IVideoDriver* createDirectX9Driver(const core::dimension2d<s32>& screenSize, HWND window,
			u32 bits, bool fullscreen, bool stencilbuffer, io::IFileSystem* io,
			bool pureSoftware, bool highPrecisionFPU, bool vsync, bool antiAlias);
		#endif

		#ifdef _IRR_COMPILE_WITH_OPENGL_
		IVideoDriver* createOpenGLDriver(const core::dimension2d<s32>& screenSize, HWND window,
			u32 bits, bool fullscreen, bool stencilBuffer, io::IFileSystem* io,
			bool vsync, bool antiAlias);
		#endif
	}
} // end namespace irr



struct SEnvMapper
{
	HWND hWnd;
	irr::CIrrDeviceWin32* irrDev;
};

irr::core::list<SEnvMapper> EnvMap;

SEnvMapper* getEnvMapperFromHWnd(HWND hWnd)
{
	irr::core::list<SEnvMapper>::Iterator it = EnvMap.begin();
	for (; it!= EnvMap.end(); ++it)
		if ((*it).hWnd == hWnd)
			return &(*it);

	return 0;
}

irr::CIrrDeviceWin32* getDeviceFromHWnd(HWND hWnd)
{
	irr::core::list<SEnvMapper>::Iterator it = EnvMap.begin();
	for (; it!= EnvMap.end(); ++it)
		if ((*it).hWnd == hWnd)
			return (*it).irrDev;

	return 0;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	#ifndef WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL 0x020A
	#endif
	#ifndef WHEEL_DELTA
	#define WHEEL_DELTA 120
	#endif

	irr::CIrrDeviceWin32* dev = 0;
	irr::SEvent event;
	SEnvMapper* envm = 0;

	BYTE allKeys[256];

	static irr::s32 ClickCount=0;
	if (GetCapture() != hWnd && ClickCount > 0)
		ClickCount = 0;

	switch (message)
	{
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		return 0;

	case WM_ERASEBKGND:
		return 0;

	case WM_SETCURSOR:
		envm = getEnvMapperFromHWnd(hWnd);
		if (envm && !envm->irrDev->getWin32CursorControl()->isVisible())
		{
			SetCursor(NULL);
			return 0;
		}
		break;

	case WM_MOUSEWHEEL:
		event.EventType = irr::EET_MOUSE_INPUT_EVENT;
		event.MouseInput.Wheel = ((irr::f32)((short)HIWORD(wParam))) / (irr::f32)WHEEL_DELTA;
		event.MouseInput.Event = irr::EMIE_MOUSE_WHEEL;

		POINT p; // fixed by jox
		p.x = 0; p.y = 0;
		ClientToScreen(hWnd, &p);
		event.MouseInput.X = LOWORD(lParam) - p.x;
		event.MouseInput.Y = HIWORD(lParam) - p.y;

		dev = getDeviceFromHWnd(hWnd);
		if (dev)
			dev->postEventFromUser(event);
		break;

	case WM_LBUTTONDOWN:
		ClickCount++;
		SetCapture(hWnd);
		event.EventType = irr::EET_MOUSE_INPUT_EVENT;
		event.MouseInput.Event = irr::EMIE_LMOUSE_PRESSED_DOWN;
		event.MouseInput.X = (short)LOWORD(lParam);
		event.MouseInput.Y = (short)HIWORD(lParam);
		dev = getDeviceFromHWnd(hWnd);
		if (dev)
			dev->postEventFromUser(event);
		return 0;

	case WM_LBUTTONUP:
		ClickCount--;
		if (ClickCount<1)
		{
			ClickCount=0;
			ReleaseCapture();
		}
		event.EventType = irr::EET_MOUSE_INPUT_EVENT;
		event.MouseInput.Event = irr::EMIE_LMOUSE_LEFT_UP;
		event.MouseInput.X = (short)LOWORD(lParam);
		event.MouseInput.Y = (short)HIWORD(lParam);
		dev = getDeviceFromHWnd(hWnd);
		if (dev)
			dev->postEventFromUser(event);
		return 0;

	case WM_RBUTTONDOWN:
		ClickCount++;
		SetCapture(hWnd);
		event.EventType = irr::EET_MOUSE_INPUT_EVENT;
		event.MouseInput.Event = irr::EMIE_RMOUSE_PRESSED_DOWN;
		event.MouseInput.X = (short)LOWORD(lParam);
		event.MouseInput.Y = (short)HIWORD(lParam);
		dev = getDeviceFromHWnd(hWnd);
		if (dev)
			dev->postEventFromUser(event);
		return 0;

	case WM_RBUTTONUP:
		ClickCount--;
		if (ClickCount<1)
		{
			ClickCount=0;
			ReleaseCapture();
		}
		event.EventType = irr::EET_MOUSE_INPUT_EVENT;
		event.MouseInput.Event = irr::EMIE_RMOUSE_LEFT_UP;
		event.MouseInput.X = (short)LOWORD(lParam);
		event.MouseInput.Y = (short)HIWORD(lParam);
		dev = getDeviceFromHWnd(hWnd);
		if (dev)
			dev->postEventFromUser(event);
		return 0;

	case WM_MBUTTONDOWN:
		ClickCount++;
		SetCapture(hWnd);
		event.EventType = irr::EET_MOUSE_INPUT_EVENT;
		event.MouseInput.Event = irr::EMIE_MMOUSE_PRESSED_DOWN;
		event.MouseInput.X = (short)LOWORD(lParam);
		event.MouseInput.Y = (short)HIWORD(lParam);
		dev = getDeviceFromHWnd(hWnd);
		if (dev)
			dev->postEventFromUser(event);
		return 0;

	case WM_MBUTTONUP:
		ClickCount--;
		if (ClickCount<1)
		{
			ClickCount=0;
			ReleaseCapture();
		}
		event.EventType = irr::EET_MOUSE_INPUT_EVENT;
		event.MouseInput.Event = irr::EMIE_MMOUSE_LEFT_UP;
		event.MouseInput.X = (short)LOWORD(lParam);
		event.MouseInput.Y = (short)HIWORD(lParam);
		dev = getDeviceFromHWnd(hWnd);
		if (dev)
			dev->postEventFromUser(event);
		return 0;

	case WM_MOUSEMOVE:
		event.EventType = irr::EET_MOUSE_INPUT_EVENT;
		event.MouseInput.Event = irr::EMIE_MOUSE_MOVED;
		event.MouseInput.X = (short)LOWORD(lParam);
		event.MouseInput.Y = (short)HIWORD(lParam);
		dev = getDeviceFromHWnd(hWnd);

		if (dev)
			dev->postEventFromUser(event);

		return 0;

	case WM_KEYDOWN:
	case WM_KEYUP:
		{
			event.EventType = irr::EET_KEY_INPUT_EVENT;
			event.KeyInput.Key = (irr::EKEY_CODE)wParam;
			event.KeyInput.PressedDown = (message==WM_KEYDOWN);
			dev = getDeviceFromHWnd(hWnd);

			WORD KeyAsc=0;
			GetKeyboardState(allKeys);
			ToAscii((UINT)wParam,(UINT)lParam,allKeys,&KeyAsc,0);

			event.KeyInput.Shift = ((allKeys[VK_SHIFT] & 0x80)!=0);
			event.KeyInput.Control = ((allKeys[VK_CONTROL] & 0x80)!=0);
			event.KeyInput.Char = (KeyAsc & 0x00ff); //KeyAsc >= 0 ? KeyAsc : 0;

			if (dev)
				dev->postEventFromUser(event);

			return 0;
		}

	case WM_SIZE:
		{
			// resize
			dev = getDeviceFromHWnd(hWnd);
			if (dev)
				dev->OnResized();
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_SYSCOMMAND:
		// prevent screensaver or monitor powersave mode from starting
		if ((wParam & 0xFFF0) == SC_SCREENSAVE ||
			(wParam & 0xFFF0) == SC_MONITORPOWER)
			return 0;
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

namespace irr
{

//! constructor
CIrrDeviceWin32::CIrrDeviceWin32(video::E_DRIVER_TYPE driverType,
				 core::dimension2d<s32> windowSize,
				 u32 bits, bool fullscreen,
				 bool stencilbuffer, bool vsync,
				 bool antiAlias,
				 bool highPrecisionFPU,
				 IEventReceiver* receiver,
				 HWND externalWindow,
				 const char* version)
: CIrrDeviceStub(version, receiver), HWnd(0), ChangedToFullScreen(false),
	FullScreen(fullscreen), IsNonNTWindows(false), Resized(false),
	ExternalWindow(false), Win32CursorControl(0)
{
	core::stringc winversion;
	getWindowsVersion(winversion);
	Operator = new COSOperator(winversion.c_str());
	os::Printer::log(winversion.c_str(), ELL_INFORMATION);

	// create window

	HINSTANCE hInstance = GetModuleHandle(0);

	#ifdef _DEBUG
	setDebugName("CIrrDeviceWin32");
	#endif

	// create the window, only if we do not use the null device
	if (driverType != video::EDT_NULL && externalWindow==0)
	{
		const c8* ClassName = "CIrrDeviceWin32";

		// Register Class
		WNDCLASSEX wcex;
		wcex.cbSize		= sizeof(WNDCLASSEX);
		wcex.style		= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= (WNDPROC)WndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hInstance;
		wcex.hIcon		= NULL;
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszMenuName	= 0;
		wcex.lpszClassName	= ClassName;
		wcex.hIconSm		= 0;

		// if there is an icon, load it
		wcex.hIcon = (HICON)LoadImage(hInstance, "irrlicht.ico", IMAGE_ICON, 0,0, LR_LOADFROMFILE); 

		RegisterClassEx(&wcex);

		// calculate client size

		RECT clientSize;
		clientSize.top = 0;
		clientSize.left = 0;
		clientSize.right = windowSize.Width;
		clientSize.bottom = windowSize.Height;

		DWORD style = WS_POPUP;

		if (!fullscreen)
			style = WS_SYSMENU | WS_BORDER | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

		AdjustWindowRect(&clientSize, style, FALSE);

		s32 realWidth = clientSize.right - clientSize.left;
		s32 realHeight = clientSize.bottom - clientSize.top;

		s32 windowLeft = (GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
		s32 windowTop = (GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;

		if (fullscreen)
		{
			windowLeft = 0;
			windowTop = 0;
		}

		// create window

		HWnd = CreateWindow( ClassName, "", style, windowLeft, windowTop,
					realWidth, realHeight,	NULL, NULL, hInstance, NULL);

		ShowWindow(HWnd , SW_SHOW);
		UpdateWindow(HWnd);

		// fix ugly ATI driver bugs. Thanks to ariaci
		MoveWindow(HWnd, windowLeft, windowTop, realWidth, realHeight, TRUE);
	}

	// attach external window
	if (externalWindow)
	{
		HWnd = externalWindow;
		if (windowSize.Width == 0 || windowSize.Height== 0) {
			RECT r;
			GetClientRect(HWnd, &r);
			windowSize.Width = r.right - r.left;
			windowSize.Height = r.bottom - r.top;
		}
		fullscreen = false;
		ExternalWindow = true;
	}

	// create cursor control

	Win32CursorControl = new CCursorControl(windowSize, HWnd, fullscreen);
	CursorControl = Win32CursorControl;

	// create driver

	createDriver(driverType, windowSize, bits, fullscreen, stencilbuffer, vsync, antiAlias, highPrecisionFPU);

	if (VideoDriver)
		createGUIAndScene();

	// register environment

	SEnvMapper em;
	em.irrDev = this;
	em.hWnd = HWnd;
	EnvMap.push_back(em);

	// set this as active window
	SetActiveWindow(HWnd);
	SetForegroundWindow(HWnd);
}



//! destructor
CIrrDeviceWin32::~CIrrDeviceWin32()
{
	// unregister environment

	irr::core::list<SEnvMapper>::Iterator it = EnvMap.begin();
	for (; it!= EnvMap.end(); ++it)
		if ((*it).hWnd == HWnd)
		{
			EnvMap.erase(it);
			break;
		}

	if (ChangedToFullScreen)
		ChangeDisplaySettings(NULL,0);
}



//! create the driver
void CIrrDeviceWin32::createDriver(video::E_DRIVER_TYPE driverType,
				   const core::dimension2d<s32>& windowSize,
				   u32 bits,
				   bool fullscreen,
				   bool stencilbuffer,
				   bool vsync,
				   bool antiAlias,
				   bool highPrecisionFPU)
{
	switch(driverType)
	{
	case video::EDT_DIRECT3D8:
		#ifdef _IRR_COMPILE_WITH_DIRECT3D_8_
		VideoDriver = video::createDirectX8Driver(windowSize, HWnd, bits, fullscreen,
			stencilbuffer, FileSystem, false, highPrecisionFPU, vsync, antiAlias);
		if (!VideoDriver)
		{
			os::Printer::log("Could not create DIRECT3D8 Driver.", ELL_ERROR);
		}
		#else
		os::Printer::log("DIRECT3D8 Driver was not compiled into this dll. Try another one.", ELL_ERROR);
		#endif // _IRR_COMPILE_WITH_DIRECT3D_8_

		break;

	case video::EDT_DIRECT3D9:
		#ifdef _IRR_COMPILE_WITH_DIRECT3D_9_
		VideoDriver = video::createDirectX9Driver(windowSize, HWnd, bits, fullscreen,
			stencilbuffer, FileSystem, false, highPrecisionFPU, vsync, antiAlias);
		if (!VideoDriver)
		{
			os::Printer::log("Could not create DIRECT3D9 Driver.", ELL_ERROR);
		}
		#else
		os::Printer::log("DIRECT3D9 Driver was not compiled into this dll. Try another one.", ELL_ERROR);
		#endif // _IRR_COMPILE_WITH_DIRECT3D_9_

		break;

	case video::EDT_OPENGL:

		#ifdef _IRR_COMPILE_WITH_OPENGL_
		if (fullscreen)	switchToFullScreen(windowSize.Width, windowSize.Height, bits);
		VideoDriver = video::createOpenGLDriver(windowSize, HWnd, bits, fullscreen, stencilbuffer, FileSystem,
			vsync, antiAlias);
		if (!VideoDriver)
		{
			os::Printer::log("Could not create OpenGL driver.", ELL_ERROR);
		}
		#else
		os::Printer::log("OpenGL driver was not compiled in.", ELL_ERROR);
		#endif
		break;

	case video::EDT_SOFTWARE:

		#ifdef _IRR_COMPILE_WITH_SOFTWARE_
		if (fullscreen)	switchToFullScreen(windowSize.Width, windowSize.Height, bits);
		VideoDriver = video::createSoftwareDriver(windowSize, fullscreen, FileSystem, this);
		#else
		os::Printer::log("Software driver was not compiled in.", ELL_ERROR);
		#endif

		break;

	case video::EDT_BURNINGSVIDEO:
		#ifdef _IRR_COMPILE_WITH_BURNINGSVIDEO_
		if (fullscreen)	switchToFullScreen(windowSize.Width, windowSize.Height, bits);
		VideoDriver = video::createSoftwareDriver2(windowSize, fullscreen, FileSystem, this);
		#else
		os::Printer::log("Burning's Video driver was not compiled in.", ELL_ERROR);
		#endif 
		break;

	case video::EDT_NULL:
		// create null driver
		VideoDriver = video::createNullDriver(FileSystem, windowSize);
		break;

	default:
		os::Printer::log("Unable to create video driver of unknown type.", ELL_ERROR);
		break;
	}
}



//! runs the device. Returns false if device wants to be deleted
bool CIrrDeviceWin32::run()
{
	os::Timer::tick();

	MSG msg;

	bool quit = false;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);

		if (ExternalWindow && msg.hwnd == HWnd)
			WndProc(HWnd, msg.message, msg.wParam, msg.lParam);
		else
			DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
			quit = true;
	}

	if (!quit)
		resizeIfNecessary();

	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return !quit;
}


//! Pause the current process for the minimum time allowed only to allow other processes to execute
void CIrrDeviceWin32::yield()
{
	Sleep(1);
	
}

//! Pause execution and let other processes to run for a specified amount of time.
void CIrrDeviceWin32::sleep(u32 timeMs, bool pauseTimer)
{
	bool wasStopped = Timer ? Timer->isStopped() : true;
	if (pauseTimer && !wasStopped)
		Timer->stop();
	
	Sleep(timeMs);

	if (pauseTimer && !wasStopped)
		Timer->start();
}


void CIrrDeviceWin32::resizeIfNecessary()
{
	if (!Resized)
		return;

	RECT r;
	GetClientRect(HWnd, &r);

	char tmp[255];

	if (r.right < 2 || r.bottom < 2)
	{
		sprintf(tmp, "Ignoring resize operation to (%ld %ld)", r.right, r.bottom);
		os::Printer::log(tmp);
	}
	else
	{
		sprintf(tmp, "Resizing window (%ld %ld)", r.right, r.bottom);
		os::Printer::log(tmp);

		getVideoDriver()->OnResize(irr::core::dimension2d<irr::s32>(r.right, r.bottom));
	}

	Resized = false;
}



//! sets the caption of the window
void CIrrDeviceWin32::setWindowCaption(const wchar_t* text)
{
	if (IsNonNTWindows)
	{
		core::stringc s = text;
		SetWindowTextA(HWnd, s.c_str());
	}
	else
		SetWindowTextW(HWnd, text);
}



//! presents a surface in the client area
void CIrrDeviceWin32::present(video::IImage* image, s32 windowId, core::rect<s32>* src, core::rect<s32>* dest )
{
	HWND hwnd = HWnd;
	if ( windowId )
		hwnd = reinterpret_cast<HWND>(windowId);

	HDC dc = GetDC(hwnd);

	if ( dc )
	{
		RECT rect;
		if (dest) {
			rect.left   = dest->UpperLeftCorner.X;
			rect.top    = dest->UpperLeftCorner.Y;
			rect.right  = dest->LowerRightCorner.X;
			rect.bottom = dest->LowerRightCorner.Y;
		} else {
			GetClientRect(hwnd, &rect);
		}
		const void* memory = (const void *)image->lock();

		BITMAPV4HEADER bi;
		ZeroMemory (&bi, sizeof(bi));
		bi.bV4Size          = sizeof(BITMAPINFOHEADER);
		bi.bV4BitCount      = image->getBitsPerPixel();
		bi.bV4Planes        = 1;
		bi.bV4Width         = image->getDimension().Width;
		bi.bV4Height        = -image->getDimension().Height;
		bi.bV4V4Compression = BI_BITFIELDS;
		bi.bV4AlphaMask     = image->getAlphaMask ();
		bi.bV4RedMask       = image->getRedMask ();
		bi.bV4GreenMask     = image->getGreenMask();
		bi.bV4BlueMask      = image->getBlueMask();

		if ( src )
		{
			StretchDIBits(dc, rect.left, rect.top, rect.right, rect.bottom,
					src->UpperLeftCorner.X, src->UpperLeftCorner.Y,
					src->getWidth(), src->getHeight(),
					memory, (const BITMAPINFO*)(&bi), DIB_RGB_COLORS, SRCCOPY);
		}
		else
		{
			StretchDIBits(dc, rect.left, rect.top, rect.right, rect.bottom,
					0, 0, image->getDimension().Width, image->getDimension().Height,
					memory, (const BITMAPINFO*)(&bi), DIB_RGB_COLORS, SRCCOPY);
		}

		image->unlock();

		ReleaseDC(hwnd, dc);
	}
}



//! notifies the device that it should close itself
void CIrrDeviceWin32::closeDevice()
{
	MSG msg;
	PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
	PostQuitMessage(0);
	PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
	DestroyWindow(HWnd);
}



//! returns if window is active. if not, nothing need to be drawn
bool CIrrDeviceWin32::isWindowActive() const
{
	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return (GetActiveWindow() == HWnd);
}


//! switches to fullscreen
bool CIrrDeviceWin32::switchToFullScreen(s32 width, s32 height, s32 bits)
{
	DEVMODE dm;
	memset(&dm, 0, sizeof(dm));
	dm.dmSize = sizeof(dm);
	// use default values from current setting
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
	dm.dmPelsWidth = width;
	dm.dmPelsHeight	= height;
	dm.dmBitsPerPel	= bits;
	dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

	LONG res = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
	if (res != DISP_CHANGE_SUCCESSFUL)
	{ // try again without forcing display frequency
		dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		res = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
	}

	bool ret = false;
	switch(res)
	{
	case DISP_CHANGE_SUCCESSFUL:
		ChangedToFullScreen = true;
		ret = true;
		break;
	case DISP_CHANGE_RESTART:
		os::Printer::log("Switch to fullscreen: The computer must be restarted in order for the graphics mode to work.", ELL_ERROR);
		break;
	case DISP_CHANGE_BADFLAGS:
		os::Printer::log("Switch to fullscreen: An invalid set of flags was passed in.", ELL_ERROR);
		break;
	case DISP_CHANGE_BADPARAM:
		os::Printer::log("Switch to fullscreen: An invalid parameter was passed in. This can include an invalid flag or combination of flags.", ELL_ERROR);
		break;
	case DISP_CHANGE_FAILED:
		os::Printer::log("Switch to fullscreen: The display driver failed the specified graphics mode.", ELL_ERROR);
		break;
	case DISP_CHANGE_BADMODE:
		os::Printer::log("Switch to fullscreen: The graphics mode is not supported.", ELL_ERROR);
		break;
	default:
		os::Printer::log("An unknown error occured while changing to fullscreen.", ELL_ERROR);
		break;
	}
	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return ret;
}


//! returns the win32 cursor control
CIrrDeviceWin32::CCursorControl* CIrrDeviceWin32::getWin32CursorControl()
{
	return Win32CursorControl;
}


//! \return Returns a pointer to a list with all video modes supported
//! by the gfx adapter.
video::IVideoModeList* CIrrDeviceWin32::getVideoModeList()
{
	if (!VideoModeList.getVideoModeCount())
	{
		// enumerate video modes.
		DWORD i=0;
		DEVMODE mode;
		memset(&mode, 0, sizeof(mode)); 
		mode.dmSize = sizeof(mode);

		while (EnumDisplaySettings(NULL, i, &mode))
		{
			VideoModeList.addMode(core::dimension2d<s32>(mode.dmPelsWidth, mode.dmPelsHeight),
				mode.dmBitsPerPel);

			++i;
		}

		if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &mode))
			VideoModeList.setDesktop(mode.dmBitsPerPel, core::dimension2d<s32>(mode.dmPelsWidth, mode.dmPelsHeight));
	}

	return &VideoModeList;
}


void CIrrDeviceWin32::getWindowsVersion(core::stringc& out)
{
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if(!(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO*) &osvi)))
	{
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) )
			return;
	}

	switch (osvi.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_NT:
		if (osvi.dwMajorVersion <= 4)
			out.append("Microsoft Windows NT ");
		else
		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
			out.append("Microsoft Windows 2000 ");
		else
		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
			out.append("Microsoft Windows XP ");
		else
		if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0 )
			out.append("Microsoft Windows Vista ");

		if( bOsVersionInfoEx )
		{
			#ifdef VER_SUITE_ENTERPRISE
			if (osvi.wProductType == VER_NT_WORKSTATION)
			{
#ifndef __BORLANDC__
				if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
					out.append("Personal ");
				else
					out.append("Professional ");
#endif
			}
			else if (osvi.wProductType == VER_NT_SERVER)
			{
				if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
					out.append("DataCenter Server ");
				else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
					out.append("Advanced Server ");
				else
					out.append("Server ");
			}
			#endif
		}
		else
		{
			HKEY hKey;
			char szProductType[80];
			DWORD dwBufLen;

			RegOpenKeyEx( HKEY_LOCAL_MACHINE,
				"SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
				0, KEY_QUERY_VALUE, &hKey );
			RegQueryValueEx( hKey, "ProductType", NULL, NULL,
				(LPBYTE) szProductType, &dwBufLen);
			RegCloseKey( hKey );

			if (lstrcmpi( "WINNT", szProductType) == 0 )
				out.append("Professional ");
			if ( lstrcmpi( "LANMANNT", szProductType) == 0 )
				out.append("Server " );
			if ( lstrcmpi( "SERVERNT", szProductType) == 0 )
				out.append("Advanced Server ");
		}

		// Display version, service pack (if any), and build number.

		char tmp[255];

		if (osvi.dwMajorVersion <= 4 )
		{
			sprintf (tmp, "version %ld.%ld %s (Build %ld)",
				osvi.dwMajorVersion,
				osvi.dwMinorVersion,
				osvi.szCSDVersion,
				osvi.dwBuildNumber & 0xFFFF);
		}
		else
		{
			sprintf (tmp, "%s (Build %ld)", osvi.szCSDVersion,
				osvi.dwBuildNumber & 0xFFFF);
		}

		out.append(tmp);
		break;

	case VER_PLATFORM_WIN32_WINDOWS:

		IsNonNTWindows = true;

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
		{
			out.append("Microsoft Windows 95 ");
			if ( osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B' )
				out.append("OSR2 " );
		}

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
		{
			out.append("Microsoft Windows 98 ");
			if ( osvi.szCSDVersion[1] == 'A' )
				out.append( "SE " );
		}

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
			out.append("Microsoft Windows Me ");

		break;

	case VER_PLATFORM_WIN32s:

		IsNonNTWindows = true;
		out.append("Microsoft Win32s ");
		break;
	}
}

//! Notifies the device, that it has been resized
void CIrrDeviceWin32::OnResized()
{
	Resized = true;
}

//! Sets if the window should be resizeable in windowed mode.
void CIrrDeviceWin32::setResizeAble(bool resize)
{
	if (ExternalWindow || !getVideoDriver() || FullScreen)
		return;

	LONG style = WS_POPUP;

	if (!resize)
		style = WS_SYSMENU | WS_BORDER | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	else
		style = WS_THICKFRAME | WS_SYSMENU | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_MAXIMIZEBOX;

	if (!SetWindowLong(HWnd, GWL_STYLE, style))
		os::Printer::log("Could not change window style.");

	RECT clientSize;
	clientSize.top = 0;
	clientSize.left = 0;
	clientSize.right = getVideoDriver()->getScreenSize().Width;
	clientSize.bottom = getVideoDriver()->getScreenSize().Height;

	AdjustWindowRect(&clientSize, style, FALSE);

	s32 realWidth = clientSize.right - clientSize.left;
	s32 realHeight = clientSize.bottom - clientSize.top;

	s32 windowLeft = (GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
	s32 windowTop = (GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;

	SetWindowPos(HWnd, HWND_TOP, windowLeft, windowTop, realWidth, realHeight,
		SWP_FRAMECHANGED | SWP_NOMOVE | SWP_SHOWWINDOW);
}


IRRLICHT_API IrrlichtDevice* IRRCALLCONV createDeviceEx(
	const SIrrlichtCreationParameters& parameters)
{
	CIrrDeviceWin32* dev = new CIrrDeviceWin32(
		parameters.DriverType,
		parameters.WindowSize,
		parameters.Bits,
		parameters.Fullscreen,
		parameters.Stencilbuffer,
		parameters.Vsync,
		parameters.AntiAlias,
		parameters.HighPrecisionFPU,
		parameters.EventReceiver,
		reinterpret_cast<HWND>(parameters.WindowId),
		parameters.SDK_version_do_not_use);

	if (dev && !dev->getVideoDriver() && parameters.DriverType != video::EDT_NULL)
	{
		dev->closeDevice(); // destroy window
		dev->run(); // consume quit message
		dev->drop();
		dev = 0;
	}

	return dev;
}


} // end namespace

#endif // _IRR_USE_WINDOWS_DEVICE_

