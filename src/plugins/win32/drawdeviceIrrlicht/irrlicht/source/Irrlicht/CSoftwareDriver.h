// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_VIDEO_SOFTWARE_H_INCLUDED__
#define __C_VIDEO_SOFTWARE_H_INCLUDED__

#include "ITriangleRenderer.h"
#include "CNullDriver.h"
#include "SViewFrustum.h"
#include "CImage.h"

namespace irr
{
namespace video
{
	class CSoftwareDriver : public CNullDriver
	{
	public:

		//! constructor
		CSoftwareDriver(const core::dimension2d<s32>& windowSize, bool fullscreen, io::IFileSystem* io, video::IImagePresenter* presenter);

		//! destructor
		virtual ~CSoftwareDriver();

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

		//! draws a vertex primitive list
		void drawVertexPrimitiveList(const void* vertices, u32 vertexCount,
				const u16* indexList, u32 primitiveCount,
				E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType);

		//! Draws a 3d line.
		virtual void draw3DLine(const core::vector3df& start,
			const core::vector3df& end, SColor color = SColor(255,255,255,255));

		//! draws an 2d image, using a color (if color is other then Color(255,255,255,255)) and the alpha channel of the texture if wanted.
		virtual void draw2DImage(const video::ITexture* texture, const core::position2d<s32>& destPos,
			const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect = 0,
			SColor color=SColor(255,255,255,255), bool useAlphaChannelOfTexture=false);

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

		//! \return Returns the name of the video driver. Example: In case of the Direct3D8
		//! driver, it would return "Direct3D8.1".
		virtual const wchar_t* getName() const;

		//! Returns type of video driver
		virtual E_DRIVER_TYPE getDriverType() const;

		//! Returns the transformation set by setTransform
		virtual const core::matrix4& getTransform(E_TRANSFORMATION_STATE state) const;

		//! Creates a render target texture.
		virtual ITexture* createRenderTargetTexture(const core::dimension2d<s32>& size, const c8* name);

		//! Clears the ZBuffer.
		virtual void clearZBuffer();

		//! Returns an image created from the last rendered frame.
		virtual IImage* createScreenShot();

		//! Returns the maximum amount of primitives (mostly vertices) which
		//! the device is able to render with one drawIndexedTriangleList
		//! call.
		virtual u32 getMaximalPrimitiveCount() const;

	protected:

		struct splane
		{
			core::vector3df Normal;
			f32 Dist;
		};

		//! sets a render target
		void setRenderTarget(video::CImage* image);

		//! sets the current Texture
		bool setTexture(video::ITexture* texture);

		video::CImage* BackBuffer;
		video::IImagePresenter* Presenter;

		//! switches to a triangle renderer
		void switchToTriangleRenderer(ETriangleRenderer renderer);

		//! void selects the right triangle renderer based on the render states.
		void selectRightTriangleRenderer();

		//! clips a triangle agains the viewing frustum
		void clipTriangle(f32* transformedPos);

		//! creates the clipping planes from the view matrix
		void createPlanes(const core::matrix4& mat);

		template<class VERTEXTYPE>
		void drawClippedIndexedTriangleListT(const VERTEXTYPE* vertices,
			s32 vertexCount, const u16* indexList, s32 triangleCount);

		core::array<S2DVertex> TransformedPoints;

		video::ITexture* RenderTargetTexture;
		video::IImage* RenderTargetSurface;
		core::position2d<s32> Render2DTranslation;
		core::dimension2d<s32> RenderTargetSize;
		core::dimension2d<s32> ViewPortSize;

		core::matrix4 TransformationMatrix[ETS_COUNT];

		ITriangleRenderer* CurrentTriangleRenderer;
		ITriangleRenderer* TriangleRenderers[ETR_COUNT];
		ETriangleRenderer CurrentRenderer;

		IZBuffer* ZBuffer;

		video::ITexture* Texture;
		scene::SViewFrustum Frustum;

		SMaterial Material;

		splane planes[6]; // current planes of the view frustum
	};

} // end namespace video
} // end namespace irr


#endif

