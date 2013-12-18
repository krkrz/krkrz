// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_MESH_MANIPULATOR_H_INCLUDED__
#define __I_MESH_MANIPULATOR_H_INCLUDED__

#include "IReferenceCounted.h"
#include "vector3d.h"
#include "aabbox3d.h"
#include "matrix4.h"
#include "IAnimatedMesh.h"
#include "SColor.h"

namespace irr
{
namespace scene
{

	class IMesh;
	class IMeshBuffer;
	struct SMesh;

	//! An interface for easy manipulation of meshes.
	/** Scale, set alpha value, flip surfaces, and so on. This exists for
	fixing problems with wrong imported or exported meshes quickly after
	loading. It is not intended for doing mesh modifications and/or
	animations during runtime.
	*/
	class IMeshManipulator : public virtual IReferenceCounted
	{
	public:

		//! Destructor
		virtual ~IMeshManipulator() {}

		//! Flips the direction of surfaces.
		/** Changes backfacing triangles to frontfacing
		triangles and vice versa.
		\param mesh Mesh on which the operation is performed. */
		virtual void flipSurfaces(IMesh* mesh) const = 0;

		//! Sets the alpha vertex color value of the whole mesh to a new value.
		/** \param mesh Mesh on which the operation is performed.
		\param alpha New alpha value. Must be a value between 0 and 255. */
		virtual void setVertexColorAlpha(IMesh* mesh, s32 alpha) const = 0;

		//! Sets the colors of all vertices to one color
		/** \param mesh Mesh on which the operation is performed.
		\param color New color. */
		virtual void setVertexColors(IMesh* mesh, video::SColor color) const = 0;

		//! Recalculates all normals of the mesh.
		/** \param mesh: Mesh on which the operation is performed.
		\param smooth: If the normals shall be smoothed. */
		virtual void recalculateNormals(IMesh* mesh, bool smooth = false) const = 0;

		//! Recalculates all normals of the mesh buffer.
		/** \param buffer: Mesh buffer on which the operation is performed.
		\param smooth: If the normals shall be smoothed. */
		virtual void recalculateNormals(IMeshBuffer* buffer, bool smooth = false) const = 0;

		//! Scales the whole mesh.
		/** \param mesh: Mesh on which the operation is performed.
		\param scale: Scale factor. */
		virtual void scaleMesh(IMesh* mesh, const core::vector3df& scale) const = 0;

		//! Applies a transformation
		/** \param mesh: Mesh on which the operation is performed.
		\param m: transformation matrix. */
		virtual void transformMesh(IMesh* mesh, const core::matrix4& m) const = 0;

		//! Clones a static IMesh into a modifiable SMesh.
		/** All meshbuffers in the returned SMesh
		are of type SMeshBuffer or SMeshBufferLightMap.
		\param mesh: Mesh to copy.
		\return Returns the cloned mesh. If you no longer need the
		cloned mesh, you should call SMesh::drop(). See
		IReferenceCounted::drop() for more information. */
		virtual SMesh* createMeshCopy(IMesh* mesh) const = 0;

		//! Creates a planar texture mapping on the mesh
		/** \param mesh: Mesh on which the operation is performed.
		\param resolution: resolution of the planar mapping. This is
		the value specifying which is the relation between world space
		and texture coordinate space. */
		virtual void makePlanarTextureMapping(IMesh* mesh, f32 resolution=0.001f) const = 0;

		//! Creates a copy of the mesh, which will only consist of S3DVertexTangents vertices.
		/** This is useful if you want to draw tangent space normal
		mapped geometry because it calculates the tangent and binormal
		data which is needed there.
		\param mesh Input mesh
		\return Mesh consisting only of S3DVertexTangents vertices. If
		you no longer need the cloned mesh, you should call
		IMesh::drop(). See IReferenceCounted::drop() for more
		information. */
		virtual IMesh* createMeshWithTangents(IMesh* mesh) const = 0;

		//! Creates a copy of the mesh, which will only consist of S3DVertex2TCoord vertices.
		/** \param mesh Input mesh
		\return Mesh consisting only of S3DVertex2TCoord vertices. If
		you no longer need the cloned mesh, you should call
		IMesh::drop(). See IReferenceCounted::drop() for more
		information. */
		virtual IMesh* createMeshWith2TCoords(IMesh* mesh) const = 0;

		//! Creates a copy of a mesh with all vertices unwelded
		/** \param mesh Input mesh
		\return Mesh consisting only of unique faces. All vertices
		which were previously shared are now duplicated. If you no
		longer need the cloned mesh, you should call IMesh::drop(). See
		IReferenceCounted::drop() for more information. */
		virtual IMesh* createMeshUniquePrimitives(IMesh* mesh) const = 0;

		//! Creates a copy of a mesh with vertices welded
		/** \param mesh Input mesh
		\param tolerance The threshold for vertex comparisons.
		\return Mesh without redundant vertices. If you no longer need
		the cloned mesh, you should call IMesh::drop(). See
		IReferenceCounted::drop() for more information. */
		virtual IMesh* createMeshWelded(IMesh* mesh, f32 tolerance=core::ROUNDING_ERROR_32) const = 0;

		//! Get amount of polygons in mesh.
		/** \param mesh Input mesh
		\return Number of polygons in mesh. */
		virtual s32 getPolyCount(IMesh* mesh) const = 0;

		//! Get amount of polygons in mesh.
		/** \param mesh Input mesh
		\return Number of polygons in mesh. */
		virtual s32 getPolyCount(IAnimatedMesh* mesh) const = 0;

		//! Create a new AnimatedMesh and adds the mesh to it
		/** \param mesh Input mesh
		\param type The type of the animated mesh to create.
		\return Newly created animated mesh with mesh as it's only
		content. When you don't need the animated mesh anymore, you
		should call IAnimatedMesh::drop(). See
		IReferenceCounted::drop() for more information. */
		virtual IAnimatedMesh * createAnimatedMesh(IMesh* mesh,
			scene::E_ANIMATED_MESH_TYPE type = scene::EAMT_UNKNOWN) const = 0;
	};

} // end namespace scene
} // end namespace irr


#endif

