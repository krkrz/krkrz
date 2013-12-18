// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_OPENGL_

#include "irrTypes.h"
#include "COpenGLTexture.h"
#include "COpenGLDriver.h"
#include "os.h"
#include "CImage.h"
#include "CColorConverter.h"

#include "irrString.h"

namespace irr
{
namespace video
{

// helper function for render to texture
static bool checkFBOStatus(COpenGLDriver* Driver);

//! constructor for usual textures
COpenGLTexture::COpenGLTexture(IImage* image, const char* name, COpenGLDriver* driver)
 : ITexture(name), Driver(driver), Image(0),
  TextureName(0), InternalFormat(GL_RGBA), PixelFormat(GL_BGRA_EXT),
  PixelType(GL_UNSIGNED_BYTE), HasMipMaps(true), IsRenderTarget(false),
  ColorFrameBuffer(0), DepthRenderBuffer(0), StencilRenderBuffer(0), Locks(0)
{
	#ifdef _DEBUG
	setDebugName("COpenGLTexture");
	#endif

	getImageData(image);

	HasMipMaps = Driver->getTextureCreationFlag(ETCF_CREATE_MIP_MAPS);
	if (Image)
	{
		glGenTextures(1, &TextureName);
		copyTexture();
	}
}

//! ColorFrameBuffer constructor
COpenGLTexture::COpenGLTexture(const core::dimension2d<s32>& size,
                                bool extPackedDepthStencilSupported,
                                const char* name,
                                COpenGLDriver* driver)
 : ITexture(name), ImageSize(size), Driver(driver), Image(0),
  TextureName(0), InternalFormat(GL_RGB8), PixelFormat(GL_RGBA),
  PixelType(GL_UNSIGNED_BYTE), HasMipMaps(false), IsRenderTarget(true),
  ColorFrameBuffer(0), DepthRenderBuffer(0), StencilRenderBuffer(0), Locks(0)
{
	#ifdef _DEBUG
	setDebugName("COpenGLTexture_FBO");
	#endif

	// generate color texture
	glGenTextures(1, &TextureName);
	glBindTexture(GL_TEXTURE_2D, TextureName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, ImageSize.Width,
		ImageSize.Height, 0, PixelFormat, PixelType, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#ifdef GL_EXT_packed_depth_stencil
	if (extPackedDepthStencilSupported)
	{
		// generate packed depth stencil texture
		glGenTextures(1, &DepthRenderBuffer);
		glBindTexture(GL_TEXTURE_2D, DepthRenderBuffer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL_EXT, ImageSize.Width,
			ImageSize.Height, 0, GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8_EXT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		StencilRenderBuffer = DepthRenderBuffer; // stencil is packed with depth
	}
	else // generate separate stencil and depth textures
#endif
	{
		// generate depth texture
		glGenTextures(1, &DepthRenderBuffer);
		glBindTexture(GL_TEXTURE_2D, DepthRenderBuffer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, ImageSize.Width,
			ImageSize.Height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // we 're in trouble! the code below does not complete the FBO currently...
        // stencil buffer is only supported with EXT_packed_depth_stencil extension (above)

//        // generate stencil texture
//        glGenTextures(1, &StencilRenderBuffer);
//        glBindTexture(GL_TEXTURE_2D, StencilRenderBuffer);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX, ImageSize.Width,
//                        ImageSize.Height, 0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, 0);
//        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

#ifdef GL_EXT_framebuffer_object
	// generate frame buffer
	Driver->extGlGenFramebuffers(1, &ColorFrameBuffer);
	Driver->extGlBindFramebuffer(GL_FRAMEBUFFER_EXT, ColorFrameBuffer);

	// attach color texture to frame buffer
	Driver->extGlFramebufferTexture2D(GL_FRAMEBUFFER_EXT,
						GL_COLOR_ATTACHMENT0_EXT,
						GL_TEXTURE_2D,
						TextureName,
						0);
	// attach depth texture to depth buffer
	Driver->extGlFramebufferTexture2D(GL_FRAMEBUFFER_EXT,
						GL_DEPTH_ATTACHMENT_EXT,
						GL_TEXTURE_2D,
						DepthRenderBuffer,
						0);
	// attach stencil texture to stencil buffer
	Driver->extGlFramebufferTexture2D(GL_FRAMEBUFFER_EXT,
						GL_STENCIL_ATTACHMENT_EXT,
						GL_TEXTURE_2D,
						StencilRenderBuffer,
						0);
	glGetError();

	// check the status
	if (!checkFBOStatus(Driver))
	{
		printf("FBO=%u, Color=%u, Depth=%u, Stencil=%u\n",
			ColorFrameBuffer, TextureName, DepthRenderBuffer, StencilRenderBuffer);
		if (ColorFrameBuffer)
			Driver->extGlDeleteFramebuffers(1, &ColorFrameBuffer);
		if (DepthRenderBuffer)
			glDeleteTextures(1, &DepthRenderBuffer);
		if (StencilRenderBuffer && StencilRenderBuffer != DepthRenderBuffer)
			glDeleteTextures(1, &StencilRenderBuffer);
		ColorFrameBuffer = 0;
		DepthRenderBuffer = 0;
		StencilRenderBuffer = 0;
	}
	Driver->extGlBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
#endif
}


//! destructor
COpenGLTexture::~COpenGLTexture()
{
	if (ColorFrameBuffer)
		Driver->extGlDeleteFramebuffers(1, &ColorFrameBuffer);
	if (DepthRenderBuffer)
		glDeleteTextures(1, &DepthRenderBuffer);
	if (StencilRenderBuffer && StencilRenderBuffer != DepthRenderBuffer)
		glDeleteTextures(1, &StencilRenderBuffer);

	glDeleteTextures(1, &TextureName);
	if (Image)
		Image->drop();
}


ECOLOR_FORMAT COpenGLTexture::getBestColorFormat(ECOLOR_FORMAT format)
{
	ECOLOR_FORMAT destFormat = ECF_A8R8G8B8;
	switch (format)
	{
		case ECF_A1R5G5B5:
			if (!Driver->getTextureCreationFlag(ETCF_ALWAYS_32_BIT))
				destFormat = ECF_A1R5G5B5;
		break;
		case ECF_R5G6B5:
			if (!Driver->getTextureCreationFlag(ETCF_ALWAYS_32_BIT))
				destFormat = ECF_A1R5G5B5;
		break;
		case ECF_A8R8G8B8:
			if (Driver->getTextureCreationFlag(ETCF_ALWAYS_16_BIT) ||
					Driver->getTextureCreationFlag(ETCF_OPTIMIZED_FOR_SPEED))
				destFormat = ECF_A1R5G5B5;
		break;
		case ECF_R8G8B8:
			if (Driver->getTextureCreationFlag(ETCF_ALWAYS_16_BIT) ||
					Driver->getTextureCreationFlag(ETCF_OPTIMIZED_FOR_SPEED))
				destFormat = ECF_A1R5G5B5;
		break;
	}
	if (Driver->getTextureCreationFlag(ETCF_NO_ALPHA_CHANNEL))
	{
		switch (destFormat)
		{
			case ECF_A1R5G5B5:
				destFormat = ECF_R5G6B5;
			break;
			case ECF_A8R8G8B8:
				destFormat = ECF_R8G8B8;
			break;
			default:
			break;
		}
	}
	return destFormat;
}


void COpenGLTexture::getImageData(IImage* image)
{
	if (!image)
	{
		os::Printer::log("No image for OpenGL texture.", ELL_ERROR);
		return;
	}

	ImageSize = image->getDimension();

	if ( !ImageSize.Width || !ImageSize.Height)
	{
		os::Printer::log("Invalid size of image for OpenGL Texture.", ELL_ERROR);
		return;
	}

	core::dimension2d<s32> nImageSize;
	if (Driver->queryFeature(EVDF_TEXTURE_NPOT))
		nImageSize=ImageSize;
	else
	{
		nImageSize.Width = getTextureSizeFromSurfaceSize(ImageSize.Width);
		nImageSize.Height = getTextureSizeFromSurfaceSize(ImageSize.Height);
	}

	ECOLOR_FORMAT destFormat = getBestColorFormat(image->getColorFormat());
	if (ImageSize==nImageSize)
		Image = new CImage(destFormat, image);
	else
	{
		Image = new CImage(destFormat, nImageSize);
		// scale texture
		image->copyToScaling(Image);
	}
}



//! copies the the texture into an open gl texture.
void COpenGLTexture::copyTexture(bool newTexture)
{
	glBindTexture(GL_TEXTURE_2D, TextureName);
	if (Driver->testGLError())
		os::Printer::log("Could not bind Texture", ELL_ERROR);

	switch (Image->getColorFormat())
	{
		case ECF_A1R5G5B5:
			InternalFormat=GL_RGBA;
			PixelFormat=GL_BGRA_EXT;
			PixelType=GL_UNSIGNED_SHORT_1_5_5_5_REV;
			break;
		case ECF_R5G6B5:
			InternalFormat=GL_RGB;
			PixelFormat=GL_BGR;
			PixelType=GL_UNSIGNED_SHORT_5_6_5_REV;
			break;
		case ECF_R8G8B8:
			InternalFormat=GL_RGB;
			PixelFormat=GL_BGR;
			PixelType=GL_UNSIGNED_BYTE;
			break;
		case ECF_A8R8G8B8:
			InternalFormat=GL_RGBA;
			PixelFormat=GL_BGRA_EXT;
			if (Driver->Version > 101)
				PixelType=GL_UNSIGNED_INT_8_8_8_8_REV;
			break;
		default:
			os::Printer::log("Unsupported texture format", ELL_ERROR);
			break;
	}

	if (newTexture)
	{
		#ifndef DISABLE_MIPMAPPING
		if (HasMipMaps && Driver->queryFeature(EVDF_MIP_MAP_AUTO_UPDATE))
		{
			// automatically generate and update mipmaps
			glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );
			AutomaticMipmapUpdate=true;
		}
		else
		{
			AutomaticMipmapUpdate=false;
			regenerateMipMapLevels();
		}
		if (HasMipMaps) // might have changed in regenerateMipMapLevels
		{
			// enable bilinear mipmap filter
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		#else
			HasMipMaps=false;
			os::Printer::log("Did not create OpenGL texture mip maps.", ELL_ERROR);
		#endif
		{
			// enable bilinear filter without mipmaps
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
	}

	void* source = Image->lock();
	if (newTexture)
		glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, Image->getDimension().Width,
			Image->getDimension().Height, 0, PixelFormat, PixelType, source);
	else
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Image->getDimension().Width,
			Image->getDimension().Height, PixelFormat, PixelType, source);
	Image->unlock();

	if (Driver->testGLError())
		os::Printer::log("Could not glTexImage2D", ELL_ERROR);
}



//! returns the size of a texture which would be the optimal size for rendering it
inline s32 COpenGLTexture::getTextureSizeFromSurfaceSize(s32 size) const
{
	s32 ts = 0x01;
	while(ts < size)
		ts <<= 1;

	return ts;
}


//! lock function
void* COpenGLTexture::lock()
{
	if (Image)
	{
		++Locks;
		return Image->lock();
	}
	else
		return 0;
}



//! unlock function
void COpenGLTexture::unlock()
{
	if (Image)
	{
		--Locks;
		Image->unlock();
		copyTexture(false);
	}
}



//! Returns size of the original image.
const core::dimension2d<s32>& COpenGLTexture::getOriginalSize() const
{
	return ImageSize;
}



//! Returns of the texture.
const core::dimension2d<s32>& COpenGLTexture::getSize() const
{
	if (Image)
		return Image->getDimension();
	else
		return ImageSize;
}



//! returns driver type of texture (=the driver, who created the texture)
E_DRIVER_TYPE COpenGLTexture::getDriverType() const
{
	return EDT_OPENGL;
}



//! returns color format of texture
ECOLOR_FORMAT COpenGLTexture::getColorFormat() const
{
	if (Image)
		return Image->getColorFormat();
	else
		return ECF_A8R8G8B8;
}



//! returns pitch of texture (in bytes)
u32 COpenGLTexture::getPitch() const
{
	if (Image)
		return Image->getPitch();
	else
		return 0;
}



//! return open gl texture name
GLuint COpenGLTexture::getOpenGLTextureName() const
{
	return TextureName;
}


//! Returns whether this texture has mipmaps
//! return true if texture has mipmaps
bool COpenGLTexture::hasMipMaps() const
{
	return HasMipMaps;
}


//! Regenerates the mip map levels of the texture. Useful after locking and
//! modifying the texture
void COpenGLTexture::regenerateMipMapLevels()
{
	if (AutomaticMipmapUpdate || !HasMipMaps)
		return;
	if ((Image->getDimension().Width==1) && (Image->getDimension().Height==1))
		return;

	// Manually create mipmaps
	u32 width=Image->getDimension().Width;
	u32 height=Image->getDimension().Height;
	u32 i=0;
	u8* target = new u8[Image->getImageDataSizeInBytes()];
	do
	{
		if (width>1)
			width>>=1;
		if (height>1)
			height>>=1;
		++i;
		Image->copyToScaling(target, width, height, Image->getColorFormat());
		glTexImage2D(GL_TEXTURE_2D, i, InternalFormat, width, height,
				0, PixelFormat, PixelType, target);
	}
	while (width!=1 || height!=1);
	delete [] target;
	Image->unlock();
}


bool COpenGLTexture::isFrameBufferObject() const
{
    return ColorFrameBuffer != 0;
}

bool COpenGLTexture::isRenderTarget() const
{
    return IsRenderTarget;
}

void COpenGLTexture::setRenderTarget(bool isTarget)
{
    IsRenderTarget = isTarget;
}


//! Bind ColorFrameBuffer (valid only if isFrameBufferObject() returns true).
void COpenGLTexture::bindFrameBufferObject()
{
#ifdef GL_EXT_framebuffer_object
    if (ColorFrameBuffer != 0)
        Driver->extGlBindFramebuffer(GL_FRAMEBUFFER_EXT, ColorFrameBuffer);
#endif
}


//! Unbind ColorFrameBuffer (valid only if isFrameBufferObject() returns true).
void COpenGLTexture::unbindFrameBufferObject()
{
#ifdef GL_EXT_framebuffer_object
    if (ColorFrameBuffer != 0)
        Driver->extGlBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
#endif
}

bool checkFBOStatus(COpenGLDriver* Driver)
{
#ifdef GL_EXT_framebuffer_object
	GLenum status = Driver->extGlCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);

	switch (status)
	{
		//Our FBO is perfect, return true
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			return true;

		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			os::Printer::log("FBO has invalid read buffer", ELL_ERROR);
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			os::Printer::log("FBO has invalid draw buffer", ELL_ERROR);
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			os::Printer::log("FBO has one or several incomplete image attachments", ELL_ERROR);
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			os::Printer::log("FBO has one or several image attachments with different internal formats", ELL_ERROR);
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			os::Printer::log("FBO has one or several image attachments with different dimensions", ELL_ERROR);
			break;

// not part of fbo_object anymore, but won't harm as it is just a return value
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT
		case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
			os::Printer::log("FBO has a duplicate image attachment", ELL_ERROR);
			break;
#endif

		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			os::Printer::log("FBO missing an image attachment", ELL_ERROR);
			break;

		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			os::Printer::log("FBO format unsupported", ELL_ERROR);
			break;

		default:
			break;
	}
	os::Printer::log("FBO error", ELL_ERROR);
#endif
	return false;
}

} // end namespace video
} // end namespace irr

#endif // _IRR_COMPILE_WITH_OPENGL_
