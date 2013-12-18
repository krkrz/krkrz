// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_PARTICLE_CYLINDER_EMITTER_H_INCLUDED__
#define __C_PARTICLE_CYLINDER_EMITTER_H_INCLUDED__

#include "IParticleCylinderEmitter.h"
#include "irrArray.h"

namespace irr
{
namespace scene
{

//! A default box emitter
class CParticleCylinderEmitter : public IParticleCylinderEmitter
{
public:

	//! constructor
	CParticleCylinderEmitter(
		const core::vector3df& center, f32 radius,
		const core::vector3df& normal, f32 length,
		bool outlineOnly = false, const core::vector3df& direction = core::vector3df(0.0f,0.03f,0.0f),
		u32 minParticlesPerSecond = 20,
		u32 maxParticlesPerSecond = 40,
		const video::SColor& minStartColor = video::SColor(255,0,0,0),
		const video::SColor& maxStartColor = video::SColor(255,255,255,255),
		u32 lifeTimeMin=2000,
		u32 lifeTimeMax=4000,
		s32 maxAngleDegrees=0);

	//! Prepares an array with new particles to emitt into the system
	//! and returns how much new particles there are.
	virtual s32 emitt(u32 now, u32 timeSinceLastCall, SParticle*& outArray);

	//! Set the center of the radius for the cylinder, at one end of the cylinder
	virtual void setCenter( const core::vector3df& center ) { Center = center; }

	//! Set the normal of the cylinder
	virtual void setNormal( const core::vector3df& normal ) { Normal = normal; }

	//! Set the radius of the cylinder
	virtual void setRadius( f32 radius ) { Radius = radius; }

	//! Set the length of the cylinder
	virtual void setLength( f32 length ) { Length = length; }

	//! Set whether or not to draw points inside the cylinder
	virtual void setOutlineOnly( bool outlineOnly ) { OutlineOnly = outlineOnly; }

	//! Set direction the emitter emits particles
	virtual void setDirection( const core::vector3df& newDirection ) { Direction = newDirection; }

	//! Set direction the emitter emits particles
	virtual void setMinParticlesPerSecond( u32 minPPS ) { MinParticlesPerSecond = minPPS; }

	//! Set direction the emitter emits particles
	virtual void setMaxParticlesPerSecond( u32 maxPPS ) { MaxParticlesPerSecond = maxPPS; }

	//! Set direction the emitter emits particles
	virtual void setMinStartColor( const video::SColor& color ) { MinStartColor = color; }

	//! Set direction the emitter emits particles
	virtual void setMaxStartColor( const video::SColor& color ) { MaxStartColor = color; }

	//! Get the center of the cylinder
	virtual const core::vector3df& getCenter() const { return Center; }

	//! Get the normal of the cylinder
	virtual const core::vector3df& getNormal() const { return Normal; }

	//! Get the radius of the cylinder
	virtual f32 getRadius() const { return Radius; }

	//! Get the center of the cylinder
	virtual f32 getLength() const { return Length; }

	//! Get whether or not to draw points inside the cylinder
	virtual bool getOutlineOnly() const { return OutlineOnly; }

	//! Gets direction the emitter emits particles
	virtual const core::vector3df& getDirection() const { return Direction; }

	//! Gets direction the emitter emits particles
	virtual u32 getMinParticlesPerSecond() const { return MinParticlesPerSecond; }

	//! Gets direction the emitter emits particles
	virtual u32 getMaxParticlesPerSecond() const { return MaxParticlesPerSecond; }

	//! Gets direction the emitter emits particles
	virtual const video::SColor& getMinStartColor() const { return MinStartColor; }

	//! Gets direction the emitter emits particles
	virtual const video::SColor& getMaxStartColor() const { return MaxStartColor; }

private:

	core::array<SParticle> Particles;

	core::vector3df	Center;
	core::vector3df	Normal;
	f32	Radius;
	f32	Length;
	bool OutlineOnly;

	core::vector3df Direction;
	u32 MinParticlesPerSecond, MaxParticlesPerSecond;
	video::SColor MinStartColor, MaxStartColor;
	u32 MinLifeTime, MaxLifeTime;

	u32 Time;
	u32 Emitted;
	s32 MaxAngleDegrees;
};

} // end namespace scene
} // end namespace irr


#endif

