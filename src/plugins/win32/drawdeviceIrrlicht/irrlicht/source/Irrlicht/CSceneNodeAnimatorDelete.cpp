// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CSceneNodeAnimatorDelete.h"
#include "ISceneManager.h"

namespace irr
{
namespace scene
{


//! constructor
CSceneNodeAnimatorDelete::CSceneNodeAnimatorDelete(ISceneManager* manager, u32 time)
: DeleteTime(time), SceneManager(manager)
{
	#ifdef _DEBUG
	setDebugName("CSceneNodeAnimatorDelete");
	#endif
}



//! destructor
CSceneNodeAnimatorDelete::~CSceneNodeAnimatorDelete()
{
}



//! animates a scene node
void CSceneNodeAnimatorDelete::animateNode(ISceneNode* node, u32 timeMs)
{
	if (timeMs > DeleteTime && node && SceneManager)
	{
		// don't delete if scene manager is attached to an editor
		if (!SceneManager->getParameters()->getAttributeAsBool(IRR_SCENE_MANAGER_IS_EDITOR))
			SceneManager->addToDeletionQueue(node);
	}
}


} // end namespace scene
} // end namespace irr

