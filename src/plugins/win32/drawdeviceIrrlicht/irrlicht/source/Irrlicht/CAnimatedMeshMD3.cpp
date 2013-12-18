// Copyright (C) 2002-2008 Nikolaus Gebhardt / Fabio Concas / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_MD3_LOADER_

#include "CAnimatedMeshMD3.h"
#include "os.h"

namespace irr
{
namespace scene
{


#if defined(_MSC_VER) ||  defined(__BORLANDC__) || defined (__BCPLUSPLUS__) 
#	pragma pack( push, packing )
#	pragma pack( 1 )
#	define PACK_STRUCT
#elif defined( __GNUC__ )
#	define PACK_STRUCT	__attribute__((packed))
#else
#	error compiler not supported
#endif


struct SMD3Bone
{
	f32  Mins[3];		// bounding box per frame
	f32  Maxs[3];
	f32  Position[3];	// position of bounding box
	f32  scale;
	c8   creator[16];
};


struct SMD3Tag
{
	c8 Name[64];		//name of 'tag' as it's usually called in the md3 files try to see it as a sub-mesh/seperate mesh-part.
	f32 position[3];	//relative position of tag
	f32 rotationMatrix[9];	//3x3 rotation direction of tag
};

struct SMD3Skin
{
	c8 name[68];	// name of skin
};



// Default alignment
#if defined(_MSC_VER) ||  defined(__BORLANDC__) || defined (__BCPLUSPLUS__) 
#	pragma pack( pop, packing )
#endif

#undef PACK_STRUCT


//! Constructor
CAnimatedMeshMD3::CAnimatedMeshMD3 ()
: Mesh ( 0 )
{
#ifdef _DEBUG
	setDebugName("CAnimatedMeshMD3");
#endif

	Mesh = new SMD3Mesh ();
	memset ( &Mesh->MD3Header, 0, sizeof ( Mesh->MD3Header ) );

	setInterpolationShift ( 0, 0 );
}


//! Destructor
CAnimatedMeshMD3::~CAnimatedMeshMD3()
{
	if ( Mesh )
		Mesh->drop ();
}


//! Returns the amount of frames in milliseconds. If the amount is 1, it is a static (=non animated) mesh.
u32 CAnimatedMeshMD3::getFrameCount() const
{
	return Mesh->MD3Header.numFrames << IPolShift;
}


//! Rendering Hint
void CAnimatedMeshMD3::setInterpolationShift ( u32 shift, u32 loopMode )
{
	IPolShift = shift;
}


//! Returns the animated tag list based on a detail level. 0 is the lowest, 255 the highest detail.
SMD3QuaterionTagList *CAnimatedMeshMD3::getTagList(s32 frame, s32 detailLevel, s32 startFrameLoop, s32 endFrameLoop)
{
	if ( 0 == Mesh )
		return 0;

	getMesh ( frame, detailLevel, startFrameLoop, endFrameLoop );
	return &TagListIPol;
}


//! Returns the animated mesh based on a detail level. 0 is the lowest, 255 the highest detail.
IMesh* CAnimatedMeshMD3::getMesh(s32 frame, s32 detailLevel, s32 startFrameLoop, s32 endFrameLoop)
{
	if ( 0 == Mesh )
		return 0;

	u32 i;

	//! check if we have the mesh in our private cache
	SCacheInfo candidate ( frame, startFrameLoop, endFrameLoop );
	if ( candidate == Current )
		return &MeshIPol;

	startFrameLoop = core::s32_max ( 0, startFrameLoop >> IPolShift );
	endFrameLoop = core::if_c_a_else_b ( endFrameLoop < 0, Mesh->MD3Header.numFrames - 1, endFrameLoop >> IPolShift );

	const u32 mask = 1 << IPolShift;

	s32 frameA;
	s32 frameB;
	f32 iPol;

	if ( LoopMode )
	{
		// correct frame to "pixel center"
		frame -= mask >> 1;

		// interpolation
		iPol = f32(frame & ( mask - 1 )) * core::reciprocal ( f32(mask) );

		// wrap anim
		frame >>= IPolShift;
		frameA = core::if_c_a_else_b ( frame < startFrameLoop, endFrameLoop, frame );
		frameB = core::if_c_a_else_b ( frameA + 1 > endFrameLoop, startFrameLoop, frameA + 1 );
	}
	else
	{
		// correct frame to "pixel center"
		frame -= mask >> 1;

		iPol = f32(frame & ( mask - 1 )) * core::reciprocal ( f32(mask) );

		// clamp anim
		frame >>= IPolShift;
		frameA = core::s32_clamp ( frame, startFrameLoop, endFrameLoop );
		frameB = core::s32_min ( frameA + 1, endFrameLoop );
	}

	// build current vertex
	for ( i = 0; i!= Mesh->Buffer.size (); ++i )
	{
		buildVertexArray(frameA, frameB, iPol,
					Mesh->Buffer[i],
					(SMeshBuffer*) MeshIPol.getMeshBuffer(i)
				);
	}
	MeshIPol.recalculateBoundingBox ();

	// build current tags
	buildTagArray ( frameA, frameB, iPol );

	Current = candidate;
	return &MeshIPol;
}

//! create a Irrlicht MeshBuffer for a MD3 MeshBuffer
IMeshBuffer * CAnimatedMeshMD3::createMeshBuffer ( const SMD3MeshBuffer * source )
{
	SMeshBuffer * dest = new SMeshBuffer ();
	dest->Vertices.set_used ( source->MeshHeader.numVertices );
	dest->Indices.set_used ( source->Indices.size () );

	u32 i;

	// fill in static face info
	for ( i = 0; i < source->Indices.size (); i += 3 )
	{
		dest->Indices[i + 0 ] = (u16) source->Indices[i + 0];
		dest->Indices[i + 1 ] = (u16) source->Indices[i + 1];
		dest->Indices[i + 2 ] = (u16) source->Indices[i + 2];
	}

	// fill in static vertex info
	for ( i = 0; i!= (u32)source->MeshHeader.numVertices; ++i )
	{
		video::S3DVertex &v = dest->Vertices [ i ];
		v.Color = 0xFFFFFFFF;
		v.TCoords.X = source->Tex[i].u;
		v.TCoords.Y = source->Tex[i].v;
	}
	return dest;
}


//! build final mesh's vertices from frames frameA and frameB with linear interpolation.
void CAnimatedMeshMD3::buildVertexArray ( u32 frameA, u32 frameB, f32 interpolate,
						const SMD3MeshBuffer * source,
						SMeshBuffer * dest
					)
{
	const u32 frameOffsetA = frameA * source->MeshHeader.numVertices;
	const u32 frameOffsetB = frameB * source->MeshHeader.numVertices;
	const f32 scale = ( 1.f/ 64.f );

	for (s32 i = 0; i != source->MeshHeader.numVertices; ++i)
	{
		video::S3DVertex &v = dest->Vertices [ i ];

		const SMD3Vertex &vA = source->Vertices [ frameOffsetA + i ];
		const SMD3Vertex &vB = source->Vertices [ frameOffsetB + i ];

		// position
		v.Pos.X = scale * ( vA.position[0] + interpolate * ( vB.position[0] - vA.position[0] ) );
		v.Pos.Y = scale * ( vA.position[2] + interpolate * ( vB.position[2] - vA.position[2] ) );
		v.Pos.Z = scale * ( vA.position[1] + interpolate * ( vB.position[1] - vA.position[1] ) );

		// normal
		const core::vector3df nA(getNormal ( vA.normal[0], vA.normal[1] ));
		const core::vector3df nB(getNormal ( vB.normal[0], vB.normal[1] ));

		v.Normal.X = nA.X + interpolate * ( nB.X - nA.X );
		v.Normal.Y = nA.Z + interpolate * ( nB.Z - nA.Z );
		v.Normal.Z = nA.Y + interpolate * ( nB.Y - nA.Y );
	}

	dest->recalculateBoundingBox ();
}


//! build final mesh's tag from frames frameA and frameB with linear interpolation.
void CAnimatedMeshMD3::buildTagArray ( u32 frameA, u32 frameB, f32 interpolate )
{
	const u32 frameOffsetA = frameA * Mesh->MD3Header.numTags;
	const u32 frameOffsetB = frameB * Mesh->MD3Header.numTags;

	for ( s32 i = 0; i != Mesh->MD3Header.numTags; ++i )
	{
		SMD3QuaterionTag &d = TagListIPol [ i ];

		const SMD3QuaterionTag &qA = Mesh->TagList.Container[ frameOffsetA + i];
		const SMD3QuaterionTag &qB = Mesh->TagList.Container[ frameOffsetB + i];

		// rotation
		d.rotation.slerp( qA.rotation, qB.rotation, interpolate );

		// position
		d.position.X = qA.position.X + interpolate * ( qB.position.X - qA.position.X );
		d.position.Y = qA.position.Y + interpolate * ( qB.position.Y - qA.position.Y );
		d.position.Z = qA.position.Z + interpolate * ( qB.position.Z - qA.position.Z );
	}
}


/*!
	loads a model
*/
bool CAnimatedMeshMD3::loadModelFile( u32 modelIndex, io::IReadFile* file)
{
	if (!file)
		return false;

	//! Check MD3Header
	{
		file->read( &Mesh->MD3Header, sizeof(SMD3Header) );

		if ( strncmp("IDP3", Mesh->MD3Header.headerID, 4) )
		{
			os::Printer::log("MD3 Loader: invalid header");
			return false;
		}
	}

	//! store model name
	Mesh->Name = file->getFileName();

	//! Bone Frames Data ( ignore )
	//! Tag Data
	const u32 totalTags = Mesh->MD3Header.numTags * Mesh->MD3Header.numFrames;

	SMD3Tag import;
	SMD3QuaterionTag exp;
	u32 i;

	file->seek( Mesh->MD3Header.tagStart );
	for (i = 0; i != totalTags; ++i )
	{
		file->read(&import, sizeof(import) );

		//! tag name
		exp.Name = import.Name;

		//! position
		exp.position.X = import.position[0];
		exp.position.Y = import.position[2];
		exp.position.Z = import.position[1];

		//! construct quaternion from a RH 3x3 Matrix
		exp.rotation.set (import.rotationMatrix[7],
					0.f,
					-import.rotationMatrix[6],
					1 + import.rotationMatrix[8]);
		exp.rotation.normalize ();
		Mesh->TagList.Container.push_back ( exp );
	}

	//! Meshes
	u32 offset = Mesh->MD3Header.tagEnd;

	SMD3Skin skin;

	for (i = 0; i != (u32)Mesh->MD3Header.numMeshes; ++i )
	{
		//! construct a new mesh buffer
		SMD3MeshBuffer * buf = new SMD3MeshBuffer ();

		// !read mesh header info
		SMD3MeshHeader &meshHeader = buf->MeshHeader;

		//! read mesh info
		file->seek( offset );
		file->read( &meshHeader, sizeof(SMD3MeshHeader) );

		//! prepare memory
		buf->Vertices.set_used ( meshHeader.numVertices * Mesh->MD3Header.numFrames );
		buf->Indices.set_used ( meshHeader.numTriangles * 3 );
		buf->Tex.set_used ( meshHeader.numVertices );


		//! read skins (shaders)
		file->seek( offset + buf->MeshHeader.offset_shaders );
		for ( s32 g = 0; g != buf->MeshHeader.numShader; ++g )
		{
			file->read( &skin, sizeof(skin) );
			buf->Shader.push_back ( skin.name );
		}

		//! read texture coordinates
		file->seek( offset + buf->MeshHeader.offset_st);
		file->read( buf->Tex.pointer(), buf->MeshHeader.numVertices * sizeof(SMD3TexCoord) );

		//! read vertices
		file->seek(offset + meshHeader.vertexStart);
		file->read( buf->Vertices.pointer(), Mesh->MD3Header.numFrames * meshHeader.numVertices * sizeof(SMD3Vertex) );

		//! read indices
		file->seek( offset + meshHeader.offset_triangles );
		file->read( buf->Indices.pointer(), meshHeader.numTriangles * sizeof(SMD3Face) );

		//! store meshBuffer
		Mesh->Buffer.push_back ( buf );

		offset += meshHeader.offset_end;
	}

	// Init Mesh Interpolation
	for ( i = 0; i != Mesh->Buffer.size (); ++i )
	{
		IMeshBuffer * buffer = createMeshBuffer ( Mesh->Buffer[i] );
		MeshIPol.addMeshBuffer ( buffer );
		buffer->drop ();
	}

	// Init Tag Interpolation
	for (i = 0; i != (u32)Mesh->MD3Header.numTags; ++i )
	{
		TagListIPol.Container.push_back ( Mesh->TagList.Container[i] );
	}

	return true;
}


SMD3Mesh * CAnimatedMeshMD3::getOriginalMesh ()
{
	return Mesh;
}


//! Returns an axis aligned bounding box
const core::aabbox3d<f32>& CAnimatedMeshMD3::getBoundingBox() const
{
	return MeshIPol.BoundingBox;
}


//! Returns the type of the animated mesh.
E_ANIMATED_MESH_TYPE CAnimatedMeshMD3::getMeshType() const
{
	return EAMT_MD3;
}


} // end namespace scene
} // end namespace irr

#endif // _IRR_COMPILE_WITH_MD3_LOADER_

