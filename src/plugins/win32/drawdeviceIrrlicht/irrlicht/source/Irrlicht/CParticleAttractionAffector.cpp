// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CParticleAttractionAffector.h"

namespace irr
{
namespace scene
{

//! constructor
CParticleAttractionAffector::CParticleAttractionAffector(
		const core::vector3df& point, f32 speed, bool attract,
		bool affectX, bool affectY, bool affectZ )
	: Point(point), Speed(speed), AffectX(affectX), AffectY(affectY),
		AffectZ(affectZ), Attract(attract), LastTime(0)
{
}


//! Affects an array of particles.
void CParticleAttractionAffector::affect(u32 now, SParticle* particlearray, u32 count)
{
	if( LastTime == 0 )
	{
		LastTime = now;
		return;
	}

	f32 timeDelta = ( now - LastTime ) / 1000.0f;
	LastTime = now;

	if( !Enabled )
		return;

	for(u32 i=0; i<count; ++i)
	{
		core::vector3df direction = (Point - particlearray[i].pos).normalize();
		direction *= Speed * timeDelta;

		if( !Attract )
			direction *= -1.0f;

		if( AffectX )
			particlearray[i].pos.X += direction.X;

		if( AffectY )
			particlearray[i].pos.Y += direction.Y;

		if( AffectZ )
			particlearray[i].pos.Z += direction.Z;
	}
}


} // end namespace scene
} // end namespace irr

