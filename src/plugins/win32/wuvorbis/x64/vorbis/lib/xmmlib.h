/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2003             *
 * by the XIPHOPHORUS Company http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: Header of SSE Function Library
 last mod: $Id: xmmlib.h,v 1.3 2005-07-08 15:00:00+09 blacksword Exp $

 ********************************************************************/

#ifndef _XMMLIB_H_INCLUDED
#define _XMMLIB_H_INCLUDED

#if !defined(STIN)
#define STIN static __inline
#endif

#if defined(__SSE__)
#include <intrin.h>
#include <tmmintrin.h>

#define PM64(x)		(*(__m64*)(x))
#define PM128(x)	(*(__m128*)(x))
#ifdef	__SSE2__
#define PM128I(x)	(*(__m128i*)(x))
#define PM128D(x)	(*(__m128d*)(x))
#endif

typedef union {
	unsigned char	si8[8];
	unsigned short	si16[4];
	unsigned long	si32[2];
	char			ssi8[8];
	short			ssi16[4];
	long			ssi32[2];
	__m64			pi64;
} __m64x;

typedef union __declspec(intrin_type) _MM_ALIGN16 __m128x{
	unsigned long	si32[4];
	float			sf[4];
	__m64			pi64[2];
	__m128			ps;
#ifdef	__SSE2__
	__m128i			pi;
	__m128d			pd;
#endif
} __m128x;

#if defined(__SSE3__)
#define	_mm_lddqu_ps(x)	_mm_castsi128_ps(_mm_lddqu_si128((__m128i*)(x)))
#else
#define	_mm_lddqu_ps(x)	_mm_loadu_ps(x)
#endif

extern _MM_ALIGN16 const unsigned long PCS_NNRN[4];
extern _MM_ALIGN16 const unsigned long PCS_NNRR[4];
extern _MM_ALIGN16 const unsigned long PCS_NRNN[4];
extern _MM_ALIGN16 const unsigned long PCS_NRNR[4];
extern _MM_ALIGN16 const unsigned long PCS_NRRN[4];
extern _MM_ALIGN16 const unsigned long PCS_NRRR[4];
extern _MM_ALIGN16 const unsigned long PCS_RNNN[4];
extern _MM_ALIGN16 const unsigned long PCS_RNRN[4];
extern _MM_ALIGN16 const unsigned long PCS_RNRR[4];
extern _MM_ALIGN16 const unsigned long PCS_RRNN[4];
extern _MM_ALIGN16 const unsigned long PCS_RNNR[4];
extern _MM_ALIGN16 const unsigned long PCS_RRRR[4];
extern _MM_ALIGN16 const unsigned long PCS_NNNR[4];
extern _MM_ALIGN16 const unsigned long PABSMASK[4];
extern _MM_ALIGN16 const unsigned long PSTARTEDGEM1[4];
extern _MM_ALIGN16 const unsigned long PSTARTEDGEM2[4];
extern _MM_ALIGN16 const unsigned long PSTARTEDGEM3[4];
extern _MM_ALIGN16 const unsigned long PENDEDGEM1[4];
extern _MM_ALIGN16 const unsigned long PENDEDGEM2[4];
extern _MM_ALIGN16 const unsigned long PENDEDGEM3[4];
extern _MM_ALIGN16 const unsigned long PMASKTABLE[16*4];

extern _MM_ALIGN16 const float PFV_0[4];
extern _MM_ALIGN16 const float PFV_1[4];
extern _MM_ALIGN16 const float PFV_2[4];
extern _MM_ALIGN16 const float PFV_4[4];
extern _MM_ALIGN16 const float PFV_8[4];
extern _MM_ALIGN16 const float PFV_INIT[4];
extern _MM_ALIGN16 const float PFV_0P5[4];
extern _MM_ALIGN16 const float PFV_M0P5[4];

extern const int bitCountTable[16];

extern void* xmm_malloc(size_t);
extern void* xmm_calloc(size_t, size_t);
extern void* xmm_realloc(void*, size_t);
extern void* xmm_align(void*);
extern void xmm_free(void*);

#if defined(_OPENMP)
STIN int _omp_get_start(int n,int partation, int thnum, int thmax)
{
  int psize = n/thmax;
  psize = (psize/partation)*partation;
  if(psize==0)
	if(thnum==0)
	  return 0;
	else
	  return n;
  else
	return psize*thnum;
}

STIN int _omp_get_end(int n,int partation, int thnum, int thmax)
{
  int psize = n/thmax;
  psize = (psize/partation)*partation;
  if(psize==0)
	  return n;
  else
	if(thnum==thmax-1)
	  return n;
	else
	  return psize*(thnum+1);
}
#endif

STIN __m128 _mm_todB_ps(__m128 x)
{
	static _MM_ALIGN16 float mparm[4] = {
		7.17711438e-7f, 7.17711438e-7f, 7.17711438e-7f, 7.17711438e-7f
	};
	static _MM_ALIGN16 float aparm[4] = {
		-764.6161886f, -764.6161886f, -764.6161886f, -764.6161886f
	};
#ifdef	__SSE2__
	__m128x	U;
	U.ps	 = _mm_and_ps(x, PM128(PABSMASK));
	U.ps	 = _mm_cvtepi32_ps(U.pi);
	U.ps	 = _mm_mul_ps(U.ps, PM128(mparm));
	U.ps	 = 	_mm_add_ps(U.ps, PM128(aparm));
	return	U.ps;
#else
#pragma warning(disable : 592)
	__m128	RESULT;
	__m128x	U;
	U.ps	 = _mm_and_ps(x, PM128(PABSMASK));
	RESULT	 = _mm_cvtpi32_ps(RESULT, U.pi64[1]);
#pragma warning(default : 592)
	RESULT	 = _mm_movelh_ps(RESULT, RESULT);
	RESULT	 = _mm_cvtpi32_ps(RESULT, U.pi64[0]);
	RESULT	 = _mm_mul_ps(RESULT, PM128(mparm));
	RESULT	 = _mm_add_ps(RESULT, PM128(aparm));
	return	RESULT;
#endif
}

STIN __m128 _mm_untnorm_ps(__m128 x)
{
	static _MM_ALIGN16 const unsigned long PIV0[4] = {
		0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000
	};
	register __m128 r;
	r	 = _mm_and_ps(x, PM128(PCS_RRRR));
	r	 = _mm_or_ps(x, PM128(PIV0));
	return	r;
}

STIN float _mm_add_horz(__m128 x)
{
#if	defined(__SSE3__)
	x	 = _mm_hadd_ps(x, x);
	x	 = _mm_hadd_ps(x, x);
#else
#pragma warning(disable : 592)
	__m128	y;
	y	 = _mm_movehl_ps(y, x);
#pragma warning(default : 592)
	x	 = _mm_add_ps(x, y);
	y	 = x;
	y	 = _mm_shuffle_ps(y, y, _MM_SHUFFLE(1,1,1,1));
	x	 = _mm_add_ss(x, y);
#endif
	return _mm_cvtss_f32(x);
}

STIN __m128 _mm_add_horz_ss(__m128 x)
{
#if	defined(__SSE3__)
	x	 = _mm_hadd_ps(x, x);
	x	 = _mm_hadd_ps(x, x);
#else
#pragma warning(disable : 592)
	__m128	y;
	y	 = _mm_movehl_ps(y, x);
#pragma warning(default : 592)
	x	 = _mm_add_ps(x, y);
	y	 = x;
	y	 = _mm_shuffle_ps(y, y, _MM_SHUFFLE(1,1,1,1));
	x	 = _mm_add_ss(x, y);
#endif
	return x;
}

STIN float _mm_max_horz(__m128 x)
{
#pragma warning(disable : 592)
	__m128	y;
	y	 = _mm_movehl_ps(y, x);
#pragma warning(default : 592)
	x	 = _mm_max_ps(x, y);
	y	 = x;
	y	 = _mm_shuffle_ps(y, y, _MM_SHUFFLE(1,1,1,1));
	x	 = _mm_max_ss(x, y);
	return _mm_cvtss_f32(x);
}

STIN float _mm_min_horz(__m128 x)
{
#pragma warning(disable : 592)
	__m128	y;
	y	 = _mm_movehl_ps(y, x);
#pragma warning(default : 592)
	x	 = _mm_min_ps(x, y);
	y	 = x;
	y	 = _mm_shuffle_ps(y, y, _MM_SHUFFLE(1,1,1,1));
	x	 = _mm_min_ss(x, y);
	return _mm_cvtss_f32(x);
}

#endif /* defined(__SSE__) */

#if	0
/*---------------------------------------------------------------------------
// for calcurate performance
//-------------------------------------------------------------------------*/
extern unsigned __int64* _perf_start(void);
extern void _perf_end(unsigned __int64 *stime, int index);
extern void _perf_result(int index);
#endif

#endif /* _XMMLIB_H_INCLUDED */
