// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_OCT_TREE_TRIANGLE_SELECTOR_H_INCLUDED__
#define __C_OCT_TREE_TRIANGLE_SELECTOR_H_INCLUDED__

#include "CTriangleSelector.h"

namespace irr
{
namespace scene
{

class ISceneNode;

//! Stupid triangle selector without optimization
class COctTreeTriangleSelector : public CTriangleSelector
{
public:

	//! Constructs a selector based on a mesh
	COctTreeTriangleSelector(IMesh* mesh, ISceneNode* node, s32 minimalPolysPerNode);

	~COctTreeTriangleSelector();

	//! Gets all triangles which lie within a specific bounding box.
	void getTriangles(core::triangle3df* triangles, s32 arraySize, s32& outTriangleCount, 
		const core::aabbox3d<f32>& box, const core::matrix4* transform=0) const;

	//! Gets all triangles which have or may have contact with a 3d line.
	virtual void getTriangles(core::triangle3df* triangles, s32 arraySize,
		s32& outTriangleCount, const core::line3d<f32>& line, 
		const core::matrix4* transform=0) const;

private:

	struct SOctTreeNode
	{
		SOctTreeNode()
		{
			for (s32 i=0; i<8; ++i)
				Child[i] = 0;
		}

		~SOctTreeNode()
		{
			for (s32 i=0; i<8; ++i)
				delete Child[i];
		}

		core::array<core::triangle3df> Triangles;
		SOctTreeNode* Child[8];
		core::aabbox3d<f32> Box;
	};


	void constructOctTree(SOctTreeNode* node);
	void deleteEmptyNodes(SOctTreeNode* node);
	void getTrianglesFromOctTree(SOctTreeNode* node, s32& trianglesWritten, s32 maximumSize, 
		const core::aabbox3d<f32>& box, const core::matrix4* transform,
		core::triangle3df* triangles) const;

	SOctTreeNode* Root;
	s32 NodeCount;
	s32 MinimalPolysPerNode;

};

} // end namespace scene
} // end namespace irr


#endif

