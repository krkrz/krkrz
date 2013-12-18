// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CLightSceneNode.h"
#include "IVideoDriver.h"
#include "ISceneManager.h"
#include "ICameraSceneNode.h"

#include "os.h"

namespace irr
{
namespace scene
{

//! constructor
CLightSceneNode::CLightSceneNode(ISceneNode* parent, ISceneManager* mgr, s32 id,
		const core::vector3df& position, video::SColorf color, f32 radius)
: ILightSceneNode(parent, mgr, id, position)
{
	#ifdef _DEBUG
	setDebugName("CLightSceneNode");
	#endif

	LightData.Radius = radius;
	LightData.DiffuseColor = color;

	// set some useful specular color
	LightData.SpecularColor = color.getInterpolated(video::SColor(255,255,255,255),0.7f);

	doLightRecalc();
}


//! pre render event
void CLightSceneNode::OnRegisterSceneNode()
{
	doLightRecalc();

	if (IsVisible)
	{
		SceneManager->registerNodeForRendering(this, ESNRP_LIGHT);
		ISceneNode::OnRegisterSceneNode();
	}
}


//! render
void CLightSceneNode::render()
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();
	if (!driver)
		return;

	if ( DebugDataVisible & scene::EDS_BBOX )
	{
		driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
		video::SMaterial m;
		m.Lighting = false;
		driver->setMaterial(m);

		switch ( LightData.Type )
		{
			case video::ELT_POINT:
			case video::ELT_SPOT:
				driver->draw3DBox(BBox, LightData.DiffuseColor.toSColor());
				break;

			case video::ELT_DIRECTIONAL:
				driver->draw3DLine(core::vector3df(0.f, 0.f, 0.f),
						LightData.Direction * LightData.Radius,
						LightData.DiffuseColor.toSColor());
				break;
		}
	}
	driver->addDynamicLight(LightData);
}


//! sets the light data
void CLightSceneNode::setLightData(const video::SLight& light)
{
	LightData = light;
}


//! \return Returns the light data.
const video::SLight& CLightSceneNode::getLightData() const
{
	return LightData;
}


//! \return Returns the light data.
video::SLight& CLightSceneNode::getLightData()
{
	return LightData;
}


//! returns the axis aligned bounding box of this node
const core::aabbox3d<f32>& CLightSceneNode::getBoundingBox() const
{
	return BBox;
}


void CLightSceneNode::doLightRecalc()
{
	if ((LightData.Type == video::ELT_SPOT) || (LightData.Type == video::ELT_DIRECTIONAL))
	{
		LightData.Direction = core::vector3df(.0f,.0f,1.0f);
		getAbsoluteTransformation().rotateVect(LightData.Direction);
		LightData.Direction.normalize();
	}
	if ((LightData.Type == video::ELT_SPOT) || (LightData.Type == video::ELT_POINT))
	{
		const f32 r = LightData.Radius * LightData.Radius * 0.5f;
		BBox.MaxEdge.set( r, r, r );
		BBox.MinEdge.set( -r, -r, -r );
		setAutomaticCulling( scene::EAC_BOX );
		LightData.Position = getAbsolutePosition();
	}
	if (LightData.Type == video::ELT_DIRECTIONAL)
	{
		BBox.reset( 0, 0, 0 );
		setAutomaticCulling( scene::EAC_OFF );
	}
}


//! Writes attributes of the scene node.
void CLightSceneNode::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const
{
	ILightSceneNode::serializeAttributes(out, options);

	out->addColorf	("AmbientColor", LightData.AmbientColor);
	out->addColorf	("DiffuseColor", LightData.DiffuseColor);
	out->addColorf	("SpecularColor", LightData.SpecularColor);
	out->addVector3d("Attenuation", LightData.Attenuation);
	out->addFloat	("Radius", LightData.Radius);
	out->addFloat	("OuterCone", LightData.OuterCone);
	out->addFloat	("InnerCone", LightData.InnerCone);
	out->addFloat	("Falloff", LightData.Falloff);
	out->addBool	("CastShadows", LightData.CastShadows);
	out->addEnum	("LightType", LightData.Type, video::LightTypeNames);
}

//! Reads attributes of the scene node.
void CLightSceneNode::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options)
{
	LightData.AmbientColor =	in->getAttributeAsColorf("AmbientColor");
	LightData.DiffuseColor =	in->getAttributeAsColorf("DiffuseColor");
	LightData.SpecularColor =	in->getAttributeAsColorf("SpecularColor");
	if (in->existsAttribute("Attenuation")) // might not exist in older files
		LightData.Attenuation =	in->getAttributeAsVector3d("Attenuation");
	if (in->existsAttribute("OuterCone")) // might not exist in older files
		LightData.OuterCone =	in->getAttributeAsFloat("OuterCone");
	if (in->existsAttribute("InnerCone")) // might not exist in older files
		LightData.InnerCone =	in->getAttributeAsFloat("InnerCone");
	if (in->existsAttribute("Falloff")) // might not exist in older files
		LightData.Falloff =	in->getAttributeAsFloat("Falloff");
	LightData.Radius =		in->getAttributeAsFloat("Radius");
	LightData.CastShadows =		in->getAttributeAsBool("CastShadows");
	LightData.Type =		(video::E_LIGHT_TYPE)in->getAttributeAsEnumeration("LightType", video::LightTypeNames);

	ILightSceneNode::deserializeAttributes(in, options);
}

//! Creates a clone of this scene node and its children.
ISceneNode* CLightSceneNode::clone(ISceneNode* newParent, ISceneManager* newManager)
{
	if (!newParent)
		newParent = Parent;
	if (!newManager)
		newManager = SceneManager;

	CLightSceneNode* nb = new CLightSceneNode(newParent,
		newManager, ID, RelativeTranslation, LightData.DiffuseColor, LightData.Radius);

	nb->cloneMembers(this, newManager);
	nb->LightData = LightData;
	nb->BBox = BBox;

	nb->drop();
	return nb;
}

} // end namespace scene
} // end namespace irr

