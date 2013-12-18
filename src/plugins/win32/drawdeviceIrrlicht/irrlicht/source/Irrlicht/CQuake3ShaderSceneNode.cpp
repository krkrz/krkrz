// Copyright (C) 2002-2008 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_BSP_LOADER_

#include "CQuake3ShaderSceneNode.h"
#include "ISceneManager.h"
#include "IVideoDriver.h"
#include "ICameraSceneNode.h"
#include "SViewFrustum.h"

namespace irr
{
namespace scene
{


CQuake3ShaderSceneNode::CQuake3ShaderSceneNode(
			scene::ISceneNode* parent, scene::ISceneManager* mgr,s32 id,
			io::IFileSystem *fileSystem, scene::IMeshBuffer *buffer,
			const quake3::SShader * shader)
	: scene::ISceneNode(parent, mgr, id), Shader ( shader ),TimeAbs ( 0.f )
{
	#ifdef _DEBUG
		core::stringc dName = "CQuake3ShaderSceneNode ";
		dName += Shader->name;

		setDebugName( dName.c_str() );
	#endif

	// name the Scene Node
	this->Name = Shader->name;

	// clone meshbuffer to modifiable buffer
	cloneBuffer ( static_cast< scene::SMeshBufferLightMap *> ( buffer ) );

	// load all Textures in all stages
	loadTextures ( fileSystem );
}


/*
	create single copies
*/
void CQuake3ShaderSceneNode::cloneBuffer ( scene::SMeshBufferLightMap * buffer )
{
	Original.Material = buffer->Material;
	MeshBuffer.Material = buffer->Material;

	Original.Indices = buffer->Indices;
	MeshBuffer.Indices = buffer->Indices;

	const u32 vsize = buffer->Vertices.size ();

	Original.Vertices.set_used ( vsize );
	MeshBuffer.Vertices.set_used ( vsize );
	for ( u32 i = 0; i!= vsize; ++i )
	{
		const video::S3DVertex2TCoords * src = &buffer->Vertices[i];
		Original.Vertices[i] = *src;

		video::S3DVertex * d = &MeshBuffer.Vertices[i];

		d->Pos = src->Pos;
		d->Color = 0xFFFFFFFF; //src->Color;
		d->Normal = src->Normal;
		d->TCoords = src->TCoords;
	}

	MeshBuffer.recalculateBoundingBox ();
	// used for sorting
	MeshBuffer.Material.setTexture(0, (video::ITexture*) Shader);
}



/*
	load the textures for all stages
*/
void CQuake3ShaderSceneNode::loadTextures ( io::IFileSystem * fileSystem )
{
	const quake3::SVarGroup *group;
	u32 i;

	// clear all stages and prefill empty
	Q3Texture.clear ();
	for ( i = 0; i != Shader->VarGroup->VariableGroup.size (); ++i )
	{
		Q3Texture.push_back ( SQ3Texture() );
	}

	u32 pos;

	// get texture map
	for ( i = 0; i < Shader->VarGroup->VariableGroup.size (); ++i )
	{
		group = Shader->getGroup ( i );

		const core::stringc &mapname = group->get ( "map" );
		if ( 0 == mapname.size () )
			continue;

		// our lightmap is passed in material.Texture[2]
		if ( mapname == "$lightmap" )
		{
			Q3Texture [i].Texture.push_back ( Original.getMaterial().getTexture(1) );
		}
		else
		{
			pos = 0;
			quake3::getTextures ( Q3Texture [i].Texture, mapname, pos, fileSystem, SceneManager->getVideoDriver() );
		}
	}

	// get anim map
	for ( i = 0; i < Shader->VarGroup->VariableGroup.size (); ++i )
	{
		if ( Q3Texture [i].Texture.size() )
			continue;

		group = Shader->getGroup ( i );

		const core::stringc &animmap = group->get ( "animmap" );
		if ( 0 == animmap.size () )
			continue;

		// first parameter is frequency
		pos = 0;
		Q3Texture [i].TextureFrequency = core::max_ ( 0.0001f, quake3::getAsFloat ( animmap, pos ) );

		quake3::getTextures ( Q3Texture [i].Texture, animmap, pos,fileSystem, SceneManager->getVideoDriver() );
	}

	// get clamp map
	for ( i = 0; i < Shader->VarGroup->VariableGroup.size (); ++i )
	{
		if ( Q3Texture [i].Texture.size() )
			continue;

		group = Shader->getGroup ( i );

		const core::stringc &clampmap = group->get ( "clampmap" );
		if ( 0 == clampmap.size () )
			continue;

		Q3Texture [i].TextureAddressMode = video::ETC_CLAMP;
		pos = 0;
		quake3::getTextures ( Q3Texture [i].Texture, clampmap, pos,fileSystem, SceneManager->getVideoDriver() );
	}
}

/*
	Register each texture stage, if first is visible
*/
void CQuake3ShaderSceneNode::OnRegisterSceneNode()
{
	PassedCulling = 0;
	StageCall = 0;
	if ( !IsVisible)
		return;

	u32 first = 1;
	for ( u32 i = 1; i < Q3Texture.size(); ++i )
	{
		if ( 0 == Q3Texture [ i ].Texture.size() && i >= 2 )
			continue;

		scene::E_SCENE_NODE_RENDER_PASS time = (scene::E_SCENE_NODE_RENDER_PASS) ( scene::ESNRP_SHADER_0 + i );
		if ( first )
		{
			setAutomaticCulling ( scene::EAC_BOX );
			PassedCulling = SceneManager->registerNodeForRendering(this, time );
			setAutomaticCulling ( scene::EAC_OFF );
			first = 0;
		}
		else
		if ( PassedCulling )
		{
			SceneManager->registerNodeForRendering(this, time );
		}
	}
}

/*
	render in multipass technique
*/
void CQuake3ShaderSceneNode::render()
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();

	const u32 stage = SceneManager->getSceneNodeRenderPass() - scene::ESNRP_SHADER_0;

	video::SMaterial material;

	const quake3::SVarGroup *group;

	// generic stage
	group = Shader->getGroup ( 1 );
	material.BackfaceCulling = quake3::getBackfaceCulling ( group->get ( "cull" ) );

	// advance current stage
	core::matrix4 texture;
	animate ( stage, texture );

	// stage 1 finished, no drawing stage ( vertex transform only )
	if ( stage == 1 )
		return;

	// current stage
	group = Shader->getGroup ( stage );


	SQ3Texture &q = Q3Texture [ stage];

	material.Lighting = false;
	material.setTexture(0, q.Texture [ q.TextureIndex ]);
	material.setTexture(1, 0);
	material.ZBuffer = quake3::getDepthFunction ( group->get ( "depthfunc" ) );
	material.ZWriteEnable = (0 == StageCall );
	material.NormalizeNormals = false;

	quake3::SBlendFunc blendfunc;
	quake3::getBlendFunc ( group->get ( "blendfunc" ), blendfunc );
	quake3::getBlendFunc ( group->get ( "alphafunc" ), blendfunc );

	material.MaterialType = blendfunc.type;
	material.MaterialTypeParam = blendfunc.param;

/*
	if ( textureTransform )
	{
		transformtex ( texture, q.TextureAddressMode );
	}
*/
	material.TextureLayer[0].TextureWrap = q.TextureAddressMode;
	material.TextureLayer[1].TextureWrap = material.TextureLayer[0].TextureWrap;
	driver->setTransform ( video::ETS_TEXTURE_0, texture );

	driver->setMaterial( material );

	if ( 0 == StageCall )
	{
		driver->setTransform(video::ETS_WORLD, core::matrix4() );
	}
	
	driver->drawMeshBuffer( &MeshBuffer );

	StageCall += 1;
}


/*!
*/
void CQuake3ShaderSceneNode::vertextransform_wave ( f32 dt, quake3::SModifierFunction &function )
{
	function.wave = core::reciprocal ( function.wave );

	const f32 phase = function.phase;

	const u32 vsize = MeshBuffer.Vertices.size();
	for ( u32 i = 0; i != vsize; ++i )
	{
		const video::S3DVertex2TCoords &src = Original.Vertices[i];
		video::S3DVertex &dst = MeshBuffer.Vertices[i];

		f32 wavephase = (src.Pos.X + src.Pos.Y + src.Pos.Z) * function.wave;
		function.phase = phase + wavephase;

		const f32 f = function.evaluate ( dt );

		dst.Pos.X = src.Pos.X + f * src.Normal.X;
		dst.Pos.Y = src.Pos.Y + f * src.Normal.Y;
		dst.Pos.Z = src.Pos.Z + f * src.Normal.Z;
	}
}

/*!
*/
void CQuake3ShaderSceneNode::vertextransform_bulge ( f32 dt, quake3::SModifierFunction &function )
{
	function.func = 0;
	function.wave = core::reciprocal ( function.bulgewidth );

	dt *= function.bulgespeed * 0.1f;
	const f32 phase = function.phase;

	const u32 vsize = MeshBuffer.Vertices.size();
	for ( u32 i = 0; i != vsize; ++i )
	{
		const video::S3DVertex2TCoords &src = Original.Vertices[i];
		video::S3DVertex &dst = MeshBuffer.Vertices[i];

		f32 wavephase = (Original.Vertices[i].TCoords.X ) * function.wave;
		function.phase = phase + wavephase;

		const f32 f = function.evaluate ( dt );

		dst.Pos.X = src.Pos.X + f * src.Normal.X;
		dst.Pos.Y = src.Pos.Y + f * src.Normal.Y;
		dst.Pos.Z = src.Pos.Z + f * src.Normal.Z;
	}
}

/*!
*/
void CQuake3ShaderSceneNode::vertextransform_autosprite ( f32 dt, quake3::SModifierFunction &function )
{
	const core::matrix4 &m = SceneManager->getActiveCamera()->getViewFrustum()->Matrices [ video::ETS_VIEW ];
	const core::vector3df view ( -m[2], -m[6] , -m[10] );

	const u32 vsize = MeshBuffer.Vertices.size();

	core::aabbox3df box;
	u32 g;

	for ( u32 i = 0; i < vsize; i += 4 )
	{
		// in pairs of 4
		box.reset ( Original.Vertices[i].Pos );
		for ( g = 1; g != 4; ++g )
		{
			box.addInternalPoint ( Original.Vertices[i + g].Pos );
		}

		core::vector3df c = box.getCenter ();
		f32 sh = 0.5f * ( box.MaxEdge.Z - box.MinEdge.Z );
		f32 sv = 0.5f * ( box.MaxEdge.Y - box.MinEdge.Y );

		const core::vector3df h ( m[0] * sh, m[4] * sh, m[8] * sh );
		const core::vector3df v ( m[1] * sv, m[5] * sv, m[9] * sv );

		MeshBuffer.Vertices[ i + 0 ].Pos = c + h + v;
		MeshBuffer.Vertices[ i + 1 ].Pos = c - h - v;
		MeshBuffer.Vertices[ i + 2 ].Pos = c + h - v;
		MeshBuffer.Vertices[ i + 3 ].Pos = c - h + v;

		MeshBuffer.Vertices[ i + 0 ].Normal = view;
		MeshBuffer.Vertices[ i + 1 ].Normal = view;
		MeshBuffer.Vertices[ i + 2 ].Normal = view;
		MeshBuffer.Vertices[ i + 3 ].Normal = view;
	}
}

/*
	Generate Vertex Color
*/
void CQuake3ShaderSceneNode::rgbgen ( f32 dt, quake3::SModifierFunction &function )
{
	u32 i;
	const u32 vsize = MeshBuffer.Vertices.size();

	switch ( function.masterfunc1 )
	{
		case 6:
			//identity
			for ( i = 0; i != vsize; ++i )
				MeshBuffer.Vertices[i].Color = 0xFFFFFFFF;
			break;
		case 7:
			// vertex
			for ( i = 0; i != vsize; ++i )
				MeshBuffer.Vertices[i].Color = Original.Vertices[i].Color;
			break;
		case 5:
		{
			// wave
			f32 f = function.evaluate ( dt ) * 255.f;
			s32 value = core::clamp ( core::floor32 ( f ), 0, 255 );
			value |= value << 8;
			value |= value << 16;

			for ( i = 0; i != vsize; ++i )
				MeshBuffer.Vertices[i].Color = value;
		} break;
	}
}

/*
	Generate Texture Coordinates
*/
u32 CQuake3ShaderSceneNode::tcgen ( f32 dt, quake3::SModifierFunction &function, core::matrix4 &texture )
{
	u32 ret = 0;
	u32 i;
	const u32 vsize = MeshBuffer.Vertices.size();

	switch ( function.tcgen )
	{
		case 8:
			// tcgen texture
			for ( i = 0; i != vsize; ++i )
				MeshBuffer.Vertices[i].TCoords = Original.Vertices[i].TCoords;
			break;
		case 9:
			// tcgen lightmap
			for ( i = 0; i != vsize; ++i )
				MeshBuffer.Vertices[i].TCoords = Original.Vertices[i].TCoords2;
			break;
		case 10:
		{
			// tcgen environment ( D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR )

			// using eye linear, sphere map may be cooler;-)
			// modelmatrix is identity
			const core::matrix4 &view = SceneManager->getActiveCamera()->getViewFrustum()->Matrices [ video::ETS_VIEW ];
			const core::matrix4 &viewinverse = SceneManager->getActiveCamera()->getViewFrustum()->Matrices [ SViewFrustum::ETS_VIEW_MODEL_INVERSE_3 ];

			// eyePlane
			core::vector3df eyePlaneS;
			core::vector3df eyePlaneT;

			viewinverse.transformVect ( eyePlaneS, core::vector3df ( 1.f, 0.f, 0.f ) );
			viewinverse.transformVect ( eyePlaneT, core::vector3df ( 0.f, 1.f, 0.f ) );

			eyePlaneS.normalize ();
			eyePlaneT.normalize ();

			core::vector3df v;
			for ( i = 0; i != vsize; ++i )
			{
				// vertex in eye space
				view.transformVect ( v, Original.Vertices[i].Pos );
				v.normalize();

				MeshBuffer.Vertices[i].TCoords.X = (1.f + eyePlaneS.dotProduct ( v ) ) * 0.5f;
				MeshBuffer.Vertices[i].TCoords.Y = 1.f - ( (1.f + eyePlaneT.dotProduct ( v ) ) * 0.5f );
			}

			ret = 0;

		} break;
	}

	return ret;
}



/*
	Transform Texture Coordinates
*/
void CQuake3ShaderSceneNode::transformtex ( const core::matrix4 &m, const u32 addressMode )
{
	u32 i;
	const u32 vsize = MeshBuffer.Vertices.size();

	f32 tx1;
	f32 ty1;

	if ( addressMode )
	{
		for ( i = 0; i != vsize; ++i )
		{
			core::vector2df &tx = MeshBuffer.Vertices[i].TCoords;

			tx1 = m[0] * tx.X + m[4] * tx.Y + m[8];
			ty1 = m[1] * tx.X + m[5] * tx.Y + m[9];

			tx.X = tx1;
			tx.Y = ty1;
		}
	}
	else
	{

		for ( i = 0; i != vsize; ++i )
		{
			core::vector2df &tx = MeshBuffer.Vertices[i].TCoords;

			tx1 = m[0] * tx.X + m[4] * tx.Y + m[8];
			ty1 = m[1] * tx.X + m[5] * tx.Y + m[9];

			tx.X = tx1 <= 0.f ? 0.f : tx1 >= 1.f ? 1.f : tx1;
			tx.Y = ty1 <= 0.f ? 0.f : ty1 >= 1.f ? 1.f : ty1;

			//tx.X = core::clamp ( tx1, 0.f, 1.f );
			//tx.Y = core::clamp ( ty1, 0.f, 1.f );
		}
	}
}




/*
	Texture & Vertex Transform Animator

	Return a Texture Transformation for this stage
	Vertex transformation are called if found

*/
void CQuake3ShaderSceneNode::animate( u32 stage,core::matrix4 &texture )
{
	const f32 dt = TimeAbs;

	const quake3::SVarGroup *group = Shader->getGroup ( stage );

	// select current texture
	if ( Q3Texture [ stage ].TextureFrequency != 0.f )
	{
		s32 v = core::floor32 ( dt * Q3Texture [ stage ].TextureFrequency );
		Q3Texture [ stage ].TextureIndex = v % Q3Texture [ stage ].Texture.size();
	}


	core::matrix4 texturem;
	core::matrix4 m2;
	quake3::SModifierFunction function;

	f32 f0;
	f32 f1;
	u32 textureMatrixFound = 0;

	// walk group for all modifiers
	for ( u32 g = 0; g != group->Variable.size (); ++g )
	{
		const quake3::SVariable &v = group->Variable[g];

		// get the modifier 
		static const c8 * modifierList[] =
		{ 
			"tcmod","deformvertexes","rgbgen","tcgen","map"
		};

		u32 pos = 0;
		function.masterfunc0 = quake3::isEqual ( v.name, pos, modifierList, 5 );

		if ( -2 == function.masterfunc0 )
			continue;

		switch ( function.masterfunc0 )
		{
			//tcmod
			case 0:
				if ( 0 == textureMatrixFound )
				{
					texturem.makeIdentity ();
					textureMatrixFound = 1;
				}
				m2.makeIdentity ();
				break;
		}

		// get the modifier function
		static const c8 * funclist[] =
		{ 
			"scroll","scale","rotate","stretch","turb",
			"wave","identity","vertex",
			"texture","lightmap","environment","$lightmap",
			"bulge","autosprite","autosprite2"
		};

		pos = 0;
		function.masterfunc1 = quake3::isEqual ( v.content, pos, funclist, 14 );

		switch ( function.masterfunc1 )
		{
			case 0:
				// scroll
				f0 = quake3::getAsFloat ( v.content, pos ) * dt;
				f1 = quake3::getAsFloat ( v.content, pos ) * dt;
				m2.setTextureTranslate ( f0, f1 );
				break;
			case 1:
				// scale
				f0 = quake3::getAsFloat ( v.content, pos );
				f1 = quake3::getAsFloat ( v.content, pos );
				m2.setTextureScale ( f0, f1 );
				break;
			case 2:
				//rotate
				m2.setTextureRotationCenter ( quake3::getAsFloat ( v.content, pos ) * core::DEGTORAD * dt );
				break;
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			{
				// turb == sin, default == sin
				function.func = 0;

				if ( function.masterfunc1 == 5 && function.masterfunc0 == 1)
				{
					// deformvertexes, wave
					function.wave = quake3::getAsFloat ( v.content, pos );
				}

				if (	function.masterfunc1 == 3 ||
						function.masterfunc1 == 4 ||
						function.masterfunc1 == 5
						)
				{
					// stretch, wave, tub
					quake3::getModifierFunc ( function, v.content, pos );
				}

				switch ( function.masterfunc1 )
				{
					case 3:
						// stretch
						f0 = core::reciprocal ( function.evaluate ( dt ) );
						m2.setTextureScaleCenter ( f0, f0 );
						break;
					case 4:
						// turb
						f0 = function.evaluate ( dt );
						function.freq *= 2.f;
						f1 = function.evaluate ( dt );
						m2.setTextureTranslate ( f1, f0 );
						break;
					case 5:
						if ( function.masterfunc0 == 1 )
							vertextransform_wave ( dt, function );
						else
							rgbgen ( dt, function );
						break;
					case 6:
					case 7:
						rgbgen ( dt, function );
						break;
				}
				
			} break;
			case 8:
			case 9:
			case 10:
				// "texture","lightmap","environment"
				function.tcgen = function.masterfunc1;
				break;
			case 11:
				// map == lightmap, tcgen == lightmap
				function.tcgen = 9;
				break;
			case 12:
				// deformvertexes bulge
				function.bulgewidth = quake3::getAsFloat ( v.content, pos );
				function.bulgeheight = quake3::getAsFloat ( v.content, pos );
				function.bulgespeed = quake3::getAsFloat ( v.content, pos );

				vertextransform_bulge ( dt, function );
				break;

			case 13:
			case 14:
				// deformvertexes autosprite
				vertextransform_autosprite ( dt, function);
				break;

		} // func

		switch ( function.masterfunc0 )
		{
			case 0:
				texturem *= m2;
				break;
		}

	} // group

	// texture coordinate modifier
	tcgen( dt, function, texturem );
}


void CQuake3ShaderSceneNode::OnAnimate(u32 timeMs)
{
	TimeAbs = f32( timeMs ) * ( 1.f/1000.f);
}

const core::aabbox3d<f32>& CQuake3ShaderSceneNode::getBoundingBox() const
{
	return MeshBuffer.getBoundingBox ();
}


u32 CQuake3ShaderSceneNode::getMaterialCount() const
{
	return Q3Texture.size();
}

video::SMaterial& CQuake3ShaderSceneNode::getMaterial(u32 i)
{
	video::SMaterial& m = MeshBuffer.getMaterial();
	m.setTexture(0, 0);
	if ( Q3Texture [ i ].TextureIndex )
		m.setTexture(0, Q3Texture [ i ].Texture [ Q3Texture [ i ].TextureIndex ]);
	return m;
}	


} // end namespace scene
} // end namespace irr

#endif

