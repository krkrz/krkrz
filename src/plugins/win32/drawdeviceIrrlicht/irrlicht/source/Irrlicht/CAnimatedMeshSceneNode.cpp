// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CAnimatedMeshSceneNode.h"
#include "IVideoDriver.h"
#include "ISceneManager.h"
#include "S3DVertex.h"
#include "os.h"
#include "CShadowVolumeSceneNode.h"
#include "IAnimatedMeshMD3.h"
#include "CSkinnedMesh.h"
#include "IDummyTransformationSceneNode.h"
#include "IBoneSceneNode.h"
#include "IMaterialRenderer.h"
#include "IMesh.h"
#include "IMeshCache.h"
#include "IAnimatedMesh.h"
#include "quaternion.h"


namespace irr
{
namespace scene
{


//! constructor
CAnimatedMeshSceneNode::CAnimatedMeshSceneNode(IAnimatedMesh* mesh, ISceneNode* parent, ISceneManager* mgr, s32 id,
			const core::vector3df& position, const core::vector3df& rotation,	const core::vector3df& scale)
: IAnimatedMeshSceneNode(parent, mgr, id, position, rotation, scale), Mesh(0),
	BeginFrameTime(0), StartFrame(0), EndFrame(0), FramesPerSecond(25.f / 1000.f ),
	CurrentFrameNr(0.f), JointMode(EJUOR_NONE), JointsUsed(false),
	TransitionTime(0), Transiting(0.f), TransitingBlend(0.f),
	Looping(true), ReadOnlyMaterials(false),
	LoopCallBack(0), PassCount(0), Shadow(0), RenderFromIdentity(0)
{
	#ifdef _DEBUG
	setDebugName("CAnimatedMeshSceneNode");
	#endif

	BeginFrameTime = os::Timer::getTime();

	setMesh(mesh);
}


//! destructor
CAnimatedMeshSceneNode::~CAnimatedMeshSceneNode()
{
	if (Mesh)
		Mesh->drop();

	if (Shadow)
		Shadow->drop();

	//for (u32 i=0; i<JointChildSceneNodes.size(); ++i)
	//	if (JointChildSceneNodes[i])
	//		JointChildSceneNodes[i]->drop();

	if (LoopCallBack)
		LoopCallBack->drop();
}


//! Sets the current frame. From now on the animation is played from this frame.
void CAnimatedMeshSceneNode::setCurrentFrame(f32 frame)
{
	// if you pass an out of range value, we just clamp it
	CurrentFrameNr = core::clamp ( frame, (f32)StartFrame, (f32)EndFrame );

	BeginFrameTime = os::Timer::getTime() - (s32)((CurrentFrameNr - StartFrame) / FramesPerSecond);

	beginTransition(); //transit to this frame if enabled
}


//! Returns the currently displayed frame number.
f32 CAnimatedMeshSceneNode::getFrameNr() const
{
	return CurrentFrameNr;
}


f32 CAnimatedMeshSceneNode::buildFrameNr(u32 timeMs)
{
	if (Transiting!=0.f)
	{
		TransitingBlend = (f32)(timeMs-BeginFrameTime) * Transiting;
		if (TransitingBlend > 1.f)
		{
			Transiting=0.f;
			TransitingBlend=0.f;
		}
	}

	if (StartFrame==EndFrame)
		return (f32)StartFrame; //Support for non animated meshes
	if (FramesPerSecond==0.f)
		return (f32)StartFrame;

	if (Looping)
	{
		// play animation looped

		const s32 lenInMs = abs(s32( (EndFrame - StartFrame) / FramesPerSecond));
		if (FramesPerSecond > 0.f) //forwards...
		{
			return StartFrame + ( (timeMs - BeginFrameTime) % lenInMs) * FramesPerSecond;
		}
		else //backwards...
		{
			return EndFrame - ( (timeMs - BeginFrameTime) % lenInMs)* -FramesPerSecond;
		}
	}
	else
	{
		// play animation non looped

		f32 frame;

		if (FramesPerSecond > 0.f) //forwards...
		{
			const f32 deltaFrame = ( timeMs - BeginFrameTime ) * FramesPerSecond;

			frame = StartFrame + deltaFrame;

			if (frame > (f32)EndFrame)
			{
				frame = (f32)EndFrame;
				if (LoopCallBack)
					LoopCallBack->OnAnimationEnd(this);
			}
		}
		else //backwards... (untested)
		{
			const f32 deltaFrame = ( timeMs - BeginFrameTime ) * -FramesPerSecond;

			frame = EndFrame - deltaFrame;

			if (frame < (f32)StartFrame)
			{
				frame = (f32)StartFrame;
				if (LoopCallBack)
					LoopCallBack->OnAnimationEnd(this);
			}

		}

		return frame;
	}
}


//! frame
void CAnimatedMeshSceneNode::OnRegisterSceneNode()
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

		// register according to material types counted

		if (solidCount)
			SceneManager->registerNodeForRendering(this, scene::ESNRP_SOLID);

		if (transparentCount)
			SceneManager->registerNodeForRendering(this, scene::ESNRP_TRANSPARENT);

		ISceneNode::OnRegisterSceneNode();
	}
}


//! OnAnimate() is called just before rendering the whole scene.
void CAnimatedMeshSceneNode::OnAnimate(u32 timeMs)
{
	CurrentFrameNr = buildFrameNr ( timeMs );

	if ( Mesh && (Mesh->getMeshType() != EAMT_SKINNED))
	{
		scene::IMesh *m = Mesh->getMesh((s32)CurrentFrameNr, 255, StartFrame, EndFrame);
		if ( m )
			Box = m->getBoundingBox();
	}

	IAnimatedMeshSceneNode::OnAnimate ( timeMs );
}


//! renders the node.
void CAnimatedMeshSceneNode::render()
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();

	if (!Mesh || !driver)
		return;


	bool isTransparentPass =
		SceneManager->getSceneNodeRenderPass() == scene::ESNRP_TRANSPARENT;

	++PassCount;

	f32 frame = getFrameNr();

	scene::IMesh* m;

	if (Mesh->getMeshType() != EAMT_SKINNED)
		m = Mesh->getMesh((s32)frame, 255, StartFrame, EndFrame);
	else
	{
		CSkinnedMesh* skinnedMesh = reinterpret_cast<CSkinnedMesh*>(Mesh);

		if (JointMode == EJUOR_CONTROL)//write to mesh
			skinnedMesh->transferJointsToMesh(JointChildSceneNodes);
		else
			skinnedMesh->animateMesh(frame, 1.0f);

		skinnedMesh->skinMesh();

		if (JointMode == EJUOR_READ)//read from mesh
		{
			skinnedMesh->recoverJointsFromMesh(JointChildSceneNodes);

			//---slow---
			for (u32 n=0;n<JointChildSceneNodes.size();++n)
				if (JointChildSceneNodes[n]->getParent()==this)
				{
					JointChildSceneNodes[n]->updateAbsolutePositionOfAllChildren(); //temp, should be an option
				}

		}
		m=skinnedMesh;

	}

	if ( 0 == m )
	{
		#ifdef _DEBUG
			os::Printer::log("Animated Mesh returned no mesh to render.", Mesh->getDebugName(), ELL_WARNING);
		#endif
	}

	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);


	if (Shadow && PassCount==1)
		Shadow->setMeshToRenderFrom(m);

	// for debug purposes only:

	bool renderMeshes = true;
	video::SMaterial mat;
	if (DebugDataVisible && PassCount==1)
	{
		// overwrite half transparency
		if ( DebugDataVisible & scene::EDS_HALF_TRANSPARENCY )
		{

			for (u32 i=0; i<m->getMeshBufferCount(); ++i)
			{
				scene::IMeshBuffer* mb = m->getMeshBuffer(i);
				mat = Materials[i];
				mat.MaterialType = video::EMT_TRANSPARENT_ADD_COLOR;
				if (RenderFromIdentity)
					driver->setTransform(video::ETS_WORLD, core::matrix4() );
				else if (Mesh->getMeshType() == EAMT_SKINNED)
					driver->setTransform(video::ETS_WORLD, AbsoluteTransformation * ((SSkinMeshBuffer*)mb)->Transformation);

				driver->setMaterial(mat);
				driver->drawMeshBuffer(mb);
			}
			renderMeshes = false;
		}
	}

	// render original meshes
	if ( renderMeshes )
	{

		for (u32 i=0; i<m->getMeshBufferCount(); ++i)
		{
			video::IMaterialRenderer* rnd = driver->getMaterialRenderer(Materials[i].MaterialType);
			bool transparent = (rnd && rnd->isTransparent());

			// only render transparent buffer if this is the transparent render pass
			// and solid only in solid pass
			if (transparent == isTransparentPass)
			{
				scene::IMeshBuffer* mb = m->getMeshBuffer(i);

				if (RenderFromIdentity)
					driver->setTransform(video::ETS_WORLD, core::matrix4() );
				else if (Mesh->getMeshType() == EAMT_SKINNED)
					driver->setTransform(video::ETS_WORLD, AbsoluteTransformation * ((SSkinMeshBuffer*)mb)->Transformation);

				driver->setMaterial(Materials[i]);
				driver->drawMeshBuffer(mb);
			}
		}
	}

	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

	// for debug purposes only:
	if (DebugDataVisible && PassCount==1)
	{
		mat.Lighting = false;
		driver->setMaterial(mat);

		// show normals
		if ( DebugDataVisible & scene::EDS_NORMALS )
		{
			IAnimatedMesh * arrow = SceneManager->addArrowMesh (
					"__debugnormal", 0xFFECEC00,
					0xFF999900, 4, 8, 1.f, 0.6f, 0.05f,
					0.3f);
			if ( 0 == arrow )
			{
				arrow = SceneManager->getMesh ( "__debugnormal" );
			}
			const IMesh *mesh = arrow->getMesh ( 0 );

			// find a good scaling factor

			core::matrix4 m2;

			// draw normals
			for (u32 g=0; g<m->getMeshBufferCount(); ++g)
			{
				const scene::IMeshBuffer* mb = m->getMeshBuffer(g);
				const u32 vSize = video::getVertexPitchFromType(mb->getVertexType());
				const video::S3DVertex* v = ( const video::S3DVertex*)mb->getVertices();
				for ( u32 i=0; i != mb->getVertexCount(); ++i )
				{
					// Align to v->normal
					core::quaternion quatRot( v->Normal.Z, 0.f, -v->Normal.X, 1 + v->Normal.Y );
					quatRot.normalize();
					quatRot.getMatrix ( m2 );

					m2.setTranslation(v->Pos);
					if (Mesh->getMeshType() == EAMT_SKINNED)
					{
						m2 = (AbsoluteTransformation * ((SSkinMeshBuffer*)mb)->Transformation) * m2;
					}
					else
						m2*=AbsoluteTransformation;

					driver->setTransform(video::ETS_WORLD, m2 );
					for ( u32 a = 0; a != mesh->getMeshBufferCount(); ++a )
						driver->drawMeshBuffer ( mesh->getMeshBuffer ( a ) );

					v = (const video::S3DVertex*) ( (u8*) v + vSize );
				}
			}
			driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
		}

		mat.MaterialType = video::EMT_SOLID;
		mat.ZBuffer = false;
		driver->setMaterial(mat);
		// show bounding box
		if ( DebugDataVisible & scene::EDS_BBOX_BUFFERS )
		{

			for (u32 g=0; g< m->getMeshBufferCount(); ++g)
			{
				const IMeshBuffer* mb = m->getMeshBuffer(g);

				if (Mesh->getMeshType() == EAMT_SKINNED)
					driver->setTransform(video::ETS_WORLD, AbsoluteTransformation * ((SSkinMeshBuffer*)mb)->Transformation);
				driver->draw3DBox( mb->getBoundingBox(),
						video::SColor(0,190,128,128) );
			}
			driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
		}

		if ( DebugDataVisible & scene::EDS_BBOX )
			driver->draw3DBox(Box, video::SColor(0,255,255,255));

		// show skeleton
		if ( DebugDataVisible & scene::EDS_SKELETON )
		{
			if (Mesh->getMeshType() == EAMT_SKINNED)
			{
				// draw skeleton

				for (u32 g=0; g < ((ISkinnedMesh*)Mesh)->getAllJoints().size(); ++g)
				{
					ISkinnedMesh::SJoint *joint=((ISkinnedMesh*)Mesh)->getAllJoints()[g];

					for (u32 n=0;n<joint->Children.size();++n)
					{
						driver->draw3DLine(joint->GlobalAnimatedMatrix.getTranslation(),
								joint->Children[n]->GlobalAnimatedMatrix.getTranslation(),
								video::SColor(0,51,66,255));
					}
				}
			}

			// show tag for quake3 models
			if (Mesh->getMeshType() == EAMT_MD3 )
			{
				IAnimatedMesh * arrow =
					SceneManager->addArrowMesh (
							"__tag_show",
							0xFF0000FF, 0xFF000088,
							4, 8, 5.f, 4.f, 0.5f,
							1.f);
				if ( 0 == arrow )
				{
					arrow = SceneManager->getMesh ( "__tag_show" );
				}
				IMesh *arrowMesh = arrow->getMesh ( 0 );

				video::SMaterial material;
				material.Lighting = false;
				driver->setMaterial(material);

				core::matrix4 matr;

				SMD3QuaterionTagList *taglist = ((IAnimatedMeshMD3*)Mesh)->getTagList(
						(s32)getFrameNr(), 255,
						getStartFrame(), getEndFrame());
				if ( taglist )
				{
					for ( u32 ts = 0; ts != taglist->size(); ++ts )
					{
						(*taglist)[ts].setto ( matr );

						driver->setTransform(video::ETS_WORLD, matr );

						for ( u32 a = 0; a != arrowMesh->getMeshBufferCount(); ++a )
							driver->drawMeshBuffer ( arrowMesh->getMeshBuffer ( a ) );
					}
				}
			}
		}

		// show mesh
		if ( DebugDataVisible & scene::EDS_MESH_WIRE_OVERLAY )
		{
			mat.Lighting = false;
			mat.Wireframe = true;
			mat.ZBuffer = true;
			driver->setMaterial(mat);


			for (u32 g=0; g<m->getMeshBufferCount(); ++g)
			{
				const IMeshBuffer* mb = m->getMeshBuffer(g);
				if (RenderFromIdentity)
					driver->setTransform(video::ETS_WORLD, core::matrix4() );
				else if (Mesh->getMeshType() == EAMT_SKINNED)
					driver->setTransform(video::ETS_WORLD, AbsoluteTransformation * ((SSkinMeshBuffer*)mb)->Transformation);
				driver->drawMeshBuffer(mb);
			}
		}
	}
}


//! Returns the current start frame number.
s32 CAnimatedMeshSceneNode::getStartFrame() const
{
	return StartFrame;
}


//! Returns the current start frame number.
s32 CAnimatedMeshSceneNode::getEndFrame() const
{
	return EndFrame;
}


//! sets the frames between the animation is looped.
//! the default is 0 - MaximalFrameCount of the mesh.
bool CAnimatedMeshSceneNode::setFrameLoop(s32 begin, s32 end)
{
	const s32 maxFrameCount = Mesh->getFrameCount() - 1;
	if ( end < begin )
	{
		StartFrame = core::s32_clamp(end, 0, maxFrameCount);
		EndFrame = core::s32_clamp(begin, StartFrame, maxFrameCount);
	}
	else
	{
		StartFrame = core::s32_clamp(begin, 0, maxFrameCount);
		EndFrame = core::s32_clamp(end, StartFrame, maxFrameCount);
	}
	setCurrentFrame ( (f32)StartFrame );

	return true;
}


//! sets the speed with witch the animation is played
void CAnimatedMeshSceneNode::setAnimationSpeed(f32 framesPerSecond)
{
	FramesPerSecond = framesPerSecond * 0.001f;
}


//! returns the axis aligned bounding box of this node
const core::aabbox3d<f32>& CAnimatedMeshSceneNode::getBoundingBox() const
{
	return Box;
}


//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use getMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
video::SMaterial& CAnimatedMeshSceneNode::getMaterial(u32 i)
{
	if ( i >= Materials.size() )
		return ISceneNode::getMaterial(i);

	return Materials[i];
}



//! returns amount of materials used by this scene node.
u32 CAnimatedMeshSceneNode::getMaterialCount() const
{
	return Materials.size();
}


//! Creates shadow volume scene node as child of this node
//! and returns a pointer to it.
IShadowVolumeSceneNode* CAnimatedMeshSceneNode::addShadowVolumeSceneNode(s32 id,
						 bool zfailmethod, f32 infinity)
{
	if (!SceneManager->getVideoDriver()->queryFeature(video::EVDF_STENCIL_BUFFER))
		return 0;

	if (Shadow)
	{
		os::Printer::log("This node already has a shadow.", ELL_WARNING);
		return 0;
	}

	Shadow = new CShadowVolumeSceneNode(this, SceneManager, id,  zfailmethod, infinity);
	return Shadow;
}


IBoneSceneNode* CAnimatedMeshSceneNode::getJointNode(const c8* jointName)
{
	if (!Mesh || Mesh->getMeshType() != EAMT_SKINNED)
		return 0;

	checkJoints();


	ISkinnedMesh *skinnedMesh=(ISkinnedMesh*)Mesh;

	s32 number = skinnedMesh->getJointNumber(jointName);

	if (number == -1)
	{
		os::Printer::log("Joint with specified name not found in skinned mesh.", jointName, ELL_WARNING);
		return 0;
	}

	if ((s32)JointChildSceneNodes.size() <= number)
	{
		os::Printer::log("Joint was found in mesh, but is not loaded into node", jointName, ELL_WARNING);
		return 0;
	}

	return getJointNode((u32)number);
}


IBoneSceneNode* CAnimatedMeshSceneNode::getJointNode(u32 jointID)
{
	if (JointChildSceneNodes.size() <= jointID)
	{
		os::Printer::log("Joint not loaded into node", ELL_WARNING);
		return 0;
	}

	return JointChildSceneNodes[jointID];
}


//! Returns a pointer to a child node, which has the same transformation as
//! the corrsesponding joint, if the mesh in this scene node is a ms3d mesh.
ISceneNode* CAnimatedMeshSceneNode::getMS3DJointNode(const c8* jointName)
{
	return  getJointNode(jointName);
}


//! Returns a pointer to a child node, which has the same transformation as
//! the corrsesponding joint, if the mesh in this scene node is a ms3d mesh.
ISceneNode* CAnimatedMeshSceneNode::getXJointNode(const c8* jointName)
{
	return  getJointNode(jointName);
}


//! Removes a child from this scene node.
//! Implemented here, to be able to remove the shadow properly, if there is one,
//! or to remove attached childs.
bool CAnimatedMeshSceneNode::removeChild(ISceneNode* child)
{
	if (child && Shadow == child)
	{
		Shadow->drop();
		Shadow = 0;
		return true;
	}

	if (ISceneNode::removeChild(child))
	{
		if (JointsUsed) //stop weird bugs caused while changing parents as the joints are being created
		{
			for (u32 i=0; i<JointChildSceneNodes.size(); ++i)
			if (JointChildSceneNodes[i] == child)
			{
				JointChildSceneNodes[i] = 0; //remove link to child
				return true;
			}
		}

		return true;
	}

	return false;
}


//! Starts a MD2 animation.
bool CAnimatedMeshSceneNode::setMD2Animation(EMD2_ANIMATION_TYPE anim)
{
	if (!Mesh || Mesh->getMeshType() != EAMT_MD2)
		return false;

	IAnimatedMeshMD2* m = (IAnimatedMeshMD2*)Mesh;

	s32 begin, end, speed;
	m->getFrameLoop(anim, begin, end, speed);

	setAnimationSpeed( f32(speed) );
	setFrameLoop(begin, end);
	return true;
}


//! Starts a special MD2 animation.
bool CAnimatedMeshSceneNode::setMD2Animation(const c8* animationName)
{
	if (!Mesh || Mesh->getMeshType() != EAMT_MD2)
		return false;

	IAnimatedMeshMD2* m = (IAnimatedMeshMD2*)Mesh;

	s32 begin, end, speed;
	if (!m->getFrameLoop(animationName, begin, end, speed))
		return false;

	setAnimationSpeed( (f32)speed );
	setFrameLoop(begin, end);
	return true;
}


//! Sets looping mode which is on by default. If set to false,
//! animations will not be looped.
void CAnimatedMeshSceneNode::setLoopMode(bool playAnimationLooped)
{
	Looping = playAnimationLooped;
}


//! Sets a callback interface which will be called if an animation
//! playback has ended. Set this to 0 to disable the callback again.
void CAnimatedMeshSceneNode::setAnimationEndCallback(IAnimationEndCallBack* callback)
{
	if (LoopCallBack)
		LoopCallBack->drop();

	LoopCallBack = callback;

	if (LoopCallBack)
		LoopCallBack->grab();
}


//! Sets if the scene node should not copy the materials of the mesh but use them in a read only style.
void CAnimatedMeshSceneNode::setReadOnlyMaterials(bool readonly)
{
	ReadOnlyMaterials = readonly;
}


//! Returns if the scene node should not copy the materials of the mesh but use them in a read only style
bool CAnimatedMeshSceneNode::isReadOnlyMaterials() const
{
	return ReadOnlyMaterials;
}


//! Writes attributes of the scene node.
void CAnimatedMeshSceneNode::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const
{
	IAnimatedMeshSceneNode::serializeAttributes(out, options);

	out->addString("Mesh", SceneManager->getMeshCache()->getMeshFilename(Mesh));
	out->addBool("Looping", Looping);
	out->addBool("ReadOnlyMaterials", ReadOnlyMaterials);
	out->addFloat("FramesPerSecond", FramesPerSecond);

	// TODO: write animation names instead of frame begin and ends
}


//! Reads attributes of the scene node.
void CAnimatedMeshSceneNode::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options)
{
	IAnimatedMeshSceneNode::deserializeAttributes(in, options);

	core::stringc oldMeshStr = SceneManager->getMeshCache()->getMeshFilename(Mesh);
	core::stringc newMeshStr = in->getAttributeAsString("Mesh");

	Looping = in->getAttributeAsBool("Looping");
	ReadOnlyMaterials = in->getAttributeAsBool("ReadOnlyMaterials");
	FramesPerSecond = in->getAttributeAsFloat("FramesPerSecond");

	if (newMeshStr != "" && oldMeshStr != newMeshStr)
	{
		IAnimatedMesh* newAnimatedMesh = SceneManager->getMesh(newMeshStr.c_str());

		if (newAnimatedMesh)
			setMesh(newAnimatedMesh);
	}

	// TODO: read animation names instead of frame begin and ends
}


//! Sets a new mesh
void CAnimatedMeshSceneNode::setMesh(IAnimatedMesh* mesh)
{
	if (!mesh)
		return; // won't set null mesh

	if (Mesh)
		Mesh->drop();

	Mesh = mesh;

	// get materials and bounding box
	Box = Mesh->getBoundingBox();

	IMesh* m = Mesh->getMesh(0,0);
	if (m)
	{
		Materials.clear();

		video::SMaterial mat;
		for (u32 i=0; i<m->getMeshBufferCount(); ++i)
		{
			IMeshBuffer* mb = m->getMeshBuffer(i);
			if (mb)
				mat = mb->getMaterial();

			Materials.push_back(mat);
		}
	}

	// get start and begin time
	setFrameLoop ( 0, Mesh->getFrameCount() );

	// grab the mesh
	if (Mesh)
		Mesh->grab();
}

// returns the absolute transformation for a special MD3 Tag if the mesh is a md3 mesh,
// or the absolutetransformation if it's a normal scenenode
const SMD3QuaterionTag& CAnimatedMeshSceneNode::getMD3TagTransformation( const core::stringc & tagname)
{
	SMD3QuaterionTag * tag = MD3Special.AbsoluteTagList.get ( tagname );
	if ( tag )
		return *tag;

	MD3Special.AbsoluteTagList.Container.push_back ( SMD3QuaterionTag ( tagname, AbsoluteTransformation ) );
	return *MD3Special.AbsoluteTagList.get ( tagname );
}


//! updates the absolute position based on the relative and the parents position
void CAnimatedMeshSceneNode::updateAbsolutePosition()
{
	IAnimatedMeshSceneNode::updateAbsolutePosition();

	if ( 0 == Mesh || Mesh->getMeshType() != EAMT_MD3 )
		return;

	SMD3QuaterionTag parent;
	if ( Parent && Parent->getType () == ESNT_ANIMATED_MESH)
	{
		parent = ((IAnimatedMeshSceneNode*) Parent)->getMD3TagTransformation ( MD3Special.Tagname );
	}

	SMD3QuaterionTag relative( RelativeTranslation, RelativeRotation );

	SMD3QuaterionTagList *taglist;
	taglist = ( (IAnimatedMeshMD3*) Mesh )->getTagList ( (s32)getFrameNr(),255,getStartFrame (),getEndFrame () );
	if ( taglist )
	{
		MD3Special.AbsoluteTagList.Container.set_used ( taglist->size () );
		for ( u32 i=0; i!= taglist->size (); ++i )
		{
			MD3Special.AbsoluteTagList[i].position = parent.position + (*taglist)[i].position + relative.position;
			MD3Special.AbsoluteTagList[i].rotation = parent.rotation * (*taglist)[i].rotation * relative.rotation;
		}
	}
}

//! Set the joint update mode (0-unused, 1-get joints only, 2-set joints only, 3-move and set)
void CAnimatedMeshSceneNode::setJointMode(E_JOINT_UPDATE_ON_RENDER mode)
{
	checkJoints();

	//if (mode<0) mode=0;
	//if (mode>3) mode=3;

	JointMode=mode;
}


//! Sets the transition time in seconds (note: This needs to enable joints, and setJointmode maybe set to 2)
//! you must call animateJoints(), or the mesh will not animate
void CAnimatedMeshSceneNode::setTransitionTime(f32 time)
{
	if (time != 0.0f)
	{
		checkJoints();
		setJointMode(EJUOR_CONTROL);
		TransitionTime = (u32)core::floor32(time*1000.0f);
	}
}

//! render mesh ignoring it's transformation. Used with ragdolls. (culling is unaffected)
void CAnimatedMeshSceneNode::setRenderFromIdentity( bool On )
{
	RenderFromIdentity=On;
}



//! updates the joint positions of this mesh
void CAnimatedMeshSceneNode::animateJoints(bool CalculateAbsolutePositions)
{
	checkJoints();

	if (Mesh && Mesh->getMeshType() == EAMT_SKINNED )
	{
		if (JointsUsed)
		{
			f32 frame = getFrameNr(); //old?

			CSkinnedMesh* skinnedMesh=reinterpret_cast<CSkinnedMesh*>(Mesh);

			skinnedMesh->animateMesh(frame, 1.0f);

			skinnedMesh->recoverJointsFromMesh( JointChildSceneNodes);

			if (CalculateAbsolutePositions)
			{
				//---slow---
				for (u32 n=0;n<JointChildSceneNodes.size();++n)
					if (JointChildSceneNodes[n]->getParent()==this)
					{
						JointChildSceneNodes[n]->updateAbsolutePositionOfAllChildren(); //temp, should be an option
					}
			}

			//-----------------------------------------
			//		Transition
			//-----------------------------------------

			if (Transiting != 0.f)
			{
				//Check the array is big enough (not really needed)
				if (PretransitingSave.size()<JointChildSceneNodes.size())
				{
					for(u32 n=PretransitingSave.size(); n<JointChildSceneNodes.size(); ++n)
						PretransitingSave.push_back(core::matrix4());
				}

				for (u32 n=0; n<JointChildSceneNodes.size(); ++n)
				{
					//------Position------

					JointChildSceneNodes[n]->setPosition(
							core::lerp(
								PretransitingSave[n].getTranslation(),
								JointChildSceneNodes[n]->getPosition(),
								TransitingBlend));

					//------Rotation------

					//Code is slow, needs to be fixed up

					const core::quaternion RotationStart(PretransitingSave[n].getRotationDegrees()*core::DEGTORAD);
					const core::quaternion RotationEnd(JointChildSceneNodes[n]->getRotation()*core::DEGTORAD);

					core::quaternion QRotation;
					QRotation.slerp(RotationStart, RotationEnd, TransitingBlend);

					core::vector3df tmpVector;
					QRotation.toEuler(tmpVector);
					tmpVector*=core::RADTODEG; //convert from radians back to degrees
					JointChildSceneNodes[n]->setRotation( tmpVector );

					//------Scale------

					//JointChildSceneNodes[n]->setScale(
					//		core::lerp(
					//			PretransitingSave[n].getScale(),
					//			JointChildSceneNodes[n]->getScale(),
					//			TransitingBlend));
				}
			}
		}
	}
}




void CAnimatedMeshSceneNode::checkJoints()
{
	if (!Mesh || Mesh->getMeshType() != EAMT_SKINNED)
		return;

	if (!JointsUsed)
	{
		//Create joints for SkinnedMesh

		((CSkinnedMesh*)Mesh)->createJoints(JointChildSceneNodes, this, SceneManager);
		((CSkinnedMesh*)Mesh)->recoverJointsFromMesh(JointChildSceneNodes);

		JointsUsed=true;
		JointMode=EJUOR_READ;
	}
}

void CAnimatedMeshSceneNode::beginTransition()
{
	if (!JointsUsed)
		return;

	if (TransitionTime != 0)
	{
		//Check the array is big enough
		if (PretransitingSave.size()<JointChildSceneNodes.size())
		{
			for(u32 n=PretransitingSave.size(); n<JointChildSceneNodes.size(); ++n)
				PretransitingSave.push_back(core::matrix4());
		}


		//Copy the position of joints
		for (u32 n=0;n<JointChildSceneNodes.size();++n)
			PretransitingSave[n]=JointChildSceneNodes[n]->getRelativeTransformation();

		Transiting = core::reciprocal((f32)TransitionTime);
	}
	TransitingBlend = 0.f;
}


} // end namespace scene
} // end namespace irr

