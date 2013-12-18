// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CIrrDeviceStub.h"
#include "ISceneManager.h"
#include "IEventReceiver.h"
#include "IFileSystem.h"
#include "IGUIEnvironment.h"
#include "os.h"
#include "IrrCompileConfig.h"
#include "CTimer.h"
#include "CLogger.h"
#include "irrString.h"

namespace irr
{

//! constructor
CIrrDeviceStub::CIrrDeviceStub(const char* version, IEventReceiver* recv)
: IrrlichtDevice(), VideoDriver(0), GUIEnvironment(0), SceneManager(0), 
	Timer(0), CursorControl(0), UserReceiver(recv), Logger(0), Operator(0),
	FileSystem(io::createFileSystem()), InputReceivingSceneManager(0)
{
	Timer = new CTimer();
	if (os::Printer::Logger)
	{
		os::Printer::Logger->grab();
		Logger = (CLogger*)os::Printer::Logger;
		Logger->setReceiver(UserReceiver);
	}
	else
	{
		Logger = new CLogger(UserReceiver);
		os::Printer::Logger = Logger;
	}

	os::Printer::Logger = Logger;

	core::stringc s = "Irrlicht Engine version ";
	s.append(getVersion());
	os::Printer::log(s.c_str(), ELL_INFORMATION);

	checkVersion(version);
}


CIrrDeviceStub::~CIrrDeviceStub()
{
	FileSystem->drop();

	if (GUIEnvironment)
		GUIEnvironment->drop();

	if (VideoDriver)
		VideoDriver->drop();

	if (SceneManager)
		SceneManager->drop();

	if (InputReceivingSceneManager)
		InputReceivingSceneManager->drop();

	if (CursorControl)
		CursorControl->drop();

	if (Operator)
		Operator->drop();

	CursorControl = 0;

	Timer->drop();

	if (Logger->drop())
		os::Printer::Logger = 0;
}


void CIrrDeviceStub::createGUIAndScene()
{
	#ifdef _IRR_COMPILE_WITH_GUI_
	// create gui environment
	GUIEnvironment = gui::createGUIEnvironment(FileSystem, VideoDriver, Operator);
	#endif 

	// create Scene manager
	SceneManager = scene::createSceneManager(VideoDriver, FileSystem, CursorControl, GUIEnvironment);

	setEventReceiver(UserReceiver);
}


//! returns the video driver
video::IVideoDriver* CIrrDeviceStub::getVideoDriver()
{
	return VideoDriver;
}



//! return file system
io::IFileSystem* CIrrDeviceStub::getFileSystem()
{
	return FileSystem;
}



//! returns the gui environment
gui::IGUIEnvironment* CIrrDeviceStub::getGUIEnvironment()
{
	return GUIEnvironment;
}



//! returns the scene manager
scene::ISceneManager* CIrrDeviceStub::getSceneManager()
{
	return SceneManager;
}


//! \return Returns a pointer to the ITimer object. With it the
//! current Time can be received.
ITimer* CIrrDeviceStub::getTimer()
{
	return Timer;
}


//! Returns the version of the engine. 
const char* CIrrDeviceStub::getVersion() const
{
	return IRRLICHT_SDK_VERSION;
}

//! \return Returns a pointer to the mouse cursor control interface.
gui::ICursorControl* CIrrDeviceStub::getCursorControl()
{
	return CursorControl;
}


//! \return Returns a pointer to a list with all video modes supported
//! by the gfx adapter.
video::IVideoModeList* CIrrDeviceStub::getVideoModeList()
{
	return &VideoModeList;
}


//! checks version of sdk and prints warning if there might be a problem
bool CIrrDeviceStub::checkVersion(const char* version)
{
	if (strcmp(getVersion(), version))
	{
		core::stringc w;
		w = "Warning: The library version of the Irrlicht Engine (";
		w += getVersion();
		w += ") does not match the version the application was compiled with (";
		w += version;
		w += "). This may cause problems.";
		os::Printer::log(w.c_str(), ELL_WARNING);
		_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
		return false;
	}

	return true;
}


//! send the event to the right receiver
bool CIrrDeviceStub::postEventFromUser(const SEvent& event)
{
	bool absorbed = false;

	if (UserReceiver)
		absorbed = UserReceiver->OnEvent(event);

	if (!absorbed && GUIEnvironment)
		absorbed = GUIEnvironment->postEventFromUser(event);

	scene::ISceneManager* inputReceiver = InputReceivingSceneManager;
	if (!inputReceiver)
		inputReceiver = SceneManager;

	if (!absorbed && inputReceiver)
		absorbed = inputReceiver->postEventFromUser(event);

	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return absorbed;
}


//! Sets a new event receiver to receive events
void CIrrDeviceStub::setEventReceiver(IEventReceiver* receiver)
{
	UserReceiver = receiver;
	Logger->setReceiver(receiver);
	if (GUIEnvironment)
		GUIEnvironment->setUserEventReceiver(receiver);
}


//! Returns poinhter to the current event receiver. Returns 0 if there is none.
IEventReceiver* CIrrDeviceStub::getEventReceiver()
{
	return UserReceiver;
}


//! \return Returns a pointer to the logger.
ILogger* CIrrDeviceStub::getLogger()
{
	return Logger;
}


//! Returns the operation system opertator object.
IOSOperator* CIrrDeviceStub::getOSOperator()
{
	return Operator;
}


//! Sets the input receiving scene manager. 
void CIrrDeviceStub::setInputReceivingSceneManager(scene::ISceneManager* sceneManager)
{
	if (InputReceivingSceneManager)
		InputReceivingSceneManager->drop();

	InputReceivingSceneManager = sceneManager;

	if (InputReceivingSceneManager)
		InputReceivingSceneManager->grab();
}



} // end namespace irr

