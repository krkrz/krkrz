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

 function: PCM data envelope analysis 
 last mod: $Id: envelope.c 8921 2005-02-14 23:03:43Z msmith $

 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ogg/ogg.h>
#include "vorbis/codec.h"
#include "codec_internal.h"

#include "os.h"
#include "scales.h"
#include "envelope.h"
#include "mdct.h"
#include "misc.h"
#ifdef __SSE__												/* SSE Optimize */
#include "xmmlib.h"
#endif														/* SSE Optimize */

void _ve_envelope_init(envelope_lookup *e,vorbis_info *vi){
  codec_setup_info *ci=vi->codec_setup;
  vorbis_info_psy_global *gi=&ci->psy_g_param;
  int ch=vi->channels;
  int i,j;
  int n=e->winlength=128;
  e->searchstep=64; /* not random */

  e->minenergy=gi->preecho_minenergy;
  e->ch=ch;
  e->storage=128;
  e->cursor=ci->blocksizes[1]/2;
  e->mdct_win=_ogg_calloc(n,sizeof(*e->mdct_win));
  mdct_init(&e->mdct,n);

  for(i=0;i<n;i++){
    e->mdct_win[i]=sin(i/(n-1.)*M_PI);
    e->mdct_win[i]*=e->mdct_win[i];
  }

  /* magic follows */
  e->band[0].begin=2;  e->band[0].end=4;
  e->band[1].begin=4;  e->band[1].end=5;
  e->band[2].begin=6;  e->band[2].end=6;
  e->band[3].begin=9;  e->band[3].end=8;
  e->band[4].begin=13;  e->band[4].end=8;
  e->band[5].begin=17;  e->band[5].end=8;
  e->band[6].begin=22;  e->band[6].end=8;

  for(j=0;j<VE_BANDS;j++){
    n=e->band[j].end;
    e->band[j].window=_ogg_malloc(n*sizeof(*e->band[0].window));
    for(i=0;i<n;i++){
      e->band[j].window[i]=sin((i+.5)/n*M_PI);
      e->band[j].total+=e->band[j].window[i];
    }
    e->band[j].total=1./e->band[j].total;
  }
  
  e->filter=_ogg_calloc(VE_BANDS*ch,sizeof(*e->filter));
  e->mark=_ogg_calloc(e->storage,sizeof(*e->mark));

}

void _ve_envelope_clear(envelope_lookup *e){
  int i;
  mdct_clear(&e->mdct);
  for(i=0;i<VE_BANDS;i++)
    _ogg_free(e->band[i].window);
  _ogg_free(e->mdct_win);
  _ogg_free(e->filter);
  _ogg_free(e->mark);
  memset(e,0,sizeof(*e));
}

/* fairly straight threshhold-by-band based until we find something
   that works better and isn't patented. */

static int _ve_amp(envelope_lookup *ve,
		   vorbis_info_psy_global *gi,
		   float *data,
		   envelope_band *bands,
		   envelope_filter_state *filters,
		   long pos){
  long n=ve->winlength;
  int ret=0;
  long i,j;
  float decay;

  /* we want to have a 'minimum bar' for energy, else we're just
     basing blocks on quantization noise that outweighs the signal
     itself (for low power signals) */

  float minV=ve->minenergy;
#ifdef	__SSE__												/* SSE Optimize */
  float *vec	 = (float*)_ogg_alloca(n*sizeof(*vec));
#else														/* SSE Optimize */
  float *vec=alloca(n*sizeof(*vec));
#endif														/* SSE Optimize */

  /* stretch is used to gradually lengthen the number of windows
     considered prevoius-to-potential-trigger */
  int stretch=max(VE_MINSTRETCH,ve->stretch/2);
  float penalty=gi->stretch_penalty-(ve->stretch/2-VE_MINSTRETCH);
  if(penalty<0.f)penalty=0.f;
  if(penalty>gi->stretch_penalty)penalty=gi->stretch_penalty;
  
  /*_analysis_output_always("lpcm",seq2,data,n,0,0,
    totalshift+pos*ve->searchstep);*/
  
 /* window and transform */
#ifdef	__SSE__												/* SSE Optimize */
	for(i=0;i<n;i+=32)
	{
		__m128	XMM0, XMM1, XMM2, XMM3;
		__m128	XMM4, XMM5, XMM6, XMM7;
		XMM0	 = _mm_load_ps(data+i   );
		XMM4	 = _mm_load_ps(ve->mdct_win+i   );
		XMM1	 = _mm_load_ps(data+i+ 4);
		XMM5	 = _mm_load_ps(ve->mdct_win+i+ 4);
		XMM2	 = _mm_load_ps(data+i+ 8);
		XMM6	 = _mm_load_ps(ve->mdct_win+i+ 8);
		XMM3	 = _mm_load_ps(data+i+12);
		XMM7	 = _mm_load_ps(ve->mdct_win+i+12);
		XMM0	 = _mm_mul_ps(XMM0, XMM4);
		XMM4	 = _mm_load_ps(data+i+16);
		XMM1	 = _mm_mul_ps(XMM1, XMM5);
		XMM5	 = _mm_load_ps(ve->mdct_win+i+16);
		XMM2	 = _mm_mul_ps(XMM2, XMM6);
		XMM6	 = _mm_load_ps(data+i+20);
		XMM3	 = _mm_mul_ps(XMM3, XMM7);
		XMM7	 = _mm_load_ps(ve->mdct_win+i+20);
		_mm_store_ps(vec+i   , XMM0);
		XMM0	 = _mm_load_ps(data+i+24);
		_mm_store_ps(vec+i+ 4, XMM1);
		XMM1	 = _mm_load_ps(ve->mdct_win+i+24);
		_mm_store_ps(vec+i+ 8, XMM2);
		XMM2	 = _mm_load_ps(data+i+28);
		_mm_store_ps(vec+i+12, XMM3);
		XMM3	 = _mm_load_ps(ve->mdct_win+i+28);
		XMM4	 = _mm_mul_ps(XMM4, XMM5);
		XMM6	 = _mm_mul_ps(XMM6, XMM7);
		XMM0	 = _mm_mul_ps(XMM0, XMM1);
		XMM2	 = _mm_mul_ps(XMM2, XMM3);
		_mm_store_ps(vec+i+16, XMM4);
		_mm_store_ps(vec+i+20, XMM6);
		_mm_store_ps(vec+i+24, XMM0);
		_mm_store_ps(vec+i+28, XMM2);
	}
	mdct_forward(&ve->mdct, vec, vec, NULL);
#else														/* SSE Optimize */
  for(i=0;i<n;i++)
    vec[i]=data[i]*ve->mdct_win[i];
  mdct_forward(&ve->mdct,vec,vec);
#endif														/* SSE Optimize */
  
  /*_analysis_output_always("mdct",seq2,vec,n/2,0,1,0); */

  /* near-DC spreading function; this has nothing to do with
     psychoacoustics, just sidelobe leakage and window size */
  {
    float temp=vec[0]*vec[0]+.7*vec[1]*vec[1]+.2*vec[2]*vec[2];
    int ptr=filters->nearptr;

    /* the accumulation is regularly refreshed from scratch to avoid
       floating point creep */
    if(ptr==0){
      decay=filters->nearDC_acc=filters->nearDC_partialacc+temp;
      filters->nearDC_partialacc=temp;
    }else{
      decay=filters->nearDC_acc+=temp;
      filters->nearDC_partialacc+=temp;
    }
    filters->nearDC_acc-=filters->nearDC[ptr];
    filters->nearDC[ptr]=temp;

    decay*=(1./(VE_NEARDC+1));
    filters->nearptr++;
    if(filters->nearptr>=VE_NEARDC)filters->nearptr=0;
    decay=todB(&decay)*.5-15.f;
  }
  
  /* perform spreading and limiting, also smooth the spectrum.  yes,
     the MDCT results in all real coefficients, but it still *behaves*
     like real/imaginary pairs */
#ifdef	__SSE__												/* SSE Optimize */
	{
		static _MM_ALIGN16 const float mparm[4]	 = {
			7.17711438e-7f/2.f, 7.17711438e-7f/2.f, 7.17711438e-7f/2.f, 7.17711438e-7f/2.f
		};
		static _MM_ALIGN16 const float aparm[4]	 = {
			-764.6161886f/2.f, -764.6161886f/2.f, -764.6161886f/2.f, -764.6161886f/2.f
		};
		static _MM_ALIGN16 const float decayinit0[4]	 = {
			0.f, 8.f,	16.f,	24.f
		};
		static _MM_ALIGN16 const float decayinit1[4]	 = {
			32.f, 40.f,	48.f,	56.f
		};
		static _MM_ALIGN16 const float p16[4]	 = {
			64.f,	64.f,	64.f,	64.f
		};
		__m128 MINV		 = _mm_set_ps1(minV);
		float	*p	 = vec;
		int midpoint	 = ((int)(-(minV-decay)/4.f)+15)&(~15);
		int last_n		 = n/2;
		__m128 DECAY0	 = _mm_set_ps1(decay);
		__m128 DECAY1	 = _mm_set_ps1(decay);
		DECAY0	 = _mm_sub_ps(DECAY0, PM128(decayinit0));
		DECAY1	 = _mm_sub_ps(DECAY1, PM128(decayinit1));
#if	defined(__SSE2__)
		for(i=0;i<midpoint;i+=16,p+=8)
		{
			__m128	XMM0, XMM2;
			__m128	XMM1, XMM3;
#if	defined(__SSE3__)
			XMM0	 = _mm_load_ps(vec+i   );
			XMM1	 = _mm_load_ps(vec+i+ 4);
			XMM2	 = _mm_load_ps(vec+i+ 8);
			XMM3	 = _mm_load_ps(vec+i+12);
			XMM0	 = _mm_mul_ps(XMM0, XMM0);
			XMM1	 = _mm_mul_ps(XMM1, XMM1);
			XMM2	 = _mm_mul_ps(XMM2, XMM2);
			XMM3	 = _mm_mul_ps(XMM3, XMM3);
			XMM0	 = _mm_hadd_ps(XMM0, XMM1);
			XMM2	 = _mm_hadd_ps(XMM2, XMM3);
#else
			__m128	XMM4, XMM5;
			XMM0	 = _mm_load_ps(vec+i   );
			XMM2	 = _mm_load_ps(vec+i+ 8);
			XMM4	 = _mm_load_ps(vec+i+ 4);
			XMM5	 = _mm_load_ps(vec+i+12);
			XMM1	 = XMM0;
			XMM3	 = XMM2;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM4,_MM_SHUFFLE(2,0,2,0));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM4,_MM_SHUFFLE(3,1,3,1));
			XMM2	 = _mm_shuffle_ps(XMM2, XMM5,_MM_SHUFFLE(2,0,2,0));
			XMM3	 = _mm_shuffle_ps(XMM3, XMM5,_MM_SHUFFLE(3,1,3,1));
			XMM0	 = _mm_mul_ps(XMM0, XMM0);
			XMM1	 = _mm_mul_ps(XMM1, XMM1);
			XMM2	 = _mm_mul_ps(XMM2, XMM2);
			XMM3	 = _mm_mul_ps(XMM3, XMM3);
			XMM0	 = _mm_add_ps(XMM0, XMM1);
			XMM2	 = _mm_add_ps(XMM2, XMM3);
#endif
			XMM0	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM0));
			XMM2	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM2));
			XMM0	 = _mm_mul_ps(XMM0, PM128(mparm));
			XMM2	 = _mm_mul_ps(XMM2, PM128(mparm));
			XMM0	 = _mm_add_ps(XMM0, PM128(aparm));
			XMM2	 = _mm_add_ps(XMM2, PM128(aparm));
			XMM0	 = _mm_max_ps(XMM0, DECAY0);
			XMM2	 = _mm_max_ps(XMM2, DECAY1);
			XMM0	 = _mm_max_ps(XMM0, MINV);
			XMM2	 = _mm_max_ps(XMM2, MINV);
			_mm_store_ps(p  , XMM0);
			_mm_store_ps(p+4, XMM2);
			DECAY0	 = _mm_sub_ps(DECAY0, PM128(p16));
			DECAY1	 = _mm_sub_ps(DECAY1, PM128(p16));
		}
		for(;i<last_n;i+=16,p+=8)
		{
			__m128	XMM0, XMM2;
			__m128	XMM1, XMM3;
#if	defined(__SSE3__)
			XMM0	 = _mm_load_ps(vec+i   );
			XMM1	 = _mm_load_ps(vec+i+ 4);
			XMM2	 = _mm_load_ps(vec+i+ 8);
			XMM3	 = _mm_load_ps(vec+i+12);
			XMM0	 = _mm_mul_ps(XMM0, XMM0);
			XMM1	 = _mm_mul_ps(XMM1, XMM1);
			XMM2	 = _mm_mul_ps(XMM2, XMM2);
			XMM3	 = _mm_mul_ps(XMM3, XMM3);
			XMM0	 = _mm_hadd_ps(XMM0, XMM1);
			XMM2	 = _mm_hadd_ps(XMM2, XMM3);
#else
			__m128	XMM4, XMM5;
			XMM0	 = _mm_load_ps(vec+i   );
			XMM2	 = _mm_load_ps(vec+i+ 8);
			XMM4	 = _mm_load_ps(vec+i+ 4);
			XMM5	 = _mm_load_ps(vec+i+12);
			XMM1	 = XMM0;
			XMM3	 = XMM2;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM4,_MM_SHUFFLE(2,0,2,0));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM4,_MM_SHUFFLE(3,1,3,1));
			XMM2	 = _mm_shuffle_ps(XMM2, XMM5,_MM_SHUFFLE(2,0,2,0));
			XMM3	 = _mm_shuffle_ps(XMM3, XMM5,_MM_SHUFFLE(3,1,3,1));
			XMM0	 = _mm_mul_ps(XMM0, XMM0);
			XMM1	 = _mm_mul_ps(XMM1, XMM1);
			XMM2	 = _mm_mul_ps(XMM2, XMM2);
			XMM3	 = _mm_mul_ps(XMM3, XMM3);
			XMM0	 = _mm_add_ps(XMM0, XMM1);
			XMM2	 = _mm_add_ps(XMM2, XMM3);
#endif
			XMM0	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM0));
			XMM2	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM2));
			XMM0	 = _mm_mul_ps(XMM0, PM128(mparm));
			XMM2	 = _mm_mul_ps(XMM2, PM128(mparm));
			XMM0	 = _mm_add_ps(XMM0, PM128(aparm));
			XMM2	 = _mm_add_ps(XMM2, PM128(aparm));
			XMM0	 = _mm_max_ps(XMM0, MINV);
			XMM2	 = _mm_max_ps(XMM2, MINV);
			_mm_store_ps(p  , XMM0);
			_mm_store_ps(p+4, XMM2);
		}
#else	/* for __SSE2__ */
/*
		SSE optimized code
*/
		for(i=0;i<midpoint;i+=16,p+=8)
		{
			__m64	MM0, MM1, MM2, MM3;
			__m128x	U0, U1;
			{
				__m128	XMM0, XMM2;
				__m128	XMM1, XMM3;
				__m128	XMM4, XMM5;
				XMM0	 = _mm_load_ps(vec+i   );
				XMM2	 = _mm_load_ps(vec+i+ 8);
				XMM4	 = _mm_load_ps(vec+i+ 4);
				XMM5	 = _mm_load_ps(vec+i+12);
				XMM1	 = XMM0;
				XMM3	 = XMM2;
				XMM0	 = _mm_shuffle_ps(XMM0, XMM4,_MM_SHUFFLE(2,0,2,0));
				XMM1	 = _mm_shuffle_ps(XMM1, XMM4,_MM_SHUFFLE(3,1,3,1));
				XMM2	 = _mm_shuffle_ps(XMM2, XMM5,_MM_SHUFFLE(2,0,2,0));
				XMM3	 = _mm_shuffle_ps(XMM3, XMM5,_MM_SHUFFLE(3,1,3,1));
				XMM0	 = _mm_mul_ps(XMM0, XMM0);
				XMM1	 = _mm_mul_ps(XMM1, XMM1);
				XMM2	 = _mm_mul_ps(XMM2, XMM2);
				XMM3	 = _mm_mul_ps(XMM3, XMM3);
				XMM0	 = _mm_add_ps(XMM0, XMM1);
				XMM2	 = _mm_add_ps(XMM2, XMM3);
				U0.ps	 = XMM0;
				U1.ps	 = XMM2;
				MM0		 = U0.pi64[1];
				MM1		 = U1.pi64[1];
				MM2		 = U0.pi64[0];
				MM3		 = U1.pi64[0];
				XMM0	 = _mm_cvtpi32_ps(XMM0, MM0);
				XMM2	 = _mm_cvtpi32_ps(XMM2, MM1);
				XMM0	 = _mm_movelh_ps(XMM0, XMM0);
				XMM2	 = _mm_movelh_ps(XMM2, XMM2);
				XMM0	 = _mm_cvtpi32_ps(XMM0, MM2);
				XMM2	 = _mm_cvtpi32_ps(XMM2, MM3);
				XMM0	 = _mm_mul_ps(XMM0, PM128(mparm));
				XMM2	 = _mm_mul_ps(XMM2, PM128(mparm));
				XMM0	 = _mm_add_ps(XMM0, PM128(aparm));
				XMM2	 = _mm_add_ps(XMM2, PM128(aparm));
				XMM0	 = _mm_max_ps(XMM0, DECAY0);
				XMM2	 = _mm_max_ps(XMM2, DECAY1);
				XMM0	 = _mm_max_ps(XMM0, MINV);
				XMM2	 = _mm_max_ps(XMM2, MINV);
				_mm_store_ps(p  , XMM0);
				_mm_store_ps(p+4, XMM2);
			}
			DECAY0	 = _mm_sub_ps(DECAY0, PM128(p16));
			DECAY1	 = _mm_sub_ps(DECAY1, PM128(p16));
		}
		for(;i<last_n;i+=16,p+=8)
		{
			__m64	MM0, MM1, MM2, MM3;
			__m128x	U0, U1;
			{
				__m128	XMM0, XMM2;
				__m128	XMM1, XMM3;
				__m128	XMM4, XMM5;
				XMM0	 = _mm_load_ps(vec+i   );
				XMM2	 = _mm_load_ps(vec+i+ 8);
				XMM4	 = _mm_load_ps(vec+i+ 4);
				XMM5	 = _mm_load_ps(vec+i+12);
				XMM1	 = XMM0;
				XMM3	 = XMM2;
				XMM0	 = _mm_shuffle_ps(XMM0, XMM4,_MM_SHUFFLE(2,0,2,0));
				XMM1	 = _mm_shuffle_ps(XMM1, XMM4,_MM_SHUFFLE(3,1,3,1));
				XMM2	 = _mm_shuffle_ps(XMM2, XMM5,_MM_SHUFFLE(2,0,2,0));
				XMM3	 = _mm_shuffle_ps(XMM3, XMM5,_MM_SHUFFLE(3,1,3,1));
				XMM0	 = _mm_mul_ps(XMM0, XMM0);
				XMM1	 = _mm_mul_ps(XMM1, XMM1);
				XMM2	 = _mm_mul_ps(XMM2, XMM2);
				XMM3	 = _mm_mul_ps(XMM3, XMM3);
				XMM0	 = _mm_add_ps(XMM0, XMM1);
				XMM2	 = _mm_add_ps(XMM2, XMM3);
				U0.ps	 = XMM0;
				U1.ps	 = XMM2;
				MM0		 = U0.pi64[1];
				MM1		 = U1.pi64[1];
				MM2		 = U0.pi64[0];
				MM3		 = U1.pi64[0];
				XMM0	 = _mm_cvtpi32_ps(XMM0, MM0);
				XMM2	 = _mm_cvtpi32_ps(XMM2, MM1);
				XMM0	 = _mm_movelh_ps(XMM0, XMM0);
				XMM2	 = _mm_movelh_ps(XMM2, XMM2);
				XMM0	 = _mm_cvtpi32_ps(XMM0, MM2);
				XMM2	 = _mm_cvtpi32_ps(XMM2, MM3);
				XMM0	 = _mm_mul_ps(XMM0, PM128(mparm));
				XMM2	 = _mm_mul_ps(XMM2, PM128(mparm));
				XMM0	 = _mm_add_ps(XMM0, PM128(aparm));
				XMM2	 = _mm_add_ps(XMM2, PM128(aparm));
				XMM0	 = _mm_max_ps(XMM0, MINV);
				XMM2	 = _mm_max_ps(XMM2, MINV);
				_mm_store_ps(p  , XMM0);
				_mm_store_ps(p+4, XMM2);
			}
		}
		_mm_empty();
#endif	/* for __SSE2__ */
	}
#else														/* SSE Optimize */
    for(i=0;i<n/2;i+=2){
    float val=vec[i]*vec[i]+vec[i+1]*vec[i+1];
    val=todB(&val)*.5f;
    if(val<decay)val=decay;
    if(val<minV)val=minV;
    vec[i>>1]=val;
    decay-=8.;
  }
#endif														/* SSE Optimize */

  /*_analysis_output_always("spread",seq2++,vec,n/4,0,0,0);*/
  
  /* perform preecho/postecho triggering by band */
  for(j=0;j<VE_BANDS;j++){

    /* accumulate amplitude */
#ifdef	__SSE__												/* SSE Optimize */
	float acc;
	float valmax,valmin;
	{
		__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5;
		if(bands[j].end!=8)
		{
			switch(bands[j].end)
			{
				case 4 :	/* bands[j].end==4(14.286%) */
					XMM0	 = _mm_lddqu_ps(vec+bands[j].begin);
					XMM1	 = _mm_load_ps(bands[j].window  );
					XMM0	 = _mm_mul_ps(XMM0, XMM1);
					break;
				case 5 :	/* bands[j].end==5(14.286%) */
					XMM0	 = _mm_lddqu_ps(vec+bands[j].begin);
					XMM2	 = _mm_load_ss(vec+bands[j].begin+4);
					XMM1	 = _mm_load_ps(bands[j].window  );
					XMM3	 = _mm_load_ss(bands[j].window+4);
					XMM0	 = _mm_mul_ps(XMM0, XMM1);
					XMM2	 = _mm_mul_ss(XMM2, XMM3);
					XMM0	 = _mm_add_ss(XMM0, XMM2);
					break;
				case 6 :	/* bands[j].end==6(14.286%) */
					XMM0	 = _mm_lddqu_ps(vec+bands[j].begin);
					XMM2	 = _mm_load_ss(vec+bands[j].begin+4);
					XMM4	 = _mm_load_ss(vec+bands[j].begin+5);
					XMM1	 = _mm_load_ps(bands[j].window  );
					XMM3	 = _mm_load_ss(bands[j].window+4);
					XMM5	 = _mm_load_ss(bands[j].window+5);
					XMM0	 = _mm_mul_ps(XMM0, XMM1);
					XMM2	 = _mm_mul_ss(XMM2, XMM3);
					XMM4	 = _mm_mul_ss(XMM4, XMM5);
					XMM2	 = _mm_add_ss(XMM2, XMM4);
					XMM0	 = _mm_add_ss(XMM0, XMM2);
					break;
			}
		}
		else	/* bands[j].end==8(57.143%) */
		{
			XMM0	 = _mm_lddqu_ps(vec+bands[j].begin  );
			XMM1	 = _mm_load_ps(bands[j].window  );
			XMM2	 = _mm_lddqu_ps(vec+bands[j].begin+4);
			XMM3	 = _mm_load_ps(bands[j].window+4);
			XMM0	 = _mm_mul_ps(XMM0, XMM1);
			XMM2	 = _mm_mul_ps(XMM2, XMM3);
			XMM0	 = _mm_add_ps(XMM0, XMM2);
		}
		acc		 = _mm_add_horz(XMM0);
	}
#else														/* SSE Optimize */
    float acc=0.;
    float valmax,valmin;
    for(i=0;i<bands[j].end;i++)
      acc+=vec[i+bands[j].begin]*bands[j].window[i];
#endif														/* SSE Optimize */
   
    acc*=bands[j].total;

    /* convert amplitude to delta */
    {
      int p,this=filters[j].ampptr;
      float postmax,postmin,premax=-99999.f,premin=99999.f;
      
      p=this;
      p--;
      if(p<0)p+=VE_AMP;
      postmax=max(acc,filters[j].ampbuf[p]);
      postmin=min(acc,filters[j].ampbuf[p]);
      
      for(i=0;i<stretch;i++){
	p--;
	if(p<0)p+=VE_AMP;
	premax=max(premax,filters[j].ampbuf[p]);
	premin=min(premin,filters[j].ampbuf[p]);
      }
      
      valmin=postmin-premin;
      valmax=postmax-premax;

      /*filters[j].markers[pos]=valmax;*/
      filters[j].ampbuf[this]=acc;
      filters[j].ampptr++;
      if(filters[j].ampptr>=VE_AMP)filters[j].ampptr=0;
    }

    /* look at min/max, decide trigger */
    if(valmax>gi->preecho_thresh[j]+penalty){
      ret|=1;
      ret|=4;
    }
    if(valmin<gi->postecho_thresh[j]-penalty)ret|=2;
  }
 
  return(ret);
}

#if 0
static int seq=0;
static ogg_int64_t totalshift=-1024;
#endif

long _ve_envelope_search(vorbis_dsp_state *v){
  vorbis_info *vi=v->vi;
  codec_setup_info *ci=vi->codec_setup;
  vorbis_info_psy_global *gi=&ci->psy_g_param;
  envelope_lookup *ve=((private_state *)(v->backend_state))->ve;
  long i,j;

  int first=ve->current/ve->searchstep;
  int last=v->pcm_current/ve->searchstep-VE_WIN;
  if(first<0)first=0;

  /* make sure we have enough storage to match the PCM */
  if(last+VE_WIN+VE_POST>ve->storage){
    ve->storage=last+VE_WIN+VE_POST; /* be sure */
    ve->mark=_ogg_realloc(ve->mark,ve->storage*sizeof(*ve->mark));
  }

  for(j=first;j<last;j++){
    int ret=0;

    ve->stretch++;
    if(ve->stretch>VE_MAXSTRETCH*2)
      ve->stretch=VE_MAXSTRETCH*2;
    
    for(i=0;i<ve->ch;i++){
      float *pcm=v->pcm[i]+ve->searchstep*(j);
      ret|=_ve_amp(ve,gi,pcm,ve->band,ve->filter+i*VE_BANDS,j);
    }

    ve->mark[j+VE_POST]=0;
    if(ret&1){
      ve->mark[j]=1;
      ve->mark[j+1]=1;
    }

    if(ret&2){
      ve->mark[j]=1;
      if(j>0)ve->mark[j-1]=1;
    }

    if(ret&4)ve->stretch=-1;
  }

  ve->current=last*ve->searchstep;

  {
    long centerW=v->centerW;
    long testW=
      centerW+
      ci->blocksizes[v->W]/4+
      ci->blocksizes[1]/2+
      ci->blocksizes[0]/4;
    
    j=ve->cursor;
    
    while(j<ve->current-(ve->searchstep)){/* account for postecho
                                             working back one window */
      if(j>=testW)return(1);
 
      ve->cursor=j;

      if(ve->mark[j/ve->searchstep]){
	if(j>centerW){

	  #if 0
	  if(j>ve->curmark){
	    float *marker=alloca(v->pcm_current*sizeof(*marker));
	    int l,m;
	    memset(marker,0,sizeof(*marker)*v->pcm_current);
	    fprintf(stderr,"mark! seq=%d, cursor:%fs time:%fs\n",
		    seq,
		    (totalshift+ve->cursor)/44100.,
		    (totalshift+j)/44100.);
	    _analysis_output_always("pcmL",seq,v->pcm[0],v->pcm_current,0,0,totalshift);
	    _analysis_output_always("pcmR",seq,v->pcm[1],v->pcm_current,0,0,totalshift);

	    _analysis_output_always("markL",seq,v->pcm[0],j,0,0,totalshift);
	    _analysis_output_always("markR",seq,v->pcm[1],j,0,0,totalshift);
	    
	    for(m=0;m<VE_BANDS;m++){
	      char buf[80];
	      sprintf(buf,"delL%d",m);
	      for(l=0;l<last;l++)marker[l*ve->searchstep]=ve->filter[m].markers[l]*.1;
	      _analysis_output_always(buf,seq,marker,v->pcm_current,0,0,totalshift);
	    }

	    for(m=0;m<VE_BANDS;m++){
	      char buf[80];
	      sprintf(buf,"delR%d",m);
	      for(l=0;l<last;l++)marker[l*ve->searchstep]=ve->filter[m+VE_BANDS].markers[l]*.1;
	      _analysis_output_always(buf,seq,marker,v->pcm_current,0,0,totalshift);
	    }

	    for(l=0;l<last;l++)marker[l*ve->searchstep]=ve->mark[l]*.4;
	    _analysis_output_always("mark",seq,marker,v->pcm_current,0,0,totalshift);
	   
	    
	    seq++;
	    
	  }
#endif

	  ve->curmark=j;
	  if(j>=testW)return(1);
	  return(0);
	}
      }
      j+=ve->searchstep;
    }
  }
  
  return(-1);
}

int _ve_envelope_mark(vorbis_dsp_state *v){
  envelope_lookup *ve=((private_state *)(v->backend_state))->ve;
  vorbis_info *vi=v->vi;
  codec_setup_info *ci=vi->codec_setup;
  long centerW=v->centerW;
  long beginW=centerW-ci->blocksizes[v->W]/4;
  long endW=centerW+ci->blocksizes[v->W]/4;
  if(v->W){
    beginW-=ci->blocksizes[v->lW]/4;
    endW+=ci->blocksizes[v->nW]/4;
  }else{
    beginW-=ci->blocksizes[0]/4;
    endW+=ci->blocksizes[0]/4;
  }

  if(ve->curmark>=beginW && ve->curmark<endW)return(1);
  {
    long first=beginW/ve->searchstep;
    long last=endW/ve->searchstep;
    long i;
    for(i=first;i<last;i++)
      if(ve->mark[i])return(1);
  }
  return(0);
}

void _ve_envelope_shift(envelope_lookup *e,long shift){
  int smallsize=e->current/e->searchstep+VE_POST; /* adjust for placing marks
						     ahead of ve->current */
  int smallshift=shift/e->searchstep;

  memmove(e->mark,e->mark+smallshift,(smallsize-smallshift)*sizeof(*e->mark));
  
  #if 0
  for(i=0;i<VE_BANDS*e->ch;i++)
    memmove(e->filter[i].markers,
	    e->filter[i].markers+smallshift,
	    (1024-smallshift)*sizeof(*(*e->filter).markers));
  totalshift+=shift;
  #endif 

  e->current-=shift;
  if(e->curmark>=0)
    e->curmark-=shift;
  e->cursor-=shift;
}






