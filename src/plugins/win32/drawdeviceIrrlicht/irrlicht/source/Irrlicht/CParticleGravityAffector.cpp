// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CParticleGravityAffector.h"
#include "os.h"
#include "IAttributes.h"

namespace irr
{
namespace scene
{

//! constructor
CParticleGravityAffector::CParticleGravityAffector(
	const core::vector3df& gravity, u32 timeForceLost)
	: IParticleGravityAffector(), TimeForceLost(static_cast<f32>(timeForceLost)), Gravity(gravity)
{
}


//! Affects an array of particles.
void CParticleGravityAffector::affect(u32 now, SParticle* particlearray, u32 count)
{
	if (!Enabled)
		return;
	f32 d;

	for (u32 i=0; i<count; ++i)
	{
		d = (now - particlearray[i].startTime) / TimeForceLost;
		if (d > 1.0f)
			d = 1.0f;
		if (d < 0.0f)
			d = 0.0f;
		d = 1.0f - d;

		particlearray[i].vector = particlearray[i].startVector.getInterpolated(Gravity, d);
	}
}

//! Writes attributes of the object.
void CParticleGravityAffector::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const
{
	out->addVector3d("Gravity", Gravity);
	out->addFloat("TimeForceLost", TimeForceLost);
}


//! Reads attributes of the object.
s32 CParticleGravityAffector::deserializeAttributes(s32 startIndex, io::IAttributes* in, io::SAttributeReadWriteOptions* options)
{
	const char* name = in->getAttributeName(startIndex);

	if (!name || strcmp(name, "Gravity"))
		return startIndex; // attribute not valid

	Gravity = in->getAttributeAsVector3d(startIndex);
	++startIndex;

	name = in->getAttributeName(startIndex);
	if (!name || strcmp(name, "TimeForceLost"))
		return startIndex; // attribute not valid

	TimeForceLost = in->getAttributeAsFloat(startIndex);

	++startIndex;
	return startIndex;
}



} // end namespace scene
} // end namespace irr

