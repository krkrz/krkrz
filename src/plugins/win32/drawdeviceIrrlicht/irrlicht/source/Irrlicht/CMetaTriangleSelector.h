// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_META_TRIANGLE_SELECTOR_H_INCLUDED__
#define __C_META_TRIANGLE_SELECTOR_H_INCLUDED__

#include "IMetaTriangleSelector.h"
#include "irrArray.h"

namespace irr
{
namespace scene
{

//! Interface for making multiple triangle selectors work as one big selector.
class CMetaTriangleSelector : public IMetaTriangleSelector
{
public:

	//! constructor
	CMetaTriangleSelector();

	//! destructor
	virtual ~CMetaTriangleSelector();

	//! Returns amount of all available triangles in this selector
	virtual s32 getTriangleCount() const;

	//! Gets all triangles.
	virtual void getTriangles(core::triangle3df* triangles, s32 arraySize,
		s32& outTriangleCount, const core::matrix4* transform=0) const;

	//! Gets all triangles which lie within a specific bounding box.
	virtual void getTriangles(core::triangle3df* triangles, s32 arraySize,
		s32& outTriangleCount, const core::aabbox3d<f32>& box, 
		const core::matrix4* transform=0) const;

	//! Gets all triangles which have or may have contact with a 3d line.
	virtual void getTriangles(core::triangle3df* triangles, s32 arraySize,
		s32& outTriangleCount, const core::line3d<f32>& line, 
		const core::matrix4* transform=0) const;

	//! Adds a triangle selector to the collection of triangle selectors
	//! in this metaTriangleSelector.
	virtual void addTriangleSelector(ITriangleSelector* toAdd); 

	//! Removes a specific triangle selector which was added before	from the collection.
	virtual bool removeTriangleSelector(ITriangleSelector* toRemove);

	//! Removes all triangle selectors from the collection.
	virtual void removeAllTriangleSelectors();

private:

	core::array<ITriangleSelector*> TriangleSelectors;
};

} // end namespace scene
} // end namespace irr


#endif

