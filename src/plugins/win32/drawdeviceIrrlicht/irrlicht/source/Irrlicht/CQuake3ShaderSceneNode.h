// Copyright (C) 2002-2008 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_QUAKE3_SCENE_NODE_H_INCLUDED__
#define __C_QUAKE3_SCENE_NODE_H_INCLUDED__

#include "ISceneNode.h"
#include "IQ3Shader.h"
#include "IFileSystem.h"
#include "SMeshBuffer.h"
#include "SMeshBufferLightMap.h"

namespace irr
{
namespace scene
{

//! Scene node which is a quake3 shader.
class CQuake3ShaderSceneNode : public scene::ISceneNode
{
public:

	CQuake3ShaderSceneNode( ISceneNode* parent, ISceneManager* mgr,s32 id,
				io::IFileSystem *fileSystem,IMeshBuffer *buffer,
				const quake3::SShader * shader
		);

	virtual void OnRegisterSceneNode();
	virtual void render();
	virtual void OnAnimate(u32 timeMs);
	virtual const core::aabbox3d<f32>& getBoundingBox() const;

	virtual u32 getMaterialCount() const;
	virtual video::SMaterial& getMaterial(u32 i);

private:
	SMeshBuffer MeshBuffer;
	SMeshBufferLightMap Original;
	const quake3::SShader * Shader;

	struct SQ3Texture
	{
		SQ3Texture() :
			TextureIndex(0),
			TextureFrequency(0.f),
			TextureAddressMode(video::ETC_REPEAT) {}

		quake3::tTexArray Texture;

		u32 TextureIndex;
		f32 TextureFrequency;
		video::E_TEXTURE_CLAMP TextureAddressMode;	// Wrapping/Clamping
	};

	core::array< SQ3Texture > Q3Texture;

	void loadTextures( io::IFileSystem * fileSystem );
	void cloneBuffer( scene::SMeshBufferLightMap * buffer );

	void vertextransform_wave( f32 dt, quake3::SModifierFunction &function );
	void vertextransform_bulge( f32 dt, quake3::SModifierFunction &function );
	void vertextransform_autosprite( f32 dt, quake3::SModifierFunction &function );

	void rgbgen( f32 dt, quake3::SModifierFunction &function );
	u32 tcgen( f32 dt, quake3::SModifierFunction &function, core::matrix4 &texture );

	void transformtex( const core::matrix4 &m, const u32 clamp );

	f32 TimeAbs;
	void animate( u32 stage, core::matrix4 &texture );


	s32 PassedCulling;
	s32 StageCall;

};


} // end namespace scene
} // end namespace irr


#endif

