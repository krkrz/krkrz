// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CMeshSceneNode.h"
#include "IVideoDriver.h"
#include "ISceneManager.h"
#include "S3DVertex.h"
#include "ICameraSceneNode.h"
#include "IMeshCache.h"
#include "IAnimatedMesh.h"
#include "IMaterialRenderer.h"

namespace irr
{
namespace scene
{



//! constructor
CMeshSceneNode::CMeshSceneNode(IMesh* mesh, ISceneNode* parent, ISceneManager* mgr, s32 id,
			const core::vector3df& position, const core::vector3df& rotation,
			const core::vector3df& scale)
: IMeshSceneNode(parent, mgr, id, position, rotation, scale), Mesh(0), PassCount(0),
	ReadOnlyMaterials(false)
{
	#ifdef _DEBUG
	setDebugName("CMeshSceneNode");
	#endif

	setMesh(mesh);
}



//! destructor
CMeshSceneNode::~CMeshSceneNode()
{
	if (Mesh)
		Mesh->drop();
}



//! frame
void CMeshSceneNode::OnRegisterSceneNode()
{
	if (IsVisible)
	{
		// because this node supports rendering of mixed mode meshes consisting of 
		// transparent and solid material at the same time, we need to go through all 
		// materials, check of what type they are and register this node for the right
		// render pass according to that.

		video::IVideoDriver* driver = SceneManager->getVideoDriver();

		PassCount = 0;
		int transparentCount = 0;
		int solidCount = 0;

		// count transparent and solid materials in this scene node
		if (ReadOnlyMaterials && Mesh)
		{
			// count mesh materials 

			for (u32 i=0; i<Mesh->getMeshBufferCount(); ++i)
			{
				scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
				video::IMaterialRenderer* rnd = mb ? driver->getMaterialRenderer(mb->getMaterial().MaterialType) : 0;

				if (rnd && rnd->isTransparent()) 
					++transparentCount;
				else 
					++solidCount;

				if (solidCount && transparentCount)
					break;
			}
		}
		else
		{
			// count copied materials 

			for (u32 i=0; i<Materials.size(); ++i)
			{
				video::IMaterialRenderer* rnd = 
					driver->getMaterialRenderer(Materials[i].MaterialType);

				if (rnd && rnd->isTransparent()) 
					++transparentCount;
				else 
					++solidCount;

				if (solidCount && transparentCount)
					break;
			}	
		}

		// register according to material types counted

		if (solidCount)
			SceneManager->registerNodeForRendering(this, scene::ESNRP_SOLID);

		if (transparentCount)
			SceneManager->registerNodeForRendering(this, scene::ESNRP_TRANSPARENT);

		ISceneNode::OnRegisterSceneNode();
	}
}



//! renders the node.
void CMeshSceneNode::render()
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();

	if (!Mesh || !driver)
		return;

	bool isTransparentPass = 
		SceneManager->getSceneNodeRenderPass() == scene::ESNRP_TRANSPARENT;

	++PassCount;

	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
	Box = Mesh->getBoundingBox();

	// for debug purposes only:
	if ( DebugDataVisible && PassCount==1)
	{
		if ( DebugDataVisible & scene::EDS_BBOX )
		{
			video::SMaterial m;
			m.Lighting = false;
			driver->setMaterial(m);
			driver->draw3DBox(Box, video::SColor(0,255,255,255));
		}
		if ( DebugDataVisible & scene::EDS_BBOX_BUFFERS )
		{
			video::SMaterial m;
			m.Lighting = false;
			driver->setMaterial(m);
			for (u32 g=0; g<Mesh->getMeshBufferCount(); ++g)
			{
				driver->draw3DBox(
					Mesh->getMeshBuffer(g)->getBoundingBox(), 
					video::SColor(0,255,255,255));
			}
		}
		if ( DebugDataVisible & scene::EDS_NORMALS )
		{
			for (u32 g=0; g<Mesh->getMeshBufferCount(); ++g)
			{
				scene::IMeshBuffer* mb = Mesh->getMeshBuffer(g);

				u32 vSize;
				u32 i;
				vSize = video::getVertexPitchFromType(mb->getVertexType());

				const video::S3DVertex* v = ( const video::S3DVertex*)mb->getVertices();
				video::SColor c ( 255, 128 ,0, 0 );
				video::SColor c1 ( 255, 255 ,0, 0 );
				for ( i = 0; i != mb->getVertexCount(); ++i )
				{
					core::vector3df h = v->Normal * 5.f;
					core::vector3df h1 = h.crossProduct ( core::vector3df ( 0.f, 1.f, 0.f ) );

					driver->draw3DLine ( v->Pos, v->Pos + h, c );
					driver->draw3DLine ( v->Pos + h, v->Pos + h + h1, c1 );
					v = (const video::S3DVertex*) ( (u8*) v + vSize );
				}
			}
		}
	}

	for (u32 i=0; i<Mesh->getMeshBufferCount(); ++i)
	{
		scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
		if (mb)
		{
			const video::SMaterial& material = ReadOnlyMaterials ? mb->getMaterial() : Materials[i];

			video::IMaterialRenderer* rnd = driver->getMaterialRenderer(material.MaterialType);
			bool transparent = (rnd && rnd->isTransparent());

			// only render transparent buffer if this is the transparent render pass
			// and solid only in solid pass
			if (transparent == isTransparentPass) 
			{
				driver->setMaterial(material);
				driver->drawMeshBuffer(mb);
			}
		}
	}
}


//! returns the axis aligned bounding box of this node
const core::aabbox3d<f32>& CMeshSceneNode::getBoundingBox() const
{
	return Mesh ? Mesh->getBoundingBox() : Box;
}


//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use getMaterialCount().
//! This function is needed for inserting the node into the scene hierarchy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
video::SMaterial& CMeshSceneNode::getMaterial(u32 i)
{
	if (Mesh && ReadOnlyMaterials && i<Mesh->getMeshBufferCount())
	{
		tmpReadOnlyMaterial = Mesh->getMeshBuffer(i)->getMaterial();
		return tmpReadOnlyMaterial;
	}

	if ( i >= Materials.size())
		return ISceneNode::getMaterial(i);

	return Materials[i];
}



//! returns amount of materials used by this scene node.
u32 CMeshSceneNode::getMaterialCount() const
{
	if (Mesh && ReadOnlyMaterials)
		return Mesh->getMeshBufferCount();

	return Materials.size();
}



//! Sets a new mesh
void CMeshSceneNode::setMesh(IMesh* mesh)
{
	if (!mesh)
		return; // won't set null mesh

	if (Mesh)
		Mesh->drop();

	Mesh = mesh;
	copyMaterials();

	if (Mesh)
		Mesh->grab();
}


void CMeshSceneNode::copyMaterials()
{
	Materials.clear();

	if (Mesh)
	{
		video::SMaterial mat;

		for (u32 i=0; i<Mesh->getMeshBufferCount(); ++i)
		{
			IMeshBuffer* mb = Mesh->getMeshBuffer(i);
			if (mb)
				mat = mb->getMaterial();

			Materials.push_back(mat);
		}
	}
}


//! Writes attributes of the scene node.
void CMeshSceneNode::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const
{
	IMeshSceneNode::serializeAttributes(out, options);

	out->addString("Mesh", SceneManager->getMeshCache()->getMeshFilename(Mesh));
	out->addBool("ReadOnlyMaterials", ReadOnlyMaterials);
}

//! Reads attributes of the scene node.
void CMeshSceneNode::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options)
{
	core::stringc oldMeshStr = SceneManager->getMeshCache()->getMeshFilename(Mesh);
	core::stringc newMeshStr = in->getAttributeAsString("Mesh");
	ReadOnlyMaterials = in->getAttributeAsBool("ReadOnlyMaterials");

	if (newMeshStr != "" && oldMeshStr != newMeshStr)
	{
		IMesh* newMesh = 0;
		IAnimatedMesh* newAnimatedMesh = SceneManager->getMesh(newMeshStr.c_str());

		if (newAnimatedMesh)
			newMesh = newAnimatedMesh->getMesh(0);

		if (newMesh)
			setMesh(newMesh);
	}

	IMeshSceneNode::deserializeAttributes(in, options);
}

//! Sets if the scene node should not copy the materials of the mesh but use them in a read only style.
/* In this way it is possible to change the materials a mesh causing all mesh scene nodes 
referencing this mesh to change too. */
void CMeshSceneNode::setReadOnlyMaterials(bool readonly)
{
	ReadOnlyMaterials = readonly;
}

//! Returns if the scene node should not copy the materials of the mesh but use them in a read only style
bool CMeshSceneNode::isReadOnlyMaterials() const
{
	return ReadOnlyMaterials;
}


//! Creates a clone of this scene node and its children.
ISceneNode* CMeshSceneNode::clone(ISceneNode* newParent, ISceneManager* newManager)
{
	if (!newParent) newParent = Parent;
	if (!newManager) newManager = SceneManager;

	CMeshSceneNode* nb = new CMeshSceneNode(Mesh, newParent, 
		newManager, ID, RelativeTranslation, RelativeRotation, RelativeScale);

	nb->cloneMembers(this, newManager);
	nb->ReadOnlyMaterials = ReadOnlyMaterials;
	nb->Materials = Materials;

	nb->drop();
	return nb;
}



} // end namespace scene
} // end namespace irr

