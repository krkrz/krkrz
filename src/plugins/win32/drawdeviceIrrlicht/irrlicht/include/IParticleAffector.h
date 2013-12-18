// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_PARTICLE_AFFECTOR_H_INCLUDED__
#define __I_PARTICLE_AFFECTOR_H_INCLUDED__

#include "IAttributeExchangingObject.h"
#include "SParticle.h"

namespace irr
{
namespace scene
{

//! Types of built in particle affectors
enum E_PARTICLE_AFFECTOR_TYPE
{
	EPAT_NONE = 0,
	EPAT_ATTRACT,
	EPAT_FADE_OUT,
	EPAT_GRAVITY,
	EPAT_ROTATE,
	EPAT_COUNT
};

//! Names for built in particle affectors
const c8* const ParticleAffectorTypeNames[] =
{
	"None",
	"Attract",
	"FadeOut",
	"Gravity",
	"Rotate",
	0
};

//! A particle affector modifies particles.
class IParticleAffector : public virtual io::IAttributeExchangingObject
{
public:

	//! constructor
	IParticleAffector() : Enabled(true) {}

	//! Affects an array of particles.
	//! \param now: Current time. (Same as ITimer::getTime() would return)
	//! \param particlearray: Array of particles.
	//! \param count: Amount of particles in array.
	virtual void affect(u32 now, SParticle* particlearray, u32 count) = 0;

	//! Sets whether or not the affector is currently enabled.
	virtual void setEnabled(bool enabled) { Enabled = enabled; }

	//! Gets whether or not the affector is currently enabled.
	virtual bool getEnabled() const { return Enabled; }

	//! Writes attributes of the object.
	//! Implement this to expose the attributes of your scene node animator for
	//! scripting languages, editors, debuggers or xml serialization purposes.
	virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const {}

	//! Reads attributes of the object.
	//! Implement this to set the attributes of your scene node animator for
	//! scripting languages, editors, debuggers or xml deserialization purposes.
	//! \param startIndex: start index where to start reading attributes.
	//! \param in: The attributes to work with.
	//! \param options: Additional options.
	//! \return: returns last index of an attribute read by this affector
	virtual s32 deserializeAttributes(s32 startIndex, io::IAttributes* in, io::SAttributeReadWriteOptions* options=0) { return 0; }

	//! Get emitter type
	virtual E_PARTICLE_AFFECTOR_TYPE getType() const = 0;

protected:
	bool Enabled;
};

} // end namespace scene
} // end namespace irr


#endif

