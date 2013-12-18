// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_LIGHT_SCENE_NODE_H_INCLUDED__
#define __I_LIGHT_SCENE_NODE_H_INCLUDED__

#include "ISceneNode.h"
#include "SLight.h"

namespace irr
{
namespace scene
{

//! Scene node which is a dynamic light.
/** You can switch the light on and off by making it visible or not. It can be
animated by ordinary scene node animators. If the light type is directional or
spot, the direction of the light source is defined by the rotation of the scene
node (assuming (0,0,1) as the local direction of the light).
*/
class ILightSceneNode : public ISceneNode
{
public:

	//! constructor
	ILightSceneNode(ISceneNode* parent, ISceneManager* mgr, s32 id,
		const core::vector3df& position = core::vector3df(0,0,0))
		: ISceneNode(parent, mgr, id, position) {}

	//! Sets the light data associated with this ILightSceneNode
	/** \param light The new light data. */
	virtual void setLightData(const video::SLight& light) = 0;

	//! Gets the light data associated with this ILightSceneNode
	/** \return The light data. */
	virtual const video::SLight& getLightData() const = 0;

	//! Gets the light data associated with this ILightSceneNode
	/** \return The light data. */
	virtual video::SLight& getLightData() = 0;
};

} // end namespace scene
} // end namespace irr


#endif

