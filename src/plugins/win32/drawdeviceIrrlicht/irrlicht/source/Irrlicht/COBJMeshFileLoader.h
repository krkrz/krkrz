// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_OBJ_MESH_FILE_LOADER_H_INCLUDED__
#define __C_OBJ_MESH_FILE_LOADER_H_INCLUDED__

#include "IMeshLoader.h"
#include "IFileSystem.h"
#include "IVideoDriver.h"
#include "irrString.h"
#include "SMeshBuffer.h"
#include "irrMap.h"

namespace irr
{
namespace scene
{

//! Meshloader capable of loading obj meshes.
class COBJMeshFileLoader : public IMeshLoader
{
public:

	//! Constructor
	COBJMeshFileLoader(io::IFileSystem* fs, video::IVideoDriver* driver);

	//! destructor
	virtual ~COBJMeshFileLoader();

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".obj")
	virtual bool isALoadableFileExtension(const c8* fileName) const;

	//! creates/loads an animated mesh from the file.
	//! \return Pointer to the created mesh. Returns 0 if loading failed.
	//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
	//! See IReferenceCounted::drop() for more information.
	virtual IAnimatedMesh* createMesh(io::IReadFile* file);

private:

	struct SObjMtl
	{
		SObjMtl() : Meshbuffer(0), Illumination(0) {
			Meshbuffer = new SMeshBuffer();
			Meshbuffer->Material.Shininess = 0.0f;
			Meshbuffer->Material.AmbientColor = video::SColorf(0.2f, 0.2f, 0.2f, 1.0f).toSColor();
			Meshbuffer->Material.DiffuseColor = video::SColorf(0.8f, 0.8f, 0.8f, 1.0f).toSColor();
			Meshbuffer->Material.SpecularColor = video::SColorf(1.0f, 1.0f, 1.0f, 1.0f).toSColor();
		}

		SObjMtl(SObjMtl& o) : Meshbuffer(o.Meshbuffer), Name(o.Name), Illumination(o.Illumination) { o.Meshbuffer->grab(); }

		core::map<video::S3DVertex, int> VertMap;
		scene::SMeshBuffer *Meshbuffer;
		core::stringc Name;
		c8 Illumination;
	};

	// returns a pointer to the first printable character available in the buffer
	const c8* goFirstWord(const c8* buf, const c8* const bufEnd);
	// returns a pointer to the first printable character after the first non-printable
	const c8* goNextWord(const c8* buf, const c8* const bufEnd);
	// returns a pointer to the next printable character after the first line break
	const c8* goNextLine(const c8* buf, const c8* const bufEnd);
	// copies the current word from the inBuf to the outBuf
	u32 copyWord(c8* outBuf, const c8* inBuf, u32 outBufLength, const c8* const pBufEnd);
	// copies the current line from the inBuf to the outBuf
	core::stringc copyLine(const c8* inBuf, const c8* const bufEnd);
	// combination of goNextWord followed by copyWord
	const c8* goAndCopyNextWord(c8* outBuf, const c8* inBuf, u32 outBufLength, const c8* const pBufEnd);

	//! Read the material from the given file
	void readMTL(const c8* fileName, core::stringc relPath);
	//! Find and return the material with the given name
	SObjMtl * findMtl(const c8* mtlName);

	//! Read RGB color
	const c8* readColor(const c8* bufPtr, video::SColor& color, const c8* const pBufEnd);
	//! Read 3d vector of floats
	const c8* readVec3(const c8* bufPtr, core::vector3df& vec, const c8* const pBufEnd);
	//! Read 2d vector of floats
	const c8* readVec2(const c8* bufPtr, core::vector2df& vec, const c8* const pBufEnd);
	//! Read boolean value represented as 'on' or 'off'
	const c8* readBool(const c8* bufPtr, bool& tf, const c8* const bufEnd);

	// reads and convert to integer the vertex indices in a line of obj file's face statement
	// -1 for the index if it doesn't exist
	// indices are changed to 0-based index instead of 1-based from the obj file
	bool retrieveVertexIndices(c8* vertexData, s32* idx, const c8* bufEnd, u32 vbsize, u32 vtsize, u32 vnsize);

	void cleanUp();

	io::IFileSystem* FileSystem;
	video::IVideoDriver* Driver;

	core::array<SObjMtl*> Materials;
};

} // end namespace scene
} // end namespace irr

#endif

