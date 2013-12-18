// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_OPEN_GL_TEXTURE_H_INCLUDED__
#define __C_OPEN_GL_TEXTURE_H_INCLUDED__

#include "ITexture.h"
#include "IImage.h"

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_OPENGL_

#ifdef _IRR_WINDOWS_API_
	// include windows headers for HWND
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <GL/gl.h>
	#include "glext.h"
#ifdef _MSC_VER
	#pragma comment(lib, "OpenGL32.lib")
	#pragma comment(lib, "GLu32.lib")
#endif
#else
	#if defined(_IRR_OPENGL_USE_EXTPOINTER_)
		#define GL_GLEXT_LEGACY 1
	#endif
	#if defined(_IRR_USE_OSX_DEVICE_)
		#include <OpenGL/gl.h>
	#else
		#include <GL/gl.h>
	#endif
	#if defined(_IRR_OPENGL_USE_EXTPOINTER_)
		#include "glext.h"
	#endif
#endif


namespace irr
{
namespace video
{

class COpenGLDriver;
//! OpenGL texture.
class COpenGLTexture : public ITexture
{
public:

	//! constructor
	COpenGLTexture(IImage* surface, const char* name, COpenGLDriver* driver=0);
	//! FrameBufferObject constructor
	COpenGLTexture(const core::dimension2d<s32>& size, bool extPackedDepthStencilSupported, const char* name, COpenGLDriver* driver=0);

	//! destructor
	virtual ~COpenGLTexture();

	//! lock function
	virtual void* lock();

	//! unlock function
	virtual void unlock();

	//! Returns original size of the texture.
	virtual const core::dimension2d<s32>& getOriginalSize() const;

	//! Returns size of the texture.
	virtual const core::dimension2d<s32>& getSize() const;

	//! returns driver type of texture (=the driver, who created the texture)
	virtual E_DRIVER_TYPE getDriverType() const;

	//! returns color format of texture
	virtual ECOLOR_FORMAT getColorFormat() const;

	//! returns pitch of texture (in bytes)
	virtual u32 getPitch() const;

	//! return open gl texture name
	GLuint getOpenGLTextureName() const;

	//! return whether this texture has mipmaps
	virtual bool hasMipMaps() const;

	//! Regenerates the mip map levels of the texture. Useful after
	//! locking and modifying the texture
	virtual void regenerateMipMapLevels();

	//! Is it a render target?
	virtual bool isRenderTarget() const;

	//! Is it a FrameBufferObject?
	bool isFrameBufferObject() const;

	//! Bind FrameBufferObject (valid only if isFrameBufferObject() returns true).
	void bindFrameBufferObject();

	//! Unbind FrameBufferObject (valid only if isFrameBufferObject() returns true).
	void unbindFrameBufferObject();

	//! sets whether this texture is intended to be used as a render target.
	void setRenderTarget(bool isTarget);

private:

	//! get the desired color format based on texture creation flags and the input format.
	ECOLOR_FORMAT getBestColorFormat(ECOLOR_FORMAT format);

	//! convert the image into an internal image with better properties for this driver.
	void getImageData(IImage* image);

	//! copies the the texture into an open gl texture.
	//! \param: newTexture is true if method is called from a newly created texture for the first time. Otherwise call with false to improve memory handling.
	void copyTexture(bool newTexture=true);

	//! returns the size of a texture which would be the optimize size for rendering it
	inline s32 getTextureSizeFromSurfaceSize(s32 size) const;

	core::dimension2d<s32> ImageSize;
	COpenGLDriver* Driver;
	IImage* Image;

	GLuint TextureName;
	GLint InternalFormat;
	GLenum PixelFormat;
	GLenum PixelType;
	bool HasMipMaps;
	bool IsRenderTarget;
	bool AutomaticMipmapUpdate;

	GLuint ColorFrameBuffer; // for FBO path
	GLuint DepthRenderBuffer; // for FBO path
	GLuint StencilRenderBuffer; // for FBO path

	u32 Locks;
};


} // end namespace video
} // end namespace irr

#endif
#endif // _IRR_COMPILE_WITH_OPENGL_

