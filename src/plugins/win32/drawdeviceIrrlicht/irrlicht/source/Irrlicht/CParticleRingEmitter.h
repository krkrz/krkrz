// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_PARTICLE_RING_EMITTER_H_INCLUDED__
#define __C_PARTICLE_RING_EMITTER_H_INCLUDED__

#include "IParticleRingEmitter.h"
#include "irrArray.h"

namespace irr
{
namespace scene
{

//! A ring emitter
class CParticleRingEmitter : public IParticleRingEmitter
{
public:

	//! constructor
	CParticleRingEmitter(
		const core::vector3df& center, f32 radius, f32 ringThickness,
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

	//! Set minimum number of particles the emitter emits per second
	virtual void setMinParticlesPerSecond( u32 minPPS ) { MinParticlesPerSecond = minPPS; }

	//! Set maximum number of particles the emitter emits per second
	virtual void setMaxParticlesPerSecond( u32 maxPPS ) { MaxParticlesPerSecond = maxPPS; }

	//! Set minimum starting color for particles
	virtual void setMinStartColor( const video::SColor& color ) { MinStartColor = color; }

	//! Set maximum starting color for particles
	virtual void setMaxStartColor( const video::SColor& color ) { MaxStartColor = color; }

	//! Set the center of the ring
	virtual void setCenter( const core::vector3df& center ) { Center = center; }

	//! Set the radius of the ring
	virtual void setRadius( f32 radius ) { Radius = radius; }

	//! Set the thickness of the ring
	virtual void setRingThickness( f32 ringThickness ) { RingThickness = ringThickness; }

	//! Gets direction the emitter emits particles
	virtual const core::vector3df& getDirection() const { return Direction; }

	//! Gets the minimum number of particles the emitter emits per second
	virtual u32 getMinParticlesPerSecond() const { return MinParticlesPerSecond; }

	//! Gets the maximum number of particles the emitter emits per second
	virtual u32 getMaxParticlesPerSecond() const { return MaxParticlesPerSecond; }

	//! Gets the minimum starting color for particles
	virtual const video::SColor& getMinStartColor() const { return MinStartColor; }

	//! Gets the maximum starting color for particles
	virtual const video::SColor& getMaxStartColor() const { return MaxStartColor; }

	//! Get the center of the ring
	virtual const core::vector3df& getCenter() const { return Center; }

	//! Get the radius of the ring
	virtual f32 getRadius() const { return Radius; }

	//! Get the thickness of the ring
	virtual f32 getRingThickness() const { return RingThickness; }

private:

	core::array<SParticle> Particles;

	core::vector3df	Center;
	f32 Radius;
	f32 RingThickness;

	core::vector3df Direction;
	u32 MinParticlesPerSecond, MaxParticlesPerSecond;
	video::SColor MinStartColor, MaxStartColor;
	u32 MinLifeTime, MaxLifeTime;

	u32 Time;
	u32 Emitted;
	s32 MaxAngleDegrees;

	f32 MinimumDistance;
	f32 MaximumDistance;
};

} // end namespace scene
} // end namespace irr


#endif

