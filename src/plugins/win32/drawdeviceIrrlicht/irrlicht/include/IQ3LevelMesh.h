// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_Q3_LEVEL_MESH_H_INCLUDED__
#define __I_Q3_LEVEL_MESH_H_INCLUDED__

#include "IAnimatedMesh.h"
#include "IQ3Shader.h"

namespace irr
{
namespace scene
{
	//! Interface for a Mesh which can be loaded directly from a Quake3 .bsp-file.
	/** The Mesh tries to load all textures of the map. There are currently
	no additional methods in this class, but maybe there will be some in later
	releases if there are feature requests. */
	class IQ3LevelMesh : public IAnimatedMesh
	{
	public:

		//! releases a Mesh from the Q3 Loader
		virtual void releaseMesh ( s32 index ) = 0;

		//! loads the shader definition
		/** Either from file ( we assume /scripts on fileNameIsValid == 0 ) */
		virtual const quake3::SShader * getShader ( const c8 * filename, s32 fileNameIsValid ) = 0;

		//! returns a already loaded Shader
		virtual const quake3::SShader * getShader ( u32 index ) const = 0;

		//! get's an interface to the entities
		virtual const quake3::tQ3EntityList & getEntityList () = 0;

	};

} // end namespace scene
} // end namespace irr

#endif

