// Copyright (C) 2006-2008 Luke Hoschke
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

// B3D Mesh loader
// File format designed by Mark Sibly for the Blitz3D engine and has been
// declared public domain

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_B3D_LOADER_

#include "CB3DMeshFileLoader.h"

#include "IVideoDriver.h"
#include "os.h"


namespace irr
{
namespace scene
{

//! Constructor
CB3DMeshFileLoader::CB3DMeshFileLoader(scene::ISceneManager* smgr)
: B3dStack(), NormalsInFile(false), SceneManager(smgr), AnimatedMesh(0), file(0)
{
	#ifdef _DEBUG
	setDebugName("CB3DMeshFileLoader");
	#endif
}


//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".bsp")
bool CB3DMeshFileLoader::isALoadableFileExtension(const c8* fileName) const
{
	return strstr(fileName, ".b3d") != 0;
}


//! creates/loads an animated mesh from the file.
//! \return Pointer to the created mesh. Returns 0 if loading failed.
//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
//! See IReferenceCounted::drop() for more information.
IAnimatedMesh* CB3DMeshFileLoader::createMesh(io::IReadFile* f)
{
	if (!f)
		return 0;

	file = f;
	AnimatedMesh = new scene::CSkinnedMesh();

	Buffers = &AnimatedMesh->getMeshBuffers();
	AllJoints = &AnimatedMesh->getAllJoints();

	if ( load() )
	{
		AnimatedMesh->finalize();
	}
	else
	{
		AnimatedMesh->drop();
		AnimatedMesh = 0;
	}

	return AnimatedMesh;
}


bool CB3DMeshFileLoader::load()
{
	B3dStack.clear();

	NormalsInFile=false;

	//------ Get header ------

	SB3dChunkHeader header;
	file->read(&header, sizeof(header));

	#ifdef __BIG_ENDIAN__
		header.size = os::Byteswap::byteswap(header.size);
	#endif

	if ( strncmp( header.name, "BB3D", 4 ) != 0 )
	{
		os::Printer::log("File is not a b3d file. Loading failed (No header found)", file->getFileName(), ELL_ERROR);
		return false;
	}

	// Add main chunk...
	B3dStack.push_back(SB3dChunk());
	B3dStack.getLast().name[0] = header.name[0];
	B3dStack.getLast().name[1] = header.name[1];
	B3dStack.getLast().name[2] = header.name[2];
	B3dStack.getLast().name[3] = header.name[3];
	B3dStack.getLast().startposition = file->getPos()-8;
	B3dStack.getLast().length = header.size+8;

	// Get file version, but ignore it, as it's not important with b3d files...
	u32 FileVersion;
	file->read(&FileVersion, sizeof(FileVersion));
	#ifdef __BIG_ENDIAN__
		FileVersion = os::Byteswap::byteswap(FileVersion);
	#endif

	//------ Read main chunk ------

	while ( B3dStack.getLast().startposition+B3dStack.getLast().length > file->getPos() )
	{
		B3dStack.push_back(SB3dChunk());

		file->read(&header, sizeof(header));

		#ifdef __BIG_ENDIAN__
			header.size = os::Byteswap::byteswap(header.size);
		#endif

		B3dStack.getLast().name[0] = header.name[0];
		B3dStack.getLast().name[1] = header.name[1];
		B3dStack.getLast().name[2] = header.name[2];
		B3dStack.getLast().name[3] = header.name[3];
		B3dStack.getLast().startposition = file->getPos() - 8;
		B3dStack.getLast().length = header.size + 8;

		bool knownChunk = false;

		if ( strncmp( B3dStack.getLast().name, "TEXS", 4 ) == 0 )
		{
			knownChunk=true;
			if (!readChunkTEXS())
				return false;
		}
		else if ( strncmp( B3dStack.getLast().name, "BRUS", 4 ) == 0 )
		{
			knownChunk=true;
			if (!readChunkBRUS())
				return false;
		}
		else if ( strncmp( B3dStack.getLast().name, "NODE", 4 ) == 0 )
		{
			knownChunk=true;
			if (!readChunkNODE((CSkinnedMesh::SJoint*)0) )
				return false;
		}

		if (!knownChunk)
		{
			os::Printer::log("Unknown chunk found in mesh base - skipping");
			file->seek(B3dStack.getLast().startposition + B3dStack.getLast().length);
			B3dStack.erase(B3dStack.size()-1);
		}
	}

	B3dStack.clear();

	BaseVertices.clear();
	AnimatedVertices_VertexID.clear();
	AnimatedVertices_BufferID.clear();

	Materials.clear();
	Textures.clear();

	Buffers=0;
	AllJoints=0;

	return true;
}


bool CB3DMeshFileLoader::readChunkNODE(CSkinnedMesh::SJoint *InJoint)
{
	core::stringc JointName = readString();

	f32 position[3], scale[3], rotation[4];
	s32 n;

	for (n=0; n<=2; n++)
		file->read(&position[n], sizeof(f32));

	for (n=0; n<=2; n++)
		file->read(&scale[n], sizeof(f32));

	for (n=0; n<=3; n++)
		file->read(&rotation[n], sizeof(f32));

	#ifdef __BIG_ENDIAN__
		for (n=0; n<=2; n++)
			position[n] = os::Byteswap::byteswap(position[n]);

		for (n=0; n<=2; n++)
			scale[n] = os::Byteswap::byteswap(scale[n]);

		for (n=0; n<=3; n++)
			rotation[n] = os::Byteswap::byteswap(rotation[n]);
	#endif

	CSkinnedMesh::SJoint *Joint = AnimatedMesh->createJoint(InJoint);

	Joint->Name = JointName;
	Joint->Animatedposition = core::vector3df(position[0],position[1],position[2]) ;
	Joint->Animatedscale = core::vector3df(scale[0],scale[1],scale[2]);
	Joint->Animatedrotation = core::quaternion(rotation[1], rotation[2], rotation[3], rotation[0]);

	//Build LocalMatrix:

	core::matrix4 positionMatrix;
	positionMatrix.setTranslation( Joint->Animatedposition );
	core::matrix4 scaleMatrix;
	scaleMatrix.setScale( Joint->Animatedscale );
	core::matrix4 rotationMatrix = Joint->Animatedrotation.getMatrix();

	Joint->LocalMatrix = positionMatrix * rotationMatrix * scaleMatrix;

	if (InJoint)
		Joint->GlobalMatrix = InJoint->GlobalMatrix * Joint->LocalMatrix;
	else
		Joint->GlobalMatrix = Joint->LocalMatrix;

	while(B3dStack.getLast().startposition + B3dStack.getLast().length > file->getPos()) // this chunk repeats
	{
		B3dStack.push_back(SB3dChunk());

		SB3dChunkHeader header;
		file->read(&header, sizeof(header));

		#ifdef __BIG_ENDIAN__
			header.size = os::Byteswap::byteswap(header.size);
		#endif

		B3dStack.getLast().name[0] = header.name[0];
		B3dStack.getLast().name[1] = header.name[1];
		B3dStack.getLast().name[2] = header.name[2];
		B3dStack.getLast().name[3] = header.name[3];
		B3dStack.getLast().length = header.size+8;
		B3dStack.getLast().startposition = file->getPos() - 8;

		bool knownChunk = false;

		if ( strncmp( B3dStack.getLast().name, "NODE", 4 ) == 0 )
		{
			knownChunk = true;
			if (!readChunkNODE(Joint))
				return false;
		}
		else if ( strncmp( B3dStack.getLast().name, "MESH", 4 ) == 0 )
		{
			knownChunk = true;
			if (!readChunkMESH(Joint))
				return false;
		}
		else if ( strncmp( B3dStack.getLast().name, "ANIM", 4 ) == 0 )
		{
			knownChunk = true;
			if (!readChunkANIM(Joint))
				return false;
		}
		else if ( strncmp( B3dStack.getLast().name, "BONE", 4 ) == 0 )
		{
			knownChunk = true;
			if (!readChunkBONE(Joint))
				return false;
		}
		else if ( strncmp( B3dStack.getLast().name, "KEYS", 4 ) == 0 )
		{
			knownChunk = true;
			if(!readChunkKEYS(Joint))
				return false;
		}

		if (!knownChunk)
		{
			os::Printer::log("Unknown chunk found in node chunk - skipping");
			file->seek(B3dStack.getLast().startposition + B3dStack.getLast().length);
			B3dStack.erase(B3dStack.size()-1);
		}
	}

	B3dStack.erase(B3dStack.size()-1);

	return true;
}

bool CB3DMeshFileLoader::readChunkMESH(CSkinnedMesh::SJoint *InJoint)
{
	const s32 vertices_Start=BaseVertices.size(); //B3Ds have Vertex ID's local within the mesh I don't want this

	s32 brush_id;

	file->read(&brush_id, sizeof(brush_id));

	#ifdef __BIG_ENDIAN__
		brush_id = os::Byteswap::byteswap(brush_id);
	#endif

	NormalsInFile=false;

	while(B3dStack.getLast().startposition + B3dStack.getLast().length>file->getPos()) //this chunk repeats
	{
		B3dStack.push_back(SB3dChunk());

		SB3dChunkHeader header;
		file->read(&header, sizeof(header));

		#ifdef __BIG_ENDIAN__
			header.size = os::Byteswap::byteswap(header.size);
		#endif

		B3dStack.getLast().name[0]=header.name[0];
		B3dStack.getLast().name[1]=header.name[1]; //Not sure of an easier way
		B3dStack.getLast().name[2]=header.name[2];
		B3dStack.getLast().name[3]=header.name[3];
		B3dStack.getLast().length = header.size + 8;
		B3dStack.getLast().startposition = file->getPos() - 8;

		bool knownChunk=false;

		if ( strncmp( B3dStack.getLast().name, "VRTS", 4 ) == 0 )
		{
			knownChunk=true;
			if (!readChunkVRTS(InJoint, 0, vertices_Start))
				return false;
		}
		else if ( strncmp( B3dStack.getLast().name, "TRIS", 4 ) == 0 )
		{
			knownChunk=true;

			scene::SSkinMeshBuffer *MeshBuffer = AnimatedMesh->createBuffer();

			if (brush_id!=-1)
				MeshBuffer->Material=Materials[brush_id].Material;

			if(readChunkTRIS(InJoint, MeshBuffer,AnimatedMesh->getMeshBuffers().size()-1, vertices_Start)==false)
				return false;

			if (!NormalsInFile && MeshBuffer->Material.Lighting) // No point wasting time on lightmapped levels
			{
				s32 i;

				for ( i=0; i<(s32)MeshBuffer->Indices.size(); i+=3)
				{
					core::plane3d<f32> p(	MeshBuffer->getVertex(MeshBuffer->Indices[i+0])->Pos,
											MeshBuffer->getVertex(MeshBuffer->Indices[i+1])->Pos,
											MeshBuffer->getVertex(MeshBuffer->Indices[i+2])->Pos);

					MeshBuffer->getVertex(MeshBuffer->Indices[i+0])->Normal += p.Normal;
					MeshBuffer->getVertex(MeshBuffer->Indices[i+1])->Normal += p.Normal;
					MeshBuffer->getVertex(MeshBuffer->Indices[i+2])->Normal += p.Normal;
				}

				for ( i = 0; i<(s32)MeshBuffer->getVertexCount(); ++i )
				{
					MeshBuffer->getVertex(i)->Normal.normalize ();
					BaseVertices[vertices_Start+i].Normal=MeshBuffer->getVertex(i)->Normal;
				}
			}
		}

		if (!knownChunk)
		{
			os::Printer::log("Unknown chunk found in mesh - skipping");
			file->seek(B3dStack.getLast().startposition + B3dStack.getLast().length);
			B3dStack.erase(B3dStack.size()-1);
		}
	}

	B3dStack.erase(B3dStack.size()-1);

	return true;
}


/*
VRTS:
  int flags                   ;1=normal values present, 2=rgba values present
  int tex_coord_sets          ;texture coords per vertex (eg: 1 for simple U/V) max=8
  int tex_coord_set_size      ;components per set (eg: 2 for simple U/V) max=4
  {
  float x,y,z                 ;always present
  float nx,ny,nz              ;vertex normal: present if (flags&1)
  float red,green,blue,alpha  ;vertex color: present if (flags&2)
  float tex_coords[tex_coord_sets][tex_coord_set_size]	;tex coords
  }
*/
bool CB3DMeshFileLoader::readChunkVRTS(CSkinnedMesh::SJoint *InJoint, scene::SSkinMeshBuffer* MeshBuffer, s32 vertices_Start)
{
	s32 flags, tex_coord_sets, tex_coord_set_size;

	file->read(&flags, sizeof(flags));
	file->read(&tex_coord_sets, sizeof(tex_coord_sets));
	file->read(&tex_coord_set_size, sizeof(tex_coord_set_size));

	#ifdef __BIG_ENDIAN__
		flags = os::Byteswap::byteswap(flags);
		tex_coord_sets = os::Byteswap::byteswap(tex_coord_sets);
		tex_coord_set_size = os::Byteswap::byteswap(tex_coord_set_size);
	#endif

	if (tex_coord_sets >= 3 || tex_coord_set_size >= 4) // Something is wrong
	{
		os::Printer::log("tex_coord_sets or tex_coord_set_size too big", file->getFileName(), ELL_ERROR);
		return false;
	}

	//------ Allocate Memory, for speed -----------//

	s32 MemoryNeeded = B3dStack.getLast().length / sizeof(f32);
	s32 NumberOfReads = 3;

	if (flags & 1)
		NumberOfReads += 3;
	if (flags & 2)
		NumberOfReads += 4;

	NumberOfReads += tex_coord_sets*tex_coord_set_size;

	MemoryNeeded /= NumberOfReads;

	BaseVertices.reallocate(MemoryNeeded + BaseVertices.size() + 1);
	AnimatedVertices_VertexID.reallocate(MemoryNeeded + AnimatedVertices_VertexID.size() + 1);

	//--------------------------------------------//

	while(B3dStack.getLast().startposition + B3dStack.getLast().length > file->getPos()) // this chunk repeats
	{
		f32 x=0.0f, y=0.0f, z=0.0f;
		f32 nx=0.0f, ny=0.0f, nz=0.0f;
		f32 red=1.0f, green=1.0f, blue=1.0f, alpha=1.0f;
		f32 tex_coords[3][4];

		file->read(&x, sizeof(x));
		file->read(&y, sizeof(y));
		file->read(&z, sizeof(z));

		if (flags & 1)
		{
			NormalsInFile = true;
			file->read(&nx, sizeof(nx));
			file->read(&ny, sizeof(ny));
			file->read(&nz, sizeof(nz));
		}

		if (flags & 2)
		{
			file->read(&red, sizeof(red));
			file->read(&green, sizeof(green));
			file->read(&blue, sizeof(blue));
			file->read(&alpha, sizeof(alpha));
		}

		for (s32 i=0; i<tex_coord_sets; ++i)
			for (s32 j=0; j<tex_coord_set_size; ++j)
				file->read(&tex_coords[i][j], sizeof(f32));

		#ifdef __BIG_ENDIAN__
			x = os::Byteswap::byteswap(x);
			y = os::Byteswap::byteswap(y);
			z = os::Byteswap::byteswap(z);

			if (flags&1)
			{
				nx = os::Byteswap::byteswap(nx);
				ny = os::Byteswap::byteswap(ny);
				nz = os::Byteswap::byteswap(nz);
			}

			if (flags & 2)
			{
				red = os::Byteswap::byteswap(red);
				green = os::Byteswap::byteswap(green);
				blue = os::Byteswap::byteswap(blue);
				alpha = os::Byteswap::byteswap(alpha);
			}

			for (s32 i=0; i<=tex_coord_sets-1; i++)
				for (s32 j=0; j<=tex_coord_set_size-1; j++)
					tex_coords[i][j] = os::Byteswap::byteswap(tex_coords[i][j]);
		#endif

		f32 tu=0.0f, tv=0.0f;
		if (tex_coord_sets >= 1 && tex_coord_set_size >= 2)
		{
			tu=tex_coords[0][0];
			tv=tex_coords[0][1];
		}

		f32 tu2=0.0f, tv2=0.0f;
		if (tex_coord_sets>=2 && tex_coord_set_size>=2)
		{
			tu2=tex_coords[1][0];
			tv2=tex_coords[1][1];
		}

		// Create Vertex...
		video::S3DVertex2TCoords Vertex(x, y, z, nx, ny, nz, video::SColorf(red, green, blue, alpha).toSColor(), tu, tv, tu2, tv2);

		// Transform the Vertex position by nested node...
		InJoint->GlobalMatrix.transformVect(Vertex.Pos);


		InJoint->GlobalMatrix.rotateVect(Vertex.Normal);

		//Add it...

		BaseVertices.push_back(Vertex);

		AnimatedVertices_VertexID.push_back(-1);
		AnimatedVertices_BufferID.push_back(-1);
	}

	B3dStack.erase(B3dStack.size()-1);

	return true;
}


bool CB3DMeshFileLoader::readChunkTRIS(CSkinnedMesh::SJoint *InJoint, scene::SSkinMeshBuffer *MeshBuffer, u32 MeshBufferID, s32 vertices_Start)
{

	bool showVertexWarning=false;

	s32 triangle_brush_id; // Note: Irrlicht can't have different brushes for each triangle (I'm using a workaround)

	file->read(&triangle_brush_id, sizeof(triangle_brush_id));

	#ifdef __BIG_ENDIAN__
		triangle_brush_id = os::Byteswap::byteswap(triangle_brush_id);
	#endif

	SB3dMaterial *B3dMaterial;

	if (triangle_brush_id != -1)
		B3dMaterial = &Materials[triangle_brush_id];
	else
		B3dMaterial = 0;

	if (B3dMaterial)
		MeshBuffer->Material = B3dMaterial->Material;

	s32 MemoryNeeded = B3dStack.getLast().length / sizeof(s32);
	MeshBuffer->Indices.reallocate(MemoryNeeded + MeshBuffer->Indices.size() + 1);

	while(B3dStack.getLast().startposition + B3dStack.getLast().length > file->getPos()) // this chunk repeats
	{
		s32 vertex_id[3];

		file->read(&vertex_id[0], sizeof(s32));
		file->read(&vertex_id[1], sizeof(s32));
		file->read(&vertex_id[2], sizeof(s32));

		#ifdef __BIG_ENDIAN__
			vertex_id[0] = os::Byteswap::byteswap(vertex_id[0]);
			vertex_id[1] = os::Byteswap::byteswap(vertex_id[1]);
			vertex_id[2] = os::Byteswap::byteswap(vertex_id[2]);
		#endif

		//Make Ids global:
		vertex_id[0] += vertices_Start;
		vertex_id[1] += vertices_Start;
		vertex_id[2] += vertices_Start;

		for(s32 i=0; i<3; ++i)
		{
			if ((u32)vertex_id[i] >= AnimatedVertices_VertexID.size())
				return false;

			if (AnimatedVertices_VertexID[ vertex_id[i] ] != -1)
			{
				if ( AnimatedVertices_BufferID[ vertex_id[i] ] != (s32)MeshBufferID ) //If this vertex is linked in a different meshbuffer
				{
					AnimatedVertices_VertexID[ vertex_id[i] ] = -1;
					AnimatedVertices_BufferID[ vertex_id[i] ] = -1;
					showVertexWarning=true;
				}
			}
			if (AnimatedVertices_VertexID[ vertex_id[i] ] == -1) //If this vertex is not in the meshbuffer
			{

				//Check for lightmapping:
				if (BaseVertices[ vertex_id[i] ].TCoords2 != core::vector2d<f32>(0,0))
					MeshBuffer->MoveTo_2TCoords(); //Will only affect the meshbuffer the first time this is called

				//Add the vertex to the meshbuffer:
				if (MeshBuffer->VertexType == video::EVT_STANDARD)
					MeshBuffer->Vertices_Standard.push_back( BaseVertices[ vertex_id[i] ] );
				else
					MeshBuffer->Vertices_2TCoords.push_back(BaseVertices[ vertex_id[i] ] );

				//create vertex id to meshbuffer index link:
				AnimatedVertices_VertexID[ vertex_id[i] ] = MeshBuffer->getVertexCount()-1;
				AnimatedVertices_BufferID[ vertex_id[i] ] = MeshBufferID;

				if (B3dMaterial)
				{
					// Apply Material/Colour/etc...
					video::S3DVertex *Vertex=MeshBuffer->getVertex(MeshBuffer->getVertexCount()-1);

					if (Vertex->Color.getAlpha() == 255) //Note: Irrlicht docs state that 0 is opaque, are they wrong?
						Vertex->Color.setAlpha( (s32)(B3dMaterial->alpha * 255.0f) );


					// Use texture's scale
					if (B3dMaterial->Textures[0])
					{
						Vertex->TCoords.X *=B3dMaterial->Textures[0]->Xscale;
						Vertex->TCoords.Y *=B3dMaterial->Textures[0]->Yscale;
					}
					/*
					if (B3dMaterial->Textures[1])
					{
						Vertex->TCoords2.X *=B3dMaterial->Textures[1]->Xscale;
						Vertex->TCoords2.Y *=B3dMaterial->Textures[1]->Yscale;
					}
					*/

				}
			}
		}

		MeshBuffer->Indices.push_back( AnimatedVertices_VertexID[ vertex_id[0] ] );
		MeshBuffer->Indices.push_back( AnimatedVertices_VertexID[ vertex_id[1] ] );
		MeshBuffer->Indices.push_back( AnimatedVertices_VertexID[ vertex_id[2] ] );
	}

	B3dStack.erase(B3dStack.size()-1);



	if (showVertexWarning)
		os::Printer::log("B3dMeshLoader: Warning, different meshbuffers linking to the same vertex, this will cause problems with animated meshes");

	return true;
}


bool CB3DMeshFileLoader::readChunkBONE(CSkinnedMesh::SJoint *InJoint)
{
	if (B3dStack.getLast().length > 8)
	{
		while(B3dStack.getLast().startposition + B3dStack.getLast().length>file->getPos()) // this chunk repeats
		{
			CSkinnedMesh::SWeight *Weight=AnimatedMesh->createWeight(InJoint);

			u32 GlobalVertexID;

			file->read(&GlobalVertexID, sizeof(GlobalVertexID));
			file->read(&Weight->strength, sizeof(Weight->strength));

			#ifdef __BIG_ENDIAN__
				GlobalVertexID = os::Byteswap::byteswap(GlobalVertexID);
				Weight->strength = os::Byteswap::byteswap(Weight->strength);
			#endif

			if (AnimatedVertices_VertexID[GlobalVertexID]==-1)
			{
				os::Printer::log("B3dMeshLoader: Weight has bad vertex id (no link to meshbuffer index found)");
				Weight->vertex_id = Weight->buffer_id = 0;
			}
			else
			{
				//Find the MeshBuffer and Vertex index from the Global Vertex ID:
				Weight->vertex_id = AnimatedVertices_VertexID[GlobalVertexID];
				Weight->buffer_id = AnimatedVertices_BufferID[GlobalVertexID];
			}
		}
	}

	B3dStack.erase(B3dStack.size()-1);
	return true;
}


bool CB3DMeshFileLoader::readChunkKEYS(CSkinnedMesh::SJoint *InJoint)
{
	s32 flags;
	file->read(&flags, sizeof(flags));
	#ifdef __BIG_ENDIAN__
		flags = os::Byteswap::byteswap(flags);
	#endif

	while(B3dStack.getLast().startposition + B3dStack.getLast().length>file->getPos()) //this chunk repeats
	{
		s32 frame;

		f32 positionData[3];
		f32 scaleData[3];
		f32 rotationData[4];

		file->read(&frame, sizeof(frame));

		if (flags&1)
			readFloats(positionData, 3);

		if (flags&2)
			readFloats(scaleData, 3);

		if (flags&4)
			readFloats(rotationData, 4);

		#ifdef __BIG_ENDIAN__
		frame = os::Byteswap::byteswap(frame);
		#endif

		core::vector3df position = core::vector3df(positionData[0], positionData[1], positionData[2]);
		core::vector3df scale = core::vector3df(scaleData[0], scaleData[1], scaleData[2]);
		core::quaternion rotation = core::quaternion(rotationData[1], rotationData[2], rotationData[3], rotationData[0]); // meant to be in this order

		// Add key frame

		if (flags & 1)
		{
			CSkinnedMesh::SPositionKey *Key=AnimatedMesh->createPositionKey(InJoint);
			Key->frame = (f32)frame;
			Key->position = position;
		}
		if (flags & 2)
		{
			CSkinnedMesh::SScaleKey *Key=AnimatedMesh->createScaleKey(InJoint);
			Key->frame = (f32)frame;
			Key->scale=scale;
		}
		if (flags & 4)
		{
			CSkinnedMesh::SRotationKey *Key=AnimatedMesh->createRotationKey(InJoint);
			Key->frame = (f32)frame;
			Key->rotation = rotation;
		}
	}

	B3dStack.erase(B3dStack.size()-1);
	return true;
}


bool CB3DMeshFileLoader::readChunkANIM(CSkinnedMesh::SJoint *InJoint)
{
	s32 AnimFlags; //not stored\used
	s32 AnimFrames;//not stored\used
	f32 AnimFPS; //not stored\used

	file->read(&AnimFlags, sizeof(s32));
	file->read(&AnimFrames, sizeof(s32));
	readFloats(&AnimFPS, 1);

	#ifdef __BIG_ENDIAN__
		AnimFlags = os::Byteswap::byteswap(AnimFlags);
		AnimFrames = os::Byteswap::byteswap(AnimFrames);
	#endif

	B3dStack.erase(B3dStack.size()-1);
	return true;
}

bool CB3DMeshFileLoader::readChunkTEXS()
{
	bool Previous32BitTextureFlag = SceneManager->getVideoDriver()->getTextureCreationFlag(video::ETCF_ALWAYS_32_BIT);
	SceneManager->getVideoDriver()->setTextureCreationFlag(video::ETCF_ALWAYS_32_BIT, true);

	while(B3dStack.getLast().startposition + B3dStack.getLast().length>file->getPos()) //this chunk repeats
	{
		core::stringc TextureName=readString();

		TextureName=stripPathFromString(file->getFileName(),true) + stripPathFromString(TextureName,false);

		SB3dTexture B3dTexture;
		B3dTexture.Texture=SceneManager->getVideoDriver()->getTexture ( TextureName.c_str() );

		file->read(&B3dTexture.Flags, sizeof(s32));
		file->read(&B3dTexture.Blend, sizeof(s32));
		readFloats(&B3dTexture.Xpos, 1);
		readFloats(&B3dTexture.Ypos, 1);
		readFloats(&B3dTexture.Xscale, 1);
		readFloats(&B3dTexture.Yscale, 1);
		readFloats(&B3dTexture.Angle, 1);

		#ifdef __BIG_ENDIAN__
			B3dTexture.Flags = os::Byteswap::byteswap(B3dTexture.Flags);
			B3dTexture.Blend = os::Byteswap::byteswap(B3dTexture.Blend);
		#endif

		Textures.push_back(B3dTexture);
	}

	B3dStack.erase(B3dStack.size()-1);

	SceneManager->getVideoDriver()->setTextureCreationFlag(video::ETCF_ALWAYS_32_BIT, Previous32BitTextureFlag);

	return true;
}


bool CB3DMeshFileLoader::readChunkBRUS()
{
	s32 n_texs;

	file->read(&n_texs, sizeof(s32));

	#ifdef __BIG_ENDIAN__
		n_texs = os::Byteswap::byteswap(n_texs);
	#endif

	while(B3dStack.getLast().startposition + B3dStack.getLast().length>file->getPos()) //this chunk repeats
	{
		// This is what blitz basic calls a brush, like a Irrlicht Material

		core::stringc MaterialName=readString(); //Not used but we still need the read it

		Materials.push_back(SB3dMaterial());
		SB3dMaterial& B3dMaterial=Materials.getLast();

		B3dMaterial.Textures[0]=0;
		B3dMaterial.Textures[1]=0;

		s32 texture_id[8];
		texture_id[0]=-1;
		texture_id[1]=-1;

		file->read(&B3dMaterial.red, sizeof(B3dMaterial.red));
		file->read(&B3dMaterial.green, sizeof(B3dMaterial.green));
		file->read(&B3dMaterial.blue, sizeof(B3dMaterial.blue));
		file->read(&B3dMaterial.alpha, sizeof(B3dMaterial.alpha));
		file->read(&B3dMaterial.shininess, sizeof(B3dMaterial.shininess));
		file->read(&B3dMaterial.blend, sizeof(B3dMaterial.blend));
		file->read(&B3dMaterial.fx, sizeof(B3dMaterial.fx));

		for (s32 n=0; n < n_texs; ++n)
			file->read(&texture_id[n], sizeof(s32)); //I'm not sure of getting the sizeof an array

		#ifdef __BIG_ENDIAN__
			B3dMaterial.red = os::Byteswap::byteswap(B3dMaterial.red);
			B3dMaterial.green = os::Byteswap::byteswap(B3dMaterial.green);
			B3dMaterial.blue = os::Byteswap::byteswap(B3dMaterial.blue);
			B3dMaterial.alpha = os::Byteswap::byteswap(B3dMaterial.alpha);
			B3dMaterial.shininess = os::Byteswap::byteswap(B3dMaterial.shininess);
			B3dMaterial.blend = os::Byteswap::byteswap(B3dMaterial.blend);
			B3dMaterial.fx = os::Byteswap::byteswap(B3dMaterial.fx);

			for (s32 n=0; n < n_texs; ++n)
				texture_id[n] = os::Byteswap::byteswap(texture_id[n]);
		#endif

		//------ Get pointers to the texture, based on the IDs ------
		if (texture_id[0] != -1)
			B3dMaterial.Textures[0]=&Textures[texture_id[0]];
		if (texture_id[1] != -1)
			B3dMaterial.Textures[1]=&Textures[texture_id[1]];


		//Fixes problems when the lightmap is on the first texture:
		if (texture_id[0] != -1)
		{
			if (Textures[texture_id[0]].Flags & 65536) // 65536 = secondary UV
			{
				SB3dTexture *TmpTexture;
				TmpTexture = B3dMaterial.Textures[1];
				B3dMaterial.Textures[1] = B3dMaterial.Textures[0];
				B3dMaterial.Textures[0] = TmpTexture;
			}
		}

		if (B3dMaterial.Textures[0] != 0)
			B3dMaterial.Material.setTexture(0, B3dMaterial.Textures[0]->Texture);
		if (B3dMaterial.Textures[1] != 0)
			B3dMaterial.Material.setTexture(1, B3dMaterial.Textures[1]->Texture);

		//If the first texture is empty:
		if (B3dMaterial.Textures[1] != 0 && B3dMaterial.Textures[0] == 0)
		{
			B3dMaterial.Textures[0] = B3dMaterial.Textures[1];
			B3dMaterial.Textures[1] = 0;
		}

		//------ Convert blitz flags/blend to irrlicht -------

		//Two textures:
		if (B3dMaterial.Textures[1])
		{
			if (B3dMaterial.alpha==1)
			{
				if (B3dMaterial.Textures[1]->Blend & 5) //(Multiply 2)
					B3dMaterial.Material.MaterialType = video::EMT_LIGHTMAP_M2;
				else
					B3dMaterial.Material.MaterialType = video::EMT_LIGHTMAP;
			}
			else
				B3dMaterial.Material.MaterialType = video::EMT_TRANSPARENT_VERTEX_ALPHA;
		}
		else if (B3dMaterial.Textures[0]) //One texture:
		{
			if (B3dMaterial.Textures[0]->Flags & 2) //(Alpha mapped)
				B3dMaterial.Material.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
			else if (B3dMaterial.Textures[0]->Flags & 4) //(Masked)
				B3dMaterial.Material.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF; // Todo: create color key texture
			else if (B3dMaterial.alpha == 1)
				B3dMaterial.Material.MaterialType = video::EMT_SOLID;
			else
				B3dMaterial.Material.MaterialType = video::EMT_TRANSPARENT_VERTEX_ALPHA;
		}
		else //No texture:
		{
			if (B3dMaterial.alpha == 1)
				B3dMaterial.Material.MaterialType = video::EMT_SOLID;
			else
				B3dMaterial.Material.MaterialType = video::EMT_TRANSPARENT_VERTEX_ALPHA;
		}

		B3dMaterial.Material.AmbientColor = video::SColorf(B3dMaterial.red, B3dMaterial.green, B3dMaterial.blue, B3dMaterial.alpha).toSColor ();

		//------ Material fx ------

		if (B3dMaterial.fx & 1) //full-bright
		{
			B3dMaterial.Material.AmbientColor = video::SColor(255, 255, 255, 255);
			B3dMaterial.Material.Lighting = false;
		}

		//if (B3dMaterial.fx & 2) //use vertex colors instead of brush color

		if (B3dMaterial.fx & 4) //flatshaded
			B3dMaterial.Material.GouraudShading = false;

		if (B3dMaterial.fx & 16) //disable backface culling
			B3dMaterial.Material.BackfaceCulling = false;

		if (B3dMaterial.fx & 32) //force vertex alpha-blending
			B3dMaterial.Material.MaterialType = video::EMT_TRANSPARENT_VERTEX_ALPHA;

		B3dMaterial.Material.DiffuseColor = video::SColorf(B3dMaterial.red, B3dMaterial.green, B3dMaterial.blue, B3dMaterial.alpha).toSColor ();
		B3dMaterial.Material.EmissiveColor = video::SColorf(0.5, 0.5, 0.5, 0).toSColor ();
		B3dMaterial.Material.SpecularColor = video::SColorf(0, 0, 0, 0).toSColor ();
		B3dMaterial.Material.Shininess = B3dMaterial.shininess;
	}

	B3dStack.erase(B3dStack.size()-1);

	return true;
}

core::stringc CB3DMeshFileLoader::readString()
{
	core::stringc newstring;
	while (file->getPos() <= file->getSize())
	{
		c8 character;
		file->read(&character, sizeof(character));
		if (character==0)
			return newstring;
		newstring.append(character);
	}
	return newstring;
}

core::stringc CB3DMeshFileLoader::stripPathFromString(core::stringc string, bool returnPath)
{
	s32 slashIndex=string.findLast('/'); // forward slash
	s32 backSlash=string.findLast('\\'); // back slash

	if (backSlash>slashIndex) slashIndex=backSlash;

	if (slashIndex==-1)//no slashes found
	{
		if (returnPath)
			return core::stringc(); //no path to return
		else
			return string;
	}

	if (returnPath)
		return string.subString(0, slashIndex + 1);
	else
		return string.subString(slashIndex+1, string.size() - (slashIndex+1));
}

void CB3DMeshFileLoader::readFloats(f32* vec, u32 count)
{
	file->read(vec, count*sizeof(f32));
	#ifdef __BIG_ENDIAN__
	for (u32 n=0; n<count; ++n)
		vec[n] = os::Byteswap::byteswap(vec[n]);
	#endif
}

} // end namespace scene
} // end namespace irr


#endif // _IRR_COMPILE_WITH_B3D_LOADER_

