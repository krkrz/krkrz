// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h" 

#ifdef _IRR_COMPILE_WITH_STL_WRITER_

#include "CSTLMeshWriter.h"
#include "os.h"
#include "IMesh.h"
#include "IMeshBuffer.h"
#include "IAttributes.h"
#include "ISceneManager.h"
#include "IMeshCache.h"
#include "IWriteFile.h"

namespace irr
{
namespace scene
{

CSTLMeshWriter::CSTLMeshWriter(scene::ISceneManager* smgr)
	: SceneManager(smgr)
{
	if (SceneManager)
		SceneManager->grab();
}


CSTLMeshWriter::~CSTLMeshWriter()
{
	if (SceneManager)
		SceneManager->drop();
}


//! Returns the type of the mesh writer
EMESH_WRITER_TYPE CSTLMeshWriter::getType() const
{
	return EMWT_STL;
}


//! writes a mesh
bool CSTLMeshWriter::writeMesh(io::IWriteFile* file, scene::IMesh* mesh, s32 flags)
{
	if (!file)
		return false;

	os::Printer::log("Writing mesh", file->getFileName());

	if (flags & scene::EMWF_WRITE_COMPRESSED)
		return writeMeshBinary(file, mesh, flags);
	else
		return writeMeshASCII(file, mesh, flags);
}


bool CSTLMeshWriter::writeMeshBinary(io::IWriteFile* file, scene::IMesh* mesh, s32 flags)
{
	// write STL MESH header

	file->write("binary ",7);
	const core::stringc name(SceneManager->getMeshCache()->getMeshFilename(mesh));
	const s32 sizeleft = 73-name.size(); // 80 byte header
	if (sizeleft<0)
		file->write(name.c_str(),73);
	else
	{
		char* buf = new char[80];
		memset(buf, 0, 80);
		file->write(name.c_str(),name.size());
		file->write(buf,sizeleft);
		delete [] buf;
	}
	u32 facenum = 0;
	for (u32 j=0; j<mesh->getMeshBufferCount(); ++j)
		facenum += mesh->getMeshBuffer(j)->getIndexCount()/3;
	file->write(&facenum,4);

	// write mesh buffers

	for (u32 i=0; i<mesh->getMeshBufferCount(); ++i)
	{
		IMeshBuffer* buffer = mesh->getMeshBuffer(i);
		if (buffer)
		{
			const u16 indexCount = buffer->getIndexCount();

			switch(buffer->getVertexType())
			{
			case video::EVT_STANDARD:
				{
					video::S3DVertex* vtx = (video::S3DVertex*)buffer->getVertices();
					const u16 attributes = 0;
					for (u32 j=0; j<indexCount; j+=3)
					{
						const core::plane3df tmpplane(vtx[buffer->getIndices()[j]].Pos,
								vtx[buffer->getIndices()[j+1]].Pos,
								vtx[buffer->getIndices()[j+2]].Pos);
						file->write(&tmpplane.Normal, 12);
						file->write(&vtx[buffer->getIndices()[j]].Pos, 12);
						file->write(&vtx[buffer->getIndices()[j+1]].Pos, 12);
						file->write(&vtx[buffer->getIndices()[j+2]].Pos, 12);
						file->write(&attributes, 2);
					}
				}
				break;
			case video::EVT_2TCOORDS:
				{
					video::S3DVertex2TCoords* vtx = (video::S3DVertex2TCoords*)buffer->getVertices();
					const u16 attributes = 0;
					for (u32 j=0; j<indexCount; j+=3)
					{
						const core::plane3df tmpplane(vtx[buffer->getIndices()[j]].Pos,
								vtx[buffer->getIndices()[j+1]].Pos,
								vtx[buffer->getIndices()[j+2]].Pos);
						file->write(&tmpplane.Normal, 12);
						file->write(&vtx[buffer->getIndices()[j]].Pos, 12);
						file->write(&vtx[buffer->getIndices()[j+1]].Pos, 12);
						file->write(&vtx[buffer->getIndices()[j+2]].Pos, 12);
						file->write(&attributes, 2);
					}
				}
				break;
			case video::EVT_TANGENTS:
				{
					video::S3DVertexTangents* vtx = (video::S3DVertexTangents*)buffer->getVertices();
					const u16 attributes = 0;
					for (u32 j=0; j<indexCount; j+=3)
					{
						const core::plane3df tmpplane(vtx[buffer->getIndices()[j]].Pos,
								vtx[buffer->getIndices()[j+1]].Pos,
								vtx[buffer->getIndices()[j+2]].Pos);
						file->write(&tmpplane.Normal, 12);
						file->write(&vtx[buffer->getIndices()[j]].Pos, 12);
						file->write(&vtx[buffer->getIndices()[j+1]].Pos, 12);
						file->write(&vtx[buffer->getIndices()[j+2]].Pos, 12);
						file->write(&attributes, 2);
					}
				}
				break;
			}
		}
	}
	return true;
}


bool CSTLMeshWriter::writeMeshASCII(io::IWriteFile* file, scene::IMesh* mesh, s32 flags)
{
	// write STL MESH header

	file->write("solid ",6);
	const core::stringc name(SceneManager->getMeshCache()->getMeshFilename(mesh));
	file->write(name.c_str(),name.size());
	file->write("\n\n",2);

	// write mesh buffers
	
	for (u32 i=0; i<mesh->getMeshBufferCount(); ++i)
	{
		IMeshBuffer* buffer = mesh->getMeshBuffer(i);
		if (buffer)
		{
			const u16 indexCount = buffer->getIndexCount();

			switch(buffer->getVertexType())
			{
			case video::EVT_STANDARD:
				{
					video::S3DVertex* vtx = (video::S3DVertex*)buffer->getVertices();
					for (u32 j=0; j<indexCount; j+=3)
						writeFace(file,
							vtx[buffer->getIndices()[j]].Pos,
							vtx[buffer->getIndices()[j+1]].Pos,
							vtx[buffer->getIndices()[j+2]].Pos);
				}
				break;
			case video::EVT_2TCOORDS:
				{
					video::S3DVertex2TCoords* vtx = (video::S3DVertex2TCoords*)buffer->getVertices();
					for (u32 j=0; j<indexCount; j+=3)
						writeFace(file,
							vtx[buffer->getIndices()[j]].Pos,
							vtx[buffer->getIndices()[j+1]].Pos,
							vtx[buffer->getIndices()[j+2]].Pos);
				}
				break;
			case video::EVT_TANGENTS:
				{
					video::S3DVertexTangents* vtx = (video::S3DVertexTangents*)buffer->getVertices();
					for (u32 j=0; j<indexCount; j+=3)
						writeFace(file,
							vtx[buffer->getIndices()[j]].Pos,
							vtx[buffer->getIndices()[j+1]].Pos,
							vtx[buffer->getIndices()[j+2]].Pos);
				}
				break;
			}
			file->write("\n",1);
		}
	}

	file->write("endsolid ",9);
	file->write(name.c_str(),name.size());

	return true;
}


void CSTLMeshWriter::getVectorAsStringLine(const core::vector3df& v, core::stringc& s) const
{
	s = v.X;
	s += " ";
	s += v.Y;
	s += " ";
	s += v.Z;
	s += "\n";
}


void CSTLMeshWriter::writeFace(io::IWriteFile* file,
		const core::vector3df& v1,
		const core::vector3df& v2,
		const core::vector3df& v3)
{
	core::stringc tmp;
	file->write("facet normal ",13);
	getVectorAsStringLine(core::plane3df(v1,v2,v3).Normal, tmp);
	file->write(tmp.c_str(),tmp.size());
	file->write("  outer loop\n",13);
	file->write("    vertex ",11);
	getVectorAsStringLine(v1, tmp);
	file->write(tmp.c_str(),tmp.size());
	file->write("    vertex ",11);
	getVectorAsStringLine(v2, tmp);
	file->write(tmp.c_str(),tmp.size());
	file->write("    vertex ",11);
	getVectorAsStringLine(v3, tmp);
	file->write(tmp.c_str(),tmp.size());
	file->write("  endloop\n",10);
	file->write("endfacet\n",9);
}


} // end namespace
} // end namespace

#endif

