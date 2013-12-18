// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_D3D9_MATERIAL_RENDERER_H_INCLUDED__
#define __C_D3D9_MATERIAL_RENDERER_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_WINDOWS_

#ifdef _IRR_COMPILE_WITH_DIRECT3D_9_
#include <d3d9.h>

#include "IMaterialRenderer.h"

namespace irr
{
namespace video
{

D3DMATRIX UnitMatrixD3D9;
D3DMATRIX SphereMapMatrixD3D9;

//! Base class for all internal D3D9 material renderers
class CD3D9MaterialRenderer : public IMaterialRenderer
{
public:

	//! Constructor
	CD3D9MaterialRenderer(IDirect3DDevice9* d3ddev, video::IVideoDriver* driver)
		: pID3DDevice(d3ddev), Driver(driver)
	{
	}

	~CD3D9MaterialRenderer()
	{
	}

	//! sets a variable in the shader.
	//! \param vertexShader: True if this should be set in the vertex shader, false if
	//! in the pixel shader.
	//! \param name: Name of the variable
	//! \param floats: Pointer to array of floats
	//! \param count: Amount of floats in array.
	virtual bool setVariable(bool vertexShader, const c8* name, const f32* floats, int count)
	{
		os::Printer::log("Invalid material to set variable in.");
		return false;
	}

protected:

	IDirect3DDevice9* pID3DDevice;
	video::IVideoDriver* Driver;
};


//! Solid material renderer
class CD3D9MaterialRenderer_SOLID : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_SOLID(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_ALPHAOP,  D3DTOP_DISABLE);
		}

		pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}
};

//! Generic Texture Blend
class CD3D9MaterialRenderer_ONETEXTURE_BLEND : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_ONETEXTURE_BLEND(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		if (material.MaterialType != lastMaterial.MaterialType ||
			material.MaterialTypeParam != lastMaterial.MaterialTypeParam ||
			resetAllRenderstates)
		{

			E_BLEND_FACTOR srcFact,dstFact;
			E_MODULATE_FUNC modulate;
			unpack_texureBlendFunc ( srcFact, dstFact, modulate, material.MaterialTypeParam );

			if (srcFact == EBF_SRC_COLOR && dstFact == EBF_ZERO)
				pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			else
			{
				pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				pID3DDevice->SetRenderState(D3DRS_SRCBLEND, getD3DBlend ( srcFact ) );
				pID3DDevice->SetRenderState(D3DRS_DESTBLEND, getD3DBlend ( dstFact ) );
			}

			pID3DDevice->SetTextureStageState (0, D3DTSS_COLOROP, getD3DModulate ( modulate ) );
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

			if ( getTexelAlpha ( srcFact ) + getTexelAlpha ( dstFact ) )
			{
				pID3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
				pID3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			}
			else
			{
				pID3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
			}

			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

	}

	private:

		u32 getD3DBlend ( E_BLEND_FACTOR factor ) const
		{
			u32 r = 0;
		switch ( factor )
			{
				case EBF_ZERO:					r = D3DBLEND_ZERO; break;
				case EBF_ONE:					r = D3DBLEND_ONE; break;
				case EBF_DST_COLOR:				r = D3DBLEND_DESTCOLOR; break;
				case EBF_ONE_MINUS_DST_COLOR:	r = D3DBLEND_INVDESTCOLOR; break;
				case EBF_SRC_COLOR:				r = D3DBLEND_SRCCOLOR; break;
				case EBF_ONE_MINUS_SRC_COLOR:	r = D3DBLEND_INVSRCCOLOR; break;
				case EBF_SRC_ALPHA:				r = D3DBLEND_SRCALPHA; break;
				case EBF_ONE_MINUS_SRC_ALPHA:	r = D3DBLEND_INVSRCALPHA; break;
				case EBF_DST_ALPHA:				r = D3DBLEND_DESTALPHA; break;
				case EBF_ONE_MINUS_DST_ALPHA:	r = D3DBLEND_INVDESTALPHA; break;
				case EBF_SRC_ALPHA_SATURATE:	r = D3DBLEND_SRCALPHASAT; break;
			}
			return r;
		}

		u32 getTexelAlpha ( E_BLEND_FACTOR factor ) const
		{
			u32 r = 0;
		switch ( factor )
			{
				case EBF_SRC_ALPHA:				r = 1; break;
				case EBF_ONE_MINUS_SRC_ALPHA:	r = 1; break;
				case EBF_DST_ALPHA:				r = 1; break;
				case EBF_ONE_MINUS_DST_ALPHA:	r = 1; break;
				case EBF_SRC_ALPHA_SATURATE:	r = 1; break;
			}
			return r;
		}

		u32 getD3DModulate ( E_MODULATE_FUNC func ) const
		{
			u32 r = D3DTOP_MODULATE;
			switch ( func )
			{
				case EMFN_MODULATE_1X: r = D3DTOP_MODULATE; break;
				case EMFN_MODULATE_2X: r = D3DTOP_MODULATE2X; break;
				case EMFN_MODULATE_4X: r = D3DTOP_MODULATE4X; break;
			}
			return r;
		}

};



//! Solid 2 layer material renderer
class CD3D9MaterialRenderer_SOLID_2_LAYER : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_SOLID_2_LAYER(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			pID3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			pID3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,  D3DTOP_DISABLE);

			pID3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0);
			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDDIFFUSEALPHA);

			pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}
};


//! Transparent add color material renderer
class CD3D9MaterialRenderer_TRANSPARENT_ADD_COLOR : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_TRANSPARENT_ADD_COLOR(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_ALPHAOP,  D3DTOP_DISABLE);

			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			pID3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			pID3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}

	//! Returns if the material is transparent. The scene management needs to know this
	//! for being able to sort the materials by opaque and transparent.
	virtual bool isTransparent() const
	{
		return true;
	}
};


//! Transparent vertex alpha material renderer
class CD3D9MaterialRenderer_TRANSPARENT_VERTEX_ALPHA : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_TRANSPARENT_VERTEX_ALPHA(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_ALPHAOP,  D3DTOP_SELECTARG1);
			pID3DDevice->SetTextureStageState (0, D3DTSS_ALPHAARG1,  D3DTA_DIFFUSE);

			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			pID3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

			pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			pID3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			pID3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}

	//! Returns if the material is transparent. The scene managment needs to know this
	//! for being able to sort the materials by opaque and transparent.
	virtual bool isTransparent() const
	{
		return true;
	}
};


//! Transparent alpha channel material renderer
class CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates
			|| material.MaterialTypeParam != lastMaterial.MaterialTypeParam )
		{
			pID3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
			pID3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pID3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pID3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
			pID3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

			pID3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
			pID3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

			pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			pID3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			pID3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

			s32 refValue = core::floor32(material.MaterialTypeParam * 255.f);
			if ( !refValue )
				refValue = 127; // default value

			pID3DDevice->SetRenderState(D3DRS_ALPHAREF, refValue);
			pID3DDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
			pID3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);

		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}

	virtual void OnUnsetMaterial()
	{
		pID3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	}

	//! Returns if the material is transparent. The scene managment needs to know this
	//! for being able to sort the materials by opaque and transparent.
	virtual bool isTransparent() const
	{
		return true;
	}
};



//! Transparent alpha channel material renderer
class CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL_REF : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL_REF(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			pID3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
			pID3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pID3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pID3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
			pID3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

			pID3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
			pID3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

			pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

			s32 refValue = core::floor32(material.MaterialTypeParam * 255);
			if ( !refValue )
				refValue = 127; // default value

			pID3DDevice->SetRenderState(D3DRS_ALPHAREF,refValue);
			pID3DDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
			pID3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}

	virtual void OnUnsetMaterial()
	{
		pID3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	}

	//! Returns if the material is transparent. The scene managment needs to know this
	//! for being able to sort the materials by opaque and transparent.
	virtual bool isTransparent() const
	{
		return false; // this material is not really transparent because it does no blending.
	}
};


//! material renderer for all kinds of linghtmaps
class CD3D9MaterialRenderer_LIGHTMAP : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_LIGHTMAP(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			if (material.MaterialType >= EMT_LIGHTMAP_LIGHTING)
			{
				// with lighting
				pID3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				pID3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				pID3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			}
			else
			{
				pID3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
				pID3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			}

			pID3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,  D3DTOP_DISABLE);
			pID3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);

			if (material.MaterialType == EMT_LIGHTMAP_ADD)
				pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD);
			else
			if (material.MaterialType == EMT_LIGHTMAP_M4)
				pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE4X);
			else
			if (material.MaterialType == EMT_LIGHTMAP_M2)
				pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
			else
				pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);

			pID3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
			pID3DDevice->SetTextureStageState (1, D3DTSS_ALPHAOP,  D3DTOP_DISABLE);

			pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}
};



//! material renderer for detail maps
class CD3D9MaterialRenderer_DETAIL_MAP : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_DETAIL_MAP(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_ALPHAOP,  D3DTOP_DISABLE);

			pID3DDevice->SetTextureStageState (1, D3DTSS_COLOROP, D3DTOP_ADDSIGNED);
			pID3DDevice->SetTextureStageState (1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState (1, D3DTSS_COLORARG2, D3DTA_CURRENT);
			pID3DDevice->SetTextureStageState (1, D3DTSS_ALPHAOP,  D3DTOP_DISABLE);

			pID3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
			pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}
};


//! sphere map material renderer
class CD3D9MaterialRenderer_SPHERE_MAP : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_SPHERE_MAP(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_ALPHAOP,  D3DTOP_DISABLE);

			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

			pID3DDevice->SetTransform( D3DTS_TEXTURE0, &SphereMapMatrixD3D9 );
			pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
			pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL );
		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}

	virtual void OnUnsetMaterial()
	{
		pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
		pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0);
		pID3DDevice->SetTransform( D3DTS_TEXTURE0, &UnitMatrixD3D9 );
	}
};


//! reflection 2 layer material renderer
class CD3D9MaterialRenderer_REFLECTION_2_LAYER : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_REFLECTION_2_LAYER(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_ALPHAOP,  D3DTOP_DISABLE);

			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
			pID3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);

			pID3DDevice->SetTransform( D3DTS_TEXTURE1, &SphereMapMatrixD3D9 );
			pID3DDevice->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
			pID3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
			pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}

	virtual void OnUnsetMaterial()
	{
		pID3DDevice->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
		pID3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1);
		pID3DDevice->SetTransform( D3DTS_TEXTURE1, &UnitMatrixD3D9 );
	}
};


//! reflection 2 layer material renderer
class CD3D9MaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_ALPHAOP,  D3DTOP_DISABLE);

			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
			pID3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);

			pID3DDevice->SetTransform( D3DTS_TEXTURE1, &SphereMapMatrixD3D9 );
			pID3DDevice->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
			pID3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
			pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

			pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			pID3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			pID3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}

	virtual void OnUnsetMaterial()
	{
		pID3DDevice->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
		pID3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1);
		pID3DDevice->SetTransform( D3DTS_TEXTURE1, &UnitMatrixD3D9 );
	}

	//! Returns if the material is transparent. The scene managment needs to know this
	//! for being able to sort the materials by opaque and transparent.
	virtual bool isTransparent() const
	{
		return true;
	}
};

} // end namespace video
} // end namespace irr

#endif
#endif
#endif

