// Copyright (C) 2005-2008 Etienne Petitjean
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_IRR_DEVICE_MACOSX_H_INCLUDED__
#define __C_IRR_DEVICE_MACOSX_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_USE_OSX_DEVICE_

#include "CIrrDeviceStub.h"
#include "IrrlichtDevice.h"
#include "IImagePresenter.h"
#include "IGUIEnvironment.h"
#include "ICursorControl.h"

#include <OpenGL/OpenGL.h>
#include <map>

namespace irr
{
	class CIrrDeviceMacOSX : public CIrrDeviceStub, video::IImagePresenter
	{
	public:

		//! constructor
		CIrrDeviceMacOSX(video::E_DRIVER_TYPE driverType,
			const core::dimension2d<s32>& windowSize,
			u32 bits, bool fullscreen,
			bool sbuffer, bool vsync,
			bool antiAlias, IEventReceiver* receiver,
			const char* version);

		//! destructor
		virtual ~CIrrDeviceMacOSX();

		//! runs the device. Returns false if device wants to be deleted
		virtual bool run();

		//! Cause the device to temporarily pause execution and let other processes to run
		// This should bring down processor usage without major performance loss for Irrlicht
		virtual void yield();

		//! Pause execution and let other processes to run for a specified amount of time.
		virtual void sleep(u32 timeMs, bool pauseTimer);

		//! sets the caption of the window
		virtual void setWindowCaption(const wchar_t* text);

		//! returns if window is active. if not, nothing need to be drawn
		virtual bool isWindowActive() const;

		//! presents a surface in the client area
		virtual void present(video::IImage* surface, s32 windowId = 0, core::rect<s32>* src=0, core::rect<s32>* src=0 );

		//! notifies the device that it should close itself
		virtual void closeDevice();

		//! Sets if the window should be resizeable in windowed mode.
		virtual void setResizeAble(bool resize);

		void flush();
		void setMouseLocation(int x,int y);
		void setResize(int width,int height);
		void setCursorVisible(bool visible);

	private:

		//! create the driver
		void createDriver(video::E_DRIVER_TYPE driverType,
			const core::dimension2d<s32>& windowSize, u32 bits, bool fullscreen,
			bool stencilbuffer, bool vsync, bool antiAlias);

		//! Implementation of the macos x cursor control
		class CCursorControl : public gui::ICursorControl
		{
		public:

			CCursorControl(const core::dimension2d<s32>& wsize, CIrrDeviceMacOSX *device) : WindowSize(wsize), IsVisible(true), InvWindowSize(0.0f, 0.0f), _device(device), UseReferenceRect(false)
			{
				CursorPos.X = CursorPos.Y = 0;
				if (WindowSize.Width!=0) InvWindowSize.Width = 1.0f / WindowSize.Width;
				if (WindowSize.Height!=0) InvWindowSize.Height = 1.0f / WindowSize.Height;
			}

			//! Changes the visible state of the mouse cursor.
			virtual void setVisible(bool visible)
			{
				IsVisible = visible;
				_device->setCursorVisible(visible);
			}

			//! Returns if the cursor is currently visible.
			virtual bool isVisible() const
			{
				return IsVisible;
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(const core::position2d<f32> &pos)
			{
				setPosition(pos.X, pos.Y);
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(f32 x, f32 y)
			{
				setPosition((s32)(x*WindowSize.Width), (s32)(y*WindowSize.Height));
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(const core::position2d<s32> &pos)
			{
				if (CursorPos.X != pos.X || CursorPos.Y != pos.Y)
					setPosition(pos.X, pos.Y);
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(s32 x, s32 y)
			{
				if (UseReferenceRect)
				{
					_device->setMouseLocation(ReferenceRect.UpperLeftCorner.X + x, ReferenceRect.UpperLeftCorner.Y + y);
				}
				else
				{
					_device->setMouseLocation(x,y);
				}
			}

			//! Returns the current position of the mouse cursor.
			virtual core::position2d<s32> getPosition()
			{
				return CursorPos;
			}

			//! Returns the current position of the mouse cursor.
			virtual core::position2d<f32> getRelativePosition()
			{
				if (!UseReferenceRect)
				{
					return core::position2d<f32>(CursorPos.X * InvWindowSize.Width,
						CursorPos.Y * InvWindowSize.Height);
				}

				return core::position2d<f32>(CursorPos.X / (f32)ReferenceRect.getWidth(),
						CursorPos.Y / (f32)ReferenceRect.getHeight());
			}

			//! Sets an absolute reference rect for calculating the cursor position.
			virtual void setReferenceRect(core::rect<s32>* rect=0)
			{
				if (rect)
				{
					ReferenceRect = *rect;
					UseReferenceRect = true;

					// prevent division through zero and uneven sizes

					if (!ReferenceRect.getHeight() || ReferenceRect.getHeight()%2)
						ReferenceRect.LowerRightCorner.Y += 1;

					if (!ReferenceRect.getWidth() || ReferenceRect.getWidth()%2)
						ReferenceRect.LowerRightCorner.X += 1;
				}
				else
					UseReferenceRect = false;
			}

			//! Updates the internal cursor position
			void updateInternalCursorPosition(int x,int y)
			{
				CursorPos.X = x;
				CursorPos.Y = y;
			}

		private:

			core::position2d<s32> CursorPos;
			core::dimension2d<s32> WindowSize;
			core::dimension2d<float> InvWindowSize;
			CIrrDeviceMacOSX	*_device;
			bool IsVisible;
			bool UseReferenceRect;
			core::rect<s32> ReferenceRect;
		};

		bool createWindow(const irr::core::dimension2d<irr::s32>& windowSize, irr::u32 bits, bool fullscreen, bool vsync, bool stencilBuffer);
		void initKeycodes();
		void storeMouseLocation();
		void postMouseEvent(void *event,irr::SEvent &ievent);
		void postKeyEvent(void *event,irr::SEvent &ievent,bool pressed);

		video::E_DRIVER_TYPE DriverType;
		bool stencilbuffer;

		void			*_window;
		CGLContextObj		_cglcontext;
		void			*_oglcontext;
		int			_width;
		int			_height;
		std::map<int,int>	_keycodes;
		int			_screenWidth;
		int			_screenHeight;
		bool			_active;
	};


} // end namespace irr

#endif // _IRR_USE_OSX_DEVICE_
#endif // __C_IRR_DEVICE_MACOSX_H_INCLUDED__

