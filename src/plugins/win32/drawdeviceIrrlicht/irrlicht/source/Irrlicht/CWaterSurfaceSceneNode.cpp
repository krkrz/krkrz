// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CWaterSurfaceSceneNode.h"
#include "ISceneManager.h"
#include "IMeshManipulator.h"
#include "IMeshCache.h"
#include "S3DVertex.h"
#include "SMesh.h"
#include "os.h"

namespace irr
{
namespace scene
{

//! constructor
CWaterSurfaceSceneNode::CWaterSurfaceSceneNode(f32 waveHeight, f32 waveSpeed, f32 waveLength, 
		IMesh* mesh, ISceneNode* parent, ISceneManager* mgr, s32 id,
		const core::vector3df& position, const core::vector3df& rotation,
		const core::vector3df& scale)
: CMeshSceneNode(mesh, parent, mgr, id, position, rotation, scale),
	WaveLength(waveLength), OneByWaveLength(1.0f/waveLength),
	WaveSpeed(waveSpeed), WaveHeight(waveHeight), OriginalMesh(0)
{
	#ifdef _DEBUG
	setDebugName("CWaterSurfaceSceneNode");
	#endif

	// create copy of the mesh
	if (!mesh)
		return;

	// Mesh is set in CMeshSceneNode constructor, now it is moved to OriginalMesh
	IMesh* clone = SceneManager->getMeshManipulator()->createMeshCopy(mesh);
	OriginalMesh = Mesh;
	Mesh = clone;
}



//! destructor
CWaterSurfaceSceneNode::~CWaterSurfaceSceneNode()
{
	// Mesh is dropped in CMeshSceneNode destructor
	if (OriginalMesh)
		OriginalMesh->drop();
}



//! frame
void CWaterSurfaceSceneNode::OnRegisterSceneNode()
{
	if (IsVisible)
	{
		SceneManager->registerNodeForRendering(this);

		animateWaterSurface();

		CMeshSceneNode::OnRegisterSceneNode();
	}
}



void CWaterSurfaceSceneNode::animateWaterSurface()
{
	if (!Mesh)
		return;

	u32 meshBufferCount = Mesh->getMeshBufferCount();
	f32 time = os::Timer::getTime() / WaveSpeed;

	for (u32 b=0; b<meshBufferCount; ++b)
	{
		const u32 vtxCnt = Mesh->getMeshBuffer(b)->getVertexCount();

		switch(Mesh->getMeshBuffer(b)->getVertexType())
		{
		case video::EVT_STANDARD:
			{
				video::S3DVertex* v =
					(video::S3DVertex*)Mesh->getMeshBuffer(b)->getVertices();

				video::S3DVertex* v2 =
					(video::S3DVertex*)OriginalMesh->getMeshBuffer(b)->getVertices();

				for (u32 i=0; i<vtxCnt; ++i)
				{
					addWave(v[i].Pos, v2[i].Pos, time);
				}

			}
			break;
		case video::EVT_2TCOORDS:
			{
				video::S3DVertex2TCoords* v =
					(video::S3DVertex2TCoords*)Mesh->getMeshBuffer(b)->getVertices();

				video::S3DVertex2TCoords* v2 =
					(video::S3DVertex2TCoords*)OriginalMesh->getMeshBuffer(b)->getVertices();

				for (u32 i=0; i<vtxCnt; ++i)
				{
					addWave(v[i].Pos, v2[i].Pos, time);
				}
			}
			break;
		case video::EVT_TANGENTS:
			{
				video::S3DVertexTangents* v =
					(video::S3DVertexTangents*)Mesh->getMeshBuffer(b)->getVertices();

				video::S3DVertexTangents* v2 =
					(video::S3DVertexTangents*)OriginalMesh->getMeshBuffer(b)->getVertices();

				for (u32 i=0; i<vtxCnt; ++i)
				{
					addWave(v[i].Pos, v2[i].Pos, time);
				}
			}
			break;
		} // end switch
	}// end for all mesh buffers

	SceneManager->getMeshManipulator()->recalculateNormals(Mesh);
}



//! Writes attributes of the scene node.
void CWaterSurfaceSceneNode::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const
{
	out->addFloat("WaveLength", WaveLength);
	out->addFloat("WaveSpeed",  WaveSpeed);
	out->addFloat("WaveHeight", WaveHeight);
	
	CMeshSceneNode::serializeAttributes(out, options);
	// serialize original mesh
	out->setAttribute("Mesh", SceneManager->getMeshCache()->getMeshFilename(OriginalMesh));
}


//! Reads attributes of the scene node.
void CWaterSurfaceSceneNode::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options)
{
	WaveLength = in->getAttributeAsFloat("WaveLength");
	OneByWaveLength = 1.0f/WaveLength;
	WaveSpeed  = in->getAttributeAsFloat("WaveSpeed");
	WaveHeight = in->getAttributeAsFloat("WaveHeight");
	
	if (Mesh)
	{
		Mesh->drop();
		Mesh = OriginalMesh;
		OriginalMesh = 0;
	}
	// deserialize original mesh
	CMeshSceneNode::deserializeAttributes(in, options);

	if (Mesh)
	{
		IMesh* clone = SceneManager->getMeshManipulator()->createMeshCopy(Mesh);
		OriginalMesh = Mesh;
		Mesh = clone;
	}
}

} // end namespace scene
} // end namespace irr

