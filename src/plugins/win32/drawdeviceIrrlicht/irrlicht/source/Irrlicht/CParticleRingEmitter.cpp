// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CParticleRingEmitter.h"
#include "os.h"

namespace irr
{
namespace scene
{

//! constructor
CParticleRingEmitter::CParticleRingEmitter(
	const core::vector3df& center, f32 radius, f32 ringThickness,
	const core::vector3df& direction, u32 minParticlesPerSecond,
	u32 maxParticlesPerSecond, const video::SColor& minStartColor,
	const video::SColor& maxStartColor, u32 lifeTimeMin, u32 lifeTimeMax,
	s32 maxAngleDegrees)
	: Center(center), Radius(radius), RingThickness(ringThickness),
		Direction(direction), MinParticlesPerSecond(minParticlesPerSecond),
		MaxParticlesPerSecond(maxParticlesPerSecond), MinStartColor(minStartColor),
		MaxStartColor(maxStartColor), MinLifeTime(lifeTimeMin), MaxLifeTime(lifeTimeMax),
		Time(0), Emitted(0), MaxAngleDegrees(maxAngleDegrees)
{
}


//! Prepares an array with new particles to emitt into the system
//! and returns how much new particles there are.
s32 CParticleRingEmitter::emitt(u32 now, u32 timeSinceLastCall, SParticle*& outArray)
{
	Time += timeSinceLastCall;

	u32 pps = (MaxParticlesPerSecond - MinParticlesPerSecond);
	f32 perSecond = pps ? (f32)MinParticlesPerSecond + (os::Randomizer::rand() % pps) : MinParticlesPerSecond;
	f32 everyWhatMillisecond = 1000.0f / perSecond;

	if(Time > everyWhatMillisecond)
	{
		Particles.set_used(0);
		u32 amount = (u32)((Time / everyWhatMillisecond) + 0.5f);
		Time = 0;
		SParticle p;

		if(amount > MaxParticlesPerSecond*2)
			amount = MaxParticlesPerSecond * 2;

		for(u32 i=0; i<amount; ++i)
		{
			f32 distance = fmodf( (f32)os::Randomizer::rand(), RingThickness * 0.5f * 1000.0f ) * 0.001f;
			s32 plusMinus = os::Randomizer::rand() % 2;
			if( plusMinus )
				distance *= -1.0f;
			distance += Radius;

			p.pos.X = Center.X + distance;
			p.pos.Y = Center.Y;
			p.pos.Z = Center.Z + distance;

			p.pos.rotateXZBy( ( os::Randomizer::rand() % 3600 ) * 0.1f, Center );

			p.startTime = now;
			p.vector = Direction;

			if(MaxAngleDegrees)
			{
				core::vector3df tgt = Direction;
				tgt.rotateXYBy((os::Randomizer::rand()%(MaxAngleDegrees*2)) - MaxAngleDegrees, Center );
				tgt.rotateYZBy((os::Randomizer::rand()%(MaxAngleDegrees*2)) - MaxAngleDegrees, Center );
				tgt.rotateXZBy((os::Randomizer::rand()%(MaxAngleDegrees*2)) - MaxAngleDegrees, Center );
				p.vector = tgt;
			}

			if(MaxLifeTime - MinLifeTime == 0)
				p.endTime = now + MinLifeTime;
			else
				p.endTime = now + MinLifeTime + (os::Randomizer::rand() % (MaxLifeTime - MinLifeTime));

			p.color = MinStartColor.getInterpolated(
				MaxStartColor, (os::Randomizer::rand() % 100) / 100.0f);

			p.startColor = p.color;
			p.startVector = p.vector;

			Particles.push_back(p);
		}

		outArray = Particles.pointer();

		return Particles.size();
	}

	return 0;
}

} // end namespace scene
} // end namespace irr

