// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_DIRECTX9_TEXTURE_H_INCLUDED__
#define __C_DIRECTX9_TEXTURE_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_DIRECT3D_9_

#include "ITexture.h"
#include "IImage.h"
#include <d3d9.h>

namespace irr
{
namespace video
{

class CD3D9Driver;
/*!
	interface for a Video Driver dependent Texture.
*/
class CD3D9Texture : public ITexture
{
public:

	//! constructor
	CD3D9Texture(IImage* image, CD3D9Driver* driver,
		u32 flags, const char* name);

	//! rendertarget constructor
	CD3D9Texture(CD3D9Driver* driver, core::dimension2d<s32> size, const char* name);

	//! destructor
	virtual ~CD3D9Texture();

	//! lock function
	virtual void* lock();

	//! unlock function
	virtual void unlock();

	//! Returns original size of the texture.
	virtual const core::dimension2d<s32>& getOriginalSize() const;

	//! Returns (=size) of the texture.
	virtual const core::dimension2d<s32>& getSize() const;

	//! returns driver type of texture (=the driver, who created the texture)
	virtual E_DRIVER_TYPE getDriverType() const;

	//! returns color format of texture
	virtual ECOLOR_FORMAT getColorFormat() const;

	//! returns pitch of texture (in bytes)
	virtual u32 getPitch() const;

	//! returns the DIRECT3D9 Texture
	IDirect3DTexture9* getDX9Texture() const;

	//! returns if texture has mipmap levels
	bool hasMipMaps() const;

	//! Regenerates the mip map levels of the texture. Useful after locking and
	//! modifying the texture
	virtual void regenerateMipMapLevels();

	//! returns if it is a render target
	virtual bool isRenderTarget() const;

	//! Returns pointer to the render target surface
	IDirect3DSurface9* getRenderTargetSurface();

private:

	void createRenderTarget();

	//! returns the size of a texture which would be the optimize size for rendering it
	inline s32 getTextureSizeFromImageSize(s32 size) const;

	//! creates the hardware texture
	bool createTexture(u32 flags);

	//! copies the image to the texture
	bool copyTexture();

	//! Get D3D color format from Irrlicht color format.
	D3DFORMAT getD3DFormatFromColorFormat(ECOLOR_FORMAT format) const;

	//! Get Irrlicht color format from D3D color format.
	ECOLOR_FORMAT getColorFormatFromD3DFormat(D3DFORMAT format);

	//! Helper function for mipmap generation.
	bool createMipMaps(u32 level=1);

	//! Helper function for mipmap generation.
	void copy16BitMipMap(char* src, char* tgt,
		s32 width, s32 height,  s32 pitchsrc, s32 pitchtgt) const;

	//! Helper function for mipmap generation.
	void copy32BitMipMap(char* src, char* tgt,
		s32 width, s32 height,  s32 pitchsrc, s32 pitchtgt) const;

	IImage* Image;
	IDirect3DDevice9* Device;
	IDirect3DTexture9* Texture;
	IDirect3DSurface9* RTTSurface;
	CD3D9Driver* Driver;
	core::dimension2d<s32> TextureSize;
	core::dimension2d<s32> ImageSize;
	s32 Pitch;
	ECOLOR_FORMAT ColorFormat;

	bool HasMipMaps;
	bool HardwareMipMaps;
	bool IsRenderTarget;
};


} // end namespace video
} // end namespace irr

#endif // _IRR_COMPILE_WITH_DIRECT3D_9_

#endif // __C_DIRECTX9_TEXTURE_H_INCLUDED__

