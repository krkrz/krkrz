// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_C_COLLADA_MESH_WRITER_H_INCLUDED__
#define __IRR_C_COLLADA_MESH_WRITER_H_INCLUDED__

#include "IMeshWriter.h"
#include "S3DVertex.h"
#include "IVideoDriver.h"

namespace irr
{
namespace io
{
	class IXMLWriter;
	class IFileSystem;
}
namespace scene
{

//! class to write meshes, implementing a COLLADA (.dae, .xml) writer
/** This writer implementation has been originally developed for irrEdit and then
merged out to the Irrlicht Engine */
class CColladaMeshWriter : public IMeshWriter
{
public:

	CColladaMeshWriter(video::IVideoDriver* driver, io::IFileSystem* fs);
	virtual ~CColladaMeshWriter();

	//! Returns the type of the mesh writer
	virtual EMESH_WRITER_TYPE getType() const;

	//! writes a mesh
	virtual bool writeMesh(io::IWriteFile* file, scene::IMesh* mesh, s32 flags=EMWF_NONE);

protected:

	bool hasSecondTextureCoordinates(video::E_VERTEX_TYPE type) const;

	struct SComponentGlobalStartPos
	{
		SComponentGlobalStartPos() : PosStartIndex(-1), PosLastIndex(-1),
				NormalStartIndex(-1), NormalLastIndex(-1),
				TCoord0StartIndex(-1), TCoord0LastIndex(-1),
				TCoord1StartIndex(-1), TCoord1LastIndex(-1)
		{ }

		s32 PosStartIndex;
		s32 PosLastIndex;

		s32 NormalStartIndex;
		s32 NormalLastIndex;

		s32 TCoord0StartIndex;
		s32 TCoord0LastIndex;

		s32 TCoord1StartIndex;
		s32 TCoord1LastIndex;
	};

	io::IFileSystem* FileSystem;
	video::IVideoDriver* VideoDriver;
	io::IXMLWriter* Writer;
};


} // end namespace
} // end namespace

#endif

