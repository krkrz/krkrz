// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_IRR_DEVICE_WIN32_H_INCLUDED__
#define __C_IRR_DEVICE_WIN32_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_USE_WINDOWS_DEVICE_

#include "CIrrDeviceStub.h"
#include "IrrlichtDevice.h"
#include "IImagePresenter.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace irr
{
	class CIrrDeviceWin32 : public CIrrDeviceStub, video::IImagePresenter
	{
	public:

		//! constructor
		CIrrDeviceWin32(video::E_DRIVER_TYPE deviceType, 
			core::dimension2d<s32> windowSize, u32 bits,
			bool fullscreen, bool stencilbuffer, bool vsync, 
			bool antiAlias, bool highPrecisionFPU,
			IEventReceiver* receiver,
			HWND window,
			const char* version);

		//! destructor
		virtual ~CIrrDeviceWin32();

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
		virtual void present(video::IImage* surface, s32 windowId = 0, core::rect<s32>* src=0, core::rect<s32>* dest=0 );

		//! notifies the device that it should close itself
		virtual void closeDevice();

		//! \return Returns a pointer to a list with all video modes
		//! supported by the gfx adapter.
		video::IVideoModeList* getVideoModeList();

		//! Notifies the device, that it has been resized
		void OnResized();

		//! Sets if the window should be resizeable in windowed mode.
		virtual void setResizeAble(bool resize=false);

		//! Implementation of the win32 cursor control
		class CCursorControl : public gui::ICursorControl
		{
		public:

			CCursorControl(const core::dimension2d<s32>& wsize, HWND hwnd, bool fullscreen)
				: WindowSize(wsize), InvWindowSize(0.0f, 0.0f), IsVisible(true),
					HWnd(hwnd), BorderX(0), BorderY(0), UseReferenceRect(false)
			{
				if (WindowSize.Width!=0)
					InvWindowSize.Width = 1.0f / WindowSize.Width;

				if (WindowSize.Height!=0)
					InvWindowSize.Height = 1.0f / WindowSize.Height;

				if (!fullscreen)
				{
					BorderX = GetSystemMetrics(SM_CXDLGFRAME);
					BorderY = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYDLGFRAME);
				}
			}

			//! Changes the visible state of the mouse cursor.
			virtual void setVisible(bool visible)
			{
				IsVisible = visible;
			}

			//! Returns if the cursor is currently visible.
			virtual bool isVisible() const
			{
				_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
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
				if (!UseReferenceRect)
					setPosition((s32)(x*WindowSize.Width), (s32)(y*WindowSize.Height));
				else
					setPosition((s32)(x*ReferenceRect.getWidth()), (s32)(y*ReferenceRect.getHeight()));
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(const core::position2d<s32> &pos)
			{
				setPosition(pos.X, pos.Y);
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(s32 x, s32 y)
			{
				RECT rect;

				if (UseReferenceRect)
				{
					SetCursorPos(ReferenceRect.UpperLeftCorner.X + x, 
								 ReferenceRect.UpperLeftCorner.Y + y);
				}
				else
				{
					if (GetWindowRect(HWnd, &rect))
						SetCursorPos(x + rect.left + BorderX, y + rect.top + BorderY);
				}

				CursorPos.X = x;
				CursorPos.Y = y;
			}

			//! Returns the current position of the mouse cursor.
			virtual core::position2d<s32> getPosition()
			{
				updateInternalCursorPosition();
				return CursorPos;
			}

			//! Returns the current position of the mouse cursor.
			virtual core::position2d<f32> getRelativePosition()
			{
				updateInternalCursorPosition();

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

		private:

			//! Updates the internal cursor position
			void updateInternalCursorPosition()
			{
				POINT p;
				GetCursorPos(&p);
				RECT rect;

				if (UseReferenceRect)
				{
					CursorPos.X = p.x - ReferenceRect.UpperLeftCorner.X;
					CursorPos.Y = p.y - ReferenceRect.UpperLeftCorner.Y;
				}
				else
				{
					if (GetWindowRect(HWnd, &rect))
					{
						CursorPos.X = p.x-rect.left-BorderX;
						CursorPos.Y = p.y-rect.top-BorderY;
					}
					else
					{
						// window seems not to be existent, so set cursor to
						// a negative value
						CursorPos.X = -1;
						CursorPos.Y = -1;
					}
				}
			}

			core::position2d<s32> CursorPos;
			core::dimension2d<s32> WindowSize;
			core::dimension2d<f32> InvWindowSize;
			bool IsVisible;
			HWND HWnd;

			s32 BorderX, BorderY;
			bool UseReferenceRect;
			core::rect<s32> ReferenceRect;
		};


		//! returns the win32 cursor control
		CCursorControl* getWin32CursorControl();

	private:

		//! create the driver
		void createDriver(video::E_DRIVER_TYPE driverType,
			const core::dimension2d<s32>& windowSize, u32 bits, bool fullscreen,
			bool stencilbuffer, bool vsync, bool antiAlias, bool highPrecisionFPU);

		//! switchs to fullscreen
		bool switchToFullScreen(s32 width, s32 height, s32 bits);

		void getWindowsVersion(core::stringc& version);

		void resizeIfNecessary();

		HWND HWnd;

		bool ChangedToFullScreen;
		bool FullScreen;
		bool IsNonNTWindows;
		bool Resized;
		bool ExternalWindow;
		CCursorControl* Win32CursorControl;
	};


} // end namespace irr

#endif
#endif

