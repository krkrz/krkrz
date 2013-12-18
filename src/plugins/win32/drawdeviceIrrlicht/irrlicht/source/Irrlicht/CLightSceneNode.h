// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_LIGHT_SCENE_NODE_H_INCLUDED__
#define __C_LIGHT_SCENE_NODE_H_INCLUDED__

#include "ILightSceneNode.h"

namespace irr
{
namespace scene
{

//! Scene node which is a dynamic light. You can switch the light on and off by 
//! making it visible or not, and let it be animated by ordinary scene node animators.
class CLightSceneNode : public ILightSceneNode
{
public:

	//! constructor
	CLightSceneNode(ISceneNode* parent, ISceneManager* mgr, s32 id,
		const core::vector3df& position, video::SColorf color, f32 range);

	virtual ~CLightSceneNode() { }

	//! pre render event
	virtual void OnRegisterSceneNode();

	//! render
	virtual void render();

	//! set node light data from light info
	virtual void setLightData(const video::SLight& light);

	//! \return Returns the light data.
	virtual const video::SLight& getLightData() const;

	//! \return Returns the light data.
	virtual video::SLight& getLightData();

	//! returns the axis aligned bounding box of this node
	virtual const core::aabbox3d<f32>& getBoundingBox() const;

	//! Returns type of the scene node
	virtual ESCENE_NODE_TYPE getType() const { return ESNT_LIGHT; }

	//! Writes attributes of the scene node.
	virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const;

	//! Reads attributes of the scene node.
	virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0);

	//! Creates a clone of this scene node and its children.
	virtual ISceneNode* clone(ISceneNode* newParent=0, ISceneManager* newManager=0); 

private:

	video::SLight LightData;
	core::aabbox3d<f32> BBox;
	void doLightRecalc();
};


} // end namespace scene
} // end namespace irr

#endif

