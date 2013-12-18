// Copyright (C) 2002-2008 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_VIDEO_2_SOFTWARE_H_INCLUDED__
#define __C_VIDEO_2_SOFTWARE_H_INCLUDED__

#include "SoftwareDriver2_compile_config.h"
#include "IBurningShader.h"
#include "CNullDriver.h"
#include "CImage.h"
#include "os.h"
#include "irrString.h"

namespace irr
{
namespace video
{
	class CSoftwareDriver2 : public CNullDriver
	{
	public:

		//! constructor
		CSoftwareDriver2(const core::dimension2d<s32>& windowSize, bool fullscreen, io::IFileSystem* io, video::IImagePresenter* presenter);

		//! destructor
		virtual ~CSoftwareDriver2();

		//! presents the rendered scene on the screen, returns false if failed
		virtual bool endScene( s32 windowId = 0, core::rect<s32>* sourceRect=0, core::rect<s32>* destRect=0, void* destDC=0 );

		//! queries the features of the driver, returns true if feature is available
		virtual bool queryFeature(E_VIDEO_DRIVER_FEATURE feature) const;

		//! sets transformation
		virtual void setTransform(E_TRANSFORMATION_STATE state, const core::matrix4& mat);

		//! sets a material
		virtual void setMaterial(const SMaterial& material);

		virtual bool setRenderTarget(video::ITexture* texture, bool clearBackBuffer,
						bool clearZBuffer, SColor color);

		//! sets a viewport
		virtual void setViewPort(const core::rect<s32>& area);

		//! clears the zbuffer
		virtual bool beginScene(bool backBuffer, bool zBuffer, SColor color);

		//! Only used by the internal engine. Used to notify the driver that
		//! the window was resized.
		virtual void OnResize(const core::dimension2d<s32>& size);

		//! returns size of the current render target
		virtual const core::dimension2d<s32>& getCurrentRenderTargetSize() const;

		//! deletes all dynamic lights there are
		virtual void deleteAllDynamicLights();

		//! adds a dynamic light
		virtual void addDynamicLight(const SLight& light);

		//! returns the maximal amount of dynamic lights the device can handle
		virtual u32 getMaximalDynamicLightAmount() const;

		//! Sets the dynamic ambient light color. The default color is
		//! (0,0,0,0) which means it is dark.
		//! \param color: New color of the ambient light.
		virtual void setAmbientLight(const SColorf& color);

		//! draws a vertex primitive list
		void drawVertexPrimitiveList(const void* vertices, u32 vertexCount,
				const u16* indexList, u32 primitiveCount,
				E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType);

		//! draws an 2d image, using a color (if color is other then Color(255,255,255,255)) and the alpha channel of the texture if wanted.
		virtual void draw2DImage(const video::ITexture* texture, const core::position2d<s32>& destPos,
			const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect = 0,
			SColor color=SColor(255,255,255,255), bool useAlphaChannelOfTexture=false);

		//! Draws a 3d line.
		virtual void draw3DLine(const core::vector3df& start,
			const core::vector3df& end, SColor color = SColor(255,255,255,255));

		//! draw an 2d rectangle
		virtual void draw2DRectangle(SColor color, const core::rect<s32>& pos,
			const core::rect<s32>* clip = 0);

		//!Draws an 2d rectangle with a gradient.
		virtual void draw2DRectangle(const core::rect<s32>& pos,
			SColor colorLeftUp, SColor colorRightUp, SColor colorLeftDown, SColor colorRightDown,
			const core::rect<s32>* clip = 0);

		//! Draws a 2d line.
		virtual void draw2DLine(const core::position2d<s32>& start,
					const core::position2d<s32>& end,
					SColor color=SColor(255,255,255,255));

		//! \return Returns the name of the video driver. Example: In case of the DirectX8
		//! driver, it would return "Direct3D8.1".
		virtual const wchar_t* getName() const;

		//! Returns type of video driver
		virtual E_DRIVER_TYPE getDriverType() const;

		//! Returns the transformation set by setTransform
		virtual const core::matrix4& getTransform(E_TRANSFORMATION_STATE state) const;

		//! Creates a render target texture.
		virtual ITexture* createRenderTargetTexture(const core::dimension2d<s32>& size, const c8* name);

		//! Clears the DepthBuffer.
		virtual void clearZBuffer();

		//! Returns an image created from the last rendered frame.
		virtual IImage* createScreenShot();

		//! Returns the maximum amount of primitives (mostly vertices) which
		//! the device is able to render with one drawIndexedTriangleList
		//! call.
		virtual u32 getMaximalPrimitiveCount() const;

	protected:

		//! sets a render target
		void setRenderTarget(video::CImage* image);

		//! sets the current Texture
		bool setTexture(u32 stage, video::ITexture* texture);

		//! returns a device dependent texture from a software surface (IImage)
		//! THIS METHOD HAS TO BE OVERRIDDEN BY DERIVED DRIVERS WITH OWN TEXTURES
		virtual video::ITexture* createDeviceDependentTexture(IImage* surface, const char* name);

		video::CImage* BackBuffer;
		video::IImagePresenter* Presenter;

		video::ITexture* RenderTargetTexture;
		video::IImage* RenderTargetSurface;
		core::dimension2d<s32> RenderTargetSize;

		//! selects the right triangle renderer based on the render states.
		void setCurrentShader();

		IBurningShader* CurrentShader;
		IBurningShader* BurningShader[ETR2_COUNT];

		IDepthBuffer* DepthBuffer;

		video::ITexture* Texture[2];
		sInternalTexture Texmap[2];

		/*
			extend Matrix Stack
			-> combined CameraProjection
			-> combined CameraProjectionWorld
			-> ClipScale from NDC to DC Space
		*/
		enum E_TRANSFORMATION_STATE_2
		{
			ETS_VIEW_PROJECTION = ETS_COUNT,
			ETS_WORLD_VIEW,
			ETS_WORLD_VIEW_INVERSE_TRANSPOSED,
			ETS_CURRENT,
			ETS_CLIPSCALE,

			ETS2_COUNT
		};

		struct SMatrixStack
		{
			s32 isIdentity;
			core::matrix4 m;
		};

		SMatrixStack Transformation[ETS2_COUNT];

		// Vertex Cache
		static const SVSize vSize[];

		SVertexCache VertexCache;

		void VertexCache_reset (const void* vertices, u32 vertexCount,
					const u16* indices, u32 indexCount,
					E_VERTEX_TYPE vType,scene::E_PRIMITIVE_TYPE pType);
		void VertexCache_get ( s4DVertex ** face );
		void VertexCache_get2 ( s4DVertex ** face );

		void VertexCache_fill ( const u32 sourceIndex,const u32 destIndex );
		s4DVertex * VertexCache_getVertex ( const u32 sourceIndex );


		// culling & clipping
		u32 clipToHyperPlane ( s4DVertex * dest, const s4DVertex * source, u32 inCount, const sVec4 &plane );
		u32 clipToFrustumTest ( const s4DVertex * v  ) const;
		u32 clipToFrustum ( s4DVertex *source, s4DVertex * temp, const u32 vIn );


#ifdef SOFTWARE_DRIVER_2_LIGHTING
		void lightVertex ( s4DVertex *dest, const S3DVertex *source );
#endif


		// holds transformed, clipped vertices
		SAlignedVertex CurrentOut;
		SAlignedVertex Temp;

		void ndc_2_dc_and_project ( s4DVertex *dest,s4DVertex *source, u32 vIn ) const;
		f32 screenarea ( const s4DVertex *v0 ) const;
		void select_polygon_mipmap ( s4DVertex *source, u32 vIn, s32 tex );
		f32 texelarea ( const s4DVertex *v0, int tex ) const;


		void ndc_2_dc_and_project2 ( const s4DVertex **v, const u32 size ) const;
		f32 screenarea2 ( const s4DVertex **v ) const;
		f32 texelarea2 ( const s4DVertex **v, int tex ) const;
		void select_polygon_mipmap2 ( s4DVertex **source, s32 tex ) const;


		sVec4 Global_AmbientLight;

		struct SInternalLight
		{
			SLight org;

			sVec4 posEyeSpace;

			f32 constantAttenuation;
			f32 linearAttenuation;
			f32 quadraticAttenuation;

			sVec4 AmbientColor;
			sVec4 DiffuseColor;
			sVec4 SpecularColor;
		};
		core::array<SInternalLight> Light;

		struct SInternalMaterial
		{
			SMaterial org;

			sVec4 AmbientColor;
			sVec4 DiffuseColor;
			sVec4 SpecularColor;
			sVec4 EmissiveColor;

			u32 SpecularEnabled;	// == Power2
		};

		SInternalMaterial Material;

		static const sVec4 NDCPlane[6];

	};

} // end namespace video
} // end namespace irr


#endif

