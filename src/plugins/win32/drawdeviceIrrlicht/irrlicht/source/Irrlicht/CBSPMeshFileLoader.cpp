// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_BSP_LOADER_

#include "CBSPMeshFileLoader.h"
#include "CQ3LevelMesh.h"

namespace irr
{
namespace scene
{

//! Constructor
CBSPMeshFileLoader::CBSPMeshFileLoader(io::IFileSystem* fs,video::IVideoDriver* driver, scene::ISceneManager* smgr)
: FileSystem(fs), Driver(driver), SceneManager(smgr)
{
	if (FileSystem)
		FileSystem->grab();

	if (Driver)
		Driver->grab();
}


//! destructor
CBSPMeshFileLoader::~CBSPMeshFileLoader()
{
	if (FileSystem)
		FileSystem->drop();

	if (Driver)
		Driver->drop();
}


//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".bsp")
bool CBSPMeshFileLoader::isALoadableFileExtension(const c8* filename) const
{
	return strstr(filename, ".bsp") || strstr(filename, ".shader");
}


//! creates/loads an animated mesh from the file.
//! \return Pointer to the created mesh. Returns 0 if loading failed.
//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
//! See IReferenceCounted::drop() for more information.
IAnimatedMesh* CBSPMeshFileLoader::createMesh(io::IReadFile* file)
{
	// load quake 3 bsp
	if (strstr(file->getFileName(), ".bsp"))
	{
		CQ3LevelMesh* q = new CQ3LevelMesh(FileSystem, Driver, SceneManager);

		q->getShader ( "scripts/models.shader", 1 );
		q->getShader ( "scripts/liquid.shader", 1 );
		//q->getShader ( "scripts/sky.shader", 1 );

		if ( q->loadFile(file) )
			return q;

		q->drop();
	}

	// load quake 3 shader container
	if (strstr(file->getFileName(), ".shader"))
	{
		CQ3LevelMesh* q = new CQ3LevelMesh(FileSystem, Driver, SceneManager);
		q->getShader ( file->getFileName(), 1 );
		return q;
	}

	return 0;
}

} // end namespace scene
} // end namespace irr

#endif // _IRR_COMPILE_WITH_BSP_LOADER_

