// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#include "CParticleMeshEmitter.h"
#include "os.h"
#include <math.h>

namespace irr
{
namespace scene
{

//! constructor
CParticleMeshEmitter::CParticleMeshEmitter(
	IMesh* mesh, bool useNormalDirection,
	const core::vector3df& direction, f32 normalDirectionModifier,
	s32 mbNumber, bool everyMeshVertex,
	u32 minParticlesPerSecond, u32 maxParticlesPerSecond,
	const video::SColor& minStartColor, const video::SColor& maxStartColor,
	u32 lifeTimeMin, u32 lifeTimeMax, s32 maxAngleDegrees )
	: Mesh(mesh), TotalVertices(0), MBCount(0), MBNumber(mbNumber),
	EveryMeshVertex(everyMeshVertex), UseNormalDirection(useNormalDirection),
	NormalDirectionModifier(normalDirectionModifier), Direction(direction),
	MinParticlesPerSecond(minParticlesPerSecond), MaxParticlesPerSecond(maxParticlesPerSecond),
	MinStartColor(minStartColor), MaxStartColor(maxStartColor),
	MinLifeTime(lifeTimeMin), MaxLifeTime(lifeTimeMax),
	Time(0), Emitted(0), MaxAngleDegrees(maxAngleDegrees)
{
	MBCount = Mesh->getMeshBufferCount();
	for( u32 i = 0; i < MBCount; ++i )
	{
		VertexPerMeshBufferList.push_back( Mesh->getMeshBuffer(i)->getVertexCount() );
		TotalVertices += Mesh->getMeshBuffer(i)->getVertexCount();
	}
}


//! Prepares an array with new particles to emitt into the system
//! and returns how much new particles there are.
s32 CParticleMeshEmitter::emitt(u32 now, u32 timeSinceLastCall, SParticle*& outArray)
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

		if(amount > MaxParticlesPerSecond * 2)
			amount = MaxParticlesPerSecond * 2;

		for(u32 i=0; i<amount; ++i)
		{
			if( EveryMeshVertex )
			{
				for( u32 j=0; j<Mesh->getMeshBufferCount(); ++j )
				{
					for( u32 k=0; k<Mesh->getMeshBuffer(j)->getVertexCount(); ++k )
					{
						switch( Mesh->getMeshBuffer(j)->getVertexType() )
						{
						case video::EVT_STANDARD:
							p.pos = ((video::S3DVertex*)Mesh->getMeshBuffer(j)->getVertices())[k].Pos;
							if( UseNormalDirection )
								p.vector = ((video::S3DVertex*)Mesh->getMeshBuffer(j)->getVertices())[k].Normal / NormalDirectionModifier;
							else
								p.vector = Direction;
							break;
						case video::EVT_TANGENTS:
							p.pos = ((video::S3DVertexTangents*)Mesh->getMeshBuffer(j)->getVertices())[k].Pos;
							if( UseNormalDirection )
								p.vector = ((video::S3DVertexTangents*)Mesh->getMeshBuffer(j)->getVertices())[k].Normal /
									NormalDirectionModifier;
							else
								p.vector = Direction;
							break;
						}

						p.startTime = now;

						if( MaxAngleDegrees )
						{
							core::vector3df tgt = p.vector;
							tgt.rotateXYBy((os::Randomizer::rand()%(MaxAngleDegrees*2)) - MaxAngleDegrees, core::vector3df(0,0,0));
							tgt.rotateYZBy((os::Randomizer::rand()%(MaxAngleDegrees*2)) - MaxAngleDegrees, core::vector3df(0,0,0));
							tgt.rotateXZBy((os::Randomizer::rand()%(MaxAngleDegrees*2)) - MaxAngleDegrees, core::vector3df(0,0,0));
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
				}
			}
			else
			{
				s32 randomMB = 0;

				if( MBNumber < 0 )
				{
					randomMB = os::Randomizer::rand() % MBCount;
				}
				else
				{
					randomMB = MBNumber;
				}

				u32 vertexNumber = Mesh->getMeshBuffer(randomMB)->getVertexCount();
				if (!vertexNumber)
					continue;
				vertexNumber = os::Randomizer::rand() % vertexNumber;

				switch( Mesh->getMeshBuffer(randomMB)->getVertexType() )
				{
				case video::EVT_STANDARD:
					p.pos = ((video::S3DVertex*)Mesh->getMeshBuffer(randomMB)->getVertices())[vertexNumber].Pos;
					if( UseNormalDirection )
						p.vector = ((video::S3DVertex*)Mesh->getMeshBuffer(randomMB)->getVertices())[vertexNumber].Normal /
							NormalDirectionModifier;
					else
						p.vector = Direction;
					break;
				case video::EVT_TANGENTS:
					p.pos = ((video::S3DVertexTangents*)Mesh->getMeshBuffer(randomMB)->getVertices())[vertexNumber].Pos;
					if( UseNormalDirection )
						p.vector = ((video::S3DVertexTangents*)Mesh->getMeshBuffer(randomMB)->getVertices())[vertexNumber].Normal /
							NormalDirectionModifier;
					else
						p.vector = Direction;
					break;
				}

				p.startTime = now;

				if( MaxAngleDegrees )
				{
					core::vector3df tgt = Direction;
					tgt.rotateXYBy((os::Randomizer::rand()%(MaxAngleDegrees*2)) - MaxAngleDegrees, core::vector3df(0,0,0));
					tgt.rotateYZBy((os::Randomizer::rand()%(MaxAngleDegrees*2)) - MaxAngleDegrees, core::vector3df(0,0,0));
					tgt.rotateXZBy((os::Randomizer::rand()%(MaxAngleDegrees*2)) - MaxAngleDegrees, core::vector3df(0,0,0));
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
		}

		outArray = Particles.pointer();

		return Particles.size();
	}

	return 0;
}

//! Set Mesh to emit particles from
void CParticleMeshEmitter::setMesh( IMesh* mesh )
{
	Mesh = mesh;

	TotalVertices = 0;
	MBCount = Mesh->getMeshBufferCount();
	for( u32 i = 0; i < MBCount; ++i )
	{
		VertexPerMeshBufferList.push_back( Mesh->getMeshBuffer(i)->getVertexCount() );
		TotalVertices += Mesh->getMeshBuffer(i)->getVertexCount();
	}
}


} // end namespace scene
} // end namespace irr

