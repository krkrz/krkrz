// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_MATH_H_INCLUDED__
#define __IRR_MATH_H_INCLUDED__

#include "IrrCompileConfig.h"
#include "irrTypes.h"
#include <math.h>

#if defined(_IRR_SOLARIS_PLATFORM_) || defined(__BORLANDC__) || defined (__BCPLUSPLUS__)
	#define sqrtf(X) (f32)sqrt((f64)(X))
	#define sinf(X) (f32)sin((f64)(X))
	#define cosf(X) (f32)cos((f64)(X))
	#define ceilf(X) (f32)ceil((f64)(X))
	#define floorf(X) (f32)floor((f64)(X))
	#define powf(X,Y) (f32)pow((f64)(X),(f64)(Y))
	#define fmodf(X,Y) (f32)fmod((f64)(X),(f64)(Y))
	#define fabsf(X) (f32)fabs((f64)(X))
#endif

namespace irr
{
namespace core
{

	//! Rounding error constant often used when comparing f32 values.

#ifdef IRRLICHT_FAST_MATH
	const f32 ROUNDING_ERROR_32 = 0.00005f;
	const f64 ROUNDING_ERROR_64 = 0.000005;
#else
	const f32 ROUNDING_ERROR_32 = 0.000001f;
	const f64 ROUNDING_ERROR_64 = 0.00000001;
#endif

#ifdef PI // make sure we don't collide with a define
#undef PI
#endif
	//! Constant for PI.
	const f32 PI		= 3.14159265359f;

	//! Constant for reciprocal of PI.
	const f32 RECIPROCAL_PI	= 1.0f/PI;

	//! Constant for half of PI.
	const f32 HALF_PI	= PI/2.0f;

#ifdef PI64 // make sure we don't collide with a define
#undef PI64
#endif
	//! Constant for 64bit PI.
	const f64 PI64		= 3.1415926535897932384626433832795028841971693993751;

	//! Constant for 64bit reciprocal of PI.
	const f64 RECIPROCAL_PI64 = 1.0/PI64;

	//! 32bit Constant for converting from degrees to radians
	const f32 DEGTORAD = PI / 180.0f;

	//! 32bit constant for converting from radians to degrees (formally known as GRAD_PI)
	const f32 RADTODEG   = 180.0f / PI;

	//! 64bit constant for converting from degrees to radians (formally known as GRAD_PI2)
	const f64 DEGTORAD64 = PI64 / 180.0;

	//! 64bit constant for converting from radians to degrees
	const f64 RADTODEG64 = 180.0 / PI64;

	//! returns minimum of two values. Own implementation to get rid of the STL (VS6 problems)
	template<class T>
	inline const T& min_(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	//! returns minimum of three values. Own implementation to get rid of the STL (VS6 problems)
	template<class T>
	inline const T& min_(const T& a, const T& b, const T& c)
	{
		return a < b ? min_(a, c) : min_(b, c);
	}

	//! returns maximum of two values. Own implementation to get rid of the STL (VS6 problems)
	template<class T>
	inline const T& max_(const T& a, const T& b)
	{
		return a < b ? b : a;
	}

	//! returns maximum of three values. Own implementation to get rid of the STL (VS6 problems)
	template<class T>
	inline const T& max_(const T& a, const T& b, const T& c)
	{
		return a < b ? max_(b, c) : max_(a, c);
	}

	//! returns abs of two values. Own implementation to get rid of STL (VS6 problems)
	template<class T>
	inline T abs_(const T& a)
	{
		return a < (T)0 ? -a : a;
	}

	//! returns linear interpolation of a and b with ratio t
	//! \return: a if t==0, b if t==1, and the linear interpolation else
	template<class T>
	inline T lerp(const T& a, const T& b, const f32 t)
	{
		return (T)(a*(1.f-t)) + (b*t);
	}

	//! clamps a value between low and high
	template <class T>
	inline const T clamp (const T& value, const T& low, const T& high)
	{
		return min_ (max_(value,low), high);
	}

	//! returns if a equals b, taking possible rounding errors into account
	inline bool equals(const f32 a, const f32 b, const f32 tolerance = ROUNDING_ERROR_32)
	{
		return (a + tolerance >= b) && (a - tolerance <= b);
	}

	//! returns if a equals b, taking possible rounding errors into account
	inline bool equals(const s32 a, const s32 b, const s32 tolerance = 0)
	{
		return (a + tolerance >= b) && (a - tolerance <= b);
	}

	//! returns if a equals b, taking possible rounding errors into account
	inline bool equals(const u32 a, const u32 b, const u32 tolerance = 0)
	{
		return (a + tolerance >= b) && (a - tolerance <= b);
	}

	//! returns if a equals zero, taking rounding errors into account
	inline bool iszero(const f32 a, const f32 tolerance = ROUNDING_ERROR_32)
	{
		return fabsf ( a ) <= tolerance;
	}

	//! returns if a equals zero, taking rounding errors into account
	inline bool iszero(const s32 a, const s32 tolerance = 0)
	{
		return ( a & 0x7ffffff ) <= tolerance;
	}

	//! returns if a equals zero, taking rounding errors into account
	inline bool iszero(const u32 a, const u32 tolerance = 0)
	{
		return a <= tolerance;
	}

	inline s32 s32_min ( s32 a, s32 b)
	{
		s32 mask = (a - b) >> 31;
		return (a & mask) | (b & ~mask);
	}

	inline s32 s32_max ( s32 a, s32 b)
	{
		s32 mask = (a - b) >> 31;
		return (b & mask) | (a & ~mask);
	}

	inline s32 s32_clamp (s32 value, s32 low, s32 high)
	{
		return s32_min (s32_max(value,low), high);
	}

	/*
		float IEEE-754 bit represenation

		0      0x00000000
		1.0    0x3f800000
		0.5    0x3f000000
		3      0x40400000
		+inf   0x7f800000
		-inf   0xff800000
		+NaN   0x7fc00000 or 0x7ff00000
		in general: number = (sign ? -1:1) * 2^(exponent) * 1.(mantissa bits)
	*/

	#define F32_AS_S32(f)		(*((s32 *) &(f)))
	#define F32_AS_U32(f)		(*((u32 *) &(f)))
	#define F32_AS_U32_POINTER(f)	( ((u32 *) &(f)))

	#define F32_VALUE_0		0x00000000
	#define F32_VALUE_1		0x3f800000
	#define F32_SIGN_BIT		0x80000000U
	#define F32_EXPON_MANTISSA	0x7FFFFFFFU

	//! code is taken from IceFPU
	//! Integer representation of a floating-point value.
	#define IR(x)				((u32&)(x))

	//! Absolute integer representation of a floating-point value
	#define AIR(x)				(IR(x)&0x7fffffff)

	//! Floating-point representation of an integer value.
	#define FR(x)				((f32&)(x))

	//! integer representation of 1.0
	#define IEEE_1_0			0x3f800000
	//! integer representation of 255.0
	#define IEEE_255_0			0x437f0000

#ifdef IRRLICHT_FAST_MATH
	#define	F32_LOWER_0(f)		(F32_AS_U32(f) >  F32_SIGN_BIT)
	#define	F32_LOWER_EQUAL_0(f)	(F32_AS_S32(f) <= F32_VALUE_0)
	#define	F32_GREATER_0(f)	(F32_AS_S32(f) >  F32_VALUE_0)
	#define	F32_GREATER_EQUAL_0(f)	(F32_AS_U32(f) <= F32_SIGN_BIT)
	#define	F32_EQUAL_1(f)		(F32_AS_U32(f) == F32_VALUE_1)
	#define	F32_EQUAL_0(f)		( (F32_AS_U32(f) & F32_EXPON_MANTISSA ) == F32_VALUE_0)

	// only same sign
	#define	F32_A_GREATER_B(a,b)	(F32_AS_S32((a)) > F32_AS_S32((b)))

#else

	#define	F32_LOWER_0(n)		((n) <  0.0f)
	#define	F32_LOWER_EQUAL_0(n)	((n) <= 0.0f)
	#define	F32_GREATER_0(n)	((n) >  0.0f)
	#define	F32_GREATER_EQUAL_0(n)	((n) >= 0.0f)
	#define	F32_EQUAL_1(n)		((n) == 1.0f)
	#define	F32_EQUAL_0(n)		((n) == 0.0f)
	#define	F32_A_GREATER_B(a,b)	((a) > (b))
#endif


#ifndef REALINLINE
	#ifdef _MSC_VER
		#define REALINLINE __forceinline
	#else
		#define REALINLINE inline
	#endif
#endif


	//! conditional set based on mask and arithmetic shift
	REALINLINE u32 if_c_a_else_b ( const s32 condition, const u32 a, const u32 b )
	{
		return ( ( -condition >> 31 ) & ( a ^ b ) ) ^ b;
	}

	//! conditional set based on mask and arithmetic shift
	REALINLINE u32 if_c_a_else_0 ( const s32 condition, const u32 a )
	{
		return ( -condition >> 31 ) & a;
	}

	/*
		if (condition) state |= m; else state &= ~m;
	*/
	REALINLINE void setbit_cond ( u32 &state, s32 condition, u32 mask )
	{
		// 0, or any postive to mask
		//s32 conmask = -condition >> 31;
		state ^= ( ( -condition >> 31 ) ^ state ) & mask;
	}



	inline f32 round_( f32 x )
	{
		return floorf( x + 0.5f );
	}

	REALINLINE void clearFPUException ()
	{
#ifdef IRRLICHT_FAST_MATH
#ifdef feclearexcept
		feclearexcept(FE_ALL_EXCEPT);
#elif defined(_MSC_VER)
		__asm fnclex;
#elif defined(__GNUC__) && defined(__x86__)
		__asm__ __volatile__ ("fclex \n\t");
#else
#  warn clearFPUException not supported.
#endif
#endif
	}

	REALINLINE f32 reciprocal_squareroot(const f32 x)
	{
#ifdef IRRLICHT_FAST_MATH
		// comes from Nvidia
#if 1
		u32 tmp = (u32(IEEE_1_0 << 1) + IEEE_1_0 - *(u32*)&x) >> 1;
		f32 y = *(f32*)&tmp;
		return y * (1.47f - 0.47f * x * y * y);
#elif defined(_MSC_VER)
		// an sse2 version
		__asm
		{
			movss	xmm0, x
			rsqrtss	xmm0, xmm0
			movss	x, xmm0
		}
		return x;
#endif
#else // no fast math
		return 1.f / sqrtf ( x );
#endif
	}



	REALINLINE f32 reciprocal ( const f32 f )
	{
#ifdef IRRLICHT_FAST_MATH
		//! i do not divide through 0.. (fpu expection)
		// instead set f to a high value to get a return value near zero..
		// -1000000000000.f.. is use minus to stay negative..
		// must test's here (plane.normal dot anything ) checks on <= 0.f
		return 1.f / f;
		//u32 x = (-(AIR(f) != 0 ) >> 31 ) & ( IR(f) ^ 0xd368d4a5 ) ^ 0xd368d4a5;
		//return 1.f / FR ( x );
#else // no fast math
		return 1.f / f;
#endif
	}


	REALINLINE f32 reciprocal_approxim ( const f32 p )
	{
#ifdef IRRLICHT_FAST_MATH
		register u32 x = 0x7F000000 - IR ( p );
		const f32 r = FR ( x );
		return r * (2.0f - p * r);
#else // no fast math
		return 1.f / p;
#endif
	}


	REALINLINE s32 floor32(f32 x)
	{
#ifdef IRRLICHT_FAST_MATH
		const f32 h = 0.5f;

		s32 t;

#if defined(_MSC_VER)
		__asm
		{
			fld	x
			fsub	h
			fistp	t
		}
#elif defined(__GNUC__)
		__asm__ __volatile__ (
			"fsub %2 \n\t"
			"fistpl %0"
			: "=m" (t)
			: "t" (x), "f" (h)
			: "st"
			);
#else
#  warn IRRLICHT_FAST_MATH not supported.
		return (s32) floorf ( x );
#endif
		return t;
#else // no fast math
		return (s32) floorf ( x );
#endif
	}


	REALINLINE s32 ceil32 ( f32 x )
	{
#ifdef IRRLICHT_FAST_MATH
		const f32 h = 0.5f;

		s32 t;

#if defined(_MSC_VER)
		__asm
		{
			fld	x
			fadd	h
			fistp	t
		}
#elif defined(__GNUC__)
		__asm__ __volatile__ (
			"fadd %2 \n\t"
			"fistpl %0 \n\t"
			: "=m"(t)
			: "t"(x), "f"(h)
			: "st"
			);
#else
#  warn IRRLICHT_FAST_MATH not supported.
		return (s32) ceilf ( x );
#endif
		return t;
#else // not fast math
		return (s32) ceilf ( x );
#endif
	}



	REALINLINE s32 round32(f32 x)
	{
#if defined(IRRLICHT_FAST_MATH)
		s32 t;

#if defined(_MSC_VER)
		__asm
		{
			fld   x
			fistp t
		}
#elif defined(__GNUC__)
		__asm__ __volatile__ (
			"fistpl %0 \n\t"
			: "=m"(t)
			: "t"(x)
			: "st"
			);
#else
#  warn IRRLICHT_FAST_MATH not supported.
		return (s32) round_(x);
#endif
		return t;
#else // no fast math
		return (s32) round_(x);
#endif
	}

	inline f32 f32_max3(const f32 a, const f32 b, const f32 c)
	{
		return a > b ? (a > c ? a : c) : (b > c ? b : c);
	}

	inline f32 f32_min3(const f32 a, const f32 b, const f32 c)
	{
		return a < b ? (a < c ? a : c) : (b < c ? b : c);
	}

	inline f32 fract ( f32 x )
	{
		return x - floorf ( x );
	}

} // end namespace core
} // end namespace irr

#endif

