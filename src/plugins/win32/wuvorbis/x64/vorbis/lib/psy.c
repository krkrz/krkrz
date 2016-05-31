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

 function: psychoacoustics not including preecho
 last mod: $Id: psy.c 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "vorbis/codec.h"
#include "codec_internal.h"

#ifdef __SSE__												/* SSE Optimize */
#include "xmmlib.h"
#endif														/* SSE Optimize */
#include "masking.h"
#include "psy.h"
#include "os.h"
#include "lpc.h"
#include "smallft.h"
#include "scales.h"
#include "misc.h"

#define NEGINF -9999.f

/*
  rephase   = reverse phase limit (postpoint)
                                                0    1    2    3    4    5    6    7    8  */
static double stereo_threshholds[]=           {0.0, 0.5, 1.0, 1.5, 2.5, 4.5, 8.5,16.5, 9e10};
static double stereo_threshholds_rephase[]=   {0.0, 0.5, 0.5, 1.0, 1.5, 1.5, 2.5, 2.5, 9e10};

static double stereo_threshholds_low[]=       {0.0, 0.5, 0.5, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0};
static double stereo_threshholds_high[]=      {0.0, 0.5, 0.5, 0.5, 1.0, 3.0, 5.5, 8.5, 0.0};


static int m3n32[] = {21,13,10,4};
static int m3n44[] = {15,9,7,3};
static int m3n48[] = {14,8,6,3};

static int temp_bfn[128] = {
 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7,
 8, 8, 8, 8, 9, 9, 9, 9,10,10,10,10,11,11,11,11,
12,12,12,12,13,13,13,13,14,14,14,14,15,15,15,15,
16,16,16,16,17,17,17,17,18,18,18,18,19,19,19,19,
20,20,20,20,21,21,21,21,22,22,22,22,23,23,23,23,
24,24,24,24,25,25,25,24,23,22,21,20,19,18,17,16,
15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
};

static float nnmid_th=0.2;


#ifdef __SSE__												/* SSE Optimize */
static _MM_ALIGN16 const float PNEGINF[4] = {NEGINF, NEGINF, NEGINF, NEGINF};

static const int temp_bfn8[128] = {
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
25,25,25,25,25,25,25,25,17,17,17,17,17,17,17,17,
 9, 9, 9, 9, 9, 9, 9, 9, 1, 1, 1, 1, 1, 1, 1, 1,
};

static const int temp_bfn4[128] = {
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,
25,25,25,25,25,25,25,25,21,21,21,21,17,17,17,17,
13,13,13,13, 9, 9, 9, 9, 5, 5, 5, 5, 1, 1, 1, 1,
};

static _MM_ALIGN16 const float PTEMP_BFN1[1] = {
	-8.0000000e+001, 
};
static _MM_ALIGN16 const float PTEMP_BFN2[2] = {
	-4.2000000e+001, -7.9000000e+001, 
};
static _MM_ALIGN16 const float PTEMP_BFN3[3] = {
	-3.0000000e+001, -5.5000000e+001, -8.0000000e+001, 
};
static _MM_ALIGN16 const float PTEMP_BFN4[4] = {
	-2.3000000e+001, -4.1000000e+001, -5.9000000e+001, -7.7000000e+001, 
	
};
static _MM_ALIGN16 const float PTEMP_BFN5[5] = {
	-2.0000000e+001, -3.5000000e+001, -5.0000000e+001, -6.5000000e+001, 
	-8.0000000e+001, 
};
static _MM_ALIGN16 const float PTEMP_BFN6[6] = {
	-1.7000000e+001, -2.9000000e+001, -4.1000000e+001, -5.3000000e+001, 
	-6.5000000e+001, -7.7000000e+001, 
};
static _MM_ALIGN16 const float PTEMP_BFN7[7] = {
	-1.5000000e+001, -2.5000000e+001, -3.5000000e+001, -4.5000000e+001, 
	-5.5000000e+001, -6.5000000e+001, -7.5000000e+001, 
};
static _MM_ALIGN16 const float PTEMP_BFN8[8] = {
	-1.4000000e+001, -2.3000000e+001, -3.2000000e+001, -4.1000000e+001, 
	-5.0000000e+001, -5.9000000e+001, -6.8000000e+001, -7.7000000e+001, 
	
};
static _MM_ALIGN16 const float PTEMP_BFN9[9] = {
	-1.3000000e+001, -2.1000000e+001, -2.9000000e+001, -3.7000000e+001, 
	-4.5000000e+001, -5.3000000e+001, -6.1000000e+001, -6.9000000e+001, 
	-7.7000000e+001, 
};
static _MM_ALIGN16 const float PTEMP_BFN10[10] = {
	-1.2000000e+001, -1.9000000e+001, -2.6000000e+001, -3.3000000e+001, 
	-4.0000000e+001, -4.7000000e+001, -5.4000000e+001, -6.1000000e+001, 
	-6.8000000e+001, -7.5000000e+001, 
};
static _MM_ALIGN16 const float PTEMP_BFN12[12] = {
	-1.1000000e+001, -1.7000000e+001, -2.3000000e+001, -2.9000000e+001, 
	-3.5000000e+001, -4.1000000e+001, -4.7000000e+001, -5.3000000e+001, 
	-5.9000000e+001, -6.5000000e+001, -7.1000000e+001, -7.7000000e+001, 
	
};
static _MM_ALIGN16 const float PTEMP_BFN15[15] = {
	-1.0000000e+001, -1.5000000e+001, -2.0000000e+001, -2.5000000e+001, 
	-3.0000000e+001, -3.5000000e+001, -4.0000000e+001, -4.5000000e+001, 
	-5.0000000e+001, -5.5000000e+001, -6.0000000e+001, -6.5000000e+001, 
	-7.0000000e+001, -7.5000000e+001, -8.0000000e+001, 
};
static _MM_ALIGN16 const float PTEMP_BFN18[18] = {
	-9.0000000e+000, -1.3000000e+001, -1.7000000e+001, -2.1000000e+001, 
	-2.5000000e+001, -2.9000000e+001, -3.3000000e+001, -3.7000000e+001, 
	-4.1000000e+001, -4.5000000e+001, -4.9000000e+001, -5.3000000e+001, 
	-5.7000000e+001, -6.1000000e+001, -6.5000000e+001, -6.9000000e+001, 
	-7.3000000e+001, -7.7000000e+001, 
};
static _MM_ALIGN16 const float PTEMP_BFN25[25] = {
	-8.0000000e+000, -1.1000000e+001, -1.4000000e+001, -1.7000000e+001, 
	-2.0000000e+001, -2.3000000e+001, -2.6000000e+001, -2.9000000e+001, 
	-3.2000000e+001, -3.5000000e+001, -3.8000000e+001, -4.1000000e+001, 
	-4.4000000e+001, -4.7000000e+001, -5.0000000e+001, -5.3000000e+001, 
	-5.6000000e+001, -5.9000000e+001, -6.2000000e+001, -6.5000000e+001, 
	-6.8000000e+001, -7.1000000e+001, -7.4000000e+001, -7.7000000e+001, 
	-8.0000000e+001, 
};

static const float *PTEMP_BFN[26]	 = {
	NULL,
	PTEMP_BFN1, PTEMP_BFN2, PTEMP_BFN3, PTEMP_BFN4, 
	PTEMP_BFN5, PTEMP_BFN6, PTEMP_BFN7, PTEMP_BFN8, 
	PTEMP_BFN9, PTEMP_BFN10, PTEMP_BFN12, PTEMP_BFN12, 
	PTEMP_BFN15, PTEMP_BFN15, PTEMP_BFN15, PTEMP_BFN18, 
	PTEMP_BFN18, PTEMP_BFN18, PTEMP_BFN25, PTEMP_BFN25, 
	PTEMP_BFN25, PTEMP_BFN25, PTEMP_BFN25, PTEMP_BFN25, 
	PTEMP_BFN25
};

/*
	for shellsort fix4 by SSE compare
*/
static _MM_ALIGN16 const int Sort4IndexConvTable[64*4]	 = {
	3,2,1,0,	/* A>B>C>D		000000	00 */
	3,2,0,1,	/* B>A>C>D		000001	01 */
	3,1,2,0,	/* A>C>B>D		000010	02 */
	0,1,2,3,	/*                      03 */
	2,3,1,0,	/* A>B>D>C		000100	04 */
	2,3,0,1,	/* B>A>D>C		000101	05 */
	0,1,2,3,	/*                      06 */
	0,1,2,3,	/*                      07 */
	0,1,2,3,	/*                      08 */
	0,1,2,3,	/*                      09 */
	0,1,2,3,	/*                      10 */
	0,1,2,3,	/*                      11 */
	0,1,2,3,	/*                      12 */
	2,0,3,1,	/* B>D>A>C		001101	13 */
	0,1,2,3,	/*                      14 */
	0,1,2,3,	/*                      15 */
	0,1,2,3,	/*                      16 */
	3,0,2,1,	/* B>C>A>D		010001	17 */
	3,1,0,2,	/* C>A>B>D		010010	18 */
	3,0,1,2,	/* C>B>A>D		010011	19 */
	0,1,2,3,	/*                      20 */
	0,1,2,3,	/*                      21 */
	0,1,2,3,	/*                      22 */
	0,1,2,3,	/*                      23 */
	0,1,2,3,	/*                      24 */
	0,3,2,1,	/* B>C>D>A		011001	25 */
	0,1,2,3,	/*                      26 */
	0,3,1,2,	/* C>B>D>A		011011	27 */
	0,1,2,3,	/*                      28 */
	0,2,3,1,	/* B>D>C>A		011101	29 */
	0,1,2,3,	/*                      30 */
	0,1,2,3,	/*                      31 */
	0,1,2,3,	/*                      32 */
	0,1,2,3,	/*                      33 */
	1,3,2,0,	/* A>C>D>B		100010	34 */
	0,1,2,3,	/*                      35 */
	2,1,3,0,	/* A>D>B>C		100100	36 */
	0,1,2,3,	/*                      37 */
	1,2,3,0,	/* A>D>C>B		100110	38 */
	0,1,2,3,	/*                      39 */
	0,1,2,3,	/*                      40 */
	0,1,2,3,	/*                      41 */
	0,1,2,3,	/*                      42 */
	0,1,2,3,	/*                      43 */
	2,1,0,3,	/* D>A>B>C		101100	44 */
	2,0,1,3,	/* D>B>A>C		101101	45 */
	1,2,0,3,	/* D>A>C>B		101110	46 */
	0,1,2,3,	/*                      47 */
	0,1,2,3,	/*                      48 */
	0,1,2,3,	/*                      49 */
	1,3,0,2,	/* C>A>D>B		110010	50 */
	0,1,2,3,	/*                      51 */
	0,1,2,3,	/*                      52 */
	0,1,2,3,	/*                      53 */
	0,1,2,3,	/*                      54 */
	0,1,2,3,	/*                      55 */
	0,1,2,3,	/*                      56 */
	0,1,2,3,	/*                      57 */
	1,0,3,2,	/* C>D>A>B		111010	58 */
	0,1,3,2,	/* C>D>B>A		111011	59 */
	0,1,2,3,	/*                      60 */
	0,2,1,3,	/* D>B>C>A		111101	61 */
	1,0,2,3,	/* D>C>A>B		111110	62 */
	0,1,2,3		/* D>C>B>A		111111	63 */
};

_MM_ALIGN16 float findex[2048];
_MM_ALIGN16 float findex2[2048];

#endif														/* SSE Optimize */

vorbis_look_psy_global *_vp_global_look(vorbis_info *vi){
  codec_setup_info *ci=vi->codec_setup;
  vorbis_info_psy_global *gi=&ci->psy_g_param;
  vorbis_look_psy_global *look=_ogg_calloc(1,sizeof(*look));

  look->channels=vi->channels;

  look->ampmax=-9999.;
  look->gi=gi;
  return(look);
}

void _vp_global_free(vorbis_look_psy_global *look){
  if(look){
    memset(look,0,sizeof(*look));
    _ogg_free(look);
  }
}

void _vi_gpsy_free(vorbis_info_psy_global *i){
  if(i){
    memset(i,0,sizeof(*i));
    _ogg_free(i);
  }
}

void _vi_psy_free(vorbis_info_psy *i){
  if(i){
    memset(i,0,sizeof(*i));
    _ogg_free(i);
  }
}

static void min_curve(float *c,
		       float *c2){
  int i;  
  for(i=0;i<EHMER_MAX;i++)if(c2[i]<c[i])c[i]=c2[i];
}
static void max_curve(float *c,
		       float *c2){
  int i;  
  for(i=0;i<EHMER_MAX;i++)if(c2[i]>c[i])c[i]=c2[i];
}

static void attenuate_curve(float *c,float att){
  int i;
  for(i=0;i<EHMER_MAX;i++)
    c[i]+=att;
}

static float ***setup_tone_curves(float curveatt_dB[P_BANDS],float binHz,int n,
				  float center_boost, float center_decay_rate){
  int i,j,k,m;
  float ath[EHMER_MAX];
  float workc[P_BANDS][P_LEVELS][EHMER_MAX];
  float athc[P_LEVELS][EHMER_MAX];
  float *brute_buffer=alloca(n*sizeof(*brute_buffer));

  float ***ret=_ogg_malloc(sizeof(*ret)*P_BANDS);

  memset(workc,0,sizeof(workc));

  for(i=0;i<P_BANDS;i++){
    /* we add back in the ATH to avoid low level curves falling off to
       -infinity and unnecessarily cutting off high level curves in the
       curve limiting (last step). */

    /* A half-band's settings must be valid over the whole band, and
       it's better to mask too little than too much */  
    int ath_offset=i*4;
    for(j=0;j<EHMER_MAX;j++){
      float min=999.;
      for(k=0;k<4;k++)
	if(j+k+ath_offset<MAX_ATH){
	  if(min>ATH[j+k+ath_offset])min=ATH[j+k+ath_offset];
	}else{
	  if(min>ATH[MAX_ATH-1])min=ATH[MAX_ATH-1];
	}
      ath[j]=min;
    }

    /* copy curves into working space, replicate the 50dB curve to 30
       and 40, replicate the 100dB curve to 110 */
    for(j=0;j<6;j++)
      memcpy(workc[i][j+2],tonemasks[i][j],EHMER_MAX*sizeof(*tonemasks[i][j]));
    memcpy(workc[i][0],tonemasks[i][0],EHMER_MAX*sizeof(*tonemasks[i][0]));
    memcpy(workc[i][1],tonemasks[i][0],EHMER_MAX*sizeof(*tonemasks[i][0]));
    
    /* apply centered curve boost/decay */
    for(j=0;j<P_LEVELS;j++){
      for(k=0;k<EHMER_MAX;k++){
	float adj=center_boost+abs(EHMER_OFFSET-k)*center_decay_rate;
	if(adj<0. && center_boost>0)adj=0.;
	if(adj>0. && center_boost<0)adj=0.;
	workc[i][j][k]+=adj;
      }
    }

    /* normalize curves so the driving amplitude is 0dB */
    /* make temp curves with the ATH overlayed */
    for(j=0;j<P_LEVELS;j++){
      attenuate_curve(workc[i][j],curveatt_dB[i]+100.-(j<2?2:j)*10.-P_LEVEL_0);
      memcpy(athc[j],ath,EHMER_MAX*sizeof(**athc));
      attenuate_curve(athc[j],+100.-j*10.f-P_LEVEL_0);
      max_curve(athc[j],workc[i][j]);
    }

    /* Now limit the louder curves.
       
       the idea is this: We don't know what the playback attenuation
       will be; 0dB SL moves every time the user twiddles the volume
       knob. So that means we have to use a single 'most pessimal' curve
       for all masking amplitudes, right?  Wrong.  The *loudest* sound
       can be in (we assume) a range of ...+100dB] SL.  However, sounds
       20dB down will be in a range ...+80], 40dB down is from ...+60],
       etc... */
    
    for(j=1;j<P_LEVELS;j++){
      min_curve(athc[j],athc[j-1]);
      min_curve(workc[i][j],athc[j]);
    }
  }

  for(i=0;i<P_BANDS;i++){
    int hi_curve,lo_curve,bin;
    ret[i]=_ogg_malloc(sizeof(**ret)*P_LEVELS);

    /* low frequency curves are measured with greater resolution than
       the MDCT/FFT will actually give us; we want the curve applied
       to the tone data to be pessimistic and thus apply the minimum
       masking possible for a given bin.  That means that a single bin
       could span more than one octave and that the curve will be a
       composite of multiple octaves.  It also may mean that a single
       bin may span > an eighth of an octave and that the eighth
       octave values may also be composited. */
    
    /* which octave curves will we be compositing? */
    bin=floor(fromOC(i*.5)/binHz);
    lo_curve=  ceil(toOC(bin*binHz+1)*2);
    hi_curve=  floor(toOC((bin+1)*binHz)*2);
    if(lo_curve>i)lo_curve=i;
    if(lo_curve<0)lo_curve=0;
    if(hi_curve>=P_BANDS)hi_curve=P_BANDS-1;

    for(m=0;m<P_LEVELS;m++){
      ret[i][m]=_ogg_malloc(sizeof(***ret)*(EHMER_MAX+2));
      
      for(j=0;j<n;j++)brute_buffer[j]=999.;
      
      /* render the curve into bins, then pull values back into curve.
	 The point is that any inherent subsampling aliasing results in
	 a safe minimum */
      for(k=lo_curve;k<=hi_curve;k++){
	int l=0;

	for(j=0;j<EHMER_MAX;j++){
	  int lo_bin= fromOC(j*.125+k*.5-2.0625)/binHz;
	  int hi_bin= fromOC(j*.125+k*.5-1.9375)/binHz+1;
	  
	  if(lo_bin<0)lo_bin=0;
	  if(lo_bin>n)lo_bin=n;
	  if(lo_bin<l)l=lo_bin;
	  if(hi_bin<0)hi_bin=0;
	  if(hi_bin>n)hi_bin=n;

	  for(;l<hi_bin && l<n;l++)
	    if(brute_buffer[l]>workc[k][m][j])
	      brute_buffer[l]=workc[k][m][j];
	}

	for(;l<n;l++)
	  if(brute_buffer[l]>workc[k][m][EHMER_MAX-1])
	    brute_buffer[l]=workc[k][m][EHMER_MAX-1];

      }

      /* be equally paranoid about being valid up to next half ocatve */
      if(i+1<P_BANDS){
	int l=0;
	k=i+1;
	for(j=0;j<EHMER_MAX;j++){
	  int lo_bin= fromOC(j*.125+i*.5-2.0625)/binHz;
	  int hi_bin= fromOC(j*.125+i*.5-1.9375)/binHz+1;
	  
	  if(lo_bin<0)lo_bin=0;
	  if(lo_bin>n)lo_bin=n;
	  if(lo_bin<l)l=lo_bin;
	  if(hi_bin<0)hi_bin=0;
	  if(hi_bin>n)hi_bin=n;

	  for(;l<hi_bin && l<n;l++)
	    if(brute_buffer[l]>workc[k][m][j])
	      brute_buffer[l]=workc[k][m][j];
	}

	for(;l<n;l++)
	  if(brute_buffer[l]>workc[k][m][EHMER_MAX-1])
	    brute_buffer[l]=workc[k][m][EHMER_MAX-1];

      }


      for(j=0;j<EHMER_MAX;j++){
	int bin=fromOC(j*.125+i*.5-2.)/binHz;
	if(bin<0){
	  ret[i][m][j+2]=-999.;
	}else{
	  if(bin>=n){
	    ret[i][m][j+2]=-999.;
	  }else{
	    ret[i][m][j+2]=brute_buffer[bin];
	  }
	}
      }

      /* add fenceposts */
      for(j=0;j<EHMER_OFFSET;j++)
	if(ret[i][m][j+2]>-200.f)break;  
      ret[i][m][0]=j;
      
      for(j=EHMER_MAX-1;j>EHMER_OFFSET+1;j--)
	if(ret[i][m][j+2]>-200.f)
	  break;
      ret[i][m][1]=j;

    }
  }

  return(ret);
}

void _vp_psy_init(vorbis_look_psy *p,vorbis_info_psy *vi,
		  vorbis_info_psy_global *gi,int n,long rate){
  long i,j,lo=-99,hi=1;
  long maxoc;
  memset(p,0,sizeof(*p));

  p->eighth_octave_lines=gi->eighth_octave_lines;
  p->shiftoc=rint(log(gi->eighth_octave_lines*8.f)/log(2.f))-1;

  p->firstoc=toOC(.25f*rate*.5/n)*(1<<(p->shiftoc+1))-gi->eighth_octave_lines;
  maxoc=toOC((n+.25f)*rate*.5/n)*(1<<(p->shiftoc+1))+.5f;
  p->total_octave_lines=maxoc-p->firstoc+1;
  p->ath=_ogg_malloc(n*sizeof(*p->ath));

  p->octave=_ogg_malloc(n*sizeof(*p->octave));
  p->bark=_ogg_malloc(n*sizeof(*p->bark));
  p->vi=vi;
  p->n=n;
  p->rate=rate;

  /* AoTuV HF weighting etc. */
  p->n25p=n/4;
  p->n33p=n/3;
  p->n75p=n*3/4;
  if(rate < 26000){
  	/* below 26kHz */
  	p->m_val = 0;
  	for(i=0; i<4; i++) p->m3n[i] = 0;
  	p->tonecomp_endp=0; // dummy
  	p->tonecomp_thres=.25;
  	p->st_freqlimit=n;
  	p->min_nn_lp=0;
  }else if(rate < 38000){
  	/* 32kHz */
  	p->m_val = .93;
  	for(i=0; i<4; i++) p->m3n[i] = m3n32[i];
  	if(n==128)      { p->tonecomp_endp= 124; p->tonecomp_thres=.5;
  	                 p->st_freqlimit=n; p->min_nn_lp=   0;}
  	else if(n==256) { p->tonecomp_endp= 248; p->tonecomp_thres=.7;
  	                 p->st_freqlimit=n; p->min_nn_lp=   0;}
  	else if(n==1024){ p->tonecomp_endp= 992; p->tonecomp_thres=.5;
  	                 p->st_freqlimit=n; p->min_nn_lp= 832;}
  	else if(n==2048){ p->tonecomp_endp=1984; p->tonecomp_thres=.7;
  	                 p->st_freqlimit=n; p->min_nn_lp=1664;}
  }else if(rate > 46000){
  	/* 48kHz */
  	p->m_val = 1.205;
  	for(i=0; i<4; i++) p->m3n[i] = m3n48[i];
  	if(n==128)      { p->tonecomp_endp=  83; p->tonecomp_thres=.5;
  	                 p->st_freqlimit=  89; p->min_nn_lp=   0;}
  	else if(n==256) { p->tonecomp_endp= 166; p->tonecomp_thres=.7;
  	                 p->st_freqlimit= 178; p->min_nn_lp=   0;}
  	else if(n==1024){ p->tonecomp_endp= 664; p->tonecomp_thres=.5;
  	                 p->st_freqlimit= 712; p->min_nn_lp= 576;}
  	else if(n==2048){ p->tonecomp_endp=1328; p->tonecomp_thres=.7;
  	                 p->st_freqlimit=1424; p->min_nn_lp=1152;}
  }else{
  	/* 44.1kHz */
  	p->m_val = 1.;
  	for(i=0; i<4; i++) p->m3n[i] = m3n44[i];
  	if(n==128)      { p->tonecomp_endp=  90; p->tonecomp_thres=.5;
  	                 p->st_freqlimit=  96; p->min_nn_lp=   0;}
  	else if(n==256) { p->tonecomp_endp= 180; p->tonecomp_thres=.7;
  	                 p->st_freqlimit= 192; p->min_nn_lp=   0;}
  	else if(n==1024){ p->tonecomp_endp= 720; p->tonecomp_thres=.5;
  	                 p->st_freqlimit= 768; p->min_nn_lp= 608;}
  	else if(n==2048){ p->tonecomp_endp=1440; p->tonecomp_thres=.7;
  	                 p->st_freqlimit=1536; p->min_nn_lp=1216;}
  }

  /* set up the lookups for a given blocksize and sample rate */

  for(i=0,j=0;i<MAX_ATH-1;i++){
    int endpos=rint(fromOC((i+1)*.125-2.)*2*n/rate);
    float base=ATH[i];
    if(j<endpos){
      float delta=(ATH[i+1]-base)/(endpos-j);
      for(;j<endpos && j<n;j++){
        p->ath[j]=base+100.;
        base+=delta;
      }
    }
  }

  for(i=0;i<n;i++){
    float bark=toBARK(rate/(2*n)*i); 

    for(;lo+vi->noisewindowlomin<i && 
	  toBARK(rate/(2*n)*lo)<(bark-vi->noisewindowlo);lo++);
    
    for(;hi<=n && (hi<i+vi->noisewindowhimin ||
	  toBARK(rate/(2*n)*hi)<(bark+vi->noisewindowhi));hi++);
    
    p->bark[i]=((lo-1)<<16)+(hi-1);

  }

  for(i=0;i<n;i++)
    p->octave[i]=toOC((i+.25f)*.5*rate/n)*(1<<(p->shiftoc+1))+.5f;

  p->tonecurves=setup_tone_curves(vi->toneatt,rate*.5/n,n,
				  vi->tone_centerboost,vi->tone_decay);
  
  /* set up rolling noise median */
  p->noiseoffset=_ogg_malloc(P_NOISECURVES*sizeof(*p->noiseoffset));
  for(i=0;i<P_NOISECURVES;i++)
    p->noiseoffset[i]=_ogg_malloc(n*sizeof(**p->noiseoffset));
  
  for(i=0;i<n;i++){
    float halfoc=toOC((i+.5)*rate/(2.*n))*2.;
    int inthalfoc;
    float del;
    
    if(halfoc<0)halfoc=0;
    if(halfoc>=P_BANDS-1)halfoc=P_BANDS-1;
    inthalfoc=(int)halfoc;
    del=halfoc-inthalfoc;
    
    for(j=0;j<P_NOISECURVES;j++)
      p->noiseoffset[j][i]=
	p->vi->noiseoff[j][inthalfoc]*(1.-del) + 
	p->vi->noiseoff[j][inthalfoc+1]*del;
    
  }
#if 0
  {
    static int ls=0;
    _analysis_output_always("noiseoff0",ls,p->noiseoffset[0],n,1,0,0);
    _analysis_output_always("noiseoff1",ls,p->noiseoffset[1],n,1,0,0);
    _analysis_output_always("noiseoff2",ls++,p->noiseoffset[2],n,1,0,0);
  }
#endif
#ifdef __SSE__												/* SSE Optimize */
	if(findex[1]==0.f)
	{
		for(i=0;i<2048;i++)
		{
			findex[i]	 = (float)(i);
			findex2[i]	 = (float)(i*i);
		}
	}
	{
		short* sb = (short*)p->bark;
		for(i=0;i<n;i++)
		{
			if(sb[i*2+1]>=0)
				break;
		}
		p->midpoint1	 = i;
		p->midpoint1_4	 = p->midpoint1&(~3);
		p->midpoint1_8	 = p->midpoint1_4&(~7);
		p->midpoint1_16	 = p->midpoint1_8&(~15);
		for(;i<n;i++)
		{
			if(sb[i*2]>=n)
				break;
		}
		p->midpoint2	 = i;
		i = (p->midpoint1+3)&(~3);
		p->midpoint2_4	 = (p->midpoint2-i)&(~3);
		p->midpoint2_8	 = p->midpoint2_4&(~7);
		p->midpoint2_16	 = p->midpoint2_8&(~15);
		p->midpoint2_4	+= i;
		p->midpoint2_8	+= i;
		p->midpoint2_16	+= i;
	}
	p->octsft=_ogg_malloc(n*sizeof(*p->octsft));
	p->octend=_ogg_malloc(n*sizeof(*p->octend));
	p->octpos=_ogg_malloc(n*sizeof(*p->octpos));
	for(i=0;i<n;i++)
	{
		long oc	 = p->octave[i];
		oc	 = oc>>p->shiftoc;

		if(oc>=P_BANDS)oc=P_BANDS-1;
		if(oc<0)oc=0;
		
		p->octsft[i]	 = oc;
		p->octpos[i]	 = ((p->octave[i]+p->octave[i+1])>>1)-p->firstoc;

	}
	for(i=0;i<n;i++)
	{
		long oc=p->octave[i];
		long j = i, k;
		while(i+1<n && p->octave[i+1]==oc){
			i++;
		}
		for(k=j;k<=i;k++)
			p->octend[k] = i;
	}
#if	defined(_OPENMP)
	p->_vp_couple_spoint0 = 0;
	p->_vp_couple_spoint1 = 0;
#endif
#endif														/* SSE Optimize */
}

void _vp_psy_clear(vorbis_look_psy *p){
  int i,j;
  if(p){
    if(p->ath)_ogg_free(p->ath);
    if(p->octave)_ogg_free(p->octave);
    if(p->bark)_ogg_free(p->bark);
    if(p->tonecurves){
      for(i=0;i<P_BANDS;i++){
	for(j=0;j<P_LEVELS;j++){
	  _ogg_free(p->tonecurves[i][j]);
	}
	_ogg_free(p->tonecurves[i]);
      }
      _ogg_free(p->tonecurves);
    }
    if(p->noiseoffset){
      for(i=0;i<P_NOISECURVES;i++){
        _ogg_free(p->noiseoffset[i]);
      }
      _ogg_free(p->noiseoffset);
    }
#ifdef __SSE__												/* SSE Optimize */
    if(p->octsft)_ogg_free(p->octsft);
    if(p->octend)_ogg_free(p->octend);
    if(p->octpos)_ogg_free(p->octpos);
#endif														/* SSE Optimize */
    memset(p,0,sizeof(*p));
  }
}

/* octave/(8*eighth_octave_lines) x scale and dB y scale */
static void seed_curve(float *seed,
		       const float **curves,
		       float amp,
		       int oc, int n,
		       int linesper,float dBoffset){
  int i,post1;
  int seedptr;
  const float *posts,*curve;
#ifdef __SSE__												/* SSE Optimize */
	__m128	SAMP	 = _mm_load_ss(&amp);
#endif														/* SSE Optimize */

  int choice=(int)((amp+dBoffset-P_LEVEL_0)*.1f);
  choice=max(choice,0);
  choice=min(choice,P_LEVELS-1);
  posts=curves[choice];
  curve=posts+2;
  post1=(int)posts[1];
  seedptr=oc+(posts[0]-EHMER_OFFSET)*linesper-(linesper>>1);

#ifdef __SSE__												/* SSE Optimize */
	i	 = posts[0];
	if(seedptr<0)
	{
		int preseedptr	 = seedptr;
		seedptr	 = (8-((-seedptr)&7));
		i	+= ((seedptr-preseedptr)>>3);
	}
	if((post1-i)*8+seedptr>=n)
		post1	 = (n-1-seedptr)/8+i+1;
	{
		int post05	 = ((post1-i)&(~1))+i;
		for(;i<post05;i+=2)
		{
			__m128	XMM0	 = _mm_load_ss(curve+i  );
			__m128	XMM1	 = _mm_load_ss(curve+i+1);
			__m128	XMM2	 = _mm_load_ss(seed+seedptr   );
			__m128	XMM3	 = _mm_load_ss(seed+seedptr+ 8);
			XMM0	 = _mm_add_ss(XMM0, SAMP);
			XMM1	 = _mm_add_ss(XMM1, SAMP);
			XMM0	 = _mm_max_ss(XMM0, XMM2);
			XMM1	 = _mm_max_ss(XMM1, XMM3);
			_mm_store_ss(seed+seedptr   , XMM0);
			_mm_store_ss(seed+seedptr+ 8, XMM1);
			seedptr	+= 16;
		}
		if(post1!=i)
		{
			__m128	XMM0	 = _mm_load_ss(curve+i  );
			__m128	XMM2	 = _mm_load_ss(seed+seedptr   );
			XMM0	 = _mm_add_ss(XMM0, SAMP);
			XMM0	 = _mm_max_ss(XMM0, XMM2);
			_mm_store_ss(seed+seedptr   , XMM0);
		}
	}
#else														/* SSE Optimize */
  for(i=posts[0];i<post1;i++){
    if(seedptr>0){
      float lin=amp+curve[i];
      if(seed[seedptr]<lin)seed[seedptr]=lin;
    }
    seedptr+=linesper;
    if(seedptr>=n)break;
  }
#endif														/* SSE Optimize */
}

static void seed_loop(vorbis_look_psy *p,
		      const float ***curves,
		      const float *f, 
		      const float *flr,
		      float *seed,
		      float specmax){
  vorbis_info_psy *vi=p->vi;
  long n=p->n,i;
  float dBoffset=vi->max_curve_dB-specmax;

  /* prime the working vector with peak values */

  for(i=0;i<n;i++){
    float max=f[i];
#ifdef __SSE__												/* SSE Optimize */
	long	oc;
	long	ei=p->octend[i];
	if(i>ei)
		continue;
	oc	 = p->octave[i];
	while(i<ei)
	{
		i++;
		if(f[i]>max)max	 = f[i];
	}
	
	if(max+6.f>flr[i])
	{
		oc	 = p->octsft[i];
#else
    long oc=p->octave[i];
    while(i+1<n && p->octave[i+1]==oc){
      i++;
      if(f[i]>max)max=f[i];
    }
    
    if(max+6.f>flr[i]){
      oc=oc>>p->shiftoc;

      if(oc>=P_BANDS)oc=P_BANDS-1;
      if(oc<0)oc=0;
#endif

      seed_curve(seed,
		 curves[oc],
		 max,
		 p->octave[i]-p->firstoc,
		 p->total_octave_lines,
		 p->eighth_octave_lines,
		 dBoffset);
    }
  }
}

static void seed_chase(float *seeds, int linesper, long n){
  long  *posstack=alloca(n*sizeof(*posstack));
  float *ampstack=alloca(n*sizeof(*ampstack));
  long   stack=0;
  long   pos=0;
#ifdef __SSE__												/* SSE Optimize */
  long   i=0;

	for(;i<n;i++)
	{
		if(stack<2)
		{
			posstack[stack]=i;
			ampstack[stack++]=seeds[i];
		}
		else
		{
			while(1)
			{
				if(seeds[i]<ampstack[stack-1])
				{
					posstack[stack]=i;
					ampstack[stack++]=seeds[i];
					break;
				}
				else
				{
					if(i<posstack[stack-1]+linesper)
					{
						if(stack>1 && ampstack[stack-1]<=ampstack[stack-2] && i<posstack[stack-2]+linesper)
						{
							/* we completely overlap, making stack-1 irrelevant.  pop it */
							stack--;
LOOP_WITH_CHECK_STACK:
							continue;
						}
					}
					posstack[stack]=i;
					ampstack[stack++]=seeds[i];
					break;
				}
			}
			i	++;
			break;
		}
	}
	for(;i<n;i++)
	{
		while(1)
		{
			if(seeds[i]<ampstack[stack-1])
			{
				posstack[stack]=i;
				ampstack[stack++]=seeds[i];
				break;
			}
			else
			{
				if(i<posstack[stack-1]+linesper)
				{
					if(ampstack[stack-1]<=ampstack[stack-2] && i<posstack[stack-2]+linesper)
					{
						/* we completely overlap, making stack-1 irrelevant.  pop it */
						stack--;
						if(stack<2)
						{
							goto LOOP_WITH_CHECK_STACK;
						}
						else
							continue;
					}
				}
				posstack[stack]=i;
				ampstack[stack++]=seeds[i];
				break;
			}
		}
	}
#else														/* SSE Optimize */
  long   i;

  for(i=0;i<n;i++){
    if(stack<2){
      posstack[stack]=i;
      ampstack[stack++]=seeds[i];
    }else{
      while(1){
	if(seeds[i]<ampstack[stack-1]){
	  posstack[stack]=i;
	  ampstack[stack++]=seeds[i];
	  break;
	}else{
	  if(i<posstack[stack-1]+linesper){
	    if(stack>1 && ampstack[stack-1]<=ampstack[stack-2] &&
	       i<posstack[stack-2]+linesper){
	      /* we completely overlap, making stack-1 irrelevant.  pop it */
	      stack--;
	      continue;
	    }
	  }
	  posstack[stack]=i;
	  ampstack[stack++]=seeds[i];
	  break;

	}
      }
    }
  }
#endif														/* SSE Optimize */

  /* the stack now contains only the positions that are relevant. Scan
     'em straight through */

#ifdef __SSE__												/* SSE Optimize */
	for(i=0;i<stack-1;i++)
	{
		long endpos;
		if(ampstack[i+1]>ampstack[i])
		{
			endpos	 = posstack[i+1];
		}
		else
		{
			endpos	 = posstack[i]+linesper+1; /* +1 is important, else bin 0 is
					discarded in short frames */
		}
		if(endpos>n)
			endpos	 = n;
		for(;pos<endpos;pos++)
			seeds[pos]=ampstack[i];
	}
	if(i<stack)
	{
		long endpos;
		endpos	 = posstack[i]+linesper+1; /* +1 is important, else bin 0 is
				discarded in short frames */
		if(endpos>n)
			endpos	 = n;
		for(;pos<endpos;pos++)
			seeds[pos]=ampstack[i];
	}
#else														/* SSE Optimize */
  for(i=0;i<stack;i++){
    long endpos;
    if(i<stack-1 && ampstack[i+1]>ampstack[i]){
      endpos=posstack[i+1];
    }else{
      endpos=posstack[i]+linesper+1; /* +1 is important, else bin 0 is
					discarded in short frames */
    }
    if(endpos>n)endpos=n;
    for(;pos<endpos;pos++)
      seeds[pos]=ampstack[i];
  }
#endif														/* SSE Optimize */
  
  /* there.  Linear time.  I now remember this was on a problem set I
     had in Grad Skool... I didn't solve it at the time ;-) */

}

/* bleaugh, this is more complicated than it needs to be */
#include<stdio.h>
static void max_seeds(vorbis_look_psy *p,
		      float *seed,
		      float *flr){
#ifdef __SSE__												/* SSE Optimize */
	long	n	 = p->total_octave_lines;
	int		linesper	 = p->eighth_octave_lines;
	long	linpos	 = 0;
	long	pos;
	float*	TEMP	 = (float*)_ogg_alloca(sizeof(float)*p->n);
	
	seed_chase(seed,linesper,n); /* for masking */
	{
		__m128	PVAL	 = _mm_set_ps1(p->vi->tone_abs_limit);
		long ln	 = n&(~15);
		for(pos=0;pos<ln;pos+=16)
		{
			__m128	XMM0	 = _mm_load_ps(seed+pos   );
			__m128	XMM1	 = _mm_load_ps(seed+pos+ 4);
			__m128	XMM2	 = _mm_load_ps(seed+pos+ 8);
			__m128	XMM3	 = _mm_load_ps(seed+pos+12);
			XMM0	 = _mm_min_ps(XMM0, PVAL);
			XMM1	 = _mm_min_ps(XMM1, PVAL);
			XMM2	 = _mm_min_ps(XMM2, PVAL);
			XMM3	 = _mm_min_ps(XMM3, PVAL);
			_mm_store_ps(seed+pos   , XMM0);
			_mm_store_ps(seed+pos+ 4, XMM1);
			_mm_store_ps(seed+pos+ 8, XMM2);
			_mm_store_ps(seed+pos+12, XMM3);
		}
		ln	 = n&(~7);
		for(;pos<ln;pos+=8)
		{
			__m128	XMM0	 = _mm_load_ps(seed+pos   );
			__m128	XMM1	 = _mm_load_ps(seed+pos+ 4);
			XMM0	 = _mm_min_ps(XMM0, PVAL);
			XMM1	 = _mm_min_ps(XMM1, PVAL);
			_mm_store_ps(seed+pos   , XMM0);
			_mm_store_ps(seed+pos+ 4, XMM1);
		}
		ln	 = n&(~3);
		for(;pos<ln;pos+=4)
		{
			__m128	XMM0	 = _mm_load_ps(seed+pos   );
			XMM0	 = _mm_min_ps(XMM0, PVAL);
			_mm_store_ps(seed+pos   , XMM0);
		}
		for(;pos<n;pos++)
		{
			__m128	XMM0	 = _mm_load_ss(seed+pos   );
			XMM0	 = _mm_min_ss(XMM0, PVAL);
			_mm_store_ss(seed+pos, XMM0);
		}
	}
	pos	 = p->octave[0]-p->firstoc-(linesper>>1);
	if(linpos+1<p->n)
	{
		float minV	 = seed[pos];
		long end	 = p->octpos[linpos];
		while(pos+1<=end)
		{
			pos	++;
			if((seed[pos]>NEGINF && seed[pos]<minV) || minV==NEGINF)
				minV	 = seed[pos];
		}
		end	 = pos+p->firstoc;
		for(;linpos<p->n&&p->octave[linpos]<=end;)
		{
			int ep = p->octend[linpos];
			for(;linpos<=ep;linpos++)
				TEMP[linpos]	 = minV;
		}
	}
	while(linpos+1<p->n)
	{
		float minV	 = seed[pos];
		long end	 = p->octpos[linpos];
		while(pos+1<=end)
		{
			pos	++;
			if(seed[pos]<minV)
				minV	 = seed[pos];
		}
		end	 = pos+p->firstoc;
		for(;linpos<p->n&&p->octave[linpos]<=end;)
		{
			int ep = p->octend[linpos];
			for(;linpos<=ep;linpos++)
				TEMP[linpos]	 = minV;
		}
	}
	
	{
		float minV	 = seed[p->total_octave_lines-1];
		for(;linpos<p->n;linpos++)
			TEMP[linpos]	 = minV;
	}
	{
		for(pos=0;pos<p->n;pos+=16)
		{
			__m128	XMM0	 = _mm_load_ps(flr+pos    );
			__m128	XMM4	 = _mm_load_ps(TEMP+pos   );
			__m128	XMM1	 = _mm_load_ps(flr+pos+  4);
			__m128	XMM5	 = _mm_load_ps(TEMP+pos+ 4);
			__m128	XMM2	 = _mm_load_ps(flr+pos+  8);
			__m128	XMM6	 = _mm_load_ps(TEMP+pos+ 8);
			__m128	XMM3	 = _mm_load_ps(flr+pos+ 12);
			__m128	XMM7	 = _mm_load_ps(TEMP+pos+12);
			XMM0	 = _mm_max_ps(XMM0, XMM4);
			XMM1	 = _mm_max_ps(XMM1, XMM5);
			XMM2	 = _mm_max_ps(XMM2, XMM6);
			XMM3	 = _mm_max_ps(XMM3, XMM7);
			_mm_store_ps(flr+pos   , XMM0);
			_mm_store_ps(flr+pos+ 4, XMM1);
			_mm_store_ps(flr+pos+ 8, XMM2);
			_mm_store_ps(flr+pos+12, XMM3);
		}
	}
#else														/* SSE Optimize */
  long   n=p->total_octave_lines;
  int    linesper=p->eighth_octave_lines;
  long   linpos=0;
  long   pos;

  seed_chase(seed,linesper,n); /* for masking */
 
  pos=p->octave[0]-p->firstoc-(linesper>>1);

  while(linpos+1<p->n){
    float minV=seed[pos];
    long end=((p->octave[linpos]+p->octave[linpos+1])>>1)-p->firstoc;
    if(minV>p->vi->tone_abs_limit)minV=p->vi->tone_abs_limit;
    while(pos+1<=end){
      pos++;
      if((seed[pos]>NEGINF && seed[pos]<minV) || minV==NEGINF)
	minV=seed[pos];
    }
    
    end=pos+p->firstoc;
    for(;linpos<p->n && p->octave[linpos]<=end;linpos++)
      if(flr[linpos]<minV)flr[linpos]=minV;
  }
  
  {
    float minV=seed[p->total_octave_lines-1];
    for(;linpos<p->n;linpos++)
      if(flr[linpos]<minV)flr[linpos]=minV;
  }
  
#endif														/* SSE Optimize */
}

#ifdef __SSE__												/* SSE Optimize */
/*
	A	 = tY * tXX - tX * tXY;
	B	 = tN * tXY - tX * tY;
	D	 = tN * tXX - tX * tX;
	R	 = (A + x * B) / D;

	Input
	TN		(N3 ,N2 ,N1 ,N0 )
	XMM0	 = (XY0,Y0 ,XX0,X0 )
	XMM1	 = (XY1,Y1 ,XX1,X1 )
	XMM4	 = (XY2,Y2 ,XX2,X2 )
	XMM3	 = (XY3,Y3 ,XX3,X3 )

	Phase 1.

	Phase 2.
	XMM0	 = 	(X3 ,X2 ,X1 ,X0 )
	XMM1	 = 	(XX3,XX2,XX1,XX0)
	XMM2	 = 	(Y3 ,Y2 ,Y1 ,Y0 )
	XMM3	 = 	(XY3,XY2,XY1,XY0)

	Phase 3.
	XMM4	 = Y*XX
	XMM5	 = X*XY
	XMM6	 = XY*TN
	XMM7	 = X*Y

	Phase 4.
	XMM4	 = Y*XX - X*XY	... A
	XMM5	 = XY*TN - X*Y	... B
	XMM6	 = XX*TN
	XMM7	 = X*X
	XMM1	 = XX*TN - X*X	... D
	
	Phase 5.
	XMM4	 = PX*B
	XMM4	 = PX*B+A
	XMM4	 = (A+PX*B)/D
*/
#define bark_noise_hybridmp_SSE_SUBC()										\
{																			\
	__m128 XMM2, XMM5, XMM6, XMM7;											\
	XMM2 = XMM0;															\
	XMM5 = XMM4;															\
	XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(1,0,1,0));			\
	XMM2	 = _mm_shuffle_ps(XMM2, XMM1, _MM_SHUFFLE(3,2,3,2));			\
	XMM4	 = _mm_shuffle_ps(XMM4, XMM3, _MM_SHUFFLE(1,0,1,0));			\
	XMM5	 = _mm_shuffle_ps(XMM5, XMM3, _MM_SHUFFLE(3,2,3,2));			\
	XMM1 = XMM0;															\
	XMM3 = XMM2;															\
	XMM0	 = _mm_shuffle_ps(XMM0, XMM4, _MM_SHUFFLE(2,0,2,0));			\
	XMM1	 = _mm_shuffle_ps(XMM1, XMM4, _MM_SHUFFLE(3,1,3,1));			\
	XMM2	 = _mm_shuffle_ps(XMM2, XMM5, _MM_SHUFFLE(2,0,2,0));			\
	XMM3	 = _mm_shuffle_ps(XMM3, XMM5, _MM_SHUFFLE(3,1,3,1));			\
	XMM4 = XMM2;															\
	XMM5 = XMM0;															\
	XMM6 = XMM3;															\
	XMM7 = XMM0;															\
	XMM4 = _mm_mul_ps(XMM4, XMM1);											\
	XMM5 = _mm_mul_ps(XMM5, XMM3);											\
	XMM3 = _mm_load_ps(findex+i);											\
	XMM6 = _mm_mul_ps(XMM6, TN.ps);											\
	XMM1 = _mm_mul_ps(XMM1, TN.ps);											\
	XMM7 = _mm_mul_ps(XMM7, XMM2);											\
	XMM0 = _mm_mul_ps(XMM0, XMM0);											\
	XMM4 = _mm_sub_ps(XMM4, XMM5);											\
	XMM6 = _mm_sub_ps(XMM6, XMM7);											\
	XMM1 = _mm_sub_ps(XMM1, XMM0);											\
	XMM6 = _mm_mul_ps(XMM6, XMM3);											\
	XMM3 = _mm_rcp_ps(XMM1);												\
	XMM4 = _mm_add_ps(XMM4, XMM6);											\
	XMM1 = _mm_mul_ps(XMM1, XMM3);											\
	XMM1 = _mm_mul_ps(XMM1, XMM3);											\
	XMM3 = _mm_add_ps(XMM3, XMM3);											\
	XMM3 = _mm_sub_ps(XMM3, XMM1);											\
	XMM4 = _mm_mul_ps(XMM4, XMM3);											\
}
#define bark_noise_hybridmp_SSE_SUBC2()										\
{																			\
	__m128 XMM2, XMM5, XMM6, XMM7;											\
	XMM2 = XMM0;															\
	XMM5 = XMM4;															\
	XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(1,0,1,0));			\
	XMM2	 = _mm_shuffle_ps(XMM2, XMM1, _MM_SHUFFLE(3,2,3,2));			\
	XMM4	 = _mm_shuffle_ps(XMM4, XMM3, _MM_SHUFFLE(1,0,1,0));			\
	XMM5	 = _mm_shuffle_ps(XMM5, XMM3, _MM_SHUFFLE(3,2,3,2));			\
	XMM1 = XMM0;															\
	XMM3 = XMM2;															\
	XMM0	 = _mm_shuffle_ps(XMM0, XMM4, _MM_SHUFFLE(2,0,2,0));			\
	XMM1	 = _mm_shuffle_ps(XMM1, XMM4, _MM_SHUFFLE(3,1,3,1));			\
	XMM2	 = _mm_shuffle_ps(XMM2, XMM5, _MM_SHUFFLE(2,0,2,0));			\
	XMM3	 = _mm_shuffle_ps(XMM3, XMM5, _MM_SHUFFLE(3,1,3,1));			\
	XMM4 = XMM2;															\
	XMM5 = XMM0;															\
	XMM6 = XMM3;															\
	XMM7 = XMM0;															\
	XMM4 = _mm_mul_ps(XMM4, XMM1);											\
	XMM5 = _mm_mul_ps(XMM5, XMM3);											\
	XMM3 = _mm_load_ps(findex+i);											\
	XMM6 = _mm_mul_ps(XMM6, TN.ps);											\
	XMM1 = _mm_mul_ps(XMM1, TN.ps);											\
	XMM7 = _mm_mul_ps(XMM7, XMM2);											\
	XMM0 = _mm_mul_ps(XMM0, XMM0);											\
	XMM4 = _mm_sub_ps(XMM4, XMM5);											\
	XMM6 = _mm_sub_ps(XMM6, XMM7);											\
	XMM1 = _mm_sub_ps(XMM1, XMM0);											\
	PA	 = XMM4;															\
	PB	 = XMM6;															\
	XMM6 = _mm_mul_ps(XMM6, XMM3);											\
	XMM3 = _mm_rcp_ps(XMM1);												\
	XMM4 = _mm_add_ps(XMM4, XMM6);											\
	XMM1 = _mm_mul_ps(XMM1, XMM3);											\
	XMM1 = _mm_mul_ps(XMM1, XMM3);											\
	XMM3 = _mm_add_ps(XMM3, XMM3);											\
	XMM3 = _mm_sub_ps(XMM3, XMM1);											\
	PD	 = XMM3;															\
	XMM4 = _mm_mul_ps(XMM4, XMM3);											\
}
#endif														/* SSE Optimize */

#ifdef __SSE__												/* SSE Optimize */
static void bark_noise_hybridmp(vorbis_look_psy *p,
								const float *f,
								float *noise,
								const float offset,
								const int fixed,
								float *work,
								float *tf){
	int		n = p->n;
	float	*N		 = work;
	__m128	*XXYY	 = (__m128*)(N+n);
	float	*xxyy	 = N+n;
	short	*sb	 = (short*)p->bark;
	
	int		i, j;
	int		lo, hi;
	int		midpoint1, midpoint2;
	float	tN, tX, tXX, tY, tXY;
	float	R, A, B, D;
	float	x;
	float	*TN = N;
	__m128	*TXXYY = XXYY;
	
	__m128	OFFSET;
	__m128	PXXYY	 = _mm_setzero_ps();
	__m128	PA, PB, PD;
	_MM_ALIGN16 __m128	TEMP[16];
	int	p0, p1;
	
	// Phase 1
	_mm_prefetch((const char*)(f     ), _MM_HINT_NTA);
	_mm_prefetch((const char*)(findex2     ), _MM_HINT_NTA);
	_mm_prefetch((const char*)(f  +16), _MM_HINT_NTA);
	_mm_prefetch((const char*)(findex2  +16), _MM_HINT_NTA);
	OFFSET	 = _mm_set_ps1(offset);
	{
		__m128	XMM0	 = _mm_load_ps(f   );
		__m128	XMM1	 = _mm_load_ps(f+ 4);
		__m128	XMM2	 = _mm_load_ps(f+ 8);
		__m128	XMM3	 = _mm_load_ps(f+12);
		__m128	XMM4, XMM5, XMM6, XMM7;
		XMM4	 = OFFSET;
		XMM5	 = _mm_load_ps(PFV_1);
		XMM0	 = _mm_add_ps(XMM0, XMM4);
		XMM1	 = _mm_add_ps(XMM1, XMM4);
		XMM2	 = _mm_add_ps(XMM2, XMM4);
		XMM3	 = _mm_add_ps(XMM3, XMM4);
		XMM0	 = _mm_max_ps(XMM0, XMM5);
		XMM1	 = _mm_max_ps(XMM1, XMM5);
		XMM2	 = _mm_max_ps(XMM2, XMM5);
		XMM3	 = _mm_max_ps(XMM3, XMM5);
		XMM4	 = XMM0;
		XMM5	 = XMM1;
		XMM6	 = XMM2;
		XMM7	 = XMM3;
		XMM0	 = _mm_mul_ps(XMM0, XMM0);
		XMM1	 = _mm_mul_ps(XMM1, XMM1);
		XMM2	 = _mm_mul_ps(XMM2, XMM2);
		XMM3	 = _mm_mul_ps(XMM3, XMM3);
		_mm_store_ps(TN   , XMM0);	/* N */
		_mm_store_ps(TN+ 4, XMM1);
		_mm_store_ps(TN+ 8, XMM2);
		_mm_store_ps(TN+12, XMM3);
		XMM0	 = _mm_mul_ps(XMM0, XMM4);
		XMM1	 = _mm_mul_ps(XMM1, XMM5);
		XMM2	 = _mm_mul_ps(XMM2, XMM6);
		XMM3	 = _mm_mul_ps(XMM3, XMM7);
		TEMP[ 1]	 = XMM0;	/* Y */
		PXXYY	 = _mm_move_ss(PXXYY, TEMP[1]);
		XMM4	 = _mm_load_ps(findex   );
		TEMP[ 5]	 = XMM1;
		XMM5	 = _mm_load_ps(findex+ 4);
		TEMP[ 9]	 = XMM2;
		XMM6	 = _mm_load_ps(findex+ 8);
		TEMP[13]	 = XMM3;
		XMM7	 = _mm_load_ps(findex+12);
		XMM0	 = _mm_mul_ps(XMM0, XMM4);
		XMM1	 = _mm_mul_ps(XMM1, XMM5);
		XMM2	 = _mm_mul_ps(XMM2, XMM6);
		XMM3	 = _mm_mul_ps(XMM3, XMM7);
		TEMP[ 3]	 = XMM0;	/* XY */
		TEMP[ 7]	 = XMM1;
		TEMP[11]	 = XMM2;
		TEMP[15]	 = XMM3;
		XMM0	 = _mm_load_ps(TN   );	/* N */
		XMM1	 = _mm_load_ps(TN+ 4);
		XMM2	 = _mm_load_ps(TN+ 8);
		XMM3	 = _mm_load_ps(TN+12);
		XMM4	 = _mm_mul_ps(XMM4, XMM0);
		XMM5	 = _mm_mul_ps(XMM5, XMM1);
		XMM6	 = _mm_mul_ps(XMM6, XMM2);
		XMM7	 = _mm_mul_ps(XMM7, XMM3);
		TEMP[ 0]	 = XMM4;	/* X */
		TEMP[ 4]	 = XMM5;
		TEMP[ 8]	 = XMM6;
		TEMP[12]	 = XMM7;
		XMM4	 = _mm_load_ps(findex2   );
		XMM5	 = _mm_load_ps(findex2+ 4);
		XMM6	 = _mm_load_ps(findex2+ 8);
		XMM7	 = _mm_load_ps(findex2+12);
		XMM0	 = _mm_mul_ps(XMM0, XMM4);
		XMM4	 = TEMP[0];	// X
		XMM1	 = _mm_mul_ps(XMM1, XMM5);
		XMM5	 = TEMP[1];	// Y
		XMM2	 = _mm_mul_ps(XMM2, XMM6);
		XMM6	 = XMM0;	// XX
		XMM3	 = _mm_mul_ps(XMM3, XMM7);
		XMM7	 = TEMP[3];	// XY
		XMM0	 = XMM4;
		TEMP[ 6]	 = XMM1;
		XMM1	 = XMM5;
		// i=0-3
		// PXXYY	 = (0, 0, 0, Y^2)
		XMM4	 = _mm_shuffle_ps(XMM4, XMM6, _MM_SHUFFLE(1,0,1,0));
		XMM0	 = _mm_shuffle_ps(XMM0, XMM6, _MM_SHUFFLE(3,2,3,2));
		TEMP[10]	 = XMM2;
		XMM5	 = _mm_shuffle_ps(XMM5, XMM7, _MM_SHUFFLE(1,0,1,0));
		XMM1	 = _mm_shuffle_ps(XMM1, XMM7, _MM_SHUFFLE(3,2,3,2));
		TEMP[14]	 = XMM3;
		XMM6	 = XMM4;
		XMM7	 = XMM0;
		XMM4	 = _mm_shuffle_ps(XMM4, XMM5, _MM_SHUFFLE(2,0,2,0));
		XMM6	 = _mm_shuffle_ps(XMM6, XMM5, _MM_SHUFFLE(3,1,3,1));
		XMM5	 = TEMP[ 4];	// X
		XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(2,0,2,0));
		XMM7	 = _mm_shuffle_ps(XMM7, XMM1, _MM_SHUFFLE(3,1,3,1));
		XMM1	 = TEMP[ 5];	// Y
		// XXYY[i+0]	 = (XY,  Y, XX,  X)	 = (0, Y^3, 0, 0)
		// To Fix (0, Y^3*.5f, 0, Y^2*.5f)
		XMM4	 = _mm_add_ps(XMM4, PXXYY);
		TN[ 0]	*= 0.5;
		XMM4	 = _mm_mul_ps(XMM4, PM128(PFV_0P5));
		TN[ 1]	+= TN[ 0];
		XMM6	 = _mm_add_ps(XMM6, XMM4);
		TN[ 2]	+= TN[ 1];
		XMM0	 = _mm_add_ps(XMM0, XMM6);
		TN[ 3]	+= TN[ 2];
		XMM7	 = _mm_add_ps(XMM7, XMM0);
		TXXYY[ 0]	 = XMM4;
		XMM4	 = TEMP[ 6];	// XX
		TXXYY[ 1]	 = XMM6;
		XMM6	 = TEMP[ 7];	// XY
		TXXYY[ 2]	 = XMM0;
		XMM0	 = XMM5;
		TXXYY[ 3]	 = XMM7;
		XMM7	 = XMM1;
		XMM5	 = _mm_shuffle_ps(XMM5, XMM4, _MM_SHUFFLE(1,0,1,0));
		XMM0	 = _mm_shuffle_ps(XMM0, XMM4, _MM_SHUFFLE(3,2,3,2));
		XMM1	 = _mm_shuffle_ps(XMM1, XMM6, _MM_SHUFFLE(1,0,1,0));
		XMM7	 = _mm_shuffle_ps(XMM7, XMM6, _MM_SHUFFLE(3,2,3,2));
		XMM4	 = XMM5;
		XMM6	 = XMM0;
		XMM5	 = _mm_shuffle_ps(XMM5, XMM1, _MM_SHUFFLE(2,0,2,0));
		XMM4	 = _mm_shuffle_ps(XMM4, XMM1, _MM_SHUFFLE(3,1,3,1));
		XMM1	 = TEMP[ 8];	// X
		XMM0	 = _mm_shuffle_ps(XMM0, XMM7, _MM_SHUFFLE(2,0,2,0));
		XMM6	 = _mm_shuffle_ps(XMM6, XMM7, _MM_SHUFFLE(3,1,3,1));
		XMM7	 = TEMP[ 9];	// Y
		XMM5	 = _mm_add_ps(XMM5, TXXYY[ 3]);
		TN[ 4]	+= TN[ 3];
		XMM4	 = _mm_add_ps(XMM4, XMM5);
		TN[ 5]	+= TN[ 4];
		XMM0	 = _mm_add_ps(XMM0, XMM4);
		TN[ 6]	+= TN[ 5];
		XMM6	 = _mm_add_ps(XMM6, XMM0);
		TN[ 7]	+= TN[ 6];
		TXXYY[ 4]	 = XMM5;
		XMM5	 = TEMP[10];	// XX
		TXXYY[ 5]	 = XMM4;
		XMM4	 = TEMP[11];	// XY
		TXXYY[ 6]	 = XMM0;
		XMM0	 = XMM1;
		TXXYY[ 7]	 = XMM6;
		XMM6	 = XMM7;
		XMM1	 = _mm_shuffle_ps(XMM1, XMM5, _MM_SHUFFLE(1,0,1,0));
		XMM0	 = _mm_shuffle_ps(XMM0, XMM5, _MM_SHUFFLE(3,2,3,2));
		XMM7	 = _mm_shuffle_ps(XMM7, XMM4, _MM_SHUFFLE(1,0,1,0));
		XMM6	 = _mm_shuffle_ps(XMM6, XMM4, _MM_SHUFFLE(3,2,3,2));
		XMM5	 = XMM1;
		XMM4	 = XMM0;
		XMM1	 = _mm_shuffle_ps(XMM1, XMM7, _MM_SHUFFLE(2,0,2,0));
		XMM5	 = _mm_shuffle_ps(XMM5, XMM7, _MM_SHUFFLE(3,1,3,1));
		XMM7	 = TEMP[12];	// X
		XMM0	 = _mm_shuffle_ps(XMM0, XMM6, _MM_SHUFFLE(2,0,2,0));
		XMM4	 = _mm_shuffle_ps(XMM4, XMM6, _MM_SHUFFLE(3,1,3,1));
		XMM6	 = TEMP[13];	// Y
		XMM1	 = _mm_add_ps(XMM1, TXXYY[ 7]);
		TN[ 8]	+= TN[ 7];
		XMM5	 = _mm_add_ps(XMM5, XMM1);
		TN[ 9]	+= TN[ 8];
		XMM0	 = _mm_add_ps(XMM0, XMM5);
		TN[10]	+= TN[ 9];
		XMM4	 = _mm_add_ps(XMM4, XMM0);
		TN[11]	+= TN[10];
		TXXYY[ 8]	 = XMM1;
		XMM1	 = TEMP[14];	// XX
		TXXYY[ 9]	 = XMM5;
		XMM5	 = TEMP[15];	// XY
		TXXYY[10]	 = XMM0;
		XMM0	 = XMM7;
		TXXYY[11]	 = XMM4;
		XMM4	 = XMM6;
		XMM7	 = _mm_shuffle_ps(XMM7, XMM1, _MM_SHUFFLE(1,0,1,0));
		XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(3,2,3,2));
		XMM6	 = _mm_shuffle_ps(XMM6, XMM5, _MM_SHUFFLE(1,0,1,0));
		XMM4	 = _mm_shuffle_ps(XMM4, XMM5, _MM_SHUFFLE(3,2,3,2));
		XMM1	 = XMM7;
		XMM5	 = XMM0;
		XMM7	 = _mm_shuffle_ps(XMM7, XMM6, _MM_SHUFFLE(2,0,2,0));
		XMM1	 = _mm_shuffle_ps(XMM1, XMM6, _MM_SHUFFLE(3,1,3,1));
		XMM0	 = _mm_shuffle_ps(XMM0, XMM4, _MM_SHUFFLE(2,0,2,0));
		XMM5	 = _mm_shuffle_ps(XMM5, XMM4, _MM_SHUFFLE(3,1,3,1));
		XMM7	 = _mm_add_ps(XMM7, TXXYY[11]);
		TN[12]	+= TN[11];
		XMM1	 = _mm_add_ps(XMM1, XMM7);
		TN[13]	+= TN[12];
		XMM0	 = _mm_add_ps(XMM0, XMM1);
		TN[14]	+= TN[13];
		XMM5	 = _mm_add_ps(XMM5, XMM0);
		TN[15]	+= TN[14];
		TXXYY[12]	 = XMM7;
		TXXYY[13]	 = XMM1;
		TXXYY[14]	 = XMM0;
		TXXYY[15]	 = XMM5;
		TN		+= 16;
		TXXYY	+= 16;
	}
	for(i=16;i<n;i+=16)
	{
		__m128	XMM0, XMM1, XMM2, XMM3;
		__m128	XMM4, XMM5, XMM6, XMM7;
		_mm_prefetch((const char*)(f+i+16), _MM_HINT_NTA);
		_mm_prefetch((const char*)(findex2+i+16), _MM_HINT_NTA);
		XMM0	 = _mm_load_ps(f+i   );
		XMM1	 = _mm_load_ps(f+i+ 4);
		XMM2	 = _mm_load_ps(f+i+ 8);
		XMM3	 = _mm_load_ps(f+i+12);
		XMM4	 = OFFSET;
		XMM5	 = _mm_load_ps(PFV_1);
		XMM0	 = _mm_add_ps(XMM0, XMM4);
		XMM1	 = _mm_add_ps(XMM1, XMM4);
		XMM2	 = _mm_add_ps(XMM2, XMM4);
		XMM3	 = _mm_add_ps(XMM3, XMM4);
		XMM0	 = _mm_max_ps(XMM0, XMM5);
		XMM1	 = _mm_max_ps(XMM1, XMM5);
		XMM2	 = _mm_max_ps(XMM2, XMM5);
		XMM3	 = _mm_max_ps(XMM3, XMM5);
		XMM4	 = XMM0;
		XMM5	 = XMM1;
		XMM6	 = XMM2;
		XMM7	 = XMM3;
		XMM0	 = _mm_mul_ps(XMM0, XMM0);
		XMM1	 = _mm_mul_ps(XMM1, XMM1);
		XMM2	 = _mm_mul_ps(XMM2, XMM2);
		XMM3	 = _mm_mul_ps(XMM3, XMM3);
		_mm_store_ps(TN   , XMM0);
		_mm_store_ps(TN+ 4, XMM1);
		_mm_store_ps(TN+ 8, XMM2);
		_mm_store_ps(TN+12, XMM3);
		XMM0	 = _mm_mul_ps(XMM0, XMM4);
		XMM1	 = _mm_mul_ps(XMM1, XMM5);
		XMM2	 = _mm_mul_ps(XMM2, XMM6);
		XMM3	 = _mm_mul_ps(XMM3, XMM7);
		TEMP[ 1]	 = XMM0;	/* Y */
		XMM4	 = _mm_load_ps(findex+i   );
		TEMP[ 5]	 = XMM1;
		XMM5	 = _mm_load_ps(findex+i+ 4);
		TEMP[ 9]	 = XMM2;
		XMM6	 = _mm_load_ps(findex+i+ 8);
		TEMP[13]	 = XMM3;
		XMM7	 = _mm_load_ps(findex+i+12);
		XMM0	 = _mm_mul_ps(XMM0, XMM4);
		XMM1	 = _mm_mul_ps(XMM1, XMM5);
		XMM2	 = _mm_mul_ps(XMM2, XMM6);
		XMM3	 = _mm_mul_ps(XMM3, XMM7);
		TEMP[ 3]	 = XMM0;	/* XY */
		TEMP[ 7]	 = XMM1;
		TEMP[11]	 = XMM2;
		TEMP[15]	 = XMM3;
		XMM0	 = _mm_load_ps(TN   );	/* N */
		XMM1	 = _mm_load_ps(TN+ 4);
		XMM2	 = _mm_load_ps(TN+ 8);
		XMM3	 = _mm_load_ps(TN+12);
		XMM4	 = _mm_mul_ps(XMM4, XMM0);
		XMM5	 = _mm_mul_ps(XMM5, XMM1);
		XMM6	 = _mm_mul_ps(XMM6, XMM2);
		XMM7	 = _mm_mul_ps(XMM7, XMM3);
		TEMP[ 0]	 = XMM4;	/* X */
		TEMP[ 4]	 = XMM5;
		TEMP[ 8]	 = XMM6;
		TEMP[12]	 = XMM7;
		XMM4	 = _mm_load_ps(findex2+i   );
		XMM5	 = _mm_load_ps(findex2+i+ 4);
		XMM6	 = _mm_load_ps(findex2+i+ 8);
		XMM7	 = _mm_load_ps(findex2+i+12);
		XMM0	 = _mm_mul_ps(XMM0, XMM4);
		XMM4	 = TEMP[ 0];	// X
		XMM1	 = _mm_mul_ps(XMM1, XMM5);
		XMM5	 = TEMP[ 1];	// Y
		XMM2	 = _mm_mul_ps(XMM2, XMM6);
		XMM6	 = XMM0;	/* XX */
		XMM0	 = XMM4;
		XMM3	 = _mm_mul_ps(XMM3, XMM7);
		XMM7	 = TEMP[ 3];	// XY
		TEMP[ 6]	 = XMM1;
		XMM1	 = XMM5;
		XMM4	 = _mm_shuffle_ps(XMM4, XMM6, _MM_SHUFFLE(1,0,1,0));
		XMM0	 = _mm_shuffle_ps(XMM0, XMM6, _MM_SHUFFLE(3,2,3,2));
		TEMP[10]	 = XMM2;
		XMM5	 = _mm_shuffle_ps(XMM5, XMM7, _MM_SHUFFLE(1,0,1,0));
		XMM1	 = _mm_shuffle_ps(XMM1, XMM7, _MM_SHUFFLE(3,2,3,2));
		TEMP[14]	 = XMM3;
		XMM6	 = XMM4;
		XMM7	 = XMM0;
		XMM4	 = _mm_shuffle_ps(XMM4, XMM5, _MM_SHUFFLE(2,0,2,0));
		XMM6	 = _mm_shuffle_ps(XMM6, XMM5, _MM_SHUFFLE(3,1,3,1));
		XMM5	 = TEMP[ 4];	// X
		XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(2,0,2,0));
		XMM7	 = _mm_shuffle_ps(XMM7, XMM1, _MM_SHUFFLE(3,1,3,1));
		XMM1	 = TEMP[ 5];	// Y
		XMM4	 = _mm_add_ps(XMM4, TXXYY[-1]);
		TN[ 0]	+= TN[-1];
		XMM6	 = _mm_add_ps(XMM6, XMM4);
		TN[ 1]	+= TN[ 0];
		XMM0	 = _mm_add_ps(XMM0, XMM6);
		TN[ 2]	+= TN[ 1];
		XMM7	 = _mm_add_ps(XMM7, XMM0);
		TN[ 3]	+= TN[ 2];
		TXXYY[ 0]	 = XMM4;
		XMM4	 = TEMP[ 6];	// XX
		TXXYY[ 1]	 = XMM6;
		XMM6	 = TEMP[ 7];	// XY
		TXXYY[ 2]	 = XMM0;
		XMM0	 = XMM5;
		TXXYY[ 3]	 = XMM7;
		XMM7	 = XMM1;
		XMM5	 = _mm_shuffle_ps(XMM5, XMM4, _MM_SHUFFLE(1,0,1,0));
		XMM0	 = _mm_shuffle_ps(XMM0, XMM4, _MM_SHUFFLE(3,2,3,2));
		XMM1	 = _mm_shuffle_ps(XMM1, XMM6, _MM_SHUFFLE(1,0,1,0));
		XMM7	 = _mm_shuffle_ps(XMM7, XMM6, _MM_SHUFFLE(3,2,3,2));
		XMM4	 = XMM5;
		XMM6	 = XMM0;
		XMM5	 = _mm_shuffle_ps(XMM5, XMM1, _MM_SHUFFLE(2,0,2,0));
		XMM4	 = _mm_shuffle_ps(XMM4, XMM1, _MM_SHUFFLE(3,1,3,1));
		XMM1	 = TEMP[ 8];	// X
		XMM0	 = _mm_shuffle_ps(XMM0, XMM7, _MM_SHUFFLE(2,0,2,0));
		XMM6	 = _mm_shuffle_ps(XMM6, XMM7, _MM_SHUFFLE(3,1,3,1));
		XMM7	 = TEMP[ 9];	// Y
		XMM5	 = _mm_add_ps(XMM5, TXXYY[ 3]);
		TN[ 4]	+= TN[ 3];
		XMM4	 = _mm_add_ps(XMM4, XMM5);
		TN[ 5]	+= TN[ 4];
		XMM0	 = _mm_add_ps(XMM0, XMM4);
		TN[ 6]	+= TN[ 5];
		XMM6	 = _mm_add_ps(XMM6, XMM0);
		TN[ 7]	+= TN[ 6];
		TXXYY[ 4]	 = XMM5;
		XMM5	 = TEMP[10];	// XX
		TXXYY[ 5]	 = XMM4;
		XMM4	 = TEMP[11];	// XY
		TXXYY[ 6]	 = XMM0;
		XMM0	 = XMM1;
		TXXYY[ 7]	 = XMM6;
		XMM6	 = XMM7;
		XMM1	 = _mm_shuffle_ps(XMM1, XMM5, _MM_SHUFFLE(1,0,1,0));
		XMM0	 = _mm_shuffle_ps(XMM0, XMM5, _MM_SHUFFLE(3,2,3,2));
		XMM7	 = _mm_shuffle_ps(XMM7, XMM4, _MM_SHUFFLE(1,0,1,0));
		XMM6	 = _mm_shuffle_ps(XMM6, XMM4, _MM_SHUFFLE(3,2,3,2));
		XMM5	 = XMM1;
		XMM4	 = XMM0;
		XMM1	 = _mm_shuffle_ps(XMM1, XMM7, _MM_SHUFFLE(2,0,2,0));
		XMM5	 = _mm_shuffle_ps(XMM5, XMM7, _MM_SHUFFLE(3,1,3,1));
		XMM7	 = TEMP[12];	// X
		XMM0	 = _mm_shuffle_ps(XMM0, XMM6, _MM_SHUFFLE(2,0,2,0));
		XMM4	 = _mm_shuffle_ps(XMM4, XMM6, _MM_SHUFFLE(3,1,3,1));
		XMM6	 = TEMP[13];	// Y
		XMM1	 = _mm_add_ps(XMM1, TXXYY[ 7]);
		TN[ 8]	+= TN[ 7];
		XMM5	 = _mm_add_ps(XMM5, XMM1);
		TN[ 9]	+= TN[ 8];
		XMM0	 = _mm_add_ps(XMM0, XMM5);
		TN[10]	+= TN[ 9];
		XMM4	 = _mm_add_ps(XMM4, XMM0);
		TN[11]	+= TN[10];
		TXXYY[ 8]	 = XMM1;
		XMM1	 = TEMP[14];	// XX
		TXXYY[ 9]	 = XMM5;
		XMM5	 = TEMP[15];	// XY
		TXXYY[10]	 = XMM0;
		XMM0	 = XMM7;
		TXXYY[11]	 = XMM4;
		XMM4	 = XMM6;
		XMM7	 = _mm_shuffle_ps(XMM7, XMM1, _MM_SHUFFLE(1,0,1,0));
		XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(3,2,3,2));
		XMM6	 = _mm_shuffle_ps(XMM6, XMM5, _MM_SHUFFLE(1,0,1,0));
		XMM4	 = _mm_shuffle_ps(XMM4, XMM5, _MM_SHUFFLE(3,2,3,2));
		XMM1	 = XMM7;
		XMM5	 = XMM0;
		XMM7	 = _mm_shuffle_ps(XMM7, XMM6, _MM_SHUFFLE(2,0,2,0));
		XMM1	 = _mm_shuffle_ps(XMM1, XMM6, _MM_SHUFFLE(3,1,3,1));
		XMM0	 = _mm_shuffle_ps(XMM0, XMM4, _MM_SHUFFLE(2,0,2,0));
		XMM5	 = _mm_shuffle_ps(XMM5, XMM4, _MM_SHUFFLE(3,1,3,1));
		XMM7	 = _mm_add_ps(XMM7, TXXYY[11]);
		TN[12]	+= TN[11];
		XMM1	 = _mm_add_ps(XMM1, XMM7);
		TN[13]	+= TN[12];
		XMM0	 = _mm_add_ps(XMM0, XMM1);
		TN[14]	+= TN[13];
		XMM5	 = _mm_add_ps(XMM5, XMM0);
		TN[15]	+= TN[14];
		TXXYY[12]	 = XMM7;
		TXXYY[13]	 = XMM1;
		TXXYY[14]	 = XMM0;
		TXXYY[15]	 = XMM5;
		TN		+= 16;
		TXXYY	+= 16;
	}
	for(i=0;i<p->midpoint1_4;i+=4)
	{
		__m128	XMM0, XMM1, XMM4, XMM3;
		__m128x	TN, TN1;
		int	p0, p1, p2, p3;
		p0	 =-sb[i*2+1];
		p1	 =-sb[i*2+3];
		p2	 =-sb[i*2+5];
		p3	 =-sb[i*2+7];
		
		XMM0	 = XXYY[p0];
		XMM1	 = XXYY[p1];
		XMM4	 = XXYY[p2];
		XMM3	 = XXYY[p3];
		
		TN.sf[0]	 = N[p0];
		TN.sf[1]	 = N[p1];
		TN.sf[2]	 = N[p2];
		TN.sf[3]	 = N[p3];
		
		XMM0	 = _mm_xor_ps(XMM0, PM128(PCS_RNNR));
		XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RNNR));
		XMM4	 = _mm_xor_ps(XMM4, PM128(PCS_RNNR));
		XMM3	 = _mm_xor_ps(XMM3, PM128(PCS_RNNR));
		
		p0	 = sb[i*2  ];
		p1	 = sb[i*2+2];
		p2	 = sb[i*2+4];
		p3	 = sb[i*2+6];
		
		XMM0	 = _mm_add_ps(XMM0, XXYY[p0]);
		XMM1	 = _mm_add_ps(XMM1, XXYY[p1]);
		XMM4	 = _mm_add_ps(XMM4, XXYY[p2]);
		XMM3	 = _mm_add_ps(XMM3, XXYY[p3]);
		
		TN1.sf[0]	 = N[p0];
		TN1.sf[1]	 = N[p1];
		TN1.sf[2]	 = N[p2];
		TN1.sf[3]	 = N[p3];
		
		TN.ps	 = _mm_add_ps(TN.ps, TN1.ps);
		
		bark_noise_hybridmp_SSE_SUBC();
		XMM4	 = _mm_max_ps(XMM4, PM128(PFV_0));
		XMM4	 = _mm_sub_ps(XMM4, OFFSET);
		_mm_store_ps(noise+i  , XMM4);
	}
	if(p->midpoint2-i<4)
	{
		x	 = (float)i;
		for (;i<p->midpoint1;i++,x+=1.f)
		{
			lo	 = sb[i*2+1];
			hi	 = sb[i*2];
			
			tN	 = N[hi] + N[-lo];
			tX	 = xxyy[hi*4  ] - xxyy[-lo*4  ];
			tXX	 = xxyy[hi*4+1] + xxyy[-lo*4+1];
			tY	 = xxyy[hi*4+2] + xxyy[-lo*4+2];
			tXY	 = xxyy[hi*4+3] - xxyy[-lo*4+3];
			
			A	 = tY * tXX - tX * tXY;
			B	 = tN * tXY - tX * tY;
			D	 = tN * tXX - tX * tX;
			R	 = (A + x * B) / D;
			if(R<0.f)
				R	 = 0.f;
			
			noise[i]	 = R - offset;
		}
		for (;i<p->midpoint2;i++,x+=1.f)
		{
			lo	 = sb[i*2+1];
			hi	 = sb[i*2];
			
			tN	 = N[hi] - N[lo];
			tX	 = xxyy[hi*4  ] - xxyy[lo*4  ];
			tXX	 = xxyy[hi*4+1] - xxyy[lo*4+1];
			tY	 = xxyy[hi*4+2] - xxyy[lo*4+2];
			tXY	 = xxyy[hi*4+3] - xxyy[lo*4+3];
			
			A	 = tY * tXX - tX * tXY;
			B	 = tN * tXY - tX * tY;
			D	 = tN * tXX - tX * tX;
			R	 = (A + x * B) / D;
			if(R<0.f)
				R	 = 0.f;
			noise[i]	 = R - offset;
		}
		j	 = (i+3)&(~3);
		j	 = (j>=n)?n:j;
		for (;i<j;i++,x+=1.f)
		{
			R	 = (A + x * B) / D;
			if(R<0.f)
				R	 = 0.f;
			
			noise[i]	 = R - offset;
		}
		PA	 = _mm_set_ps1(A);
		PB	 = _mm_set_ps1(B);
		PD	 = _mm_set_ps1(1.f/D);
	}
	else
	{
		switch(p->midpoint1%4)
		{
			case 0:
				break;
			case 1:
				{
					__m128	XMM0, XMM1, XMM4, XMM3;
					__m128x	TN, TN1;
					int	p0, p1, p2, p3;
					p0	 =-sb[i*2+1];
					p1	 = sb[i*2+2];
					p2	 = sb[i*2+4];
					p3	 = sb[i*2+6];
					
					XMM0	 = XXYY[p0];
					XMM1	 = XXYY[p1];
					XMM4	 = XXYY[p2];
					XMM3	 = XXYY[p3];
					
					TN.sf[0]	 = N[p0];
					TN.sf[1]	 = N[p1];
					TN.sf[2]	 = N[p2];
					TN.sf[3]	 = N[p3];
					
					XMM0	 = _mm_xor_ps(XMM0, PM128(PCS_RNNR));
					
					p0	 = sb[i*2  ];
					p1	 = sb[i*2+3];
					p2	 = sb[i*2+5];
					p3	 = sb[i*2+7];
					
					XMM0	 = _mm_add_ps(XMM0, XXYY[p0]);
					XMM1	 = _mm_sub_ps(XMM1, XXYY[p1]);
					XMM4	 = _mm_sub_ps(XMM4, XXYY[p2]);
					XMM3	 = _mm_sub_ps(XMM3, XXYY[p3]);
					
					TN1.sf[0]	 = N[p0];
					TN1.sf[1]	 = N[p1];
					TN1.sf[2]	 = N[p2];
					TN1.sf[3]	 = N[p3];
					
					TN.ps	 = _mm_sub_ps(TN.ps, _mm_xor_ps(TN1.ps, PM128(PCS_NNNR)));
					
					bark_noise_hybridmp_SSE_SUBC();
					XMM4	 = _mm_max_ps(XMM4, PM128(PFV_0));
					XMM4	 = _mm_sub_ps(XMM4, OFFSET);
					_mm_store_ps(noise+i  , XMM4);
					i	+= 4;
				}
				break;
			case 2:
				{
					__m128	XMM0, XMM1, XMM4, XMM3;
					__m128x	TN, TN1;
					int	p0, p1, p2, p3;
					p0	 =-sb[i*2+1];
					p1	 =-sb[i*2+3];
					p2	 = sb[i*2+4];
					p3	 = sb[i*2+6];
					
					XMM0	 = XXYY[p0];
					XMM1	 = XXYY[p1];
					XMM4	 = XXYY[p2];
					XMM3	 = XXYY[p3];
					
					TN.sf[0]	 = N[p0];
					TN.sf[1]	 = N[p1];
					TN.sf[2]	 = N[p2];
					TN.sf[3]	 = N[p3];
					
					XMM0	 = _mm_xor_ps(XMM0, PM128(PCS_RNNR));
					XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RNNR));
					
					p0	 = sb[i*2  ];
					p1	 = sb[i*2+2];
					p2	 = sb[i*2+5];
					p3	 = sb[i*2+7];
					
					XMM0	 = _mm_add_ps(XMM0, XXYY[p0]);
					XMM1	 = _mm_add_ps(XMM1, XXYY[p1]);
					XMM4	 = _mm_sub_ps(XMM4, XXYY[p2]);
					XMM3	 = _mm_sub_ps(XMM3, XXYY[p3]);
					
					TN1.sf[0]	 = N[p0];
					TN1.sf[1]	 = N[p1];
					TN1.sf[2]	 = N[p2];
					TN1.sf[3]	 = N[p3];
					
					TN.ps	 = _mm_sub_ps(TN.ps, _mm_xor_ps(TN1.ps, PM128(PCS_NNRR)));
					
					bark_noise_hybridmp_SSE_SUBC();
					XMM4	 = _mm_max_ps(XMM4, PM128(PFV_0));
					XMM4	 = _mm_sub_ps(XMM4, OFFSET);
					_mm_store_ps(noise+i  , XMM4);
					i	+= 4;
				}
				break;
			case 3:
				{
					__m128	XMM0, XMM1, XMM4, XMM3;
					__m128x	TN, TN1;
					int	p0, p1, p2, p3;
					p0	 =-sb[i*2+1];
					p1	 =-sb[i*2+3];
					p2	 =-sb[i*2+5];
					p3	 = sb[i*2+6];
					
					XMM0	 = XXYY[p0];
					XMM1	 = XXYY[p1];
					XMM4	 = XXYY[p2];
					XMM3	 = XXYY[p3];
					
					TN.sf[0]	 = N[p0];
					TN.sf[1]	 = N[p1];
					TN.sf[2]	 = N[p2];
					TN.sf[3]	 = N[p3];
					
					XMM0	 = _mm_xor_ps(XMM0, PM128(PCS_RNNR));
					XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RNNR));
					XMM4	 = _mm_xor_ps(XMM4, PM128(PCS_RNNR));
					
					p0	 = sb[i*2  ];
					p1	 = sb[i*2+2];
					p2	 = sb[i*2+4];
					p3	 = sb[i*2+7];
					
					XMM0	 = _mm_add_ps(XMM0, XXYY[p0]);
					XMM1	 = _mm_add_ps(XMM1, XXYY[p1]);
					XMM4	 = _mm_add_ps(XMM4, XXYY[p2]);
					XMM3	 = _mm_sub_ps(XMM3, XXYY[p3]);
					
					TN1.sf[0]	 = N[p0];
					TN1.sf[1]	 = N[p1];
					TN1.sf[2]	 = N[p2];
					TN1.sf[3]	 = N[p3];
					
					TN.ps	 = _mm_sub_ps(TN.ps, _mm_xor_ps(TN1.ps, PM128(PCS_NRRR)));
					
					bark_noise_hybridmp_SSE_SUBC();
					XMM4	 = _mm_max_ps(XMM4, PM128(PFV_0));
					XMM4	 = _mm_sub_ps(XMM4, OFFSET);
					_mm_store_ps(noise+i  , XMM4);
					i	+= 4;
				}
				break;
		}
		for(;i<p->midpoint2_16;i+=16)
		{
			register __m128	XMM0, XMM1, XMM2, XMM3;
			register __m128	XMM4, XMM5, XMM6, XMM7;
			__m128x	TN0, TN1, TN2;
			int	p0, p1, p2, p3;
			p0	 = sb[i*2   ];
			p1	 = sb[i*2+ 2];
			p2	 = sb[i*2+ 4];
			p3	 = sb[i*2+ 6];
			XMM0	 = XXYY[p0];
			XMM1	 = XXYY[p1];
			XMM4	 = XXYY[p2];
			XMM3	 = XXYY[p3];
			TN0.sf[0]	 = N[p0];
			TN0.sf[1]	 = N[p1];
			TN0.sf[2]	 = N[p2];
			TN0.sf[3]	 = N[p3];
			p0	 = sb[i*2+ 1];
			p1	 = sb[i*2+ 3];
			p2	 = sb[i*2+ 5];
			p3	 = sb[i*2+ 7];
			XMM2	 = XXYY[p0];
			XMM5	 = XXYY[p1];
			XMM6	 = XXYY[p2];
			XMM7	 = XXYY[p3];
			XMM0	 = _mm_sub_ps(XMM0, XMM2);
			XMM1	 = _mm_sub_ps(XMM1, XMM5);
			XMM4	 = _mm_sub_ps(XMM4, XMM6);
			XMM3	 = _mm_sub_ps(XMM3, XMM7);
			TN1.sf[0]	 = N[p0];
			TN1.sf[1]	 = N[p1];
			TN1.sf[2]	 = N[p2];
			TN1.sf[3]	 = N[p3];
			XMM2	 = XMM0;
			XMM5	 = XMM4;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(1,0,1,0));
			XMM2	 = _mm_shuffle_ps(XMM2, XMM1, _MM_SHUFFLE(3,2,3,2));
			XMM7	 = TN0.ps;
			XMM6	 = TN1.ps;
			XMM4	 = _mm_shuffle_ps(XMM4, XMM3, _MM_SHUFFLE(1,0,1,0));
			XMM5	 = _mm_shuffle_ps(XMM5, XMM3, _MM_SHUFFLE(3,2,3,2));
			p0	 = sb[i*2+ 8];
			p1	 = sb[i*2+10];
			XMM1	 = XMM0;
			XMM3	 = XMM2;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM4, _MM_SHUFFLE(2,0,2,0));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM4, _MM_SHUFFLE(3,1,3,1));
			XMM7	 = _mm_sub_ps(XMM7, XMM6);
			p2	 = sb[i*2+12];
			XMM2	 = _mm_shuffle_ps(XMM2, XMM5, _MM_SHUFFLE(2,0,2,0));
			XMM3	 = _mm_shuffle_ps(XMM3, XMM5, _MM_SHUFFLE(3,1,3,1));
			TN0.ps	 = XMM7;
			p3	 = sb[i*2+14];
			XMM4	 = XMM2;
			XMM5	 = XMM0;
			XMM6	 = XMM3;
			XMM7	 = XMM0;
			XMM4	 = _mm_mul_ps(XMM4, XMM1);
			XMM5	 = _mm_mul_ps(XMM5, XMM3);
			XMM3	 = TN0.ps;
			XMM6	 = _mm_mul_ps(XMM6, XMM3);
			XMM1	 = _mm_mul_ps(XMM1, XMM3);
			XMM3	 = _mm_load_ps(findex+i   );
			XMM7	 = _mm_mul_ps(XMM7, XMM2);
			XMM2	 = XXYY[p0];
			XMM0	 = _mm_mul_ps(XMM0, XMM0);
			XMM4	 = _mm_sub_ps(XMM4, XMM5);
			XMM5	 = XXYY[p1];
			XMM6	 = _mm_sub_ps(XMM6, XMM7);
			XMM7	 = XXYY[p2];
			XMM1	 = _mm_sub_ps(XMM1, XMM0);
			XMM0	 = XXYY[p3];
			XMM6	 = _mm_mul_ps(XMM6, XMM3);
			XMM3	 = _mm_rcp_ps(XMM1);
			TN0.sf[0]	 = N[p0];
			TN0.sf[1]	 = N[p1];
			XMM4	 = _mm_add_ps(XMM4, XMM6);
			XMM1	 = _mm_mul_ps(XMM1, XMM3);
			TN0.sf[2]	 = N[p2];
			TN0.sf[3]	 = N[p3];
			XMM1	 = _mm_mul_ps(XMM1, XMM3);
			p0	 = sb[i*2+ 9];
			p1	 = sb[i*2+11];
			XMM3	 = _mm_add_ps(XMM3, XMM3);
			p2	 = sb[i*2+13];
			p3	 = sb[i*2+15];
			XMM3	 = _mm_sub_ps(XMM3, XMM1);
			XMM1	 = _mm_load_ps(PFV_0);
			XMM6	 = XXYY[p0];
			XMM4	 = _mm_mul_ps(XMM4, XMM3);
			XMM3	 = OFFSET;
			XMM4	 = _mm_max_ps(XMM4, XMM1);
			XMM1	 = XXYY[p1];
			XMM4	 = _mm_sub_ps(XMM4, XMM3);
			XMM3	 = XXYY[p2];
			_mm_store_ps(noise+i   , XMM4);
			XMM4	 = XXYY[p3];
			XMM2	 = _mm_sub_ps(XMM2, XMM6);
			XMM5	 = _mm_sub_ps(XMM5, XMM1);
			XMM7	 = _mm_sub_ps(XMM7, XMM3);
			XMM0	 = _mm_sub_ps(XMM0, XMM4);
			TN1.sf[0]	 = N[p0];
			TN1.sf[1]	 = N[p1];
			TN1.sf[2]	 = N[p2];
			TN1.sf[3]	 = N[p3];
			XMM6	 = XMM2;
			XMM1	 = XMM7;
			XMM2	 = _mm_shuffle_ps(XMM2, XMM5, _MM_SHUFFLE(1,0,1,0));
			XMM6	 = _mm_shuffle_ps(XMM6, XMM5, _MM_SHUFFLE(3,2,3,2));
			XMM4	 = TN0.ps;
			XMM3	 = TN1.ps;
			XMM7	 = _mm_shuffle_ps(XMM7, XMM0, _MM_SHUFFLE(1,0,1,0));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM0, _MM_SHUFFLE(3,2,3,2));
			p0	 = sb[i*2+16];
			p1	 = sb[i*2+18];
			XMM4	 = _mm_sub_ps(XMM4, XMM3);
			XMM5	 = XMM2;
			XMM0	 = XMM6;
			XMM2	 = _mm_shuffle_ps(XMM2, XMM7, _MM_SHUFFLE(2,0,2,0));
			XMM5	 = _mm_shuffle_ps(XMM5, XMM7, _MM_SHUFFLE(3,1,3,1));
			p2	 = sb[i*2+20];
			p3	 = sb[i*2+22];
			TN0.ps	 = XMM4;
			XMM6	 = _mm_shuffle_ps(XMM6, XMM1, _MM_SHUFFLE(2,0,2,0));
			XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(3,1,3,1));
			TN2.sf[0]	 = N[p0];
			TN2.sf[1]	 = N[p1];
			TN2.sf[2]	 = N[p2];
			TN2.sf[3]	 = N[p3];
			XMM7	 = XMM6;
			XMM1	 = XMM2;
			XMM3	 = XMM0;
			XMM4	 = XMM2;
			XMM7	 = _mm_mul_ps(XMM7, XMM5);
			XMM1	 = _mm_mul_ps(XMM1, XMM0);
			XMM0	 = TN0.ps;
			XMM3	 = _mm_mul_ps(XMM3, XMM0);
			XMM5	 = _mm_mul_ps(XMM5, XMM0);
			XMM0	 = _mm_load_ps(findex+i+ 4);
			XMM4	 = _mm_mul_ps(XMM4, XMM6);
			XMM6	 = XXYY[p0];
			XMM2	 = _mm_mul_ps(XMM2, XMM2);
			XMM7	 = _mm_sub_ps(XMM7, XMM1);
			XMM1	 = XXYY[p1];
			XMM3	 = _mm_sub_ps(XMM3, XMM4);
			XMM4	 = XXYY[p2];
			XMM5	 = _mm_sub_ps(XMM5, XMM2);
			XMM2	 = XXYY[p3];
			XMM3	 = _mm_mul_ps(XMM3, XMM0);
			XMM0	 = _mm_rcp_ps(XMM5);
			p0	 = sb[i*2+17];
			p1	 = sb[i*2+19];
			XMM7	 = _mm_add_ps(XMM7, XMM3);
			XMM3	 = XXYY[p0];
			XMM5	 = _mm_mul_ps(XMM5, XMM0);
			p2	 = sb[i*2+21];
			p3	 = sb[i*2+23];
			XMM5	 = _mm_mul_ps(XMM5, XMM0);
			XMM0	 = _mm_add_ps(XMM0, XMM0);
			TN1.sf[0]	 = N[p0];
			XMM0	 = _mm_sub_ps(XMM0, XMM5);
			XMM5	 = _mm_load_ps(PFV_0);
			XMM7	 = _mm_mul_ps(XMM7, XMM0);
			TN1.sf[1]	 = N[p1];
			XMM0	 = OFFSET;
			XMM7	 = _mm_max_ps(XMM7, XMM5);
			TN1.sf[2]	 = N[p2];
			XMM5	 = XXYY[p1];
			XMM7	 = _mm_sub_ps(XMM7, XMM0);
			TN1.sf[3]	 = N[p3];
			XMM0	 = XXYY[p2];
			_mm_store_ps(noise+i+ 4, XMM7);
			XMM7	 = XXYY[p3];
			XMM6	 = _mm_sub_ps(XMM6, XMM3);
			XMM1	 = _mm_sub_ps(XMM1, XMM5);
			XMM4	 = _mm_sub_ps(XMM4, XMM0);
			XMM2	 = _mm_sub_ps(XMM2, XMM7);
			XMM3	 = XMM6;
			XMM5	 = XMM4;
			XMM6	 = _mm_shuffle_ps(XMM6, XMM1, _MM_SHUFFLE(1,0,1,0));
			XMM3	 = _mm_shuffle_ps(XMM3, XMM1, _MM_SHUFFLE(3,2,3,2));
			XMM7	 = TN2.ps;
			XMM0	 = TN1.ps;
			XMM4	 = _mm_shuffle_ps(XMM4, XMM2, _MM_SHUFFLE(1,0,1,0));
			XMM5	 = _mm_shuffle_ps(XMM5, XMM2, _MM_SHUFFLE(3,2,3,2));
			p0	 = sb[i*2+24];
			p1	 = sb[i*2+26];
			XMM1	 = XMM6;
			XMM2	 = XMM3;
			XMM6	 = _mm_shuffle_ps(XMM6, XMM4, _MM_SHUFFLE(2,0,2,0));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM4, _MM_SHUFFLE(3,1,3,1));
			XMM7	 = _mm_sub_ps(XMM7, XMM0);
			p2	 = sb[i*2+28];
			XMM3	 = _mm_shuffle_ps(XMM3, XMM5, _MM_SHUFFLE(2,0,2,0));
			XMM2	 = _mm_shuffle_ps(XMM2, XMM5, _MM_SHUFFLE(3,1,3,1));
			TN0.ps	 = XMM7;
			p3	 = sb[i*2+30];
			XMM4	 = XMM3;
			XMM5	 = XMM6;
			XMM0	 = XMM2;
			XMM7	 = XMM6;
			XMM4	 = _mm_mul_ps(XMM4, XMM1);
			XMM5	 = _mm_mul_ps(XMM5, XMM2);
			XMM2	 = TN0.ps;
			XMM0	 = _mm_mul_ps(XMM0, XMM2);
			XMM1	 = _mm_mul_ps(XMM1, XMM2);
			XMM2	 = _mm_load_ps(findex+i+ 8);
			XMM7	 = _mm_mul_ps(XMM7, XMM3);
			XMM3	 = XXYY[p0];
			XMM6	 = _mm_mul_ps(XMM6, XMM6);
			XMM4	 = _mm_sub_ps(XMM4, XMM5);
			XMM5	 = XXYY[p1];
			XMM0	 = _mm_sub_ps(XMM0, XMM7);
			XMM7	 = XXYY[p2];
			XMM1	 = _mm_sub_ps(XMM1, XMM6);
			XMM6	 = XXYY[p3];
			XMM0	 = _mm_mul_ps(XMM0, XMM2);
			XMM2	 = _mm_rcp_ps(XMM1);
			TN0.sf[0]	 = N[p0];
			TN0.sf[1]	 = N[p1];
			XMM4	 = _mm_add_ps(XMM4, XMM0);
			XMM1	 = _mm_mul_ps(XMM1, XMM2);
			TN0.sf[2]	 = N[p2];
			TN0.sf[3]	 = N[p3];
			XMM1	 = _mm_mul_ps(XMM1, XMM2);
			p0	 = sb[i*2+25];
			p1	 = sb[i*2+27];
			XMM2	 = _mm_add_ps(XMM2, XMM2);
			p2	 = sb[i*2+29];
			p3	 = sb[i*2+31];
			XMM2	 = _mm_sub_ps(XMM2, XMM1);
			XMM1	 = _mm_load_ps(PFV_0);
			XMM0	 = XXYY[p0];
			XMM4	 = _mm_mul_ps(XMM4, XMM2);
			XMM2	 = OFFSET;
			XMM4	 = _mm_max_ps(XMM4, XMM1);
			XMM1	 = XXYY[p1];
			XMM4	 = _mm_sub_ps(XMM4, XMM2);
			XMM2	 = XXYY[p2];
			_mm_store_ps(noise+i+ 8, XMM4);
			XMM4	 = XXYY[p3];
			XMM3	 = _mm_sub_ps(XMM3, XMM0);
			XMM5	 = _mm_sub_ps(XMM5, XMM1);
			XMM7	 = _mm_sub_ps(XMM7, XMM2);
			XMM6	 = _mm_sub_ps(XMM6, XMM4);
			TN1.sf[0]	 = N[p0];
			TN1.sf[1]	 = N[p1];
			TN1.sf[2]	 = N[p2];
			TN1.sf[3]	 = N[p3];
			XMM0	 = XMM3;
			XMM1	 = XMM7;
			XMM3	 = _mm_shuffle_ps(XMM3, XMM5, _MM_SHUFFLE(1,0,1,0));
			XMM0	 = _mm_shuffle_ps(XMM0, XMM5, _MM_SHUFFLE(3,2,3,2));
			XMM4	 = TN0.ps;
			XMM2	 = TN1.ps;
			XMM7	 = _mm_shuffle_ps(XMM7, XMM6, _MM_SHUFFLE(1,0,1,0));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM6, _MM_SHUFFLE(3,2,3,2));
			XMM4	 = _mm_sub_ps(XMM4, XMM2);
			XMM5	 = XMM3;
			XMM6	 = XMM0;
			XMM3	 = _mm_shuffle_ps(XMM3, XMM7, _MM_SHUFFLE(2,0,2,0));
			XMM5	 = _mm_shuffle_ps(XMM5, XMM7, _MM_SHUFFLE(3,1,3,1));
			TN0.ps	 = XMM4;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(2,0,2,0));
			XMM6	 = _mm_shuffle_ps(XMM6, XMM1, _MM_SHUFFLE(3,1,3,1));
			XMM7	 = XMM0;
			XMM1	 = XMM3;
			XMM2	 = XMM6;
			XMM4	 = XMM3;
			XMM7	 = _mm_mul_ps(XMM7, XMM5);
			XMM1	 = _mm_mul_ps(XMM1, XMM6);
			XMM6	 = TN0.ps;
			XMM2	 = _mm_mul_ps(XMM2, XMM6);
			XMM5	 = _mm_mul_ps(XMM5, XMM6);
			XMM6	 = _mm_load_ps(findex+i+12);
			XMM4	 = _mm_mul_ps(XMM4, XMM0);
			XMM3	 = _mm_mul_ps(XMM3, XMM3);
			XMM7	 = _mm_sub_ps(XMM7, XMM1);
			XMM2	 = _mm_sub_ps(XMM2, XMM4);
			XMM5	 = _mm_sub_ps(XMM5, XMM3);
			XMM2	 = _mm_mul_ps(XMM2, XMM6);
			XMM6	 = _mm_rcp_ps(XMM5);
			XMM7	 = _mm_add_ps(XMM7, XMM2);
			XMM5	 = _mm_mul_ps(XMM5, XMM6);
			XMM5	 = _mm_mul_ps(XMM5, XMM6);
			XMM6	 = _mm_add_ps(XMM6, XMM6);
			XMM6	 = _mm_sub_ps(XMM6, XMM5);
			XMM5	 = _mm_load_ps(PFV_0);
			XMM7	 = _mm_mul_ps(XMM7, XMM6);
			XMM6	 = OFFSET;
			XMM7	 = _mm_max_ps(XMM7, XMM5);
			XMM7	 = _mm_sub_ps(XMM7, XMM6);
			_mm_store_ps(noise+i+12, XMM7);
		}
		for(;i<p->midpoint2_8;i+=8)
		{
			register __m128	XMM0, XMM1, XMM2, XMM3;
			register __m128	XMM4, XMM5, XMM6, XMM7;
			__m128x	TN0, TN1;
			int	p0, p1, p2, p3;
			p0	 = sb[i*2   ];
			p1	 = sb[i*2+ 2];
			p2	 = sb[i*2+ 4];
			p3	 = sb[i*2+ 6];
			XMM0	 = XXYY[p0];
			XMM1	 = XXYY[p1];
			XMM4	 = XXYY[p2];
			XMM3	 = XXYY[p3];
			TN0.sf[0]	 = N[p0];
			TN0.sf[1]	 = N[p1];
			TN0.sf[2]	 = N[p2];
			TN0.sf[3]	 = N[p3];
			p0	 = sb[i*2+ 1];
			p1	 = sb[i*2+ 3];
			p2	 = sb[i*2+ 5];
			p3	 = sb[i*2+ 7];
			XMM2	 = XXYY[p0];
			XMM5	 = XXYY[p1];
			XMM6	 = XXYY[p2];
			XMM7	 = XXYY[p3];
			XMM0	 = _mm_sub_ps(XMM0, XMM2);
			XMM1	 = _mm_sub_ps(XMM1, XMM5);
			XMM4	 = _mm_sub_ps(XMM4, XMM6);
			XMM3	 = _mm_sub_ps(XMM3, XMM7);
			TN1.sf[0]	 = N[p0];
			TN1.sf[1]	 = N[p1];
			TN1.sf[2]	 = N[p2];
			TN1.sf[3]	 = N[p3];
			XMM2	 = XMM0;
			XMM5	 = XMM4;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(1,0,1,0));
			XMM2	 = _mm_shuffle_ps(XMM2, XMM1, _MM_SHUFFLE(3,2,3,2));
			XMM7	 = TN0.ps;
			XMM6	 = TN1.ps;
			XMM4	 = _mm_shuffle_ps(XMM4, XMM3, _MM_SHUFFLE(1,0,1,0));
			XMM5	 = _mm_shuffle_ps(XMM5, XMM3, _MM_SHUFFLE(3,2,3,2));
			p0	 = sb[i*2+ 8];
			p1	 = sb[i*2+10];
			XMM1	 = XMM0;
			XMM3	 = XMM2;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM4, _MM_SHUFFLE(2,0,2,0));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM4, _MM_SHUFFLE(3,1,3,1));
			XMM7	 = _mm_sub_ps(XMM7, XMM6);
			p2	 = sb[i*2+12];
			XMM2	 = _mm_shuffle_ps(XMM2, XMM5, _MM_SHUFFLE(2,0,2,0));
			XMM3	 = _mm_shuffle_ps(XMM3, XMM5, _MM_SHUFFLE(3,1,3,1));
			TN0.ps	 = XMM7;
			p3	 = sb[i*2+14];
			XMM4	 = XMM2;
			XMM5	 = XMM0;
			XMM6	 = XMM3;
			XMM7	 = XMM0;
			XMM4	 = _mm_mul_ps(XMM4, XMM1);
			XMM5	 = _mm_mul_ps(XMM5, XMM3);
			XMM3	 = TN0.ps;
			XMM6	 = _mm_mul_ps(XMM6, XMM3);
			XMM1	 = _mm_mul_ps(XMM1, XMM3);
			XMM3	 = _mm_load_ps(findex+i   );
			XMM7	 = _mm_mul_ps(XMM7, XMM2);
			XMM2	 = XXYY[p0];
			XMM0	 = _mm_mul_ps(XMM0, XMM0);
			XMM4	 = _mm_sub_ps(XMM4, XMM5);
			XMM5	 = XXYY[p1];
			XMM6	 = _mm_sub_ps(XMM6, XMM7);
			XMM7	 = XXYY[p2];
			XMM1	 = _mm_sub_ps(XMM1, XMM0);
			XMM0	 = XXYY[p3];
			XMM6	 = _mm_mul_ps(XMM6, XMM3);
			XMM3	 = _mm_rcp_ps(XMM1);
			TN0.sf[0]	 = N[p0];
			TN0.sf[1]	 = N[p1];
			XMM4	 = _mm_add_ps(XMM4, XMM6);
			XMM1	 = _mm_mul_ps(XMM1, XMM3);
			TN0.sf[2]	 = N[p2];
			TN0.sf[3]	 = N[p3];
			XMM1	 = _mm_mul_ps(XMM1, XMM3);
			p0	 = sb[i*2+ 9];
			p1	 = sb[i*2+11];
			XMM3	 = _mm_add_ps(XMM3, XMM3);
			p2	 = sb[i*2+13];
			p3	 = sb[i*2+15];
			XMM3	 = _mm_sub_ps(XMM3, XMM1);
			XMM1	 = _mm_load_ps(PFV_0);
			XMM6	 = XXYY[p0];
			XMM4	 = _mm_mul_ps(XMM4, XMM3);
			XMM3	 = OFFSET;
			XMM4	 = _mm_max_ps(XMM4, XMM1);
			XMM1	 = XXYY[p1];
			XMM4	 = _mm_sub_ps(XMM4, XMM3);
			XMM3	 = XXYY[p2];
			_mm_store_ps(noise+i   , XMM4);
			XMM4	 = XXYY[p3];
			XMM2	 = _mm_sub_ps(XMM2, XMM6);
			XMM5	 = _mm_sub_ps(XMM5, XMM1);
			XMM7	 = _mm_sub_ps(XMM7, XMM3);
			XMM0	 = _mm_sub_ps(XMM0, XMM4);
			TN1.sf[0]	 = N[p0];
			TN1.sf[1]	 = N[p1];
			TN1.sf[2]	 = N[p2];
			TN1.sf[3]	 = N[p3];
			XMM6	 = XMM2;
			XMM1	 = XMM7;
			XMM2	 = _mm_shuffle_ps(XMM2, XMM5, _MM_SHUFFLE(1,0,1,0));
			XMM6	 = _mm_shuffle_ps(XMM6, XMM5, _MM_SHUFFLE(3,2,3,2));
			XMM4	 = TN0.ps;
			XMM3	 = TN1.ps;
			XMM7	 = _mm_shuffle_ps(XMM7, XMM0, _MM_SHUFFLE(1,0,1,0));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM0, _MM_SHUFFLE(3,2,3,2));
			XMM4	 = _mm_sub_ps(XMM4, XMM3);
			XMM5	 = XMM2;
			XMM0	 = XMM6;
			XMM2	 = _mm_shuffle_ps(XMM2, XMM7, _MM_SHUFFLE(2,0,2,0));
			XMM5	 = _mm_shuffle_ps(XMM5, XMM7, _MM_SHUFFLE(3,1,3,1));
			TN0.ps	 = XMM4;
			XMM6	 = _mm_shuffle_ps(XMM6, XMM1, _MM_SHUFFLE(2,0,2,0));
			XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(3,1,3,1));
			XMM7	 = XMM6;
			XMM1	 = XMM2;
			XMM3	 = XMM0;
			XMM4	 = XMM2;
			XMM7	 = _mm_mul_ps(XMM7, XMM5);
			XMM1	 = _mm_mul_ps(XMM1, XMM0);
			XMM0	 = TN0.ps;
			XMM3	 = _mm_mul_ps(XMM3, XMM0);
			XMM5	 = _mm_mul_ps(XMM5, XMM0);
			XMM0	 = _mm_load_ps(findex+i+ 4);
			XMM4	 = _mm_mul_ps(XMM4, XMM6);
			XMM2	 = _mm_mul_ps(XMM2, XMM2);
			XMM7	 = _mm_sub_ps(XMM7, XMM1);
			XMM3	 = _mm_sub_ps(XMM3, XMM4);
			XMM5	 = _mm_sub_ps(XMM5, XMM2);
			XMM3	 = _mm_mul_ps(XMM3, XMM0);
			XMM0	 = _mm_rcp_ps(XMM5);
			XMM7	 = _mm_add_ps(XMM7, XMM3);
			XMM5	 = _mm_mul_ps(XMM5, XMM0);
			XMM5	 = _mm_mul_ps(XMM5, XMM0);
			XMM0	 = _mm_add_ps(XMM0, XMM0);
			XMM0	 = _mm_sub_ps(XMM0, XMM5);
			XMM5	 = _mm_load_ps(PFV_0);
			XMM7	 = _mm_mul_ps(XMM7, XMM0);
			XMM0	 = OFFSET;
			XMM7	 = _mm_max_ps(XMM7, XMM5);
			XMM7	 = _mm_sub_ps(XMM7, XMM0);
			_mm_store_ps(noise+i+ 4, XMM7);
		}
		for(;i<p->midpoint2_4;i+=4)
		{
			register __m128	XMM0, XMM1, XMM2, XMM3;
			register __m128	XMM4, XMM5, XMM6, XMM7;
			__m128x	TN0, TN1;
			int	p0, p1, p2, p3;
			p0	 = sb[i*2   ];
			p1	 = sb[i*2+ 2];
			p2	 = sb[i*2+ 4];
			p3	 = sb[i*2+ 6];
			
			XMM0	 = XXYY[p0];
			XMM1	 = XXYY[p1];
			XMM4	 = XXYY[p2];
			XMM3	 = XXYY[p3];
			
			TN0.sf[0]	 = N[p0];
			TN0.sf[1]	 = N[p1];
			TN0.sf[2]	 = N[p2];
			TN0.sf[3]	 = N[p3];
			
			p0	 = sb[i*2+ 1];
			p1	 = sb[i*2+ 3];
			p2	 = sb[i*2+ 5];
			p3	 = sb[i*2+ 7];
			
			XMM2	 = XXYY[p0];
			XMM5	 = XXYY[p1];
			XMM6	 = XXYY[p2];
			XMM7	 = XXYY[p3];

			XMM0	 = _mm_sub_ps(XMM0, XMM2);
			XMM1	 = _mm_sub_ps(XMM1, XMM5);
			XMM4	 = _mm_sub_ps(XMM4, XMM6);
			XMM3	 = _mm_sub_ps(XMM3, XMM7);
			
			XMM2	 = XMM0;
			XMM5	 = XMM4;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(1,0,1,0));
			XMM2	 = _mm_shuffle_ps(XMM2, XMM1, _MM_SHUFFLE(3,2,3,2));
			TN1.sf[0]	 = N[p0];
			TN1.sf[1]	 = N[p1];
			XMM7	 = TN0.ps;
			XMM4	 = _mm_shuffle_ps(XMM4, XMM3, _MM_SHUFFLE(1,0,1,0));
			XMM5	 = _mm_shuffle_ps(XMM5, XMM3, _MM_SHUFFLE(3,2,3,2));
			TN1.sf[2]	 = N[p2];
			TN1.sf[3]	 = N[p3];
			XMM1	 = XMM0;
			XMM3	 = XMM2;
			XMM6	 = TN1.ps;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM4, _MM_SHUFFLE(2,0,2,0));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM4, _MM_SHUFFLE(3,1,3,1));
			XMM7	 = _mm_sub_ps(XMM7, XMM6);
			XMM2	 = _mm_shuffle_ps(XMM2, XMM5, _MM_SHUFFLE(2,0,2,0));
			XMM3	 = _mm_shuffle_ps(XMM3, XMM5, _MM_SHUFFLE(3,1,3,1));
			TN0.ps	 = XMM7;
			XMM4	 = XMM2;
			XMM5	 = XMM0;
			XMM6	 = XMM3;
			XMM7	 = XMM0;
			XMM4	 = _mm_mul_ps(XMM4, XMM1);
			XMM5	 = _mm_mul_ps(XMM5, XMM3);
			XMM3	 = TN0.ps;
			XMM6	 = _mm_mul_ps(XMM6, XMM3);
			XMM1	 = _mm_mul_ps(XMM1, XMM3);
			XMM3	 = _mm_load_ps(findex+i   );
			XMM7	 = _mm_mul_ps(XMM7, XMM2);
			XMM0	 = _mm_mul_ps(XMM0, XMM0);
			XMM4	 = _mm_sub_ps(XMM4, XMM5);
			XMM6	 = _mm_sub_ps(XMM6, XMM7);
			XMM1	 = _mm_sub_ps(XMM1, XMM0);
			XMM6	 = _mm_mul_ps(XMM6, XMM3);
			XMM3	 = _mm_rcp_ps(XMM1);
			XMM4	 = _mm_add_ps(XMM4, XMM6);
			XMM1	 = _mm_mul_ps(XMM1, XMM3);
			XMM1	 = _mm_mul_ps(XMM1, XMM3);
			XMM3	 = _mm_add_ps(XMM3, XMM3);
			XMM3	 = _mm_sub_ps(XMM3, XMM1);
			XMM1	 = _mm_load_ps(PFV_0);
			XMM4	 = _mm_mul_ps(XMM4, XMM3);
			XMM3	 = OFFSET;
			XMM4	 = _mm_max_ps(XMM4, XMM1);
			XMM4	 = _mm_sub_ps(XMM4, XMM3);
			_mm_store_ps(noise+i   , XMM4);
		}
		if(i!=n)
		{
			__m128	XMM0, XMM1, XMM4, XMM3;
			__m128x	TN, TN1;
			int	p0, p1, p2;
			switch(p->midpoint2%4)
			{
				case 0:
					{
						lo	 = sb[i*2-1];
						hi	 = sb[i*2-2];
						
						tN	 = N[hi] - N[lo];
						tX	 = xxyy[hi*4  ] - xxyy[lo*4  ];
						tXX	 = xxyy[hi*4+1] - xxyy[lo*4+1];
						tY	 = xxyy[hi*4+2] - xxyy[lo*4+2];
						tXY	 = xxyy[hi*4+3] - xxyy[lo*4+3];
						
						A	 = tY * tXX - tX * tXY;
						B	 = tN * tXY - tX * tY;
						D	 = tN * tXX - tX * tX;
						PA	 = _mm_set_ps1(A);
						PB	 = _mm_set_ps1(B);
						PD	 = _mm_set_ps1(1.f/D);
					}
					break;
				case 1:
					{
						p0	 = sb[i*2  ];
						
						XMM0	 = XXYY[p0];
						
						TN.ps	 = _mm_set_ps1(N[p0]);
						
						p0	 = sb[i*2+1];
						
						XMM1	 =
						XMM4	 =
						XMM3	 =
						XMM0	 = _mm_sub_ps(XMM0, XXYY[p0]);
						
						TN1.ps	 = _mm_set_ps1(N[p0]);
						
						TN.ps	 = _mm_sub_ps(TN.ps, TN1.ps);
						
						bark_noise_hybridmp_SSE_SUBC2();
						XMM4	 = _mm_max_ps(XMM4, PM128(PFV_0));
						XMM4	 = _mm_sub_ps(XMM4, OFFSET);
						_mm_store_ps(noise+i  , XMM4);
						i	+= 4;
						PA		 = _mm_shuffle_ps(PA, PA, _MM_SHUFFLE(0,0,0,0));
						PB		 = _mm_shuffle_ps(PB, PB, _MM_SHUFFLE(0,0,0,0));
						PD		 = _mm_shuffle_ps(PD, PD, _MM_SHUFFLE(0,0,0,0));
					}
					break;
				case 2:
					{
						p0	 = sb[i*2  ];
						p1	 = sb[i*2+2];
						
						XMM0	 = XXYY[p0];
						XMM1	 = XXYY[p1];
						
						TN.sf[0]	 = N[p0];
						TN.sf[1]	 =
						TN.sf[2]	 =
						TN.sf[3]	 = N[p1];
						
						p0	 = sb[i*2+1];
						p1	 = sb[i*2+3];
						
						XMM0	 = _mm_sub_ps(XMM0, XXYY[p0]);
						XMM4	 =
						XMM3	 =
						XMM1	 = _mm_sub_ps(XMM1, XXYY[p1]);
						
						TN1.sf[0]	 = N[p0];
						TN1.sf[1]	 =
						TN1.sf[2]	 =
						TN1.sf[3]	 = N[p1];
						
						TN.ps	 = _mm_sub_ps(TN.ps, TN1.ps);
						
						bark_noise_hybridmp_SSE_SUBC2();
						XMM4	 = _mm_max_ps(XMM4, PM128(PFV_0));
						XMM4	 = _mm_sub_ps(XMM4, OFFSET);
						_mm_store_ps(noise+i  , XMM4);
						i	+= 4;
						PA		 = _mm_shuffle_ps(PA, PA, _MM_SHUFFLE(1,1,1,1));
						PB		 = _mm_shuffle_ps(PB, PB, _MM_SHUFFLE(1,1,1,1));
						PD		 = _mm_shuffle_ps(PD, PD, _MM_SHUFFLE(1,1,1,1));
					}
					break;
				case 3:
					{
						p0	 = sb[i*2  ];
						p1	 = sb[i*2+2];
						p2	 = sb[i*2+4];
						
						XMM0	 = XXYY[p0];
						XMM1	 = XXYY[p1];
						XMM4	 = XXYY[p2];
						
						TN.sf[0]	 = N[p0];
						TN.sf[1]	 = N[p1];
						TN.sf[2]	 =
						TN.sf[3]	 = N[p2];
						
						p0	 = sb[i*2+1];
						p1	 = sb[i*2+3];
						p2	 = sb[i*2+5];
						
						XMM0	 = _mm_sub_ps(XMM0, XXYY[p0]);
						XMM1	 = _mm_sub_ps(XMM1, XXYY[p1]);
						XMM3	 =
						XMM4	 = _mm_sub_ps(XMM4, XXYY[p2]);
						
						TN1.sf[0]	 = N[p0];
						TN1.sf[1]	 = N[p1];
						TN1.sf[2]	 =
						TN1.sf[3]	 = N[p2];
						
						TN.ps	 = _mm_sub_ps(TN.ps, TN1.ps);
						
						bark_noise_hybridmp_SSE_SUBC2();
						XMM4	 = _mm_max_ps(XMM4, PM128(PFV_0));
						XMM4	 = _mm_sub_ps(XMM4, OFFSET);
						_mm_store_ps(noise+i  , XMM4);
						i	+= 4;
						PA		 = _mm_shuffle_ps(PA, PA, _MM_SHUFFLE(2,2,2,2));
						PB		 = _mm_shuffle_ps(PB, PB, _MM_SHUFFLE(2,2,2,2));
						PD		 = _mm_shuffle_ps(PD, PD, _MM_SHUFFLE(2,2,2,2));
					}
					break;
			}
		}
	}
	if(i<n)
	{
		__m128	XMM0	 = PA;
		__m128	XMM1	 = PB;
		__m128	XMM2	 = _mm_set_ps1(-offset);
		XMM0	 = _mm_mul_ps(XMM0, PD);
		XMM1	 = _mm_mul_ps(XMM1, PD);
		XMM0	 = _mm_sub_ps(XMM0, OFFSET);
		if(i%8!=0)
		{
			__m128	XMM4	 = _mm_load_ps(findex+i   );
			XMM4	 = _mm_mul_ps(XMM4, XMM1);
			XMM4	 = _mm_add_ps(XMM4, XMM0);
			XMM4	 = _mm_max_ps(XMM4, XMM2);
			_mm_store_ps(noise+i  , XMM4);
			i	+= 4;
		}
		if(i%16!=0)
		{
			__m128	XMM4	 = _mm_load_ps(findex+i   );
			__m128	XMM5	 = _mm_load_ps(findex+i+ 4);
			XMM4	 = _mm_mul_ps(XMM4, XMM1);
			XMM5	 = _mm_mul_ps(XMM5, XMM1);
			XMM4	 = _mm_add_ps(XMM4, XMM0);
			XMM5	 = _mm_add_ps(XMM5, XMM0);
			XMM4	 = _mm_max_ps(XMM4, XMM2);
			XMM5	 = _mm_max_ps(XMM5, XMM2);
			_mm_store_ps(noise+i  , XMM4);
			_mm_store_ps(noise+i+4, XMM5);
			i	+= 8;
		}
		for(;i<n;i+=16)
		{
			__m128	XMM4	 = _mm_load_ps(findex+i   );
			__m128	XMM5	 = _mm_load_ps(findex+i+ 4);
			__m128	XMM6	 = _mm_load_ps(findex+i+ 8);
			__m128	XMM7	 = _mm_load_ps(findex+i+12);
			XMM4	 = _mm_mul_ps(XMM4, XMM1);
			XMM5	 = _mm_mul_ps(XMM5, XMM1);
			XMM6	 = _mm_mul_ps(XMM6, XMM1);
			XMM7	 = _mm_mul_ps(XMM7, XMM1);
			XMM4	 = _mm_add_ps(XMM4, XMM0);
			XMM5	 = _mm_add_ps(XMM5, XMM0);
			XMM6	 = _mm_add_ps(XMM6, XMM0);
			XMM7	 = _mm_add_ps(XMM7, XMM0);
			XMM4	 = _mm_max_ps(XMM4, XMM2);
			XMM5	 = _mm_max_ps(XMM5, XMM2);
			XMM6	 = _mm_max_ps(XMM6, XMM2);
			XMM7	 = _mm_max_ps(XMM7, XMM2);
			_mm_store_ps(noise+i   , XMM4);
			_mm_store_ps(noise+i+ 4, XMM5);
			_mm_store_ps(noise+i+ 8, XMM6);
			_mm_store_ps(noise+i+12, XMM7);
		}
	}

	if (fixed <= 0) return;

	midpoint1	 = (fixed+1)/2;
	midpoint2	 = n-fixed/2;
	
	j	 = midpoint1&(~7);
	p1	 = fixed / 2;
	p0	 = p1 - 3;
	
	for(i=0;i<j;i+=8)
	{
		__m128	XMM0, XMM1, XMM2, XMM3;
		__m128	XMM4, XMM5, XMM6, XMM7;
		__m128x	TN, TN1;

		XMM5	 = _mm_lddqu_ps(N+p0);
		XMM0	 = XXYY[p0+3];
		XMM1	 = XXYY[p0+2];
		XMM4	 = XXYY[p0+1];
		XMM3	 = XXYY[p0  ];
		TN.ps	 = _mm_shuffle_ps(XMM5, XMM5, _MM_SHUFFLE(0,1,2,3));
		XMM0	 = _mm_xor_ps(XMM0, PM128(PCS_RNNR));
		XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RNNR));
		XMM4	 = _mm_xor_ps(XMM4, PM128(PCS_RNNR));
		XMM3	 = _mm_xor_ps(XMM3, PM128(PCS_RNNR));
		XMM5	 = _mm_lddqu_ps(N+p1);
		XMM0	 = _mm_add_ps(XMM0, XXYY[p1  ]);
		XMM1	 = _mm_add_ps(XMM1, XXYY[p1+1]);
		XMM4	 = _mm_add_ps(XMM4, XXYY[p1+2]);
		XMM3	 = _mm_add_ps(XMM3, XXYY[p1+3]);
		TN.ps	 = _mm_add_ps(TN.ps, XMM5);
		XMM2 = XMM0;
		XMM5 = XMM4;
		XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(1,0,1,0));
		XMM2	 = _mm_shuffle_ps(XMM2, XMM1, _MM_SHUFFLE(3,2,3,2));
		XMM4	 = _mm_shuffle_ps(XMM4, XMM3, _MM_SHUFFLE(1,0,1,0));
		XMM5	 = _mm_shuffle_ps(XMM5, XMM3, _MM_SHUFFLE(3,2,3,2));
		XMM1 = XMM0;
		XMM3 = XMM2;
		XMM0	 = _mm_shuffle_ps(XMM0, XMM4, _MM_SHUFFLE(2,0,2,0));
		XMM1	 = _mm_shuffle_ps(XMM1, XMM4, _MM_SHUFFLE(3,1,3,1));
		XMM2	 = _mm_shuffle_ps(XMM2, XMM5, _MM_SHUFFLE(2,0,2,0));
		XMM3	 = _mm_shuffle_ps(XMM3, XMM5, _MM_SHUFFLE(3,1,3,1));
		XMM4 = XMM2;
		XMM5 = XMM0;
		XMM6 = XMM3;
		XMM7 = XMM0;
		XMM4 = _mm_mul_ps(XMM4, XMM1);
		XMM5 = _mm_mul_ps(XMM5, XMM3);
		XMM3 = _mm_load_ps(findex+i);
		XMM6 = _mm_mul_ps(XMM6, TN.ps);
		XMM1 = _mm_mul_ps(XMM1, TN.ps);
		XMM7 = _mm_mul_ps(XMM7, XMM2);
		XMM2	 = _mm_lddqu_ps(N+p0-4);
		XMM0 = _mm_mul_ps(XMM0, XMM0);
		XMM4 = _mm_sub_ps(XMM4, XMM5);
		XMM5	 = XXYY[p0-1];
		XMM6 = _mm_sub_ps(XMM6, XMM7);
		XMM7	 = XXYY[p0-2];
		XMM1 = _mm_sub_ps(XMM1, XMM0);
		XMM0	 = XXYY[p0-3];
		XMM6 = _mm_mul_ps(XMM6, XMM3);
		XMM3 = _mm_rcp_ps(XMM1);
		XMM4 = _mm_add_ps(XMM4, XMM6);
		XMM6	 = XXYY[p0-4];
		XMM1 = _mm_mul_ps(XMM1, XMM3);
		TN1.ps	 = _mm_shuffle_ps(XMM2, XMM2, _MM_SHUFFLE(0,1,2,3));
		XMM1 = _mm_mul_ps(XMM1, XMM3);
		XMM5	 = _mm_xor_ps(XMM5, PM128(PCS_RNNR));
		XMM3 = _mm_add_ps(XMM3, XMM3);
		XMM7	 = _mm_xor_ps(XMM7, PM128(PCS_RNNR));
		XMM3 = _mm_sub_ps(XMM3, XMM1);
		XMM0	 = _mm_xor_ps(XMM0, PM128(PCS_RNNR));
		XMM4 = _mm_mul_ps(XMM4, XMM3);
		XMM6	 = _mm_xor_ps(XMM6, PM128(PCS_RNNR));
		XMM2	 = _mm_lddqu_ps(N+p1+4);
		XMM4	 = _mm_sub_ps(XMM4, OFFSET);
		XMM5	 = _mm_add_ps(XMM5, XXYY[p1+4]);
		XMM4	 = _mm_min_ps(XMM4, PM128(noise+i  ));
		XMM7	 = _mm_add_ps(XMM7, XXYY[p1+5]);
		XMM0	 = _mm_add_ps(XMM0, XXYY[p1+6]);
		_mm_store_ps(noise+i  , XMM4);
		XMM6	 = _mm_add_ps(XMM6, XXYY[p1+7]);
		TN1.ps	 = _mm_add_ps(TN1.ps, XMM2);
		XMM1 = XMM5;
		XMM2 = XMM0;
		XMM5	 = _mm_shuffle_ps(XMM5, XMM7, _MM_SHUFFLE(1,0,1,0));
		XMM1	 = _mm_shuffle_ps(XMM1, XMM7, _MM_SHUFFLE(3,2,3,2));
		XMM0	 = _mm_shuffle_ps(XMM0, XMM6, _MM_SHUFFLE(1,0,1,0));
		XMM2	 = _mm_shuffle_ps(XMM2, XMM6, _MM_SHUFFLE(3,2,3,2));
		XMM7 = XMM5;
		XMM6 = XMM1;
		XMM5	 = _mm_shuffle_ps(XMM5, XMM0, _MM_SHUFFLE(2,0,2,0));
		XMM7	 = _mm_shuffle_ps(XMM7, XMM0, _MM_SHUFFLE(3,1,3,1));
		XMM1	 = _mm_shuffle_ps(XMM1, XMM2, _MM_SHUFFLE(2,0,2,0));
		XMM6	 = _mm_shuffle_ps(XMM6, XMM2, _MM_SHUFFLE(3,1,3,1));
		XMM0 = XMM1;
		XMM2 = XMM5;
		XMM3 = XMM6;
		XMM4 = XMM5;
		XMM0 = _mm_mul_ps(XMM0, XMM7);
		XMM2 = _mm_mul_ps(XMM2, XMM6);
		XMM6 = _mm_load_ps(findex+i+4);
		XMM3 = _mm_mul_ps(XMM3, TN1.ps);
		XMM7 = _mm_mul_ps(XMM7, TN1.ps);
		XMM4 = _mm_mul_ps(XMM4, XMM1);
		XMM5 = _mm_mul_ps(XMM5, XMM5);
		XMM0 = _mm_sub_ps(XMM0, XMM2);
		XMM3 = _mm_sub_ps(XMM3, XMM4);
		XMM7 = _mm_sub_ps(XMM7, XMM5);
		XMM3 = _mm_mul_ps(XMM3, XMM6);
		XMM6 = _mm_rcp_ps(XMM7);
		XMM0 = _mm_add_ps(XMM0, XMM3);
		XMM7 = _mm_mul_ps(XMM7, XMM6);
		XMM7 = _mm_mul_ps(XMM7, XMM6);
		XMM6 = _mm_add_ps(XMM6, XMM6);
		XMM6 = _mm_sub_ps(XMM6, XMM7);
		XMM6 = _mm_mul_ps(XMM6, XMM0);
		XMM6	 = _mm_sub_ps(XMM6, OFFSET);
		XMM6	 = _mm_min_ps(XMM6, PM128(noise+i+4));
		_mm_store_ps(noise+i+4, XMM6);
		p0 -= 8;
		p1 += 8;
	}
	j	 = midpoint1&(~3);
	for(;i<j;i+=4)
	{
		__m128	XMM0, XMM1, XMM4, XMM3, XMM5;
		__m128x	TN;
		
		XMM5	 = _mm_lddqu_ps(N+p0);
		XMM0	 = XXYY[p0+3];
		XMM1	 = XXYY[p0+2];
		XMM4	 = XXYY[p0+1];
		XMM3	 = XXYY[p0  ];
		TN.ps	 = _mm_shuffle_ps(XMM5, XMM5, _MM_SHUFFLE(0,1,2,3));
		
		XMM0	 = _mm_xor_ps(XMM0, PM128(PCS_RNNR));
		XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RNNR));
		XMM4	 = _mm_xor_ps(XMM4, PM128(PCS_RNNR));
		XMM3	 = _mm_xor_ps(XMM3, PM128(PCS_RNNR));
		
		XMM5	 = _mm_lddqu_ps(N+p1);
		XMM0	 = _mm_add_ps(XMM0, XXYY[p1  ]);
		XMM1	 = _mm_add_ps(XMM1, XXYY[p1+1]);
		XMM4	 = _mm_add_ps(XMM4, XXYY[p1+2]);
		XMM3	 = _mm_add_ps(XMM3, XXYY[p1+3]);
		
		TN.ps	 = _mm_add_ps(TN.ps, XMM5);
		
		bark_noise_hybridmp_SSE_SUBC();
		XMM4	 = _mm_sub_ps(XMM4, OFFSET);
		XMM4	 = _mm_min_ps(XMM4, PM128(noise+i  ));
		_mm_store_ps(noise+i  , XMM4);
		p0 -= 4;
		p1 += 4;
	}
	if(midpoint2-i<4)
	{
		x	 = (float)i;
		for (;i<midpoint1;i++,x+=1.f)
		{
			hi	 = i + fixed / 2;
			lo	 = hi - fixed;
	
			tN	 = N[hi] + N[-lo];
			tX	 = xxyy[hi*4  ] - xxyy[-lo*4  ];
			tXX	 = xxyy[hi*4+1] + xxyy[-lo*4+1];
			tY	 = xxyy[hi*4+2] + xxyy[-lo*4+2];
			tXY	 = xxyy[hi*4+3] - xxyy[-lo*4+3];
	
			A	 = tY * tXX - tX * tXY;
			B	 = tN * tXY - tX * tY;
			D	 = tN * tXX - tX * tX;
			R	 = (A + x * B) / D;
	
			if(R - offset < noise[i])
				noise[i]	 = R - offset;
		}
		for (;i<midpoint2;i++,x+=1.f)
		{
			hi	 = i + fixed / 2;
			lo	 = hi - fixed;
	
			tN	 = N[hi] - N[lo];
			tX	 = xxyy[hi*4  ] - xxyy[lo*4  ];
			tXX	 = xxyy[hi*4+1] - xxyy[lo*4+1];
			tY	 = xxyy[hi*4+2] - xxyy[lo*4+2];
			tXY	 = xxyy[hi*4+3] - xxyy[lo*4+3];
			
			A	 = tY * tXX - tX * tXY;
			B	 = tN * tXY - tX * tY;
			D	 = tN * tXX - tX * tX;
			R	 = (A + x * B) / D;
			if(R - offset < noise[i])
				noise[i]	 = R - offset;
		}
		j	 = (i+3)&(~3);
		j	 = (j>=n)?n:j;
		for (;i<j;i++,x+=1.f)
		{
			R	 = (A + x * B) / D;
			if(R - offset < noise[i])
				noise[i]	 = R - offset;
		}
		PA	 = _mm_set_ps1(A);
		PB	 = _mm_set_ps1(B);
		PD	 = _mm_set_ps1(D);
	}
	else
	{
		switch(midpoint1%4)
		{
			case 0:
				break;
			case 1:
				{
					__m128	XMM0, XMM1, XMM4, XMM3;
					__m128x	TN, TN1;
					int	p0, p1;
					p0	 = -((i  ) + fixed / 2 - fixed);
					p1	 = (i+1) + fixed / 2;
					
					XMM0	 = XXYY[p0  ];
					XMM1	 = XXYY[p1  ];
					XMM4	 = XXYY[p1+1];
					XMM3	 = XXYY[p1+2];
					
					TN.sf[0]	 = N[p0  ];
					TN.sf[1]	 = N[p1  ];
					TN.sf[2]	 = N[p1+1];
					TN.sf[3]	 = N[p1+2];
					
					XMM0	 = _mm_xor_ps(XMM0, PM128(PCS_RNNR));
					
					p0	 = (i  ) + fixed / 2;
					p1	-= fixed;
					
					XMM0	 = _mm_add_ps(XMM0, XXYY[p0  ]);
					XMM1	 = _mm_sub_ps(XMM1, XXYY[p1  ]);
					XMM4	 = _mm_sub_ps(XMM4, XXYY[p1+1]);
					XMM3	 = _mm_sub_ps(XMM3, XXYY[p1+2]);
					
					TN1.sf[0]	 = N[p0  ];
					TN1.sf[1]	 = N[p1  ];
					TN1.sf[2]	 = N[p1+1];
					TN1.sf[3]	 = N[p1+2];
					
					TN.ps	 = _mm_sub_ps(TN.ps, _mm_xor_ps(TN1.ps, PM128(PCS_NNNR)));
					
					bark_noise_hybridmp_SSE_SUBC();
					XMM4	 = _mm_sub_ps(XMM4, OFFSET);
					XMM4	 = _mm_min_ps(XMM4, PM128(noise+i  ));
					_mm_store_ps(noise+i  , XMM4);
					i	+= 4;
				}
				break;
			case 2:
				{
					__m128	XMM0, XMM1, XMM4, XMM3;
					__m128x	TN, TN1;
					int	p0, p1;
					p0	 = -((i  ) + fixed / 2 - fixed);
					p1	 = (i+2) + fixed / 2;
					
					XMM0	 = XXYY[p0  ];
					XMM1	 = XXYY[p0-1];
					XMM4	 = XXYY[p1  ];
					XMM3	 = XXYY[p1+1];
					
					TN.sf[0]	 = N[p0  ];
					TN.sf[1]	 = N[p0-1];
					TN.sf[2]	 = N[p1  ];
					TN.sf[3]	 = N[p1+1];
					
					XMM0	 = _mm_xor_ps(XMM0, PM128(PCS_RNNR));
					XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RNNR));
					
					p0	 = (i  ) + fixed / 2;
					p1	-= fixed;
					
					XMM0	 = _mm_add_ps(XMM0, XXYY[p0  ]);
					XMM1	 = _mm_add_ps(XMM1, XXYY[p0+1]);
					XMM4	 = _mm_sub_ps(XMM4, XXYY[p1  ]);
					XMM3	 = _mm_sub_ps(XMM3, XXYY[p1+1]);
					
					TN1.sf[0]	 = N[p0  ];
					TN1.sf[1]	 = N[p0+1];
					TN1.sf[2]	 = N[p1  ];
					TN1.sf[3]	 = N[p1+1];
					
					TN.ps	 = _mm_sub_ps(TN.ps, _mm_xor_ps(TN1.ps, PM128(PCS_NNRR)));
					
					bark_noise_hybridmp_SSE_SUBC();
					XMM4	 = _mm_sub_ps(XMM4, OFFSET);
					XMM4	 = _mm_min_ps(XMM4, PM128(noise+i  ));
					_mm_store_ps(noise+i  , XMM4);
					i	+= 4;
				}
				break;
			case 3:
				{
					__m128	XMM0, XMM1, XMM4, XMM3;
					__m128x	TN, TN1;
					int	p0, p1;
					p0	 = -((i  ) + fixed / 2 - fixed);
					p1	 = (i+3) + fixed / 2;
					
					XMM0	 = XXYY[p0  ];
					XMM1	 = XXYY[p0-1];
					XMM4	 = XXYY[p0-2];
					XMM3	 = XXYY[p1  ];
					
					TN.sf[0]	 = N[p0  ];
					TN.sf[1]	 = N[p0-1];
					TN.sf[2]	 = N[p0-2];
					TN.sf[3]	 = N[p1  ];
					
					XMM0	 = _mm_xor_ps(XMM0, PM128(PCS_RNNR));
					XMM1	 = _mm_xor_ps(XMM1, PM128(PCS_RNNR));
					XMM4	 = _mm_xor_ps(XMM4, PM128(PCS_RNNR));
					
					p0	 = (i  ) + fixed / 2;
					p1	-= fixed;
					
					XMM0	 = _mm_add_ps(XMM0, XXYY[p0  ]);
					XMM1	 = _mm_add_ps(XMM1, XXYY[p0+1]);
					XMM4	 = _mm_sub_ps(XMM4, XXYY[p0+2]);
					XMM3	 = _mm_sub_ps(XMM3, XXYY[p1  ]);
					
					TN1.sf[0]	 = N[p0  ];
					TN1.sf[1]	 = N[p0+1];
					TN1.sf[2]	 = N[p0+2];
					TN1.sf[3]	 = N[p1  ];
					
					TN.ps	 = _mm_sub_ps(TN.ps, _mm_xor_ps(TN1.ps, PM128(PCS_NRRR)));
					
					bark_noise_hybridmp_SSE_SUBC();
					XMM4	 = _mm_sub_ps(XMM4, OFFSET);
					XMM4	 = _mm_min_ps(XMM4, PM128(noise+i  ));
					_mm_store_ps(noise+i  , XMM4);
					i	+= 4;
				}
				break;
		}
		p0	 = i  + fixed / 2;
		p1	 = p0 - fixed;
		j	 = ((midpoint2-i)&(~15))+i;
		for(;i<j;i+=16)
		{
			__m128	XMM0, XMM1, XMM2, XMM3;
			__m128	XMM4, XMM5, XMM6, XMM7;
			__m128x	TN, TN1;
			
			XMM0	 = XXYY[p0   ];
			XMM1	 = XXYY[p0+ 1];
			XMM4	 = XXYY[p0+ 2];
			XMM3	 = XXYY[p0+ 3];
			TN.ps	 = _mm_lddqu_ps(N+p0   );
			XMM5	 = _mm_lddqu_ps(N+p1   );
			XMM0	 = _mm_sub_ps(XMM0, XXYY[p1   ]);
			XMM1	 = _mm_sub_ps(XMM1, XXYY[p1+ 1]);
			XMM4	 = _mm_sub_ps(XMM4, XXYY[p1+ 2]);
			XMM3	 = _mm_sub_ps(XMM3, XXYY[p1+ 3]);
			TN.ps	 = _mm_sub_ps(TN.ps, XMM5);
			XMM2 = XMM0;
			XMM5 = XMM4;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(1,0,1,0));
			XMM2	 = _mm_shuffle_ps(XMM2, XMM1, _MM_SHUFFLE(3,2,3,2));
			XMM4	 = _mm_shuffle_ps(XMM4, XMM3, _MM_SHUFFLE(1,0,1,0));
			XMM5	 = _mm_shuffle_ps(XMM5, XMM3, _MM_SHUFFLE(3,2,3,2));
			XMM1 = XMM0;
			XMM3 = XMM2;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM4, _MM_SHUFFLE(2,0,2,0));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM4, _MM_SHUFFLE(3,1,3,1));
			XMM2	 = _mm_shuffle_ps(XMM2, XMM5, _MM_SHUFFLE(2,0,2,0));
			XMM3	 = _mm_shuffle_ps(XMM3, XMM5, _MM_SHUFFLE(3,1,3,1));
			XMM4 = XMM2;
			XMM5 = XMM0;
			XMM6 = XMM3;
			XMM7 = XMM0;
			XMM4 = _mm_mul_ps(XMM4, XMM1);
			XMM5 = _mm_mul_ps(XMM5, XMM3);
			XMM3 = _mm_load_ps(findex+i   );
			XMM6 = _mm_mul_ps(XMM6, TN.ps);
			XMM1 = _mm_mul_ps(XMM1, TN.ps);
			XMM7 = _mm_mul_ps(XMM7, XMM2);
			XMM2	 = XXYY[p0+ 4];
			XMM0 = _mm_mul_ps(XMM0, XMM0);
			XMM4 = _mm_sub_ps(XMM4, XMM5);
			XMM5	 = XXYY[p0+ 5];
			XMM6 = _mm_sub_ps(XMM6, XMM7);
			XMM7	 = XXYY[p0+ 6];
			XMM1 = _mm_sub_ps(XMM1, XMM0);
			XMM0	 = XXYY[p0+ 7];
			XMM6 = _mm_mul_ps(XMM6, XMM3);

			TN1.ps	 = _mm_lddqu_ps(N+p0+ 4);
			XMM3 = _mm_rcp_ps(XMM1);
			XMM4 = _mm_add_ps(XMM4, XMM6);
			XMM6	 = _mm_lddqu_ps(N+p1+ 4);
			XMM1 = _mm_mul_ps(XMM1, XMM3);
			XMM2	 = _mm_sub_ps(XMM2, XXYY[p1+ 4]);
			XMM1 = _mm_mul_ps(XMM1, XMM3);
			XMM5	 = _mm_sub_ps(XMM5, XXYY[p1+ 5]);
			XMM3 = _mm_add_ps(XMM3, XMM3);
			XMM7	 = _mm_sub_ps(XMM7, XXYY[p1+ 6]);
			XMM3 = _mm_sub_ps(XMM3, XMM1);
			XMM0	 = _mm_sub_ps(XMM0, XXYY[p1+ 7]);
			XMM4 = _mm_mul_ps(XMM4, XMM3);
			TN1.ps	 = _mm_sub_ps(TN1.ps, XMM6);
			XMM4	 = _mm_sub_ps(XMM4, OFFSET);
			XMM1 = XMM2;
			XMM4	 = _mm_min_ps(XMM4, PM128(noise+i   ));
			XMM6 = XMM7;
			_mm_store_ps(noise+i   , XMM4);
			XMM2	 = _mm_shuffle_ps(XMM2, XMM5, _MM_SHUFFLE(1,0,1,0));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM5, _MM_SHUFFLE(3,2,3,2));
			XMM7	 = _mm_shuffle_ps(XMM7, XMM0, _MM_SHUFFLE(1,0,1,0));
			XMM6	 = _mm_shuffle_ps(XMM6, XMM0, _MM_SHUFFLE(3,2,3,2));
			XMM5 = XMM2;
			XMM0 = XMM1;
			XMM2	 = _mm_shuffle_ps(XMM2, XMM7, _MM_SHUFFLE(2,0,2,0));
			XMM5	 = _mm_shuffle_ps(XMM5, XMM7, _MM_SHUFFLE(3,1,3,1));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM6, _MM_SHUFFLE(2,0,2,0));
			XMM0	 = _mm_shuffle_ps(XMM0, XMM6, _MM_SHUFFLE(3,1,3,1));
			XMM7 = XMM1;
			XMM6 = XMM2;
			XMM3 = XMM0;
			XMM4 = XMM2;
			XMM7 = _mm_mul_ps(XMM7, XMM5);
			XMM6 = _mm_mul_ps(XMM6, XMM0);
			XMM0 = _mm_load_ps(findex+i+ 4);
			XMM3 = _mm_mul_ps(XMM3, TN1.ps);
			XMM5 = _mm_mul_ps(XMM5, TN1.ps);
			XMM4 = _mm_mul_ps(XMM4, XMM1);
			XMM1	 = XXYY[p0+ 8];
			XMM2 = _mm_mul_ps(XMM2, XMM2);
			XMM7 = _mm_sub_ps(XMM7, XMM6);
			XMM6	 = XXYY[p0+ 9];
			XMM3 = _mm_sub_ps(XMM3, XMM4);
			XMM4	 = XXYY[p0+10];
			XMM5 = _mm_sub_ps(XMM5, XMM2);
			XMM2	 = XXYY[p0+11];
			XMM3 = _mm_mul_ps(XMM3, XMM0);
			TN.ps	 = _mm_lddqu_ps(N+p0+ 8);
			XMM0 = _mm_rcp_ps(XMM5);
			XMM7 = _mm_add_ps(XMM7, XMM3);
			XMM3	 = _mm_lddqu_ps(N+p1+ 8);
			XMM5 = _mm_mul_ps(XMM5, XMM0);
			XMM1	 = _mm_sub_ps(XMM1, XXYY[p1+ 8]);
			XMM5 = _mm_mul_ps(XMM5, XMM0);
			XMM6	 = _mm_sub_ps(XMM6, XXYY[p1+ 9]);
			XMM0 = _mm_add_ps(XMM0, XMM0);
			XMM4	 = _mm_sub_ps(XMM4, XXYY[p1+10]);
			XMM0 = _mm_sub_ps(XMM0, XMM5);
			XMM2	 = _mm_sub_ps(XMM2, XXYY[p1+11]);
			XMM7 = _mm_mul_ps(XMM7, XMM0);
			TN.ps	 = _mm_sub_ps(TN.ps, XMM3);
			XMM7	 = _mm_sub_ps(XMM7, OFFSET);
			XMM5 = XMM1;
			XMM7	 = _mm_min_ps(XMM7, PM128(noise+i+ 4));
			XMM3 = XMM4;
			_mm_store_ps(noise+i+ 4, XMM7);
			XMM1	 = _mm_shuffle_ps(XMM1, XMM6, _MM_SHUFFLE(1,0,1,0));
			XMM5	 = _mm_shuffle_ps(XMM5, XMM6, _MM_SHUFFLE(3,2,3,2));
			XMM4	 = _mm_shuffle_ps(XMM4, XMM2, _MM_SHUFFLE(1,0,1,0));
			XMM3	 = _mm_shuffle_ps(XMM3, XMM2, _MM_SHUFFLE(3,2,3,2));
			XMM6 = XMM1;
			XMM2 = XMM5;
			XMM1	 = _mm_shuffle_ps(XMM1, XMM4, _MM_SHUFFLE(2,0,2,0));
			XMM6	 = _mm_shuffle_ps(XMM6, XMM4, _MM_SHUFFLE(3,1,3,1));
			XMM5	 = _mm_shuffle_ps(XMM5, XMM3, _MM_SHUFFLE(2,0,2,0));
			XMM2	 = _mm_shuffle_ps(XMM2, XMM3, _MM_SHUFFLE(3,1,3,1));
			XMM4 = XMM5;
			XMM3 = XMM1;
			XMM0 = XMM2;
			XMM7 = XMM1;
			XMM4 = _mm_mul_ps(XMM4, XMM6);
			XMM3 = _mm_mul_ps(XMM3, XMM2);
			XMM2 = _mm_load_ps(findex+i+ 8);
			XMM0 = _mm_mul_ps(XMM0, TN.ps);
			XMM6 = _mm_mul_ps(XMM6, TN.ps);
			XMM7 = _mm_mul_ps(XMM7, XMM5);
			XMM5	 = XXYY[p0+12];
			XMM1 = _mm_mul_ps(XMM1, XMM1);
			XMM4 = _mm_sub_ps(XMM4, XMM3);
			XMM3	 = XXYY[p0+13];
			XMM0 = _mm_sub_ps(XMM0, XMM7);
			XMM7	 = XXYY[p0+14];
			XMM6 = _mm_sub_ps(XMM6, XMM1);
			XMM1	 = XXYY[p0+15];
			XMM0 = _mm_mul_ps(XMM0, XMM2);
			TN1.ps	 = _mm_lddqu_ps(N+p0+12);
			XMM2 = _mm_rcp_ps(XMM6);
			XMM4 = _mm_add_ps(XMM4, XMM0);
			XMM0	 = _mm_lddqu_ps(N+p1+12);
			XMM6 = _mm_mul_ps(XMM6, XMM2);
			XMM5	 = _mm_sub_ps(XMM5, XXYY[p1+12]);
			XMM6 = _mm_mul_ps(XMM6, XMM2);
			XMM3	 = _mm_sub_ps(XMM3, XXYY[p1+13]);
			XMM2 = _mm_add_ps(XMM2, XMM2);
			XMM7	 = _mm_sub_ps(XMM7, XXYY[p1+14]);
			XMM2 = _mm_sub_ps(XMM2, XMM6);
			XMM1	 = _mm_sub_ps(XMM1, XXYY[p1+15]);
			XMM4 = _mm_mul_ps(XMM4, XMM2);
			TN1.ps	 = _mm_sub_ps(TN1.ps, XMM0);
			XMM4	 = _mm_sub_ps(XMM4, OFFSET);
			XMM6 = XMM5;
			XMM4	 = _mm_min_ps(XMM4, PM128(noise+i+ 8));
			XMM0 = XMM7;
			_mm_store_ps(noise+i+ 8, XMM4);
			XMM5	 = _mm_shuffle_ps(XMM5, XMM3, _MM_SHUFFLE(1,0,1,0));
			XMM6	 = _mm_shuffle_ps(XMM6, XMM3, _MM_SHUFFLE(3,2,3,2));
			XMM7	 = _mm_shuffle_ps(XMM7, XMM1, _MM_SHUFFLE(1,0,1,0));
			XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(3,2,3,2));
			XMM3 = XMM5;
			XMM1 = XMM6;
			XMM5	 = _mm_shuffle_ps(XMM5, XMM7, _MM_SHUFFLE(2,0,2,0));
			XMM3	 = _mm_shuffle_ps(XMM3, XMM7, _MM_SHUFFLE(3,1,3,1));
			XMM6	 = _mm_shuffle_ps(XMM6, XMM0, _MM_SHUFFLE(2,0,2,0));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM0, _MM_SHUFFLE(3,1,3,1));
			XMM7 = XMM6;
			XMM0 = XMM5;
			XMM2 = XMM1;
			XMM4 = XMM5;
			XMM7 = _mm_mul_ps(XMM7, XMM3);
			XMM0 = _mm_mul_ps(XMM0, XMM1);
			XMM1 = _mm_load_ps(findex+i+12);
			XMM2 = _mm_mul_ps(XMM2, TN1.ps);
			XMM3 = _mm_mul_ps(XMM3, TN1.ps);
			XMM4 = _mm_mul_ps(XMM4, XMM6);
			XMM5 = _mm_mul_ps(XMM5, XMM5);
			XMM7 = _mm_sub_ps(XMM7, XMM0);
			XMM2 = _mm_sub_ps(XMM2, XMM4);
			XMM3 = _mm_sub_ps(XMM3, XMM5);
			XMM2 = _mm_mul_ps(XMM2, XMM1);
			XMM1 = _mm_rcp_ps(XMM3);
			XMM7 = _mm_add_ps(XMM7, XMM2);
			XMM3 = _mm_mul_ps(XMM3, XMM1);
			XMM3 = _mm_mul_ps(XMM3, XMM1);
			XMM1 = _mm_add_ps(XMM1, XMM1);
			XMM1 = _mm_sub_ps(XMM1, XMM3);
			XMM7 = _mm_mul_ps(XMM7, XMM1);
			XMM7	 = _mm_sub_ps(XMM7, OFFSET);
			XMM7	 = _mm_min_ps(XMM7, PM128(noise+i+12));
			_mm_store_ps(noise+i+12, XMM7);

			p0 += 16;
			p1 += 16;
		}
		j	 = ((midpoint2-i)&(~7))+i;
		for(;i<j;i+=8)
		{
			__m128	XMM0, XMM1, XMM2, XMM3;
			__m128	XMM4, XMM5, XMM6, XMM7;
			__m128x	TN, TN1;
			
			XMM0	 = XXYY[p0  ];
			XMM1	 = XXYY[p0+1];
			XMM4	 = XXYY[p0+2];
			XMM3	 = XXYY[p0+3];
			TN.ps	 = _mm_lddqu_ps(N+p0   );
			XMM5	 = _mm_lddqu_ps(N+p1   );
			XMM0	 = _mm_sub_ps(XMM0, XXYY[p1  ]);
			XMM1	 = _mm_sub_ps(XMM1, XXYY[p1+1]);
			XMM4	 = _mm_sub_ps(XMM4, XXYY[p1+2]);
			XMM3	 = _mm_sub_ps(XMM3, XXYY[p1+3]);
			TN.ps	 = _mm_sub_ps(TN.ps, XMM5);
			XMM2 = XMM0;
			XMM5 = XMM4;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(1,0,1,0));
			XMM2	 = _mm_shuffle_ps(XMM2, XMM1, _MM_SHUFFLE(3,2,3,2));
			XMM4	 = _mm_shuffle_ps(XMM4, XMM3, _MM_SHUFFLE(1,0,1,0));
			XMM5	 = _mm_shuffle_ps(XMM5, XMM3, _MM_SHUFFLE(3,2,3,2));
			XMM1 = XMM0;
			XMM3 = XMM2;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM4, _MM_SHUFFLE(2,0,2,0));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM4, _MM_SHUFFLE(3,1,3,1));
			XMM2	 = _mm_shuffle_ps(XMM2, XMM5, _MM_SHUFFLE(2,0,2,0));
			XMM3	 = _mm_shuffle_ps(XMM3, XMM5, _MM_SHUFFLE(3,1,3,1));
			XMM4 = XMM2;
			XMM5 = XMM0;
			XMM6 = XMM3;
			XMM7 = XMM0;
			XMM4 = _mm_mul_ps(XMM4, XMM1);
			XMM5 = _mm_mul_ps(XMM5, XMM3);
			XMM3 = _mm_load_ps(findex+i  );
			XMM6 = _mm_mul_ps(XMM6, TN.ps);
			XMM1 = _mm_mul_ps(XMM1, TN.ps);
			XMM7 = _mm_mul_ps(XMM7, XMM2);
			XMM2	 = XXYY[p0+4];
			XMM0 = _mm_mul_ps(XMM0, XMM0);
			XMM4 = _mm_sub_ps(XMM4, XMM5);
			XMM5	 = XXYY[p0+5];
			XMM6 = _mm_sub_ps(XMM6, XMM7);
			XMM7	 = XXYY[p0+6];
			XMM1 = _mm_sub_ps(XMM1, XMM0);
			XMM0	 = XXYY[p0+7];
			XMM6 = _mm_mul_ps(XMM6, XMM3);
			TN1.ps	 = _mm_lddqu_ps(N+p0+ 4);
			XMM3 = _mm_rcp_ps(XMM1);
			XMM4 = _mm_add_ps(XMM4, XMM6);
			XMM6	 = _mm_lddqu_ps(N+p1+ 4);
			XMM1 = _mm_mul_ps(XMM1, XMM3);
			XMM2	 = _mm_sub_ps(XMM2, XXYY[p1+4]);
			XMM1 = _mm_mul_ps(XMM1, XMM3);
			XMM5	 = _mm_sub_ps(XMM5, XXYY[p1+5]);
			XMM3 = _mm_add_ps(XMM3, XMM3);
			XMM7	 = _mm_sub_ps(XMM7, XXYY[p1+6]);
			XMM3 = _mm_sub_ps(XMM3, XMM1);
			XMM0	 = _mm_sub_ps(XMM0, XXYY[p1+7]);
			XMM4 = _mm_mul_ps(XMM4, XMM3);
			TN1.ps	 = _mm_sub_ps(TN1.ps, XMM6);
			XMM4	 = _mm_sub_ps(XMM4, OFFSET);
			XMM1 = XMM2;
			XMM4	 = _mm_min_ps(XMM4, PM128(noise+i  ));
			XMM6 = XMM7;
			_mm_store_ps(noise+i  , XMM4);
			XMM2	 = _mm_shuffle_ps(XMM2, XMM5, _MM_SHUFFLE(1,0,1,0));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM5, _MM_SHUFFLE(3,2,3,2));
			XMM7	 = _mm_shuffle_ps(XMM7, XMM0, _MM_SHUFFLE(1,0,1,0));
			XMM6	 = _mm_shuffle_ps(XMM6, XMM0, _MM_SHUFFLE(3,2,3,2));
			XMM5 = XMM2;
			XMM0 = XMM1;
			XMM2	 = _mm_shuffle_ps(XMM2, XMM7, _MM_SHUFFLE(2,0,2,0));
			XMM5	 = _mm_shuffle_ps(XMM5, XMM7, _MM_SHUFFLE(3,1,3,1));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM6, _MM_SHUFFLE(2,0,2,0));
			XMM0	 = _mm_shuffle_ps(XMM0, XMM6, _MM_SHUFFLE(3,1,3,1));
			XMM7 = XMM1;
			XMM6 = XMM2;
			XMM3 = XMM0;
			XMM4 = XMM2;
			XMM7 = _mm_mul_ps(XMM7, XMM5);
			XMM6 = _mm_mul_ps(XMM6, XMM0);
			XMM0 = _mm_load_ps(findex+i+4);
			XMM3 = _mm_mul_ps(XMM3, TN1.ps);
			XMM5 = _mm_mul_ps(XMM5, TN1.ps);
			XMM4 = _mm_mul_ps(XMM4, XMM1);
			XMM2 = _mm_mul_ps(XMM2, XMM2);
			XMM7 = _mm_sub_ps(XMM7, XMM6);
			XMM3 = _mm_sub_ps(XMM3, XMM4);
			XMM5 = _mm_sub_ps(XMM5, XMM2);
			XMM3 = _mm_mul_ps(XMM3, XMM0);
			XMM0 = _mm_rcp_ps(XMM5);
			XMM7 = _mm_add_ps(XMM7, XMM3);
			XMM5 = _mm_mul_ps(XMM5, XMM0);
			XMM5 = _mm_mul_ps(XMM5, XMM0);
			XMM0 = _mm_add_ps(XMM0, XMM0);
			XMM0 = _mm_sub_ps(XMM0, XMM5);
			XMM7 = _mm_mul_ps(XMM7, XMM0);
			XMM7	 = _mm_sub_ps(XMM7, OFFSET);
			XMM7	 = _mm_min_ps(XMM7, PM128(noise+i+4));
			_mm_store_ps(noise+i+4, XMM7);

			p0 += 8;
			p1 += 8;
		}
		j	 = midpoint2&(~3);
		for(;i<j;i+=4)
		{
			__m128	XMM0, XMM1, XMM4, XMM3;
			__m128x	TN;
			__m128	XMM5;
			
			XMM0	 = XXYY[p0  ];
			XMM1	 = XXYY[p0+1];
			XMM4	 = XXYY[p0+2];
			XMM3	 = XXYY[p0+3];
			TN.ps	 = _mm_lddqu_ps(N+p0   );
			XMM5	 = _mm_lddqu_ps(N+p1   );
			XMM0	 = _mm_sub_ps(XMM0, XXYY[p1  ]);
			XMM1	 = _mm_sub_ps(XMM1, XXYY[p1+1]);
			XMM4	 = _mm_sub_ps(XMM4, XXYY[p1+2]);
			XMM3	 = _mm_sub_ps(XMM3, XXYY[p1+3]);
			
			TN.ps	 = _mm_sub_ps(TN.ps, XMM5);
			
			bark_noise_hybridmp_SSE_SUBC();
			XMM4	 = _mm_sub_ps(XMM4, OFFSET);
			XMM4	 = _mm_min_ps(XMM4, PM128(noise+i  ));
			_mm_store_ps(noise+i  , XMM4);
			p0 += 4;
			p1 += 4;
		}
		if(i!=n)
		{
			switch(midpoint2%4)
			{
				case 0:
					{
						hi	 = (i-1) + fixed / 2;
						lo	 = hi - fixed;
						
						tN	 = N[hi] - N[lo];
						tX	 = xxyy[hi*4  ] - xxyy[lo*4  ];
						tXX	 = xxyy[hi*4+1] - xxyy[lo*4+1];
						tY	 = xxyy[hi*4+2] - xxyy[lo*4+2];
						tXY	 = xxyy[hi*4+3] - xxyy[lo*4+3];
						
						A	 = tY * tXX - tX * tXY;
						B	 = tN * tXY - tX * tY;
						D	 = tN * tXX - tX * tX;
						PA	 = _mm_set_ps1(A);
						PB	 = _mm_set_ps1(B);
						PD	 = _mm_set_ps1(1.f/D);
					}
					break;
				case 1:
					{
						__m128	XMM0, XMM1, XMM4, XMM3;
						__m128x	TN, TN1;
						int p0	 = (i  ) + fixed / 2;
						
						XMM0	 =
						XMM1	 =
						XMM4	 =
						XMM3	 = XXYY[p0];
						
						TN.ps	 = _mm_set_ps1(N[p0]);
						
						p0	-= fixed;
						
						XMM0	 =
						XMM4	 =
						XMM3	 =
						XMM1	 = _mm_sub_ps(XMM3, XXYY[p0]);
						
						TN1.ps	 = _mm_set_ps1(N[p0]);
						
						TN.ps	 = _mm_sub_ps(TN.ps, TN1.ps);
						
						bark_noise_hybridmp_SSE_SUBC2();
						XMM4	 = _mm_sub_ps(XMM4, OFFSET);
						XMM4	 = _mm_min_ps(XMM4, PM128(noise+i  ));
						_mm_store_ps(noise+i  , XMM4);
						i	+= 4;
						PA		 = _mm_shuffle_ps(PA, PA, _MM_SHUFFLE(0,0,0,0));
						PB		 = _mm_shuffle_ps(PB, PB, _MM_SHUFFLE(0,0,0,0));
						PD		 = _mm_shuffle_ps(PD, PD, _MM_SHUFFLE(0,0,0,0));
					}
					break;
				case 2:
					{
						__m128	XMM0, XMM1, XMM4, XMM3;
						__m128x	TN;
						__m128	XMM5;
						int p0	 = (i  ) + fixed / 2;
						
						XMM5	 = _mm_lddqu_ps(N+p0);
						XMM0	 = XXYY[p0  ];
						XMM1	 =
						XMM4	 =
						XMM3	 = XXYY[p0+1];
						TN.ps	 = _mm_shuffle_ps(XMM5, XMM5, _MM_SHUFFLE(1,1,1,0));
						
						p0	-= fixed;
						
						XMM5	 = _mm_lddqu_ps(N+p0);
						XMM0	 = _mm_sub_ps(XMM0, XXYY[p0  ]);
						XMM4	 =
						XMM3	 =
						XMM1	 = _mm_sub_ps(XMM3, XXYY[p0+1]);
						XMM5	 = _mm_shuffle_ps(XMM5, XMM5, _MM_SHUFFLE(1,1,1,0));
						
						TN.ps	 = _mm_sub_ps(TN.ps, XMM5);
						
						bark_noise_hybridmp_SSE_SUBC2();
						XMM4	 = _mm_sub_ps(XMM4, OFFSET);
						XMM4	 = _mm_min_ps(XMM4, PM128(noise+i  ));
						_mm_store_ps(noise+i  , XMM4);
						i	+= 4;
						PA		 = _mm_shuffle_ps(PA, PA, _MM_SHUFFLE(1,1,1,1));
						PB		 = _mm_shuffle_ps(PB, PB, _MM_SHUFFLE(1,1,1,1));
						PD		 = _mm_shuffle_ps(PD, PD, _MM_SHUFFLE(1,1,1,1));
					}
					break;
				case 3:
					{
						__m128	XMM0, XMM1, XMM4, XMM3;
						__m128x	TN;
						__m128	XMM5;
						int p0	 = (i  ) + fixed / 2;
						
						XMM5	 = _mm_lddqu_ps(N+p0);
						XMM0	 = XXYY[p0  ];
						XMM1	 = XXYY[p0+1];
						XMM4	 =
						XMM3	 = XXYY[p0+2];
						TN.ps	 = _mm_shuffle_ps(XMM5, XMM5, _MM_SHUFFLE(2,2,1,0));
						
						p0	-=  fixed;
						
						XMM5	 = _mm_lddqu_ps(N+p0);
						XMM0	 = _mm_sub_ps(XMM0, XXYY[p0  ]);
						XMM1	 = _mm_sub_ps(XMM1, XXYY[p0+1]);
						XMM4	 = 
						XMM3	 = _mm_sub_ps(XMM3, XXYY[p0+2]);
						XMM5	 = _mm_shuffle_ps(XMM5, XMM5, _MM_SHUFFLE(2,2,1,0));
						
						TN.ps	 = _mm_sub_ps(TN.ps, XMM5);
						
						bark_noise_hybridmp_SSE_SUBC2();
						XMM4	 = _mm_sub_ps(XMM4, OFFSET);
						XMM4	 = _mm_min_ps(XMM4, PM128(noise+i  ));
						_mm_store_ps(noise+i  , XMM4);
						i	+= 4;
						PA		 = _mm_shuffle_ps(PA, PA, _MM_SHUFFLE(2,2,2,2));
						PB		 = _mm_shuffle_ps(PB, PB, _MM_SHUFFLE(2,2,2,2));
						PD		 = _mm_shuffle_ps(PD, PD, _MM_SHUFFLE(2,2,2,2));
					}
					break;
			}
		}
	}
	if(i<n)
	{
		__m128	XMM0	 = PA;
		__m128	XMM1	 = PB;
		XMM0	 = _mm_mul_ps(XMM0, PD);
		XMM1	 = _mm_mul_ps(XMM1, PD);
		XMM0	 = _mm_sub_ps(XMM0, OFFSET);
		if(i%8!=0)
		{
			__m128	XMM4	 = _mm_load_ps(findex+i);
			XMM4	 = _mm_mul_ps(XMM4, XMM1);
			XMM4	 = _mm_add_ps(XMM4, XMM0);
			XMM4	 = _mm_min_ps(XMM4, PM128(noise+i   ));
			_mm_store_ps(noise+i   , XMM4);
			i	+= 4;
		}
		if(i%16!=0)
		{
			__m128	XMM4	 = _mm_load_ps(findex+i  );
			__m128	XMM5	 = _mm_load_ps(findex+i+4);
			__m128	XMM6	 = _mm_load_ps(noise+i   );
			__m128	XMM7	 = _mm_load_ps(noise+i+ 4);
			XMM4	 = _mm_mul_ps(XMM4, XMM1);
			XMM5	 = _mm_mul_ps(XMM5, XMM1);
			XMM4	 = _mm_add_ps(XMM4, XMM0);
			XMM5	 = _mm_add_ps(XMM5, XMM0);
			XMM6	 = _mm_min_ps(XMM6, XMM4);
			XMM7	 = _mm_min_ps(XMM7, XMM5);
			_mm_store_ps(noise+i   , XMM6);
			_mm_store_ps(noise+i+ 4, XMM7);
			i	+= 8;
		}
		for(;i<n;i+=32)
		{
			__m128	XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
			XMM4	 = _mm_load_ps(findex+i   );
			XMM5	 = _mm_load_ps(findex+i+ 4);
			XMM6	 = _mm_load_ps(noise+i    );
			XMM7	 = _mm_load_ps(noise+i+  4);
			XMM2	 = _mm_load_ps(findex+i+ 8);
			XMM3	 = _mm_load_ps(findex+i+12);
			XMM4	 = _mm_mul_ps(XMM4, XMM1);
			XMM5	 = _mm_mul_ps(XMM5, XMM1);
			XMM4	 = _mm_add_ps(XMM4, XMM0);
			XMM5	 = _mm_add_ps(XMM5, XMM0);
			XMM6	 = _mm_min_ps(XMM6, XMM4);
			XMM7	 = _mm_min_ps(XMM7, XMM5);
			XMM4	 = _mm_load_ps(noise+i+  8);
			XMM5	 = _mm_load_ps(noise+i+ 12);
			_mm_store_ps(noise+i   , XMM6);
			_mm_store_ps(noise+i+ 4, XMM7);
			XMM2	 = _mm_mul_ps(XMM2, XMM1);
			XMM3	 = _mm_mul_ps(XMM3, XMM1);
			XMM2	 = _mm_add_ps(XMM2, XMM0);
			XMM3	 = _mm_add_ps(XMM3, XMM0);
			XMM2	 = _mm_min_ps(XMM2, XMM4);
			XMM3	 = _mm_min_ps(XMM3, XMM5);
			_mm_store_ps(noise+i+ 8, XMM2);
			_mm_store_ps(noise+i+12, XMM3);
		}
	}
#else														/* SSE Optimize */
static void bark_noise_hybridmp(int n,const long *b,
                                const float *f,
                                float *noise,
                                const float offset,
                                const int fixed){
  
  float *N=alloca(n*sizeof(*N));
  float *X=alloca(n*sizeof(*N));
  float *XX=alloca(n*sizeof(*N));
  float *Y=alloca(n*sizeof(*N));
  float *XY=alloca(n*sizeof(*N));

  float tN, tX, tXX, tY, tXY;
  int i;

  int lo, hi;
  float R, A, B, D;
  float w, x, y;

  tN = tX = tXX = tY = tXY = 0.f;

  y = f[0] + offset;
  if (y < 1.f) y = 1.f;

  w = y * y * .5;
    
  tN += w;
  tX += w;
  tY += w * y;

  N[0] = tN;
  X[0] = tX;
  XX[0] = tXX;
  Y[0] = tY;
  XY[0] = tXY;

  for (i = 1, x = 1.f; i < n; i++, x += 1.f) {
    
    y = f[i] + offset;
    if (y < 1.f) y = 1.f;

    w = y * y;
    
    tN += w;
    tX += w * x;
    tXX += w * x * x;
    tY += w * y;
    tXY += w * x * y;

    N[i] = tN;
    X[i] = tX;
    XX[i] = tXX;
    Y[i] = tY;
    XY[i] = tXY;
  }
  
  for (i = 0, x = 0.f;; i++, x += 1.f) {
    
    lo = b[i] >> 16;
    if( lo>=0 ) break;
    hi = b[i] & 0xffff;
    
    tN = N[hi] + N[-lo];
    tX = X[hi] - X[-lo];
    tXX = XX[hi] + XX[-lo];
    tY = Y[hi] + Y[-lo];    
    tXY = XY[hi] - XY[-lo];
    
    A = tY * tXX - tX * tXY;
    B = tN * tXY - tX * tY;
    D = tN * tXX - tX * tX;
    R = (A + x * B) / D;
    if (R < 0.f)
      R = 0.f;
    
    noise[i] = R - offset;
  }
  
  for ( ;; i++, x += 1.f) {
    
    lo = b[i] >> 16;
    hi = b[i] & 0xffff;
    if(hi>=n)break;
    
    tN = N[hi] - N[lo];
    tX = X[hi] - X[lo];
    tXX = XX[hi] - XX[lo];
    tY = Y[hi] - Y[lo];
    tXY = XY[hi] - XY[lo];
    
    A = tY * tXX - tX * tXY;
    B = tN * tXY - tX * tY;
    D = tN * tXX - tX * tX;
    R = (A + x * B) / D;
    if (R < 0.f) R = 0.f;
    
    noise[i] = R - offset;
  }
  for ( ; i < n; i++, x += 1.f) {
    
    R = (A + x * B) / D;
    if (R < 0.f) R = 0.f;
    
    noise[i] = R - offset;
  }
  
  if (fixed <= 0) return;
  
  for (i = 0, x = 0.f;; i++, x += 1.f) {
    hi = i + fixed / 2;
    lo = hi - fixed;
    if(lo>=0)break;

    tN = N[hi] + N[-lo];
    tX = X[hi] - X[-lo];
    tXX = XX[hi] + XX[-lo];
    tY = Y[hi] + Y[-lo];
    tXY = XY[hi] - XY[-lo];
    
    
    A = tY * tXX - tX * tXY;
    B = tN * tXY - tX * tY;
    D = tN * tXX - tX * tX;
    R = (A + x * B) / D;

    if (R - offset < noise[i]) noise[i] = R - offset;
  }
  for ( ;; i++, x += 1.f) {
    
    hi = i + fixed / 2;
    lo = hi - fixed;
    if(hi>=n)break;
    
    tN = N[hi] - N[lo];
    tX = X[hi] - X[lo];
    tXX = XX[hi] - XX[lo];
    tY = Y[hi] - Y[lo];
    tXY = XY[hi] - XY[lo];
    
    A = tY * tXX - tX * tXY;
    B = tN * tXY - tX * tY;
    D = tN * tXX - tX * tX;
    R = (A + x * B) / D;
    
    if (R - offset < noise[i]) noise[i] = R - offset;
  }
  for ( ; i < n; i++, x += 1.f) {
    R = (A + x * B) / D;
    if (R - offset < noise[i]) noise[i] = R - offset;
  }
#endif														/* SSE Optimize */
}

static float FLOOR1_fromdB_INV_LOOKUP[256]={
  0.F, 8.81683e+06F, 8.27882e+06F, 7.77365e+06F, // 1-4
  7.29930e+06F, 6.85389e+06F, 6.43567e+06F, 6.04296e+06F, // 5-8
  5.67422e+06F, 5.32798e+06F, 5.00286e+06F, 4.69759e+06F, // 9-12
  4.41094e+06F, 4.14178e+06F, 3.88905e+06F, 3.65174e+06F, // 13-16
  3.42891e+06F, 3.21968e+06F, 3.02321e+06F, 2.83873e+06F, // 17-20
  2.66551e+06F, 2.50286e+06F, 2.35014e+06F, 2.20673e+06F, // 21-24
  2.07208e+06F, 1.94564e+06F, 1.82692e+06F, 1.71544e+06F, // 25-28
  1.61076e+06F, 1.51247e+06F, 1.42018e+06F, 1.33352e+06F, // 29-32
  1.25215e+06F, 1.17574e+06F, 1.10400e+06F, 1.03663e+06F, // 33-36
  973377.F, 913981.F, 858210.F, 805842.F, // 37-40
  756669.F, 710497.F, 667142.F, 626433.F, // 41-44
  588208.F, 552316.F, 518613.F, 486967.F, // 45-48
  457252.F, 429351.F, 403152.F, 378551.F, // 49-52
  355452.F, 333762.F, 313396.F, 294273.F, // 53-56
  276316.F, 259455.F, 243623.F, 228757.F, // 57-60
  214798.F, 201691.F, 189384.F, 177828.F, // 61-64
  166977.F, 156788.F, 147221.F, 138237.F, // 65-68
  129802.F, 121881.F, 114444.F, 107461.F, // 69-72
  100903.F, 94746.3F, 88964.9F, 83536.2F, // 73-76
  78438.8F, 73652.5F, 69158.2F, 64938.1F, // 77-80
  60975.6F, 57254.9F, 53761.2F, 50480.6F, // 81-84
  47400.3F, 44507.9F, 41792.0F, 39241.9F, // 85-88
  36847.3F, 34598.9F, 32487.7F, 30505.3F, // 89-92
  28643.8F, 26896.0F, 25254.8F, 23713.7F, // 93-96
  22266.7F, 20908.0F, 19632.2F, 18434.2F, // 97-100
  17309.4F, 16253.1F, 15261.4F, 14330.1F, // 101-104
  13455.7F, 12634.6F, 11863.7F, 11139.7F, // 105-108
  10460.0F, 9821.72F, 9222.39F, 8659.64F, // 109-112
  8131.23F, 7635.06F, 7169.17F, 6731.70F, // 113-116
  6320.93F, 5935.23F, 5573.06F, 5232.99F, // 117-120
  4913.67F, 4613.84F, 4332.30F, 4067.94F, // 121-124
  3819.72F, 3586.64F, 3367.78F, 3162.28F, // 125-128
  2969.31F, 2788.13F, 2617.99F, 2458.24F, // 129-132
  2308.24F, 2167.39F, 2035.14F, 1910.95F, // 133-136
  1794.35F, 1684.85F, 1582.04F, 1485.51F, // 137-140
  1394.86F, 1309.75F, 1229.83F, 1154.78F, // 141-144
  1084.32F, 1018.15F, 956.024F, 897.687F, // 145-148
  842.910F, 791.475F, 743.179F, 697.830F, // 149-152
  655.249F, 615.265F, 577.722F, 542.469F, // 153-156
  509.367F, 478.286F, 449.101F, 421.696F, // 157-160
  395.964F, 371.803F, 349.115F, 327.812F, // 161-164
  307.809F, 289.026F, 271.390F, 254.830F, // 165-168
  239.280F, 224.679F, 210.969F, 198.096F, // 169-172
  186.008F, 174.658F, 164.000F, 153.993F, // 173-176
  144.596F, 135.773F, 127.488F, 119.708F, // 177-180
  112.404F, 105.545F, 99.1046F, 93.0572F, // 181-184
  87.3788F, 82.0469F, 77.0404F, 72.3394F, // 185-188
  67.9252F, 63.7804F, 59.8885F, 56.2341F, // 189-192
  52.8027F, 49.5807F, 46.5553F, 43.7144F, // 193-196
  41.0470F, 38.5423F, 36.1904F, 33.9821F, // 197-200
  31.9085F, 29.9614F, 28.1332F, 26.4165F, // 201-204
  24.8045F, 23.2910F, 21.8697F, 20.5352F, // 205-208
  19.2822F, 18.1056F, 17.0008F, 15.9634F, // 209-212
  14.9893F, 14.0746F, 13.2158F, 12.4094F, // 213-216
  11.6522F, 10.9411F, 10.2735F, 9.64662F, // 217-220
  9.05798F, 8.50526F, 7.98626F, 7.49894F, // 221-224
  7.04135F, 6.61169F, 6.20824F, 5.82941F, // 225-228
  5.47370F, 5.13970F, 4.82607F, 4.53158F, // 229-232
  4.25507F, 3.99542F, 3.75162F, 3.52269F, // 233-236
  3.30774F, 3.10590F, 2.91638F, 2.73842F, // 237-240
  2.57132F, 2.41442F, 2.26709F, 2.12875F, // 241-244
  1.99885F, 1.87688F, 1.76236F, 1.65482F, // 245-248
  1.55384F, 1.45902F, 1.36999F, 1.28640F, // 249-252
  1.20790F, 1.13419F, 1.06499F, 1.F // 253-256
};

void _vp_remove_floor(vorbis_look_psy *p,
		      float *mdct,
		      int *codedflr,
		      float *residue,
		      int sliding_lowpass){ 

  int i,n=p->n;
 
  if(sliding_lowpass>n)sliding_lowpass=n;
  
#ifdef __SSE__												/* SSE Optimize */
{
#if	defined(_MSC_VER)
	int j;
	for(j=0;j<256;j+=16)
	{
		_mm_prefetch((const char*)(FLOOR1_fromdB_INV_LOOKUP+j  ), _MM_HINT_NTA);
		_mm_prefetch((const char*)(FLOOR1_fromdB_INV_LOOKUP+j+8), _MM_HINT_NTA);
	}
	_asm{
		push	ebp
		push	ebx
		mov		ecx, sliding_lowpass
		mov		edi, mdct
		mov		esi, codedflr
		mov		ebx, residue
		lea		ecx, [esi+ecx*4]
		align	4
	_vp_remove_floor_0:
		mov		eax, [esi   ]
		mov		edx, [esi+ 4]
		movss	xmm0, FLOOR1_fromdB_INV_LOOKUP[eax*4]
		movss	xmm1, FLOOR1_fromdB_INV_LOOKUP[edx*4]
		mov		eax, [esi+ 8]
		mov		edx, [esi+12]
		mulss	xmm0, [edi   ]
		mulss	xmm1, [edi+ 4]
		movss	xmm2, FLOOR1_fromdB_INV_LOOKUP[eax*4]
		movss	xmm3, FLOOR1_fromdB_INV_LOOKUP[edx*4]
		mov		eax, [esi+16]
		mov		edx, [esi+20]
		mulss	xmm2, [edi+ 8]
		mulss	xmm3, [edi+12]
		movss	xmm4, FLOOR1_fromdB_INV_LOOKUP[eax*4]
		movss	xmm5, FLOOR1_fromdB_INV_LOOKUP[edx*4]
		mov		eax, [esi+24]
		mov		edx, [esi+28]
		movss	[ebx   ], xmm0
		movss	[ebx+ 4], xmm1
		movss	xmm6, FLOOR1_fromdB_INV_LOOKUP[eax*4]
		movss	xmm7, FLOOR1_fromdB_INV_LOOKUP[edx*4]
		mulss	xmm4, [edi+16]
		mulss	xmm5, [edi+20]
		mov		eax, [esi+32]
		mov		edx, [esi+36]
		movss	[ebx+ 8], xmm2
		movss	[ebx+12], xmm3
		movss	xmm0, FLOOR1_fromdB_INV_LOOKUP[eax*4]
		movss	xmm1, FLOOR1_fromdB_INV_LOOKUP[edx*4]
		mulss	xmm6, [edi+24]
		mulss	xmm7, [edi+28]
		mov		eax, [esi+40]
		mov		edx, [esi+44]
		movss	[ebx+16], xmm4
		movss	[ebx+20], xmm5
		mulss	xmm0, [edi+32]
		mulss	xmm1, [edi+36]
		movss	[ebx+24], xmm6
		movss	[ebx+28], xmm7
		movss	xmm2, FLOOR1_fromdB_INV_LOOKUP[eax*4]
		movss	xmm3, FLOOR1_fromdB_INV_LOOKUP[edx*4]
		mov		eax, [esi+48]
		mov		edx, [esi+52]
		mulss	xmm2, [edi+40]
		mulss	xmm3, [edi+44]
		movss	xmm4, FLOOR1_fromdB_INV_LOOKUP[eax*4]
		movss	xmm5, FLOOR1_fromdB_INV_LOOKUP[edx*4]
		mov		eax, [esi+56]
		mov		edx, [esi+60]
		movss	[ebx+32], xmm0
		movss	[ebx+36], xmm1
		movss	xmm6, FLOOR1_fromdB_INV_LOOKUP[eax*4]
		movss	xmm7, FLOOR1_fromdB_INV_LOOKUP[edx*4]
		mulss	xmm4, [edi+48]
		mulss	xmm5, [edi+52]
		movss	[ebx+40], xmm2
		movss	[ebx+44], xmm3
		mulss	xmm6, [edi+56]
		mulss	xmm7, [edi+60]
		movss	[ebx+48], xmm4
		movss	[ebx+52], xmm5
		lea		ebx, [ebx+64]
		lea		esi, [esi+64]
		lea		edi, [edi+64]
		movss	[ebx+56-64], xmm6
		movss	[ebx+60-64], xmm7

		cmp		esi, ecx
		jl		_vp_remove_floor_0
		pop		ebx
		pop		ebp
	};
  for(i=sliding_lowpass;i<n;i++)
    residue[i]=0.;
#else
	int j;
	float *work = (float*)_ogg_alloca(sliding_lowpass*sizeof(float));

	for(j=0;j<256;j+=16)
	{
		_mm_prefetch((const char*)(FLOOR1_fromdB_INV_LOOKUP+j  ), _MM_HINT_NTA);
		_mm_prefetch((const char*)(FLOOR1_fromdB_INV_LOOKUP+j+8), _MM_HINT_NTA);
	}
	for(i=0;i<sliding_lowpass;i+=4)
	{
		work[i  ]	 = FLOOR1_fromdB_INV_LOOKUP[codedflr[i  ]];
		work[i+1]	 = FLOOR1_fromdB_INV_LOOKUP[codedflr[i+1]];
		work[i+2]	 = FLOOR1_fromdB_INV_LOOKUP[codedflr[i+2]];
		work[i+3]	 = FLOOR1_fromdB_INV_LOOKUP[codedflr[i+3]];
	}
	for(i=0;i<sliding_lowpass;i+=16)
	{
		__m128	XMM0, XMM1, XMM2, XMM3;
		__m128	XMM4, XMM5, XMM6, XMM7;
		XMM0	 = _mm_load_ps(mdct+i   );
		XMM4	 = _mm_load_ps(work+i   );
		XMM1	 = _mm_load_ps(mdct+i+ 4);
		XMM5	 = _mm_load_ps(work+i+ 4);
		XMM2	 = _mm_load_ps(mdct+i+ 8);
		XMM6	 = _mm_load_ps(work+i+ 8);
		XMM3	 = _mm_load_ps(mdct+i+12);
		XMM7	 = _mm_load_ps(work+i+12);
		XMM0	 = _mm_mul_ps(XMM0, XMM4);
		XMM1	 = _mm_mul_ps(XMM1, XMM5);
		XMM2	 = _mm_mul_ps(XMM2, XMM6);
		XMM3	 = _mm_mul_ps(XMM3, XMM7);
		_mm_store_ps(residue+i   , XMM0);
		_mm_store_ps(residue+i+ 4, XMM1);
		_mm_store_ps(residue+i+ 8, XMM2);
		_mm_store_ps(residue+i+12, XMM3);
	}
#endif
}
#else														/* SSE Optimize */
  for(i=0;i<sliding_lowpass;i++){
    residue[i]=
      mdct[i]*FLOOR1_fromdB_INV_LOOKUP[codedflr[i]];
  }
#endif														/* SSE Optimize */

  for(;i<n;i++)
    residue[i]=0.;
}

void _vp_noisemask(vorbis_look_psy *p,
		   float noise_compand_level,
		   float *logmdct, 
		   float *logmask){

  int i,n=p->n;
#ifdef __SSE__												/* SSE Optimize */
	float *work		 = (float*)_ogg_alloca(n*sizeof(*work)*2);
	float *bwork	 = (float*)_ogg_alloca(n*sizeof(float)*5);

#else														/* SSE Optimize */
  float *work=alloca(n*sizeof(*work));
#endif														/* SSE Optimize */

#ifdef __SSE__												/* SSE Optimize */
	bark_noise_hybridmp(p,logmdct,logmask,
		      140.,-1, bwork, work+n);

	for(i=0;i<n;i+=16)
	{
		__m128	XMM0, XMM1, XMM2, XMM3;
		__m128	XMM4, XMM5, XMM6, XMM7;
		XMM0	 = _mm_load_ps(logmdct+i   );
		XMM4	 = _mm_load_ps(logmask+i   );
		XMM1	 = _mm_load_ps(logmdct+i+ 4);
		XMM5	 = _mm_load_ps(logmask+i+ 4);
		XMM2	 = _mm_load_ps(logmdct+i+ 8);
		XMM6	 = _mm_load_ps(logmask+i+ 8);
		XMM3	 = _mm_load_ps(logmdct+i+12);
		XMM7	 = _mm_load_ps(logmask+i+12);
		XMM0	 = _mm_sub_ps(XMM0, XMM4);
		XMM1	 = _mm_sub_ps(XMM1, XMM5);
		XMM2	 = _mm_sub_ps(XMM2, XMM6);
		XMM3	 = _mm_sub_ps(XMM3, XMM7);
		_mm_store_ps(work+i   , XMM0);
		_mm_store_ps(work+i+ 4, XMM1);
		_mm_store_ps(work+i+ 8, XMM2);
		_mm_store_ps(work+i+12, XMM3);
	}

	bark_noise_hybridmp(p,work,logmask,0.,
		      p->vi->noisewindowfixed, bwork, work+n);
#else														/* SSE Optimize */
  bark_noise_hybridmp(n,p->bark,logmdct,logmask,
		      140.,-1);

  for(i=0;i<n;i++)work[i]=logmdct[i]-logmask[i];

  bark_noise_hybridmp(n,p->bark,work,logmask,0.,
		      p->vi->noisewindowfixed);
#endif														/* SSE Optimize */

#ifdef __SSE__												/* SSE Optimize */
	for(i=0;i<n;i+=16)
	{
		__m128	XMM0, XMM1, XMM2, XMM3;
		__m128	XMM4, XMM5, XMM6, XMM7;
		XMM0	 = _mm_load_ps(logmdct+i   );
		XMM4	 = _mm_load_ps(work+i   );
		XMM1	 = _mm_load_ps(logmdct+i+ 4);
		XMM5	 = _mm_load_ps(work+i+ 4);
		XMM2	 = _mm_load_ps(logmdct+i+ 8);
		XMM6	 = _mm_load_ps(work+i+ 8);
		XMM3	 = _mm_load_ps(logmdct+i+12);
		XMM7	 = _mm_load_ps(work+i+12);
		XMM0	 = _mm_sub_ps(XMM0, XMM4);
		XMM1	 = _mm_sub_ps(XMM1, XMM5);
		XMM2	 = _mm_sub_ps(XMM2, XMM6);
		XMM3	 = _mm_sub_ps(XMM3, XMM7);
		_mm_store_ps(work+i   , XMM0);
		_mm_store_ps(work+i+ 4, XMM1);
		_mm_store_ps(work+i+ 8, XMM2);
		_mm_store_ps(work+i+12, XMM3);
	}
#else														/* SSE Optimize */
  for(i=0;i<n;i++)work[i]=logmdct[i]-work[i];
#endif														/* SSE Optimize */
  
#if 0
  {
    static int seq=0;

    float work2[n];
    for(i=0;i<n;i++){
      work2[i]=logmask[i]+work[i];
    }
    
    if(seq&1)
      _analysis_output("median2R",seq/2,work,n,1,0,0);
    else
      _analysis_output("median2L",seq/2,work,n,1,0,0);
    
    if(seq&1)
      _analysis_output("envelope2R",seq/2,work2,n,1,0,0);
    else
      _analysis_output("envelope2L",seq/2,work2,n,1,0,0);
    seq++;
  }
#endif

  /* aoTuV M5 extension */
  i=0;
  if((p->vi->noisecompand_high[NOISE_COMPAND_LEVELS-1] > 1) && (noise_compand_level > 0)){
  	int thter = p->n33p;
  	for(;i<thter;i++){
    	int dB=logmask[i]+.5;
    	if(dB>=NOISE_COMPAND_LEVELS)dB=NOISE_COMPAND_LEVELS-1;
    	if(dB<0)dB=0;
    	logmask[i]= work[i]+p->vi->noisecompand[dB]-
    	  ((p->vi->noisecompand[dB]-p->vi->noisecompand_high[dB])*noise_compand_level);
  	}
  }
#ifdef __SSE__												/* SSE Optimize */
	{
		static _MM_ALIGN16 const float NCLMAX[4]	 = {
			NOISE_COMPAND_LEVELS-1, NOISE_COMPAND_LEVELS-1,
			NOISE_COMPAND_LEVELS-1, NOISE_COMPAND_LEVELS-1
		};
		int spm4 = (i+15)&(~15);
		for(;i<spm4;i++){
			int dB	 = logmask[i]+.5;
			if(dB>=NOISE_COMPAND_LEVELS)
				dB	 = NOISE_COMPAND_LEVELS-1;
			if(dB<0)
				dB	 = 0;
			logmask[i]	 =  work[i]+p->vi->noisecompand[dB];
		}
		{
			register float* fwork2	 = (float*)(work+n);
			for(i=spm4;i<n;i+=16)
			{
#if	!defined(__SSE2__)
				__m64	MM0, MM1, MM2, MM3, MM4, MM5, MM6, MM7;
#endif
				__m128	XMM0, XMM1, XMM2, XMM3;
				XMM0	 = _mm_load_ps(logmask+i   );
				XMM1	 = _mm_load_ps(logmask+i+ 4);
				XMM2	 = _mm_load_ps(logmask+i+ 8);
				XMM3	 = _mm_load_ps(logmask+i+12);
				XMM0	 = _mm_min_ps(XMM0, PM128(NCLMAX));
				XMM1	 = _mm_min_ps(XMM1, PM128(NCLMAX));
				XMM2	 = _mm_min_ps(XMM2, PM128(NCLMAX));
				XMM3	 = _mm_min_ps(XMM3, PM128(NCLMAX));
				XMM0	 = _mm_max_ps(XMM0, PM128(PFV_0));
				XMM1	 = _mm_max_ps(XMM1, PM128(PFV_0));
				XMM2	 = _mm_max_ps(XMM2, PM128(PFV_0));
				XMM3	 = _mm_max_ps(XMM3, PM128(PFV_0));
#if	defined(__SSE2__)
				_mm_store_si128((__m128i*)(fwork2+i   ), _mm_cvtps_epi32(XMM0));
				_mm_store_si128((__m128i*)(fwork2+i+ 4), _mm_cvtps_epi32(XMM1));
				_mm_store_si128((__m128i*)(fwork2+i+ 8), _mm_cvtps_epi32(XMM2));
				_mm_store_si128((__m128i*)(fwork2+i+12), _mm_cvtps_epi32(XMM3));
			}
#else
				MM0		 = _mm_cvtps_pi32(XMM0);
				MM2		 = _mm_cvtps_pi32(XMM1);
				MM4		 = _mm_cvtps_pi32(XMM2);
				MM6 	 = _mm_cvtps_pi32(XMM3);
				XMM0	 = _mm_movehl_ps(XMM0, XMM0);
				XMM1	 = _mm_movehl_ps(XMM1, XMM1);
				XMM2	 = _mm_movehl_ps(XMM2, XMM2);
				XMM3	 = _mm_movehl_ps(XMM3, XMM3);
				MM1		 = _mm_cvtps_pi32(XMM0);
				MM3		 = _mm_cvtps_pi32(XMM1);
				MM5		 = _mm_cvtps_pi32(XMM2);
				MM7		 = _mm_cvtps_pi32(XMM3);
				PM64(fwork2+i   )	 = MM0;
				PM64(fwork2+i+ 4)	 = MM2;
				PM64(fwork2+i+ 8)	 = MM4;
				PM64(fwork2+i+ 2)	 = MM1;
				PM64(fwork2+i+12)	 = MM6;
				PM64(fwork2+i+ 6)	 = MM3;
				PM64(fwork2+i+10)	 = MM5;
				PM64(fwork2+i+14)	 = MM7;
			}
			_mm_empty();
#endif
			for(i=spm4;i<n;i+=4)
			{
				fwork2[i  ]	 = p->vi->noisecompand[*((int*)(fwork2+i  ))];
				fwork2[i+1]	 = p->vi->noisecompand[*((int*)(fwork2+i+1))];
				fwork2[i+2]	 = p->vi->noisecompand[*((int*)(fwork2+i+2))];
				fwork2[i+3]	 = p->vi->noisecompand[*((int*)(fwork2+i+3))];
			}
			for(i=spm4;i<n;i+=16)
			{
				__m128	XMM0	 = _mm_load_ps(fwork2+i   );
				__m128	XMM4	 = _mm_load_ps(work+i   );
				__m128	XMM1	 = _mm_load_ps(fwork2+i+ 4);
				__m128	XMM5	 = _mm_load_ps(work+i+ 4);
				__m128	XMM2	 = _mm_load_ps(fwork2+i+ 8);
				__m128	XMM6	 = _mm_load_ps(work+i+ 8);
				__m128	XMM3	 = _mm_load_ps(fwork2+i+12);
				__m128	XMM7	 = _mm_load_ps(work+i+12);
				XMM0	 = _mm_add_ps(XMM0, XMM4);
				XMM1	 = _mm_add_ps(XMM1, XMM5);
				XMM2	 = _mm_add_ps(XMM2, XMM6);
				XMM3	 = _mm_add_ps(XMM3, XMM7);
				_mm_store_ps(logmask+i   , XMM0);
				_mm_store_ps(logmask+i+ 4, XMM1);
				_mm_store_ps(logmask+i+ 8, XMM2);
				_mm_store_ps(logmask+i+12, XMM3);
			}
		}
	}
#else														/* SSE Optimize */
  for(;i<n;i++){
    int dB=logmask[i]+.5;
    if(dB>=NOISE_COMPAND_LEVELS)dB=NOISE_COMPAND_LEVELS-1;
    if(dB<0)dB=0;
    logmask[i]= work[i]+p->vi->noisecompand[dB];
  }
#endif														/* SSE Optimize */

}

void _vp_tonemask(vorbis_look_psy *p,
		  float *logfft,
		  float *logmask,
		  float global_specmax,
		  float local_specmax){

  int i,n=p->n;

#ifdef __SSE__												/* SSE Optimize */
	int seedsize = (p->total_octave_lines+31)&(~31);
	float *seed = (float*)_ogg_alloca(sizeof(*seed)*seedsize);
	float att=local_specmax+p->vi->ath_adjatt;
	{
		__m128	XMM0	 = PM128(PNEGINF);
		for(i=0;i<seedsize;i+=32)
		{
			_mm_store_ps(seed+i   , XMM0);
			_mm_store_ps(seed+i+ 4, XMM0);
			_mm_store_ps(seed+i+ 8, XMM0);
			_mm_store_ps(seed+i+12, XMM0);
			_mm_store_ps(seed+i+16, XMM0);
			_mm_store_ps(seed+i+20, XMM0);
			_mm_store_ps(seed+i+24, XMM0);
			_mm_store_ps(seed+i+28, XMM0);
		}
	}
	/* set the ATH (floating below localmax, not global max by a
	   specified att) */
	if(att<p->vi->ath_maxatt)att=p->vi->ath_maxatt;
	
	{
		__m128	pm = _mm_set_ps1(att);
		for(i=0;i<n;i+=16)
		{
			__m128	XMM0, XMM1, XMM2, XMM3;
			XMM0	 = _mm_load_ps(p->ath+i   );
			XMM1	 = _mm_load_ps(p->ath+i+ 4);
			XMM2	 = _mm_load_ps(p->ath+i+ 8);
			XMM3	 = _mm_load_ps(p->ath+i+12);
			XMM0	 = _mm_add_ps(XMM0, pm);
			XMM1	 = _mm_add_ps(XMM1, pm);
			XMM2	 = _mm_add_ps(XMM2, pm);
			XMM3	 = _mm_add_ps(XMM3, pm);
			_mm_store_ps(logmask+i   , XMM0);
			_mm_store_ps(logmask+i+ 4, XMM1);
			_mm_store_ps(logmask+i+ 8, XMM2);
			_mm_store_ps(logmask+i+12, XMM3);
		}
	}
#else														/* SSE Optimize */
  float *seed=alloca(sizeof(*seed)*p->total_octave_lines);
  float att=local_specmax+p->vi->ath_adjatt;
  for(i=0;i<p->total_octave_lines;i++)seed[i]=NEGINF;
  
  /* set the ATH (floating below localmax, not global max by a
     specified att) */
  if(att<p->vi->ath_maxatt)att=p->vi->ath_maxatt;
  
  for(i=0;i<n;i++)
    logmask[i]=p->ath[i]+att;
#endif														/* SSE Optimize */

  /* tone masking */
  seed_loop(p,(const float ***)p->tonecurves,logfft,logmask,seed,global_specmax);
  max_seeds(p,seed,logmask);

}

void _vp_offset_and_mix(vorbis_look_psy *p,
			float *noise,
			float *tone,
			int offset_select,
			float *logmask,
			float *mdct,
			float *logmdct,
			float *lastmdct, float *tempmdct,
			float low_compand,
			int end_block,
			int blocktype, int modenumber,
			int nW_modenumber,
#ifdef	__SSE__												/* SSE Optimize */
			int lW_blocktype, int lW_modenumber, int lW_no,
			float *tlogmdct){
#else														/* SSE Optimize */
			int lW_blocktype, int lW_modenumber, int lW_no){
#endif														/* SSE Optimize */

  int i,j,n=p->n;
  int m2_sw=0,  padth; /* aoTuV for M2 */
  int it_sw, *m3n, m3_count; /* aoTuV for M3 */
  int m4_end, lp_pos, m4_start; /* aoTuV for M4 */
  float de, coeffi, cx; /* aoTuV for M1 */
  float toneth; /* aoTuV for M2 */
  float noise_rate, noise_rate_low, noise_center, rate_mod; /* aoTuV for M3 */
  float m4_thres; /* aoTuV for M4 */
  float toneatt=p->vi->tone_masteratt[offset_select];
#ifdef __SSE__												/* SSE Optimize */
  static _MM_ALIGN16 const float PCOEFFI[4]	 = {-17.2f, -17.2f, -17.2f, -17.2f};
  static __m128 PCX0;
  static __m128 PCX1;
  static _MM_ALIGN16 const float PM160[4] = {-160.f, -160.f, -160.f, -160.f};
  static _MM_ALIGN16 const float PM140[4] = {-140.f, -140.f, -140.f, -140.f};
  static _MM_ALIGN16 const float PP0001[4] = {0.0001f, 0.0001f, 0.0001f, 0.0001f};
  static _MM_ALIGN16 const float PP1[4]   = {0.1f, 0.1f, 0.1f, 0.1f};
  static _MM_ALIGN16 const float P5[4]    = {5.f, 5.f, 5.f, 5.f};
  static _MM_ALIGN16 const float P20[4]	  = {20.f, 20.f, 20.f, 20.f};
  static _MM_ALIGN16 const float P30[4]   = {30.f, 30.f, 30.f, 30.f};
  __m128 PTONEATT;
  __m128 PNOISEMAXSUPP;
  __m128 PLOW_COMPAND;
  __m128 PPADTH;
  __m128 PNOISE_CENTER;
  __m128 PNOISE_RATE;
  __m128 PNOISE_RATE_LOW;
  __m128 PFV_C0, PFV_C1;
  __m128 PM4_THRES;
  int midpoint;
#endif														/* SSE Optimize */

  cx = p->m_val;
  m3n = p->m3n;
  m4_start=p->vi->normal_start;
  m4_end = p->tonecomp_endp;
  m4_thres = p->tonecomp_thres;
  lp_pos=9999;
  
  end_block+=p->vi->normal_partition;
  if(end_block>n)end_block=n;
  
  /* Collapse of low(mid) frequency is prevented. (for 32/44/48kHz q-2) */
  if(low_compand<0 || toneatt<25.)low_compand=0;
  else low_compand*=(toneatt-25.);
  
  /** @ M2 PRE **/
  if(p->vi->normal_thresh<.48){
  	if((cx > 0.5) && !modenumber && blocktype && (n==128)){
    	if(p->vi->normal_thresh>.35) padth = 10+(int)(p->vi->flacint*100);
    	else padth = 10;
    	m2_sw=1;
    }
  }

  /** @ M3 PRE **/
  m3_count = 3;
  if(toneatt < 3) m3_count = 2; // q6~
  if((n == 128) && !modenumber && !blocktype){
  	if(!lW_blocktype && !lW_modenumber){ /* last window "short" - type "impulse" */
  		if(lW_no < 8){
  			/* impulse - @impulse case1 */
  			noise_rate = 0.7-(float)(lW_no-1)/17;
  			noise_center = (float)(lW_no*m3_count);
  		}else{
  			/* impulse - @impulse case2 */
  			noise_rate = 0.3;
  			noise_center = 25;
  			if((lW_no*m3_count) < 24) noise_center = lW_no*m3_count;
  		}
  		if(offset_select == 1){
#ifdef __SSE__												/* SSE Optimize */
		  for(i=0; i<128; i+=16)
		  {
			__m128 XMM0 = _mm_load_ps(tempmdct+i   );
			__m128 XMM1 = _mm_load_ps(tempmdct+i+ 4);
			__m128 XMM2 = _mm_load_ps(tempmdct+i+ 8);
			__m128 XMM3 = _mm_load_ps(tempmdct+i+12);
			XMM0 = _mm_sub_ps(XMM0, PM128(P5));
			XMM1 = _mm_sub_ps(XMM1, PM128(P5));
			XMM2 = _mm_sub_ps(XMM2, PM128(P5));
			XMM3 = _mm_sub_ps(XMM3, PM128(P5));
			_mm_store_ps(tempmdct+i   , XMM0);
			_mm_store_ps(tempmdct+i+ 4, XMM1);
			_mm_store_ps(tempmdct+i+ 8, XMM2);
			_mm_store_ps(tempmdct+i+12, XMM3);
		  }
#else														/* SSE Optimize */
  			for(i=0; i<128; i++) tempmdct[i] -= 5;
#endif														/* SSE Optimize */
  		}
  	}else{ /* non_impulse - @Short(impulse) case */
  		noise_rate = 0.7;
  		noise_center = 0;
  		if(offset_select == 1){
#ifdef __SSE__												/* SSE Optimize */
		  for(i=0; i<128; i+=16)
		  {
			__m128 XMM0 = _mm_load_ps(lastmdct+i   );
			__m128 XMM1 = _mm_load_ps(lastmdct+i+ 4);
			__m128 XMM2 = _mm_load_ps(lastmdct+i+ 8);
			__m128 XMM3 = _mm_load_ps(lastmdct+i+12);
			XMM0 = _mm_sub_ps(XMM0, PM128(P5));
			XMM1 = _mm_sub_ps(XMM1, PM128(P5));
			XMM2 = _mm_sub_ps(XMM2, PM128(P5));
			XMM3 = _mm_sub_ps(XMM3, PM128(P5));
			_mm_store_ps(tempmdct+i   , XMM0);
			_mm_store_ps(tempmdct+i+ 4, XMM1);
			_mm_store_ps(tempmdct+i+ 8, XMM2);
			_mm_store_ps(tempmdct+i+12, XMM3);
		  }
#else														/* SSE Optimize */
  			for(i=0; i<128; i++) tempmdct[i] = lastmdct[i] - 5;
#endif														/* SSE Optimize */
  		}
  	}
  	noise_rate_low = 0;
  	it_sw = 1;
  }else{
  	it_sw = 0;
  }
  
  /** @ M3&M4 PRE **/
  if(cx < 0.5){
  	it_sw = 0; /* for M3 */
  	m4_end=end_block; /* for M4 */
  }else if(p->vi->normal_thresh>1.){
  	m4_start = 9999;
  }else{
  	if(m4_end>end_block)lp_pos=m4_end;
  	else lp_pos=end_block;
  }

#ifdef __SSE__												/* SSE Optimize */
/*
  printf("M4S = %d\n", m4_start);
  printf("M4E = %d\n", m4_end);
  printf("LP  = %d\n\n", lp_pos);
*/
  if(offset_select==1)
  {
	PTONEATT        = _mm_set_ps1(toneatt);
	PNOISEMAXSUPP   = _mm_set_ps1(p->vi->noisemaxsupp);
	PLOW_COMPAND    = _mm_set_ps1(low_compand);
	PPADTH          = _mm_set_ps1(1.0f/padth);
	PNOISE_CENTER   = _mm_set_ps1(noise_center);
	PNOISE_RATE     = _mm_set_ps1(noise_rate);
	PNOISE_RATE_LOW = _mm_set_ps1(noise_rate_low);
	PCX0            = _mm_set_ps1(-0.005 *cx);
	PCX1            = _mm_set_ps1(-0.0003*cx);
	PFV_C0          = _mm_set_ps1(1.0f-17.2f*cx*0.005f);
	PFV_C1          = _mm_set_ps1(1.0f-17.2f*cx*0.0003f);
	PM4_THRES       = _mm_set_ps1(m4_thres);
	if(it_sw){
	  for(i=0;i<n;i+=16)
	  {
		__m128	XMM0	 = _mm_load_ps(logmdct+i   );
		__m128	XMM1	 = _mm_load_ps(logmdct+i+ 4);
		__m128	XMM2	 = _mm_load_ps(logmdct+i+ 8);
		__m128	XMM3	 = _mm_load_ps(logmdct+i+12);
		XMM0	 = _mm_sub_ps(XMM0, PM128(P5));
		XMM1	 = _mm_sub_ps(XMM1, PM128(P5));
		XMM2	 = _mm_sub_ps(XMM2, PM128(P5));
		XMM3	 = _mm_sub_ps(XMM3, PM128(P5));
		_mm_store_ps(tlogmdct+i   , XMM0);
		_mm_store_ps(tlogmdct+i+ 4, XMM1);
		_mm_store_ps(tlogmdct+i+ 8, XMM2);
		_mm_store_ps(tlogmdct+i+12, XMM3);
	  }
	}
	midpoint	 = (m3n[1]+4)&(~3);
	for(i=0;i<midpoint;i++)
	{
	  float val= noise[i]+p->noiseoffset[1][i];
	  float tval= tone[i]+toneatt;
	  tval-=low_compand;
	  if(val>p->vi->noisemaxsupp)val=p->vi->noisemaxsupp;

	  if(m2_sw){
		if((logmdct[i]-lastmdct[i]) > 20){
		  if(i > m3n[3]) val -= (logmdct[i]-lastmdct[i]-20)/padth;
		  else val -= (logmdct[i]-lastmdct[i]-20)/(padth+padth);
		}
	  }

	  if(it_sw){
		const float* ptempbuf = PTEMP_BFN[temp_bfn[i]];
		for(j=1; j<=temp_bfn[i]; j++,ptempbuf++){
		  float tempbuf = logmdct[i]+(*ptempbuf);
		  if( (tempmdct[i+j] < tempbuf) && (tempmdct[i+j] < tlogmdct[i+j]) )
			tempmdct[i+j] = tlogmdct[i+j];
		}
		if(val > tval){
		  if( (val>lastmdct[i]) && (logmdct[i]>(tempmdct[i]+noise_center)) ){
			float valmask=0;
			tempmdct[i] = logmdct[i];

			if(logmdct[i]>lastmdct[i]){
			  rate_mod = noise_rate;
			}else{
			  rate_mod = noise_rate_low;
			}
			if(i > m3n[1]){
			  if((val-tval)>30) valmask=((val-tval-30)/10+30)*rate_mod;
			  else valmask=(val-tval)*rate_mod;
			}else if(i > m3n[2]){
			  if((val-tval)>20) valmask=((val-tval-20)/10+20)*rate_mod;
			  else valmask=(val-tval)*rate_mod;
			}else if(i > m3n[3]){
			  if((val-tval)>10) valmask=((val-tval-10)/10+10)*rate_mod*0.5;
			  else valmask=(val-tval)*rate_mod*0.5;
			}else{
			  if((val-tval)>10) valmask=((val-tval-10)/10+10)*rate_mod*0.3;
			  else valmask=(val-tval)*rate_mod*0.3;
			}
			if((val-valmask)>lastmdct[i])val-=valmask;
			else val=lastmdct[i];
		  }
		}
	  }

	  if(val>tval){
		logmask[i]=val;
	  }else logmask[i]=tval;

	  coeffi = -17.2;
	  val = val - logmdct[i];

	  if(val > coeffi){
		de = 1.0-((val-coeffi)*0.005*cx);
		if(de < 0) de = 0.0001;
	  }else
		de = 1.0-((val-coeffi)*0.0003*cx);
	  mdct[i] *= de;
	}
	if(n<=m4_start&&n<=lp_pos)
	{
	  for(;i<n;i+=4)
	  {
		__m128 PVAL  = _mm_load_ps(noise+i);
		__m128 PTVAL = _mm_load_ps(tone+i);
		PVAL  = _mm_add_ps(PVAL, PM128(p->noiseoffset[1]+i));
		PTVAL = _mm_add_ps(PTVAL, PTONEATT);
		PVAL  = _mm_min_ps(PVAL, PNOISEMAXSUPP);
		PTVAL = _mm_sub_ps(PTVAL, PLOW_COMPAND);
		if(m2_sw)
		{
		  __m128 XMM0 = _mm_load_ps(logmdct+i);
		  __m128 XMM1 = _mm_load_ps(lastmdct+i);
		  __m128 XMM2 = _mm_load_ps(P20);
		  XMM0 = _mm_sub_ps(XMM0, XMM1);
		  XMM0 = _mm_sub_ps(XMM0, XMM2);
		  XMM1 = XMM0;
		  XMM0 = _mm_mul_ps(XMM0, PPADTH);
		  XMM1 = _mm_cmplt_ps(XMM1, PM128(PFV_0));
		  XMM1 = _mm_andnot_ps(XMM1, XMM0);
		  PVAL = _mm_sub_ps(PVAL, XMM1);
		}
		if(it_sw){
		  int k;
		  for(k=0;k<4;k++)
		  {
			const float* ptempbuf = PTEMP_BFN[temp_bfn[i+k]];
			__m128 PLOGMDCT = _mm_set_ps1(logmdct[i+k]);
			if(((i+k)&3)==3)
			{
			  for(j=1; j<temp_bfn8[i+k]; j+=8, ptempbuf+=8)
			  {
				__m128 XMM0, XMM1, XMM2;
				__m128 XMM3, XMM4, XMM5;
				XMM0 = _mm_load_ps(ptempbuf  );
				XMM3 = _mm_load_ps(ptempbuf+4);
				XMM1 = _mm_load_ps(tempmdct+i+j+k  );
				XMM4 = _mm_load_ps(tempmdct+i+j+k+4);
				XMM0 = _mm_add_ps(XMM0, PLOGMDCT);
				XMM3 = _mm_add_ps(XMM3, PLOGMDCT);
				XMM2 = _mm_load_ps(tlogmdct+i+j+k  );
				XMM5 = _mm_load_ps(tlogmdct+i+j+k+4);
				XMM0 = _mm_min_ps(XMM0, XMM2);
				XMM3 = _mm_min_ps(XMM3, XMM5);
				XMM0 = _mm_cmple_ps(XMM0, XMM1);
				XMM3 = _mm_cmple_ps(XMM3, XMM4);
				XMM1 = _mm_and_ps(XMM1, XMM0);
				XMM4 = _mm_and_ps(XMM4, XMM3);
				XMM0 = _mm_andnot_ps(XMM0, XMM2);
				XMM3 = _mm_andnot_ps(XMM3, XMM5);
				XMM1 = _mm_or_ps(XMM1, XMM0);
				XMM4 = _mm_or_ps(XMM4, XMM3);
				_mm_store_ps(tempmdct+i+j+k  , XMM1);
				_mm_store_ps(tempmdct+i+j+k+4, XMM4);
			  }
			  for(; j<temp_bfn4[i+k]; j+=4, ptempbuf+=4)
			  {
				__m128 XMM0, XMM1, XMM2;
				XMM0 = _mm_load_ps(ptempbuf);
				XMM1 = _mm_load_ps(tempmdct+i+j+k);
				XMM0 = _mm_add_ps(XMM0, PLOGMDCT);
				XMM2 = _mm_load_ps(tlogmdct+i+j+k);
				XMM0 = _mm_min_ps(XMM0, XMM2);
				XMM0 = _mm_cmple_ps(XMM0, XMM1);
				XMM1 = _mm_and_ps(XMM1, XMM0);
				XMM0 = _mm_andnot_ps(XMM0, XMM2);
				XMM1 = _mm_or_ps(XMM1, XMM0);
				_mm_store_ps(tempmdct+i+j+k, XMM1);
			  }
			}
			else
			{
			  for(j=1; j<temp_bfn8[i+k]; j+=8, ptempbuf+=8)
			  {
				__m128 XMM0, XMM1, XMM2;
				__m128 XMM3, XMM4, XMM5;
				XMM0 = _mm_load_ps(ptempbuf  );
				XMM3 = _mm_load_ps(ptempbuf+4);
				XMM1 = _mm_lddqu_ps(tempmdct+i+j+k  );
				XMM4 = _mm_lddqu_ps(tempmdct+i+j+k+4);
				XMM0 = _mm_add_ps(XMM0, PLOGMDCT);
				XMM3 = _mm_add_ps(XMM3, PLOGMDCT);
				XMM2 = _mm_lddqu_ps(tlogmdct+i+j+k  );
				XMM5 = _mm_lddqu_ps(tlogmdct+i+j+k+4);
				XMM0 = _mm_min_ps(XMM0, XMM2);
				XMM3 = _mm_min_ps(XMM3, XMM5);
				XMM0 = _mm_cmple_ps(XMM0, XMM1);
				XMM3 = _mm_cmple_ps(XMM3, XMM4);
				XMM1 = _mm_and_ps(XMM1, XMM0);
				XMM4 = _mm_and_ps(XMM4, XMM3);
				XMM0 = _mm_andnot_ps(XMM0, XMM2);
				XMM3 = _mm_andnot_ps(XMM3, XMM5);
				XMM1 = _mm_or_ps(XMM1, XMM0);
				XMM4 = _mm_or_ps(XMM4, XMM3);
				_mm_storeu_ps(tempmdct+i+j+k  , XMM1);
				_mm_storeu_ps(tempmdct+i+j+k+4, XMM4);
			  }
			  for(; j<temp_bfn4[i+k]; j+=4, ptempbuf+=4)
			  {
				__m128 XMM0, XMM1, XMM2;
				XMM0 = _mm_load_ps(ptempbuf);
				XMM1 = _mm_lddqu_ps(tempmdct+i+j+k  );
				XMM0 = _mm_add_ps(XMM0, PLOGMDCT);
				XMM2 = _mm_lddqu_ps(tlogmdct+i+j+k  );
				XMM0 = _mm_min_ps(XMM0, XMM2);
				XMM0 = _mm_cmple_ps(XMM0, XMM1);
				XMM1 = _mm_and_ps(XMM1, XMM0);
				XMM0 = _mm_andnot_ps(XMM0, XMM2);
				XMM1 = _mm_or_ps(XMM1, XMM0);
				_mm_storeu_ps(tempmdct+i+j+k, XMM1);
			  }
			}
			switch(temp_bfn[i+k]-j)
			{
			case 0 :
			  {
				__m128 XMM0, XMM1, XMM2;
				XMM0 = _mm_load_ss(ptempbuf);
				XMM1 = _mm_load_ss(tempmdct+i+j+k);
				XMM0 = _mm_add_ss(XMM0, PLOGMDCT);
				XMM2 = _mm_load_ss(tlogmdct+i+j+k);
				XMM0 = _mm_min_ss(XMM0, XMM2);
				XMM0 = _mm_cmple_ss(XMM0, XMM1);
				XMM1 = _mm_and_ps(XMM1, XMM0);
				XMM0 = _mm_andnot_ps(XMM0, XMM2);
				XMM1 = _mm_or_ps(XMM1, XMM0);
				_mm_store_ss(tempmdct+i+j+k, XMM1);
			  }
			  break;
			case 1 :
			  {
				__m128 XMM0, XMM1, XMM2;
#pragma warning(disable : 592)
				XMM0 = _mm_loadl_pi(XMM0, (__m64*)ptempbuf);
				XMM1 = _mm_loadl_pi(XMM1, (__m64*)(tempmdct+i+j+k));
				XMM0 = _mm_add_ps(XMM0, PLOGMDCT);
				XMM2 = _mm_loadl_pi(XMM2, (__m64*)(tlogmdct+i+j+k));
#pragma warning(default : 592)
				XMM0 = _mm_min_ps(XMM0, XMM2);
				XMM0 = _mm_cmple_ps(XMM0, XMM1);
				XMM1 = _mm_and_ps(XMM1, XMM0);
				XMM0 = _mm_andnot_ps(XMM0, XMM2);
				XMM1 = _mm_or_ps(XMM1, XMM0);
				_mm_storel_pi((__m64*)(tempmdct+i+j+k), XMM1);
			  }
			  break;
			case 2 :
			  {
				__m128 XMM0, XMM1, XMM2;
				XMM0 = _mm_load_ps(ptempbuf);
				XMM1 = _mm_lddqu_ps(tempmdct+i+j+k);
				XMM0 = _mm_add_ps(XMM0, PLOGMDCT);
				XMM2 = _mm_lddqu_ps(tlogmdct+i+j+k);
				XMM0 = _mm_min_ps(XMM0, XMM2);
				XMM0 = _mm_cmple_ps(XMM0, XMM1);
				XMM1 = _mm_and_ps(XMM1, XMM0);
				XMM0 = _mm_andnot_ps(XMM0, XMM2);
				XMM1 = _mm_or_ps(XMM1, XMM0);
				_mm_storel_pi((__m64*)(tempmdct+i+j+k), XMM1);
				XMM1 = _mm_movehl_ps(XMM1, XMM1);
				_mm_store_ss(tempmdct+i+j+k+2, XMM1);
			  }
			  break;
			case 3 :
			  break;
			}
		  }
		  {
			__m128 XMM0 = _mm_cmpgt_ps(PVAL, _mm_max_ps(PTVAL, PM128(lastmdct+i)));
			if(_mm_movemask_ps(XMM0))
			{
			  __m128 XMM1 = _mm_cmpgt_ps(PM128(logmdct+i), _mm_add_ps(PM128(tempmdct+i), PNOISE_CENTER));
			  __m128 XMM2, XMM3, XMM4;
			  XMM0 = _mm_and_ps(XMM0, XMM1);
			  if(_mm_movemask_ps(XMM0))
			  {
				XMM1 = _mm_load_ps(logmdct+i);
				XMM2 = XMM0;
				XMM3 = XMM1;
				XMM3 = _mm_or_ps(
				  _mm_and_ps(XMM3, XMM2),
				  _mm_andnot_ps(XMM2, PM128(tempmdct+i))
				);
				_mm_store_ps(tempmdct+i, XMM3);
				XMM1 = _mm_cmpgt_ps(XMM1, PM128(lastmdct+i));
				XMM2 = _mm_or_ps(
				  _mm_and_ps(PNOISE_RATE, XMM1),
				  _mm_andnot_ps(XMM1, PNOISE_RATE_LOW)
				);	/* rate_mod */
				XMM1 = _mm_sub_ps(PVAL, PTVAL);
				XMM3 = XMM1;
				XMM1 = _mm_sub_ps(XMM1, PM128(P30));
				XMM4 = _mm_cmpgt_ps(XMM1, PM128(PFV_0));
				XMM1 = _mm_mul_ps(XMM1, PM128(PP1));
				XMM1 = _mm_add_ps(XMM1, PM128(P30));
				XMM1 = _mm_and_ps(XMM1, XMM4);
				XMM4 = _mm_andnot_ps(XMM4, XMM3);
				XMM1 = _mm_or_ps(XMM1, XMM4);
				XMM1 = _mm_mul_ps(XMM1, XMM2);
				XMM3 = PVAL;
				XMM3 = _mm_sub_ps(XMM3, XMM1);
				XMM3 = _mm_max_ps(XMM3, PM128(lastmdct+i));
				XMM3 = _mm_and_ps(XMM3, XMM0);
				XMM0 = _mm_andnot_ps(XMM0, PVAL);
				PVAL = _mm_or_ps(XMM3, XMM0);
			  }
			}
		  }
		}
		_mm_store_ps(logmask+i, _mm_max_ps(PVAL, PTVAL));
		{
		  __m128 XMM0, XMM1, XMM2;
		  PVAL = _mm_sub_ps(PVAL, PM128(logmdct+i));
		  XMM0 = PVAL;
		  XMM1 = PVAL;
		  XMM2 = PVAL;
		  XMM0 = _mm_cmpgt_ps(XMM0, PM128(PCOEFFI));
		  XMM1 = _mm_mul_ps(XMM1, PCX0);
		  XMM2 = _mm_mul_ps(XMM2, PCX1);
		  XMM1 = _mm_add_ps(XMM1, PFV_C0);
		  XMM2 = _mm_add_ps(XMM2, PFV_C1);
		  XMM1 = _mm_max_ps(XMM1, PM128(PP0001));
		  XMM1 = _mm_and_ps(XMM1, XMM0);
		  XMM0 = _mm_andnot_ps(XMM0, XMM2);
		  XMM1 = _mm_or_ps(XMM1, XMM0);
		  XMM1 = _mm_mul_ps(XMM1, PM128(mdct+i));
		  _mm_store_ps(mdct+i, XMM1);
		}
	  }
	}
	else if(lp_pos>=m4_end&&n>lp_pos)
	{
	  char RunMode[2048];
	  j	 = (m3n[1]+4)&(~3);
	  midpoint	 = m4_start&(~3);
	  for(;j<midpoint;j+=4)
		RunMode[j] = 1;	/* SSE-1 */
	  midpoint	 = (m4_start+4)&(~3);	/* i>m4_start is not 1=>m4_start */
	  for(;j<midpoint;j+=4)
		RunMode[j] = 0;	/* Normal */
	  midpoint	 = m4_end&(~3);
	  for(;j<midpoint;j+=4)
		RunMode[j] = 2;	/* SSE-2 */
	  midpoint	 = (m4_end+3)&(~3);
	  for(;j<midpoint;j+=4)
		RunMode[j] = 0;	/* Normal */
	  midpoint	 = lp_pos&(~3);
	  for(;j<midpoint;j+=4)
		RunMode[j] = 1;	/* SSE-1 */
	  midpoint	 = (lp_pos+3)&(~3);
	  for(;j<midpoint;j+=4)
		RunMode[j] = 3;	/* SSE-3 */
	  for(;j<n;j+=4)
		RunMode[j] = 4;	/* SSE-4 */
	  for(;i<n;i+=4)
	  {
		__m128 PVAL  = _mm_load_ps(noise+i);
		__m128 PTVAL = _mm_load_ps(tone+i);
		PVAL  = _mm_add_ps(PVAL, PM128(p->noiseoffset[1]+i));
		PTVAL = _mm_add_ps(PTVAL, PTONEATT);
		PVAL  = _mm_min_ps(PVAL, PNOISEMAXSUPP);
		PTVAL = _mm_sub_ps(PTVAL, PLOW_COMPAND);
		if(m2_sw)
		{
		  __m128 XMM0 = _mm_load_ps(logmdct+i);
		  __m128 XMM1 = _mm_load_ps(lastmdct+i);
		  __m128 XMM2 = _mm_load_ps(P20);
		  XMM0 = _mm_sub_ps(XMM0, XMM1);
		  XMM0 = _mm_sub_ps(XMM0, XMM2);
		  XMM1 = XMM0;
		  XMM0 = _mm_mul_ps(XMM0, PPADTH);
		  XMM1 = _mm_cmplt_ps(XMM1, PM128(PFV_0));
		  XMM1 = _mm_andnot_ps(XMM1, XMM0);
		  PVAL = _mm_sub_ps(PVAL, XMM1);
		}
		if(it_sw){
		  int k;
		  for(k=0;k<4;k++)
		  {
			const float* ptempbuf = PTEMP_BFN[temp_bfn[i+k]];
			__m128 PLOGMDCT = _mm_set_ps1(logmdct[i+k]);
			if(((i+k)&3)==3)
			{
			  for(j=1; j<temp_bfn8[i+k]; j+=8, ptempbuf+=8)
			  {
				__m128 XMM0, XMM1, XMM2;
				__m128 XMM3, XMM4, XMM5;
				XMM0 = _mm_load_ps(ptempbuf  );
				XMM3 = _mm_load_ps(ptempbuf+4);
				XMM1 = _mm_load_ps(tempmdct+i+j+k  );
				XMM4 = _mm_load_ps(tempmdct+i+j+k+4);
				XMM0 = _mm_add_ps(XMM0, PLOGMDCT);
				XMM3 = _mm_add_ps(XMM3, PLOGMDCT);
				XMM2 = _mm_load_ps(tlogmdct+i+j+k  );
				XMM5 = _mm_load_ps(tlogmdct+i+j+k+4);
				XMM0 = _mm_min_ps(XMM0, XMM2);
				XMM3 = _mm_min_ps(XMM3, XMM5);
				XMM0 = _mm_cmple_ps(XMM0, XMM1);
				XMM3 = _mm_cmple_ps(XMM3, XMM4);
				XMM1 = _mm_and_ps(XMM1, XMM0);
				XMM4 = _mm_and_ps(XMM4, XMM3);
				XMM0 = _mm_andnot_ps(XMM0, XMM2);
				XMM3 = _mm_andnot_ps(XMM3, XMM5);
				XMM1 = _mm_or_ps(XMM1, XMM0);
				XMM4 = _mm_or_ps(XMM4, XMM3);
				_mm_store_ps(tempmdct+i+j+k  , XMM1);
				_mm_store_ps(tempmdct+i+j+k+4, XMM4);
			  }
			  for(; j<temp_bfn4[i+k]; j+=4, ptempbuf+=4)
			  {
				__m128 XMM0, XMM1, XMM2;
				XMM0 = _mm_load_ps(ptempbuf);
				XMM1 = _mm_load_ps(tempmdct+i+j+k);
				XMM0 = _mm_add_ps(XMM0, PLOGMDCT);
				XMM2 = _mm_load_ps(tlogmdct+i+j+k);
				XMM0 = _mm_min_ps(XMM0, XMM2);
				XMM0 = _mm_cmple_ps(XMM0, XMM1);
				XMM1 = _mm_and_ps(XMM1, XMM0);
				XMM0 = _mm_andnot_ps(XMM0, XMM2);
				XMM1 = _mm_or_ps(XMM1, XMM0);
				_mm_store_ps(tempmdct+i+j+k, XMM1);
			  }
			}
			else
			{
			  for(j=1; j<temp_bfn8[i+k]; j+=8, ptempbuf+=8)
			  {
				__m128 XMM0, XMM1, XMM2;
				__m128 XMM3, XMM4, XMM5;
				XMM0 = _mm_load_ps(ptempbuf  );
				XMM3 = _mm_load_ps(ptempbuf+4);
				XMM1 = _mm_lddqu_ps(tempmdct+i+j+k  );
				XMM4 = _mm_lddqu_ps(tempmdct+i+j+k+4);
				XMM0 = _mm_add_ps(XMM0, PLOGMDCT);
				XMM3 = _mm_add_ps(XMM3, PLOGMDCT);
				XMM2 = _mm_lddqu_ps(tlogmdct+i+j+k  );
				XMM5 = _mm_lddqu_ps(tlogmdct+i+j+k+4);
				XMM0 = _mm_min_ps(XMM0, XMM2);
				XMM3 = _mm_min_ps(XMM3, XMM5);
				XMM0 = _mm_cmple_ps(XMM0, XMM1);
				XMM3 = _mm_cmple_ps(XMM3, XMM4);
				XMM1 = _mm_and_ps(XMM1, XMM0);
				XMM4 = _mm_and_ps(XMM4, XMM3);
				XMM0 = _mm_andnot_ps(XMM0, XMM2);
				XMM3 = _mm_andnot_ps(XMM3, XMM5);
				XMM1 = _mm_or_ps(XMM1, XMM0);
				XMM4 = _mm_or_ps(XMM4, XMM3);
				_mm_storeu_ps(tempmdct+i+j+k  , XMM1);
				_mm_storeu_ps(tempmdct+i+j+k+4, XMM4);
			  }
			  for(; j<temp_bfn4[i+k]; j+=4, ptempbuf+=4)
			  {
				__m128 XMM0, XMM1, XMM2;
				XMM0 = _mm_load_ps(ptempbuf);
				XMM1 = _mm_lddqu_ps(tempmdct+i+j+k  );
				XMM0 = _mm_add_ps(XMM0, PLOGMDCT);
				XMM2 = _mm_lddqu_ps(tlogmdct+i+j+k  );
				XMM0 = _mm_min_ps(XMM0, XMM2);
				XMM0 = _mm_cmple_ps(XMM0, XMM1);
				XMM1 = _mm_and_ps(XMM1, XMM0);
				XMM0 = _mm_andnot_ps(XMM0, XMM2);
				XMM1 = _mm_or_ps(XMM1, XMM0);
				_mm_storeu_ps(tempmdct+i+j+k, XMM1);
			  }
			}
			switch(temp_bfn[i+k]-j)
			{
			case 0 :
			  {
				__m128 XMM0, XMM1, XMM2;
				XMM0 = _mm_load_ss(ptempbuf);
				XMM1 = _mm_load_ss(tempmdct+i+j+k);
				XMM0 = _mm_add_ss(XMM0, PLOGMDCT);
				XMM2 = _mm_load_ss(tlogmdct+i+j+k);
				XMM0 = _mm_min_ss(XMM0, XMM2);
				XMM0 = _mm_cmple_ss(XMM0, XMM1);
				XMM1 = _mm_and_ps(XMM1, XMM0);
				XMM0 = _mm_andnot_ps(XMM0, XMM2);
				XMM1 = _mm_or_ps(XMM1, XMM0);
				_mm_store_ss(tempmdct+i+j+k, XMM1);
			  }
			  break;
			case 1 :
			  {
				__m128 XMM0, XMM1, XMM2;
#pragma warning(disable : 592)
				XMM0 = _mm_loadl_pi(XMM0, (__m64*)ptempbuf);
				XMM1 = _mm_loadl_pi(XMM1, (__m64*)(tempmdct+i+j+k));
				XMM0 = _mm_add_ps(XMM0, PLOGMDCT);
				XMM2 = _mm_loadl_pi(XMM2, (__m64*)(tlogmdct+i+j+k));
#pragma warning(default : 592)
				XMM0 = _mm_min_ps(XMM0, XMM2);
				XMM0 = _mm_cmple_ps(XMM0, XMM1);
				XMM1 = _mm_and_ps(XMM1, XMM0);
				XMM0 = _mm_andnot_ps(XMM0, XMM2);
				XMM1 = _mm_or_ps(XMM1, XMM0);
				_mm_storel_pi((__m64*)(tempmdct+i+j+k), XMM1);
			  }
			  break;
			case 2 :
			  {
				__m128 XMM0, XMM1, XMM2;
				XMM0 = _mm_load_ps(ptempbuf);
				XMM1 = _mm_lddqu_ps(tempmdct+i+j+k);
				XMM0 = _mm_add_ps(XMM0, PLOGMDCT);
				XMM2 = _mm_lddqu_ps(tlogmdct+i+j+k);
				XMM0 = _mm_min_ps(XMM0, XMM2);
				XMM0 = _mm_cmple_ps(XMM0, XMM1);
				XMM1 = _mm_and_ps(XMM1, XMM0);
				XMM0 = _mm_andnot_ps(XMM0, XMM2);
				XMM1 = _mm_or_ps(XMM1, XMM0);
				_mm_storel_pi((__m64*)(tempmdct+i+j+k), XMM1);
				XMM1 = _mm_movehl_ps(XMM1, XMM1);
				_mm_store_ss(tempmdct+i+j+k+2, XMM1);
			  }
			  break;
			case 3 :
			  break;
			}
		  }
		  {
			__m128 XMM0 = _mm_cmpgt_ps(PVAL, _mm_max_ps(PTVAL, PM128(lastmdct+i)));
			if(_mm_movemask_ps(XMM0))
			{
			  __m128 XMM1 = _mm_cmpgt_ps(PM128(logmdct+i), _mm_add_ps(PM128(tempmdct+i), PNOISE_CENTER));
			  __m128 XMM2, XMM3, XMM4;
			  XMM0 = _mm_and_ps(XMM0, XMM1);
			  if(_mm_movemask_ps(XMM0))
			  {
				XMM1 = _mm_load_ps(logmdct+i);
				XMM2 = XMM0;
				XMM3 = XMM1;
				XMM3 = _mm_or_ps(
				  _mm_and_ps(XMM3, XMM2),
				  _mm_andnot_ps(XMM2, PM128(tempmdct+i))
				);
				_mm_store_ps(tempmdct+i, XMM3);
				XMM1 = _mm_cmpgt_ps(XMM1, PM128(lastmdct+i));
				XMM2 = _mm_or_ps(
				  _mm_and_ps(PNOISE_RATE, XMM1),
				  _mm_andnot_ps(XMM1, PNOISE_RATE_LOW)
				);	/* rate_mod */
				XMM1 = _mm_sub_ps(PVAL, PTVAL);
				XMM3 = XMM1;
				XMM1 = _mm_sub_ps(XMM1, PM128(P30));
				XMM4 = _mm_cmpgt_ps(XMM1, PM128(PFV_0));
				XMM1 = _mm_mul_ps(XMM1, PM128(PP1));
				XMM1 = _mm_add_ps(XMM1, PM128(P30));
				XMM1 = _mm_and_ps(XMM1, XMM4);
				XMM4 = _mm_andnot_ps(XMM4, XMM3);
				XMM1 = _mm_or_ps(XMM1, XMM4);
				XMM1 = _mm_mul_ps(XMM1, XMM2);
				XMM3 = PVAL;
				XMM3 = _mm_sub_ps(XMM3, XMM1);
				XMM3 = _mm_max_ps(XMM3, PM128(lastmdct+i));
				XMM3 = _mm_and_ps(XMM3, XMM0);
				XMM0 = _mm_andnot_ps(XMM0, PVAL);
				PVAL = _mm_or_ps(XMM3, XMM0);
			  }
			}
		  }
		}
		switch(RunMode[i])
		{
			default:
			case 0: /* Default */
			  {
				int k;
				__m128x T0, T1;
				T0.ps = PVAL;
				T1.ps = PTVAL;
				for(k=0;k<4;k++){
				  float val  = T0.sf[k];
				  float tval = T1.sf[k];
				  if(i+k>=lp_pos)logmdct[i+k]=-160;
				  if(val>tval){
					logmask[i+k]=val;
				  }else if((i+k>m4_start) && (i+k<m4_end) && (logmdct[i+k]>-140)){
					if(logmdct[i+k]>val){
					  if(logmdct[i+k]<tval)tval-=(tval-val)*m4_thres;
					}else{
					  if(val<tval)tval-=(tval-val)*m4_thres;
					}
					logmask[i+k]=tval;
				  }else logmask[i+k]=tval;
				  T1.sf[k] = tval;
				}
				PTVAL = T1.ps;
			  }
			  break;
			case 1: /* SSE-1 */
			  _mm_store_ps(logmask+i, _mm_max_ps(PVAL, PTVAL));
			  break;
			case 2: /* SSE-2(m4_start - m4_end) */
			  {
				/*
				  A: val>tval
				  B: logmdct>-140
				  C: logmdct>val
				  D: logmdct<tval
				  E: val<tval
				  T0 = A for val
				  T1 = a(b|B(Cd|ce)) for logmdct
				  T2 = T0|T1 for tval
				  T3 = t2 for tval*
				  logmask = val&T0 | tval&T1 | tval*&T3
				  tval = tval&T2 | tval*&T3
				*/
				__m128 XMM0, XMM1, XMM2, XMM3, XMM4;
				XMM4 = _mm_cmpgt_ps(PVAL, PTVAL);				/* T0:A */
				if(_mm_movemask_ps(XMM4)==15)
				  _mm_store_ps(logmask+i, PVAL);
				else
				{
				  XMM2 = _mm_cmple_ps(PM128(logmdct+i), PVAL);	/* c */
				  XMM0 = _mm_cmple_ps(PTVAL, PM128(logmdct+i));	/* d */
				  XMM1 = _mm_cmple_ps(PTVAL, PVAL);				/* e */
				  XMM1 = _mm_and_ps(XMM1, XMM2);				/* ce */
				  XMM2 = _mm_andnot_ps(XMM2, XMM0);				/* Cd */
				  XMM1 = _mm_or_ps(XMM1, XMM2);					/* Cd|ce */
				  XMM3 = _mm_cmple_ps(PM128(logmdct+i),PM128(PM140));	/* b */
				  XMM2 = XMM3;
				  XMM2 = _mm_andnot_ps(XMM2, XMM1);				/* B(Cd|ce) */
				  XMM3 = _mm_or_ps(XMM3, XMM2);					/* b|B(Cd|ce) */
				  XMM1 = XMM4;
				  XMM1 = _mm_andnot_ps(XMM1, XMM3);				/* T1:a(b|B(Cd|ce)) */
				  XMM2 = _mm_or_ps(XMM4, XMM1);					/* T2:T0|T1 */
				  XMM4 = _mm_and_ps(XMM4, PVAL);				/* val&T0 */
				  XMM1 = _mm_and_ps(XMM1, PTVAL);				/* tval&T1 */
				  XMM3 = _mm_sub_ps(PVAL, PTVAL);
				  XMM3 = _mm_mul_ps(XMM3, PM4_THRES);
				  XMM3 = _mm_add_ps(XMM3, PTVAL);				/* tval* */
				  PTVAL = _mm_and_ps(PTVAL, XMM2);				/* tval&T2 */
				  XMM2 = _mm_andnot_ps(XMM2, XMM3);				/* tval*&T3 */
				  PTVAL = _mm_or_ps(PTVAL, XMM2);				/* tval = tval&T2 | tval*&T3 */
				  XMM1 = _mm_or_ps(XMM1, XMM2);					/* tval&T1 | tval*&T3 */
				  XMM4 = _mm_or_ps(XMM4, XMM1);					/* val&T0 | tval&T1 | tval*&T3 */
				  _mm_store_ps(logmask+i, XMM4);
				}
			  }
			  break;
			case 3: /* SSE-3(block include lp_pos) */
			  {
				int k;
				for(k=0;k<4;k++)
				  if(i+k>=lp_pos)logmdct[i+k]=-160;
			  }
			  _mm_store_ps(logmask+i, _mm_max_ps(PVAL, PTVAL));
			  break;
			case 4: /* SSE-4(i>=lp_pos) */
			  _mm_store_ps(logmdct+i, PM128(PM160));
			  _mm_store_ps(logmask+i, _mm_max_ps(PVAL, PTVAL));
			  break;
		}
		{
		  __m128 XMM0, XMM1, XMM2;
		  PVAL = _mm_sub_ps(PVAL, PM128(logmdct+i));
		  XMM0 = PVAL;
		  XMM1 = PVAL;
		  XMM2 = PVAL;
		  XMM0 = _mm_cmpgt_ps(XMM0, PM128(PCOEFFI));
		  XMM1 = _mm_mul_ps(XMM1, PCX0);
		  XMM2 = _mm_mul_ps(XMM2, PCX1);
		  XMM1 = _mm_add_ps(XMM1, PFV_C0);
		  XMM2 = _mm_add_ps(XMM2, PFV_C1);
		  XMM1 = _mm_max_ps(XMM1, PM128(PP0001));
		  XMM1 = _mm_and_ps(XMM1, XMM0);
		  XMM0 = _mm_andnot_ps(XMM0, XMM2);
		  XMM1 = _mm_or_ps(XMM1, XMM0);
		  XMM1 = _mm_mul_ps(XMM1, PM128(mdct+i));
		  _mm_store_ps(mdct+i, XMM1);
		}
	  }
	}
	else
	  goto SAFE_MODE;
  }
  else
  {
SAFE_MODE:
#endif														/* SSE Optimize */
  for(i=0;i<n;i++){
    float val= noise[i]+p->noiseoffset[offset_select][i];
    float tval= tone[i]+toneatt;
    if(i<=m4_start)tval-=low_compand;
    if(val>p->vi->noisemaxsupp)val=p->vi->noisemaxsupp;
    
    /* AoTuV */
    /** @ M2 MAIN **
    floor is pulled below suitably. (padding block only) (#2)
    by Aoyumi @ 2006/06/14
    */
    if(m2_sw){
    	// the conspicuous low level pre-echo of the padding block origin is reduced. 
    	if((logmdct[i]-lastmdct[i]) > 20){
    		if(i > m3n[3]) val -= (logmdct[i]-lastmdct[i]-20)/padth;
    		else val -= (logmdct[i]-lastmdct[i]-20)/(padth+padth);
    	}
    }
    
    /* AoTuV */
    /** @ M3 MAIN **
    Dynamic impulse block noise control. (#4)
    48/44.1/32kHz only.
    by Aoyumi @ 2006/02/02
    */
    if(it_sw){
    	for(j=1; j<=temp_bfn[i]; j++){
    		float tempbuf = logmdct[i]-(75/temp_bfn[i]*j)-5;
			if( (tempmdct[i+j] < tempbuf) && (tempmdct[i+j] < (logmdct[i+j]-5)) )
			 tempmdct[i+j] = logmdct[i+j] - 5;
		}
    	if(val > tval){
    		if( (val>lastmdct[i]) && (logmdct[i]>(tempmdct[i]+noise_center)) ){
    			float valmask=0;
    			tempmdct[i] = logmdct[i];
    			
    			if(logmdct[i]>lastmdct[i]){
    				rate_mod = noise_rate;
    			}else{
    				rate_mod = noise_rate_low;
				}
				if(i > m3n[1]){
						if((val-tval)>30) valmask=((val-tval-30)/10+30)*rate_mod;
						else valmask=(val-tval)*rate_mod;
				}else if(i > m3n[2]){
						if((val-tval)>20) valmask=((val-tval-20)/10+20)*rate_mod;
						else valmask=(val-tval)*rate_mod;
				}else if(i > m3n[3]){
						if((val-tval)>10) valmask=((val-tval-10)/10+10)*rate_mod*0.5;
						else valmask=(val-tval)*rate_mod*0.5;
				}else{
					if((val-tval)>10) valmask=((val-tval-10)/10+10)*rate_mod*0.3;
					else valmask=(val-tval)*rate_mod*0.3;
				}
				if((val-valmask)>lastmdct[i])val-=valmask;
				else val=lastmdct[i];
			}
   		}
   	}
   	
   	/* This affects calculation of a floor curve. */
   	if(i>=lp_pos)logmdct[i]=-160;
   	
     /* AoTuV */
	/** @ M4 MAIN **
	The purpose of this portion is working Noise Normalization more correctly. 
	(There is this in order to prevent extreme boost of floor)
	  m4_start = start point
	  m4_end   = end point
	  m4_thres = threshold
	by Aoyumi @ 2006/03/20
	*/
    //logmask[i]=max(val,tval);
    if(val>tval){
		logmask[i]=val;
	}else if((i>m4_start) && (i<m4_end) && (logmdct[i]>-140)){
		if(logmdct[i]>val){
			if(logmdct[i]<tval)tval-=(tval-val)*m4_thres;
		}else{
			if(val<tval)tval-=(tval-val)*m4_thres;
		}
		logmask[i]=tval;
	}else logmask[i]=tval;

    /* AoTuV */
    /** @ M1 **
	The following codes improve a noise problem.  
	A fundamental idea uses the value of masking and carries out
	the relative compensation of the MDCT. 
	However, this code is not perfect and all noise problems cannot be solved. 
	by Aoyumi @ 2004/04/18
    */

    if(offset_select == 1) {
      coeffi = -17.2;       /* coeffi is a -17.2dB threshold */
      val = val - logmdct[i];  /* val == mdct line value relative to floor in dB */
      
      if(val > coeffi){
	/* mdct value is > -17.2 dB below floor */
	
	de = 1.0-((val-coeffi)*0.005*cx);
	/* pro-rated attenuation:
	   -0.00 dB boost if mdct value is -17.2dB (relative to floor) 
	   -0.77 dB boost if mdct value is 0dB (relative to floor) 
	   -1.64 dB boost if mdct value is +17.2dB (relative to floor) 
	   etc... */
	
	if(de < 0) de = 0.0001;
      }else
	/* mdct value is <= -17.2 dB below floor */
	
	de = 1.0-((val-coeffi)*0.0003*cx);
      /* pro-rated attenuation:
	 +0.00 dB atten if mdct value is -17.2dB (relative to floor) 
	 +0.45 dB atten if mdct value is -34.4dB (relative to floor) 
	 etc... */
      
      mdct[i] *= de;
      
    }
  }
#ifdef __SSE__												/* SSE Optimize */
  }
#endif														/* SSE Optimize */

  /** @ M3 SET lastmdct **/
  if(offset_select == 1){
#ifdef __SSE__												/* SSE Optimize */
	if(n == 1024)
	{
	  if(!nW_modenumber)
	  {
		for(i=0; i<128; i+=16)
		{
		  __m128	XMM0, XMM1, XMM2, XMM3;
		  __m128	XMM4, XMM5, XMM6, XMM7;
		  XMM0	 = _mm_load_ps(logmdct+i*8    );
		  XMM1	 = _mm_load_ps(logmdct+i*8+  4);
		  XMM2	 = _mm_load_ps(logmdct+i*8+  8);
		  XMM3	 = _mm_load_ps(logmdct+i*8+ 12);
		  XMM4	 = _mm_load_ps(logmdct+i*8+ 16);
		  XMM5	 = _mm_load_ps(logmdct+i*8+ 20);
		  XMM6	 = _mm_load_ps(logmdct+i*8+ 24);
		  XMM7	 = _mm_load_ps(logmdct+i*8+ 28);
		  XMM0	 = _mm_min_ps(XMM0, XMM1);
		  XMM2	 = _mm_min_ps(XMM2, XMM3);
		  XMM4	 = _mm_min_ps(XMM4, XMM5);
		  XMM6	 = _mm_min_ps(XMM6, XMM7);
		  XMM1	 = XMM0;
		  XMM5	 = XMM4;
		  XMM0	 = _mm_shuffle_ps(XMM0, XMM2, _MM_SHUFFLE(2,0,2,0));
		  XMM4	 = _mm_shuffle_ps(XMM4, XMM6, _MM_SHUFFLE(2,0,2,0));
		  XMM3	 = _mm_load_ps(logmdct+i*8+ 32);
		  XMM7	 = _mm_load_ps(logmdct+i*8+ 36);
		  XMM1	 = _mm_shuffle_ps(XMM1, XMM2, _MM_SHUFFLE(3,1,3,1));
		  XMM5	 = _mm_shuffle_ps(XMM5, XMM6, _MM_SHUFFLE(3,1,3,1));
		  XMM2	 = XMM0;
		  XMM6	 = XMM1;
		  XMM0	 = _mm_shuffle_ps(XMM0, XMM4, _MM_SHUFFLE(2,0,2,0));
		  XMM1	 = _mm_shuffle_ps(XMM1, XMM5, _MM_SHUFFLE(2,0,2,0));
		  XMM2	 = _mm_shuffle_ps(XMM2, XMM4, _MM_SHUFFLE(3,1,3,1));
		  XMM6	 = _mm_shuffle_ps(XMM6, XMM5, _MM_SHUFFLE(3,1,3,1));
		  XMM4	 = _mm_load_ps(logmdct+i*8+ 40);
		  XMM5	 = _mm_load_ps(logmdct+i*8+ 44);
		  XMM1	 = _mm_min_ps(XMM1, XMM0);
		  XMM0	 = _mm_load_ps(logmdct+i*8+ 48);
		  XMM6	 = _mm_min_ps(XMM6, XMM2);
		  XMM2	 = _mm_load_ps(logmdct+i*8+ 52);
		  XMM6	 = _mm_min_ps(XMM6, XMM1);
		  XMM1	 = _mm_load_ps(logmdct+i*8+ 56);
		  _mm_store_ps(lastmdct+i   , XMM6);
		  XMM6	 = _mm_load_ps(logmdct+i*8+ 60);
		  XMM3	 = _mm_min_ps(XMM3, XMM7);
		  XMM4	 = _mm_min_ps(XMM4, XMM5);
		  XMM0	 = _mm_min_ps(XMM0, XMM2);
		  XMM1	 = _mm_min_ps(XMM1, XMM6);
		  XMM7	 = XMM3;
		  XMM2	 = XMM0;
		  XMM3	 = _mm_shuffle_ps(XMM3, XMM4, _MM_SHUFFLE(2,0,2,0));
		  XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(2,0,2,0));
		  XMM5	 = _mm_load_ps(logmdct+i*8+ 64);
		  XMM6	 = _mm_load_ps(logmdct+i*8+ 68);
		  XMM7	 = _mm_shuffle_ps(XMM7, XMM4, _MM_SHUFFLE(3,1,3,1));
		  XMM2	 = _mm_shuffle_ps(XMM2, XMM1, _MM_SHUFFLE(3,1,3,1));
		  XMM4	 = XMM3;
		  XMM1	 = XMM7;
		  XMM3	 = _mm_shuffle_ps(XMM3, XMM0, _MM_SHUFFLE(2,0,2,0));
		  XMM7	 = _mm_shuffle_ps(XMM7, XMM2, _MM_SHUFFLE(2,0,2,0));
		  XMM4	 = _mm_shuffle_ps(XMM4, XMM0, _MM_SHUFFLE(3,1,3,1));
		  XMM1	 = _mm_shuffle_ps(XMM1, XMM2, _MM_SHUFFLE(3,1,3,1));
		  XMM0	 = _mm_load_ps(logmdct+i*8+ 72);
		  XMM2	 = _mm_load_ps(logmdct+i*8+ 76);
		  XMM7	 = _mm_min_ps(XMM7, XMM3);
		  XMM3	 = _mm_load_ps(logmdct+i*8+ 80);
		  XMM1	 = _mm_min_ps(XMM1, XMM4);
		  XMM4	 = _mm_load_ps(logmdct+i*8+ 84);
		  XMM1	 = _mm_min_ps(XMM1, XMM7);
		  XMM7	 = _mm_load_ps(logmdct+i*8+ 88);
		  _mm_store_ps(lastmdct+i+ 4, XMM1);
		  XMM1	 = _mm_load_ps(logmdct+i*8+ 92);
		  XMM5	 = _mm_min_ps(XMM5, XMM6);
		  XMM0	 = _mm_min_ps(XMM0, XMM2);
		  XMM3	 = _mm_min_ps(XMM3, XMM4);
		  XMM7	 = _mm_min_ps(XMM7, XMM1);
		  XMM6	 = XMM5;
		  XMM4	 = XMM3;
		  XMM5	 = _mm_shuffle_ps(XMM5, XMM0, _MM_SHUFFLE(2,0,2,0));
		  XMM3	 = _mm_shuffle_ps(XMM3, XMM7, _MM_SHUFFLE(2,0,2,0));
		  XMM2	 = _mm_load_ps(logmdct+i*8+ 96);
		  XMM1	 = _mm_load_ps(logmdct+i*8+100);
		  XMM6	 = _mm_shuffle_ps(XMM6, XMM0, _MM_SHUFFLE(3,1,3,1));
		  XMM4	 = _mm_shuffle_ps(XMM4, XMM7, _MM_SHUFFLE(3,1,3,1));
		  XMM0	 = XMM5;
		  XMM7	 = XMM6;
		  XMM5	 = _mm_shuffle_ps(XMM5, XMM3, _MM_SHUFFLE(2,0,2,0));
		  XMM6	 = _mm_shuffle_ps(XMM6, XMM4, _MM_SHUFFLE(2,0,2,0));
		  XMM0	 = _mm_shuffle_ps(XMM0, XMM3, _MM_SHUFFLE(3,1,3,1));
		  XMM7	 = _mm_shuffle_ps(XMM7, XMM4, _MM_SHUFFLE(3,1,3,1));
		  XMM3	 = _mm_load_ps(logmdct+i*8+104);
		  XMM4	 = _mm_load_ps(logmdct+i*8+108);
		  XMM6	 = _mm_min_ps(XMM6, XMM5);
		  XMM5	 = _mm_load_ps(logmdct+i*8+112);
		  XMM7	 = _mm_min_ps(XMM7, XMM0);
		  XMM0	 = _mm_load_ps(logmdct+i*8+116);
		  XMM7	 = _mm_min_ps(XMM7, XMM6);
		  XMM6	 = _mm_load_ps(logmdct+i*8+120);
		  _mm_store_ps(lastmdct+i+ 8, XMM7);
		  XMM7	 = _mm_load_ps(logmdct+i*8+124);
		  XMM2	 = _mm_min_ps(XMM2, XMM1);
		  XMM3	 = _mm_min_ps(XMM3, XMM4);
		  XMM5	 = _mm_min_ps(XMM5, XMM0);
		  XMM6	 = _mm_min_ps(XMM6, XMM7);
		  XMM1	 = XMM2;
		  XMM0	 = XMM5;
		  XMM2	 = _mm_shuffle_ps(XMM2, XMM3, _MM_SHUFFLE(2,0,2,0));
		  XMM5	 = _mm_shuffle_ps(XMM5, XMM6, _MM_SHUFFLE(2,0,2,0));
		  XMM1	 = _mm_shuffle_ps(XMM1, XMM3, _MM_SHUFFLE(3,1,3,1));
		  XMM0	 = _mm_shuffle_ps(XMM0, XMM6, _MM_SHUFFLE(3,1,3,1));
		  XMM3	 = XMM2;
		  XMM6	 = XMM1;
		  XMM2	 = _mm_shuffle_ps(XMM2, XMM5, _MM_SHUFFLE(2,0,2,0));
		  XMM1	 = _mm_shuffle_ps(XMM1, XMM0, _MM_SHUFFLE(2,0,2,0));
		  XMM3	 = _mm_shuffle_ps(XMM3, XMM5, _MM_SHUFFLE(3,1,3,1));
		  XMM6	 = _mm_shuffle_ps(XMM6, XMM0, _MM_SHUFFLE(3,1,3,1));
		  XMM1	 = _mm_min_ps(XMM1, XMM2);
		  XMM6	 = _mm_min_ps(XMM6, XMM3);
		  XMM6	 = _mm_min_ps(XMM6, XMM1);
		  _mm_store_ps(lastmdct+i+12, XMM6);
		}
	  }
	}
	else
	  if(n == 128)
	  {
		for(i=0;i<128;i+=32)
		{
		  __m128	XMM0	 = _mm_load_ps(logmdct+i   );
		  __m128	XMM1	 = _mm_load_ps(logmdct+i+ 4);
		  __m128	XMM2	 = _mm_load_ps(logmdct+i+ 8);
		  __m128	XMM3	 = _mm_load_ps(logmdct+i+12);
		  __m128	XMM4	 = _mm_load_ps(logmdct+i+16);
		  __m128	XMM5	 = _mm_load_ps(logmdct+i+20);
		  __m128	XMM6	 = _mm_load_ps(logmdct+i+24);
		  __m128	XMM7	 = _mm_load_ps(logmdct+i+28);
		  _mm_store_ps(lastmdct+i   , XMM0);
		  _mm_store_ps(lastmdct+i+ 4, XMM1);
		  _mm_store_ps(lastmdct+i+ 8, XMM2);
		  _mm_store_ps(lastmdct+i+12, XMM3);
		  _mm_store_ps(lastmdct+i+16, XMM4);
		  _mm_store_ps(lastmdct+i+20, XMM5);
		  _mm_store_ps(lastmdct+i+24, XMM6);
		  _mm_store_ps(lastmdct+i+28, XMM7);
		}
	  }
#else														/* SSE Optimize */
	if(n == 1024){
		if(!nW_modenumber){
			for(i=0; i<128; i++){
				lastmdct[i] = logmdct[i*8];
				for(j=1; j<8; j++){
					if(lastmdct[i] > logmdct[i*8+j]){
						lastmdct[i] = logmdct[i*8+j];
					}
				}
			}
		}
	}else if(n == 128){
		for(i=0; i<128; i++) lastmdct[i] = logmdct[i];
	}
#endif														/* SSE Optimize */
  }
}

float _vp_ampmax_decay(float amp,vorbis_dsp_state *vd){
  vorbis_info *vi=vd->vi;
  codec_setup_info *ci=vi->codec_setup;
  vorbis_info_psy_global *gi=&ci->psy_g_param;

  int n=ci->blocksizes[vd->W]/2;
  float secs=(float)n/vi->rate;

  amp+=secs*gi->ampmax_att_per_sec;
  if(amp<-9999)amp=-9999;
  return(amp);
}

#ifdef	__SSE__												/* SSE Optimize */
STIN void couple_lossless(float A, float B, 
#else														/* SSE Optimize */
static void couple_lossless(float A, float B, 
#endif														/* SSE Optimize */
			    float *qA, float *qB){
  int test1=fabs(*qA)>fabs(*qB);
  test1-= fabs(*qA)<fabs(*qB);
  
  if(!test1)test1=((fabs(A)>fabs(B))<<1)-1;
  if(test1==1){
    *qB=(*qA>0.f?*qA-*qB:*qB-*qA);
  }else{
    float temp=*qB;  
    *qB=(*qB>0.f?*qA-*qB:*qB-*qA);
    *qA=temp;
  }

  if(*qB>fabs(*qA)*1.9999f){
    *qB= -fabs(*qA)*2.f;
    *qA= -*qA;
  }
}

#ifdef	__SSE__												/* SSE Optimize */
	/*
		Phase 1.
			fabs(*qA)>fabs(*qB)	test1 =  1
			fabs(*qA)>fabs(*qB)	test1 = -1
			fabs(*qA)=fabs(*qB)	fabs(qA)> fabs(B)	test1 = -1
			fabs(*qA)=fabs(*qB)	fabs(qA)<=fabs(B)	test1 =  1
		
		Phase 2.
			*qB	 = S(*qA)^(*qA-*qB)	(test1==1)
			*qB	 = S(*qB)^(*qA-*qB)	(test1!=1)
			*qA= Old *qA			(test1==1)
			*qA= Old *qB			(test1!=1)
		
		Phase 3.
			*qB	 = -fabs(*qA)*2.f	(*qB >fabs(*qA)*1.9999f)
			*qB	 = *qB				(*qB<=fabs(*qA)*1.9999f)
			*qA	 = -*qA				(*qB >fabs(*qA)*1.9999f)
			*qA	 =  *qA				(*qB<=fabs(*qA)*1.9999f)
	*/
STIN void couple_lossless_ps(float *A, float *B, float *qA, float *qB)
{
	/*
		Phase 1
	*/
	__m128	PQA	 = _mm_load_ps(qA);
	__m128	PQB	 = _mm_load_ps(qB);
	__m128	FQA	 = _mm_and_ps(PQA, PM128(PABSMASK));
	__m128	FQB	 = _mm_and_ps(PQB, PM128(PABSMASK));
	__m128	XMM0	 = _mm_and_ps(PM128(A), PM128(PABSMASK));
	__m128	XMM1	 = _mm_and_ps(PM128(B), PM128(PABSMASK));
	__m128	PTEST1;
	__m128	PTEST2;
	__m128	PFQA2M;
	
	XMM0	 = _mm_cmpgt_ps(XMM0, XMM1);
	XMM1	 = _mm_cmpneq_ps(FQA, FQB);
	PTEST1	 = _mm_or_ps(
					_mm_and_ps(_mm_cmpgt_ps(FQA, FQB), XMM1),
					_mm_andnot_ps(XMM1, XMM0)
				);
	PTEST2	 = PTEST1;
	
	/*
		Phase 2
	*/
	XMM0	 = _mm_and_ps(PQA, PM128(PCS_RRRR));	/* Sign of PQA */
	XMM1	 = _mm_and_ps(PQB, PM128(PCS_RRRR));	/* Sign of PQB */
	XMM0	 = _mm_and_ps(XMM0, PTEST2);
	XMM1	 = _mm_andnot_ps(PTEST2, XMM1);
	XMM0	 = _mm_or_ps(XMM0, XMM1);				/* Sign of new *qB */
	XMM1	 = _mm_sub_ps(PQA, PQB);				/* New *qB Body */
	XMM1	 = _mm_xor_ps(XMM1, XMM0);				/* New qB */
	PQA		 = _mm_and_ps(PQA, PTEST1);
	PQB		 = _mm_andnot_ps(PTEST1, PQB);
	XMM0	 = _mm_or_ps(PQA, PQB);					/* New qA */
	
	/*
		Phase 3
	*/
	PFQA2M	 = _mm_mul_ps(FQA, PM128(PFV_2));
	
	PTEST1	 = _mm_cmpge_ps(XMM1, PFQA2M);		/* Mask of *qB >= fabs(*qA)*2.f */
	PTEST2	 = PTEST1;
	PQB		 = _mm_xor_ps(PFQA2M, PM128(PCS_RRRR));	/* -fabs(qA)*2.f */
	PQA		 = _mm_xor_ps(XMM0   , PM128(PCS_RRRR));	/* -qA */
	PQB		 = _mm_or_ps(_mm_and_ps(PQB, PTEST1), _mm_andnot_ps(PTEST1, XMM1));
	PQA		 = _mm_or_ps(_mm_and_ps(PQA, PTEST2), _mm_andnot_ps(PTEST2, XMM0));
	_mm_store_ps(qB, PQB);
	_mm_store_ps(qA, PQA);
}
#endif														/* SSE Optimize */

static const float hypot_lookup[32]={
  -0.009935, -0.011245, -0.012726, -0.014397, 
  -0.016282, -0.018407, -0.020800, -0.023494, 
  -0.026522, -0.029923, -0.033737, -0.038010, 
  -0.042787, -0.048121, -0.054064, -0.060671, 
  -0.068000, -0.076109, -0.085054, -0.094892, 
  -0.105675, -0.117451, -0.130260, -0.144134, 
  -0.159093, -0.175146, -0.192286, -0.210490, 
  -0.229718, -0.249913, -0.271001, -0.292893};

#ifdef	__SSE__												/* SSE Optimize */
STIN void precomputed_couple_point(float premag,
#else														/* SSE Optimize */
static void precomputed_couple_point(float premag,
#endif														/* SSE Optimize */
				     int floorA,int floorB,
				     float *mag, float *ang){
  
  int test=(floorA>floorB)-1;
  int offset=31-abs(floorA-floorB);
  float floormag=hypot_lookup[((offset<0)-1)&offset]+1.f; // floormag = 0.990065 ~ 0.707107

  floormag*=FLOOR1_fromdB_INV_LOOKUP[(floorB&test)|(floorA&(~test))];

  *mag=premag*floormag;
  *ang=0.f;
}

#ifdef	__SSE__												/* SSE Optimize */
STIN void precomputed_couple_point_ps(float *premag,
				     int *floorA,int *floorB,
				     float *mag, float *ang){
	__m128	XMM0;
	__m128x	PI0, PI1;
#ifdef	__SSE2__
	{
		static _MM_ALIGN16 const unsigned int PFI0[4]	 = { 0,  0,  0,  0};
		static _MM_ALIGN16 const unsigned int PFI31[4]	 = {31, 31, 31, 31};
		__m128i	PFA	 = PM128I(floorA);
		__m128i	PFB	 = PM128I(floorB);
		__m128i	XMM0	 = PFA;
		__m128i	XMM1	 = PFA;
		__m128i	XMM2	 = PM128I(PFI31);
		__m128i	XMM3	 = _mm_setzero_si128();
		XMM0	 = _mm_cmpgt_epi32(XMM0, PFB);
		PFA		 = _mm_and_si128(PFA, XMM0);
		XMM0	 = _mm_andnot_si128(XMM0, PFB);
		PFA		 = _mm_or_si128(PFA, XMM0);
		PI1.pi	 = PFA;

		XMM1	 = _mm_sub_epi32(XMM1, PFB);
		XMM3	 = _mm_cmpgt_epi32(XMM3, XMM1);
		XMM1	 = _mm_xor_si128(XMM1, XMM3);
		XMM1	 = _mm_sub_epi32(XMM1, XMM3);
		XMM2	 = _mm_sub_epi32(XMM2, XMM1);
		XMM3	 = XMM2;
		XMM3	 = _mm_cmpgt_epi32(XMM3, PM128I(PFI0));
		XMM2	 = _mm_and_si128(XMM2, XMM3);
		PI0.pi	 = XMM2;
	}
	PI0.sf[0]	 = hypot_lookup[PI0.si32[0]];
	PI0.sf[1]	 = hypot_lookup[PI0.si32[1]];
	PI0.sf[2]	 = hypot_lookup[PI0.si32[2]];
	PI0.sf[3]	 = hypot_lookup[PI0.si32[3]];
	PI1.sf[0]	 = FLOOR1_fromdB_INV_LOOKUP[PI1.si32[0]];
	PI1.sf[1]	 = FLOOR1_fromdB_INV_LOOKUP[PI1.si32[1]];
	PI1.sf[2]	 = FLOOR1_fromdB_INV_LOOKUP[PI1.si32[2]];
	PI1.sf[3]	 = FLOOR1_fromdB_INV_LOOKUP[PI1.si32[3]];
#else
	int test0	 = (*(floorA  )>*(floorB  ))-1;
	int test1	 = (*(floorA+1)>*(floorB+1))-1;
	int test2	 = (*(floorA+2)>*(floorB+2))-1;
	int test3	 = (*(floorA+3)>*(floorB+3))-1;
	int offset0	 = 31-abs(*(floorA  )-*(floorB  ));
	int offset1	 = 31-abs(*(floorA+1)-*(floorB+1));
	int offset2	 = 31-abs(*(floorA+2)-*(floorB+2));
	int offset3	 = 31-abs(*(floorA+3)-*(floorB+3));
	PI0.sf[0]	 = hypot_lookup[((offset0<0)-1)&offset0];
	PI0.sf[1]	 = hypot_lookup[((offset1<0)-1)&offset1];
	PI0.sf[2]	 = hypot_lookup[((offset2<0)-1)&offset2];
	PI0.sf[3]	 = hypot_lookup[((offset3<0)-1)&offset3];

	PI1.sf[0]	 = FLOOR1_fromdB_INV_LOOKUP[(*(floorB  )&test0)|(*(floorA  )&(~test0))];
	PI1.sf[1]	 = FLOOR1_fromdB_INV_LOOKUP[(*(floorB+1)&test1)|(*(floorA+1)&(~test1))];
	PI1.sf[2]	 = FLOOR1_fromdB_INV_LOOKUP[(*(floorB+2)&test2)|(*(floorA+2)&(~test2))];
	PI1.sf[3]	 = FLOOR1_fromdB_INV_LOOKUP[(*(floorB+3)&test3)|(*(floorA+3)&(~test3))];
#endif

	XMM0	 = _mm_add_ps(PI0.ps, PM128(PFV_1));
	XMM0	 = _mm_mul_ps(XMM0, PI1.ps);
	XMM0	 = _mm_mul_ps(XMM0, PM128(premag));
	_mm_store_ps(mag, XMM0);
	_mm_store_ps(ang, _mm_setzero_ps());
}
#endif														/* SSE Optimize */

/* just like below, this is currently set up to only do
   single-step-depth coupling.  Otherwise, we'd have to do more
   copying (which will be inevitable later) */

/* doing the real circular magnitude calculation is audibly superior
   to (A+B)/sqrt(2) */
static float dipole_hypot(float a, float b){
  if(a>0.){
    if(b>0.)return sqrt(a*a+b*b);
    if(a>-b)return sqrt(a*a-b*b);
    return -sqrt(b*b-a*a);
  }
  if(b<0.)return -sqrt(a*a+b*b);
  if(-a>b)return -sqrt(a*a-b*b);
  return sqrt(b*b-a*a);
}
#ifdef	__SSE__												/* SSE Optimize */
/*
	a>0 b>0						 sqrt(a*a+b*b)
	a>0 b<=0 a>abs(b)			 sqrt(a*a-b*b)
	a>0 b<=0 a<=abs(b)			-sqrt(b*b-a*a)
	a<=0 b<0					-sqrt(a*a+b*b)
	a<=0 b>=0 abs(a)>abs(b)		-sqrt(a*a-b*b)
	a<=0 b>=0 abs(a)<=abs(b)	 sqrt(b*b-a*a)

	sa	sb	fa<=fb	rs	s(a*b)	s(a*b)&(fa<=fb)	s(a*b)&(fa<=fb)^sa
	0	0	*		0	0		0				0
	0	1	0		0	1		0				0
	0	1	1		1	1		1				1
	1	1	*		1	0		0				1
	1	0	0		1	1		0				1
	1	0	1		0	1		1				0

	sa	sb	fa<=fb	(a&~(fa<=fb))|(b&(fa<=fb))	(a&(fa<=fb))|(b&~(fa<=fb))
	0	0	*		*							*
	0	1	0		a							b
	0	1	1		b							a
	1	1	*		*							*
	1	0	0		a							b
	1	0	1		b							a
*/
STIN __m128 dipole_hypot_ps(float* a, float *b)
{
	__m128	XMM0, XMM1, XMM2, XMM3;
	__m128	A	 = _mm_load_ps(a);
	__m128	B	 = _mm_load_ps(b);
	__m128	PMASK	 = _mm_cmple_ps(_mm_and_ps(A, PM128(PABSMASK)), _mm_and_ps(B, PM128(PABSMASK)));
	XMM2	 = _mm_cmplt_ps(_mm_mul_ps(A, B), PM128(PFV_0));	/* XMM2 = MASK(S(A*B) */
	XMM0	 = _mm_and_ps(A, PM128(PCS_RRRR));					/* XMM0 = SA */
	XMM1	 = _mm_xor_ps(
					_mm_and_ps(
						_mm_and_ps(XMM2, PM128(PCS_RRRR)),
						PMASK
					),
					XMM0
				);												/* XMM1 = Sign of result */
	A		 = _mm_mul_ps(A, A);
	B		 = _mm_mul_ps(B, B);
	XMM2	 = _mm_and_ps(XMM2, PM128(PCS_RRRR));
	XMM3	 = _mm_min_ps(A, B);
	XMM0	 = _mm_max_ps(A, B);
	XMM3	 = _mm_or_ps(XMM3, XMM2);
	B		 = _mm_or_ps(_mm_sqrt_ps(_mm_add_ps(XMM0, XMM3)), XMM1);
	return	B;
}
#endif														/* SSE Optimize */
static float round_hypot(float a, float b){
  if(a>0.){
    if(b>0.)return sqrt(a*a+b*b);
    if(a>-b)return sqrt(a*a+b*b);
    return -sqrt(b*b+a*a);
  }
  if(b<0.)return -sqrt(a*a+b*b);
  if(-a>b)return -sqrt(a*a+b*b);
  return sqrt(b*b+a*a);
}
#ifdef	__SSE__												/* SSE Optimize */
#define round_hypot_ps(d, PA, PB)																		\
{																										\
	__m128	R0, SA;																						\
	{																									\
		__m128	SAMB;																					\
		{																								\
			__m128	FASB;																				\
			{																							\
				__m128	P2A, P2B;																		\
				{																						\
					__m128	FA, FB;																		\
					{																					\
						__m128	A	 = _mm_load_ps(PA);													\
						__m128	B	 = _mm_load_ps(PB);													\
						SA		 = _mm_and_ps(A, PM128(PCS_RRRR));		/* sign of a */					\
						FA		 = _mm_and_ps(A, PM128(PABSMASK));		/* FA = fabs(a) */				\
						FB		 = _mm_and_ps(B, PM128(PABSMASK));		/* FB = fabs(b) */				\
						P2A		 = _mm_mul_ps(A, A);					/* a*a */						\
						P2B		 = _mm_mul_ps(B, B);					/* b*b */						\
						SAMB	 = _mm_mul_ps(A, B);					/* a*b */						\
					}																					\
					FASB	 = _mm_cmple_ps(FA, FB);					/* mask of fa<fb */				\
				}																						\
				R0		 = _mm_add_ps(P2A, P2B);						/* a*a+b*b */					\
			}																							\
			FASB	 = _mm_and_ps(FASB, PM128(PCS_RRRR));				/* sign of F(a)-F(b) */			\
			R0		 = _mm_sqrt_ps(R0);									/* sqrt(a*a+b*b) */				\
			SAMB	 = _mm_and_ps(SAMB, FASB);															\
		}																								\
		SA		 = _mm_xor_ps(SA, SAMB);								/* If a<0, reverse sign */		\
	}																									\
	R0		 = _mm_xor_ps(R0, SA);										/* set sign to result */		\
	_mm_store_ps(d, R0);																				\
}
#endif														/* SSE Optimize */
/* modified hypot by aoyumi 
    better method should be found. */
#ifdef	__SSE__												/* SSE Optimize */
#if	0
/*
	a>0 b>0						 sqrt(a*a+b*b*0.92)
	a>0 b<=0 a>abs(b)			 sqrt(a*a-b*b*0.16)
	a>0 b<=0 a<=abs(b)			-sqrt(b*b-a*a*0.16)
	a<=0 b<0					-sqrt(a*a+b*b*0.92)
	a<=0 b>=0 abs(a)>b			-sqrt(a*a-b*b*0.16)
	a<=0 b>=0 abs(a)<=b			 sqrt(b*b-a*a*0.16)

	sa	sb	fa<=fb	rs	s(a*b)	s(a*b)&(fa<=fb)	s(a*b)&(fa<=fb)^sa
	0	0	*		0	0		0				0
	0	1	0		0	1		0				0
	0	1	1		1	1		1				1
	1	1	*		1	0		0				1
	1	0	0		1	1		0				1
	1	0	1		0	1		1				0
*/
STIN __m128 min_indemnity_dipole_hypot_ps(float* a, float *b)
{
	static _MM_ALIGN16 const float PFV_p92[4]	 = {0.92f, 0.92f, 0.92f, 0.92f};
	static _MM_ALIGN16 const float PFV_mp16[4]	 = {-0.16f, -0.16f, -0.16f, -0.16f};
	static _MM_ALIGN16 const float PFV_mp5[4]	 = {-0.5f, -0.5f, -0.5f, -0.5f};
	static _MM_ALIGN16 const float PFV_1p5[4]	 = {1.5f, 1.5f, 1.5f, 1.5f};
	__m128	XMM0, XMM1, XMM2, XMM3;
	__m128	A	 = _mm_load_ps(a);
	__m128	B	 = _mm_load_ps(b);
	__m128	PMASK	 = _mm_cmple_ps(_mm_and_ps(A, PM128(PABSMASK)), _mm_and_ps(B, PM128(PABSMASK)));
	XMM2	 = _mm_cmplt_ps(_mm_mul_ps(A, B), PM128(PFV_0));	/* XMM2 = MASK(S(A*B) */
	XMM0	 = _mm_and_ps(A, PM128(PCS_RRRR));					/* XMM0 = SA */
	XMM3	 = XMM2;
	XMM3	 = _mm_and_ps(XMM3, PMASK);
	XMM1	 = XMM3;
	XMM1	 = _mm_and_ps(XMM1, PM128(PCS_RRRR));
	XMM1	 = _mm_xor_ps(XMM1, XMM0);
	A		 = _mm_mul_ps(A, A);
	B		 = _mm_mul_ps(B, B);
	XMM0	 = _mm_or_ps(
					_mm_and_ps(PM128(PFV_mp16), XMM2),
					_mm_andnot_ps(XMM2, PM128(PFV_p92))
				);												/* XMM0 = Packed Multi Value */
	XMM2	 = XMM3;
	PMASK	 = B;
	B		 = _mm_or_ps(
					_mm_and_ps(B, XMM2),
					_mm_andnot_ps(XMM2, A)
				);
	A		 = _mm_or_ps(
					_mm_and_ps(A, XMM3),
					_mm_andnot_ps(XMM3, PMASK)
				);
	A		 = _mm_mul_ps(A, XMM0);
	B		 = _mm_add_ps(B, A);
#if	1
	XMM0	 = _mm_rsqrt_ps(B);
	XMM2	 = XMM0;
	XMM3	 = B;
	XMM3	 = _mm_mul_ps(XMM3, XMM0);
	XMM3	 = _mm_mul_ps(XMM3, XMM0);
	XMM3	 = _mm_mul_ps(XMM3, XMM0);
	XMM3	 = _mm_mul_ps(XMM3, PM128(PFV_mp5));
	XMM2	 = _mm_mul_ps(XMM2, PM128(PFV_1p5));
	XMM2	 = _mm_add_ps(XMM2, XMM3);
	B		 = _mm_mul_ps(B, XMM2);
#else
	B		 = _mm_sqrt_ps(B);
#endif
	B		 = _mm_or_ps(B, XMM1);
	return	B;
}
#endif
#endif														/* SSE Optimize */
#if !defined(__SSE__)										/* SSE Optimize */
static float min_indemnity_dipole_hypot(float a, float b){
  float thnor=0.92;
  float threv=0.84;
  float a2 = a*a;
  float b2 = b*b;
  if(a>0.){
    if(b>0.)return sqrt(a2+b2*thnor);
    if(a>-b)return sqrt(a2-b2+b2*threv); 
    return -sqrt(b2-a2+a2*threv);
  }
  if(b<0.)return -sqrt(a2+b2*thnor);
  if(-a>b)return -sqrt(a2-b2+b2*threv);
  return sqrt(b2-a2+a2*threv);
}
#endif														/* SSE Optimize */


/* revert to round hypot for now */
float **_vp_quantize_couple_memo(vorbis_block *vb,
				 vorbis_info_psy_global *g,
				 vorbis_look_psy *p,
				 vorbis_info_mapping0 *vi,
#if	defined(_OPENMP)
				 float **mdct,
				 float **ret,
				 int thnum,
				 int thmax){
#else
				 float **mdct){
#endif
  
  int i,j,n=p->n;
#if	!defined(_OPENMP)
  float **ret=_vorbis_block_alloc(vb,vi->coupling_steps*sizeof(*ret));
#endif
  int limit=g->coupling_pointlimit[p->vi->blockflag][PACKETBLOBS/2];
  
  if(1){ // set new hypot
  	for(i=0;i<vi->coupling_steps;i++){
    	float *mdctM=mdct[vi->coupling_mag[i]];
    	float *mdctA=mdct[vi->coupling_ang[i]];
    	
#if	!defined(_OPENMP)
    	ret[i]=_vorbis_block_alloc(vb,n*sizeof(**ret));
#else
		int s = _omp_get_start(n, 16, thnum, thmax);
		int e = _omp_get_end(n, 16, thnum, thmax);
#endif
#ifdef	__SSE__												/* SSE Optimize */
#if	defined(_OPENMP)
		for(j=s;j<e;j+=16)
#else
		for(j=0;j<n;j+=16)
#endif
		{
			static _MM_ALIGN16 const float PFV_p92[4]	 = {0.92f, 0.92f, 0.92f, 0.92f};
			static _MM_ALIGN16 const float PFV_mp16[4]	 = {-0.16f, -0.16f, -0.16f, -0.16f};
			static _MM_ALIGN16 const float PFV_mp5[4]	 = {-0.5f, -0.5f, -0.5f, -0.5f};
			static _MM_ALIGN16 const float PFV_1p5[4]	 = {1.5f, 1.5f, 1.5f, 1.5f};
			__m128	XMM0, XMM1, XMM2, XMM3;
			__m128	XMM4, XMM5, XMM6, XMM7;
			XMM0	 = _mm_load_ps(mdctM+j   );
			XMM1	 = _mm_load_ps(mdctA+j   );
			XMM2	 = _mm_load_ps(PABSMASK);
			XMM3	 = _mm_load_ps(PFV_0);
			XMM4	 = XMM0;
			XMM5	 = XMM0;
			XMM6	 = XMM0;
			XMM7	 = XMM1;
			XMM5	 = _mm_mul_ps(XMM5, XMM1);
			XMM4	 = _mm_and_ps(XMM4, XMM2);
			XMM7	 = _mm_and_ps(XMM7, XMM2);
			XMM2	 = _mm_load_ps(PCS_RRRR);
			XMM5	 = _mm_cmplt_ps(XMM5, XMM3);
			XMM4	 = _mm_cmple_ps(XMM4, XMM7);
			XMM6	 = _mm_and_ps(XMM6, XMM2);
			XMM3	 = XMM5;
			XMM3	 = _mm_and_ps(XMM3, XMM4);
			XMM7	 = XMM3;
			XMM7	 = _mm_and_ps(XMM7, XMM2);
			XMM2	 = _mm_load_ps(PFV_p92);
			XMM7	 = _mm_xor_ps(XMM7, XMM6);
			XMM6	 = _mm_load_ps(PFV_mp16);
			XMM0	 = _mm_mul_ps(XMM0, XMM0);
			XMM6	 = _mm_and_ps(XMM6, XMM5);
			XMM1	 = _mm_mul_ps(XMM1, XMM1);
			XMM5	 = _mm_andnot_ps(XMM5, XMM2);
			XMM4	 = XMM1;
			XMM6	 = _mm_or_ps(XMM6, XMM5);
			XMM5	 = XMM3;
			XMM2	 = XMM0;
			XMM1	 = _mm_and_ps(XMM1, XMM5);
			XMM0	 = _mm_and_ps(XMM0, XMM3);
			XMM5	 = _mm_andnot_ps(XMM5, XMM2);
			XMM3	 = _mm_andnot_ps(XMM3, XMM4);
			XMM2	 = _mm_load_ps(PFV_mp5);
			XMM4	 = _mm_load_ps(PFV_1p5);
			XMM1	 = _mm_or_ps(XMM1, XMM5);
			XMM0	 = _mm_or_ps(XMM0, XMM3);
			XMM0	 = _mm_mul_ps(XMM0, XMM6);
			XMM1	 = _mm_add_ps(XMM1, XMM0);
			XMM6	 = _mm_rsqrt_ps(XMM1);
			XMM5	 = XMM6;
			XMM3	 = XMM1;
			XMM3	 = _mm_mul_ps(XMM3, XMM6);
			XMM3	 = _mm_mul_ps(XMM3, XMM6);
			XMM0	 = _mm_load_ps(mdctM+j+ 4);
			XMM3	 = _mm_mul_ps(XMM3, XMM6);
			XMM6	 = _mm_load_ps(mdctA+j+ 4);
			XMM3	 = _mm_mul_ps(XMM3, XMM2);
			XMM2	 = _mm_load_ps(PABSMASK);
			XMM5	 = _mm_mul_ps(XMM5, XMM4);
			XMM4	 = _mm_load_ps(PFV_0);
			XMM5	 = _mm_add_ps(XMM5, XMM3);
			XMM3	 = XMM0;
			XMM1		 = _mm_mul_ps(XMM1, XMM5);
			XMM5	 = XMM0;
			XMM1		 = _mm_or_ps(XMM1, XMM7);
			XMM7	 = XMM0;
			_mm_store_ps(ret[i]+j   , XMM1);
			XMM1	 = XMM6;
			XMM5	 = _mm_mul_ps(XMM5, XMM6);
			XMM3	 = _mm_and_ps(XMM3, XMM2);
			XMM1	 = _mm_and_ps(XMM1, XMM2);
			XMM2	 = _mm_load_ps(PCS_RRRR);
			XMM5	 = _mm_cmplt_ps(XMM5, XMM4);
			XMM3	 = _mm_cmple_ps(XMM3, XMM1);
			XMM7	 = _mm_and_ps(XMM7, XMM2);
			XMM4	 = XMM5;
			XMM4	 = _mm_and_ps(XMM4, XMM3);
			XMM1	 = XMM4;
			XMM1	 = _mm_and_ps(XMM1, XMM2);
			XMM2	 = _mm_load_ps(PFV_p92);
			XMM1	 = _mm_xor_ps(XMM1, XMM7);
			XMM7	 = _mm_load_ps(PFV_mp16);
			XMM0	 = _mm_mul_ps(XMM0, XMM0);
			XMM7	 = _mm_and_ps(XMM7, XMM5);
			XMM6	 = _mm_mul_ps(XMM6, XMM6);
			XMM5	 = _mm_andnot_ps(XMM5, XMM2);
			XMM3	 = XMM6;
			XMM7	 = _mm_or_ps(XMM7, XMM5);
			XMM5	 = XMM4;
			XMM2	 = XMM0;
			XMM6	 = _mm_and_ps(XMM6, XMM5);
			XMM0	 = _mm_and_ps(XMM0, XMM4);
			XMM5	 = _mm_andnot_ps(XMM5, XMM2);
			XMM4	 = _mm_andnot_ps(XMM4, XMM3);
			XMM2	 = _mm_load_ps(PFV_mp5);
			XMM3	 = _mm_load_ps(PFV_1p5);
			XMM6	 = _mm_or_ps(XMM6, XMM5);
			XMM0	 = _mm_or_ps(XMM0, XMM4);
			XMM0	 = _mm_mul_ps(XMM0, XMM7);
			XMM6	 = _mm_add_ps(XMM6, XMM0);
			XMM7	 = _mm_rsqrt_ps(XMM6);
			XMM5	 = XMM7;
			XMM4	 = XMM6;
			XMM4	 = _mm_mul_ps(XMM4, XMM7);
			XMM4	 = _mm_mul_ps(XMM4, XMM7);
			XMM0	 = _mm_load_ps(mdctM+j+ 8);
			XMM4	 = _mm_mul_ps(XMM4, XMM7);
			XMM7	 = _mm_load_ps(mdctA+j+ 8);
			XMM4	 = _mm_mul_ps(XMM4, XMM2);
			XMM2	 = _mm_load_ps(PABSMASK);
			XMM5	 = _mm_mul_ps(XMM5, XMM3);
			XMM3	 = _mm_load_ps(PFV_0);
			XMM5	 = _mm_add_ps(XMM5, XMM4);
			XMM4	 = XMM0;
			XMM6		 = _mm_mul_ps(XMM6, XMM5);
			XMM5	 = XMM0;
			XMM6		 = _mm_or_ps(XMM6, XMM1);
			XMM1	 = XMM0;
			_mm_store_ps(ret[i]+j+ 4, XMM6);
			XMM6	 = XMM7;
			XMM5	 = _mm_mul_ps(XMM5, XMM7);
			XMM4	 = _mm_and_ps(XMM4, XMM2);
			XMM6	 = _mm_and_ps(XMM6, XMM2);
			XMM2	 = _mm_load_ps(PCS_RRRR);
			XMM5	 = _mm_cmplt_ps(XMM5, XMM3);
			XMM4	 = _mm_cmple_ps(XMM4, XMM6);
			XMM1	 = _mm_and_ps(XMM1, XMM2);
			XMM3	 = XMM5;
			XMM3	 = _mm_and_ps(XMM3, XMM4);
			XMM6	 = XMM3;
			XMM6	 = _mm_and_ps(XMM6, XMM2);
			XMM2	 = _mm_load_ps(PFV_p92);
			XMM6	 = _mm_xor_ps(XMM6, XMM1);
			XMM1	 = _mm_load_ps(PFV_mp16);
			XMM0	 = _mm_mul_ps(XMM0, XMM0);
			XMM1	 = _mm_and_ps(XMM1, XMM5);
			XMM7	 = _mm_mul_ps(XMM7, XMM7);
			XMM5	 = _mm_andnot_ps(XMM5, XMM2);
			XMM4	 = XMM7;
			XMM1	 = _mm_or_ps(XMM1, XMM5);
			XMM5	 = XMM3;
			XMM2	 = XMM0;
			XMM7	 = _mm_and_ps(XMM7, XMM5);
			XMM0	 = _mm_and_ps(XMM0, XMM3);
			XMM5	 = _mm_andnot_ps(XMM5, XMM2);
			XMM3	 = _mm_andnot_ps(XMM3, XMM4);
			XMM2	 = _mm_load_ps(PFV_mp5);
			XMM4	 = _mm_load_ps(PFV_1p5);
			XMM7	 = _mm_or_ps(XMM7, XMM5);
			XMM0	 = _mm_or_ps(XMM0, XMM3);
			XMM0	 = _mm_mul_ps(XMM0, XMM1);
			XMM7	 = _mm_add_ps(XMM7, XMM0);
			XMM1	 = _mm_rsqrt_ps(XMM7);
			XMM5	 = XMM1;
			XMM3	 = XMM7;
			XMM3	 = _mm_mul_ps(XMM3, XMM1);
			XMM3	 = _mm_mul_ps(XMM3, XMM1);
			XMM0	 = _mm_load_ps(mdctM+j+12);
			XMM3	 = _mm_mul_ps(XMM3, XMM1);
			XMM1	 = _mm_load_ps(mdctA+j+12);
			XMM3	 = _mm_mul_ps(XMM3, XMM2);
			XMM2	 = _mm_load_ps(PABSMASK);
			XMM5	 = _mm_mul_ps(XMM5, XMM4);
			XMM4	 = _mm_load_ps(PFV_0);
			XMM5	 = _mm_add_ps(XMM5, XMM3);
			XMM3	 = XMM0;
			XMM7		 = _mm_mul_ps(XMM7, XMM5);
			XMM5	 = XMM0;
			XMM7		 = _mm_or_ps(XMM7, XMM6);
			XMM6	 = XMM0;
			_mm_store_ps(ret[i]+j+ 8, XMM7);
			XMM7	 = XMM1;
			XMM5	 = _mm_mul_ps(XMM5, XMM1);
			XMM3	 = _mm_and_ps(XMM3, XMM2);
			XMM7	 = _mm_and_ps(XMM7, XMM2);
			XMM2	 = _mm_load_ps(PCS_RRRR);
			XMM5	 = _mm_cmplt_ps(XMM5, XMM4);
			XMM3	 = _mm_cmple_ps(XMM3, XMM7);
			XMM6	 = _mm_and_ps(XMM6, XMM2);
			XMM4	 = XMM5;
			XMM4	 = _mm_and_ps(XMM4, XMM3);
			XMM7	 = XMM4;
			XMM7	 = _mm_and_ps(XMM7, XMM2);
			XMM2	 = _mm_load_ps(PFV_p92);
			XMM7	 = _mm_xor_ps(XMM7, XMM6);
			XMM6	 = _mm_load_ps(PFV_mp16);
			XMM0	 = _mm_mul_ps(XMM0, XMM0);
			XMM6	 = _mm_and_ps(XMM6, XMM5);
			XMM1	 = _mm_mul_ps(XMM1, XMM1);
			XMM5	 = _mm_andnot_ps(XMM5, XMM2);
			XMM3	 = XMM1;
			XMM6	 = _mm_or_ps(XMM6, XMM5);
			XMM5	 = XMM4;
			XMM2	 = XMM0;
			XMM1	 = _mm_and_ps(XMM1, XMM5);
			XMM0	 = _mm_and_ps(XMM0, XMM4);
			XMM5	 = _mm_andnot_ps(XMM5, XMM2);
			XMM4	 = _mm_andnot_ps(XMM4, XMM3);
			XMM2	 = _mm_load_ps(PFV_mp5);
			XMM3	 = _mm_load_ps(PFV_1p5);
			XMM1	 = _mm_or_ps(XMM1, XMM5);
			XMM0	 = _mm_or_ps(XMM0, XMM4);
			XMM0	 = _mm_mul_ps(XMM0, XMM6);
			XMM1	 = _mm_add_ps(XMM1, XMM0);
			XMM6	 = _mm_rsqrt_ps(XMM1);
			XMM5	 = XMM6;
			XMM4	 = XMM1;
			XMM4	 = _mm_mul_ps(XMM4, XMM6);
			XMM4	 = _mm_mul_ps(XMM4, XMM6);
			XMM4	 = _mm_mul_ps(XMM4, XMM6);
			XMM4	 = _mm_mul_ps(XMM4, XMM2);
			XMM5	 = _mm_mul_ps(XMM5, XMM3);
			XMM5	 = _mm_add_ps(XMM5, XMM4);
			XMM1		 = _mm_mul_ps(XMM1, XMM5);
			XMM1		 = _mm_or_ps(XMM1, XMM7);
			_mm_store_ps(ret[i]+j+12, XMM1);
		}
#else														/* SSE Optimize */
    	for(j=0;j<n;j++)
    	 ret[i][j]=min_indemnity_dipole_hypot(mdctM[j],mdctA[j]);
#endif														/* SSE Optimize */
  	}
  }else{
    for(i=0;i<vi->coupling_steps;i++){
    	float *mdctM=mdct[vi->coupling_mag[i]];
    	float *mdctA=mdct[vi->coupling_ang[i]];
    	
#if	!defined(_OPENMP)
    	ret[i]=_vorbis_block_alloc(vb,n*sizeof(**ret));
#endif
#ifdef	__SSE__												/* SSE Optimize */
		{
			float	*p	 = ret[i];
			int limit4	 = limit&(~7);
			for(j=0;j<limit4;j+=8)
			{
				_mm_store_ps(p+j  , dipole_hypot_ps(mdctM+j  , mdctA+j  ));
				_mm_store_ps(p+j+4, dipole_hypot_ps(mdctM+j+4, mdctA+j+4));
			}
			limit4	 = limit&(~3);
			for(;j<limit4;j+=4)
			{
				_mm_store_ps(p+j  , dipole_hypot_ps(mdctM+j  , mdctA+j  ));
			}
			for(;j<limit;j++)
				p[j]	 = dipole_hypot(mdctM[j],mdctA[j]);
			limit4	 = (limit+3)&(~3);
			limit4	 = (limit4>=n)?n:limit4;
			for(;j<limit4;j++)
				p[j]	 = round_hypot(mdctM[j],mdctA[j]);
			limit4	 = (limit+7)&(~7);
			limit4	 = (limit4>=n)?n:limit4;
			for(;j<limit4;j+=4)
			{
				round_hypot_ps(&p[j  ], &mdctM[j  ], &mdctA[j  ]);
			}
			for(;j<n;j+=8)
			{
				round_hypot_ps(&p[j  ], &mdctM[j  ], &mdctA[j  ]);
				round_hypot_ps(&p[j+4], &mdctM[j+4], &mdctA[j+4]);
			}
		}
#else														/* SSE Optimize */
    	for(j=0;j<limit;j++)
    	 ret[i][j]=dipole_hypot(mdctM[j],mdctA[j]);
    	for(;j<n;j++)
      	 ret[i][j]=round_hypot(mdctM[j],mdctA[j]);
#endif														/* SSE Optimize */
  	}
  }
  return(ret);
}

/* this is for per-channel noise normalization */
#ifdef	__SSE__												/* SSE Optimize */
#define C(a,b)\
  (data[a]>=data[b])
/*
0	ACBA
1	DDCB
2	ACDC

0<1	D>A D>C C>B B>A
0<2	000 000 D>B C>A
Cond.		(0<2<<4)|(0<1)	SCODE

D>C>B>A		111111	63		3210
C>D>B>A		111011	59		2310
D>B>C>A		111101	61		3120
B>D>C>A		011101	29		1320
C>B>D>A		011011	27		2130
B>C>D>A		011001	25		1230
D>C>A>B		111110	30		3201
C>D>A>B		111010	58		2301
D>A>C>B		101110	46		3021
A>D>C>B		100110	38		0321
C>A>D>B		110010	50		2031
A>C>D>B		100010	18		0231
D>B>A>C		101101	45		3102
B>D>A>C		001101	13		1302
D>A>B>C		101100	44		3012
A>D>B>C		100100	36		0312
B>A>D>C		000101	 5		1032
A>B>D>C		000100	 4		0132
C>B>A>D		010011	19		2103
B>C>A>D		010001	17		1203
C>A>B>D		010010	18		2013
A>C>B>D		000010	 2		0213
B>A>C>D		000001	 1		1023
A>B>C>D		000000	 0		0123

A>B>C>D		000000	 0		0123
B>A>C>D		000001	 1		1023
A>C>B>D		000010	 2		0213
A>B>D>C		000100	 4		0132
B>A>D>C		000101	 5		1032
B>D>A>C		001101	13		1302
B>C>A>D		010001	17		1203
C>A>B>D		010010	18		2013
C>B>A>D		010011	19		2103
B>C>D>A		011001	25		1230
C>B>D>A		011011	27		2130
B>D>C>A		011101	29		1320
A>C>D>B		100010	34		0231
A>D>B>C		100100	36		0312
A>D>C>B		100110	38		0321
D>A>B>C		101100	44		3012
D>B>A>C		101101	45		3102
D>A>C>B		101110	46		3021
C>A>D>B		110010	50		2031
C>D>A>B		111010	58		2301
C>D>B>A		111011	59		2310
D>B>C>A		111101	61		3120
D>C>A>B		111110	62		3201
D>C>B>A		111111	63		3210

*/

STIN void SORT4x2(float *i, int *n)
{
	int	c0, c1;
#if	defined(__SSE2__)
	__m128i	XMM0, XMM1;
	static _MM_ALIGN16 unsigned int PI4[4] = {4, 4, 4, 4};
#endif
	{
		__m128	P0, P1, P2, P3, P4, P5;
		P0	 = _mm_load_ps(i  );
		P3	 = _mm_load_ps(i+4);
		P1	 = P0;
		P2	 = P0;
		P4	 = P3;
		P5	 = P3;
		P0	 = _mm_shuffle_ps(P0, P0, _MM_SHUFFLE(0,2,1,0));
		P1	 = _mm_shuffle_ps(P1, P1, _MM_SHUFFLE(3,3,2,1));
		P2	 = _mm_shuffle_ps(P2, P2, _MM_SHUFFLE(0,2,3,2));
		P3	 = _mm_shuffle_ps(P3, P3, _MM_SHUFFLE(0,2,1,0));
		P4	 = _mm_shuffle_ps(P4, P4, _MM_SHUFFLE(3,3,2,1));
		P5	 = _mm_shuffle_ps(P5, P5, _MM_SHUFFLE(0,2,3,2));
	
		P1	 = _mm_cmplt_ps(P1, P0);
		P2	 = _mm_cmplt_ps(P2, P0);
		P4	 = _mm_cmplt_ps(P4, P3);
		P5	 = _mm_cmplt_ps(P5, P3);
		c0	 = _mm_movemask_ps(P2);
		c1	 = _mm_movemask_ps(P5);
		c0	 = c0 << 4;
		c1	 = c1 << 4;
		c0	 = c0|_mm_movemask_ps(P1);
		c1	 = c1|_mm_movemask_ps(P4);
		c0	 = c0 << 2;
		c1	 = c1 << 2;
	}
#if	defined(__SSE2__)
	XMM1	 = _mm_load_si128((__m128i*)(Sort4IndexConvTable+c1));
	XMM0	 = _mm_load_si128((__m128i*)(Sort4IndexConvTable+c0));
	XMM1	 = _mm_add_epi32(XMM1, PM128I(PI4));
	_mm_store_si128((__m128i*)(n  ), XMM0);
	_mm_store_si128((__m128i*)(n+4), XMM1);
#else
	n[0]	 =Sort4IndexConvTable[c0  ];
	n[1]	 =Sort4IndexConvTable[c0+1];
	n[2]	 =Sort4IndexConvTable[c0+2];
	n[3]	 =Sort4IndexConvTable[c0+3];
	n[4]	 =Sort4IndexConvTable[c1  ]+4;
	n[5]	 =Sort4IndexConvTable[c1+1]+4;
	n[6]	 =Sort4IndexConvTable[c1+2]+4;
	n[7]	 =Sort4IndexConvTable[c1+3]+4;
#endif
}

STIN void sortindex_fix8(int *index,
                         float *data,
                         int offset){
	_MM_ALIGN16 int n[8];
	index	+= offset;
	data	+= offset;
	SORT4x2(data, n);
	{
		__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
		XMM0	 = _mm_load_ss(data+n[0]);
		XMM4	 = _mm_load_ss(data+n[4]);
		XMM1	 = _mm_load_ss(data+n[1]);
		XMM5	 = _mm_load_ss(data+n[5]);
		XMM2	 = _mm_load_ss(data+n[2]);
		XMM6	 = _mm_load_ss(data+n[6]);
		XMM3	 = _mm_load_ss(data+n[3]);
		XMM7	 = _mm_load_ss(data+n[7]);
		if(!_mm_comilt_ss(XMM0, XMM4)){
			index[0]	 = n[0]+offset;
			if(!_mm_comilt_ss(XMM1, XMM4)){
				index[1]	 = n[1]+offset;
				if(!_mm_comilt_ss(XMM2, XMM4)){
					index[2]	 = n[2]+offset;
					if(!_mm_comilt_ss(XMM3, XMM4)){
						index[3]	 = n[3]+offset;
						index[4]	 = n[4]+offset;
						index[5]	 = n[5]+offset;
						index[6]	 = n[6]+offset;
						index[7]	 = n[7]+offset;
					}else{
						index[3]	 = n[4]+offset;
SORT8_4_35:
						if(!_mm_comilt_ss(XMM3, XMM5)){
							index[4]	 = n[3]+offset;
							index[5]	 = n[5]+offset;
							index[6]	 = n[6]+offset;
							index[7]	 = n[7]+offset;
						}else{
							index[4]	 = n[5]+offset;
SORT8_5_36:
							if(!_mm_comilt_ss(XMM3, XMM6)){
								index[5]	 = n[3]+offset;
								index[6]	 = n[6]+offset;
								index[7]	 = n[7]+offset;
							}else{
								index[5]	 = n[6]+offset;
SORT8_6_37:
								if(!_mm_comilt_ss(XMM3, XMM7)){
									index[6]	 = n[3]+offset;
									index[7]	 = n[7]+offset;
								}else{
									index[6]	 = n[7]+offset;
									index[7]	 = n[3]+offset;
								}
							}
						}
					}
				}else{
					index[2]	 = n[4]+offset;
SORT8_3_25:
					if(!_mm_comilt_ss(XMM2, XMM5)){
						index[3]	 = n[2]+offset;
						goto SORT8_4_35;
					}else{
						index[3]	 = n[5]+offset;
SORT8_4_26:
						if(!_mm_comilt_ss(XMM2, XMM6)){
							index[4]	 = n[2]+offset;
							goto SORT8_5_36;
						}else{
							index[4]	 = n[6]+offset;
SORT8_5_27:
							if(!_mm_comilt_ss(XMM2, XMM7)){
								index[5]	 = n[2]+offset;
								goto SORT8_6_37;
							}else{
								index[5]	 = n[7]+offset;
								index[6]	 = n[2]+offset;
								index[7]	 = n[3]+offset;
							}
						}
					}
				}
			}else{
				index[1]	 = n[4]+offset;
SORT8_2_15:
				if(!_mm_comilt_ss(XMM1, XMM5)){
					index[2]	 = n[1]+offset;
					goto SORT8_3_25;
				}else{
					index[2]	 = n[5]+offset;
SORT8_3_16:
					if(!_mm_comilt_ss(XMM1, XMM6)){
						index[3]	 = n[1]+offset;
						goto SORT8_4_26;
					}else{
						index[3]	 = n[6]+offset;
SORT8_4_17:
						if(!_mm_comilt_ss(XMM1, XMM7)){
							index[4]	 = n[1]+offset;
							goto SORT8_5_27;
						}else{
							index[4]	 = n[7]+offset;
							index[5]	 = n[1]+offset;
							index[6]	 = n[2]+offset;
							index[7]	 = n[3]+offset;
						}
					}
				}
			}
		}else{
			index[0]	 = n[4]+offset;
			if(!_mm_comilt_ss(XMM0, XMM5)){
				index[1]	 = n[0]+offset;
				goto SORT8_2_15;
			}else{
				index[1]	 = n[5]+offset;
				if(!_mm_comilt_ss(XMM0, XMM6)){
					index[2]	 = n[0]+offset;
					goto SORT8_3_16;
				}else{
					index[2]	 = n[6]+offset;
					if(!_mm_comilt_ss(XMM0, XMM7)){
						index[3]	 = n[0]+offset;
						goto SORT8_4_17;
					}else{
						index[3]	 = n[7]+offset;
						index[4]	 = n[0]+offset;
						index[5]	 = n[1]+offset;
						index[6]	 = n[2]+offset;
						index[7]	 = n[3]+offset;
					}
				}
			}
		}
	}
}
STIN void sortindex_fix16(int *index,
						  int *n,
						  float *data,
						  int j){
	__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
	index	+= j;
	n		+= j;
	XMM0	 = _mm_load_ss(data+n[0]);
	XMM4	 = _mm_load_ss(data+n[8]);
	XMM1	 = _mm_load_ss(data+n[1]);
	XMM5	 = _mm_load_ss(data+n[9]);
	XMM2	 = _mm_load_ss(data+n[2]);
	XMM6	 = _mm_load_ss(data+n[10]);
	XMM3	 = _mm_load_ss(data+n[3]);
	XMM7	 = _mm_load_ss(data+n[11]);
	if(!_mm_comilt_ss(XMM0, XMM4)){
	  index[0]	 = n[0];
	  if(!_mm_comilt_ss(XMM1, XMM4)){
		index[1]	 = n[1];
		if(!_mm_comilt_ss(XMM2, XMM4)){
		  index[2]	 = n[2];
		  if(!_mm_comilt_ss(XMM3, XMM4)){
			index[3]	 = n[3];
			XMM0	 = _mm_load_ss(data+n[4]);
			XMM1	 = _mm_load_ss(data+n[5]);
			XMM2	 = _mm_load_ss(data+n[6]);
			XMM3	 = _mm_load_ss(data+n[7]);
			if(!_mm_comilt_ss(XMM0, XMM4)){
			  index[4]	 = n[4];
			  if(!_mm_comilt_ss(XMM1, XMM4)){
				index[5]	 = n[5];
				if(!_mm_comilt_ss(XMM2, XMM4)){
				  index[6]	 = n[6];
				  if(!_mm_comilt_ss(XMM3, XMM4)){
					index[7]	 = n[7];
					index[8]	 = n[8];
					index[9]	 = n[9];
					index[10]	 = n[10];
					index[11]	 = n[11];
					index[12]	 = n[12];
					index[13]	 = n[13];
					index[14]	 = n[14];
					index[15]	 = n[15];
				  }else{
					index[7]	 = n[8];
SORT16_080709:
					if(!_mm_comilt_ss(XMM3, XMM5)){
					  index[8]	 = n[7];
					  index[9]	 = n[9];
					  index[10]	 = n[10];
					  index[11]	 = n[11];
					  index[12]	 = n[12];
					  index[13]	 = n[13];
					  index[14]	 = n[14];
					  index[15]	 = n[15];
					}else{
					  index[8]	 = n[9];
SORT16_09070A:
					  if(!_mm_comilt_ss(XMM3, XMM6)){
						index[9]	 = n[7];
						index[10]	 = n[10];
						index[11]	 = n[11];
						index[12]	 = n[12];
						index[13]	 = n[13];
						index[14]	 = n[14];
						index[15]	 = n[15];
					  }else{
						index[9]	 = n[10];
SORT16_0A070B:
						if(!_mm_comilt_ss(XMM3, XMM7)){
						  index[10]	 = n[7];
						  index[11]	 = n[11];
						  index[12]	 = n[12];
						  index[13]	 = n[13];
						  index[14]	 = n[14];
						  index[15]	 = n[15];
						}else{
						  index[10]	 = n[11];
						  XMM4	 = _mm_load_ss(data+n[12]);
						  XMM5	 = _mm_load_ss(data+n[13]);
						  XMM6	 = _mm_load_ss(data+n[14]);
						  XMM7	 = _mm_load_ss(data+n[15]);
SORT16_0B070C:
						  if(!_mm_comilt_ss(XMM3, XMM4)){
							index[11]	 = n[7];
							index[12]	 = n[12];
							index[13]	 = n[13];
							index[14]	 = n[14];
							index[15]	 = n[15];
						  }else{
							index[11]	 = n[12];
SORT16_0C070D:
							if(!_mm_comilt_ss(XMM3, XMM5)){
							  index[12]	 = n[7];
							  index[13]	 = n[13];
							  index[14]	 = n[14];
							  index[15]	 = n[15];
							}else{
							  index[12]	 = n[13];
SORT16_0D070E:
							  if(!_mm_comilt_ss(XMM3, XMM6)){
								index[13]	 = n[7];
								index[14]	 = n[14];
								index[15]	 = n[15];
							  }else{
								index[13]	 = n[14];
SORT16_0E070F:
								if(!_mm_comilt_ss(XMM3, XMM7)){
								  index[14]	 = n[7];
								  index[15]	 = n[15];
								}else{
								  index[14]	 = n[15];
								  index[15]	 = n[7];
								}
							  }
							}
						  }
						}
					  }
					}
				  }
				}else{
				  index[6]	 = n[8];
SORT16_070609:
				  if(!_mm_comilt_ss(XMM2, XMM5)){
					index[7]	 = n[6];
					goto SORT16_080709;
				  }else{
					index[7]	 = n[9];
SORT16_08060A:
					if(!_mm_comilt_ss(XMM2, XMM6)){
					  index[8]	 = n[6];
					  goto SORT16_09070A;
					}else{
					  index[8]	 = n[10];
SORT16_09060B:
					  if(!_mm_comilt_ss(XMM2, XMM7)){
						index[9]	 = n[6];
						goto SORT16_0A070B;
					  }else{
						index[9]	 = n[11];
						XMM4	 = _mm_load_ss(data+n[12]);
						XMM5	 = _mm_load_ss(data+n[13]);
						XMM6	 = _mm_load_ss(data+n[14]);
						XMM7	 = _mm_load_ss(data+n[15]);
SORT16_0A060C:
						if(!_mm_comilt_ss(XMM2, XMM4)){
						  index[10]	 = n[6];
						  goto SORT16_0B070C;
						}else{
						  index[10]	 = n[12];
SORT16_0B060D:
						  if(!_mm_comilt_ss(XMM2, XMM5)){
							index[11]	 = n[6];
							goto SORT16_0C070D;
						  }else{
							index[11]	 = n[13];
SORT16_0C060E:
							if(!_mm_comilt_ss(XMM2, XMM6)){
							  index[12]	 = n[6];
							  goto SORT16_0D070E;
							}else{
							  index[12]	 = n[14];
SORT16_0D060F:
							  if(!_mm_comilt_ss(XMM2, XMM7)){
								index[13]	 = n[6];
								goto SORT16_0E070F;
							  }else{
								index[13]	 = n[15];
								index[14]	 = n[6];
								index[15]	 = n[7];
							  }
							}
						  }
						}
					  }
					}
				  }
				}
			  }else{
				index[5]	 = n[8];
SORT16_060509:
				if(!_mm_comilt_ss(XMM1, XMM5)){
				  index[6]	 = n[5];
				  goto SORT16_070609;
				}else{
				  index[6]	 = n[9];
SORT16_07050A:
				  if(!_mm_comilt_ss(XMM1, XMM6)){
					index[7]	 = n[5];
					goto SORT16_08060A;
				  }else{
					index[7]	 = n[10];
SORT16_08050B:
					if(!_mm_comilt_ss(XMM1, XMM7)){
					  index[8]	 = n[5];
					  goto SORT16_09060B;
					}else{
					  index[8]	 = n[11];
					  XMM4	 = _mm_load_ss(data+n[12]);
					  XMM5	 = _mm_load_ss(data+n[13]);
					  XMM6	 = _mm_load_ss(data+n[14]);
					  XMM7	 = _mm_load_ss(data+n[15]);
SORT16_09050C:
					  if(!_mm_comilt_ss(XMM1, XMM4)){
						index[9]	 = n[5];
						goto SORT16_0A060C;
					  }else{
						index[9]	 = n[12];
SORT16_0A050D:
						if(!_mm_comilt_ss(XMM1, XMM5)){
						  index[10]	 = n[5];
						  goto SORT16_0B060D;
						}else{
						  index[10]	 = n[13];
SORT16_0B050E:
						  if(!_mm_comilt_ss(XMM1, XMM6)){
							index[11]	 = n[5];
							goto SORT16_0C060E;
						  }else{
							index[11]	 = n[14];
SORT16_0C050F:
							if(!_mm_comilt_ss(XMM1, XMM7)){
							  index[12]	 = n[5];
							  goto SORT16_0D060F;
							}else{
							  index[12]	 = n[15];
							  index[13]	 = n[5];
							  index[14]	 = n[6];
							  index[15]	 = n[7];
							}
						  }
						}
					  }
					}
				  }
				}
			  }
			}else{
			  index[4]	 = n[8];
SORT16_050409:
			  if(!_mm_comilt_ss(XMM0, XMM5)){
				index[5]	 = n[4];
				goto SORT16_060509;
			  }else{
				index[5]	 = n[9];
SORT16_06040A:
				if(!_mm_comilt_ss(XMM0, XMM6)){
				  index[6]	 = n[4];
				  goto SORT16_07050A;
				}else{
				  index[6]	 = n[10];
SORT16_07040B:
				  if(!_mm_comilt_ss(XMM0, XMM7)){
					index[7]	 = n[4];
					goto SORT16_08050B;
				  }else{
					index[7]	 = n[11];
					XMM4	 = _mm_load_ss(data+n[12]);
					XMM5	 = _mm_load_ss(data+n[13]);
					XMM6	 = _mm_load_ss(data+n[14]);
					XMM7	 = _mm_load_ss(data+n[15]);
SORT16_08040C:
					if(!_mm_comilt_ss(XMM0, XMM4)){
					  index[8]	 = n[4];
					  goto SORT16_09050C;
					}else{
					  index[8]	 = n[12];
SORT16_09040D:
					  if(!_mm_comilt_ss(XMM0, XMM5)){
						index[9]	 = n[4];
						goto SORT16_0A050D;
					  }else{
						index[9]	 = n[13];
SORT16_0A040E:
						if(!_mm_comilt_ss(XMM0, XMM6)){
						  index[10]	 = n[4];
						  goto SORT16_0B050E;
						}else{
						  index[10]	 = n[14];
SORT16_0B040F:
						  if(!_mm_comilt_ss(XMM0, XMM7)){
							index[11]	 = n[4];
							goto SORT16_0C050F;
						  }else{
							index[11]	 = n[15];
							index[12]	 = n[4];
							index[13]	 = n[5];
							index[14]	 = n[6];
							index[15]	 = n[7];
						  }
						}
					  }
					}
				  }
				}
			  }
			}
		  }else{
			index[3]	 = n[8];
SORT16_040309:
			if(!_mm_comilt_ss(XMM3, XMM5)){
			  index[4]	 = n[3];
			  XMM0	 = _mm_load_ss(data+n[4]);
			  XMM1	 = _mm_load_ss(data+n[5]);
			  XMM2	 = _mm_load_ss(data+n[6]);
			  XMM3	 = _mm_load_ss(data+n[7]);
			  goto SORT16_050409;
			}else{
			  index[4]	 = n[9];
SORT16_05030A:
			  if(!_mm_comilt_ss(XMM3, XMM6)){
				index[5]	 = n[3];
				XMM0	 = _mm_load_ss(data+n[4]);
				XMM1	 = _mm_load_ss(data+n[5]);
				XMM2	 = _mm_load_ss(data+n[6]);
				XMM3	 = _mm_load_ss(data+n[7]);
				goto SORT16_06040A;
			  }else{
				index[5]	 = n[10];
SORT16_06030B:
				if(!_mm_comilt_ss(XMM3, XMM7)){
				  index[6]	 = n[3];
				  XMM0	 = _mm_load_ss(data+n[4]);
				  XMM1	 = _mm_load_ss(data+n[5]);
				  XMM2	 = _mm_load_ss(data+n[6]);
				  XMM3	 = _mm_load_ss(data+n[7]);
				  goto SORT16_07040B;
				}else{
				  index[6]	 = n[11];
				  XMM4	 = _mm_load_ss(data+n[12]);
				  XMM5	 = _mm_load_ss(data+n[13]);
				  XMM6	 = _mm_load_ss(data+n[14]);
				  XMM7	 = _mm_load_ss(data+n[15]);
SORT16_07030C:
				  if(!_mm_comilt_ss(XMM3, XMM4)){
					index[7]	 = n[3];
					XMM0	 = _mm_load_ss(data+n[4]);
					XMM1	 = _mm_load_ss(data+n[5]);
					XMM2	 = _mm_load_ss(data+n[6]);
					XMM3	 = _mm_load_ss(data+n[7]);
					goto SORT16_08040C;
				  }else{
					index[7]	 = n[12];
SORT16_08030D:
					if(!_mm_comilt_ss(XMM3, XMM5)){
					  index[8]	 = n[3];
					  XMM0	 = _mm_load_ss(data+n[4]);
					  XMM1	 = _mm_load_ss(data+n[5]);
					  XMM2	 = _mm_load_ss(data+n[6]);
					  XMM3	 = _mm_load_ss(data+n[7]);
					  goto SORT16_09040D;
					}else{
					  index[8]	 = n[13];
SORT16_09030E:
					  if(!_mm_comilt_ss(XMM3, XMM6)){
						index[9]	 = n[3];
						XMM0	 = _mm_load_ss(data+n[4]);
						XMM1	 = _mm_load_ss(data+n[5]);
						XMM2	 = _mm_load_ss(data+n[6]);
						XMM3	 = _mm_load_ss(data+n[7]);
						goto SORT16_0A040E;
					  }else{
						index[9]	 = n[14];
SORT16_0A030F:
						if(!_mm_comilt_ss(XMM3, XMM7)){
						  index[10]	 = n[3];
						  XMM0	 = _mm_load_ss(data+n[4]);
						  XMM1	 = _mm_load_ss(data+n[5]);
						  XMM2	 = _mm_load_ss(data+n[6]);
						  XMM3	 = _mm_load_ss(data+n[7]);
						  goto SORT16_0B040F;
						}else{
						  index[10]	 = n[15];
						  index[11]	 = n[3];
						  index[12]	 = n[4];
						  index[13]	 = n[5];
						  index[14]	 = n[6];
						  index[15]	 = n[7];
						}
					  }
					}
				  }
				}
			  }
			}
		  }
		}else{
		  index[2]	 = n[8];
SORT16_030209:
		  if(!_mm_comilt_ss(XMM2, XMM5)){
			index[3]	 = n[2];
			goto SORT16_040309;
		  }else{
			index[3]	 = n[9];
SORT16_04020A:
			if(!_mm_comilt_ss(XMM2, XMM6)){
			  index[4]	 = n[2];
			  goto SORT16_05030A;
			}else{
			  index[4]	 = n[10];
SORT16_05020B:
			  if(!_mm_comilt_ss(XMM2, XMM7)){
				index[5]	 = n[2];
				goto SORT16_06030B;
			  }else{
				index[5]	 = n[11];
				XMM4	 = _mm_load_ss(data+n[12]);
				XMM5	 = _mm_load_ss(data+n[13]);
				XMM6	 = _mm_load_ss(data+n[14]);
				XMM7	 = _mm_load_ss(data+n[15]);
SORT16_06020C:
				if(!_mm_comilt_ss(XMM2, XMM4)){
				  index[6]	 = n[2];
				  goto SORT16_07030C;
				}else{
				  index[6]	 = n[12];
SORT16_07020D:
				  if(!_mm_comilt_ss(XMM2, XMM5)){
					index[7]	 = n[2];
					goto SORT16_08030D;
				  }else{
					index[7]	 = n[13];
SORT16_08020E:
					if(!_mm_comilt_ss(XMM2, XMM6)){
					  index[8]	 = n[2];
					  goto SORT16_09030E;
					}else{
					  index[8]	 = n[14];
SORT16_09020F:
					  if(!_mm_comilt_ss(XMM2, XMM7)){
						index[9]	 = n[2];
						goto SORT16_0A030F;
					  }else{
						index[9]	 = n[15];
						index[10]	 = n[2];
						index[11]	 = n[3];
						index[12]	 = n[4];
						index[13]	 = n[5];
						index[14]	 = n[6];
						index[15]	 = n[7];
					  }
					}
				  }
				}
			  }
			}
		  }
		}
	  }else{
		index[1]	 = n[8];
SORT16_020109:
		if(!_mm_comilt_ss(XMM1, XMM5)){
		  index[2]	 = n[1];
		  goto SORT16_030209;
		}else{
		  index[2]	 = n[9];
SORT16_03010A:
		  if(!_mm_comilt_ss(XMM1, XMM6)){
			index[3]	 = n[1];
			goto SORT16_04020A;
		  }else{
			index[3]	 = n[10];
SORT16_04010B:
			if(!_mm_comilt_ss(XMM1, XMM7)){
			  index[4]	 = n[1];
			  goto SORT16_05020B;
			}else{
			  index[4]	 = n[11];
			  XMM4	 = _mm_load_ss(data+n[12]);
			  XMM5	 = _mm_load_ss(data+n[13]);
			  XMM6	 = _mm_load_ss(data+n[14]);
			  XMM7	 = _mm_load_ss(data+n[15]);
SORT16_05010C:
			  if(!_mm_comilt_ss(XMM1, XMM4)){
				index[5]	 = n[1];
				goto SORT16_06020C;
			  }else{
				index[5]	 = n[12];
SORT16_06010D:
				if(!_mm_comilt_ss(XMM1, XMM5)){
				  index[6]	 = n[1];
				  goto SORT16_07020D;
				}else{
				  index[6]	 = n[13];
SORT16_07010E:
				  if(!_mm_comilt_ss(XMM1, XMM6)){
					index[7]	 = n[1];
					goto SORT16_08020E;
				  }else{
					index[7]	 = n[14];
SORT16_08010F:
					if(!_mm_comilt_ss(XMM1, XMM7)){
					  index[8]	 = n[1];
					  goto SORT16_09020F;
					}else{
					  index[8]	 = n[15];
					  index[9]	 = n[1];
					  index[10]	 = n[2];
					  index[11]	 = n[3];
					  index[12]	 = n[4];
					  index[13]	 = n[5];
					  index[14]	 = n[6];
					  index[15]	 = n[7];
					}
				  }
				}
			  }
			}
		  }
		}
	  }
	}else{
	  index[0]	 = n[8];
	  if(!_mm_comilt_ss(XMM0, XMM5)){
		index[1]	 = n[0];
		goto SORT16_020109;
	  }else{
		index[1]	 = n[9];
		if(!_mm_comilt_ss(XMM0, XMM6)){
		  index[2]	 = n[0];
		  goto SORT16_03010A;
		}else{
		  index[2]	 = n[10];
		  if(!_mm_comilt_ss(XMM0, XMM7)){
			index[3]	 = n[0];
			goto SORT16_04010B;
		  }else{
			index[3]	 = n[11];
			XMM4	 = _mm_load_ss(data+n[12]);
			XMM5	 = _mm_load_ss(data+n[13]);
			XMM6	 = _mm_load_ss(data+n[14]);
			XMM7	 = _mm_load_ss(data+n[15]);
			if(!_mm_comilt_ss(XMM0, XMM4)){
			  index[4]	 = n[0];
			  goto SORT16_05010C;
			}else{
			  index[4]	 = n[12];
			  if(!_mm_comilt_ss(XMM0, XMM5)){
				index[5]	 = n[0];
				goto SORT16_06010D;
			  }else{
				index[5]	 = n[13];
				if(!_mm_comilt_ss(XMM0, XMM6)){
				  index[6]	 = n[0];
				  goto SORT16_07010E;
				}else{
				  index[6]	 = n[14];
				  if(!_mm_comilt_ss(XMM0, XMM7)){
					index[7]	 = n[0];
					goto SORT16_08010F;
				  }else{
					index[7]	 = n[15];
					index[8]	 = n[0];
					index[9]	 = n[1];
					index[10]	 = n[2];
					index[11]	 = n[3];
					index[12]	 = n[4];
					index[13]	 = n[5];
					index[14]	 = n[6];
					index[15]	 = n[7];
				  }
				}
			  }
			}
		  }
		}
	  }
	}
}
STIN void sortindex_fix32(int *index,
						  float *data,
						  int offset){
	_MM_ALIGN16 int n[32];
	__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
	sortindex_fix8(index,data,offset   );
	sortindex_fix8(index,data,offset+ 8);
	sortindex_fix8(index,data,offset+16);
	sortindex_fix8(index,data,offset+24);
	index+=offset;
	sortindex_fix16(n,index,data, 0);
	sortindex_fix16(n,index,data,16);
	XMM0	 = _mm_load_ss(data+n[0]);
	XMM4	 = _mm_load_ss(data+n[16]);
	XMM1	 = _mm_load_ss(data+n[1]);
	XMM5	 = _mm_load_ss(data+n[17]);
	XMM2	 = _mm_load_ss(data+n[2]);
	XMM6	 = _mm_load_ss(data+n[18]);
	XMM3	 = _mm_load_ss(data+n[3]);
	XMM7	 = _mm_load_ss(data+n[19]);
	if(!_mm_comilt_ss(XMM0, XMM4)){
	  index[0]	 = n[0];
	  if(!_mm_comilt_ss(XMM1, XMM4)){
		index[1]	 = n[1];
		if(!_mm_comilt_ss(XMM2, XMM4)){
		  index[2]	 = n[2];
		  if(!_mm_comilt_ss(XMM3, XMM4)){
			index[3]	 = n[3];
			XMM0	 = _mm_load_ss(data+n[4]);
			XMM1	 = _mm_load_ss(data+n[5]);
			XMM2	 = _mm_load_ss(data+n[6]);
			XMM3	 = _mm_load_ss(data+n[7]);
			if(!_mm_comilt_ss(XMM0, XMM4)){
			  index[4]	 = n[4];
			  if(!_mm_comilt_ss(XMM1, XMM4)){
				index[5]	 = n[5];
				if(!_mm_comilt_ss(XMM2, XMM4)){
				  index[6]	 = n[6];
				  if(!_mm_comilt_ss(XMM3, XMM4)){
					index[7]	 = n[7];
					XMM0	 = _mm_load_ss(data+n[8]);
					XMM1	 = _mm_load_ss(data+n[9]);
					XMM2	 = _mm_load_ss(data+n[10]);
					XMM3	 = _mm_load_ss(data+n[11]);
					if(!_mm_comilt_ss(XMM0, XMM4)){
					  index[8]	 = n[8];
					  if(!_mm_comilt_ss(XMM1, XMM4)){
						index[9]	 = n[9];
						if(!_mm_comilt_ss(XMM2, XMM4)){
						  index[10]	 = n[10];
						  if(!_mm_comilt_ss(XMM3, XMM4)){
							index[11]	 = n[11];
							XMM0	 = _mm_load_ss(data+n[12]);
							XMM1	 = _mm_load_ss(data+n[13]);
							XMM2	 = _mm_load_ss(data+n[14]);
							XMM3	 = _mm_load_ss(data+n[15]);
							if(!_mm_comilt_ss(XMM0, XMM4)){
							  index[12]	 = n[12];
							  if(!_mm_comilt_ss(XMM1, XMM4)){
								index[13]	 = n[13];
								if(!_mm_comilt_ss(XMM2, XMM4)){
								  index[14]	 = n[14];
								  if(!_mm_comilt_ss(XMM3, XMM4)){
									index[15]	 = n[15];
									index[16]	 = n[16];
									index[17]	 = n[17];
									index[18]	 = n[18];
									index[19]	 = n[19];
									index[20]	 = n[20];
									index[21]	 = n[21];
									index[22]	 = n[22];
									index[23]	 = n[23];
									index[24]	 = n[24];
									index[25]	 = n[25];
									index[26]	 = n[26];
									index[27]	 = n[27];
									index[28]	 = n[28];
									index[29]	 = n[29];
									index[30]	 = n[30];
									index[31]	 = n[31];
								  }else{
									index[15]	 = n[16];
SORT32_100F11:
									if(!_mm_comilt_ss(XMM3, XMM5)){
									  index[16]	 = n[15];
									  index[17]	 = n[17];
									  index[18]	 = n[18];
									  index[19]	 = n[19];
									  index[20]	 = n[20];
									  index[21]	 = n[21];
									  index[22]	 = n[22];
									  index[23]	 = n[23];
									  index[24]	 = n[24];
									  index[25]	 = n[25];
									  index[26]	 = n[26];
									  index[27]	 = n[27];
									  index[28]	 = n[28];
									  index[29]	 = n[29];
									  index[30]	 = n[30];
									  index[31]	 = n[31];
									}else{
									  index[16]	 = n[17];
SORT32_110F12:
									  if(!_mm_comilt_ss(XMM3, XMM6)){
										index[17]	 = n[15];
										index[18]	 = n[18];
										index[19]	 = n[19];
										index[20]	 = n[20];
										index[21]	 = n[21];
										index[22]	 = n[22];
										index[23]	 = n[23];
										index[24]	 = n[24];
										index[25]	 = n[25];
										index[26]	 = n[26];
										index[27]	 = n[27];
										index[28]	 = n[28];
										index[29]	 = n[29];
										index[30]	 = n[30];
										index[31]	 = n[31];
									  }else{
										index[17]	 = n[18];
SORT32_120F13:
										if(!_mm_comilt_ss(XMM3, XMM7)){
										  index[18]	 = n[15];
										  index[19]	 = n[19];
										  index[20]	 = n[20];
										  index[21]	 = n[21];
										  index[22]	 = n[22];
										  index[23]	 = n[23];
										  index[24]	 = n[24];
										  index[25]	 = n[25];
										  index[26]	 = n[26];
										  index[27]	 = n[27];
										  index[28]	 = n[28];
										  index[29]	 = n[29];
										  index[30]	 = n[30];
										  index[31]	 = n[31];
										}else{
										  index[18]	 = n[19];
										  XMM4	 = _mm_load_ss(data+n[20]);
										  XMM5	 = _mm_load_ss(data+n[21]);
										  XMM6	 = _mm_load_ss(data+n[22]);
										  XMM7	 = _mm_load_ss(data+n[23]);
SORT32_130F14:
										  if(!_mm_comilt_ss(XMM3, XMM4)){
											index[19]	 = n[15];
											index[20]	 = n[20];
											index[21]	 = n[21];
											index[22]	 = n[22];
											index[23]	 = n[23];
											index[24]	 = n[24];
											index[25]	 = n[25];
											index[26]	 = n[26];
											index[27]	 = n[27];
											index[28]	 = n[28];
											index[29]	 = n[29];
											index[30]	 = n[30];
											index[31]	 = n[31];
										  }else{
											index[19]	 = n[20];
SORT32_140F15:
											if(!_mm_comilt_ss(XMM3, XMM5)){
											  index[20]	 = n[15];
											  index[21]	 = n[21];
											  index[22]	 = n[22];
											  index[23]	 = n[23];
											  index[24]	 = n[24];
											  index[25]	 = n[25];
											  index[26]	 = n[26];
											  index[27]	 = n[27];
											  index[28]	 = n[28];
											  index[29]	 = n[29];
											  index[30]	 = n[30];
											  index[31]	 = n[31];
											}else{
											  index[20]	 = n[21];
SORT32_150F16:
											  if(!_mm_comilt_ss(XMM3, XMM6)){
												index[21]	 = n[15];
												index[22]	 = n[22];
												index[23]	 = n[23];
												index[24]	 = n[24];
												index[25]	 = n[25];
												index[26]	 = n[26];
												index[27]	 = n[27];
												index[28]	 = n[28];
												index[29]	 = n[29];
												index[30]	 = n[30];
												index[31]	 = n[31];
											  }else{
												index[21]	 = n[22];
SORT32_160F17:
												if(!_mm_comilt_ss(XMM3, XMM7)){
												  index[22]	 = n[15];
												  index[23]	 = n[23];
												  index[24]	 = n[24];
												  index[25]	 = n[25];
												  index[26]	 = n[26];
												  index[27]	 = n[27];
												  index[28]	 = n[28];
												  index[29]	 = n[29];
												  index[30]	 = n[30];
												  index[31]	 = n[31];
												}else{
												  index[22]	 = n[23];
												  XMM4	 = _mm_load_ss(data+n[24]);
												  XMM5	 = _mm_load_ss(data+n[25]);
												  XMM6	 = _mm_load_ss(data+n[26]);
												  XMM7	 = _mm_load_ss(data+n[27]);
SORT32_170F18:
												  if(!_mm_comilt_ss(XMM3, XMM4)){
													index[23]	 = n[15];
													index[24]	 = n[24];
													index[25]	 = n[25];
													index[26]	 = n[26];
													index[27]	 = n[27];
													index[28]	 = n[28];
													index[29]	 = n[29];
													index[30]	 = n[30];
													index[31]	 = n[31];
												  }else{
													index[23]	 = n[24];
SORT32_180F19:
													if(!_mm_comilt_ss(XMM3, XMM5)){
													  index[24]	 = n[15];
													  index[25]	 = n[25];
													  index[26]	 = n[26];
													  index[27]	 = n[27];
													  index[28]	 = n[28];
													  index[29]	 = n[29];
													  index[30]	 = n[30];
													  index[31]	 = n[31];
													}else{
													  index[24]	 = n[25];
SORT32_190F1A:
													  if(!_mm_comilt_ss(XMM3, XMM6)){
														index[25]	 = n[15];
														index[26]	 = n[26];
														index[27]	 = n[27];
														index[28]	 = n[28];
														index[29]	 = n[29];
														index[30]	 = n[30];
														index[31]	 = n[31];
													  }else{
														index[25]	 = n[26];
SORT32_1A0F1B:
														if(!_mm_comilt_ss(XMM3, XMM7)){
														  index[26]	 = n[15];
														  index[27]	 = n[27];
														  index[28]	 = n[28];
														  index[29]	 = n[29];
														  index[30]	 = n[30];
														  index[31]	 = n[31];
														}else{
														  index[26]	 = n[27];
														  XMM4	 = _mm_load_ss(data+n[28]);
														  XMM5	 = _mm_load_ss(data+n[29]);
														  XMM6	 = _mm_load_ss(data+n[30]);
														  XMM7	 = _mm_load_ss(data+n[31]);
SORT32_1B0F1C:
														  if(!_mm_comilt_ss(XMM3, XMM4)){
															index[27]	 = n[15];
															index[28]	 = n[28];
															index[29]	 = n[29];
															index[30]	 = n[30];
															index[31]	 = n[31];
														  }else{
															index[27]	 = n[28];
SORT32_1C0F1D:
															if(!_mm_comilt_ss(XMM3, XMM5)){
															  index[28]	 = n[15];
															  index[29]	 = n[29];
															  index[30]	 = n[30];
															  index[31]	 = n[31];
															}else{
															  index[28]	 = n[29];
SORT32_1D0F1E:
															  if(!_mm_comilt_ss(XMM3, XMM6)){
																index[29]	 = n[15];
																index[30]	 = n[30];
																index[31]	 = n[31];
															  }else{
																index[29]	 = n[30];
SORT32_1E0F1F:
																if(!_mm_comilt_ss(XMM3, XMM7)){
																  index[30]	 = n[15];
																  index[31]	 = n[31];
																}else{
																  index[30]	 = n[31];
																  index[31]	 = n[15];
																}
															  }
															}
														  }
														}
													  }
													}
												  }
												}
											  }
											}
										  }
										}
									  }
									}
								  }
								}else{
								  index[14]	 = n[16];
SORT32_0F0E11:
								  if(!_mm_comilt_ss(XMM2, XMM5)){
									index[15]	 = n[14];
									goto SORT32_100F11;
								  }else{
									index[15]	 = n[17];
SORT32_100E12:
									if(!_mm_comilt_ss(XMM2, XMM6)){
									  index[16]	 = n[14];
									  goto SORT32_110F12;
									}else{
									  index[16]	 = n[18];
SORT32_110E13:
									  if(!_mm_comilt_ss(XMM2, XMM7)){
										index[17]	 = n[14];
										goto SORT32_120F13;
									  }else{
										index[17]	 = n[19];
										XMM4	 = _mm_load_ss(data+n[20]);
										XMM5	 = _mm_load_ss(data+n[21]);
										XMM6	 = _mm_load_ss(data+n[22]);
										XMM7	 = _mm_load_ss(data+n[23]);
SORT32_120E14:
										if(!_mm_comilt_ss(XMM2, XMM4)){
										  index[18]	 = n[14];
										  goto SORT32_130F14;
										}else{
										  index[18]	 = n[20];
SORT32_130E15:
										  if(!_mm_comilt_ss(XMM2, XMM5)){
											index[19]	 = n[14];
											goto SORT32_140F15;
										  }else{
											index[19]	 = n[21];
SORT32_140E16:
											if(!_mm_comilt_ss(XMM2, XMM6)){
											  index[20]	 = n[14];
											  goto SORT32_150F16;
											}else{
											  index[20]	 = n[22];
SORT32_150E17:
											  if(!_mm_comilt_ss(XMM2, XMM7)){
												index[21]	 = n[14];
												goto SORT32_160F17;
											  }else{
												index[21]	 = n[23];
												XMM4	 = _mm_load_ss(data+n[24]);
												XMM5	 = _mm_load_ss(data+n[25]);
												XMM6	 = _mm_load_ss(data+n[26]);
												XMM7	 = _mm_load_ss(data+n[27]);
SORT32_160E18:
												if(!_mm_comilt_ss(XMM2, XMM4)){
												  index[22]	 = n[14];
												  goto SORT32_170F18;
												}else{
												  index[22]	 = n[24];
SORT32_170E19:
												  if(!_mm_comilt_ss(XMM2, XMM5)){
													index[23]	 = n[14];
													goto SORT32_180F19;
												  }else{
													index[23]	 = n[25];
SORT32_180E1A:
													if(!_mm_comilt_ss(XMM2, XMM6)){
													  index[24]	 = n[14];
													  goto SORT32_190F1A;
													}else{
													  index[24]	 = n[26];
SORT32_190E1B:
													  if(!_mm_comilt_ss(XMM2, XMM7)){
														index[25]	 = n[14];
														goto SORT32_1A0F1B;
													  }else{
														index[25]	 = n[27];
														XMM4	 = _mm_load_ss(data+n[28]);
														XMM5	 = _mm_load_ss(data+n[29]);
														XMM6	 = _mm_load_ss(data+n[30]);
														XMM7	 = _mm_load_ss(data+n[31]);
SORT32_1A0E1C:
														if(!_mm_comilt_ss(XMM2, XMM4)){
														  index[26]	 = n[14];
														  goto SORT32_1B0F1C;
														}else{
														  index[26]	 = n[28];
SORT32_1B0E1D:
														  if(!_mm_comilt_ss(XMM2, XMM5)){
															index[27]	 = n[14];
															goto SORT32_1C0F1D;
														  }else{
															index[27]	 = n[29];
SORT32_1C0E1E:
															if(!_mm_comilt_ss(XMM2, XMM6)){
															  index[28]	 = n[14];
															  goto SORT32_1D0F1E;
															}else{
															  index[28]	 = n[30];
SORT32_1D0E1F:
															  if(!_mm_comilt_ss(XMM2, XMM7)){
																index[29]	 = n[14];
																goto SORT32_1E0F1F;
															  }else{
																index[29]	 = n[31];
																index[30]	 = n[14];
																index[31]	 = n[15];
															  }
															}
														  }
														}
													  }
													}
												  }
												}
											  }
											}
										  }
										}
									  }
									}
								  }
								}
							  }else{
								index[13]	 = n[16];
SORT32_0E0D11:
								if(!_mm_comilt_ss(XMM1, XMM5)){
								  index[14]	 = n[13];
								  goto SORT32_0F0E11;
								}else{
								  index[14]	 = n[17];
SORT32_0F0D12:
								  if(!_mm_comilt_ss(XMM1, XMM6)){
									index[15]	 = n[13];
									goto SORT32_100E12;
								  }else{
									index[15]	 = n[18];
SORT32_100D13:
									if(!_mm_comilt_ss(XMM1, XMM7)){
									  index[16]	 = n[13];
									  goto SORT32_110E13;
									}else{
									  index[16]	 = n[19];
									  XMM4	 = _mm_load_ss(data+n[20]);
									  XMM5	 = _mm_load_ss(data+n[21]);
									  XMM6	 = _mm_load_ss(data+n[22]);
									  XMM7	 = _mm_load_ss(data+n[23]);
SORT32_110D14:
									  if(!_mm_comilt_ss(XMM1, XMM4)){
										index[17]	 = n[13];
										goto SORT32_120E14;
									  }else{
										index[17]	 = n[20];
SORT32_120D15:
										if(!_mm_comilt_ss(XMM1, XMM5)){
										  index[18]	 = n[13];
										  goto SORT32_130E15;
										}else{
										  index[18]	 = n[21];
SORT32_130D16:
										  if(!_mm_comilt_ss(XMM1, XMM6)){
											index[19]	 = n[13];
											goto SORT32_140E16;
										  }else{
											index[19]	 = n[22];
SORT32_140D17:
											if(!_mm_comilt_ss(XMM1, XMM7)){
											  index[20]	 = n[13];
											  goto SORT32_150E17;
											}else{
											  index[20]	 = n[23];
											  XMM4	 = _mm_load_ss(data+n[24]);
											  XMM5	 = _mm_load_ss(data+n[25]);
											  XMM6	 = _mm_load_ss(data+n[26]);
											  XMM7	 = _mm_load_ss(data+n[27]);
SORT32_150D18:
											  if(!_mm_comilt_ss(XMM1, XMM4)){
												index[21]	 = n[13];
												goto SORT32_160E18;
											  }else{
												index[21]	 = n[24];
SORT32_160D19:
												if(!_mm_comilt_ss(XMM1, XMM5)){
												  index[22]	 = n[13];
												  goto SORT32_170E19;
												}else{
												  index[22]	 = n[25];
SORT32_170D1A:
												  if(!_mm_comilt_ss(XMM1, XMM6)){
													index[23]	 = n[13];
													goto SORT32_180E1A;
												  }else{
													index[23]	 = n[26];
SORT32_180D1B:
													if(!_mm_comilt_ss(XMM1, XMM7)){
													  index[24]	 = n[13];
													  goto SORT32_190E1B;
													}else{
													  index[24]	 = n[27];
													  XMM4	 = _mm_load_ss(data+n[28]);
													  XMM5	 = _mm_load_ss(data+n[29]);
													  XMM6	 = _mm_load_ss(data+n[30]);
													  XMM7	 = _mm_load_ss(data+n[31]);
SORT32_190D1C:
													  if(!_mm_comilt_ss(XMM1, XMM4)){
														index[25]	 = n[13];
														goto SORT32_1A0E1C;
													  }else{
														index[25]	 = n[28];
SORT32_1A0D1D:
														if(!_mm_comilt_ss(XMM1, XMM5)){
														  index[26]	 = n[13];
														  goto SORT32_1B0E1D;
														}else{
														  index[26]	 = n[29];
SORT32_1B0D1E:
														  if(!_mm_comilt_ss(XMM1, XMM6)){
															index[27]	 = n[13];
															goto SORT32_1C0E1E;
														  }else{
															index[27]	 = n[30];
SORT32_1C0D1F:
															if(!_mm_comilt_ss(XMM1, XMM7)){
															  index[28]	 = n[13];
															  goto SORT32_1D0E1F;
															}else{
															  index[28]	 = n[31];
															  index[29]	 = n[13];
															  index[30]	 = n[14];
															  index[31]	 = n[15];
															}
														  }
														}
													  }
													}
												  }
												}
											  }
											}
										  }
										}
									  }
									}
								  }
								}
							  }
							}else{
							  index[12]	 = n[16];
SORT32_0D0C11:
							  if(!_mm_comilt_ss(XMM0, XMM5)){
								index[13]	 = n[12];
								goto SORT32_0E0D11;
							  }else{
								index[13]	 = n[17];
SORT32_0E0C12:
								if(!_mm_comilt_ss(XMM0, XMM6)){
								  index[14]	 = n[12];
								  goto SORT32_0F0D12;
								}else{
								  index[14]	 = n[18];
SORT32_0F0C13:
								  if(!_mm_comilt_ss(XMM0, XMM7)){
									index[15]	 = n[12];
									goto SORT32_100D13;
								  }else{
									index[15]	 = n[19];
									XMM4	 = _mm_load_ss(data+n[20]);
									XMM5	 = _mm_load_ss(data+n[21]);
									XMM6	 = _mm_load_ss(data+n[22]);
									XMM7	 = _mm_load_ss(data+n[23]);
SORT32_100C14:
									if(!_mm_comilt_ss(XMM0, XMM4)){
									  index[16]	 = n[12];
									  goto SORT32_110D14;
									}else{
									  index[16]	 = n[20];
SORT32_110C15:
									  if(!_mm_comilt_ss(XMM0, XMM5)){
										index[17]	 = n[12];
										goto SORT32_120D15;
									  }else{
										index[17]	 = n[21];
SORT32_120C16:
										if(!_mm_comilt_ss(XMM0, XMM6)){
										  index[18]	 = n[12];
										  goto SORT32_130D16;
										}else{
										  index[18]	 = n[22];
SORT32_130C17:
										  if(!_mm_comilt_ss(XMM0, XMM7)){
											index[19]	 = n[12];
											goto SORT32_140D17;
										  }else{
											index[19]	 = n[23];
											XMM4	 = _mm_load_ss(data+n[24]);
											XMM5	 = _mm_load_ss(data+n[25]);
											XMM6	 = _mm_load_ss(data+n[26]);
											XMM7	 = _mm_load_ss(data+n[27]);
SORT32_140C18:
											if(!_mm_comilt_ss(XMM0, XMM4)){
											  index[20]	 = n[12];
											  goto SORT32_150D18;
											}else{
											  index[20]	 = n[24];
SORT32_150C19:
											  if(!_mm_comilt_ss(XMM0, XMM5)){
												index[21]	 = n[12];
												goto SORT32_160D19;
											  }else{
												index[21]	 = n[25];
SORT32_160C1A:
												if(!_mm_comilt_ss(XMM0, XMM6)){
												  index[22]	 = n[12];
												  goto SORT32_170D1A;
												}else{
												  index[22]	 = n[26];
SORT32_170C1B:
												  if(!_mm_comilt_ss(XMM0, XMM7)){
													index[23]	 = n[12];
													goto SORT32_180D1B;
												  }else{
													index[23]	 = n[27];
													XMM4	 = _mm_load_ss(data+n[28]);
													XMM5	 = _mm_load_ss(data+n[29]);
													XMM6	 = _mm_load_ss(data+n[30]);
													XMM7	 = _mm_load_ss(data+n[31]);
SORT32_180C1C:
													if(!_mm_comilt_ss(XMM0, XMM4)){
													  index[24]	 = n[12];
													  goto SORT32_190D1C;
													}else{
													  index[24]	 = n[28];
SORT32_190C1D:
													  if(!_mm_comilt_ss(XMM0, XMM5)){
														index[25]	 = n[12];
														goto SORT32_1A0D1D;
													  }else{
														index[25]	 = n[29];
SORT32_1A0C1E:
														if(!_mm_comilt_ss(XMM0, XMM6)){
														  index[26]	 = n[12];
														  goto SORT32_1B0D1E;
														}else{
														  index[26]	 = n[30];
SORT32_1B0C1F:
														  if(!_mm_comilt_ss(XMM0, XMM7)){
															index[27]	 = n[12];
															goto SORT32_1C0D1F;
														  }else{
															index[27]	 = n[31];
															index[28]	 = n[12];
															index[29]	 = n[13];
															index[30]	 = n[14];
															index[31]	 = n[15];
														  }
														}
													  }
													}
												  }
												}
											  }
											}
										  }
										}
									  }
									}
								  }
								}
							  }
							}
						  }else{
							index[11]	 = n[16];
SORT32_0C0B11:
							if(!_mm_comilt_ss(XMM3, XMM5)){
							  index[12]	 = n[11];
							  XMM0	 = _mm_load_ss(data+n[12]);
							  XMM1	 = _mm_load_ss(data+n[13]);
							  XMM2	 = _mm_load_ss(data+n[14]);
							  XMM3	 = _mm_load_ss(data+n[15]);
							  goto SORT32_0D0C11;
							}else{
							  index[12]	 = n[17];
SORT32_0D0B12:
							  if(!_mm_comilt_ss(XMM3, XMM6)){
								index[13]	 = n[11];
								XMM0	 = _mm_load_ss(data+n[12]);
								XMM1	 = _mm_load_ss(data+n[13]);
								XMM2	 = _mm_load_ss(data+n[14]);
								XMM3	 = _mm_load_ss(data+n[15]);
								goto SORT32_0E0C12;
							  }else{
								index[13]	 = n[18];
SORT32_0E0B13:
								if(!_mm_comilt_ss(XMM3, XMM7)){
								  index[14]	 = n[11];
								  XMM0	 = _mm_load_ss(data+n[12]);
								  XMM1	 = _mm_load_ss(data+n[13]);
								  XMM2	 = _mm_load_ss(data+n[14]);
								  XMM3	 = _mm_load_ss(data+n[15]);
								  goto SORT32_0F0C13;
								}else{
								  index[14]	 = n[19];
								  XMM4	 = _mm_load_ss(data+n[20]);
								  XMM5	 = _mm_load_ss(data+n[21]);
								  XMM6	 = _mm_load_ss(data+n[22]);
								  XMM7	 = _mm_load_ss(data+n[23]);
SORT32_0F0B14:
								  if(!_mm_comilt_ss(XMM3, XMM4)){
									index[15]	 = n[11];
									XMM0	 = _mm_load_ss(data+n[12]);
									XMM1	 = _mm_load_ss(data+n[13]);
									XMM2	 = _mm_load_ss(data+n[14]);
									XMM3	 = _mm_load_ss(data+n[15]);
									goto SORT32_100C14;
								  }else{
									index[15]	 = n[20];
SORT32_100B15:
									if(!_mm_comilt_ss(XMM3, XMM5)){
									  index[16]	 = n[11];
									  XMM0	 = _mm_load_ss(data+n[12]);
									  XMM1	 = _mm_load_ss(data+n[13]);
									  XMM2	 = _mm_load_ss(data+n[14]);
									  XMM3	 = _mm_load_ss(data+n[15]);
									  goto SORT32_110C15;
									}else{
									  index[16]	 = n[21];
SORT32_110B16:
									  if(!_mm_comilt_ss(XMM3, XMM6)){
										index[17]	 = n[11];
										XMM0	 = _mm_load_ss(data+n[12]);
										XMM1	 = _mm_load_ss(data+n[13]);
										XMM2	 = _mm_load_ss(data+n[14]);
										XMM3	 = _mm_load_ss(data+n[15]);
										goto SORT32_120C16;
									  }else{
										index[17]	 = n[22];
SORT32_120B17:
										if(!_mm_comilt_ss(XMM3, XMM7)){
										  index[18]	 = n[11];
										  XMM0	 = _mm_load_ss(data+n[12]);
										  XMM1	 = _mm_load_ss(data+n[13]);
										  XMM2	 = _mm_load_ss(data+n[14]);
										  XMM3	 = _mm_load_ss(data+n[15]);
										  goto SORT32_130C17;
										}else{
										  index[18]	 = n[23];
										  XMM4	 = _mm_load_ss(data+n[24]);
										  XMM5	 = _mm_load_ss(data+n[25]);
										  XMM6	 = _mm_load_ss(data+n[26]);
										  XMM7	 = _mm_load_ss(data+n[27]);
SORT32_130B18:
										  if(!_mm_comilt_ss(XMM3, XMM4)){
											index[19]	 = n[11];
											XMM0	 = _mm_load_ss(data+n[12]);
											XMM1	 = _mm_load_ss(data+n[13]);
											XMM2	 = _mm_load_ss(data+n[14]);
											XMM3	 = _mm_load_ss(data+n[15]);
											goto SORT32_140C18;
										  }else{
											index[19]	 = n[24];
SORT32_140B19:
											if(!_mm_comilt_ss(XMM3, XMM5)){
											  index[20]	 = n[11];
											  XMM0	 = _mm_load_ss(data+n[12]);
											  XMM1	 = _mm_load_ss(data+n[13]);
											  XMM2	 = _mm_load_ss(data+n[14]);
											  XMM3	 = _mm_load_ss(data+n[15]);
											  goto SORT32_150C19;
											}else{
											  index[20]	 = n[25];
SORT32_150B1A:
											  if(!_mm_comilt_ss(XMM3, XMM6)){
												index[21]	 = n[11];
												XMM0	 = _mm_load_ss(data+n[12]);
												XMM1	 = _mm_load_ss(data+n[13]);
												XMM2	 = _mm_load_ss(data+n[14]);
												XMM3	 = _mm_load_ss(data+n[15]);
												goto SORT32_160C1A;
											  }else{
												index[21]	 = n[26];
SORT32_160B1B:
												if(!_mm_comilt_ss(XMM3, XMM7)){
												  index[22]	 = n[11];
												  XMM0	 = _mm_load_ss(data+n[12]);
												  XMM1	 = _mm_load_ss(data+n[13]);
												  XMM2	 = _mm_load_ss(data+n[14]);
												  XMM3	 = _mm_load_ss(data+n[15]);
												  goto SORT32_170C1B;
												}else{
												  index[22]	 = n[27];
												  XMM4	 = _mm_load_ss(data+n[28]);
												  XMM5	 = _mm_load_ss(data+n[29]);
												  XMM6	 = _mm_load_ss(data+n[30]);
												  XMM7	 = _mm_load_ss(data+n[31]);
SORT32_170B1C:
												  if(!_mm_comilt_ss(XMM3, XMM4)){
													index[23]	 = n[11];
													XMM0	 = _mm_load_ss(data+n[12]);
													XMM1	 = _mm_load_ss(data+n[13]);
													XMM2	 = _mm_load_ss(data+n[14]);
													XMM3	 = _mm_load_ss(data+n[15]);
													goto SORT32_180C1C;
												  }else{
													index[23]	 = n[28];
SORT32_180B1D:
													if(!_mm_comilt_ss(XMM3, XMM5)){
													  index[24]	 = n[11];
													  XMM0	 = _mm_load_ss(data+n[12]);
													  XMM1	 = _mm_load_ss(data+n[13]);
													  XMM2	 = _mm_load_ss(data+n[14]);
													  XMM3	 = _mm_load_ss(data+n[15]);
													  goto SORT32_190C1D;
													}else{
													  index[24]	 = n[29];
SORT32_190B1E:
													  if(!_mm_comilt_ss(XMM3, XMM6)){
														index[25]	 = n[11];
														XMM0	 = _mm_load_ss(data+n[12]);
														XMM1	 = _mm_load_ss(data+n[13]);
														XMM2	 = _mm_load_ss(data+n[14]);
														XMM3	 = _mm_load_ss(data+n[15]);
														goto SORT32_1A0C1E;
													  }else{
														index[25]	 = n[30];
SORT32_1A0B1F:
														if(!_mm_comilt_ss(XMM3, XMM7)){
														  index[26]	 = n[11];
														  XMM0	 = _mm_load_ss(data+n[12]);
														  XMM1	 = _mm_load_ss(data+n[13]);
														  XMM2	 = _mm_load_ss(data+n[14]);
														  XMM3	 = _mm_load_ss(data+n[15]);
														  goto SORT32_1B0C1F;
														}else{
														  index[26]	 = n[31];
														  index[27]	 = n[11];
														  index[28]	 = n[12];
														  index[29]	 = n[13];
														  index[30]	 = n[14];
														  index[31]	 = n[15];
														}
													  }
													}
												  }
												}
											  }
											}
										  }
										}
									  }
									}
								  }
								}
							  }
							}
						  }
						}else{
						  index[10]	 = n[16];
SORT32_0B0A11:
						  if(!_mm_comilt_ss(XMM2, XMM5)){
							index[11]	 = n[10];
							goto SORT32_0C0B11;
						  }else{
							index[11]	 = n[17];
SORT32_0C0A12:
							if(!_mm_comilt_ss(XMM2, XMM6)){
							  index[12]	 = n[10];
							  goto SORT32_0D0B12;
							}else{
							  index[12]	 = n[18];
SORT32_0D0A13:
							  if(!_mm_comilt_ss(XMM2, XMM7)){
								index[13]	 = n[10];
								goto SORT32_0E0B13;
							  }else{
								index[13]	 = n[19];
								XMM4	 = _mm_load_ss(data+n[20]);
								XMM5	 = _mm_load_ss(data+n[21]);
								XMM6	 = _mm_load_ss(data+n[22]);
								XMM7	 = _mm_load_ss(data+n[23]);
SORT32_0E0A14:
								if(!_mm_comilt_ss(XMM2, XMM4)){
								  index[14]	 = n[10];
								  goto SORT32_0F0B14;
								}else{
								  index[14]	 = n[20];
SORT32_0F0A15:
								  if(!_mm_comilt_ss(XMM2, XMM5)){
									index[15]	 = n[10];
									goto SORT32_100B15;
								  }else{
									index[15]	 = n[21];
SORT32_100A16:
									if(!_mm_comilt_ss(XMM2, XMM6)){
									  index[16]	 = n[10];
									  goto SORT32_110B16;
									}else{
									  index[16]	 = n[22];
SORT32_110A17:
									  if(!_mm_comilt_ss(XMM2, XMM7)){
										index[17]	 = n[10];
										goto SORT32_120B17;
									  }else{
										index[17]	 = n[23];
										XMM4	 = _mm_load_ss(data+n[24]);
										XMM5	 = _mm_load_ss(data+n[25]);
										XMM6	 = _mm_load_ss(data+n[26]);
										XMM7	 = _mm_load_ss(data+n[27]);
SORT32_120A18:
										if(!_mm_comilt_ss(XMM2, XMM4)){
										  index[18]	 = n[10];
										  goto SORT32_130B18;
										}else{
										  index[18]	 = n[24];
SORT32_130A19:
										  if(!_mm_comilt_ss(XMM2, XMM5)){
											index[19]	 = n[10];
											goto SORT32_140B19;
										  }else{
											index[19]	 = n[25];
SORT32_140A1A:
											if(!_mm_comilt_ss(XMM2, XMM6)){
											  index[20]	 = n[10];
											  goto SORT32_150B1A;
											}else{
											  index[20]	 = n[26];
SORT32_150A1B:
											  if(!_mm_comilt_ss(XMM2, XMM7)){
												index[21]	 = n[10];
												goto SORT32_160B1B;
											  }else{
												index[21]	 = n[27];
												XMM4	 = _mm_load_ss(data+n[28]);
												XMM5	 = _mm_load_ss(data+n[29]);
												XMM6	 = _mm_load_ss(data+n[30]);
												XMM7	 = _mm_load_ss(data+n[31]);
SORT32_160A1C:
												if(!_mm_comilt_ss(XMM2, XMM4)){
												  index[22]	 = n[10];
												  goto SORT32_170B1C;
												}else{
												  index[22]	 = n[28];
SORT32_170A1D:
												  if(!_mm_comilt_ss(XMM2, XMM5)){
													index[23]	 = n[10];
													goto SORT32_180B1D;
												  }else{
													index[23]	 = n[29];
SORT32_180A1E:
													if(!_mm_comilt_ss(XMM2, XMM6)){
													  index[24]	 = n[10];
													  goto SORT32_190B1E;
													}else{
													  index[24]	 = n[30];
SORT32_190A1F:
													  if(!_mm_comilt_ss(XMM2, XMM7)){
														index[25]	 = n[10];
														goto SORT32_1A0B1F;
													  }else{
														index[25]	 = n[31];
														index[26]	 = n[10];
														index[27]	 = n[11];
														index[28]	 = n[12];
														index[29]	 = n[13];
														index[30]	 = n[14];
														index[31]	 = n[15];
													  }
													}
												  }
												}
											  }
											}
										  }
										}
									  }
									}
								  }
								}
							  }
							}
						  }
						}
					  }else{
						index[9]	 = n[16];
SORT32_0A0911:
						if(!_mm_comilt_ss(XMM1, XMM5)){
						  index[10]	 = n[9];
						  goto SORT32_0B0A11;
						}else{
						  index[10]	 = n[17];
SORT32_0B0912:
						  if(!_mm_comilt_ss(XMM1, XMM6)){
							index[11]	 = n[9];
							goto SORT32_0C0A12;
						  }else{
							index[11]	 = n[18];
SORT32_0C0913:
							if(!_mm_comilt_ss(XMM1, XMM7)){
							  index[12]	 = n[9];
							  goto SORT32_0D0A13;
							}else{
							  index[12]	 = n[19];
							  XMM4	 = _mm_load_ss(data+n[20]);
							  XMM5	 = _mm_load_ss(data+n[21]);
							  XMM6	 = _mm_load_ss(data+n[22]);
							  XMM7	 = _mm_load_ss(data+n[23]);
SORT32_0D0914:
							  if(!_mm_comilt_ss(XMM1, XMM4)){
								index[13]	 = n[9];
								goto SORT32_0E0A14;
							  }else{
								index[13]	 = n[20];
SORT32_0E0915:
								if(!_mm_comilt_ss(XMM1, XMM5)){
								  index[14]	 = n[9];
								  goto SORT32_0F0A15;
								}else{
								  index[14]	 = n[21];
SORT32_0F0916:
								  if(!_mm_comilt_ss(XMM1, XMM6)){
									index[15]	 = n[9];
									goto SORT32_100A16;
								  }else{
									index[15]	 = n[22];
SORT32_100917:
									if(!_mm_comilt_ss(XMM1, XMM7)){
									  index[16]	 = n[9];
									  goto SORT32_110A17;
									}else{
									  index[16]	 = n[23];
									  XMM4	 = _mm_load_ss(data+n[24]);
									  XMM5	 = _mm_load_ss(data+n[25]);
									  XMM6	 = _mm_load_ss(data+n[26]);
									  XMM7	 = _mm_load_ss(data+n[27]);
SORT32_110918:
									  if(!_mm_comilt_ss(XMM1, XMM4)){
										index[17]	 = n[9];
										goto SORT32_120A18;
									  }else{
										index[17]	 = n[24];
SORT32_120919:
										if(!_mm_comilt_ss(XMM1, XMM5)){
										  index[18]	 = n[9];
										  goto SORT32_130A19;
										}else{
										  index[18]	 = n[25];
SORT32_13091A:
										  if(!_mm_comilt_ss(XMM1, XMM6)){
											index[19]	 = n[9];
											goto SORT32_140A1A;
										  }else{
											index[19]	 = n[26];
SORT32_14091B:
											if(!_mm_comilt_ss(XMM1, XMM7)){
											  index[20]	 = n[9];
											  goto SORT32_150A1B;
											}else{
											  index[20]	 = n[27];
											  XMM4	 = _mm_load_ss(data+n[28]);
											  XMM5	 = _mm_load_ss(data+n[29]);
											  XMM6	 = _mm_load_ss(data+n[30]);
											  XMM7	 = _mm_load_ss(data+n[31]);
SORT32_15091C:
											  if(!_mm_comilt_ss(XMM1, XMM4)){
												index[21]	 = n[9];
												goto SORT32_160A1C;
											  }else{
												index[21]	 = n[28];
SORT32_16091D:
												if(!_mm_comilt_ss(XMM1, XMM5)){
												  index[22]	 = n[9];
												  goto SORT32_170A1D;
												}else{
												  index[22]	 = n[29];
SORT32_17091E:
												  if(!_mm_comilt_ss(XMM1, XMM6)){
													index[23]	 = n[9];
													goto SORT32_180A1E;
												  }else{
													index[23]	 = n[30];
SORT32_18091F:
													if(!_mm_comilt_ss(XMM1, XMM7)){
													  index[24]	 = n[9];
													  goto SORT32_190A1F;
													}else{
													  index[24]	 = n[31];
													  index[25]	 = n[9];
													  index[26]	 = n[10];
													  index[27]	 = n[11];
													  index[28]	 = n[12];
													  index[29]	 = n[13];
													  index[30]	 = n[14];
													  index[31]	 = n[15];
													}
												  }
												}
											  }
											}
										  }
										}
									  }
									}
								  }
								}
							  }
							}
						  }
						}
					  }
					}else{
					  index[8]	 = n[16];
SORT32_090811:
					  if(!_mm_comilt_ss(XMM0, XMM5)){
						index[9]	 = n[8];
						goto SORT32_0A0911;
					  }else{
						index[9]	 = n[17];
SORT32_0A0812:
						if(!_mm_comilt_ss(XMM0, XMM6)){
						  index[10]	 = n[8];
						  goto SORT32_0B0912;
						}else{
						  index[10]	 = n[18];
SORT32_0B0813:
						  if(!_mm_comilt_ss(XMM0, XMM7)){
							index[11]	 = n[8];
							goto SORT32_0C0913;
						  }else{
							index[11]	 = n[19];
							XMM4	 = _mm_load_ss(data+n[20]);
							XMM5	 = _mm_load_ss(data+n[21]);
							XMM6	 = _mm_load_ss(data+n[22]);
							XMM7	 = _mm_load_ss(data+n[23]);
SORT32_0C0814:
							if(!_mm_comilt_ss(XMM0, XMM4)){
							  index[12]	 = n[8];
							  goto SORT32_0D0914;
							}else{
							  index[12]	 = n[20];
SORT32_0D0815:
							  if(!_mm_comilt_ss(XMM0, XMM5)){
								index[13]	 = n[8];
								goto SORT32_0E0915;
							  }else{
								index[13]	 = n[21];
SORT32_0E0816:
								if(!_mm_comilt_ss(XMM0, XMM6)){
								  index[14]	 = n[8];
								  goto SORT32_0F0916;
								}else{
								  index[14]	 = n[22];
SORT32_0F0817:
								  if(!_mm_comilt_ss(XMM0, XMM7)){
									index[15]	 = n[8];
									goto SORT32_100917;
								  }else{
									index[15]	 = n[23];
									XMM4	 = _mm_load_ss(data+n[24]);
									XMM5	 = _mm_load_ss(data+n[25]);
									XMM6	 = _mm_load_ss(data+n[26]);
									XMM7	 = _mm_load_ss(data+n[27]);
SORT32_100818:
									if(!_mm_comilt_ss(XMM0, XMM4)){
									  index[16]	 = n[8];
									  goto SORT32_110918;
									}else{
									  index[16]	 = n[24];
SORT32_110819:
									  if(!_mm_comilt_ss(XMM0, XMM5)){
										index[17]	 = n[8];
										goto SORT32_120919;
									  }else{
										index[17]	 = n[25];
SORT32_12081A:
										if(!_mm_comilt_ss(XMM0, XMM6)){
										  index[18]	 = n[8];
										  goto SORT32_13091A;
										}else{
										  index[18]	 = n[26];
SORT32_13081B:
										  if(!_mm_comilt_ss(XMM0, XMM7)){
											index[19]	 = n[8];
											goto SORT32_14091B;
										  }else{
											index[19]	 = n[27];
											XMM4	 = _mm_load_ss(data+n[28]);
											XMM5	 = _mm_load_ss(data+n[29]);
											XMM6	 = _mm_load_ss(data+n[30]);
											XMM7	 = _mm_load_ss(data+n[31]);
SORT32_14081C:
											if(!_mm_comilt_ss(XMM0, XMM4)){
											  index[20]	 = n[8];
											  goto SORT32_15091C;
											}else{
											  index[20]	 = n[28];
SORT32_15081D:
											  if(!_mm_comilt_ss(XMM0, XMM5)){
												index[21]	 = n[8];
												goto SORT32_16091D;
											  }else{
												index[21]	 = n[29];
SORT32_16081E:
												if(!_mm_comilt_ss(XMM0, XMM6)){
												  index[22]	 = n[8];
												  goto SORT32_17091E;
												}else{
												  index[22]	 = n[30];
SORT32_17081F:
												  if(!_mm_comilt_ss(XMM0, XMM7)){
													index[23]	 = n[8];
													goto SORT32_18091F;
												  }else{
													index[23]	 = n[31];
													index[24]	 = n[8];
													index[25]	 = n[9];
													index[26]	 = n[10];
													index[27]	 = n[11];
													index[28]	 = n[12];
													index[29]	 = n[13];
													index[30]	 = n[14];
													index[31]	 = n[15];
												  }
												}
											  }
											}
										  }
										}
									  }
									}
								  }
								}
							  }
							}
						  }
						}
					  }
					}
				  }else{
					index[7]	 = n[16];
SORT32_080711:
					if(!_mm_comilt_ss(XMM3, XMM5)){
					  index[8]	 = n[7];
					  XMM0	 = _mm_load_ss(data+n[8]);
					  XMM1	 = _mm_load_ss(data+n[9]);
					  XMM2	 = _mm_load_ss(data+n[10]);
					  XMM3	 = _mm_load_ss(data+n[11]);
					  goto SORT32_090811;
					}else{
					  index[8]	 = n[17];
SORT32_090712:
					  if(!_mm_comilt_ss(XMM3, XMM6)){
						index[9]	 = n[7];
						XMM0	 = _mm_load_ss(data+n[8]);
						XMM1	 = _mm_load_ss(data+n[9]);
						XMM2	 = _mm_load_ss(data+n[10]);
						XMM3	 = _mm_load_ss(data+n[11]);
						goto SORT32_0A0812;
					  }else{
						index[9]	 = n[18];
SORT32_0A0713:
						if(!_mm_comilt_ss(XMM3, XMM7)){
						  index[10]	 = n[7];
						  XMM0	 = _mm_load_ss(data+n[8]);
						  XMM1	 = _mm_load_ss(data+n[9]);
						  XMM2	 = _mm_load_ss(data+n[10]);
						  XMM3	 = _mm_load_ss(data+n[11]);
						  goto SORT32_0B0813;
						}else{
						  index[10]	 = n[19];
						  XMM4	 = _mm_load_ss(data+n[20]);
						  XMM5	 = _mm_load_ss(data+n[21]);
						  XMM6	 = _mm_load_ss(data+n[22]);
						  XMM7	 = _mm_load_ss(data+n[23]);
SORT32_0B0714:
						  if(!_mm_comilt_ss(XMM3, XMM4)){
							index[11]	 = n[7];
							XMM0	 = _mm_load_ss(data+n[8]);
							XMM1	 = _mm_load_ss(data+n[9]);
							XMM2	 = _mm_load_ss(data+n[10]);
							XMM3	 = _mm_load_ss(data+n[11]);
							goto SORT32_0C0814;
						  }else{
							index[11]	 = n[20];
SORT32_0C0715:
							if(!_mm_comilt_ss(XMM3, XMM5)){
							  index[12]	 = n[7];
							  XMM0	 = _mm_load_ss(data+n[8]);
							  XMM1	 = _mm_load_ss(data+n[9]);
							  XMM2	 = _mm_load_ss(data+n[10]);
							  XMM3	 = _mm_load_ss(data+n[11]);
							  goto SORT32_0D0815;
							}else{
							  index[12]	 = n[21];
SORT32_0D0716:
							  if(!_mm_comilt_ss(XMM3, XMM6)){
								index[13]	 = n[7];
								XMM0	 = _mm_load_ss(data+n[8]);
								XMM1	 = _mm_load_ss(data+n[9]);
								XMM2	 = _mm_load_ss(data+n[10]);
								XMM3	 = _mm_load_ss(data+n[11]);
								goto SORT32_0E0816;
							  }else{
								index[13]	 = n[22];
SORT32_0E0717:
								if(!_mm_comilt_ss(XMM3, XMM7)){
								  index[14]	 = n[7];
								  XMM0	 = _mm_load_ss(data+n[8]);
								  XMM1	 = _mm_load_ss(data+n[9]);
								  XMM2	 = _mm_load_ss(data+n[10]);
								  XMM3	 = _mm_load_ss(data+n[11]);
								  goto SORT32_0F0817;
								}else{
								  index[14]	 = n[23];
								  XMM4	 = _mm_load_ss(data+n[24]);
								  XMM5	 = _mm_load_ss(data+n[25]);
								  XMM6	 = _mm_load_ss(data+n[26]);
								  XMM7	 = _mm_load_ss(data+n[27]);
SORT32_0F0718:
								  if(!_mm_comilt_ss(XMM3, XMM4)){
									index[15]	 = n[7];
									XMM0	 = _mm_load_ss(data+n[8]);
									XMM1	 = _mm_load_ss(data+n[9]);
									XMM2	 = _mm_load_ss(data+n[10]);
									XMM3	 = _mm_load_ss(data+n[11]);
									goto SORT32_100818;
								  }else{
									index[15]	 = n[24];
SORT32_100719:
									if(!_mm_comilt_ss(XMM3, XMM5)){
									  index[16]	 = n[7];
									  XMM0	 = _mm_load_ss(data+n[8]);
									  XMM1	 = _mm_load_ss(data+n[9]);
									  XMM2	 = _mm_load_ss(data+n[10]);
									  XMM3	 = _mm_load_ss(data+n[11]);
									  goto SORT32_110819;
									}else{
									  index[16]	 = n[25];
SORT32_11071A:
									  if(!_mm_comilt_ss(XMM3, XMM6)){
										index[17]	 = n[7];
										XMM0	 = _mm_load_ss(data+n[8]);
										XMM1	 = _mm_load_ss(data+n[9]);
										XMM2	 = _mm_load_ss(data+n[10]);
										XMM3	 = _mm_load_ss(data+n[11]);
										goto SORT32_12081A;
									  }else{
										index[17]	 = n[26];
SORT32_12071B:
										if(!_mm_comilt_ss(XMM3, XMM7)){
										  index[18]	 = n[7];
										  XMM0	 = _mm_load_ss(data+n[8]);
										  XMM1	 = _mm_load_ss(data+n[9]);
										  XMM2	 = _mm_load_ss(data+n[10]);
										  XMM3	 = _mm_load_ss(data+n[11]);
										  goto SORT32_13081B;
										}else{
										  index[18]	 = n[27];
										  XMM4	 = _mm_load_ss(data+n[28]);
										  XMM5	 = _mm_load_ss(data+n[29]);
										  XMM6	 = _mm_load_ss(data+n[30]);
										  XMM7	 = _mm_load_ss(data+n[31]);
SORT32_13071C:
										  if(!_mm_comilt_ss(XMM3, XMM4)){
											index[19]	 = n[7];
											XMM0	 = _mm_load_ss(data+n[8]);
											XMM1	 = _mm_load_ss(data+n[9]);
											XMM2	 = _mm_load_ss(data+n[10]);
											XMM3	 = _mm_load_ss(data+n[11]);
											goto SORT32_14081C;
										  }else{
											index[19]	 = n[28];
SORT32_14071D:
											if(!_mm_comilt_ss(XMM3, XMM5)){
											  index[20]	 = n[7];
											  XMM0	 = _mm_load_ss(data+n[8]);
											  XMM1	 = _mm_load_ss(data+n[9]);
											  XMM2	 = _mm_load_ss(data+n[10]);
											  XMM3	 = _mm_load_ss(data+n[11]);
											  goto SORT32_15081D;
											}else{
											  index[20]	 = n[29];
SORT32_15071E:
											  if(!_mm_comilt_ss(XMM3, XMM6)){
												index[21]	 = n[7];
												XMM0	 = _mm_load_ss(data+n[8]);
												XMM1	 = _mm_load_ss(data+n[9]);
												XMM2	 = _mm_load_ss(data+n[10]);
												XMM3	 = _mm_load_ss(data+n[11]);
												goto SORT32_16081E;
											  }else{
												index[21]	 = n[30];
SORT32_16071F:
												if(!_mm_comilt_ss(XMM3, XMM7)){
												  index[22]	 = n[7];
												  XMM0	 = _mm_load_ss(data+n[8]);
												  XMM1	 = _mm_load_ss(data+n[9]);
												  XMM2	 = _mm_load_ss(data+n[10]);
												  XMM3	 = _mm_load_ss(data+n[11]);
												  goto SORT32_17081F;
												}else{
												  index[22]	 = n[31];
												  index[23]	 = n[7];
												  index[24]	 = n[8];
												  index[25]	 = n[9];
												  index[26]	 = n[10];
												  index[27]	 = n[11];
												  index[28]	 = n[12];
												  index[29]	 = n[13];
												  index[30]	 = n[14];
												  index[31]	 = n[15];
												}
											  }
											}
										  }
										}
									  }
									}
								  }
								}
							  }
							}
						  }
						}
					  }
					}
				  }
				}else{
				  index[6]	 = n[16];
SORT32_070611:
				  if(!_mm_comilt_ss(XMM2, XMM5)){
					index[7]	 = n[6];
					goto SORT32_080711;
				  }else{
					index[7]	 = n[17];
SORT32_080612:
					if(!_mm_comilt_ss(XMM2, XMM6)){
					  index[8]	 = n[6];
					  goto SORT32_090712;
					}else{
					  index[8]	 = n[18];
SORT32_090613:
					  if(!_mm_comilt_ss(XMM2, XMM7)){
						index[9]	 = n[6];
						goto SORT32_0A0713;
					  }else{
						index[9]	 = n[19];
						XMM4	 = _mm_load_ss(data+n[20]);
						XMM5	 = _mm_load_ss(data+n[21]);
						XMM6	 = _mm_load_ss(data+n[22]);
						XMM7	 = _mm_load_ss(data+n[23]);
SORT32_0A0614:
						if(!_mm_comilt_ss(XMM2, XMM4)){
						  index[10]	 = n[6];
						  goto SORT32_0B0714;
						}else{
						  index[10]	 = n[20];
SORT32_0B0615:
						  if(!_mm_comilt_ss(XMM2, XMM5)){
							index[11]	 = n[6];
							goto SORT32_0C0715;
						  }else{
							index[11]	 = n[21];
SORT32_0C0616:
							if(!_mm_comilt_ss(XMM2, XMM6)){
							  index[12]	 = n[6];
							  goto SORT32_0D0716;
							}else{
							  index[12]	 = n[22];
SORT32_0D0617:
							  if(!_mm_comilt_ss(XMM2, XMM7)){
								index[13]	 = n[6];
								goto SORT32_0E0717;
							  }else{
								index[13]	 = n[23];
								XMM4	 = _mm_load_ss(data+n[24]);
								XMM5	 = _mm_load_ss(data+n[25]);
								XMM6	 = _mm_load_ss(data+n[26]);
								XMM7	 = _mm_load_ss(data+n[27]);
SORT32_0E0618:
								if(!_mm_comilt_ss(XMM2, XMM4)){
								  index[14]	 = n[6];
								  goto SORT32_0F0718;
								}else{
								  index[14]	 = n[24];
SORT32_0F0619:
								  if(!_mm_comilt_ss(XMM2, XMM5)){
									index[15]	 = n[6];
									goto SORT32_100719;
								  }else{
									index[15]	 = n[25];
SORT32_10061A:
									if(!_mm_comilt_ss(XMM2, XMM6)){
									  index[16]	 = n[6];
									  goto SORT32_11071A;
									}else{
									  index[16]	 = n[26];
SORT32_11061B:
									  if(!_mm_comilt_ss(XMM2, XMM7)){
										index[17]	 = n[6];
										goto SORT32_12071B;
									  }else{
										index[17]	 = n[27];
										XMM4	 = _mm_load_ss(data+n[28]);
										XMM5	 = _mm_load_ss(data+n[29]);
										XMM6	 = _mm_load_ss(data+n[30]);
										XMM7	 = _mm_load_ss(data+n[31]);
SORT32_12061C:
										if(!_mm_comilt_ss(XMM2, XMM4)){
										  index[18]	 = n[6];
										  goto SORT32_13071C;
										}else{
										  index[18]	 = n[28];
SORT32_13061D:
										  if(!_mm_comilt_ss(XMM2, XMM5)){
											index[19]	 = n[6];
											goto SORT32_14071D;
										  }else{
											index[19]	 = n[29];
SORT32_14061E:
											if(!_mm_comilt_ss(XMM2, XMM6)){
											  index[20]	 = n[6];
											  goto SORT32_15071E;
											}else{
											  index[20]	 = n[30];
SORT32_15061F:
											  if(!_mm_comilt_ss(XMM2, XMM7)){
												index[21]	 = n[6];
												goto SORT32_16071F;
											  }else{
												index[21]	 = n[31];
												index[22]	 = n[6];
												index[23]	 = n[7];
												index[24]	 = n[8];
												index[25]	 = n[9];
												index[26]	 = n[10];
												index[27]	 = n[11];
												index[28]	 = n[12];
												index[29]	 = n[13];
												index[30]	 = n[14];
												index[31]	 = n[15];
											  }
											}
										  }
										}
									  }
									}
								  }
								}
							  }
							}
						  }
						}
					  }
					}
				  }
				}
			  }else{
				index[5]	 = n[16];
SORT32_060511:
				if(!_mm_comilt_ss(XMM1, XMM5)){
				  index[6]	 = n[5];
				  goto SORT32_070611;
				}else{
				  index[6]	 = n[17];
SORT32_070512:
				  if(!_mm_comilt_ss(XMM1, XMM6)){
					index[7]	 = n[5];
					goto SORT32_080612;
				  }else{
					index[7]	 = n[18];
SORT32_080513:
					if(!_mm_comilt_ss(XMM1, XMM7)){
					  index[8]	 = n[5];
					  goto SORT32_090613;
					}else{
					  index[8]	 = n[19];
					  XMM4	 = _mm_load_ss(data+n[20]);
					  XMM5	 = _mm_load_ss(data+n[21]);
					  XMM6	 = _mm_load_ss(data+n[22]);
					  XMM7	 = _mm_load_ss(data+n[23]);
SORT32_090514:
					  if(!_mm_comilt_ss(XMM1, XMM4)){
						index[9]	 = n[5];
						goto SORT32_0A0614;
					  }else{
						index[9]	 = n[20];
SORT32_0A0515:
						if(!_mm_comilt_ss(XMM1, XMM5)){
						  index[10]	 = n[5];
						  goto SORT32_0B0615;
						}else{
						  index[10]	 = n[21];
SORT32_0B0516:
						  if(!_mm_comilt_ss(XMM1, XMM6)){
							index[11]	 = n[5];
							goto SORT32_0C0616;
						  }else{
							index[11]	 = n[22];
SORT32_0C0517:
							if(!_mm_comilt_ss(XMM1, XMM7)){
							  index[12]	 = n[5];
							  goto SORT32_0D0617;
							}else{
							  index[12]	 = n[23];
							  XMM4	 = _mm_load_ss(data+n[24]);
							  XMM5	 = _mm_load_ss(data+n[25]);
							  XMM6	 = _mm_load_ss(data+n[26]);
							  XMM7	 = _mm_load_ss(data+n[27]);
SORT32_0D0518:
							  if(!_mm_comilt_ss(XMM1, XMM4)){
								index[13]	 = n[5];
								goto SORT32_0E0618;
							  }else{
								index[13]	 = n[24];
SORT32_0E0519:
								if(!_mm_comilt_ss(XMM1, XMM5)){
								  index[14]	 = n[5];
								  goto SORT32_0F0619;
								}else{
								  index[14]	 = n[25];
SORT32_0F051A:
								  if(!_mm_comilt_ss(XMM1, XMM6)){
									index[15]	 = n[5];
									goto SORT32_10061A;
								  }else{
									index[15]	 = n[26];
SORT32_10051B:
									if(!_mm_comilt_ss(XMM1, XMM7)){
									  index[16]	 = n[5];
									  goto SORT32_11061B;
									}else{
									  index[16]	 = n[27];
									  XMM4	 = _mm_load_ss(data+n[28]);
									  XMM5	 = _mm_load_ss(data+n[29]);
									  XMM6	 = _mm_load_ss(data+n[30]);
									  XMM7	 = _mm_load_ss(data+n[31]);
SORT32_11051C:
									  if(!_mm_comilt_ss(XMM1, XMM4)){
										index[17]	 = n[5];
										goto SORT32_12061C;
									  }else{
										index[17]	 = n[28];
SORT32_12051D:
										if(!_mm_comilt_ss(XMM1, XMM5)){
										  index[18]	 = n[5];
										  goto SORT32_13061D;
										}else{
										  index[18]	 = n[29];
SORT32_13051E:
										  if(!_mm_comilt_ss(XMM1, XMM6)){
											index[19]	 = n[5];
											goto SORT32_14061E;
										  }else{
											index[19]	 = n[30];
SORT32_14051F:
											if(!_mm_comilt_ss(XMM1, XMM7)){
											  index[20]	 = n[5];
											  goto SORT32_15061F;
											}else{
											  index[20]	 = n[31];
											  index[21]	 = n[5];
											  index[22]	 = n[6];
											  index[23]	 = n[7];
											  index[24]	 = n[8];
											  index[25]	 = n[9];
											  index[26]	 = n[10];
											  index[27]	 = n[11];
											  index[28]	 = n[12];
											  index[29]	 = n[13];
											  index[30]	 = n[14];
											  index[31]	 = n[15];
											}
										  }
										}
									  }
									}
								  }
								}
							  }
							}
						  }
						}
					  }
					}
				  }
				}
			  }
			}else{
			  index[4]	 = n[16];
SORT32_050411:
			  if(!_mm_comilt_ss(XMM0, XMM5)){
				index[5]	 = n[4];
				goto SORT32_060511;
			  }else{
				index[5]	 = n[17];
SORT32_060412:
				if(!_mm_comilt_ss(XMM0, XMM6)){
				  index[6]	 = n[4];
				  goto SORT32_070512;
				}else{
				  index[6]	 = n[18];
SORT32_070413:
				  if(!_mm_comilt_ss(XMM0, XMM7)){
					index[7]	 = n[4];
					goto SORT32_080513;
				  }else{
					index[7]	 = n[19];
					XMM4	 = _mm_load_ss(data+n[20]);
					XMM5	 = _mm_load_ss(data+n[21]);
					XMM6	 = _mm_load_ss(data+n[22]);
					XMM7	 = _mm_load_ss(data+n[23]);
SORT32_080414:
					if(!_mm_comilt_ss(XMM0, XMM4)){
					  index[8]	 = n[4];
					  goto SORT32_090514;
					}else{
					  index[8]	 = n[20];
SORT32_090415:
					  if(!_mm_comilt_ss(XMM0, XMM5)){
						index[9]	 = n[4];
						goto SORT32_0A0515;
					  }else{
						index[9]	 = n[21];
SORT32_0A0416:
						if(!_mm_comilt_ss(XMM0, XMM6)){
						  index[10]	 = n[4];
						  goto SORT32_0B0516;
						}else{
						  index[10]	 = n[22];
SORT32_0B0417:
						  if(!_mm_comilt_ss(XMM0, XMM7)){
							index[11]	 = n[4];
							goto SORT32_0C0517;
						  }else{
							index[11]	 = n[23];
							XMM4	 = _mm_load_ss(data+n[24]);
							XMM5	 = _mm_load_ss(data+n[25]);
							XMM6	 = _mm_load_ss(data+n[26]);
							XMM7	 = _mm_load_ss(data+n[27]);
SORT32_0C0418:
							if(!_mm_comilt_ss(XMM0, XMM4)){
							  index[12]	 = n[4];
							  goto SORT32_0D0518;
							}else{
							  index[12]	 = n[24];
SORT32_0D0419:
							  if(!_mm_comilt_ss(XMM0, XMM5)){
								index[13]	 = n[4];
								goto SORT32_0E0519;
							  }else{
								index[13]	 = n[25];
SORT32_0E041A:
								if(!_mm_comilt_ss(XMM0, XMM6)){
								  index[14]	 = n[4];
								  goto SORT32_0F051A;
								}else{
								  index[14]	 = n[26];
SORT32_0F041B:
								  if(!_mm_comilt_ss(XMM0, XMM7)){
									index[15]	 = n[4];
									goto SORT32_10051B;
								  }else{
									index[15]	 = n[27];
									XMM4	 = _mm_load_ss(data+n[28]);
									XMM5	 = _mm_load_ss(data+n[29]);
									XMM6	 = _mm_load_ss(data+n[30]);
									XMM7	 = _mm_load_ss(data+n[31]);
SORT32_10041C:
									if(!_mm_comilt_ss(XMM0, XMM4)){
									  index[16]	 = n[4];
									  goto SORT32_11051C;
									}else{
									  index[16]	 = n[28];
SORT32_11041D:
									  if(!_mm_comilt_ss(XMM0, XMM5)){
										index[17]	 = n[4];
										goto SORT32_12051D;
									  }else{
										index[17]	 = n[29];
SORT32_12041E:
										if(!_mm_comilt_ss(XMM0, XMM6)){
										  index[18]	 = n[4];
										  goto SORT32_13051E;
										}else{
										  index[18]	 = n[30];
SORT32_13041F:
										  if(!_mm_comilt_ss(XMM0, XMM7)){
											index[19]	 = n[4];
											goto SORT32_14051F;
										  }else{
											index[19]	 = n[31];
											index[20]	 = n[4];
											index[21]	 = n[5];
											index[22]	 = n[6];
											index[23]	 = n[7];
											index[24]	 = n[8];
											index[25]	 = n[9];
											index[26]	 = n[10];
											index[27]	 = n[11];
											index[28]	 = n[12];
											index[29]	 = n[13];
											index[30]	 = n[14];
											index[31]	 = n[15];
										  }
										}
									  }
									}
								  }
								}
							  }
							}
						  }
						}
					  }
					}
				  }
				}
			  }
			}
		  }else{
			index[3]	 = n[16];
SORT32_040311:
			if(!_mm_comilt_ss(XMM3, XMM5)){
			  index[4]	 = n[3];
			  XMM0	 = _mm_load_ss(data+n[4]);
			  XMM1	 = _mm_load_ss(data+n[5]);
			  XMM2	 = _mm_load_ss(data+n[6]);
			  XMM3	 = _mm_load_ss(data+n[7]);
			  goto SORT32_050411;
			}else{
			  index[4]	 = n[17];
SORT32_050312:
			  if(!_mm_comilt_ss(XMM3, XMM6)){
				index[5]	 = n[3];
				XMM0	 = _mm_load_ss(data+n[4]);
				XMM1	 = _mm_load_ss(data+n[5]);
				XMM2	 = _mm_load_ss(data+n[6]);
				XMM3	 = _mm_load_ss(data+n[7]);
				goto SORT32_060412;
			  }else{
				index[5]	 = n[18];
SORT32_060313:
				if(!_mm_comilt_ss(XMM3, XMM7)){
				  index[6]	 = n[3];
				  XMM0	 = _mm_load_ss(data+n[4]);
				  XMM1	 = _mm_load_ss(data+n[5]);
				  XMM2	 = _mm_load_ss(data+n[6]);
				  XMM3	 = _mm_load_ss(data+n[7]);
				  goto SORT32_070413;
				}else{
				  index[6]	 = n[19];
				  XMM4	 = _mm_load_ss(data+n[20]);
				  XMM5	 = _mm_load_ss(data+n[21]);
				  XMM6	 = _mm_load_ss(data+n[22]);
				  XMM7	 = _mm_load_ss(data+n[23]);
SORT32_070314:
				  if(!_mm_comilt_ss(XMM3, XMM4)){
					index[7]	 = n[3];
					XMM0	 = _mm_load_ss(data+n[4]);
					XMM1	 = _mm_load_ss(data+n[5]);
					XMM2	 = _mm_load_ss(data+n[6]);
					XMM3	 = _mm_load_ss(data+n[7]);
					goto SORT32_080414;
				  }else{
					index[7]	 = n[20];
SORT32_080315:
					if(!_mm_comilt_ss(XMM3, XMM5)){
					  index[8]	 = n[3];
					  XMM0	 = _mm_load_ss(data+n[4]);
					  XMM1	 = _mm_load_ss(data+n[5]);
					  XMM2	 = _mm_load_ss(data+n[6]);
					  XMM3	 = _mm_load_ss(data+n[7]);
					  goto SORT32_090415;
					}else{
					  index[8]	 = n[21];
SORT32_090316:
					  if(!_mm_comilt_ss(XMM3, XMM6)){
						index[9]	 = n[3];
						XMM0	 = _mm_load_ss(data+n[4]);
						XMM1	 = _mm_load_ss(data+n[5]);
						XMM2	 = _mm_load_ss(data+n[6]);
						XMM3	 = _mm_load_ss(data+n[7]);
						goto SORT32_0A0416;
					  }else{
						index[9]	 = n[22];
SORT32_0A0317:
						if(!_mm_comilt_ss(XMM3, XMM7)){
						  index[10]	 = n[3];
						  XMM0	 = _mm_load_ss(data+n[4]);
						  XMM1	 = _mm_load_ss(data+n[5]);
						  XMM2	 = _mm_load_ss(data+n[6]);
						  XMM3	 = _mm_load_ss(data+n[7]);
						  goto SORT32_0B0417;
						}else{
						  index[10]	 = n[23];
						  XMM4	 = _mm_load_ss(data+n[24]);
						  XMM5	 = _mm_load_ss(data+n[25]);
						  XMM6	 = _mm_load_ss(data+n[26]);
						  XMM7	 = _mm_load_ss(data+n[27]);
SORT32_0B0318:
						  if(!_mm_comilt_ss(XMM3, XMM4)){
							index[11]	 = n[3];
							XMM0	 = _mm_load_ss(data+n[4]);
							XMM1	 = _mm_load_ss(data+n[5]);
							XMM2	 = _mm_load_ss(data+n[6]);
							XMM3	 = _mm_load_ss(data+n[7]);
							goto SORT32_0C0418;
						  }else{
							index[11]	 = n[24];
SORT32_0C0319:
							if(!_mm_comilt_ss(XMM3, XMM5)){
							  index[12]	 = n[3];
							  XMM0	 = _mm_load_ss(data+n[4]);
							  XMM1	 = _mm_load_ss(data+n[5]);
							  XMM2	 = _mm_load_ss(data+n[6]);
							  XMM3	 = _mm_load_ss(data+n[7]);
							  goto SORT32_0D0419;
							}else{
							  index[12]	 = n[25];
SORT32_0D031A:
							  if(!_mm_comilt_ss(XMM3, XMM6)){
								index[13]	 = n[3];
								XMM0	 = _mm_load_ss(data+n[4]);
								XMM1	 = _mm_load_ss(data+n[5]);
								XMM2	 = _mm_load_ss(data+n[6]);
								XMM3	 = _mm_load_ss(data+n[7]);
								goto SORT32_0E041A;
							  }else{
								index[13]	 = n[26];
SORT32_0E031B:
								if(!_mm_comilt_ss(XMM3, XMM7)){
								  index[14]	 = n[3];
								  XMM0	 = _mm_load_ss(data+n[4]);
								  XMM1	 = _mm_load_ss(data+n[5]);
								  XMM2	 = _mm_load_ss(data+n[6]);
								  XMM3	 = _mm_load_ss(data+n[7]);
								  goto SORT32_0F041B;
								}else{
								  index[14]	 = n[27];
								  XMM4	 = _mm_load_ss(data+n[28]);
								  XMM5	 = _mm_load_ss(data+n[29]);
								  XMM6	 = _mm_load_ss(data+n[30]);
								  XMM7	 = _mm_load_ss(data+n[31]);
SORT32_0F031C:
								  if(!_mm_comilt_ss(XMM3, XMM4)){
									index[15]	 = n[3];
									XMM0	 = _mm_load_ss(data+n[4]);
									XMM1	 = _mm_load_ss(data+n[5]);
									XMM2	 = _mm_load_ss(data+n[6]);
									XMM3	 = _mm_load_ss(data+n[7]);
									goto SORT32_10041C;
								  }else{
									index[15]	 = n[28];
SORT32_10031D:
									if(!_mm_comilt_ss(XMM3, XMM5)){
									  index[16]	 = n[3];
									  XMM0	 = _mm_load_ss(data+n[4]);
									  XMM1	 = _mm_load_ss(data+n[5]);
									  XMM2	 = _mm_load_ss(data+n[6]);
									  XMM3	 = _mm_load_ss(data+n[7]);
									  goto SORT32_11041D;
									}else{
									  index[16]	 = n[29];
SORT32_11031E:
									  if(!_mm_comilt_ss(XMM3, XMM6)){
										index[17]	 = n[3];
										XMM0	 = _mm_load_ss(data+n[4]);
										XMM1	 = _mm_load_ss(data+n[5]);
										XMM2	 = _mm_load_ss(data+n[6]);
										XMM3	 = _mm_load_ss(data+n[7]);
										goto SORT32_12041E;
									  }else{
										index[17]	 = n[30];
SORT32_12031F:
										if(!_mm_comilt_ss(XMM3, XMM7)){
										  index[18]	 = n[3];
										  XMM0	 = _mm_load_ss(data+n[4]);
										  XMM1	 = _mm_load_ss(data+n[5]);
										  XMM2	 = _mm_load_ss(data+n[6]);
										  XMM3	 = _mm_load_ss(data+n[7]);
										  goto SORT32_13041F;
										}else{
										  index[18]	 = n[31];
										  index[19]	 = n[3];
										  index[20]	 = n[4];
										  index[21]	 = n[5];
										  index[22]	 = n[6];
										  index[23]	 = n[7];
										  index[24]	 = n[8];
										  index[25]	 = n[9];
										  index[26]	 = n[10];
										  index[27]	 = n[11];
										  index[28]	 = n[12];
										  index[29]	 = n[13];
										  index[30]	 = n[14];
										  index[31]	 = n[15];
										}
									  }
									}
								  }
								}
							  }
							}
						  }
						}
					  }
					}
				  }
				}
			  }
			}
		  }
		}else{
		  index[2]	 = n[16];
SORT32_030211:
		  if(!_mm_comilt_ss(XMM2, XMM5)){
			index[3]	 = n[2];
			goto SORT32_040311;
		  }else{
			index[3]	 = n[17];
SORT32_040212:
			if(!_mm_comilt_ss(XMM2, XMM6)){
			  index[4]	 = n[2];
			  goto SORT32_050312;
			}else{
			  index[4]	 = n[18];
SORT32_050213:
			  if(!_mm_comilt_ss(XMM2, XMM7)){
				index[5]	 = n[2];
				goto SORT32_060313;
			  }else{
				index[5]	 = n[19];
				XMM4	 = _mm_load_ss(data+n[20]);
				XMM5	 = _mm_load_ss(data+n[21]);
				XMM6	 = _mm_load_ss(data+n[22]);
				XMM7	 = _mm_load_ss(data+n[23]);
SORT32_060214:
				if(!_mm_comilt_ss(XMM2, XMM4)){
				  index[6]	 = n[2];
				  goto SORT32_070314;
				}else{
				  index[6]	 = n[20];
SORT32_070215:
				  if(!_mm_comilt_ss(XMM2, XMM5)){
					index[7]	 = n[2];
					goto SORT32_080315;
				  }else{
					index[7]	 = n[21];
SORT32_080216:
					if(!_mm_comilt_ss(XMM2, XMM6)){
					  index[8]	 = n[2];
					  goto SORT32_090316;
					}else{
					  index[8]	 = n[22];
SORT32_090217:
					  if(!_mm_comilt_ss(XMM2, XMM7)){
						index[9]	 = n[2];
						goto SORT32_0A0317;
					  }else{
						index[9]	 = n[23];
						XMM4	 = _mm_load_ss(data+n[24]);
						XMM5	 = _mm_load_ss(data+n[25]);
						XMM6	 = _mm_load_ss(data+n[26]);
						XMM7	 = _mm_load_ss(data+n[27]);
SORT32_0A0218:
						if(!_mm_comilt_ss(XMM2, XMM4)){
						  index[10]	 = n[2];
						  goto SORT32_0B0318;
						}else{
						  index[10]	 = n[24];
SORT32_0B0219:
						  if(!_mm_comilt_ss(XMM2, XMM5)){
							index[11]	 = n[2];
							goto SORT32_0C0319;
						  }else{
							index[11]	 = n[25];
SORT32_0C021A:
							if(!_mm_comilt_ss(XMM2, XMM6)){
							  index[12]	 = n[2];
							  goto SORT32_0D031A;
							}else{
							  index[12]	 = n[26];
SORT32_0D021B:
							  if(!_mm_comilt_ss(XMM2, XMM7)){
								index[13]	 = n[2];
								goto SORT32_0E031B;
							  }else{
								index[13]	 = n[27];
								XMM4	 = _mm_load_ss(data+n[28]);
								XMM5	 = _mm_load_ss(data+n[29]);
								XMM6	 = _mm_load_ss(data+n[30]);
								XMM7	 = _mm_load_ss(data+n[31]);
SORT32_0E021C:
								if(!_mm_comilt_ss(XMM2, XMM4)){
								  index[14]	 = n[2];
								  goto SORT32_0F031C;
								}else{
								  index[14]	 = n[28];
SORT32_0F021D:
								  if(!_mm_comilt_ss(XMM2, XMM5)){
									index[15]	 = n[2];
									goto SORT32_10031D;
								  }else{
									index[15]	 = n[29];
SORT32_10021E:
									if(!_mm_comilt_ss(XMM2, XMM6)){
									  index[16]	 = n[2];
									  goto SORT32_11031E;
									}else{
									  index[16]	 = n[30];
SORT32_11021F:
									  if(!_mm_comilt_ss(XMM2, XMM7)){
										index[17]	 = n[2];
										goto SORT32_12031F;
									  }else{
										index[17]	 = n[31];
										index[18]	 = n[2];
										index[19]	 = n[3];
										index[20]	 = n[4];
										index[21]	 = n[5];
										index[22]	 = n[6];
										index[23]	 = n[7];
										index[24]	 = n[8];
										index[25]	 = n[9];
										index[26]	 = n[10];
										index[27]	 = n[11];
										index[28]	 = n[12];
										index[29]	 = n[13];
										index[30]	 = n[14];
										index[31]	 = n[15];
									  }
									}
								  }
								}
							  }
							}
						  }
						}
					  }
					}
				  }
				}
			  }
			}
		  }
		}
	  }else{
		index[1]	 = n[16];
SORT32_020111:
		if(!_mm_comilt_ss(XMM1, XMM5)){
		  index[2]	 = n[1];
		  goto SORT32_030211;
		}else{
		  index[2]	 = n[17];
SORT32_030112:
		  if(!_mm_comilt_ss(XMM1, XMM6)){
			index[3]	 = n[1];
			goto SORT32_040212;
		  }else{
			index[3]	 = n[18];
SORT32_040113:
			if(!_mm_comilt_ss(XMM1, XMM7)){
			  index[4]	 = n[1];
			  goto SORT32_050213;
			}else{
			  index[4]	 = n[19];
			  XMM4	 = _mm_load_ss(data+n[20]);
			  XMM5	 = _mm_load_ss(data+n[21]);
			  XMM6	 = _mm_load_ss(data+n[22]);
			  XMM7	 = _mm_load_ss(data+n[23]);
SORT32_050114:
			  if(!_mm_comilt_ss(XMM1, XMM4)){
				index[5]	 = n[1];
				goto SORT32_060214;
			  }else{
				index[5]	 = n[20];
SORT32_060115:
				if(!_mm_comilt_ss(XMM1, XMM5)){
				  index[6]	 = n[1];
				  goto SORT32_070215;
				}else{
				  index[6]	 = n[21];
SORT32_070116:
				  if(!_mm_comilt_ss(XMM1, XMM6)){
					index[7]	 = n[1];
					goto SORT32_080216;
				  }else{
					index[7]	 = n[22];
SORT32_080117:
					if(!_mm_comilt_ss(XMM1, XMM7)){
					  index[8]	 = n[1];
					  goto SORT32_090217;
					}else{
					  index[8]	 = n[23];
					  XMM4	 = _mm_load_ss(data+n[24]);
					  XMM5	 = _mm_load_ss(data+n[25]);
					  XMM6	 = _mm_load_ss(data+n[26]);
					  XMM7	 = _mm_load_ss(data+n[27]);
SORT32_090118:
					  if(!_mm_comilt_ss(XMM1, XMM4)){
						index[9]	 = n[1];
						goto SORT32_0A0218;
					  }else{
						index[9]	 = n[24];
SORT32_0A0119:
						if(!_mm_comilt_ss(XMM1, XMM5)){
						  index[10]	 = n[1];
						  goto SORT32_0B0219;
						}else{
						  index[10]	 = n[25];
SORT32_0B011A:
						  if(!_mm_comilt_ss(XMM1, XMM6)){
							index[11]	 = n[1];
							goto SORT32_0C021A;
						  }else{
							index[11]	 = n[26];
SORT32_0C011B:
							if(!_mm_comilt_ss(XMM1, XMM7)){
							  index[12]	 = n[1];
							  goto SORT32_0D021B;
							}else{
							  index[12]	 = n[27];
							  XMM4	 = _mm_load_ss(data+n[28]);
							  XMM5	 = _mm_load_ss(data+n[29]);
							  XMM6	 = _mm_load_ss(data+n[30]);
							  XMM7	 = _mm_load_ss(data+n[31]);
SORT32_0D011C:
							  if(!_mm_comilt_ss(XMM1, XMM4)){
								index[13]	 = n[1];
								goto SORT32_0E021C;
							  }else{
								index[13]	 = n[28];
SORT32_0E011D:
								if(!_mm_comilt_ss(XMM1, XMM5)){
								  index[14]	 = n[1];
								  goto SORT32_0F021D;
								}else{
								  index[14]	 = n[29];
SORT32_0F011E:
								  if(!_mm_comilt_ss(XMM1, XMM6)){
									index[15]	 = n[1];
									goto SORT32_10021E;
								  }else{
									index[15]	 = n[30];
SORT32_10011F:
									if(!_mm_comilt_ss(XMM1, XMM7)){
									  index[16]	 = n[1];
									  goto SORT32_11021F;
									}else{
									  index[16]	 = n[31];
									  index[17]	 = n[1];
									  index[18]	 = n[2];
									  index[19]	 = n[3];
									  index[20]	 = n[4];
									  index[21]	 = n[5];
									  index[22]	 = n[6];
									  index[23]	 = n[7];
									  index[24]	 = n[8];
									  index[25]	 = n[9];
									  index[26]	 = n[10];
									  index[27]	 = n[11];
									  index[28]	 = n[12];
									  index[29]	 = n[13];
									  index[30]	 = n[14];
									  index[31]	 = n[15];
									}
								  }
								}
							  }
							}
						  }
						}
					  }
					}
				  }
				}
			  }
			}
		  }
		}
	  }
	}else{
	  index[0]	 = n[16];
	  if(!_mm_comilt_ss(XMM0, XMM5)){
		index[1]	 = n[0];
		goto SORT32_020111;
	  }else{
		index[1]	 = n[17];
		if(!_mm_comilt_ss(XMM0, XMM6)){
		  index[2]	 = n[0];
		  goto SORT32_030112;
		}else{
		  index[2]	 = n[18];
		  if(!_mm_comilt_ss(XMM0, XMM7)){
			index[3]	 = n[0];
			goto SORT32_040113;
		  }else{
			index[3]	 = n[19];
			XMM4	 = _mm_load_ss(data+n[20]);
			XMM5	 = _mm_load_ss(data+n[21]);
			XMM6	 = _mm_load_ss(data+n[22]);
			XMM7	 = _mm_load_ss(data+n[23]);
			if(!_mm_comilt_ss(XMM0, XMM4)){
			  index[4]	 = n[0];
			  goto SORT32_050114;
			}else{
			  index[4]	 = n[20];
			  if(!_mm_comilt_ss(XMM0, XMM5)){
				index[5]	 = n[0];
				goto SORT32_060115;
			  }else{
				index[5]	 = n[21];
				if(!_mm_comilt_ss(XMM0, XMM6)){
				  index[6]	 = n[0];
				  goto SORT32_070116;
				}else{
				  index[6]	 = n[22];
				  if(!_mm_comilt_ss(XMM0, XMM7)){
					index[7]	 = n[0];
					goto SORT32_080117;
				  }else{
					index[7]	 = n[23];
					XMM4	 = _mm_load_ss(data+n[24]);
					XMM5	 = _mm_load_ss(data+n[25]);
					XMM6	 = _mm_load_ss(data+n[26]);
					XMM7	 = _mm_load_ss(data+n[27]);
					if(!_mm_comilt_ss(XMM0, XMM4)){
					  index[8]	 = n[0];
					  goto SORT32_090118;
					}else{
					  index[8]	 = n[24];
					  if(!_mm_comilt_ss(XMM0, XMM5)){
						index[9]	 = n[0];
						goto SORT32_0A0119;
					  }else{
						index[9]	 = n[25];
						if(!_mm_comilt_ss(XMM0, XMM6)){
						  index[10]	 = n[0];
						  goto SORT32_0B011A;
						}else{
						  index[10]	 = n[26];
						  if(!_mm_comilt_ss(XMM0, XMM7)){
							index[11]	 = n[0];
							goto SORT32_0C011B;
						  }else{
							index[11]	 = n[27];
							XMM4	 = _mm_load_ss(data+n[28]);
							XMM5	 = _mm_load_ss(data+n[29]);
							XMM6	 = _mm_load_ss(data+n[30]);
							XMM7	 = _mm_load_ss(data+n[31]);
							if(!_mm_comilt_ss(XMM0, XMM4)){
							  index[12]	 = n[0];
							  goto SORT32_0D011C;
							}else{
							  index[12]	 = n[28];
							  if(!_mm_comilt_ss(XMM0, XMM5)){
								index[13]	 = n[0];
								goto SORT32_0E011D;
							  }else{
								index[13]	 = n[29];
								if(!_mm_comilt_ss(XMM0, XMM6)){
								  index[14]	 = n[0];
								  goto SORT32_0F011E;
								}else{
								  index[14]	 = n[30];
								  if(!_mm_comilt_ss(XMM0, XMM7)){
									index[15]	 = n[0];
									goto SORT32_10011F;
								  }else{
									index[15]	 = n[31];
									index[16]	 = n[0];
									index[17]	 = n[1];
									index[18]	 = n[2];
									index[19]	 = n[3];
									index[20]	 = n[4];
									index[21]	 = n[5];
									index[22]	 = n[6];
									index[23]	 = n[7];
									index[24]	 = n[8];
									index[25]	 = n[9];
									index[26]	 = n[10];
									index[27]	 = n[11];
									index[28]	 = n[12];
									index[29]	 = n[13];
									index[30]	 = n[14];
									index[31]	 = n[15];
								  }
								}
							  }
							}
						  }
						}
					  }
					}
				  }
				}
			  }
			}
		  }
		}
	  }
	}
}
static void sortindex_shellsort(int *index,
								float *data,
								int offset,
								int count){
  int gap,pos,left,i,j;
  index+=offset;
  for(i=0;i<count;i++)index[i]=i+offset;
  gap=1;
  while (gap<=count)gap=gap*3+1;
  gap/=3;
  if(gap>=4)gap/=3;
  while(gap>0){
	for(pos=gap;pos<count;pos++){
	  for(left=pos-gap;left>=0;left-=gap){
		i=index[left];j=index[left+gap];
		if(!C(i,j)){
		  index[left]=j;
		  index[left+gap]=i;
		}else break;
	  }
	}
	gap/=3;
  }
}
#else														/* SSE Optimize */
#define C(o,a,b)\
  (fabs(data[o+a])>=fabs(data[o+b]))
#define O(o,a,b,c,d)\
  {n[o]=o+a;n[o+1]=o+b;n[o+2]=o+c;n[o+3]=o+d;}
#define SORT4(o)\
  if(C(o,2,3))if(C(o,0,1))if(C(o,0,2))if(C(o,1,2))O(o,0,1,2,3)\
        else if(C(o,1,3))O(o,0,2,1,3)\
          else O(o,0,2,3,1)\
      else if(C(o,0,3))if(C(o,1,3))O(o,2,0,1,3)\
          else O(o,2,0,3,1)\
        else O(o,2,3,0,1)\
    else if(C(o,1,2))if(C(o,0,2))O(o,1,0,2,3)\
        else if(C(o,0,3))O(o,1,2,0,3)\
          else O(o,1,2,3,0)\
      else if(C(o,1,3))if(C(o,0,3))O(o,2,1,0,3)\
          else O(o,2,1,3,0)\
        else O(o,2,3,1,0)\
  else if(C(o,0,1))if(C(o,0,3))if(C(o,1,3))O(o,0,1,3,2)\
        else if(C(o,1,2))O(o,0,3,1,2)\
          else O(o,0,3,2,1)\
      else if(C(o,0,2))if(C(o,1,2))O(o,3,0,1,2)\
          else O(o,3,0,2,1)\
        else O(o,3,2,0,1)\
    else if(C(o,1,3))if(C(o,0,3))O(o,1,0,3,2)\
        else if(C(o,0,2))O(o,1,3,0,2)\
          else O(o,1,3,2,0)\
      else if(C(o,1,2))if(C(o,0,2))O(o,3,1,0,2)\
          else O(o,3,1,2,0)\
        else O(o,3,2,1,0)

static void sortindex_fix8(int *index,
                           float *data,
                           int offset){
  int i,j,k,n[8];
  index+=offset;
  data+=offset;
  SORT4(0)
  SORT4(4)
  j=0;k=4;
  for(i=0;i<8;i++)
    index[i]=n[(k>=8)||(j<4)&&C(0,n[j],n[k])?j++:k++]+offset;
}

static void sortindex_fix32(int *index,
                            float *data,
                            int offset){
  int i,j,k,n[32];
  for(i=0;i<32;i+=8)
    sortindex_fix8(index,data,offset+i);
  index+=offset;
  for(i=j=0,k=8;i<16;i++)
    n[i]=index[(k>=16)||(j<8)&&C(0,index[j],index[k])?j++:k++];
  for(i=j=16,k=24;i<32;i++)
    n[i]=index[(k>=32)||(j<24)&&C(0,index[j],index[k])?j++:k++];
  for(i=j=0,k=16;i<32;i++)
    index[i]=n[(k>=32)||(j<16)&&C(0,n[j],n[k])?j++:k++];
}

static void sortindex_shellsort(int *index,
                                float *data,
                                int offset,
                                int count){
  int gap,pos,left,right,i,j;
  index+=offset;
  for(i=0;i<count;i++)index[i]=i+offset;
  gap=1;
  while (gap<=count)gap=gap*3+1;
  gap/=3;
  if(gap>=4)gap/=3;
  while(gap>0){
    for(pos=gap;pos<count;pos++){
      for(left=pos-gap;left>=0;left-=gap){
        i=index[left];j=index[left+gap];
        if(!C(0,i,j)){
          index[left]=j;
          index[left+gap]=i;
        }else break;
      }
    }
    gap/=3;
  }
}
#endif														/* SSE Optimize */

static void sortindex(int *index,
                      float *data,
                      int offset,
                      int count){
  if(count==8)sortindex_fix8(index,data,offset);
  else if(count==32)sortindex_fix32(index,data,offset);
  else sortindex_shellsort(index,data,offset,count);
}

#undef C
#ifndef	__SSE__												/* SSE Optimize */
/* this is for per-channel noise normalization */
static int apsort(const void *a, const void *b){
  float f1=fabs(**(float**)a);
  float f2=fabs(**(float**)b);
  return (f1<f2)-(f1>f2);
}
#undef O
#undef SORT4
#endif														/* SSE Optimize */

int **_vp_quantize_couple_sort(vorbis_block *vb,
			       vorbis_look_psy *p,
			       vorbis_info_mapping0 *vi,
#ifdef	__SSE__												/* SSE Optimize */
			       float **mags,
#if defined(_OPENMP)
				   float *temp,
				   int **ret,
				   int thnum,
				   int thmax){
#else
				   float *temp){
#endif
#else														/* SSE Optimize */
				   float **mags){
#endif														/* SSE Optimize */


#ifdef	__SSE__												/* SSE Optimize */
  if(p->vi->normal_point_p){
    int i,j,n=p->n;
#if !defined(_OPENMP)
    int **ret=_vorbis_block_alloc(vb,vi->coupling_steps*sizeof(*ret));
#else
	int s = _omp_get_start(n, 16, thnum, thmax);
	int e = _omp_get_end(n, 16, thnum, thmax);
#endif
    int partition=p->vi->normal_partition;
    
	for(i=0;i<vi->coupling_steps;i++)
	{
#if	defined(_OPENMP)
		for(j=s;j<e;j+=16)
#else
		for(j=0;j<n;j+=16)
#endif
		{
			__m128	XMM0	 = _mm_load_ps(mags[i]+j   );
			__m128	XMM1	 = _mm_load_ps(mags[i]+j+ 4);
			__m128	XMM2	 = _mm_load_ps(mags[i]+j+ 8);
			__m128	XMM3	 = _mm_load_ps(mags[i]+j+12);
			XMM0	 = _mm_and_ps(XMM0, PM128(PABSMASK));
			XMM1	 = _mm_and_ps(XMM1, PM128(PABSMASK));
			XMM2	 = _mm_and_ps(XMM2, PM128(PABSMASK));
			XMM3	 = _mm_and_ps(XMM3, PM128(PABSMASK));
			_mm_store_ps(temp+j   , XMM0);
			_mm_store_ps(temp+j+ 4, XMM1);
			_mm_store_ps(temp+j+ 8, XMM2);
			_mm_store_ps(temp+j+12, XMM3);
		}
#if !defined(_OPENMP)
		ret[i]=_vorbis_block_alloc(vb,n*sizeof(**ret));
#endif
	
#if	defined(_OPENMP)
		for(j=s;j<e;j+=partition)
#else
		for(j=0;j<n;j+=partition)
#endif
		{
			sortindex(ret[i], temp, j, partition);
		}
	}
    return(ret);
  }
  return(NULL);
#else														/* SSE Optimize */
#ifdef OPT_SORT
  if(p->vi->normal_point_p){
    int i,j,n=p->n;
    int **ret=_vorbis_block_alloc(vb,vi->coupling_steps*sizeof(*ret));
    int partition=p->vi->normal_partition;
    
    for(i=0;i<vi->coupling_steps;i++){
      ret[i]=_vorbis_block_alloc(vb,n*sizeof(**ret));
      
      for(j=0;j<n;j+=partition){
      sortindex(ret[i],mags[i],j,partition);
      }
    }
    return(ret);
  }
  return(NULL);
#else
  if(p->vi->normal_point_p){
    int i,j,k,n=p->n;
    int **ret=_vorbis_block_alloc(vb,vi->coupling_steps*sizeof(*ret));
    int partition=p->vi->normal_partition;
    float **work=alloca(sizeof(*work)*partition);
    
    for(i=0;i<vi->coupling_steps;i++){
      ret[i]=_vorbis_block_alloc(vb,n*sizeof(**ret));
      
      for(j=0;j<n;j+=partition){
	for(k=0;k<partition;k++)work[k]=mags[i]+k+j;
	qsort(work,partition,sizeof(*work),apsort);
	for(k=0;k<partition;k++)ret[i][k+j]=work[k]-mags[i];
      }
    }
    return(ret);
  }
  return(NULL);
#endif
#endif														/* SSE Optimize */
}

#ifdef	__SSE__												/* SSE Optimize */
void _vp_noise_normalize_sort(vorbis_look_psy *p,
			      float *magnitudes,int *sortedindex,float *temp){
	int j, n=p->n;
	vorbis_info_psy	*vi=p->vi;
	int	partition=vi->normal_partition;
	int	start=vi->normal_start;

	int k;
	j	 = start;
	k	 = (j+15)&(~15);
	k	 = (k>=n)?n:k;
	for(;j<k;j++)
	{
		__m128	XMM0	 = _mm_load_ss(magnitudes+j);
		XMM0	 = _mm_and_ps(XMM0, PM128(PABSMASK));
		_mm_store_ss(temp+j,XMM0);
	}
	for(;j<n;j+=16)
	{
		__m128	XMM0	 = _mm_load_ps(magnitudes+j   );
		__m128	XMM1	 = _mm_load_ps(magnitudes+j+ 4);
		__m128	XMM2	 = _mm_load_ps(magnitudes+j+ 8);
		__m128	XMM3	 = _mm_load_ps(magnitudes+j+12);
		XMM0	 = _mm_and_ps(XMM0, PM128(PABSMASK));
		XMM1	 = _mm_and_ps(XMM1, PM128(PABSMASK));
		XMM2	 = _mm_and_ps(XMM2, PM128(PABSMASK));
		XMM3	 = _mm_and_ps(XMM3, PM128(PABSMASK));
		_mm_store_ps(temp+j   , XMM0);
		_mm_store_ps(temp+j+ 4, XMM1);
		_mm_store_ps(temp+j+ 8, XMM2);
		_mm_store_ps(temp+j+12, XMM3);
	}
	for(j=start;j<n;j+=partition)
	{
		if(j+partition>n)
			partition	 = n-j;
		sortindex(sortedindex-start, temp, j, partition);
	}
#else														/* SSE Optimize */
void _vp_noise_normalize_sort(vorbis_look_psy *p,
			      float *magnitudes,int *sortedindex){
  int i,j,n=p->n;
  vorbis_info_psy *vi=p->vi;
  int partition=vi->normal_partition;
  float **work=alloca(sizeof(*work)*partition);
  int start=vi->normal_start;

  for(j=start;j<n;j+=partition){
    if(j+partition>n)partition=n-j;
    for(i=0;i<partition;i++)work[i]=magnitudes+i+j;
    qsort(work,partition,sizeof(*work),apsort);
    for(i=0;i<partition;i++){
      sortedindex[i+j-start]=work[i]-magnitudes;
    }
  }
#endif														/* SSE Optimize */
}

void _vp_noise_normalize(vorbis_look_psy *p,
			 float *in,float *out,int *sortedindex){
  int i,j=0,n=p->n,min_energy;
  vorbis_info_psy *vi=p->vi;
  int partition=vi->normal_partition;
  int start=vi->normal_start;

  if(start>n)start=n;

  if(vi->normal_channel_p){
#ifdef	__SSE__												/* SSE Optimize */
	{
		int k;
		k	 = start&(~15);
		for(;j<k;j+=16)
		{
			__m128	XMM0, XMM1, XMM2, XMM3;
#if	!defined(__SSE2__)
			__m64	MM0, MM1, MM2, MM3;
			__m64	MM4, MM5, MM6, MM7;
#endif
			XMM0	 = _mm_load_ps(in+j   );
			XMM1	 = _mm_load_ps(in+j+ 4);
			XMM2	 = _mm_load_ps(in+j+ 8);
			XMM3	 = _mm_load_ps(in+j+12);
#if	defined(__SSE2__)
			XMM0	 = _mm_cvtepi32_ps(_mm_cvtps_epi32(XMM0));
			XMM1	 = _mm_cvtepi32_ps(_mm_cvtps_epi32(XMM1));
			XMM2	 = _mm_cvtepi32_ps(_mm_cvtps_epi32(XMM2));
			XMM3	 = _mm_cvtepi32_ps(_mm_cvtps_epi32(XMM3));
#else
			MM0		 = _mm_cvtps_pi32(XMM0);
			MM2		 = _mm_cvtps_pi32(XMM1);
			MM4		 = _mm_cvtps_pi32(XMM2);
			MM6		 = _mm_cvtps_pi32(XMM3);
			XMM0	 = _mm_movehl_ps(XMM0, XMM0);
			XMM1	 = _mm_movehl_ps(XMM1, XMM1);
			XMM2	 = _mm_movehl_ps(XMM2, XMM2);
			XMM3	 = _mm_movehl_ps(XMM3, XMM3);
			MM1		 = _mm_cvtps_pi32(XMM0);
			MM3		 = _mm_cvtps_pi32(XMM1);
			MM5		 = _mm_cvtps_pi32(XMM2);
			MM7		 = _mm_cvtps_pi32(XMM3);
			XMM0	 = _mm_cvtpi32_ps(XMM0, MM1);
			XMM1	 = _mm_cvtpi32_ps(XMM1, MM3);
			XMM2	 = _mm_cvtpi32_ps(XMM2, MM5);
			XMM3	 = _mm_cvtpi32_ps(XMM3, MM7);
			XMM0	 = _mm_movelh_ps(XMM0, XMM0);
			XMM1	 = _mm_movelh_ps(XMM1, XMM1);
			XMM2	 = _mm_movelh_ps(XMM2, XMM2);
			XMM3	 = _mm_movelh_ps(XMM3, XMM3);
			XMM0	 = _mm_cvtpi32_ps(XMM0, MM0);
			XMM1	 = _mm_cvtpi32_ps(XMM1, MM2);
			XMM2	 = _mm_cvtpi32_ps(XMM2, MM4);
			XMM3	 = _mm_cvtpi32_ps(XMM3, MM6);
#endif
			_mm_store_ps(out+j   , XMM0);
			_mm_store_ps(out+j+ 4, XMM1);
			_mm_store_ps(out+j+ 8, XMM2);
			_mm_store_ps(out+j+12, XMM3);
		}
#if	!defined(__SSE2__)
		_mm_empty();
#endif
		for(;j<start;j++)
			out[j]	 = rint(in[j]);
	}
#else														/* SSE Optimize */
    for(;j<start;j++)
      out[j]=rint(in[j]);
#endif														/* SSE Optimize */
    
    for(;j+partition<=n;j+=partition){
#ifdef	__SSE__												/* SSE Optimize */
      float acc;
      int k;
      int energy_loss;
#else
      float acc=0.;
      int k;
      int energy_loss=0;
#endif
      int nn_num=0;
      int freqband_mid=j+16;
      int freqband_flag=0;
      
#ifdef	__SSE__												/* SSE Optimize */
	  {
		if(partition==8)
		{
		  int c0, c1;
#if defined(__SSE2__)
		  __m128 XMM0, XMM1, XMM2, XMM3;
		  XMM0 = _mm_load_ps(in+j  );
		  XMM1 = _mm_load_ps(in+j+4);
		  XMM2 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM0));
		  XMM3 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM1));
		  XMM2 = _mm_castsi128_ps(_mm_cmpeq_epi32(_mm_castps_si128(XMM2), PM128I(PFV_0)));
		  XMM3 = _mm_castsi128_ps(_mm_cmpeq_epi32(_mm_castps_si128(XMM3), PM128I(PFV_0)));
		  XMM0 = _mm_and_ps(XMM0, XMM2);
		  XMM1 = _mm_and_ps(XMM1, XMM3);
		  XMM0 = _mm_mul_ps(XMM0, XMM0);
		  XMM1 = _mm_mul_ps(XMM1, XMM1);
		  c0   = _mm_movemask_ps(_mm_castsi128_ps(XMM2));
		  XMM0 = _mm_add_ps(XMM0, XMM1);
		  c1   = _mm_movemask_ps(_mm_castsi128_ps(XMM3));
#else
		  __m128 XMM0, XMM1, XMM2, XMM3, XMM4, XMM5;
		  XMM0 = _mm_load_ps(in+j  );
		  XMM1 = _mm_load_ps(in+j+4);
		  XMM4 = XMM0;
		  XMM5 = XMM1;
		  XMM2 = _mm_cmplt_ps(PM128(PFV_M0P5), XMM0);
		  XMM3 = _mm_cmplt_ps(PM128(PFV_M0P5), XMM1);
		  XMM4 = _mm_cmplt_ps(XMM4, PM128(PFV_0P5));
		  XMM5 = _mm_cmplt_ps(XMM5, PM128(PFV_0P5));
		  XMM2 = _mm_and_ps(XMM2, XMM4);
		  XMM3 = _mm_and_ps(XMM3, XMM5);
		  XMM0 = _mm_and_ps(XMM0, XMM2);
		  XMM1 = _mm_and_ps(XMM1, XMM3);
		  XMM0 = _mm_mul_ps(XMM0, XMM0);
		  XMM1 = _mm_mul_ps(XMM1, XMM1);
		  c0   = _mm_movemask_ps(XMM2);
		  XMM0 = _mm_add_ps(XMM0, XMM1);
		  c1   = _mm_movemask_ps(XMM3);
#endif
		  acc = _mm_add_horz(XMM0);
		  energy_loss  = bitCountTable[c0];
		  energy_loss += bitCountTable[c1];
		}
		else if(partition==32)
		{
		  int c0, c1;
#if defined(__SSE2__)
		  __m128 XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6;
		  XMM0 = _mm_load_ps(in+j   );
		  XMM1 = _mm_load_ps(in+j+ 4);
		  XMM4 = _mm_load_ps(in+j+ 8);
		  XMM5 = _mm_load_ps(in+j+12);
		  XMM2 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM0));
		  XMM3 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM1));
		  XMM6 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM4));
		  XMM2 = _mm_castsi128_ps(_mm_cmpeq_epi32(_mm_castps_si128(XMM2), PM128I(PFV_0)));
		  XMM3 = _mm_castsi128_ps(_mm_cmpeq_epi32(_mm_castps_si128(XMM3), PM128I(PFV_0)));
		  XMM0 = _mm_and_ps(XMM0, XMM2);
		  XMM1 = _mm_and_ps(XMM1, XMM3);
		  XMM0 = _mm_mul_ps(XMM0, XMM0);
		  XMM1 = _mm_mul_ps(XMM1, XMM1);
		  c0   = _mm_movemask_ps(_mm_castsi128_ps(XMM2));
		  XMM2 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM5));
		  XMM6 = _mm_castsi128_ps(_mm_cmpeq_epi32(_mm_castps_si128(XMM6), PM128I(PFV_0)));
		  XMM0 = _mm_add_ps(XMM0, XMM1);
		  XMM1 = _mm_load_ps(in+j+16);
		  c1   = _mm_movemask_ps(_mm_castsi128_ps(XMM3));
		  XMM3 = _mm_load_ps(in+j+20);
		  XMM2 = _mm_castsi128_ps(_mm_cmpeq_epi32(_mm_castps_si128(XMM2), PM128I(PFV_0)));
		  XMM4 = _mm_and_ps(XMM4, XMM6);
		  XMM5 = _mm_and_ps(XMM5, XMM2);
		  energy_loss  = bitCountTable[c0];
		  energy_loss += bitCountTable[c1];
		  XMM4 = _mm_mul_ps(XMM4, XMM4);
		  XMM5 = _mm_mul_ps(XMM5, XMM5);
		  c0   = _mm_movemask_ps(_mm_castsi128_ps(XMM6));
		  XMM6 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM1));
		  XMM4 = _mm_add_ps(XMM4, XMM5);
		  XMM5 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM3));
		  c1   = _mm_movemask_ps(_mm_castsi128_ps(XMM2));
		  XMM2 = _mm_load_ps(in+j+24);
		  energy_loss += bitCountTable[c0];
		  XMM6 = _mm_castsi128_ps(_mm_cmpeq_epi32(_mm_castps_si128(XMM6), PM128I(PFV_0)));
		  XMM5 = _mm_castsi128_ps(_mm_cmpeq_epi32(_mm_castps_si128(XMM5), PM128I(PFV_0)));
		  XMM0 = _mm_add_ps(XMM0, XMM4);
		  XMM4 = _mm_load_ps(in+j+28);
		  energy_loss += bitCountTable[c1];
		  XMM1 = _mm_and_ps(XMM1, XMM6);
		  XMM3 = _mm_and_ps(XMM3, XMM5);
		  XMM1 = _mm_mul_ps(XMM1, XMM1);
		  XMM3 = _mm_mul_ps(XMM3, XMM3);
		  c0   = _mm_movemask_ps(_mm_castsi128_ps(XMM6));
		  XMM6 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM2));
		  XMM1 = _mm_add_ps(XMM1, XMM3);
		  XMM3 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM4));
		  c1   = _mm_movemask_ps(_mm_castsi128_ps(XMM5));
		  energy_loss += bitCountTable[c0];
		  XMM6 = _mm_castsi128_ps(_mm_cmpeq_epi32(_mm_castps_si128(XMM6), PM128I(PFV_0)));
		  XMM3 = _mm_castsi128_ps(_mm_cmpeq_epi32(_mm_castps_si128(XMM3), PM128I(PFV_0)));
		  XMM0 = _mm_add_ps(XMM0, XMM1);
		  energy_loss += bitCountTable[c1];
		  XMM2 = _mm_and_ps(XMM2, XMM6);
		  XMM4 = _mm_and_ps(XMM4, XMM3);
		  XMM2 = _mm_mul_ps(XMM2, XMM2);
		  XMM4 = _mm_mul_ps(XMM4, XMM4);
		  c0   = _mm_movemask_ps(_mm_castsi128_ps(XMM6));
		  XMM2 = _mm_add_ps(XMM2, XMM4);
		  c1   = _mm_movemask_ps(_mm_castsi128_ps(XMM3));
		  energy_loss += bitCountTable[c0];
		  XMM0 = _mm_add_ps(XMM0, XMM2);
		  energy_loss += bitCountTable[c1];
		  acc = _mm_add_horz(XMM0);
#else
		  __m128 XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6;
		  XMM0 = _mm_load_ps(in+j   );
		  XMM1 = _mm_load_ps(in+j+ 4);
		  XMM6 = _mm_load_ps(in+j+ 8);
		  XMM4 = XMM0;
		  XMM5 = XMM1;
		  XMM2 = _mm_cmplt_ps(PM128(PFV_M0P5), XMM0);
		  XMM3 = _mm_cmplt_ps(PM128(PFV_M0P5), XMM1);
		  XMM4 = _mm_cmplt_ps(XMM4, PM128(PFV_0P5));
		  XMM5 = _mm_cmplt_ps(XMM5, PM128(PFV_0P5));
		  XMM2 = _mm_and_ps(XMM2, XMM4);
		  XMM4 = _mm_load_ps(in+j+12);
		  XMM3 = _mm_and_ps(XMM3, XMM5);
		  XMM5 = XMM6;
		  XMM0 = _mm_and_ps(XMM0, XMM2);
		  XMM2 = XMM4;
		  XMM1 = _mm_and_ps(XMM1, XMM3);
		  XMM0 = _mm_mul_ps(XMM0, XMM0);
		  XMM1 = _mm_mul_ps(XMM1, XMM1);
		  c0   = _mm_movemask_ps(XMM2);
		  XMM0 = _mm_add_ps(XMM0, XMM1);
		  XMM1 = _mm_cmplt_ps(PM128(PFV_M0P5), XMM6);
		  c1   = _mm_movemask_ps(XMM3);
		  XMM3 = _mm_cmplt_ps(PM128(PFV_M0P5), XMM4);
		  XMM5 = _mm_cmplt_ps(XMM5, PM128(PFV_0P5));
		  XMM2 = _mm_cmplt_ps(XMM2, PM128(PFV_0P5));
		  energy_loss += bitCountTable[c0];
		  energy_loss += bitCountTable[c1];
		  XMM1 = _mm_and_ps(XMM1, XMM5);
		  XMM5 = _mm_load_ps(in+j+16);
		  XMM3 = _mm_and_ps(XMM3, XMM2);
		  XMM2 = _mm_load_ps(in+j+20);
		  XMM6 = _mm_and_ps(XMM6, XMM1);
		  XMM4 = _mm_and_ps(XMM4, XMM3);
		  XMM6 = _mm_mul_ps(XMM6, XMM6);
		  XMM4 = _mm_mul_ps(XMM4, XMM4);
		  c0   = _mm_movemask_ps(XMM1);
		  XMM1 = XMM5;
		  XMM6 = _mm_add_ps(XMM6, XMM4);
		  XMM4 = XMM2;
		  c1   = _mm_movemask_ps(XMM3);
		  XMM3 = _mm_cmplt_ps(PM128(PFV_M0P5), XMM5);
		  XMM0 = _mm_add_ps(XMM0, XMM6);
		  XMM6 = _mm_cmplt_ps(PM128(PFV_M0P5), XMM2);
		  XMM1 = _mm_cmplt_ps(XMM1, PM128(PFV_0P5));
		  XMM4 = _mm_cmplt_ps(XMM4, PM128(PFV_0P5));
		  energy_loss += bitCountTable[c0];
		  energy_loss += bitCountTable[c1];
		  XMM3 = _mm_and_ps(XMM3, XMM1);
		  XMM1 = _mm_load_ps(in+j+24);
		  XMM6 = _mm_and_ps(XMM6, XMM4);
		  XMM4 = _mm_load_ps(in+j+28);
		  XMM5 = _mm_and_ps(XMM5, XMM3);
		  XMM2 = _mm_and_ps(XMM2, XMM6);
		  XMM5 = _mm_mul_ps(XMM5, XMM5);
		  XMM2 = _mm_mul_ps(XMM2, XMM2);
		  c0   = _mm_movemask_ps(XMM3);
		  XMM3 = XMM1;
		  XMM5 = _mm_add_ps(XMM5, XMM2);
		  XMM2 = XMM4;
		  c1   = _mm_movemask_ps(XMM6);
		  XMM6 = _mm_cmplt_ps(PM128(PFV_M0P5), XMM1);
		  XMM0 = _mm_add_ps(XMM0, XMM5);
		  XMM5 = _mm_cmplt_ps(PM128(PFV_M0P5), XMM4);
		  XMM3 = _mm_cmplt_ps(XMM3, PM128(PFV_0P5));
		  XMM2 = _mm_cmplt_ps(XMM2, PM128(PFV_0P5));
		  energy_loss += bitCountTable[c0];
		  energy_loss += bitCountTable[c1];
		  XMM6 = _mm_and_ps(XMM6, XMM3);
		  XMM5 = _mm_and_ps(XMM5, XMM2);
		  XMM1 = _mm_and_ps(XMM1, XMM6);
		  XMM4 = _mm_and_ps(XMM4, XMM5);
		  XMM1 = _mm_mul_ps(XMM1, XMM1);
		  XMM4 = _mm_mul_ps(XMM4, XMM4);
		  c0   = _mm_movemask_ps(XMM6);
		  XMM1 = _mm_add_ps(XMM1, XMM4);
		  c1   = _mm_movemask_ps(XMM5);
		  energy_loss += bitCountTable[c0];
		  XMM0 = _mm_add_ps(XMM0, XMM1);
		  energy_loss += bitCountTable[c1];
		  acc = _mm_add_horz(XMM0);
#endif
		}
		else
		{
		  acc = 0.f;
		  energy_loss = 0;
		  for(i=j;i<j+partition;i++){
			if(rint(in[i])==0.f){
			  acc+=in[i]*in[i];
			  energy_loss++;
			}
		  }
		}
	  }
#else														/* SSE Optimize */
      for(i=j;i<j+partition;i++){
        if(rint(in[i])==0.f){
        	acc+=in[i]*in[i];
        	energy_loss++;
        }
      }
#endif														/* SSE Optimize */
      /* When an energy loss is large, NN processing is carried out in the middle of partition. */
      /*if(energy_loss==32 && fabs(in[freqband_mid])>nnmid_th){
      	if(in[freqband_mid]*in[freqband_mid]<.25f){
      		i=0;
      		if(acc>=vi->normal_thresh){
      			out[freqband_mid]=unitnorm(in[freqband_mid]);
      			acc-=1.;
      			freqband_flag=1;
      			nn_num++;
      		}
      	}
      }*/
      
      /* NN main */
      for(i=0;i<partition;i++){
      	k=sortedindex[i+j-start];
      	if(in[k]*in[k]>=.25f){ // or rint(in[k])!=0.f
      		out[k]=rint(in[k]);
      		//acc-=in[k]*in[k];
      	}else{
      		if(acc<vi->normal_thresh)break;
      		if(freqband_flag && freqband_mid==k)continue;
      		out[k]=unitnorm(in[k]);
      		acc-=1.;
      		nn_num++;
      	}
      }
      
      /* The minimum energy complement */
      /*min_energy=32-energy_loss+nn_num;
      if(min_energy<2 || (j<=p->min_nn_lp && min_energy==2)){
      	k=sortedindex[i+j-start];
      	if(freqband_flag && freqband_mid==k){
      		i++;
      		k=sortedindex[i+j-start];
	    }
	    if(!(fabs(in[k])<0.3)){
	    	out[k]=unitnorm(in[k]);
	    	i++;
	    }
	  }*/
	  
	  // The last process
      for(;i<partition;i++){
      	k=sortedindex[i+j-start];
      	if(freqband_flag && freqband_mid==k)continue;
      	else out[k]=0.;
      }
    }
  }
  
  for(;j<n;j++)
    out[j]=rint(in[j]);
  
}

void _vp_couple(int blobno,
		vorbis_info_psy_global *g,
		vorbis_look_psy *p,
		vorbis_info_mapping0 *vi,
		float **res,
		float **mag_memo,
		int   **mag_sort,
		int   **ifloor,
		int   *nonzero,
		int  sliding_lowpass,
#if defined(_OPENMP)
		float **mdct, float **res_org,
		int thnum, int thmax){
#else
		float **mdct, float **res_org){
#endif

  int i,j,k,n=p->n;

  /* perform any requested channel coupling */
  /* point stereo can only be used in a first stage (in this encoder)
     because of the dependency on floor lookups */
  for(i=0;i<vi->coupling_steps;i++){

    /* once we're doing multistage coupling in which a channel goes
       through more than one coupling step, the floor vector
       magnitudes will also have to be recalculated an propogated
       along with PCM.  Right now, we're not (that will wait until 5.1
       most likely), so the code isn't here yet. The memory management
       here is all assuming single depth couplings anyway. */

    /* make sure coupling a zero and a nonzero channel results in two
       nonzero channels. */
    if(nonzero[vi->coupling_mag[i]] ||
       nonzero[vi->coupling_ang[i]]){
     

      float *rM=res[vi->coupling_mag[i]];
      float *rA=res[vi->coupling_ang[i]];
      float *rMo=res_org[vi->coupling_mag[i]];
      float *rAo=res_org[vi->coupling_ang[i]];
      float *qM=rM+n;
      float *qA=rA+n;
      float *mdctM=mdct[vi->coupling_mag[i]];
      float *mdctA=mdct[vi->coupling_ang[i]];
      int *floorM=ifloor[vi->coupling_mag[i]];
      int *floorA=ifloor[vi->coupling_ang[i]];
      float prepoint=stereo_threshholds[g->coupling_prepointamp[blobno]];
      float postpoint=stereo_threshholds[g->coupling_postpointamp[blobno]];
      float sth_low=stereo_threshholds_low[g->coupling_prepointamp[blobno]];
      float sth_high=stereo_threshholds_high[g->coupling_postpointamp[blobno]];
      float postpoint_backup;
      float st_thresh;
      int partition=(p->vi->normal_point_p?p->vi->normal_partition:p->n);
      int limit=g->coupling_pointlimit[p->vi->blockflag][blobno];
      int pointlimit=limit;
      int freqlimit=p->st_freqlimit;
#ifdef	__SSE__												/* SSE Optimize */
      _MM_ALIGN16 unsigned int Mc_treshp[2048];
      _MM_ALIGN16 unsigned int Ac_treshp[2048];
      _MM_ALIGN16 float rMs[2048];
      _MM_ALIGN16 float rAs[2048];
      _MM_ALIGN16 unsigned int mdctMA[2048];
      int midpoint0	 = (limit/partition)*partition;
      int midpoint1	 = ((limit+partition-1)/partition)*partition;
	  int s, e;
#else														/* SSE Optimize */
      unsigned char Mc_treshp[2048];
      unsigned char Ac_treshp[2048];
#endif														/* SSE Optimize */
      int lof_st;
      int hif_st;
      int hif_stcopy;
      int old_lof_st=0;
      int old_hif_st=0;
      int Afreq_num=0;
      int Mfreq_num=0;
      int stcont_start=0; // M6 start point
      
#if defined(_OPENMP)
	  if(thnum==0){
#endif
      nonzero[vi->coupling_mag[i]]=1; 
      nonzero[vi->coupling_ang[i]]=1; 
#if defined(_OPENMP)
	  }
	  if(thmax==2)
	  {
		if(thnum==0)
		{
		  s = 0;
		  if(p->_vp_couple_spoint0==0)
		  {
		    int hcost = (2*p->n-midpoint1)/2;
			int cost = 0;
			e = 0;
			while(cost<hcost)
			{
			  if(e<midpoint1)
				cost += partition;
			  else
				cost += partition*2;
			  e += partition;
			}
			p->_vp_couple_spoint0 = e;
		  }else
			e = p->_vp_couple_spoint0;
		}
		else
		{
		  if(p->_vp_couple_spoint1==0)
		  {
			int hcost = (2*p->n-midpoint1)/2;
			int cost = 0;
			s = 0;
			while(cost<hcost)
			{
			  if(s<midpoint1)
				cost += partition;
			  else
				cost += partition*2;
			  s += partition;
			}
			p->_vp_couple_spoint1 = s;
		  }else
			s = p->_vp_couple_spoint1;
		  e = p->n;
		}
	  }
	  else
	  {
		s = _omp_get_start(p->n, partition, thnum, thmax);
		e = _omp_get_end(p->n, partition, thnum, thmax);
	  }
#else
	  s = 0;
	  e = p->n;
#endif
       
      postpoint_backup=postpoint;
      
      /** @ M6 PRE **/
      // lossless only?
#ifdef	__SSE__												/* SSE Optimize */
	  for(j=0;j<e;j+=16)
	  {
		__m128 XMM0, XMM1, XMM2, XMM3, XMM4, XMM5;
		XMM0 = _mm_load_ps(mdctM+j   );
		XMM2 = _mm_load_ps(mdctA+j   );
		XMM1 = _mm_load_ps(mdctM+j+ 4);
		XMM3 = _mm_load_ps(mdctA+j+ 4);
		XMM4 = _mm_load_ps(mdctM+j+ 8);
		XMM5 = _mm_load_ps(mdctA+j+ 8);
		XMM0 = _mm_mul_ps(XMM0, XMM2);
		XMM1 = _mm_mul_ps(XMM1, XMM3);
		XMM3 = _mm_load_ps(mdctA+j+12);
		XMM2 = _mm_load_ps(mdctM+j+12);
		XMM4 = _mm_mul_ps(XMM4, XMM5);
		XMM3 = _mm_mul_ps(XMM3, XMM2);
		XMM5 = _mm_load_ps(rMo+j   );
		XMM2 = _mm_load_ps(rMo+j+ 4);
		XMM0 = _mm_cmplt_ps(XMM0, PM128(PFV_0));
		XMM1 = _mm_cmplt_ps(XMM1, PM128(PFV_0));
		XMM4 = _mm_cmplt_ps(XMM4, PM128(PFV_0));
		XMM3 = _mm_cmplt_ps(XMM3, PM128(PFV_0));
		_mm_store_ps(mdctMA+j   , XMM0);
		XMM0 = _mm_load_ps(rMo+j+ 8);
		_mm_store_ps(mdctMA+j+ 4, XMM1);
		XMM1 = _mm_load_ps(rMo+j+12);
		_mm_store_ps(mdctMA+j+ 8, XMM4);
		_mm_store_ps(mdctMA+j+12, XMM3);
		XMM5 = _mm_and_ps(XMM5, PM128(PABSMASK));
		XMM2 = _mm_and_ps(XMM2, PM128(PABSMASK));
		XMM0 = _mm_and_ps(XMM0, PM128(PABSMASK));
		XMM1 = _mm_and_ps(XMM1, PM128(PABSMASK));
		_mm_store_ps(rMs+j   , XMM5);
		_mm_store_ps(rMs+j+ 4, XMM2);
		_mm_store_ps(rMs+j+ 8, XMM0);
		_mm_store_ps(rMs+j+12, XMM1);
		XMM5 = _mm_load_ps(rAo+j   );
		XMM2 = _mm_load_ps(rAo+j+ 4);
		XMM0 = _mm_load_ps(rAo+j+ 8);
		XMM1 = _mm_load_ps(rAo+j+12);
		XMM5 = _mm_and_ps(XMM5, PM128(PABSMASK));
		XMM2 = _mm_and_ps(XMM2, PM128(PABSMASK));
		XMM0 = _mm_and_ps(XMM0, PM128(PABSMASK));
		XMM1 = _mm_and_ps(XMM1, PM128(PABSMASK));
		_mm_store_ps(rAs+j   , XMM5);
		_mm_store_ps(rAs+j+ 4, XMM2);
		_mm_store_ps(rAs+j+ 8, XMM0);
		_mm_store_ps(rAs+j+12, XMM1);
	  }
	  for(;j<n;j+=16)
	  {
		__m128 XMM0, XMM1, XMM2, XMM3, XMM4, XMM5;
		XMM0 = _mm_load_ps(mdctM+j   );
		XMM2 = _mm_load_ps(mdctA+j   );
		XMM1 = _mm_load_ps(mdctM+j+ 4);
		XMM3 = _mm_load_ps(mdctA+j+ 4);
		XMM4 = _mm_load_ps(mdctM+j+ 8);
		XMM5 = _mm_load_ps(mdctA+j+ 8);
		XMM0 = _mm_mul_ps(XMM0, XMM2);
		XMM1 = _mm_mul_ps(XMM1, XMM3);
		XMM3 = _mm_load_ps(mdctA+j+12);
		XMM2 = _mm_load_ps(mdctM+j+12);
		XMM4 = _mm_mul_ps(XMM4, XMM5);
		XMM3 = _mm_mul_ps(XMM3, XMM2);
		XMM0 = _mm_cmplt_ps(XMM0, PM128(PFV_0));
		XMM1 = _mm_cmplt_ps(XMM1, PM128(PFV_0));
		XMM4 = _mm_cmplt_ps(XMM4, PM128(PFV_0));
		XMM3 = _mm_cmplt_ps(XMM3, PM128(PFV_0));
		_mm_store_ps(mdctMA+j   , XMM0);
		_mm_store_ps(mdctMA+j+ 4, XMM1);
		_mm_store_ps(mdctMA+j+ 8, XMM4);
		_mm_store_ps(mdctMA+j+12, XMM3);
	  }
	  if(!stereo_threshholds[g->coupling_postpointamp[blobno]])stcont_start=n;
	  else{
		static _MM_ALIGN16 unsigned int PUI1[4] = { 1, 1, 1, 1};
		int freqlimit16 = freqlimit&(~15);
		__m128 PST_THRESH;
		// exception handling
		if((postpoint-sth_high)<prepoint)sth_high=postpoint-prepoint;
		// start point setup
		for(j=0;j<n;j++){
		  stcont_start=j;
		  if(p->noiseoffset[1][j]>=-2)break;
		}
		// start point correction & threshold setup 
		st_thresh=.1;
		if(p->m_val<.5){
		  // low frequency limit
		  if(stcont_start<limit)stcont_start=limit;
		}else if(p->vi->normal_thresh>1.)st_thresh=.5;
		PST_THRESH = _mm_set_ps1(st_thresh);
		for(j=0;j<freqlimit16;j+=16){
		  __m128 XMM0, XMM1, XMM2, XMM3;
		  XMM0 = _mm_load_ps(rM+j   );
		  XMM1 = _mm_load_ps(rM+j+ 4);
		  XMM2 = _mm_load_ps(rM+j+ 8);
		  XMM3 = _mm_load_ps(rM+j+12);
		  XMM0 = _mm_and_ps(XMM0, PM128(PABSMASK));
		  XMM1 = _mm_and_ps(XMM1, PM128(PABSMASK));
		  XMM2 = _mm_and_ps(XMM2, PM128(PABSMASK));
		  XMM3 = _mm_and_ps(XMM3, PM128(PABSMASK));
		  XMM0 = _mm_cmplt_ps(XMM0, PST_THRESH);
		  XMM1 = _mm_cmplt_ps(XMM1, PST_THRESH);
		  XMM2 = _mm_cmplt_ps(XMM2, PST_THRESH);
		  XMM3 = _mm_cmplt_ps(XMM3, PST_THRESH);
		  XMM0 = _mm_and_ps(XMM0, PM128(PUI1));
		  XMM1 = _mm_and_ps(XMM1, PM128(PUI1));
		  XMM2 = _mm_and_ps(XMM2, PM128(PUI1));
		  XMM3 = _mm_and_ps(XMM3, PM128(PUI1));
		  _mm_store_ps(Mc_treshp+j   , XMM0);
		  _mm_store_ps(Mc_treshp+j+ 4, XMM1);
		  _mm_store_ps(Mc_treshp+j+ 8, XMM2);
		  _mm_store_ps(Mc_treshp+j+12, XMM3);
		  XMM0 = _mm_load_ps(rA+j   );
		  XMM1 = _mm_load_ps(rA+j+ 4);
		  XMM2 = _mm_load_ps(rA+j+ 8);
		  XMM3 = _mm_load_ps(rA+j+12);
		  XMM0 = _mm_and_ps(XMM0, PM128(PABSMASK));
		  XMM1 = _mm_and_ps(XMM1, PM128(PABSMASK));
		  XMM2 = _mm_and_ps(XMM2, PM128(PABSMASK));
		  XMM3 = _mm_and_ps(XMM3, PM128(PABSMASK));
		  XMM0 = _mm_cmplt_ps(XMM0, PST_THRESH);
		  XMM1 = _mm_cmplt_ps(XMM1, PST_THRESH);
		  XMM2 = _mm_cmplt_ps(XMM2, PST_THRESH);
		  XMM3 = _mm_cmplt_ps(XMM3, PST_THRESH);
		  XMM0 = _mm_and_ps(XMM0, PM128(PUI1));
		  XMM1 = _mm_and_ps(XMM1, PM128(PUI1));
		  XMM2 = _mm_and_ps(XMM2, PM128(PUI1));
		  XMM3 = _mm_and_ps(XMM3, PM128(PUI1));
		  _mm_store_ps(Ac_treshp+j   , XMM0);
		  _mm_store_ps(Ac_treshp+j+ 4, XMM1);
		  _mm_store_ps(Ac_treshp+j+ 8, XMM2);
		  _mm_store_ps(Ac_treshp+j+12, XMM3);
		}
		for(;j<=freqlimit;j++){ // or j<n
		  if(fabs(rM[j])<st_thresh)Mc_treshp[j]=1;
		  else Mc_treshp[j]=0;
		  if(fabs(rA[j])<st_thresh)Ac_treshp[j]=1;
		  else Ac_treshp[j]=0;
		}
	  }
	  if(n<=sliding_lowpass&&p->vi->normal_point_p&&partition%8==0)
	  {
		static _MM_ALIGN16 const float PP001[4]	 = {0.001f, 0.001f, 0.001f, 0.001f};
		static _MM_ALIGN16 const float P1000[4]	 = {1000.f, 1000.f, 1000.f, 1000.f};
		__m128	PPOSTPOINT_BACKUP	 = _mm_set_ps1(postpoint_backup);
		__m128	PDUMMYPOINT		 = 
			_mm_set_ps1(stereo_threshholds_rephase[g->coupling_postpointamp[blobno]]);
		_MM_ALIGN16 float slowM[2048];
		_MM_ALIGN16 float slowA[2048];
		_MM_ALIGN16 float shigh[2048];
		int	midpoint0	 = (limit/partition)*partition;
		int	midpoint1	 = ((limit+partition-1)/partition)*partition;
		for(j=0;j<e;j+=partition){
		  float rpacc;
		  int energy_loss=0;
		  int nn_num=0;

		  for(k=0;k<partition;k++){
			int l=k+j;
			float slow=0.f;
			float shighM=0.f;
			float shighA=0.f;

			slowM[l] = prepoint;
			slowA[l] = prepoint;
			shigh[l] = 0.f;

			postpoint=postpoint_backup;

			/* AoTuV */
			/** @ M6 MAIN **
			The threshold of a stereo is changed dynamically. 
			by Aoyumi @ 2006/06/04
			*/
			if(l>=stcont_start){
			  int m;
			  int lof_num;
			  int hif_num;

			  // (It may be better to calculate this in advance) 
			  lof_st=l-(l/2)*.167;
			  hif_st=l+l*.167;

			  hif_stcopy=hif_st;

			  // limit setting
			  if(hif_st>freqlimit)hif_st=freqlimit;

			  if(old_lof_st || old_hif_st){
				if(hif_st>l){
				  // hif_st, lof_st ...absolute value
				  // lof_num, hif_num ...relative value

				  // low freq.(lower)
				  lof_num=lof_st-old_lof_st;
				  if(lof_num==0){
					Afreq_num+=Ac_treshp[l-1];
					Mfreq_num+=Mc_treshp[l-1];
				  }else if(lof_num==1){
					Afreq_num+=Ac_treshp[l-1];
					Mfreq_num+=Mc_treshp[l-1];
					Afreq_num-=Ac_treshp[old_lof_st];
					Mfreq_num-=Mc_treshp[old_lof_st];
				  }//else puts("err. low");

				  // high freq.(higher)
				  hif_num=hif_st-old_hif_st;
				  if(hif_num==0){
					Afreq_num-=Ac_treshp[l];
					Mfreq_num-=Mc_treshp[l];
				  }else if(hif_num==1){
					Afreq_num-=Ac_treshp[l];
					Mfreq_num-=Mc_treshp[l];
					Afreq_num+=Ac_treshp[hif_st];
					Mfreq_num+=Mc_treshp[hif_st];
				  }else if(hif_num==2){
					Afreq_num-=Ac_treshp[l];
					Mfreq_num-=Mc_treshp[l];
					Afreq_num+=Ac_treshp[hif_st];
					Mfreq_num+=Mc_treshp[hif_st];
					Afreq_num+=Ac_treshp[hif_st-1];
					Mfreq_num+=Mc_treshp[hif_st-1];
				  }//else puts("err. high");
				}
			  }else{
				for(m=lof_st; m<=hif_st; m++){
				  if(m==l)continue;
				  if(Ac_treshp[m]) Afreq_num++;
				  if(Mc_treshp[m]) Mfreq_num++;
				}
			  }
			  if(l>=limit){
				shigh[l]=sth_high/(hif_stcopy-lof_st);
				shighA=shigh[l]*Afreq_num;
				shighM=shigh[l]*Mfreq_num;
				if((shighA+rAs[l])>(shighM+rMs[l]))shigh[l]=shighA;
				else shigh[l]=shighM;
			  }else{
				slow=sth_low/(hif_stcopy-lof_st);
				slowA[l]=slow*Afreq_num;
				slowM[l]=slow*Mfreq_num;
				if(p->noiseoffset[1][l]<-1){
				  slowA[l]*=(p->noiseoffset[1][l]+2);
				  slowM[l]*=(p->noiseoffset[1][l]+2);
				}
				slowA[l] = prepoint - slowA[l];
				slowM[l] = prepoint - slowM[l];
			  }
			  old_lof_st=lof_st;
			  old_hif_st=hif_st;
			}
		  }
		}

		/* Phase 0 */
		if(s<midpoint0)
		{
		  int te;
		  if(e>=midpoint0)
			te = midpoint0;
		  else
			te = e;
		  for(j=s;j<te;j+=partition){
			int energy_loss=0;
			for(k=0;k<partition;k+=4)
			{
			  int l	 = k+j;
			  int ifc0, m, o;
			  __m128 XMM0, XMM1, XMM2, XMM3;
			  XMM0 = _mm_load_ps(rMs+l  );
			  XMM2 = _mm_load_ps(slowM+l  );
			  XMM1 = _mm_load_ps(rAs+l  );
			  XMM3 = _mm_load_ps(slowA+l  );
			  XMM0 = _mm_cmplt_ps(XMM0, XMM2);
			  XMM1 = _mm_cmplt_ps(XMM1, XMM3);
			  XMM1 = _mm_and_ps(XMM1, XMM0);
			  ifc0 = _mm_movemask_ps(XMM1);
			  if(ifc0==0)
			  {
				couple_lossless_ps(rM+l, rA+l, qM+l, qA+l);
				l += 4;
			  }
			  else if(ifc0==0xF)
			  {
				precomputed_couple_point_ps(&mag_memo[i][l],
				  floorM+l,floorA+l,
				  qM+l,qA+l);
				XMM0 = _mm_load_ps(qM+l);
#if defined(__SSE2__)
				XMM0 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM0));
				XMM0 = _mm_castsi128_ps(_mm_cmpeq_epi32(XMM0, PM128I(PFV_0)));
#else
				XMM1 = _mm_cmplt_ps(PM128(PFV_M0P5), XMM0);
				XMM0 = _mm_cmplt_ps(XMM0, PM128(PFV_0P5));
				XMM0 = _mm_and_ps(XMM0, XMM1);
#endif
				energy_loss += bitCountTable[_mm_movemask_ps(XMM0)];
				l += 4;
			  }
			  else
			  {
				for(m=0,o=1;m<4;m++)
				{
				  if(ifc0&o)
					precomputed_couple_point(mag_memo[i][l],
					  floorM[l],floorA[l],
					  qM+l,qA+l);
				  else
					couple_lossless(rM[l],rA[l],qM+l,qA+l);
				  l ++;
				  o = o << 1;
				}
				XMM0 = _mm_load_ps(qM+l-4);
#if defined(__SSE2__)
				XMM0 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM0));
				XMM0 = _mm_castsi128_ps(_mm_cmpeq_epi32(XMM0, PM128I(PFV_0)));
#else
				XMM2 = _mm_cmplt_ps(PM128(PFV_M0P5), XMM0);
				XMM0 = _mm_cmplt_ps(XMM0, PM128(PFV_0P5));
				XMM0 = _mm_and_ps(XMM0, XMM2);
#endif
				XMM0 = _mm_and_ps(XMM0, XMM1);
				energy_loss += bitCountTable[_mm_movemask_ps(XMM0)];
			  }
			}
			{
			  int min_energy = 32-energy_loss;
			  if(min_energy<2 || (j<=p->min_nn_lp && min_energy==2)){
				int l;
				float ab;
				for(;k<partition;k++){
				  l=mag_sort[i][j+k];
				  ab=fabs(qM[l]);
				  if(ab<0.04)break;
#if	1
				  if( ((mdctM[l]>0.) && (mdctA[l]<0.)) || ((mdctA[l]>0.) && (mdctM[l]<0.))
					&& ab<0.11)break; // 0.11
#else
				  if(mdctMA[l] && ab < 0.11)break;
#endif
				  if( l>=pointlimit){
					__m128 XMM0 = _mm_load_ss(qM+l);
					if(_mm_cvtss_si32(XMM0)==0){
					  qM[l]=unitnorm(qM[l]);
					  break;
					}
				  }
				}
			  }
			}
		  }
		}
		/* Phase 1 */
		if(s<=midpoint0&&e>=midpoint1)
		{
		  for(j=midpoint0;j<midpoint1;j+=partition)
		  {
			__m128	PACC;
			int midpoint033 = (limit-midpoint0)&(~3);
			int midpoint066 = (limit-midpoint0+3)&(~3);
			float acc=0.f;
			float rpacc;
			int energy_loss=0;
			int nn_num=0;

			for(k=0;k<midpoint033;k+=4)
			{
			  int l	 = k+j;
			  int ifc0, m, o;
			  __m128 XMM0, XMM1, XMM2, XMM3;
			  XMM0 = _mm_load_ps(rMs+l  );
			  XMM2 = _mm_load_ps(slowM+l  );
			  XMM1 = _mm_load_ps(rAs+l  );
			  XMM3 = _mm_load_ps(slowA+l  );
			  XMM0 = _mm_cmplt_ps(XMM0, XMM2);
			  XMM1 = _mm_cmplt_ps(XMM1, XMM3);
			  XMM1 = _mm_and_ps(XMM1, XMM0);
			  ifc0 = _mm_movemask_ps(XMM1);
			  if(ifc0==0)
			  {
				couple_lossless_ps(rM+l, rA+l, qM+l, qA+l);
				l += 4;
			  }
			  else if(ifc0==0xF)
			  {
				precomputed_couple_point_ps(&mag_memo[i][l],
				  floorM+l,floorA+l,
				  qM+l,qA+l);
				XMM0 = _mm_load_ps(qM+l);
#if defined(__SSE2__)
				XMM0 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM0));
				XMM0 = _mm_castsi128_ps(_mm_cmpeq_epi32(XMM0, PM128I(PFV_0)));
#else
				XMM2 = _mm_cmplt_ps(PM128(PFV_M0P5), XMM0);
				XMM0 = _mm_cmplt_ps(XMM0, PM128(PFV_0P5));
				XMM0 = _mm_and_ps(XMM0, XMM2);
#endif
				energy_loss += bitCountTable[_mm_movemask_ps(XMM0)];
				l += 4;
			  }
			  else
			  {
				for(m=0,o=1;m<4;m++)
				{
				  if(ifc0&o)
					precomputed_couple_point(mag_memo[i][l],
					  floorM[l],floorA[l],
					  qM+l,qA+l);
				  else
					couple_lossless(rM[l],rA[l],qM+l,qA+l);
				  l ++;
				  o = o << 1;
				}
				XMM0 = _mm_load_ps(qM+l-4);
#if defined(__SSE2__)
				XMM0 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM0));
				XMM0 = _mm_castsi128_ps(_mm_cmpeq_epi32(XMM0, PM128I(PFV_0)));
#else
				XMM2 = _mm_cmplt_ps(PM128(PFV_M0P5), XMM0);
				XMM0 = _mm_cmplt_ps(XMM0, PM128(PFV_0P5));
				XMM0 = _mm_and_ps(XMM0, XMM2);
#endif
				XMM0 = _mm_and_ps(XMM0, XMM1);
				energy_loss += bitCountTable[_mm_movemask_ps(XMM0)];
			  }
			}
			for(;k<midpoint066;k++)
			{
			  int l=k+j;
			  float a=mdctM[l];
			  float b=mdctA[l];
			  float dummypoint;
			  float hypot_reserve;

			  postpoint=postpoint_backup;

			  if(l>=limit){
				postpoint-=shigh[l];
				/* The following prevents an extreme reduction of residue. (2ch stereo only) */
				if(mdctMA[l]){
				  hypot_reserve = fabs(fabs(a)-fabs(b));
				  if(hypot_reserve < 0.001){ // 0~0.000999-
					dummypoint = stereo_threshholds_rephase[g->coupling_postpointamp[blobno]];
					dummypoint = dummypoint+((postpoint-dummypoint)*(hypot_reserve*1000));
					if(postpoint > dummypoint) postpoint = dummypoint;
				  }
				}
			  }

			  if((l>=limit && rMs[l]<postpoint && rAs[l]<postpoint) ||
				(rMs[l]<slowM[l] && rAs[l]<slowA[l])){

				  __m128 XMM0;
				  if(l>=0&&l<=n)
				  {
					precomputed_couple_point(mag_memo[i][l],
					  floorM[l],floorA[l],
					  qM+l,qA+l);
				  }
				  //if(rint(qM[l])==0.f)acc+=qM[l]*qM[l];
				  XMM0 = _mm_load_ss(qM+l);
				  if(_mm_cvtss_si32(XMM0)==0){
					energy_loss++;
					if(l>=limit)acc+=qM[l]*qM[l];
				  }
			  }else{
				couple_lossless(rM[l],rA[l],qM+l,qA+l);
			  }
			}
			PACC	 = _mm_set_ss(acc);
			for(;k<partition;k+=4)
			{
			  int l	 = k+j;
			  int ifc0, m, o;
			  __m128 XMM0, XMM1, XMM2, XMM3, XMM4, XMM5;
			  XMM3 = PPOSTPOINT_BACKUP;
			  XMM4 = _mm_load_ps(shigh+l  );
			  XMM0 = _mm_load_ps(mdctM+l  );
			  XMM1 = _mm_load_ps(mdctA+l  );
			  XMM2 = _mm_load_ps(mdctMA+l  );
			  XMM3 = _mm_sub_ps(XMM3, XMM4);	/* postpoint */
			  if(_mm_movemask_ps(XMM2)!=0)
					{
					  XMM5 = XMM3;								/* copy of postpoint */
					  XMM0 = _mm_and_ps(XMM0, PM128(PABSMASK));
					  XMM1 = _mm_and_ps(XMM1, PM128(PABSMASK));
					  XMM0 = _mm_sub_ps(XMM0, XMM1);
					  XMM0 = _mm_and_ps(XMM0, PM128(PABSMASK));	/* hypot_reserve */
					  XMM1 = _mm_cmplt_ps(XMM0, PM128(PP001));	/* Mask of hypot_reserve */
					  XMM0 = _mm_mul_ps(XMM0, PM128(P1000));
					  XMM5 = _mm_sub_ps(XMM5, PDUMMYPOINT);
					  XMM0 = _mm_mul_ps(XMM0, XMM5);
					  XMM0 = _mm_add_ps(XMM0, PDUMMYPOINT);		/* dummypoint */
					  XMM1 = _mm_and_ps(XMM1, XMM2);
					  XMM0 = _mm_min_ps(XMM0, XMM3);
					  XMM0 = _mm_or_ps(
						_mm_and_ps(XMM0, XMM1),
						_mm_andnot_ps(XMM1, XMM3)
						);											/* postpoint */
					}
			  else
				XMM0 = XMM3;
			  XMM3 = _mm_load_ps(slowM+l  );
			  XMM4 = _mm_load_ps(slowA+l  );
			  XMM1 = _mm_load_ps(rMs+l  );
			  XMM2 = _mm_load_ps(rAs+l  );
			  XMM3 = _mm_max_ps(XMM3, XMM0);
			  XMM4 = _mm_max_ps(XMM4, XMM0);
			  XMM1 = _mm_cmplt_ps(XMM1, XMM3);
			  XMM2 = _mm_cmplt_ps(XMM2, XMM4);
			  XMM1 = _mm_and_ps(XMM1, XMM2);
			  ifc0 = _mm_movemask_ps(XMM1);
			  if(ifc0==0)
			  {
				couple_lossless_ps(rM+l, rA+l, qM+l, qA+l);
				l += 4;
			  }
			  else if(ifc0==0xF)
			  {
				precomputed_couple_point_ps(&mag_memo[i][l],
				  floorM+l,floorA+l,
				  qM+l,qA+l);
				XMM0 = _mm_load_ps(qM+l);
				XMM2 = XMM0;
#if defined(__SSE2__)
				XMM0 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM0));
				XMM0 = _mm_castsi128_ps(_mm_cmpeq_epi32(XMM0, PM128I(PFV_0)));
#else
				XMM3 = _mm_cmplt_ps(PM128(PFV_M0P5), XMM0);
				XMM0 = _mm_cmplt_ps(XMM0, PM128(PFV_0P5));
				XMM0 = _mm_and_ps(XMM0, XMM3);
#endif
				XMM2 = _mm_and_ps(XMM2, XMM0);
				energy_loss += bitCountTable[_mm_movemask_ps(XMM0)];
				XMM2 = _mm_mul_ps(XMM2, XMM2);
				PACC = _mm_add_ps(PACC, XMM2);
				l += 4;
			  }
			  else
			  {
				for(m=0,o=1;m<4;m++)
				{
				  if(ifc0&o)
					precomputed_couple_point(mag_memo[i][l],
					  floorM[l],floorA[l],
					  qM+l,qA+l);
				  else
					couple_lossless(rM[l],rA[l],qM+l,qA+l);
				  l ++;
				  o = o << 1;
				}
				XMM0 = _mm_load_ps(qM+l-4);
				XMM2 = XMM0;
#if defined(__SSE2__)
				XMM0 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM0));
				XMM0 = _mm_castsi128_ps(_mm_cmpeq_epi32(XMM0, PM128I(PFV_0)));
#else
				XMM3 = _mm_cmplt_ps(PM128(PFV_M0P5), XMM0);
				XMM0 = _mm_cmplt_ps(XMM0, PM128(PFV_0P5));
				XMM0 = _mm_and_ps(XMM0, XMM3);
#endif
				XMM2 = _mm_and_ps(XMM2, XMM0);
				XMM2 = _mm_and_ps(XMM2, XMM1);
				energy_loss += bitCountTable[_mm_movemask_ps(XMM0)];
				XMM2 = _mm_mul_ps(XMM2, XMM2);
				PACC = _mm_add_ps(PACC, XMM2);
			  }
			}
			acc = _mm_add_horz(PACC);
			{
			  int freqband_mid=j+16;
			  int freqband_flag=0;
			  int min_energy;

			  rpacc=acc;
			  /* When the energy loss of a partition is large, NN is performed in the middle of partition.
			  for 48/44.1/32kHz */
			  if(energy_loss==32 && fabs(qM[freqband_mid])>nnmid_th && acc>=p->vi->normal_thresh
				&& freqband_mid>=pointlimit){
				  __m128 XMM0;
				  XMM0 = _mm_load_ss(qM+freqband_mid);
				  if(_mm_cvtss_si32(XMM0)==0){
					if(mdctMA[freqband_mid]){
					  acc-=1.f;
					  rpacc-=1.32;
					}else{
					  acc-=1.f;
					  rpacc-=1.f;
					}
					qM[freqband_mid]=unitnorm(qM[freqband_mid]);
					freqband_flag=1;
					nn_num++;
				  }
			  }
			  /* NN main (point stereo) */
			  for(k=0;k<partition && acc>=p->vi->normal_thresh;k++){
				int l;
				l=mag_sort[i][j+k];
				if(freqband_mid==l && freqband_flag)continue;
				if(l>=pointlimit){
				  __m128 XMM0 = _mm_load_ss(qM+l);
				  if(_mm_cvtss_si32(XMM0)==0){
					if(mdctMA[l]){
					  if(rpacc<p->vi->normal_thresh)continue;
					  acc-=1.f;
					  rpacc-=1.32;
					}else{
					  acc-=1.f;
					  rpacc-=1.f;
					}
					qM[l]=unitnorm(qM[l]);
					nn_num++;
				  }
				}
			  }
			  /* The minimum energy complement.
			  for 48/44.1/32kHz */
			  min_energy=32-energy_loss+nn_num;
			  if(min_energy<2 || (j<=p->min_nn_lp && min_energy==2)){
				int l;
				float ab;
				for(;k<partition;k++){
				  l=mag_sort[i][j+k];
				  ab=fabs(qM[l]);
				  if(ab<0.04)break;
#if	1
				  if( ((mdctM[l]>0.) && (mdctA[l]<0.)) || ((mdctA[l]>0.) && (mdctM[l]<0.))
					&& ab<0.11)break; // 0.11
#else
				  if(mdctMA[l] && ab < 0.11)break;
#endif
				  if(l>=pointlimit){
					__m128 XMM0 = _mm_load_ss(qM+l);
					if(_mm_cvtss_si32(XMM0)==0){
					  qM[l]=unitnorm(qM[l]);
					  break;
					}
				  }
				}
			  }
			}
		  }
		}
		/* Phase 2 */
		if(e>midpoint1)
		{
		  int ts;
		  if(s<midpoint1)
			ts = midpoint1;
		  else
			ts = s;
		  for(j=ts;j<e;j+=partition){
			float acc=0.f;
			float rpacc;
			int energy_loss=0;
			int nn_num=0;
			__m128	PACC	 = _mm_setzero_ps();

			for(k=0;k<partition;k+=4)
			{
			  int l	 = k+j;
			  int ifc0, m, o;
			  __m128 XMM0, XMM1, XMM2, XMM3, XMM4, XMM5;
			  XMM3 = PPOSTPOINT_BACKUP;
			  XMM4 = _mm_load_ps(shigh+l  );
			  XMM0 = _mm_load_ps(mdctM+l  );
			  XMM1 = _mm_load_ps(mdctA+l  );
			  XMM2 = _mm_load_ps(mdctMA+l  );
			  XMM3 = _mm_sub_ps(XMM3, XMM4);	/* postpoint */
			  if(_mm_movemask_ps(XMM2)!=0)
					{
					  XMM5 = XMM3;								/* copy of postpoint */
					  XMM0 = _mm_and_ps(XMM0, PM128(PABSMASK));
					  XMM1 = _mm_and_ps(XMM1, PM128(PABSMASK));
					  XMM0 = _mm_sub_ps(XMM0, XMM1);
					  XMM0 = _mm_and_ps(XMM0, PM128(PABSMASK));	/* hypot_reserve */
					  XMM1 = _mm_cmplt_ps(XMM0, PM128(PP001));	/* Mask of hypot_reserve */
					  XMM0 = _mm_mul_ps(XMM0, PM128(P1000));
					  XMM5 = _mm_sub_ps(XMM5, PDUMMYPOINT);
					  XMM0 = _mm_mul_ps(XMM0, XMM5);
					  XMM0 = _mm_add_ps(XMM0, PDUMMYPOINT);		/* dummypoint */
					  XMM1 = _mm_and_ps(XMM1, XMM2);
					  XMM0 = _mm_min_ps(XMM0, XMM3);
					  XMM0 = _mm_or_ps(
						_mm_and_ps(XMM0, XMM1),
						_mm_andnot_ps(XMM1, XMM3)
						);											/* postpoint */
					}
			  else
				XMM0 = XMM3;
			  XMM3 = _mm_load_ps(slowM+l  );
			  XMM4 = _mm_load_ps(slowA+l  );
			  XMM1 = _mm_load_ps(rMs+l  );
			  XMM2 = _mm_load_ps(rAs+l  );
			  XMM3 = _mm_max_ps(XMM3, XMM0);
			  XMM4 = _mm_max_ps(XMM4, XMM0);
			  XMM1 = _mm_cmplt_ps(XMM1, XMM3);
			  XMM2 = _mm_cmplt_ps(XMM2, XMM4);
			  XMM1 = _mm_and_ps(XMM1, XMM2);
			  ifc0 = _mm_movemask_ps(XMM1);
			  if(ifc0==0)
			  {
				couple_lossless_ps(rM+l, rA+l, qM+l, qA+l);
				l += 4;
			  }
			  else if(ifc0==0xF)
			  {
				precomputed_couple_point_ps(&mag_memo[i][l],
				  floorM+l,floorA+l,
				  qM+l,qA+l);
				XMM0 = _mm_load_ps(qM+l);
				XMM2 = XMM0;
#if defined(__SSE2__)
				XMM0 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM0));
				XMM0 = _mm_castsi128_ps(_mm_cmpeq_epi32(XMM0, PM128I(PFV_0)));
#else
				XMM3 = _mm_cmplt_ps(PM128(PFV_M0P5), XMM0);
				XMM0 = _mm_cmplt_ps(XMM0, PM128(PFV_0P5));
				XMM0 = _mm_and_ps(XMM0, XMM3);
#endif
				XMM2 = _mm_and_ps(XMM2, XMM0);
				energy_loss += bitCountTable[_mm_movemask_ps(XMM0)];
				XMM2 = _mm_mul_ps(XMM2, XMM2);
				PACC = _mm_add_ps(PACC, XMM2);
				l += 4;
			  }
			  else
			  {
				for(m=0,o=1;m<4;m++)
				{
				  if(ifc0&o)
					precomputed_couple_point(mag_memo[i][l],
					  floorM[l],floorA[l],
					  qM+l,qA+l);
				  else
					couple_lossless(rM[l],rA[l],qM+l,qA+l);
				  l ++;
				  o = o << 1;
				}
				XMM0 = _mm_load_ps(qM+l-4);
				XMM2 = XMM0;
#if defined(__SSE2__)
				XMM0 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM0));
				XMM0 = _mm_castsi128_ps(_mm_cmpeq_epi32(XMM0, PM128I(PFV_0)));
#else
				XMM3 = _mm_cmplt_ps(PM128(PFV_M0P5), XMM0);
				XMM0 = _mm_cmplt_ps(XMM0, PM128(PFV_0P5));
				XMM0 = _mm_and_ps(XMM0, XMM3);
#endif
				XMM2 = _mm_and_ps(XMM2, XMM0);
				XMM2 = _mm_and_ps(XMM2, XMM1);
				energy_loss += bitCountTable[_mm_movemask_ps(XMM0)];
				XMM2 = _mm_mul_ps(XMM2, XMM2);
				PACC = _mm_add_ps(PACC, XMM2);
			  }
			}
			acc = _mm_add_horz(PACC);
			{
			  int freqband_mid=j+16;
			  int freqband_flag=0;
			  int min_energy;

			  rpacc=acc;
			  /* When the energy loss of a partition is large, NN is performed in the middle of partition.
			  for 48/44.1/32kHz */
			  if(energy_loss==32 && fabs(qM[freqband_mid])>nnmid_th && acc>=p->vi->normal_thresh
				&& freqband_mid>=pointlimit){
				  __m128 XMM0;
				  XMM0 = _mm_load_ss(qM+freqband_mid);
				  if(_mm_cvtss_si32(XMM0)==0){
					if(mdctMA[freqband_mid]){
					  acc-=1.f;
					  rpacc-=1.32;
					}else{
					  acc-=1.f;
					  rpacc-=1.f;
					}
					qM[freqband_mid]=unitnorm(qM[freqband_mid]);
					freqband_flag=1;
					nn_num++;
				  }
			  }
			  /* NN main (point stereo) */
			  for(k=0;k<partition && acc>=p->vi->normal_thresh;k++){
				int l;
				l=mag_sort[i][j+k];
				if(freqband_mid==l && freqband_flag)continue;
				if(l>=pointlimit && rint(qM[l])==0.f){
				  if(mdctMA[l]){
					if(rpacc<p->vi->normal_thresh)continue;
					acc-=1.f;
					rpacc-=1.32;
				  }else{
					acc-=1.f;
					rpacc-=1.f;
				  }
				  qM[l]=unitnorm(qM[l]);
				  nn_num++;
				}
			  }
			  /* The minimum energy complement.
			  for 48/44.1/32kHz */
			  min_energy=32-energy_loss+nn_num;
			  if(min_energy<2 || (j<=p->min_nn_lp && min_energy==2)){
				int l;
				float ab;
				for(;k<partition;k++){
				  __m128 XMM0;
				  l=mag_sort[i][j+k];
				  ab=fabs(qM[l]);
				  if(ab<0.04)break;
#if	1
				  if( ((mdctM[l]>0.) && (mdctA[l]<0.)) || ((mdctA[l]>0.) && (mdctM[l]<0.))
					&& ab<0.11)break; // 0.11
#else
				  if(mdctMA[l] && ab < 0.11)break;
#endif
				  if(l>=pointlimit){
					__m128 XMM0 = _mm_load_ss(qM+l);
					if(_mm_cvtss_si32(XMM0)==0){
					  qM[l]=unitnorm(qM[l]);
					  break;
					}
				  }
				}
			  }
			}
		  }
		}
	  }
	  else
	  {
		_MM_ALIGN16 float slowM[2048];
		_MM_ALIGN16 float slowA[2048];
		_MM_ALIGN16 float shigh[2048];
		int	midpoint0	 = (limit/partition)*partition;
		int	midpoint1	 = ((limit+partition-1)/partition)*partition;
		for(j=0;j<e;j+=partition){
		  float rpacc;
		  int energy_loss=0;
		  int nn_num=0;

		  for(k=0;k<partition;k++){
			int l=k+j;
			float slow=0.f;
			float shighM=0.f;
			float shighA=0.f;

			slowM[l] = prepoint;
			slowA[l] = prepoint;
			shigh[l] = 0.f;

			postpoint=postpoint_backup;

			/* AoTuV */
			/** @ M6 MAIN **
			The threshold of a stereo is changed dynamically. 
			by Aoyumi @ 2006/06/04
			*/
			if(l>=stcont_start){
			  int m;
			  int lof_num;
			  int hif_num;

			  // (It may be better to calculate this in advance) 
			  lof_st=l-(l/2)*.167;
			  hif_st=l+l*.167;

			  hif_stcopy=hif_st;

			  // limit setting
			  if(hif_st>freqlimit)hif_st=freqlimit;

			  if(old_lof_st || old_hif_st){
				if(hif_st>l){
				  // hif_st, lof_st ...absolute value
				  // lof_num, hif_num ...relative value

				  // low freq.(lower)
				  lof_num=lof_st-old_lof_st;
				  if(lof_num==0){
					Afreq_num+=Ac_treshp[l-1];
					Mfreq_num+=Mc_treshp[l-1];
				  }else if(lof_num==1){
					Afreq_num+=Ac_treshp[l-1];
					Mfreq_num+=Mc_treshp[l-1];
					Afreq_num-=Ac_treshp[old_lof_st];
					Mfreq_num-=Mc_treshp[old_lof_st];
				  }//else puts("err. low");

				  // high freq.(higher)
				  hif_num=hif_st-old_hif_st;
				  if(hif_num==0){
					Afreq_num-=Ac_treshp[l];
					Mfreq_num-=Mc_treshp[l];
				  }else if(hif_num==1){
					Afreq_num-=Ac_treshp[l];
					Mfreq_num-=Mc_treshp[l];
					Afreq_num+=Ac_treshp[hif_st];
					Mfreq_num+=Mc_treshp[hif_st];
				  }else if(hif_num==2){
					Afreq_num-=Ac_treshp[l];
					Mfreq_num-=Mc_treshp[l];
					Afreq_num+=Ac_treshp[hif_st];
					Mfreq_num+=Mc_treshp[hif_st];
					Afreq_num+=Ac_treshp[hif_st-1];
					Mfreq_num+=Mc_treshp[hif_st-1];
				  }//else puts("err. high");
				}
			  }else{
				for(m=lof_st; m<=hif_st; m++){
				  if(m==l)continue;
				  if(Ac_treshp[m]) Afreq_num++;
				  if(Mc_treshp[m]) Mfreq_num++;
				}
			  }
			  if(l>=limit){
				shigh[l]=sth_high/(hif_stcopy-lof_st);
				shighA=shigh[l]*Afreq_num;
				shighM=shigh[l]*Mfreq_num;
				if((shighA+rAs[l])>(shighM+rMs[l]))shigh[l]=shighA;
				else shigh[l]=shighM;
			  }else{
				slow=sth_low/(hif_stcopy-lof_st);
				slowA[l]=slow*Afreq_num;
				slowM[l]=slow*Mfreq_num;
				if(p->noiseoffset[1][l]<-1){
				  slowA[l]*=(p->noiseoffset[1][l]+2);
				  slowM[l]*=(p->noiseoffset[1][l]+2);
				}
				slowA[l] = prepoint - slowA[l];
				slowM[l] = prepoint - slowM[l];
			  }
			  old_lof_st=lof_st;
			  old_hif_st=hif_st;
			}
		  }
		}

#if defined(_OPENMP)
		for(j=s;j<e;j+=partition){
#else
		for(j=0;j<n;j+=partition){
#endif
		  float acc=0.f;
		  float rpacc;
		  int energy_loss=0;
		  int nn_num=0;

		  for(k=0;k<partition;k++){
			int l=k+j;
			float a=mdctM[l];
			float b=mdctA[l];
			float dummypoint;
			float hypot_reserve;

			postpoint=postpoint_backup;

			if(l>=limit){
			  postpoint-=shigh[l];
			  if(mdctMA[l]){
				hypot_reserve = fabs(fabs(a)-fabs(b));
				if(hypot_reserve < 0.001){ // 0~0.000999-
				  dummypoint = stereo_threshholds_rephase[g->coupling_postpointamp[blobno]];
				  dummypoint = dummypoint+((postpoint-dummypoint)*(hypot_reserve*1000));
				  if(postpoint > dummypoint) postpoint = dummypoint;
				}
			  }
			}

			if((l>=limit && rMs[l]<postpoint && rAs[l]<postpoint) ||
			  (rMs[l]<slowM[l] && rAs[l]<slowA[l])){

				if(l>=0&&l<=n)
				{
				  precomputed_couple_point(mag_memo[i][l],
					floorM[l],floorA[l],
					qM+l,qA+l);
				}
				if(rint(qM[l])==0.f){
				  energy_loss++;
				  if(l>=limit)acc+=qM[l]*qM[l];
				}
			}else{
			  couple_lossless(rM[l],rA[l],qM+l,qA+l);
			}
		  }

		  {
			int freqband_mid=j+16;
			int freqband_flag=0;
			int min_energy;

			rpacc=acc;
			/* When the energy loss of a partition is large, NN is performed in the middle of partition.
			for 48/44.1/32kHz */
			if(energy_loss==32 && fabs(qM[freqband_mid])>nnmid_th && acc>=p->vi->normal_thresh
			  && freqband_mid>=pointlimit && rint(qM[freqband_mid])==0.f){
				if(mdctMA[freqband_mid]){
					acc-=1.f;
					rpacc-=1.32;
				}else{
				  acc-=1.f;
				  rpacc-=1.f;
				}
				qM[freqband_mid]=unitnorm(qM[freqband_mid]);
				freqband_flag=1;
				nn_num++;
			}
			/* NN main (point stereo) */
			for(k=0;k<partition && acc>=p->vi->normal_thresh;k++){
			  int l;
			  l=mag_sort[i][j+k];
			  if(freqband_mid==l && freqband_flag)continue;
			  if(l>=pointlimit && rint(qM[l])==0.f){
				if(mdctMA[l]){
				  if(rpacc<p->vi->normal_thresh)continue;
				  acc-=1.f;
				  rpacc-=1.32;
				}else{
				  acc-=1.f;
				  rpacc-=1.f;
				}
				qM[l]=unitnorm(qM[l]);
				nn_num++;
			  }
			}
			/* The minimum energy complement.
			for 48/44.1/32kHz */
			min_energy=32-energy_loss+nn_num;
			if(min_energy<2 || (j<=p->min_nn_lp && min_energy==2)){
			  int l;
			  float ab;
			  for(;k<partition;k++){
				l=mag_sort[i][j+k];
				ab=fabs(qM[l]);
				if(ab<0.04)break;
#if	1
				if( ((mdctM[l]>0.) && (mdctA[l]<0.)) || ((mdctA[l]>0.) && (mdctM[l]<0.))
				 && ab<0.11)break; // 0.11
#else
				if(mdctMA[l] && ab < 0.11)break;
#endif
				if(rint(qM[l])==0.f && l>=pointlimit){
				  qM[l]=unitnorm(qM[l]);
				  break;
				}
			  }
			}
		  }
		}
	  }
#else														/* SSE Optimize */
      if(!stereo_threshholds[g->coupling_postpointamp[blobno]])stcont_start=n;
      else{
      	// exception handling
      	if((postpoint-sth_high)<prepoint)sth_high=postpoint-prepoint;
      	// start point setup
      	for(j=0;j<n;j++){
      		stcont_start=j;
      		if(p->noiseoffset[1][j]>=-2)break;
      	}
      	// start point correction & threshold setup 
      	st_thresh=.1;
      	if(p->m_val<.5){
      		// low frequency limit
      		if(stcont_start<limit)stcont_start=limit;
      	}else if(p->vi->normal_thresh>1.)st_thresh=.5;
      	for(j=0;j<=freqlimit;j++){ // or j<n
      		if(fabs(rM[j])<st_thresh)Mc_treshp[j]=1;
      		else Mc_treshp[j]=0;
      		if(fabs(rA[j])<st_thresh)Ac_treshp[j]=1;
      		else Ac_treshp[j]=0;
      	}
      }
      
      for(j=0;j<n;j+=partition){
	float acc=0.f;
	float rpacc;
	int energy_loss=0;
	int nn_num=0;

	for(k=0;k<partition;k++){
	  int l=k+j;
	  float a=mdctM[l];
	  float b=mdctA[l];
	  float dummypoint;
	  float hypot_reserve;
	  float slow=0.f;
	  float shigh=0.f;
	  float slowM=0.f;
	  float slowA=0.f;
	  float shighM=0.f;
	  float shighA=0.f;
	  float rMs=fabs(rMo[l]);
      float rAs=fabs(rAo[l]);

	  postpoint=postpoint_backup;
	  
	  /* AoTuV */
	  /** @ M6 MAIN **
	    The threshold of a stereo is changed dynamically. 
	    by Aoyumi @ 2006/06/04
	  */
	  if(l>=stcont_start){
	  	int m;
	  	int lof_num;
	  	int hif_num;
	  	
	  	// (It may be better to calculate this in advance) 
	  	lof_st=l-(l/2)*.167;
	  	hif_st=l+l*.167;
	  
	  	hif_stcopy=hif_st;
	  	
	  	// limit setting
	  	if(hif_st>freqlimit)hif_st=freqlimit;
	  	
	  	if(old_lof_st || old_hif_st){
	  		if(hif_st>l){
	  			// hif_st, lof_st ...absolute value
	  			// lof_num, hif_num ...relative value
	  			
	  			// low freq.(lower)
	  			lof_num=lof_st-old_lof_st;
	  			if(lof_num==0){
	  				Afreq_num+=Ac_treshp[l-1];
	  				Mfreq_num+=Mc_treshp[l-1];
	  			}else if(lof_num==1){
	  				Afreq_num+=Ac_treshp[l-1];
	  				Mfreq_num+=Mc_treshp[l-1];
	  				Afreq_num-=Ac_treshp[old_lof_st];
	  				Mfreq_num-=Mc_treshp[old_lof_st];
	  			}//else puts("err. low");
	  			
	  			// high freq.(higher)
	  			hif_num=hif_st-old_hif_st;
	  			if(hif_num==0){
	  				Afreq_num-=Ac_treshp[l];
	  				Mfreq_num-=Mc_treshp[l];
	  			}else if(hif_num==1){
	  				Afreq_num-=Ac_treshp[l];
	  				Mfreq_num-=Mc_treshp[l];
	  				Afreq_num+=Ac_treshp[hif_st];
	  				Mfreq_num+=Mc_treshp[hif_st];
	  			}else if(hif_num==2){
	  				Afreq_num-=Ac_treshp[l];
	  				Mfreq_num-=Mc_treshp[l];
	  				Afreq_num+=Ac_treshp[hif_st];
	  				Mfreq_num+=Mc_treshp[hif_st];
	  				Afreq_num+=Ac_treshp[hif_st-1];
	  				Mfreq_num+=Mc_treshp[hif_st-1];
	  			}//else puts("err. high");
	  		}
	  	}else{
	  		for(m=lof_st; m<=hif_st; m++){
	  			if(m==l)continue;
	  			if(Ac_treshp[m]) Afreq_num++;
	  			if(Mc_treshp[m]) Mfreq_num++;
			}
	  	}
	  	if(l>=limit){
	  		shigh=sth_high/(hif_stcopy-lof_st);
	  		shighA=shigh*Afreq_num;
	  		shighM=shigh*Mfreq_num;
	  		if((shighA+rAs)>(shighM+rMs))shigh=shighA;
	  		else shigh=shighM;
		}else{
			slow=sth_low/(hif_stcopy-lof_st);
			slowA=slow*Afreq_num;
			slowM=slow*Mfreq_num;
			if(p->noiseoffset[1][l]<-1){
				slowA*=(p->noiseoffset[1][l]+2);
				slowM*=(p->noiseoffset[1][l]+2);
			}
		}
		old_lof_st=lof_st;
	  	old_hif_st=hif_st;
	  }
	  if(l>=limit){
	    postpoint-=shigh;
	    /* The following prevents an extreme reduction of residue. (2ch stereo only) */
	  	if( ((a>0.) && (b<0.)) || ((b>0.) && (a<0.)) ){
	  		hypot_reserve = fabs(fabs(a)-fabs(b));
	  		if(hypot_reserve < 0.001){ // 0~0.000999-
	  			dummypoint = stereo_threshholds_rephase[g->coupling_postpointamp[blobno]];
	  			dummypoint = dummypoint+((postpoint-dummypoint)*(hypot_reserve*1000));
	  			if(postpoint > dummypoint) postpoint = dummypoint;
	  		}
      	}
	  }
	  
	  if(l<sliding_lowpass){
	    if((l>=limit && rMs<postpoint && rAs<postpoint) ||
	       (rMs<(prepoint-slowM) && rAs<(prepoint-slowA))){

		  if(l>=0&&l<=n)
		  {
	      precomputed_couple_point(mag_memo[i][l],
				       floorM[l],floorA[l],
				       qM+l,qA+l);
		  }
	      //if(rint(qM[l])==0.f)acc+=qM[l]*qM[l];
	      if(rint(qM[l])==0.f){
	      	energy_loss++;
	      	if(l>=limit)acc+=qM[l]*qM[l];
	      }
	    }else{
	      couple_lossless(rM[l],rA[l],qM+l,qA+l);
	    }
	  }else{
	    qM[l]=0.;
	    qA[l]=0.;
	  }
	}

	if(p->vi->normal_point_p){
	  int freqband_mid=j+16;
	  int freqband_flag=0;
	  int min_energy;
	  
	  rpacc=acc;
	  /* When the energy loss of a partition is large, NN is performed in the middle of partition.
	      for 48/44.1/32kHz */
	  if(energy_loss==32 && fabs(qM[freqband_mid])>nnmid_th && acc>=p->vi->normal_thresh
	   && freqband_mid>=pointlimit && rint(qM[freqband_mid])==0.f){
	  	if( ((mdctM[freqband_mid]>0.) && (mdctA[freqband_mid]<0.)) ||
	  	 ((mdctA[freqband_mid]>0.) && (mdctM[freqband_mid]<0.)) ){
	  	 acc-=1.f;
	  	 rpacc-=1.32;
	  	}else{
	  	 acc-=1.f;
	  	 rpacc-=1.f;
	  	}
	  	qM[freqband_mid]=unitnorm(qM[freqband_mid]);
	  	freqband_flag=1;
	  	nn_num++;
	  }
	  /* NN main (point stereo) */
	  for(k=0;k<partition && acc>=p->vi->normal_thresh;k++){
	    int l;
	    l=mag_sort[i][j+k];
	    if(freqband_mid==l && freqband_flag)continue;
	    if(l<sliding_lowpass && l>=pointlimit && rint(qM[l])==0.f){
	      if( ((mdctM[l]>0.) && (mdctA[l]<0.)) || ((mdctA[l]>0.) && (mdctM[l]<0.)) ){
	        if(rpacc<p->vi->normal_thresh)continue;
	        acc-=1.f;
	        rpacc-=1.32;
	      }else{
	        acc-=1.f;
	        rpacc-=1.f;
	      }
	      qM[l]=unitnorm(qM[l]);
	      nn_num++;
	    }
	  }
	  /* The minimum energy complement.
	      for 48/44.1/32kHz */
	  min_energy=32-energy_loss+nn_num;
	  if(min_energy<2 || (j<=p->min_nn_lp && min_energy==2)){
	  	int l;
	  	float ab;
	    for(;k<partition;k++){
	    	l=mag_sort[i][j+k];
	    	ab=fabs(qM[l]);
	    	if(ab<0.04)break;
	    	if( ((mdctM[l]>0.) && (mdctA[l]<0.)) || ((mdctA[l]>0.) && (mdctM[l]<0.))
	    	 && ab<0.11)break; // 0.11
	    	if(rint(qM[l])==0.f && l>=pointlimit){
	    		qM[l]=unitnorm(qM[l]);
	    		break;
	    	}
	    }
	  }
	}
      }
#endif														/* SSE Optimize */
    }
  }
}

/*  aoTuV M5
	noise_compand_level of low frequency is determined from the level of high frequency. 
	by Aoyumi @ 2005/09/14
	
	return value
	[normal compander] 0 <> 1.0 [high compander] 
	-1 @ disable
*/
float lb_loudnoise_fix(vorbis_look_psy *p,
		float noise_compand_level,
		float *logmdct,
		int lW_modenumber,
		int blocktype, int modenumber){

	int i, n=p->n, nq1=p->n25p, nq3=p->n75p;
	double hi_th=0;
	
	if(p->m_val < 0.5)return(-1); /* 48/44.1/32kHz only */
	if(p->vi->normal_thresh>.45)return(-1); /* under q3 */

	/* select trans. block(short>>long case). */
	if(!modenumber)return(-1);
	if(blocktype || lW_modenumber)return(noise_compand_level);

	/* calculation of a threshold. */
	for(i=nq1; i<nq3; i++){
		if(logmdct[i]>-130)hi_th += logmdct[i];
		else hi_th += -130;
	}
	hi_th /= n;
	
	/* calculation of a high_compand_level */
	if(hi_th > -40.) noise_compand_level=-1;
	else if(hi_th < -50.) noise_compand_level=1.;
	else noise_compand_level=1.-((hi_th+50)/10);

	return(noise_compand_level);
}
