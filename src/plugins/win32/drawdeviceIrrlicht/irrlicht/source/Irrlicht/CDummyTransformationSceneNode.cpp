// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CDummyTransformationSceneNode.h"

namespace irr
{
namespace scene
{

//! constructor
CDummyTransformationSceneNode::CDummyTransformationSceneNode(
	ISceneNode* parent, ISceneManager* mgr, s32 id)
	: IDummyTransformationSceneNode(parent, mgr, id)
{
	#ifdef _DEBUG
	setDebugName("CDummyTransformationSceneNode");
	#endif

	setAutomaticCulling(scene::EAC_OFF);
}



//! destructor
CDummyTransformationSceneNode::~CDummyTransformationSceneNode()
{
}



//! returns the axis aligned bounding box of this node
const core::aabbox3d<f32>& CDummyTransformationSceneNode::getBoundingBox() const
{
	return Box;
}



//! Returns a reference to the current relative transformation matrix.
//! This is the matrix, this scene node uses instead of scale, translation
//! and rotation.
core::matrix4& CDummyTransformationSceneNode::getRelativeTransformationMatrix()
{
	return RelativeTransformationMatrix;
}


//! Returns the relative transformation of the scene node.
core::matrix4 CDummyTransformationSceneNode::getRelativeTransformation() const
{
	return RelativeTransformationMatrix;
}


} // end namespace scene
} // end namespace irr

