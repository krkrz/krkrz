// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_PARTICLE_SPHERE_EMITTER_H_INCLUDED__
#define __C_PARTICLE_SPHERE_EMITTER_H_INCLUDED__

#include "IParticleSphereEmitter.h"
#include "irrArray.h"
#include "aabbox3d.h"

namespace irr
{
namespace scene
{

//! A default box emitter
class CParticleSphereEmitter : public IParticleSphereEmitter
{
public:

	//! constructor
	CParticleSphereEmitter(
		const core::vector3df& center, f32 radius,
		const core::vector3df& direction = core::vector3df(0.0f,0.03f,0.0f),
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

	//! Set direction the emitter emits particles
	virtual void setDirection( const core::vector3df& newDirection ) { Direction = newDirection; }

	//! Set minimum number of particles per second.
	virtual void setMinParticlesPerSecond( u32 minPPS ) { MinParticlesPerSecond = minPPS; }

	//! Set maximum number of particles per second.
	virtual void setMaxParticlesPerSecond( u32 maxPPS ) { MaxParticlesPerSecond = maxPPS; }

	//! Set minimum start color
	virtual void setMinStartColor( const video::SColor& color ) { MinStartColor = color; }

	//! Set maximum start color
	virtual void setMaxStartColor( const video::SColor& color ) { MaxStartColor = color; }

	//! Set the center of the sphere for particle emissions
	virtual void setCenter( const core::vector3df& center ) { Center = center; }

	//! Set the radius of the sphere for particle emissions
	virtual void setRadius( f32 radius ) { Radius = radius; }

	//! Gets direction the emitter emits particles
	virtual const core::vector3df& getDirection() const { return Direction; }

	//! Get minimum number of particles per second.
	virtual u32 getMinParticlesPerSecond() const { return MinParticlesPerSecond; }

	//! Get maximum number of particles per second.
	virtual u32 getMaxParticlesPerSecond() const { return MaxParticlesPerSecond; }

	//! Get minimum start color
	virtual const video::SColor& getMinStartColor() const { return MinStartColor; }

	//! Get maximum start color
	virtual const video::SColor& getMaxStartColor() const { return MaxStartColor; }

	//! Get the center of the sphere for particle emissions
	virtual const core::vector3df& getCenter() const { return Center; }

	//! Get the radius of the sphere for particle emissions
	virtual f32 getRadius() const { return Radius; }

private:

	core::array<SParticle> Particles;

	core::vector3df	Center;
	f32 Radius;

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

