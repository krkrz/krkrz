// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_CAMERA_MAYA_SCENE_NODE_H_INCLUDED__
#define __C_CAMERA_MAYA_SCENE_NODE_H_INCLUDED__

#include "CCameraSceneNode.h"
#include "vector2d.h"

namespace irr
{
namespace scene
{

	class CCameraMayaSceneNode : public CCameraSceneNode
	{
	public:

		//! constructor
		CCameraMayaSceneNode(ISceneNode* parent, ISceneManager* mgr, s32 id,
			f32 rotateSpeed = -1500.0f, f32 zoomSpeed = 200.0f, f32 translationSpeed = 100.0f);

		//! destructor
		virtual ~CCameraMayaSceneNode();

		//! It is possible to send mouse and key events to the camera. Most cameras
		//! may ignore this input, but camera scene nodes which are created for 
		//! example with scene::ISceneManager::addMayaCameraSceneNode or
		//! scene::ISceneManager::addMeshViewerCameraSceneNode, may want to get this input
		//! for changing their position, look at target or whatever. 
		virtual bool OnEvent(const SEvent& event);

		//! OnAnimate() is called just before rendering the whole scene.
		//! nodes may calculate or store animations here, and may do other useful things,
		//! dependent on what they are.
		virtual void OnAnimate(u32 timeMs);

		//! Sets the position of the node. Note that the position is
		//! relative to the parent.
		virtual void setPosition(const core::vector3df& newpos);

		//! Sets the position of the node. Note that the position is
		//! relative to the parent.
		virtual void setTarget(const core::vector3df& newpos);

		//! Sets the rotation speed
		virtual void setRotateSpeed(const f32 speed);

		//! Sets the movement speed
		virtual void setMoveSpeed(const f32 speed);

		//! Gets the rotation speed
		virtual f32 getRotateSpeed();

		// Gets the movement speed
		virtual f32 getMoveSpeed();

		//! Returns type of the scene node
		virtual ESCENE_NODE_TYPE getType() const { return ESNT_CAMERA_MAYA; }

	private:

		void allKeysUp();
		void animate();
		bool isMouseKeyDown(s32 key);
		void updateAnimationState();

		bool MouseKeys[3];

		core::vector3df Pos;
		bool Zooming, Rotating, Moving, Translating;
		f32 ZoomSpeed;
		f32 RotateSpeed;
		f32 TranslateSpeed;
		f32 RotateStartX, RotateStartY;
		f32 ZoomStartX, ZoomStartY;
		f32 TranslateStartX, TranslateStartY;
		f32 CurrentZoom;
		f32 RotX, RotY;
		core::vector3df OldTarget;

		core::vector2df MousePos;
	};

} // end namespace
} // end namespace

#endif

