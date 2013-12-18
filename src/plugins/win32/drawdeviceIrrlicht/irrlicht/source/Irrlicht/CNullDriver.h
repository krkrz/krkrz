// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_VIDEO_NULL_H_INCLUDED__
#define __C_VIDEO_NULL_H_INCLUDED__

#include "IVideoDriver.h"
#include "IFileSystem.h"
#include "IImagePresenter.h"
#include "IGPUProgrammingServices.h"
#include "irrArray.h"
#include "irrString.h"
#include "IAttributes.h"
#include "IMeshBuffer.h"
#include "CFPSCounter.h"
#include "S3DVertex.h"
#include "SLight.h"
#include "SExposedVideoData.h"

namespace irr
{
namespace io
{
	class IWriteFile;
	class IReadFile;
} // end namespace io
namespace video
{
	class IImageLoader;
	class IImageWriter;

	class CNullDriver : public IVideoDriver, public IGPUProgrammingServices
	{
	public:

		//! constructor
		CNullDriver(io::IFileSystem* io, const core::dimension2d<s32>& screenSize);

		//! destructor
		virtual ~CNullDriver();

		virtual bool beginScene(bool backBuffer, bool zBuffer, SColor color);

		virtual bool endScene( s32 windowId = 0, core::rect<s32>* sourceRect=0, core::rect<s32>* destRect=0, void* destDC=0 );

		//! queries the features of the driver, returns true if feature is available
		virtual bool queryFeature(E_VIDEO_DRIVER_FEATURE feature) const;

		//! sets transformation
		virtual void setTransform(E_TRANSFORMATION_STATE state, const core::matrix4& mat);

		//! sets a material
		virtual void setMaterial(const SMaterial& material);

		//! loads a Texture
		virtual ITexture* getTexture(const c8* filename);

		//! loads a Texture
		virtual ITexture* getTexture(io::IReadFile* file);

		//! Returns a texture by index
		virtual ITexture* getTextureByIndex(u32 index);

		//! Returns amount of textures currently loaded
		virtual u32 getTextureCount() const;

		//! Renames a texture
		virtual void renameTexture(ITexture* texture, const c8* newName);

		//! creates a Texture
		virtual ITexture* addTexture(const core::dimension2d<s32>& size, const c8* name, ECOLOR_FORMAT format = ECF_A8R8G8B8);

		//! sets a render target
		virtual bool setRenderTarget(video::ITexture* texture, bool clearBackBuffer, 
						 bool clearZBuffer, SColor color);

		//! sets a viewport
		virtual void setViewPort(const core::rect<s32>& area);

		//! gets the area of the current viewport
		virtual const core::rect<s32>& getViewPort() const;

		//! draws a vertex primitive list
		virtual void drawVertexPrimitiveList(const void* vertices, u32 vertexCount,
				const u16* indexList, u32 primitiveCount,
				E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType);

		//! draws an indexed triangle list
		virtual void drawIndexedTriangleList(const S3DVertex* vertices, u32 vertexCount, const u16* indexList, u32 triangleCount);

		//! draws an indexed triangle list
		virtual void drawIndexedTriangleList(const S3DVertex2TCoords* vertices, u32 vertexCount, const u16* indexList, u32 triangleCount);

		//! Draws an indexed triangle list.
		virtual void drawIndexedTriangleList(const S3DVertexTangents* vertices,
			u32 vertexCount, const u16* indexList, u32 triangleCount);

		//! Draws an indexed triangle fan.
		virtual void drawIndexedTriangleFan(const S3DVertex* vertices,
			u32 vertexCount, const u16* indexList, u32 triangleCount);

		//! Draws an indexed triangle list.
		virtual void drawIndexedTriangleFan(const S3DVertex2TCoords* vertices,
			u32 vertexCount, const u16* indexList, u32 triangleCount);

		//! Draws an indexed triangle fan.
		inline void drawIndexedTriangleFan(const S3DVertexTangents* vertices,
			u32 vertexCount, const u16* indexList, u32 triangleCount);

		//! Draws a 3d line.
		virtual void draw3DLine(const core::vector3df& start,
			const core::vector3df& end, SColor color = SColor(255,255,255,255));

		//! Draws a 3d triangle.
		virtual void draw3DTriangle(const core::triangle3df& triangle,
			SColor color = SColor(255,255,255,255));

		//! Draws a 3d axis aligned box.
		virtual void draw3DBox(const core::aabbox3d<f32>& box,
			SColor color = SColor(255,255,255,255));

		//! draws an 2d image
		virtual void draw2DImage(const video::ITexture* texture, const core::position2d<s32>& destPos);

		//! draws a set of 2d images, using a color and the alpha
		/** channel of the texture if desired. The images are drawn
		beginning at pos and concatenated in one line. All drawings
		are clipped against clipRect (if != 0).
		The subtextures are defined by the array of sourceRects
		and are chosen by the indices given.
		\param texture: Texture to be drawn.
		\param pos: Upper left 2d destination position where the image will be drawn.
		\param sourceRects: Source rectangles of the image.
		\param indices: List of indices which choose the actual rectangle used each time.
		\param kerningWidth: offset on position
		\param clipRect: Pointer to rectangle on the screen where the image is clipped to.
		This pointer can be 0. Then the image is not clipped.
		\param color: Color with which the image is colored.
		Note that the alpha component is used: If alpha is other than 255, the image will be transparent.
		\param useAlphaChannelOfTexture: If true, the alpha channel of the texture is
		used to draw the image. */
		virtual void draw2DImage(const video::ITexture* texture,
				const core::position2d<s32>& pos,
				const core::array<core::rect<s32> >& sourceRects,
				const core::array<s32>& indices,
				s32 kerningWidth = 0,
				const core::rect<s32>* clipRect = 0,
				SColor color=SColor(255,255,255,255),
				bool useAlphaChannelOfTexture=false);

		//! draws an 2d image, using a color (if color is other then Color(255,255,255,255)) and the alpha channel of the texture if wanted.
		virtual void draw2DImage(const video::ITexture* texture, const core::position2d<s32>& destPos, 
			const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect = 0,
			SColor color=SColor(255,255,255,255), bool useAlphaChannelOfTexture=false);

		//! Draws a part of the texture into the rectangle.
		virtual void draw2DImage(const video::ITexture* texture, const core::rect<s32>& destRect,
			const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect = 0,
			video::SColor* colors=0, bool useAlphaChannelOfTexture=false);

		//! draw an 2d rectangle
		virtual void draw2DRectangle(SColor color, const core::rect<s32>& pos, const core::rect<s32>* clip = 0);

		//!Draws an 2d rectangle with a gradient.
		virtual void draw2DRectangle(const core::rect<s32>& pos,
			SColor colorLeftUp, SColor colorRightUp, SColor colorLeftDown, SColor colorRightDown,
			const core::rect<s32>* clip = 0);

		//! Draws a 2d line. 
		virtual void draw2DLine(const core::position2d<s32>& start,
					const core::position2d<s32>& end,
					SColor color=SColor(255,255,255,255));

		//! Draws a non filled concyclic reqular 2d polyon.
		virtual void draw2DPolygon(core::position2d<s32> center, 
			f32 radius, video::SColor Color, s32 vertexCount);

		virtual void setFog(SColor color=SColor(0,255,255,255), bool linearFog=true,
			f32 start=50.0f, f32 end=100.0f, 
			f32 density=0.01f, bool pixelFog=false, bool rangeFog=false);

		//! returns screen size
		virtual const core::dimension2d<s32>& getScreenSize() const;

		//! returns screen size
		virtual const core::dimension2d<s32>& getCurrentRenderTargetSize() const;

		// returns current frames per second value
		virtual s32 getFPS() const;

		//! returns amount of primitives (mostly triangles) were drawn in the last frame.
		//! very useful method for statistics.
		virtual u32 getPrimitiveCountDrawn( u32 param = 0 ) const;

		//! deletes all dynamic lights there are
		virtual void deleteAllDynamicLights();

		//! adds a dynamic light
		virtual void addDynamicLight(const SLight& light);

		//! returns the maximal amount of dynamic lights the device can handle
		virtual u32 getMaximalDynamicLightAmount() const;

		//! \return Returns the name of the video driver. Example: In case of the DIRECT3D8
		//! driver, it would return "Direct3D8.1".
		virtual const wchar_t* getName() const;

		//! Sets the dynamic ambient light color. The default color is
		//! (0,0,0,0) which means it is dark.
		//! \param color: New color of the ambient light.
		virtual void setAmbientLight(const SColorf& color);

		//! Adds an external image loader to the engine.
		virtual void addExternalImageLoader(IImageLoader* loader);

		//! Adds an external image writer to the engine.
		virtual void addExternalImageWriter(IImageWriter* writer);

		//! Draws a shadow volume into the stencil buffer. To draw a stencil shadow, do
		//! this: Frist, draw all geometry. Then use this method, to draw the shadow
		//! volume. Then, use IVideoDriver::drawStencilShadow() to visualize the shadow.
		virtual void drawStencilShadowVolume(const core::vector3df* triangles, s32 count, bool zfail=true);

		//! Fills the stencil shadow with color. After the shadow volume has been drawn
		//! into the stencil buffer using IVideoDriver::drawStencilShadowVolume(), use this
		//! to draw the color of the shadow. 
		virtual void drawStencilShadow(bool clearStencilBuffer=false, 
			video::SColor leftUpEdge = video::SColor(0,0,0,0), 
			video::SColor rightUpEdge = video::SColor(0,0,0,0),
			video::SColor leftDownEdge = video::SColor(0,0,0,0),
			video::SColor rightDownEdge = video::SColor(0,0,0,0));

		//! Returns current amount of dynamic lights set
		//! \return Current amount of dynamic lights set
		virtual u32 getDynamicLightCount() const;

		//! Returns light data which was previously set with IVideDriver::addDynamicLight().
		//! \param idx: Zero based index of the light. Must be greater than 0 and smaller
		//! than IVideoDriver()::getDynamicLightCount.
		//! \return Light data.
		virtual const SLight& getDynamicLight(u32 idx) const;

		//! Removes a texture from the texture cache and deletes it, freeing lot of
		//! memory. 
		virtual void removeTexture(ITexture* texture);

		//! Removes all texture from the texture cache and deletes them, freeing lot of
		//! memory. 
		virtual void removeAllTextures();

		//! Creates a render target texture.
		virtual ITexture* createRenderTargetTexture(const core::dimension2d<s32>& size, const c8* name);

		//! Creates an 1bit alpha channel of the texture based of an color key.
		virtual void makeColorKeyTexture(video::ITexture* texture, video::SColor color) const;

		//! Creates an 1bit alpha channel of the texture based of an color key position.
		virtual void makeColorKeyTexture(video::ITexture* texture, core::position2d<s32> colorKeyPixelPos) const;

		//! Creates a normal map from a height map texture. 
		//! \param amplitude: Constant value by which the height information is multiplied.
		virtual void makeNormalMapTexture(video::ITexture* texture, f32 amplitude=1.0f) const;

		//! Returns the maximum amount of primitives (mostly vertices) which
		//! the device is able to render with one drawIndexedTriangleList
		//! call.
		virtual u32 getMaximalPrimitiveCount() const;

		//! Enables or disables a texture creation flag.
		virtual void setTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag, bool enabled);

		//! Returns if a texture creation flag is enabled or disabled.
		virtual bool getTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag) const;

		//! Creates a software image from a file. 
		virtual IImage* createImageFromFile(const char* filename);

		//! Creates a software image from a file. 
		virtual IImage* createImageFromFile(io::IReadFile* file);

		//! Creates a software image from a byte array.
		/** \param useForeignMemory: If true, the image will use the data pointer
		directly and own it from now on, which means it will also try to delete [] the
		data when the image will be destructed. If false, the memory will by copied. */
		virtual IImage* createImageFromData(ECOLOR_FORMAT format, 
			const core::dimension2d<s32>& size, void *data,
			bool ownForeignMemory=true, bool deleteForeignMemory = true);

		//! Creates an empty software image.
                virtual IImage* createImage(ECOLOR_FORMAT format, const core::dimension2d<s32>& size);


		//! Creates a software image from another image.
                virtual IImage* createImage(ECOLOR_FORMAT format, IImage *imageToCopy);

		//! Creates a software image from part of another image.
                virtual IImage* createImage(IImage* imageToCopy,
                        const core::position2d<s32>& pos, const core::dimension2d<s32>& size);

		//! Draws a mesh buffer
		virtual void drawMeshBuffer(const scene::IMeshBuffer* mb);

		//! Only used by the engine internally.
		/** Used to notify the driver that the window was resized. */
		virtual void OnResize(const core::dimension2d<s32>& size);

		//! Adds a new material renderer to the video device.
		virtual s32 addMaterialRenderer(IMaterialRenderer* renderer,
				const char* name = 0);

		//! Returns driver and operating system specific data about the IVideoDriver.
		virtual const SExposedVideoData& getExposedVideoData();

		//! Returns type of video driver
		virtual E_DRIVER_TYPE getDriverType() const;

		//! Returns the transformation set by setTransform
		virtual const core::matrix4& getTransform(E_TRANSFORMATION_STATE state) const;

		//! Returns pointer to the IGPUProgrammingServices interface.
		virtual IGPUProgrammingServices* getGPUProgrammingServices();
		
		//! Adds a new material renderer to the VideoDriver, using pixel and/or 
		//! vertex shaders to render geometry.
		virtual s32 addShaderMaterial(const c8* vertexShaderProgram = 0,
			const c8* pixelShaderProgram = 0,
			IShaderConstantSetCallBack* callback = 0,
			E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
			s32 userData=0);

		//! Like IGPUProgrammingServices::addShaderMaterial(), but tries to load the 
		//! programs from files.
		virtual s32 addShaderMaterialFromFiles(io::IReadFile* vertexShaderProgram = 0,
			io::IReadFile* pixelShaderProgram = 0,
			IShaderConstantSetCallBack* callback = 0,
			E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
			s32 userData=0);

		//! Like IGPUProgrammingServices::addShaderMaterial(), but tries to load the 
		//! programs from files.
		virtual s32 addShaderMaterialFromFiles(const c8* vertexShaderProgramFileName = 0,
			const c8* pixelShaderProgramFileName = 0,
			IShaderConstantSetCallBack* callback = 0,
			E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
			s32 userData=0);

		//! Returns pointer to material renderer or null
		virtual IMaterialRenderer* getMaterialRenderer(u32 idx);

		//! Returns amount of currently available material renderers.
		virtual u32 getMaterialRendererCount() const;

		//! Returns name of the material renderer
		virtual const char* getMaterialRendererName(u32 idx) const;

		//! Adds a new material renderer to the VideoDriver, based on a high level shading 
		//! language. Currently only HLSL in D3D9 is supported. 
		virtual s32 addHighLevelShaderMaterial(
			const c8* vertexShaderProgram,
			const c8* vertexShaderEntryPointName = 0,
			E_VERTEX_SHADER_TYPE vsCompileTarget = EVST_VS_1_1,
			const c8* pixelShaderProgram = 0, 
			const c8* pixelShaderEntryPointName = 0,
			E_PIXEL_SHADER_TYPE psCompileTarget = EPST_PS_1_1,
			IShaderConstantSetCallBack* callback = 0,
			E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
			s32 userData=0);   

		//! Like IGPUProgrammingServices::addShaderMaterial() (look there for a detailed description),
		//! but tries to load the programs from files. 
		virtual s32 addHighLevelShaderMaterialFromFiles(
			const c8* vertexShaderProgram,
			const c8* vertexShaderEntryPointName = "main",
			E_VERTEX_SHADER_TYPE vsCompileTarget = EVST_VS_1_1,
			const c8* pixelShaderProgram = 0, 
			const c8* pixelShaderEntryPointName = "main",
			E_PIXEL_SHADER_TYPE psCompileTarget = EPST_PS_1_1,
			IShaderConstantSetCallBack* callback = 0,
			E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
			s32 userData=0); 

		//! Like IGPUProgrammingServices::addShaderMaterial() (look there for a detailed description),
		//! but tries to load the programs from files. 
		virtual s32 addHighLevelShaderMaterialFromFiles(
			io::IReadFile* vertexShaderProgram,
			const c8* vertexShaderEntryPointName = "main",
			E_VERTEX_SHADER_TYPE vsCompileTarget = EVST_VS_1_1,
			io::IReadFile* pixelShaderProgram = 0, 
			const c8* pixelShaderEntryPointName = "main",
			E_PIXEL_SHADER_TYPE psCompileTarget = EPST_PS_1_1,
			IShaderConstantSetCallBack* callback = 0,
			E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
			s32 userData=0); 

		//! Returns a pointer to the mesh manipulator.
		virtual scene::IMeshManipulator* getMeshManipulator();
		
		//! Clears the ZBuffer. 
		virtual void clearZBuffer();

		//! Returns an image created from the last rendered frame.
		virtual IImage* createScreenShot();

		//! Writes the provided image to disk file
		virtual bool writeImageToFile(IImage* image, const char* filename, u32 param = 0);

		//! Sets the name of a material renderer. 
		virtual void setMaterialRendererName(s32 idx, const char* name);

		//! Creates material attributes list from a material, usable for serialization and more.
		virtual io::IAttributes* createAttributesFromMaterial(const video::SMaterial& material);

		//! Fills an SMaterial structure from attributes.
		virtual void fillMaterialStructureFromAttributes(video::SMaterial& outMaterial, io::IAttributes* attributes);

		//! looks if the image is already loaded
		virtual video::ITexture* findTexture(const c8* filename);

		//! Set/unset a clipping plane.
		//! There are at least 6 clipping planes available for the user to set at will.
		//! \param index: The plane index. Must be between 0 and MaxUserClipPlanes.
		//! \param plane: The plane itself.
		//! \param enable: If true, enable the clipping plane else disable it.
		virtual bool setClipPlane(u32 index, const core::plane3df& plane, bool enable=false);

		//! Enable/disable a clipping plane.
		//! There are at least 6 clipping planes available for the user to set at will.
		//! \param index: The plane index. Must be between 0 and MaxUserClipPlanes.
		//! \param enable: If true, enable the clipping plane else disable it.
		virtual void enableClipPlane(u32 index, bool enable);

		virtual void setAllowZWriteOnTransparent(bool flag)
		{ AllowZWriteOnTransparent=flag; }

	protected:

		//! deletes all textures
		void deleteAllTextures();

		//! opens the file and loads it into the surface
		video::ITexture* loadTextureFromFile(io::IReadFile* file, const c8* hashName = 0);

		//! adds a surface, not loaded or created by the Irrlicht Engine
		void addTexture(video::ITexture* surface);

		//! Creates a texture from a loaded IImage.
		virtual ITexture* addTexture(const c8* name, IImage* image);

		//! returns a device dependent texture from a software surface (IImage)
		//! THIS METHOD HAS TO BE OVERRIDDEN BY DERIVED DRIVERS WITH OWN TEXTURES
		virtual video::ITexture* createDeviceDependentTexture(IImage* surface, const char* name);

		//! checks triangle count and print warning if wrong
		bool checkPrimitiveCount(u32 prmcnt) const;

		// adds a material renderer and drops it afterwards. To be used for internal creation
		s32 addAndDropMaterialRenderer(IMaterialRenderer* m);

		//! deletes all material renderers
		void deleteMaterialRenders();

		// prints renderer version
		void printVersion();

		//! normal map lookup 32 bit version
		inline f32 nml32(int x, int y, int pitch, int height, s32 *p) const
		{
			if (x < 0) x = pitch-1; if (x >= pitch) x = 0;
			if (y < 0) y = height-1; if (y >= height) y = 0;
			return (f32)(((p[(y * pitch) + x])>>16) & 0xff);
		}

		//! normal map lookup 16 bit version
		inline f32 nml16(int x, int y, int pitch, int height, s16 *p) const
		{
			if (x < 0) x = pitch-1; if (x >= pitch) x = 0;
			if (y < 0) y = height-1; if (y >= height) y = 0;
			
			return (f32) getAverage ( p[(y * pitch) + x] );
		}

		struct SSurface
		{
			video::ITexture* Surface;

			bool operator < (const SSurface& other) const
			{
				return Surface->getName() < other.Surface->getName();
			}
		};

		struct SMaterialRenderer
		{
			core::stringc Name;
			IMaterialRenderer* Renderer;
		};

		struct SDummyTexture : public ITexture
		{
			SDummyTexture(const char* name) : ITexture(name), size(0,0) {};

			virtual void* lock() { return 0; };
			virtual void unlock(){}
			virtual const core::dimension2d<s32>& getOriginalSize() const { return size; }
			virtual const core::dimension2d<s32>& getSize() const { return size; }
			virtual E_DRIVER_TYPE getDriverType() const { return video::EDT_NULL; }
			virtual ECOLOR_FORMAT getColorFormat() const { return video::ECF_A1R5G5B5; };
			virtual u32 getPitch() const { return 0; }
			virtual void regenerateMipMapLevels() {};
			core::dimension2d<s32> size;
		};

		core::array<SSurface> Textures;
		core::array<video::IImageLoader*> SurfaceLoader;
		core::array<video::IImageWriter*> SurfaceWriter;
		core::array<SLight> Lights;
		core::array<SMaterialRenderer> MaterialRenderers;

		io::IFileSystem* FileSystem;

		//! mesh manipulator
		scene::IMeshManipulator* MeshManipulator;

		core::rect<s32> ViewPort;
		core::dimension2d<s32> ScreenSize;
		core::matrix4 TransformationMatrix;

		CFPSCounter FPSCounter;

		u32 PrimitivesDrawn;

		u32 TextureCreationFlags;

		f32 FogStart;
		f32 FogEnd;
		f32 FogDensity;
		SColor FogColor;
		bool LinearFog;
		bool PixelFog;
		bool RangeFog;

		bool AllowZWriteOnTransparent;

		SExposedVideoData ExposedData;
	};

} // end namespace video
} // end namespace irr


#endif

