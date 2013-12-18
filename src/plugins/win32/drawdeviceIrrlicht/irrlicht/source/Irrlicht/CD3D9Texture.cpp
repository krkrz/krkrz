// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_DIRECT3D_9_

#define _IRR_DONT_DO_MEMORY_DEBUGGING_HERE
#include "CD3D9Texture.h"
#include "CD3D9Driver.h"
#include "os.h"

#include <d3dx9tex.h>

#ifndef _IRR_COMPILE_WITH_DIRECT3D_8_
// The D3DXFilterTexture function seems to get linked wrong when
// compiling with both D3D8 and 9, causing it not to work in the D3D9 device.
// So mipmapgeneration is replaced with my own bad generation in d3d 8 when
// compiling with both D3D 8 and 9.
// #define _IRR_USE_D3DXFilterTexture_
#endif // _IRR_COMPILE_WITH_DIRECT3D_8_

#ifdef _IRR_USE_D3DXFilterTexture_
#pragma comment(lib, "d3dx9.lib")
#endif

namespace irr
{
namespace video
{

//! rendertarget constructor
CD3D9Texture::CD3D9Texture(CD3D9Driver* driver, core::dimension2d<s32> size, const char* name)
: ITexture(name), Image(0), Texture(0), RTTSurface(0), Driver(driver),
	TextureSize(size), ImageSize(size), Pitch(0),
	HasMipMaps(false), HardwareMipMaps(false), IsRenderTarget(true)
{
	#ifdef _DEBUG
	setDebugName("CD3D9Texture");
	#endif

	Device=driver->getExposedVideoData().D3D9.D3DDev9;
	if (Device)
		Device->AddRef();

	createRenderTarget();
}


//! constructor
CD3D9Texture::CD3D9Texture(IImage* image, CD3D9Driver* driver,
					   u32 flags, const char* name)
: ITexture(name), Image(image), Texture(0), RTTSurface(0), Driver(driver),
TextureSize(0,0), ImageSize(0,0), Pitch(0),
HasMipMaps(false), HardwareMipMaps(false), IsRenderTarget(false)
{
	#ifdef _DEBUG
	setDebugName("CD3D9Texture");
	#endif

	const bool generateMipLevels = Driver->getTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS);

	Device=driver->getExposedVideoData().D3D9.D3DDev9;
	if (Device)
		Device->AddRef();

	if (Image)
	{
		Image->grab();

		if (createTexture(flags))
		{
			if (copyTexture() && generateMipLevels)
			{
				// create mip maps.
				#ifdef _IRR_USE_D3DXFilterTexture_
					// The D3DXFilterTexture function seems to get linked wrong when
					// compiling with both D3D8 and 9, causing it not to work in the D3D9 device.
					// So mipmapgeneration is replaced with my own bad generation
					HRESULT hr  = D3DXFilterTexture(Texture, NULL, D3DX_DEFAULT, D3DX_DEFAULT);
					if (FAILED(hr))
						os::Printer::log("Could not create direct3d mip map levels.", ELL_WARNING);
					else
						HasMipMaps = true;
				#else
					createMipMaps();
					HasMipMaps = true;
				#endif
			}
		}
		else
			os::Printer::log("Could not create DIRECT3D9 Texture.", ELL_WARNING);
	}
}


//! destructor
CD3D9Texture::~CD3D9Texture()
{
	if (Device)
		Device->Release();

	if (Image)
		Image->drop();

	if (Texture)
		Texture->Release();

	if (RTTSurface)
		RTTSurface->Release();
}


void CD3D9Texture::createRenderTarget()
{
	TextureSize.Width = getTextureSizeFromImageSize(TextureSize.Width);
	TextureSize.Height = getTextureSizeFromImageSize(TextureSize.Height);

	// get backbuffer format to create the render target in the
	// same format

	IDirect3DSurface9* bb;
	D3DFORMAT d3DFormat = D3DFMT_A8R8G8B8;

	if (!FAILED(Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &bb)))
	{
		D3DSURFACE_DESC desc;
		bb->GetDesc(&desc);
		d3DFormat = desc.Format;

		if (d3DFormat == D3DFMT_X8R8G8B8)
			d3DFormat = D3DFMT_A8R8G8B8;

		bb->Release();
	}
	else
	{
		os::Printer::log("Could not create RenderTarget texture: could not get BackBuffer.",
			ELL_WARNING);
		return;
	}

	// create texture
	HRESULT hr;

	hr = Device->CreateTexture(
		TextureSize.Width,
		TextureSize.Height,
		1, // mip map level count, we don't want mipmaps here
		D3DUSAGE_RENDERTARGET,
		d3DFormat,
		D3DPOOL_DEFAULT,
		&Texture,
		NULL);

	// get irrlicht format from D3D format
	ColorFormat = getColorFormatFromD3DFormat(d3DFormat);

	if (FAILED(hr))
		os::Printer::log("Could not create render target texture");
}


bool CD3D9Texture::createMipMaps(u32 level)
{
	if (level==0)
		return true;

	if (HardwareMipMaps && Texture)
	{
		// generate mipmaps in hardware
		Texture->GenerateMipSubLevels();
		return true;
	}
	// os::Printer::log("manual mipmap");

	IDirect3DSurface9* upperSurface = 0;
	IDirect3DSurface9* lowerSurface = 0;

	// get upper level
	HRESULT hr = Texture->GetSurfaceLevel(level-1, &upperSurface);
	if (FAILED(hr) || !upperSurface)
	{
		os::Printer::log("Could not get upper surface level for mip map generation", ELL_WARNING);
		return false;
	}

	// get lower level
	hr = Texture->GetSurfaceLevel(level, &lowerSurface);
	if (FAILED(hr) || !lowerSurface)
	{
		os::Printer::log("Could not get lower surface level for mip map generation", ELL_WARNING);
		upperSurface->Release();
		return false;
	}

	D3DSURFACE_DESC upperDesc, lowerDesc;
	upperSurface->GetDesc(&upperDesc);
	lowerSurface->GetDesc(&lowerDesc);

	D3DLOCKED_RECT upperlr;
	D3DLOCKED_RECT lowerlr;

	// lock upper surface
	if (FAILED(upperSurface->LockRect(&upperlr, NULL, 0)))
	{
		upperSurface->Release();
		lowerSurface->Release();
		os::Printer::log("Could not lock upper texture for mip map generation", ELL_WARNING);
		return false;
	}

	// lock lower surface
	if (FAILED(lowerSurface->LockRect(&lowerlr, NULL, 0)))
	{
		upperSurface->UnlockRect();
		upperSurface->Release();
		lowerSurface->Release();
		os::Printer::log("Could not lock lower texture for mip map generation", ELL_WARNING);
		return false;
	}

	if (upperDesc.Format != lowerDesc.Format)
	{
		os::Printer::log("Cannot copy mip maps with different formats.", ELL_WARNING);
	}
	else
	{
		if ((upperDesc.Format == D3DFMT_A1R5G5B5) || (upperDesc.Format == D3DFMT_R5G6B5))
			copy16BitMipMap((char*)upperlr.pBits, (char*)lowerlr.pBits,
					lowerDesc.Width, lowerDesc.Height,
					upperlr.Pitch, lowerlr.Pitch);
		else
		if (upperDesc.Format == D3DFMT_A8R8G8B8)
			copy32BitMipMap((char*)upperlr.pBits, (char*)lowerlr.pBits,
					lowerDesc.Width, lowerDesc.Height,
					upperlr.Pitch, lowerlr.Pitch);
		else
			os::Printer::log("Unsupported mipmap format, cannot copy.", ELL_WARNING);
	}

	bool result=true;
	// unlock
	if (FAILED(upperSurface->UnlockRect()))
		result=false;
	if (FAILED(lowerSurface->UnlockRect()))
		result=false;

	// release
	upperSurface->Release();
	lowerSurface->Release();

	if (!result || (upperDesc.Width < 3 && upperDesc.Height < 3))
		return result; // stop generating levels

	// generate next level
	return createMipMaps(level+1);
}


//! creates the hardware texture
bool CD3D9Texture::createTexture(u32 flags)
{
	core::dimension2d<s32> optSize;
	ImageSize = Image->getDimension();

	if (Driver->queryFeature(EVDF_TEXTURE_NPOT))
		optSize=ImageSize;
	else
	{
		optSize.Width = getTextureSizeFromImageSize(ImageSize.Width);
		optSize.Height = getTextureSizeFromImageSize(ImageSize.Height);
	}

	HRESULT hr;
	D3DFORMAT format = D3DFMT_A1R5G5B5;

	switch(getTextureFormatFromFlags(flags))
	{
	case ETCF_ALWAYS_16_BIT:
		format = D3DFMT_A1R5G5B5; break;
	case ETCF_ALWAYS_32_BIT:
		format = D3DFMT_A8R8G8B8; break;
	case ETCF_OPTIMIZED_FOR_QUALITY:
		{
			switch(Image->getColorFormat())
			{
			case ECF_R8G8B8:
			case ECF_A8R8G8B8:
				format = D3DFMT_A8R8G8B8; break;
			case ECF_A1R5G5B5:
			case ECF_R5G6B5:
				format = D3DFMT_A1R5G5B5; break;
			}
		}
		break;
	case ETCF_OPTIMIZED_FOR_SPEED:
		format = D3DFMT_A1R5G5B5;
		break;
	default:
		break;
	}
	if (Driver->getTextureCreationFlag(video::ETCF_NO_ALPHA_CHANNEL))
	{
		if (format == D3DFMT_A8R8G8B8)
			format = D3DFMT_R8G8B8;
		else if (format == D3DFMT_A1R5G5B5)
			format = D3DFMT_R5G6B5;
	}

	const bool mipmaps = Driver->getTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS);

	DWORD usage = 0;

	// This enables hardware mip map generation.
	if (mipmaps && Driver->queryFeature(EVDF_MIP_MAP_AUTO_UPDATE))
	{
		LPDIRECT3D9 intf = Driver->getExposedVideoData().D3D9.D3D9;
		D3DDISPLAYMODE d3ddm;
		intf->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);

		if (D3D_OK==intf->CheckDeviceFormat(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,d3ddm.Format,D3DUSAGE_AUTOGENMIPMAP,D3DRTYPE_TEXTURE,format))
		{
			usage = D3DUSAGE_AUTOGENMIPMAP;
			HardwareMipMaps = true;
		}
	}

	hr = Device->CreateTexture(optSize.Width, optSize.Height,
		mipmaps ? 0 : 1, // number of mipmaplevels (0 = automatic all)
		usage, // usage
		format, D3DPOOL_MANAGED , &Texture, NULL);

	if (FAILED(hr))
	{
		// try brute force 16 bit
		HardwareMipMaps = false;
		if (format == D3DFMT_A8R8G8B8)
			format = D3DFMT_A1R5G5B5;
		else if (format == D3DFMT_R8G8B8)
			format = D3DFMT_R5G6B5;
		else
			return false;

		hr = Device->CreateTexture(optSize.Width, optSize.Height,
			mipmaps ? 0 : 1, // number of mipmaplevels (0 = automatic all)
			0, format, D3DPOOL_MANAGED, &Texture, NULL);
	}

	ColorFormat = getColorFormatFromD3DFormat(format);
	return (SUCCEEDED(hr));
}


D3DFORMAT CD3D9Texture::getD3DFormatFromColorFormat(ECOLOR_FORMAT format) const
{
	switch(format)
	{
		case ECF_A1R5G5B5:
			return D3DFMT_A1R5G5B5;
		case ECF_R5G6B5:
			return D3DFMT_R5G6B5;
		case ECF_R8G8B8:
			return D3DFMT_R8G8B8;
		case ECF_A8R8G8B8:
			return D3DFMT_A8R8G8B8;
	}
	return D3DFMT_UNKNOWN;
}


ECOLOR_FORMAT CD3D9Texture::getColorFormatFromD3DFormat(D3DFORMAT format)
{
	switch(format)
	{
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A1R5G5B5:
		Pitch = TextureSize.Width * 2;
		return ECF_A1R5G5B5;
	case D3DFMT_A8B8G8R8:
	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
		Pitch = TextureSize.Width * 4;
		return ECF_A8R8G8B8;
	case D3DFMT_R5G6B5:
		Pitch = TextureSize.Width * 2;
		return ECF_R5G6B5;
	case D3DFMT_R8G8B8:
		Pitch = TextureSize.Width * 3;
		return ECF_R8G8B8;
	default:
		return (ECOLOR_FORMAT)0;
	};
}


//! copies the image to the texture
bool CD3D9Texture::copyTexture()
{
	if (Texture && Image)
	{
		D3DSURFACE_DESC desc;
		Texture->GetLevelDesc(0, &desc);

		TextureSize.Width = desc.Width;
		TextureSize.Height = desc.Height;

		D3DLOCKED_RECT rect;
		HRESULT hr = Texture->LockRect(0, &rect, 0, 0);
		if (FAILED(hr))
		{
			os::Printer::log("Could not lock D3D9 Texture.", ELL_ERROR);
			return false;
		}

		Pitch = rect.Pitch;
		Image->copyToScaling(rect.pBits, TextureSize.Width, TextureSize.Height, ColorFormat, Pitch);

		hr = Texture->UnlockRect(0);
		if (FAILED(hr))
		{
			os::Printer::log("Could not unlock D3D9 Texture.", ELL_ERROR);
			return false;
		}
	}

	return true;
}


//! lock function
void* CD3D9Texture::lock()
{
	if (!Texture)
		return 0;

	HRESULT hr;
	D3DLOCKED_RECT rect;
	if(!IsRenderTarget)
	{
		hr = Texture->LockRect(0, &rect, 0, 0);
	}
	else
	{
		D3DSURFACE_DESC desc;
		Texture->GetLevelDesc(0, &desc);
		if (!RTTSurface)
		{
			hr = Device->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &RTTSurface, NULL);
			if (FAILED(hr))
			{
				os::Printer::log("Could not lock DIRECT3D9 Texture.", ELL_ERROR);
				return 0;
			}
		}

		IDirect3DSurface9 *surface = NULL;
		hr = Texture->GetSurfaceLevel(0, &surface);
		if (FAILED(hr))
		{
			os::Printer::log("Could not lock DIRECT3D9 Texture.", ELL_ERROR);
			return 0;
		}
		hr = Device->GetRenderTargetData(surface, RTTSurface);
		surface->Release();
		if(FAILED(hr))
		{
			os::Printer::log("Could not lock DIRECT3D9 Texture.", ELL_ERROR);
			return 0;
		}
		hr = RTTSurface->LockRect(&rect, NULL, 0);
		if(FAILED(hr))
		{
			os::Printer::log("Could not lock DIRECT3D9 Texture.", ELL_ERROR);
			return 0;
		}
		return rect.pBits;
	}
	if (FAILED(hr))
	{
		os::Printer::log("Could not lock DIRECT3D9 Texture.", ELL_ERROR);
		return 0;
	}

	return rect.pBits;
}



//! unlock function
void CD3D9Texture::unlock()
{
	if (!Texture)
		return;

	if (!IsRenderTarget)
		Texture->UnlockRect(0);
	else if (RTTSurface)
		RTTSurface->UnlockRect();
}


//! Returns original size of the texture.
const core::dimension2d<s32>& CD3D9Texture::getOriginalSize() const
{
	return ImageSize;
}


//! Returns (=size) of the texture.
const core::dimension2d<s32>& CD3D9Texture::getSize() const
{
	return TextureSize;
}


//! returns the size of a texture which would be the optimize size for rendering it
inline s32 CD3D9Texture::getTextureSizeFromImageSize(s32 size) const
{
	s32 ts = 0x01;

	while(ts < size)
		ts <<= 1;

	return ts;
}



//! returns driver type of texture (=the driver, who created the texture)
E_DRIVER_TYPE CD3D9Texture::getDriverType() const
{
	return EDT_DIRECT3D9;
}



//! returns color format of texture
ECOLOR_FORMAT CD3D9Texture::getColorFormat() const
{
	return ColorFormat;
}



//! returns pitch of texture (in bytes)
u32 CD3D9Texture::getPitch() const
{
	return Pitch;
}



//! returns the DIRECT3D9 Texture
IDirect3DTexture9* CD3D9Texture::getDX9Texture() const
{
	return Texture;
}


//! returns if texture has mipmap levels
bool CD3D9Texture::hasMipMaps() const
{
	return HasMipMaps;
}


void CD3D9Texture::copy16BitMipMap(char* src, char* tgt,
				   s32 width, s32 height,
				   s32 pitchsrc, s32 pitchtgt) const
{
	for (s32 y=0; y<height; ++y)
	{
		for (s32 x=0; x<width; ++x)
		{
			u32 a=0, r=0, g=0, b=0;

			for (s32 dy=0; dy<2; ++dy)
			{
				const s32 tgy = (y*2)+dy;
				for (s32 dx=0; dx<2; ++dx)
				{
					const s32 tgx = (x*2)+dx;

					SColor c;
					if (ColorFormat == ECF_A1R5G5B5)
						c = A1R5G5B5toA8R8G8B8(*(u16*)(&src[(tgx*2)+(tgy*pitchsrc)]));
					else
						c = R5G6B5toA8R8G8B8(*(u16*)(&src[(tgx*2)+(tgy*pitchsrc)]));

					a += c.getAlpha();
					r += c.getRed();
					g += c.getGreen();
					b += c.getBlue();
				}
			}

			a /= 4;
			r /= 4;
			g /= 4;
			b /= 4;

			u16 c;
			if (ColorFormat == ECF_A1R5G5B5)
				c = RGBA16(r,g,b,a);
			else
				c = A8R8G8B8toR5G6B5(SColor(a,r,g,b).color);
			*(u16*)(&tgt[(x*2)+(y*pitchtgt)]) = c;
		}
	}
}


void CD3D9Texture::copy32BitMipMap(char* src, char* tgt,
				   s32 width, s32 height,
				   s32 pitchsrc, s32 pitchtgt) const
{
	for (s32 y=0; y<height; ++y)
	{
		for (s32 x=0; x<width; ++x)
		{
			u32 a=0, r=0, g=0, b=0;
			SColor c;

			for (s32 dy=0; dy<2; ++dy)
			{
				const s32 tgy = (y*2)+dy;
				for (s32 dx=0; dx<2; ++dx)
				{
					const s32 tgx = (x*2)+dx;

					c = *(u32*)(&src[(tgx*4)+(tgy*pitchsrc)]);

					a += c.getAlpha();
					r += c.getRed();
					g += c.getGreen();
					b += c.getBlue();
				}
			}

			a /= 4;
			r /= 4;
			g /= 4;
			b /= 4;

			c.set(a, r, g, b);
			*(u32*)(&tgt[(x*4)+(y*pitchtgt)]) = c.color;
		}
	}
}


//! Regenerates the mip map levels of the texture. Useful after locking and
//! modifying the texture
void CD3D9Texture::regenerateMipMapLevels()
{
	if (HasMipMaps)
		createMipMaps();
}


//! returns if it is a render target
bool CD3D9Texture::isRenderTarget() const
{
	return IsRenderTarget;
}

//! Returns pointer to the render target surface
IDirect3DSurface9* CD3D9Texture::getRenderTargetSurface()
{
	if (!IsRenderTarget)
		return 0;

	IDirect3DSurface9 *pRTTSurface = 0;
	if (Texture)
		Texture->GetSurfaceLevel(0, &pRTTSurface);

	if (pRTTSurface)
		pRTTSurface->Release();

	return pRTTSurface;
}


} // end namespace video
} // end namespace irr

#endif // _IRR_COMPILE_WITH_DIRECT3D_9_

