// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __E_SCENE_NODE_TYPES_H_INCLUDED__
#define __E_SCENE_NODE_TYPES_H_INCLUDED__

#include "irrTypes.h"

namespace irr
{
namespace scene
{

	//! An enumeration for all types of built-in scene nodes
	/** A scene node type is represented by a four character code
	such as 'cube' or 'mesh' instead of simple numbers, to avoid
	name clashes with external scene nodes.*/
	enum ESCENE_NODE_TYPE
	{
		//! simple cube scene node
		ESNT_CUBE           = MAKE_IRR_ID('c','u','b','e'),

		//! Sphere scene node
		ESNT_SPHERE         = MAKE_IRR_ID('s','p','h','r'),

		//! Text Scene Node
		ESNT_TEXT           = MAKE_IRR_ID('t','e','x','t'),

		//! Water Surface Scene Node
		ESNT_WATER_SURFACE  = MAKE_IRR_ID('w','a','t','r'),

		//! Terrain Scene Node
		ESNT_TERRAIN        = MAKE_IRR_ID('t','e','r','r'),

		//! Sky Box Scene Node
		ESNT_SKY_BOX        = MAKE_IRR_ID('s','k','y','_'),

		//! Shadow Volume Scene Node
		ESNT_SHADOW_VOLUME  = MAKE_IRR_ID('s','h','d','w'),

		//! OctTree Scene Node
		ESNT_OCT_TREE       = MAKE_IRR_ID('o','c','t','t'),

		//! Mesh Scene Node
		ESNT_MESH           = MAKE_IRR_ID('m','e','s','h'),

		//! Light Scene Node
		ESNT_LIGHT          = MAKE_IRR_ID('l','g','h','t'),

		//! Empty Scene Node
		ESNT_EMPTY          = MAKE_IRR_ID('e','m','t','y'),

		//! Dummy Transformation Scene Node
		ESNT_DUMMY_TRANSFORMATION = MAKE_IRR_ID('d','m','m','y'),

		//! Camera Scene Node
		ESNT_CAMERA         = MAKE_IRR_ID('c','a','m','_'),

		//! Maya Camera Scene Node
		ESNT_CAMERA_MAYA    = MAKE_IRR_ID('c','a','m','M'),

		//! First Person Shooter style Camera
		ESNT_CAMERA_FPS     = MAKE_IRR_ID('c','a','m','F'),

		//! Billboard Scene Node
		ESNT_BILLBOARD      = MAKE_IRR_ID('b','i','l','l'),

		//! Animated Mesh Scene Node
		ESNT_ANIMATED_MESH  = MAKE_IRR_ID('a','m','s','h'),

		//! Particle System Scene Node
		ESNT_PARTICLE_SYSTEM = MAKE_IRR_ID('p','t','c','l'),

		//! Quake3 Model Scene Node ( has tag to link to )
		ESNT_MD3_SCENE_NODE  = MAKE_IRR_ID('m','d','3','_'),

		//! Unknown scene node
		ESNT_UNKNOWN        = MAKE_IRR_ID('u','n','k','n')
	};



} // end namespace scene
} // end namespace irr


#endif

