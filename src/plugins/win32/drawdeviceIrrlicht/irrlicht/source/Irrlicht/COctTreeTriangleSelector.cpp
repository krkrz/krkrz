// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "COctTreeTriangleSelector.h"
#include "ISceneNode.h"

#include "os.h"

namespace irr
{
namespace scene
{

//! constructor
COctTreeTriangleSelector::COctTreeTriangleSelector(IMesh* mesh,
		ISceneNode* node, s32 minimalPolysPerNode)
	: CTriangleSelector(mesh, node), Root(0), NodeCount(0),
	 MinimalPolysPerNode(minimalPolysPerNode)
{
	#ifdef _DEBUG
	setDebugName("COctTreeTriangleSelector");
	#endif
	
	if (!Triangles.empty())
	{
		u32 start = os::Timer::getRealTime();

		// create the triangle octtree
		Root = new SOctTreeNode();
		Root->Triangles = Triangles;
		constructOctTree(Root);

		u32 end = os::Timer::getRealTime();
		c8 tmp[255];
		sprintf(tmp, "Needed %ums to create OctTreeTriangleSelector.(%d nodes, %u polys)", 
			end - start, NodeCount, Triangles.size());
		os::Printer::log(tmp, ELL_INFORMATION);
	}
}


//! destructor
COctTreeTriangleSelector::~COctTreeTriangleSelector()
{
	delete Root;
}


void COctTreeTriangleSelector::constructOctTree(SOctTreeNode* node)
{
	++NodeCount;

	node->Box.reset(node->Triangles[0].pointA);

	// get bounding box
	const u32 cnt = node->Triangles.size();
	for (u32 i=0; i<cnt; ++i)
	{
		node->Box.addInternalPoint(node->Triangles[i].pointA);
		node->Box.addInternalPoint(node->Triangles[i].pointB);
		node->Box.addInternalPoint(node->Triangles[i].pointC);
	}

	const core::vector3df& middle = node->Box.getCenter();
	core::vector3df edges[8];
	node->Box.getEdges(edges);

	core::aabbox3d<f32> box;
	core::array<core::triangle3df> keepTriangles;

	// calculate children

	if (!node->Box.isEmpty() && (s32)node->Triangles.size() > MinimalPolysPerNode)
	for (s32 ch=0; ch<8; ++ch)
	{
		box.reset(middle);
		box.addInternalPoint(edges[ch]);
		node->Child[ch] = new SOctTreeNode();

		for (s32 i=0; i<(s32)node->Triangles.size(); ++i)
		{
			if (node->Triangles[i].isTotalInsideBox(box))
			{
				node->Child[ch]->Triangles.push_back(node->Triangles[i]);
				//node->Triangles.erase(i);
				//--i;
			}
			else
			{
				keepTriangles.push_back(node->Triangles[i]);
			}
		}
		memcpy(node->Triangles.pointer(), keepTriangles.pointer(), 
			sizeof(core::triangle3df)*keepTriangles.size());

		node->Triangles.set_used(keepTriangles.size());
		keepTriangles.set_used(0);

		if (node->Child[ch]->Triangles.empty())
		{
			delete node->Child[ch];
			node->Child[ch] = 0;
		}
		else
			constructOctTree(node->Child[ch]);
	}
}



//! Gets all triangles which lie within a specific bounding box.
void COctTreeTriangleSelector::getTriangles(core::triangle3df* triangles, 
					s32 arraySize, s32& outTriangleCount, 
					const core::aabbox3d<f32>& box,
					const core::matrix4* transform) const
{
	core::matrix4 mat;
	core::aabbox3d<f32> invbox = box;

	if (SceneNode)
	{
		mat = SceneNode->getAbsoluteTransformation();
		mat.makeInverse();
		mat.transformBox(invbox);
	}

	mat.makeIdentity();

	if (transform)
		mat = *transform;

	if (SceneNode)
		mat *= SceneNode->getAbsoluteTransformation();

	s32 trianglesWritten = 0;

	if (Root)
		getTrianglesFromOctTree(Root, trianglesWritten,
			arraySize, invbox, &mat, triangles);

	outTriangleCount = trianglesWritten;
}


void COctTreeTriangleSelector::getTrianglesFromOctTree(
		SOctTreeNode* node, s32& trianglesWritten,
		s32 maximumSize, const core::aabbox3d<f32>& box,
		const core::matrix4* mat, core::triangle3df* triangles) const
{
	if (!box.intersectsWithBox(node->Box))
		return;

	s32 cnt = node->Triangles.size();
	if (cnt + trianglesWritten > maximumSize)
		cnt -= cnt + trianglesWritten - maximumSize;

	s32 i;
	
	for (i=0; i<cnt; ++i)
	{
		triangles[trianglesWritten] = node->Triangles[i];
		mat->transformVect(triangles[trianglesWritten].pointA);
		mat->transformVect(triangles[trianglesWritten].pointB);
		mat->transformVect(triangles[trianglesWritten].pointC);
		++trianglesWritten;
	}

	for (i=0; i<8; ++i)
		if (node->Child[i])
			getTrianglesFromOctTree(node->Child[i], trianglesWritten,
			maximumSize, box, mat, triangles);
}


//! Gets all triangles which have or may have contact with a 3d line.
void COctTreeTriangleSelector::getTriangles(core::triangle3df* triangles, s32 arraySize,
		s32& outTriangleCount, const core::line3d<f32>& line, 
		const core::matrix4* transform) const
{
	core::aabbox3d<f32> box(line.start);
	box.addInternalPoint(line.end);

	// TODO: Could be optimized for line a little bit more.
	COctTreeTriangleSelector::getTriangles(triangles, arraySize, outTriangleCount,
		box, transform);
}


} // end namespace scene
} // end namespace irr

