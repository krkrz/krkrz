// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_SCENE_NODE_ANIMATOR_TEXTURE_H_INCLUDED__
#define __C_SCENE_NODE_ANIMATOR_TEXTURE_H_INCLUDED__

#include "irrArray.h"
#include "ISceneNode.h"

namespace irr
{
namespace scene
{
	class CSceneNodeAnimatorTexture : public ISceneNodeAnimator
	{
	public:

		//! constructor
		CSceneNodeAnimatorTexture(const core::array<video::ITexture*>& textures,
			s32 timePerFrame, bool loop, u32 now);

		//! destructor
		virtual ~CSceneNodeAnimatorTexture();

		//! animates a scene node
		virtual void animateNode(ISceneNode* node, u32 timeMs);

		//! Writes attributes of the scene node animator.
		virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const;

		//! Reads attributes of the scene node animator.
		virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0);

		//! Returns type of the scene node animator
		virtual ESCENE_NODE_ANIMATOR_TYPE getType() const { return ESNAT_TEXTURE; }

	private:

		void clearTextures();

		core::array<video::ITexture*> Textures;
		u32 TimePerFrame;
		u32 StartTime;
		u32 EndTime;
		bool Loop;
	};


} // end namespace scene
} // end namespace irr

#endif

