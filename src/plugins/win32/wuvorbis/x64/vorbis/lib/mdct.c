/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2002             *
 * by the XIPHOPHORUS Company http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: normalized modified discrete cosine transform
           power of two length transform only [64 <= n ]
 last mod: $Id: mdct.c 7187 2004-07-20 07:24:27Z xiphmont $

 Original algorithm adapted long ago from _The use of multirate filter
 banks for coding of high quality digital audio_, by T. Sporer,
 K. Brandenburg and B. Edler, collection of the European Signal
 Processing Conference (EUSIPCO), Amsterdam, June 1992, Vol.1, pp
 211-214

 The below code implements an algorithm that no longer looks much like
 that presented in the paper, but the basic structure remains if you
 dig deep enough to see it.

 This module DOES NOT INCLUDE code to generate/apply the window
 function.  Everybody has their own weird favorite including me... I
 happen to like the properties of y=sin(.5PI*sin^2(x)), but others may
 vehemently disagree.

 ********************************************************************/

/* this can also be run as an integer transform by uncommenting a
   define in mdct.h; the integerization is a first pass and although
   it's likely stable for Vorbis, the dynamic range is constrained and
   roundoff isn't done (so it's noisy).  Consider it functional, but
   only a starting point.  There's no point on a machine with an FPU */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vorbis/codec.h"
#include "mdct.h"
#include "os.h"
#include "misc.h"
#ifdef __SSE__												/* SSE Optimize */
#include "xmmlib.h"
#endif														/* SSE Optimize */


/* build lookups for trig functions; also pre-figure scaling and
   some window function algebra. */

void mdct_init(mdct_lookup *lookup,int n){
  int   *bitrev=_ogg_malloc(sizeof(*bitrev)*(n/4));
  DATA_TYPE *T=_ogg_malloc(sizeof(*T)*(n+n/4));
  
  int i;
  int n2=n>>1;
  int log2n=lookup->log2n=rint(log((float)n)/log(2.f));
  lookup->n=n;
  lookup->trig=T;
  lookup->bitrev=bitrev;

/* trig lookups... */

  for(i=0;i<n/4;i++){
    T[i*2]=FLOAT_CONV(cos((M_PI/n)*(4*i)));
    T[i*2+1]=FLOAT_CONV(-sin((M_PI/n)*(4*i)));
    T[n2+i*2]=FLOAT_CONV(cos((M_PI/(2*n))*(2*i+1)));
    T[n2+i*2+1]=FLOAT_CONV(sin((M_PI/(2*n))*(2*i+1)));
  }
  for(i=0;i<n/8;i++){
    T[n+i*2]=FLOAT_CONV(cos((M_PI/n)*(4*i+2))*.5);
    T[n+i*2+1]=FLOAT_CONV(-sin((M_PI/n)*(4*i+2))*.5);
  }

  /* bitreverse lookup... */

  {
    int mask=(1<<(log2n-1))-1,i,j;
    int msb=1<<(log2n-2);
    for(i=0;i<n/8;i++){
      int acc=0;
      for(j=0;msb>>j;j++)
	if((msb>>j)&i)acc|=1<<j;
      bitrev[i*2]=((~acc)&mask)-1;
      bitrev[i*2+1]=acc;

    }
  }
  lookup->scale=FLOAT_CONV(4.f/n);
#ifdef __SSE__												/* SSE Optimize */
	{
		__m128	pscalem	 = _mm_set_ps1(lookup->scale);
		float *S, *U;
		int n2	 = n>>1;
		int n4	 = n>>2;
		int n8	 = n>>3;
		int j;
		/*
			for mdct_bitreverse
		*/
		T	 = _ogg_malloc(sizeof(*T)*n2);
		lookup->trig_bitreverse	 = T;
		S	 = lookup->trig+n;
		for(i=0;i<n4;i+=8)
		{
			__m128	XMM0	 = _mm_load_ps(S+i   );
			__m128	XMM1	 = _mm_load_ps(S+i+ 4);
			__m128	XMM2	 = XMM0;
			__m128	XMM3	 = XMM1;
			XMM2	 = _mm_shuffle_ps(XMM2, XMM2, _MM_SHUFFLE(2,3,0,1));
			XMM3	 = _mm_shuffle_ps(XMM3, XMM3, _MM_SHUFFLE(2,3,0,1));
			XMM2	 = _mm_xor_ps(XMM2, PM128(PCS_RNRN));
			XMM3	 = _mm_xor_ps(XMM3, PM128(PCS_RNRN));
			_mm_store_ps(T+i*2   , XMM0);
			_mm_store_ps(T+i*2+ 4, XMM2);
			_mm_store_ps(T+i*2+ 8, XMM1);
			_mm_store_ps(T+i*2+12, XMM3);
		}
		/*
			for mdct_forward part 0
		*/
		T	 = _ogg_malloc(sizeof(*T)*(n*2));
		lookup->trig_forward	 = T;
		S	 = lookup->trig;
		for(i=0,j=n2-4;i<n8;i+=4,j-=4)
		{
			__m128	XMM0, XMM1, XMM2, XMM3;
#ifdef _DEBUG
			XMM0 = XMM1 = XMM2 = XMM3 = _mm_setzero_ps();
#endif
#pragma warning(disable : 592)
			XMM0	 = _mm_loadl_pi(XMM0, (__m64*)(S+j+2));
			XMM2	 = _mm_loadl_pi(XMM2, (__m64*)(S+j  ));
			XMM0	 = _mm_loadh_pi(XMM0, (__m64*)(S+i  ));
			XMM2	 = _mm_loadh_pi(XMM2, (__m64*)(S+i+2));
#pragma warning(default : 592)
			XMM1	 = XMM0;
			XMM3	 = XMM2;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM0, _MM_SHUFFLE(2,3,0,1));
			XMM2	 = _mm_shuffle_ps(XMM2, XMM2, _MM_SHUFFLE(2,3,0,1));
			XMM0	 = _mm_xor_ps(XMM0, PM128(PCS_RRNN));
			XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RNNR));
			XMM2	 = _mm_xor_ps(XMM2, PM128(PCS_RRNN));
			XMM3	 = _mm_xor_ps(XMM3, PM128(PCS_RNNR));
			_mm_store_ps(T+i*4   , XMM0);
			_mm_store_ps(T+i*4+ 4, XMM1);
			_mm_store_ps(T+i*4+ 8, XMM2);
			_mm_store_ps(T+i*4+12, XMM3);
		}
		for(;i<n4;i+=4,j-=4)
		{
			__m128	XMM0, XMM1, XMM2, XMM3;
#ifdef _DEBUG
			XMM0 = XMM1 = XMM2 = XMM3 = _mm_setzero_ps();
#endif
#pragma warning(disable : 592)
			XMM0	 = _mm_loadl_pi(XMM0, (__m64*)(S+j+2));
			XMM2	 = _mm_loadl_pi(XMM2, (__m64*)(S+j  ));
			XMM0	 = _mm_loadh_pi(XMM0, (__m64*)(S+i  ));
			XMM2	 = _mm_loadh_pi(XMM2, (__m64*)(S+i+2));
#pragma warning(default : 592)
			XMM1	 = XMM0;
			XMM3	 = XMM2;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM0, _MM_SHUFFLE(2,3,0,1));
			XMM2	 = _mm_shuffle_ps(XMM2, XMM2, _MM_SHUFFLE(2,3,0,1));
			XMM0	 = _mm_xor_ps(XMM0, PM128(PCS_NNRR));
			XMM2	 = _mm_xor_ps(XMM2, PM128(PCS_NNRR));
			XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RNNR));
			XMM3	 = _mm_xor_ps(XMM3, PM128(PCS_RNNR));
			_mm_store_ps(T+i*4   , XMM0);
			_mm_store_ps(T+i*4+ 4, XMM1);
			_mm_store_ps(T+i*4+ 8, XMM2);
			_mm_store_ps(T+i*4+12, XMM3);
		}
		/*
			for mdct_forward part 1
		*/
		T	 = lookup->trig_forward+n;
		S	 = lookup->trig+n2;
		for(i=0;i<n4;i+=4){
			__m128	XMM0, XMM1, XMM2;
			XMM0	 = _mm_load_ps(S+4);
			XMM2	 = _mm_load_ps(S  );
			XMM1	 = XMM0;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM2,_MM_SHUFFLE(1,3,1,3));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM2,_MM_SHUFFLE(0,2,0,2));
			XMM0	 = _mm_mul_ps(XMM0, pscalem);
			XMM1	 = _mm_mul_ps(XMM1, pscalem);
			_mm_store_ps(T   , XMM0);
			_mm_store_ps(T+ 4, XMM1);
			XMM1	 = _mm_shuffle_ps(XMM1, XMM1, _MM_SHUFFLE(0,1,2,3));
			XMM0	 = _mm_shuffle_ps(XMM0, XMM0, _MM_SHUFFLE(0,1,2,3));
			_mm_store_ps(T+ 8, XMM1);
			_mm_store_ps(T+12, XMM0);
			S		+= 8;
			T		+= 16;
		}
		/*
			for mdct_backward part 0
		*/
		S	 = U	 = lookup->trig+n4;
		T	 = _ogg_malloc(sizeof(*T)*(n+n2));
		lookup->trig_backward	 = T;
		for(i=0;i<n4;i+=4)
		{
			__m128	XMM0, XMM1, XMM2, XMM3;
			U		-= 4;
			XMM0	 = _mm_load_ps(S);
			XMM2	 = _mm_load_ps(U);
			XMM1	 = XMM0;
			XMM3	 = XMM2;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM0, _MM_SHUFFLE(1,1,3,3));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM1, _MM_SHUFFLE(0,0,2,2));
			XMM2	 = _mm_shuffle_ps(XMM2, XMM2, _MM_SHUFFLE(0,1,2,3));
			XMM3	 = _mm_shuffle_ps(XMM3, XMM3, _MM_SHUFFLE(1,0,3,2));
			XMM0	 = _mm_xor_ps(XMM0, PM128(PCS_NRNR));
			XMM3	 = _mm_xor_ps(XMM3, PM128(PCS_NRNR));
			_mm_store_ps(T   , XMM0);
			_mm_store_ps(T+ 4, XMM1);
			_mm_store_ps(T+ 8, XMM2);
			_mm_store_ps(T+12, XMM3);
			S		+= 4;
			T		+= 16;
		}
		/*
			for mdct_backward part 1
		*/
		S	 = lookup->trig+n2;
		T	 = lookup->trig_backward+n;
		for(i=0;i<n4;i+=4)
		{
			__m128	XMM0, XMM1, XMM2;
			XMM0	 = _mm_load_ps(S  );
			XMM2	 = _mm_load_ps(S+4);
			XMM1	 = XMM0;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM2, _MM_SHUFFLE(3,1,3,1));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM2, _MM_SHUFFLE(2,0,2,0));
			_mm_store_ps(T  , XMM0);
			_mm_store_ps(T+4, XMM1);
			S		+= 8;
			T		+= 8;
		}
		/*
			for mdct_butterfly_first
		*/
		S	 = lookup->trig;
		T	 = _ogg_malloc(sizeof(*T)*n*2);
		lookup->trig_butterfly_first	 = T;
		for(i=0;i<n4;i+=4)
		{
			__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5;
			XMM2	 = _mm_load_ps(S   );
			XMM0	 = _mm_load_ps(S+ 4);
			XMM5	 = _mm_load_ps(S+ 8);
			XMM3	 = _mm_load_ps(S+12);
			XMM1	 = XMM0;
			XMM4	 = XMM3;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM2, _MM_SHUFFLE(0,1,0,1));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM2, _MM_SHUFFLE(1,0,1,0));
			XMM3	 = _mm_shuffle_ps(XMM3, XMM5, _MM_SHUFFLE(0,1,0,1));
			XMM4	 = _mm_shuffle_ps(XMM4, XMM5, _MM_SHUFFLE(1,0,1,0));
			XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RNRN));
			XMM4	 = _mm_xor_ps(XMM4, PM128(PCS_RNRN));
			_mm_store_ps(T   , XMM1);
			_mm_store_ps(T+ 4, XMM4);
			_mm_store_ps(T+ 8, XMM0);
			_mm_store_ps(T+12, XMM3);
			XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RRRR));
			XMM4	 = _mm_xor_ps(XMM4, PM128(PCS_RRRR));
			XMM0	 = _mm_xor_ps(XMM0, PM128(PCS_RRRR));
			XMM3	 = _mm_xor_ps(XMM3, PM128(PCS_RRRR));
			_mm_store_ps(T+n   , XMM1);
			_mm_store_ps(T+n+ 4, XMM4);
			_mm_store_ps(T+n+ 8, XMM0);
			_mm_store_ps(T+n+12, XMM3);
			S	+= 16;
			T	+= 16;
		}
		/*
			for mdct_butterfly_generic(trigint=8)
		*/
		S	 = lookup->trig;
		T	 = _ogg_malloc(sizeof(*T)*n2);
		lookup->trig_butterfly_generic8	 = T;
		for(i=0;i<n;i+=32)
		{
			__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5;

			XMM0	 = _mm_load_ps(S+ 24);
			XMM2	 = _mm_load_ps(S+ 16);
			XMM3	 = _mm_load_ps(S+  8);
			XMM5	 = _mm_load_ps(S    );
			XMM1	 = XMM0;
			XMM4	 = XMM3;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM2, _MM_SHUFFLE(0,1,0,1));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM2, _MM_SHUFFLE(1,0,1,0));
			XMM3	 = _mm_shuffle_ps(XMM3, XMM5, _MM_SHUFFLE(0,1,0,1));
			XMM4	 = _mm_shuffle_ps(XMM4, XMM5, _MM_SHUFFLE(1,0,1,0));
			XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RNRN));
			XMM4	 = _mm_xor_ps(XMM4, PM128(PCS_RNRN));
			_mm_store_ps(T   , XMM0);
			_mm_store_ps(T+ 4, XMM1);
			_mm_store_ps(T+ 8, XMM3);
			_mm_store_ps(T+12, XMM4);
			S	+= 32;
			T	+= 16;
		}
		/*
			for mdct_butterfly_generic(trigint=16)
		*/
		S	 = lookup->trig;
		T	 = _ogg_malloc(sizeof(*T)*n4);
		lookup->trig_butterfly_generic16	 = T;
		for(i=0;i<n;i+=64)
		{
			__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5;

			XMM0	 = _mm_load_ps(S+ 48);
			XMM2	 = _mm_load_ps(S+ 32);
			XMM3	 = _mm_load_ps(S+ 16);
			XMM5	 = _mm_load_ps(S    );
			XMM1	 = XMM0;
			XMM4	 = XMM3;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM2, _MM_SHUFFLE(0,1,0,1));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM2, _MM_SHUFFLE(1,0,1,0));
			XMM3	 = _mm_shuffle_ps(XMM3, XMM5, _MM_SHUFFLE(0,1,0,1));
			XMM4	 = _mm_shuffle_ps(XMM4, XMM5, _MM_SHUFFLE(1,0,1,0));
			XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RNRN));
			XMM4	 = _mm_xor_ps(XMM4, PM128(PCS_RNRN));
			_mm_store_ps(T   , XMM0);
			_mm_store_ps(T+ 4, XMM1);
			_mm_store_ps(T+ 8, XMM3);
			_mm_store_ps(T+12, XMM4);
			S	+= 64;
			T	+= 16;
		}
		/*
			for mdct_butterfly_generic(trigint=32)
		*/
		if(n<128)
			lookup->trig_butterfly_generic32	 = NULL;
		else
		{
			S	 = lookup->trig;
			T	 = _ogg_malloc(sizeof(*T)*n8);
			lookup->trig_butterfly_generic32	 = T;
			for(i=0;i<n;i+=128)
			{
				__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5;
	
				XMM0	 = _mm_load_ps(S+ 96);
				XMM2	 = _mm_load_ps(S+ 64);
				XMM3	 = _mm_load_ps(S+ 32);
				XMM5	 = _mm_load_ps(S    );
				XMM1	 = XMM0;
				XMM4	 = XMM3;
				XMM0	 = _mm_shuffle_ps(XMM0, XMM2, _MM_SHUFFLE(0,1,0,1));
				XMM1	 = _mm_shuffle_ps(XMM1, XMM2, _MM_SHUFFLE(1,0,1,0));
				XMM3	 = _mm_shuffle_ps(XMM3, XMM5, _MM_SHUFFLE(0,1,0,1));
				XMM4	 = _mm_shuffle_ps(XMM4, XMM5, _MM_SHUFFLE(1,0,1,0));
				XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RNRN));
				XMM4	 = _mm_xor_ps(XMM4, PM128(PCS_RNRN));
				_mm_store_ps(T   , XMM0);
				_mm_store_ps(T+ 4, XMM1);
				_mm_store_ps(T+ 8, XMM3);
				_mm_store_ps(T+12, XMM4);
				S	+= 128;
				T	+= 16;
			}
		}
		/*
			for mdct_butterfly_generic(trigint=64)
		*/
		if(n<256)
			lookup->trig_butterfly_generic64	 = NULL;
		else
		{
			S	 = lookup->trig;
			T	 = _ogg_malloc(sizeof(*T)*(n8>>1));
			lookup->trig_butterfly_generic64	 = T;
			for(i=0;i<n;i+=256)
			{
				__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5;
	
				XMM0	 = _mm_load_ps(S+192);
				XMM2	 = _mm_load_ps(S+128);
				XMM3	 = _mm_load_ps(S+ 64);
				XMM5	 = _mm_load_ps(S    );
				XMM1	 = XMM0;
				XMM4	 = XMM3;
				XMM0	 = _mm_shuffle_ps(XMM0, XMM2, _MM_SHUFFLE(0,1,0,1));
				XMM1	 = _mm_shuffle_ps(XMM1, XMM2, _MM_SHUFFLE(1,0,1,0));
				XMM3	 = _mm_shuffle_ps(XMM3, XMM5, _MM_SHUFFLE(0,1,0,1));
				XMM4	 = _mm_shuffle_ps(XMM4, XMM5, _MM_SHUFFLE(1,0,1,0));
				XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RNRN));
				XMM4	 = _mm_xor_ps(XMM4, PM128(PCS_RNRN));
				_mm_store_ps(T   , XMM0);
				_mm_store_ps(T+ 4, XMM1);
				_mm_store_ps(T+ 8, XMM3);
				_mm_store_ps(T+12, XMM4);
				S	+= 256;
				T	+= 16;
			}
		}
	}
#endif														/* SSE Optimize */
}

/* 8 point butterfly (in place, 4 register) */
STIN void mdct_butterfly_8(DATA_TYPE *x){
#ifdef __SSE__												/* SSE Optimize */
	__m128	XMM0, XMM1, XMM2, XMM3;
	XMM0	 = _mm_load_ps(x+4);
	XMM1	 = _mm_load_ps(x  );
	XMM2	 = XMM0;
	XMM0	 = _mm_sub_ps(XMM0, XMM1);
	XMM2	 = _mm_add_ps(XMM2, XMM1);

	XMM1	 = XMM0;
	XMM3	 = XMM2;

	XMM0	 = _mm_shuffle_ps(XMM0, XMM0, _MM_SHUFFLE(3,2,3,2));
	XMM1	 = _mm_shuffle_ps(XMM1, XMM1, _MM_SHUFFLE(0,1,0,1));
	XMM2	 = _mm_shuffle_ps(XMM2, XMM2, _MM_SHUFFLE(3,2,3,2));
	XMM3	 = _mm_shuffle_ps(XMM3, XMM3, _MM_SHUFFLE(1,0,1,0));

	XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_NRRN));
	XMM3	 = _mm_xor_ps(XMM3, PM128(PCS_NNRR));

	XMM0	 = _mm_add_ps(XMM0, XMM1);
	XMM2	 = _mm_add_ps(XMM2, XMM3);

	_mm_store_ps(x  , XMM0);
	_mm_store_ps(x+4, XMM2);
#else														/* SSE Optimize */
  REG_TYPE r0   = x[6] + x[2];
  REG_TYPE r1   = x[6] - x[2];
  REG_TYPE r2   = x[4] + x[0];
  REG_TYPE r3   = x[4] - x[0];

	   x[6] = r0   + r2;
	   x[4] = r0   - r2;
	   
	   r0   = x[5] - x[1];
	   r2   = x[7] - x[3];
	   x[0] = r1   + r0;
	   x[2] = r1   - r0;
	   
	   r0   = x[5] + x[1];
	   r1   = x[7] + x[3];
	   x[3] = r2   + r3;
	   x[1] = r2   - r3;
	   x[7] = r1   + r0;
	   x[5] = r1   - r0;
	   
#endif														/* SSE Optimize */
}

/* 16 point butterfly (in place, 4 register) */
STIN void mdct_butterfly_16(DATA_TYPE *x){
#ifdef __SSE__												/* SSE Optimize */
	static _MM_ALIGN16 const float PFV0[4] = { cPI2_8,  cPI2_8,     1.f,    -1.f};
	static _MM_ALIGN16 const float PFV1[4] = { cPI2_8, -cPI2_8,     0.f,     0.f};
	static _MM_ALIGN16 const float PFV2[4] = { cPI2_8,  cPI2_8,     1.f,     1.f};
	static _MM_ALIGN16 const float PFV3[4] = {-cPI2_8,  cPI2_8,     0.f,     0.f};
	__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;

	XMM3	 = _mm_load_ps(x+12);
	XMM0	 = _mm_load_ps(x   );
	XMM1	 = _mm_load_ps(x+ 4);
	XMM2	 = _mm_load_ps(x+ 8);
	XMM4	 = XMM3;
	XMM5	 = XMM0;
	XMM0	 = _mm_sub_ps(XMM0, XMM2);
	XMM3	 = _mm_sub_ps(XMM3, XMM1);
	XMM2	 = _mm_add_ps(XMM2, XMM5);
	XMM4	 = _mm_add_ps(XMM4, XMM1);
	XMM1	 = XMM0;
	XMM5	 = XMM3;
	_mm_store_ps(x+ 8, XMM2);
	_mm_store_ps(x+12, XMM4);
	XMM0	 = _mm_shuffle_ps(XMM0, XMM0, _MM_SHUFFLE(2,3,1,1));
	XMM2	 = _mm_load_ps(PFV0);
	XMM1	 = _mm_shuffle_ps(XMM1, XMM1, _MM_SHUFFLE(2,3,0,0));
	XMM4	 = _mm_load_ps(PFV1);
	XMM3	 = _mm_shuffle_ps(XMM3, XMM3, _MM_SHUFFLE(3,2,0,0));
	XMM6	 = _mm_load_ps(PFV2);
	XMM5	 = _mm_shuffle_ps(XMM5, XMM5, _MM_SHUFFLE(3,2,1,1));
	XMM7	 = _mm_load_ps(PFV3);
	XMM0	 = _mm_mul_ps(XMM0, XMM2);
	XMM1	 = _mm_mul_ps(XMM1, XMM4);
	XMM3	 = _mm_mul_ps(XMM3, XMM6);
	XMM5	 = _mm_mul_ps(XMM5, XMM7);
	XMM0	 = _mm_add_ps(XMM0, XMM1);
	XMM3	 = _mm_add_ps(XMM3, XMM5);
	_mm_store_ps(x   , XMM0);
	_mm_store_ps(x+ 4, XMM3);
#else														/* SSE Optimize */
  REG_TYPE r0     = x[1]  - x[9];
  REG_TYPE r1     = x[0]  - x[8];

           x[8]  += x[0];
           x[9]  += x[1];
           x[0]   = MULT_NORM((r0   + r1) * cPI2_8);
           x[1]   = MULT_NORM((r0   - r1) * cPI2_8);

           r0     = x[3]  - x[11];
           r1     = x[10] - x[2];
           x[10] += x[2];
           x[11] += x[3];
           x[2]   = r0;
           x[3]   = r1;

           r0     = x[12] - x[4];
           r1     = x[13] - x[5];
           x[12] += x[4];
           x[13] += x[5];
           x[4]   = MULT_NORM((r0   - r1) * cPI2_8);
           x[5]   = MULT_NORM((r0   + r1) * cPI2_8);

           r0     = x[14] - x[6];
           r1     = x[15] - x[7];
           x[14] += x[6];
           x[15] += x[7];
           x[6]  = r0;
           x[7]  = r1;
#endif														/* SSE Optimize */

	   mdct_butterfly_8(x);
	   mdct_butterfly_8(x+8);
}

/* 32 point butterfly (in place, 4 register) */
STIN void mdct_butterfly_32(DATA_TYPE *x){
#ifdef __SSE__												/* SSE Optimize */
	static _MM_ALIGN16 const float PFV0[4]	 = {-cPI3_8, -cPI1_8, -cPI2_8, -cPI2_8};
	static _MM_ALIGN16 const float PFV1[4]	 = {-cPI1_8,  cPI3_8, -cPI2_8,  cPI2_8};
	static _MM_ALIGN16 const float PFV2[4]	 = {-cPI1_8, -cPI3_8,    -1.f,     1.f};
	static _MM_ALIGN16 const float PFV3[4]	 = {-cPI3_8,  cPI1_8,     0.f,     0.f};
	static _MM_ALIGN16 const float PFV4[4]	 = { cPI3_8,  cPI3_8,  cPI2_8,  cPI2_8};
	static _MM_ALIGN16 const float PFV5[4]	 = {-cPI1_8,  cPI1_8, -cPI2_8,  cPI2_8};
	static _MM_ALIGN16 const float PFV6[4]	 = { cPI1_8,  cPI3_8,     1.f,     1.f};
	static _MM_ALIGN16 const float PFV7[4]	 = {-cPI3_8,  cPI1_8,     0.f,     0.f};
	__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;

	XMM0	 = _mm_load_ps(x+16);
	XMM1	 = _mm_load_ps(x+20);
	XMM2	 = _mm_load_ps(x+24);
	XMM3	 = _mm_load_ps(x+28);
	XMM4	 = XMM0;
	XMM5	 = XMM1;
	XMM6	 = XMM2;
	XMM7	 = XMM3;

	XMM0	 = _mm_sub_ps(XMM0, PM128(x   ));
	XMM1	 = _mm_sub_ps(XMM1, PM128(x+ 4));
	XMM2	 = _mm_sub_ps(XMM2, PM128(x+ 8));
	XMM3	 = _mm_sub_ps(XMM3, PM128(x+12));
	XMM4	 = _mm_add_ps(XMM4, PM128(x   ));
	XMM5	 = _mm_add_ps(XMM5, PM128(x+ 4));
	XMM6	 = _mm_add_ps(XMM6, PM128(x+ 8));
	XMM7	 = _mm_add_ps(XMM7, PM128(x+12));
	_mm_store_ps(x+16, XMM4);
	_mm_store_ps(x+20, XMM5);
	_mm_store_ps(x+24, XMM6);
	_mm_store_ps(x+28, XMM7);

#if	defined(__SSE3__)
	XMM4	 = _mm_moveldup_ps(XMM0);
	XMM5	 = XMM1;
	XMM0	 = _mm_movehdup_ps(XMM0);
	XMM6	 = XMM2;
	XMM7	 = XMM3;
#else
	XMM4	 = XMM0;
	XMM5	 = XMM1;
	XMM6	 = XMM2;
	XMM7	 = XMM3;

	XMM0	 = _mm_shuffle_ps(XMM0, XMM0, _MM_SHUFFLE(3,3,1,1));
	XMM4	 = _mm_shuffle_ps(XMM4, XMM4, _MM_SHUFFLE(2,2,0,0));
#endif
	XMM1	 = _mm_shuffle_ps(XMM1, XMM1, _MM_SHUFFLE(2,3,1,1));
	XMM5	 = _mm_shuffle_ps(XMM5, XMM5, _MM_SHUFFLE(2,3,0,0));
	XMM2	 = _mm_shuffle_ps(XMM2, XMM2, _MM_SHUFFLE(2,2,1,0));
	XMM6	 = _mm_shuffle_ps(XMM6, XMM6, _MM_SHUFFLE(3,3,0,1));
	XMM3	 = _mm_shuffle_ps(XMM3, XMM3, _MM_SHUFFLE(3,2,0,0));
	XMM7	 = _mm_shuffle_ps(XMM7, XMM7, _MM_SHUFFLE(3,2,1,1));
	XMM0	 = _mm_mul_ps(XMM0, PM128(PFV0));
	XMM4	 = _mm_mul_ps(XMM4, PM128(PFV1));
	XMM1	 = _mm_mul_ps(XMM1, PM128(PFV2));
	XMM5	 = _mm_mul_ps(XMM5, PM128(PFV3));
	XMM2	 = _mm_mul_ps(XMM2, PM128(PFV4));
	XMM6	 = _mm_mul_ps(XMM6, PM128(PFV5));
	XMM3	 = _mm_mul_ps(XMM3, PM128(PFV6));
	XMM7	 = _mm_mul_ps(XMM7, PM128(PFV7));
	XMM0	 = _mm_add_ps(XMM0, XMM4);
	XMM1	 = _mm_add_ps(XMM1, XMM5);
	XMM2	 = _mm_add_ps(XMM2, XMM6);
	XMM3	 = _mm_add_ps(XMM3, XMM7);
	_mm_store_ps(x   , XMM0);
	_mm_store_ps(x+ 4, XMM1);
	_mm_store_ps(x+ 8, XMM2);
	_mm_store_ps(x+12, XMM3);
#else														/* SSE Optimize */
  REG_TYPE r0     = x[30] - x[14];
  REG_TYPE r1     = x[31] - x[15];

           x[30] +=         x[14];           
	   x[31] +=         x[15];
           x[14]  =         r0;              
	   x[15]  =         r1;

           r0     = x[28] - x[12];   
	   r1     = x[29] - x[13];
           x[28] +=         x[12];           
	   x[29] +=         x[13];
           x[12]  = MULT_NORM( r0 * cPI1_8  -  r1 * cPI3_8 );
	   x[13]  = MULT_NORM( r0 * cPI3_8  +  r1 * cPI1_8 );

           r0     = x[26] - x[10];
	   r1     = x[27] - x[11];
	   x[26] +=         x[10];
	   x[27] +=         x[11];
	   x[10]  = MULT_NORM(( r0  - r1 ) * cPI2_8);
	   x[11]  = MULT_NORM(( r0  + r1 ) * cPI2_8);

	   r0     = x[24] - x[8];
	   r1     = x[25] - x[9];
	   x[24] += x[8];
	   x[25] += x[9];
	   x[8]   = MULT_NORM( r0 * cPI3_8  -  r1 * cPI1_8 );
	   x[9]   = MULT_NORM( r1 * cPI3_8  +  r0 * cPI1_8 );

	   r0     = x[22] - x[6];
	   r1     = x[7]  - x[23];
	   x[22] += x[6];
	   x[23] += x[7];
	   x[6]   = r1;
	   x[7]   = r0;

	   r0     = x[4]  - x[20];
	   r1     = x[5]  - x[21];
	   x[20] += x[4];
	   x[21] += x[5];
	   x[4]   = MULT_NORM( r1 * cPI1_8  +  r0 * cPI3_8 );
	   x[5]   = MULT_NORM( r1 * cPI3_8  -  r0 * cPI1_8 );

	   r0     = x[2]  - x[18];
	   r1     = x[3]  - x[19];
	   x[18] += x[2];
	   x[19] += x[3];
	   x[2]   = MULT_NORM(( r1  + r0 ) * cPI2_8);
	   x[3]   = MULT_NORM(( r1  - r0 ) * cPI2_8);

	   r0     = x[0]  - x[16];
	   r1     = x[1]  - x[17];
	   x[16] += x[0];
	   x[17] += x[1];
	   x[0]   = MULT_NORM( r1 * cPI3_8  +  r0 * cPI1_8 );
	   x[1]   = MULT_NORM( r1 * cPI1_8  -  r0 * cPI3_8 );
#endif														/* SSE Optimize */

	   mdct_butterfly_16(x);
	   mdct_butterfly_16(x+16);

}

/* N point first stage butterfly (in place, 2 register) */
#ifdef __SSE__												/* SSE Optimize */
STIN void mdct_butterfly_first_backward(int n,float *T,
					float *x,
					int points, float *zX0, float *zX1)
{
	float	*X1	 = x +  points - 8;
	float	*X2	 = x + (points>>1) - 8;

	/*
		Part of X2[*]=0.f
	*/
	while(X2>=zX0){
		__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
		XMM0	 = _mm_load_ps(X1+4);
		XMM1	 = _mm_load_ps(X1  );
#if	defined(__SSE3__)
		XMM2	 = _mm_moveldup_ps(XMM0);
		XMM3	 = _mm_moveldup_ps(XMM1);
		XMM0	 = _mm_movehdup_ps(XMM0);
		XMM1	 = _mm_movehdup_ps(XMM1);
#else
		XMM2	 = XMM0;
		XMM3	 = XMM1;
		XMM0	 = _mm_shuffle_ps(XMM0 , XMM0 , _MM_SHUFFLE(3,3,1,1));
		XMM1	 = _mm_shuffle_ps(XMM1 , XMM1 , _MM_SHUFFLE(3,3,1,1));
		XMM2	 = _mm_shuffle_ps(XMM2 , XMM2 , _MM_SHUFFLE(2,2,0,0));
		XMM3	 = _mm_shuffle_ps(XMM3 , XMM3 , _MM_SHUFFLE(2,2,0,0));
#endif
		XMM4	 = _mm_load_ps(T   );
		XMM5	 = _mm_load_ps(T+ 4);
		XMM6	 = _mm_load_ps(T+ 8);
		XMM7	 = _mm_load_ps(T+12);
		XMM2	 = _mm_mul_ps(XMM2, XMM4);
		XMM3	 = _mm_mul_ps(XMM3, XMM5);
		XMM0	 = _mm_mul_ps(XMM0, XMM6);
		XMM1	 = _mm_mul_ps(XMM1, XMM7);
		XMM0	 = _mm_add_ps(XMM0, XMM2);
		XMM1	 = _mm_add_ps(XMM1, XMM3);
		_mm_store_ps(X2+4, XMM0);
		_mm_store_ps(X2  , XMM1);
		X1	-= 8;
		X2	-= 8;
		T	+= 16;
	}
	/*
		Part of Normal
	*/
	while(X1>=zX1){
		__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
		XMM0	 = _mm_load_ps(X1+4);
		XMM1	 = _mm_load_ps(X1  );
		XMM2	 = _mm_load_ps(X2+4);
		XMM3	 = _mm_load_ps(X2  );
		XMM4	 = XMM0;
		XMM5	 = XMM1;
		XMM0	 = _mm_sub_ps(XMM0, XMM2);
		XMM1	 = _mm_sub_ps(XMM1, XMM3);
		XMM4	 = _mm_add_ps(XMM4, XMM2);
		XMM5	 = _mm_add_ps(XMM5, XMM3);
#if	defined(__SSE3__)
		XMM2	 = _mm_moveldup_ps(XMM0);
		XMM3	 = _mm_moveldup_ps(XMM1);
		_mm_store_ps(X1+4, XMM4);
		_mm_store_ps(X1  , XMM5);
		XMM0	 = _mm_movehdup_ps(XMM0);
		XMM1	 = _mm_movehdup_ps(XMM1);
#else
		XMM2	 = XMM0;
		XMM3	 = XMM1;
		_mm_store_ps(X1+4, XMM4);
		_mm_store_ps(X1  , XMM5);
		XMM0	 = _mm_shuffle_ps(XMM0 , XMM0 , _MM_SHUFFLE(3,3,1,1));
		XMM1	 = _mm_shuffle_ps(XMM1 , XMM1 , _MM_SHUFFLE(3,3,1,1));
		XMM2	 = _mm_shuffle_ps(XMM2 , XMM2 , _MM_SHUFFLE(2,2,0,0));
		XMM3	 = _mm_shuffle_ps(XMM3 , XMM3 , _MM_SHUFFLE(2,2,0,0));
#endif
		XMM4	 = _mm_load_ps(T   );
		XMM5	 = _mm_load_ps(T+ 4);
		XMM6	 = _mm_load_ps(T+ 8);
		XMM7	 = _mm_load_ps(T+12);
		XMM2	 = _mm_mul_ps(XMM2, XMM4);
		XMM3	 = _mm_mul_ps(XMM3, XMM5);
		XMM0	 = _mm_mul_ps(XMM0, XMM6);
		XMM1	 = _mm_mul_ps(XMM1, XMM7);
		XMM0	 = _mm_add_ps(XMM0, XMM2);
		XMM1	 = _mm_add_ps(XMM1, XMM3);
		_mm_store_ps(X2+4, XMM0);
		_mm_store_ps(X2  , XMM1);
		X1	-= 8;
		X2	-= 8;
		T	+= 16;
	}
	/*
		Part of X1[*]=0.f
	*/
	T	+= n;
	while(X2>=x){
		__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
		XMM0	 = _mm_load_ps(X2+4);
		XMM1	 = _mm_load_ps(X2  );
		_mm_store_ps(X1+4, XMM0);
		_mm_store_ps(X1  , XMM1);
#if	defined(__SSE3__)
		XMM2	 = _mm_moveldup_ps(XMM0);
		XMM3	 = _mm_moveldup_ps(XMM1);
		XMM0	 = _mm_movehdup_ps(XMM0);
		XMM1	 = _mm_movehdup_ps(XMM1);
#else
		XMM2	 = XMM0;
		XMM3	 = XMM1;
		XMM0	 = _mm_shuffle_ps(XMM0 , XMM0 , _MM_SHUFFLE(3,3,1,1));
		XMM1	 = _mm_shuffle_ps(XMM1 , XMM1 , _MM_SHUFFLE(3,3,1,1));
		XMM2	 = _mm_shuffle_ps(XMM2 , XMM2 , _MM_SHUFFLE(2,2,0,0));
		XMM3	 = _mm_shuffle_ps(XMM3 , XMM3 , _MM_SHUFFLE(2,2,0,0));
#endif
		XMM4	 = _mm_load_ps(T   );
		XMM5	 = _mm_load_ps(T+ 4);
		XMM6	 = _mm_load_ps(T+ 8);
		XMM7	 = _mm_load_ps(T+12);
		XMM2	 = _mm_mul_ps(XMM2, XMM4);
		XMM3	 = _mm_mul_ps(XMM3, XMM5);
		XMM0	 = _mm_mul_ps(XMM0, XMM6);
		XMM1	 = _mm_mul_ps(XMM1, XMM7);
		XMM0	 = _mm_add_ps(XMM0, XMM2);
		XMM1	 = _mm_add_ps(XMM1, XMM3);
		_mm_store_ps(X2+4, XMM0);
		_mm_store_ps(X2  , XMM1);
		X1	-= 8;
		X2	-= 8;
		T	+= 16;
	}
}
#endif														/* SSE Optimize */

STIN void mdct_butterfly_first(DATA_TYPE *T,
					DATA_TYPE *x,
					int points){
  
#ifdef __SSE__												/* SSE Optimize */
	float	*X1	 = x +  points - 8;
	float	*X2	 = x + (points>>1) - 8;

	do{
		__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
		XMM0	 = _mm_load_ps(X1+4);
		XMM1	 = _mm_load_ps(X1  );
		XMM2	 = _mm_load_ps(X2+4);
		XMM3	 = _mm_load_ps(X2  );
		XMM4	 = XMM0;
		XMM5	 = XMM1;
		XMM0	 = _mm_sub_ps(XMM0, XMM2);
		XMM1	 = _mm_sub_ps(XMM1, XMM3);
		XMM4	 = _mm_add_ps(XMM4, XMM2);
		XMM5	 = _mm_add_ps(XMM5, XMM3);
#if	defined(__SSE3__)
		XMM2	 = _mm_moveldup_ps(XMM0);
		XMM3	 = _mm_moveldup_ps(XMM1);
		_mm_store_ps(X1+4, XMM4);
		_mm_store_ps(X1  , XMM5);
		XMM0	 = _mm_movehdup_ps(XMM0);
		XMM1	 = _mm_movehdup_ps(XMM1);
#else
		XMM2	 = XMM0;
		XMM3	 = XMM1;
		_mm_store_ps(X1+4, XMM4);
		_mm_store_ps(X1  , XMM5);
		XMM0	 = _mm_shuffle_ps(XMM0 , XMM0 , _MM_SHUFFLE(3,3,1,1));
		XMM1	 = _mm_shuffle_ps(XMM1 , XMM1 , _MM_SHUFFLE(3,3,1,1));
		XMM2	 = _mm_shuffle_ps(XMM2 , XMM2 , _MM_SHUFFLE(2,2,0,0));
		XMM3	 = _mm_shuffle_ps(XMM3 , XMM3 , _MM_SHUFFLE(2,2,0,0));
#endif
		XMM4	 = _mm_load_ps(T   );
		XMM5	 = _mm_load_ps(T+ 4);
		XMM6	 = _mm_load_ps(T+ 8);
		XMM7	 = _mm_load_ps(T+12);
		XMM2	 = _mm_mul_ps(XMM2, XMM4);
		XMM3	 = _mm_mul_ps(XMM3, XMM5);
		XMM0	 = _mm_mul_ps(XMM0, XMM6);
		XMM1	 = _mm_mul_ps(XMM1, XMM7);
		XMM0	 = _mm_add_ps(XMM0, XMM2);
		XMM1	 = _mm_add_ps(XMM1, XMM3);
		_mm_store_ps(X2+4, XMM0);
		_mm_store_ps(X2  , XMM1);
		X1	-= 8;
		X2	-= 8;
		T	+= 16;
	}while(X2>=x);
#else														/* SSE Optimize */
  DATA_TYPE *x1        = x          + points      - 8;
  DATA_TYPE *x2        = x          + (points>>1) - 8;
  REG_TYPE   r0;
  REG_TYPE   r1;

  do{
    
               r0      = x1[6]      -  x2[6];
	       r1      = x1[7]      -  x2[7];
	       x1[6]  += x2[6];
	       x1[7]  += x2[7];
	       x2[6]   = MULT_NORM(r1 * T[1]  +  r0 * T[0]);
	       x2[7]   = MULT_NORM(r1 * T[0]  -  r0 * T[1]);
	       
	       r0      = x1[4]      -  x2[4];
	       r1      = x1[5]      -  x2[5];
	       x1[4]  += x2[4];
	       x1[5]  += x2[5];
	       x2[4]   = MULT_NORM(r1 * T[5]  +  r0 * T[4]);
	       x2[5]   = MULT_NORM(r1 * T[4]  -  r0 * T[5]);
	       
	       r0      = x1[2]      -  x2[2];
	       r1      = x1[3]      -  x2[3];
	       x1[2]  += x2[2];
	       x1[3]  += x2[3];
	       x2[2]   = MULT_NORM(r1 * T[9]  +  r0 * T[8]);
	       x2[3]   = MULT_NORM(r1 * T[8]  -  r0 * T[9]);
	       
	       r0      = x1[0]      -  x2[0];
	       r1      = x1[1]      -  x2[1];
	       x1[0]  += x2[0];
	       x1[1]  += x2[1];
	       x2[0]   = MULT_NORM(r1 * T[13] +  r0 * T[12]);
	       x2[1]   = MULT_NORM(r1 * T[12] -  r0 * T[13]);
	       
    x1-=8;
    x2-=8;
    T+=16;

  }while(x2>=x);
#endif														/* SSE Optimize */
}

/* N/stage point generic N stage butterfly (in place, 2 register) */
#ifdef __SSE__												/* SSE Optimize */
STIN void mdct_butterfly_generic(mdct_lookup *init,
#else														/* SSE Optimize */
STIN void mdct_butterfly_generic(DATA_TYPE *T,
#endif														/* SSE Optimize */
					  DATA_TYPE *x,
					  int points,
					  int trigint){
  
#ifdef __SSE__												/* SSE Optimize */
	float *T;
	float *x1	 = x +  points     - 8;
	float *x2	 = x + (points>>1) - 8;
	switch(trigint)
	{
		default :
			T	 = init->trig;
			do
			{
				__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6;
				XMM0	 = _mm_load_ps(x1  );
				XMM1	 = _mm_load_ps(x2  );
				XMM2	 = _mm_load_ps(x1+4);
				XMM3	 = _mm_load_ps(x2+4);
				XMM4	 = XMM0;
				XMM5	 = XMM2;
				XMM0	 = _mm_sub_ps(XMM0, XMM1);
				XMM2	 = _mm_sub_ps(XMM2, XMM3);
				XMM4	 = _mm_add_ps(XMM4, XMM1);
				XMM5	 = _mm_add_ps(XMM5, XMM3);
				XMM1	 = XMM0;
				XMM3	 = XMM2;
				_mm_store_ps(x1  , XMM4);
				_mm_store_ps(x1+4, XMM5);
#if	defined(__SSE3__)
				XMM0	 = _mm_movehdup_ps(XMM0);
				XMM1	 = _mm_moveldup_ps(XMM1);
				XMM2	 = _mm_movehdup_ps(XMM2);
				XMM3	 = _mm_moveldup_ps(XMM3);
#else
				XMM0	 = _mm_shuffle_ps(XMM0, XMM0, _MM_SHUFFLE(3,3,1,1));
				XMM1	 = _mm_shuffle_ps(XMM1, XMM1, _MM_SHUFFLE(2,2,0,0));
				XMM2	 = _mm_shuffle_ps(XMM2, XMM2, _MM_SHUFFLE(3,3,1,1));
				XMM3	 = _mm_shuffle_ps(XMM3, XMM3, _MM_SHUFFLE(2,2,0,0));
#endif
				XMM4	 = _mm_load_ps(T+trigint*3);
				XMM5	 = _mm_load_ps(T+trigint*3);
				XMM6	 = _mm_load_ps(T+trigint*2);
				XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RNRN));
				XMM4	 = _mm_shuffle_ps(XMM4, XMM6, _MM_SHUFFLE(0,1,0,1));
				XMM5	 = _mm_shuffle_ps(XMM5, XMM6, _MM_SHUFFLE(1,0,1,0));
				XMM0	 = _mm_mul_ps(XMM0, XMM4);
				XMM1	 = _mm_mul_ps(XMM1, XMM5);
				XMM4	 = _mm_load_ps(T+trigint  );
				XMM5	 = _mm_load_ps(T+trigint  );
				XMM6	 = _mm_load_ps(T          );
				XMM3	 = _mm_xor_ps(XMM3, PM128(PCS_RNRN));
				XMM4	 = _mm_shuffle_ps(XMM4, XMM6, _MM_SHUFFLE(0,1,0,1));
				XMM5	 = _mm_shuffle_ps(XMM5, XMM6, _MM_SHUFFLE(1,0,1,0));
				XMM2	 = _mm_mul_ps(XMM2, XMM4);
				XMM3	 = _mm_mul_ps(XMM3, XMM5);
				XMM0	 = _mm_add_ps(XMM0, XMM1);
				XMM2	 = _mm_add_ps(XMM2, XMM3);
				_mm_store_ps(x2  , XMM0);
				_mm_store_ps(x2+4, XMM2);
				T	+= trigint*4;
				x1	-= 8;
				x2	-= 8;
			}
			while(x2>=x);
			return;
		case  8:
			T	 = init->trig_butterfly_generic8;
			break;
		case 16:
			T	 = init->trig_butterfly_generic16;
			break;
		case 32:
			T	 = init->trig_butterfly_generic32;
			break;
		case 64:
			T	 = init->trig_butterfly_generic64;
			break;
	}
	_mm_prefetch(T   , _MM_HINT_NTA);
	do
	{
		__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
#if defined(_OPENMP)
		_mm_prefetch(T+16, _MM_HINT_NTA);
		XMM0	 = _mm_load_ps(x1   );
		XMM1	 = _mm_load_ps(x2   );
		XMM2	 = _mm_load_ps(x1+ 4);
		XMM3	 = _mm_load_ps(x2+ 4);
		XMM4	 = XMM0;
		XMM5	 = XMM2;
		XMM0	 = _mm_sub_ps(XMM0, XMM1);
		XMM2	 = _mm_sub_ps(XMM2, XMM3);
		XMM4	 = _mm_add_ps(XMM4, XMM1);
		XMM5	 = _mm_add_ps(XMM5, XMM3);
#if	defined(__SSE3__)
		XMM1	 = _mm_moveldup_ps(XMM0);
		XMM3	 = _mm_moveldup_ps(XMM2);
		_mm_store_ps(x1   , XMM4);
		_mm_store_ps(x1+ 4, XMM5);
		XMM0	 = _mm_movehdup_ps(XMM0);
		XMM2	 = _mm_movehdup_ps(XMM2);
#else
		XMM1	 = XMM0;
		XMM3	 = XMM2;
		XMM0	 = _mm_shuffle_ps(XMM0, XMM0, _MM_SHUFFLE(3,3,1,1));
		XMM1	 = _mm_shuffle_ps(XMM1, XMM1, _MM_SHUFFLE(2,2,0,0));
		_mm_store_ps(x1   , XMM4);
		_mm_store_ps(x1+ 4, XMM5);
		XMM2	 = _mm_shuffle_ps(XMM2, XMM2, _MM_SHUFFLE(3,3,1,1));
		XMM3	 = _mm_shuffle_ps(XMM3, XMM3, _MM_SHUFFLE(2,2,0,0));
#endif
		_mm_prefetch(T+32, _MM_HINT_NTA);
		XMM4	 = _mm_load_ps(T   );
		XMM5	 = _mm_load_ps(T+ 4);
		XMM6	 = _mm_load_ps(T+ 8);
		XMM7	 = _mm_load_ps(T+12);
		XMM0	 = _mm_mul_ps(XMM0, XMM4);
		XMM4	 = _mm_load_ps(x1- 8);
		XMM1	 = _mm_mul_ps(XMM1, XMM5);
		XMM5	 = _mm_load_ps(x2- 8);
		XMM2	 = _mm_mul_ps(XMM2, XMM6);
		XMM6	 = _mm_load_ps(x1- 4);
		XMM3	 = _mm_mul_ps(XMM3, XMM7);
		XMM7	 = _mm_load_ps(x2- 4);
		XMM0	 = _mm_add_ps(XMM0, XMM1);
		XMM1	 = XMM4;
		XMM2	 = _mm_add_ps(XMM2, XMM3);
		XMM3	 = XMM6;
		XMM4	 = _mm_sub_ps(XMM4, XMM5);
		XMM6	 = _mm_sub_ps(XMM6, XMM7);
		_mm_store_ps(x2   , XMM0);
		_mm_store_ps(x2+ 4, XMM2);
		XMM1	 = _mm_add_ps(XMM1, XMM5);
		XMM3	 = _mm_add_ps(XMM3, XMM7);
#if	defined(__SSE3__)
		XMM5	 = _mm_moveldup_ps(XMM4);
		XMM7	 = _mm_moveldup_ps(XMM6);
		_mm_store_ps(x1- 8, XMM1);
		_mm_store_ps(x1- 4, XMM3);
		XMM4	 = _mm_movehdup_ps(XMM4);
		XMM6	 = _mm_movehdup_ps(XMM6);
#else
		XMM5	 = XMM4;
		XMM7	 = XMM6;
		XMM4	 = _mm_shuffle_ps(XMM4, XMM4, _MM_SHUFFLE(3,3,1,1));
		XMM5	 = _mm_shuffle_ps(XMM5, XMM5, _MM_SHUFFLE(2,2,0,0));
		_mm_store_ps(x1- 8, XMM1);
		_mm_store_ps(x1- 4, XMM3);
		XMM6	 = _mm_shuffle_ps(XMM6, XMM6, _MM_SHUFFLE(3,3,1,1));
		XMM7	 = _mm_shuffle_ps(XMM7, XMM7, _MM_SHUFFLE(2,2,0,0));
#endif
		_mm_prefetch(T+16, _MM_HINT_NTA);
		XMM1	 = _mm_load_ps(T+16);
		XMM3	 = _mm_load_ps(T+20);
		XMM0	 = _mm_load_ps(T+24);
		XMM2	 = _mm_load_ps(T+28);
		XMM4	 = _mm_mul_ps(XMM4, XMM1);
		XMM1	 = _mm_load_ps(x1-16);
		XMM5	 = _mm_mul_ps(XMM5, XMM3);
		XMM3	 = _mm_load_ps(x2-16);
		XMM6	 = _mm_mul_ps(XMM6, XMM0);
		XMM0	 = _mm_load_ps(x1-12);
		XMM7	 = _mm_mul_ps(XMM7, XMM2);
		XMM2	 = _mm_load_ps(x2-12);
		XMM4	 = _mm_add_ps(XMM4, XMM5);
		XMM5	 = XMM1;
		XMM6	 = _mm_add_ps(XMM6, XMM7);
		XMM7	 = XMM0;
		XMM1	 = _mm_sub_ps(XMM1, XMM3);
		XMM0	 = _mm_sub_ps(XMM0, XMM2);
		_mm_store_ps(x2- 8, XMM4);
		_mm_store_ps(x2- 4, XMM6);
		XMM5	 = _mm_add_ps(XMM5, XMM3);
		XMM7	 = _mm_add_ps(XMM7, XMM2);
#if	defined(__SSE3__)
		XMM3	 = _mm_moveldup_ps(XMM1);
		XMM2	 = _mm_moveldup_ps(XMM0);
		_mm_store_ps(x1-16, XMM5);
		_mm_store_ps(x1-12, XMM7);
		XMM1	 = _mm_movehdup_ps(XMM1);
		XMM0	 = _mm_movehdup_ps(XMM0);
#else
		XMM3	 = XMM1;
		XMM2	 = XMM0;
		XMM1	 = _mm_shuffle_ps(XMM1, XMM1, _MM_SHUFFLE(3,3,1,1));
		XMM3	 = _mm_shuffle_ps(XMM3, XMM3, _MM_SHUFFLE(2,2,0,0));
		_mm_store_ps(x1-16, XMM5);
		_mm_store_ps(x1-12, XMM7);
		XMM0	 = _mm_shuffle_ps(XMM0, XMM0, _MM_SHUFFLE(3,3,1,1));
		XMM2	 = _mm_shuffle_ps(XMM2, XMM2, _MM_SHUFFLE(2,2,0,0));
#endif
		_mm_prefetch(T+32, _MM_HINT_NTA);
		XMM5	 = _mm_load_ps(T+32);
		XMM7	 = _mm_load_ps(T+36);
		XMM4	 = _mm_load_ps(T+40);
		XMM6	 = _mm_load_ps(T+44);
		XMM1	 = _mm_mul_ps(XMM1, XMM5);
		XMM5	 = _mm_load_ps(x1-24);
		XMM3	 = _mm_mul_ps(XMM3, XMM7);
		XMM7	 = _mm_load_ps(x2-24);
		XMM0	 = _mm_mul_ps(XMM0, XMM4);
		XMM4	 = _mm_load_ps(x1-20);
		XMM2	 = _mm_mul_ps(XMM2, XMM6);
		XMM6	 = _mm_load_ps(x2-20);
		XMM1	 = _mm_add_ps(XMM1, XMM3);
		XMM3	 = XMM5;
		XMM0	 = _mm_add_ps(XMM0, XMM2);
		XMM2	 = XMM4;
		XMM5	 = _mm_sub_ps(XMM5, XMM7);
		XMM4	 = _mm_sub_ps(XMM4, XMM6);
		_mm_store_ps(x2-16, XMM1);
		_mm_store_ps(x2-12, XMM0);
		XMM3	 = _mm_add_ps(XMM3, XMM7);
		XMM2	 = _mm_add_ps(XMM2, XMM6);
#if	defined(__SSE3__)
		XMM7	 = _mm_moveldup_ps(XMM5);
		XMM6	 = _mm_moveldup_ps(XMM4);
		_mm_store_ps(x1-24, XMM3);
		_mm_store_ps(x1-20, XMM2);
		XMM5	 = _mm_movehdup_ps(XMM5);
		XMM4	 = _mm_movehdup_ps(XMM4);
#else
		XMM7	 = XMM5;
		XMM6	 = XMM4;
		XMM5	 = _mm_shuffle_ps(XMM5, XMM5, _MM_SHUFFLE(3,3,1,1));
		XMM7	 = _mm_shuffle_ps(XMM7, XMM7, _MM_SHUFFLE(2,2,0,0));
		_mm_store_ps(x1-24, XMM3);
		_mm_store_ps(x1-20, XMM2);
		XMM4	 = _mm_shuffle_ps(XMM4, XMM4, _MM_SHUFFLE(3,3,1,1));
		XMM6	 = _mm_shuffle_ps(XMM6, XMM6, _MM_SHUFFLE(2,2,0,0));
#endif
		XMM3	 = _mm_load_ps(T+48);
		XMM2	 = _mm_load_ps(T+52);
		XMM1	 = _mm_load_ps(T+56);
		XMM0	 = _mm_load_ps(T+60);
		XMM5	 = _mm_mul_ps(XMM5, XMM3);
		XMM7	 = _mm_mul_ps(XMM7, XMM2);
		XMM4	 = _mm_mul_ps(XMM4, XMM1);
		XMM6	 = _mm_mul_ps(XMM6, XMM0);
		XMM5	 = _mm_add_ps(XMM5, XMM7);
		XMM4	 = _mm_add_ps(XMM4, XMM6);
		_mm_store_ps(x2-24, XMM5);
		_mm_store_ps(x2-20, XMM4);
		T	+= 64;
		x1	-= 32;
		x2	-= 32;
#else
		_mm_prefetch(T+16, _MM_HINT_NTA);
		XMM0	 = _mm_load_ps(x1  );
		XMM1	 = _mm_load_ps(x2  );
		XMM2	 = _mm_load_ps(x1+4);
		XMM3	 = _mm_load_ps(x2+4);
		XMM4	 = XMM0;
		XMM5	 = XMM2;
		XMM0	 = _mm_sub_ps(XMM0, XMM1);
		XMM2	 = _mm_sub_ps(XMM2, XMM3);
		XMM4	 = _mm_add_ps(XMM4, XMM1);
		XMM5	 = _mm_add_ps(XMM5, XMM3);
#if	defined(__SSE3__)
		XMM1	 = _mm_moveldup_ps(XMM0);
		XMM3	 = _mm_moveldup_ps(XMM2);
		_mm_store_ps(x1  , XMM4);
		_mm_store_ps(x1+4, XMM5);
		XMM0	 = _mm_movehdup_ps(XMM0);
		XMM2	 = _mm_movehdup_ps(XMM2);
#else
		XMM1	 = XMM0;
		XMM3	 = XMM2;
		XMM0	 = _mm_shuffle_ps(XMM0, XMM0, _MM_SHUFFLE(3,3,1,1));
		XMM1	 = _mm_shuffle_ps(XMM1, XMM1, _MM_SHUFFLE(2,2,0,0));
		_mm_store_ps(x1  , XMM4);
		_mm_store_ps(x1+4, XMM5);
		XMM2	 = _mm_shuffle_ps(XMM2, XMM2, _MM_SHUFFLE(3,3,1,1));
		XMM3	 = _mm_shuffle_ps(XMM3, XMM3, _MM_SHUFFLE(2,2,0,0));
#endif
		XMM4	 = _mm_load_ps(T   );
		XMM5	 = _mm_load_ps(T+ 4);
		XMM6	 = _mm_load_ps(T+ 8);
		XMM7	 = _mm_load_ps(T+12);
		XMM0	 = _mm_mul_ps(XMM0, XMM4);
		XMM1	 = _mm_mul_ps(XMM1, XMM5);
		XMM2	 = _mm_mul_ps(XMM2, XMM6);
		XMM3	 = _mm_mul_ps(XMM3, XMM7);
		XMM0	 = _mm_add_ps(XMM0, XMM1);
		XMM2	 = _mm_add_ps(XMM2, XMM3);
		_mm_store_ps(x2  , XMM0);
		_mm_store_ps(x2+4, XMM2);
		T	+= 16;
		x1	-= 8;
		x2	-= 8;
#endif
	}
	while(x2>=x);
#else														/* SSE Optimize */
  DATA_TYPE *x1        = x          + points      - 8;
  DATA_TYPE *x2        = x          + (points>>1) - 8;
  REG_TYPE   r0;
  REG_TYPE   r1;

  do{
    
               r0      = x1[6]      -  x2[6];
	       r1      = x1[7]      -  x2[7];
	       x1[6]  += x2[6];
	       x1[7]  += x2[7];
	       x2[6]   = MULT_NORM(r1 * T[1]  +  r0 * T[0]);
	       x2[7]   = MULT_NORM(r1 * T[0]  -  r0 * T[1]);
	       
	       T+=trigint;
	       
	       r0      = x1[4]      -  x2[4];
	       r1      = x1[5]      -  x2[5];
	       x1[4]  += x2[4];
	       x1[5]  += x2[5];
	       x2[4]   = MULT_NORM(r1 * T[1]  +  r0 * T[0]);
	       x2[5]   = MULT_NORM(r1 * T[0]  -  r0 * T[1]);
	       
	       T+=trigint;
	       
	       r0      = x1[2]      -  x2[2];
	       r1      = x1[3]      -  x2[3];
	       x1[2]  += x2[2];
	       x1[3]  += x2[3];
	       x2[2]   = MULT_NORM(r1 * T[1]  +  r0 * T[0]);
	       x2[3]   = MULT_NORM(r1 * T[0]  -  r0 * T[1]);
	       
	       T+=trigint;
	       
	       r0      = x1[0]      -  x2[0];
	       r1      = x1[1]      -  x2[1];
	       x1[0]  += x2[0];
	       x1[1]  += x2[1];
	       x2[0]   = MULT_NORM(r1 * T[1]  +  r0 * T[0]);
	       x2[1]   = MULT_NORM(r1 * T[0]  -  r0 * T[1]);

	       T+=trigint;
    x1-=8;
    x2-=8;

  }while(x2>=x);
#endif														/* SSE Optimize */
}

#ifdef __SSE__												/* SSE Optimize */
STIN void mdct_butterflies_backward(mdct_lookup *init,
			     float *x,
			     int points, float *x0, float *x1){
  
  int stages=init->log2n-5;
  int i,j;
  
  if(--stages>0){
    mdct_butterfly_first_backward(init->n,init->trig_butterfly_first,x,points,x0,x1);
  }

  for(i=1;--stages>0;i++){
    for(j=0;j<(1<<i);j++)
      mdct_butterfly_generic(init,x+(points>>i)*j,points>>i,4<<i);
  }

  for(j=0;j<points;j+=32)
    mdct_butterfly_32(x+j);

}
#endif														/* SSE Optimize */

STIN void mdct_butterflies(mdct_lookup *init,
			     DATA_TYPE *x,
			     int points){
  
#ifndef __SSE__												/* SSE Optimize */
  DATA_TYPE *T=init->trig;
#endif														/* SSE Optimize */
  int stages=init->log2n-5;
  int i,j;
  
  if(--stages>0){
#ifdef __SSE__												/* SSE Optimize */
    mdct_butterfly_first(init->trig_butterfly_first,x,points);
#else														/* SSE Optimize */
    mdct_butterfly_first(T,x,points);
#endif														/* SSE Optimize */
  }

  for(i=1;--stages>0;i++){
    for(j=0;j<(1<<i);j++)
#ifdef __SSE__												/* SSE Optimize */
      mdct_butterfly_generic(init,x+(points>>i)*j,points>>i,4<<i);
#else														/* SSE Optimize */
      mdct_butterfly_generic(T,x+(points>>i)*j,points>>i,4<<i);
#endif														/* SSE Optimize */
  }

  for(j=0;j<points;j+=32)
    mdct_butterfly_32(x+j);

}

void mdct_clear(mdct_lookup *l){
  if(l){
    if(l->trig)_ogg_free(l->trig);
    if(l->bitrev)_ogg_free(l->bitrev);
#ifdef __SSE__												/* SSE Optimize */
    if(l->trig_bitreverse)_ogg_free(l->trig_bitreverse);
    if(l->trig_forward)_ogg_free(l->trig_forward);
    if(l->trig_backward)_ogg_free(l->trig_backward);
    if(l->trig_butterfly_first)_ogg_free(l->trig_butterfly_first);
    if(l->trig_butterfly_generic8)_ogg_free(l->trig_butterfly_generic8);
    if(l->trig_butterfly_generic16)_ogg_free(l->trig_butterfly_generic16);
    if(l->trig_butterfly_generic32)_ogg_free(l->trig_butterfly_generic32);
    if(l->trig_butterfly_generic64)_ogg_free(l->trig_butterfly_generic64);
#endif														/* SSE Optimize */
    memset(l,0,sizeof(*l));
  }
}

STIN void mdct_bitreverse(mdct_lookup *init, 
			    DATA_TYPE *x){
  int        n       = init->n;
  int       *bit     = init->bitrev;
#ifdef __SSE__												/* SSE Optimize */
	float *w0      = x;
	float *w1      = x = w0+(n>>1);
	float *T       = init->trig_bitreverse;
	
	do
	{
		float *x0	 = x+bit[0];
		float *x1	 = x+bit[1];
		float *x2	 = x+bit[2];
		float *x3	 = x+bit[3];
		
		__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
		w1		 -= 4;
		
		XMM0	 = _mm_lddqu_ps(x0);
		XMM1	 = _mm_lddqu_ps(x1);
		XMM4	 = _mm_lddqu_ps(x2);
		XMM7	 = _mm_lddqu_ps(x3);
		XMM2	 = XMM0;
		XMM3	 = XMM1;
		XMM5	 = XMM0;
		XMM6	 = XMM1;
		
		XMM0	 = _mm_shuffle_ps(XMM0, XMM4, _MM_SHUFFLE(0,1,0,1));
		XMM1	 = _mm_shuffle_ps(XMM1, XMM7, _MM_SHUFFLE(0,1,0,1));
		XMM2	 = _mm_shuffle_ps(XMM2, XMM4, _MM_SHUFFLE(0,0,0,0));
		XMM3	 = _mm_shuffle_ps(XMM3, XMM7, _MM_SHUFFLE(0,0,0,0));
		XMM5	 = _mm_shuffle_ps(XMM5, XMM4, _MM_SHUFFLE(1,1,1,1));
		XMM6	 = _mm_shuffle_ps(XMM6, XMM7, _MM_SHUFFLE(1,1,1,1));
		XMM4	 = _mm_load_ps(T  );
		XMM7	 = _mm_load_ps(T+4);

		XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RNRN));
		XMM2	 = _mm_add_ps(XMM2, XMM3);
		XMM5	 = _mm_sub_ps(XMM5, XMM6);

		XMM0	 = _mm_add_ps(XMM0, XMM1);
		XMM2	 = _mm_mul_ps(XMM2, XMM4);
		XMM5	 = _mm_mul_ps(XMM5, XMM7);

		XMM0	 = _mm_mul_ps(XMM0, PM128(PFV_0P5));
		XMM2	 = _mm_add_ps(XMM2, XMM5);

		XMM1	 = XMM0;
		XMM3	 = XMM2;

#if	defined(__SSE3__)
		XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RNRN));

		XMM0	 = _mm_add_ps(XMM0, XMM2);
		XMM1	 = _mm_addsub_ps(XMM1, XMM3);
#else
		XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RNRN));
		XMM3	 = _mm_xor_ps(XMM3, PM128(PCS_RNRN));

		XMM0	 = _mm_add_ps(XMM0, XMM2);
		XMM1	 = _mm_sub_ps(XMM1, XMM3);
#endif
		_mm_store_ps(w0, XMM0);
		_mm_storeh_pi((__m64*)(w1  ), XMM1);
		_mm_storel_pi((__m64*)(w1+2), XMM1);
		
		T		+= 8;
		bit		+= 4;
		w0		+= 4;
		
	}
	while(w0<w1);
#else														/* SSE Optimize */
  DATA_TYPE *w0      = x;
  DATA_TYPE *w1      = x = w0+(n>>1);
  DATA_TYPE *T       = init->trig+n;

  do{
    DATA_TYPE *x0    = x+bit[0];
    DATA_TYPE *x1    = x+bit[1];

    REG_TYPE  r0     = x0[1]  - x1[1];
    REG_TYPE  r1     = x0[0]  + x1[0];
    REG_TYPE  r2     = MULT_NORM(r1     * T[0]   + r0 * T[1]);
    REG_TYPE  r3     = MULT_NORM(r1     * T[1]   - r0 * T[0]);

	      w1    -= 4;

              r0     = HALVE(x0[1] + x1[1]);
              r1     = HALVE(x0[0] - x1[0]);
      
	      w0[0]  = r0     + r2;
	      w1[2]  = r0     - r2;
	      w0[1]  = r1     + r3;
	      w1[3]  = r3     - r1;

              x0     = x+bit[2];
              x1     = x+bit[3];

              r0     = x0[1]  - x1[1];
              r1     = x0[0]  + x1[0];
              r2     = MULT_NORM(r1     * T[2]   + r0 * T[3]);
              r3     = MULT_NORM(r1     * T[3]   - r0 * T[2]);

              r0     = HALVE(x0[1] + x1[1]);
              r1     = HALVE(x0[0] - x1[0]);
      
	      w0[2]  = r0     + r2;
	      w1[0]  = r0     - r2;
	      w0[3]  = r1     + r3;
	      w1[1]  = r3     - r1;

	      T     += 4;
	      bit   += 4;
	      w0    += 4;

  }while(w0<w1);
#endif														/* SSE Optimize */
}

void mdct_backward(mdct_lookup *init, DATA_TYPE *in, DATA_TYPE *out){
  int n=init->n;
  int n2=n>>1;
  int n4=n>>2;

#ifdef __SSE__												/* SSE Optimize */
	/* rotate */
	
	float *iX	 = in+n2-8;
	float *oX0	 = out+n2+n4;
	float *T	 = init->trig_backward;
	float *oX1   = oX0;
	float *zX0, *zX1;
	
	if(n<1024)
	{
		do
		{
			int c0, c1;
			__m128	XMM0, XMM1, XMM2, XMM3;
			XMM0	 = _mm_load_ps(iX- 8);
			XMM1	 = _mm_load_ps(iX- 4);
			XMM2	 = _mm_load_ps(iX   );
			XMM3	 = _mm_load_ps(iX+ 4);
			XMM0	 = _mm_cmpneq_ps(XMM0, PM128(PFV_0));
			XMM1	 = _mm_cmpneq_ps(XMM1, PM128(PFV_0));
			XMM2	 = _mm_cmpneq_ps(XMM2, PM128(PFV_0));
			XMM3	 = _mm_cmpneq_ps(XMM3, PM128(PFV_0));
			XMM0	 = _mm_or_ps(XMM0, XMM1);
			XMM2	 = _mm_or_ps(XMM2, XMM3);
			c0		 = _mm_movemask_ps(XMM0);
			c1		 = _mm_movemask_ps(XMM2);
			c0		|= c1;
			if(!c0)
			{
				oX0		-= 8;
				_mm_store_ps(oX0   , PM128(PFV_0));
				_mm_store_ps(oX0+ 4, PM128(PFV_0));
				_mm_store_ps(oX1   , PM128(PFV_0));
				_mm_store_ps(oX1+ 4, PM128(PFV_0));
				iX		-= 16;
				oX1		+= 8;
				T		+= 32;
			}
			else
				break;
		}while(iX>=in);
	}
	else
	{
		do
		{
			int c0, c1;
			__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5;
			XMM0	 = _mm_load_ps(iX-24);
			XMM1	 = _mm_load_ps(iX-20);
			XMM2	 = _mm_load_ps(iX-16);
			XMM3	 = _mm_load_ps(iX-12);
			XMM0	 = _mm_cmpneq_ps(XMM0, PM128(PFV_0));
			XMM1	 = _mm_cmpneq_ps(XMM1, PM128(PFV_0));
			XMM2	 = _mm_cmpneq_ps(XMM2, PM128(PFV_0));
			XMM3	 = _mm_cmpneq_ps(XMM3, PM128(PFV_0));
			XMM0	 = _mm_or_ps(XMM0, XMM1);
			XMM2	 = _mm_or_ps(XMM2, XMM3);
			XMM4	 = _mm_load_ps(iX- 8);
			XMM5	 = _mm_load_ps(iX- 4);
			XMM1	 = _mm_load_ps(iX   );
			XMM3	 = _mm_load_ps(iX+ 4);
			XMM4	 = _mm_cmpneq_ps(XMM4, PM128(PFV_0));
			XMM5	 = _mm_cmpneq_ps(XMM5, PM128(PFV_0));
			XMM1	 = _mm_cmpneq_ps(XMM1, PM128(PFV_0));
			XMM3	 = _mm_cmpneq_ps(XMM3, PM128(PFV_0));
			XMM4	 = _mm_or_ps(XMM4, XMM5);
			XMM1	 = _mm_or_ps(XMM1, XMM3);
			XMM0	 = _mm_or_ps(XMM0, XMM4);
			XMM2	 = _mm_or_ps(XMM2, XMM1);
			c0		 = _mm_movemask_ps(XMM0);
			c1		 = _mm_movemask_ps(XMM2);
			c0		|= c1;
			if(!c0)
			{
				oX0		-= 16;
				_mm_store_ps(oX0   , PM128(PFV_0));
				_mm_store_ps(oX0+ 4, PM128(PFV_0));
				_mm_store_ps(oX0+ 8, PM128(PFV_0));
				_mm_store_ps(oX0+12, PM128(PFV_0));
				_mm_store_ps(oX1   , PM128(PFV_0));
				_mm_store_ps(oX1+ 4, PM128(PFV_0));
				_mm_store_ps(oX1+ 8, PM128(PFV_0));
				_mm_store_ps(oX1+12, PM128(PFV_0));
				iX		-= 32;
				oX1		+= 16;
				T		+= 64;
			}
			else
				break;
		}while(iX>=in);
	}
	zX0	 = oX0;
	zX1	 = oX1;
	while(iX>=in)
	{
		__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
#if defined(_OPENMP)
		oX0		-= 8;
		XMM0	 = _mm_load_ps(iX   );
		XMM4	 = _mm_load_ps(iX+ 4);
		XMM2	 = _mm_load_ps(T   );
		XMM3	 = _mm_load_ps(T+ 4);
		XMM1	 = XMM0;
		XMM5	 = XMM0;
		XMM6	 = XMM4;
		XMM7	 = XMM4;
		XMM0	 = _mm_shuffle_ps(XMM0, XMM4, _MM_SHUFFLE(1,3,1,3));
		XMM1	 = _mm_shuffle_ps(XMM1, XMM4, _MM_SHUFFLE(3,1,3,1));
		XMM6	 = _mm_shuffle_ps(XMM6, XMM5, _MM_SHUFFLE(0,0,0,0));
		XMM7	 = _mm_shuffle_ps(XMM7, XMM5, _MM_SHUFFLE(2,2,2,2));
		XMM4	 = _mm_load_ps(T+ 8);
		XMM5	 = _mm_load_ps(T+12);
		XMM0	 = _mm_mul_ps(XMM0, XMM2);
		XMM2	 = _mm_load_ps(iX- 8);
		XMM1	 = _mm_mul_ps(XMM1, XMM3);
		XMM3	 = _mm_load_ps(iX- 4);
		XMM6	 = _mm_mul_ps(XMM6, XMM4);
		XMM4	 = _mm_load_ps(T+16);
		XMM7	 = _mm_mul_ps(XMM7, XMM5);
		XMM5	 = _mm_load_ps(T+20);
		XMM0	 = _mm_sub_ps(XMM0, XMM1);
		XMM1	 = XMM2;
		XMM6	 = _mm_sub_ps(XMM6, XMM7);
		XMM7	 = XMM2;
		_mm_store_ps(oX0+ 4, XMM0);
		XMM0	 = XMM3;
		_mm_store_ps(oX1   , XMM6);
		XMM6	 = XMM3;
		XMM2	 = _mm_shuffle_ps(XMM2, XMM3, _MM_SHUFFLE(1,3,1,3));
		XMM1	 = _mm_shuffle_ps(XMM1, XMM3, _MM_SHUFFLE(3,1,3,1));
		XMM0	 = _mm_shuffle_ps(XMM0, XMM7, _MM_SHUFFLE(0,0,0,0));
		XMM6	 = _mm_shuffle_ps(XMM6, XMM7, _MM_SHUFFLE(2,2,2,2));
		XMM3	 = _mm_load_ps(T+24);
		XMM7	 = _mm_load_ps(T+28);
		XMM2	 = _mm_mul_ps(XMM2, XMM4);
		XMM1	 = _mm_mul_ps(XMM1, XMM5);
		XMM0	 = _mm_mul_ps(XMM0, XMM3);
		XMM6	 = _mm_mul_ps(XMM6, XMM7);
		XMM2	 = _mm_sub_ps(XMM2, XMM1);
		XMM0	 = _mm_sub_ps(XMM0, XMM6);
		_mm_store_ps(oX0   , XMM2);
		_mm_store_ps(oX1+ 4, XMM0);
		iX		-= 16;
		oX1		+= 8;
		T		+= 32;
#else
		oX0		-= 4;
		XMM0	 = _mm_load_ps(iX  );
		XMM4	 = _mm_load_ps(iX+4);
		XMM2	 = _mm_load_ps(T   );
		XMM3	 = _mm_load_ps(T+ 4);
		XMM1	 = XMM0;
		XMM5	 = XMM0;
		XMM6	 = XMM4;
		XMM7	 = XMM4;
		XMM0	 = _mm_shuffle_ps(XMM0, XMM4, _MM_SHUFFLE(1,3,1,3));
		XMM1	 = _mm_shuffle_ps(XMM1, XMM4, _MM_SHUFFLE(3,1,3,1));
		XMM6	 = _mm_shuffle_ps(XMM6, XMM5, _MM_SHUFFLE(0,0,0,0));
		XMM7	 = _mm_shuffle_ps(XMM7, XMM5, _MM_SHUFFLE(2,2,2,2));
		XMM4	 = _mm_load_ps(T+ 8);
		XMM5	 = _mm_load_ps(T+12);
		XMM0	 = _mm_mul_ps(XMM0, XMM2);
		XMM1	 = _mm_mul_ps(XMM1, XMM3);
		XMM6	 = _mm_mul_ps(XMM6, XMM4);
		XMM7	 = _mm_mul_ps(XMM7, XMM5);
		XMM0	 = _mm_sub_ps(XMM0, XMM1);
		XMM6	 = _mm_sub_ps(XMM6, XMM7);
		_mm_store_ps(oX0, XMM0);
		_mm_store_ps(oX1, XMM6);
		iX		-= 8;
		oX1		+= 4;
		T		+= 16;
#endif
	}
	
	mdct_butterflies_backward(init,out+n2,n2,zX0,zX1);
	mdct_bitreverse(init,out);
	
	/* roatate + window */
	
	{
		float *oX1	 = out+n2+n4;
		float *oX2	 = out+n2+n4;
		float *iX	 = out;
		float *T	 = init->trig_backward+n;
		
		do
		{
			__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5;
#if defined(_OPENMP)
			__m128	XMM6;
			oX1		-=8;
			XMM0	 = _mm_load_ps(iX   );
			XMM4	 = _mm_load_ps(iX+ 4);
			XMM2	 = _mm_load_ps(T   );
			XMM3	 = _mm_load_ps(T+ 4);
			XMM1	 = XMM0;
			XMM6	 = _mm_load_ps(iX+ 8);
			XMM0	 = _mm_shuffle_ps(XMM0, XMM4, _MM_SHUFFLE(2,0,2,0));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM4, _MM_SHUFFLE(3,1,3,1));
			XMM4	 = XMM0;
			XMM5	 = XMM1;
			XMM0	 = _mm_mul_ps(XMM0, XMM2);
			XMM1	 = _mm_mul_ps(XMM1, XMM3);
			XMM4	 = _mm_mul_ps(XMM4, XMM3);
			XMM3	 = _mm_load_ps(iX+12);
			XMM5	 = _mm_mul_ps(XMM5, XMM2);
			XMM2	 = _mm_load_ps(T+ 8);
			XMM0	 = _mm_sub_ps(XMM0, XMM1);
			XMM1	 = _mm_load_ps(T+12);
			XMM4	 = _mm_add_ps(XMM4, XMM5);
			XMM5	 = XMM6;
			XMM6	 = _mm_shuffle_ps(XMM6, XMM3, _MM_SHUFFLE(2,0,2,0));
			XMM0	 = _mm_shuffle_ps(XMM0, XMM0, _MM_SHUFFLE(0,1,2,3));
			XMM4	 = _mm_xor_ps(XMM4, PM128(PCS_RRRR));
			XMM5	 = _mm_shuffle_ps(XMM5, XMM3, _MM_SHUFFLE(3,1,3,1));
			XMM3	 = XMM6;
			_mm_store_ps(oX1+ 4, XMM0);
			XMM0	 = XMM5;
			_mm_store_ps(oX2   , XMM4);
			XMM6	 = _mm_mul_ps(XMM6, XMM2);
			XMM5	 = _mm_mul_ps(XMM5, XMM1);
			XMM3	 = _mm_mul_ps(XMM3, XMM1);
			XMM0	 = _mm_mul_ps(XMM0, XMM2);
			XMM6	 = _mm_sub_ps(XMM6, XMM5);
			XMM3	 = _mm_add_ps(XMM3, XMM0);
			XMM6	 = _mm_shuffle_ps(XMM6, XMM6, _MM_SHUFFLE(0,1,2,3));
			XMM3	 = _mm_xor_ps(XMM3, PM128(PCS_RRRR));
			_mm_store_ps(oX1   , XMM6);
			_mm_store_ps(oX2+ 4, XMM3);
			oX2		+= 8;
			iX		+= 16;
			T		+= 16;
#else
			oX1		-=4;
			XMM0	 = _mm_load_ps(iX  );
			XMM4	 = _mm_load_ps(iX+4);
			XMM2	 = _mm_load_ps(T  );
			XMM3	 = _mm_load_ps(T+4);
			XMM1	 = XMM0;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM4, _MM_SHUFFLE(2,0,2,0));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM4, _MM_SHUFFLE(3,1,3,1));
			XMM4	 = XMM0;
			XMM5	 = XMM1;
			XMM0	 = _mm_mul_ps(XMM0, XMM2);
			XMM1	 = _mm_mul_ps(XMM1, XMM3);
			XMM4	 = _mm_mul_ps(XMM4, XMM3);
			XMM5	 = _mm_mul_ps(XMM5, XMM2);
			XMM0	 = _mm_sub_ps(XMM0, XMM1);
			XMM4	 = _mm_add_ps(XMM4, XMM5);
			XMM0	 = _mm_shuffle_ps(XMM0, XMM0, _MM_SHUFFLE(0,1,2,3));
			XMM4	 = _mm_xor_ps(XMM4, PM128(PCS_RRRR));
			_mm_store_ps(oX1, XMM0);
			_mm_store_ps(oX2, XMM4);
			oX2		+= 4;
			iX		+= 8;
			T		+= 8;
#endif
		}while(iX<oX1);
		
		iX	 = out+n2+n4;
		oX1	 = out+n4;
		oX2	 = oX1;
		
		do
		{
			__m128	XMM0, XMM1, XMM2, XMM3;
			oX1		-= 16;
			iX		-= 16;
			XMM0	 = _mm_load_ps(iX+12);
			XMM1	 = _mm_load_ps(iX+ 8);
			XMM2	 = _mm_load_ps(iX+ 4);
			XMM3	 = _mm_load_ps(iX   );
			_mm_store_ps(oX1+12, XMM0);
			_mm_store_ps(oX1+ 8, XMM1);
			_mm_store_ps(oX1+ 4, XMM2);
			_mm_store_ps(oX1   , XMM3);
			XMM0	 = _mm_shuffle_ps(XMM0, XMM0, _MM_SHUFFLE(0,1,2,3));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM1, _MM_SHUFFLE(0,1,2,3));
			XMM2	 = _mm_shuffle_ps(XMM2, XMM2, _MM_SHUFFLE(0,1,2,3));
			XMM3	 = _mm_shuffle_ps(XMM3, XMM3, _MM_SHUFFLE(0,1,2,3));
			XMM0	 = _mm_xor_ps(XMM0, PM128(PCS_RRRR));
			XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RRRR));
			XMM2	 = _mm_xor_ps(XMM2, PM128(PCS_RRRR));
			XMM3	 = _mm_xor_ps(XMM3, PM128(PCS_RRRR));
			_mm_store_ps(oX2   , XMM0);
			_mm_store_ps(oX2+ 4, XMM1);
			_mm_store_ps(oX2+ 8, XMM2);
			_mm_store_ps(oX2+12, XMM3);
			oX2		+= 16;
		}while(oX2<iX);
		
		iX	 = out+n2+n4;
		oX1	 = out+n2+n4;
		oX2	 = out+n2;
		
		if(n4>16)
		{
			do
			{
				__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
				oX1		-= 32;
				XMM0	 = _mm_load_ps(iX+28);
				XMM1	 = _mm_load_ps(iX+24);
				XMM2	 = _mm_load_ps(iX+20);
				XMM3	 = _mm_load_ps(iX+16);
				XMM4	 = _mm_load_ps(iX+12);
				XMM5	 = _mm_load_ps(iX+ 8);
				XMM6	 = _mm_load_ps(iX+ 4);
				XMM7	 = _mm_load_ps(iX   );
				XMM0	 = _mm_shuffle_ps(XMM0, XMM0, _MM_SHUFFLE(0,1,2,3));
				XMM1	 = _mm_shuffle_ps(XMM1, XMM1, _MM_SHUFFLE(0,1,2,3));
				XMM2	 = _mm_shuffle_ps(XMM2, XMM2, _MM_SHUFFLE(0,1,2,3));
				XMM3	 = _mm_shuffle_ps(XMM3, XMM3, _MM_SHUFFLE(0,1,2,3));
				XMM4	 = _mm_shuffle_ps(XMM4, XMM4, _MM_SHUFFLE(0,1,2,3));
				XMM5	 = _mm_shuffle_ps(XMM5, XMM5, _MM_SHUFFLE(0,1,2,3));
				XMM6	 = _mm_shuffle_ps(XMM6, XMM6, _MM_SHUFFLE(0,1,2,3));
				XMM7	 = _mm_shuffle_ps(XMM7, XMM7, _MM_SHUFFLE(0,1,2,3));
				_mm_store_ps(oX1   , XMM0);
				_mm_store_ps(oX1+ 4, XMM1);
				_mm_store_ps(oX1+ 8, XMM2);
				_mm_store_ps(oX1+12, XMM3);
				_mm_store_ps(oX1+16, XMM4);
				_mm_store_ps(oX1+20, XMM5);
				_mm_store_ps(oX1+24, XMM6);
				_mm_store_ps(oX1+28, XMM7);
				iX		+= 32;
			}while(oX1>oX2);
		}
		else
		{
			do
			{
				__m128	XMM0, XMM1, XMM2, XMM3;
				oX1		-= 16;
				XMM0	 = _mm_load_ps(iX+12);
				XMM1	 = _mm_load_ps(iX+ 8);
				XMM2	 = _mm_load_ps(iX+ 4);
				XMM3	 = _mm_load_ps(iX   );
				XMM0	 = _mm_shuffle_ps(XMM0, XMM0, _MM_SHUFFLE(0,1,2,3));
				XMM1	 = _mm_shuffle_ps(XMM1, XMM1, _MM_SHUFFLE(0,1,2,3));
				XMM2	 = _mm_shuffle_ps(XMM2, XMM2, _MM_SHUFFLE(0,1,2,3));
				XMM3	 = _mm_shuffle_ps(XMM3, XMM3, _MM_SHUFFLE(0,1,2,3));
				_mm_store_ps(oX1   , XMM0);
				_mm_store_ps(oX1+ 4, XMM1);
				_mm_store_ps(oX1+ 8, XMM2);
				_mm_store_ps(oX1+12, XMM3);
				iX		+= 16;
			}while(oX1>oX2);
		}
	}
#else														/* SSE Optimize */
  /* rotate */

  DATA_TYPE *iX = in+n2-7;
  DATA_TYPE *oX = out+n2+n4;
  DATA_TYPE *T  = init->trig+n4;

  do{
    oX         -= 4;
    oX[0]       = MULT_NORM(-iX[2] * T[3] - iX[0]  * T[2]);
    oX[1]       = MULT_NORM (iX[0] * T[3] - iX[2]  * T[2]);
    oX[2]       = MULT_NORM(-iX[6] * T[1] - iX[4]  * T[0]);
    oX[3]       = MULT_NORM (iX[4] * T[1] - iX[6]  * T[0]);
    iX         -= 8;
    T          += 4;
  }while(iX>=in);

  iX            = in+n2-8;
  oX            = out+n2+n4;
  T             = init->trig+n4;

  do{
    T          -= 4;
    oX[0]       =  MULT_NORM (iX[4] * T[3] + iX[6] * T[2]);
    oX[1]       =  MULT_NORM (iX[4] * T[2] - iX[6] * T[3]);
    oX[2]       =  MULT_NORM (iX[0] * T[1] + iX[2] * T[0]);
    oX[3]       =  MULT_NORM (iX[0] * T[0] - iX[2] * T[1]);
    iX         -= 8;
    oX         += 4;
  }while(iX>=in);

  mdct_butterflies(init,out+n2,n2);
  mdct_bitreverse(init,out);

  /* roatate + window */

  {
    DATA_TYPE *oX1=out+n2+n4;
    DATA_TYPE *oX2=out+n2+n4;
    DATA_TYPE *iX =out;
    T             =init->trig+n2;
    
    do{
      oX1-=4;

      oX1[3]  =  MULT_NORM (iX[0] * T[1] - iX[1] * T[0]);
      oX2[0]  = -MULT_NORM (iX[0] * T[0] + iX[1] * T[1]);

      oX1[2]  =  MULT_NORM (iX[2] * T[3] - iX[3] * T[2]);
      oX2[1]  = -MULT_NORM (iX[2] * T[2] + iX[3] * T[3]);

      oX1[1]  =  MULT_NORM (iX[4] * T[5] - iX[5] * T[4]);
      oX2[2]  = -MULT_NORM (iX[4] * T[4] + iX[5] * T[5]);

      oX1[0]  =  MULT_NORM (iX[6] * T[7] - iX[7] * T[6]);
      oX2[3]  = -MULT_NORM (iX[6] * T[6] + iX[7] * T[7]);

      oX2+=4;
      iX    +=   8;
      T     +=   8;
    }while(iX<oX1);

    iX=out+n2+n4;
    oX1=out+n4;
    oX2=oX1;

    do{
      oX1-=4;
      iX-=4;

      oX2[0] = -(oX1[3] = iX[3]);
      oX2[1] = -(oX1[2] = iX[2]);
      oX2[2] = -(oX1[1] = iX[1]);
      oX2[3] = -(oX1[0] = iX[0]);

      oX2+=4;
    }while(oX2<iX);

    iX=out+n2+n4;
    oX1=out+n2+n4;
    oX2=out+n2;
    do{
      oX1-=4;
      oX1[0]= iX[3];
      oX1[1]= iX[2];
      oX1[2]= iX[1];
      oX1[3]= iX[0];
      iX+=4;
    }while(oX1>oX2);
  }
#endif														/* SSE Optimize */
}

#ifdef __SSE__												/* SSE Optimize */
void mdct_forward(mdct_lookup *init, DATA_TYPE *in, DATA_TYPE *out, DATA_TYPE *out1){
	int n	 = init->n;
	int n2	 = n>>1;
	int n4	 = n>>2;
	int n8	 = n>>3;
	float *w	 = (float*)_ogg_alloca(n*sizeof(*w)); /* forward needs working space */
	float *w2	 = w+n2;
	
	/* rotate */
	
	/* window + rotate + step 1 */
	
	int i, j;
	
	float *x0	 = in+n2+n4-8;
	float *x1	 = in+n2+n4;
	float *T	 = init->trig_forward;
	
#pragma warning(disable : 592)
	for(i=0,j=n2-2;i<n8;i+=4,j-=4)
	{
		__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
		XMM0	 = _mm_load_ps(x0    + 4);
		XMM4	 = _mm_load_ps(x0       );
		XMM1	 = _mm_load_ps(x0+i*4+ 8);
		XMM5	 = _mm_load_ps(x0+i*4+12);
		XMM2	 = _mm_load_ps(T   );
		XMM3	 = _mm_load_ps(T+ 4);
		XMM6	 = _mm_load_ps(T+ 8);
		XMM7	 = _mm_load_ps(T+12);
		XMM0	 = _mm_shuffle_ps(XMM0, XMM0, _MM_SHUFFLE(0,1,2,3));
		XMM4	 = _mm_shuffle_ps(XMM4, XMM4, _MM_SHUFFLE(0,1,2,3));
		XMM0	 = _mm_add_ps(XMM0, XMM1);
		XMM4	 = _mm_add_ps(XMM4, XMM5);
		XMM1	 = XMM0;
		XMM5	 = XMM4;
		XMM0	 = _mm_shuffle_ps(XMM0, XMM0, _MM_SHUFFLE(0,0,3,3));
		XMM1	 = _mm_shuffle_ps(XMM1, XMM1, _MM_SHUFFLE(2,2,1,1));
		XMM4	 = _mm_shuffle_ps(XMM4, XMM4, _MM_SHUFFLE(0,0,3,3));
		XMM5	 = _mm_shuffle_ps(XMM5, XMM5, _MM_SHUFFLE(2,2,1,1));
		XMM0	 = _mm_mul_ps(XMM0, XMM2);
		XMM1	 = _mm_mul_ps(XMM1, XMM3);
		XMM4	 = _mm_mul_ps(XMM4, XMM6);
		XMM5	 = _mm_mul_ps(XMM5, XMM7);
		XMM0	 = _mm_sub_ps(XMM0, XMM1);
		XMM4	 = _mm_sub_ps(XMM4, XMM5);
		_mm_storel_pi((__m64*)(w2+i  ), XMM0);
		_mm_storeh_pi((__m64*)(w2+j  ), XMM0);
		_mm_storel_pi((__m64*)(w2+i+2), XMM4);
		_mm_storeh_pi((__m64*)(w2+j-2), XMM4);
		x0	-= 8;
		T	+= 16;
	}

	x0	 = in;
	x1	 = in+n2-8;
	
	for(;i<n4;i+=4,j-=4)
	{
		__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
		XMM1	 = _mm_load_ps(x1+4);
		XMM5	 = _mm_load_ps(x1  );
		XMM0	 = _mm_load_ps(x0  );
		XMM4	 = _mm_load_ps(x0+4);
		XMM2	 = _mm_load_ps(T   );
		XMM3	 = _mm_load_ps(T+ 4);
		XMM6	 = _mm_load_ps(T+ 8);
		XMM7	 = _mm_load_ps(T+12);
		XMM1	 = _mm_shuffle_ps(XMM1, XMM1, _MM_SHUFFLE(0,1,2,3));
		XMM5	 = _mm_shuffle_ps(XMM5, XMM5, _MM_SHUFFLE(0,1,2,3));
		XMM0	 = _mm_sub_ps(XMM0, XMM1);
		XMM4	 = _mm_sub_ps(XMM4, XMM5);
		XMM1	 = XMM0;
		XMM5	 = XMM4;
		XMM0	 = _mm_shuffle_ps(XMM0, XMM0, _MM_SHUFFLE(0,0,3,3));
		XMM1	 = _mm_shuffle_ps(XMM1, XMM1, _MM_SHUFFLE(2,2,1,1));
		XMM4	 = _mm_shuffle_ps(XMM4, XMM4, _MM_SHUFFLE(0,0,3,3));
		XMM5	 = _mm_shuffle_ps(XMM5, XMM5, _MM_SHUFFLE(2,2,1,1));
		XMM0	 = _mm_mul_ps(XMM0, XMM2);
		XMM1	 = _mm_mul_ps(XMM1, XMM3);
		XMM4	 = _mm_mul_ps(XMM4, XMM6);
		XMM5	 = _mm_mul_ps(XMM5, XMM7);
		XMM0	 = _mm_add_ps(XMM0, XMM1);
		XMM4	 = _mm_add_ps(XMM4, XMM5);
		_mm_storel_pi((__m64*)(w2+i  ), XMM0);
		_mm_storeh_pi((__m64*)(w2+j  ), XMM0);
		_mm_storel_pi((__m64*)(w2+i+2), XMM4);
		_mm_storeh_pi((__m64*)(w2+j-2), XMM4);
		x0	+= 8;
		x1	-= 8;
		T	+= 16;
	}
#pragma warning(default : 592)

	mdct_butterflies(init, w+n2, n2);
	mdct_bitreverse(init, w);
	
	/* roatate + window */
	
	T	 = init->trig_forward+n;
	x0	  =out +n2;

	if(out1!=NULL)
	{
		x1	  =out1+n2;
#if defined(_OPENMP)
		for(i=0;i<n4;i+=8){
			__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
			x0	-= 8;
			x1	-= 8;
			XMM0	 = _mm_load_ps(w+ 4);
			XMM4	 = _mm_load_ps(w   );
			XMM2	 = XMM0;
			XMM1	 = _mm_load_ps(T   );
			XMM3	 = _mm_load_ps(T+ 4);
			XMM6	 = _mm_load_ps(T+ 8);
			XMM7	 = _mm_load_ps(T+12);
			XMM0	 = _mm_shuffle_ps(XMM0, XMM4,_MM_SHUFFLE(0,2,0,2));
			XMM2	 = _mm_shuffle_ps(XMM2, XMM4,_MM_SHUFFLE(1,3,1,3));
			XMM4	 = XMM0;
			XMM5	 = XMM2;
			XMM0	 = _mm_mul_ps(XMM0, XMM1);
			XMM1	 = _mm_load_ps(w+12);
			XMM2	 = _mm_mul_ps(XMM2, XMM3);
			XMM3	 = _mm_load_ps(w+ 8);
			XMM4	 = _mm_shuffle_ps(XMM4, XMM4, _MM_SHUFFLE(0,1,2,3));
			XMM5	 = _mm_shuffle_ps(XMM5, XMM5, _MM_SHUFFLE(0,1,2,3));
			XMM4	 = _mm_mul_ps(XMM4, XMM6);
			XMM6	 = XMM1;
			XMM5	 = _mm_mul_ps(XMM5, XMM7);
			XMM7	 = _mm_load_ps(T+16);
			XMM0	 = _mm_sub_ps(XMM0, XMM2);
			XMM2	 = _mm_load_ps(T+20);
			XMM4	 = _mm_add_ps(XMM4, XMM5);
			XMM5	 = _mm_load_ps(T+24);
			_mm_store_ps(x0    +4, XMM0);
			_mm_store_ps(x1    +4, XMM0);
			XMM0	 = _mm_load_ps(T+28);
			XMM1	 = _mm_shuffle_ps(XMM1, XMM3,_MM_SHUFFLE(0,2,0,2));
			XMM6	 = _mm_shuffle_ps(XMM6, XMM3,_MM_SHUFFLE(1,3,1,3));
			_mm_store_ps(out +i  , XMM4);
			_mm_store_ps(out1+i  , XMM4);
			XMM3	 = XMM1;
			XMM4	 = XMM6;
			XMM1	 = _mm_mul_ps(XMM1, XMM7);
			XMM6	 = _mm_mul_ps(XMM6, XMM2);
			XMM3	 = _mm_shuffle_ps(XMM3, XMM3, _MM_SHUFFLE(0,1,2,3));
			XMM4	 = _mm_shuffle_ps(XMM4, XMM4, _MM_SHUFFLE(0,1,2,3));
			XMM3	 = _mm_mul_ps(XMM3, XMM5);
			XMM4	 = _mm_mul_ps(XMM4, XMM0);
			XMM1	 = _mm_sub_ps(XMM1, XMM6);
			XMM3	 = _mm_add_ps(XMM3, XMM4);
			_mm_store_ps(x0      , XMM1);
			_mm_store_ps(x1      , XMM1);
			_mm_store_ps(out +i+4, XMM3);
			_mm_store_ps(out1+i+4, XMM3);
			w	+= 16;
			T	+= 32;
		}
	}
	else
	{
		for(i=0;i<n4;i+=8){
			__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
			x0	-= 8;
			XMM0	 = _mm_load_ps(w+ 4);
			XMM4	 = _mm_load_ps(w   );
			XMM2	 = XMM0;
			XMM1	 = _mm_load_ps(T   );
			XMM3	 = _mm_load_ps(T+ 4);
			XMM6	 = _mm_load_ps(T+ 8);
			XMM7	 = _mm_load_ps(T+12);
			XMM0	 = _mm_shuffle_ps(XMM0, XMM4,_MM_SHUFFLE(0,2,0,2));
			XMM2	 = _mm_shuffle_ps(XMM2, XMM4,_MM_SHUFFLE(1,3,1,3));
			XMM4	 = XMM0;
			XMM5	 = XMM2;
			XMM0	 = _mm_mul_ps(XMM0, XMM1);
			XMM1	 = _mm_load_ps(w+12);
			XMM2	 = _mm_mul_ps(XMM2, XMM3);
			XMM3	 = _mm_load_ps(w+ 8);
			XMM4	 = _mm_shuffle_ps(XMM4, XMM4, _MM_SHUFFLE(0,1,2,3));
			XMM5	 = _mm_shuffle_ps(XMM5, XMM5, _MM_SHUFFLE(0,1,2,3));
			XMM4	 = _mm_mul_ps(XMM4, XMM6);
			XMM6	 = XMM1;
			XMM5	 = _mm_mul_ps(XMM5, XMM7);
			XMM7	 = _mm_load_ps(T+16);
			XMM0	 = _mm_sub_ps(XMM0, XMM2);
			XMM2	 = _mm_load_ps(T+20);
			XMM4	 = _mm_add_ps(XMM4, XMM5);
			XMM5	 = _mm_load_ps(T+24);
			_mm_store_ps(x0    +4, XMM0);
			XMM0	 = _mm_load_ps(T+28);
			XMM1	 = _mm_shuffle_ps(XMM1, XMM3,_MM_SHUFFLE(0,2,0,2));
			XMM6	 = _mm_shuffle_ps(XMM6, XMM3,_MM_SHUFFLE(1,3,1,3));
			_mm_store_ps(out +i  , XMM4);
			XMM3	 = XMM1;
			XMM4	 = XMM6;
			XMM1	 = _mm_mul_ps(XMM1, XMM7);
			XMM6	 = _mm_mul_ps(XMM6, XMM2);
			XMM3	 = _mm_shuffle_ps(XMM3, XMM3, _MM_SHUFFLE(0,1,2,3));
			XMM4	 = _mm_shuffle_ps(XMM4, XMM4, _MM_SHUFFLE(0,1,2,3));
			XMM3	 = _mm_mul_ps(XMM3, XMM5);
			XMM4	 = _mm_mul_ps(XMM4, XMM0);
			XMM1	 = _mm_sub_ps(XMM1, XMM6);
			XMM3	 = _mm_add_ps(XMM3, XMM4);
			_mm_store_ps(x0      , XMM1);
			_mm_store_ps(out +i+4, XMM3);
			w	+= 16;
			T	+= 32;
		}
#else
		for(i=0;i<n4;i+=4){
			__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
			x0	-= 4;
			x1	-= 4;
			XMM0	 = _mm_load_ps(w+4);
			XMM4	 = _mm_load_ps(w  );
			XMM2	 = XMM0;
			XMM1	 = _mm_load_ps(T   );
			XMM3	 = _mm_load_ps(T+ 4);
			XMM6	 = _mm_load_ps(T+ 8);
			XMM7	 = _mm_load_ps(T+12);
			XMM0	 = _mm_shuffle_ps(XMM0, XMM4,_MM_SHUFFLE(0,2,0,2));
			XMM2	 = _mm_shuffle_ps(XMM2, XMM4,_MM_SHUFFLE(1,3,1,3));
			XMM4	 = XMM0;
			XMM5	 = XMM2;
			XMM0	 = _mm_mul_ps(XMM0, XMM1);
			XMM2	 = _mm_mul_ps(XMM2, XMM3);
			XMM4	 = _mm_shuffle_ps(XMM4, XMM4, _MM_SHUFFLE(0,1,2,3));
			XMM5	 = _mm_shuffle_ps(XMM5, XMM5, _MM_SHUFFLE(0,1,2,3));
			XMM4	 = _mm_mul_ps(XMM4, XMM6);
			XMM5	 = _mm_mul_ps(XMM5, XMM7);
			XMM0	 = _mm_sub_ps(XMM0, XMM2);
			XMM4	 = _mm_add_ps(XMM4, XMM5);
			_mm_store_ps(x0    , XMM0);
			_mm_store_ps(x1    , XMM0);
			_mm_store_ps(out +i, XMM4);
			_mm_store_ps(out1+i, XMM4);
			w	+= 8;
			T	+= 16;
		}
	}
	else
	{
		for(i=0;i<n4;i+=4){
			__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
			x0	-= 4;
			XMM0	 = _mm_load_ps(w+4);
			XMM4	 = _mm_load_ps(w  );
			XMM2	 = XMM0;
			XMM1	 = _mm_load_ps(T   );
			XMM3	 = _mm_load_ps(T+ 4);
			XMM6	 = _mm_load_ps(T+ 8);
			XMM7	 = _mm_load_ps(T+12);
			XMM0	 = _mm_shuffle_ps(XMM0, XMM4,_MM_SHUFFLE(0,2,0,2));
			XMM2	 = _mm_shuffle_ps(XMM2, XMM4,_MM_SHUFFLE(1,3,1,3));
			XMM4	 = XMM0;
			XMM5	 = XMM2;
			XMM0	 = _mm_mul_ps(XMM0, XMM1);
			XMM2	 = _mm_mul_ps(XMM2, XMM3);
			XMM4	 = _mm_shuffle_ps(XMM4, XMM4, _MM_SHUFFLE(0,1,2,3));
			XMM5	 = _mm_shuffle_ps(XMM5, XMM5, _MM_SHUFFLE(0,1,2,3));
			XMM4	 = _mm_mul_ps(XMM4, XMM6);
			XMM5	 = _mm_mul_ps(XMM5, XMM7);
			XMM0	 = _mm_sub_ps(XMM0, XMM2);
			XMM4	 = _mm_add_ps(XMM4, XMM5);
			_mm_store_ps(x0    , XMM0);
			_mm_store_ps(out +i, XMM4);
			w	+= 8;
			T	+= 16;
		}
#endif
	}
#else														/* SSE Optimize */
void mdct_forward(mdct_lookup *init, DATA_TYPE *in, DATA_TYPE *out){
  int n=init->n;
  int n2=n>>1;
  int n4=n>>2;
  int n8=n>>3;
  DATA_TYPE *w=alloca(n*sizeof(*w)); /* forward needs working space */
  DATA_TYPE *w2=w+n2;

  /* rotate */

  /* window + rotate + step 1 */
  
  REG_TYPE r0;
  REG_TYPE r1;
  DATA_TYPE *x0=in+n2+n4;
  DATA_TYPE *x1=x0+1;
  DATA_TYPE *T=init->trig+n2;
  
  int i=0;
  
  for(i=0;i<n8;i+=2){
    x0 -=4;
    T-=2;
    r0= x0[2] + x1[0];
    r1= x0[0] + x1[2];       
    w2[i]=   MULT_NORM(r1*T[1] + r0*T[0]);
    w2[i+1]= MULT_NORM(r1*T[0] - r0*T[1]);
    x1 +=4;
  }

  x1=in+1;
  
  for(;i<n2-n8;i+=2){
    T-=2;
    x0 -=4;
    r0= x0[2] - x1[0];
    r1= x0[0] - x1[2];       
    w2[i]=   MULT_NORM(r1*T[1] + r0*T[0]);
    w2[i+1]= MULT_NORM(r1*T[0] - r0*T[1]);
    x1 +=4;
  }
    
  x0=in+n;

  for(;i<n2;i+=2){
    T-=2;
    x0 -=4;
    r0= -x0[2] - x1[0];
    r1= -x0[0] - x1[2];       
    w2[i]=   MULT_NORM(r1*T[1] + r0*T[0]);
    w2[i+1]= MULT_NORM(r1*T[0] - r0*T[1]);
    x1 +=4;
  }


  mdct_butterflies(init,w+n2,n2);
  mdct_bitreverse(init,w);

  /* roatate + window */

  T=init->trig+n2;
  x0=out+n2;

  for(i=0;i<n4;i++){
    x0--;
    out[i] =MULT_NORM((w[0]*T[0]+w[1]*T[1])*init->scale);
    x0[0]  =MULT_NORM((w[0]*T[1]-w[1]*T[0])*init->scale);
    w+=2;
    T+=2;
  }
#endif														/* SSE Optimize */
}
