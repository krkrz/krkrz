// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CIrrDeviceLinux.h"

#ifdef _IRR_USE_LINUX_DEVICE_

#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <time.h>
#include "IEventReceiver.h"
#include "os.h"
#include "CTimer.h"
#include "irrString.h"
#include "Keycodes.h"
#include "COSOperator.h"
#include "CColorConverter.h"
#include "SIrrCreationParameters.h"
#include <X11/XKBlib.h>

namespace irr
{
	namespace video
	{
		IVideoDriver* createOpenGLDriver(const core::dimension2d<s32>& screenSize,
			bool fullscreen, bool stencilBuffer, io::IFileSystem* io, bool vsync, bool antiAlias);
	}
} // end namespace irr



namespace irr
{

const char* wmDeleteWindow = "WM_DELETE_WINDOW";

//! constructor
CIrrDeviceLinux::CIrrDeviceLinux(video::E_DRIVER_TYPE driverType,
	const core::dimension2d<s32>& windowSize,
	u32 bits, bool fullscreen,
	bool sbuffer, bool vsync, bool antiAlias,
	IEventReceiver* receiver,
	const char* version)
 : CIrrDeviceStub(version, receiver),
#ifdef _IRR_COMPILE_WITH_X11_
	display(0), visual(0), screennr(0), window(0), StdHints(0), SoftwareImage(0),
#endif
#ifdef _IRR_COMPILE_WITH_OPENGL_
	Context(0),
#endif
	Fullscreen(fullscreen), StencilBuffer(sbuffer),
	AntiAlias(antiAlias), DriverType(driverType),
	Width(windowSize.Width), Height(windowSize.Height), Depth(24), 
	Close(false), WindowActive(false), WindowMinimized(false),
	UseXVidMode(false), UseXRandR(false), UseGLXWindow(false),
	AutorepeatSupport(0)
{
	#ifdef _DEBUG
	setDebugName("CIrrDeviceLinux");
	#endif

	// print version, distribution etc.
	// thx to LynxLuna for pointing me to the uname function
	core::stringc linuxversion;
	struct utsname LinuxInfo; 
	uname(&LinuxInfo);

	linuxversion += LinuxInfo.sysname;
	linuxversion += " ";
	linuxversion += LinuxInfo.release;
	linuxversion += " ";
	linuxversion += LinuxInfo.version;
	linuxversion += " ";
	linuxversion += LinuxInfo.machine;

	Operator = new COSOperator(linuxversion.c_str());
	os::Printer::log(linuxversion.c_str(), ELL_INFORMATION);

	// create keymap
	createKeyMap();

	// create window
	if (driverType != video::EDT_NULL)
	{
		// create the window, only if we do not use the null device
		if (!createWindow(windowSize, bits))
			return;
	}

	// create cursor control
	CursorControl = new CCursorControl(this, driverType == video::EDT_NULL);

	// create driver
	createDriver(windowSize, vsync);

	if (!VideoDriver)
		return;

	createGUIAndScene();
}


//! destructor
CIrrDeviceLinux::~CIrrDeviceLinux()
{
#ifdef _IRR_COMPILE_WITH_X11_
	if (StdHints)
		XFree(StdHints);
	if (display)
	{
		#ifdef _IRR_COMPILE_WITH_OPENGL_
		if (Context)
		{
			if (!glXMakeCurrent(display, None, NULL))
				os::Printer::log("Could not release glx context.", ELL_WARNING);
			glXDestroyContext(display, Context);
			if (UseGLXWindow)
				glXDestroyWindow(display, glxWin);
		}
		#endif // #ifdef _IRR_COMPILE_WITH_OPENGL_

		#ifdef _IRR_LINUX_X11_VIDMODE_
		if (UseXVidMode && Fullscreen)
		{
			XF86VidModeSwitchToMode(display, screennr, &oldVideoMode);
			XF86VidModeSetViewPort(display, screennr, 0, 0);
		}
		#endif
		#ifdef _IRR_LINUX_X11_RANDR_
		if (UseXRandR && Fullscreen)
		{
			XRRScreenConfiguration *config=XRRGetScreenInfo(display,DefaultRootWindow(display));
			XRRSetScreenConfig(display,config,DefaultRootWindow(display),oldRandrMode,oldRandrRotation,CurrentTime);
			XRRFreeScreenConfigInfo(config);
		}
		#endif

		if (DriverType == video::EDT_SOFTWARE || DriverType == video::EDT_BURNINGSVIDEO)
			XDestroyImage(SoftwareImage);
		XDestroyWindow(display,window);
		XCloseDisplay(display);
	}
	if (visual)
		XFree(visual);

#endif // #ifdef _IRR_COMPILE_WITH_X11_
}



#if defined(_IRR_COMPILE_WITH_X11_) && defined(_DEBUG)
int IrrPrintXError(Display *display, XErrorEvent *event)
{
	char msg[256];

	XGetErrorText(display, event->error_code, msg, 256);
	os::Printer::log("XErrorEvent", msg, ELL_WARNING);
	return 0;
}
#endif


bool CIrrDeviceLinux::createWindow(const core::dimension2d<s32>& windowSize,
						u32 bits)
{
	Width = windowSize.Width;
	Height = windowSize.Height;

#ifdef _IRR_COMPILE_WITH_X11_
#ifdef _DEBUG
	os::Printer::log("Creating X window...", ELL_INFORMATION);
	XSetErrorHandler(IrrPrintXError);
#endif

	display = XOpenDisplay(0);
	if (!display)
	{
		os::Printer::log("Error: Need running XServer to start Irrlicht Engine.", ELL_ERROR);
		return false;
	}

	screennr = DefaultScreen(display);

	// query extension

	if (Fullscreen)
	{
		s32 eventbase, errorbase;
		s32 bestMode = -1;
		s32 defaultDepth=DefaultDepth(display,screennr);

		#ifdef _IRR_LINUX_X11_VIDMODE_
		if (XF86VidModeQueryExtension(display, &eventbase, &errorbase))
		{
			// enumerate video modes
			s32 modeCount;
			XF86VidModeModeInfo** modes;

			XF86VidModeGetAllModeLines(display, screennr, &modeCount, &modes);

			// save current video mode
			oldVideoMode = *modes[0];

			// find fitting mode

			VideoModeList.setDesktop(defaultDepth, core::dimension2d<s32>(
				modes[0]->hdisplay, modes[0]->vdisplay));
			for (s32 i = 0; i<modeCount; ++i)
			{
				if (bestMode==-1 && modes[i]->hdisplay >= Width && modes[i]->vdisplay >= Height)
					bestMode = i;
				else if (bestMode!=-1 &&
						modes[i]->hdisplay >= Width &&
						modes[i]->vdisplay >= Height &&
						modes[i]->hdisplay < modes[bestMode]->hdisplay &&
						modes[i]->vdisplay < modes[bestMode]->vdisplay)
					bestMode = i;
				VideoModeList.addMode(core::dimension2d<s32>(
					modes[i]->hdisplay, modes[i]->vdisplay), defaultDepth);
			}
			if (bestMode != -1)
			{
				os::Printer::log("Starting fullscreen mode...", ELL_INFORMATION);
				XF86VidModeSwitchToMode(display, screennr, modes[bestMode]);
				XF86VidModeSetViewPort(display, screennr, 0, 0);
				UseXVidMode=true;
			}
			else
			{
				os::Printer::log("Could not find specified video mode, running windowed.", ELL_WARNING);
				Fullscreen = false;
			}

			XFree(modes);
		}
		else
		#endif
		#ifdef _IRR_LINUX_X11_RANDR_
		if (XRRQueryExtension(display, &eventbase, &errorbase))
		{
			s32 modeCount;
			XRRScreenConfiguration *config=XRRGetScreenInfo(display,DefaultRootWindow(display));
			oldRandrMode=XRRConfigCurrentConfiguration(config,&oldRandrRotation);
			XRRScreenSize *modes=XRRConfigSizes(config,&modeCount);
			VideoModeList.setDesktop(defaultDepth, core::dimension2d<s32>(
				modes[oldRandrMode].width, modes[oldRandrMode].height));
			for (s32 i = 0; i<modeCount; ++i)
			{
				if (bestMode==-1 && (u32)modes[i].width >= Width && (u32)modes[i].height >= Height)
					bestMode = i;
				else if (bestMode!=-1 &&
						(u32)modes[i].width >= Width &&
						(u32)modes[i].height >= Height &&
						modes[i].width < modes[bestMode].width &&
						modes[i].height < modes[bestMode].height)
					bestMode = i;
				VideoModeList.addMode(core::dimension2d<s32>(
					modes[i].width, modes[i].height), defaultDepth);
			}
			if (bestMode != -1)
			{
				XRRSetScreenConfig(display,config,DefaultRootWindow(display),bestMode,oldRandrRotation,CurrentTime);
				UseXRandR=true;
			}
			XRRFreeScreenConfigInfo(config);
		}
		else
		#endif
		{
			os::Printer::log("VidMode or RandR extension must be installed to allow Irrlicht "
			"to switch to fullscreen mode. Running in windowed mode instead.", ELL_WARNING);
			Fullscreen = false;
		}
	}
	
#ifdef _IRR_COMPILE_WITH_OPENGL_

	Context=0;
	GLXFBConfig glxFBConfig;
	int major, minor;
	bool isAvailableGLX=false;
	if (DriverType==video::EDT_OPENGL)
	{
		isAvailableGLX=glXQueryExtension(display,&major,&minor);
		if (isAvailableGLX && glXQueryVersion(display, &major, &minor))
		{
			if (major==1 && minor>2)
			{
				const int MAX_SAMPLES = 16;
				// attribute array for the draw buffer
				int visualAttrBuffer[] =
				{
				    GLX_RENDER_TYPE, GLX_RGBA_BIT,
				    GLX_RED_SIZE, 4,
				    GLX_GREEN_SIZE, 4,
				    GLX_BLUE_SIZE, 4,
				    GLX_ALPHA_SIZE, 4,
				    GLX_DEPTH_SIZE, 16,
				    GLX_DOUBLEBUFFER, GL_TRUE,
				    GLX_STENCIL_SIZE, 1,
				    GLX_SAMPLE_BUFFERS_ARB, GL_TRUE,
				    GLX_SAMPLES_ARB, MAX_SAMPLES,
				    None
				};

				GLXFBConfig *configList=0;
				int nitems=0;
				if (!AntiAlias)
				{
					visualAttrBuffer[17] = GL_FALSE;
					visualAttrBuffer[19] = 0;
				}
				if (StencilBuffer)
				{
					configList=glXChooseFBConfig(display, screennr, visualAttrBuffer,&nitems);
					if (!configList && AntiAlias)
					{
						while (!configList && (visualAttrBuffer[19]>1))
						{
							visualAttrBuffer[19] >>= 1;
							configList=glXChooseFBConfig(display, screennr, visualAttrBuffer,&nitems);
						}
						if (!configList)
						{
							visualAttrBuffer[17] = GL_FALSE;
							visualAttrBuffer[19] = 0;
							configList=glXChooseFBConfig(display, screennr, visualAttrBuffer,&nitems);
							if (configList)
							{
								os::Printer::log("No FSAA available.", ELL_WARNING);
								AntiAlias=false;
							}
							else
							{
								//reenable multisampling
								visualAttrBuffer[17] = GL_TRUE;
								visualAttrBuffer[19] = MAX_SAMPLES;
							}
						}
					}
				}
				// Next try without stencil buffer
				if (!configList)
				{
					if (StencilBuffer)
						os::Printer::log("No stencilbuffer available, disabling stencil shadows.", ELL_WARNING);
					StencilBuffer = false;
					visualAttrBuffer[15]=0;

					configList=glXChooseFBConfig(display, screennr, visualAttrBuffer,&nitems);
					if (!configList && AntiAlias)
					{
						while (!configList && (visualAttrBuffer[19]>1))
						{
							visualAttrBuffer[19] >>= 1;
							configList=glXChooseFBConfig(display, screennr, visualAttrBuffer,&nitems);
						}
						if (!configList)
						{
							visualAttrBuffer[17] = GL_FALSE;
							visualAttrBuffer[19] = 0;
							configList=glXChooseFBConfig(display, screennr, visualAttrBuffer,&nitems);
							if (configList)
							{
								os::Printer::log("No FSAA available.", ELL_WARNING);
								AntiAlias=false;
							}
							else
							{
								//reenable multisampling
								visualAttrBuffer[17] = GL_TRUE;
								visualAttrBuffer[19] = MAX_SAMPLES;
							}
						}
					}
				}
				// Next try without double buffer
				if (!configList)
				{
					os::Printer::log("No doublebuffering available.", ELL_WARNING);
					visualAttrBuffer[13] = GL_FALSE;
					configList=glXChooseFBConfig(display, screennr, visualAttrBuffer,&nitems);
					if (!configList && AntiAlias)
					{
						while (!configList && (visualAttrBuffer[19]>1))
						{
							visualAttrBuffer[19] >>= 1;
							configList=glXChooseFBConfig(display, screennr, visualAttrBuffer,&nitems);
						}
						if (!configList)
						{
							visualAttrBuffer[17] = GL_FALSE;
							visualAttrBuffer[19] = 0;
							configList=glXChooseFBConfig(display, screennr, visualAttrBuffer,&nitems);
							if (configList)
							{
								os::Printer::log("No FSAA available.", ELL_WARNING);
								AntiAlias=false;
							}
							else
							{
								//reenable multisampling
								visualAttrBuffer[17] = GL_TRUE;
								visualAttrBuffer[19] = MAX_SAMPLES;
							}
						}
					}
				}
				if (configList)
				{
					glxFBConfig=configList[0];
					XFree(configList);
					UseGLXWindow=true;
					visual = glXGetVisualFromFBConfig(display,glxFBConfig);
				}
			}
			else
			{
				// attribute array for the draw buffer
				int visualAttrBuffer[] =
				{
				    GLX_RGBA, GL_TRUE,
				    GLX_RED_SIZE, 4,
				    GLX_GREEN_SIZE, 4,
				    GLX_BLUE_SIZE, 4,
				    GLX_ALPHA_SIZE, 4,
				    GLX_DEPTH_SIZE, 16,
				    GLX_DOUBLEBUFFER, GL_TRUE,
				    GLX_STENCIL_SIZE, 1,
				    None
				};

				if (StencilBuffer)
					visual=glXChooseVisual(display, screennr, visualAttrBuffer);
				if (!visual)
				{
					if (StencilBuffer)
					{
						os::Printer::log("No stencilbuffer available, disabling stencil shadows.", ELL_WARNING);
						StencilBuffer = false;
					}
					visualAttrBuffer[15]=0;

					visual=glXChooseVisual(display, screennr, visualAttrBuffer);
					if (!visual)
					{
						os::Printer::log("No doublebuffering available.", ELL_WARNING);
						visualAttrBuffer[13] = GL_FALSE;
						visual=glXChooseVisual(display, screennr, visualAttrBuffer);
					}
				}
			}
		}
		else
			os::Printer::log("No GLX support available. OpenGL driver will not work.", ELL_WARNING);
	}
	
#endif // _IRR_COMPILE_WITH_OPENGL_

	// create visual with standard X methods
	if (!visual)
	{
		XVisualInfo visTempl; //Template to hold requested values
		int visNumber; // Return value of available visuals

		visTempl.screen = screennr;
		visTempl.depth = 16;
		while ((!visual) && (visTempl.depth<=32))
		{
			visual = XGetVisualInfo(display, VisualScreenMask|VisualDepthMask,
				&visTempl, &visNumber);
			visTempl.depth+=8;
		}
	}

	if (!visual)
	{
		os::Printer::log("Fatal error, could not get visual.", ELL_ERROR);
		XCloseDisplay(display);
		display=0;
		return false;
	}

	// create color map
	Colormap colormap;
	colormap = XCreateColormap(display,
			    RootWindow(display, visual->screen),
			    visual->visual, AllocNone);

	attributes.colormap = colormap;
	attributes.border_pixel = 0;
	attributes.event_mask = KeyPressMask | ButtonPressMask |
			StructureNotifyMask | PointerMotionMask |
			ButtonReleaseMask | KeyReleaseMask;

	// create Window, either for Fullscreen or windowed mode
	if (Fullscreen)
	{
		attributes.override_redirect = True;

		window = XCreateWindow(display,
				RootWindow(display, visual->screen),
				0, 0, Width, Height, 0, visual->depth,
				InputOutput, visual->visual,
				CWBorderPixel | CWColormap | CWEventMask |
				CWOverrideRedirect, &attributes);

		XWarpPointer(display, None, window, 0, 0, 0, 0, 0, 0);
		XMapRaised(display, window);
		XGrabKeyboard(display, window, True, GrabModeAsync,
			GrabModeAsync, CurrentTime);
		XGrabPointer(display, window, True, ButtonPressMask,
			GrabModeAsync, GrabModeAsync, window, None, CurrentTime);
	}
	else
	{ // we want windowed mode
		attributes.event_mask |= ExposureMask;
		attributes.event_mask |= FocusChangeMask;

		window = XCreateWindow(display,
				RootWindow(display, visual->screen),
				0, 0, Width, Height, 0, visual->depth,
				InputOutput, visual->visual,
				CWBorderPixel | CWColormap | CWEventMask,
				&attributes);

		Atom wmDelete;
		wmDelete = XInternAtom(display, wmDeleteWindow, True);
		XSetWMProtocols(display, window, &wmDelete, 1);
		XMapRaised(display, window);
	}
	WindowActive=true;
	XkbSetDetectableAutoRepeat(display, True, &AutorepeatSupport);

#ifdef _IRR_COMPILE_WITH_OPENGL_

	// connect glx context to window
	if (isAvailableGLX && DriverType==video::EDT_OPENGL)
	{
	if (UseGLXWindow)
	{
		glxWin=glXCreateWindow(display,glxFBConfig,window,NULL);
		if (glxWin)
		{
			// create glx context
			Context = glXCreateNewContext(display, glxFBConfig, GLX_RGBA_TYPE, NULL, True);
			if (Context)
			{
				if (!glXMakeContextCurrent(display, glxWin, glxWin, Context))
				{
					os::Printer::log("Could not make context current.", ELL_WARNING);
					glXDestroyContext(display, Context);
				}
			}
			else
			{
				os::Printer::log("Could not create GLX rendering context.", ELL_WARNING);
			}
		}
		else
		{
			os::Printer::log("Could not create GLX window.", ELL_WARNING);
		}
	}
	else
	{
		Context = glXCreateContext(display, visual, NULL, True);
		if (Context)
		{
			if (!glXMakeCurrent(display, window, Context))
			{
				os::Printer::log("Could not make context current.", ELL_WARNING);
				glXDestroyContext(display, Context);
			}
		}
		else
		{
			os::Printer::log("Could not create GLX rendering context.", ELL_WARNING);
		}
	}
	}
#endif // _IRR_COMPILE_WITH_OPENGL_

	Window tmp;
	u32 borderWidth;
	int x,y;

	XGetGeometry(display, window, &tmp, &x, &y, &Width, &Height, &borderWidth, &Depth);
	StdHints = XAllocSizeHints();
	long num;
	XGetWMNormalHints(display, window, StdHints, &num);

	// create an XImage for the software renderer 
	//(thx to Nadav for some clues on how to do that!)

	if (DriverType == video::EDT_SOFTWARE || DriverType == video::EDT_BURNINGSVIDEO)
	{
		SoftwareImage = XCreateImage(display, 
			visual->visual, visual->depth, 
			ZPixmap, 0, 0, Width, Height,
			BitmapPad(display), 0);

		// use malloc because X will free it later on
		SoftwareImage->data = (char*) malloc(SoftwareImage->bytes_per_line * SoftwareImage->height * sizeof(char));
	} 

#endif // #ifdef _IRR_COMPILE_WITH_X11_
	return true;
}


//! create the driver
void CIrrDeviceLinux::createDriver(const core::dimension2d<s32>& windowSize,
				   bool vsync)
{
	switch(DriverType)
	{
#ifdef _IRR_COMPILE_WITH_X11_

	case video::EDT_SOFTWARE:
		#ifdef _IRR_COMPILE_WITH_SOFTWARE_
		VideoDriver = video::createSoftwareDriver(windowSize, Fullscreen, FileSystem, this);
		#else
		os::Printer::log("No Software driver support compiled in.", ELL_ERROR);
		#endif
		break;
		
	case video::EDT_BURNINGSVIDEO:
		#ifdef _IRR_COMPILE_WITH_BURNINGSVIDEO_
		VideoDriver = video::createSoftwareDriver2(windowSize, Fullscreen, FileSystem, this);
		#else
		os::Printer::log("Burning's video driver was not compiled in.", ELL_ERROR);
		#endif
		break;

	case video::EDT_OPENGL:
	#ifdef _IRR_COMPILE_WITH_OPENGL_
		if (Context)
			VideoDriver = video::createOpenGLDriver(windowSize, Fullscreen, StencilBuffer, FileSystem, vsync, AntiAlias);
	#else
		os::Printer::log("No OpenGL support compiled in.", ELL_ERROR);
	#endif
		break;

	case video::EDT_DIRECT3D8:
	case video::EDT_DIRECT3D9:
		os::Printer::log("This driver is not available in Linux. Try OpenGL or Software renderer.",
			ELL_ERROR);
		break;

	case video::EDT_NULL:
		VideoDriver = video::createNullDriver(FileSystem, windowSize);
		break;

	default:
		os::Printer::log("Unable to create video driver of unknown type.", ELL_ERROR);
		break;
#else
	case video::EDT_NULL:
		VideoDriver = video::createNullDriver(FileSystem, windowSize);
		break;
	default:
		os::Printer::log("No X11 support compiled in. Only Null driver available.", ELL_ERROR);
		break;
#endif
	}
}



//! runs the device. Returns false if device wants to be deleted
bool CIrrDeviceLinux::run()
{
	os::Timer::tick();

#ifdef _IRR_COMPILE_WITH_X11_
	if ((DriverType != video::EDT_NULL) && display)
	{
		SEvent irrevent;

		while (XPending(display) > 0 && !Close)
		{
			XEvent event;
			XNextEvent(display, &event);

			switch (event.type)
			{
			case ConfigureNotify:
				// check for changed window size
				if ((event.xconfigure.width != (int) Width) ||
					(event.xconfigure.height != (int) Height))
				{
					Width = event.xconfigure.width;
					Height = event.xconfigure.height;

					// resize image data
					if (DriverType == video::EDT_SOFTWARE || DriverType == video::EDT_BURNINGSVIDEO)
					{
						XDestroyImage(SoftwareImage);

						SoftwareImage = XCreateImage(display, 
							visual->visual, visual->depth, 
							ZPixmap, 0, 0, Width, Height,
							BitmapPad(display), 0);

						// use malloc because X will free it later on
						SoftwareImage->data = (char*) malloc(SoftwareImage->bytes_per_line * SoftwareImage->height * sizeof(char));
					} 

					if (VideoDriver)
						VideoDriver->OnResize(core::dimension2d<s32>(Width, Height));
				}
				break;

			case MapNotify:
				WindowMinimized=false;
				break;

			case UnmapNotify:
				WindowMinimized=true;
				break;

			case FocusIn:
				WindowActive=true;
				break;

			case FocusOut:
				WindowActive=false;
				break;

			case MotionNotify:
				irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
				irrevent.MouseInput.Event = irr::EMIE_MOUSE_MOVED;
				irrevent.MouseInput.X = event.xbutton.x;
				irrevent.MouseInput.Y = event.xbutton.y;

				postEventFromUser(irrevent);
				break;

			case ButtonPress:
			case ButtonRelease:

				irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
				irrevent.MouseInput.X = event.xbutton.x;
				irrevent.MouseInput.Y = event.xbutton.y;

				irrevent.MouseInput.Event = irr::EMIE_COUNT;

				switch(event.xbutton.button)
				{
				case  Button1:
					irrevent.MouseInput.Event =
						(event.type == ButtonPress) ? irr::EMIE_LMOUSE_PRESSED_DOWN : irr::EMIE_LMOUSE_LEFT_UP;
					break;

				case  Button3:
					irrevent.MouseInput.Event =
						(event.type == ButtonPress) ? irr::EMIE_RMOUSE_PRESSED_DOWN : irr::EMIE_RMOUSE_LEFT_UP;
					break;

				case  Button2:
					irrevent.MouseInput.Event =
						(event.type == ButtonPress) ? irr::EMIE_MMOUSE_PRESSED_DOWN : irr::EMIE_MMOUSE_LEFT_UP;
					break;

				case  Button4:
					irrevent.MouseInput.Event = EMIE_MOUSE_WHEEL;
					irrevent.MouseInput.Wheel = 1.0f;
					break;

				case  Button5:
					irrevent.MouseInput.Event = EMIE_MOUSE_WHEEL;
					irrevent.MouseInput.Wheel = -1.0f;
					break;
				}

				if (irrevent.MouseInput.Event != irr::EMIE_COUNT)
					postEventFromUser(irrevent);
				break;

			case MappingNotify:
				XRefreshKeyboardMapping (&event.xmapping) ;
				break;

			case KeyRelease:
				if (0 == AutorepeatSupport)
				{
					// check for Autorepeat manually
					// We'll do the same as Windows does: Only send KeyPressed
					// So every KeyRelease is a real release
					XEvent next_event;
					XPeekEvent (event.xkey.display, &next_event);
					if ((next_event.type == KeyPress) &&
						(next_event.xkey.keycode == event.xkey.keycode) &&
						(next_event.xkey.time == event.xkey.time))
					{
						/* Ignore the key release event */
						break;
					}
				}
				// fall-through in case the release should be handled
			case KeyPress:
				{
					SKeyMap mp;
					//mp.X11Key = XLookupKeysym(&event.xkey, 0);
					char buf[5]="\0\0\0\0";
					XLookupString(&event.xkey, buf, 4, &mp.X11Key, NULL);

					s32 idx = KeyMap.binary_search(mp);

					if (idx != -1)
						irrevent.KeyInput.Key = (EKEY_CODE)KeyMap[idx].Win32Key;
					else
					{
						irrevent.KeyInput.Key = (EKEY_CODE)0;
						os::Printer::log("Could not find win32 key for x11 key.", ELL_WARNING);
					}
					irrevent.EventType = irr::EET_KEY_INPUT_EVENT;
					irrevent.KeyInput.PressedDown = (event.type == KeyPress);
					mbtowc(&irrevent.KeyInput.Char, buf, 4);
					irrevent.KeyInput.Control = (event.xkey.state & ControlMask) != 0;
					irrevent.KeyInput.Shift = (event.xkey.state & ShiftMask) != 0;
					postEventFromUser(irrevent);
				}
				break;

			case ClientMessage:
				{
					char *atom = XGetAtomName(display, event.xclient.message_type);
					if (*atom == *wmDeleteWindow)
					{
						os::Printer::log("Quit message received.", ELL_INFORMATION);
						Close = true;
					}
					XFree(atom);
				}
				break;

			default:
				break;
			} // end switch

		} // end while
	}
#endif //_IRR_COMPILE_WITH_X11_

	return !Close;
}


//! Pause the current process for the minimum time allowed only to allow other processes to execute
void CIrrDeviceLinux::yield()
{
	struct timespec ts = {0,0};
	nanosleep(&ts, NULL);
}

//! Pause execution and let other processes to run for a specified amount of time.
void CIrrDeviceLinux::sleep(u32 timeMs, bool pauseTimer=false)
{
	bool wasStopped = Timer ? Timer->isStopped() : true;
	
	struct timespec ts;
	ts.tv_sec = (time_t) (timeMs / 1000);
	ts.tv_nsec = (long) (timeMs % 1000) * 1000000;

	if (pauseTimer && !wasStopped)
		Timer->stop();

	nanosleep(&ts, NULL);

	if (pauseTimer && !wasStopped)
		Timer->start();
}

//! sets the caption of the window
void CIrrDeviceLinux::setWindowCaption(const wchar_t* text)
{
#ifdef _IRR_COMPILE_WITH_X11_
	if (DriverType == video::EDT_NULL)
		return;

	XTextProperty txt;
	XwcTextListToTextProperty(display, const_cast<wchar_t**>(&text), 1, XStdICCTextStyle, &txt);
	XSetWMName(display, window, &txt);
	XSetWMIconName(display, window, &txt);
	XFree(txt.value);
#endif
}



//! presents a surface in the client area
void CIrrDeviceLinux::present(video::IImage* image, s32 windowId, core::rect<s32>* src, core::rect<s32>* dest )
{
#ifdef _IRR_COMPILE_WITH_X11_
	// this is only necessary for software drivers.
	if (DriverType != video::EDT_SOFTWARE && DriverType != video::EDT_BURNINGSVIDEO)
		return;

	// thx to Nadav, who send me some clues of how to display the image 
	// to the X Server.

	if (image->getColorFormat() != video::ECF_A1R5G5B5 &&
	    image->getColorFormat() != video::ECF_A8R8G8B8)
	{
		os::Printer::log("Internal error, can only present A1R5G5B5 and A8R8G8B8 pictures.");
		return;
	}
	
	int destwidth = SoftwareImage->width;
	int destheight = SoftwareImage->height;
	int srcwidth = image->getDimension().Width;
	int srcheight = image->getDimension().Height;
	// clip images
	srcheight = srcheight < destheight ? srcheight : destheight;
	
	if ( image->getColorFormat() == video::ECF_A8R8G8B8 )
	{
		// display 24/32 bit image
		
		s32* srcdata = (s32*)image->lock();
		
		if ((Depth == 32)||(Depth == 24))
		{	
			int destPitch = SoftwareImage->bytes_per_line;
			u8* destData = reinterpret_cast<u8*>(SoftwareImage->data);
			
			for (int y=0; y<srcheight; ++y)
			{
				video::CColorConverter::convert_A8R8G8B8toA8R8G8B8(srcdata,srcwidth<destwidth?srcwidth:destwidth,destData);
				srcdata+=srcwidth;
				destData+=destPitch;
			}
		}
		else
		if (Depth == 16)
		{
			// convert to R5G6B6
			
			int destPitch = SoftwareImage->bytes_per_line;
			u8* destData = reinterpret_cast<u8*>(SoftwareImage->data);
			
			for (int y=0; y<srcheight; ++y)
			{
				video::CColorConverter::convert_A8R8G8B8toR5G6B5(srcdata,srcwidth<destwidth?srcwidth:destwidth,destData);
				srcdata+=srcwidth;
				destData+=destPitch;
			}
		}
		else
			os::Printer::log("Unsupported screen depth.");
		
		image->unlock();
	}
	else
	{
		// display 16 bit image
	
		s16* srcdata = (s16*)image->lock();
	
		if (Depth == 16)
		{
			// convert from A1R5G5B5 to R5G6B6
			
			int destPitch = SoftwareImage->bytes_per_line;
			u8* destData = reinterpret_cast<u8*>(SoftwareImage->data);
			
			for (int y=0; y<srcheight; ++y)
			{
				video::CColorConverter::convert_A1R5G5B5toR5G6B5(srcdata,srcwidth<destwidth?srcwidth:destwidth,destData);
				srcdata+=srcwidth;
				destData+=destPitch;
			}
		}
		else
		if ((Depth == 32)||(Depth == 24))
		{
			// convert from A1R5G5B5 to X8R8G8B8
				
			int destPitch = SoftwareImage->bytes_per_line;
			u8* destData = reinterpret_cast<u8*>(SoftwareImage->data);
			
			for (int y=0; y<srcheight; ++y)
			{
				video::CColorConverter::convert_A1R5G5B5toA8R8G8B8(srcdata,srcwidth<destwidth?srcwidth:destwidth,destData);
				srcdata+=srcwidth;
				destData+=destPitch;
			}
		}
		else
			os::Printer::log("Unsupported screen depth.");
	
		image->unlock();
	}

	GC gc = DefaultGC(display, DefaultScreen(display));
	XPutImage(display, window, gc, SoftwareImage, 0, 0, 0, 0, destwidth, destheight);
#endif
}



//! notifies the device that it should close itself
void CIrrDeviceLinux::closeDevice()
{
	Close = true;
}



//! returns if window is active. if not, nothing need to be drawn
bool CIrrDeviceLinux::isWindowActive() const
{
	return WindowActive;
}



//! Sets if the window should be resizeable in windowed mode.
void CIrrDeviceLinux::setResizeAble(bool resize)
{
#ifdef _IRR_COMPILE_WITH_X11_
	if (DriverType == video::EDT_NULL)
		return;

	XUnmapWindow(display, window);
	if ( !resize )
	{
		// Must be heap memory because data size depends on X Server
		XSizeHints *hints = XAllocSizeHints();
		hints->flags=PSize|PMinSize|PMaxSize;
		hints->min_width=hints->max_width=hints->base_width=Width;
		hints->min_height=hints->max_height=hints->base_height=Height;
		XSetWMNormalHints(display, window, hints);
		XFree(hints);
	}
	else
	{
		XSetWMNormalHints(display, window, StdHints);
	}
	XMapWindow(display, window);
	XFlush(display);
#endif // #ifdef _IRR_COMPILE_WITH_X11_
}


//! \return Returns a pointer to a list with all video modes supported
//! by the gfx adapter.
video::IVideoModeList* CIrrDeviceLinux::getVideoModeList()
{
#ifdef _IRR_COMPILE_WITH_X11_
	if (!VideoModeList.getVideoModeCount())
	{
		bool temporaryDisplay = false;

		if (!display)
		{
			display = XOpenDisplay(0);
			temporaryDisplay=true;
		}
		if (display)
		{
			s32 eventbase, errorbase;
			s32 defaultDepth=DefaultDepth(display,screennr);

			#ifdef _IRR_LINUX_X11_VIDMODE_
			if (XF86VidModeQueryExtension(display, &eventbase, &errorbase))
			{
				// enumerate video modes
				int modeCount;
				XF86VidModeModeInfo** modes;

				XF86VidModeGetAllModeLines(display, screennr, &modeCount, &modes);

				// save current video mode
				oldVideoMode = *modes[0];

				// find fitting mode

				VideoModeList.setDesktop(defaultDepth, core::dimension2d<s32>(
					modes[0]->hdisplay, modes[0]->vdisplay));
				for (int i = 0; i<modeCount; ++i)
				{
					VideoModeList.addMode(core::dimension2d<s32>(
						modes[i]->hdisplay, modes[i]->vdisplay), defaultDepth);
				}
				XFree(modes);
			}
			else
			#endif
			#ifdef _IRR_LINUX_X11_RANDR_
			if (XRRQueryExtension(display, &eventbase, &errorbase))
			{
				int modeCount;
				XRRScreenConfiguration *config=XRRGetScreenInfo(display,DefaultRootWindow(display));
				oldRandrMode=XRRConfigCurrentConfiguration(config,&oldRandrRotation);
				XRRScreenSize *modes=XRRConfigSizes(config,&modeCount);
				VideoModeList.setDesktop(defaultDepth, core::dimension2d<s32>(
					modes[oldRandrMode].width, modes[oldRandrMode].height));
				for (int i = 0; i<modeCount; ++i)
				{
					VideoModeList.addMode(core::dimension2d<s32>(
						modes[i].width, modes[i].height), defaultDepth);
				}
				XRRFreeScreenConfigInfo(config);
			}
			else
			#endif
			{
				os::Printer::log("VidMode or RandR X11 extension requireed for VideoModeList." , ELL_WARNING);
			}
		}
		if (display && temporaryDisplay)
		{
			XCloseDisplay(display);
			display=0;
		}
	}
#endif

	return &VideoModeList;
}



void CIrrDeviceLinux::createKeyMap()
{
	// I don't know if this is the best method  to create
	// the lookuptable, but I'll leave it like that until
	// I find a better version.

#ifdef _IRR_COMPILE_WITH_X11_
	KeyMap.push_back(SKeyMap(XK_BackSpace, KEY_BACK));
	KeyMap.push_back(SKeyMap(XK_Tab, KEY_TAB));
	KeyMap.push_back(SKeyMap(XK_Linefeed, 0)); // ???
	KeyMap.push_back(SKeyMap(XK_Clear, KEY_CLEAR));
	KeyMap.push_back(SKeyMap(XK_Return, KEY_RETURN));
	KeyMap.push_back(SKeyMap(XK_Pause, KEY_PAUSE));
	KeyMap.push_back(SKeyMap(XK_Scroll_Lock, KEY_SCROLL));
	KeyMap.push_back(SKeyMap(XK_Sys_Req, 0)); // ???
	KeyMap.push_back(SKeyMap(XK_Escape, KEY_ESCAPE));
	KeyMap.push_back(SKeyMap(XK_Insert, KEY_INSERT));
	KeyMap.push_back(SKeyMap(XK_Delete, KEY_DELETE));
	KeyMap.push_back(SKeyMap(XK_Home, KEY_HOME));
	KeyMap.push_back(SKeyMap(XK_Left, KEY_LEFT));
	KeyMap.push_back(SKeyMap(XK_Up, KEY_UP));
	KeyMap.push_back(SKeyMap(XK_Right, KEY_RIGHT));
	KeyMap.push_back(SKeyMap(XK_Down, KEY_DOWN));
	KeyMap.push_back(SKeyMap(XK_Prior, KEY_PRIOR));
	KeyMap.push_back(SKeyMap(XK_Page_Up, KEY_PRIOR));
	KeyMap.push_back(SKeyMap(XK_Next, KEY_NEXT));
	KeyMap.push_back(SKeyMap(XK_Page_Down, KEY_NEXT));
	KeyMap.push_back(SKeyMap(XK_End, KEY_END));
	KeyMap.push_back(SKeyMap(XK_Begin, KEY_HOME));
	KeyMap.push_back(SKeyMap(XK_KP_Space, KEY_SPACE));
	KeyMap.push_back(SKeyMap(XK_KP_Tab, KEY_TAB));
	KeyMap.push_back(SKeyMap(XK_KP_Enter, KEY_RETURN));
	KeyMap.push_back(SKeyMap(XK_KP_F1, KEY_F1));
	KeyMap.push_back(SKeyMap(XK_KP_F2, KEY_F2));
	KeyMap.push_back(SKeyMap(XK_KP_F3, KEY_F3));
	KeyMap.push_back(SKeyMap(XK_KP_F4, KEY_F4));
	KeyMap.push_back(SKeyMap(XK_KP_Left, KEY_LEFT));
	KeyMap.push_back(SKeyMap(XK_KP_Up, KEY_UP));
	KeyMap.push_back(SKeyMap(XK_KP_Right, KEY_RIGHT));
	KeyMap.push_back(SKeyMap(XK_KP_Down, KEY_DOWN));
	KeyMap.push_back(SKeyMap(XK_KP_Prior, KEY_PRIOR));
	KeyMap.push_back(SKeyMap(XK_KP_Page_Up, KEY_PRIOR));
	KeyMap.push_back(SKeyMap(XK_KP_Next, KEY_NEXT));
	KeyMap.push_back(SKeyMap(XK_KP_Page_Down, KEY_NEXT));
	KeyMap.push_back(SKeyMap(XK_KP_End, KEY_END));
	KeyMap.push_back(SKeyMap(XK_KP_Begin, KEY_HOME));
	KeyMap.push_back(SKeyMap(XK_KP_Insert, KEY_INSERT));
	KeyMap.push_back(SKeyMap(XK_KP_Delete, KEY_DELETE));
	KeyMap.push_back(SKeyMap(XK_KP_Equal, 0)); // ???
	KeyMap.push_back(SKeyMap(XK_KP_Multiply, KEY_MULTIPLY));
	KeyMap.push_back(SKeyMap(XK_KP_Add, KEY_ADD));
	KeyMap.push_back(SKeyMap(XK_KP_Separator, KEY_SEPARATOR));
	KeyMap.push_back(SKeyMap(XK_KP_Subtract, KEY_SUBTRACT));
	KeyMap.push_back(SKeyMap(XK_KP_Decimal, KEY_DECIMAL));
	KeyMap.push_back(SKeyMap(XK_KP_Divide, KEY_DIVIDE));
	KeyMap.push_back(SKeyMap(XK_KP_0, KEY_KEY_0));
	KeyMap.push_back(SKeyMap(XK_KP_1, KEY_KEY_1));
	KeyMap.push_back(SKeyMap(XK_KP_2, KEY_KEY_2));
	KeyMap.push_back(SKeyMap(XK_KP_3, KEY_KEY_3));
	KeyMap.push_back(SKeyMap(XK_KP_4, KEY_KEY_4));
	KeyMap.push_back(SKeyMap(XK_KP_5, KEY_KEY_5));
	KeyMap.push_back(SKeyMap(XK_KP_6, KEY_KEY_6));
	KeyMap.push_back(SKeyMap(XK_KP_7, KEY_KEY_7));
	KeyMap.push_back(SKeyMap(XK_KP_8, KEY_KEY_8));
	KeyMap.push_back(SKeyMap(XK_KP_9, KEY_KEY_9));
	KeyMap.push_back(SKeyMap(XK_F1, KEY_F1));
	KeyMap.push_back(SKeyMap(XK_F2, KEY_F2));
	KeyMap.push_back(SKeyMap(XK_F3, KEY_F3));
	KeyMap.push_back(SKeyMap(XK_F4, KEY_F4));
	KeyMap.push_back(SKeyMap(XK_F5, KEY_F5));
	KeyMap.push_back(SKeyMap(XK_F6, KEY_F6));
	KeyMap.push_back(SKeyMap(XK_F7, KEY_F7));
	KeyMap.push_back(SKeyMap(XK_F8, KEY_F8));
	KeyMap.push_back(SKeyMap(XK_F9, KEY_F9));
	KeyMap.push_back(SKeyMap(XK_F10, KEY_F10));
	KeyMap.push_back(SKeyMap(XK_F11, KEY_F11));
	KeyMap.push_back(SKeyMap(XK_F12, KEY_F12));
	KeyMap.push_back(SKeyMap(XK_Shift_L, KEY_LSHIFT));
	KeyMap.push_back(SKeyMap(XK_Shift_R, KEY_RSHIFT));
	KeyMap.push_back(SKeyMap(XK_Control_L, KEY_LCONTROL));
	KeyMap.push_back(SKeyMap(XK_Control_R, KEY_RCONTROL));
	KeyMap.push_back(SKeyMap(XK_Caps_Lock, KEY_CAPITAL));
	KeyMap.push_back(SKeyMap(XK_Shift_Lock, KEY_CAPITAL));
	KeyMap.push_back(SKeyMap(XK_Meta_L, KEY_LWIN));
	KeyMap.push_back(SKeyMap(XK_Meta_R, KEY_RWIN));
	KeyMap.push_back(SKeyMap(XK_Alt_L, KEY_LMENU));
	KeyMap.push_back(SKeyMap(XK_Alt_R, KEY_RMENU));
	KeyMap.push_back(SKeyMap(XK_ISO_Level3_Shift, KEY_RMENU));
	KeyMap.push_back(SKeyMap(XK_Menu, KEY_MENU));
	KeyMap.push_back(SKeyMap(XK_space, KEY_SPACE));
	KeyMap.push_back(SKeyMap(XK_exclam, 0)); //?
	KeyMap.push_back(SKeyMap(XK_quotedbl, 0)); //?
	KeyMap.push_back(SKeyMap(XK_section, 0)); //?
	KeyMap.push_back(SKeyMap(XK_numbersign, 0)); //?
	KeyMap.push_back(SKeyMap(XK_dollar, 0)); //?
	KeyMap.push_back(SKeyMap(XK_percent, 0)); //?
	KeyMap.push_back(SKeyMap(XK_ampersand, 0)); //?
	KeyMap.push_back(SKeyMap(XK_apostrophe, 0)); //?
	KeyMap.push_back(SKeyMap(XK_parenleft, 0)); //?
	KeyMap.push_back(SKeyMap(XK_parenright, 0)); //?
	KeyMap.push_back(SKeyMap(XK_asterisk, 0)); //?
	KeyMap.push_back(SKeyMap(XK_plus, KEY_PLUS)); //?
	KeyMap.push_back(SKeyMap(XK_comma, KEY_COMMA)); //?
	KeyMap.push_back(SKeyMap(XK_minus, KEY_MINUS)); //?
	KeyMap.push_back(SKeyMap(XK_period, KEY_PERIOD)); //? 
	KeyMap.push_back(SKeyMap(XK_slash, 0)); //?
	KeyMap.push_back(SKeyMap(XK_0, KEY_KEY_0));
	KeyMap.push_back(SKeyMap(XK_1, KEY_KEY_1));
	KeyMap.push_back(SKeyMap(XK_2, KEY_KEY_2));
	KeyMap.push_back(SKeyMap(XK_3, KEY_KEY_3));
	KeyMap.push_back(SKeyMap(XK_4, KEY_KEY_4));
	KeyMap.push_back(SKeyMap(XK_5, KEY_KEY_5));
	KeyMap.push_back(SKeyMap(XK_6, KEY_KEY_6));
	KeyMap.push_back(SKeyMap(XK_7, KEY_KEY_7));
	KeyMap.push_back(SKeyMap(XK_8, KEY_KEY_8));
	KeyMap.push_back(SKeyMap(XK_9, KEY_KEY_9));
	KeyMap.push_back(SKeyMap(XK_colon, 0)); //?
	KeyMap.push_back(SKeyMap(XK_semicolon, 0)); //?
	KeyMap.push_back(SKeyMap(XK_less, 0)); //?
	KeyMap.push_back(SKeyMap(XK_equal, 0)); //?
	KeyMap.push_back(SKeyMap(XK_greater, 0)); //?
	KeyMap.push_back(SKeyMap(XK_question, 0)); //?
	KeyMap.push_back(SKeyMap(XK_at, 0)); //?
	KeyMap.push_back(SKeyMap(XK_mu, 0)); //?
	KeyMap.push_back(SKeyMap(XK_EuroSign, 0)); //?
	KeyMap.push_back(SKeyMap(XK_A, KEY_KEY_A));
	KeyMap.push_back(SKeyMap(XK_B, KEY_KEY_B));
	KeyMap.push_back(SKeyMap(XK_C, KEY_KEY_C));
	KeyMap.push_back(SKeyMap(XK_D, KEY_KEY_D));
	KeyMap.push_back(SKeyMap(XK_E, KEY_KEY_E));
	KeyMap.push_back(SKeyMap(XK_F, KEY_KEY_F));
	KeyMap.push_back(SKeyMap(XK_G, KEY_KEY_G));
	KeyMap.push_back(SKeyMap(XK_H, KEY_KEY_H));
	KeyMap.push_back(SKeyMap(XK_I, KEY_KEY_I));
	KeyMap.push_back(SKeyMap(XK_J, KEY_KEY_J));
	KeyMap.push_back(SKeyMap(XK_K, KEY_KEY_K));
	KeyMap.push_back(SKeyMap(XK_L, KEY_KEY_L));
	KeyMap.push_back(SKeyMap(XK_M, KEY_KEY_M));
	KeyMap.push_back(SKeyMap(XK_N, KEY_KEY_N));
	KeyMap.push_back(SKeyMap(XK_O, KEY_KEY_O));
	KeyMap.push_back(SKeyMap(XK_P, KEY_KEY_P));
	KeyMap.push_back(SKeyMap(XK_Q, KEY_KEY_Q));
	KeyMap.push_back(SKeyMap(XK_R, KEY_KEY_R));
	KeyMap.push_back(SKeyMap(XK_S, KEY_KEY_S));
	KeyMap.push_back(SKeyMap(XK_T, KEY_KEY_T));
	KeyMap.push_back(SKeyMap(XK_U, KEY_KEY_U));
	KeyMap.push_back(SKeyMap(XK_V, KEY_KEY_V));
	KeyMap.push_back(SKeyMap(XK_W, KEY_KEY_W));
	KeyMap.push_back(SKeyMap(XK_X, KEY_KEY_X));
	KeyMap.push_back(SKeyMap(XK_Y, KEY_KEY_Y));
	KeyMap.push_back(SKeyMap(XK_Z, KEY_KEY_Z));
	KeyMap.push_back(SKeyMap(XK_Adiaeresis, 0)); //?
	KeyMap.push_back(SKeyMap(XK_Odiaeresis, 0)); //?
	KeyMap.push_back(SKeyMap(XK_Udiaeresis, 0)); //?
	KeyMap.push_back(SKeyMap(XK_bracketleft, 0)); //?
	KeyMap.push_back(SKeyMap(XK_backslash, 0)); //?
	KeyMap.push_back(SKeyMap(XK_bracketright, 0)); //?
	KeyMap.push_back(SKeyMap(XK_asciicircum, 0)); //?
	KeyMap.push_back(SKeyMap(XK_degree, 0)); //?
	KeyMap.push_back(SKeyMap(XK_underscore, 0)); //?
	KeyMap.push_back(SKeyMap(XK_grave, 0)); //?
	KeyMap.push_back(SKeyMap(XK_acute, 0)); //?
	KeyMap.push_back(SKeyMap(XK_quoteleft, 0)); //?
	KeyMap.push_back(SKeyMap(XK_a, KEY_KEY_A));
	KeyMap.push_back(SKeyMap(XK_b, KEY_KEY_B));
	KeyMap.push_back(SKeyMap(XK_c, KEY_KEY_C));
	KeyMap.push_back(SKeyMap(XK_d, KEY_KEY_D));
	KeyMap.push_back(SKeyMap(XK_e, KEY_KEY_E));
	KeyMap.push_back(SKeyMap(XK_f, KEY_KEY_F));
	KeyMap.push_back(SKeyMap(XK_g, KEY_KEY_G));
	KeyMap.push_back(SKeyMap(XK_h, KEY_KEY_H));
	KeyMap.push_back(SKeyMap(XK_i, KEY_KEY_I));
	KeyMap.push_back(SKeyMap(XK_j, KEY_KEY_J));
	KeyMap.push_back(SKeyMap(XK_k, KEY_KEY_K));
	KeyMap.push_back(SKeyMap(XK_l, KEY_KEY_L));
	KeyMap.push_back(SKeyMap(XK_m, KEY_KEY_M));
	KeyMap.push_back(SKeyMap(XK_n, KEY_KEY_N));
	KeyMap.push_back(SKeyMap(XK_o, KEY_KEY_O));
	KeyMap.push_back(SKeyMap(XK_p, KEY_KEY_P));
	KeyMap.push_back(SKeyMap(XK_q, KEY_KEY_Q));
	KeyMap.push_back(SKeyMap(XK_r, KEY_KEY_R));
	KeyMap.push_back(SKeyMap(XK_s, KEY_KEY_S));
	KeyMap.push_back(SKeyMap(XK_t, KEY_KEY_T));
	KeyMap.push_back(SKeyMap(XK_u, KEY_KEY_U));
	KeyMap.push_back(SKeyMap(XK_v, KEY_KEY_V));
	KeyMap.push_back(SKeyMap(XK_w, KEY_KEY_W));
	KeyMap.push_back(SKeyMap(XK_x, KEY_KEY_X));
	KeyMap.push_back(SKeyMap(XK_y, KEY_KEY_Y));
	KeyMap.push_back(SKeyMap(XK_z, KEY_KEY_Z));
	KeyMap.push_back(SKeyMap(XK_ssharp, 0)); //?
	KeyMap.push_back(SKeyMap(XK_adiaeresis, 0)); //?
	KeyMap.push_back(SKeyMap(XK_odiaeresis, 0)); //?
	KeyMap.push_back(SKeyMap(XK_udiaeresis, 0)); //?

	KeyMap.sort();
#endif
}


IRRLICHT_API IrrlichtDevice* IRRCALLCONV createDeviceEx(const SIrrlichtCreationParameters& param)
{
	CIrrDeviceLinux* dev = new CIrrDeviceLinux(
		param.DriverType,
		param.WindowSize,
		param.Bits,
		param.Fullscreen,
		param.Stencilbuffer,
		param.Vsync,
		param.AntiAlias,
		param.EventReceiver,
		param.SDK_version_do_not_use);
	
	if (dev && !dev->getVideoDriver() && param.DriverType != video::EDT_NULL)
	{
		dev->drop();
		dev = 0;
	}

	return dev;
}


} // end namespace

#endif // _IRR_USE_LINUX_DEVICE_

