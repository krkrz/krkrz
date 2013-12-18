// Copyright (C) 2007-2008 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_ANIMATED_MESH_MD3_H_INCLUDED__
#define __I_ANIMATED_MESH_MD3_H_INCLUDED__

#include "IAnimatedMesh.h"
#include "IQ3Shader.h"
#include "quaternion.h"

namespace irr
{
namespace scene
{

	enum eMD3Models
	{
		EMD3_HEAD = 0,
		EMD3_UPPER,
		EMD3_LOWER,
		EMD3_WEAPON,
		EMD3_NUMMODELS
	};

	//! Animation list
	enum EMD3_ANIMATION_TYPE
	{
		// Animations for both lower and upper parts of the player
		EMD3_BOTH_DEATH_1 = 0,
		EMD3_BOTH_DEAD_1,
		EMD3_BOTH_DEATH_2,
		EMD3_BOTH_DEAD_2,
		EMD3_BOTH_DEATH_3,
		EMD3_BOTH_DEAD_3,

		// Animations for the upper part
		EMD3_TORSO_GESTURE,
		EMD3_TORSO_ATTACK_1,
		EMD3_TORSO_ATTACK_2,
		EMD3_TORSO_DROP,
		EMD3_TORSO_RAISE,
		EMD3_TORSO_STAND_1,
		EMD3_TORSO_STAND_2,

		// Animations for the lower part
		EMD3_LEGS_WALK_CROUCH,
		EMD3_LEGS_WALK,
		EMD3_LEGS_RUN,
		EMD3_LEGS_BACK,
		EMD3_LEGS_SWIM,
		EMD3_LEGS_JUMP_1,
		EMD3_LEGS_LAND_1,
		EMD3_LEGS_JUMP_2,
		EMD3_LEGS_LAND_2,
		EMD3_LEGS_IDLE,
		EMD3_LEGS_IDLE_CROUCH,
		EMD3_LEGS_TURN,

		//! Not an animation, but amount of animation types.
		EMD3_ANIMATION_COUNT
	};

	struct SMD3AnimationInfo
	{
		//! First frame
		s32 first;
		//! Last frame
		s32 num;
		//! Looping frames
		s32 looping;
		//! Frames per second
		s32 fps;
	};


// byte-align structures
#if defined(_MSC_VER) || defined(__BORLANDC__) || defined (__BCPLUSPLUS__)
#	pragma pack( push, packing )
#	pragma pack( 1 )
#	define PACK_STRUCT
#elif defined( __GNUC__ )
#	define PACK_STRUCT	__attribute__((packed))
#else
#	error compiler not supported
#endif

	//! this holds the header info of the MD3 file
	struct SMD3Header
	{
		c8	headerID[4];	//id of file, always "IDP3"
		s32	Version;	//this is a version number, always 15
		s8	fileName[68];	//sometimes left Blank... 65 chars, 32bit aligned == 68 chars
		s32	numFrames;	//number of KeyFrames
		s32	numTags;	//number of 'tags' per frame
		s32	numMeshes;	//number of meshes/skins
		s32	numMaxSkins;	//maximum number of unique skins used in md3 file
		s32	headerSize;	//always equal to the length of this header
		s32	tagStart;	//starting position of tag-structures
		s32	tagEnd;		//ending position of tag-structures/starting position of mesh-structures
		s32	fileSize;
	};

	//! this holds the header info of an MD3 mesh section
	struct SMD3MeshHeader
	{
		c8 meshID[4];		//id, must be IDP3
		c8 meshName[68];	//name of mesh 65 chars, 32 bit aligned == 68 chars

		s32 numFrames;		//number of meshframes in mesh
		s32 numShader;		//number of skins in mesh
		s32 numVertices;	//number of vertices
		s32 numTriangles;	//number of Triangles

		s32 offset_triangles;	//starting position of Triangle data, relative to start of Mesh_Header
		s32 offset_shaders;	//size of header
		s32 offset_st;		//starting position of texvector data, relative to start of Mesh_Header
		s32 vertexStart;	//starting position of vertex data,relative to start of Mesh_Header
		s32 offset_end;
	};


	//! Compressed Vertex Data
	struct SMD3Vertex
	{
		s16 position[3];
		u8 normal[2];
	};

	//! Texture Coordinate
	struct SMD3TexCoord
	{
		f32 u;
		f32 v;
	};

	//! Triangle Index
	struct SMD3Face
	{
		s32 Index[3];
	};


// Default alignment
#if defined(_MSC_VER) || defined(__BORLANDC__) || defined (__BCPLUSPLUS__)
#	pragma pack( pop, packing )
#endif

#undef PACK_STRUCT

	//! Holding Frame Data for a Mesh
	struct SMD3MeshBuffer : public IReferenceCounted
	{
		SMD3MeshHeader MeshHeader;

		core::array < core::stringc > Shader;
		core::array < s32 > Indices;
		core::array < SMD3Vertex > Vertices;
		core::array < SMD3TexCoord > Tex;
	};

	//! hold a tag info for connecting meshes
	/** Basically its an alternate way to describe a transformation. */
	struct SMD3QuaterionTag
	{
		SMD3QuaterionTag() {}

		SMD3QuaterionTag( const core::stringc& name )
			: Name ( name ) {}

		// construct from a matrix
		SMD3QuaterionTag ( const core::stringc& name, const core::matrix4 &m )
		{
			Name = name;
			position = m.getTranslation ();
			rotation = m;
		}

		// set to matrix
		void setto ( core::matrix4 &m )
		{
			rotation.getMatrix ( m );
			m.setTranslation ( position );
		}

		// construct from a position and euler angles in degrees
		SMD3QuaterionTag ( const core::vector3df &pos, const core::vector3df &angle )
		{
			position = pos;
			rotation.set ( angle * core::DEGTORAD );
		}

		bool operator == ( const SMD3QuaterionTag &other ) const
		{
			return Name == other.Name;
		}

		core::stringc Name;
		core::vector3df position;
		core::quaternion rotation;
	};

	//! holds a associative list of named quaternions
	struct SMD3QuaterionTagList : public virtual IReferenceCounted
	{
		SMD3QuaterionTag* get ( const core::stringc& name )
		{
			SMD3QuaterionTag search ( name );
			s32 index = Container.linear_search ( search );
			if ( index >= 0 )
				return &Container[index];
			return 0;
		}

		u32 size () const
		{
			return Container.size();
		}

		SMD3QuaterionTag& operator[] (u32 index )
		{
			return Container[index];
		}

		core::array < SMD3QuaterionTag > Container;
	};


	//! Holding Frames Buffers and Tag Infos
	struct SMD3Mesh: public IReferenceCounted
	{
		~SMD3Mesh()
		{
			for (u32 i=0; i<Buffer.size(); ++i)
				Buffer[i]->drop();
		}

		SMD3Header MD3Header;
		core::stringc Name;
		core::array < SMD3MeshBuffer * > Buffer;
		SMD3QuaterionTagList TagList;
	};


	//! Interface for using some special functions of MD3 meshes
	class IAnimatedMeshMD3 : public IAnimatedMesh
	{
	public:

		//! tune how many frames you want to render inbetween.
		virtual void setInterpolationShift ( u32 shift, u32 loopMode ) = 0;

		//! get the tag list of the mesh.
		virtual SMD3QuaterionTagList *getTagList(s32 frame, s32 detailLevel, s32 startFrameLoop, s32 endFrameLoop) = 0;

		//! get the original md3 mesh.
		virtual SMD3Mesh * getOriginalMesh () = 0;
	};

} // end namespace scene
} // end namespace irr

#endif

