// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_IRRLICHT_DEVICE_H_INCLUDED__
#define __I_IRRLICHT_DEVICE_H_INCLUDED__

#include "IReferenceCounted.h"
#include "dimension2d.h"
#include "IVideoDriver.h"
#include "EDriverTypes.h"
#include "IEventReceiver.h"
#include "ICursorControl.h"
#include "IVideoModeList.h"
#include "ITimer.h"
#include "IOSOperator.h"

namespace irr
{
	class ILogger;
	class IEventReceiver;

	namespace io {
		class IFileSystem;
	} // end namespace io

	namespace gui {
		class IGUIEnvironment;
	} // end namespace gui

	namespace scene {
		class ISceneManager;
	} // end namespace scene

	//! The Irrlicht device. You can create it with createDevice() or createDeviceEx().
	/** This is the most important class of the Irrlicht Engine. You can access everything
	in the engine if you have a pointer to an instance of this class.
	There should be only one instance of this class at any time.
	*/
	class IrrlichtDevice : public virtual IReferenceCounted
	{
	public:

		//! Destructor
		virtual ~IrrlichtDevice() {}

		//! Runs the device.
		/** Also increments the virtual timer by calling
		ITimer::tick();. You can prevent this
		by calling ITimer::stop(); before and ITimer::start() after
		calling IrrlichtDevice::run(). Returns false if device wants
		to be deleted. Use it in this way:
		\code
		while(device->run())
		{
			// draw everything here
		}
		\endcode
		If you want the device to do nothing if the window is inactive
		(recommended), use the slightly enhanced code shown at isWindowActive().

		Note if you are running Irrlicht inside an external, custom
		created window: Calling Device->run() will cause Irrlicht to
		dispatch windows messages internally.
		If you are running Irrlicht in your own custom window, you can
		also simply use your own message loop using GetMessage,
		DispatchMessage and whatever and simply don't use this method.
		But note that Irrlicht will not be able to fetch user input
		then. See irr::SIrrlichtCreationParameters::WindowId for more
		informations and example code.
		*/
		virtual bool run() = 0;

		//! Cause the device to temporarily pause execution and let other processes run.
		/** This should bring down processor usage without major
		performance loss for Irrlicht */
		virtual void yield() = 0;

		//! Pause execution and let other processes to run for a specified amount of time.
		/** It may not wait the full given time, as sleep may be interrupted
		\param timeMs: Time to sleep for in milisecs.
		\param pauseTimer: If true, pauses the device timer while sleeping
		*/
		virtual void sleep(u32 timeMs, bool pauseTimer=false) = 0;

		//! Provides access to the video driver for drawing 3d and 2d geometry.
		/** \return Pointer the video driver. */
		virtual video::IVideoDriver* getVideoDriver() = 0;

		//! Provides access to the virtual file system.
		/** \return Pointer to the file system. */
		virtual io::IFileSystem* getFileSystem() = 0;

		//! Provides access to the 2d user interface environment.
		/** \return Pointer to the gui environment. */
		virtual gui::IGUIEnvironment* getGUIEnvironment() = 0;

		//! Provides access to the scene manager.
		/** \return Pointer to the scene manager. */
		virtual scene::ISceneManager* getSceneManager() = 0;

		//! Provides access to the cursor control.
		/** \return Pointer to the mouse cursor control interface. */
		virtual gui::ICursorControl* getCursorControl() = 0;

		//! Provides access to the message logger.
		/** \return Pointer to the logger. */
		virtual ILogger* getLogger() = 0;

		//! Gets a list with all video modes available.
		/** If you are confused now, because you think you have to
		create an Irrlicht Device with a video mode before being able
		to get the video mode list, let me tell you that there is no
		need to start up an Irrlicht Device with EDT_DIRECT3D8,
		EDT_OPENGL or EDT_SOFTWARE: For this (and for lots of other
		reasons) the null driver, EDT_NULL exists.
		\return Pointer to a list with all video modes supported
		by the gfx adapter. */
		virtual video::IVideoModeList* getVideoModeList() = 0;

		//! Provides access to the operation system operator object.
		/** The OS operator provides methods for
		getting system specific informations and doing system
		specific operations, such as exchanging data with the clipboard
		or reading the operation system version.
		\return Pointer to the OS operator. */
		virtual IOSOperator* getOSOperator() = 0;

		//! Provides access to the engine's timer.
		/** The system time can be retrieved by it as
		well as the virtual time, which also can be manipulated.
		\return Pointer to the ITimer object. */
		virtual ITimer* getTimer() = 0;

		//! Sets the caption of the window.
		/** \param text: New text of the window caption. */
		virtual void setWindowCaption(const wchar_t* text) = 0;

		//! Returns if the window is active.
		/** If the window is inactive,
		nothing needs to be drawn. So if you don't want to draw anything
		when the window is inactive, create your drawing loop this way:
		\code
		while(device->run())
		{
			if (device->isWindowActive())
			{
				// draw everything here
			}
			else
				device->yield();
		}
		\endcode
		\return True if window is active. */
		virtual bool isWindowActive() const = 0;

		//! Notifies the device that it should close itself.
		/** IrrlichtDevice::run() will always return false after closeDevice() was called. */
		virtual void closeDevice() = 0;

		//! Get the version of the engine.
		/** The returned string
		will look like this: "1.2.3" or this: "1.2".
		\return String which contains the version. */
		virtual const c8* getVersion() const = 0;

		//! Sets a new event receiver to receive events.
		/** \param receiver New receiver to be used. */
		virtual void setEventReceiver(IEventReceiver* receiver) = 0;

		//! Provides access to the current event receiver.
		/** \return Pointer to the current event receiver. Returns 0 if there is none. */
		virtual IEventReceiver* getEventReceiver() = 0;

		//! Sends a user created event to the engine.
		/** Is is usually not necessary to use this. However, if you
		are using an own input library for example for doing joystick
		input, you can use this to post key or mouse input events to
		the engine. Internally, this method only delegates the events
		further to the scene manager and the GUI environment. */
		virtual bool postEventFromUser(const SEvent& event) = 0;

		//! Sets the input receiving scene manager.
		/** If set to null, the main scene manager (returned by
		GetSceneManager()) will receive the input
		\param sceneManager New scene manager to be used. */
		virtual void setInputReceivingSceneManager(scene::ISceneManager* sceneManager) = 0;

		//! Sets if the window should be resizeable in windowed mode.
		/** The default is false. This method only works in windowed
		mode.
		\param resize Flag whether the window should be resizeable. */
		virtual void setResizeAble(bool resize=false) = 0;
	};

} // end namespace irr

#endif

