// Copyright (C) 2002-2008 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CSoftwareDriver2.h"

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_BURNINGSVIDEO_

#include "SoftwareDriver2_helper.h"
#include "CSoftwareTexture2.h"
#include "CSoftware2MaterialRenderer.h"
#include "S3DVertex.h"
#include "S4DVertex.h"


namespace irr
{
namespace video
{


//! constructor
CSoftwareDriver2::CSoftwareDriver2(const core::dimension2d<s32>& windowSize, bool fullscreen, io::IFileSystem* io, video::IImagePresenter* presenter)
: CNullDriver(io, windowSize), BackBuffer(0), Presenter(presenter),
	RenderTargetTexture(0), RenderTargetSurface(0), CurrentShader(0),
	 DepthBuffer(0), CurrentOut ( 12 * 2, 128 ), Temp ( 12 * 2, 128 )
{
	#ifdef _DEBUG
	setDebugName("CSoftwareDriver2");
	#endif

	Texture[0] = 0;
	Texture[1] = 0;
	Texmap[0].Texture = 0;
	Texmap[1].Texture = 0;

	// create backbuffer
	BackBuffer = new CImage(ECF_SOFTWARE2, windowSize);
	BackBuffer->fill(SColor(0));
	
	// create z buffer

	DepthBuffer = video::createDepthBuffer(BackBuffer->getDimension());

	// create triangle renderers

	irr::memset32 ( BurningShader, 0, sizeof ( BurningShader ) );
	//BurningShader[ETR_FLAT] = createTRFlat2(DepthBuffer);
	//BurningShader[ETR_FLAT_WIRE] = createTRFlatWire2(DepthBuffer);
	BurningShader[ETR_GOURAUD] = createTriangleRendererGouraud2(DepthBuffer);
	BurningShader[ETR_GOURAUD_ALPHA] = createTriangleRendererGouraudAlpha2(DepthBuffer );
	BurningShader[ETR_GOURAUD_ALPHA_NOZ] = createTRGouraudAlphaNoZ2(DepthBuffer );
	//BurningShader[ETR_GOURAUD_WIRE] = createTriangleRendererGouraudWire2(DepthBuffer);
	//BurningShader[ETR_TEXTURE_FLAT] = createTriangleRendererTextureFlat2(DepthBuffer);
	//BurningShader[ETR_TEXTURE_FLAT_WIRE] = createTriangleRendererTextureFlatWire2(DepthBuffer);
	BurningShader[ETR_TEXTURE_GOURAUD] = createTriangleRendererTextureGouraud2(DepthBuffer);
	BurningShader[ETR_TEXTURE_GOURAUD_LIGHTMAP] = createTriangleRendererTextureLightMap2_M1(DepthBuffer);
	BurningShader[ETR_TEXTURE_GOURAUD_LIGHTMAP_M2] = createTriangleRendererTextureLightMap2_M2(DepthBuffer);
	BurningShader[ETR_TEXTURE_GOURAUD_LIGHTMAP_M4] = createTriangleRendererGTextureLightMap2_M4(DepthBuffer);
	BurningShader[ETR_TEXTURE_LIGHTMAP_M4] = createTriangleRendererTextureLightMap2_M4(DepthBuffer);
	BurningShader[ETR_TEXTURE_GOURAUD_LIGHTMAP_ADD] = createTriangleRendererTextureLightMap2_Add(DepthBuffer);
	BurningShader[ETR_TEXTURE_GOURAUD_DETAIL_MAP] = createTriangleRendererTextureDetailMap2(DepthBuffer);

	BurningShader[ETR_TEXTURE_GOURAUD_WIRE] = createTriangleRendererTextureGouraudWire2(DepthBuffer);
	BurningShader[ETR_TEXTURE_GOURAUD_NOZ] = createTRTextureGouraudNoZ2();
	BurningShader[ETR_TEXTURE_GOURAUD_ADD] = createTRTextureGouraudAdd2(DepthBuffer);
	BurningShader[ETR_TEXTURE_GOURAUD_ADD_NO_Z] = createTRTextureGouraudAddNoZ2(DepthBuffer);
	BurningShader[ETR_TEXTURE_GOURAUD_VERTEX_ALPHA] = createTriangleRendererTextureVertexAlpha2 ( DepthBuffer );

	BurningShader[ETR_TEXTURE_GOURAUD_ALPHA] = createTRTextureGouraudAlpha(DepthBuffer );
	BurningShader[ETR_TEXTURE_GOURAUD_ALPHA_NOZ] = createTRTextureGouraudAlphaNoZ( DepthBuffer );

	BurningShader[ETR_TEXTURE_BLEND] = createTRTextureBlend( DepthBuffer );


	// add the same renderer for all solid types
	CSoftware2MaterialRenderer_SOLID* smr = new CSoftware2MaterialRenderer_SOLID( this);
	CSoftware2MaterialRenderer_TRANSPARENT_ADD_COLOR* tmr = new CSoftware2MaterialRenderer_TRANSPARENT_ADD_COLOR( this);
	CSoftware2MaterialRenderer_UNSUPPORTED * umr = new CSoftware2MaterialRenderer_UNSUPPORTED ( this );

	//!TODO: addMaterialRenderer depends on pushing order....
	addMaterialRenderer ( smr ); // EMT_SOLID
	addMaterialRenderer ( smr ); // EMT_SOLID_2_LAYER,
	addMaterialRenderer ( smr ); // EMT_LIGHTMAP,
	addMaterialRenderer ( tmr ); // EMT_LIGHTMAP_ADD,
	addMaterialRenderer ( smr ); // EMT_LIGHTMAP_M2,
	addMaterialRenderer ( smr ); // EMT_LIGHTMAP_M4,
	addMaterialRenderer ( smr ); // EMT_LIGHTMAP_LIGHTING,
	addMaterialRenderer ( smr ); // EMT_LIGHTMAP_LIGHTING_M2,
	addMaterialRenderer ( smr ); // EMT_LIGHTMAP_LIGHTING_M4,
	addMaterialRenderer ( smr ); // EMT_DETAIL_MAP,
	addMaterialRenderer ( umr ); // EMT_SPHERE_MAP,
	addMaterialRenderer ( smr ); // EMT_REFLECTION_2_LAYER,
	addMaterialRenderer ( tmr ); // EMT_TRANSPARENT_ADD_COLOR,
	addMaterialRenderer ( tmr ); // EMT_TRANSPARENT_ALPHA_CHANNEL,	
	addMaterialRenderer ( tmr ); // EMT_TRANSPARENT_ALPHA_CHANNEL_REF,	
	addMaterialRenderer ( tmr ); // EMT_TRANSPARENT_VERTEX_ALPHA,
	addMaterialRenderer ( smr ); // EMT_TRANSPARENT_REFLECTION_2_LAYER,
	addMaterialRenderer ( umr ); // EMT_NORMAL_MAP_SOLID,
	addMaterialRenderer ( umr ); // EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR,
	addMaterialRenderer ( umr ); // EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA,
	addMaterialRenderer ( umr ); // EMT_PARALLAX_MAP_SOLID,
	addMaterialRenderer ( umr ); // EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR,
	addMaterialRenderer ( umr ); // EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA,
	addMaterialRenderer ( tmr ); // EMT_ONETEXTURE_BLEND

	smr->drop ();
	tmr->drop ();
	umr->drop ();


	// select render target

	setRenderTarget(BackBuffer);


	Global_AmbientLight.set ( 0.f, 0.f, 0.f, 0.f );

	// select the right renderer
	setCurrentShader();

}



//! destructor
CSoftwareDriver2::~CSoftwareDriver2()
{
	// delete Backbuffer
	BackBuffer->drop();

	// delete triangle renderers

	for (s32 i=0; i<ETR2_COUNT; ++i)
		if (BurningShader[i])
			BurningShader[i]->drop();

	// delete zbuffer

	if (DepthBuffer)
		DepthBuffer->drop();

	// delete current texture

	if ( Texture[0] )
		Texture[0]->drop();

	if ( Texture[1] )
		Texture[1]->drop();

	if (RenderTargetTexture)
		RenderTargetTexture->drop();

	if (RenderTargetSurface)
		RenderTargetSurface->drop();
}



//! void selects the right triangle renderer based on the render states.
void CSoftwareDriver2::setCurrentShader()
{
	EBurningFFShader shader = ETR_TEXTURE_GOURAUD;

	bool zMaterialTest = true;
	switch ( Material.org.MaterialType )
	{
		case EMT_ONETEXTURE_BLEND:
			shader = ETR_TEXTURE_BLEND;
			zMaterialTest = false;
			break;

		case EMT_TRANSPARENT_ALPHA_CHANNEL_REF:
		case EMT_TRANSPARENT_ALPHA_CHANNEL:
			if ( Material.org.ZBuffer )
			{
				shader = ETR_TEXTURE_GOURAUD_ALPHA;
			}
			else
			{
				shader = ETR_TEXTURE_GOURAUD_ALPHA_NOZ;
			}
			zMaterialTest = false;
			break;

		case EMT_TRANSPARENT_ADD_COLOR:
			if ( Material.org.ZBuffer )
			{
				shader = ETR_TEXTURE_GOURAUD_ADD;
			}
			else
			{
				shader = ETR_TEXTURE_GOURAUD_ADD_NO_Z;
			}
			zMaterialTest = false;
			break;

		case EMT_TRANSPARENT_VERTEX_ALPHA:
			shader = ETR_TEXTURE_GOURAUD_VERTEX_ALPHA;
			break;

		case EMT_LIGHTMAP:
		case EMT_LIGHTMAP_LIGHTING:
			shader = ETR_TEXTURE_GOURAUD_LIGHTMAP;
			break;

		case EMT_LIGHTMAP_M2:
		case EMT_LIGHTMAP_LIGHTING_M2:
			shader = ETR_TEXTURE_GOURAUD_LIGHTMAP_M2;
			break;

		case EMT_LIGHTMAP_LIGHTING_M4:
			if ( Material.org.getTexture(1) )
				shader = ETR_TEXTURE_GOURAUD_LIGHTMAP_M4;
			break;
		case EMT_LIGHTMAP_M4:
			if ( Material.org.getTexture(1) )
				shader = ETR_TEXTURE_LIGHTMAP_M4;
			break;

		case EMT_LIGHTMAP_ADD:
			if ( Material.org.getTexture(1) )
				shader = ETR_TEXTURE_GOURAUD_LIGHTMAP_ADD;
			break;

		case EMT_DETAIL_MAP:
			shader = ETR_TEXTURE_GOURAUD_DETAIL_MAP;
			break;

		default:
			break;

	}

	if ( zMaterialTest && !Material.org.ZBuffer && !Material.org.ZWriteEnable)
	{
		shader = ETR_TEXTURE_GOURAUD_NOZ;
	}

	if ( 0 == Material.org.getTexture(0) )
	{
		shader = ETR_GOURAUD;
	}

	if ( Material.org.Wireframe )
	{
		shader = ETR_TEXTURE_GOURAUD_WIRE;
	}

	// switchToTriangleRenderer
	CurrentShader = BurningShader[shader];
	if ( CurrentShader )
	{
		CurrentShader->setZCompareFunc ( Material.org.ZBuffer );
		switch ( shader )
		{
			case ETR_TEXTURE_GOURAUD_ALPHA:
			case ETR_TEXTURE_GOURAUD_ALPHA_NOZ:
				CurrentShader->setParam ( 0, Material.org.MaterialTypeParam );
				break;

			case EMT_ONETEXTURE_BLEND:
			{
				E_BLEND_FACTOR srcFact,dstFact;
				E_MODULATE_FUNC modulate;
				unpack_texureBlendFunc ( srcFact, dstFact, modulate, Material.org.MaterialTypeParam );
				CurrentShader->setParam ( 0, Material.org.MaterialTypeParam );
			}
			break;
			default:
			break;
		}

		CurrentShader->setRenderTarget(RenderTargetSurface, ViewPort);
	}
}



//! queries the features of the driver, returns true if feature is available
bool CSoftwareDriver2::queryFeature(E_VIDEO_DRIVER_FEATURE feature) const
{
	switch (feature)
	{
#ifdef SOFTWARE_DRIVER_2_BILINEAR
	case EVDF_BILINEAR_FILTER:
		return true;
#endif
#ifdef SOFTWARE_DRIVER_2_MIPMAPPING
	case EVDF_MIP_MAP:
		return true;
#endif
	case EVDF_RENDER_TO_TARGET:
	case EVDF_MULTITEXTURE:
	case EVDF_HARDWARE_TL:
		return true;

	default:
		return false;
	}
}



//! sets transformation
void CSoftwareDriver2::setTransform(E_TRANSFORMATION_STATE state, const core::matrix4& mat)
{
	Transformation[state].m = mat;
	Transformation[state].isIdentity = mat.isIdentity();

	switch ( state )
	{
		case ETS_VIEW:
			Transformation[ETS_VIEW_PROJECTION].m.setbyproduct_nocheck (
				Transformation[ETS_PROJECTION].m,
				Transformation[ETS_VIEW].m
			);
			break;

		case ETS_WORLD:
			if ( Transformation[state].isIdentity )
			{
				Transformation[ETS_CURRENT] = Transformation[ETS_VIEW_PROJECTION];
			}
			else
			{
				Transformation[ETS_CURRENT].m.setbyproduct_nocheck (
					Transformation[ETS_VIEW_PROJECTION].m,
					Transformation[ETS_WORLD].m
				);
			}
			Transformation[ETS_CURRENT].isIdentity = 0;

#ifdef SOFTWARE_DRIVER_2_LIGHTING
			if ( Material.org.Lighting )
			{
				if ( Transformation[state].isIdentity )
				{
					Transformation[ETS_WORLD_VIEW] = Transformation[ETS_VIEW];
				}
				else
				{
					Transformation[ETS_WORLD_VIEW].m.setbyproduct_nocheck (
						Transformation[ETS_VIEW].m,
						Transformation[ETS_WORLD].m
					);
				}

				core::matrix4 m2 ( Transformation[ETS_WORLD_VIEW].m );
				m2.makeInverse ();
				m2.getTransposed ( Transformation[ETS_WORLD_VIEW_INVERSE_TRANSPOSED].m );
			}
#endif
			break;
		default:
			break;
	}
}



//! sets the current Texture
bool CSoftwareDriver2::setTexture(u32 stage, video::ITexture* texture)
{
	if (texture && texture->getDriverType() != EDT_BURNINGSVIDEO)
	{
		os::Printer::log("Fatal Error: Tried to set a texture not owned by this driver.", ELL_ERROR);
		return false;
	}

	if (Texture[stage])
		Texture[stage]->drop();

	Texture[stage] = texture;

	if (Texture[stage])
		Texture[stage]->grab();

	if (Texture[stage])
		Texmap[stage].Texture = (video::CSoftwareTexture2*) Texture[stage];

	setCurrentShader();
	return true;
}



//! sets a material
void CSoftwareDriver2::setMaterial(const SMaterial& material)
{
	Material.org = material;

	Material.AmbientColor.setA8R8G8B8 ( Material.org.AmbientColor.color );
	Material.DiffuseColor.setA8R8G8B8 ( Material.org.DiffuseColor.color );
	Material.EmissiveColor.setA8R8G8B8 ( Material.org.EmissiveColor.color );
	Material.SpecularColor.setA8R8G8B8 ( Material.org.SpecularColor.color );

	Material.SpecularEnabled = Material.org.Shininess != 0.f;
	if (Material.SpecularEnabled)
		Material.org.NormalizeNormals = true;

	for (u32 i = 0; i < 2; ++i)
	{
		setTexture( i, Material.org.getTexture(i) );
		setTransform((E_TRANSFORMATION_STATE) (ETS_TEXTURE_0 + i), 
				material.getTextureMatrix(i));
	}
}



//! clears the zbuffer
bool CSoftwareDriver2::beginScene(bool backBuffer, bool zBuffer, SColor color)
{

	CNullDriver::beginScene(backBuffer, zBuffer, color);

	if (backBuffer)
		BackBuffer->fill( color );

	if (DepthBuffer && zBuffer)
		DepthBuffer->clear();

	return true;
}

//! presents the rendered scene on the screen, returns false if failed
bool CSoftwareDriver2::endScene( s32 windowId, core::rect<s32>* sourceRect, core::rect<s32>* destRect, void* destDC)
{
	CNullDriver::endScene();

	Presenter->present(BackBuffer, windowId, sourceRect, destRect );

	return true;
}




//! sets a render target
bool CSoftwareDriver2::setRenderTarget(video::ITexture* texture, bool clearBackBuffer, 
								 bool clearZBuffer, SColor color)
{
	if (texture && texture->getDriverType() != EDT_BURNINGSVIDEO)
	{
		os::Printer::log("Fatal Error: Tried to set a texture not owned by this driver.", ELL_ERROR);
		return false;
	}

	if (RenderTargetTexture)
		RenderTargetTexture->drop();

	RenderTargetTexture = texture;

	if (RenderTargetTexture)
	{
		RenderTargetTexture->grab();
		setRenderTarget(((CSoftwareTexture2*)RenderTargetTexture)->getTexture());
	}
	else
	{
		setRenderTarget(BackBuffer);
	}

	if (RenderTargetSurface && (clearBackBuffer || clearZBuffer))
	{
		if (clearZBuffer)
			DepthBuffer->clear();

		if (clearBackBuffer)
			((video::CImage*)RenderTargetSurface)->fill( color );
	}

	return true;
}


//! sets a render target
void CSoftwareDriver2::setRenderTarget(video::CImage* image)
{
	if (RenderTargetSurface)
		RenderTargetSurface->drop();

	RenderTargetSurface = image;
	RenderTargetSize.Width = 0;
	RenderTargetSize.Height = 0;

	if (RenderTargetSurface)
	{
		RenderTargetSurface->grab();
		RenderTargetSize = RenderTargetSurface->getDimension();
	}

	setViewPort(core::rect<s32>(0,0,RenderTargetSize.Width,RenderTargetSize.Height));

	if (DepthBuffer)
		DepthBuffer->setSize(RenderTargetSize);
}



//! sets a viewport
void CSoftwareDriver2::setViewPort(const core::rect<s32>& area)
{
	ViewPort = area;

	core::rect<s32> rendert(0,0,RenderTargetSize.Width,RenderTargetSize.Height);
	ViewPort.clipAgainst(rendert);

	Transformation [ ETS_CLIPSCALE ].m.buildNDCToDCMatrix ( ViewPort, 1 );


	if (CurrentShader)
		CurrentShader->setRenderTarget(RenderTargetSurface, ViewPort);
}

/*
	generic plane clipping in homogenous coordinates
	special case ndc frustum <-w,w>,<-w,w>,<-w,w>
	can be rewritten with compares e.q near plane, a.z < -a.w and b.z < -b.w
*/

const sVec4 CSoftwareDriver2::NDCPlane[6] =
{
	sVec4(  0.f,  0.f, -1.f, -1.f ),	// near
	sVec4(  0.f,  0.f,  1.f, -1.f ),	// far
	sVec4(  1.f,  0.f,  0.f, -1.f ),	// left
	sVec4( -1.f,  0.f,  0.f, -1.f ),	// right
	sVec4(  0.f,  1.f,  0.f, -1.f ),	// bottom
	sVec4(  0.f, -1.f,  0.f, -1.f )		// top
};



/*
	test a vertex if it's inside the standard frustum

	this is the generic one..

	f32 dotPlane;
	for ( u32 i = 0; i!= 6; ++i )
	{
		dotPlane = v->Pos.dotProduct ( NDCPlane[i] );
		core::setbit_cond( flag, dotPlane <= 0.f, 1 << i );
	}

	// this is the base for ndc frustum <-w,w>,<-w,w>,<-w,w>
	core::setbit_cond( flag, ( v->Pos.z - v->Pos.w ) <= 0.f, 1 );
	core::setbit_cond( flag, (-v->Pos.z - v->Pos.w ) <= 0.f, 2 );
	core::setbit_cond( flag, ( v->Pos.x - v->Pos.w ) <= 0.f, 4 );
	core::setbit_cond( flag, (-v->Pos.x - v->Pos.w ) <= 0.f, 8 );
	core::setbit_cond( flag, ( v->Pos.y - v->Pos.w ) <= 0.f, 16 );
	core::setbit_cond( flag, (-v->Pos.y - v->Pos.w ) <= 0.f, 32 );

*/
#ifdef _MSC_VER

REALINLINE u32 CSoftwareDriver2::clipToFrustumTest ( const s4DVertex * v  ) const
{
	f32 test[6];
	u32 flag;
	const f32 w = - v->Pos.w;

	// a conditional move is needed....FCOMI ( but we don't have it )
	// so let the fpu calculate and write it back.
	// cpu makes the compare, interleaving

	test[0] =  v->Pos.z + w;
	test[1] = -v->Pos.z + w;
	test[2] =  v->Pos.x + w;
	test[3] = -v->Pos.x + w;
	test[4] =  v->Pos.y + w;
	test[5] = -v->Pos.y + w;

	flag  = (IR ( test[0] )              ) >> 31;
	flag |= (IR ( test[1] ) & 0x80000000 ) >> 30;
	flag |= (IR ( test[2] ) & 0x80000000 ) >> 29;
	flag |= (IR ( test[3] ) & 0x80000000 ) >> 28;
	flag |= (IR ( test[4] ) & 0x80000000 ) >> 27;
	flag |= (IR ( test[5] ) & 0x80000000 ) >> 26;

/*
	flag  = F32_LOWER_EQUAL_0 ( test[0] );
	flag |= F32_LOWER_EQUAL_0 ( test[1] ) << 1;
	flag |= F32_LOWER_EQUAL_0 ( test[2] ) << 2;
	flag |= F32_LOWER_EQUAL_0 ( test[3] ) << 3;
	flag |= F32_LOWER_EQUAL_0 ( test[4] ) << 4;
	flag |= F32_LOWER_EQUAL_0 ( test[5] ) << 5;
*/
	return flag;
}

#else


REALINLINE u32 CSoftwareDriver2::clipToFrustumTest ( const s4DVertex * v  ) const
{
	u32 flag = 0;
	for ( u32 i = 0; i!= 6; ++i )
	{
		core::setbit_cond( flag, v->Pos.dotProduct ( NDCPlane[i] ) <= 0.f, 1 << i );
	}
	return flag;
}

#endif // _MSC_VER 

u32 CSoftwareDriver2::clipToHyperPlane ( s4DVertex * dest, const s4DVertex * source, u32 inCount, const sVec4 &plane )
{
	u32 outCount = 0;
	s4DVertex * out = dest;

	const s4DVertex * a;
	const s4DVertex * b = source;

	f32 bDotPlane;

	bDotPlane = b->Pos.dotProduct ( plane );

	for( u32 i = 1; i < inCount + 1; ++i)
	{
		const s32 condition = i - inCount;
		const s32 index = (( ( condition >> 31 ) & ( i ^ condition ) ) ^ condition ) << 1;

		a = &source[ index ];

		// current point inside
		if ( a->Pos.dotProduct ( plane ) <= 0.f )
		{
			// last point outside
			if ( F32_GREATER_0 ( bDotPlane ) )
			{
				// intersect line segment with plane
				out->interpolate ( *b, *a, bDotPlane / (b->Pos - a->Pos).dotProduct ( plane ) );
				out += 2;
				outCount += 1;
			}

			// copy current to out
			//*out = *a;
			irr::memcpy32_small ( out, a, SIZEOF_SVERTEX * 2 );
			b = out;

			out += 2;
			outCount += 1;
		}
		else
		{
			// current point outside

			if ( F32_LOWER_EQUAL_0 (  bDotPlane ) )
			{
				// previous was inside
				// intersect line segment with plane
				out->interpolate ( *b, *a, bDotPlane / (b->Pos - a->Pos).dotProduct ( plane ) );
				out += 2;
				outCount += 1;
			}
			// pointer
			b = a;
		}

		bDotPlane = b->Pos.dotProduct ( plane );

	}

	return outCount;
}


u32 CSoftwareDriver2::clipToFrustum ( s4DVertex *v0, s4DVertex * v1, const u32 vIn )
{
	u32 vOut = vIn;

	vOut = clipToHyperPlane ( v1, v0, vOut, NDCPlane[0] ); if ( vOut < vIn ) return vOut;
	vOut = clipToHyperPlane ( v0, v1, vOut, NDCPlane[1] ); if ( vOut < vIn ) return vOut;
	vOut = clipToHyperPlane ( v1, v0, vOut, NDCPlane[2] ); if ( vOut < vIn ) return vOut;
	vOut = clipToHyperPlane ( v0, v1, vOut, NDCPlane[3] ); if ( vOut < vIn ) return vOut;
	vOut = clipToHyperPlane ( v1, v0, vOut, NDCPlane[4] ); if ( vOut < vIn ) return vOut;
	vOut = clipToHyperPlane ( v0, v1, vOut, NDCPlane[5] );
	return vOut;
}

/*!
 Part I:
	apply Clip Scale matrix
	From Normalized Device Coordiante ( NDC ) Space to Device Coordinate Space ( DC )

 Part II:
	Project homogeneous vector
	homogeneous to non-homogenous coordinates ( dividebyW )

	Incoming: ( xw, yw, zw, w, u, v, 1, R, G, B, A )
	Outgoing: ( xw/w, yw/w, zw/w, w/w, u/w, v/w, 1/w, R/w, G/w, B/w, A/w )


	replace w/w by 1/w
*/
inline void CSoftwareDriver2::ndc_2_dc_and_project ( s4DVertex *dest,s4DVertex *source, u32 vIn ) const
{
	u32 g;

	for ( g = 0; g != vIn; g += 2 )
	{
		if ( (dest[g].flag & VERTEX4D_PROJECTED ) == VERTEX4D_PROJECTED )
			continue;

		dest[g].flag = source[g].flag | VERTEX4D_PROJECTED;

		const f32 w = source[g].Pos.w;
		const f32 iw = core::reciprocal ( w );

		// to device coordinates
		dest[g].Pos.x = iw * ( source[g].Pos.x * Transformation [ ETS_CLIPSCALE ].m[ 0] + w * Transformation [ ETS_CLIPSCALE ].m[12] );
		dest[g].Pos.y = iw * ( source[g].Pos.y * Transformation [ ETS_CLIPSCALE ].m[ 5] + w * Transformation [ ETS_CLIPSCALE ].m[13] );

#ifndef SOFTWARE_DRIVER_2_USE_WBUFFER
		dest[g].Pos.z = iw * source[g].Pos.z;
#endif

	#ifdef SOFTWARE_DRIVER_2_USE_VERTEX_COLOR
		#ifdef SOFTWARE_DRIVER_2_PERSPECTIVE_CORRECT
			dest[g].Color[0] = source[g].Color[0] * iw;
		#else
			dest[g].Color[0] = source[g].Color[0];
		#endif

	#endif

		dest[g].Pos.w = iw;

	}

}


inline void CSoftwareDriver2::ndc_2_dc_and_project2 ( const s4DVertex **v, const u32 size ) const
{
	u32 g;

	for ( g = 0; g != size; g += 1 )
	{
		s4DVertex * a = (s4DVertex*) v[g];

		if ( (a[1].flag & VERTEX4D_PROJECTED ) == VERTEX4D_PROJECTED )
			continue;

		a[1].flag = a->flag | VERTEX4D_PROJECTED;

		// project homogenous vertex, store 1/w
		const f32 w = a->Pos.w;
		const f32 iw = core::reciprocal ( w );

		// to device coordinates
		a[1].Pos.x = iw * ( a->Pos.x * Transformation [ ETS_CLIPSCALE ].m[ 0] + w * Transformation [ ETS_CLIPSCALE ].m[12] );
		a[1].Pos.y = iw * ( a->Pos.y * Transformation [ ETS_CLIPSCALE ].m[ 5] + w * Transformation [ ETS_CLIPSCALE ].m[13] );

#ifndef SOFTWARE_DRIVER_2_USE_WBUFFER
		a[1].Pos.z = a->Pos.z * iw;
#endif

	#ifdef SOFTWARE_DRIVER_2_USE_VERTEX_COLOR
		#ifdef SOFTWARE_DRIVER_2_PERSPECTIVE_CORRECT
			a[1].Color[0] = a->Color[0] * iw;
		#else
			a[1].Color[0] = a->Color[0];
		#endif
	#endif

		a[1].Pos.w = iw;

	}

}


/*!
	crossproduct in projected 2D -> screen area triangle
*/
inline f32 CSoftwareDriver2::screenarea ( const s4DVertex *v ) const
{
	return	( ( v[3].Pos.x - v[1].Pos.x ) * ( v[5].Pos.y - v[1].Pos.y ) ) -
			( ( v[3].Pos.y - v[1].Pos.y ) * ( v[5].Pos.x - v[1].Pos.x ) );
}


/*!
*/
inline f32 CSoftwareDriver2::texelarea ( const s4DVertex *v, int tex ) const
{
	f32 x0,y0, x1,y1, z;

	x0 = v[2].Tex[tex].x - v[0].Tex[tex].x;
	y0 = v[2].Tex[tex].y - v[0].Tex[tex].y;
	x1 = v[4].Tex[tex].x - v[0].Tex[tex].x;
	y1 = v[4].Tex[tex].y - v[0].Tex[tex].y;

	z = x0*y1 - x1*y0;

	const core::dimension2d<s32> &d = Texmap[tex].Texture->getMaxSize();
	z *= d.Height;
	z *= d.Width;
	return z;
}

/*!
	crossproduct in projected 2D
*/
inline f32 CSoftwareDriver2::screenarea2 ( const s4DVertex **v ) const
{
	return	( (( v[1] + 1 )->Pos.x - (v[0] + 1 )->Pos.x ) * ( (v[2] + 1 )->Pos.y - (v[0] + 1 )->Pos.y ) ) -
			( (( v[1] + 1 )->Pos.y - (v[0] + 1 )->Pos.y ) * ( (v[2] + 1 )->Pos.x - (v[0] + 1 )->Pos.x ) );
}

/*!
*/
inline f32 CSoftwareDriver2::texelarea2 ( const s4DVertex **v, s32 tex ) const
{
	f32 z;

	z =		(v[1]->Tex[tex].x - v[0]->Tex[tex].x ) *
			(v[2]->Tex[tex].y - v[0]->Tex[tex].y )
		 -	(v[2]->Tex[tex].x - v[0]->Tex[tex].x ) *
			(v[1]->Tex[tex].y - v[0]->Tex[tex].y )
		;

	const core::dimension2d<s32> &d = Texmap[tex].Texture->getMaxSize();
	z *= d.Height;
	z *= d.Width;
	return z;
}




/*!
*/
inline void CSoftwareDriver2::select_polygon_mipmap ( s4DVertex *v, u32 vIn, s32 tex )
{
	f32 f[2];
	const core::dimension2d<s32>& dim = Texmap[tex].Texture->getSize();

	f[0] = (f32) dim.Width;
	f[1] = (f32) dim.Height;

#ifdef SOFTWARE_DRIVER_2_PERSPECTIVE_CORRECT
	for ( u32 g = 0; g != vIn; g += 2 )
	{
		(v + g + 1 )->Tex[tex].x	= (v + g + 0)->Tex[tex].x * ( v + g + 1 )->Pos.w * f[0];
		(v + g + 1 )->Tex[tex].y	= (v + g + 0)->Tex[tex].y * ( v + g + 1 )->Pos.w * f[1];
	}
#else
	for ( u32 g = 0; g != vIn; g += 2 )
	{
		(v + g + 1 )->Tex[tex].x	= (v + g + 0)->Tex[tex].x * f[0];
		(v + g + 1 )->Tex[tex].y	= (v + g + 0)->Tex[tex].y * f[1];
	}
#endif
}

inline void CSoftwareDriver2::select_polygon_mipmap2 ( s4DVertex **v, s32 tex ) const
{
	f32 f[2];
	const core::dimension2d<s32>& dim = Texmap[tex].Texture->getSize();

	f[0] = (f32) dim.Width;
	f[1] = (f32) dim.Height;

#ifdef SOFTWARE_DRIVER_2_PERSPECTIVE_CORRECT
	(v[0] + 1 )->Tex[tex].x	= v[0]->Tex[tex].x * ( v[0] + 1 )->Pos.w * f[0];
	(v[0] + 1 )->Tex[tex].y	= v[0]->Tex[tex].y * ( v[0] + 1 )->Pos.w * f[1];

	(v[1] + 1 )->Tex[tex].x	= v[1]->Tex[tex].x * ( v[1] + 1 )->Pos.w * f[0];
	(v[1] + 1 )->Tex[tex].y	= v[1]->Tex[tex].y * ( v[1] + 1 )->Pos.w * f[1];

	(v[2] + 1 )->Tex[tex].x	= v[2]->Tex[tex].x * ( v[2] + 1 )->Pos.w * f[0];
	(v[2] + 1 )->Tex[tex].y	= v[2]->Tex[tex].y * ( v[2] + 1 )->Pos.w * f[1];

#else
	(v[0] + 1 )->Tex[tex].x	= v[0]->Tex[tex].x * f[0];
	(v[0] + 1 )->Tex[tex].y	= v[0]->Tex[tex].y * f[1];

	(v[1] + 1 )->Tex[tex].x	= v[1]->Tex[tex].x * f[0];
	(v[1] + 1 )->Tex[tex].y	= v[1]->Tex[tex].y * f[1];

	(v[2] + 1 )->Tex[tex].x	= v[2]->Tex[tex].x * f[0];
	(v[2] + 1 )->Tex[tex].y	= v[2]->Tex[tex].y * f[1];
#endif
}

// Vertex Cache
const SVSize CSoftwareDriver2::vSize[] =
{
	{ VERTEX4D_FORMAT_0, sizeof(S3DVertex),1 },
	{ VERTEX4D_FORMAT_1, sizeof(S3DVertex2TCoords),2 },
	{ VERTEX4D_FORMAT_2, sizeof(S3DVertexTangents),2 }
};



/*!
	fill a cache line with transformed, light and clipp test triangles
*/
void CSoftwareDriver2::VertexCache_fill(const u32 sourceIndex,
					const u32 destIndex)
{
	u8 * source;
	s4DVertex *dest;

	source = (u8*) VertexCache.vertices + ( sourceIndex * vSize[VertexCache.vType].Pitch );

	// it's a look ahead so we never hit it..
	// but give priority...
	//VertexCache.info[ destIndex ].hit = hitCount;

	// store info
	VertexCache.info[ destIndex ].index = sourceIndex;
	VertexCache.info[ destIndex ].hit = 0;

	// destination Vertex
	dest = (s4DVertex *) ( (u8*) VertexCache.mem.data + ( destIndex << ( SIZEOF_SVERTEX_LOG2 + 1  ) ) );

	// transform Model * World * Camera * Projection * NDCSpace matrix
	Transformation [ ETS_CURRENT].m.transformVect ( &dest->Pos.x, ((S3DVertex*) source )->Pos );

#ifdef SOFTWARE_DRIVER_2_USE_VERTEX_COLOR

	// light Vertex
	#ifdef SOFTWARE_DRIVER_2_LIGHTING
		lightVertex ( dest, ((S3DVertex*) source ) );
	#else
		dest->Color[0].setA8R8G8B8 ( ((S3DVertex*) source )->Color.color );
	#endif
#endif

	// transfer texture coordinates
	if ( Transformation [ ETS_TEXTURE_0 ].isIdentity )
	{
		// only look on first transform
		irr::memcpy32_small ( &dest->Tex[0],
						&((S3DVertex*) source )->TCoords,
						vSize[VertexCache.vType].TexSize * ( sizeof ( f32 ) * 2 )
					);
	}
	else
	{
	/*
			Generate texture coordinates as linear functions so that:
				u = Ux*x + Uy*y + Uz*z + Uw 
				v = Vx*x + Vy*y + Vz*z + Vw
			The matrix M for this case is:
				Ux  Vx  0  0 
				Uy  Vy  0  0 
				Uz  Vz  0  0 
				Uw  Vw  0  0 
	*/

		const core::vector2d<f32> *src = &((S3DVertex*) source )->TCoords;
		u32 t;

		for ( t = 0; t != vSize[VertexCache.vType].TexSize; ++t )
		{
			const core::matrix4& M = Transformation [ ETS_TEXTURE_0 + t ].m;
			if ( Material.org.TextureLayer[0].TextureWrap==ETC_REPEAT )
			{
				dest->Tex[t].x = M[0] * src[t].X + M[4] * src[t].Y + M[8];
				dest->Tex[t].y = M[1] * src[t].X + M[5] * src[t].Y + M[9];
			}
			else
			{
				f32 tx1, ty1;

				tx1 = M[0] * src[t].X + M[4] * src[t].Y + M[8];
				ty1 = M[1] * src[t].X + M[5] * src[t].Y + M[9];

				dest->Tex[t].x = tx1 <= 0.f ? 0.f : tx1 >= 1.f ? 1.f : tx1;
				dest->Tex[t].y = ty1 <= 0.f ? 0.f : ty1 >= 1.f ? 1.f : ty1;

				//dest->Tex[t].x = core::clamp ( M[0] * src[t].X + M[4] * src[t].Y + M[8], 0.f, 1.f );
				//dest->Tex[t].y = core::clamp ( M[1] * src[t].X + M[5] * src[t].Y + M[9], 0.f, 1.f );
			}
		}

	}

	dest[0].flag = dest[1].flag = vSize[VertexCache.vType].Format;

	// test vertex 
	dest[0].flag |= clipToFrustumTest ( dest);

	// to DC Space, project homogenous vertex
	if ( (dest[0].flag & VERTEX4D_CLIPMASK ) == VERTEX4D_INSIDE )
	{
		ndc_2_dc_and_project2 ( (const s4DVertex**) &dest, 1 );
	}

	//return dest;
}

//

REALINLINE s4DVertex * CSoftwareDriver2::VertexCache_getVertex ( const u32 sourceIndex )
{
	for ( s32 i = 0; i < VERTEXCACHE_ELEMENT; ++i )
	{
		if ( VertexCache.info[ i ].index == sourceIndex )
		{
			return (s4DVertex *) ( (u8*) VertexCache.mem.data + ( i << ( SIZEOF_SVERTEX_LOG2 + 1  ) ) );
		}
	}
	return 0;
}


/*
	Cache based on linear walk indices
	fill blockwise on the next 16(Cache_Size) unique vertices in indexlist
	merge the next 16 vertices with the current
*/
REALINLINE void CSoftwareDriver2::VertexCache_get ( s4DVertex ** face )
{
	SCacheInfo info[VERTEXCACHE_ELEMENT];

	// next primitive must be complete in cache
	if (	VertexCache.indicesIndex - VertexCache.indicesRun < 3 &&
			VertexCache.indicesIndex < VertexCache.indexCount
		)
	{
		// rewind to start of primitive
		VertexCache.indicesIndex = VertexCache.indicesRun;

		irr::memset32 ( info, VERTEXCACHE_MISS, sizeof ( info ) );

		// get the next unique vertices cache line
		u32 fillIndex = 0;
		u32 dIndex;
		u32 i;

		while ( VertexCache.indicesIndex < VertexCache.indexCount &&
				fillIndex < VERTEXCACHE_ELEMENT
				)
		{
			u32 sourceIndex = VertexCache.indices [ VertexCache.indicesIndex++ ];

			// if not exist, push back
			s32 exist = 0;
			for ( dIndex = 0;  dIndex < fillIndex; ++dIndex )
			{
				if ( info[ dIndex ].index == sourceIndex )
				{
					exist = 1;
					break;
				}
			}

			if ( 0 == exist )
			{
				info[fillIndex++].index = sourceIndex;
			}
		}

		// clear marks
		for ( i = 0; i!= VERTEXCACHE_ELEMENT; ++i )
		{
			VertexCache.info[i].hit = 0;
		}

		// mark all exisiting
		for ( i = 0; i!= fillIndex; ++i )
		{
			for ( dIndex = 0;  dIndex < VERTEXCACHE_ELEMENT; ++dIndex )
			{
				if ( VertexCache.info[ dIndex ].index == info[i].index )
				{
					info[i].hit = dIndex;
					VertexCache.info[ dIndex ].hit = 1;
					break;
				}
			}
		}

		// fill new
		for ( i = 0; i!= fillIndex; ++i )
		{
			if ( info[i].hit != VERTEXCACHE_MISS )
				continue;

			for ( dIndex = 0;  dIndex < VERTEXCACHE_ELEMENT; ++dIndex )
			{
				if ( 0 == VertexCache.info[dIndex].hit )
				{
					VertexCache_fill ( info[i].index, dIndex );
					VertexCache.info[dIndex].hit += 1;
					info[i].hit = dIndex;
					break;
				}
			}
		}
	}

	const u32 i0 = core::if_c_a_else_0 ( VertexCache.pType != scene::EPT_TRIANGLE_FAN, VertexCache.indicesRun );

	face[0] = VertexCache_getVertex ( VertexCache.indices[ i0    ] );
	face[1] = VertexCache_getVertex ( VertexCache.indices[ VertexCache.indicesRun + 1] );
	face[2] = VertexCache_getVertex ( VertexCache.indices[ VertexCache.indicesRun + 2] );

	VertexCache.indicesRun += VertexCache.primitivePitch;
}

REALINLINE void CSoftwareDriver2::VertexCache_get2 ( s4DVertex ** face )
{
	const u32 i0 = core::if_c_a_else_0 ( VertexCache.pType != scene::EPT_TRIANGLE_FAN, VertexCache.indicesRun );

	VertexCache_fill ( VertexCache.indices[ i0    ], 0 );
	VertexCache_fill ( VertexCache.indices[ VertexCache.indicesRun + 1], 1 );
	VertexCache_fill ( VertexCache.indices[ VertexCache.indicesRun + 2], 2 );

	VertexCache.indicesRun += VertexCache.primitivePitch;

	face[0] = (s4DVertex *) ( (u8*) VertexCache.mem.data + ( 0 << ( SIZEOF_SVERTEX_LOG2 + 1  ) ) );
	face[1] = (s4DVertex *) ( (u8*) VertexCache.mem.data + ( 1 << ( SIZEOF_SVERTEX_LOG2 + 1  ) ) );
	face[2] = (s4DVertex *) ( (u8*) VertexCache.mem.data + ( 2 << ( SIZEOF_SVERTEX_LOG2 + 1  ) ) );

}

void CSoftwareDriver2::VertexCache_reset ( const void* vertices, u32 vertexCount, 
											const u16* indices, u32 primitiveCount, 
											E_VERTEX_TYPE vType,scene::E_PRIMITIVE_TYPE pType )
{
	VertexCache.vertices = vertices;
	VertexCache.vertexCount = vertexCount;

	VertexCache.indices = indices;
	VertexCache.indicesIndex = 0;
	VertexCache.indicesRun = 0;

	VertexCache.vType = vType;
	VertexCache.pType = pType;

	switch ( VertexCache.pType )
	{
		case scene::EPT_TRIANGLES:
			VertexCache.indexCount = primitiveCount + primitiveCount + primitiveCount;
			VertexCache.primitivePitch = 3;
			break;
		case scene::EPT_TRIANGLE_FAN:
			VertexCache.indexCount = primitiveCount + 2;
			VertexCache.primitivePitch = 1;
			break;
	}

	irr::memset32 ( VertexCache.info, VERTEXCACHE_MISS, sizeof ( VertexCache.info ) );

}


//! draws a vertex primitive list
void CSoftwareDriver2::drawVertexPrimitiveList(const void* vertices, u32 vertexCount, const u16* indexList, u32 primitiveCount, E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType)
{
	if (!checkPrimitiveCount(primitiveCount))
		return;

	CNullDriver::drawVertexPrimitiveList(vertices, vertexCount, indexList, primitiveCount, vType, pType);

	if ( 0 == CurrentShader )
		return;


	VertexCache_reset ( vertices, vertexCount, indexList, primitiveCount, vType, pType );

	const s4DVertex * face[3];

	f32 dc_area;
	s32 lodLevel;
	u32 i;
	u32 g;

	for ( i = 0; i < (u32) primitiveCount; ++i )
	{
		VertexCache_get ( (s4DVertex**) face );

		// if fully outside or outside on same side
		if ( ( (face[0]->flag | face[1]->flag | face[2]->flag) & VERTEX4D_CLIPMASK )
				!= VERTEX4D_INSIDE
			)
			continue;

		// if fully inside
		if ( ( face[0]->flag & face[1]->flag & face[2]->flag & VERTEX4D_CLIPMASK ) == VERTEX4D_INSIDE )
		{
			dc_area = screenarea2 ( face );
			if ( Material.org.BackfaceCulling && F32_LOWER_EQUAL_0 ( dc_area ) )
			{
				continue;
			}

			dc_area = core::reciprocal ( dc_area );
			// select mipmap
			for ( g = 0; g != 2; ++g )
			{
				if ( 0 == Texmap[g].Texture )
				{
					CurrentShader->setTexture(g, 0, 0);
					continue;
				}

				lodLevel = s32_log2_f32 ( texelarea2 ( face, g ) * dc_area );

				CurrentShader->setTexture(g, Texmap[g].Texture, lodLevel);
				select_polygon_mipmap2 ( (s4DVertex**) face, g );

			}

			// rasterize
			CurrentShader->drawTriangle ( face[0] + 1, face[1] + 1, face[2] + 1 );
			continue;
		}

		// else if not complete inside clipping necessary
		irr::memcpy32_small ( ( (u8*) CurrentOut.data + ( 0 << ( SIZEOF_SVERTEX_LOG2 + 1 ) ) ), face[0], SIZEOF_SVERTEX * 2 );
		irr::memcpy32_small ( ( (u8*) CurrentOut.data + ( 1 << ( SIZEOF_SVERTEX_LOG2 + 1 ) ) ), face[1], SIZEOF_SVERTEX * 2 );
		irr::memcpy32_small ( ( (u8*) CurrentOut.data + ( 2 << ( SIZEOF_SVERTEX_LOG2 + 1 ) ) ), face[2], SIZEOF_SVERTEX * 2 );

		u32 flag = CurrentOut.data->flag & VERTEX4D_FORMAT_MASK;

		for ( g = 0; g != CurrentOut.ElementSize; ++g )
		{
			CurrentOut.data[g].flag = flag;
			Temp.data[g].flag = flag;
		}

		u32 vOut;
		vOut = clipToFrustum ( CurrentOut.data, Temp.data, 3 );
/*
		if ( vOut < 3 )
		{
			char buf[256];
			struct SCheck
			{
				u32 flag;
				const char * name;
			};
			
			SCheck check[5];
			check[0].flag = face[0]->flag;
			check[0].name = "face0";
			check[1].flag = face[1]->flag;
			check[1].name = "face1";
			check[2].flag = face[2]->flag;
			check[2].name = "face2";
			check[3].flag = (face[0]->flag & face[1]->flag & face[2]->flag);
			check[3].name = "AND  ";
			check[4].flag = (face[0]->flag | face[1]->flag | face[2]->flag);
			check[4].name = "OR   ";

			for ( s32 h = 0; h!= 5; ++h )
			{
				sprintf ( buf, "%s: %d %d %d %d %d %d",
								check[h].name,
								( check[h].flag & 1 ),
								( check[h].flag & 2 ) >> 1,
								( check[h].flag & 4 ) >> 2,
								( check[h].flag & 8 ) >> 3,
								( check[h].flag & 16 ) >> 4,
								( check[h].flag & 32 ) >> 5
							);
				os::Printer::print ( buf );
			}

			sprintf ( buf, "Vout: %d\n", vOut );
			os::Printer::print ( buf );

			int hold = 1;
		}
*/
		if ( vOut < 3 )
			continue;

		vOut <<= 1;

		// to DC Space, project homogenous vertex
		ndc_2_dc_and_project ( CurrentOut.data + 1, CurrentOut.data, vOut );

/*
		// if not complete inside clipping necessary
		if ( ( test & VERTEX4D_INSIDE ) != VERTEX4D_INSIDE )
		{
			u32 v[2] = { PointerAsValue ( Temp ) , PointerAsValue ( CurrentOut ) };
			for ( g = 0; g != 6; ++g )
			{
				vOut = clipToHyperPlane ( (s4DVertex*) v[0], (s4DVertex*) v[1], vOut, NDCPlane[g] );
				if ( vOut < 3 )
					break;

				v[0] ^= v[1];
				v[1] ^= v[0];
				v[0] ^= v[1];

			}

			if ( vOut < 3 )
				continue;

		}
*/

		// check 2d backface culling on first
		dc_area = screenarea ( CurrentOut.data );
		if ( Material.org.BackfaceCulling && F32_LOWER_EQUAL_0 ( dc_area ) )
			continue;

		// select mipmap
		for ( g = 0; g != 2; ++g )
		{
			if ( 0 == Texmap[g].Texture )
			{
				CurrentShader->setTexture(g, 0, 0);
				continue;
			}

			lodLevel = s32_log2_f32 ( texelarea ( CurrentOut.data, g ) / dc_area );

			CurrentShader->setTexture(g, Texmap[g].Texture, lodLevel);
			select_polygon_mipmap ( CurrentOut.data, vOut, g );
		}

		// re-tesselate ( triangle-fan, 0-1-2,0-2-3.. )
		for ( g = 0; g <= vOut - 6; g += 2 )
		{
			// rasterize
			CurrentShader->drawTriangle ( CurrentOut.data + 0 + 1,
													CurrentOut.data + g + 3,
													CurrentOut.data + g + 5
												);
		}

	}

	// dump statistics
/*
	char buf [64];
	sprintf ( buf,"VCount:%d PCount:%d CacheMiss: %d",
					vertexCount, primitiveCount,
					VertexCache.CacheMiss
				);
	os::Printer::print ( buf );
*/
}

//! Sets the dynamic ambient light color. The default color is
//! (0,0,0,0) which means it is dark.
//! \param color: New color of the ambient light.
void CSoftwareDriver2::setAmbientLight(const SColorf& color)
{
	Global_AmbientLight.setColorf ( color );
}


//! adds a dynamic light
void CSoftwareDriver2::addDynamicLight(const SLight& dl)
{
	if ( Light.size () >= getMaximalDynamicLightAmount () )
		return;

	SInternalLight l;
	l.org = dl;

	// light in eye space
	Transformation[ETS_VIEW].m.transformVect ( &l.posEyeSpace.x, l.org.Position );

	l.constantAttenuation = l.org.Attenuation.X;
	l.linearAttenuation = l.org.Attenuation.Y;
	l.quadraticAttenuation = l.org.Attenuation.Z;

	l.AmbientColor.setColorf ( l.org.AmbientColor );
	l.DiffuseColor.setColorf ( l.org.DiffuseColor );
	l.SpecularColor.setColorf ( l.org.SpecularColor );

	switch ( dl.Type )
	{
		case video::ELT_DIRECTIONAL:
		{
			l.posEyeSpace.normalize_xyz ();
		} break;
	}

	Light.push_back ( l );
	CNullDriver::addDynamicLight( l.org );
}

//! deletes all dynamic lights there are
void CSoftwareDriver2::deleteAllDynamicLights()
{
	Light.set_used ( 0 );
	CNullDriver::deleteAllDynamicLights();

}

//! returns the maximal amount of dynamic lights the device can handle
u32 CSoftwareDriver2::getMaximalDynamicLightAmount() const
{
	return 8;
}



#ifdef SOFTWARE_DRIVER_2_LIGHTING

/*!
*/
void CSoftwareDriver2::lightVertex ( s4DVertex *dest, const S3DVertex *source )
{
	// apply lighting model
	if ( false == Material.org.Lighting )
	{
		// should use the DiffuseColor but using pre-lit vertex color
		dest->Color[0].setA8R8G8B8 ( source->Color.color );
		return;
	}

	if ( Lights.size () == 0 )
	{
		dest->Color[0] = Material.EmissiveColor;
		return;
	}

	// eyespace
/*
	core::matrix4 modelview = Transformation[ETS_WORLD].m * Transformation[ETS_VIEW].m;

	core::matrix4 m2 ( modelview );
	m2.makeInverse ();
	core::matrix4 modelviewinversetransposed ( m2.getTransposed() );
*/

	sVec4 vertexEyeSpace;
	sVec4 normalEyeSpace;
	sVec4 vertexEyeSpaceUnit;

	// vertex in eye space
	Transformation[ETS_WORLD_VIEW].m.transformVect ( &vertexEyeSpace.x, source->Pos );
	vertexEyeSpace.project_xyz ();

	vertexEyeSpaceUnit = vertexEyeSpace;
	vertexEyeSpaceUnit.normalize_xyz();

	// vertex normal in eye-space
	//modelviewinversetransposed.transformVect ( &normalEyeSpace.x, source->Normal );
	Transformation[ETS_WORLD_VIEW_INVERSE_TRANSPOSED].m.rotateVect ( &normalEyeSpace.x, source->Normal );
	if ( Material.org.NormalizeNormals )
	{
		normalEyeSpace.normalize_xyz();
	}


	sVec4 ambient;
	sVec4 diffuse;
	sVec4 specular;


	// the universe started in darkness..
	ambient.set ( 0.f, 0.f, 0.f, 0.f );
	diffuse.set ( 0.f, 0.f, 0.f, 0.f );
	specular.set ( 0.f, 0.f, 0.f, 0.f );

	f32 attenuation = 1.f;

	u32 i;
	for ( i = 0; i!= Light.size (); ++i )
	{
		const SInternalLight &light = Light[i];

		sVec4 vp;			// unit vector vertex to light
		sVec4 lightHalf;		// blinn-phong reflection


		switch ( light.org.Type )
		{
			case video::ELT_POINT:
			{
				// surface to light
				vp.x = light.posEyeSpace.x - vertexEyeSpace.x;
				vp.y = light.posEyeSpace.y - vertexEyeSpace.y;
				vp.z = light.posEyeSpace.z - vertexEyeSpace.z;

				// irrlicht attenuation model
#if 0
				const f32 d = vp.get_inverse_length_xyz();

				vp.x *= d;
				vp.y *= d;
				vp.z *= d;
				attenuation = light.org.Radius * d;

#else
				const f32 d = vp.get_length_xyz();
				attenuation = core::reciprocal (light.constantAttenuation +
									light.linearAttenuation * d +
									light.quadraticAttenuation * d * d
								);

				// normalize surface to light
				vp.normalize_xyz();
#endif

				lightHalf.x = vp.x - vertexEyeSpaceUnit.x;
				lightHalf.y = vp.y - vertexEyeSpaceUnit.y;
				lightHalf.z = vp.z - vertexEyeSpaceUnit.z;
				lightHalf.normalize_xyz();
		
			} break;

			case video::ELT_DIRECTIONAL:
			{
				attenuation = 1.f;
				vp = light.posEyeSpace;

				// half angle = lightvector + eye vector ( 0, 0, 1 )
				lightHalf.x = vp.x;
				lightHalf.y = vp.y;
				lightHalf.z = vp.z - 1.f;
				lightHalf.normalize_xyz();
			} break;
		}

		// build diffuse reflection

		//angle between normal and light vector
		f32 dotVP = core::max_ ( 0.f, normalEyeSpace.dot_xyz ( vp ) );
		f32 dotHV = core::max_ ( 0.f, normalEyeSpace.dot_xyz ( lightHalf ) );

		f32 pf;
		if ( dotVP == 0.0 )
		{
			pf = 0.f;
		}
		else
		{
			pf = (f32)pow(dotHV, Material.org.Shininess );
		}

		// accumulate ambient
		ambient += light.AmbientColor * attenuation;
		diffuse += light.DiffuseColor * ( dotVP * attenuation );
		specular += light.SpecularColor * ( pf * attenuation );

	}

	sVec4 dColor;

	dColor = Global_AmbientLight;
	dColor += Material.EmissiveColor;
	dColor += ambient * Material.AmbientColor;
	dColor += diffuse * Material.DiffuseColor;
	dColor += specular * Material.SpecularColor;
	dColor.saturate();

	dest->Color[0] = dColor;
}

#endif


//! draws an 2d image, using a color (if color is other then Color(255,255,255,255)) and the alpha channel of the texture if wanted.
void CSoftwareDriver2::draw2DImage(const video::ITexture* texture, const core::position2d<s32>& destPos,
					 const core::rect<s32>& sourceRect, 
					 const core::rect<s32>* clipRect, SColor color, 
					 bool useAlphaChannelOfTexture)
{
	if (texture)
	{
		if (texture->getDriverType() != EDT_BURNINGSVIDEO)
		{
			os::Printer::log("Fatal Error: Tried to copy from a surface not owned by this driver.", ELL_ERROR);
			return;
		}

		if (useAlphaChannelOfTexture)
			((CSoftwareTexture2*)texture)->getImage()->copyToWithAlpha(
				BackBuffer, destPos, sourceRect, color, clipRect);
		else
			((CSoftwareTexture2*)texture)->getImage()->copyTo(
				BackBuffer, destPos, sourceRect, clipRect);
	}
}



//! Draws a 2d line. 
void CSoftwareDriver2::draw2DLine(const core::position2d<s32>& start,
					const core::position2d<s32>& end,
					SColor color)
{
	((CImage*)BackBuffer)->drawLine(start, end, color );
}



//! draw an 2d rectangle
void CSoftwareDriver2::draw2DRectangle(SColor color, const core::rect<s32>& pos,
									 const core::rect<s32>* clip)
{
	if (clip)
	{
		core::rect<s32> p(pos);

		p.clipAgainst(*clip);

		if(!p.isValid())  
			return;

		BackBuffer->drawRectangle(p, color);
	}
	else
	{
		if(!pos.isValid())
			return;

		BackBuffer->drawRectangle(pos, color);
	}
}

//! Only used by the internal engine. Used to notify the driver that
//! the window was resized.
void CSoftwareDriver2::OnResize(const core::dimension2d<s32>& size)
{
	// make sure width and height are multiples of 2
	core::dimension2d<s32> realSize(size);

	if (realSize.Width % 2)
		realSize.Width += 1;

	if (realSize.Height % 2)
		realSize.Height += 1;

	if (ScreenSize != realSize)
	{
		if (ViewPort.getWidth() == ScreenSize.Width &&
			ViewPort.getHeight() == ScreenSize.Height)
		{
			ViewPort = core::rect<s32>(core::position2d<s32>(0,0), realSize);
		}

		ScreenSize = realSize;

		bool resetRT = (RenderTargetSurface == BackBuffer);

		BackBuffer->drop();
		BackBuffer = new CImage(ECF_SOFTWARE2, realSize);

		if (resetRT)
			setRenderTarget(BackBuffer);
	}
}

//! returns the current render target size
const core::dimension2d<s32>& CSoftwareDriver2::getCurrentRenderTargetSize() const
{
	return RenderTargetSize;
}

//!Draws an 2d rectangle with a gradient.
void CSoftwareDriver2::draw2DRectangle(const core::rect<s32>& position,
	SColor colorLeftUp, SColor colorRightUp, SColor colorLeftDown, SColor colorRightDown,
	const core::rect<s32>* clip)
{
#ifdef SOFTWARE_DRIVER_2_USE_VERTEX_COLOR

	core::rect<s32> pos = position;

	if (clip)
		pos.clipAgainst(*clip);

	if (!pos.isValid())
		return;

	const core::dimension2d<s32> renderTargetSize ( ViewPort.getSize() );

	const s32 xPlus = -(renderTargetSize.Width>>1);
	const f32 xFact = 1.0f / (renderTargetSize.Width>>1);

	const s32 yPlus = renderTargetSize.Height-(renderTargetSize.Height>>1);
	const f32 yFact = 1.0f / (renderTargetSize.Height>>1);

	// fill VertexCache direct
	s4DVertex *v;

	VertexCache.vertexCount = 4;

	VertexCache.info[0].index = 0;
	VertexCache.info[1].index = 1;
	VertexCache.info[2].index = 2;
	VertexCache.info[3].index = 3;

	v = &VertexCache.mem.data [ 0 ];

	v[0].Pos.set ( (f32)(pos.UpperLeftCorner.X+xPlus) * xFact, (f32)(yPlus-pos.UpperLeftCorner.Y) * yFact, 0.f, 1.f );
	v[0].Color[0].setA8R8G8B8 ( colorLeftUp.color );

	v[2].Pos.set ( (f32)(pos.LowerRightCorner.X+xPlus) * xFact, (f32)(yPlus- pos.UpperLeftCorner.Y) * yFact, 0.f, 1.f );
	v[2].Color[0].setA8R8G8B8 ( colorRightUp.color );

	v[4].Pos.set ( (f32)(pos.LowerRightCorner.X+xPlus) * xFact, (f32)(yPlus-pos.LowerRightCorner.Y) * yFact, 0.f ,1.f );
	v[4].Color[0].setA8R8G8B8 ( colorRightDown.color );

	v[6].Pos.set ( (f32)(pos.UpperLeftCorner.X+xPlus) * xFact, (f32)(yPlus-pos.LowerRightCorner.Y) * yFact, 0.f, 1.f );
	v[6].Color[0].setA8R8G8B8 ( colorLeftDown.color );

	s32 i;
	u32 g;

	for ( i = 0; i!= 8; i += 2 )
	{
		v[i + 0].flag = clipToFrustumTest ( v + i );
		v[i + 1].flag = 0;
		if ( (v[i].flag & VERTEX4D_INSIDE ) == VERTEX4D_INSIDE )
		{
			ndc_2_dc_and_project ( v + i + 1, v + i, 2 );
		}
	}


	IBurningShader * render;

	render = BurningShader [ ETR_GOURAUD_ALPHA_NOZ ];
	render->setRenderTarget(RenderTargetSurface, ViewPort);

	static const s16 indexList[6] = {0,1,2,0,2,3};

	s4DVertex * face[3];

	for ( i = 0; i!= 6; i += 3 )
	{
		face[0] = VertexCache_getVertex ( indexList [ i + 0 ] );
		face[1] = VertexCache_getVertex ( indexList [ i + 1 ] );
		face[2] = VertexCache_getVertex ( indexList [ i + 2 ] );

		// test clipping
		u32 test = face[0]->flag & face[1]->flag & face[2]->flag & VERTEX4D_INSIDE;

		if ( test == VERTEX4D_INSIDE )
		{
			render->drawTriangle ( face[0] + 1, face[1] + 1, face[2] + 1 );
			continue;
		}
		// Todo: all vertices are clipped in 2d..
		// is this true ?
		u32 vOut = 6;
		memcpy ( CurrentOut.data + 0, face[0], sizeof ( s4DVertex ) * 2 );
		memcpy ( CurrentOut.data + 2, face[1], sizeof ( s4DVertex ) * 2 );
		memcpy ( CurrentOut.data + 4, face[2], sizeof ( s4DVertex ) * 2 );

		vOut = clipToFrustum ( CurrentOut.data, Temp.data, 3 );
		if ( vOut < 3 )
			continue;

		vOut <<= 1;
		// to DC Space, project homogenous vertex
		ndc_2_dc_and_project ( CurrentOut.data + 1, CurrentOut.data, vOut );

		// re-tesselate ( triangle-fan, 0-1-2,0-2-3.. )
		for ( g = 0; g <= vOut - 6; g += 2 )
		{
			// rasterize
			render->drawTriangle ( CurrentOut.data + 1, &CurrentOut.data[g + 3], &CurrentOut.data[g + 5] );
		}

	}
#else
	draw2DRectangle ( colorLeftUp, position, clip );
#endif
}



//! Draws a 3d line.
void CSoftwareDriver2::draw3DLine(const core::vector3df& start,
	const core::vector3df& end, SColor color)
{
	Transformation [ ETS_CURRENT].m.transformVect ( &CurrentOut.data[0].Pos.x, start );
	Transformation [ ETS_CURRENT].m.transformVect ( &CurrentOut.data[2].Pos.x, end );

	u32 g;
	u32 vOut;

	// no clipping flags
	for ( g = 0; g != CurrentOut.ElementSize; ++g )
	{
		CurrentOut.data[g].flag = 0;
		Temp.data[g].flag = 0;
	}

	// vertices count per line
	vOut = clipToFrustum ( CurrentOut.data, Temp.data, 2 );
	if ( vOut < 2 )
		return;

	vOut <<= 1;

	IBurningShader * line;
	line = BurningShader [ ETR_TEXTURE_GOURAUD_WIRE ];
	line->setRenderTarget(RenderTargetSurface, ViewPort);

	// to DC Space, project homogenous vertex
	ndc_2_dc_and_project ( CurrentOut.data + 1, CurrentOut.data, vOut );

	// unproject vertex color
#ifdef SOFTWARE_DRIVER_2_USE_VERTEX_COLOR
	for ( g = 0; g != vOut; g+= 2 )
	{
		CurrentOut.data[ g + 1].Color[0].setA8R8G8B8 ( color.color );
	}
#endif


	for ( g = 0; g <= vOut - 4; g += 2 )
	{
		// rasterize
		line->drawLine ( CurrentOut.data + 1, CurrentOut.data + g + 3 );
	}
}



//! \return Returns the name of the video driver. Example: In case of the DirectX8
//! driver, it would return "Direct3D8.1".
const wchar_t* CSoftwareDriver2::getName() const
{
#ifdef BURNINGVIDEO_RENDERER_BEAUTIFUL
	return L"burnings video 0.38b";
#elif defined ( BURNINGVIDEO_RENDERER_ULTRA_FAST )
	return L"burnings video 0.38uf";
#elif defined ( BURNINGVIDEO_RENDERER_FAST )
	return L"burnings video 0.38f";
#else
	return L"burnings video 0.38";
#endif
}

//! Returns type of video driver
E_DRIVER_TYPE CSoftwareDriver2::getDriverType() const
{
	return EDT_BURNINGSVIDEO;
}

//! Returns the transformation set by setTransform
const core::matrix4& CSoftwareDriver2::getTransform(E_TRANSFORMATION_STATE state) const
{
	return Transformation[state].m;
}

//! Creates a render target texture.
ITexture* CSoftwareDriver2::createRenderTargetTexture(const core::dimension2d<s32>& size, const c8* name)
{
	CImage* img = new CImage(ECF_SOFTWARE2, size);

	ITexture* tex = new CSoftwareTexture2(img, name, false, true);
	img->drop();
	return tex;	
}


//! Clears the DepthBuffer. 
void CSoftwareDriver2::clearZBuffer()
{
	if (DepthBuffer)
		DepthBuffer->clear();
}


//! Returns an image created from the last rendered frame.
IImage* CSoftwareDriver2::createScreenShot()
{
	return new CImage(BackBuffer->getColorFormat(), BackBuffer);
}


//! returns a device dependent texture from a software surface (IImage)
//! THIS METHOD HAS TO BE OVERRIDDEN BY DERIVED DRIVERS WITH OWN TEXTURES
ITexture* CSoftwareDriver2::createDeviceDependentTexture(IImage* surface, const char* name)
{
	return new CSoftwareTexture2(surface, name, getTextureCreationFlag(ETCF_CREATE_MIP_MAPS));

}

//! Returns the maximum amount of primitives (mostly vertices) which
//! the device is able to render with one drawIndexedTriangleList
//! call.
u32 CSoftwareDriver2::getMaximalPrimitiveCount() const
{
	return 0x00800000;
}

} // end namespace video
} // end namespace irr

#endif // _IRR_COMPILE_WITH_BURNINGSVIDEO_

namespace irr
{
namespace video
{

//! creates a video driver
IVideoDriver* createSoftwareDriver2(const core::dimension2d<s32>& windowSize, bool fullscreen, io::IFileSystem* io, video::IImagePresenter* presenter)
{
	#ifdef _IRR_COMPILE_WITH_BURNINGSVIDEO_
	return new CSoftwareDriver2(windowSize, fullscreen, io, presenter);
	#else
	return 0;
	#endif // _IRR_COMPILE_WITH_BURNINGSVIDEO_
}



} // end namespace video
} // end namespace irr
