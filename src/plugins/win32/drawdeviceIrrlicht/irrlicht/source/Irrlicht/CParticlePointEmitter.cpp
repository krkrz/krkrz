// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CParticlePointEmitter.h"
#include "os.h"
#include "IAttributes.h"

namespace irr
{
namespace scene
{

//! constructor
CParticlePointEmitter::CParticlePointEmitter(
	const core::vector3df& direction, u32 minParticlesPerSecond,
	u32 maxParticlesPerSecond, video::SColor minStartColor,
	video::SColor maxStartColor, u32 lifeTimeMin, u32 lifeTimeMax,
	s32 maxAngleDegrees)
 : Direction(direction), MinParticlesPerSecond(minParticlesPerSecond),
	MaxParticlesPerSecond(maxParticlesPerSecond),
	MinStartColor(minStartColor), MaxStartColor(maxStartColor),
	MinLifeTime(lifeTimeMin), MaxLifeTime(lifeTimeMax),
	MaxAngleDegrees(maxAngleDegrees), Time(0), Emitted(0)
{

}



//! Prepares an array with new particles to emitt into the system
//! and returns how much new particles there are.
s32 CParticlePointEmitter::emitt(u32 now, u32 timeSinceLastCall, SParticle*& outArray)
{
	Time += timeSinceLastCall;

	u32 pps = (MaxParticlesPerSecond - MinParticlesPerSecond);
	f32 perSecond = pps ? (f32)MinParticlesPerSecond + (os::Randomizer::rand() % pps) : MinParticlesPerSecond;
	f32 everyWhatMillisecond = 1000.0f / perSecond;

	if (Time > everyWhatMillisecond)
	{
		Time = 0;
		Particle.startTime = now;
		Particle.vector = Direction;

		if (MaxAngleDegrees)
		{
			core::vector3df tgt = Direction;
			tgt.rotateXYBy((os::Randomizer::rand()%(MaxAngleDegrees*2)) - MaxAngleDegrees, core::vector3df(0,0,0));
			tgt.rotateYZBy((os::Randomizer::rand()%(MaxAngleDegrees*2)) - MaxAngleDegrees, core::vector3df(0,0,0));
			tgt.rotateXZBy((os::Randomizer::rand()%(MaxAngleDegrees*2)) - MaxAngleDegrees, core::vector3df(0,0,0));
			Particle.vector = tgt;
		}

		if (MaxLifeTime - MinLifeTime == 0)
			Particle.endTime = now + MinLifeTime;
		else
			Particle.endTime = now + MinLifeTime + (os::Randomizer::rand() % (MaxLifeTime - MinLifeTime));

		Particle.color = MinStartColor.getInterpolated(
			MaxStartColor, (os::Randomizer::rand() % 100) / 100.0f);

		Particle.startColor = Particle.color;
		Particle.startVector = Particle.vector;
		outArray = &Particle;
		return 1;
	}

	return 0;
}


//! Writes attributes of the object.
void CParticlePointEmitter::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const
{
	out->addVector3d("Direction", Direction);
	out->addInt("MinParticlesPerSecond", MinParticlesPerSecond);
	out->addInt("MaxParticlesPerSecond", MaxParticlesPerSecond);
	out->addColor("MinStartColor", MinStartColor);
	out->addColor("MaxStartColor", MaxStartColor);
	out->addInt("MinLifeTime", MinLifeTime);
	out->addInt("MaxLifeTime", MaxLifeTime);
	out->addInt("MaxAngleDegrees", MaxAngleDegrees);
}


//! Reads attributes of the object.
s32 CParticlePointEmitter::deserializeAttributes(s32 startIndex, io::IAttributes* in, io::SAttributeReadWriteOptions* options)
{
	Direction = in->getAttributeAsVector3d("Direction");
	if (Direction.getLength() == 0)
		Direction.set(0,0.01f,0);

	MinParticlesPerSecond = in->getAttributeAsInt("MinParticlesPerSecond");
	MaxParticlesPerSecond = in->getAttributeAsInt("MaxParticlesPerSecond");

	MinParticlesPerSecond = core::max_(1u, MinParticlesPerSecond);
	MaxParticlesPerSecond = core::max_(MaxParticlesPerSecond, 1u);
	MaxParticlesPerSecond = core::min_(MaxParticlesPerSecond, 200u);
	MinParticlesPerSecond = core::min_(MinParticlesPerSecond, MaxParticlesPerSecond);

	MinStartColor = in->getAttributeAsColor("MinStartColor");
	MaxStartColor = in->getAttributeAsColor("MaxStartColor");
	MinLifeTime = in->getAttributeAsInt("MinLifeTime");
	MaxLifeTime = in->getAttributeAsInt("MaxLifeTime");
	MaxAngleDegrees = in->getAttributeAsInt("MaxAngleDegrees");

	MinLifeTime = core::max_(0u, MinLifeTime);
	MaxLifeTime = core::max_(MaxLifeTime, MinLifeTime);
	MinLifeTime = core::min_(MinLifeTime, MaxLifeTime);

	return in->findAttribute("MaxAngleDegrees");
}


} // end namespace scene
} // end namespace irr

