// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_BSP_MESH_FILE_LOADER_H_INCLUDED__
#define __C_BSP_MESH_FILE_LOADER_H_INCLUDED__

#include "IMeshLoader.h"
#include "IFileSystem.h"
#include "IVideoDriver.h"
#include "ISceneManager.h"

namespace irr
{
namespace scene
{

//! Meshloader capable of loading Quake 3 BSP files and shaders
class CBSPMeshFileLoader : public IMeshLoader
{
public:

	//! Constructor
	CBSPMeshFileLoader(io::IFileSystem* fs, video::IVideoDriver* driver, scene::ISceneManager* smgr);

	//! destructor
	virtual ~CBSPMeshFileLoader();

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".bsp")
	virtual bool isALoadableFileExtension(const c8* fileName) const;

	//! creates/loads an animated mesh from the file.
	//! \return Pointer to the created mesh. Returns 0 if loading failed.
	//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
	//! See IReferenceCounted::drop() for more information.
	virtual IAnimatedMesh* createMesh(io::IReadFile* file);

private:

	io::IFileSystem* FileSystem;
	video::IVideoDriver* Driver;
	scene::ISceneManager* SceneManager;
};

} // end namespace scene
} // end namespace irr

#endif

