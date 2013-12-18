// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CMeshManipulator.h"
#include "SMesh.h"
#include "CMeshBuffer.h"
#include "SAnimatedMesh.h"
#include "os.h"

namespace irr
{
namespace scene
{

//! Recalculates the normals in vertex array.
//! This template function was a member of the CMeshManipulator class, but
//! visual studio 6.0 didn't like it.
template<class VTXTYPE>
inline void recalculateNormalsT_Flat(VTXTYPE* v, int vtxcnt,
					u16* idx, int idxcnt)
{
	for (int i=0; i<idxcnt; i+=3)
	{
		const core::plane3d<f32> p(v[idx[i+0]].Pos, v[idx[i+1]].Pos, v[idx[i+2]].Pos);
		v[idx[i+0]].Normal = p.Normal;
		v[idx[i+1]].Normal = p.Normal;
		v[idx[i+2]].Normal = p.Normal;
	}
}

template<class VTXTYPE>
inline void recalculateNormalsT_Smooth(VTXTYPE* v, int vtxcnt,
					u16* idx, int idxcnt)
{
	s32 i;

	for ( i = 0; i!= vtxcnt; ++i )
	{
		v[i].Normal.set ( 0.f, 0.f, 0.f );
	}

	for ( i=0; i<idxcnt; i+=3)
	{
		const core::plane3d<f32> p(v[idx[i+0]].Pos, v[idx[i+1]].Pos, v[idx[i+2]].Pos);
		v[idx[i+0]].Normal += p.Normal;
		v[idx[i+1]].Normal += p.Normal;
		v[idx[i+2]].Normal += p.Normal;
	}

	for ( i = 0; i!= vtxcnt; ++i )
	{
		v[i].Normal.normalize ();
	}
}

//! Recalculates normals in a vertex array.
//! This template function was a member of the CMeshManipulator class, but
//! visual studio 6.0 didn't like it.
template<class VERTEXTYPE>
inline void makePlanarMappingT(VERTEXTYPE *v,
				   int vtxcnt,
				   u16* idx, int idxcnt, f32 resolution)
{
	for (int i=0; i<idxcnt; i+=3)
	{
		core::plane3d<f32> p(v[idx[i+0]].Pos, v[idx[i+1]].Pos, v[idx[i+2]].Pos);
		p.Normal.X = fabsf(p.Normal.X);
		p.Normal.Y = fabsf(p.Normal.Y);
		p.Normal.Z = fabsf(p.Normal.Z);
		// calculate planar mapping worldspace coordinates

		if (p.Normal.X > p.Normal.Y && p.Normal.X > p.Normal.Z)
		{
			for (s32 o=0; o<3; ++o)
			{
				v[idx[i+o]].TCoords.X = v[idx[i+o]].Pos.Y * resolution;
				v[idx[i+o]].TCoords.Y = v[idx[i+o]].Pos.Z * resolution;
			}
		}
		else
		if (p.Normal.Y > p.Normal.X && p.Normal.Y > p.Normal.Z)
		{
			for (s32 o=0; o<3; ++o)
			{
				v[idx[i+o]].TCoords.X = v[idx[i+o]].Pos.X * resolution;
				v[idx[i+o]].TCoords.Y = v[idx[i+o]].Pos.Z * resolution;
			}
		}
		else
		{
			for (s32 o=0; o<3; ++o)
			{
				v[idx[i+o]].TCoords.X = v[idx[i+o]].Pos.X * resolution;
				v[idx[i+o]].TCoords.Y = v[idx[i+o]].Pos.Y * resolution;
			}
		}
	}
}


//! Flips the direction of surfaces. Changes backfacing triangles to frontfacing
//! triangles and vice versa.
//! \param mesh: Mesh on which the operation is performed.
void CMeshManipulator::flipSurfaces(scene::IMesh* mesh) const
{
	if (!mesh)
		return;

	const u32 bcount = mesh->getMeshBufferCount();
	for (u32 b=0; b<bcount; ++b)
	{
		IMeshBuffer* buffer = mesh->getMeshBuffer(b);
		const u32 idxcnt = buffer->getIndexCount();
		u16* idx = buffer->getIndices();
		s32 tmp;

		for (u32 i=0; i<idxcnt; i+=3)
		{
			tmp = idx[i+1];
			idx[i+1] = idx[i+2];
			idx[i+2] = tmp;
		}
	}
}



//! Sets the alpha vertex color value of the whole mesh to a new value
//! \param mesh: Mesh on which the operation is performed.
void CMeshManipulator::setVertexColorAlpha(scene::IMesh* mesh, s32 alpha) const
{
	if (!mesh)
		return;

	u32 i;

	const u32 bcount = mesh->getMeshBufferCount();
	for ( u32 b=0; b<bcount; ++b)
	{
		IMeshBuffer* buffer = mesh->getMeshBuffer(b);
		void* v = buffer->getVertices();
		u32 vtxcnt = buffer->getVertexCount();

		switch(buffer->getVertexType())
		{
		case video::EVT_STANDARD:
			{
				for ( i=0; i<vtxcnt; ++i)
					((video::S3DVertex*)v)[i].Color.setAlpha(alpha);
			}
			break;
		case video::EVT_2TCOORDS:
			{
				for ( i=0; i<vtxcnt; ++i)
					((video::S3DVertex2TCoords*)v)[i].Color.setAlpha(alpha);
			}
			break;
		case video::EVT_TANGENTS:
			{
				for ( i=0; i<vtxcnt; ++i)
					((video::S3DVertexTangents*)v)[i].Color.setAlpha(alpha);
			}
			break;
		}
	}
}



//! Sets the colors of all vertices to one color
void CMeshManipulator::setVertexColors(IMesh* mesh, video::SColor color) const
{
	if (!mesh)
		return;

	const u32 bcount = mesh->getMeshBufferCount();
	for (u32 b=0; b<bcount; ++b)
	{
		IMeshBuffer* buffer = mesh->getMeshBuffer(b);
		void* v = buffer->getVertices();
		const u32 vtxcnt = buffer->getVertexCount();
		u32 i;

		switch(buffer->getVertexType())
		{
		case video::EVT_STANDARD:
			{
				for ( i=0; i<vtxcnt; ++i)
					((video::S3DVertex*)v)[i].Color = color;
			}
			break;
		case video::EVT_2TCOORDS:
			{
				for ( i=0; i<vtxcnt; ++i)
					((video::S3DVertex2TCoords*)v)[i].Color = color;
			}
			break;
		case video::EVT_TANGENTS:
			{
				for ( i=0; i<vtxcnt; ++i)
					((video::S3DVertexTangents*)v)[i].Color = color;
			}
			break;
		}
	}
}



//! Recalculates all normals of the mesh buffer.
/** \param buffer: Mesh buffer on which the operation is performed. */
void CMeshManipulator::recalculateNormals(IMeshBuffer* buffer, bool smooth) const
{
	if (!buffer)
		return;

	u32 vtxcnt = buffer->getVertexCount();
	u32 idxcnt = buffer->getIndexCount();
	u16* idx = buffer->getIndices();

	switch(buffer->getVertexType())
	{
	case video::EVT_STANDARD:
		{
			video::S3DVertex* v = (video::S3DVertex*)buffer->getVertices();

			if (!smooth)
				recalculateNormalsT_Flat(v, vtxcnt, idx, idxcnt);
			else
				recalculateNormalsT_Smooth(v, vtxcnt, idx, idxcnt);
		}
		break;
	case video::EVT_2TCOORDS:
		{
			video::S3DVertex2TCoords* v = (video::S3DVertex2TCoords*)buffer->getVertices();

			if (!smooth)
				recalculateNormalsT_Flat(v, vtxcnt, idx, idxcnt);
			else
				recalculateNormalsT_Smooth(v, vtxcnt, idx, idxcnt);
		}
		break;
	case video::EVT_TANGENTS:
		{
			// TODO: recalculate tangent and binormal
			video::S3DVertexTangents* v = (video::S3DVertexTangents*)buffer->getVertices();
			if (!smooth)
				recalculateNormalsT_Flat(v, vtxcnt, idx, idxcnt);
			else
				recalculateNormalsT_Smooth(v, vtxcnt, idx, idxcnt);
		}
	}
}



//! Recalculates all normals of the mesh.
//! \param mesh: Mesh on which the operation is performed.
void CMeshManipulator::recalculateNormals(scene::IMesh* mesh, bool smooth) const
{
	if (!mesh)
		return;

	const u32 bcount = mesh->getMeshBufferCount();
	for ( u32 b=0; b<bcount; ++b)
		recalculateNormals(mesh->getMeshBuffer(b), smooth);
}


//! Applies a transformation
/** \param mesh: Mesh on which the operation is performed.
	\param m: matrix. */
void CMeshManipulator::transformMesh(scene::IMesh* mesh, const core::matrix4& m) const
{
	if (!mesh)
		return;

	core::aabbox3df meshbox;
	core::aabbox3df bufferbox;
	u32 i;

	const u32 bcount = mesh->getMeshBufferCount();
	for ( u32 b=0; b<bcount; ++b)
	{
		IMeshBuffer* buffer = mesh->getMeshBuffer(b);

		const u32 vtxcnt = buffer->getVertexCount();
		const u32 vtxPitch = video::getVertexPitchFromType(buffer->getVertexType());

		video::S3DVertex* v = (video::S3DVertex*) buffer->getVertices();

		for ( i=0; i < 1; ++i)
		{
			m.transformVect ( v->Pos);
			m.rotateVect ( v->Normal );
			v->Normal.normalize();

			bufferbox.reset( v->Pos);
			v = (video::S3DVertex*) ((u8*) v + vtxPitch);
		}

		for ( ;i < vtxcnt; ++i)
		{
			m.transformVect ( v->Pos);
			m.rotateVect ( v->Normal );
			v->Normal.normalize();

			bufferbox.addInternalPoint( v->Pos);
			v = (video::S3DVertex*) ((u8*) v + vtxPitch);
		}

		buffer->setBoundingBox(bufferbox);

		if (b == 0)
			meshbox.reset(buffer->getBoundingBox());
		else
			meshbox.addInternalBox(buffer->getBoundingBox());
	}

	mesh->setBoundingBox( meshbox );
}


//! Scales the whole mesh.
//! \param mesh: Mesh on which the operation is performed.
void CMeshManipulator::scaleMesh(scene::IMesh* mesh, const core::vector3df& scale) const
{
	if (!mesh)
		return;

	core::aabbox3df meshbox;

	const u32 bcount = mesh->getMeshBufferCount();
	for ( u32 b=0; b<bcount; ++b)
	{
		IMeshBuffer* buffer = mesh->getMeshBuffer(b);
		void* v = buffer->getVertices();
		const u32 vtxcnt = buffer->getVertexCount();
		core::aabbox3df bufferbox;
		u32 i;

		switch(buffer->getVertexType())
		{
		case video::EVT_STANDARD:
			{
				if (vtxcnt != 0)
					bufferbox.reset(((video::S3DVertex*)v)[0].Pos * scale);

				for ( i=0; i<vtxcnt; ++i)
				{
					((video::S3DVertex*)v)[i].Pos *= scale;
					bufferbox.addInternalPoint(((video::S3DVertex*)v)[i].Pos);
				}
			}
			break;

		case video::EVT_2TCOORDS:
			{
				if (vtxcnt != 0)
					bufferbox.reset(((video::S3DVertex2TCoords*)v)[0].Pos * scale);

				for ( i=0; i<vtxcnt; ++i)
				{
					((video::S3DVertex2TCoords*)v)[i].Pos *= scale;
					bufferbox.addInternalPoint(((video::S3DVertex2TCoords*)v)[i].Pos);
				}
			}
			break;

		case video::EVT_TANGENTS:
			{
				if (vtxcnt != 0)
					bufferbox.reset(((video::S3DVertexTangents*)v)[0].Pos * scale);

				for ( i=0; i<vtxcnt; ++i)
				{
					((video::S3DVertexTangents*)v)[i].Pos *= scale;
					bufferbox.addInternalPoint(((video::S3DVertexTangents*)v)[i].Pos);
				}
			}
			break;
		}

		buffer->setBoundingBox( bufferbox );

		if (b == 0)
			meshbox.reset(buffer->getBoundingBox());
		else
			meshbox.addInternalBox(buffer->getBoundingBox());
	}

	mesh->setBoundingBox( meshbox );
}



//! Clones a static IMesh into a modifyable SMesh.
SMesh* CMeshManipulator::createMeshCopy(scene::IMesh* mesh) const
{
	if (!mesh)
		return 0;

	SMesh* clone = new SMesh();

	const u32 meshBufferCount = mesh->getMeshBufferCount();

	for ( u32 b=0; b<meshBufferCount; ++b)
	{
		const u32 vtxCnt = mesh->getMeshBuffer(b)->getVertexCount();
		const u32 idxCnt = mesh->getMeshBuffer(b)->getIndexCount();
		const u16* idx = mesh->getMeshBuffer(b)->getIndices();
		u32 i;

		switch(mesh->getMeshBuffer(b)->getVertexType())
		{
		case video::EVT_STANDARD:
			{
				SMeshBuffer* buffer = new SMeshBuffer();
				buffer->Material = mesh->getMeshBuffer(b)->getMaterial();

				video::S3DVertex* v =
					(video::S3DVertex*)mesh->getMeshBuffer(b)->getVertices();

				buffer->Vertices.reallocate(vtxCnt);
				for (i=0; i<vtxCnt; ++i)
					buffer->Vertices.push_back(v[i]);

				buffer->Indices.reallocate(idxCnt);
				for (i=0; i<idxCnt; ++i)
					buffer->Indices.push_back(idx[i]);

				clone->addMeshBuffer(buffer);
				buffer->drop();
			}
			break;
		case video::EVT_2TCOORDS:
			{
				SMeshBufferLightMap* buffer = new SMeshBufferLightMap();
				buffer->Material = mesh->getMeshBuffer(b)->getMaterial();

				video::S3DVertex2TCoords* v =
					(video::S3DVertex2TCoords*)mesh->getMeshBuffer(b)->getVertices();

				buffer->Vertices.reallocate(vtxCnt);
				for (i=0; i<vtxCnt; ++i)
					buffer->Vertices.push_back(v[i]);

				buffer->Indices.reallocate(idxCnt);
				for (i=0; i<idxCnt; ++i)
					buffer->Indices.push_back(idx[i]);

				clone->addMeshBuffer(buffer);
				buffer->drop();
			}
			break;
		case video::EVT_TANGENTS:
			{
				SMeshBufferTangents* buffer = new SMeshBufferTangents();
				buffer->Material = mesh->getMeshBuffer(b)->getMaterial();

				video::S3DVertexTangents* v =
					(video::S3DVertexTangents*)mesh->getMeshBuffer(b)->getVertices();

				buffer->Vertices.reallocate(vtxCnt);
				for (i=0; i<vtxCnt; ++i)
					buffer->Vertices.push_back(v[i]);

				buffer->Indices.reallocate(idxCnt);
				for (i=0; i<idxCnt; ++i)
					buffer->Indices.push_back(idx[i]);

				clone->addMeshBuffer(buffer);
				buffer->drop();
			}
			break;
		}// end switch

	}// end for all mesh buffers

	clone->BoundingBox = mesh->getBoundingBox();
	return clone;
}



//! Creates a planar texture mapping on the mesh
//! \param mesh: Mesh on which the operation is performed.
//! \param resolution: resolution of the planar mapping. This is the value
//! specifying which is the releation between world space and
//! texture coordinate space.
void CMeshManipulator::makePlanarTextureMapping(scene::IMesh* mesh, f32 resolution=0.01f) const
{
	if (!mesh)
		return;

	const u32 bcount = mesh->getMeshBufferCount();
	for ( u32 b=0; b<bcount; ++b)
	{
		IMeshBuffer* buffer = mesh->getMeshBuffer(b);
		u32 vtxcnt = buffer->getVertexCount();
		u32 idxcnt = buffer->getIndexCount();
		u16* idx = buffer->getIndices();

		switch(buffer->getVertexType())
		{
		case video::EVT_STANDARD:
			{
				video::S3DVertex* v = (video::S3DVertex*)buffer->getVertices();
				makePlanarMappingT(v, vtxcnt, idx, idxcnt, resolution);
			}
			break;
		case video::EVT_2TCOORDS:
			{
				video::S3DVertex2TCoords* v = (video::S3DVertex2TCoords*)buffer->getVertices();
				makePlanarMappingT(v, vtxcnt, idx, idxcnt, resolution);
			}
			break;
		case video::EVT_TANGENTS:
			{
				video::S3DVertexTangents* v = (video::S3DVertexTangents*)buffer->getVertices();
				makePlanarMappingT(v, vtxcnt, idx, idxcnt, resolution);
			}
			break;
		}
	}
}



//! Creates a copy of the mesh, which will only consist of unique primitives
IMesh* CMeshManipulator::createMeshUniquePrimitives(IMesh* mesh) const
{
	if (!mesh)
		return 0;

	SMesh* clone = new SMesh();

	const u32 meshBufferCount = mesh->getMeshBufferCount();

	for ( u32 b=0; b<meshBufferCount; ++b)
	{
		const s32 idxCnt = mesh->getMeshBuffer(b)->getIndexCount();
		const u16* idx = mesh->getMeshBuffer(b)->getIndices();

		switch(mesh->getMeshBuffer(b)->getVertexType())
		{
		case video::EVT_STANDARD:
			{
				SMeshBuffer* buffer = new SMeshBuffer();
				buffer->Material = mesh->getMeshBuffer(b)->getMaterial();

				video::S3DVertex* v =
					(video::S3DVertex*)mesh->getMeshBuffer(b)->getVertices();

				buffer->Vertices.reallocate(idxCnt);
				buffer->Indices.reallocate(idxCnt);
				for (s32 i=0; i<idxCnt; i += 3)
				{
					buffer->Vertices.push_back( v[idx[i + 0 ]] );
					buffer->Vertices.push_back( v[idx[i + 1 ]] );
					buffer->Vertices.push_back( v[idx[i + 2 ]] );

					buffer->Indices.push_back( i + 0 );
					buffer->Indices.push_back( i + 1 );
					buffer->Indices.push_back( i + 2 );
				}

				clone->addMeshBuffer(buffer);
				buffer->drop();
			}
			break;
		case video::EVT_2TCOORDS:
			{
				SMeshBufferLightMap* buffer = new SMeshBufferLightMap();
				buffer->Material = mesh->getMeshBuffer(b)->getMaterial();

				video::S3DVertex2TCoords* v =
					(video::S3DVertex2TCoords*)mesh->getMeshBuffer(b)->getVertices();

				buffer->Vertices.reallocate(idxCnt);
				buffer->Indices.reallocate(idxCnt);
				for (s32 i=0; i<idxCnt; i += 3)
				{
					buffer->Vertices.push_back( v[idx[i + 0 ]] );
					buffer->Vertices.push_back( v[idx[i + 1 ]] );
					buffer->Vertices.push_back( v[idx[i + 2 ]] );

					buffer->Indices.push_back( i + 0 );
					buffer->Indices.push_back( i + 1 );
					buffer->Indices.push_back( i + 2 );
				}
				clone->addMeshBuffer(buffer);
				buffer->drop();
			}
			break;
		case video::EVT_TANGENTS:
			{
				SMeshBufferTangents* buffer = new SMeshBufferTangents();
				buffer->Material = mesh->getMeshBuffer(b)->getMaterial();

				video::S3DVertexTangents* v =
					(video::S3DVertexTangents*)mesh->getMeshBuffer(b)->getVertices();

				buffer->Vertices.reallocate(idxCnt);
				buffer->Indices.reallocate(idxCnt);
				for (s32 i=0; i<idxCnt; i += 3)
				{
					buffer->Vertices.push_back( v[idx[i + 0 ]] );
					buffer->Vertices.push_back( v[idx[i + 1 ]] );
					buffer->Vertices.push_back( v[idx[i + 2 ]] );

					buffer->Indices.push_back( i + 0 );
					buffer->Indices.push_back( i + 1 );
					buffer->Indices.push_back( i + 2 );
				}

				clone->addMeshBuffer(buffer);
				buffer->drop();
			}
			break;
		}// end switch

	}// end for all mesh buffers

	clone->BoundingBox = mesh->getBoundingBox();
	return clone;
}

//! Creates a copy of a mesh, which will have identical vertices welded together
IMesh* CMeshManipulator::createMeshWelded(IMesh *mesh, f32 tolerance) const
{
	SMesh* clone = new SMesh();
	clone->BoundingBox = mesh->getBoundingBox();

	core::array<u16> redirects;

	for (u32 b=0; b<mesh->getMeshBufferCount(); ++b)
	{
		// reset redirect list
		redirects.set_used(mesh->getMeshBuffer(b)->getVertexCount());

		u16* indices = 0;
		u32 indexCount = 0;
		core::array<u16>* outIdx = 0;

		switch(mesh->getMeshBuffer(b)->getVertexType())
		{
		case video::EVT_STANDARD:
		{
			SMeshBuffer* buffer = new SMeshBuffer();
			buffer->BoundingBox = mesh->getMeshBuffer(b)->getBoundingBox();
			buffer->Material = mesh->getMeshBuffer(b)->getMaterial();
			clone->addMeshBuffer(buffer);
			buffer->drop();

			video::S3DVertex* v =
					(video::S3DVertex*)mesh->getMeshBuffer(b)->getVertices();

			u32 vertexCount = mesh->getMeshBuffer(b)->getVertexCount();

			indices = mesh->getMeshBuffer(b)->getIndices();
			indexCount = mesh->getMeshBuffer(b)->getIndexCount();
			outIdx = &buffer->Indices;

			buffer->Vertices.reallocate(vertexCount);

			for (u32 i=0; i < vertexCount; ++i)
			{
				bool found = false;
				for (u32 j=0; j < i; ++j)
				{
					if ( v[i].Pos.equals( v[j].Pos, tolerance) &&
						 v[i].Normal.equals( v[j].Normal, tolerance) &&
						 v[i].TCoords.equals( v[j].TCoords ) &&
						(v[i].Color == v[j].Color) )
					{
						redirects[i] = redirects[j];
						found = true;
						break;
					}
				}
				if (!found)
				{
					redirects[i] = buffer->Vertices.size();
					buffer->Vertices.push_back(v[i]);
				}
			}
			
			break;
		}
		case video::EVT_2TCOORDS:
		{
			SMeshBufferLightMap* buffer = new SMeshBufferLightMap();
			buffer->BoundingBox = mesh->getMeshBuffer(b)->getBoundingBox();
			buffer->Material = mesh->getMeshBuffer(b)->getMaterial();
			clone->addMeshBuffer(buffer);
			buffer->drop();

			video::S3DVertex2TCoords* v =
					(video::S3DVertex2TCoords*)mesh->getMeshBuffer(b)->getVertices();

			u32 vertexCount = mesh->getMeshBuffer(b)->getVertexCount();

			indices = mesh->getMeshBuffer(b)->getIndices();
			indexCount = mesh->getMeshBuffer(b)->getIndexCount();
			outIdx = &buffer->Indices;

			buffer->Vertices.reallocate(vertexCount);

			for (u32 i=0; i < vertexCount; ++i)
			{
				bool found = false;
				for (u32 j=0; j < i; ++j)
				{
					if ( v[i].Pos.equals( v[j].Pos, tolerance) &&
						 v[i].Normal.equals( v[j].Normal, tolerance) &&
						 v[i].TCoords.equals( v[j].TCoords ) &&
						 v[i].TCoords2.equals( v[j].TCoords2 ) &&
						(v[i].Color == v[j].Color) )
					{
						redirects[i] = redirects[j];
						found = true;
						break;
					}
				}
				if (!found)
				{
					redirects[i] = buffer->Vertices.size();
					buffer->Vertices.push_back(v[i]);
				}
			}
			break;
		}
		case video::EVT_TANGENTS:
		{
			SMeshBufferTangents* buffer = new SMeshBufferTangents();
			buffer->BoundingBox = mesh->getMeshBuffer(b)->getBoundingBox();
			buffer->Material = mesh->getMeshBuffer(b)->getMaterial();
			clone->addMeshBuffer(buffer);
			buffer->drop();

			video::S3DVertexTangents* v =
					(video::S3DVertexTangents*)mesh->getMeshBuffer(b)->getVertices();

			u32 vertexCount = mesh->getMeshBuffer(b)->getVertexCount();

			indices = mesh->getMeshBuffer(b)->getIndices();
			indexCount = mesh->getMeshBuffer(b)->getIndexCount();
			outIdx = &buffer->Indices;

			buffer->Vertices.reallocate(vertexCount);

			for (u32 i=0; i < vertexCount; ++i)
			{
				bool found = false;
				for (u32 j=0; j < i; ++j)
				{
					if ( v[i].Pos.equals( v[j].Pos, tolerance) &&
						 v[i].Normal.equals( v[j].Normal, tolerance) &&
						 v[i].TCoords.equals( v[j].TCoords ) &&
						 v[i].Tangent.equals( v[j].Tangent, tolerance ) &&
						 v[i].Binormal.equals( v[j].Binormal, tolerance ) &&
						(v[i].Color == v[j].Color) )
					{
						redirects[i] = redirects[j];
						found = true;
						break;
					}
				}
				if (!found)
				{
					redirects[i] = buffer->Vertices.size();
					buffer->Vertices.push_back(v[i]);
				}
			}
			break;
		}
		default:
			os::Printer::log("Cannot create welded mesh, vertex type unsupported", ELL_ERROR);
			break;
		}

		// write the buffer's index list
		core::array<u16> &Indices = *outIdx;

		Indices.set_used(indexCount);
		for (u32 i=0; i<indexCount; ++i)
		{
			Indices[i] = redirects[ indices[i] ];
		}
	}
	return clone;
}


//! Creates a copy of the mesh, which will only consist of S3DVertexTangents vertices.
IMesh* CMeshManipulator::createMeshWithTangents(IMesh* mesh) const
{
	if (!mesh)
		return 0;

	// copy mesh and fill data into SMeshBufferTangents

	SMesh* clone = new SMesh();
	const u32 meshBufferCount = mesh->getMeshBufferCount();
	u32 b;

	for (b=0; b<meshBufferCount; ++b)
	{
		const s32 idxCnt = mesh->getMeshBuffer(b)->getIndexCount();
		const u16* idx = mesh->getMeshBuffer(b)->getIndices();

		SMeshBufferTangents* buffer = new SMeshBufferTangents();
		buffer->Material = mesh->getMeshBuffer(b)->getMaterial();

		// copy vertices

		buffer->Vertices.reallocate(idxCnt);
		switch(mesh->getMeshBuffer(b)->getVertexType())
		{
		case video::EVT_STANDARD:
			{
				video::S3DVertex* v =
					(video::S3DVertex*)mesh->getMeshBuffer(b)->getVertices();

				for (s32 i=0; i<idxCnt; ++i)
					buffer->Vertices.push_back(
						video::S3DVertexTangents(
							v[idx[i]].Pos, v[idx[i]].Color, v[idx[i]].TCoords));
			}
			break;
		case video::EVT_2TCOORDS:
			{
				video::S3DVertex2TCoords* v =
					(video::S3DVertex2TCoords*)mesh->getMeshBuffer(b)->getVertices();

				for (s32 i=0; i<idxCnt; ++i)
					buffer->Vertices.push_back(video::S3DVertexTangents(
						v[idx[i]].Pos, v[idx[i]].Color, v[idx[i]].TCoords));
			}
			break;
		case video::EVT_TANGENTS:
			{
				video::S3DVertexTangents* v =
					(video::S3DVertexTangents*)mesh->getMeshBuffer(b)->getVertices();

				for (s32 i=0; i<idxCnt; ++i)
					buffer->Vertices.push_back(v[idx[i]]);
			}
			break;
		}

		// create new indices

		buffer->Indices.set_used(idxCnt);
		for (s32 i=0; i<idxCnt; ++i)
			buffer->Indices[i] = i;

		// add new buffer
		clone->addMeshBuffer(buffer);
		buffer->drop();
	}

	clone->BoundingBox = mesh->getBoundingBox();

	// now calculate tangents
	for (b=0; b<meshBufferCount; ++b)
	{
		const s32 idxCnt = clone->getMeshBuffer(b)->getIndexCount();

		u16* idx = clone->getMeshBuffer(b)->getIndices();
		video::S3DVertexTangents* v =
			(video::S3DVertexTangents*)clone->getMeshBuffer(b)->getVertices();

		for (s32 i=0; i<idxCnt; i+=3)
		{
			calculateTangents(
				v[idx[i+0]].Normal,
				v[idx[i+0]].Tangent,
				v[idx[i+0]].Binormal,
				v[idx[i+0]].Pos,
				v[idx[i+1]].Pos,
				v[idx[i+2]].Pos,
				v[idx[i+0]].TCoords,
				v[idx[i+1]].TCoords,
				v[idx[i+2]].TCoords);

			calculateTangents(
				v[idx[i+1]].Normal,
				v[idx[i+1]].Tangent,
				v[idx[i+1]].Binormal,
				v[idx[i+1]].Pos,
				v[idx[i+2]].Pos,
				v[idx[i+0]].Pos,
				v[idx[i+1]].TCoords,
				v[idx[i+2]].TCoords,
				v[idx[i+0]].TCoords);

			calculateTangents(
				v[idx[i+2]].Normal,
				v[idx[i+2]].Tangent,
				v[idx[i+2]].Binormal,
				v[idx[i+2]].Pos,
				v[idx[i+0]].Pos,
				v[idx[i+1]].Pos,
				v[idx[i+2]].TCoords,
				v[idx[i+0]].TCoords,
				v[idx[i+1]].TCoords);
		}
	}

	return clone;
}



//! Creates a copy of the mesh, which will only consist of S3DVertex2TCoords vertices.
IMesh* CMeshManipulator::createMeshWith2TCoords(IMesh* mesh) const
{
	if (!mesh)
		return 0;

	// copy mesh and fill data into SMeshBufferLightMap

	SMesh* clone = new SMesh();
	const u32 meshBufferCount = mesh->getMeshBufferCount();
	u32 b;

	for (b=0; b<meshBufferCount; ++b)
	{
		const s32 idxCnt = mesh->getMeshBuffer(b)->getIndexCount();
		const u16* idx = mesh->getMeshBuffer(b)->getIndices();

		SMeshBufferLightMap* buffer = new SMeshBufferLightMap();
		buffer->Material = mesh->getMeshBuffer(b)->getMaterial();

		// copy vertices

		buffer->Vertices.reallocate(idxCnt);
		switch(mesh->getMeshBuffer(b)->getVertexType())
		{
		case video::EVT_STANDARD:
			{
				video::S3DVertex* v =
					(video::S3DVertex*)mesh->getMeshBuffer(b)->getVertices();

				for (s32 i=0; i<idxCnt; ++i)
					buffer->Vertices.push_back(
						video::S3DVertex2TCoords(
							v[idx[i]].Pos, v[idx[i]].Color, v[idx[i]].TCoords, v[idx[i]].TCoords));
			}
			break;
		case video::EVT_2TCOORDS:
			{
				video::S3DVertex2TCoords* v =
					(video::S3DVertex2TCoords*)mesh->getMeshBuffer(b)->getVertices();

				for (s32 i=0; i<idxCnt; ++i)
					buffer->Vertices.push_back(v[idx[i]]);
			}
			break;
		case video::EVT_TANGENTS:
			{
				video::S3DVertexTangents* v =
					(video::S3DVertexTangents*)mesh->getMeshBuffer(b)->getVertices();

				for (s32 i=0; i<idxCnt; ++i)
					buffer->Vertices.push_back(video::S3DVertex2TCoords(
						v[idx[i]].Pos, v[idx[i]].Color, v[idx[i]].TCoords, v[idx[i]].TCoords));
			}
			break;
		}

		// create new indices

		buffer->Indices.set_used(idxCnt);
		for (s32 i=0; i<idxCnt; ++i)
			buffer->Indices[i] = i;

		// add new buffer
		clone->addMeshBuffer(buffer);
		buffer->drop();
	}

	clone->BoundingBox = mesh->getBoundingBox();

	return clone;
}



void CMeshManipulator::calculateTangents(
	core::vector3df& normal,
	core::vector3df& tangent,
	core::vector3df& binormal,
	const core::vector3df& vt1, const core::vector3df& vt2, const core::vector3df& vt3, // vertices
	const core::vector2df& tc1, const core::vector2df& tc2, const core::vector2df& tc3) // texture coords
{
	// choose one of them:
	//#define USE_NVIDIA_GLH_VERSION // use version used by nvidia in glh headers
	#define USE_IRR_VERSION

#ifdef USE_IRR_VERSION

	core::vector3df v1 = vt1 - vt2;
	core::vector3df v2 = vt3 - vt1;
	normal = v2.crossProduct(v1);
	normal.normalize();

	// binormal

	f32 deltaX1 = tc1.X - tc2.X;
	f32 deltaX2 = tc3.X - tc1.X;
	binormal = (v1 * deltaX2) - (v2 * deltaX1);
	binormal.normalize();

	// tangent

	f32 deltaY1 = tc1.Y - tc2.Y;
	f32 deltaY2 = tc3.Y - tc1.Y;
	tangent = (v1 * deltaY2) - (v2 * deltaY1);
	tangent.normalize();

	// adjust

	core::vector3df txb = tangent.crossProduct(binormal);
	if (txb.dotProduct(normal) < 0.0f)
	{
		tangent *= -1.0f;
		binormal *= -1.0f;
	}

#endif // USE_IRR_VERSION

#ifdef USE_NVIDIA_GLH_VERSION

	tangent.set(0,0,0);
	binormal.set(0,0,0);

	core::vector3df v1(vt2.X - vt1.X, tc2.X - tc1.X, tc2.Y - tc1.Y);
	core::vector3df v2(vt3.X - vt1.X, tc3.X - tc1.X, tc3.Y - tc1.Y);

	core::vector3df txb = v1.crossProduct(v2);
	if ( !core::iszero ( txb.X ) )
	{
		tangent.X  = -txb.Y / txb.X;
		binormal.X = -txb.Z / txb.X;
	}

	v1.X = vt2.Y - vt1.Y;
	v2.X = vt3.Y - vt1.Y;
	txb = v1.crossProduct(v2);

	if ( !core::iszero ( txb.X ) )
	{
		tangent.Y  = -txb.Y / txb.X;
		binormal.Y = -txb.Z / txb.X;
	}

	v1.X = vt2.Z - vt1.Z;
	v2.X = vt3.Z - vt1.Z;
	txb = v1.crossProduct(v2);

	if ( !core::iszero ( txb.X ) )
	{
		tangent.Z  = -txb.Y / txb.X;
		binormal.Z = -txb.Z / txb.X;
	}

	tangent.normalize();
	binormal.normalize();

	normal = tangent.crossProduct(binormal);
	normal.normalize();

	binormal = tangent.crossProduct(normal);
	binormal.normalize();

	core::plane3d<f32> pl(vt1, vt2, vt3);

	if(normal.dotProduct(pl.Normal) < 0.0f )
		normal *= -1.0f;

#endif // USE_NVIDIA_GLH_VERSION
}



//! Returns amount of polygons in mesh.
s32 CMeshManipulator::getPolyCount(scene::IMesh* mesh) const
{
	if (!mesh)
		return 0;

	s32 trianglecount = 0;

	for (u32 g=0; g<mesh->getMeshBufferCount(); ++g)
		trianglecount += mesh->getMeshBuffer(g)->getIndexCount() / 3;

	return trianglecount;
}



//! Returns amount of polygons in mesh.
s32 CMeshManipulator::getPolyCount(scene::IAnimatedMesh* mesh) const
{
	if (mesh && mesh->getFrameCount() != 0)
		return getPolyCount(mesh->getMesh(0));

	return 0;
}

//! create a new AnimatedMesh and adds the mesh to it
IAnimatedMesh * CMeshManipulator::createAnimatedMesh(scene::IMesh* mesh, scene::E_ANIMATED_MESH_TYPE type) const
{
	return new SAnimatedMesh(mesh, type);
}


} // end namespace scene
} // end namespace irr

