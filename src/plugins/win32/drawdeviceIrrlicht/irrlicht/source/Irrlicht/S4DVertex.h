// Copyright (C) 2002-2008 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h


#ifndef __S_4D_VERTEX_H_INCLUDED__
#define __S_4D_VERTEX_H_INCLUDED__

#include "SoftwareDriver2_compile_config.h"
#include "SoftwareDriver2_helper.h"
#include "irrAllocator.h"

namespace irr
{

namespace video
{

struct sVec2
{
	f32 x;
	f32 y;

	sVec2 () {}

	sVec2 ( f32 _x, f32 _y )
		: x ( _x ), y ( _y ) {}

	void set ( f32 _x, f32 _y )
	{
		x = _x;
		y = _y;
	}

	// f = a * t + b * ( 1 - t )
	void interpolate(const sVec2& a, const sVec2& b, const f32 t)
	{
		x = b.x + ( ( a.x - b.x ) * t );
		y = b.y + ( ( a.y - b.y ) * t );
	}

	sVec2 operator-(const sVec2& other) const
	{
		return sVec2(x - other.x, y - other.y);
	}

	sVec2 operator+(const sVec2& other) const
	{
		return sVec2(x + other.x, y + other.y);
	}

	void operator+=(const sVec2& other)
	{
		x += other.x;
		y += other.y;
	}

	sVec2 operator*(const f32 s) const
	{
		return sVec2(x * s , y * s);
	}

	void operator*=( const f32 s)
	{
		x *= s;
		y *= s;
	}

	void operator=(const sVec2& other)
	{
		x = other.x;
		y = other.y;
	}

};

// A8R8G8B8
struct sVec4;
struct sCompressedVec4
{
	u32 argb;

	void setA8R8G8B8 ( u32 value )
	{
		argb = value;
	}

	void setColorf ( const video::SColorf & color )
	{
		argb = 	core::floor32 ( color.a * 255.f ) << 24 |
				core::floor32 ( color.r * 255.f ) << 16 |
				core::floor32 ( color.g * 255.f ) << 8  |
				core::floor32 ( color.b * 255.f );
	}

	void setVec4 ( const sVec4 & v );

	// f = a * t + b * ( 1 - t )
	void interpolate(const sCompressedVec4& a, const sCompressedVec4& b, const f32 t)
	{
		argb = PixelBlend32 ( b.argb, a.argb, core::floor32 ( t * 256.f ) );
	}


};


struct sVec4
{
	f32 x, y, z, w;

	sVec4 () {}

	sVec4 ( f32 _x, f32 _y, f32 _z, f32 _w )
		: x ( _x ), y ( _y ), z( _z ), w ( _w ){}

	void set ( f32 _x, f32 _y, f32 _z, f32 _w )
	{
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}

	void setA8R8G8B8 ( u32 argb )
	{
		x = ( ( argb & 0xFF000000 ) >> 24 ) * ( 1.f / 255.f );
		y = ( ( argb & 0x00FF0000 ) >> 16 ) * ( 1.f / 255.f );
		z = ( ( argb & 0x0000FF00 ) >>  8 ) * ( 1.f / 255.f );
		w = ( ( argb & 0x000000FF )       ) * ( 1.f / 255.f );
	}


	void setColorf ( const video::SColorf & color )
	{
		x = color.a;
		y = color.r;
		z = color.g;
		w = color.b;
	}

	void saturate ()
	{
		x = core::clamp ( x, 0.f, 1.f );
		y = core::clamp ( y, 0.f, 1.f );
		z = core::clamp ( z, 0.f, 1.f );
		w = core::clamp ( w, 0.f, 1.f );
	}

	// f = a * t + b * ( 1 - t )
	void interpolate(const sVec4& a, const sVec4& b, const f32 t)
	{
		x = b.x + ( ( a.x - b.x ) * t );
		y = b.y + ( ( a.y - b.y ) * t );
		z = b.z + ( ( a.z - b.z ) * t );
		w = b.w + ( ( a.w - b.w ) * t );
	}


	f32 dotProduct(const sVec4& other) const
	{
		return x*other.x + y*other.y + z*other.z + w*other.w;
	}

	f32 dot_xyz( const sVec4& other) const
	{
		return x*other.x + y*other.y + z*other.z;
	}

	f32 get_length_xyz () const
	{
		return sqrtf ( x * x + y * y + z * z );
	}

	f32 get_inverse_length_xyz () const
	{
		return core::reciprocal_squareroot ( x * x + y * y + z * z );
	}


	void normalize_xyz ()
	{
		const f32 l = core::reciprocal_squareroot ( x * x + y * y + z * z );

		x *= l;
		y *= l;
		z *= l;
	}

	void project_xyz ()
	{
		w = core::reciprocal ( w );
		x *= w;
		y *= w;
		z *= w;
	}

	sVec4 operator-(const sVec4& other) const
	{
		return sVec4(x - other.x, y - other.y, z - other.z,w - other.w);
	}

	sVec4 operator+(const sVec4& other) const
	{
		return sVec4(x + other.x, y + other.y, z + other.z,w + other.w);
	}

	void operator+=(const sVec4& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
	}

	sVec4 operator*(f32 s) const
	{
		return sVec4(x * s , y * s, z * s,w * s);
	}

	sVec4 operator*(const sVec4 &other) const
	{
		return sVec4(x * other.x , y * other.y, z * other.z,w * other.w);
	}

	void operator*=(f32 s)
	{
		x *= s;
		y *= s;
		z *= s;
		w *= s;
	}

	void operator*=(const sVec4 &other)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		w *= other.w;
	}

	void operator=(const sVec4& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;
	}
};

inline void sCompressedVec4::setVec4 ( const sVec4 & v )
{
	argb = 	core::floor32 ( v.x * 255.f ) << 24 |
			core::floor32 ( v.y * 255.f ) << 16 |
			core::floor32 ( v.z * 255.f ) << 8  |
			core::floor32 ( v.w * 255.f );
}


enum e4DVertexFlag
{
	VERTEX4D_INSIDE		= 0x0000003F,
	VERTEX4D_CLIPMASK	= 0x0000003F,
	VERTEX4D_PROJECTED	= 0x00000100,

	VERTEX4D_FORMAT_MASK	= 0xFFFF0000,
	VERTEX4D_FORMAT_0	= 0x00010000,
	VERTEX4D_FORMAT_1	= VERTEX4D_FORMAT_0 | 0x00020000,
	VERTEX4D_FORMAT_2	= VERTEX4D_FORMAT_1 | 0x00040000
};

// dummy Vertex
struct __s4DVertex
{
	sVec4 Pos;

#ifdef SOFTWARE_DRIVER_2_USE_VERTEX_COLOR
	sVec4 Color[1];
#endif

	sVec2 Tex[2];
	u32 flag;
};

#define SIZEOF_SVERTEX	64
#define SIZEOF_SVERTEX_LOG2	6

struct s4DVertex
{
	sVec4 Pos;

#ifdef SOFTWARE_DRIVER_2_USE_VERTEX_COLOR
	sVec4 Color[1];
#endif

	sVec2 Tex[2];

	u32 flag;

	u8 fill [ SIZEOF_SVERTEX - sizeof (__s4DVertex) ];

	// f = a * t + b * ( 1 - t )
	void interpolate(const s4DVertex& b, const s4DVertex& a, const f32 t)
	{
		Pos.interpolate ( a.Pos, b.Pos, t );

#ifdef SOFTWARE_DRIVER_2_USE_VERTEX_COLOR
		Color[0].interpolate ( a.Color[0], b.Color[0], t );
#endif

		Tex[0].interpolate ( a.Tex[0], b.Tex[0], t );

		if ( (flag & VERTEX4D_FORMAT_1 ) == VERTEX4D_FORMAT_1 )
		{
			Tex[1].interpolate ( a.Tex[1], b.Tex[1], t );
		}


	}
};

// ----------------- Vertex Cache ---------------------------

struct SAlignedVertex
{
	SAlignedVertex ( u32 element, u32 aligned )
		: ElementSize ( element )
	{
		u32 byteSize = (ElementSize << SIZEOF_SVERTEX_LOG2 ) + aligned;
		mem = new u8 [ byteSize ];
		//data = (s4DVertex*) ((PointerAsValue ( mem ) + (aligned-1) ) & ~ ( aligned - 1 ) );
		data = (s4DVertex*) mem;
	}

	virtual ~SAlignedVertex ()
	{
		delete [] mem;
	}

	s4DVertex *data;
	u8 *mem;
	u32 ElementSize;
};


// hold info for different Vertex Types
struct SVSize
{
	u32 Format;
	u32 Pitch;
	u32 TexSize;
};


// a cache info
struct SCacheInfo
{
	u32 index;
	u32 hit;
};

#define VERTEXCACHE_ELEMENT	16
#define VERTEXCACHE_MISS 0xFFFFFFFF
struct SVertexCache
{
	SVertexCache (): mem ( VERTEXCACHE_ELEMENT * 2, 128 ) {}

	SCacheInfo info[VERTEXCACHE_ELEMENT];


	// Transformed and lite, clipping state
	// + Clipped, Projected
	SAlignedVertex mem;

	// source
	const void* vertices;
	u32 vertexCount;

	const u16* indices;
	u32 indexCount;
	u32 indicesIndex;

	u32 indicesRun;

	// primitives consist of x vertices
	u32 primitivePitch;

	u32 vType;		//E_VERTEX_TYPE
	u32 pType;		//scene::E_PRIMITIVE_TYPE

};


// swap 2 pointer
inline void swapVertexPointer(const s4DVertex** v1, const s4DVertex** v2)
{
	const s4DVertex* b = *v1;
	*v1 = *v2;
	*v2 = b;
}


// ------------------------ Internal Scanline Rasterizer -----------------------------


// internal scan convert
struct sScanConvertData
{
	s32 left;			// major edge left/right
	s32 right;		// !left

	f32 invDeltaY[3];	// inverse edge delta y

	f32 x[2];			// x coordinate
	f32 slopeX[2];		// x slope along edges

#if defined ( SOFTWARE_DRIVER_2_USE_WBUFFER ) || defined ( SOFTWARE_DRIVER_2_PERSPECTIVE_CORRECT )
	f32 w[2];			// w coordinate
	fp24 slopeW[2];		// w slope along edges
#else
	f32 z[2];			// z coordinate
	f32 slopeZ[2];		// z slope along edges
#endif

	sVec4 c[2];			// color
	sVec4 slopeC[2];	// color slope along edges

	sVec2 t0[2];			// texture
	sVec2 slopeT0[2];	// texture slope along edges

	sVec2 t1[2];			// texture
	sVec2 slopeT1[2];	// texture slope along edges

};

// passed to scan Line
struct sScanLineData
{
	s32 y;			// y position of scanline
	f32 x[2];			// x start, x end of scanline

#if defined ( SOFTWARE_DRIVER_2_USE_WBUFFER ) || defined ( SOFTWARE_DRIVER_2_PERSPECTIVE_CORRECT )
	f32 w[2];			// w start, w end of scanline
#else
	f32 z[2];			// z start, z end of scanline
#endif

#ifdef SOFTWARE_DRIVER_2_USE_VERTEX_COLOR
	sVec4 c[2];		// color start, color end of scanline
#endif

	sVec2 t0[2];		// texture start, texture end of scanline
	sVec2 t1[2];		// texture start, texture end of scanline
};


/*
	load a color value
*/
inline void getTexel_plain2 (	tFixPoint &r, tFixPoint &g, tFixPoint &b, 
							const sVec4 &v
							)
{
	r = f32_to_fixPoint ( v.y );
	g = f32_to_fixPoint ( v.z );
	b = f32_to_fixPoint ( v.w );
}

/*
	load a color value
*/
inline void getSample_color (	tFixPoint &a, tFixPoint &r, tFixPoint &g, tFixPoint &b, 
							const sVec4 &v
							)
{
	a = f32_to_fixPoint ( v.x );
	r = f32_to_fixPoint ( v.y, COLOR_MAX * FIX_POINT_F32_MUL);
	g = f32_to_fixPoint ( v.z, COLOR_MAX * FIX_POINT_F32_MUL);
	b = f32_to_fixPoint ( v.w, COLOR_MAX * FIX_POINT_F32_MUL);
}

/*
	load a color value
*/
inline void getSample_color (	tFixPoint &r, tFixPoint &g, tFixPoint &b, 
							const sVec4 &v
							)
{
	r = f32_to_fixPoint ( v.y, COLOR_MAX * FIX_POINT_F32_MUL);
	g = f32_to_fixPoint ( v.z, COLOR_MAX * FIX_POINT_F32_MUL);
	b = f32_to_fixPoint ( v.w, COLOR_MAX * FIX_POINT_F32_MUL);
}



}

}

#endif 

