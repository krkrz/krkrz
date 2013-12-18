// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __E_DRIVER_FEATURES_H_INCLUDED__
#define __E_DRIVER_FEATURES_H_INCLUDED__

namespace irr
{
namespace video
{

	//! enumeration for querying features of the video driver.
	enum E_VIDEO_DRIVER_FEATURE
	{
		//! Is driver able to render to a surface?
		EVDF_RENDER_TO_TARGET = 0,

		//! Is hardeware transform and lighting supported?
		EVDF_HARDWARE_TL,

		//! Are multiple textures per material possible?
		EVDF_MULTITEXTURE,

		//! Is driver able to render with a bilinear filter applied?
		EVDF_BILINEAR_FILTER,

		//! Can the driver handle mip maps?
		EVDF_MIP_MAP,

		//! Can the driver update mip maps automatically?
		EVDF_MIP_MAP_AUTO_UPDATE,

		//! Are stencilbuffers switched on and does the device support stencil buffers?
		EVDF_STENCIL_BUFFER,

		//! Is Vertex Shader 1.1 supported?
		EVDF_VERTEX_SHADER_1_1,

		//! Is Vertex Shader 2.0 supported?
		EVDF_VERTEX_SHADER_2_0,

		//! Is Vertex Shader 3.0 supported?
		EVDF_VERTEX_SHADER_3_0,

		//! Is Pixel Shader 1.1 supported?
		EVDF_PIXEL_SHADER_1_1,

		//! Is Pixel Shader 1.2 supported?
		EVDF_PIXEL_SHADER_1_2,

		//! Is Pixel Shader 1.3 supported?
		EVDF_PIXEL_SHADER_1_3,

		//! Is Pixel Shader 1.4 supported?
		EVDF_PIXEL_SHADER_1_4,

		//! Is Pixel Shader 2.0 supported?
		EVDF_PIXEL_SHADER_2_0,

		//! Is Pixel Shader 3.0 supported?
		EVDF_PIXEL_SHADER_3_0,

		//! Are ARB vertex programs v1.0 supported?
		EVDF_ARB_VERTEX_PROGRAM_1,

		//! Are ARB fragment programs v1.0 supported?
		EVDF_ARB_FRAGMENT_PROGRAM_1,

		//! Is GLSL supported?
		EVDF_ARB_GLSL,

		//! Is HLSL supported?
		EVDF_HLSL,

		//! Are non-power-of-two textures supported?
		EVDF_TEXTURE_NPOT,

		//! Are framebuffer objects supported?
		EVDF_FRAMEBUFFER_OBJECT,

		//! Only used for counting the elements of this enum
		EVDF_COUNT
	};

} // end namespace video
} // end namespace irr


#endif

