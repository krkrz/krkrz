// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __E_MATERIAL_FLAGS_H_INCLUDED__
#define __E_MATERIAL_FLAGS_H_INCLUDED__

namespace irr
{
namespace video
{

	//! Material flags
	enum E_MATERIAL_FLAG
	{
		//! Draw as wireframe or filled triangles? Default: false
		EMF_WIREFRAME = 0,

		//! Draw as point cloud or filled triangles? Default: false
		EMF_POINTCLOUD,

		//! Flat or Gouraud shading? Default: true
		EMF_GOURAUD_SHADING,

		//! Will this material be lighted? Default: true
		EMF_LIGHTING,

		//! Is the ZBuffer enabled? Default: true
		EMF_ZBUFFER,

		//! May be written to the zbuffer or is it readonly. Default: true
		//! This flag is ignored, if the material type is a transparent type.
		EMF_ZWRITE_ENABLE,

		//! Is backfaceculling enabled? Default: true
		EMF_BACK_FACE_CULLING,

		//! Is bilinear filtering enabled? Default: true
		EMF_BILINEAR_FILTER,

		//! Is trilinear filtering enabled? Default: false
		//! If the trilinear filter flag is enabled,
		//! the bilinear filtering flag is ignored.
		EMF_TRILINEAR_FILTER,

		//! Is anisotropic filtering? Default: false
		//! In Irrlicht you can use anisotropic texture filtering in
		//! conjunction with bilinear or trilinear texture filtering
		//! to improve rendering results. Primitives will look less
		//! blurry with this flag switched on.
		EMF_ANISOTROPIC_FILTER,

		//! Is fog enabled? Default: false
		EMF_FOG_ENABLE,

		//! Normalizes normals.You can enable this if you need
		//! to scale a dynamic lighted model. Usually, its normals will get scaled
		//! too then and it will get darker. If you enable the EMF_NORMALIZE_NORMALS flag,
		//! the normals will be normalized again, and the model will look as bright as it should.
		EMF_NORMALIZE_NORMALS,

		//! Access to all layers texture wrap settings. Overwrites separate layer settings.
		EMF_TEXTURE_WRAP,

		//! This is not a flag, but a value indicating how much flags there are.
		EMF_MATERIAL_FLAG_COUNT
	};

} // end namespace video
} // end namespace irr


#endif // __E_MATERIAL_FLAGS_H_INCLUDED__

