// Copyright (C) 2005-2008 Etienne Petitjean
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "IrrCompileConfig.h"

#ifdef _IRR_USE_OSX_DEVICE_

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>

#include "CIrrDeviceMacOSX.h"
#include "IEventReceiver.h"
#include "irrList.h"
#include "os.h"
#include "CTimer.h"
#include "irrString.h"
#include "Keycodes.h"
#include <stdio.h>
#include <sys/utsname.h>
#include "COSOperator.h"
#include "irrlicht.h"
#import <wchar.h>
#import <time.h>
#import "AppDelegate.h"

namespace irr
{
	namespace video
	{
		IVideoDriver* createOpenGLDriver(const core::dimension2d<s32>& screenSize, CIrrDeviceMacOSX *device, bool fullscreen, bool stencilBuffer, io::IFileSystem* io, bool vsync, bool antiAlias);
	}
} // end namespace irr

static bool firstLaunch = true;

namespace irr
{
//! constructor
CIrrDeviceMacOSX::CIrrDeviceMacOSX(video::E_DRIVER_TYPE driverType,
				const core::dimension2d<s32>& windowSize,
				u32 bits, bool fullscreen,
				bool sbuffer, bool vsync,
				bool antiAlias, IEventReceiver* receiver,
				const char* version)
 : CIrrDeviceStub(version, receiver), DriverType(driverType), stencilbuffer(sbuffer), _window(NULL), _active(true), _oglcontext(NULL), _cglcontext(NULL)
{
	struct utsname name;
	NSString	*path;

	#ifdef _DEBUG
	setDebugName("CIrrDeviceMacOSX");
	#endif

	if (firstLaunch)
	{
		firstLaunch = false;

		[[NSAutoreleasePool alloc] init];
		[NSApplication sharedApplication];
		[NSApp setDelegate:[[[AppDelegate alloc] initWithDevice:this] autorelease]];
		[NSBundle loadNibNamed:@"MainMenu" owner:[NSApp delegate]];
		[NSApp finishLaunching];

		path = [[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent];
		chdir([path cString]);
	}

	uname(&name);
	Operator = new COSOperator(name.version);
	os::Printer::log(name.version,ELL_INFORMATION);

	initKeycodes();
	if (driverType != video::EDT_NULL) createWindow(windowSize,bits,fullscreen,vsync,stencilbuffer);
	CursorControl = new CCursorControl(windowSize, this);
	createDriver(driverType,windowSize,bits,fullscreen,stencilbuffer,vsync,antiAlias);
	createGUIAndScene();
}

CIrrDeviceMacOSX::~CIrrDeviceMacOSX()
{
	closeDevice();
}

void CIrrDeviceMacOSX::closeDevice()
{
	if (_window != NULL)
	{
		[_window setIsVisible:FALSE];

		if (_oglcontext != NULL)
		{
			[_oglcontext clearDrawable];
			[_oglcontext release];
			_oglcontext = NULL;
		}

		[_window setReleasedWhenClosed:TRUE];
		[_window release];
		_window = NULL;
	}
	else
	{
		if (_cglcontext != NULL)
		{
			CGLSetCurrentContext(NULL);
			CGLClearDrawable(_cglcontext);
			CGLDestroyContext(_cglcontext);
		}
	}

	_active = FALSE;
	_cglcontext = NULL;
}

bool CIrrDeviceMacOSX::createWindow(const irr::core::dimension2d<irr::s32>& windowSize, irr::u32 bits, bool fullscreen, bool vsync, bool stencilBuffer)
{
	int				index;
	CGDisplayErr			error;
	bool				result;
	NSOpenGLPixelFormat		*format;
	CGDirectDisplayID		display;
	CGLPixelFormatObj		pixelFormat;
	CGRect				displayRect;
	CGLPixelFormatAttribute		fullattribs[32];
	NSOpenGLPixelFormatAttribute	windowattribs[32];
	CFDictionaryRef			displaymode,olddisplaymode;
	long				numPixelFormats,newSwapInterval;

	result = false;
	display = CGMainDisplayID();
	_screenWidth = (int) CGDisplayPixelsWide(display);
	_screenHeight = (int) CGDisplayPixelsHigh(display);

	if (!fullscreen)
	{
		_window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,windowSize.Width,windowSize.Height) styleMask:NSTitledWindowMask+NSClosableWindowMask+NSResizableWindowMask backing:NSBackingStoreBuffered defer:FALSE];
		if (_window != NULL)
		{
			index = 0;
			windowattribs[index++] = NSOpenGLPFANoRecovery;
			windowattribs[index++] = NSOpenGLPFADoubleBuffer;
			windowattribs[index++] = NSOpenGLPFAAccelerated;
			windowattribs[index++] = NSOpenGLPFADepthSize;
			windowattribs[index++] = (NSOpenGLPixelFormatAttribute)16;
			windowattribs[index++] = NSOpenGLPFAColorSize;
			windowattribs[index++] = (NSOpenGLPixelFormatAttribute)bits;

			if (stencilBuffer)
			{
				windowattribs[index++] = NSOpenGLPFAStencilSize;
				windowattribs[index++] = (NSOpenGLPixelFormatAttribute)1;
			}

			windowattribs[index++] = (NSOpenGLPixelFormatAttribute)NULL;

			format = [[NSOpenGLPixelFormat alloc] initWithAttributes:windowattribs];
			if (format != NULL)
			{
				_oglcontext = [[NSOpenGLContext alloc] initWithFormat:format shareContext:NULL];
				[format release];
			}

			if (_oglcontext != NULL)
			{
				[_window center];
				[_window setDelegate:[NSApp delegate]];
				[_oglcontext setView:[_window contentView]];
				[_window setAcceptsMouseMovedEvents:TRUE];
				[_window setIsVisible:TRUE];
				[_window makeKeyAndOrderFront:nil];

				_cglcontext = (CGLContextObj) [_oglcontext CGLContextObj];
				_width = windowSize.Width;
				_height = windowSize.Height;
				result = true;
			}
		}
	}
	else
	{
		displaymode = CGDisplayBestModeForParameters(display,bits,windowSize.Width,windowSize.Height,NULL);
		if (displaymode != NULL)
		{
			olddisplaymode = CGDisplayCurrentMode(display);
			error = CGCaptureAllDisplays();
			if (error == CGDisplayNoErr)
			{
				error = CGDisplaySwitchToMode(display,displaymode);
				if (error == CGDisplayNoErr)
				{
					pixelFormat = NULL;
					numPixelFormats = 0;

					index = 0;
					fullattribs[index++] = kCGLPFAFullScreen;
					fullattribs[index++] = kCGLPFADisplayMask;
					fullattribs[index++] = (CGLPixelFormatAttribute)CGDisplayIDToOpenGLDisplayMask(display);
					fullattribs[index++] = kCGLPFADoubleBuffer;
					fullattribs[index++] = kCGLPFAAccelerated;
					fullattribs[index++] = kCGLPFADepthSize;
					fullattribs[index++] = (CGLPixelFormatAttribute)16;
					fullattribs[index++] = kCGLPFAColorSize;
					fullattribs[index++] = (CGLPixelFormatAttribute)bits;

					if (stencilBuffer)
					{
						fullattribs[index++] = kCGLPFAStencilSize;
						fullattribs[index++] = (CGLPixelFormatAttribute)1;
					}

					fullattribs[index++] = (CGLPixelFormatAttribute)NULL;
					CGLChoosePixelFormat(fullattribs,&pixelFormat,&numPixelFormats);

					if (pixelFormat != NULL)
					{
						CGLCreateContext(pixelFormat,NULL,&_cglcontext);
						CGLDestroyPixelFormat(pixelFormat);
					}

					if (_cglcontext != NULL)
					{
						CGLSetFullScreen(_cglcontext);
						displayRect = CGDisplayBounds(display);
						_width = (int)displayRect.size.width;
						_height = (int)displayRect.size.height;
						result = true;
					}
				}
			}
		}
	}

	if (result)
	{
		CGLSetCurrentContext(_cglcontext);
		newSwapInterval = (vsync) ? 1 : 0;
		CGLSetParameter(_cglcontext,kCGLCPSwapInterval,&newSwapInterval);
		glViewport(0,0,_width,_height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}

	return (result);
}

void CIrrDeviceMacOSX::setResize(int width,int height)
{
	_width = width;
	_height = height;
	[_oglcontext update];
	getVideoDriver()->OnResize(core::dimension2d<s32>(width, height));
}

void CIrrDeviceMacOSX::createDriver(video::E_DRIVER_TYPE driverType,const core::dimension2d<s32>& windowSize,u32 bits,bool fullscreen,bool stencilbuffer, bool vsync, bool antiAlias)
{
	switch (driverType)
	{
		case video::EDT_SOFTWARE:
		#ifdef _IRR_COMPILE_WITH_SOFTWARE_
			VideoDriver = video::createSoftwareDriver(windowSize, fullscreen, FileSystem, this);
		#else
			os::Printer::log("No Software driver support compiled in.", ELL_ERROR);
		#endif
			break;

		case video::EDT_BURNINGSVIDEO:
		#ifdef _IRR_COMPILE_WITH_BURNINGSVIDEO_
			VideoDriver = video::createSoftwareDriver2(windowSize, fullscreen, FileSystem, this);
		#else
			os::Printer::log("Burning's video driver was not compiled in.", ELL_ERROR);
		#endif
			break;

		case video::EDT_OPENGL:
		#ifdef _IRR_COMPILE_WITH_OPENGL_
			VideoDriver = video::createOpenGLDriver(windowSize, this, fullscreen, stencilbuffer, FileSystem, vsync, antiAlias);
		#else
			os::Printer::log("No OpenGL support compiled in.", ELL_ERROR);
		#endif
			break;

		case video::EDT_DIRECT3D8:
		case video::EDT_DIRECT3D9:
			os::Printer::log("This driver is not available in OSX. Try OpenGL or Software renderer.", ELL_ERROR);
			break;

		case video::EDT_NULL:
			VideoDriver = video::createNullDriver(FileSystem, windowSize);
			break;

		default:
			os::Printer::log("Unable to create video driver of unknown type.", ELL_ERROR);
			break;
	}
}

void CIrrDeviceMacOSX::flush()
{
	if (_cglcontext != NULL)
	{
		glFinish();
		CGLFlushDrawable(_cglcontext);
	}
}

bool CIrrDeviceMacOSX::run()
{
	NSEvent		*event;
	irr::SEvent	ievent;

	os::Timer::tick();
	storeMouseLocation();

	event = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
	if (event != nil)
	{
		bzero(&ievent,sizeof(ievent));

		switch([event type])
		{
			case NSKeyDown:
				postKeyEvent(event,ievent,true);
				break;

			case NSKeyUp:
				postKeyEvent(event,ievent,false);
				break;

			case NSLeftMouseDown:
				ievent.EventType = irr::EET_MOUSE_INPUT_EVENT;
				ievent.MouseInput.Event = irr::EMIE_LMOUSE_PRESSED_DOWN;
				postMouseEvent(event,ievent);
				break;

			case NSLeftMouseUp:
				ievent.EventType = irr::EET_MOUSE_INPUT_EVENT;
				ievent.MouseInput.Event = irr::EMIE_LMOUSE_LEFT_UP;
				postMouseEvent(event,ievent);
				break;

			case NSMouseMoved:
			case NSLeftMouseDragged:
			case NSRightMouseDragged:
				ievent.EventType = irr::EET_MOUSE_INPUT_EVENT;
				ievent.MouseInput.Event = irr::EMIE_MOUSE_MOVED;
				postMouseEvent(event,ievent);
				break;

			case NSRightMouseDown:
				ievent.EventType = irr::EET_MOUSE_INPUT_EVENT;
				ievent.MouseInput.Event = irr::EMIE_RMOUSE_PRESSED_DOWN;
				postMouseEvent(event,ievent);
				break;

			case NSRightMouseUp:
				ievent.EventType = irr::EET_MOUSE_INPUT_EVENT;
				ievent.MouseInput.Event = irr::EMIE_RMOUSE_LEFT_UP;
				postMouseEvent(event,ievent);
				break;

			case NSScrollWheel:
				ievent.EventType = irr::EET_MOUSE_INPUT_EVENT;
				ievent.MouseInput.Event = irr::EMIE_MOUSE_WHEEL;
				ievent.MouseInput.Wheel = [event deltaY];
				if (ievent.MouseInput.Wheel < 1.0f) ievent.MouseInput.Wheel *= 10.0f;
				else ievent.MouseInput.Wheel *= 5.0f;
				postMouseEvent(event,ievent);
				break;

			default:
				[NSApp sendEvent:event];
				break;
		}
	}

	return (![[NSApp delegate] isQuit] && _active);
}

//! Pause the current process for the minimum time allowed only to allow other processes to execute
void CIrrDeviceMacOSX::yield()
{
	// TODO: Does this work or maybe is there a better way?
	struct timespec ts = {0,0};
	nanosleep(&ts, NULL);
}

//! Pause execution and let other processes to run for a specified amount of time.
void CIrrDeviceMacOSX::sleep(u32 timeMs, bool pauseTimer=false)
{
	// TODO: Does this work or maybe is there a better way?

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

void CIrrDeviceMacOSX::present(video::IImage* image, s32 windowId, core::rect<s32>* src )
{
}

void CIrrDeviceMacOSX::setWindowCaption(const wchar_t* text)
{
	size_t	size;
	char	title[1024];

	if (_window != NULL)
	{
		size = wcstombs(title,text,1024);
		if (size == 1024) title[1023] = 0;
		[_window setTitle:[NSString stringWithCString:title length:size]];
	}
}

bool CIrrDeviceMacOSX::isWindowActive() const
{
	return (_active);
}

void CIrrDeviceMacOSX::postKeyEvent(void *event,irr::SEvent &ievent,bool pressed)
{
	NSString				*str;
	std::map<int,int>::const_iterator	iter;
	unsigned int				result,c,mkey,mchar;
	const unsigned char			*cStr;
	BOOL					skipCommand;

	str = [event characters];
	if (str != nil && [str length] > 0)
	{
		mkey = mchar = 0;
		skipCommand = false;
		c = [str characterAtIndex:0];

		iter = _keycodes.find(c);
		if (iter != _keycodes.end()) mkey = (*iter).second;
		else
		{
			cStr = (unsigned char *)[str cStringUsingEncoding:NSWindowsCP1252StringEncoding];
			if (cStr != NULL && strlen((char*)cStr) > 0)
			{
				mchar = cStr[0];
				mkey = toupper(mchar);
				if ([event modifierFlags] & NSCommandKeyMask)
				{
					if (mkey == 'C' || mkey == 'V' || mkey == 'X')
					{
						mchar = 0;
						skipCommand = true;
					}
				}
			}
		}

		ievent.EventType = irr::EET_KEY_INPUT_EVENT;
		ievent.KeyInput.Key = (irr::EKEY_CODE)mkey;
		ievent.KeyInput.PressedDown = pressed;
		ievent.KeyInput.Shift = ([event modifierFlags] & NSShiftKeyMask) != 0;
		ievent.KeyInput.Control = ([event modifierFlags] & NSControlKeyMask) != 0;
		ievent.KeyInput.Char = (irr::EKEY_CODE)mchar;

		if (skipCommand)
			ievent.KeyInput.Control = true;
		else if ([event modifierFlags] & NSCommandKeyMask)
			[NSApp sendEvent:(NSEvent *)event];

		postEventFromUser(ievent);
	}
}

void CIrrDeviceMacOSX::postMouseEvent(void *event,irr::SEvent &ievent)
{
	BOOL	post = true;

	if (_window != NULL)
	{
		ievent.MouseInput.X = (int)[event locationInWindow].x;
		ievent.MouseInput.Y = _height - (int)[event locationInWindow].y;
		if (ievent.MouseInput.Y < 0) post = false;
	}
	else
	{
		ievent.MouseInput.X = (int)[NSEvent mouseLocation].x;
		ievent.MouseInput.Y = _height - (int)[NSEvent mouseLocation].y;
	}

	if (post) postEventFromUser(ievent);
	[NSApp sendEvent:(NSEvent *)event];
}

void CIrrDeviceMacOSX::storeMouseLocation()
{
	NSPoint	p;
	int	x,y;

	p = [NSEvent mouseLocation];

	if (_window != NULL)
	{
		p = [_window convertScreenToBase:p];
		x = (int)p.x;
		y = _height - (int)p.y;
	}
	else
	{
		x = (int)p.x;
		y = _screenHeight - (int)p.y;
	}

	((CCursorControl *)CursorControl)->updateInternalCursorPosition(x,y);
}

void CIrrDeviceMacOSX::setMouseLocation(int x,int y)
{
	NSPoint	p;
	CGPoint	c;

	if (_window != NULL)
	{
		p.x = (float) x;
		p.y = (float) (_height - y);
		p = [_window convertBaseToScreen:p];
		p.y = _screenHeight - p.y;
	}
	else
	{
		p.x = (float) x;
		p.y = (float) (_height - y);
	}

	c.x = p.x;
	c.y = p.y;
	CGSetLocalEventsSuppressionInterval(0);
	CGWarpMouseCursorPosition(c);
}

void CIrrDeviceMacOSX::setCursorVisible(bool visible)
{
	CGDirectDisplayID		display;

	display = CGMainDisplayID();
	if (visible) CGDisplayShowCursor(display);
	else CGDisplayHideCursor(display);
}

void CIrrDeviceMacOSX::initKeycodes()
{
	_keycodes[NSUpArrowFunctionKey] = irr::KEY_UP;
	_keycodes[NSDownArrowFunctionKey] = irr::KEY_DOWN;
	_keycodes[NSLeftArrowFunctionKey] = irr::KEY_LEFT;
	_keycodes[NSRightArrowFunctionKey] = irr::KEY_RIGHT;
	_keycodes[NSF1FunctionKey] = irr::KEY_F1;
	_keycodes[NSF2FunctionKey] = irr::KEY_F2;
	_keycodes[NSF3FunctionKey] = irr::KEY_F3;
	_keycodes[NSF4FunctionKey] = irr::KEY_F4;
	_keycodes[NSF5FunctionKey] = irr::KEY_F5;
	_keycodes[NSF6FunctionKey] = irr::KEY_F6;
	_keycodes[NSF7FunctionKey] = irr::KEY_F7;
	_keycodes[NSF8FunctionKey] = irr::KEY_F8;
	_keycodes[NSF9FunctionKey] = irr::KEY_F9;
	_keycodes[NSF10FunctionKey] = irr::KEY_F10;
	_keycodes[NSF11FunctionKey] = irr::KEY_F11;
	_keycodes[NSF12FunctionKey] = irr::KEY_F12;
	_keycodes[NSF13FunctionKey] = irr::KEY_F13;
	_keycodes[NSF14FunctionKey] = irr::KEY_F14;
	_keycodes[NSF15FunctionKey] = irr::KEY_F15;
	_keycodes[NSF16FunctionKey] = irr::KEY_F16;
	_keycodes[NSHomeFunctionKey] = irr::KEY_HOME;
	_keycodes[NSEndFunctionKey] = irr::KEY_END;
	_keycodes[NSInsertFunctionKey] = irr::KEY_INSERT;
	_keycodes[NSDeleteFunctionKey] = irr::KEY_DELETE;
	_keycodes[NSHelpFunctionKey] = irr::KEY_HELP;
	_keycodes[NSSelectFunctionKey] = irr::KEY_SELECT;
	_keycodes[NSPrintFunctionKey] = irr::KEY_PRINT;
	_keycodes[NSExecuteFunctionKey] = irr::KEY_EXECUT;
	_keycodes[NSPrintScreenFunctionKey] = irr::KEY_SNAPSHOT;
	_keycodes[NSPauseFunctionKey] = irr::KEY_PAUSE;
	_keycodes[NSScrollLockFunctionKey] = irr::KEY_SCROLL;
	_keycodes[0x7F] = irr::KEY_BACK;
	_keycodes[0x09] = irr::KEY_TAB;
	_keycodes[0x0D] = irr::KEY_RETURN;
	_keycodes[0x03] = irr::KEY_RETURN;
	_keycodes[0x1B] = irr::KEY_ESCAPE;
}

//! Sets if the window should be resizeable in windowed mode.

void CIrrDeviceMacOSX::setResizeAble(bool resize)
{
	// todo: implement resize
}


IRRLICHT_API IrrlichtDevice* IRRCALLCONV createDeviceEx(const SIrrlichtCreationParameters& param)
{
	CIrrDeviceMacOSX* dev = new CIrrDeviceMacOSX(
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

}

#endif // _IRR_USE_OSX_DEVICE_

