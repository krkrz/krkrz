// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CNullDriver.h"
#include "CSoftwareTexture.h"
#include "os.h"
#include "CImage.h"
#include "CAttributes.h"
#include "IReadFile.h"
#include "IWriteFile.h"
#include "IImageLoader.h"
#include "IImageWriter.h"
#include "IMaterialRenderer.h"
#include "CMeshManipulator.h"


namespace irr
{
namespace video
{

//! creates a loader which is able to load windows bitmaps
IImageLoader* createImageLoaderBMP();

//! creates a loader which is able to load jpeg images
IImageLoader* createImageLoaderJPG();

//! creates a loader which is able to load targa images
IImageLoader* createImageLoaderTGA();

//! creates a loader which is able to load psd images
IImageLoader* createImageLoaderPSD();

//! creates a loader which is able to load pcx images
IImageLoader* createImageLoaderPCX();

//! creates a loader which is able to load png images
IImageLoader* createImageLoaderPNG();

//! creates a loader which is able to load ppm/pgm/pbm images
IImageLoader* createImageLoaderPPM();

//! creates a loader which is able to load bmp images
IImageWriter* createImageWriterBMP();

//! creates a loader which is able to load jpg images
IImageWriter* createImageWriterJPG();

//! creates a loader which is able to load tga images
IImageWriter* createImageWriterTGA();

//! creates a loader which is able to load psd images
IImageWriter* createImageWriterPSD();

//! creates a loader which is able to load pcx images
IImageWriter* createImageWriterPCX();

//! creates a loader which is able to load png images
IImageWriter* createImageWriterPNG();

//! creates a loader which is able to load ppm images
IImageWriter* createImageWriterPPM();



//! constructor
CNullDriver::CNullDriver(io::IFileSystem* io, const core::dimension2d<s32>& screenSize)
: FileSystem(io), MeshManipulator(0), ViewPort(0,0,0,0), ScreenSize(screenSize),
	PrimitivesDrawn(0), TextureCreationFlags(0), AllowZWriteOnTransparent(false)
{
	#ifdef _DEBUG
	setDebugName("CNullDriver");
	#endif

	setFog();

	setTextureCreationFlag(ETCF_ALWAYS_32_BIT, true);
	setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, true);

	ViewPort = core::rect<s32>(core::position2d<s32>(0,0), screenSize);

	// create manipulator
	MeshManipulator = new scene::CMeshManipulator();

	if (FileSystem)
		FileSystem->grab();

	// create surface loader

#ifdef _IRR_COMPILE_WITH_BMP_LOADER_
	SurfaceLoader.push_back(video::createImageLoaderBMP());
#endif
#ifdef _IRR_COMPILE_WITH_JPG_LOADER_
	SurfaceLoader.push_back(video::createImageLoaderJPG());
#endif
#ifdef _IRR_COMPILE_WITH_TGA_LOADER_
	SurfaceLoader.push_back(video::createImageLoaderTGA());
#endif
#ifdef _IRR_COMPILE_WITH_PSD_LOADER_
	SurfaceLoader.push_back(video::createImageLoaderPSD());
#endif
#ifdef _IRR_COMPILE_WITH_PCX_LOADER_
	SurfaceLoader.push_back(video::createImageLoaderPCX());
#endif
#ifdef _IRR_COMPILE_WITH_PNG_LOADER_
	SurfaceLoader.push_back(video::createImageLoaderPNG());
#endif
#ifdef _IRR_COMPILE_WITH_PPM_LOADER_
	SurfaceLoader.push_back(video::createImageLoaderPPM());
#endif

#ifdef _IRR_COMPILE_WITH_BMP_WRITER_
	SurfaceWriter.push_back(video::createImageWriterBMP());
#endif
#ifdef _IRR_COMPILE_WITH_JPG_WRITER_
	SurfaceWriter.push_back(video::createImageWriterJPG());
#endif
#ifdef _IRR_COMPILE_WITH_TGA_WRITER_
	SurfaceWriter.push_back(video::createImageWriterTGA());
#endif
#ifdef _IRR_COMPILE_WITH_PSD_WRITER_
	SurfaceWriter.push_back(video::createImageWriterPSD());
#endif
#ifdef _IRR_COMPILE_WITH_PCX_WRITER_
	SurfaceWriter.push_back(video::createImageWriterPCX());
#endif
#ifdef _IRR_COMPILE_WITH_PNG_WRITER_
	SurfaceWriter.push_back(video::createImageWriterPNG());
#endif
#ifdef _IRR_COMPILE_WITH_PPM_WRITER_
	SurfaceWriter.push_back(video::createImageWriterPPM());
#endif

	// set ExposedData to 0
	memset(&ExposedData, 0, sizeof(ExposedData));
}


//! destructor
CNullDriver::~CNullDriver()
{
	if (FileSystem)
		FileSystem->drop();

	if (MeshManipulator)
		MeshManipulator->drop();

	deleteAllTextures();

	u32 i;
	for (i=0; i<SurfaceLoader.size(); ++i)
		SurfaceLoader[i]->drop();

	for (i=0; i<SurfaceWriter.size(); ++i)
		SurfaceWriter[i]->drop();

	deleteMaterialRenders();
}


//! Adds an external surface loader to the engine.
void CNullDriver::addExternalImageLoader(IImageLoader* loader)
{
	if (!loader)
		return;

	loader->grab();
	SurfaceLoader.push_back(loader);
}


//! Adds an external surface writer to the engine.
void CNullDriver::addExternalImageWriter(IImageWriter* writer)
{
	if (!writer)
		return;

	writer->grab();
	SurfaceWriter.push_back(writer);
}


//! deletes all textures
void CNullDriver::deleteAllTextures()
{
	for (u32 i=0; i<Textures.size(); ++i)
		Textures[i].Surface->drop();

	Textures.clear();
}



//! applications must call this method before performing any rendering. returns false if failed.
bool CNullDriver::beginScene(bool backBuffer, bool zBuffer, SColor color)
{
	core::clearFPUException ();
	PrimitivesDrawn = 0;
	return true;
}



//! applications must call this method after performing any rendering. returns false if failed.
bool CNullDriver::endScene( s32 windowId, core::rect<s32>* sourceRect, core::rect<s32>* destRect, void* destDC)
{
	FPSCounter.registerFrame(os::Timer::getRealTime(), PrimitivesDrawn);
	return true;
}



//! queries the features of the driver, returns true if feature is available
bool CNullDriver::queryFeature(E_VIDEO_DRIVER_FEATURE feature) const
{
	return false;
}



//! sets transformation
void CNullDriver::setTransform(E_TRANSFORMATION_STATE state, const core::matrix4& mat)
{
}


//! Returns the transformation set by setTransform
const core::matrix4& CNullDriver::getTransform(E_TRANSFORMATION_STATE state) const
{
	return TransformationMatrix;
}



//! sets a material
void CNullDriver::setMaterial(const SMaterial& material)
{
}



//! Removes a texture from the texture cache and deletes it, freeing lot of
//! memory.
void CNullDriver::removeTexture(ITexture* texture)
{
	if (!texture)
		return;

	for (u32 i=0; i<Textures.size(); ++i)
		if (Textures[i].Surface == texture)
		{
			texture->drop();
			Textures.erase(i);
		}
}


//! Removes all texture from the texture cache and deletes them, freeing lot of
//! memory.
void CNullDriver::removeAllTextures()
{
	deleteAllTextures();
}


//! Returns a texture by index
ITexture* CNullDriver::getTextureByIndex(u32 i)
{
	if ( i < Textures.size() )
		return Textures[i].Surface;

	return 0;
}


//! Returns amount of textures currently loaded
u32 CNullDriver::getTextureCount() const
{
	return Textures.size();
}


//! Renames a texture
void CNullDriver::renameTexture(ITexture* texture, const c8* newName)
{
	// we can do a const_cast here safely, the name of the ITexture interface
	// is just readonly to prevent the user changing the texture name without invoking
	// this method, because the textures will need resorting afterwards

	core::stringc& name = const_cast<core::stringc&>(texture->getName());
	name = newName;

	Textures.sort();
}


//! loads a Texture
ITexture* CNullDriver::getTexture(const c8* filename)
{
	ITexture* texture = findTexture(filename);

	if (texture)
		return texture;

	io::IReadFile* file = FileSystem->createAndOpenFile(filename);

	if (file)
	{
		texture = loadTextureFromFile(file, filename);
		file->drop();

		if (texture)
		{
			addTexture(texture);
			texture->drop(); // drop it because we created it, one grab too much
		}
		else
			os::Printer::log("Could not load texture", filename, ELL_ERROR);
		return texture;
	}
	else
	{
		os::Printer::log("Could not open file of texture", filename, ELL_ERROR);
		return 0;
	}
}


//! loads a Texture
ITexture* CNullDriver::getTexture(io::IReadFile* file)
{
	ITexture* texture = 0;

	if (file)
	{
		texture = findTexture(file->getFileName());

		if (texture)
			return texture;

		texture = loadTextureFromFile(file);

		if (texture)
		{
			addTexture(texture);
			texture->drop(); // drop it because we created it, one grab too much
		}
	}

	if (!texture)
		os::Printer::log("Could not load texture", file->getFileName(), ELL_ERROR);

	return texture;
}


//! opens the file and loads it into the surface
video::ITexture* CNullDriver::loadTextureFromFile(io::IReadFile* file, const c8 *hashName )
{
	ITexture* texture = 0;
	IImage* image = createImageFromFile(file);

	if (image)
	{
		// create texture from surface
		texture = createDeviceDependentTexture(image, hashName ? hashName : file->getFileName() );
		os::Printer::log("Loaded texture", file->getFileName());
		image->drop();
	}

	return texture;
}



//! adds a surface, not loaded or created by the Irrlicht Engine
void CNullDriver::addTexture(video::ITexture* texture)
{
	if (texture)
	{
		SSurface s;
		s.Surface = texture;
		texture->grab();

		Textures.push_back(s);

		// the new texture is now at the end of the texture list. when searching for
		// the next new texture, the texture array will be sorted and the index of this texture
		// will be changed. to let the order be more consistent to the user, sort
		// the textures now already although this isn't necessary:

		Textures.sort();
	}
}



//! looks if the image is already loaded
video::ITexture* CNullDriver::findTexture(const c8* filename)
{
	if (!filename)
		filename = "";

	SSurface s;
	SDummyTexture dummy(filename);
	s.Surface = &dummy;

	s32 index = Textures.binary_search(s);
	if (index != -1)
		return Textures[index].Surface;

	return 0;
}



//! Creates a texture from a loaded IImage.
ITexture* CNullDriver::addTexture(const c8* name, IImage* image)
{
	if (!name || !image)
		return 0;

	ITexture* t = createDeviceDependentTexture(image, name);
	if (t)
	{
		addTexture(t);
		t->drop();
	}
	return t;
}



//! creates a Texture
ITexture* CNullDriver::addTexture(const core::dimension2d<s32>& size,
				const c8* name, ECOLOR_FORMAT format)
{
	if (!name)
		return 0;

	IImage* image = new CImage(format, size);
	ITexture* t = createDeviceDependentTexture(image, name);
	image->drop();
	addTexture(t);

	if (t)
		t->drop();

	return t;
}



//! returns a device dependent texture from a software surface (IImage)
//! THIS METHOD HAS TO BE OVERRIDDEN BY DERIVED DRIVERS WITH OWN TEXTURES
ITexture* CNullDriver::createDeviceDependentTexture(IImage* surface, const char* name)
{
	#ifdef _IRR_COMPILE_WITH_SOFTWARE_
	return new CSoftwareTexture(surface, name);
	#else
	return 0;
	#endif
}



//! sets a render target
bool CNullDriver::setRenderTarget(video::ITexture* texture, bool clearBackBuffer,
					bool clearZBuffer, SColor color)
{
	return false;
}



//! sets a viewport
void CNullDriver::setViewPort(const core::rect<s32>& area)
{
}



//! gets the area of the current viewport
const core::rect<s32>& CNullDriver::getViewPort() const
{
	return ViewPort;
}



//! draws a vertex primitive list
void CNullDriver::drawVertexPrimitiveList(const void* vertices, u32 vertexCount, const u16* indexList, u32 primitiveCount, E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType)
{
	PrimitivesDrawn += primitiveCount;
}



//! draws an indexed triangle list
inline void CNullDriver::drawIndexedTriangleList(const S3DVertex* vertices, u32 vertexCount, const u16* indexList, u32 triangleCount)
{
	drawVertexPrimitiveList(vertices, vertexCount, indexList, triangleCount, EVT_STANDARD, scene::EPT_TRIANGLES);
}



//! draws an indexed triangle list
inline void CNullDriver::drawIndexedTriangleList(const S3DVertex2TCoords* vertices, u32 vertexCount, const u16* indexList, u32 triangleCount)
{
	drawVertexPrimitiveList(vertices, vertexCount, indexList, triangleCount, EVT_2TCOORDS, scene::EPT_TRIANGLES);
}


//! Draws an indexed triangle list.
inline void CNullDriver::drawIndexedTriangleList(const S3DVertexTangents* vertices,
	u32 vertexCount, const u16* indexList, u32 triangleCount)
{
	drawVertexPrimitiveList(vertices, vertexCount, indexList, triangleCount, EVT_TANGENTS, scene::EPT_TRIANGLES);
}



//! Draws an indexed triangle fan.
inline void CNullDriver::drawIndexedTriangleFan(const S3DVertex* vertices,
	u32 vertexCount, const u16* indexList, u32 triangleCount)
{
	drawVertexPrimitiveList(vertices, vertexCount, indexList, triangleCount, EVT_STANDARD, scene::EPT_TRIANGLE_FAN);
}



//! Draws an indexed triangle fan.
inline void CNullDriver::drawIndexedTriangleFan(const S3DVertex2TCoords* vertices,
	u32 vertexCount, const u16* indexList, u32 triangleCount)
{
	drawVertexPrimitiveList(vertices, vertexCount, indexList, triangleCount, EVT_2TCOORDS, scene::EPT_TRIANGLE_FAN);
}



//! Draws an indexed triangle fan.
inline void CNullDriver::drawIndexedTriangleFan(const S3DVertexTangents* vertices,
	u32 vertexCount, const u16* indexList, u32 triangleCount)
{
	drawVertexPrimitiveList(vertices, vertexCount, indexList, triangleCount, EVT_TANGENTS, scene::EPT_TRIANGLE_FAN);
}



//! Draws a 3d line.
void CNullDriver::draw3DLine(const core::vector3df& start,
				const core::vector3df& end, SColor color)
{
}



//! Draws a 3d triangle.
void CNullDriver::draw3DTriangle(const core::triangle3df& triangle, SColor color)
{
	draw3DLine(triangle.pointA, triangle.pointB, color);
	draw3DLine(triangle.pointB, triangle.pointC, color);
	draw3DLine(triangle.pointC, triangle.pointA, color);
}



//! Draws a 3d axis aligned box.
void CNullDriver::draw3DBox(const core::aabbox3d<f32>& box, SColor color)
{
	core::vector3df edges[8];
	box.getEdges(edges);

	// TODO: optimize into one big drawIndexPrimitive call.

	draw3DLine(edges[5], edges[1], color);
	draw3DLine(edges[1], edges[3], color);
	draw3DLine(edges[3], edges[7], color);
	draw3DLine(edges[7], edges[5], color);
	draw3DLine(edges[0], edges[2], color);
	draw3DLine(edges[2], edges[6], color);
	draw3DLine(edges[6], edges[4], color);
	draw3DLine(edges[4], edges[0], color);
	draw3DLine(edges[1], edges[0], color);
	draw3DLine(edges[3], edges[2], color);
	draw3DLine(edges[7], edges[6], color);
	draw3DLine(edges[5], edges[4], color);
}



//! draws an 2d image
void CNullDriver::draw2DImage(const video::ITexture* texture, const core::position2d<s32>& destPos)
{
	if (!texture)
		return;

	draw2DImage(texture,destPos, core::rect<s32>(core::position2d<s32>(0,0), texture->getOriginalSize()));
}



//! draws a set of 2d images, using a color and the alpha channel of the
//! texture if desired. The images are drawn beginning at pos and concatenated
//! in one line. All drawings are clipped against clipRect (if != 0).
//! The subtextures are defined by the array of sourceRects and are chosen
//! by the indices given.
void CNullDriver::draw2DImage(const video::ITexture* texture,
				const core::position2d<s32>& pos,
				const core::array<core::rect<s32> >& sourceRects,
				const core::array<s32>& indices,
				s32 kerningWidth,
				const core::rect<s32>* clipRect, SColor color,
				bool useAlphaChannelOfTexture)
{
	core::position2d<s32> target(pos);

	for (u32 i=0; i<indices.size(); ++i)
	{
		draw2DImage(texture, target, sourceRects[indices[i]],
				clipRect, color, useAlphaChannelOfTexture);
		target.X += sourceRects[indices[i]].getWidth();
		target.X += kerningWidth;
	}
}



//! Draws a part of the texture into the rectangle.
void CNullDriver::draw2DImage(const video::ITexture* texture, const core::rect<s32>& destRect,
	const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect,
	video::SColor* colors, bool useAlphaChannelOfTexture)
{
}



//! draws an 2d image, using a color (if color is other then Color(255,255,255,255)) and the alpha channel of the texture if wanted.
void CNullDriver::draw2DImage(const video::ITexture* texture, const core::position2d<s32>& destPos,
				const core::rect<s32>& sourceRect,
				const core::rect<s32>* clipRect, SColor color,
				bool useAlphaChannelOfTexture)
{
}



//! draw an 2d rectangle
void CNullDriver::draw2DRectangle(SColor color, const core::rect<s32>& pos, const core::rect<s32>* clip)
{
	draw2DRectangle(pos, color, color, color, color, clip);
}



//!Draws an 2d rectangle with a gradient.
void CNullDriver::draw2DRectangle(const core::rect<s32>& pos,
	SColor colorLeftUp, SColor colorRightUp, SColor colorLeftDown, SColor colorRightDown,
	const core::rect<s32>* clip)
{
}



//! Draws a 2d line.
void CNullDriver::draw2DLine(const core::position2d<s32>& start,
				const core::position2d<s32>& end, SColor color)
{
}



//! Draws a non filled concyclic regular 2d polyon.
void CNullDriver::draw2DPolygon(core::position2d<s32> center,
	f32 radius, video::SColor color, s32 count)
{
	if (count < 2)
		return;

	core::position2d<s32> first;
	core::position2d<s32> a,b;

	for (s32 j=0; j<count; ++j)
	{
		b = a;

		f32 p = j / (f32)count * (core::PI*2);
		a = center + core::position2d<s32>((s32)(sin(p)*radius), (s32)(cos(p)*radius));

		if (j==0)
			first = a;
		else
			draw2DLine(a, b, color);
	}

	draw2DLine(a, first, color);
}




//! returns screen size
const core::dimension2d<s32>& CNullDriver::getScreenSize() const
{
	return ScreenSize;
}

//! returns the current render target size,
//! or the screen size if render targets are not implemented
const core::dimension2d<s32>& CNullDriver::getCurrentRenderTargetSize() const
{
	return ScreenSize;
}


// returns current frames per second value
s32 CNullDriver::getFPS() const
{
	return FPSCounter.getFPS();
}



//! returns amount of primitives (mostly triangles) were drawn in the last frame.
//! very useful method for statistics.
u32 CNullDriver::getPrimitiveCountDrawn( u32 param ) const
{
	return (0 == param) ? FPSCounter.getPrimitive() : (1 == param) ? FPSCounter.getPrimitiveAverage() : FPSCounter.getPrimitiveTotal();
}



//! Sets the dynamic ambient light color. The default color is
//! (0,0,0,0) which means it is dark.
//! \param color: New color of the ambient light.
void CNullDriver::setAmbientLight(const SColorf& color)
{
}



//! \return Returns the name of the video driver. Example: In case of the DIRECT3D8
//! driver, it would return "Direct3D8".
const wchar_t* CNullDriver::getName() const
{
	return L"Irrlicht NullDevice";
}



//! Draws a shadow volume into the stencil buffer. To draw a stencil shadow, do
//! this: Frist, draw all geometry. Then use this method, to draw the shadow
//! volume. Then, use IVideoDriver::drawStencilShadow() to visualize the shadow.
void CNullDriver::drawStencilShadowVolume(const core::vector3df* triangles, s32 count, bool zfail)
{
}


//! Fills the stencil shadow with color. After the shadow volume has been drawn
//! into the stencil buffer using IVideoDriver::drawStencilShadowVolume(), use this
//! to draw the color of the shadow.
void CNullDriver::drawStencilShadow(bool clearStencilBuffer,
		video::SColor leftUpEdge, video::SColor rightUpEdge,
		video::SColor leftDownEdge, video::SColor rightDownEdge)
{
}


//! deletes all dynamic lights there are
void CNullDriver::deleteAllDynamicLights()
{
	Lights.set_used(0);
}


//! adds a dynamic light
void CNullDriver::addDynamicLight(const SLight& light)
{
	Lights.push_back(light);
}



//! returns the maximal amount of dynamic lights the device can handle
u32 CNullDriver::getMaximalDynamicLightAmount() const
{
	return 0;
}


//! Returns current amount of dynamic lights set
//! \return Current amount of dynamic lights set
u32 CNullDriver::getDynamicLightCount() const
{
	return Lights.size();
}


//! Returns light data which was previously set by IVideoDriver::addDynamicLight().
//! \param idx: Zero based index of the light. Must be greater than 0 and smaller
//! than IVideoDriver()::getDynamicLightCount.
//! \return Light data.
const SLight& CNullDriver::getDynamicLight(u32 idx) const
{
	if ( idx < Lights.size() )
		return Lights[idx];
	else
		return *((SLight*)0);
}


//! Creates an 1bit alpha channel of the texture based of an color key.
void CNullDriver::makeColorKeyTexture(video::ITexture* texture, video::SColor color) const
{
	if (!texture)
		return;

	if (texture->getColorFormat() != ECF_A1R5G5B5 &&
		texture->getColorFormat() != ECF_A8R8G8B8 )
	{
		os::Printer::log("Error: Unsupported texture color format for making color key channel.", ELL_ERROR);
		return;
	}

	if (texture->getColorFormat() == ECF_A1R5G5B5)
	{
		s16 *p = (s16*)texture->lock();

		if (!p)
		{
			os::Printer::log("Could not lock texture for making color key channel.", ELL_ERROR);
			return;
		}

		core::dimension2d<s32> dim = texture->getSize();
		s32 pitch = texture->getPitch() / 2;

		// color with alpha enabled (color opaque)
		s16 ref = (0x1<<15) | (0x7fff & color.toA1R5G5B5());

		for (s32 y=0; y<dim.Height; ++y)
		{
			for (s32 x=0; x<pitch; ++x)
			{
				s16 c = (0x1<<15) | (0x7fff & p[y*pitch + x]);
				p[y*pitch + x] = (c == ref) ? 0 : c;
			}
		}

		texture->unlock();
	}
	else
	{
		s32 *p = (s32*)texture->lock();

		if (!p)
		{
			os::Printer::log("Could not lock texture for making color key channel.", ELL_ERROR);
			return;
		}

		core::dimension2d<s32> dim = texture->getSize();
		s32 pitch = texture->getPitch() / 4;

		// color with alpha enabled (color opaque)
		s32 ref = 0xff000000 | (0x00ffffff & color.color);

		for (s32 y=0; y<dim.Height; ++y)
		{
			for (s32 x=0; x<pitch; ++x)
			{
				s32 c = (0xff<<24) | (0x00ffffff & p[y*pitch + x]);
				p[y*pitch + x] = (c == ref) ? 0 : c;
			}
		}

		texture->unlock();
	}
}



//! Creates an 1bit alpha channel of the texture based of an color key position.
void CNullDriver::makeColorKeyTexture(video::ITexture* texture,
					core::position2d<s32> colorKeyPixelPos) const
{
	if (!texture)
		return;

	if (texture->getColorFormat() != ECF_A1R5G5B5 &&
		texture->getColorFormat() != ECF_A8R8G8B8 )
	{
		os::Printer::log("Error: Unsupported texture color format for making color key channel.", ELL_ERROR);
		return;
	}

	if (texture->getColorFormat() == ECF_A1R5G5B5)
	{
		s16 *p = (s16*)texture->lock();

		if (!p)
		{
			os::Printer::log("Could not lock texture for making color key channel.", ELL_ERROR);
			return;
		}

		core::dimension2d<s32> dim = texture->getSize();
		s32 pitch = texture->getPitch() / 2;

		s16 ref = (0x1<<15) | (0x7fff & p[colorKeyPixelPos.Y*dim.Width + colorKeyPixelPos.X]);

		for (s32 y=0; y<dim.Height; ++y)
		{
			for (s32 x=0; x<pitch; ++x)
			{
				s16 c = (0x1<<15) | (0x7fff & p[y*pitch + x]);
				p[y*pitch + x] = (c == ref) ? 0 : c;
			}
		}

		texture->unlock();
	}
	else
	{
		s32 *p = (s32*)texture->lock();

		if (!p)
		{
			os::Printer::log("Could not lock texture for making color key channel.", ELL_ERROR);
			return;
		}

		core::dimension2d<s32> dim = texture->getSize();
		s32 pitch = texture->getPitch() / 4;

		s32 ref = (0xff<<24) | (0x00ffffff & p[colorKeyPixelPos.Y*dim.Width + colorKeyPixelPos.X]);

		for (s32 y=0; y<dim.Height; ++y)
		{
			for (s32 x=0; x<pitch; ++x)
			{
				s32 c = (0xff<<24) | (0x00ffffff & p[y*pitch + x]);
				p[y*pitch + x] = (c == ref) ? 0 : c;
			}
		}

		texture->unlock();
	}
}



//! Creates a normal map from a height map texture.
//! \param amplitude: Constant value by which the height information is multiplied.
void CNullDriver::makeNormalMapTexture(video::ITexture* texture, f32 amplitude) const
{
	if (!texture)
		return;

	if (texture->getColorFormat() != ECF_A1R5G5B5 &&
		texture->getColorFormat() != ECF_A8R8G8B8 )
	{
		os::Printer::log("Error: Unsupported texture color format for making normal map.", ELL_ERROR);
		return;
	}

	core::dimension2d<s32> dim = texture->getSize();
	amplitude = amplitude / 255.0f;
	f32 vh = dim.Height / (f32)dim.Width;
	f32 hh = dim.Width / (f32)dim.Height;

	if (texture->getColorFormat() == ECF_A8R8G8B8)
	{
		// ECF_A8R8G8B8 version

		s32 *p = (s32*)texture->lock();

		if (!p)
		{
			os::Printer::log("Could not lock texture for making normal map.", ELL_ERROR);
			return;
		}

		// copy texture

		s32 pitch = texture->getPitch() / 4;

		s32* in = new s32[dim.Height * pitch];
		memcpy(in, p, dim.Height * pitch * 4);

		for (s32 x=0; x<pitch; ++x)
			for (s32 y=0; y<dim.Height; ++y)
			{
				// TODO: this could be optimized really a lot

				core::vector3df h1((x-1)*hh, nml32(x-1, y, pitch, dim.Height, in)*amplitude, y*vh);
				core::vector3df h2((x+1)*hh, nml32(x+1, y, pitch, dim.Height, in)*amplitude, y*vh);
				//core::vector3df v1(x*hh, nml32(x, y-1, pitch, dim.Height, in)*amplitude, (y-1)*vh);
				//core::vector3df v2(x*hh, nml32(x, y+1, pitch, dim.Height, in)*amplitude, (y+1)*vh);
				core::vector3df v1(x*hh, nml32(x, y+1, pitch, dim.Height, in)*amplitude, (y-1)*vh);
				core::vector3df v2(x*hh, nml32(x, y-1, pitch, dim.Height, in)*amplitude, (y+1)*vh);

				core::vector3df v = v1-v2;
				core::vector3df h = h1-h2;

				core::vector3df n = v.crossProduct(h);
				n.normalize();
				n *= 0.5f;
				n += core::vector3df(0.5f,0.5f,0.5f); // now between 0 and 1
				n *= 255.0f;

				s32 height = (s32)nml32(x, y, pitch, dim.Height, in);
				p[y*pitch + x] = video::SColor(
					height, // store height in alpha
					(s32)n.X, (s32)n.Z, (s32)n.Y).color;
			}

		delete [] in;
		texture->unlock();
	}
	else
	{
		// ECF_A1R5G5B5 version

		s16 *p = (s16*)texture->lock();

		if (!p)
		{
			os::Printer::log("Could not lock texture for making normal map.", ELL_ERROR);
			return;
		}

		s32 pitch = texture->getPitch() / 2;

		// copy texture

		s16* in = new s16[dim.Height * pitch];
		memcpy(in, p, dim.Height * pitch * 2);

		for (s32 x=0; x<pitch; ++x)
			for (s32 y=0; y<dim.Height; ++y)
			{
				// TODO: this could be optimized really a lot

				core::vector3df h1((x-1)*hh, nml16(x-1, y, pitch, dim.Height, in)*amplitude, y*vh);
				core::vector3df h2((x+1)*hh, nml16(x+1, y, pitch, dim.Height, in)*amplitude, y*vh);
				core::vector3df v1(x*hh, nml16(x, y-1, pitch, dim.Height, in)*amplitude, (y-1)*vh);
				core::vector3df v2(x*hh, nml16(x, y+1, pitch, dim.Height, in)*amplitude, (y+1)*vh);

				core::vector3df v = v1-v2;
				core::vector3df h = h1-h2;

				core::vector3df n = v.crossProduct(h);
				n.normalize();
				n *= 0.5f;
				n += core::vector3df(0.5f,0.5f,0.5f); // now between 0 and 1
				n *= 255.0f;

				p[y*pitch + x] = video::RGB16((s32)n.X, (s32)n.Z, (s32)n.Y);
			}

		delete [] in;
		texture->unlock();
	}

	texture->regenerateMipMapLevels();
}


//! Returns the maximum amount of primitives (mostly vertices) which
//! the device is able to render with one drawIndexedTriangleList
//! call.
u32 CNullDriver::getMaximalPrimitiveCount() const
{
	return 0xFFFFFFFF;
}


//! checks triangle count and print warning if wrong
bool CNullDriver::checkPrimitiveCount(u32 prmCount) const
{
	const u32 m = getMaximalPrimitiveCount();

	if (prmCount > m)
	{
		char tmp[1024];
		sprintf(tmp,"Could not draw triangles, too many primitives(%u), maxium is %u.", prmCount, m);
		os::Printer::log(tmp, ELL_ERROR);
		return false;
	}

	return true;
}

//! Enables or disables a texture creation flag.
void CNullDriver::setTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag, bool enabled)
{
	if (enabled && ((flag == ETCF_ALWAYS_16_BIT) || (flag == ETCF_ALWAYS_32_BIT)
		|| (flag == ETCF_OPTIMIZED_FOR_QUALITY) || (flag == ETCF_OPTIMIZED_FOR_SPEED)))
	{
		// disable other formats
		setTextureCreationFlag(ETCF_ALWAYS_16_BIT, false);
		setTextureCreationFlag(ETCF_ALWAYS_32_BIT, false);
		setTextureCreationFlag(ETCF_OPTIMIZED_FOR_QUALITY, false);
		setTextureCreationFlag(ETCF_OPTIMIZED_FOR_SPEED, false);
	}

	// set flag
	TextureCreationFlags = (TextureCreationFlags & (~flag)) |
		((((u32)!enabled)-1) & flag);
}


//! Returns if a texture creation flag is enabled or disabled.
bool CNullDriver::getTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag) const
{
	return (TextureCreationFlags & flag)!=0;
}


//! Creates a software image from a file.
IImage* CNullDriver::createImageFromFile(const char* filename)
{
	if (!filename)
		return 0;

	IImage* image = 0;
	io::IReadFile* file = FileSystem->createAndOpenFile(filename);

	if (file)
	{
		image = createImageFromFile(file);
		file->drop();
	}
	else
		os::Printer::log("Could not open file of image", filename, ELL_ERROR);

	return image;
}


//! Creates a software image from a file.
IImage* CNullDriver::createImageFromFile(io::IReadFile* file)
{
	if (!file)
		return 0;

	IImage* image = 0;

	u32 i;

	// try to load file based on file extension
	for (i=0; i<SurfaceLoader.size(); ++i)
	{
		if (SurfaceLoader[i]->isALoadableFileExtension(file->getFileName()))
		{
			// reset file position which might have changed due to previous loadImage calls
			file->seek(0);
			image = SurfaceLoader[i]->loadImage(file);
			if (image)
				return image;
		}
	}

	// try to load file based on what is in it
	for (i=0; i<SurfaceLoader.size(); ++i)
	{
		// dito
		file->seek(0);
		if (SurfaceLoader[i]->isALoadableFileFormat(file))
		{
			file->seek(0);
			image = SurfaceLoader[i]->loadImage(file);
			if (image)
				return image;
		}
	}

	return 0; // failed to load
}



//! Writes the provided image to disk file
bool CNullDriver::writeImageToFile(IImage* image, const char* filename,u32 param)
{
	for (u32 i=0; i<SurfaceWriter.size(); ++i)
	{
		if (SurfaceWriter[i]->isAWriteableFileExtension(filename))
		{
			io::IWriteFile* file = FileSystem->createAndWriteFile(filename);
			if (file)
			{
				bool written = SurfaceWriter[i]->writeImage(file, image, param);
				file->drop();
				if (written)
					return true;
			}
		}
	}
	return false; // failed to write
}



//! Creates a software image from a byte array.
IImage* CNullDriver::createImageFromData(ECOLOR_FORMAT format,
					const core::dimension2d<s32>& size,
					void *data, bool ownForeignMemory,
					bool deleteMemory)
{
	return new CImage(format, size, data, ownForeignMemory, deleteMemory);
}


//! Creates an empty software image.
IImage* CNullDriver::createImage(ECOLOR_FORMAT format, const core::dimension2d<s32>& size)
{
        return new CImage(format, size);
}


//! Creates a software image from another image.
IImage* CNullDriver::createImage(ECOLOR_FORMAT format, IImage *imageToCopy)
{
        return new CImage(format, imageToCopy);
}


//! Creates a software image from part of another image.
IImage* CNullDriver::createImage(IImage* imageToCopy, const core::position2d<s32>& pos, const core::dimension2d<s32>& size)
{
        return new CImage(imageToCopy, pos, size);
}


//! Sets the fog mode.
void CNullDriver::setFog(SColor color, bool linearFog, f32 start, f32 end,
		f32 density, bool pixelFog, bool rangeFog)
{
	FogColor = color;
	LinearFog = linearFog;
	FogStart = start;
	FogEnd = end;
	FogDensity = density;
	PixelFog = pixelFog;
	RangeFog = rangeFog;
}


//! Draws a mesh buffer
void CNullDriver::drawMeshBuffer(const scene::IMeshBuffer* mb)
{
	if (!mb)
		return;

	drawVertexPrimitiveList(mb->getVertices(), mb->getVertexCount(), mb->getIndices(), mb->getIndexCount()/3, mb->getVertexType(), scene::EPT_TRIANGLES);
}



//! Only used by the internal engine. Used to notify the driver that
//! the window was resized.
void CNullDriver::OnResize(const core::dimension2d<s32>& size)
{
	if (ViewPort.getWidth() == ScreenSize.Width &&
		ViewPort.getHeight() == ScreenSize.Height)
		ViewPort = core::rect<s32>(core::position2d<s32>(0,0), size);

	ScreenSize = size;
}

// adds a material renderer and drops it afterwards. To be used for internal creation
s32 CNullDriver::addAndDropMaterialRenderer(IMaterialRenderer* m)
{
	s32 i = addMaterialRenderer(m);

	if (m)
		m->drop();

	return i;
}


//! Adds a new material renderer to the video device.
s32 CNullDriver::addMaterialRenderer(IMaterialRenderer* renderer, const char* name)
{
	if (!renderer)
		return -1;

	SMaterialRenderer r;
	r.Renderer = renderer;
	r.Name = name;

	if (name == 0 && (MaterialRenderers.size() < (sizeof(sBuiltInMaterialTypeNames) / sizeof(char*))-1 ))
	{
		// set name of built in renderer so that we don't have to implement name
		// setting in all available renderers.
		r.Name = sBuiltInMaterialTypeNames[MaterialRenderers.size()];
	}

	MaterialRenderers.push_back(r);
	renderer->grab();

	return MaterialRenderers.size()-1;
}


//! Sets the name of a material renderer.
void CNullDriver::setMaterialRendererName(s32 idx, const char* name)
{
	if (idx < s32(sizeof(sBuiltInMaterialTypeNames) / sizeof(char*))-1 ||
		idx >= (s32)MaterialRenderers.size())
		return;

	MaterialRenderers[idx].Name = name;
}


//! Creates material attributes list from a material, usable for serialization and more.
io::IAttributes* CNullDriver::createAttributesFromMaterial(const video::SMaterial& material)
{
	io::CAttributes* attr = new io::CAttributes(this);

	attr->addEnum("Type", material.MaterialType, sBuiltInMaterialTypeNames);

	attr->addColor("Ambient", material.AmbientColor);
	attr->addColor("Diffuse", material.DiffuseColor);
	attr->addColor("Emissive", material.EmissiveColor);
	attr->addColor("Specular", material.SpecularColor);

	attr->addFloat("Shininess", material.Shininess);
	attr->addFloat("Param1", material.MaterialTypeParam);
	attr->addFloat("Param2", material.MaterialTypeParam2);

	core::stringc prefix="Texture";
	u32 i;
	for (i=0; i<MATERIAL_MAX_TEXTURES; ++i)
		attr->addTexture((prefix+(i+1)).c_str(), material.getTexture(i));

	attr->addBool("Wireframe", material.Wireframe);
	attr->addBool("GouraudShading", material.GouraudShading);
	attr->addBool("Lighting", material.Lighting);
	attr->addBool("ZWriteEnable", material.ZWriteEnable);
	attr->addInt("ZBuffer", material.ZBuffer);
	attr->addBool("BackfaceCulling", material.BackfaceCulling);
	attr->addBool("FogEnable", material.FogEnable);
	attr->addBool("NormalizeNormals", material.NormalizeNormals);

	prefix = "BilinearFilter";
	for (i=0; i<MATERIAL_MAX_TEXTURES; ++i)
		attr->addBool((prefix+(i+1)).c_str(), material.TextureLayer[i].BilinearFilter);
	prefix = "TrilinearFilter";
	for (i=0; i<MATERIAL_MAX_TEXTURES; ++i)
		attr->addBool((prefix+(i+1)).c_str(), material.TextureLayer[i].TrilinearFilter);
	prefix = "AnisotropicFilter";
	for (i=0; i<MATERIAL_MAX_TEXTURES; ++i)
		attr->addBool((prefix+(i+1)).c_str(), material.TextureLayer[i].AnisotropicFilter);
	prefix="TextureWrap";
	for (i=0; i<MATERIAL_MAX_TEXTURES; ++i)
		attr->addEnum((prefix+(i+1)).c_str(), material.TextureLayer[i].TextureWrap, aTextureClampNames);

	return attr;
}


//! Fills an SMaterial structure from attributes.
void CNullDriver::fillMaterialStructureFromAttributes(video::SMaterial& outMaterial, io::IAttributes* attr)
{
	outMaterial.MaterialType = video::EMT_SOLID;

	core::stringc name = attr->getAttributeAsString("Type");

	u32 i;

	for ( i=0; i < MaterialRenderers.size(); ++i)
		if ( name == MaterialRenderers[i].Name )
		{
			outMaterial.MaterialType = (video::E_MATERIAL_TYPE)i;
			break;
		}

	outMaterial.AmbientColor = attr->getAttributeAsColor("Ambient");
	outMaterial.DiffuseColor = attr->getAttributeAsColor("Diffuse");
	outMaterial.EmissiveColor = attr->getAttributeAsColor("Emissive");
	outMaterial.SpecularColor = attr->getAttributeAsColor("Specular");

	outMaterial.Shininess = attr->getAttributeAsFloat("Shininess");
	outMaterial.MaterialTypeParam = attr->getAttributeAsFloat("Param1");
	outMaterial.MaterialTypeParam2 = attr->getAttributeAsFloat("Param2");

	core::stringc prefix="Texture";
	for (i=0; i<MATERIAL_MAX_TEXTURES; ++i)
		outMaterial.setTexture(i, attr->getAttributeAsTexture((prefix+(i+1)).c_str()));

	outMaterial.Wireframe = attr->getAttributeAsBool("Wireframe");
	outMaterial.GouraudShading = attr->getAttributeAsBool("GouraudShading");
	outMaterial.Lighting = attr->getAttributeAsBool("Lighting");
	outMaterial.ZWriteEnable = attr->getAttributeAsBool("ZWriteEnable");
	outMaterial.ZBuffer = attr->getAttributeAsInt("ZBuffer");
	outMaterial.BackfaceCulling = attr->getAttributeAsBool("BackfaceCulling");
	outMaterial.FogEnable = attr->getAttributeAsBool("FogEnable");
	outMaterial.NormalizeNormals = attr->getAttributeAsBool("NormalizeNormals");
	prefix = "BilinearFilter";
	if (attr->existsAttribute(prefix.c_str())) // legacy
		outMaterial.setFlag(EMF_BILINEAR_FILTER, attr->getAttributeAsBool(prefix.c_str()));
	else
		for (i=0; i<MATERIAL_MAX_TEXTURES; ++i)
			outMaterial.TextureLayer[i].BilinearFilter = attr->getAttributeAsBool((prefix+(i+1)).c_str());

	prefix = "TrilinearFilter";
	if (attr->existsAttribute(prefix.c_str())) // legacy
		outMaterial.setFlag(EMF_TRILINEAR_FILTER, attr->getAttributeAsBool(prefix.c_str()));
	else
		for (i=0; i<MATERIAL_MAX_TEXTURES; ++i)
			outMaterial.TextureLayer[i].TrilinearFilter = attr->getAttributeAsBool((prefix+(i+1)).c_str());

	prefix = "AnisotropicFilter";
	if (attr->existsAttribute(prefix.c_str())) // legacy
		outMaterial.setFlag(EMF_ANISOTROPIC_FILTER, attr->getAttributeAsBool(prefix.c_str()));
	else
		for (i=0; i<MATERIAL_MAX_TEXTURES; ++i)
			outMaterial.TextureLayer[i].AnisotropicFilter = attr->getAttributeAsBool((prefix+(i+1)).c_str());

	prefix = "TextureWrap";
	for (i=0; i<MATERIAL_MAX_TEXTURES; ++i)
		outMaterial.TextureLayer[i].TextureWrap = (E_TEXTURE_CLAMP)attr->getAttributeAsEnumeration((prefix+(i+1)).c_str(), aTextureClampNames);
}


//! Returns driver and operating system specific data about the IVideoDriver.
const SExposedVideoData& CNullDriver::getExposedVideoData()
{
	return ExposedData;
}


//! Returns type of video driver
E_DRIVER_TYPE CNullDriver::getDriverType() const
{
	return EDT_NULL;
}

//! deletes all material renderers
void CNullDriver::deleteMaterialRenders()
{
	// delete material renderers
	for (int i=0; i<(int)MaterialRenderers.size(); ++i)
		if (MaterialRenderers[i].Renderer)
			MaterialRenderers[i].Renderer->drop();

	MaterialRenderers.clear();
}

//! Returns pointer to material renderer or null
IMaterialRenderer* CNullDriver::getMaterialRenderer(u32 idx)
{
	if ( idx < MaterialRenderers.size() )
		return MaterialRenderers[idx].Renderer;
	else
		return 0;
}



//! Returns amount of currently available material renderers.
u32 CNullDriver::getMaterialRendererCount() const
{
	return MaterialRenderers.size();
}


//! Returns name of the material renderer
const char* CNullDriver::getMaterialRendererName(u32 idx) const
{
	if ( idx < MaterialRenderers.size() )
		return MaterialRenderers[idx].Name.c_str();

	return 0;
}


//! Returns pointer to the IGPUProgrammingServices interface.
IGPUProgrammingServices* CNullDriver::getGPUProgrammingServices()
{
	return 0;
}

//! Adds a new material renderer to the VideoDriver, based on a high level shading
//! language. Currently only HLSL in D3D9 is supported.
s32 CNullDriver::addHighLevelShaderMaterial(
	const c8* vertexShaderProgram,
	const c8* vertexShaderEntryPointName,
	E_VERTEX_SHADER_TYPE vsCompileTarget,
	const c8* pixelShaderProgram,
	const c8* pixelShaderEntryPointName,
	E_PIXEL_SHADER_TYPE psCompileTarget,
	IShaderConstantSetCallBack* callback,
	E_MATERIAL_TYPE baseMaterial,
	s32 userData)
{
	os::Printer::log("High level shader materials not available (yet) in this driver, sorry");
	return -1;
}

//! Like IGPUProgrammingServices::addShaderMaterial() (look there for a detailed description),
//! but tries to load the programs from files.
s32 CNullDriver::addHighLevelShaderMaterialFromFiles(
	const c8* vertexShaderProgram,
	const c8* vertexShaderEntryPointName,
	E_VERTEX_SHADER_TYPE vsCompileTarget,
	const c8* pixelShaderProgram,
	const c8* pixelShaderEntryPointName,
	E_PIXEL_SHADER_TYPE psCompileTarget,
	IShaderConstantSetCallBack* callback,
	E_MATERIAL_TYPE baseMaterial,
	s32 userData)
{
	io::IReadFile* vsfile = 0;
	io::IReadFile* psfile = 0;

	if (vertexShaderProgram)
	{
		vsfile = FileSystem->createAndOpenFile(vertexShaderProgram);
		if (!vsfile)
		{
			os::Printer::log("Could not open vertex shader program file",
				vertexShaderProgram, ELL_WARNING);
			return -1;
		}
	}

	if (pixelShaderProgram)
	{
		psfile = FileSystem->createAndOpenFile(pixelShaderProgram);
		if (!psfile)
		{
			os::Printer::log("Could not open pixel shader program file",
				pixelShaderProgram, ELL_WARNING);
			if (vsfile)
				vsfile->drop();
			return -1;
		}
	}

	s32 result = addHighLevelShaderMaterialFromFiles(
		vsfile, vertexShaderEntryPointName, vsCompileTarget,
		psfile, pixelShaderEntryPointName, psCompileTarget,
		callback, baseMaterial, userData);

	if (psfile)
		psfile->drop();

	if (vsfile)
		vsfile->drop();

	return result;
}

//! Like IGPUProgrammingServices::addShaderMaterial() (look there for a detailed description),
//! but tries to load the programs from files.
s32 CNullDriver::addHighLevelShaderMaterialFromFiles(
	io::IReadFile* vertexShaderProgram,
	const c8* vertexShaderEntryPointName,
	E_VERTEX_SHADER_TYPE vsCompileTarget,
	io::IReadFile* pixelShaderProgram,
	const c8* pixelShaderEntryPointName,
	E_PIXEL_SHADER_TYPE psCompileTarget,
	IShaderConstantSetCallBack* callback,
	E_MATERIAL_TYPE baseMaterial,
	s32 userData)
{
	c8* vs = 0;
	c8* ps = 0;

	if (vertexShaderProgram)
	{
		const long size = vertexShaderProgram->getSize();
		if (size)
		{
			vs = new c8[size+1];
			vertexShaderProgram->read(vs, size);
			vs[size] = 0;
		}
	}

	if (pixelShaderProgram)
	{
		const long size = pixelShaderProgram->getSize();
		if (size)
		{
			ps = new c8[size+1];
			pixelShaderProgram->read(ps, size);
			ps[size] = 0;
		}
	}

	s32 result = this->addHighLevelShaderMaterial(
		vs, vertexShaderEntryPointName, vsCompileTarget,
		ps, pixelShaderEntryPointName, psCompileTarget,
		callback, baseMaterial, userData);

	delete [] vs;
	delete [] ps;

	return result;
}

//! Adds a new material renderer to the VideoDriver, using pixel and/or
//! vertex shaders to render geometry.
s32 CNullDriver::addShaderMaterial(const c8* vertexShaderProgram,
	const c8* pixelShaderProgram,
	IShaderConstantSetCallBack* callback,
	E_MATERIAL_TYPE baseMaterial,
	s32 userData)
{
	os::Printer::log("Shader materials not implemented yet in this driver, sorry.");
	return -1;
}

//! Like IGPUProgrammingServices::addShaderMaterial(), but tries to load the
//! programs from files.
s32 CNullDriver::addShaderMaterialFromFiles(io::IReadFile* vertexShaderProgram,
	io::IReadFile* pixelShaderProgram,
	IShaderConstantSetCallBack* callback,
	E_MATERIAL_TYPE baseMaterial,
	s32 userData)
{
	c8* vs = 0;
	c8* ps = 0;

	if (vertexShaderProgram)
	{
		const long size = vertexShaderProgram->getSize();
		if (size)
		{
			vs = new c8[size+1];
			vertexShaderProgram->read(vs, size);
			vs[size] = 0;
		}
	}

	if (pixelShaderProgram)
	{
		const long size = pixelShaderProgram->getSize();
		if (size)
		{
			ps = new c8[size+1];
			pixelShaderProgram->read(ps, size);
			ps[size] = 0;
		}
	}

	s32 result = addShaderMaterial(vs, ps, callback, baseMaterial, userData);

	delete [] vs;
	delete [] ps;

	return result;
}



//! Like IGPUProgrammingServices::addShaderMaterial(), but tries to load the
//! programs from files.
s32 CNullDriver::addShaderMaterialFromFiles(const c8* vertexShaderProgramFileName,
	const c8* pixelShaderProgramFileName,
	IShaderConstantSetCallBack* callback,
	E_MATERIAL_TYPE baseMaterial,
	s32 userData)
{
	io::IReadFile* vsfile = 0;
	io::IReadFile* psfile = 0;

	if (vertexShaderProgramFileName)
	{
		vsfile = FileSystem->createAndOpenFile(vertexShaderProgramFileName);
		if (!vsfile)
		{
			os::Printer::log("Could not open vertex shader program file",
				vertexShaderProgramFileName, ELL_WARNING);
			return -1;
		}
	}

	if (pixelShaderProgramFileName)
	{
		psfile = FileSystem->createAndOpenFile(pixelShaderProgramFileName);
		if (!psfile)
		{
			os::Printer::log("Could not open pixel shader program file",
				pixelShaderProgramFileName, ELL_WARNING);
			if (vsfile)
				vsfile->drop();
			return -1;
		}
	}

	s32 result = addShaderMaterialFromFiles(vsfile, psfile, callback,
		baseMaterial, userData);

	if (psfile)
		psfile->drop();

	if (vsfile)
		vsfile->drop();

	return result;
}

//! Creates a render target texture.
ITexture* CNullDriver::createRenderTargetTexture(const core::dimension2d<s32>& size, const c8* name)
{
	return 0;
}

//! Clears the ZBuffer.
void CNullDriver::clearZBuffer()
{
}


//! Returns a pointer to the mesh manipulator.
scene::IMeshManipulator* CNullDriver::getMeshManipulator()
{
	return MeshManipulator;
}


//! Returns an image created from the last rendered frame.
IImage* CNullDriver::createScreenShot()
{
	return 0;
}

// prints renderer version
void CNullDriver::printVersion()
{
	core::stringw namePrint = L"Using renderer: ";
	namePrint += getName();
	os::Printer::log(namePrint.c_str(), ELL_INFORMATION);
}


//! creates a video driver
IVideoDriver* createNullDriver(io::IFileSystem* io, const core::dimension2d<s32>& screenSize)
{
	return new CNullDriver(io, screenSize);
}


//! Set/unset a clipping plane.
//! There are at least 6 clipping planes available for the user to set at will.
//! \param index: The plane index. Must be between 0 and MaxUserClipPlanes.
//! \param plane: The plane itself.
//! \param enable: If true, enable the clipping plane else disable it.
bool CNullDriver::setClipPlane(u32 index, const core::plane3df& plane, bool enable)
{
	return false;
}

//! Enable/disable a clipping plane.
//! There are at least 6 clipping planes available for the user to set at will.
//! \param index: The plane index. Must be between 0 and MaxUserClipPlanes.
//! \param enable: If true, enable the clipping plane else disable it.
void CNullDriver::enableClipPlane(u32 index, bool enable)
{
	// not necessary
}


} // end namespace
} // end namespace

