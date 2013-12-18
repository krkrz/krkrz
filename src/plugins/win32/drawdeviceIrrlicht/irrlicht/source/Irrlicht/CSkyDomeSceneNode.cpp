// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
// Code for this scene node has been contributed by Anders la Cour-Harbo (alc)

#include "CSkyDomeSceneNode.h"
#include "IVideoDriver.h"
#include "ISceneManager.h"
#include "ICameraSceneNode.h"
#include "os.h"

namespace irr
{
namespace scene
{

/* horiRes and vertRes:
	Controls the number of faces along the horizontal axis (30 is a good value)
	and the number of faces along the vertical axis (8 is a good value).

	texturePercentage:
	Only the top texturePercentage of the image is used, e.g. 0.8 uses the top 80% of the image,
	1.0 uses the entire image. This is useful as some landscape images have a small banner
	at the bottom that you don't want.

	spherePercentage:
	This controls how far around the sphere the sky dome goes. For value 1.0 you get exactly the upper
	hemisphere, for 1.1 you get slightly more, and for 2.0 you get a full sphere. It is sometimes useful
	to use a value slightly bigger than 1 to avoid a gap between some ground place and the sky. This
	parameters stretches the image to fit the chosen "sphere-size". */

CSkyDomeSceneNode::CSkyDomeSceneNode(video::ITexture* sky, u32 horiRes, u32 vertRes,
			f64 texturePercentage, f64 spherePercentage, ISceneNode* parent, ISceneManager* mgr, s32 id)
			: ISceneNode(parent, mgr, id)
{
	#ifdef _DEBUG
	setDebugName("CSkyDomeSceneNode");
	#endif

	f64 radius = 1000.0; /* Adjust this to get more or less perspective distorsion. */
	f64 azimuth, azimuth_step;
	f64 elevation, elevation_step;
	u32 k;

	video::S3DVertex vtx;

	AutomaticCullingState = scene::EAC_OFF;

	Buffer.Material.Lighting = false;
	Buffer.Material.ZBuffer = false;
	Buffer.Material.ZWriteEnable = false;
	Buffer.Material.setTexture(0, sky);
	Buffer.BoundingBox.MaxEdge.set(0,0,0);
	Buffer.BoundingBox.MinEdge.set(0,0,0);

	azimuth_step = 2.*core::PI64/(f64)horiRes;
	if (spherePercentage<0.)
		spherePercentage=-spherePercentage;
	if (spherePercentage>2.)
		spherePercentage=2.;
	elevation_step = spherePercentage*core::PI64/2./(f64)vertRes;

	Buffer.Vertices.reallocate((horiRes+1)*(vertRes+1));
	Buffer.Indices.reallocate(3*(2*vertRes-1)*horiRes);

	vtx.Color.set(255,255,255,255);
	vtx.Normal.set(0.0f,0.0f,0.0f);

	const f32 tcV = (f32)texturePercentage/(f32)vertRes;
	for (k = 0, azimuth = 0; k <= horiRes; ++k)
	{
		elevation = core::PI64/2.;
		const f32 tcU = (f32)k/(f32)horiRes;
		const f64 sinA = sin(azimuth);
		const f64 cosA = cos(azimuth);
		for (u32 j = 0; j <= vertRes; ++j)
		{
			const f64 cosEr = radius*cos(elevation);
			vtx.Pos.set((f32) (cosEr*sinA),
					(f32) (radius*sin(elevation)+50.0f),
					(f32) (cosEr*cosA));

			vtx.TCoords.set(tcU, (f32)j*tcV);

			Buffer.Vertices.push_back(vtx);
			elevation -= elevation_step;
		}
		azimuth += azimuth_step;
	}

	for (k = 0; k < horiRes; ++k)
	{
		Buffer.Indices.push_back(vertRes+2+(vertRes+1)*k);
		Buffer.Indices.push_back(1+(vertRes+1)*k);
		Buffer.Indices.push_back(0+(vertRes+1)*k);

		for (u32 j = 1; j < vertRes; ++j)
		{
			Buffer.Indices.push_back(vertRes+2+(vertRes+1)*k+j);
			Buffer.Indices.push_back(1+(vertRes+1)*k+j);
			Buffer.Indices.push_back(0+(vertRes+1)*k+j);

			Buffer.Indices.push_back(vertRes+1+(vertRes+1)*k+j);
			Buffer.Indices.push_back(vertRes+2+(vertRes+1)*k+j);
			Buffer.Indices.push_back(0+(vertRes+1)*k+j);
		}
	}
}


//! renders the node.
void CSkyDomeSceneNode::render()
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();
	scene::ICameraSceneNode* camera = SceneManager->getActiveCamera();

	if (!camera || !driver)
		return;

	if ( !camera->isOrthogonal() )
	{
		core::matrix4 mat(AbsoluteTransformation);
		mat.setTranslation(camera->getAbsolutePosition());

		driver->setTransform(video::ETS_WORLD, mat);

		driver->setMaterial(Buffer.Material);
		driver->drawMeshBuffer(&Buffer);
	}
}

//! returns the axis aligned bounding box of this node
const core::aabbox3d<f32>& CSkyDomeSceneNode::getBoundingBox() const
{
	return Buffer.BoundingBox;
}


void CSkyDomeSceneNode::OnRegisterSceneNode()
{
	if (IsVisible)
	{
		SceneManager->registerNodeForRendering(this, ESNRP_SKY_BOX);
		ISceneNode::OnRegisterSceneNode();
	}
}


//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use getMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
video::SMaterial& CSkyDomeSceneNode::getMaterial(u32 i)
{
	return Buffer.Material;
}


//! returns amount of materials used by this scene node.
u32 CSkyDomeSceneNode::getMaterialCount() const
{
	return 1;
}


}
}

