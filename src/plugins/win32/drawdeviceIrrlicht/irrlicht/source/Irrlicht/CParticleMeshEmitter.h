// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_PARTICLE_MESH_EMITTER_H_INCLUDED__
#define __C_PARTICLE_MESH_EMITTER_H_INCLUDED__

#include "IParticleMeshEmitter.h"
#include "irrArray.h"
#include "aabbox3d.h"
#include "IMeshBuffer.h"

namespace irr
{
namespace scene
{

//! A default box emitter
class CParticleMeshEmitter : public IParticleMeshEmitter
{
public:

	//! constructor
	CParticleMeshEmitter(
		IMesh* mesh, bool useNormalDirection = true,
		const core::vector3df& direction = core::vector3df(0.0f,0.0f,0.0f),
		f32 normalDirectionModifier = 100.0f,
		s32 mbNumber = -1,
		bool everyMeshVertex = false,
		u32 minParticlesPerSecond = 20,
		u32 maxParticlesPerSecond = 40,
		const video::SColor& minStartColor = video::SColor(255,0,0,0),
		const video::SColor& maxStartColor = video::SColor(255,255,255,255),
		u32 lifeTimeMin = 2000,
		u32 lifeTimeMax = 4000,
		s32 maxAngleDegrees = 0
	);

	//! Prepares an array with new particles to emitt into the system
	//! and returns how much new particles there are.
	virtual s32 emitt(u32 now, u32 timeSinceLastCall, SParticle*& outArray);

	//! Set Mesh to emit particles from
	virtual void setMesh( IMesh* mesh );

	//! Set whether to use vertex normal for direction, or direction specified
	virtual void setUseNormalDirection( bool useNormalDirection ) { UseNormalDirection = useNormalDirection; }

	//! Set direction the emitter emits particles
	virtual void setDirection( const core::vector3df& newDirection ) { Direction = newDirection; }

	//! Set the amount that the normal is divided by for getting a particles direction
	virtual void setNormalDirectionModifier( f32 normalDirectionModifier ) { NormalDirectionModifier = normalDirectionModifier; }

	//! Sets whether to emit min<->max particles for every vertex per second, or to pick
	//! min<->max vertices every second
	virtual void setEveryMeshVertex( bool everyMeshVertex ) { EveryMeshVertex = everyMeshVertex; }

	//! Set minimum number of particles the emitter emits per second
	virtual void setMinParticlesPerSecond( u32 minPPS ) { MinParticlesPerSecond = minPPS; }

	//! Set maximum number of particles the emitter emits per second
	virtual void setMaxParticlesPerSecond( u32 maxPPS ) { MaxParticlesPerSecond = maxPPS; }

	//! Set minimum starting color for particles
	virtual void setMinStartColor( const video::SColor& color ) { MinStartColor = color; }

	//! Set maximum starting color for particles
	virtual void setMaxStartColor( const video::SColor& color ) { MaxStartColor = color; }

	//! Get Mesh we're emitting particles from
	virtual const IMesh* getMesh() const { return Mesh; }

	//! Get whether to use vertex normal for direciton, or direction specified
	virtual bool isUsingNormalDirection() const { return UseNormalDirection; }

	//! Get direction the emitter emits particles
	virtual const core::vector3df& getDirection() const { return Direction; }

	//! Get the amount that the normal is divided by for getting a particles direction
	virtual f32 getNormalDirectionModifier() const { return NormalDirectionModifier; }

	//! Gets whether to emit min<->max particles for every vertex per second, or to pick
	//! min<->max vertices every second
	virtual bool getEveryMeshVertex() const { return EveryMeshVertex; }

	//! Get the minimum number of particles the emitter emits per second
	virtual u32 getMinParticlesPerSecond() const { return MinParticlesPerSecond; }

	//! Get the maximum number of particles the emitter emits per second
	virtual u32 getMaxParticlesPerSecond() const { return MaxParticlesPerSecond; }

	//! Get the minimum starting color for particles
	virtual const video::SColor& getMinStartColor() const { return MinStartColor; }

	//! Get the maximum starting color for particles
	virtual const video::SColor& getMaxStartColor() const { return MaxStartColor; }

private:

	const IMesh* Mesh;
	s32 TotalVertices;
	u32 MBCount;
	s32 MBNumber;
	core::array<s32>	VertexPerMeshBufferList;

	bool EveryMeshVertex;
	bool UseNormalDirection;
	f32 NormalDirectionModifier;
	core::array<SParticle> Particles;
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


#endif // __C_PARTICLE_MESH_EMITTER_H_INCLUDED__

