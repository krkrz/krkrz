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

/*
	June 20, 2003    W.Dee <dee@kikyou.info>

		added some SSE/3DNow optimizations.
		This file is still under license of original libvorbis
		license.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vorbis/codec.h"
#include "mdct.h"
#include "os.h"
#include "misc.h"

/* build lookups for trig functions; also pre-figure scaling and
   some window function algebra. */
extern int CPU_SSE;
extern int CPU_3DN;

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
}

__declspec(align(16)) static int neg_0_2[] = {0x80000000, 0x0, 0x80000000, 0x0};
__declspec(align(16)) static int neg_0_1_2_3[] = {0x80000000, 0x80000000, 0x80000000, 0x80000000};
__declspec(align(16)) static int neg_1_2[] = {0x0, 0x80000000, 0x80000000, 0x0};
__declspec(align(16)) static int neg_1_3[] = {0x0, 0x80000000, 0x0, 0x80000000};
__declspec(align(16)) static int neg_0_1[] = {0x80000000, 0x80000000, 0x0, 0x0};
__declspec(align(16)) static int neg_0[] = {0x80000000, 0x0, 0x0, 0x0};

__declspec(align(16)) static float packed_cPI2_8_inv[] = {cPI2_8, cPI2_8, 1.0, -1.0};
__declspec(align(16)) static float packed_cPI2_8[] = {cPI2_8, cPI2_8, 1.0, 1.0};

__declspec(align(16)) static float packed_2_3_a[] = {cPI1_8, cPI3_8, 1.0, 1.0};
__declspec(align(16)) static float packed_2_3_b[] = {-cPI3_8, cPI1_8, 0.0, 0.0};

__declspec(align(16)) static float packed_3_1_c[] = {cPI3_8, cPI3_8, cPI2_8, cPI2_8};
__declspec(align(16)) static float packed_3_1_d[] = {-cPI1_8, cPI1_8, -cPI2_8, cPI2_8};

__declspec(align(16)) static float packed_1_3_b[] = {cPI1_8, cPI3_8, 1.0, -1.0};
__declspec(align(16)) static float packed_1_3_a[] = {cPI3_8, -cPI1_8, 0.0, 0.0};

__declspec(align(16)) static float packed_ddbb[] = {cPI3_8, cPI1_8, cPI2_8, cPI2_8};
__declspec(align(16)) static float packed_ccaa[] = {cPI1_8, -cPI3_8, cPI2_8, -cPI2_8};

__declspec(align(16)) static float packed_halve[] = {0.5, 0.5, 0.5, 0.5};


__declspec(align(8)) static int neg_q_1[] = {0, 0x80000000};
__declspec(align(8)) static int neg_q_0[] = {0x80000000, 0};
__declspec(align(8)) static int neg_q_0_1[] = {0x80000000, 0x80000000};
__declspec(align(8)) static float packed_halve_q[] = {0.5, 0.5};

__declspec(align(8)) static float pi_q_18_m38[] = { cPI1_8, -cPI3_8 };
__declspec(align(8)) static float pi_q_38_18[] =  { cPI3_8, cPI1_8 };
__declspec(align(8)) static float pi_q_28_28[] =  { cPI2_8, cPI2_8 };
__declspec(align(8)) static float pi_q_38_m18[] = { cPI3_8, -cPI1_8 };
__declspec(align(8)) static float pi_q_18_38[] =  { cPI1_8, cPI3_8 };
__declspec(align(8)) static float pi_q_m18_38[] = { -cPI1_8, cPI3_8 };
__declspec(align(8)) static float pi_q_m38_18[] = { -cPI3_8, cPI1_8 };



/* 8 point butterfly (in place, 4 register) (3dn) */
STIN void mdct_butterfly_8_3dn(DATA_TYPE *x){
	_asm
	{
		mov				eax,				x
	}
	_asm
	{
		// make mm1=(D,C) mm0=(B,A)
		movq			mm0,				[eax]					// mm0:	x[1]			x[0]
		movq			mm2,				[eax+4*4]				// mm2:	x[5]			x[4]

		movq			mm1,				mm0
		pxor			mm0,				neg_q_1					// mm0:	-x[1]			x[0]
		pxor			mm1,				neg_q_0					// mm1:	x[1]			-x[0]
		pfadd			mm0,				mm2						// mm0:	B				A			***mm0
		pfadd			mm1,				mm2						// mm1:	D				C			***mm1

		// make mm3=(H,G) mm2=(F,E)
		movq			mm2,				[eax+4*2]				// mm2:	x[3]			x[2]
		movq			mm4,				[eax+4*6]				// mm4:	x[7]			x[6]

		movq			mm3,				mm2
		pxor			mm2,				neg_q_1					// mm2:	-x[3]			x[2]
		pxor			mm3,				neg_q_0					// mm3:	x[3]			-x[2]
		pfadd			mm2,				mm4						// mm2:	F				E			***mm2
		pfadd			mm3,				mm4						// mm3:	H				G			***mm3

		// make (H,E) (D,A)
		movq			mm4,				mm2
		movq			mm5,				mm0
		punpckldq		mm4,				mm4						// mm4:	E				E
		punpckldq		mm5,				mm5						// mm5:	A				A
		punpckhdq		mm4,				mm3						// mm4:	H				E
		punpckhdq		mm5,				mm1						// mm5:	D				A

		// make x[4..7]
		movq			mm6,				mm4
		pfadd			mm6,				mm5						// mm5:	H+D				E+A
		movq			[eax +6*4],			mm6						// store

		pfsub			mm4,				mm5						// mm4:	H-D				E-A
		movq			[eax +4*4],			mm4

		// make mm3=(F,G) mm0=(C,B)
		punpckhdq		mm2,				mm2						// mm2:	F				F
		punpckhdq		mm0,				mm0						// mm0:	B				B
		punpckldq		mm3,				mm2						// mm3:	F				G
		punpckldq		mm0,				mm1						// mm0:	C				B

		// make x[0..3]
		movq			mm4,				mm3
		pxor			mm0,				neg_q_1					// mm0:	-C				B
		pfadd			mm4,				mm0						// mm4:	F-C				G+B
		movq			[eax +0*4],			mm4						// store
		pfsub			mm3,				mm0						// mm3:	F+C				G-B
		movq			[eax +2*4],			mm3						// store

	}

/*
	float A,B,C,D,E,F,G,H;
	A   = x[4] + x[0];
	B   = x[5] - x[1];
	C   = x[4] - x[0];
	D   = x[5] + x[1];
	E   = x[6] + x[2];
	F   = x[7] - x[3];
	G   = x[6] - x[2];
	H   = x[7] + x[3];
	x[0] = G   + B;
	x[1] = F   - C;
	x[2] = G   - B;
	x[3] = F   + C;
	x[4] = E   - A;
	x[5] = H   - D;
	x[6] = E   + A;
	x[7] = H   + D;
*/
}

/* 8 point butterfly (in place, 4 register) */
STIN void mdct_butterfly_8(DATA_TYPE *x){
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
}

/* 16 point butterfly (sse) */
STIN void mdct_butterfly_16_sse(DATA_TYPE *x){
_asm
{
	mov			eax,	x


	movaps		xmm0,	[eax]						// a xmm0 =	x3		x2		x1		x0
	movaps		xmm5,	[eax + 16]					// b xmm4 =	x7		x6		x5		x4
	movaps		xmm1,	[eax + 32]					// a xmm1 =	x11		x10		x9		x8
	movaps		xmm4,	[eax + 48]					// b xmm4 =	x15		x14		x13		x12

	movaps		xmm2,	xmm0						// a xmm2 =	x3		x2		x1		x0
	movaps		xmm6,	xmm4						// b xmm6 =	x15		x14		x13		x12
	subps		xmm2,	xmm1						// a xmm2 =	d		-c		b		a
	subps		xmm6,	xmm5						// b xmm6 =	d		c		b		a

	addps		xmm0,	xmm1						// a
	addps		xmm4,	xmm5						// b

	movaps		xmm7,	xmm6						// b xmm7 =	d		c		b		a
	movaps		xmm3,	xmm2						// a xmm3 =	d		-c		b		a
	movaps		xmm1,	xmm6						// b xmm1 =	d		c		b		a

	shufps		xmm3,	xmm3,	2*64+3*16+1*4+1		// a xmm3 =	-c		d		b		b
	shufps		xmm7,	xmm7,	3*64+2*16+0*4+0		// b xmm7 =	d		c		a		a
	shufps		xmm2,	xmm2,	2*64+3*16+0*4+0		// a xmm2 =	-c		d		a		a
	shufps		xmm1,	xmm1,	3*64+2*16+1*4+1		// b xmm1 =	d		c		b		b
	xorps		xmm2,	neg_0						// a xmm2 =	-c		d		a		-a
	xorps		xmm1,	neg_0						// b xmm1 =	d		c		b		-b
	xorps		xmm6,	xmm6						// b xmm6 =	0		0		0		0
	movhps		xmm2,	xmm6						// a xmm2 =	0		0		a		-a
	movhps		xmm1,	xmm6						// b xmm1 =	0		0		b		-b

	subps		xmm3,	xmm2						// a xmm3 =	-c		d		b-a		b+a
	addps		xmm7,	xmm1						// b xmm7 = d		c		a+b		a-b
	mulps		xmm3,	packed_cPI2_8_inv			// a xmm3 =	c		d		(b-a)*C	(b+a)*C
	mulps		xmm7,	packed_cPI2_8				// b xmm7 =	d		c		(a+b)*C	(a-b)*C

	movaps		xmm2,	xmm7						// a xmm2 =	x7		x6		x5		x4
	movaps		xmm6,	xmm4						// b xmm5 =	x7		x6		x5		x4
	addps		xmm2,	xmm3						// a xmm2 =	d		c		b		a
	addps		xmm6,	xmm0						// b xmm6 =	d		c		b		a

	subps		xmm7,	xmm3						// a xmm7 =	h		g		f		e
	subps		xmm4,	xmm0						// b xmm4 =	h		g		f		e

	movaps		xmm1,	xmm7						// a xmm1 =	h		g		f		e
	movaps		xmm5,	xmm4						// b xmm5 =	h		g		f		e
	shufps		xmm1,	xmm1,	3*64+2*16+3*4+2		// a xmm1 =	h		g		h		g
	shufps		xmm5,	xmm5,	3*64+2*16+3*4+2		// b xmm5 =	h		g		h		g
	shufps		xmm7,	xmm7,	0*64+1*16+0*4+1		// a xmm7 =	e		f		e		f
	shufps		xmm4,	xmm4,	0*64+1*16+0*4+1		// b xmm4 =	e		f		e		f
	xorps		xmm7,	neg_1_2						// a xmm7 =	e		-f		-e		f
	xorps		xmm4,	neg_1_2						// b xmm4 =	e		-f		-e		f
	addps		xmm7,	xmm1						// a xmm7 =	h+e		g-f		h-e		g+f
	addps		xmm4,	xmm5						// b xmm4 =	h+e		g-f		h-e		g+f
	movaps		[eax],	xmm7						// a store x[3] x[2] x[1] x[0]
	movaps		[eax + 32],	xmm4					// b store x[3] x[2] x[1] x[0]

	movaps		xmm1,	xmm2						// a xmm1 =	d		c		b		a
	movaps		xmm5,	xmm6						// b xmm5 =	d		c		b		a
	shufps		xmm1,	xmm1,	3*64+2*16+3*4+2		// a xmm1 =	d		c		d		c
	shufps		xmm5,	xmm5,	3*64+2*16+3*4+2		// b xmm5 =	d		c		d		c
	shufps		xmm2,	xmm2,	1*64+0*16+1*4+0		// a xmm2 =	b		a		b		a
	shufps		xmm6,	xmm6,	1*64+0*16+1*4+0		// b xmm6 =	b		a		b		a
	xorps		xmm2,	neg_0_1						// a xmm2 =	b		a		-b		-a
	xorps		xmm6,	neg_0_1						// b xmm6 =	b		a		-b		-a
	addps		xmm2,	xmm1						// a xmm2 =	d+b		c+a		d-b		c-a
	addps		xmm6,	xmm5						// b xmm6 =	d+b		c+a		d-b		c-a
	movaps		[eax + 16],	xmm2					// a store x[7] x[6] x[5] x[4]
	movaps		[eax + 16 + 32],	xmm6			// b store x[7] x[6] x[5] x[4]
}
/*
 REG_TYPE a,b,c,d;

a      = (x[0]  - x[8]);
b      = (x[1]  - x[9]);
c      =-(x[2]  - x[10]);
d      = (x[3]  - x[11]);
x[8]  += x[0];
x[9]  += x[1];
x[10] += x[2];
x[11] += x[3];
x[0]   = MULT_NORM((b   + a) * cPI2_8);
x[1]   = MULT_NORM((b   - a) * cPI2_8);
x[2]   = d;
x[3]   = c;

a      = x[12] - x[4];
b      = x[13] - x[5];
c      = x[14] - x[6];
d      = x[15] - x[7];
x[12] += x[4];
x[13] += x[5];
x[14] += x[6];
x[15] += x[7];
x[4]   = MULT_NORM((a   - b) * cPI2_8);
x[5]   = MULT_NORM((a   + b) * cPI2_8);
x[6]   = c;
x[7]   = d;
*/

}

/* 16 point butterfly (in place, 4 register) (3dn) */
STIN void mdct_butterfly_16_3dn(DATA_TYPE *x){
  REG_TYPE r0;
  REG_TYPE r1;

	_asm
	{
		mov			eax,		x
	}
	_asm
	{
		movq				mm0,				[eax +  0*4]
/**/	movq				mm3,				[eax +  2*4]
		movq				mm1,				mm0
/**/	movq				mm4,				[eax + 10*4]
		pfsub				mm1,				[eax +  8*4]
/**/	movq				mm5,				mm4
		pfadd				mm0,				[eax +  8*4]
/**/	pxor				mm3,				neg_q_0
		movq				[eax +  8*4],		mm0
/**/	pxor				mm5,				neg_q_1
/**/	pfadd				mm4,				[eax +  2*4]

		movq				mm2,				mm1
/**/	pfadd				mm5,				mm3
		pxor				mm2,				neg_q_0
/**/	movq				[eax + 10*4],		mm4
		pfacc				mm1,				mm2
/**/	punpckldq			mm3,				mm5
		pfmul				mm1,				pi_q_28_28
/**/	punpckhdq			mm5,				mm3
		movq				[eax +  0*4],		mm1
/**/	movq				[eax +  2*4],		mm5
	}
/*
		   r0 = x[0]  - x[8];
		   r1 = x[1]  - x[9];
           x[8]  += x[0];
           x[9]  += x[1];
           x[0]   = MULT_NORM(( r1   + r0) * cPI2_8);
           x[1]   = MULT_NORM(( r1   - r0) * cPI2_8);
           r0     = - x[2] + x[10] ;
           r1     =   x[3] - x[11];
           x[10] += x[2];
           x[11] += x[3];
           x[2]   = r1;
           x[3]   = r0;
*/
	_asm
	{
		movq				mm0,				[eax + 12*4]
/**/	movq				mm6,				[eax + 14*4]
		movq				mm1,				mm0
/**/	movq				mm7,				mm6
		pfsub				mm1,				[eax +  4*4]
		pfadd				mm0,				[eax +  4*4]
/**/	pfadd				mm6,				[eax +  6*4]
		movq				[eax + 12*4],		mm0

		movq				mm2,				mm1
/**/	pfsub				mm7,				[eax +  6*4]
		pxor				mm1,				neg_q_1
/**/	movq				[eax + 14*4],		mm6
		pfacc				mm1,				mm2
/**/	movq				[eax +  6*4],		mm7
		pfmul				mm1,				pi_q_28_28
		movq				[eax +  4*4],		mm1
	}
/*
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
*/

   mdct_butterfly_8_3dn(x);
   mdct_butterfly_8_3dn(x+8);
}

/* 16 point butterfly (in place, 4 register) */
STIN void mdct_butterfly_16(DATA_TYPE *x){
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

   mdct_butterfly_8(x);
   mdct_butterfly_8(x+8);
}

/* 32 point butterfly (sse) */
STIN void mdct_butterfly_32_sse(DATA_TYPE *x){
//REG_TYPE	a,b,c,d;
_asm
{
	mov			eax,	x
}


_asm
{
	movaps		xmm5,	[eax + 8*4]					// b xmm5 =	x11		x10		x9		x8
	movaps		xmm1,	[eax + 12*4]				// a xmm1 =	x15		x14		x13		x12
	movaps		xmm4,	[eax + 24*4]				// b xmm4 =	x27		x26		x25		x24
	movaps		xmm0,	[eax + 28*4]				// a xmm0 =	x31		x30		x29		x28


	movaps		xmm6,	xmm4						// b xmm6 =	x27		x26		x25		x24
	movaps		xmm2,	xmm0						// a xmm2 =	x31		x30		x29		x28
	subps		xmm6,	xmm5						// b xmm6 =	d		c		b		a
	subps		xmm2,	xmm1						// a xmm2 =	d		c		b		a

	addps		xmm4,	xmm5						// b
	addps		xmm0,	xmm1						// a
	movaps		[eax + 24*4],	xmm4				// b store x27 x26 x25 x24
	movaps		[eax + 28*4],	xmm0				// a store x31 x30 x29 x28

	movaps		xmm3,	xmm2						// a xmm3 =	d		c		b		a
	movaps		xmm7,	xmm6						// b xmm7 =	d		c		b		a
	shufps		xmm3,	xmm3,	3*64+2*16+0*4+0		// a xmm3 =	d		c		a		a
	shufps		xmm7,	xmm7,	2*64+2*16+1*4+0		// b xmm7 =	c		c		b		a
	prefetcht0	[eax + 64*4]
	shufps		xmm2,	xmm2,	3*64+2*16+1*4+1		// a xmm2 =	d		c		b		b
	shufps		xmm6,	xmm6,	3*64+3*16+0*4+1		// b xmm6 =	d		d		a		b

	mulps		xmm7,	packed_3_1_c				// b xmm7 =	c*C2	c*C2	b*C3	a*C3
	mulps		xmm3,	packed_2_3_a				// a xmm3 =	d		c		a*C3	a*C1
	mulps		xmm6,	packed_3_1_d				// b xmm6 =	d*C2	-d*C2	a*C1	-b*C1
	mulps		xmm2,	packed_2_3_b				// a xmm2 =	0		0		b*C1	-b*C3

	addps		xmm6,	xmm7						// b
	addps		xmm2,	xmm3						// a
	movaps		[eax + 8*4],	xmm6				// b store x11 x10 x9 x8
	movaps		[eax + 12*4],	xmm2				// a store x15 x14 x13 x12
}

_asm
{
	movaps		xmm4,	[eax + 0*4]					// b xmm4 =	x3		x2		x1		x0
	movaps		xmm0,	[eax + 4*4]					// a xmm0 =	x7		x6		x5		x4
	movaps		xmm5,	[eax + 16*4]				// b xmm5 =	x19		x18		x17		x16
	movaps		xmm1,	[eax + 20*4]				// a xmm1 =	x23		x22		x21		x20

	movaps		xmm6,	xmm4						// b xmm6 =	x3		x2		x1		x0
	movaps		xmm2,	xmm0						// a xmm2 =	x7		x6		x5		x4
	subps		xmm6,	xmm5						// b xmm6 =	d		c		b		a
	subps		xmm2,	xmm1						// a xmm2 =	d		-c		b		a

	addps		xmm4,	xmm5						// b
	addps		xmm0,	xmm1						// a
	movaps		[eax + 16*4],	xmm4				// b store x19 x18 x17 x16
	movaps		[eax + 20*4],	xmm0				// a store x23 x22 x21 x20

	movaps		xmm3,	xmm2						// a xmm3 =	d		-c		b		a
	movaps		xmm7,	xmm6						// b xmm7 =	d		c		b		a
	shufps		xmm3,	xmm3,	2*64+3*16+1*4+1		// a xmm3 =	-c		d		b		b
	shufps		xmm7,	xmm7,	3*64+3*16+1*4+1		// b xmm7 =	d		d		b		b
	shufps		xmm2,	xmm2,	2*64+3*16+0*4+0		// a xmm2 =	-c		d		a		a
	shufps		xmm6,	xmm6,	2*64+2*16+0*4+0		// b xmm6 =	c		c		a		a

	mulps		xmm7,	packed_ddbb					// b xmm7 =	d*C2	d*C2	b*C1	b*C3
	mulps		xmm3,	packed_1_3_b				// a xmm3 =	c		d		b*C3	b*C1
	mulps		xmm6,	packed_ccaa					// b xmm6 =	-c*C2	c*C2	-a*C3	a*C1
	mulps		xmm2,	packed_1_3_a				// a xmm2 =	0		0		-a*C1	a*C3

	addps		xmm6,	xmm7						// b
	addps		xmm2,	xmm3						// a
	movaps		[eax + 0*4],	xmm6				// b store x4 x3 x2 x1
	movaps		[eax + 4*4],	xmm2				// a store x7 x6 x5 x4
}
/*
a     = x[28] - x[12];
b     = x[29] - x[13];
c     = x[30] - x[14];
d     = x[31] - x[15];
x[28] += x[12];
x[29] += x[13];
x[30] += x[14];
x[31] += x[15];
x[12]  = MULT_NORM( a * cPI1_8  -  b * cPI3_8 );
x[13]  = MULT_NORM( a * cPI3_8  +  b * cPI1_8 );
x[14]  = c;
x[15]  = d;
a     = x[24] - x[8];
b     = x[25] - x[9];
c     = x[26] - x[10];
d     = x[27] - x[11];
x[24] += x[8];
x[25] += x[9];
x[26] += x[10];
x[27] += x[11];
x[8]   = MULT_NORM( a * cPI3_8  -  b * cPI1_8 );
x[9]   = MULT_NORM( b * cPI3_8  +  a * cPI1_8 );
x[10]  = MULT_NORM(( c  - d ) * cPI2_8);
x[11]  = MULT_NORM(( c  + d ) * cPI2_8);
a     =  (x[4]  - x[20]);
b     =  (x[5]  - x[21]);
c     = -(x[6]  - x[22]);
d     =  (x[7]  - x[23]);
x[20] += x[4];
x[21] += x[5];
x[22] += x[6];
x[23] += x[7];
x[4]   = MULT_NORM( b * cPI1_8  +  a * cPI3_8 );
x[5]   = MULT_NORM( b * cPI3_8  -  a * cPI1_8 );
x[6]   = d;
x[7]   = c;
a     = x[0]  - x[16];
b     = x[1]  - x[17];
c     = x[2]  - x[18];
d     = x[3]  - x[19];
x[16] += x[0];
x[17] += x[1];
x[18] += x[2];
x[19] += x[3];
x[0]   = MULT_NORM( b * cPI3_8  +  a * cPI1_8 );
x[1]   = MULT_NORM( b * cPI1_8  -  a * cPI3_8 );
x[2]   = MULT_NORM(( d  + c ) * cPI2_8);
x[3]   = MULT_NORM(( d  - c ) * cPI2_8);
*/

	   mdct_butterfly_16_sse(x);
	   mdct_butterfly_16_sse(x+16);
}

/* 32 point butterfly (in place, 4 register) (3dn) */
STIN void mdct_butterfly_32_3dn(DATA_TYPE *x){

	REG_TYPE r0;
	REG_TYPE r1;
	REG_TYPE r2;
	REG_TYPE r3;

	_asm
	{
		mov			eax,			x	
	}

	_asm
	{
/**/	movq				mm2,				[eax + 28*4]
		movq				mm0,				[eax + 30*4]
/**/	movq				mm3,				mm2
		movq				mm1,				mm0
/**/	pfsub				mm3,				[eax + 12*4]
		pfsub				mm1,				[eax + 14*4]
/**/	pfadd				mm2,				[eax + 12*4]
		pfadd				mm0,				[eax + 14*4]
/**/	movq				[eax + 28*4],		mm2
		movq				[eax + 30*4],		mm0
/**/	movq				mm2,				mm3
		movq				[eax + 14*4],		mm1
/**/	pfmul				mm3,				pi_q_18_m38

/**/	pfmul				mm2,				pi_q_38_18
		movq				mm4,				[eax + 26*4]
/**/	pfacc				mm3,				mm2
		movq				mm5,				mm4
/**/	movq				[eax + 12*4],		mm3

		pfsub				mm5,				[eax + 10*4]
		pfadd				mm4,				[eax + 10*4]
		movq				[eax + 26*4],		mm4

		movq				mm6,				mm5
/**/	movq				mm7,				[eax + 24*4]
		pxor				mm6,				neg_q_1
/**/	movq				mm0,				mm7
		pfacc				mm6,				mm5					// mm6 =	r0+r1		r0-r1
/**/	pfsub				mm0,				[eax +  8*4]
		pfmul				mm6,				pi_q_28_28
/**/	pfadd				mm7,				[eax +  8*4]
		movq				[eax + 10*4],		mm6

/**/	movq				[eax + 24*4],		mm7
		movq				mm2,				[eax +  6*4]
/**/	movq				mm1,				mm0
		movq				mm3,				[eax + 22*4]
/**/	pfmul				mm0,				pi_q_38_m18
		movq				mm4,				mm3
/**/	pfmul				mm1,				pi_q_18_38
		pxor				mm2,				neg_q_0
/**/	pfacc				mm0,				mm1
		pxor				mm4,				neg_q_1
/**/	movq				[eax +  8*4],		mm0

		pfadd				mm3,				[eax +  6*4]
/**/	movq				mm5,				[eax +  4*4]
		pfadd				mm4,				mm2
/**/	movq				mm6,				mm5
		movq				[eax + 22*4],		mm3
/**/	pfsub				mm6,				[eax + 20*4]
		punpckldq			mm2,				mm4
/**/	pfadd				mm5,				[eax + 20*4]
		punpckhdq			mm4,				mm2
/**/	movq				[eax + 20*4],		mm5
		movq				[eax +  6*4],		mm4

/*!*/	movq				mm3,				[eax +  0*4]
/**/	movq				mm7,				mm6
/*!*/	movq				mm4,				mm3
		movq				mm0,				[eax +  2*4]
/*!*/	pfsub				mm4,				[eax + 16*4]
/**/	pfmul				mm6,				pi_q_38_18
		movq				mm1,				mm0
/*!*/	pfadd				mm3,				[eax + 16*4]
/**/	pfmul				mm7,				pi_q_m18_38
		pfsub				mm1,				[eax + 18*4]
/*!*/	movq				[eax + 16*4],		mm3
/**/	pfacc				mm6,				mm7
		pfadd				mm0,				[eax + 18*4]
/**/	movq				[eax +  4*4],		mm6

		movq				[eax + 18*4],		mm0
/*!*/	movq				mm5,				mm4
		movq				mm2,				mm1					// mm2 =	r1			r0
/*!*/	pfmul				mm4,				pi_q_18_38
		pxor				mm2,				neg_q_0				// mm2 =	r1			-r0
/*!*/	pfmul				mm5,				pi_q_m38_18
		pfacc				mm1,				mm2					// mm2 =	r1-r0		r1+r0
/*!*/	pfacc				mm4,				mm5
		pfmul				mm1,				pi_q_28_28			// mm2 =	(r1-r0)*C	(r1+r0)*C
/*!*/	movq				[eax +  0*4],		mm4
		movq				[eax +  2*4],		mm1

	}
/* 

	   r0	  = x[30] - x[14];
	   r1	  = x[31] - x[15];
	   r2     = x[30] + x[14];
	   r3     = x[31] + x[15];
       x[30]  = r2;
	   x[31]  = r3;
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
	   x[9]   = MULT_NORM( r0 * cPI1_8  +  r1 * cPI3_8 );
	   r0     = -x[6] + x[22];
	   r1     =  x[7] - x[23];
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
*/
	   mdct_butterfly_16_3dn(x);
	   mdct_butterfly_16_3dn(x+16);

	   _asm {femms}
}
/* 32 point butterfly (in place, 4 register) */
STIN void mdct_butterfly_32(DATA_TYPE *x){
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

	   mdct_butterfly_16(x);
	   mdct_butterfly_16(x+16);
}

/* N point first stage butterfly (in place, 2 register) */
STIN void mdct_butterfly_first_sse(DATA_TYPE *T,
					DATA_TYPE *x,
					int points){

  DATA_TYPE *x1        = x          + points      - 8;
  DATA_TYPE *x2        = x          + (points>>1) - 8;
  REG_TYPE   r0;
  REG_TYPE   r1;

_asm
{
	push		ebx
	mov			eax,		x1
	mov			ebx,		x2
	mov			esi,		T
	mov			ecx,		x
	movaps		xmm7,		neg_1_3

loopg:
	movaps		xmm0,		[eax + 16]					// xmm0 =	x1_7		x1_6		x1_5		x1_4
	movaps		xmm1,		[ebx + 16]					// xmm1 =	x2_7		x2_6		x2_5		x2_4


	movaps		xmm2,		xmm0						// xmm3 =	x1_7		x1_6		x1_5		x1_4
	subps		xmm0,		xmm1						// xmm0 =	d			c			b			a
	addps		xmm2,		xmm1						//
	movaps		[eax + 16],	xmm2						// store

	movhps		xmm2,		[esi]						// xmm2 =	T1			T0			*			*
	movlps		xmm2,		[esi + 16]					// xmm2 =	T1			T0			T1t			T0t

	movaps		xmm3,		xmm2						// xmm3 =	T1			T0			T1t			T0t
	shufps		xmm3,		xmm3,	2*64+3*16+0*4+1		// xmm3 =	T0			T1			T0t			T1t

	movaps		xmm4,		xmm0						// xmm4 =	d			c			b			a
	prefetcht0				[esi + 64]
	shufps		xmm4,		xmm4,	3*64+3*16+1*4+1		// xmm4 =	d			d			b			b

	mulps		xmm3,		xmm4						// xmm3 =	d*T0		d*T1		b*T0t		b*T1t

	shufps		xmm0,		xmm0,	2*64+2*16+0*4+0		// xmm0 =	c			c			a			a

	mulps		xmm0,		xmm2						// xmm0 =	c*T1		c*T0		a*T1t		a*T0t

	xorps		xmm0,		xmm7						// xmm0 =	-c*T1t		c*T0		-a*T1t		a*T0t

	addps		xmm0,		xmm3
	movaps		[ebx + 16],	xmm0

	add			esi,		32

	movaps		xmm0,		[eax]						// xmm0 =	x1_3		x1_2		x1_1		x1_0
	movaps		xmm1,		[ebx]						// xmm1 =	x2_3		x2_2		x2_1		x2_0
	movaps		xmm2,		xmm0						// xmm3 =	x1_3		x1_2		x1_1		x1_0
	subps		xmm0,		xmm1						// xmm0 =	d			c			b			a
	addps		xmm2,		xmm1						//
	movaps		[eax],		xmm2						// store

	movhps		xmm2,		[esi]						// xmm2 =	T1			T0			*			*
	movlps		xmm2,		[esi + 16]					// xmm2 =	T1			T0			T1t			T0t

	movaps		xmm3,		xmm2						// xmm3 =	T1			T0			T1t			T0t
	shufps		xmm3,		xmm3,	2*64+3*16+0*4+1		// xmm3 =	T0			T1			T0t			T1t

	movaps		xmm4,		xmm0						// xmm4 =	d			c			b			a
	shufps		xmm4,		xmm4,	3*64+3*16+1*4+1		// xmm4 =	d			d			b			b

	mulps		xmm3,		xmm4						// xmm3 =	d*T0		d*T1		b*T0t		b*T1t

	shufps		xmm0,		xmm0,	2*64+2*16+0*4+0		// xmm0 =	c			c			a			a

	mulps		xmm0,		xmm2						// xmm0 =	c*T1		c*T0		a*T1t		a*T0t

	xorps		xmm0,		xmm7						// xmm0 =	-c*T1t		c*T0		-a*T1t		a*T0t

	addps		xmm0,		xmm3
	movaps		[ebx],		xmm0

	add			esi,		32

	sub			ebx,		8*4
	sub			eax,		8*4

	cmp			ebx,		ecx

	jae			loopg

	pop			ebx
}
/*
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
*/
}

/* N point first stage butterfly (in place, 2 register) */
STIN void mdct_butterfly_first(DATA_TYPE *T,
					DATA_TYPE *x,
					int points){

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
}

/* N/stage point generic N stage butterfly (sse) */
STIN void mdct_butterfly_generic_sse(DATA_TYPE *T,
					  DATA_TYPE *x,
					  int points,
					  int trigint){

  DATA_TYPE *x1        = x          + points      - 8;
  DATA_TYPE *x2        = x          + (points>>1) - 8;

_asm
{
	push		ebx
	mov			eax,		x1
	mov			ebx,		x2
	mov			esi,		T
	mov			edx,		trigint
	shl			edx,		2
	mov			ecx,		x
	mov			edi,		edx
	add			edi,		edx							// trigint * 2

loopg:
	movaps		xmm4,		[eax]						// b xmm4 =	x1_3		x1_2		x1_1		x1_0
	movaps		xmm0,		[eax + 16]					// a xmm0 =	x1_7		x1_6		x1_5		x1_4
	movaps		xmm5,		[ebx]						// b xmm5 =	x2_3		x2_2		x2_1		x2_0
	movaps		xmm1,		[ebx + 16]					// a xmm1 =	x2_7		x2_6		x2_5		x2_4

	movaps		xmm2,		xmm0						// a xmm2 =	x1_7		x1_6		x1_5		x1_4
	movaps		xmm6,		xmm4						// b xmm6 =	x1_3		x1_2		x1_1		x1_0
	subps		xmm0,		xmm1						// a xmm0 =	d			c			b			a
	subps		xmm4,		xmm5						// b xmm4 =	d			c			b			a
	addps		xmm6,		xmm5						// b
	addps		xmm2,		xmm1						// a
	movaps		[eax],		xmm6						// b store
	movaps		[eax + 16],	xmm2						// a store

	movhps		xmm2,		[esi]						// a xmm2 =	T1			T0			*			*
	movhps		xmm6,		[esi + edi]					// b xmm6 =	T1			T0			*			*
	movlps		xmm2,		[esi + edx]					// a xmm2 =	T1			T0			T1t			T0t
	add			esi,		edx
	movlps		xmm6,		[esi + edi]					// b xmm6 =	T1			T0			T1t			T0t

	movaps		xmm3,		xmm2						// a xmm3 =	T1			T0			T1t			T0t
	movaps		xmm7,		xmm6						// b xmm7 =	T1			T0			T1t			T0t
	shufps		xmm3,		xmm3,	2*64+3*16+0*4+1		// a xmm3 =	T0			T1			T0t			T1t
	shufps		xmm7,		xmm7,	2*64+3*16+0*4+1		// b xmm7 =	T0			T1			T0t			T1t

	movaps		xmm1,		xmm0						// a xmm1 =	d			c			b			a
	movaps		xmm5,		xmm4						// b xmm5 =	d			c			b			a
	shufps		xmm1,		xmm1,	3*64+3*16+1*4+1		// a xmm1 =	d			d			b			b
	shufps		xmm5,		xmm5,	3*64+3*16+1*4+1		// b xmm5 =	d			d			b			b

	mulps		xmm3,		xmm1						// a xmm3 =	d*T0		d*T1		b*T0t		b*T1t
	mulps		xmm7,		xmm5						// b xmm7 =	d*T0		d*T1		b*T0t		b*T1t

	shufps		xmm4,		xmm4,	2*64+2*16+0*4+0		// b xmm4 =	c			c			a			a
	shufps		xmm0,		xmm0,	2*64+2*16+0*4+0		// a xmm0 =	c			c			a			a

	mulps		xmm4,		xmm6						// b xmm4 =	c*T1		c*T0		a*T1t		a*T0t
	mulps		xmm0,		xmm2						// a xmm0 =	c*T1		c*T0		a*T1t		a*T0t

	xorps		xmm4,		neg_1_3						// b xmm4 =	-c*T1t		c*T0		-a*T1t		a*T0t
	xorps		xmm0,		neg_1_3						// a xmm0 =	-c*T1t		c*T0		-a*T1t		a*T0t

	addps		xmm4,		xmm7						// b
	addps		xmm0,		xmm3						// a
	movaps		[ebx],		xmm4						// b
	movaps		[ebx + 16],	xmm0						// a

	add			esi,		edi
	add			esi,		edx

	sub			ebx,		8*4
	sub			eax,		8*4

	cmp			ebx,		ecx

	jae			loopg
	pop			ebx
}

/*
  do{
	       a      = x1[4]      -  x2[4];
	       b      = x1[5]      -  x2[5];
           c      = x1[6]      -  x2[6];
	       d      = x1[7]      -  x2[7];
	       x1[4]  += x2[4];
	       x1[5]  += x2[5];
	       x1[6]  += x2[6];
	       x1[7]  += x2[7];
	       x2[4]   = MULT_NORM(b * T[1 + trigint]  +  a * T[0 + trigint]);
	       x2[5]   = MULT_NORM(b * T[0 + trigint]  -  a * T[1 + trigint]);
	       x2[6]   = MULT_NORM(d * T[1]            +  c * T[0]);
	       x2[7]   = MULT_NORM(d * T[0]            -  c * T[1]);
	       T+=trigint * 2;
	       a      = x1[0]      -  x2[0];
	       b      = x1[1]      -  x2[1];
	       c      = x1[2]      -  x2[2];
	       d      = x1[3]      -  x2[3];
	       x1[0]  += x2[0];
	       x1[1]  += x2[1];
	       x1[2]  += x2[2];
	       x1[3]  += x2[3];
	       x2[0]   = MULT_NORM(b * T[1 + trigint]  +  a * T[0 + trigint]);
	       x2[1]   = MULT_NORM(b * T[0 + trigint]  -  a * T[1 + trigint]);
	       x2[2]   = MULT_NORM(d * T[1]            +  c * T[0]);
	       x2[3]   = MULT_NORM(d * T[0]            -  c * T[1]);
	       T+=trigint * 2;
	x1-=8;
    x2-=8;
  }while(x2>=x);
*/
}
/* N/stage point generic N stage butterfly (3dn) */
STIN void mdct_butterfly_generic_3dn(DATA_TYPE *T,
					  DATA_TYPE *x,
					  int points,
					  int trigint){
  DATA_TYPE *x1        = x          + points      - 8;
  DATA_TYPE *x2        = x          + (points>>1) - 8;
//  REG_TYPE   r0;
//  REG_TYPE   r1;
  _asm
  {
		mov			eax,				x1					// eax = x1
		mov			ecx,				x2					// ecx = x2
		mov			edi,				x
		mov			edx,				T
		mov			esi,				trigint
		shl			esi,				2
  }
loop_bg3:
/*
  do{
*/
	 _asm
	{
		movq		mm0,				[edx]				// mm0 =	T[1]		T[0]
		movq		mm1,				mm0
		punpckldq	mm2,				mm1
		punpckhdq	mm1,				mm2					// mm1 =	T[0]		T[1]
		pxor		mm1,				neg_q_0				// mm1 =	-T[0]		T[1]

		movq		mm2,				[eax +6*4]			// mm2 =	x1[7]		x1[6]
		movq		mm3,				[ecx +6*4]			// mm3 =	x2[7]		x2[6]
		movq		mm4,				mm2
		pfsub		mm2,				mm3					// mm2 =	r1			r0
		pfadd		mm4,				mm3					// mm4 =	x1[7]+x2[7]	x1[6]+x2[6]
		movq		[eax +6*4],			mm4					// x1[7], x2[6] store

		pfmul		mm0,				mm2					// mm0 =	r1*T[1]		r0*T[0]
		add			edx,				esi					// T += trigint
		pfmul		mm1,				mm2					// mm1 =	r1*T[0]		-r0*T[1]
		pfacc		mm0,				mm1					// mm0 =	r1*T1+r0*T0	r1*T0-r0*T1
		movq		[ecx +6*4],			mm0					// x2[7], x2[6] store

	}
/*
           r0      = x1[6]      -  x2[6];
	       r1      = x1[7]      -  x2[7];
	       x1[6]  += x2[6];
	       x1[7]  += x2[7];
	       x2[6]   = MULT_NORM(r1 * T[1]  +  r0 * T[0]);
	       x2[7]   = MULT_NORM(r1 * T[0]  -  r0 * T[1]);

	       T+=trigint;
*/
	_asm
	{
		movq		mm0,				[edx]				// mm0 =	T[1]		T[0]
		movq		mm1,				mm0
		punpckldq	mm2,				mm1
		punpckhdq	mm1,				mm2					// mm1 =	T[0]		T[1]
		pxor		mm1,				neg_q_0				// mm1 =	-T[0]		T[1]

		movq		mm2,				[eax +4*4]			// mm2 =	x1[5]		x1[4]
		movq		mm3,				[ecx +4*4]			// mm3 =	x2[5]		x2[4]
		movq		mm4,				mm2
		pfsub		mm2,				mm3					// mm2 =	r1			r0
		pfadd		mm4,				mm3					// mm4 =	x1[5]+x2[5]	x1[4]+x2[4]
		movq		[eax +4*4],			mm4					// x1[5], x2[4] store

		pfmul		mm0,				mm2					// mm0 =	r1*T[1]		r0*T[0]
		add			edx,				esi					// T += trigint
		pfmul		mm1,				mm2					// mm1 =	r1*T[0]		-r0*T[1]
		pfacc		mm0,				mm1					// mm0 =	r1*T1+r0*T0	r1*T0-r0*T1
		movq		[ecx +4*4],			mm0					// x2[5], x2[4] store

	}
/*
	       r0      = x1[4]      -  x2[4];
	       r1      = x1[5]      -  x2[5];
	       x1[4]  += x2[4];
	       x1[5]  += x2[5];
	       x2[4]   = MULT_NORM(r1 * T[1]  +  r0 * T[0]);
	       x2[5]   = MULT_NORM(r1 * T[0]  -  r0 * T[1]);

	       T+=trigint;
*/
	_asm
	{
		movq		mm0,				[edx]				// mm0 =	T[1]		T[0]
		movq		mm1,				mm0
		punpckldq	mm2,				mm1
		punpckhdq	mm1,				mm2					// mm1 =	T[0]		T[1]
		pxor		mm1,				neg_q_0				// mm1 =	-T[0]		T[1]

		movq		mm2,				[eax +2*4]			// mm2 =	x1[3]		x1[2]
		movq		mm3,				[ecx +2*4]			// mm3 =	x2[3]		x2[2]
		movq		mm4,				mm2
		pfsub		mm2,				mm3					// mm2 =	r1			r0
		pfadd		mm4,				mm3					// mm4 =	x1[3]+x2[3]	x1[2]+x2[2]
		movq		[eax +2*4],			mm4					// x1[3], x2[2] store

		pfmul		mm0,				mm2					// mm0 =	r1*T[1]		r0*T[0]
		add			edx,				esi					// T += trigint
		pfmul		mm1,				mm2					// mm1 =	r1*T[0]		-r0*T[1]
		pfacc		mm0,				mm1					// mm0 =	r1*T1+r0*T0	r1*T0-r0*T1
		movq		[ecx +2*4],			mm0					// x2[3], x2[2] store

	}
/*
	       r0      = x1[2]      -  x2[2];
	       r1      = x1[3]      -  x2[3];
	       x1[2]  += x2[2];
	       x1[3]  += x2[3];
	       x2[2]   = MULT_NORM(r1 * T[1]  +  r0 * T[0]);
	       x2[3]   = MULT_NORM(r1 * T[0]  -  r0 * T[1]);

	       T+=trigint;
*/
	_asm
	{
		movq		mm0,				[edx]				// mm0 =	T[1]		T[0]
		movq		mm1,				mm0
		punpckldq	mm2,				mm1
		punpckhdq	mm1,				mm2					// mm1 =	T[0]		T[1]
		pxor		mm1,				neg_q_0				// mm1 =	-T[0]		T[1]

		movq		mm2,				[eax +0*4]			// mm2 =	x1[1]		x1[0]
		movq		mm3,				[ecx +0*4]			// mm3 =	x2[1]		x2[0]
		movq		mm4,				mm2
		pfsub		mm2,				mm3					// mm2 =	r1			r0
		pfadd		mm4,				mm3					// mm4 =	x1[1]+x2[1]	x1[0]+x2[0]
		movq		[eax +0*4],			mm4					// x1[1], x2[0] store

		pfmul		mm0,				mm2					// mm0 =	r1*T[1]		r0*T[0]
		add			edx,				esi					// T += trigint
		pfmul		mm1,				mm2					// mm1 =	r1*T[0]		-r0*T[1]
		pfacc		mm0,				mm1					// mm0 =	r1*T1+r0*T0	r1*T0-r0*T1
		movq		[ecx +0*4],			mm0					// x2[1], x2[0] store
	}

/*
	       r0      = x1[0]      -  x2[0];
	       r1      = x1[1]      -  x2[1];
	       x1[0]  += x2[0];
	       x1[1]  += x2[1];
	       x2[0]   = MULT_NORM(r1 * T[1]  +  r0 * T[0]);
	       x2[1]   = MULT_NORM(r1 * T[0]  -  r0 * T[1]);

	       T+=trigint;
*/
	_asm
	{
		sub			eax,				8*4
		sub			ecx,				8*4

		cmp			ecx,				edi

		jae			loop_bg3
		femms
	}
/*
    x1-=8;
    x2-=8;

  }while(x2>=x);
*/
}
STIN void mdct_butterfly_generic(DATA_TYPE *T,
					  DATA_TYPE *x,
					  int points,
					  int trigint){
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
}

STIN void mdct_butterflies(mdct_lookup *init,
			     DATA_TYPE *x,
			     int points){

  DATA_TYPE *T=init->trig;
  int stages=init->log2n-5;
  int i,j;

  if(--stages>0){
	if(CPU_SSE)
		mdct_butterfly_first_sse(T,x,points);
	else
		mdct_butterfly_first(T,x,points);
  }

  	if(CPU_SSE)
	{
	  for(i=1;--stages>0;i++){
		for(j=0;j<(1<<i);j++)
		  mdct_butterfly_generic_sse(T,x+(points>>i)*j,points>>i,4<<i);
	  }
	}
	else if(CPU_3DN)
	{
	  for(i=1;--stages>0;i++){
		for(j=0;j<(1<<i);j++)
		  mdct_butterfly_generic_3dn(T,x+(points>>i)*j,points>>i,4<<i);
	  }
	}
	else
	{
	  for(i=1;--stages>0;i++){
		for(j=0;j<(1<<i);j++)
		  mdct_butterfly_generic(T,x+(points>>i)*j,points>>i,4<<i);
	  }
	}

	if(CPU_SSE)
	{
	  for(j=0;j<points;j+=32)
		mdct_butterfly_32_sse(x+j);
	}
	else if(CPU_3DN)
	{
	  for(j=0;j<points;j+=32)
		mdct_butterfly_32_3dn(x+j);
	}
	else
	{
	  for(j=0;j<points;j+=32)
		mdct_butterfly_32(x+j);
	}
}

void mdct_clear(mdct_lookup *l){
  if(l){
    if(l->trig)_ogg_free(l->trig);
    if(l->bitrev)_ogg_free(l->bitrev);
    memset(l,0,sizeof(*l));
  }
}

STIN void mdct_bitreverse_sse(mdct_lookup *init,
			    DATA_TYPE *x){
  int        n       = init->n;
  int       *bit     = init->bitrev;
  DATA_TYPE *w0      = x;
  DATA_TYPE *w1      = x = w0+(n>>1);
  DATA_TYPE *T       = init->trig+n;

_asm
{
	push		ebx
	mov			eax,		x
	mov			ebx,		bit
	mov			ecx,		w0
	mov			edx,		w1
	mov			esi,		T

loopf:
	sub			edx,		4*4							// w1 -= 4

	mov			edi,		[ebx + 0]					// edi = x0
	movlps		xmm0,		[edi*4 + eax]				// xmm0 =	*			*			x0[1]		x0[0]
	mov			edi,		[ebx + 4]					// edi = x1
	movlps		xmm1,		[edi*4 + eax]				// xmm1 =	*			*			x1[1]		x1[0]
	mov			edi,		[ebx + 8]					// edi = x2
	movhps		xmm0,		[edi*4 + eax]				// xmm0 =	x2[1]		x2[0]		x0[1]		x0[0]
	mov			edi,		[ebx + 12]					// edi = x3
	movhps		xmm1,		[edi*4 + eax]				// xmm1 =	x3[1]		x3[0]		x1[1]		x1[0]

	movaps		xmm2,		xmm1						// xmm2 =	x3[1]		x3[0]		x1[1]		x1[0]
	movaps		xmm3,		[esi]						// xmm3 =	T[3]		T[2]		T[1]		T[0]
	xorps		xmm2,		neg_1_3						// xmm2 =	-x3[1]		x3[0]		-x1[1]		x1[0]
	movaps		xmm4,		xmm3						// xmm4 =	T[3]		T[2]		T[1]		T[0]
	addps		xmm2,		xmm0						// xmm2 =	r4			r5			r0			r1
	shufps		xmm4,		xmm4,	2*64+3*16+0*4+1		// xmm4 =	T[2]		T[3]		T[0]		T[1]
	movaps		xmm5,		xmm2						// xmm5 =	r4			r5			r0			r1
	xorps		xmm4,		neg_1_3						// xmm4 =	-T[2]		T[3]		-T[0]		T[1]
	shufps		xmm5,		xmm5,	2*64+2*16+0*4+0		// xmm5 =	r5			r5			r1			r1
//	prefetcht0	[esi + 32]
	mulps		xmm3,		xmm5						// xmm3 =	r5*T[3]		r5*T[2]		r1*T[1]		r1*T[0]
	shufps		xmm2,		xmm2,	3*64+3*16+1*4+1		// xmm2 =	r4			r4			r0			r0
	xorps		xmm1,		neg_1_3						// xmm1 =	-x3[1]		x3[0]		-x1[1]		x1[0]
	mulps		xmm2,		xmm4						// xmm2 =	-r4*T[2]	r4*T[3]		-r0*T[0]	r0*T[1]
	subps		xmm0,		xmm1						// xmm0 =	x2[1]+x3[1]	x2[0]-x3[0]	x0[1]+x1[1]	x0[0]-x1[0]
	addps		xmm2,		xmm3						// xmm2 =	r7			r6			r3			r2
	mulps		xmm0,		packed_halve				// xmm0 =	r4			r5			r0			r1
	add			ecx,		4*4							// w0 += 4

	movaps		xmm6,		xmm0						// xmm6 =	r4			r5			r0			r1
	add			esi,		4*4							// T += 4
	shufps		xmm6,		xmm6,	2*64+3*16+0*4+1		// xmm6 =	r5			r4			r1			r0
	add			ebx,		4*4							// bit += 4
	addps		xmm6,		xmm2						// xmm6 =	r5+r7		r4+r6		r1+r3		r0+r2
	movaps		[ecx - 4*4],	xmm6					// store w0

	shufps		xmm2,		xmm2,	1*64+0*16+3*4+2		// xmm2 =	r3			r2			r7			r6
	shufps		xmm0,		xmm0,	0*64+1*16+2*4+3		// xmm0 =	r1			r0			r5			r4
	subps		xmm2,		xmm0						// xmm2 =	r3-r1		r2-r0		r7-r5		r6-r4
	cmp			ecx,		edx
	xorps		xmm2,		neg_0_2						// xmm2 =	r3-r1		-r2+r0		r7-r5		-r6+r4
	movaps		[edx],		xmm2						// store w1

	jb			loopf
	pop			ebx
}

/*

  DATA_TYPE *x0, *x1, *x2, *x3;
  REG_TYPE r0, r1, r2, r3;
  REG_TYPE r4, r5, r6, r7;

  do{

w1    -= 4;


x0     = x+bit[0];
x1     = x+bit[1];
x2     = x+bit[2];
x3     = x+bit[3];

r1     = x0[0]  + x1[0];
r0     = x0[1]  - x1[1];
r5     = x2[0]  + x3[0];
r4     = x2[1]  - x3[1];

r2     = MULT_NORM(r1     * T[0]   + r0 * T[1]);
r3     = MULT_NORM(r1     * T[1]   - r0 * T[0]);
r6     = MULT_NORM(r5     * T[2]   + r4 * T[3]);
r7     = MULT_NORM(r5     * T[3]   - r4 * T[2]);

r1     = HALVE(x0[0] - x1[0]);
r0     = HALVE(x0[1] + x1[1]);
r5     = HALVE(x2[0] - x3[0]);
r4     = HALVE(x2[1] + x3[1]);

w0[0]  =   r0     + r2;
w0[1]  =   r1     + r3;
w0[2]  =   r4     + r6;
w0[3]  =   r5     + r7;

w1[0]  = -(r6     - r4);
w1[1]  =  (r7     - r5);
w1[2]  = -(r2     - r0);
w1[3]  =  (r3     - r1);

T     += 4;
bit   += 4;
w0    += 4;

  }while(w0<w1);

*/
}


STIN void mdct_bitreverse_3dn(mdct_lookup *init,
			    DATA_TYPE *x){
  int        n       = init->n;
  int       *bit     = init->bitrev;
  DATA_TYPE *w0      = x;
  DATA_TYPE *w1      = x = w0+(n>>1);
  DATA_TYPE *T       = init->trig+n;

	_asm
	{
		push		ebx
		mov			ecx,			bit
		mov			esi,			w0
		mov			edi,			w1
		mov			edx,			T
	}
loopbr3:
/*
  do{
*/

	_asm
	{
		mov			eax,			[ecx + 0*4]
		shl			eax,			2
		add			eax,			x									// eax=	x0
		mov			ebx,			[ecx + 1*4]
		shl			ebx,			2
		add			ebx,			x									// ebx=	x1
	}
/*

    DATA_TYPE *x0    = x+bit[0];
    DATA_TYPE *x1    = x+bit[1];
*/

	_asm
	{
		movq		mm0,				[edx]				// a mm0 =	T[1]		T[0]
		movq		mm4,				[eax]				// b mm4 =	x0[1]		x0[0]
		movq		mm1,				mm0					// a 
		movq		mm5,				[ebx]				// b mm5 =	x1[1]		x1[0]
		punpckldq	mm2,				mm1					// a 
		pxor		mm5,				neg_q_0				// b mm5 =	x1[1]		-x1[0]
		punpckhdq	mm1,				mm2					// a mm1 =	T[0]		T[1]
		pfadd		mm4,				mm5					// b mm4 =	x0[1]+x1[1]	x0[0]-x1[0]
		pxor		mm1,				neg_q_1				// a mm1 =	-T[0]		T[1]

		movq		mm2,				[eax]				// a mm2 =	x0[1]		x0[0]
		movq		mm3,				[ebx]				// a mm3 =	x1[1]		x1[0]
		pfmul		mm4,				packed_halve_q		// b
		pxor		mm3,				neg_q_1				// a mm3 =	-x1[1]		x1[0]
		sub			edi,				4*4
		pfadd		mm2,				mm3					// a mm2 =	x0[1]-x1[1]	x0[0]+x1[0]
															// a mm2 =	r0			r1		(swapped)

		pfmul		mm1,				mm2					// a mm1 =	-r0*T[0]	r1*T[1]
		pfmul		mm0,				mm2					// a mm0 =	r0*T[1]		r1*T[0]
		punpckldq	mm5,				mm4					// b
		pfacc		mm0,				mm1					// a mm0 =	r1*T0+r0*T1	r1*T1-r0*T0
															// a mm0 =	r3,			r2

		punpckhdq	mm4,				mm5					// b mm4 =	x0[0]-x1[0]	x0[1]+x1[1]
															// b mm4 =	r1			r0
	}

/*
    REG_TYPE  r0     = x0[1]  - x1[1];
    REG_TYPE  r1     = x0[0]  + x1[0];
    REG_TYPE  r2     = MULT_NORM(r1     * T[0]   + r0 * T[1]);
    REG_TYPE  r3     = MULT_NORM(r1     * T[1]   - r0 * T[0]);
              r0     = HALVE(x0[1] + x1[1]);
              r1     = HALVE(x0[0] - x1[0]);
	      w1    -= 4;
*/
	_asm
	{
		add			esi,				4*4
		movq		mm1,				mm4
		mov			eax,			[ecx + 2*4]
		pfadd		mm1,				mm0					// mm1 =	r0+r2		r1+r3
		shl			eax,			2
		movq		[esi-4*4],		mm1					// w0[1],w0[0]  store
		add			eax,			x									// eax=	x0
		pxor		mm4,				neg_q_1				// mm4 =	-r1			r0
		mov			ebx,			[ecx + 3*4]
		pxor		mm0,				neg_q_0				// mm0 =	r3			-r2
		shl			ebx,			2
		pfadd		mm0,				mm4					// mm4 =	-r1+r3		r0-r2
		movq		[edi+2*4],			mm0					// w1[3],w1[2] store
		add			ebx,			x									// ebx=	x1
	}
/*

	      w0[0]  = r0     + r2;
	      w0[1]  = r1     + r3;
	      w1[2]  = r0     - r2;
	      w1[3]  = -r1    + r3;
              x0     = x+bit[2];
              x1     = x+bit[3];
*/
	_asm
	{
		movq		mm0,				[edx+4*2]			// a mm0 =	T[3]		T[2]
		movq		mm4,				[eax]				// b mm4 =	x0[1]		x0[0]
		movq		mm1,				mm0					// a
		movq		mm5,				[ebx]				// b mm5 =	x1[1]		x1[0]
		punpckldq	mm2,				mm1					// a
		pxor		mm5,				neg_q_0				// b mm5 =	x1[1]		-x1[0]
		punpckhdq	mm1,				mm2					// a mm1 =	T[2]		T[3]
		pfadd		mm4,				mm5					// b mm4 =	x0[1]+x1[1]	x0[0]-x1[0]
		pxor		mm1,				neg_q_1				// a mm1 =	-T[2]		T[3]

		movq		mm2,				[eax]				// a mm2 =	x0[1]		x0[0]
		movq		mm3,				[ebx]				// a mm3 =	x1[1]		x1[0]
		pfmul		mm4,				packed_halve_q		// b
		pxor		mm3,				neg_q_1				// a mm3 =	-x1[1]		x1[0]
		punpckldq	mm5,				mm4					// b
		pfadd		mm2,				mm3					// a mm2 =	x0[1]-x1[1]	x0[0]+x1[0]
															// a mm2 =	r0			r1		(swapped)
		pfmul		mm1,				mm2					// a mm1 =	-r0*T[2]		r1*T[3]
		punpckhdq	mm4,				mm5					// b mm4 =	x0[0]-x1[0]	x0[1]+x1[1]
		pfmul		mm0,				mm2					// a mm0 =	r0*T[3]		r1*T[2]
		pfacc		mm0,				mm1					// a mm0 =	r1*T2+r0*T3	r1*T3-r0*T2
															// a mm0 =	r3,			r2
															// b mm4 =	r1			r0
	}
/*
              r0     = x0[1]  - x1[1];
              r1     = x0[0]  + x1[0];
              r2     = MULT_NORM(r1     * T[2]   + r0 * T[3]);
              r3     = MULT_NORM(r1     * T[3]   - r0 * T[2]);
              r0     = HALVE(x0[1] + x1[1]);
              r1     = HALVE(x0[0] - x1[0]);
*/
	_asm
	{
	}

/*
*/
	_asm
	{
		movq		mm6,				mm4
		add			edx,				4*4
		pfadd		mm6,				mm0					// mm6 =	r0+r2		r1+r3
		movq		[esi+2*4-4*4],		mm6					// w0[3],w0[2]  store
		pxor		mm4,				neg_q_1				// mm4 =	-r1			r0
		pxor		mm0,				neg_q_0				// mm0 =	r3			-r2
		add			ecx,				4*4
		pfadd		mm0,				mm4					// mm4 =	-r1+r3		r0-r2
		movq		[edi],				mm0					// w1[1],w1[0] store
	}
/*
	      w0[2]  = r0     + r2;
	      w0[3]  = r1     + r3;
	      w1[0]  = r0     - r2;
	      w1[1]  =-r1     + r3;
	      T     += 4;
	      bit   += 4;
	      w0    += 4;
*/

	_asm
	{
		cmp		esi,	edi
		jb		loopbr3
	}
/*

  }while(w0<w1);
*/

	_asm
	{
		pop		ebx
		femms
	}

}

STIN void mdct_bitreverse(mdct_lookup *init,
			    DATA_TYPE *x){
  int        n       = init->n;
  int       *bit     = init->bitrev;
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
}

void mdct_backward_sse(mdct_lookup *init, DATA_TYPE *_in, DATA_TYPE *_out){
  int n=init->n;
  int n2=n>>1;
  int n4=n>>2;


  /* rotate */

  DATA_TYPE *iX = _in+n2-7;
  DATA_TYPE *oX = _out+n2+n4;
  DATA_TYPE *T  = init->trig+n4;

_asm
{
	mov			edi,	oX
	mov			esi,	iX
	mov			edx,	T
	mov			ecx,	_in
	movaps		xmm7,	neg_0_2

loop0:
	sub			edi,	4*4
	movaps		xmm0,	[edx]							// xmm0 =  T3   T2   T1   T0
	movups		xmm1,	[esi]							// xmm1 =  iX3  iX2  iX1  iX0
	movups		xmm2,	[esi + 4*4]						// xmm2 =  iX7  iX6  iX5  iX4

	prefetcht0	[esi - 64]

	movaps		xmm3,	xmm0							// xmm3 =  T3   T2   T1   T0
	movaps		xmm4,	xmm1							// xmm4 =  iX3  iX2  iX1  iX0
	shufps		xmm3,	xmm0,	1*64+1*16+3*4+3			// xmm3 =  T1   T1   T3   T3
	shufps		xmm4,	xmm2,	0*64+2*16+0*4+2			// xmm4 =  iX4  iX6  iX0  iX2


	movaps		xmm1,	xmm4							// xmm1 =  iX4  iX6  iX0  iX2
	xorps		xmm4,	xmm7							// xmm4 =  iX4 -iX6  iX0 -iX2

	sub			esi,	8*4

	mulps		xmm4,	xmm3							// xmm4 = tmp0

	prefetcht0	[edx + 64]

	shufps		xmm0,	xmm0,	0*64+0*16+2*4+2			// xmm0 =  T0   T0   T2   T2

	add			edx,	4*4

	shufps		xmm1,	xmm1,	2*64+3*16+0*4+1			// xmm1 =  iX6  iX4  iX2  iX0
	mulps		xmm1,	xmm0							// xmm0 = tmp1

	cmp			esi,	ecx

	subps		xmm4,	xmm1							// xmm4 = tmp0 - tmp1
	movaps		[edi],	xmm4							// oX = xmm4


	jae			loop0

}

/*
  do{
    oX         -= 4;
    oX[0]   = MULT_NORM(-iX[2] * T[3] - iX[0]  * T[2]);
    oX[1]   = MULT_NORM (iX[0] * T[3] - iX[2]  * T[2]);
    oX[2]   = MULT_NORM(-iX[6] * T[1] - iX[4]  * T[0]);
    oX[3]   = MULT_NORM (iX[4] * T[1] - iX[6]  * T[0]);
    iX         -= 8;
    T          += 4;
  }while(iX>=_in);
*/

  iX            = _in+n2-8;
  oX            = _out+n2+n4;
  T             = init->trig+n4;

_asm
{
	mov			edi,	oX
	mov			esi,	iX
	mov			edx,	T
	mov			ecx,	_in
	movaps		xmm7,	neg_0_2
	sub			edx,	4*4

loop1:
	movaps		xmm0,	[edx]							// xmm0 =  T3   T2   T1   T0
	movaps		xmm1,	[esi]							// xmm1 =  iX3  iX2  iX1  iX0
	movaps		xmm2,	[esi + 4*4]						// xmm2 =  iX7  iX6  iX5  iX4

	movaps		xmm3,	xmm0							// xmm3 =  T3   T2   T1   T0

	prefetcht0	[edx - 64]

	shufps		xmm3,	xmm3,	0*64+1*16+2*4+3			// xmm3 =  T0   T1   T2   T3

	add			edi,	4*4

	movaps		xmm4,	xmm2							// xmm4 =  iX7  iX6  iX5  iX4

	prefetcht0	[esi - 64]

	shufps		xmm4,	xmm1,	0*64+0*16+0*4+0			// xmm4 =  iX0  iX0  iX4  iX4

	sub			esi,	8*4

	mulps		xmm4,	xmm3							// xmm4 = tmp0

	shufps		xmm2,	xmm1,	2*64+2*16+2*4+2			// xmm2 =  iX2  iX2  iX6  iX6

	shufps		xmm0,	xmm0,	1*64+0*16+3*4+2			// xmm0 =  T1   T0   T3   T2

	sub			edx,	4*4

	xorps		xmm0,	xmm7							// xmm0 = -T1   T0  -T3   T2

	mulps		xmm0,	xmm2							// xmm0 = tmp1

	cmp			esi,	ecx

	subps		xmm4,	xmm0							// xmm4 = tmp0 - tmp1
	movaps		[edi - 4*4],	xmm4					// oX = xmm4

	jae			loop1
}

/*
  do{
    T          -= 4;
    oX[0]       =  MULT_NORM (iX[4] * T[3] + iX[6] * T[2]);
    oX[1]       =  MULT_NORM (iX[4] * T[2] - iX[6] * T[3]);
    oX[2]       =  MULT_NORM (iX[0] * T[1] + iX[2] * T[0]);
    oX[3]       =  MULT_NORM (iX[0] * T[0] - iX[2] * T[1]);
    iX         -= 8;
    oX         += 4;
  }while(iX>=_in);
*/

  mdct_butterflies(init,_out+n2,n2);
  mdct_bitreverse_sse(init,_out);

  /* roatate + window */

  {
    DATA_TYPE *oX1=_out+n2+n4;
    DATA_TYPE *oX2=_out+n2+n4;
    DATA_TYPE *iX =_out;
    T             =init->trig+n2;

_asm
{
	push		ebx
	mov			edi,	oX2
	mov			edx,	oX1
	mov			esi,	iX
	mov			ebx,	T

loop3:
	sub			edx,	4*4

	movaps		xmm0,	[ebx]							// xmm0 =	T3	T2	T1	T0
	movaps		xmm1,	[ebx + 16]						// xmm1 =	T7	T6	T5	T4
	movaps		xmm2,	[esi]							// xmm2 =	iX3	iX2	iX1	iX0
	movaps		xmm3,	[esi + 16]						// xmm3 =	iX7	iX6	iX5	iX4


	movaps		xmm4,	xmm1							// xmm4 =	T7	T6	T5	T4
	movaps		xmm6,	xmm1							// xmm6 =	T7	T6	T5	T4
//	prefetcht0	[ebx + 64]
	shufps		xmm4,	xmm0,	1*64+3*16+1*4+3			// xmm4 =	T1	T3	T5	T7
	shufps		xmm6,	xmm0,	0*64+2*16+0*4+2			// xmm6 =	T0	T2	T4	T6

	movaps		xmm5,	xmm3							// xmm5 =	iX7	iX6	iX5	iX4
	movaps		xmm7,	xmm3							// xmm7 =	iX7	iX6	iX5	iX4
	shufps		xmm5,	xmm2,	0*64+2*16+0*4+2			// xmm5 =	iX0	iX2	iX4	iX6
	shufps		xmm7,	xmm2,	1*64+3*16+1*4+3			// xmm7 =	iX1	iX3	iX5	iX7

//	prefetcht0	[esi + 64]
	mulps		xmm4,	xmm5							// xmm4 = tmp0


	mulps		xmm6,	xmm7							// xmm6 = tmp1
	add			edi,	4*4
	movaps		xmm7,	xmm0							// xmm7 =	T3	T2	T1	T0

	subps		xmm4,	xmm6							// xmm4 = tmp0 - tmp1

	shufps		xmm7,	xmm1,	2*64+0*16+2*4+0			// xmm7 =	T6	T4	T2	T0

	movaps		xmm5,	xmm2							// xmm5 =	iX3	iX2	iX1	iX0
	movaps		[edx],	xmm4

	shufps		xmm5,	xmm3,	2*64+0*16+2*4+0			// xmm5 =	iX0	iX2	iX4	iX6

	mulps		xmm7,	xmm5							// xmm7 = tmp2
	add			esi,	8*4

	movaps		xmm5,	xmm0							// xmm5 =	T3	T2	T1	T0
	movaps		xmm6,	xmm2							// xmm6 =	iX3	iX2	iX1	iX0
	shufps		xmm5,	xmm1,	3*64+1*16+3*4+1			// xmm5 =	T0	T2	T4	T6

	add			ebx,	8*4
	shufps		xmm6,	xmm3,	3*64+1*16+3*4+1			// xmm6 =	iX1	iX3	iX5	iX7

	cmp			esi,	edx

	mulps		xmm5,	xmm6							// xmm5 = tmp3

	addps		xmm7,	xmm5							// xmm7 = tmp2 + tmp3

	xorps		xmm7,	neg_0_1_2_3						// negate xmm7

	movaps		[edi - 4*4],	xmm7

	jb			loop3
	pop			ebx
}
/*
	do{
      oX1-=4;

      oX1[0]  =  MULT_NORM (iX[6] * T[7] - iX[7] * T[6]);
      oX1[1]  =  MULT_NORM (iX[4] * T[5] - iX[5] * T[4]);
      oX1[2]  =  MULT_NORM (iX[2] * T[3] - iX[3] * T[2]);
	  oX1[3]  =  MULT_NORM (iX[0] * T[1] - iX[1] * T[0]);

      oX2[0]  = -MULT_NORM (iX[0] * T[0] + iX[1] * T[1]);
      oX2[1]  = -MULT_NORM (iX[2] * T[2] + iX[3] * T[3]);
      oX2[2]  = -MULT_NORM (iX[4] * T[4] + iX[5] * T[5]);
      oX2[3]  = -MULT_NORM (iX[6] * T[6] + iX[7] * T[7]);

      oX2+=4;
      iX    +=   8;
      T     +=   8;
    }while(iX<oX1);
*/
    iX=_out+n2+n4;
    oX1=_out+n4;
    oX2=oX1;

_asm
{
	mov			edi,	oX2
	mov			esi,	oX1
	mov			edx,	iX
	movaps		xmm7,	neg_0_1_2_3

loop4:
	sub			edx,	4*4
	sub			esi,	4*4
	lea			edi,	[edi + 4*4]
	movaps		xmm0,	[edx]
	movaps		[esi],	xmm0
	prefetcht0	[edx - 32]
	cmp			edi,	edx
	shufps		xmm0,	xmm0,	0*64+1*16+2*4+3
	xorps		xmm0,	xmm7
	movaps		[edi - 4*4],	xmm0

	jb			loop4
}
/*
    do{
      oX1-=4;
      iX-=4;

      oX2[0] = -(oX1[3] = iX[3]);
      oX2[1] = -(oX1[2] = iX[2]);
      oX2[2] = -(oX1[1] = iX[1]);
      oX2[3] = -(oX1[0] = iX[0]);

      oX2+=4;
    }while(oX2<iX);
*/
    iX=_out+n2+n4;
    oX1=_out+n2+n4;
    oX2=_out+n2;
_asm
{
	mov			edi,	oX1
	mov			esi,	iX
	mov			edx,	oX2

loop5:
	sub			edi,	4*4

	movaps		xmm0,	[esi]
	shufps		xmm0,	xmm0,	0*64+1*16+2*4+3
	movaps		[edi],	xmm0

	add			esi,	4*4

	cmp			edi,	edx
	ja			loop5
}
/*
    do{
      oX1-=4;
      oX1[0]= iX[3];
      oX1[1]= iX[2];
      oX1[2]= iX[1];
      oX1[3]= iX[0];
      iX+=4;
    }while(oX1>oX2);
*/
  }
}

void mdct_backward_3dn(mdct_lookup *init, DATA_TYPE *_in, DATA_TYPE *_out){
{
  int n=init->n;
  int n2=n>>1;
  int n4=n>>2;

  /* rotate */

  DATA_TYPE *iX = _in+n2-7;
  DATA_TYPE *oX = _out+n2+n4;
  DATA_TYPE *T  = init->trig+n4;

  _asm
  {
		mov		eax,		iX
		mov		ecx,		oX
		mov		edx,		T
		mov		edi,		_in

loop3dna:
		sub				ecx,			4*4

		movq			mm1,			[edx + 2*4]			// a mm1 =	T[3]			T[2]
		movq			mm5,			[edx + 0*4]			// b mm5 =	T[1]			T[0]
		movd			mm2,			[eax]				// a mm2 =	iX[1]			iX[0]
		movd			mm6,			[eax + 4*4]			// b mm6 =	iX[5]			iX[4]
		movd			mm3,			[eax + 2*4]			// a mm3 =	iX[3]			iX[2]
		movd			mm7,			[eax + 6*4]			// b mm7 =	iX[7]			iX6]
		movq			mm4,			mm2					// a
		movq			mm0,			mm6					// b
		punpckldq		mm2,			mm3					// a mm2 =	iX[2]			iX[0]
		punpckldq		mm6,			mm7					// b mm6 =	iX[6]			iX[4]
		pfmul			mm2,			mm1					// a mm2 =	iX[2]*T[3]		iX[0]*T[2]
  	prefetch	[edx + 32]
		pfmul			mm6,			mm5					// b mm6 =	iX[6]*T[1]		iX[4]*T[0]
		punpckldq		mm3,			mm4					// a mm3 =	iX[0]			iX[2]
		punpckldq		mm7,			mm0					// b mm7 =	iX[4]			iX[6]
		pfmul			mm3,			mm1					// a mm3 =	iX[0]*T[3]		iX[2]*T[2]
  	prefetch	[eax - 32]
		pfmul			mm7,			mm5					// b mm7 =	iX[4]*T[1]		iX[6]*T[0]
		pxor			mm2,			neg_q_0_1			// a mm2 =	-iX[2]*T[3]		-iX[0]*T[2]
		pxor			mm6,			neg_q_0_1			// b mm6 =	-iX[6]*T[1]		-iX[4]*T[0]
		pxor			mm3,			neg_q_0				// a mm3 =	iX[0]*T[3]		-iX[2]*T[2]
		pxor			mm7,			neg_q_0				// b mm7 =	iX[4]*T[1]		-iX[6]*T[0]
		pfacc			mm2,			mm3					// a mm2 =	oX[1]			oX[2]
		pfacc			mm6,			mm7					// b mm6 =	oX[1]			oX[2]
		movq			[ecx+0*4],		mm2					// a store
		movq			[ecx+2*4],		mm6					// b store

		sub				eax,			8*4
		add				edx,			4*4

		cmp				eax,			edi
		jae				loop3dna
		femms
  }
/*


  do{
    oX         -= 4;
    oX[0]       = MULT_NORM(-iX[2] * T[3] - iX[0]  * T[2]);
    oX[1]       = MULT_NORM (iX[0] * T[3] - iX[2]  * T[2]);
    oX[2]       = MULT_NORM(-iX[6] * T[1] - iX[4]  * T[0]);
    oX[3]       = MULT_NORM (iX[4] * T[1] - iX[6]  * T[0]);
    iX         -= 8;
    T          += 4;
  }while(iX>=_in);

*/

  iX            = _in+n2-8;
  oX            = _out+n2+n4;
  T             = init->trig+n4;

	_asm
  	{
		mov		eax,		iX
		mov		ecx,		oX
		mov		edx,		T
		mov		edi,		_in
  	}
loop3dnb:
	_asm
	{

		movq			mm1,			[edx + 2*4 -4*4]	// a mm1 =	T[3]			T[2]
		movq			mm0,			[edx + 0*4 -4*4]	// b mm0 =	T[1]			T[0]
		movq			mm2,			[eax + 4*4]			// a mm2 =	iX[5]			iX[4]
		movq			mm3,			[eax + 0*4]			// b mm3 =	iX[1]			iX[0]

		movq			mm5,			mm1					// a
		movq			mm4,			mm0					// b
		punpckldq		mm6,			mm5					// a
		punpckldq		mm7,			mm4					// b
		punpckhdq		mm5,			mm6					// a mm5 =	T[2]			T[3]
		punpckhdq		mm4,			mm7					// b mm4 =	T[0]			T[1]
		punpckldq		mm2,			[eax + 6*4]			// a mm2 =	iX[6]			iX[4]
		sub				edx,			4*4
		punpckldq		mm3,			[eax + 2*4]			// b mm3 =	iX[2]			iX[0]
		pfmul			mm5,			mm2					// a mm5 =	iX[6]*T[2]		iX[4]*T[3]
		add				ecx,			4*4
		pfmul			mm4,			mm3					// b mm4 =	iX[2]*T[0]		iX[0]*T[1]
		sub				eax,			8*4
		pfmul			mm1,			mm2					// a mm1 =	iX[6]*T[3]		iX[4]*T[2]
		cmp				eax,			edi
		pfmul			mm0,			mm3					// b mm0 =	iX[2]*T[1]		iX[0]*T[0]
		pxor			mm1,			neg_q_1				// a mm1 =	-iX[6]*T[3]		iX[4]*T[2]
		pxor			mm0,			neg_q_1				// b mm0 =	-iX[2]*T[1]		iX[0]*T[0]
		pfacc			mm5,			mm1					// a mm5 =	oX[1]			oX[0]
		pfacc			mm4,			mm0					// b mm4 =	oX[1]			oX[0]
		movq			[ecx+0*4 -4*4],	mm5					// a store
		movq			[ecx+2*4 -4*4],	mm4					// b store


		jae				loop3dnb
		femms
	}

/*
  do{
    T          -= 4;
    oX[0]       =  MULT_NORM (iX[4] * T[3] + iX[6] * T[2]);
    oX[1]       =  MULT_NORM (iX[4] * T[2] - iX[6] * T[3]);
    oX[2]       =  MULT_NORM (iX[0] * T[1] + iX[2] * T[0]);
    oX[3]       =  MULT_NORM (iX[0] * T[0] - iX[2] * T[1]);
    iX         -= 8;
    oX         += 4;
  }while(iX>=_in);
*/



  mdct_butterflies(init,_out+n2,n2);
  mdct_bitreverse_3dn(init,_out);

  /* roatate + window */

  {
    DATA_TYPE *oX1=_out+n2+n4;
    DATA_TYPE *oX2=_out+n2+n4;
    DATA_TYPE *iX =_out;
    T             =init->trig+n2;
	_asm
  	{
  		mov				ecx,			oX1
  		mov				edx,			oX2
  		mov				eax,			iX
  		mov				edi,			T
  	}
loop3dnc:
  	_asm
  	{
  		sub				ecx,			4*4

//      oX1[3]  =  MULT_NORM (iX[0] * T[1] - iX[1] * T[0]);
//      oX2[0]  = -MULT_NORM (iX[0] * T[0] + iX[1] * T[1]);
//      oX1[2]  =  MULT_NORM (iX[2] * T[3] - iX[3] * T[2]);
//      oX2[1]  = -MULT_NORM (iX[2] * T[2] + iX[3] * T[3]);
	  		movq			mm0,			[edi]				// mm0 =	T1				T0
	  		movq			mm2,			[edi+2*4]
	  		movq			mm3,			[eax+2*4]
	  		movq			mm1,			[eax]				// mm1 =	iX1				iX0
	  		movq			mm4,			mm0
	  		movq			mm5,			mm2
	  		punpckldq		mm6,			mm4
	  		punpckldq		mm7,			mm5
	  		punpckhdq		mm4,			mm6					// mm4 =	T0				T1
	  		punpckhdq		mm5,			mm7
	  		pfmul			mm0,			mm1					// mm0 =	iX1*T1			iX0*T0
	  		pfmul			mm2,			mm3
	  		pfmul			mm4,			mm1					// mm4 =	iX1*T0			iX0*T1
	  		pfmul			mm5,			mm3
	  		pxor			mm4,			neg_q_1				// mm4 =	-iX1*T0			iX0*T1
	  		pxor			mm5,			neg_q_1
	  		pfacc			mm4,			mm0					// mm4 =	iX0*T0+iX1*T1	iX0*T1-iX1*T0
	  		pfacc			mm5,			mm2
	  		pxor			mm4,			neg_q_1				// mm4 =	-iX0*T0-iX1*T1	iX0*T1-iX1*T0
	  		pxor			mm5,			neg_q_1
	  															// mm4 =	oX2[0]			oX1[3]

	  															// mm5 =	oX2[1]			oX1[2]


//      oX1[1]  =  MULT_NORM (iX[4] * T[5] - iX[5] * T[4]);
//      oX2[2]  = -MULT_NORM (iX[4] * T[4] + iX[5] * T[5]);
//      oX1[0]  =  MULT_NORM (iX[6] * T[7] - iX[7] * T[6]);
//      oX2[3]  = -MULT_NORM (iX[6] * T[6] + iX[7] * T[7]);
	  		movq			mm0,			[edi+4*4]
	  		movq			mm1,			[eax+4*4]
	  		movq			mm6,			mm0
	  		punpckldq		mm3,			mm6
	  		punpckhdq		mm6,			mm3
	  		pfmul			mm0,			mm1
  			pfmul			mm6,			mm1


			movq			mm1,			[edi+6*4]
	  		movq			mm2,			[eax+6*4]
	  		movq			mm7,			mm1
	  		punpckldq		mm3,			mm7
	  		punpckhdq		mm7,			mm3
	  		pfmul			mm1,			mm2
  			pfmul			mm7,			mm2

  			pxor			mm6,			neg_q_1
  			pxor			mm7,			neg_q_1
	  		pfacc			mm6,			mm0
	  		pfacc			mm7,			mm1
	  		pxor			mm6,			neg_q_1
	  		pxor			mm7,			neg_q_1
	  															// mm6 =	oX2[2]			oX1[1]

	  															// mm7 =	oX2[3]			oX1[0]
	  		// make oX1[1] oX1[0]
	  		movq			mm0,			mm7
	  		punpckldq		mm0,			mm6
	  		movq			[ecx],			mm0

	  		// make oX1[3] oX1[2]
	  		movq			mm1,			mm5
	  		punpckldq		mm1,			mm4
	  		movq			[ecx+2*4],		mm1

	  		// make oX2[1] oX2[0]
	  		punpckhdq		mm4,			mm5
	  		movq			[edx],			mm4

	  		// make oX2[3] oX2[2]
			punpckhdq		mm6,			mm7
	  		movq			[edx+2*4],		mm6

  		add				edi,			8*4
  		add				eax,			8*4
		add				edx,			4*4

  		cmp				eax,			ecx
  		jb				loop3dnc
		femms
  	}

/*
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

*/
    iX=_out+n2+n4;
    oX1=_out+n4;
    oX2=oX1;
	_asm
	{
		mov		edi,		iX
		mov		ecx,		oX1
		mov		edx,		oX2
loop3dnd:

		sub		ecx,		4*4
		sub		edi,		4*4

		mov		eax,			[edi + 3*4]
		mov		esi,			[edi + 2*4]
		mov		[ecx + 3*4],	eax
		mov		[ecx + 2*4],	esi
		xor		eax,			0x80000000
		xor		esi,			0x80000000
		mov		[edx + 0*4],	eax
		mov		[edx + 1*4],	esi


		mov		eax,			[edi + 1*4]
		mov		esi,			[edi + 0*4]
		mov		[ecx + 1*4],	eax
		mov		[ecx + 0*4],	esi
		xor		eax,			0x80000000
		xor		esi,			0x80000000
		mov		[edx + 2*4],	eax
		mov		[edx + 3*4],	esi


		add		edx,		4*4
		cmp		edx,		edi
		jb		loop3dnd
	}


/*
   do{
      oX1-=4;
      iX-=4;

      oX2[0] = -(oX1[3] = iX[3]);
      oX2[1] = -(oX1[2] = iX[2]);
      oX2[2] = -(oX1[1] = iX[1]);
      oX2[3] = -(oX1[0] = iX[0]);

      oX2+=4;
    }while(oX2<iX);
*/

    iX=_out+n2+n4;
    oX1=_out+n2+n4;
    oX2=_out+n2;
    do{
      oX1-=4;
      oX1[0]= iX[3];
      oX1[1]= iX[2];
      oX1[2]= iX[1];
      oX1[3]= iX[0];
      iX+=4;
    }while(oX1>oX2);
  }
	}
}



void mdct_backward(mdct_lookup *init, DATA_TYPE *in, DATA_TYPE *out){
	if(CPU_SSE)
	{
			mdct_backward_sse(init, in, out);
		return;
	}
	else if(CPU_3DN)
	{
			mdct_backward_3dn(init, in, out);
		return;
	}

	{
  int n=init->n;
  int n2=n>>1;
  int n4=n>>2;

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
	_asm
	{
		mov		edi,		iX
		mov		ecx,		oX1
		mov		edx,		oX2
loopasmd:

		sub		ecx,		4*4
		sub		edi,		4*4

		mov		eax,			[edi + 3*4]
		mov		esi,			[edi + 2*4]
		mov		[ecx + 3*4],	eax
		mov		[ecx + 2*4],	esi
		xor		eax,			0x80000000
		xor		esi,			0x80000000
		mov		[edx + 0*4],	eax
		mov		[edx + 1*4],	esi


		mov		eax,			[edi + 1*4]
		mov		esi,			[edi + 0*4]
		mov		[ecx + 1*4],	eax
		mov		[ecx + 0*4],	esi
		xor		eax,			0x80000000
		xor		esi,			0x80000000
		mov		[edx + 2*4],	eax
		mov		[edx + 3*4],	esi


		add		edx,		4*4
		cmp		edx,		edi
		jb		loopasmd
	}
/*
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
*/
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
	}
}



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
}

