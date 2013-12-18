// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_MESH_MANIPULATOR_H_INCLUDED__
#define __C_MESH_MANIPULATOR_H_INCLUDED__

#include "IMeshManipulator.h"

namespace irr
{
namespace scene
{

//! An interface for easy manipulation of meshes.
/** Scale, set alpha value, flip surfaces, and so on. This exists for fixing problems 
 with wrong imported or exported meshes quickly after loading. It is not intended for doing mesh
 modifications and/or animations during runtime.
*/
class CMeshManipulator : public IMeshManipulator
{
public:

	//! destructor
	virtual ~CMeshManipulator() {}

	//! Flips the direction of surfaces. Changes backfacing triangles to frontfacing
	//! triangles and vice versa.
	//! \param mesh: Mesh on which the operation is performed.
	virtual void flipSurfaces(scene::IMesh* mesh) const;

	//! Sets the alpha vertex color value of the whole mesh to a new value
	//! \param mesh: Mesh on which the operation is performed.
	//! \param alpha: New alpha for the vertex color.
	virtual void setVertexColorAlpha(scene::IMesh* mesh, s32 alpha) const;

	//! Sets the colors of all vertices to one color
	virtual void setVertexColors(IMesh* mesh, video::SColor color) const;

	//! Recalculates all normals of the mesh.
	/** \param mesh: Mesh on which the operation is performed.
	    \param smooth: Whether to use smoothed normals. */
	virtual void recalculateNormals(scene::IMesh* mesh, bool smooth = false) const;

	//! Recalculates all normals of the mesh buffer.
	/** \param buffer: Mesh buffer on which the operation is performed.
	    \param smooth: Whether to use smoothed normals. */
	virtual void recalculateNormals(IMeshBuffer* buffer, bool smooth = false) const;

	//! Scales the whole mesh.
	//! \param mesh: Mesh on which the operation is performed.
	//! \param scale: 3D Vector, defining the value, for each axis, to scale the mesh by.
	virtual void scaleMesh(scene::IMesh* mesh, const core::vector3df& scale) const;

	//! Applies a transformation
	/** \param mesh: Mesh on which the operation is performed.
		\param m: transformation matrix. */
	virtual void transformMesh(scene::IMesh* mesh, const core::matrix4& m) const;

	//! Clones a static IMesh into a modifiable SMesh.
	virtual SMesh* createMeshCopy(scene::IMesh* mesh) const;

	//! Creates a planar texture mapping on the mesh
	//! \param mesh: Mesh on which the operation is performed.
	//! \param resolution: resolution of the planar mapping. This is the value
	//! specifying which is the relation between world space and 
	//! texture coordinate space.
	virtual void makePlanarTextureMapping(scene::IMesh* mesh, f32 resolution) const;

	//! Creates a copy of the mesh, which will only consist of S3DVertexTangents vertices.
	//! This is useful if you want to draw tangent space normal mapped geometry because
	//! it calculates the tangent and binormal data which is needed there.
	//! \param mesh: Input mesh
	//! \return Mesh consiting only of S3DVertexNormalMapped vertices.
	//! If you no longer need the cloned mesh, you should call IMesh::drop().
	//! See IReferenceCounted::drop() for more information.
	virtual IMesh* createMeshWithTangents(IMesh* mesh) const;
	virtual IMesh* createMeshWith2TCoords(IMesh* mesh) const;

	virtual IMesh* createMeshUniquePrimitives(IMesh* mesh) const;

	virtual IMesh* createMeshWelded(IMesh *mesh, f32 tolerance=core::ROUNDING_ERROR_32) const;

	//! Returns amount of polygons in mesh.
	virtual s32 getPolyCount(scene::IMesh* mesh) const;

	//! Returns amount of polygons in mesh.
	virtual s32 getPolyCount(scene::IAnimatedMesh* mesh) const;

	//! create a new AnimatedMesh and adds the mesh to it
	virtual IAnimatedMesh * createAnimatedMesh(scene::IMesh* mesh,scene::E_ANIMATED_MESH_TYPE type) const;

private:

	static void calculateTangents(core::vector3df& normal, 
		core::vector3df& tangent, 
		core::vector3df& binormal, 
		const core::vector3df& vt1, const core::vector3df& vt2, const core::vector3df& vt3,
		const core::vector2df& tc1, const core::vector2df& tc2, const core::vector2df& tc3);
};

} // end namespace scene
} // end namespace irr


#endif

