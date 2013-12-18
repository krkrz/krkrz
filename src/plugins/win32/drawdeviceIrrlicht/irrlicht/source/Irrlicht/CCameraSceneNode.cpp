// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CCameraSceneNode.h"
#include "ISceneManager.h"
#include "IVideoDriver.h"
#include "os.h"

namespace irr
{
namespace scene
{


//! constructor
CCameraSceneNode::CCameraSceneNode(ISceneNode* parent, ISceneManager* mgr, s32 id, 
	const core::vector3df& position, const core::vector3df& lookat)
	: ICameraSceneNode(parent, mgr, id, position, core::vector3df(0.0f, 0.0f, 0.0f),
			core::vector3df(1.0f, 1.0f, 1.0f)), InputReceiverEnabled(true)
{
	#ifdef _DEBUG
	setDebugName("CCameraSceneNode");
	#endif

	// set default view

	UpVector.set(0.0f, 1.0f, 0.0f);
	Target.set(lookat);

	// set default projection

	Fovy = core::PI / 2.5f;	// Field of view, in radians. 
	Aspect = 4.0f / 3.0f;	// Aspect ratio. 
	ZNear = 1.0f;		// value of the near view-plane. 
	ZFar = 3000.0f;		// Z-value of the far view-plane. 

	video::IVideoDriver* d = mgr->getVideoDriver();
	if (d)
		Aspect = (f32)d->getCurrentRenderTargetSize().Width /
			(f32)d->getCurrentRenderTargetSize().Height;

	recalculateProjectionMatrix();
	recalculateViewArea();
}



//! destructor
CCameraSceneNode::~CCameraSceneNode()
{
}


//! Disables or enables the camera to get key or mouse inputs.
void CCameraSceneNode::setInputReceiverEnabled(bool enabled)
{
	InputReceiverEnabled = enabled;
}


//! Returns if the input receiver of the camera is currently enabled.
bool CCameraSceneNode::isInputReceiverEnabled() const
{
	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return InputReceiverEnabled;
}


//! Sets the projection matrix of the camera. The core::matrix4 class has some methods
//! to build a projection matrix. e.g: core::matrix4::buildProjectionMatrixPerspectiveFovLH
//! \param projection: The new projection matrix of the camera. 
void CCameraSceneNode::setProjectionMatrix(const core::matrix4& projection)
{
	ViewArea.Matrices [ video::ETS_PROJECTION ] = projection;
	ViewArea.setTransformState ( video::ETS_PROJECTION );
}



//! Gets the current projection matrix of the camera
//! \return Returns the current projection matrix of the camera.
const core::matrix4& CCameraSceneNode::getProjectionMatrix() const
{
	return ViewArea.Matrices [ video::ETS_PROJECTION ];
}



//! Gets the current view matrix of the camera
//! \return Returns the current view matrix of the camera.
const core::matrix4& CCameraSceneNode::getViewMatrix() const
{
	return ViewArea.Matrices [ video::ETS_VIEW ];
}



//! It is possible to send mouse and key events to the camera. Most cameras
//! may ignore this input, but camera scene nodes which are created for 
//! example with scene::ISceneManager::addMayaCameraSceneNode or
//! scene::ISceneManager::addFPSCameraSceneNode, may want to get this input
//! for changing their position, look at target or whatever. 
bool CCameraSceneNode::OnEvent(const SEvent& event)
{
	return false;
}



//! sets the look at target of the camera
//! \param pos: Look at target of the camera.
void CCameraSceneNode::setTarget(const core::vector3df& pos)
{
	Target = pos;
}



//! Gets the current look at target of the camera
//! \return Returns the current look at target of the camera
core::vector3df CCameraSceneNode::getTarget() const
{
	return Target;
}



//! sets the up vector of the camera
//! \param pos: New upvector of the camera.
void CCameraSceneNode::setUpVector(const core::vector3df& pos)
{
	UpVector = pos;
}



//! Gets the up vector of the camera.
//! \return Returns the up vector of the camera.
core::vector3df CCameraSceneNode::getUpVector() const
{
	return UpVector;
}


f32 CCameraSceneNode::getNearValue() const 
{
	return ZNear;
}

f32 CCameraSceneNode::getFarValue() const 
{
	return ZFar;
}

f32 CCameraSceneNode::getAspectRatio() const 
{
	return Aspect;
}

f32 CCameraSceneNode::getFOV() const 
{
	return Fovy;
}

void CCameraSceneNode::setNearValue(f32 f)
{
	ZNear = f;
	recalculateProjectionMatrix();
}

void CCameraSceneNode::setFarValue(f32 f)
{
	ZFar = f;
	recalculateProjectionMatrix();
}

void CCameraSceneNode::setAspectRatio(f32 f)
{
	Aspect = f;
	recalculateProjectionMatrix();
}

void CCameraSceneNode::setFOV(f32 f)
{
	Fovy = f;
	recalculateProjectionMatrix();
}

void CCameraSceneNode::recalculateProjectionMatrix()
{
	ViewArea.Matrices [ video::ETS_PROJECTION ].buildProjectionMatrixPerspectiveFovLH(Fovy, Aspect, ZNear, ZFar);
	ViewArea.setTransformState ( video::ETS_PROJECTION );
}


//! prerender
void CCameraSceneNode::OnRegisterSceneNode()
{
	// if upvector and vector to the target are the same, we have a
	// problem. so solve this problem:

	core::vector3df pos = getAbsolutePosition();
	core::vector3df tgtv = Target - pos;
	tgtv.normalize();

	core::vector3df up = UpVector;
	up.normalize();

	f32 dp = tgtv.dotProduct(up);

	if ( core::equals ( fabs ( dp ), 1.f ) )
	{
		up.X += 0.5f;
	}

	ViewArea.Matrices [ video::ETS_VIEW ].buildCameraLookAtMatrixLH(pos, Target, up);
	ViewArea.setTransformState ( video::ETS_VIEW );
	recalculateViewArea();

	if ( SceneManager->getActiveCamera () == this )
		SceneManager->registerNodeForRendering(this, ESNRP_CAMERA);

	if (IsVisible)
		ISceneNode::OnRegisterSceneNode();
}



//! render
void CCameraSceneNode::render()
{	
	video::IVideoDriver* driver = SceneManager->getVideoDriver();
	if ( driver)
	{
		driver->setTransform(video::ETS_PROJECTION, ViewArea.Matrices [ video::ETS_PROJECTION ] );
		driver->setTransform(video::ETS_VIEW, ViewArea.Matrices [ video::ETS_VIEW ] );
	}
}


//! returns the axis aligned bounding box of this node
const core::aabbox3d<f32>& CCameraSceneNode::getBoundingBox() const
{
	return ViewArea.getBoundingBox();
}



//! returns the view frustum. needed sometimes by bsp or lod render nodes.
const SViewFrustum* CCameraSceneNode::getViewFrustum() const
{
	return &ViewArea;
}

core::vector3df CCameraSceneNode::getAbsolutePosition() const
{
	return AbsoluteTransformation.getTranslation();
}

void CCameraSceneNode::recalculateViewArea()
{
	ViewArea.cameraPosition = getAbsolutePosition();
	ViewArea.setFrom ( ViewArea.Matrices [ SViewFrustum::ETS_VIEW_PROJECTION_3 ] );
/*
	video::IVideoDriver* driver = SceneManager->getVideoDriver();
	if ( driver)
	{
		driver->setTransform(video::ETS_PROJECTION, ViewArea.Matrices [ video::ETS_PROJECTION ] );
		driver->setTransform(video::ETS_VIEW, ViewArea.Matrices [ video::ETS_VIEW ] );
	}
*/
}


//! Writes attributes of the scene node.
void CCameraSceneNode::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const
{
	ISceneNode::serializeAttributes(out, options);

	out->addVector3d("Target", Target);
	out->addVector3d("UpVector", UpVector);
	out->addFloat("Fovy", Fovy);
	out->addFloat("Aspect", Aspect);
	out->addFloat("ZNear", ZNear);
	out->addFloat("ZFar", ZFar);
}


//! Reads attributes of the scene node.
void CCameraSceneNode::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options)
{
	ISceneNode::deserializeAttributes(in, options);

	Target = in->getAttributeAsVector3d("Target");
	UpVector = in->getAttributeAsVector3d("UpVector");
	Fovy = in->getAttributeAsFloat("Fovy");
	Aspect = in->getAttributeAsFloat("Aspect");
	ZNear = in->getAttributeAsFloat("ZNear");
	ZFar = in->getAttributeAsFloat("ZFar");

	recalculateProjectionMatrix();
	recalculateViewArea();	
}


} // end namespace
} // end namespace

