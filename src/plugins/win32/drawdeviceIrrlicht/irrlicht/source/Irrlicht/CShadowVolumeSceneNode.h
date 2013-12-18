// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_SHADOW_VOLUME_SCENE_NODE_H_INCLUDED__
#define __C_SHADOW_VOLUME_SCENE_NODE_H_INCLUDED__

#include "IShadowVolumeSceneNode.h"

namespace irr
{
namespace scene
{

	//! Scene node for rendering a shadow volume into a stencil buffer.
	class CShadowVolumeSceneNode : public IShadowVolumeSceneNode
	{
	public:

		//! constructor
		CShadowVolumeSceneNode(ISceneNode* parent, ISceneManager* mgr, 
			s32 id,  bool zfailmethod=true, f32 infinity=10000.0f);

		//! destructor
		virtual ~CShadowVolumeSceneNode();

		//! sets the mesh from which the shadow volume should be generated.
		virtual void setMeshToRenderFrom(const IMesh* mesh);

		//! pre render method
		virtual void OnRegisterSceneNode();

		//! renders the node.
		virtual void render();

		//! returns the axis aligned bounding box of this node
		virtual const core::aabbox3d<f32>& getBoundingBox() const;

		//! Returns type of the scene node
		virtual ESCENE_NODE_TYPE getType() const { return ESNT_SHADOW_VOLUME; }

	private:

		struct SShadowVolume
		{
			core::vector3df* vertices;
			s32 count;
			s32 size;
		};

		void createShadowVolume(const core::vector3df& pos);
		void createZPassVolume(s32 faceCount, s32& numEdges, core::vector3df light, SShadowVolume* svp, bool caps);
		void createZFailVolume(s32 faceCount, s32& numEdges, const core::vector3df& light, SShadowVolume* svp);
		void addEdge(s32& numEdges, u16 v0, u16 v1);

		//! Generates adjacency information based on mesh indices.
		void calculateAdjacency(f32 epsilon=0.0001f);

		core::aabbox3d<f32> Box;

		u16* Indices;
		core::vector3df* Vertices;
		u16* Adjacency;
		bool* FaceData; // used for zfail method, if face is front facing
		bool UseZFailMethod;

		s32 IndexCountAllocated;
		s32 VertexCountAllocated;
		s32 IndexCount;
		s32 VertexCount;

		core::array<SShadowVolume> ShadowVolumes; // a shadow volume for every light
		s32 ShadowVolumesUsed;

		u16* Edges;
		s32 EdgeCount;

		f32 Infinity;
	};

} // end namespace scene
} // end namespace irr

#endif

