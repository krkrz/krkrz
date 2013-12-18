// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CParticleCylinderEmitter.h"
#include "os.h"

namespace irr
{
namespace scene
{

//! constructor
CParticleCylinderEmitter::CParticleCylinderEmitter(
	const core::vector3df& center, f32 radius,
	const core::vector3df& normal, f32 length,
	bool outlineOnly, const core::vector3df& direction,
	u32 minParticlesPerSecond, u32 maxParticlesPerSecond,
	const video::SColor& minStartColor, const video::SColor& maxStartColor,
	u32 lifeTimeMin, u32 lifeTimeMax, s32 maxAngleDegrees)
	: Center(center), Normal(normal), Radius(radius), Length(length), OutlineOnly( outlineOnly ),
	Direction(direction), MinParticlesPerSecond(minParticlesPerSecond),
	MaxParticlesPerSecond(maxParticlesPerSecond),
	MinStartColor(minStartColor), MaxStartColor(maxStartColor),
	MinLifeTime(lifeTimeMin), MaxLifeTime(lifeTimeMax), Time(0), Emitted(0),
	MaxAngleDegrees(maxAngleDegrees)
{
}


//! Prepares an array with new particles to emitt into the system
//! and returns how much new particles there are.
s32 CParticleCylinderEmitter::emitt(u32 now, u32 timeSinceLastCall, SParticle*& outArray)
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
			// Random distance from center if outline only is not true
			f32 distance;
			if( !OutlineOnly )
				distance = fmodf( (f32)os::Randomizer::rand(), Radius * 1000.0f ) * 0.001f;
			else
				distance = Radius;

			// Random direction from center
			p.pos.X = Center.X + distance;
			p.pos.Y = Center.Y;
			p.pos.Z = Center.Z + distance;
			p.pos.rotateXZBy( os::Randomizer::rand() % 360, Center );

			// Random length
			const f32 length = fmodf( (f32)os::Randomizer::rand(), Length * 1000.0f ) * 0.001f;

			// Random point along the cylinders length
			p.pos += Normal * length;

			p.startTime = now;
			p.vector = Direction;

			if( MaxAngleDegrees )
			{
				core::vector3df tgt = Direction;
				tgt.rotateXYBy((os::Randomizer::rand()%(MaxAngleDegrees*2)) - MaxAngleDegrees, core::vector3df());
				tgt.rotateYZBy((os::Randomizer::rand()%(MaxAngleDegrees*2)) - MaxAngleDegrees, core::vector3df());
				tgt.rotateXZBy((os::Randomizer::rand()%(MaxAngleDegrees*2)) - MaxAngleDegrees, core::vector3df());
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

