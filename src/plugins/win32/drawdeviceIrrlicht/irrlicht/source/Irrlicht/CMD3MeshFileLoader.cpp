// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_MD3_LOADER_

#include "CMD3MeshFileLoader.h"
#include "CAnimatedMeshMD3.h"
#include "irrString.h"

namespace irr
{
namespace scene
{

//! Constructor
CMD3MeshFileLoader::CMD3MeshFileLoader(io::IFileSystem* fs, video::IVideoDriver* driver)
{
}


//! destructor
CMD3MeshFileLoader::~CMD3MeshFileLoader()
{
}


//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".bsp")
bool CMD3MeshFileLoader::isALoadableFileExtension(const c8* filename) const
{
	return strstr(filename, ".md3") != 0;
}


IAnimatedMesh* CMD3MeshFileLoader::createMesh(io::IReadFile* file)
{
	CAnimatedMeshMD3 * mesh = new CAnimatedMeshMD3();

	if ( mesh->loadModelFile ( 0, file ) )
		return mesh;
	
	mesh->drop ();
	return 0;
}


} // end namespace scene
} // end namespace irr

#endif // _IRR_COMPILE_WITH_MD3_LOADER_

