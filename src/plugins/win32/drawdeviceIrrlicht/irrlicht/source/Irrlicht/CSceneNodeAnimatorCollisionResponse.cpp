// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CSceneNodeAnimatorCollisionResponse.h"
#include "ISceneCollisionManager.h"
#include "ISceneManager.h"
#include "os.h"

namespace irr
{
namespace scene
{

//! constructor
CSceneNodeAnimatorCollisionResponse::CSceneNodeAnimatorCollisionResponse(
		ISceneManager* scenemanager,
		ITriangleSelector* world, ISceneNode* object,
		const core::vector3df& ellipsoidRadius,
		const core::vector3df& gravityPerSecond,
		const core::vector3df& ellipsoidTranslation,
		f32 slidingSpeed)
: Radius(ellipsoidRadius), Gravity(gravityPerSecond / 1000.0f), Translation(ellipsoidTranslation),
	World(world), Object(object), SceneManager(scenemanager),
	SlidingSpeed(slidingSpeed), Falling(false)
{
	if (World)
		World->grab();

	if (Object)
		LastPosition = Object->getPosition();

	LastTime = os::Timer::getTime();
	FallStartTime = LastTime;
}



//! destructor
CSceneNodeAnimatorCollisionResponse::~CSceneNodeAnimatorCollisionResponse()
{
	if (World)
		World->drop();
}


//! Returns if the attached scene node is falling, which means that
//! there is no blocking wall from the scene node in the direction of
//! the gravity.
bool CSceneNodeAnimatorCollisionResponse::isFalling() const
{
	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return Falling;
}


//! Sets the radius of the ellipsoid with which collision detection and
//! response is done.
void CSceneNodeAnimatorCollisionResponse::setEllipsoidRadius(
	const core::vector3df& radius)
{
	Radius = radius;
}

//! Returns the radius of the ellipsoid with wich the collision detection and
//! response is done.
core::vector3df CSceneNodeAnimatorCollisionResponse::getEllipsoidRadius() const
{
	return Radius;
}


//! Sets the gravity of the environment.
void CSceneNodeAnimatorCollisionResponse::setGravity(const core::vector3df& gravity)
{
	Gravity = gravity;
}


//! Returns current vector of gravity.
core::vector3df CSceneNodeAnimatorCollisionResponse::getGravity() const
{
	return Gravity;
}


//! Sets the translation of the ellipsoid for collision detection.
void CSceneNodeAnimatorCollisionResponse::setEllipsoidTranslation(const core::vector3df &translation)
{
	Translation = translation;
}



//! Returns the translation of the ellipsoid for collision detection.
core::vector3df CSceneNodeAnimatorCollisionResponse::getEllipsoidTranslation() const
{
	return Translation;
}


//! Sets a triangle selector holding all triangles of the world with which
//! the scene node may collide.
void CSceneNodeAnimatorCollisionResponse::setWorld(ITriangleSelector* newWorld)
{
	Falling = false;

	LastTime = os::Timer::getTime();
	FallStartTime = LastTime;


	if (World)
		World->drop();

	World = newWorld;
	if (World)
		World->grab();

}



//! Returns the current triangle selector containing all triangles for
//! collision detection.
ITriangleSelector* CSceneNodeAnimatorCollisionResponse::getWorld() const
{
	return World;
}



void CSceneNodeAnimatorCollisionResponse::animateNode(ISceneNode* node, u32 timeMs)
{
	if (node != Object)
	{
		os::Printer::log("CollisionResponseAnimator only works with same scene node as set as object during creation", ELL_ERROR);
		return;
	}

	if (!World)
		return;

	u32 diff = timeMs - LastTime;
	LastTime = timeMs;

	core::vector3df pos = Object->getPosition();
	core::vector3df vel = pos - LastPosition;

	//g = Gravity * (f32)((timeMs - FallStartTime) * diff);

	f32 dt = 1.f;
	if (Falling)
	{
		dt = f32 ( ( timeMs - FallStartTime ) * diff );
	}
	core::vector3df g = Gravity * dt;

	core::triangle3df triangle = RefTriangle;

	core::vector3df force = vel + g;

	const core::vector3df nullVector ( 0.f, 0.f, 0.f );

	if ( force != nullVector )
	{
		// TODO: divide SlidingSpeed by frame time

		bool f = false;
		pos = SceneManager->getSceneCollisionManager()->getCollisionResultPosition(
				World, LastPosition-Translation,
				Radius, vel, triangle, f, SlidingSpeed, g);

		pos += Translation;

		if (f)//triangle == RefTriangle)
		{
			if (!Falling)
				FallStartTime = timeMs;

			Falling = true;
		}
		else
			Falling = false;

		Object->setPosition(pos);
	}

	LastPosition = Object->getPosition();
}

//! Writes attributes of the scene node animator.
void CSceneNodeAnimatorCollisionResponse::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const
{
	out->addVector3d("Radius", Radius);
	out->addVector3d("Gravity", Gravity);
	out->addVector3d("Translation", Translation);
}

//! Reads attributes of the scene node animator.
void CSceneNodeAnimatorCollisionResponse::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options)
{
	Radius = in->getAttributeAsVector3d("Radius");
	Gravity = in->getAttributeAsVector3d("Gravity");
	Translation = in->getAttributeAsVector3d("Translation");
}



} // end namespace scene
} // end namespace irr

