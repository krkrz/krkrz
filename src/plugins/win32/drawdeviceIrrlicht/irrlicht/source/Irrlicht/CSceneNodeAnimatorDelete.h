// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_SCENE_NODE_ANIMATOR_DELETE_H_INCLUDED__
#define __C_SCENE_NODE_ANIMATOR_DELETE_H_INCLUDED__

#include "ISceneNode.h"

namespace irr
{
namespace scene
{
	class CSceneNodeAnimatorDelete : public ISceneNodeAnimator
	{
	public:

		//! constructor
		CSceneNodeAnimatorDelete(ISceneManager* manager, u32 when);

		//! destructor
		virtual ~CSceneNodeAnimatorDelete();

		//! animates a scene node
		virtual void animateNode(ISceneNode* node, u32 timeMs);

		//! Returns type of the scene node animator
		virtual ESCENE_NODE_ANIMATOR_TYPE getType() const
		{
			return ESNAT_DELETION;
		}

	private:

		u32 DeleteTime;
		ISceneManager* SceneManager;
	};


} // end namespace scene
} // end namespace irr

#endif

