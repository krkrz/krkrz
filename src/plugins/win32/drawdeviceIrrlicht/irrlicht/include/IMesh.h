// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_MESH_H_INCLUDED__
#define __I_MESH_H_INCLUDED__

#include "IReferenceCounted.h"
#include "SMaterial.h"

namespace irr
{
namespace scene
{
	class IMeshBuffer;

	//! Class for accessing a mesh with multiple mesh buffers.
	/** An IMesh is nothing more than a collection of some mesh buffers
	(IMeshBuffer). SMesh is a simple implementation of an IMesh.
	*/
	class IMesh : public virtual IReferenceCounted
	{
	public:

		//! Destructor
		virtual ~IMesh() { }

		//! Returns the amount of mesh buffers.
		/** \return Returns the amount of mesh buffers (IMeshBuffer) in this mesh. */
		virtual u32 getMeshBufferCount() const = 0;

		//! Returns pointer to a mesh buffer.
		/** \param nr: Zero based index of the mesh buffer. The maximum value is
		getMeshBufferCount() - 1;
		\return Returns the pointer to the mesh buffer or
		NULL if there is no such mesh buffer. */
		virtual IMeshBuffer* getMeshBuffer(u32 nr) const = 0;

		//! Returns pointer to a mesh buffer which fits a material
		/** \param material: material to search for
		\return Returns the pointer to the mesh buffer or
		NULL if there is no such mesh buffer. */
		virtual IMeshBuffer* getMeshBuffer( const video::SMaterial &material) const = 0;

		//! Returns an axis aligned bounding box of the mesh.
		/** \return A bounding box of this mesh is returned. */
		virtual const core::aabbox3d<f32>& getBoundingBox() const = 0;

		//! set user axis aligned bounding box
		/** \param box New bounding box to use for the mesh. */
		virtual void setBoundingBox( const core::aabbox3df& box) = 0;

		//! Sets a flag of all contained materials to a new value.
		/** \param flag: Flag to set in all materials.
		\param newvalue: New value to set in all materials. */
		virtual void setMaterialFlag(video::E_MATERIAL_FLAG flag, bool newvalue) = 0;
	};

} // end namespace scene
} // end namespace irr

#endif

