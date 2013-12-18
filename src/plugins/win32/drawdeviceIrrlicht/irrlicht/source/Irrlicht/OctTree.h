// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_OCT_TREE_H_INCLUDED__
#define __C_OCT_TREE_H_INCLUDED__

#include "SViewFrustum.h"
#include "S3DVertex.h"
#include "aabbox3d.h"
#include "irrArray.h"
#include "irrString.h"

namespace irr
{

//! template octtree. T must be a vertex type which has a member
//! called .Pos, which is a core::vertex3df position.
template <class T>
class OctTree
{
public:

	u32 nodeCount;

	struct SMeshChunk
	{
		core::array<T> Vertices;
		core::array<u16> Indices;
		s32 MaterialId;
	};

	struct SIndexChunk
	{
		core::array<u16> Indices;
		s32 MaterialId;
	};

	struct SIndexData
	{
		u16* Indices;
		s32 CurrentSize;
		s32 MaxSize;
	};



	//! constructor
	OctTree(const core::array<SMeshChunk>& meshes, s32 minimalPolysPerNode=128)
	{
		nodeCount = 0;

		IndexDataCount = meshes.size();
		IndexData = new SIndexData[IndexDataCount];

		// construct array of all indices

		core::array<SIndexChunk>* indexChunks = new core::array<SIndexChunk>;
		SIndexChunk ic;

		for (u32 i=0; i<meshes.size(); ++i)
		{
			IndexData[i].CurrentSize = 0;
			IndexData[i].MaxSize = meshes[i].Indices.size();
			IndexData[i].Indices = new u16[IndexData[i].MaxSize];

			ic.MaterialId = meshes[i].MaterialId;
			indexChunks->push_back(ic);

			SIndexChunk& tic = (*indexChunks)[i];

			for (u32 t=0; t<meshes[i].Indices.size(); ++t)
				tic.Indices.push_back(meshes[i].Indices[t]);
		}

		// create tree

		Root = new OctTreeNode(nodeCount, 0, meshes, indexChunks, minimalPolysPerNode);
	}

	//! returns all ids of polygons partially or fully enclosed 
	//! by this bounding box.
	void calculatePolys(const core::aabbox3d<f32>& box)
	{
		for (u32 i=0; i<IndexDataCount; ++i)
			IndexData[i].CurrentSize = 0;

		Root->getPolys(box, IndexData, 0);
	}

	//! returns all ids of polygons partially or fully enclosed 
	//! by a view frustum.
	void calculatePolys(const scene::SViewFrustum& frustum)
	{
		for (u32 i=0; i<IndexDataCount; ++i)
			IndexData[i].CurrentSize = 0;

		Root->getPolys(frustum, IndexData, 0);
	}


	SIndexData* getIndexData()
	{
		return IndexData;
	}

	u32 getIndexDataCount()
	{
		return IndexDataCount;
	}

	// for debug purposes only, renders the bounding boxes of the tree
	void renderBoundingBoxes(const core::aabbox3d<f32>& box, 
		core::array< core::aabbox3d<f32> >&outBoxes)
	{
		Root->renderBoundingBoxes(box, outBoxes);
	}

	//! destructor
	~OctTree()
	{
		for (u32 i=0; i<IndexDataCount; ++i)
			delete [] IndexData[i].Indices;

		delete [] IndexData;
		delete Root;
	}

private:

	// private inner class
	class OctTreeNode
	{
	public:

		// constructor
		OctTreeNode(u32& nodeCount, u32 currentdepth,
			const core::array<SMeshChunk>& allmeshdata,
			core::array<SIndexChunk>* indices,
			s32 minimalPolysPerNode) : IndexData(0),
			Depth(currentdepth+1)
		{
			++nodeCount;
			
			u32 i; // new ISO for scoping problem with different compilers

			for (i=0; i<8; ++i)
				Children[i] = 0;

			if (indices->empty())
			{
				delete indices;
				return;
			}

			bool found = false;

			// find first point for bounding box

			for (i=0; i<indices->size(); ++i)
			{
				if (!(*indices)[i].Indices.empty())
				{
					Box.reset(allmeshdata[i].Vertices[(*indices)[i].Indices[0]].Pos);
					found = true;
					break;
				}
			}

			if (!found)
			{
				delete indices;
				return;
			}

			s32 totalPrimitives = 0;

			// now lets calculate our bounding box
			for (i=0; i<indices->size(); ++i)
			{
				totalPrimitives += (*indices)[i].Indices.size();
				for (u32 j=0; j<(*indices)[i].Indices.size(); ++j)
					Box.addInternalPoint(allmeshdata[i].Vertices[(*indices)[i].Indices[j]].Pos);
			}

			core::vector3df middle = Box.getCenter();
			core::vector3df edges[8];
			Box.getEdges(edges);
			
			// calculate all children
			core::aabbox3d<f32> box;
			core::array<u16> keepIndices;

			if (totalPrimitives > minimalPolysPerNode && !Box.isEmpty())
			for (s32 ch=0; ch<8; ++ch)
			{
				box.reset(middle);
				box.addInternalPoint(edges[ch]);

				// create indices for child
				core::array<SIndexChunk>* cindexChunks = new core::array<SIndexChunk>;

				bool added = false;

				for (i=0; i<allmeshdata.size(); ++i)
				{
					SIndexChunk ic;
					ic.MaterialId = allmeshdata[i].MaterialId;
					cindexChunks->push_back(ic);

					SIndexChunk& tic = (*cindexChunks)[i];

					for (u32 t=0; t<(*indices)[i].Indices.size(); t+=3)
					{
						if (box.isPointInside(allmeshdata[i].Vertices[(*indices)[i].Indices[t]].Pos) &&
							box.isPointInside(allmeshdata[i].Vertices[(*indices)[i].Indices[t+1]].Pos) &&
							box.isPointInside(allmeshdata[i].Vertices[(*indices)[i].Indices[t+2]].Pos))
						{
							tic.Indices.push_back((*indices)[i].Indices[t]);
							tic.Indices.push_back((*indices)[i].Indices[t+1]);
							tic.Indices.push_back((*indices)[i].Indices[t+2]);

							added = true;
						}
						else
						{
							keepIndices.push_back((*indices)[i].Indices[t]);
							keepIndices.push_back((*indices)[i].Indices[t+1]);
							keepIndices.push_back((*indices)[i].Indices[t+2]);
						}
					}
					
					memcpy( (*indices)[i].Indices.pointer(), keepIndices.pointer(), keepIndices.size()*sizeof(u16));
					(*indices)[i].Indices.set_used(keepIndices.size());
					keepIndices.set_used(0);
				}

				if (added)
					Children[ch] = new OctTreeNode(nodeCount, Depth, 
						allmeshdata, cindexChunks, minimalPolysPerNode);
				else
					delete cindexChunks;

			} // end for all possible children

			IndexData = indices;
		}



		// destructor
		~OctTreeNode()
		{
			delete IndexData;

			for (u32 i=0; i<8; ++i)
				delete Children[i];
		}



		// returns all ids of polygons partially or full enclosed 
		// by this bounding box.
		void getPolys(const core::aabbox3d<f32>& box, SIndexData* idxdata, u32 parentTest ) const
		{
			// if not full inside
			if ( parentTest != 2 )
			{
				// partially inside ?
				parentTest = (u32) Box.intersectsWithBox(box);
				if ( 0 == parentTest )
					return;

				// fully inside ?
				parentTest+= Box.isFullInside(box);
			}

			//if (Box.intersectsWithBox(box))
			{
				u32 cnt = IndexData->size();
				u32 i; // new ISO for scoping problem in some compilers
				
				for (i=0; i<cnt; ++i)
				{
					s32 idxcnt = (*IndexData)[i].Indices.size();

					if (idxcnt)
					{
						memcpy(&idxdata[i].Indices[idxdata[i].CurrentSize], 
							&(*IndexData)[i].Indices[0], idxcnt * sizeof(s16));
						idxdata[i].CurrentSize += idxcnt;
					}
				}

				for (i=0; i<8; ++i)
					if (Children[i])
						Children[i]->getPolys(box, idxdata,parentTest);
			}
		}



		// returns all ids of polygons partially or full enclosed 
		// by the view frustum.
		void getPolys(const scene::SViewFrustum& frustum, SIndexData* idxdata,u32 parentTest) const
		{
			s32 i; // new ISO for scoping problem in some compilers
			
			// not fully inside
			//if ( parentTest != 2 )
			{
				core::vector3df edges[8];
				Box.getEdges(edges);

				for (i=0; i<scene::SViewFrustum::VF_PLANE_COUNT; ++i)
				{
					bool boxInFrustum=false;

					for (int j=0; j<8; ++j)
						if (frustum.planes[i].classifyPointRelation(edges[j]) != core::ISREL3D_FRONT)
						{
							boxInFrustum=true;
							break;
						}

					if (!boxInFrustum) // all edges outside
						return;
				}
			}

			s32 cnt = IndexData->size();
			
			for (i=0; i<cnt; ++i)
			{
				s32 idxcnt = (*IndexData)[i].Indices.size();

				if (idxcnt)
				{
					memcpy(&idxdata[i].Indices[idxdata[i].CurrentSize], 
						&(*IndexData)[i].Indices[0], idxcnt * sizeof(s16));
					idxdata[i].CurrentSize += idxcnt;
				}
			}

			for (i=0; i<8; ++i)
				if (Children[i])
					Children[i]->getPolys(frustum, idxdata,parentTest);
		}



		void renderBoundingBoxes(const core::aabbox3d<f32>& box,
			core::array< core::aabbox3d<f32> >&outBoxes)
		{
			if (Box.intersectsWithBox(box))
			{
				outBoxes.push_back(Box);
				
				for (u32 i=0; i<8; ++i)
					if (Children[i])
						Children[i]->renderBoundingBoxes(box, outBoxes);
			}
		}

	private:

		core::aabbox3df Box;
		core::array<SIndexChunk>* IndexData;
		OctTreeNode* Children[8];
		u32 Depth;
	};

	OctTreeNode* Root;
	SIndexData* IndexData;
	u32 IndexDataCount;
};

} // end namespace

#endif

