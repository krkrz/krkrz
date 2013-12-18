// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"

#ifdef _IRR_USE_SDL_DEVICE_

#include "CIrrDeviceSDL.h"
#include "IEventReceiver.h"
#include "irrList.h"
#include "os.h"
#include "CTimer.h"
#include "irrString.h"
#include "Keycodes.h"
#include "COSOperator.h"
#include <stdio.h>
#include <stdlib.h>
#include "SIrrCreationParameters.h"
#include <SDL/SDL_syswm.h>

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
CIrrDeviceSDL::CIrrDeviceSDL(video::E_DRIVER_TYPE driverType,
				const core::dimension2d<s32>& windowSize,
				u32 bits,
				bool fullscreen, bool stencilbuffer, bool vsync,
				bool antiAlias, IEventReceiver* receiver,
				void* windowID, const char* version)
	: CIrrDeviceStub(version, receiver), Depth(bits),
	Fullscreen(fullscreen), Stencilbuffer(stencilbuffer), Vsync(vsync),
	AntiAlias(antiAlias), Resizeable(false),
	Screen((SDL_Surface*)windowID), SDL_Flags(SDL_HWSURFACE|SDL_ANYFORMAT),
	Width(windowSize.Width), Height(windowSize.Height), Close(0),
	WindowActive(false)
{
	#ifdef _DEBUG
	setDebugName("CIrrDeviceSDL");
	#endif

	// Initialize SDL... Timer for sleep, video for the obvious, and
	// noparachute prevents SDL from catching fatal errors.
	if ( SDL_Init( SDL_INIT_TIMER|SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE ) < 0 )
	{
		os::Printer::log( "Unable to initialize SDL!", SDL_GetError());
		Close = 1;
	}

	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);

	SDL_GetWMInfo(&info);
	core::stringc sdlversion = "SDL Version ";
	sdlversion += info.version.major;
	sdlversion += ".";
	sdlversion += info.version.minor;
	sdlversion += ".";
	sdlversion += info.version.patch;

	Operator = new COSOperator(sdlversion.c_str());
	os::Printer::log(sdlversion.c_str(), ELL_INFORMATION);

	// create keymap
	createKeyMap();

	if ( Fullscreen )
		SDL_Flags |= SDL_FULLSCREEN;
	if (driverType == video::EDT_OPENGL)
		SDL_Flags |= SDL_OPENGL;
	else
		SDL_Flags |= SDL_DOUBLEBUF;
	// create window
	if (driverType != video::EDT_NULL)
	{
		// create the window, only if we do not use the null device
		createWindow(driverType);
	}

	// create cursor control
	CursorControl = new CCursorControl(this);

	// create driver
	createDriver(driverType, windowSize);

	if (VideoDriver)
		createGUIAndScene();
}



//! destructor
CIrrDeviceSDL::~CIrrDeviceSDL()
{
	if (Screen)
		SDL_FreeSurface(Screen);
	SDL_Quit();
}



bool CIrrDeviceSDL::createWindow(video::E_DRIVER_TYPE driverType)
{
	if ( Close )
		return false;

	if (driverType == video::EDT_OPENGL)
	{
		if (Depth==16)
		{
			SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
			SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
			SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
		}
		else
		{
			SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
			SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
			SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
		}
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, Depth);
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	}

	if ( !Screen )
		Screen = SDL_SetVideoMode( Width, Height, Depth, SDL_Flags );
	if ( !Screen )
	{
		os::Printer::log( "Could not initialize display!" );
		return false;
	}

	return true;
}


//! create the driver
void CIrrDeviceSDL::createDriver(video::E_DRIVER_TYPE driverType,
				const core::dimension2d<s32>& windowSize)
{
	switch(driverType)
	{
	case video::EDT_DIRECT3D8:
	case video::EDT_DIRECT3D9:
		os::Printer::log("This driver is not available in SDL.", ELL_ERROR);
		break;

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
		VideoDriver = video::createOpenGLDriver(windowSize, Fullscreen, Stencilbuffer, FileSystem, Vsync, AntiAlias);
	#else
		os::Printer::log("No OpenGL support compiled in.", ELL_ERROR);
	#endif
		break;

	case video::EDT_NULL:
		VideoDriver = video::createNullDriver(FileSystem, windowSize);
		break;

	default:
		os::Printer::log("Unable to create video driver of unknown type.", ELL_ERROR);
		break;
	}
}



//! runs the device. Returns false if device wants to be deleted
bool CIrrDeviceSDL::run()
{
	os::Timer::tick();

	SEvent irrevent;

	while ( !Close && SDL_PollEvent( &SDL_event ) )
	{
		switch ( SDL_event.type )
		{
		case SDL_MOUSEMOTION:
			irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
			irrevent.MouseInput.Event = irr::EMIE_MOUSE_MOVED;
			MouseX = irrevent.MouseInput.X = SDL_event.motion.x;
			MouseY = irrevent.MouseInput.Y = SDL_event.motion.y;

			postEventFromUser(irrevent);
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:

			irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
			irrevent.MouseInput.X = SDL_event.button.x;
			irrevent.MouseInput.Y = SDL_event.button.y;

			irrevent.MouseInput.Event = irr::EMIE_MOUSE_MOVED;

			switch(SDL_event.button.button)
			{
			case  1:
				irrevent.MouseInput.Event =
					(SDL_event.type == SDL_MOUSEBUTTONDOWN) ? irr::EMIE_LMOUSE_PRESSED_DOWN : irr::EMIE_LMOUSE_LEFT_UP;
				break;

			case  2:
				irrevent.MouseInput.Event =
					(SDL_event.type == SDL_MOUSEBUTTONDOWN) ? irr::EMIE_RMOUSE_PRESSED_DOWN : irr::EMIE_RMOUSE_LEFT_UP;
				break;

			case  3:
				irrevent.MouseInput.Event =
					(SDL_event.type == SDL_MOUSEBUTTONDOWN) ? irr::EMIE_MMOUSE_PRESSED_DOWN : irr::EMIE_MMOUSE_LEFT_UP;
				break;
			}

			if (irrevent.MouseInput.Event != irr::EMIE_MOUSE_MOVED)
				postEventFromUser(irrevent);
			break;

		case SDL_KEYDOWN:
		case SDL_KEYUP:
			{
				SKeyMap mp;
				mp.SDLKey = SDL_event.key.keysym.sym;
				s32 idx = KeyMap.binary_search(mp);

				if (idx != -1)
				{
					irrevent.EventType = irr::EET_KEY_INPUT_EVENT;
					irrevent.KeyInput.Key = (EKEY_CODE)KeyMap[idx].Win32Key;
					irrevent.KeyInput.PressedDown = (SDL_event.type == SDL_KEYDOWN);
					postEventFromUser(irrevent);
				}
				else
					os::Printer::log("Could not find win32 key for SDL key.");
			}
			break;

		case SDL_QUIT:
			Close = true;
			break;

		case SDL_ACTIVEEVENT:
			if (SDL_event.active.state == SDL_APPMOUSEFOCUS)
				WindowActive = (SDL_event.active.gain==1);
			break;

		default:
			break;
		} // end switch

	} // end while

	return !Close;
}


//! pause execution temporarily
void CIrrDeviceSDL::yield()
{
	SDL_Delay(0);
}


//! pause execution for a specified time
void CIrrDeviceSDL::sleep(u32 timeMs, bool pauseTimer)
{
	bool wasStopped = Timer ? Timer->isStopped() : true;
	if (pauseTimer && !wasStopped)
		Timer->stop();
	
	SDL_Delay(timeMs);

	if (pauseTimer && !wasStopped)
		Timer->start();
}


//! sets the caption of the window
void CIrrDeviceSDL::setWindowCaption(const wchar_t* text)
{
	core::stringc textc = text;
	SDL_WM_SetCaption( textc.c_str( ), textc.c_str( ) );
}



//! presents a surface in the client area
void CIrrDeviceSDL::present(video::IImage* surface, s32 windowId, core::rect<s32>* src, core::rect<s32>* dest)
{
	SDL_Rect srcClip;
	SDL_Surface *sdlSurface = SDL_CreateRGBSurfaceFrom(
			surface->lock(), surface->getDimension().Width, surface->getDimension().Height,
			surface->getBitsPerPixel(), surface->getPitch(),
			surface->getRedMask(), surface->getGreenMask(), surface->getBlueMask(), 0);
	if (src)
	{
		srcClip.x = src->UpperLeftCorner.X;
		srcClip.y = src->UpperLeftCorner.Y;
		srcClip.w = src->getWidth();
		srcClip.h = src->getHeight();
		SDL_BlitSurface(sdlSurface, &srcClip, Screen, NULL);
	}
	else
		SDL_BlitSurface(sdlSurface, NULL, Screen, NULL);
	SDL_UpdateRect(Screen, 0, 0, surface->getDimension().Width, surface->getDimension().Height);
	SDL_FreeSurface(sdlSurface);
	surface->unlock();
}



//! notifies the device that it should close itself
void CIrrDeviceSDL::closeDevice()
{
	Close = true;
}



//! \return Returns a pointer to a list with all video modes supported
video::IVideoModeList* CIrrDeviceSDL::getVideoModeList()
{
	if (!VideoModeList.getVideoModeCount())
	{
		// enumerate video modes.
		const SDL_VideoInfo *vi = SDL_GetVideoInfo();
		SDL_Rect **modes = SDL_ListModes(vi->vfmt, SDL_Flags);
		if (modes != (SDL_Rect **)0)
		{
			if (modes == (SDL_Rect **)-1)
				os::Printer::log("All modes available.\n");
			else
			{
				for (u32 i=0; modes[i]; ++i)
					VideoModeList.addMode(core::dimension2d<s32>(modes[i]->w, modes[i]->h), vi->vfmt->BitsPerPixel);
			}
		}
	}

	return &VideoModeList;
}



//! Sets if the window should be resizeable in windowed mode.
void CIrrDeviceSDL::setResizeAble(bool resize)
{
	if (resize != Resizeable)
	{
		if (resize)
			SDL_Flags |= SDL_RESIZABLE;
		else
			SDL_Flags &= ~SDL_RESIZABLE;
		SDL_FreeSurface(Screen);
		Screen = SDL_SetVideoMode( Width, Height, Depth, SDL_Flags );
		Resizeable = resize;
	}
}



//! returns if window is active. if not, nothing need to be drawn
bool CIrrDeviceSDL::isWindowActive() const
{
	return WindowActive;
}


void CIrrDeviceSDL::createKeyMap()
{
	// I don't know if this is the best method  to create
	// the lookuptable, but I'll leave it like that until
	// I find a better version.

	KeyMap.push_back(SKeyMap(SDLK_BACKSPACE, KEY_BACK));
	KeyMap.push_back(SKeyMap(SDLK_TAB, KEY_TAB));

	//KeyMap.push_back(SKeyMap(XK_Linefeed, 0)); // ???

	KeyMap.push_back(SKeyMap(SDLK_CLEAR, KEY_CLEAR));
	KeyMap.push_back(SKeyMap(SDLK_RETURN, KEY_RETURN));
	KeyMap.push_back(SKeyMap(SDLK_PAUSE, KEY_PAUSE));

	//KeyMap.push_back(SKeyMap(XK_Scroll_Lock, 0)); // ???
	//KeyMap.push_back(SKeyMap(XK_Sys_Req, 0)); // ???

	KeyMap.push_back(SKeyMap(SDLK_ESCAPE, KEY_ESCAPE));
	KeyMap.push_back(SKeyMap(SDLK_DELETE, KEY_DELETE));
	KeyMap.push_back(SKeyMap(SDLK_HOME, KEY_HOME));
	KeyMap.push_back(SKeyMap(SDLK_LEFT, KEY_LEFT));
	KeyMap.push_back(SKeyMap(SDLK_UP, KEY_UP));
	KeyMap.push_back(SKeyMap(SDLK_RIGHT, KEY_RIGHT));
	KeyMap.push_back(SKeyMap(SDLK_DOWN, KEY_DOWN));

	KeyMap.push_back(SKeyMap(SDLK_PAGEUP, KEY_PRIOR));
	KeyMap.push_back(SKeyMap(SDLK_PAGEDOWN, KEY_NEXT));
	KeyMap.push_back(SKeyMap(SDLK_END, KEY_END));
	KeyMap.push_back(SKeyMap(SDLK_HOME, KEY_HOME));
	KeyMap.push_back(SKeyMap(SDLK_SPACE, KEY_SPACE));
	KeyMap.push_back(SKeyMap(SDLK_TAB, KEY_TAB));
	KeyMap.push_back(SKeyMap(SDLK_RETURN, KEY_RETURN));

	KeyMap.push_back(SKeyMap(SDLK_F1,  KEY_F1));
	KeyMap.push_back(SKeyMap(SDLK_F2,  KEY_F2));
	KeyMap.push_back(SKeyMap(SDLK_F3,  KEY_F3));
	KeyMap.push_back(SKeyMap(SDLK_F4,  KEY_F4));
	KeyMap.push_back(SKeyMap(SDLK_F5,  KEY_F5));
	KeyMap.push_back(SKeyMap(SDLK_F6,  KEY_F6));
	KeyMap.push_back(SKeyMap(SDLK_F7,  KEY_F7));
	KeyMap.push_back(SKeyMap(SDLK_F8,  KEY_F8));
	KeyMap.push_back(SKeyMap(SDLK_F9,  KEY_F9));
	KeyMap.push_back(SKeyMap(SDLK_F10, KEY_F10));
	KeyMap.push_back(SKeyMap(SDLK_F11, KEY_F11));
	KeyMap.push_back(SKeyMap(SDLK_F12, KEY_F12));

	KeyMap.push_back(SKeyMap(SDLK_LSHIFT, KEY_LSHIFT));
	KeyMap.push_back(SKeyMap(SDLK_RSHIFT, KEY_RSHIFT));
	KeyMap.push_back(SKeyMap(SDLK_LCTRL,  KEY_LCONTROL));
	KeyMap.push_back(SKeyMap(SDLK_RCTRL,  KEY_RCONTROL));

	KeyMap.push_back(SKeyMap(SDLK_0, KEY_KEY_0));
	KeyMap.push_back(SKeyMap(SDLK_1, KEY_KEY_1));
	KeyMap.push_back(SKeyMap(SDLK_2, KEY_KEY_2));
	KeyMap.push_back(SKeyMap(SDLK_3, KEY_KEY_3));
	KeyMap.push_back(SKeyMap(SDLK_4, KEY_KEY_4));
	KeyMap.push_back(SKeyMap(SDLK_5, KEY_KEY_5));
	KeyMap.push_back(SKeyMap(SDLK_6, KEY_KEY_6));
	KeyMap.push_back(SKeyMap(SDLK_7, KEY_KEY_7));
	KeyMap.push_back(SKeyMap(SDLK_8, KEY_KEY_8));
	KeyMap.push_back(SKeyMap(SDLK_9, KEY_KEY_9));

	//KeyMap.push_back(SKeyMap(XK_colon, 0)); //?
	//KeyMap.push_back(SKeyMap(XK_semicolon, 0)); //?
	//KeyMap.push_back(SKeyMap(XK_less, 0)); //?
	//KeyMap.push_back(SKeyMap(XK_equal, 0)); //?
	//KeyMap.push_back(SKeyMap(XK_greater, 0)); //?
	//KeyMap.push_back(SKeyMap(XK_question, 0)); //?
	//KeyMap.push_back(SKeyMap(XK_at, 0)); //?

	KeyMap.push_back(SKeyMap(SDLK_a, KEY_KEY_A));
	KeyMap.push_back(SKeyMap(SDLK_b, KEY_KEY_B));
	KeyMap.push_back(SKeyMap(SDLK_c, KEY_KEY_C));
	KeyMap.push_back(SKeyMap(SDLK_d, KEY_KEY_D));
	KeyMap.push_back(SKeyMap(SDLK_e, KEY_KEY_E));
	KeyMap.push_back(SKeyMap(SDLK_f, KEY_KEY_F));
	KeyMap.push_back(SKeyMap(SDLK_g, KEY_KEY_G));
	KeyMap.push_back(SKeyMap(SDLK_h, KEY_KEY_H));
	KeyMap.push_back(SKeyMap(SDLK_i, KEY_KEY_I));
	KeyMap.push_back(SKeyMap(SDLK_j, KEY_KEY_J));
	KeyMap.push_back(SKeyMap(SDLK_k, KEY_KEY_K));
	KeyMap.push_back(SKeyMap(SDLK_l, KEY_KEY_L));
	KeyMap.push_back(SKeyMap(SDLK_m, KEY_KEY_M));
	KeyMap.push_back(SKeyMap(SDLK_n, KEY_KEY_N));
	KeyMap.push_back(SKeyMap(SDLK_o, KEY_KEY_O));
	KeyMap.push_back(SKeyMap(SDLK_p, KEY_KEY_P));
	KeyMap.push_back(SKeyMap(SDLK_q, KEY_KEY_Q));
	KeyMap.push_back(SKeyMap(SDLK_r, KEY_KEY_R));
	KeyMap.push_back(SKeyMap(SDLK_s, KEY_KEY_S));
	KeyMap.push_back(SKeyMap(SDLK_t, KEY_KEY_T));
	KeyMap.push_back(SKeyMap(SDLK_u, KEY_KEY_U));
	KeyMap.push_back(SKeyMap(SDLK_v, KEY_KEY_V));
	KeyMap.push_back(SKeyMap(SDLK_w, KEY_KEY_W));
	KeyMap.push_back(SKeyMap(SDLK_x, KEY_KEY_X));
	KeyMap.push_back(SKeyMap(SDLK_y, KEY_KEY_Y));
	KeyMap.push_back(SKeyMap(SDLK_z, KEY_KEY_Z));

	//KeyMap.push_back(SKeyMap(XK_bracketleft, 0)); //?
	//KeyMap.push_back(SKeyMap(XK_backslash, 0)); //?
	//KeyMap.push_back(SKeyMap(XK_bracketright, 0)); //?
	//KeyMap.push_back(SKeyMap(XK_asciicircum, 0)); //?
	//KeyMap.push_back(SKeyMap(XK_underscore, 0)); //?
	//KeyMap.push_back(SKeyMap(XK_grave, 0)); //?
	//KeyMap.push_back(SKeyMap(XK_quoteleft, 0)); //?

	KeyMap.sort();
}

IRRLICHT_API IrrlichtDevice* IRRCALLCONV createDeviceEx(const SIrrlichtCreationParameters& param)
{
	CIrrDeviceSDL* dev = new CIrrDeviceSDL(
		param.DriverType,
		param.WindowSize,
		param.Bits,
		param.Fullscreen,
		param.Stencilbuffer,
		param.Vsync,
		param.AntiAlias,
		param.EventReceiver,
		param.WindowId,
		param.SDK_version_do_not_use);

	if (dev && !dev->getVideoDriver() && param.DriverType != video::EDT_NULL)
	{
		dev->drop();
		dev = 0;
	}

	return dev;
}


} // end namespace irr

#endif // _IRR_USE_SDL_DEVICE_

