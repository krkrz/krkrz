// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CCameraMayaSceneNode.h"
#include "IVideoDriver.h"
#include "ISceneManager.h"

namespace irr
{
namespace scene
{

//! constructor
CCameraMayaSceneNode::CCameraMayaSceneNode(ISceneNode* parent, ISceneManager* mgr, s32 id,
	 f32 rs, f32 zs, f32 ts)
: CCameraSceneNode(parent, mgr, id), 
	Zooming(false), Rotating(false), Moving(false), Translating(false),
	ZoomSpeed(zs), RotateSpeed(rs), TranslateSpeed(ts),
	RotateStartX(0.0f), RotateStartY(0.0f), ZoomStartX(0.0f), ZoomStartY(0.0f),
	TranslateStartX(0.0f), TranslateStartY(0.0f), CurrentZoom(70.0f), RotX(0.0f), RotY(0.0f)
{
	#ifdef _DEBUG
	setDebugName("CCameraMayaSceneNode");
	#endif

	Target.set(0.0f, 0.0f, 0.0f);
	OldTarget = Target;

	allKeysUp();
	recalculateViewArea();
}


//! destructor
CCameraMayaSceneNode::~CCameraMayaSceneNode()
{
}


//! It is possible to send mouse and key events to the camera. Most cameras
//! may ignore this input, but camera scene nodes which are created for 
//! example with scene::ISceneManager::addMayaCameraSceneNode or
//! scene::ISceneManager::addMeshViewerCameraSceneNode, may want to get this input
//! for changing their position, look at target or whatever.
bool CCameraMayaSceneNode::OnEvent(const SEvent& event)
{
	if (event.EventType != EET_MOUSE_INPUT_EVENT ||
		!InputReceiverEnabled)
		return false;

	switch(event.MouseInput.Event)
	{
	case EMIE_LMOUSE_PRESSED_DOWN:
		MouseKeys[0] = true;
		break;
	case EMIE_RMOUSE_PRESSED_DOWN:
		MouseKeys[2] = true;
		break;
	case EMIE_MMOUSE_PRESSED_DOWN:
		MouseKeys[1] = true;
		break;
	case EMIE_LMOUSE_LEFT_UP:
		MouseKeys[0] = false;
		break;
	case EMIE_RMOUSE_LEFT_UP:
		MouseKeys[2] = false;
		break;
	case EMIE_MMOUSE_LEFT_UP:
		MouseKeys[1] = false;
		break;
	case EMIE_MOUSE_MOVED:
		{
			video::IVideoDriver* driver = SceneManager->getVideoDriver();
			if (driver)
			{
				core::dimension2d<s32> ssize = SceneManager->getVideoDriver()->getScreenSize();
				MousePos.X = event.MouseInput.X / (f32)ssize.Width;
				MousePos.Y = event.MouseInput.Y / (f32)ssize.Height;
			}
		}
		break;
	case EMIE_MOUSE_WHEEL:
	case EMIE_COUNT:
		break;
	}
	return true;
}

//! OnAnimate() is called just before rendering the whole scene.
//! nodes may calculate or store animations here, and may do other useful things,
//! dependent on what they are.
void CCameraMayaSceneNode::OnAnimate(u32 timeMs)
{
	animate();

	ISceneNode::setPosition(Pos);
	updateAbsolutePosition();

	// This scene node cannot be animated by scene node animators, so
	// don't invoke them.
}


bool CCameraMayaSceneNode::isMouseKeyDown(s32 key)
{
	return MouseKeys[key];
}



void CCameraMayaSceneNode::animate()
{
	//Alt + LM = Rotate around camera pivot
	//Alt + LM + MM = Dolly forth/back in view direction (speed % distance camera pivot - max distance to pivot)
	//Alt + MM = Move on camera plane (Screen center is about the mouse pointer, depending on move speed)

	const SViewFrustum* va = getViewFrustum();

	f32 nRotX = RotX;
	f32 nRotY = RotY;
	f32 nZoom = CurrentZoom;

	if ( (isMouseKeyDown(0) && isMouseKeyDown(2)) || isMouseKeyDown(1) )
	{
		if (!Zooming)
		{
			ZoomStartX = MousePos.X;
			ZoomStartY = MousePos.Y;
			Zooming = true;
			nZoom = CurrentZoom;
		}
		else
		{
			f32 old = nZoom;
			nZoom += (ZoomStartX - MousePos.X) * ZoomSpeed;

			f32 targetMinDistance = 0.1f;
			if (nZoom < targetMinDistance) // jox: fixed bug: bounce back when zooming to close
				nZoom = targetMinDistance;

			if (nZoom < 0)
				nZoom = old;
		}
	}
	else
	{
		if (Zooming)
		{
			f32 old = CurrentZoom;
			CurrentZoom = CurrentZoom + (ZoomStartX - MousePos.X ) * ZoomSpeed;
			nZoom = CurrentZoom;

			if (nZoom < 0)
				nZoom = CurrentZoom = old;
		}

		Zooming = false;
	}

	// Translation ---------------------------------

	core::vector3df translate(OldTarget);

	core::vector3df tvectX = Pos - Target;
	tvectX = tvectX.crossProduct(UpVector);
	tvectX.normalize();

	core::vector3df tvectY = (va->getFarLeftDown() - va->getFarRightDown());
	tvectY = tvectY.crossProduct(UpVector.Y > 0 ? Pos - Target : Target - Pos);
	tvectY.normalize();
	

	if (isMouseKeyDown(2) && !Zooming)
	{
		if (!Translating)
		{
			TranslateStartX = MousePos.X;
			TranslateStartY = MousePos.Y;
			Translating = true;
		}
		else
		{
			translate +=	tvectX * (TranslateStartX - MousePos.X)*TranslateSpeed + 
								tvectY * (TranslateStartY - MousePos.Y)*TranslateSpeed;
		}
	}
	else
	{
		if (Translating)
		{
			translate +=	tvectX * (TranslateStartX - MousePos.X)*TranslateSpeed + 
								tvectY * (TranslateStartY - MousePos.Y)*TranslateSpeed;
			OldTarget = translate;
		}

		Translating = false;
	}

	// Rotation ------------------------------------

	if (isMouseKeyDown(0) && !Zooming)
	{
		if (!Rotating)
		{
			RotateStartX = MousePos.X;
			RotateStartY = MousePos.Y;
			Rotating = true;
			nRotX = RotX;
			nRotY = RotY;
		}
		else
		{
			nRotX += (RotateStartX - MousePos.X) * RotateSpeed;
			nRotY += (RotateStartY - MousePos.Y) * RotateSpeed;
		}
	}
	else
	{
		if (Rotating)
		{
			RotX = RotX + (RotateStartX - MousePos.X) * RotateSpeed;
			RotY = RotY + (RotateStartY - MousePos.Y) * RotateSpeed;
			nRotX = RotX;
			nRotY = RotY;
		}

		Rotating = false;
	}

	// Set Pos ------------------------------------

	Target = translate;

	Pos.X = nZoom + Target.X;
	Pos.Y = Target.Y;
	Pos.Z = Target.Z;

	Pos.rotateXYBy(nRotY, Target);
	Pos.rotateXZBy(-nRotX, Target);

	// Rotation Error ----------------------------

	// jox: fixed bug: jitter when rotating to the top and bottom of y
	UpVector.set(0,1,0);
	UpVector.rotateXYBy(-nRotY, core::vector3df(0,0,0));
	UpVector.rotateXZBy(-nRotX+180.f, core::vector3df(0,0,0));

	/*if (nRotY < 0.0f)
		nRotY *= -1.0f;
	
	nRotY = (f32)fmod(nRotY, 360.0f);
	
	
	if (nRotY >= 90.0f && nRotY <= 270.0f)
		UpVector.set(0, -1, 0);
	else
		UpVector.set(0, 1, 0);*/
}


void CCameraMayaSceneNode::allKeysUp()
{
	for (s32 i=0; i<3; ++i)
		MouseKeys[i] = false;
}

// function added by jox: fix setPosition()
void CCameraMayaSceneNode::setPosition(const core::vector3df& pos) 
{
	Pos = pos;
	updateAnimationState();

	ISceneNode::setPosition(pos); 
}

// function added by jox: fix setTarget()
void CCameraMayaSceneNode::setTarget(const core::vector3df& pos) 
{
	Target = OldTarget = pos;
	updateAnimationState();
}


// function added by jox
void CCameraMayaSceneNode::updateAnimationState() 
{
	core::vector3df pos(Pos - Target);

	// X rotation
	core::vector2df vec2d(pos.X, pos.Z);
	RotX = (f32)vec2d.getAngle();

	// Y rotation
	pos.rotateXZBy(RotX, core::vector3df());
	vec2d.set(pos.X, pos.Y);
	RotY = -(f32)vec2d.getAngle();

	// Zoom
	CurrentZoom = (f32)Pos.getDistanceFrom(Target);
}

//! Sets the rotation speed
void CCameraMayaSceneNode::setRotateSpeed(const f32 speed)
{
	RotateSpeed = speed;	
}

//! Sets the movement speed
void CCameraMayaSceneNode::setMoveSpeed(const f32 speed)
{
	TranslateSpeed = speed;
}

//! Gets the rotation speed
f32 CCameraMayaSceneNode::getRotateSpeed()
{
	return RotateSpeed;
}

// Gets the movement speed
f32 CCameraMayaSceneNode::getMoveSpeed()
{
	return TranslateSpeed;
}

} // end namespace
} // end namespace

