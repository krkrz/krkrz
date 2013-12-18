// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CCameraFPSSceneNode.h"
#include "IVideoDriver.h"
#include "ISceneManager.h"
#include "os.h"
#include "Keycodes.h"

namespace irr
{
namespace scene
{

const f32 MAX_VERTICAL_ANGLE = 88.0f;

//! constructor
CCameraFPSSceneNode::CCameraFPSSceneNode(ISceneNode* parent, ISceneManager* mgr,
		gui::ICursorControl* cursorControl, s32 id, f32 rotateSpeed , f32 moveSpeed,f32 jumpSpeed,
		SKeyMap* keyMapArray, s32 keyMapSize, bool noVerticalMovement)
: CCameraSceneNode(parent, mgr, id), CursorControl(cursorControl),
	MoveSpeed(moveSpeed), RotateSpeed(rotateSpeed), JumpSpeed(jumpSpeed),
	firstUpdate(true), LastAnimationTime(0), NoVerticalMovement(noVerticalMovement)
{
	#ifdef _DEBUG
	setDebugName("CCameraFPSSceneNode");
	#endif

	if (CursorControl)
		CursorControl->grab();

	MoveSpeed /= 1000.0f;

	recalculateViewArea();

	allKeysUp();

	// create key map
	if (!keyMapArray || !keyMapSize)
	{
		// create default key map
		KeyMap.push_back(SCamKeyMap(0, irr::KEY_UP));
		KeyMap.push_back(SCamKeyMap(1, irr::KEY_DOWN));
		KeyMap.push_back(SCamKeyMap(2, irr::KEY_LEFT));
		KeyMap.push_back(SCamKeyMap(3, irr::KEY_RIGHT));
		KeyMap.push_back(SCamKeyMap(4, irr::KEY_KEY_J));
	}
	else
	{
		// create custom key map

		for (s32 i=0; i<keyMapSize; ++i)
		{
			switch(keyMapArray[i].Action)
			{
			case EKA_MOVE_FORWARD: KeyMap.push_back(SCamKeyMap(0, keyMapArray[i].KeyCode));
				break;
			case EKA_MOVE_BACKWARD: KeyMap.push_back(SCamKeyMap(1, keyMapArray[i].KeyCode));
				break;
			case EKA_STRAFE_LEFT: KeyMap.push_back(SCamKeyMap(2, keyMapArray[i].KeyCode));
				break;
			case EKA_STRAFE_RIGHT: KeyMap.push_back(SCamKeyMap(3, keyMapArray[i].KeyCode));
				break;
			case EKA_JUMP_UP: KeyMap.push_back(SCamKeyMap(4, keyMapArray[i].KeyCode));
				break;
			default:
				break;
			} // end switch
		} // end for
	}// end if
}


//! destructor
CCameraFPSSceneNode::~CCameraFPSSceneNode()
{
	if (CursorControl)
		CursorControl->drop();
}


//! It is possible to send mouse and key events to the camera. Most cameras
//! may ignore this input, but camera scene nodes which are created for 
//! example with scene::ISceneManager::addMayaCameraSceneNode or
//! scene::ISceneManager::addFPSCameraSceneNode, may want to get this input
//! for changing their position, look at target or whatever. 
bool CCameraFPSSceneNode::OnEvent(const SEvent& event)
{
	if (event.EventType == EET_KEY_INPUT_EVENT)
	{
		const u32 cnt = KeyMap.size();
		for (u32 i=0; i<cnt; ++i)
			if (KeyMap[i].keycode == event.KeyInput.Key)
			{
				CursorKeys[KeyMap[i].action] = event.KeyInput.PressedDown; 

				if ( InputReceiverEnabled )
					return true;
			}
	}

	return false;
}



//! OnAnimate() is called just before rendering the whole scene.
//! nodes may calculate or store animations here, and may do other useful things,
//! dependent on what they are.
void CCameraFPSSceneNode::OnAnimate(u32 timeMs)
{
	animate( timeMs );

	core::list<ISceneNodeAnimator*>::Iterator ait = Animators.begin();
				for (; ait != Animators.end(); ++ait)
					(*ait)->animateNode(this, timeMs);

	updateAbsolutePosition();
	Target = getPosition() + TargetVector;

	core::list<ISceneNode*>::Iterator it = Children.begin();
				for (; it != Children.end(); ++it)
					(*it)->OnAnimate(timeMs);
}


void CCameraFPSSceneNode::animate( u32 timeMs )
{
	const u32 camIsMe = SceneManager->getActiveCamera() == this;

	if (firstUpdate)
	{
		if (CursorControl && camIsMe)
		{
			CursorControl->setPosition(0.5f, 0.5f);
			CenterCursor = CursorControl->getRelativePosition();
		}

		LastAnimationTime = os::Timer::getTime();

		firstUpdate = false;
	}

	// get time. only operate on valid camera
	f32 timeDiff = 0.f;

	if ( camIsMe )
	{
		timeDiff = (f32) ( timeMs - LastAnimationTime );
		LastAnimationTime = timeMs;
	}


	// update position
	core::vector3df pos = getPosition();	

	// Update rotation
//	if (InputReceiverEnabled)
	{
		Target.set(0,0,1);


		if (CursorControl && InputReceiverEnabled && camIsMe )
		{
			core::position2d<f32> cursorpos = CursorControl->getRelativePosition();

			if (!core::equals(cursorpos.X, CenterCursor.X) ||
				!core::equals(cursorpos.Y, CenterCursor.Y))
			{
				RelativeRotation.X *= -1.0f;
				RelativeRotation.Y *= -1.0f;

				RelativeRotation.Y += (0.5f - cursorpos.X) * RotateSpeed;
				RelativeRotation.X = core::clamp (	RelativeRotation.X + (0.5f - cursorpos.Y) * RotateSpeed,
													-MAX_VERTICAL_ANGLE,
													+MAX_VERTICAL_ANGLE
												);

				RelativeRotation.X *= -1.0f;
				RelativeRotation.Y *= -1.0f;

				CursorControl->setPosition(0.5f, 0.5f);
				CenterCursor = CursorControl->getRelativePosition();
			}
		}

		// set target

		core::matrix4 mat;
		mat.setRotationDegrees(core::vector3df( RelativeRotation.X, RelativeRotation.Y, 0));
		mat.transformVect(Target);

		core::vector3df movedir = Target;

		if (NoVerticalMovement)
			movedir.Y = 0.f;

		movedir.normalize();

		if (InputReceiverEnabled && camIsMe)
		{
			if (CursorKeys[0])
				pos += movedir * timeDiff * MoveSpeed;

			if (CursorKeys[1])
				pos -= movedir * timeDiff * MoveSpeed;

			// strafing

			core::vector3df strafevect = Target;
			strafevect = strafevect.crossProduct(UpVector);

			if (NoVerticalMovement)
				strafevect.Y = 0.0f;

			strafevect.normalize();

			if (CursorKeys[2])
				pos += strafevect * timeDiff * MoveSpeed;

			if (CursorKeys[3])
				pos -= strafevect * timeDiff * MoveSpeed;

			// jumping ( need's a gravity , else it's a fly to the World-UpVector )
			if (CursorKeys[4])
			{
				pos += UpVector * timeDiff * JumpSpeed;
			}
		}

		// write translation

		setPosition(pos);
	}

	// write right target

	TargetVector = Target;
	Target += pos;

}

void CCameraFPSSceneNode::allKeysUp()
{
	for (s32 i=0; i<6; ++i)
		CursorKeys[i] = false;
}


//! sets the look at target of the camera
//! \param pos: Look at target of the camera.
void CCameraFPSSceneNode::setTarget(const core::vector3df& tgt)
{
	updateAbsolutePosition();
	core::vector3df vect = tgt - getAbsolutePosition();
	vect = vect.getHorizontalAngle();
	RelativeRotation.X = vect.X;
	RelativeRotation.Y = vect.Y;

	if (RelativeRotation.X > MAX_VERTICAL_ANGLE)
		 RelativeRotation.X -= 360.0f;
}

//! Disables or enables the camera to get key or mouse inputs.
void CCameraFPSSceneNode::setInputReceiverEnabled(bool enabled)
{
   // So we don't skip when we return from a non-enabled mode and the
   // mouse cursor is now not in the middle of the screen
   if( !InputReceiverEnabled && enabled )
      firstUpdate = true;

   InputReceiverEnabled = enabled;
} 

//! Sets the rotation speed
void CCameraFPSSceneNode::setRotateSpeed(const f32 speed)
{
	RotateSpeed = speed;	
}

//! Sets the movement speed
void CCameraFPSSceneNode::setMoveSpeed(const f32 speed)
{
	MoveSpeed = speed;
}

//! Gets the rotation speed
f32 CCameraFPSSceneNode::getRotateSpeed()
{
	return RotateSpeed;
}

// Gets the movement speed
f32 CCameraFPSSceneNode::getMoveSpeed()
{
	return MoveSpeed;
}

} // end namespace
} // end namespace

