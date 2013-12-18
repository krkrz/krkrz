// Copyright (C) 2002-2008 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_BURNINGSVIDEO_

#include "SoftwareDriver2_compile_config.h"
#include "IBurningShader.h"

namespace irr
{

namespace video
{

	const tFixPointu IBurningShader::dithermask[ 4 * 4] =
		{
			0x00,0x80,0x20,0xa0,
			0xc0,0x40,0xe0,0x60,
			0x30,0xb0,0x10,0x90,
			0xf0,0x70,0xd0,0x50
		};

	IBurningShader::IBurningShader(IDepthBuffer* zbuffer)
		:RenderTarget(0),ZBuffer(zbuffer)
	{
		IT[0].Texture = 0;
		IT[1].Texture = 0;

		#ifdef _DEBUG
		setDebugName("CTRTextureLightMap2_M1");
		#endif

		if (ZBuffer)
			zbuffer->grab();
	}


	//! destructor
	IBurningShader::~IBurningShader()
	{
		if (RenderTarget)
			RenderTarget->drop();

		if (ZBuffer)
			ZBuffer->drop();

		if ( IT[0].Texture )
			IT[0].Texture->drop();

		if ( IT[1].Texture )
			IT[1].Texture->drop();
	}

	//! sets a render target
	void IBurningShader::setRenderTarget(video::IImage* surface, const core::rect<s32>& viewPort)
	{
		if (RenderTarget)
			RenderTarget->drop();

		RenderTarget = surface;

		if (RenderTarget)
		{
			SurfaceWidth = RenderTarget->getDimension().Width;
			RenderTarget->grab();
		}
	}


	//! sets the Texture
	void IBurningShader::setTexture( u32 stage, video::CSoftwareTexture2* texture, s32 lodLevel)
	{
		sInternalTexture *it = &IT[stage];

		if ( it->Texture)
			it->Texture->drop();

		it->Texture = texture;

		if ( it->Texture)
		{
			it->Texture->grab();

			// select mignify and magnify ( lodLevel )
			//SOFTWARE_DRIVER_2_MIPMAPPING_LOD_BIAS
			it->lodLevel = lodLevel;
			it->Texture->setCurrentMipMapLOD (
				core::s32_clamp ( lodLevel + SOFTWARE_DRIVER_2_MIPMAPPING_LOD_BIAS, 0, SOFTWARE_DRIVER_2_MIPMAPPING_MAX - 1 )
				);

			// prepare for optimal fixpoint
			it->pitchlog2 = s32_log2_s32 ( it->Texture->getPitch() );

			it->textureXMask = s32_to_fixPoint ( it->Texture->getSize().Width - 1 ) & FIX_POINT_UNSIGNED_MASK;
			it->textureYMask = s32_to_fixPoint ( it->Texture->getSize().Height - 1 ) & FIX_POINT_UNSIGNED_MASK;
		}
	}


} // end namespace video
} // end namespace irr

#endif // _IRR_COMPILE_WITH_BURNINGSVIDEO_
