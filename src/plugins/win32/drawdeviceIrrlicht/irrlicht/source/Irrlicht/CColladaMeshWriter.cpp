// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_COLLADA_WRITER_

#include "CColladaMeshWriter.h"
#include "os.h"
#include "IFileSystem.h"
#include "IWriteFile.h"
#include "IXMLWriter.h"
#include "IMesh.h"
#include "IAttributes.h"

namespace irr
{
namespace scene
{


CColladaMeshWriter::CColladaMeshWriter(video::IVideoDriver* driver,
					io::IFileSystem* fs)
	: FileSystem(fs), VideoDriver(driver), Writer(0)
{
	if (VideoDriver)
		VideoDriver->grab();

	if (FileSystem)
		FileSystem->grab();
}


CColladaMeshWriter::~CColladaMeshWriter()
{
	if (VideoDriver)
		VideoDriver->drop();

	if (FileSystem)
		FileSystem->drop();
}


//! Returns the type of the mesh writer
EMESH_WRITER_TYPE CColladaMeshWriter::getType() const
{
	return EMWT_COLLADA;
}


//! writes a mesh
bool CColladaMeshWriter::writeMesh(io::IWriteFile* file, scene::IMesh* mesh, s32 flags)
{
	if (!file)
		return false;

	Writer = FileSystem->createXMLWriter(file);

	if (!Writer)
	{
		os::Printer::log("Could not write file", file->getFileName());
		return false;
	}

	os::Printer::log("Writing mesh", file->getFileName());

	// write COLLADA header

	Writer->writeXMLHeader();

	Writer->writeElement(L"COLLADA", false,
		L"xmlns", L"http://www.collada.org/2005/COLLADASchema",
		L"version", L"1.2.0");
	Writer->writeLineBreak();

	// write asset data

	Writer->writeElement(L"asset", false);
	Writer->writeLineBreak();

	Writer->writeElement(L"revision", false);
	Writer->writeText(L"1.0");
	Writer->writeClosingTag(L"revision");
	Writer->writeLineBreak();

	Writer->writeElement(L"authoring_tool", false);
	core::stringw strautoring = L"Irrlicht Engine / irrEdit";  // this code has originated from irrEdit 0.7
	Writer->writeText(strautoring.c_str());
	Writer->writeClosingTag(L"authoring_tool");
	Writer->writeLineBreak();

	Writer->writeClosingTag(L"asset");
	Writer->writeLineBreak();

	// write all materials

	Writer->writeElement(L"library", false,	L"type", L"MATERIAL");
	Writer->writeLineBreak();

	u32 i;
	for (i=0; i<mesh->getMeshBufferCount(); ++i)
	{
		core::stringw strMat = "mat";
		strMat += i;

		Writer->writeElement(L"material", false,
			L"name", strMat.c_str(),
			L"id", strMat.c_str());
		Writer->writeLineBreak();

		// write all interesting material parameters as parameter

		io::IAttributes* attributes = VideoDriver->createAttributesFromMaterial(
			mesh->getMeshBuffer(i)->getMaterial());

		u32 count = attributes->getAttributeCount();
		for (u32 attridx=0; attridx<count; ++attridx)
		{
			core::stringw str = attributes->getAttributeName(attridx);

			Writer->writeElement(L"param", false,
				L"name", str.c_str(),
				L"type", attributes->getAttributeTypeString(attridx) );

			str = attributes->getAttributeAsString(attridx).c_str();
			Writer->writeText(str.c_str());

			Writer->writeClosingTag(L"param");
			Writer->writeLineBreak();
		}

		attributes->drop();

		Writer->writeClosingTag(L"material");
		Writer->writeLineBreak();
	}

	Writer->writeClosingTag(L"library");
	Writer->writeLineBreak();

	// write mesh

	Writer->writeElement(L"library", false,	L"type", L"GEOMETRY");
	Writer->writeLineBreak();

	Writer->writeElement(L"geometry", false, L"id", L"mesh", L"name", L"mesh");
	Writer->writeLineBreak();
	Writer->writeElement(L"mesh");
	Writer->writeLineBreak();

	// do some statistics for the mesh to know which stuff needs to be saved into
	// the file:
	// - count vertices
	// - check for the need of a second texture coordinate
	// - count amount of second texture coordinates
	// - check for the need of tangents (TODO)

	u32 totalVertexCount = 0;
	u32 totalTCoords2Count = 0;
	bool needsTangents = false; // TODO: tangents not supported here yet

	for (i=0; i<mesh->getMeshBufferCount(); ++i)
	{
		totalVertexCount += mesh->getMeshBuffer(i)->getVertexCount();

		if (hasSecondTextureCoordinates(mesh->getMeshBuffer(i)->getVertexType()))
			totalTCoords2Count += mesh->getMeshBuffer(i)->getVertexCount();

		if (!needsTangents)
			needsTangents = mesh->getMeshBuffer(i)->getVertexType() == video::EVT_TANGENTS;
	}

	SComponentGlobalStartPos* globalIndices = new SComponentGlobalStartPos[mesh->getMeshBufferCount()];

	// write positions

	Writer->writeElement(L"source", false, L"id", L"mesh-Pos");
	Writer->writeLineBreak();

	core::stringw vertexCountStr = (totalVertexCount*3);
	Writer->writeElement(L"array", false, L"id", L"mesh-Pos-array",
							L"type", L"float", L"count", vertexCountStr.c_str());
	Writer->writeLineBreak();

	for (i=0; i<mesh->getMeshBufferCount(); ++i)
	{
		scene::IMeshBuffer* buffer = mesh->getMeshBuffer(i);
		video::E_VERTEX_TYPE vtxType = buffer->getVertexType();
		u32 vertexCount = buffer->getVertexCount();

		globalIndices[i].PosStartIndex = 0;

		if (i!=0)
			globalIndices[i].PosStartIndex = globalIndices[i-1].PosLastIndex + 1;

		globalIndices[i].PosLastIndex = globalIndices[i].PosStartIndex + vertexCount - 1;

		switch(vtxType)
		{
		case video::EVT_STANDARD:
			{
				video::S3DVertex* vtx = (video::S3DVertex*)buffer->getVertices();
				for (u32 j=0; j<vertexCount; ++j)
				{
					core::stringw str;
					str += vtx[j].Pos.X;
					str += " ";
					str += vtx[j].Pos.Y;
					str += " ";
					str += vtx[j].Pos.Z;

					Writer->writeText(str.c_str());
					Writer->writeLineBreak();
				}
			}
			break;
		case video::EVT_2TCOORDS:
			{
				video::S3DVertex2TCoords* vtx = (video::S3DVertex2TCoords*)buffer->getVertices();
				for (u32 j=0; j<vertexCount; ++j)
				{
					core::stringw str;
					str += vtx[j].Pos.X;
					str += " ";
					str += vtx[j].Pos.Y;
					str += " ";
					str += vtx[j].Pos.Z;

					Writer->writeText(str.c_str());
					Writer->writeLineBreak();
				}
			}
			break;
		case video::EVT_TANGENTS:
			{
				video::S3DVertexTangents* vtx = (video::S3DVertexTangents*)buffer->getVertices();
				for (u32 j=0; j<vertexCount; ++j)
				{
					core::stringw str;
					str += vtx[j].Pos.X;
					str += " ";
					str += vtx[j].Pos.Y;
					str += " ";
					str += vtx[j].Pos.Z;

					Writer->writeText(str.c_str());
					Writer->writeLineBreak();
				}
			}
			break;
		}
	}

	Writer->writeClosingTag(L"array");
	Writer->writeLineBreak();

		Writer->writeElement(L"technique", false, L"profile", L"COMMON");
		Writer->writeLineBreak();

		vertexCountStr = totalVertexCount;

			Writer->writeElement(L"accessor", false, L"source", L"#mesh-Pos-array",
					L"count", vertexCountStr.c_str(), L"stride", L"3");
			Writer->writeLineBreak();

			Writer->writeElement(L"param", true, L"name", L"X", L"type", L"float", L"flow", L"OUT");
			Writer->writeLineBreak();
			Writer->writeElement(L"param", true, L"name", L"Y", L"type", L"float", L"flow", L"OUT");
			Writer->writeLineBreak();
			Writer->writeElement(L"param", true, L"name", L"Z", L"type", L"float", L"flow", L"OUT");
			Writer->writeLineBreak();

			Writer->writeClosingTag(L"accessor");
			Writer->writeLineBreak();

		Writer->writeClosingTag(L"technique");
		Writer->writeLineBreak();

	Writer->writeClosingTag(L"source");
	Writer->writeLineBreak();

	// write texture coordinates

	Writer->writeElement(L"source", false, L"id", L"mesh-TexCoord0");
	Writer->writeLineBreak();

	vertexCountStr = (totalVertexCount*2);
	Writer->writeElement(L"array", false, L"id", L"mesh-TexCoord0-array",
							L"type", L"float", L"count", vertexCountStr.c_str());
	Writer->writeLineBreak();

	for (i=0; i<mesh->getMeshBufferCount(); ++i)
	{
		scene::IMeshBuffer* buffer = mesh->getMeshBuffer(i);
		video::E_VERTEX_TYPE vtxType = buffer->getVertexType();
		u32 vertexCount = buffer->getVertexCount();

		globalIndices[i].TCoord0StartIndex = 0;

		if (i!=0)
			globalIndices[i].TCoord0StartIndex = globalIndices[i-1].TCoord0LastIndex + 1;

		globalIndices[i].TCoord0LastIndex = globalIndices[i].TCoord0StartIndex + vertexCount - 1;

		switch(vtxType)
		{
		case video::EVT_STANDARD:
			{
				video::S3DVertex* vtx = (video::S3DVertex*)buffer->getVertices();
				for (u32 j=0; j<vertexCount; ++j)
				{
					core::stringw str;
					str += vtx[j].TCoords.X;
					str += " ";
					str += vtx[j].TCoords.Y;

					Writer->writeText(str.c_str());
					Writer->writeLineBreak();
				}
			}
			break;
		case video::EVT_2TCOORDS:
			{
				video::S3DVertex2TCoords* vtx = (video::S3DVertex2TCoords*)buffer->getVertices();
				for (u32 j=0; j<vertexCount; ++j)
				{
					core::stringw str;
					str += vtx[j].TCoords.X;
					str += " ";
					str += vtx[j].TCoords.Y;

					Writer->writeText(str.c_str());
					Writer->writeLineBreak();
				}
			}
			break;
		case video::EVT_TANGENTS:
			{
				video::S3DVertexTangents* vtx = (video::S3DVertexTangents*)buffer->getVertices();
				for (u32 j=0; j<vertexCount; ++j)
				{
					core::stringw str;
					str += vtx[j].TCoords.X;
					str += " ";
					str += vtx[j].TCoords.Y;

					Writer->writeText(str.c_str());
					Writer->writeLineBreak();
				}
			}
			break;
		}
	}

	Writer->writeClosingTag(L"array");
	Writer->writeLineBreak();

		Writer->writeElement(L"technique", false, L"profile", L"COMMON");
		Writer->writeLineBreak();

		vertexCountStr = totalVertexCount;

			Writer->writeElement(L"accessor", false, L"source", L"#mesh-TexCoord0-array",
					L"count", vertexCountStr.c_str(), L"stride", L"2");
			Writer->writeLineBreak();

			Writer->writeElement(L"param", true, L"name", L"U", L"type", L"float", L"flow", L"OUT");
			Writer->writeLineBreak();
			Writer->writeElement(L"param", true, L"name", L"V", L"type", L"float", L"flow", L"OUT");
			Writer->writeLineBreak();

			Writer->writeClosingTag(L"accessor");
			Writer->writeLineBreak();

		Writer->writeClosingTag(L"technique");
		Writer->writeLineBreak();

	Writer->writeClosingTag(L"source");
	Writer->writeLineBreak();

	// write normals

	Writer->writeElement(L"source", false, L"id", L"mesh-Normal");
	Writer->writeLineBreak();

	vertexCountStr = (totalVertexCount*3);
	Writer->writeElement(L"array", false, L"id", L"mesh-Normal-array",
			L"type", L"float", L"count", vertexCountStr.c_str());
	Writer->writeLineBreak();

	for (i=0; i<mesh->getMeshBufferCount(); ++i)
	{
		scene::IMeshBuffer* buffer = mesh->getMeshBuffer(i);
		video::E_VERTEX_TYPE vtxType = buffer->getVertexType();
		u32 vertexCount = buffer->getVertexCount();

		globalIndices[i].NormalStartIndex = 0;

		if (i!=0)
			globalIndices[i].NormalStartIndex = globalIndices[i-1].NormalLastIndex + 1;

		globalIndices[i].NormalLastIndex = globalIndices[i].NormalStartIndex + vertexCount - 1;

		switch(vtxType)
		{
		case video::EVT_STANDARD:
			{
				video::S3DVertex* vtx = (video::S3DVertex*)buffer->getVertices();
				for (u32 j=0; j<vertexCount; ++j)
				{
					core::stringw str;
					str += vtx[j].Normal.X;
					str += " ";
					str += vtx[j].Normal.Y;
					str += " ";
					str += vtx[j].Normal.Z;

					Writer->writeText(str.c_str());
					Writer->writeLineBreak();
				}
			}
			break;
		case video::EVT_2TCOORDS:
			{
				video::S3DVertex2TCoords* vtx = (video::S3DVertex2TCoords*)buffer->getVertices();
				for (u32 j=0; j<vertexCount; ++j)
				{
					core::stringw str;
					str += vtx[j].Normal.X;
					str += " ";
					str += vtx[j].Normal.Y;
					str += " ";
					str += vtx[j].Normal.Z;

					Writer->writeText(str.c_str());
					Writer->writeLineBreak();
				}
			}
			break;
		case video::EVT_TANGENTS:
			{
				video::S3DVertexTangents* vtx = (video::S3DVertexTangents*)buffer->getVertices();
				for (u32 j=0; j<vertexCount; ++j)
				{
					core::stringw str;
					str += vtx[j].Normal.X;
					str += " ";
					str += vtx[j].Normal.Y;
					str += " ";
					str += vtx[j].Normal.Z;

					Writer->writeText(str.c_str());
					Writer->writeLineBreak();
				}
			}
			break;
		}
	}

	Writer->writeClosingTag(L"array");
	Writer->writeLineBreak();

		Writer->writeElement(L"technique", false, L"profile", L"COMMON");
		Writer->writeLineBreak();

		vertexCountStr = totalVertexCount;

		Writer->writeElement(L"accessor", false, L"source", L"#mesh-Normal-array",
								L"count", vertexCountStr.c_str(), L"stride", L"3");
		Writer->writeLineBreak();

			Writer->writeElement(L"param", true, L"name", L"X", L"type", L"float", L"flow", L"OUT");
			Writer->writeLineBreak();
			Writer->writeElement(L"param", true, L"name", L"Y", L"type", L"float", L"flow", L"OUT");
			Writer->writeLineBreak();
			Writer->writeElement(L"param", true, L"name", L"Z", L"type", L"float", L"flow", L"OUT");
			Writer->writeLineBreak();

		Writer->writeClosingTag(L"accessor");
		Writer->writeLineBreak();

		Writer->writeClosingTag(L"technique");
		Writer->writeLineBreak();

	Writer->writeClosingTag(L"source");
	Writer->writeLineBreak();

	// write second set of texture coordinates

	if (totalTCoords2Count)
	{
		Writer->writeElement(L"source", false, L"id", L"mesh-TexCoord1");
		Writer->writeLineBreak();

		vertexCountStr = (totalTCoords2Count*2);
		Writer->writeElement(L"array", false, L"id", L"mesh-TexCoord1-array",
								L"type", L"float", L"count", vertexCountStr.c_str());
		Writer->writeLineBreak();

		for (i=0; i<mesh->getMeshBufferCount(); ++i)
		{
			scene::IMeshBuffer* buffer = mesh->getMeshBuffer(i);
			video::E_VERTEX_TYPE vtxType = buffer->getVertexType();
			u32 vertexCount = buffer->getVertexCount();

			if (hasSecondTextureCoordinates(vtxType))
			{
				globalIndices[i].TCoord1StartIndex = 0;

				if (i!=0 && globalIndices[i-1].TCoord1LastIndex != -1)
					globalIndices[i].TCoord1StartIndex = globalIndices[i-1].TCoord1LastIndex + 1;

				globalIndices[i].TCoord1LastIndex = globalIndices[i].TCoord1StartIndex + vertexCount - 1;

				switch(vtxType)
				{
				case video::EVT_2TCOORDS:
					{
						video::S3DVertex2TCoords* vtx = (video::S3DVertex2TCoords*)buffer->getVertices();
						for (u32 j=0; j<vertexCount; ++j)
						{
							core::stringw str;
							str += vtx[j].TCoords2.X;
							str += " ";
							str += vtx[j].TCoords2.Y;

							Writer->writeText(str.c_str());
							Writer->writeLineBreak();
						}
					}
					break;
				default:
					break;
				}
			} // end this buffer has 2 texture coordinates
		}

		Writer->writeClosingTag(L"array");
		Writer->writeLineBreak();

			Writer->writeElement(L"technique", false, L"profile", L"COMMON");
			Writer->writeLineBreak();

			vertexCountStr = totalTCoords2Count;

			Writer->writeElement(L"accessor", false, L"source", L"#mesh-TexCoord1-array",
									L"count", vertexCountStr.c_str(), L"stride", L"2");
			Writer->writeLineBreak();

				Writer->writeElement(L"param", true, L"name", L"U", L"type", L"float", L"flow", L"OUT");
				Writer->writeLineBreak();
				Writer->writeElement(L"param", true, L"name", L"V", L"type", L"float", L"flow", L"OUT");
				Writer->writeLineBreak();

			Writer->writeClosingTag(L"accessor");
			Writer->writeLineBreak();

			Writer->writeClosingTag(L"technique");
			Writer->writeLineBreak();

		Writer->writeClosingTag(L"source");
		Writer->writeLineBreak();
	}

	// write tangents

	// TODO

	// write vertices

	Writer->writeElement(L"vertices", false, L"id", L"mesh-Vtx");
	Writer->writeLineBreak();

	Writer->writeElement(L"input", true, L"semantic", L"POSITION", L"source", L"#mesh-Pos");
	Writer->writeLineBreak();

	Writer->writeClosingTag(L"vertices");
	Writer->writeLineBreak();

	// write polygons

	for (i=0; i<mesh->getMeshBufferCount(); ++i)
	{
		scene::IMeshBuffer* buffer = mesh->getMeshBuffer(i);

		u32 polyCount = buffer->getIndexCount() / 3;
		core::stringw strPolyCount = polyCount;
		core::stringw strMat = "#mat";
		strMat += i;

		Writer->writeElement(L"polygons", false, L"count", strPolyCount.c_str(),
								L"material", strMat.c_str());
		Writer->writeLineBreak();

		Writer->writeElement(L"input", true, L"semantic", L"VERTEX", L"source", L"#mesh-Vtx", L"idx", L"0");
		Writer->writeLineBreak();
		Writer->writeElement(L"input", true, L"semantic", L"TEXCOORD", L"source", L"#mesh-TexCoord0", L"idx", L"1");
		Writer->writeLineBreak();
		Writer->writeElement(L"input", true, L"semantic", L"NORMAL", L"source", L"#mesh-Normal", L"idx", L"2");
		Writer->writeLineBreak();

		bool has2ndTexCoords = hasSecondTextureCoordinates(buffer->getVertexType());
		if (has2ndTexCoords)
		{
			Writer->writeElement(L"input", true, L"semantic", L"TEXCOORD", L"source", L"#mesh-TexCoord1", L"idx", L"3");
			Writer->writeLineBreak();
		}

		// write indices now

		s32 posIdx = globalIndices[i].PosStartIndex;
		s32 tCoordIdx = globalIndices[i].TCoord0StartIndex;
		s32 normalIdx = globalIndices[i].NormalStartIndex;
		s32 tCoord2Idx = globalIndices[i].TCoord1StartIndex;

		for (u32 p=0; p<polyCount; ++p)
		{
			Writer->writeElement(L"p", false);

			core::stringw strP;

			strP += buffer->getIndices()[(p*3) + 0] + posIdx;
			strP += " ";
			strP += buffer->getIndices()[(p*3) + 0] + tCoordIdx;
			strP += " ";
			strP += buffer->getIndices()[(p*3) + 0] + normalIdx;
			strP += " ";
			if (has2ndTexCoords)
			{
				strP += buffer->getIndices()[(p*3) + 0] + tCoord2Idx;
				strP += " ";
			}

			strP += buffer->getIndices()[(p*3) + 1] + posIdx;
			strP += " ";
			strP += buffer->getIndices()[(p*3) + 1] + tCoordIdx;
			strP += " ";
			strP += buffer->getIndices()[(p*3) + 1] + normalIdx;
			strP += " ";
			if (has2ndTexCoords)
			{
				strP += buffer->getIndices()[(p*3) + 1] + tCoord2Idx;
				strP += " ";
			}

			strP += buffer->getIndices()[(p*3) + 2] + posIdx;
			strP += " ";
			strP += buffer->getIndices()[(p*3) + 2] + tCoordIdx;
			strP += " ";
			strP += buffer->getIndices()[(p*3) + 2] + normalIdx;
			if (has2ndTexCoords)
			{
				strP += " ";
				strP += buffer->getIndices()[(p*3) + 2] + tCoord2Idx;
			}

			Writer->writeText(strP.c_str());

			Writer->writeClosingTag(L"p");
			Writer->writeLineBreak();
		}

		// close index buffer section

		Writer->writeClosingTag(L"polygons");
		Writer->writeLineBreak();
	}

	// close mesh and geometry

	Writer->writeClosingTag(L"mesh");
	Writer->writeLineBreak();
	Writer->writeClosingTag(L"geometry");
	Writer->writeLineBreak();

	Writer->writeClosingTag(L"library");
	Writer->writeLineBreak();

	// close everything

	Writer->writeClosingTag(L"COLLADA");
	Writer->drop();

	delete [] globalIndices;

	return true;
}


bool CColladaMeshWriter::hasSecondTextureCoordinates(video::E_VERTEX_TYPE type) const
{
	return type == video::EVT_2TCOORDS;
}


} // end namespace
} // end namespace

#endif

