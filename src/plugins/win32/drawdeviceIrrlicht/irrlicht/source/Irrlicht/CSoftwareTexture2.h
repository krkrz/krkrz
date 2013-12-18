// Copyright (C) 2002-2008 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_SOFTWARE_2_TEXTURE_H_INCLUDED__
#define __C_SOFTWARE_2_TEXTURE_H_INCLUDED__

#include "SoftwareDriver2_compile_config.h"

#include "ITexture.h"
#include "CImage.h"

namespace irr
{
namespace video
{

/*!
	interface for a Video Driver dependent Texture.
*/
class CSoftwareTexture2 : public ITexture
{
public:

	//! constructor
	CSoftwareTexture2(IImage* surface, const char* name, bool generateMipLevels, bool isRenderTarget=false);

	//! destructor
	virtual ~CSoftwareTexture2();

	//! lock function
	virtual void* lock()
	{
		return MipMap[MipMapLOD]->lock();
	}

	//! unlock function
	virtual void unlock()
	{
		MipMap[MipMapLOD]->unlock();
	}

	//! Returns original size of the texture.
	virtual const core::dimension2d<s32>& getOriginalSize() const
	{
		//return MipMap[0]->getDimension();
		return OrigSize;
	}

	//! Returns the size of the largest mipmap.
	const core::dimension2d<s32>& getMaxSize() const
	{
		return MipMap[0]->getDimension();
	}

	//! Returns (=size) of the texture.
	virtual const core::dimension2d<s32>& getSize() const
	{
		return MipMap[MipMapLOD]->getDimension();
	}

	//! returns unoptimized surface
	virtual CImage* getImage() const
	{
		return MipMap[0];
	}

	//! returns texture surface
	virtual CImage* getTexture() const
	{
		return MipMap[MipMapLOD];
	}


	//! returns driver type of texture (=the driver, who created the texture)
	virtual E_DRIVER_TYPE getDriverType() const
	{
		return EDT_BURNINGSVIDEO;
	}

	//! returns color format of texture
	virtual ECOLOR_FORMAT getColorFormat() const
	{
		return ECF_SOFTWARE2;
	}

	//! returns pitch of texture (in bytes)
	virtual u32 getPitch() const
	{
		return MipMap[MipMapLOD]->getPitch();
	}

	//! Regenerates the mip map levels of the texture. Useful after locking and 
	//! modifying the texture
	virtual void regenerateMipMapLevels();

	//! Select a Mipmap Level
	virtual void setCurrentMipMapLOD ( s32 lod )
	{
		if ( HasMipMaps )
			MipMapLOD = lod;
	}
	
	//! support mipmaps
	virtual bool hasMipMaps() const
	{
		return HasMipMaps;
	}

	//! is a render target
	virtual bool isRenderTarget() const
	{
		return IsRenderTarget;
	}

private:

	//! returns the size of a texture which would be the optimize size for rendering it
	inline s32 getTextureSizeFromSurfaceSize(s32 size) const;

	core::dimension2d<s32> OrigSize;

	CImage * MipMap[SOFTWARE_DRIVER_2_MIPMAPPING_MAX];

	s32 MipMapLOD;
	bool HasMipMaps, IsRenderTarget;
};


} // end namespace video
} // end namespace irr

#endif

