// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_OCT_TREE_SCENE_NODE_H_INCLUDED__
#define __C_OCT_TREE_SCENE_NODE_H_INCLUDED__

#include "ISceneNode.h"
#include "IMesh.h"
#include "OctTree.h"

namespace irr
{
namespace scene
{
	//! implementation of the IBspTreeSceneNode
	class COctTreeSceneNode : public ISceneNode
	{
	public:

		//! constructor
		COctTreeSceneNode(ISceneNode* parent, ISceneManager* mgr, s32 id, 
			s32 minimalPolysPerNode=128);

		//! destructor
		virtual ~COctTreeSceneNode();

		virtual void OnRegisterSceneNode();

		//! renders the node.
		virtual void render();

		//! returns the axis aligned bounding box of this node
		virtual const core::aabbox3d<f32>& getBoundingBox() const;

		//! creates the tree
		bool createTree(IMesh* mesh);

		//! returns the material based on the zero based index i. To get the amount
		//! of materials used by this scene node, use getMaterialCount().
		//! This function is needed for inserting the node into the scene hirachy on a
		//! optimal position for minimizing renderstate changes, but can also be used
		//! to directly modify the material of a scene node.
		virtual video::SMaterial& getMaterial(u32 i);
		
		//! returns amount of materials used by this scene node.
		virtual u32 getMaterialCount() const;

		//! Writes attributes of the scene node.
		virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const;

		//! Reads attributes of the scene node.
		virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0);

		//! Returns type of the scene node
		virtual ESCENE_NODE_TYPE getType() const { return ESNT_OCT_TREE; }

	private:

		void deleteTree();

		core::aabbox3d<f32> Box;

		OctTree<video::S3DVertex>* StdOctTree;
		core::array< OctTree<video::S3DVertex>::SMeshChunk > StdMeshes;

		OctTree<video::S3DVertex2TCoords>* LightMapOctTree;
		core::array< OctTree<video::S3DVertex2TCoords>::SMeshChunk > LightMapMeshes;

		video::E_VERTEX_TYPE vertexType;
		core::array< video::SMaterial > Materials;

		//IMesh* Mesh;
		core::stringc MeshName;
		s32 MinimalPolysPerNode;
		s32 PassCount;
	};

} // end namespace scene
} // end namespace irr

#endif

