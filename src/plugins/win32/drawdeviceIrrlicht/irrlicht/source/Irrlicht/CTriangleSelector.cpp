// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CTriangleSelector.h"
#include "ISceneNode.h"
#include "IMeshBuffer.h"

namespace irr
{
namespace scene
{

//! constructor
CTriangleSelector::CTriangleSelector(ISceneNode* node)
: SceneNode(node)
{
	#ifdef _DEBUG
	setDebugName("CTriangleSelector");
	#endif
}


//! constructor
CTriangleSelector::CTriangleSelector(IMesh* mesh, ISceneNode* node)
: SceneNode(node)
{
	#ifdef _DEBUG
	setDebugName("CTriangleSelector");
	#endif

	const u32 cnt = mesh->getMeshBufferCount();
	for (u32 i=0; i<cnt; ++i)
	{
		IMeshBuffer* buf = mesh->getMeshBuffer(i);

		s32 idxCnt = buf->getIndexCount();
		const u16* const indices = buf->getIndices();
		core::triangle3df tri;

		switch (buf->getVertexType())
		{
		case video::EVT_STANDARD:
			{
				video::S3DVertex* vtx = (video::S3DVertex*)buf->getVertices();
				for (s32 j=0; j<idxCnt; j+=3)
				{
					Triangles.push_back(core::triangle3df(
							vtx[indices[j+0]].Pos,
							vtx[indices[j+1]].Pos,
							vtx[indices[j+2]].Pos));
				}
			}
			break;
		case video::EVT_2TCOORDS:
			{
				video::S3DVertex2TCoords* vtx = (video::S3DVertex2TCoords*)buf->getVertices();
				for (s32 j=0; j<idxCnt; j+=3)
				{
					Triangles.push_back(core::triangle3df(
							vtx[indices[j+0]].Pos,
							vtx[indices[j+1]].Pos,
							vtx[indices[j+2]].Pos));
				}
			}
			break;
		case video::EVT_TANGENTS:
			{
				video::S3DVertexTangents* vtx = (video::S3DVertexTangents*)buf->getVertices();
				for (s32 j=0; j<idxCnt; j+=3)
				{
					Triangles.push_back(core::triangle3df(
							vtx[indices[j+0]].Pos,
							vtx[indices[j+1]].Pos,
							vtx[indices[j+2]].Pos));
				}
			}
			break;
		}
	}
}


//! constructor
CTriangleSelector::CTriangleSelector(core::aabbox3d<f32> box, ISceneNode* node)
: SceneNode(node)
{
	#ifdef _DEBUG
	setDebugName("CTriangleSelector");
	#endif

	// TODO
}


//! Gets all triangles.
void CTriangleSelector::getTriangles(core::triangle3df* triangles,
					s32 arraySize, s32& outTriangleCount, 
					const core::matrix4* transform) const
{
	s32 cnt = Triangles.size();
	if (cnt > arraySize)
		cnt = arraySize;

	core::matrix4 mat;

	if (transform)
		mat = *transform;

	if (SceneNode)
		mat *= SceneNode->getAbsoluteTransformation();

	for (s32 i=0; i<cnt; ++i)
	{
		triangles[i] = Triangles[i];
		mat.transformVect(triangles[i].pointA);
		mat.transformVect(triangles[i].pointB);
		mat.transformVect(triangles[i].pointC);
	}

	outTriangleCount = cnt;
}



//! Gets all triangles which lie within a specific bounding box.
void CTriangleSelector::getTriangles(core::triangle3df* triangles, 
					s32 arraySize, s32& outTriangleCount, 
					const core::aabbox3d<f32>& box,
					const core::matrix4* transform) const
{
	// return all triangles
	getTriangles(triangles, arraySize, outTriangleCount, transform);
}


//! Gets all triangles which have or may have contact with a 3d line.
void CTriangleSelector::getTriangles(core::triangle3df* triangles,
					s32 arraySize, s32& outTriangleCount,
					const core::line3d<f32>& line,
					const core::matrix4* transform) const
{
	// return all triangles
	getTriangles(triangles, arraySize, outTriangleCount, transform);
}


//! Returns amount of all available triangles in this selector
s32 CTriangleSelector::getTriangleCount() const
{
	return Triangles.size();
}



} // end namespace scene
} // end namespace irr

