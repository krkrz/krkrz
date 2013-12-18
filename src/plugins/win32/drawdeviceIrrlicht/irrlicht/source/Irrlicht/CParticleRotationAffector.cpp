// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CParticleRotationAffector.h"

namespace irr
{
namespace scene
{

//! constructor
CParticleRotationAffector::CParticleRotationAffector( const core::vector3df& speed, const core::vector3df& pivotPoint )
		: PivotPoint(pivotPoint), Speed(speed), LastTime(0)
{
}


//! Affects an array of particles.
void CParticleRotationAffector::affect(u32 now, SParticle* particlearray, u32 count)
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
		if( Speed.X != 0.0f )
			particlearray[i].pos.rotateYZBy( timeDelta * Speed.X, PivotPoint );

		if( Speed.Y != 0.0f )
			particlearray[i].pos.rotateXZBy( timeDelta * Speed.Y, PivotPoint );

		if( Speed.Z != 0.0f )
			particlearray[i].pos.rotateXYBy( timeDelta * Speed.Z, PivotPoint );
	}
}


} // end namespace scene
} // end namespace irr

