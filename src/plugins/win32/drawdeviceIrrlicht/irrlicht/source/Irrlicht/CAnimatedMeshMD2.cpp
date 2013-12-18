// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_MD2_LOADER_

#include "CAnimatedMeshMD2.h"
#include "os.h"
#include "SColor.h"
#include "IReadFile.h"
#include "irrMath.h"

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

	// structs needed to load the md2-format

	const s32 MD2_MAGIC_NUMBER = 844121161;
	const s32 MD2_VERSION		= 8;
	const s32 MD2_MAX_VERTS		= 2048;

	// TA: private
	const s32 MD2_FRAME_SHIFT	= 2;
	const f32 MD2_FRAME_SHIFT_RECIPROCAL = 1.f / ( 1 << MD2_FRAME_SHIFT );

	struct SMD2Header
	{
		s32 magic;
		s32 version;
		s32 skinWidth;
		s32 skinHeight;
		s32 frameSize;
		s32 numSkins;
		s32 numVertices;
		s32 numTexcoords;
		s32 numTriangles;
		s32 numGlCommands;
		s32 numFrames;
		s32 offsetSkins;
		s32 offsetTexcoords;
		s32 offsetTriangles;
		s32 offsetFrames;
		s32 offsetGlCommands;
		s32 offsetEnd;
	} PACK_STRUCT;

	struct SMD2Vertex
	{
		u8 vertex[3];
		u8 lightNormalIndex;
	} PACK_STRUCT;

	struct SMD2Frame
	{
		f32	scale[3];
		f32	translate[3];
		c8	name[16];
		SMD2Vertex vertices[1];
	} PACK_STRUCT;

	struct SMD2Triangle
	{
		u16 vertexIndices[3];
		u16 textureIndices[3];
	} PACK_STRUCT;

	struct SMD2TextureCoordinate
	{
		s16 s;
		s16 t;
	} PACK_STRUCT;

	struct SMD2GLCommand
	{
		f32 s, t;
		s32 vertexIndex;
	} PACK_STRUCT;

// Default alignment
#if defined(_MSC_VER) ||  defined(__BORLANDC__) || defined (__BCPLUSPLUS__) 
#	pragma pack( pop, packing )
#endif

#undef PACK_STRUCT


const s32 Q2_VERTEX_NORMAL_TABLE_SIZE = 162;

static const f32 Q2_VERTEX_NORMAL_TABLE[Q2_VERTEX_NORMAL_TABLE_SIZE][3] = {
	{-0.525731f, 0.000000f, 0.850651f},
	{-0.442863f, 0.238856f, 0.864188f},
	{-0.295242f, 0.000000f, 0.955423f},
	{-0.309017f, 0.500000f, 0.809017f},
	{-0.162460f, 0.262866f, 0.951056f},
	{0.000000f, 0.000000f, 1.000000f},
	{0.000000f, 0.850651f, 0.525731f},
	{-0.147621f, 0.716567f, 0.681718f},
	{0.147621f, 0.716567f, 0.681718f},
	{0.000000f, 0.525731f, 0.850651f},
	{0.309017f, 0.500000f, 0.809017f},
	{0.525731f, 0.000000f, 0.850651f},
	{0.295242f, 0.000000f, 0.955423f},
	{0.442863f, 0.238856f, 0.864188f},
	{0.162460f, 0.262866f, 0.951056f},
	{-0.681718f, 0.147621f, 0.716567f},
	{-0.809017f, 0.309017f, 0.500000f},
	{-0.587785f, 0.425325f, 0.688191f},
	{-0.850651f, 0.525731f, 0.000000f},
	{-0.864188f, 0.442863f, 0.238856f},
	{-0.716567f, 0.681718f, 0.147621f},
	{-0.688191f, 0.587785f, 0.425325f},
	{-0.500000f, 0.809017f, 0.309017f},
	{-0.238856f, 0.864188f, 0.442863f},
	{-0.425325f, 0.688191f, 0.587785f},
	{-0.716567f, 0.681718f, -0.147621f},
	{-0.500000f, 0.809017f, -0.309017f},
	{-0.525731f, 0.850651f, 0.000000f},
	{0.000000f, 0.850651f, -0.525731f},
	{-0.238856f, 0.864188f, -0.442863f},
	{0.000000f, 0.955423f, -0.295242f},
	{-0.262866f, 0.951056f, -0.162460f},
	{0.000000f, 1.000000f, 0.000000f},
	{0.000000f, 0.955423f, 0.295242f},
	{-0.262866f, 0.951056f, 0.162460f},
	{0.238856f, 0.864188f, 0.442863f},
	{0.262866f, 0.951056f, 0.162460f},
	{0.500000f, 0.809017f, 0.309017f},
	{0.238856f, 0.864188f, -0.442863f},
	{0.262866f, 0.951056f, -0.162460f},
	{0.500000f, 0.809017f, -0.309017f},
	{0.850651f, 0.525731f, 0.000000f},
	{0.716567f, 0.681718f, 0.147621f},
	{0.716567f, 0.681718f, -0.147621f},
	{0.525731f, 0.850651f, 0.000000f},
	{0.425325f, 0.688191f, 0.587785f},
	{0.864188f, 0.442863f, 0.238856f},
	{0.688191f, 0.587785f, 0.425325f},
	{0.809017f, 0.309017f, 0.500000f},
	{0.681718f, 0.147621f, 0.716567f},
	{0.587785f, 0.425325f, 0.688191f},
	{0.955423f, 0.295242f, 0.000000f},
	{1.000000f, 0.000000f, 0.000000f},
	{0.951056f, 0.162460f, 0.262866f},
	{0.850651f, -0.525731f, 0.000000f},
	{0.955423f, -0.295242f, 0.000000f},
	{0.864188f, -0.442863f, 0.238856f},
	{0.951056f, -0.162460f, 0.262866f},
	{0.809017f, -0.309017f, 0.500000f},
	{0.681718f, -0.147621f, 0.716567f},
	{0.850651f, 0.000000f, 0.525731f},
	{0.864188f, 0.442863f, -0.238856f},
	{0.809017f, 0.309017f, -0.500000f},
	{0.951056f, 0.162460f, -0.262866f},
	{0.525731f, 0.000000f, -0.850651f},
	{0.681718f, 0.147621f, -0.716567f},
	{0.681718f, -0.147621f, -0.716567f},
	{0.850651f, 0.000000f, -0.525731f},
	{0.809017f, -0.309017f, -0.500000f},
	{0.864188f, -0.442863f, -0.238856f},
	{0.951056f, -0.162460f, -0.262866f},
	{0.147621f, 0.716567f, -0.681718f},
	{0.309017f, 0.500000f, -0.809017f},
	{0.425325f, 0.688191f, -0.587785f},
	{0.442863f, 0.238856f, -0.864188f},
	{0.587785f, 0.425325f, -0.688191f},
	{0.688191f, 0.587785f, -0.425325f},
	{-0.147621f, 0.716567f, -0.681718f},
	{-0.309017f, 0.500000f, -0.809017f},
	{0.000000f, 0.525731f, -0.850651f},
	{-0.525731f, 0.000000f, -0.850651f},
	{-0.442863f, 0.238856f, -0.864188f},
	{-0.295242f, 0.000000f, -0.955423f},
	{-0.162460f, 0.262866f, -0.951056f},
	{0.000000f, 0.000000f, -1.000000f},
	{0.295242f, 0.000000f, -0.955423f},
	{0.162460f, 0.262866f, -0.951056f},
	{-0.442863f, -0.238856f, -0.864188f},
	{-0.309017f, -0.500000f, -0.809017f},
	{-0.162460f, -0.262866f, -0.951056f},
	{0.000000f, -0.850651f, -0.525731f},
	{-0.147621f, -0.716567f, -0.681718f},
	{0.147621f, -0.716567f, -0.681718f},
	{0.000000f, -0.525731f, -0.850651f},
	{0.309017f, -0.500000f, -0.809017f},
	{0.442863f, -0.238856f, -0.864188f},
	{0.162460f, -0.262866f, -0.951056f},
	{0.238856f, -0.864188f, -0.442863f},
	{0.500000f, -0.809017f, -0.309017f},
	{0.425325f, -0.688191f, -0.587785f},
	{0.716567f, -0.681718f, -0.147621f},
	{0.688191f, -0.587785f, -0.425325f},
	{0.587785f, -0.425325f, -0.688191f},
	{0.000000f, -0.955423f, -0.295242f},
	{0.000000f, -1.000000f, 0.000000f},
	{0.262866f, -0.951056f, -0.162460f},
	{0.000000f, -0.850651f, 0.525731f},
	{0.000000f, -0.955423f, 0.295242f},
	{0.238856f, -0.864188f, 0.442863f},
	{0.262866f, -0.951056f, 0.162460f},
	{0.500000f, -0.809017f, 0.309017f},
	{0.716567f, -0.681718f, 0.147621f},
	{0.525731f, -0.850651f, 0.000000f},
	{-0.238856f, -0.864188f, -0.442863f},
	{-0.500000f, -0.809017f, -0.309017f},
	{-0.262866f, -0.951056f, -0.162460f},
	{-0.850651f, -0.525731f, 0.000000f},
	{-0.716567f, -0.681718f, -0.147621f},
	{-0.716567f, -0.681718f, 0.147621f},
	{-0.525731f, -0.850651f, 0.000000f},
	{-0.500000f, -0.809017f, 0.309017f},
	{-0.238856f, -0.864188f, 0.442863f},
	{-0.262866f, -0.951056f, 0.162460f},
	{-0.864188f, -0.442863f, 0.238856f},
	{-0.809017f, -0.309017f, 0.500000f},
	{-0.688191f, -0.587785f, 0.425325f},
	{-0.681718f, -0.147621f, 0.716567f},
	{-0.442863f, -0.238856f, 0.864188f},
	{-0.587785f, -0.425325f, 0.688191f},
	{-0.309017f, -0.500000f, 0.809017f},
	{-0.147621f, -0.716567f, 0.681718f},
	{-0.425325f, -0.688191f, 0.587785f},
	{-0.162460f, -0.262866f, 0.951056f},
	{0.442863f, -0.238856f, 0.864188f},
	{0.162460f, -0.262866f, 0.951056f},
	{0.309017f, -0.500000f, 0.809017f},
	{0.147621f, -0.716567f, 0.681718f},
	{0.000000f, -0.525731f, 0.850651f},
	{0.425325f, -0.688191f, 0.587785f},
	{0.587785f, -0.425325f, 0.688191f},
	{0.688191f, -0.587785f, 0.425325f},
	{-0.955423f, 0.295242f, 0.000000f},
	{-0.951056f, 0.162460f, 0.262866f},
	{-1.000000f, 0.000000f, 0.000000f},
	{-0.850651f, 0.000000f, 0.525731f},
	{-0.955423f, -0.295242f, 0.000000f},
	{-0.951056f, -0.162460f, 0.262866f},
	{-0.864188f, 0.442863f, -0.238856f},
	{-0.951056f, 0.162460f, -0.262866f},
	{-0.809017f, 0.309017f, -0.500000f},
	{-0.864188f, -0.442863f, -0.238856f},
	{-0.951056f, -0.162460f, -0.262866f},
	{-0.809017f, -0.309017f, -0.500000f},
	{-0.681718f, 0.147621f, -0.716567f},
	{-0.681718f, -0.147621f, -0.716567f},
	{-0.850651f, 0.000000f, -0.525731f},
	{-0.688191f, 0.587785f, -0.425325f},
	{-0.587785f, 0.425325f, -0.688191f},
	{-0.425325f, 0.688191f, -0.587785f},
	{-0.425325f, -0.688191f, -0.587785f},
	{-0.587785f, -0.425325f, -0.688191f},
	{-0.688191f, -0.587785f, -0.425325f},
	};

struct SMD2AnimationType
{
	s32 begin;
	s32 end;
	s32 fps;
};

static const SMD2AnimationType MD2AnimationTypeList[21] =
{
    {   0,  39,  9 },   // STAND
    {  40,  45, 10 },   // RUN
    {  46,  53, 10 },   // ATTACK
    {  54,  57,  7 },   // PAIN_A
    {  58,  61,  7 },   // PAIN_B
    {  62,  65,  7 },   // PAIN_C
    {  66,  71,  7 },   // JUMP
    {  72,  83,  7 },   // FLIP
    {  84,  94,  7 },   // SALUTE
    {  95, 111, 10 },   // FALLBACK
    { 112, 122,  7 },   // WAVE
    { 123, 134,  6 },   // POINT
    { 135, 153, 10 },   // CROUCH_STAND
    { 154, 159,  7 },   // CROUCH_WALK
    { 160, 168, 10 },   // CROUCH_ATTACK
    { 169, 172,  7 },   // CROUCH_PAIN
    { 173, 177,  5 },   // CROUCH_DEATH
    { 178, 183,  7 },   // DEATH_FALLBACK
    { 184, 189,  7 },   // DEATH_FALLFORWARD
    { 190, 197,  7 },   // DEATH_FALLBACKSLOW
    { 198, 198,  5 },   // BOOM
};


//! constructor
CAnimatedMeshMD2::CAnimatedMeshMD2()
: FrameList(0), FrameCount(0), TriangleCount(0)
{
	#ifdef _DEBUG
	IAnimatedMesh::setDebugName("CAnimatedMeshMD2 IAnimatedMesh");
	IMesh::setDebugName("CAnimatedMeshMD2 IMesh");
	#endif
}



//! destructor
CAnimatedMeshMD2::~CAnimatedMeshMD2()
{
	if (FrameList)
		delete [] FrameList;
}



//! returns the amount of frames in milliseconds. If the amount is 1, it is a static (=non animated) mesh.
u32 CAnimatedMeshMD2::getFrameCount() const
{
	return FrameCount<<MD2_FRAME_SHIFT;
}



//! returns the animated mesh based on a detail level. 0 is the lowest, 255 the highest detail. Note, that some Meshes will ignore the detail level.
IMesh* CAnimatedMeshMD2::getMesh(s32 frame, s32 detailLevel, s32 startFrameLoop, s32 endFrameLoop)
{
	if ((u32)frame > getFrameCount())
		frame = (frame % getFrameCount());

	if (startFrameLoop == -1 && endFrameLoop == -1)
	{
		startFrameLoop = 0;
		endFrameLoop = getFrameCount();
	}

	updateInterpolationBuffer(frame, startFrameLoop, endFrameLoop);
	return this;
}


//! returns amount of mesh buffers.
u32 CAnimatedMeshMD2::getMeshBufferCount() const
{
	return 1;
}


//! returns pointer to a mesh buffer
IMeshBuffer* CAnimatedMeshMD2::getMeshBuffer(u32 nr) const
{
	return const_cast<IMeshBuffer*>(reinterpret_cast<const IMeshBuffer*>(&InterpolationBuffer));
}


//! Returns pointer to a mesh buffer which fits a material
IMeshBuffer* CAnimatedMeshMD2::getMeshBuffer(const video::SMaterial &material) const
{
	if (InterpolationBuffer.Material == material)
		return const_cast<IMeshBuffer*>(reinterpret_cast<const IMeshBuffer*>(&InterpolationBuffer));
	else
		return 0;
}


// updates the interpolation buffer
void CAnimatedMeshMD2::updateInterpolationBuffer(s32 frame, s32 startFrameLoop, s32 endFrameLoop)
{
	u32 firstFrame, secondFrame;
	f32 div;

	// TA: resolve missing ipol in loop between end-start

	if (endFrameLoop - startFrameLoop == 0)
	{
		firstFrame = frame>>MD2_FRAME_SHIFT;
		secondFrame = frame>>MD2_FRAME_SHIFT;
		div = 1.0f;
	}
	else
	{
		// key frames
		u32 s = startFrameLoop >> MD2_FRAME_SHIFT;
		u32 e = endFrameLoop >> MD2_FRAME_SHIFT;

		firstFrame = frame >> MD2_FRAME_SHIFT;
		secondFrame = core::if_c_a_else_b ( firstFrame + 1 > e, s, firstFrame + 1 );

		firstFrame = core::s32_min ( FrameCount - 1, firstFrame );
		secondFrame = core::s32_min ( FrameCount - 1, secondFrame );

		//div = (frame % (1<<MD2_FRAME_SHIFT)) / (f32)(1<<MD2_FRAME_SHIFT);
		frame &= (1<<MD2_FRAME_SHIFT) - 1;
		div = frame * MD2_FRAME_SHIFT_RECIPROCAL;
	}

	video::S3DVertex* target = reinterpret_cast<video::S3DVertex*>(InterpolationBuffer.getVertices());
	video::S3DVertex* first = FrameList[firstFrame].pointer();
	video::S3DVertex* second = FrameList[secondFrame].pointer();

	// interpolate both frames
	const u32 count = FrameList[firstFrame].size();
	for (u32 i=0; i<count; ++i)
	{
		target->Pos = (second->Pos - first->Pos) * div + first->Pos;
		target->Normal = (second->Normal - first->Normal) * div + first->Normal;

		++target;
		++first;
		++second;
	}

	//update bounding box
	InterpolationBuffer.setBoundingBox(BoxList[secondFrame].getInterpolated(BoxList[firstFrame], div));
}


//! loads an md2 file
bool CAnimatedMeshMD2::loadFile(io::IReadFile* file)
{
	if (!file)
		return false;

	SMD2Header header;

	file->read(&header, sizeof(SMD2Header));

#ifdef __BIG_ENDIAN__
	header.magic = os::Byteswap::byteswap(header.magic);
	header.version = os::Byteswap::byteswap(header.version);
	header.skinWidth = os::Byteswap::byteswap(header.skinWidth);
	header.skinHeight = os::Byteswap::byteswap(header.skinHeight);
	header.frameSize = os::Byteswap::byteswap(header.frameSize);
	header.numSkins = os::Byteswap::byteswap(header.numSkins);
	header.numVertices = os::Byteswap::byteswap(header.numVertices);
	header.numTexcoords = os::Byteswap::byteswap(header.numTexcoords);
	header.numTriangles = os::Byteswap::byteswap(header.numTriangles);
	header.numGlCommands = os::Byteswap::byteswap(header.numGlCommands);
	header.numFrames = os::Byteswap::byteswap(header.numFrames);
	header.offsetSkins = os::Byteswap::byteswap(header.offsetSkins);
	header.offsetTexcoords = os::Byteswap::byteswap(header.offsetTexcoords);
	header.offsetTriangles = os::Byteswap::byteswap(header.offsetTriangles);
	header.offsetFrames = os::Byteswap::byteswap(header.offsetFrames);
	header.offsetGlCommands = os::Byteswap::byteswap(header.offsetGlCommands);
	header.offsetEnd = os::Byteswap::byteswap(header.offsetEnd);
#endif

	if (header.magic != MD2_MAGIC_NUMBER || header.version != MD2_VERSION)
	{
		os::Printer::log("MD2 Loader: Wrong file header", file->getFileName(), ELL_WARNING);
		return false;
	}

	// create Memory for indices and frames

	TriangleCount = header.numTriangles;
	if (FrameList)
		delete [] FrameList;
	FrameList = new core::array<video::S3DVertex>[header.numFrames];
	FrameCount = header.numFrames;

	s32 i;

	for (i=0; i<header.numFrames; ++i)
		FrameList[i].reallocate(header.numVertices);

	// read TextureCoords

	file->seek(header.offsetTexcoords);
	SMD2TextureCoordinate* textureCoords = new SMD2TextureCoordinate[header.numTexcoords];

	if (!file->read(textureCoords, sizeof(SMD2TextureCoordinate)*header.numTexcoords))
	{
		os::Printer::log("MD2 Loader: Error reading TextureCoords.", file->getFileName(), ELL_ERROR);
		return false;
	}

#ifdef __BIG_ENDIAN__
	for (i=0; i<header.numTexcoords; ++i)
	{
		textureCoords[i].s = os::Byteswap::byteswap(textureCoords[i].s);
		textureCoords[i].t = os::Byteswap::byteswap(textureCoords[i].t);
	}
#endif

	// read Triangles

	file->seek(header.offsetTriangles);

	SMD2Triangle *triangles = new SMD2Triangle[header.numTriangles];
	if (!file->read(triangles, header.numTriangles *sizeof(SMD2Triangle)))
	{
		os::Printer::log("MD2 Loader: Error reading triangles.", file->getFileName(), ELL_ERROR);
		return false;
	}

#ifdef __BIG_ENDIAN__
	for (i=0; i<header.numTriangles; ++i)
	{
		triangles[i].vertexIndices[0] = os::Byteswap::byteswap(triangles[i].vertexIndices[0]);
		triangles[i].vertexIndices[1] = os::Byteswap::byteswap(triangles[i].vertexIndices[1]);
		triangles[i].vertexIndices[2] = os::Byteswap::byteswap(triangles[i].vertexIndices[2]);
		triangles[i].textureIndices[0] = os::Byteswap::byteswap(triangles[i].textureIndices[0]);
		triangles[i].textureIndices[1] = os::Byteswap::byteswap(triangles[i].textureIndices[1]);
		triangles[i].textureIndices[2] = os::Byteswap::byteswap(triangles[i].textureIndices[2]);
	}
#endif

	// read Vertices

	u8 buffer[MD2_MAX_VERTS*4+128];
	SMD2Frame* frame = (SMD2Frame*)buffer;

	core::array< core::vector3df >* vertices = new core::array< core::vector3df >[header.numFrames];
	core::array< core::vector3df >* normals = new core::array< core::vector3df >[header.numFrames];

	file->seek(header.offsetFrames);

	for (i = 0; i<header.numFrames; ++i)
	{
		// read vertices

		file->read(frame, header.frameSize);

#ifdef __BIG_ENDIAN__
		frame->scale[0] = os::Byteswap::byteswap(frame->scale[0]);
		frame->scale[1] = os::Byteswap::byteswap(frame->scale[1]);
		frame->scale[2] = os::Byteswap::byteswap(frame->scale[2]);
		frame->translate[0] = os::Byteswap::byteswap(frame->translate[0]);
		frame->translate[1] = os::Byteswap::byteswap(frame->translate[1]);
		frame->translate[2] = os::Byteswap::byteswap(frame->translate[2]);
#endif
		// store frame data

		SFrameData fdata;
		fdata.begin = i;
		fdata.end = i;
		fdata.fps = 7;

		if (frame->name[0])
		{
			for (s32 s = 0; frame->name[s]!=0 && (frame->name[s] < '0' ||
				frame->name[s] > '9'); ++s)
				fdata.name += frame->name[s];

			if (!FrameData.empty() && FrameData[FrameData.size()-1].name == fdata.name)
				++FrameData[FrameData.size()-1].end;
			else
				FrameData.push_back(fdata);
		}

		// add vertices

		vertices[i].reallocate(header.numVertices);
		for (s32 j=0; j<header.numVertices; ++j)
		{
			core::vector3df v;
			v.X = frame->vertices[j].vertex[0] * frame->scale[0] + frame->translate[0];
			v.Z = frame->vertices[j].vertex[1] * frame->scale[1] + frame->translate[1];
			v.Y = frame->vertices[j].vertex[2] * frame->scale[2] + frame->translate[2];

			vertices[i].push_back(v);

			u8 normalidx = frame->vertices[j].lightNormalIndex;
			if (normalidx < Q2_VERTEX_NORMAL_TABLE_SIZE)
			{
				v.X = Q2_VERTEX_NORMAL_TABLE[normalidx][0];
				v.Z = Q2_VERTEX_NORMAL_TABLE[normalidx][1];
				v.Y = Q2_VERTEX_NORMAL_TABLE[normalidx][2];
			}

			normals[i].push_back(v);
		}

		// calculate bounding boxes
		if (header.numVertices)
		{
			core::aabbox3d<f32> box;
			box.reset(vertices[i][0]);

			for (s32 j=1; j<header.numVertices; ++j)
				box.addInternalPoint(vertices[i][j]);

			BoxList.push_back(box);
		}

	}

	// put triangles into frame list

	f32 dmaxs = 1.0f/(header.skinWidth);
	f32 dmaxt = 1.0f/(header.skinHeight);

	video::S3DVertex vtx;
	vtx.Color = video::SColor(255,255,255,255);

	for (s32 f = 0; f<header.numFrames; ++f)
	{
		core::array< core::vector3df >& vert = vertices[f];

		for (s32 t=0; t<header.numTriangles; ++t)
		{
			for (s32 n=0; n<3; ++n)
			{
				vtx.Pos = vert[triangles[t].vertexIndices[n]];
				vtx.Normal = normals[f].pointer()[triangles[t].vertexIndices[n]];
				vtx.TCoords.X = (textureCoords[triangles[t].textureIndices[n]].s + 0.5f) * dmaxs;
				vtx.TCoords.Y = (textureCoords[triangles[t].textureIndices[n]].t + 0.5f) * dmaxt;
				FrameList[f].push_back(vtx);
			}
		}
	}

	// create indices

	InterpolationBuffer.Indices.reallocate(header.numVertices);
	s16 count = TriangleCount*3;
	for (s16 n=0; n<count; n+=3)
	{
		InterpolationBuffer.Indices.push_back(n);
		InterpolationBuffer.Indices.push_back(n+1);
		InterpolationBuffer.Indices.push_back(n+2);
	}

	// reallocate interpolate buffer
	if (header.numFrames)
	{
		u32 currCount = FrameList[0].size();
		InterpolationBuffer.Vertices.set_used(currCount);

		for (u32 num=0; num<currCount; ++num)
		{
			InterpolationBuffer.Vertices[num].TCoords = FrameList[0].pointer()[num].TCoords;
			InterpolationBuffer.Vertices[num].Color = vtx.Color;
		}
	}

	// clean up

	delete [] normals;
	delete [] vertices;
	delete [] triangles;
	delete [] textureCoords;

	// return

	calculateBoundingBox();

	return true;
}


//! calculates the bounding box
void CAnimatedMeshMD2::calculateBoundingBox()
{
	InterpolationBuffer.BoundingBox.reset(0,0,0);

	if (FrameCount)
	{
		u32 defaultFrame = 1;

		if (defaultFrame>=FrameCount)
			defaultFrame = 0;

		for (u32 j=0; j<FrameList[defaultFrame].size(); ++j)
			InterpolationBuffer.BoundingBox.addInternalPoint(FrameList[defaultFrame].pointer()[j].Pos);
	}
}


//! sets a flag of all contained materials to a new value
void CAnimatedMeshMD2::setMaterialFlag(video::E_MATERIAL_FLAG flag, bool newvalue)
{
	InterpolationBuffer.Material.setFlag(flag, newvalue);
}


//! returns an axis aligned bounding box
const core::aabbox3d<f32>& CAnimatedMeshMD2::getBoundingBox() const
{
	return InterpolationBuffer.BoundingBox;
}


//! set user axis aligned bounding box
void CAnimatedMeshMD2::setBoundingBox( const core::aabbox3df& box)
{
	InterpolationBuffer.BoundingBox = box;
}


//! Returns the type of the animated mesh.
E_ANIMATED_MESH_TYPE CAnimatedMeshMD2::getMeshType() const
{
	return EAMT_MD2;
}


//! Returns frame loop data for a special MD2 animation type.
void CAnimatedMeshMD2::getFrameLoop(EMD2_ANIMATION_TYPE l,
									s32& outBegin, s32& outEnd, s32& outFPS) const
{
	if (l < 0 || l >= EMAT_COUNT)
		return;

	outBegin = MD2AnimationTypeList[l].begin << MD2_FRAME_SHIFT;
	outEnd = MD2AnimationTypeList[l].end << MD2_FRAME_SHIFT;

	// correct to anim between last->first frame
	outEnd += MD2_FRAME_SHIFT == 0 ? 1 : ( 1 << MD2_FRAME_SHIFT ) - 1;
	outFPS = MD2AnimationTypeList[l].fps << MD2_FRAME_SHIFT;
}


//! Returns frame loop data for a special MD2 animation type.
bool CAnimatedMeshMD2::getFrameLoop(const c8* name,
	s32& outBegin, s32&outEnd, s32& outFPS) const
{
	for (u32 i=0; i<FrameData.size(); ++i)
	{
		if (FrameData[i].name == name)
		{
			outBegin = FrameData[i].begin << MD2_FRAME_SHIFT;
			outEnd = FrameData[i].end << MD2_FRAME_SHIFT;
			outEnd += MD2_FRAME_SHIFT == 0 ? 1 : ( 1 << MD2_FRAME_SHIFT ) - 1;
			outFPS = FrameData[i].fps << MD2_FRAME_SHIFT;
			return true;
		}
	}

	return false;
}


//! Returns amount of md2 animations in this file.
s32 CAnimatedMeshMD2::getAnimationCount() const
{
	return FrameData.size();
}


//! Returns name of md2 animation.
const c8* CAnimatedMeshMD2::getAnimationName(s32 nr) const
{
	if ((u32)nr >= FrameData.size())
		return 0;

	return FrameData[nr].name.c_str();
}


} // end namespace scene
} // end namespace irr

#endif // _IRR_COMPILE_WITH_MD2_LOADER_

