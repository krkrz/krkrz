// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CMeshCache.h"
#include "IAnimatedMesh.h"
#include "IMesh.h"

namespace irr
{
namespace scene
{


CMeshCache::~CMeshCache()
{
	clear();
}


//! adds a mesh to the list
void CMeshCache::addMesh(const c8* filename, IAnimatedMesh* mesh)
{
	mesh->grab();

	MeshEntry e;
	e.Mesh = mesh;
	e.Name = filename;
	e.Name.make_lower();

	Meshes.push_back(e);
}


//! Removes a mesh from the cache.
void CMeshCache::removeMesh(const IAnimatedMesh* const mesh)
{
	if ( !mesh )
		return;
	for (u32 i=0; i<Meshes.size(); ++i)
	{
		if (Meshes[i].Mesh == mesh)
		{
			Meshes[i].Mesh->drop();
			Meshes.erase(i);
			return;
		}
	}
}


//! Removes a mesh from the cache.
void CMeshCache::removeMesh(const IMesh* const mesh)
{
	if ( !mesh )
		return;
	for (u32 i=0; i<Meshes.size(); ++i)
	{
		if (Meshes[i].Mesh && Meshes[i].Mesh->getMesh(0) == mesh)
		{
			Meshes[i].Mesh->drop();
			Meshes.erase(i);
			return;
		}
	}
}


//! Returns amount of loaded meshes
u32 CMeshCache::getMeshCount() const
{
	return Meshes.size();
}


//! Returns current number of the mesh
s32 CMeshCache::getMeshIndex(const IAnimatedMesh* const mesh) const
{
	for (u32 i=0; i<Meshes.size(); ++i)
		if (Meshes[i].Mesh == mesh)
			return (s32)i;

	return -1;
}



//! Returns current index number of the mesh, and -1 if it is not in the cache.
s32 CMeshCache::getMeshIndex(const IMesh* const mesh) const
{
	for (u32 i=0; i<Meshes.size(); ++i)
	{
		if (Meshes[i].Mesh && Meshes[i].Mesh->getMesh(0) == mesh)
			return (s32)i;
	}

	return -1;
}


//! Returns a mesh based on its index number
IAnimatedMesh* CMeshCache::getMeshByIndex(u32 number)
{
	if (number >= Meshes.size())
		return 0;

	return Meshes[number].Mesh;
}


//! Returns a mesh based on its file name.
IAnimatedMesh* CMeshCache::getMeshByFilename(const c8* filename)
{
	MeshEntry e;
	e.Name = filename;
	e.Name.make_lower();
	s32 id = Meshes.binary_search(e);
	return (id != -1) ? Meshes[id].Mesh : 0;
}


//! Returns name of a mesh based on its index number
const c8* CMeshCache::getMeshFilename(u32 number) const
{
	if (number >= Meshes.size())
		return 0;

	return Meshes[number].Name.c_str();
}



//! Returns the filename of a loaded mesh, if there is any. Returns 0 if there is none.
const c8* CMeshCache::getMeshFilename(const IAnimatedMesh* const mesh) const
{
	for (u32 i=0; i<Meshes.size(); ++i)
	{
		if (Meshes[i].Mesh == mesh)
			return Meshes[i].Name.c_str();
	}

	return 0;
}


//! Returns the filename of a loaded mesh, if there is any. Returns 0 if there is none.
const c8* CMeshCache::getMeshFilename(const IMesh* const mesh) const
{
	for (u32 i=0; i<Meshes.size(); ++i)
	{
		if (Meshes[i].Mesh && Meshes[i].Mesh->getMesh(0) == mesh)
			return Meshes[i].Name.c_str();
	}

	return 0;
}



//! Renames a loaded mesh, if possible.
bool CMeshCache::setMeshFilename(u32 index, const c8* filename)
{
	if (index >= Meshes.size())
		return false;

	Meshes[index].Name = filename;
	Meshes.sort();
	return true;
}


//! Renames a loaded mesh, if possible.
bool CMeshCache::setMeshFilename(const IAnimatedMesh* const mesh, const c8* filename)
{
	for (u32 i=0; i<Meshes.size(); ++i)
	{
		if (Meshes[i].Mesh == mesh)
		{
			Meshes[i].Name = filename;
			Meshes.sort();
			return true;
		}
	}

	return false;
}


//! Renames a loaded mesh, if possible.
bool CMeshCache::setMeshFilename(const IMesh* const mesh, const c8* filename)
{
	for (u32 i=0; i<Meshes.size(); ++i)
	{
		if (Meshes[i].Mesh && Meshes[i].Mesh->getMesh(0) == mesh)
		{
			Meshes[i].Name = filename;
			Meshes.sort();
			return true;
		}
	}

	return false;
}


//! returns if a mesh already was loaded
bool CMeshCache::isMeshLoaded(const c8* filename)
{
	return getMeshByFilename(filename) != 0;
}


//! Clears the whole mesh cache, removing all meshes.
void CMeshCache::clear()
{
	for (u32 i=0; i<Meshes.size(); ++i)
		Meshes[i].Mesh->drop();

	Meshes.clear();
}

//! Clears all meshes that are held in the mesh cache but not used anywhere else.
void CMeshCache::clearUnusedMeshes()
{
	for (u32 i=0; i<Meshes.size(); ++i)
	{
		if (Meshes[i].Mesh->getReferenceCount() == 1)
		{
			Meshes[i].Mesh->drop();
			Meshes.erase(i);
			--i;
		}
	}
}


} // end namespace scene
} // end namespace irr

