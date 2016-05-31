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

 function: channel mapping 0 implementation
 last mod: $Id: mapping0.c 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ogg/ogg.h>
#include "vorbis/codec.h"
#include "codec_internal.h"
#include "codebook.h"
#include "window.h"
#include "registry.h"
#include "psy.h"
#include "misc.h"
#ifdef __SSE__												/* SSE Optimize */
#include <float.h>
#include "xmmlib.h"
#ifdef	_OPENMP
#include <windows.h>
#include <omp.h>

typedef struct {
  vorbis_info_residue0 *info;
  
  int         parts;
  int         stages;
  codebook   *fullbooks;
  codebook   *phrasebook;
  codebook ***partbooks;

  int         partvals;
  int       **decodemap;

  long      postbits;
  long      phrasebits;
  long      frames;

#if defined(TRAIN_RES) || defined(TRAIN_RESAUX)
  int        train_seq;
  long      *training_data[8][64];
  float      training_max[8][64];
  float      training_min[8][64];
  float     tmin;
  float     tmax;
#endif

} vorbis_look_residue0;
#endif
#endif														/* SSE Optimize */

/* simplistic, wasteful way of doing this (unique lookup for each
   mode/submapping); there should be a central repository for
   identical lookups.  That will require minor work, so I'm putting it
   off as low priority.

   Why a lookup for each backend in a given mode?  Because the
   blocksize is set by the mode, and low backend lookups may require
   parameters from other areas of the mode/mapping */

static void mapping0_free_info(vorbis_info_mapping *i){
  vorbis_info_mapping0 *info=(vorbis_info_mapping0 *)i;
  if(info){
    memset(info,0,sizeof(*info));
    _ogg_free(info);
  }
}

static int ilog(unsigned int v){
  int ret=0;
  if(v)--v;
  while(v){
    ret++;
    v>>=1;
  }
  return(ret);
}

static void mapping0_pack(vorbis_info *vi,vorbis_info_mapping *vm,
			  oggpack_buffer *opb){
  int i;
  vorbis_info_mapping0 *info=(vorbis_info_mapping0 *)vm;

  /* another 'we meant to do it this way' hack...  up to beta 4, we
     packed 4 binary zeros here to signify one submapping in use.  We
     now redefine that to mean four bitflags that indicate use of
     deeper features; bit0:submappings, bit1:coupling,
     bit2,3:reserved. This is backward compatable with all actual uses
     of the beta code. */

  if(info->submaps>1){
    oggpack_write(opb,1,1);
    oggpack_write(opb,info->submaps-1,4);
  }else
    oggpack_write(opb,0,1);

  if(info->coupling_steps>0){
    oggpack_write(opb,1,1);
    oggpack_write(opb,info->coupling_steps-1,8);
    
    for(i=0;i<info->coupling_steps;i++){
      oggpack_write(opb,info->coupling_mag[i],ilog(vi->channels));
      oggpack_write(opb,info->coupling_ang[i],ilog(vi->channels));
    }
  }else
    oggpack_write(opb,0,1);
  
  oggpack_write(opb,0,2); /* 2,3:reserved */

  /* we don't write the channel submappings if we only have one... */
  if(info->submaps>1){
    for(i=0;i<vi->channels;i++)
      oggpack_write(opb,info->chmuxlist[i],4);
  }
  for(i=0;i<info->submaps;i++){
    oggpack_write(opb,0,8); /* time submap unused */
    oggpack_write(opb,info->floorsubmap[i],8);
    oggpack_write(opb,info->residuesubmap[i],8);
  }
}

/* also responsible for range checking */
static vorbis_info_mapping *mapping0_unpack(vorbis_info *vi,oggpack_buffer *opb){
  int i;
  vorbis_info_mapping0 *info=_ogg_calloc(1,sizeof(*info));
  codec_setup_info     *ci=vi->codec_setup;
  memset(info,0,sizeof(*info));

  if(oggpack_read(opb,1))
    info->submaps=oggpack_read(opb,4)+1;
  else
    info->submaps=1;

  if(oggpack_read(opb,1)){
    info->coupling_steps=oggpack_read(opb,8)+1;

    for(i=0;i<info->coupling_steps;i++){
      int testM=info->coupling_mag[i]=oggpack_read(opb,ilog(vi->channels));
      int testA=info->coupling_ang[i]=oggpack_read(opb,ilog(vi->channels));

      if(testM<0 || 
	 testA<0 || 
	 testM==testA || 
	 testM>=vi->channels ||
	 testA>=vi->channels) goto err_out;
    }

  }

  if(oggpack_read(opb,2)>0)goto err_out; /* 2,3:reserved */
    
  if(info->submaps>1){
    for(i=0;i<vi->channels;i++){
      info->chmuxlist[i]=oggpack_read(opb,4);
      if(info->chmuxlist[i]>=info->submaps)goto err_out;
    }
  }
  for(i=0;i<info->submaps;i++){
    oggpack_read(opb,8); /* time submap unused */
    info->floorsubmap[i]=oggpack_read(opb,8);
    if(info->floorsubmap[i]>=ci->floors)goto err_out;
    info->residuesubmap[i]=oggpack_read(opb,8);
    if(info->residuesubmap[i]>=ci->residues)goto err_out;
  }

  return info;

 err_out:
  mapping0_free_info(info);
  return(NULL);
}

#include "os.h"
#include "lpc.h"
#include "lsp.h"
#include "envelope.h"
#include "mdct.h"
#include "psy.h"
#include "scales.h"

#if 0
static long seq=0;
static ogg_int64_t total=0;
static float FLOOR1_fromdB_LOOKUP[256]={
  1.0649863e-07F, 1.1341951e-07F, 1.2079015e-07F, 1.2863978e-07F, 
  1.3699951e-07F, 1.4590251e-07F, 1.5538408e-07F, 1.6548181e-07F, 
  1.7623575e-07F, 1.8768855e-07F, 1.9988561e-07F, 2.128753e-07F, 
  2.2670913e-07F, 2.4144197e-07F, 2.5713223e-07F, 2.7384213e-07F, 
  2.9163793e-07F, 3.1059021e-07F, 3.3077411e-07F, 3.5226968e-07F, 
  3.7516214e-07F, 3.9954229e-07F, 4.2550680e-07F, 4.5315863e-07F, 
  4.8260743e-07F, 5.1396998e-07F, 5.4737065e-07F, 5.8294187e-07F, 
  6.2082472e-07F, 6.6116941e-07F, 7.0413592e-07F, 7.4989464e-07F, 
  7.9862701e-07F, 8.5052630e-07F, 9.0579828e-07F, 9.6466216e-07F, 
  1.0273513e-06F, 1.0941144e-06F, 1.1652161e-06F, 1.2409384e-06F, 
  1.3215816e-06F, 1.4074654e-06F, 1.4989305e-06F, 1.5963394e-06F, 
  1.7000785e-06F, 1.8105592e-06F, 1.9282195e-06F, 2.0535261e-06F, 
  2.1869758e-06F, 2.3290978e-06F, 2.4804557e-06F, 2.6416497e-06F, 
  2.8133190e-06F, 2.9961443e-06F, 3.1908506e-06F, 3.3982101e-06F, 
  3.6190449e-06F, 3.8542308e-06F, 4.1047004e-06F, 4.3714470e-06F, 
  4.6555282e-06F, 4.9580707e-06F, 5.2802740e-06F, 5.6234160e-06F, 
  5.9888572e-06F, 6.3780469e-06F, 6.7925283e-06F, 7.2339451e-06F, 
  7.7040476e-06F, 8.2047000e-06F, 8.7378876e-06F, 9.3057248e-06F, 
  9.9104632e-06F, 1.0554501e-05F, 1.1240392e-05F, 1.1970856e-05F, 
  1.2748789e-05F, 1.3577278e-05F, 1.4459606e-05F, 1.5399272e-05F, 
  1.6400004e-05F, 1.7465768e-05F, 1.8600792e-05F, 1.9809576e-05F, 
  2.1096914e-05F, 2.2467911e-05F, 2.3928002e-05F, 2.5482978e-05F, 
  2.7139006e-05F, 2.8902651e-05F, 3.0780908e-05F, 3.2781225e-05F, 
  3.4911534e-05F, 3.7180282e-05F, 3.9596466e-05F, 4.2169667e-05F, 
  4.4910090e-05F, 4.7828601e-05F, 5.0936773e-05F, 5.4246931e-05F, 
  5.7772202e-05F, 6.1526565e-05F, 6.5524908e-05F, 6.9783085e-05F, 
  7.4317983e-05F, 7.9147585e-05F, 8.4291040e-05F, 8.9768747e-05F, 
  9.5602426e-05F, 0.00010181521F, 0.00010843174F, 0.00011547824F, 
  0.00012298267F, 0.00013097477F, 0.00013948625F, 0.00014855085F, 
  0.00015820453F, 0.00016848555F, 0.00017943469F, 0.00019109536F, 
  0.00020351382F, 0.00021673929F, 0.00023082423F, 0.00024582449F, 
  0.00026179955F, 0.00027881276F, 0.00029693158F, 0.00031622787F, 
  0.00033677814F, 0.00035866388F, 0.00038197188F, 0.00040679456F, 
  0.00043323036F, 0.00046138411F, 0.00049136745F, 0.00052329927F, 
  0.00055730621F, 0.00059352311F, 0.00063209358F, 0.00067317058F, 
  0.00071691700F, 0.00076350630F, 0.00081312324F, 0.00086596457F, 
  0.00092223983F, 0.00098217216F, 0.0010459992F, 0.0011139742F, 
  0.0011863665F, 0.0012634633F, 0.0013455702F, 0.0014330129F, 
  0.0015261382F, 0.0016253153F, 0.0017309374F, 0.0018434235F, 
  0.0019632195F, 0.0020908006F, 0.0022266726F, 0.0023713743F, 
  0.0025254795F, 0.0026895994F, 0.0028643847F, 0.0030505286F, 
  0.0032487691F, 0.0034598925F, 0.0036847358F, 0.0039241906F, 
  0.0041792066F, 0.0044507950F, 0.0047400328F, 0.0050480668F, 
  0.0053761186F, 0.0057254891F, 0.0060975636F, 0.0064938176F, 
  0.0069158225F, 0.0073652516F, 0.0078438871F, 0.0083536271F, 
  0.0088964928F, 0.009474637F, 0.010090352F, 0.010746080F, 
  0.011444421F, 0.012188144F, 0.012980198F, 0.013823725F, 
  0.014722068F, 0.015678791F, 0.016697687F, 0.017782797F, 
  0.018938423F, 0.020169149F, 0.021479854F, 0.022875735F, 
  0.024362330F, 0.025945531F, 0.027631618F, 0.029427276F, 
  0.031339626F, 0.033376252F, 0.035545228F, 0.037855157F, 
  0.040315199F, 0.042935108F, 0.045725273F, 0.048696758F, 
  0.051861348F, 0.055231591F, 0.058820850F, 0.062643361F, 
  0.066714279F, 0.071049749F, 0.075666962F, 0.080584227F, 
  0.085821044F, 0.091398179F, 0.097337747F, 0.10366330F, 
  0.11039993F, 0.11757434F, 0.12521498F, 0.13335215F, 
  0.14201813F, 0.15124727F, 0.16107617F, 0.17154380F, 
  0.18269168F, 0.19456402F, 0.20720788F, 0.22067342F, 
  0.23501402F, 0.25028656F, 0.26655159F, 0.28387361F, 
  0.30232132F, 0.32196786F, 0.34289114F, 0.36517414F, 
  0.38890521F, 0.41417847F, 0.44109412F, 0.46975890F, 
  0.50028648F, 0.53279791F, 0.56742212F, 0.60429640F, 
  0.64356699F, 0.68538959F, 0.72993007F, 0.77736504F, 
  0.82788260F, 0.88168307F, 0.9389798F, 1.F, 
};

#endif 

extern int *floor1_fit(vorbis_block *vb,vorbis_look_floor *look,
		       const float *logmdct,   /* in */
#if	defined(_OPENMP)
		       const float *logmask,
			   int *in_output);
#else
		       const float *logmask);
#endif
extern int *floor1_interpolate_fit(vorbis_block *vb,vorbis_look_floor *look,
				   int *A,int *B,
#if	defined(_OPENMP)
				   int del,
				   int *in_output);
#else
				   int del);
#endif
extern int floor1_encode(oggpack_buffer *opb,vorbis_block *vb,
			 vorbis_look_floor *look,
#if	defined(_OPENMP)
			 int *post,int *ilogmask, oggpack_writecache *opwc);
#else
			 int *post,int *ilogmask);
#endif

#ifdef __SSE__												/* SSE Optimize */
static void mapping_forward_sub0(float *pcm, float *logfft, float scale_dB,
								 float *local_ampmax, int i, int n)
{
	_MM_ALIGN16 const float mparm[4] = {
		7.17711438e-7f/2.f, 7.17711438e-7f/2.f, 7.17711438e-7f/2.f, 7.17711438e-7f/2.f,
	};
	__m128	SCALEdB;
	__m128	LAM0;
#if	!defined(__SSE2__)
	__m128	LAM1;
#endif
	int	j, k;
	SCALEdB	 = _mm_set_ps1(scale_dB+.345f-764.6161886f/2.f);
	LAM0	 = _mm_set_ps1(local_ampmax[i]);
#if	defined(__SSE2__)
	if(n>=256&&n<=4096)
	{
		/*
			Cation! This routhine is for SSE optimized fft only.
		*/
		float	rfv	 = logfft[0];
		logfft[0]	 = 0.f;
		logfft[1]	 = 0.f;
#if	defined(__SSE3__)
#if defined(__OPENMP)
		/*
			SSE3 optimized code for multithread(unrolled)
		*/
		for(j=0,k=0;j<n;j+=64,k+=32)
		{
			__m128	XMM0, XMM1, XMM2, XMM3;
			__m128	XMM4;
			XMM0	 = _mm_load_ps(pcm+j   );
			XMM1	 = _mm_load_ps(pcm+j+ 4);
			XMM2	 = _mm_load_ps(pcm+j+ 8);
			XMM3	 = _mm_load_ps(pcm+j+12);
			XMM0	 = _mm_mul_ps(XMM0, XMM0);
			XMM1	 = _mm_mul_ps(XMM1, XMM1);
			XMM2	 = _mm_mul_ps(XMM2, XMM2);
			XMM3	 = _mm_mul_ps(XMM3, XMM3);
			XMM0	 = _mm_hadd_ps(XMM0, XMM1);
			XMM2	 = _mm_hadd_ps(XMM2, XMM3);
			XMM4	 = _mm_load_ps(pcm+j+16);
			XMM1	 = _mm_load_ps(pcm+j+20);
			XMM0	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM0));
			XMM2	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM2));
			XMM3	 = _mm_load_ps(pcm+j+24);
			XMM0	 = _mm_mul_ps(XMM0, PM128(mparm));
			XMM2	 = _mm_mul_ps(XMM2, PM128(mparm));
			XMM4	 = _mm_mul_ps(XMM4, XMM4);
			XMM0	 = _mm_add_ps(XMM0, SCALEdB);
			XMM2	 = _mm_add_ps(XMM2, SCALEdB);
			XMM1	 = _mm_mul_ps(XMM1, XMM1);
			_mm_store_ps(logfft+k   , XMM0);
			_mm_store_ps(logfft+k+ 4, XMM2);
			XMM3	 = _mm_mul_ps(XMM3, XMM3);
			XMM0		 = _mm_max_ps(XMM0, XMM2);
			XMM2	 = _mm_load_ps(pcm+j+28);
			LAM0		 = _mm_max_ps(LAM0, XMM0);
			XMM2	 = _mm_mul_ps(XMM2, XMM2);
			XMM4	 = _mm_hadd_ps(XMM4, XMM1);
			XMM3	 = _mm_hadd_ps(XMM3, XMM2);
			XMM0	 = _mm_load_ps(pcm+j+32);
			XMM1	 = _mm_load_ps(pcm+j+36);
			XMM4	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM4));
			XMM3	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM3));
			XMM2	 = _mm_load_ps(pcm+j+40);
			XMM4	 = _mm_mul_ps(XMM4, PM128(mparm));
			XMM3	 = _mm_mul_ps(XMM3, PM128(mparm));
			XMM0	 = _mm_mul_ps(XMM0, XMM0);
			XMM4	 = _mm_add_ps(XMM4, SCALEdB);
			XMM3	 = _mm_add_ps(XMM3, SCALEdB);
			XMM1	 = _mm_mul_ps(XMM1, XMM1);
			_mm_store_ps(logfft+k+ 8, XMM4);
			_mm_store_ps(logfft+k+12, XMM3);
			XMM4		 = _mm_max_ps(XMM4, XMM3);
			LAM0		 = _mm_max_ps(LAM0, XMM4);
			XMM3	 = _mm_load_ps(pcm+j+44);
			XMM2	 = _mm_mul_ps(XMM2, XMM2);
			XMM3	 = _mm_mul_ps(XMM3, XMM3);
			XMM0	 = _mm_hadd_ps(XMM0, XMM1);
			XMM2	 = _mm_hadd_ps(XMM2, XMM3);
			XMM4	 = _mm_load_ps(pcm+j+48);
			XMM1	 = _mm_load_ps(pcm+j+52);
			XMM0	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM0));
			XMM2	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM2));
			XMM3	 = _mm_load_ps(pcm+j+56);
			XMM0	 = _mm_mul_ps(XMM0, PM128(mparm));
			XMM2	 = _mm_mul_ps(XMM2, PM128(mparm));
			XMM4	 = _mm_mul_ps(XMM4, XMM4);
			XMM0	 = _mm_add_ps(XMM0, SCALEdB);
			XMM2	 = _mm_add_ps(XMM2, SCALEdB);
			XMM1	 = _mm_mul_ps(XMM1, XMM1);
			_mm_store_ps(logfft+k+16, XMM0);
			_mm_store_ps(logfft+k+20, XMM2);
			XMM3	 = _mm_mul_ps(XMM3, XMM3);
			XMM0		 = _mm_max_ps(XMM0, XMM2);
			XMM2	 = _mm_load_ps(pcm+j+60);
			LAM0		 = _mm_max_ps(LAM0, XMM0);
			XMM2	 = _mm_mul_ps(XMM2, XMM2);
			XMM4	 = _mm_hadd_ps(XMM4, XMM1);
			XMM3	 = _mm_hadd_ps(XMM3, XMM2);
			XMM4	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM4));
			XMM3	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM3));
			XMM4	 = _mm_mul_ps(XMM4, PM128(mparm));
			XMM3	 = _mm_mul_ps(XMM3, PM128(mparm));
			XMM4	 = _mm_add_ps(XMM4, SCALEdB);
			XMM3	 = _mm_add_ps(XMM3, SCALEdB);
			_mm_store_ps(logfft+k+24, XMM4);
			_mm_store_ps(logfft+k+28, XMM3);
			XMM4		 = _mm_max_ps(XMM4, XMM3);
			LAM0		 = _mm_max_ps(LAM0, XMM4);
		}
#else	/* for singlethread */
		/*
			SSE3 optimized code
		*/
		for(j=0,k=0;j<n;j+=16,k+=8)
		{
			__m128	XMM0, XMM2;
			__m128	XMM1, XMM3;
			XMM0	 = _mm_load_ps(pcm+j   );
			XMM1	 = _mm_load_ps(pcm+j+ 4);
			XMM2	 = _mm_load_ps(pcm+j+ 8);
			XMM3	 = _mm_load_ps(pcm+j+12);
			XMM0	 = _mm_mul_ps(XMM0, XMM0);
			XMM1	 = _mm_mul_ps(XMM1, XMM1);
			XMM2	 = _mm_mul_ps(XMM2, XMM2);
			XMM3	 = _mm_mul_ps(XMM3, XMM3);
			XMM0	 = _mm_hadd_ps(XMM0, XMM1);
			XMM2	 = _mm_hadd_ps(XMM2, XMM3);
			XMM0	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM0));
			XMM2	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM2));
			XMM0	 = _mm_mul_ps(XMM0, PM128(mparm));
			XMM2	 = _mm_mul_ps(XMM2, PM128(mparm));
			XMM0	 = _mm_add_ps(XMM0, SCALEdB);
			XMM2	 = _mm_add_ps(XMM2, SCALEdB);
			_mm_store_ps(logfft+k   , XMM0);
			_mm_store_ps(logfft+k+ 4, XMM2);
			XMM0		 = _mm_max_ps(XMM0, XMM2);
			LAM0		 = _mm_max_ps(LAM0, XMM0);
		}
#endif
#else	/* for SSE2 */
		/*
			SSE2 optimized code
		*/
		for(j=0,k=0;j<n;j+=16,k+=8)
		{
			__m128	XMM0, XMM2;
			__m128	XMM1, XMM3;
			__m128	XMM4, XMM5;
			XMM0	 = _mm_load_ps(pcm+j   );
			XMM2	 = _mm_load_ps(pcm+j+ 8);
			XMM4	 = _mm_load_ps(pcm+j+ 4);
			XMM5	 = _mm_load_ps(pcm+j+12);
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
			XMM0	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM0));
			XMM2	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM2));
			XMM0	 = _mm_mul_ps(XMM0, PM128(mparm));
			XMM2	 = _mm_mul_ps(XMM2, PM128(mparm));
			XMM0	 = _mm_add_ps(XMM0, SCALEdB);
			XMM2	 = _mm_add_ps(XMM2, SCALEdB);
			_mm_store_ps(logfft+k   , XMM0);
			_mm_store_ps(logfft+k+ 4, XMM2);
			XMM0		 = _mm_max_ps(XMM0, XMM2);
			LAM0		 = _mm_max_ps(LAM0, XMM0);
		}
#endif
		local_ampmax[i]	 =  _mm_max_horz(LAM0);
		logfft[0]	 = rfv;
	}
	else
	{
		/*
			SSE2 optimized code
		*/
		int Cnt	 = ((n-2)&(~15))+1;
		for(j=1;j<Cnt;j+=16){
		__m128	XMM0, XMM3;
#if	defined(__SSE3__)
			{
				__m128	XMM2, XMM5;
				XMM0	 = _mm_lddqu_ps(pcm+j   );
				XMM2	 = _mm_lddqu_ps(pcm+j+ 4);
				XMM3	 = _mm_lddqu_ps(pcm+j+ 8);
				XMM5	 = _mm_lddqu_ps(pcm+j+12);
				XMM0	 = _mm_mul_ps(XMM0, XMM0);
				XMM2	 = _mm_mul_ps(XMM2, XMM2);
				XMM3	 = _mm_mul_ps(XMM3, XMM3);
				XMM5	 = _mm_mul_ps(XMM5, XMM5);
				XMM0	 = _mm_hadd_ps(XMM0, XMM2);
				XMM3	 = _mm_hadd_ps(XMM3, XMM5);
			}
#else
			{
				__m128	XMM2, XMM5;
				{
					__m128	XMM1, XMM4;
					XMM0	 = _mm_loadu_ps(pcm+j   );
					XMM1	 = _mm_loadu_ps(pcm+j+ 4);
					XMM3	 = _mm_loadu_ps(pcm+j+ 8);
					XMM4	 = _mm_loadu_ps(pcm+j+12);
					XMM2	 = XMM0;
					XMM5	 = XMM3;
					XMM0	 = _mm_shuffle_ps(XMM0, XMM1,_MM_SHUFFLE(2,0,2,0));
					XMM2	 = _mm_shuffle_ps(XMM2, XMM1,_MM_SHUFFLE(3,1,3,1));
					XMM3	 = _mm_shuffle_ps(XMM3, XMM4,_MM_SHUFFLE(2,0,2,0));
					XMM5	 = _mm_shuffle_ps(XMM5, XMM4,_MM_SHUFFLE(3,1,3,1));
				}
				XMM0	 = _mm_mul_ps(XMM0, XMM0);
				XMM3	 = _mm_mul_ps(XMM3, XMM3);
				XMM2	 = _mm_mul_ps(XMM2, XMM2);
				XMM5	 = _mm_mul_ps(XMM5, XMM5);
				XMM0	 = _mm_add_ps(XMM0, XMM2);
				XMM3	 = _mm_add_ps(XMM3, XMM5);
			}
#endif
			XMM0	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM0));
			XMM3	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM3));
			XMM0	 = _mm_mul_ps(XMM0, PM128(mparm  ));
			XMM3	 = _mm_mul_ps(XMM3, PM128(mparm+4));
			XMM0	 = _mm_add_ps(XMM0, SCALEdB);
			XMM3	 = _mm_add_ps(XMM3, SCALEdB);
			_mm_storeu_ps(logfft+((j+1)>>1), XMM0);
			_mm_storeu_ps(logfft+((j+9)>>1), XMM3);
			XMM0		 = _mm_max_ps(XMM0, XMM3);
			LAM0		 = _mm_max_ps(LAM0, XMM0);
		}
		Cnt	 = ((n-2)&(~7))+1;
		for(;j<Cnt;j+=8){
			__m128	XMM0;
#if	defined(__SSE3__)
			{
				__m128	XMM1;
				XMM0	 = _mm_lddqu_ps(pcm+j   );
				XMM1	 = _mm_lddqu_ps(pcm+j+ 4);
				XMM0	 = _mm_mul_ps(XMM0, XMM0);
				XMM1	 = _mm_mul_ps(XMM1, XMM1);
				XMM0	 = _mm_hadd_ps(XMM0, XMM1);
			}
#else
			{
				__m128	XMM2;
				{
					__m128	XMM1;
					XMM0	 = _mm_loadu_ps(pcm+j   );
					XMM1	 = _mm_loadu_ps(pcm+j+ 4);
					XMM2	 = XMM0;
					XMM0	 = _mm_shuffle_ps(XMM0, XMM1,_MM_SHUFFLE(2,0,2,0));
					XMM2	 = _mm_shuffle_ps(XMM2, XMM1,_MM_SHUFFLE(3,1,3,1));
				}
				XMM0	 = _mm_mul_ps(XMM0, XMM0);
				XMM2	 = _mm_mul_ps(XMM2, XMM2);
				XMM0	 = _mm_add_ps(XMM0, XMM2);
			}
#endif
			XMM0	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM0));
			XMM0	 = _mm_mul_ps(XMM0, PM128(mparm));
			XMM0	 = _mm_add_ps(XMM0, SCALEdB);
			_mm_storeu_ps(&logfft[(j+1)>>1], XMM0);
			LAM0		 = _mm_max_ps(LAM0, XMM0);
		}
		local_ampmax[i]	 = _mm_max_horz(LAM0);
		for(;j<n;j+=2){
			float	temp	 = pcm[j]*pcm[j]+pcm[j+1]*pcm[j+1];
			temp=logfft[(j+1)>>1]=scale_dB+.5f*todB(&temp)  + .345; /* +
										.345 is a hack; the original todB
										estimation used on IEEE 754
										compliant machines had a bug that
										returned dB values about a third
										of a decibel too high.  The bug
										was harmless because tunings
										implicitly took that into
										account.  However, fixing the bug
										in the estimator requires
										changing all the tunings as well.
										For now, it's easier to sync
										things back up here, and
										recalibrate the tunings in the
										next major model upgrade. */
			if(temp>local_ampmax[i])
				local_ampmax[i]	 = temp;
		}
	}
#else	/* for __SSE2__ */
	/*
		SSE optimized code
	*/
	LAM1	 = LAM0;
	if(n>=256&&n<=4096)
	{
		/*
			Cation! This routhine is for SSE optimized fft only.
		*/
		float	rfv	 = logfft[0];
		logfft[0]	 = 0.f;
		logfft[1]	 = 0.f;
		for(j=0,k=0;j<n;j+=32,k+=16)
		{
			__m64	MM0, MM1, MM2, MM3;
			__m64	MM4, MM5, MM6, MM7;
			__m128x	U0, U1, U2, U3;
			{
				__m128	XMM0, XMM1, XMM2, XMM3;
				__m128	XMM4, XMM5;
				XMM0	 = _mm_load_ps(pcm+j   );
				XMM2	 = _mm_load_ps(pcm+j+ 8);
				XMM4	 = _mm_load_ps(pcm+j+ 4);
				XMM5	 = _mm_load_ps(pcm+j+12);
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
				XMM4	 = _mm_load_ps(pcm+j+16);
				U0.ps	 = XMM0;
				U1.ps	 = XMM2;
				XMM1	 = _mm_load_ps(pcm+j+24);
				XMM0	 = _mm_load_ps(pcm+j+20);
				XMM2	 = _mm_load_ps(pcm+j+28);
				XMM5	 = XMM4;
				XMM3	 = XMM1;
				XMM4	 = _mm_shuffle_ps(XMM4, XMM0,_MM_SHUFFLE(2,0,2,0));
				XMM5	 = _mm_shuffle_ps(XMM5, XMM0,_MM_SHUFFLE(3,1,3,1));
				XMM1	 = _mm_shuffle_ps(XMM1, XMM2,_MM_SHUFFLE(2,0,2,0));
				XMM3	 = _mm_shuffle_ps(XMM3, XMM2,_MM_SHUFFLE(3,1,3,1));
				MM0		 = U0.pi64[1];
				MM1		 = U1.pi64[1];
				MM2		 = U0.pi64[0];
				MM3		 = U1.pi64[0];
				XMM4	 = _mm_mul_ps(XMM4, XMM4);
				XMM5	 = _mm_mul_ps(XMM5, XMM5);
				XMM1	 = _mm_mul_ps(XMM1, XMM1);
				XMM3	 = _mm_mul_ps(XMM3, XMM3);
				XMM4	 = _mm_add_ps(XMM4, XMM5);
				XMM1	 = _mm_add_ps(XMM1, XMM3);
				XMM0	 = _mm_cvtpi32_ps(XMM0, MM0);
				XMM2	 = _mm_cvtpi32_ps(XMM2, MM1);
				U2.ps	 = XMM4;
				U3.ps	 = XMM1;
				MM4		 = U2.pi64[1];
				MM5		 = U3.pi64[1];
				MM6		 = U2.pi64[0];
				MM7		 = U3.pi64[0];
				XMM5	 = _mm_cvtpi32_ps(XMM5, MM4);
				XMM3	 = _mm_cvtpi32_ps(XMM3, MM5);
				XMM0	 = _mm_movelh_ps(XMM0, XMM0);
				XMM2	 = _mm_movelh_ps(XMM2, XMM2);
				XMM5	 = _mm_movelh_ps(XMM5, XMM5);
				XMM3	 = _mm_movelh_ps(XMM3, XMM3);
				XMM0	 = _mm_cvtpi32_ps(XMM0, MM2);
				XMM2	 = _mm_cvtpi32_ps(XMM2, MM3);
				XMM5	 = _mm_cvtpi32_ps(XMM5, MM6);
				XMM3	 = _mm_cvtpi32_ps(XMM3, MM7);
				XMM0	 = _mm_mul_ps(XMM0, PM128(mparm));
				XMM2	 = _mm_mul_ps(XMM2, PM128(mparm));
				XMM5	 = _mm_mul_ps(XMM5, PM128(mparm));
				XMM3	 = _mm_mul_ps(XMM3, PM128(mparm));
				XMM0	 = _mm_add_ps(XMM0, SCALEdB);
				XMM2	 = _mm_add_ps(XMM2, SCALEdB);
				XMM5	 = _mm_add_ps(XMM5, SCALEdB);
				XMM3	 = _mm_add_ps(XMM3, SCALEdB);
				_mm_store_ps(logfft+k   , XMM0);
				_mm_store_ps(logfft+k+ 4, XMM2);
				_mm_store_ps(logfft+k+ 8, XMM5);
				_mm_store_ps(logfft+k+12, XMM3);
				XMM0		 = _mm_max_ps(XMM0, XMM2);
				XMM5		 = _mm_max_ps(XMM5, XMM3);
				LAM0		 = _mm_max_ps(LAM0, XMM0);
				LAM1		 = _mm_max_ps(LAM1, XMM5);
			}
		}
		_mm_empty();
		logfft[0]	 = rfv;
		LAM0		 = _mm_max_ps(LAM0, LAM1);
		local_ampmax[i]	 = _mm_max_horz(LAM0);
	}
	else
	{
		__m64	MM0, MM1, MM2, MM3;
		__m128x	U0, U1;
		int Cnt	 = ((n-2)&(~15))+1;
		for(j=1;j<Cnt;j+=16){
			__m128	XMM0, XMM3;
			{
				__m128	XMM2, XMM5;
				{
					__m128	XMM1, XMM4;
					XMM0	 = _mm_loadu_ps(pcm+j   );
					XMM1	 = _mm_loadu_ps(pcm+j+ 4);
					XMM3	 = _mm_loadu_ps(pcm+j+ 8);
					XMM4	 = _mm_loadu_ps(pcm+j+12);
					XMM2	 = XMM0;
					XMM5	 = XMM3;
					XMM0	 = _mm_shuffle_ps(XMM0, XMM1,_MM_SHUFFLE(2,0,2,0));
					XMM2	 = _mm_shuffle_ps(XMM2, XMM1,_MM_SHUFFLE(3,1,3,1));
					XMM3	 = _mm_shuffle_ps(XMM3, XMM4,_MM_SHUFFLE(2,0,2,0));
					XMM5	 = _mm_shuffle_ps(XMM5, XMM4,_MM_SHUFFLE(3,1,3,1));
				}
				XMM0	 = _mm_mul_ps(XMM0, XMM0);
				XMM3	 = _mm_mul_ps(XMM3, XMM3);
				XMM2	 = _mm_mul_ps(XMM2, XMM2);
				XMM5	 = _mm_mul_ps(XMM5, XMM5);
				XMM0	 = _mm_add_ps(XMM0, XMM2);
				XMM3	 = _mm_add_ps(XMM3, XMM5);
			}
			U0.ps	 = XMM0;
			U1.ps	 = XMM3;
			MM0		 = U0.pi64[1];
			MM1		 = U1.pi64[1];
			MM2		 = U0.pi64[0];
			MM3		 = U1.pi64[0];
			XMM0	 = _mm_cvtpi32_ps(XMM0, MM0);
			XMM3	 = _mm_cvtpi32_ps(XMM3, MM1);
			XMM0	 = _mm_movelh_ps(XMM0, XMM0);
			XMM3	 = _mm_movelh_ps(XMM3, XMM3);
			XMM0	 = _mm_cvtpi32_ps(XMM0, MM2);
			XMM3	 = _mm_cvtpi32_ps(XMM3, MM3);
			XMM0	 = _mm_mul_ps(XMM0, PM128(mparm));
			XMM3	 = _mm_mul_ps(XMM3, PM128(mparm));
			XMM0	 = _mm_add_ps(XMM0, SCALEdB);
			XMM3	 = _mm_add_ps(XMM3, SCALEdB);
			_mm_storeu_ps(logfft+((j+1)>>1), XMM0);
			_mm_storeu_ps(logfft+((j+9)>>1), XMM3);
			LAM0		 = _mm_max_ps(LAM0, XMM0);
			LAM1		 = _mm_max_ps(LAM1, XMM3);
		}
		Cnt	 = ((n-2)&(~7))+1;
		for(;j<Cnt;j+=8){
			__m128	XMM0;
			{
				__m128	XMM2;
				{
					__m128	XMM1;
					XMM0	 = _mm_loadu_ps(pcm+j   );
					XMM1	 = _mm_loadu_ps(pcm+j+ 4);
					XMM2	 = XMM0;
					XMM0	 = _mm_shuffle_ps(XMM0, XMM1,_MM_SHUFFLE(2,0,2,0));
					XMM2	 = _mm_shuffle_ps(XMM2, XMM1,_MM_SHUFFLE(3,1,3,1));
				}
				XMM0	 = _mm_mul_ps(XMM0, XMM0);
				XMM2	 = _mm_mul_ps(XMM2, XMM2);
				XMM0	 = _mm_add_ps(XMM0, XMM2);
			}
			U0.ps	 = XMM0;
			MM0		 = U0.pi64[1];
			MM1		 = U0.pi64[0];
			XMM0	 = _mm_cvtpi32_ps(XMM0, MM0);
			XMM0	 = _mm_movelh_ps(XMM0, XMM0);
			XMM0	 = _mm_cvtpi32_ps(XMM0, MM1);
			XMM0	 = _mm_mul_ps(XMM0, PM128(mparm));
			XMM0	 = _mm_add_ps(XMM0, SCALEdB);
			_mm_storeu_ps(logfft+((j+1)>>1), XMM0);
			LAM0		 = _mm_max_ps(LAM0, XMM0);
		}
		LAM0		 = _mm_max_ps(LAM0, LAM1);
		_mm_empty();
		local_ampmax[i]	 = _mm_max_horz(LAM0);
		for(;j<n;j+=2){
			float	temp	 = pcm[j]*pcm[j]+pcm[j+1]*pcm[j+1];
			temp=logfft[(j+1)>>1]=scale_dB+.5f*todB(&temp)  + .345; /* +
										.345 is a hack; the original todB
										estimation used on IEEE 754
										compliant machines had a bug that
										returned dB values about a third
										of a decibel too high.  The bug
										was harmless because tunings
										implicitly took that into
										account.  However, fixing the bug
										in the estimator requires
										changing all the tunings as well.
										For now, it's easier to sync
										things back up here, and
										recalibrate the tunings in the
										next major model upgrade. */
			if(temp>local_ampmax[i])
				local_ampmax[i]	 = temp;
		}
	}
#endif
}

static void mapping_forward_sub1(float *mdct, float *logmdct, int n)
{
	static _MM_ALIGN16 const float mparm[4]	 = {
		7.17711438e-7f, 7.17711438e-7f, 7.17711438e-7f, 7.17711438e-7f
	};
	static _MM_ALIGN16 const float PFV0[4]	 = {
		0.345f-764.6161886f,	0.345f-764.6161886f,
		0.345f-764.6161886f,	0.345f-764.6161886f
	};
	int j;
#if	defined(__SSE2__)
	/*
		SSE2 optimized code
	*/
	for(j=0;j<n/2;j+=16)
	{
		__m128	XMM0, XMM1, XMM2, XMM3;
		XMM0	 = _mm_load_ps(mdct+j   );
		XMM1	 = _mm_load_ps(mdct+j+ 4);
		XMM2	 = _mm_load_ps(mdct+j+ 8);
		XMM3	 = _mm_load_ps(mdct+j+12);
		XMM0	 = _mm_and_ps(XMM0, PM128(PABSMASK));
		XMM1	 = _mm_and_ps(XMM1, PM128(PABSMASK));
		XMM2	 = _mm_and_ps(XMM2, PM128(PABSMASK));
		XMM3	 = _mm_and_ps(XMM3, PM128(PABSMASK));
		XMM0	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM0));
		XMM1	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM1));
		XMM2	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM2));
		XMM3	 = _mm_cvtepi32_ps(_mm_castps_si128(XMM3));
		XMM0	 = _mm_mul_ps(XMM0, PM128(mparm));
		XMM1	 = _mm_mul_ps(XMM1, PM128(mparm));
		XMM2	 = _mm_mul_ps(XMM2, PM128(mparm));
		XMM3	 = _mm_mul_ps(XMM3, PM128(mparm));
		XMM0	 = _mm_add_ps(XMM0, PM128(PFV0));
		XMM1	 = _mm_add_ps(XMM1, PM128(PFV0));
		XMM2	 = _mm_add_ps(XMM2, PM128(PFV0));
		XMM3	 = _mm_add_ps(XMM3, PM128(PFV0));
		_mm_store_ps(logmdct+j   , XMM0);
		_mm_store_ps(logmdct+j+ 4, XMM1);
		_mm_store_ps(logmdct+j+ 8, XMM2);
		_mm_store_ps(logmdct+j+12, XMM3);
	}
#else	/* __SSE2__ */
	/*
		SSE optimized code
	*/
	for(j=0;j<n/2;j+=16)
	{
		__m128x	U0, U1, U2, U3;
		__m128	XMM0, XMM1, XMM2, XMM3;
		XMM0	 = _mm_load_ps(mdct+j   );
		XMM1	 = _mm_load_ps(mdct+j+ 4);
		XMM2	 = _mm_load_ps(mdct+j+ 8);
		XMM3	 = _mm_load_ps(mdct+j+12);
		XMM0	 = _mm_and_ps(XMM0, PM128(PABSMASK));
		XMM1	 = _mm_and_ps(XMM1, PM128(PABSMASK));
		XMM2	 = _mm_and_ps(XMM2, PM128(PABSMASK));
		XMM3	 = _mm_and_ps(XMM3, PM128(PABSMASK));
		U0.ps	 = XMM0;
		U1.ps	 = XMM1;
		U2.ps	 = XMM2;
		U3.ps	 = XMM3;
		XMM0	 = _mm_cvtpi32_ps(XMM0, U0.pi64[1]);
		XMM1	 = _mm_cvtpi32_ps(XMM1, U1.pi64[1]);
		XMM2	 = _mm_cvtpi32_ps(XMM2, U2.pi64[1]);
		XMM3	 = _mm_cvtpi32_ps(XMM3, U3.pi64[1]);
		XMM0	 = _mm_movelh_ps(XMM0, XMM0);
		XMM1	 = _mm_movelh_ps(XMM1, XMM1);
		XMM2	 = _mm_movelh_ps(XMM2, XMM2);
		XMM3	 = _mm_movelh_ps(XMM3, XMM3);
		XMM0	 = _mm_cvtpi32_ps(XMM0, U0.pi64[0]);
		XMM1	 = _mm_cvtpi32_ps(XMM1, U1.pi64[0]);
		XMM2	 = _mm_cvtpi32_ps(XMM2, U2.pi64[0]);
		XMM3	 = _mm_cvtpi32_ps(XMM3, U3.pi64[0]);
		XMM0	 = _mm_mul_ps(XMM0, PM128(mparm));
		XMM1	 = _mm_mul_ps(XMM1, PM128(mparm));
		XMM2	 = _mm_mul_ps(XMM2, PM128(mparm));
		XMM3	 = _mm_mul_ps(XMM3, PM128(mparm));
		XMM0	 = _mm_add_ps(XMM0, PM128(PFV0));
		XMM1	 = _mm_add_ps(XMM1, PM128(PFV0));
		XMM2	 = _mm_add_ps(XMM2, PM128(PFV0));
		XMM3	 = _mm_add_ps(XMM3, PM128(PFV0));
		_mm_store_ps(logmdct+j   , XMM0);
		_mm_store_ps(logmdct+j+ 4, XMM1);
		_mm_store_ps(logmdct+j+ 8, XMM2);
		_mm_store_ps(logmdct+j+12, XMM3);
	}
	_mm_empty();
#endif
}
#endif														/* SSE Optimize */

#if	defined(_OPENMP)&&defined(__SSE__)
#endif

static int mapping0_forward(vorbis_block *vb){
  vorbis_dsp_state      *vd=vb->vd;
  vorbis_info           *vi=vd->vi;
  codec_setup_info      *ci=vi->codec_setup;
  private_state         *b=vb->vd->backend_state;
  vorbis_block_internal *vbi=(vorbis_block_internal *)vb->internal;
  vorbis_info_floor1    *vif=ci->floor_param[vb->W];
  int                    n=vb->pcmend;
  int i,j,k;

  int    *nonzero    = alloca(sizeof(*nonzero)*vi->channels);
#if	defined(_OPENMP)&&defined(__SSE__)
  float **gmdct            = alloca(vi->channels*sizeof(*gmdct));
  float **gmdct_org        = alloca(vi->channels*sizeof(*gmdct_org));
  float **res_org          = alloca(vi->channels*sizeof(*res_org));
  int **ilogmaskch         = alloca(vi->channels*sizeof(*ilogmaskch));
  int ***floor_posts       = alloca(vi->channels*sizeof(*floor_posts));
  int ***floor_posts_org   = alloca(vi->channels*sizeof(*floor_posts_org));
  int *sflag0              = alloca(vi->channels*sizeof(sflag0));
  int *sflag1              = alloca(vi->channels*sizeof(sflag1));
  int *sflag2              = alloca(vi->channels*sizeof(sflag2));
  int *sflag3              = alloca(vi->channels*sizeof(sflag3));
  int *exitflag            = alloca(vi->channels*sizeof(exitflag));
  float **res_bundle       = alloca(vi->channels*sizeof(*res_bundle));
  float **couple_bundle    = alloca(vi->channels*sizeof(*couple_bundle));
  int *zerobundle          = alloca(vi->channels*sizeof(zerobundle));
  int **sortindex          = alloca(vi->channels*sizeof(*sortindex));
  oggpack_writecache *opwc = alloca(vi->channels*sizeof(oggpack_writecache));
  int samples_per_partition;
  int resn;
  long **partword;
  int ch_in_bundle;
  int resnum;

  float global_ampmax    = vbi->ampmax;
  float *local_ampmax    = alloca(vi->channels*sizeof(*local_ampmax));
  int blocktype          = vbi->blocktype;

  int modenumber=vb->W;
  vorbis_info_mapping0 *info=ci->map_param[modenumber];
  vorbis_look_psy *psy_look=
	b->psy+blocktype+(vb->W?2:0);
  float **mag_memo       = alloca(info->coupling_steps*sizeof(*mag_memo));
  int **mag_sort         = alloca(info->coupling_steps*sizeof(*mag_sort));

  vb->mode=modenumber;

  if(info->submaps==1&&!vorbis_bitrate_managed(vb)&&vi->channels>1)
  {
    /*
      for fast reduce(info->submaps==1)
    */
    vorbis_look_residue0 *rlook;
    vorbis_info_residue0 *rinfo;
    resnum=info->residuesubmap[0];
    rlook=(vorbis_look_residue0 *)(b->residue[resnum]);
    rinfo=rlook->info;
    samples_per_partition=rinfo->grouping;
    resn=rinfo->end-rinfo->begin;
    partword = alloca(vi->channels*sizeof(*partword));
    partword[0] = _ogg_alloca(resn*vi->channels/samples_per_partition*sizeof(*partword[0]));
    memset(partword[0],0,resn*vi->channels/samples_per_partition*sizeof(*partword[0]));
  }

  memset(sortindex,0,sizeof(*sortindex)*vi->channels);
  for(i=0;i<vi->channels;i++){
	gmdct[i]           = _ogg_alloca(n/2*sizeof(**gmdct));
	gmdct_org[i]       = _ogg_alloca(n/2*sizeof(**gmdct_org));
	res_org[i]         = _ogg_alloca(n/2*sizeof(**res_org));
	floor_posts[i]     = _ogg_alloca(PACKETBLOBS*sizeof(**floor_posts));
	floor_posts_org[i] = _ogg_alloca(PACKETBLOBS*sizeof(**floor_posts_org));
	ilogmaskch[i]      = _ogg_alloca(n/2*sizeof(**ilogmaskch));
	sortindex[i]       = _ogg_alloca(n/2*sizeof(**sortindex));
	sflag0[i]          = 0;
	sflag1[i]          = 0;
	sflag2[i]          = 0;
	sflag3[i]          = 0;
	exitflag[i]        = 0;
	opwc[i].count      = 0;
	opwc[i].data       = _ogg_alloca(258*sizeof(oggpack_writeinfo));
	for(j=0;j<PACKETBLOBS;j++)
	{
		floor_posts_org[i][j] = _ogg_alloca(128*sizeof(int));
	}
  }
  for(i=0;i<info->coupling_steps;i++){
	mag_memo[i]        = _ogg_alloca(psy_look->n*sizeof(**mag_memo));
	mag_sort[i]        = _ogg_alloca(psy_look->n*sizeof(**mag_sort));
  }
#pragma omp parallel for private(j, k) if(1) num_threads(vi->channels)
  for(i=0;i<vi->channels;i++){
	int thmax = omp_get_num_threads();
	int thnum = omp_get_thread_num();
	float global_ampmax=vbi->ampmax;
	float scale=4.f/n;
	float scale_dB;

	float *pcm     =vb->pcm[i]; 
	float *logfft  =pcm;

	scale_dB=todB(&scale) + .345;

	_vorbis_apply_window(pcm,b->window,ci->blocksizes,vb->lW,vb->W,vb->nW);

	mdct_forward(b->transform[vb->W][0],pcm,gmdct[i], gmdct_org[i]);

	drft_forward(&b->fft_look[vb->W],pcm);
	logfft[0]=scale_dB+todB(pcm)  + .345;
	local_ampmax[i]=logfft[0];
	mapping_forward_sub0(pcm, logfft, scale_dB, local_ampmax, i, n);

	if(local_ampmax[i]>0.f)local_ampmax[i]=0.f;
	sflag0[i] = 1;
	{
	  float *noise    = _ogg_alloca(n/2*sizeof(*noise));
	  float *tone     = _ogg_alloca(n/2*sizeof(*tone));
	  int submap      = info->chmuxlist[i];
	  float *mdct     = gmdct[i];
	  float *logfft   = vb->pcm[i];
	  float *logmdct  = logfft+n/2;
	  float *logmask  = logfft;
	  float *lastmdct = b->nblock+i*128;
	  float *tempmdct = b->tblock+i*128;
	  float *lowcomp  = b->lownoise_compand_level+i;

	  vb->mode=modenumber;

	  memset(floor_posts[i],0,sizeof(**floor_posts)*PACKETBLOBS);
	  mapping_forward_sub1(mdct, logmdct, n);

	  *lowcomp=
		lb_loudnoise_fix(psy_look,
		*lowcomp,
		logmdct,
		b->lW_modenumber,
		blocktype, modenumber);

	  _vp_noisemask(psy_look,
		*lowcomp,
		logmdct,
		noise);

SLOOP0:
	  for(j=0;j<vi->channels;j++)
	  {
		int s;
		if(i==j)continue;
		s = sflag0[j];
		if(!s)
		{
		  Sleep(0);
#if	!defined(__PROF__)
		  goto SLOOP0;
#endif
		}
	  }
	  for(j=0;j<vi->channels;j++)
		if(local_ampmax[j]>global_ampmax)global_ampmax=local_ampmax[j];

	  _vp_tonemask(psy_look,
		logfft,
		tone,
		global_ampmax,
		local_ampmax[i]);

	  _vp_offset_and_mix(psy_look,
		noise,
		tone,
		1,
		logmask,
		mdct,
		logmdct,
		lastmdct, tempmdct,
		*lowcomp,
		vif->n,
		blocktype, modenumber,
		vb->nW,
		b->lW_blocktype, b->lW_modenumber, b->lW_no,
		res_org[i]);

	  sflag1[i] = 1;
	  if(ci->floor_type[info->floorsubmap[submap]]!=1)
	  {
		exitflag[i] = -1;
		continue;
	  }

	  floor_posts[i][PACKETBLOBS/2]=
		floor1_fit(vb,b->flr[info->floorsubmap[submap]],
		logmdct,
		logmask,
		floor_posts_org[i][PACKETBLOBS/2]);

	  if(vorbis_bitrate_managed(vb) && floor_posts[i][PACKETBLOBS/2])
	  {
		_vp_offset_and_mix(psy_look,
		  noise,
		  tone,
		  2,
		  logmask,
		  mdct,
		  logmdct,
		  lastmdct, tempmdct,
		  *lowcomp,
		  vif->n,
		  blocktype, modenumber,
		  vb->nW,
		  b->lW_blocktype, b->lW_modenumber, b->lW_no,
		  res_org[i]);

		floor_posts[i][PACKETBLOBS-1]=
		  floor1_fit(vb,b->flr[info->floorsubmap[submap]],
		  logmdct,
		  logmask,
		  floor_posts_org[i][PACKETBLOBS-1]);

		_vp_offset_and_mix(psy_look,
		  noise,
		  tone,
		  0,
		  logmask,
		  mdct,
		  logmdct,
		  lastmdct, tempmdct,
		  *lowcomp,
		  vif->n,
		  blocktype, modenumber,
		  vb->nW,
		  b->lW_blocktype, b->lW_modenumber, b->lW_no,
		  res_org[i]);

		floor_posts[i][0]=
		  floor1_fit(vb,b->flr[info->floorsubmap[submap]],
		  logmdct,
		  logmask,
		  floor_posts_org[i][0]);

		for(k=1;k<PACKETBLOBS/2;k++)
		  floor_posts[i][k]=
		  floor1_interpolate_fit(vb,b->flr[info->floorsubmap[submap]],
		  floor_posts[i][0],
		  floor_posts[i][PACKETBLOBS/2],
		  k*65536/(PACKETBLOBS/2),
		  floor_posts_org[i][k]);
		for(k=PACKETBLOBS/2+1;k<PACKETBLOBS-1;k++)
		  floor_posts[i][k]=
		  floor1_interpolate_fit(vb,b->flr[info->floorsubmap[submap]],
		  floor_posts[i][PACKETBLOBS/2],
		  floor_posts[i][PACKETBLOBS-1],
		  (k-PACKETBLOBS/2)*65536/(PACKETBLOBS/2),
		  floor_posts_org[i][k]);
	  }
	}
	if(thnum==0)
	{
	  vbi->ampmax=global_ampmax;
	}
SLOOP1:
	for(j=0;j<vi->channels;j++)
	{
	  int s;
	  if(i==j)continue;
		s = sflag1[j];
	  if(!s)
	  {
		Sleep(0);
#if	!defined(__PROF__)
		goto SLOOP1;
#endif
	  }
	}
	if(info->coupling_steps){
	  mag_memo=_vp_quantize_couple_memo(vb,
		&ci->psy_g_param,
		psy_look,
		info,
		gmdct,
		mag_memo, thnum, thmax);

	  mag_sort=_vp_quantize_couple_sort(vb,
		psy_look,
		info,
		mag_memo,
		res_org[i],
		mag_sort, thnum, thmax);
	}
	if(psy_look->vi->normal_channel_p){
	  float *mdct    =gmdct[i];
	  _vp_noise_normalize_sort(psy_look,mdct,sortindex[i],res_org[i]);
	}
	if(!vorbis_bitrate_managed(vb))	/* for VBR */
	{
	  int submap=info->chmuxlist[i];
	  float *mdct    =gmdct[i];
	  float *mdct_org=gmdct_org[i];
	  float *res     =vb->pcm[i];
	  float *resorgch=res_org[i];
	  int   *ilogmask=ilogmaskch[i];

	  k=PACKETBLOBS/2;
	  nonzero[i]=floor1_encode(NULL,vb,b->flr[info->floorsubmap[submap]],
		floor_posts[i][k],
		ilogmask, &opwc[i]);
	  _vp_remove_floor(psy_look,
		mdct,
		ilogmask,
		res,
		ci->psy_g_param.sliding_lowpass[vb->W][k]);

	  _vp_remove_floor(psy_look,
		mdct_org,
		ilogmask,
		resorgch,
		ci->psy_g_param.sliding_lowpass[vb->W][k]);

	  _vp_noise_normalize(psy_look,res,res+n/2,sortindex[i]);
	  sflag2[i] = 1;
SLOOP2:
	  for(j=0;j<vi->channels;j++)
	  {
		int s;
		if(i==j)continue;
		  s = sflag2[j];
		if(!s)
		{
		  Sleep(0);
#if	!defined(__PROF__)
		  goto SLOOP2;
#endif
		}
	  }
#if	!defined(__PROF__)
	  if(info->coupling_steps){
#else
	  if(info->coupling_steps&&i==vi->channels-1){
#endif
		_vp_couple(k,
		  &ci->psy_g_param,
		  psy_look,
		  info,
		  vb->pcm,
		  mag_memo,
		  mag_sort,
		  ilogmaskch,
		  nonzero,
		  ci->psy_g_param.sliding_lowpass[vb->W][k],
#if	!defined(__PROF__)
		  gmdct, res_org, thnum, thmax);
#else
		  gmdct, res_org, 0, 1);
#endif
	  }
	  if(info->submaps==1&&vi->channels>1)
	  {
		if(thnum==0)
		{
		  ch_in_bundle=0;
		  for(j=0;j<vi->channels;j++){
			if(info->chmuxlist[j]==0){
			  zerobundle[ch_in_bundle]=0;
			  if(nonzero[j])zerobundle[ch_in_bundle]=1;
			  res_bundle[ch_in_bundle]=vb->pcm[j];
			  couple_bundle[ch_in_bundle++]=vb->pcm[j]+n/2;
			}
		  }
		}
		sflag3[i] = 1;
SLOOP3:
		for(j=0;j<vi->channels;j++)
		{
		  int s;
		  if(i==j)continue;
			s = sflag3[j];
		  if(!s)
		  {
			Sleep(0);
#if	!defined(__PROF__)
			goto SLOOP3;
#endif
		  }
		}
		_residue_mt_P[ci->residue_type[resnum]]->
		class(vb,b->residue[resnum],couple_bundle,zerobundle,ch_in_bundle, partword,thnum,thmax);
	  }
	}
  }	/* end of parallel for */

  if(!vorbis_bitrate_managed(vb))	/* for VBR */
  {
	oggpack_buffer *opb;
	k=PACKETBLOBS/2;

	opb=vbi->packetblob[k];
	oggpack_write(opb,0,1);
	oggpack_write(opb,modenumber,b->modebits);
	if(vb->W){
	  oggpack_write(opb,vb->lW,1);
	  oggpack_write(opb,vb->nW,1);
	}

	for(i=0;i<vi->channels;i++){
	  vorbis_oggpack_cacheflush(&opwc[i], opb);
	}

	if(info->submaps==1&&vi->channels>1)
	{
	  _residue_P[ci->residue_type[resnum]]->
		forward(opb,vb,b->residue[resnum],
		couple_bundle,NULL,zerobundle,ch_in_bundle,partword);
	}
	else
	{
	  for(i=0;i<info->submaps;i++){
		long **classifications;
		ch_in_bundle=0;
		resnum=info->residuesubmap[i];

		for(j=0;j<vi->channels;j++){
		  if(info->chmuxlist[j]==i){
			zerobundle[ch_in_bundle]=0;
			if(nonzero[j])zerobundle[ch_in_bundle]=1;
			res_bundle[ch_in_bundle]=vb->pcm[j];
			couple_bundle[ch_in_bundle++]=vb->pcm[j]+n/2;
		  }
		}

		classifications=_residue_P[ci->residue_type[resnum]]->
		class(vb,b->residue[resnum],couple_bundle,zerobundle,ch_in_bundle);

		_residue_P[ci->residue_type[resnum]]->
		  forward(opb,vb,b->residue[resnum],
		  couple_bundle,NULL,zerobundle,ch_in_bundle,classifications);
	  }
	}

	if((blocktype == b->lW_blocktype) && (modenumber == b->lW_modenumber)) b->lW_no++;
	else b->lW_no = 1;
	b->lW_blocktype = blocktype;
	b->lW_modenumber = modenumber;
  }
  else	/* for ABR */
  {
	for(k=(vorbis_bitrate_managed(vb)?0:PACKETBLOBS/2);
	  k<=(vorbis_bitrate_managed(vb)?PACKETBLOBS-1:PACKETBLOBS/2);
	  k++){
		oggpack_buffer *opb;

		for(i=0;i<vi->channels;i++){
		  int submap=info->chmuxlist[i];
		  float *mdct    =gmdct[i];
		  float *mdct_org=gmdct_org[i];
		  float *res     =vb->pcm[i];
		  float *resorgch=res_org[i];
		  int   *ilogmask=ilogmaskch[i];

		  nonzero[i]=floor1_encode(opb,vb,b->flr[info->floorsubmap[submap]],
			floor_posts[i][k],
			ilogmask, &opwc[i]);
		  _vp_remove_floor(psy_look,
			mdct,
			ilogmask,
			res,
			ci->psy_g_param.sliding_lowpass[vb->W][k]);

		  _vp_remove_floor(psy_look,
			mdct_org,
			ilogmask,
			resorgch,
			ci->psy_g_param.sliding_lowpass[vb->W][k]);

		  _vp_noise_normalize(psy_look,res,res+n/2,sortindex[i]);
		}

		opb=vbi->packetblob[k];
		oggpack_write(opb,0,1);
		oggpack_write(opb,modenumber,b->modebits);
		if(vb->W){
		  oggpack_write(opb,vb->lW,1);
		  oggpack_write(opb,vb->nW,1);
		}

		for(i=0;i<vi->channels;i++){
		  vorbis_oggpack_cacheflush(&opwc[i], opb);
		}
		if(info->coupling_steps){
		  _vp_couple(k,
			&ci->psy_g_param,
			psy_look,
			info,
			vb->pcm,
			mag_memo,
			mag_sort,
			ilogmaskch,
			nonzero,
			ci->psy_g_param.sliding_lowpass[vb->W][k],
			gmdct, res_org, 0,1);
		}

		for(i=0;i<info->submaps;i++){
		  long **classifications;
		  ch_in_bundle=0;
		  resnum=info->residuesubmap[i];

		  for(j=0;j<vi->channels;j++){
			if(info->chmuxlist[j]==i){
			  zerobundle[ch_in_bundle]=0;
			  if(nonzero[j])zerobundle[ch_in_bundle]=1;
			  res_bundle[ch_in_bundle]=vb->pcm[j];
			  couple_bundle[ch_in_bundle++]=vb->pcm[j]+n/2;
			}
		  }

		  classifications=_residue_P[ci->residue_type[resnum]]->
		  class(vb,b->residue[resnum],couple_bundle,zerobundle,ch_in_bundle);

		  _residue_P[ci->residue_type[resnum]]->
			forward(opb,vb,b->residue[resnum],
			couple_bundle,NULL,zerobundle,ch_in_bundle,classifications);
		}

		/* set last-window type & number */
		if((blocktype == b->lW_blocktype) && (modenumber == b->lW_modenumber)) b->lW_no++;
		else b->lW_no = 1;
		b->lW_blocktype = blocktype;
		b->lW_modenumber = modenumber;
		/* ok, done encoding.  Next protopacket. */
	}
  }

  for(i=0;i<vi->channels;i++){
	if(exitflag[i])return(-1);
  }
#else	/* for _OPENMP */
  float  **gmdct     = _vorbis_block_alloc(vb,vi->channels*sizeof(*gmdct));
  float  **gmdct_org = _vorbis_block_alloc(vb,vi->channels*sizeof(*gmdct_org));
  float  **res_org   = _vorbis_block_alloc(vb,vi->channels*sizeof(*res_org));
  int    **ilogmaskch= _vorbis_block_alloc(vb,vi->channels*sizeof(*ilogmaskch));
  int ***floor_posts = _vorbis_block_alloc(vb,vi->channels*sizeof(*floor_posts));
  
  float global_ampmax=vbi->ampmax;
  float *local_ampmax=alloca(sizeof(*local_ampmax)*vi->channels);
  int blocktype=vbi->blocktype;

  int modenumber=vb->W;
  vorbis_info_mapping0 *info=ci->map_param[modenumber];
  vorbis_look_psy *psy_look=
    b->psy+blocktype+(vb->W?2:0);

  vb->mode=modenumber;

  for(i=0;i<vi->channels;i++){
    float scale=4.f/n;
    float scale_dB;

    float *pcm     =vb->pcm[i]; 
    float *logfft  =pcm;

    gmdct[i]=_vorbis_block_alloc(vb,n/2*sizeof(**gmdct));
    gmdct_org[i]=_vorbis_block_alloc(vb,n/2*sizeof(**gmdct_org));
    res_org[i]=_vorbis_block_alloc(vb,n/2*sizeof(**res_org));

    scale_dB=todB(&scale) + .345; /* + .345 is a hack; the original
                                     todB estimation used on IEEE 754
                                     compliant machines had a bug that
                                     returned dB values about a third
                                     of a decibel too high.  The bug
                                     was harmless because tunings
                                     implicitly took that into
                                     account.  However, fixing the bug
                                     in the estimator requires
                                     changing all the tunings as well.
                                     For now, it's easier to sync
                                     things back up here, and
                                     recalibrate the tunings in the
                                     next major model upgrade. */

#if 0
    if(vi->channels==2)
      if(i==0)
	_analysis_output("pcmL",seq,pcm,n,0,0,total-n/2);
      else
	_analysis_output("pcmR",seq,pcm,n,0,0,total-n/2);
#endif
  
    /* window the PCM data */
    _vorbis_apply_window(pcm,b->window,ci->blocksizes,vb->lW,vb->W,vb->nW);

#if 0
    if(vi->channels==2)
      if(i==0)
	_analysis_output("windowedL",seq,pcm,n,0,0,total-n/2);
      else
	_analysis_output("windowedR",seq,pcm,n,0,0,total-n/2);
#endif

    /* transform the PCM data */
    /* only MDCT right now.... */
#if	defined(__SSE__)										/* SSE Optimize */
    mdct_forward(b->transform[vb->W][0],pcm,gmdct[i], gmdct_org[i]);
#else														/* SSE Optimize */
    mdct_forward(b->transform[vb->W][0],pcm,gmdct[i]);
    memcpy(gmdct_org[i], gmdct[i], n/2*sizeof(**gmdct_org));
#endif														/* SSE Optimize */
    
    /* FFT yields more accurate tonal estimation (not phase sensitive) */
    drft_forward(&b->fft_look[vb->W],pcm);
    logfft[0]=scale_dB+todB(pcm)  + .345; /* + .345 is a hack; the
                                     original todB estimation used on
                                     IEEE 754 compliant machines had a
                                     bug that returned dB values about
                                     a third of a decibel too high.
                                     The bug was harmless because
                                     tunings implicitly took that into
                                     account.  However, fixing the bug
                                     in the estimator requires
                                     changing all the tunings as well.
                                     For now, it's easier to sync
                                     things back up here, and
                                     recalibrate the tunings in the
                                     next major model upgrade. */
    local_ampmax[i]=logfft[0];
#ifdef __SSE__												/* SSE Optimize */
	mapping_forward_sub0(pcm, logfft, scale_dB, local_ampmax, i, n);
#else														/* SSE Optimize */
    for(j=1;j<n-1;j+=2){
      float temp=pcm[j]*pcm[j]+pcm[j+1]*pcm[j+1];
      temp=logfft[(j+1)>>1]=scale_dB+.5f*todB(&temp)  + .345; /* +
                                     .345 is a hack; the original todB
                                     estimation used on IEEE 754
                                     compliant machines had a bug that
                                     returned dB values about a third
                                     of a decibel too high.  The bug
                                     was harmless because tunings
                                     implicitly took that into
                                     account.  However, fixing the bug
                                     in the estimator requires
                                     changing all the tunings as well.
                                     For now, it's easier to sync
                                     things back up here, and
                                     recalibrate the tunings in the
                                     next major model upgrade. */
      if(temp>local_ampmax[i])local_ampmax[i]=temp;
    }
#endif														/* SSE Optimize */

    if(local_ampmax[i]>0.f)local_ampmax[i]=0.f;
    if(local_ampmax[i]>global_ampmax)global_ampmax=local_ampmax[i];

#if 0
    if(vi->channels==2){
      if(i==0){
	_analysis_output("fftL",seq,logfft,n/2,1,0,0);
      }else{
	_analysis_output("fftR",seq,logfft,n/2,1,0,0);
      }
    }
#endif

  }
  
  {
    float   *noise        = _vorbis_block_alloc(vb,n/2*sizeof(*noise));
    float   *tone         = _vorbis_block_alloc(vb,n/2*sizeof(*tone));
    
    for(i=0;i<vi->channels;i++){
      /* the encoder setup assumes that all the modes used by any
	 specific bitrate tweaking use the same floor */
      
      int submap=info->chmuxlist[i];
      
      /* the following makes things clearer to *me* anyway */
      float *mdct    =gmdct[i];
      float *logfft  =vb->pcm[i];
      
      float *logmdct =logfft+n/2;
      float *logmask =logfft;
      
      float *lastmdct = b->nblock+i*128;
      float *tempmdct = b->tblock+i*128;
      
      float *lowcomp = b->lownoise_compand_level+i;

      vb->mode=modenumber;

      floor_posts[i]=_vorbis_block_alloc(vb,PACKETBLOBS*sizeof(**floor_posts));
      memset(floor_posts[i],0,sizeof(**floor_posts)*PACKETBLOBS);
      
#ifdef __SSE__												/* SSE Optimize */
	mapping_forward_sub1(mdct, logmdct, n);
#else														/* SSE Optimize */
      for(j=0;j<n/2;j++)
	logmdct[j]=todB(mdct+j)  + .345; /* + .345 is a hack; the original
                                     todB estimation used on IEEE 754
                                     compliant machines had a bug that
                                     returned dB values about a third
                                     of a decibel too high.  The bug
                                     was harmless because tunings
                                     implicitly took that into
                                     account.  However, fixing the bug
                                     in the estimator requires
                                     changing all the tunings as well.
                                     For now, it's easier to sync
                                     things back up here, and
                                     recalibrate the tunings in the
                                     next major model upgrade. */
#endif														/* SSE Optimize */

#if 0
      if(vi->channels==2){
	if(i==0)
	  _analysis_output("mdctL",seq,logmdct,n/2,1,0,0);
	else
	  _analysis_output("mdctR",seq,logmdct,n/2,1,0,0);
      }else{
	_analysis_output("mdct",seq,logmdct,n/2,1,0,0);
      }
#endif 
      
      /* first step; noise masking.  Not only does 'noise masking'
         give us curves from which we can decide how much resolution
         to give noise parts of the spectrum, it also implicitly hands
         us a tonality estimate (the larger the value in the
         'noise_depth' vector, the more tonal that area is) */

      *lowcomp=
      	lb_loudnoise_fix(psy_look,
      			*lowcomp,
      			logmdct,
      			b->lW_modenumber,
      			blocktype, modenumber);
      
      _vp_noisemask(psy_look,
      		*lowcomp,
		    logmdct,
		    noise); /* noise does not have by-frequency offset
                               bias applied yet */
#if 0
      if(vi->channels==2){
	if(i==0)
	  _analysis_output("noiseL",seq,noise,n/2,1,0,0);
	else
	  _analysis_output("noiseR",seq,noise,n/2,1,0,0);
      }
#endif

      /* second step: 'all the other crap'; all the stuff that isn't
         computed/fit for bitrate management goes in the second psy
         vector.  This includes tone masking, peak limiting and ATH */

      _vp_tonemask(psy_look,
		   logfft,
		   tone,
		   global_ampmax,
		   local_ampmax[i]);

#if 0
      if(vi->channels==2){
	if(i==0)
	  _analysis_output("toneL",seq,tone,n/2,1,0,0);
	else
	  _analysis_output("toneR",seq,tone,n/2,1,0,0);
      }
#endif

      /* third step; we offset the noise vectors, overlay tone
	 masking.  We then do a floor1-specific line fit.  If we're
	 performing bitrate management, the line fit is performed
	 multiple times for up/down tweakage on demand. */

#if 0
      {
      float aotuv[psy_look->n];
#endif

	_vp_offset_and_mix(psy_look,
			   noise,
			   tone,
			   1,
			   logmask,
			   mdct,
			   logmdct,
			   lastmdct, tempmdct,
			   *lowcomp,
			   vif->n,
			   blocktype, modenumber,
			   vb->nW,
#ifdef __SSE__												/* SSE Optimize */
			   b->lW_blocktype, b->lW_modenumber, b->lW_no,
			   res_org[i]);
#else														/* SSE Optimize */
			   b->lW_blocktype, b->lW_modenumber, b->lW_no);
#endif														/* SSE Optimize */
	
#if 0
	if(vi->channels==2){
	  if(i==0)
	    _analysis_output("aotuvM1_L",seq,aotuv,psy_look->n,1,1,0);
	  else
	    _analysis_output("aotuvM1_R",seq,aotuv,psy_look->n,1,1,0);
	}
      }
#endif


#if 0
      if(vi->channels==2){
	if(i==0)
	  _analysis_output("mask1L",seq,logmask,n/2,1,0,0);
	else
	  _analysis_output("mask1R",seq,logmask,n/2,1,0,0);
      }
#endif

      /* this algorithm is hardwired to floor 1 for now; abort out if
         we're *not* floor1.  This won't happen unless someone has
         broken the encode setup lib.  Guard it anyway. */
      if(ci->floor_type[info->floorsubmap[submap]]!=1)return(-1);

      floor_posts[i][PACKETBLOBS/2]=
	floor1_fit(vb,b->flr[info->floorsubmap[submap]],
		   logmdct,
		   logmask);
      
      /* are we managing bitrate?  If so, perform two more fits for
         later rate tweaking (fits represent hi/lo) */
      if(vorbis_bitrate_managed(vb) && floor_posts[i][PACKETBLOBS/2]){
	/* higher rate by way of lower noise curve */

	_vp_offset_and_mix(psy_look,
			   noise,
			   tone,
			   2,
			   logmask,
			   mdct,
			   logmdct,
			   lastmdct, tempmdct,
			   *lowcomp,
			   vif->n,
			   blocktype, modenumber,
			   vb->nW,
#ifdef __SSE__												/* SSE Optimize */
			   b->lW_blocktype, b->lW_modenumber, b->lW_no,
			   res_org[i]);
#else														/* SSE Optimize */
			   b->lW_blocktype, b->lW_modenumber, b->lW_no);
#endif														/* SSE Optimize */

#if 0
	if(vi->channels==2){
	  if(i==0)
	    _analysis_output("mask2L",seq,logmask,n/2,1,0,0);
	  else
	    _analysis_output("mask2R",seq,logmask,n/2,1,0,0);
	}
#endif
	
	floor_posts[i][PACKETBLOBS-1]=
	  floor1_fit(vb,b->flr[info->floorsubmap[submap]],
		     logmdct,
		     logmask);
      
	/* lower rate by way of higher noise curve */
	_vp_offset_and_mix(psy_look,
			   noise,
			   tone,
			   0,
			   logmask,
			   mdct,
			   logmdct,
			   lastmdct, tempmdct,
			   *lowcomp,
			   vif->n,
			   blocktype, modenumber,
			   vb->nW,
#ifdef __SSE__												/* SSE Optimize */
			   b->lW_blocktype, b->lW_modenumber, b->lW_no,
			   res_org[i]);
#else														/* SSE Optimize */
			   b->lW_blocktype, b->lW_modenumber, b->lW_no);
#endif														/* SSE Optimize */

#if 0
	if(vi->channels==2)
	  if(i==0)
	    _analysis_output("mask0L",seq,logmask,n/2,1,0,0);
	  else
	    _analysis_output("mask0R",seq,logmask,n/2,1,0,0);
#endif

	floor_posts[i][0]=
	  floor1_fit(vb,b->flr[info->floorsubmap[submap]],
		     logmdct,
		     logmask);
	
	/* we also interpolate a range of intermediate curves for
           intermediate rates */
	for(k=1;k<PACKETBLOBS/2;k++)
	  floor_posts[i][k]=
	    floor1_interpolate_fit(vb,b->flr[info->floorsubmap[submap]],
				   floor_posts[i][0],
				   floor_posts[i][PACKETBLOBS/2],
				   k*65536/(PACKETBLOBS/2));
	for(k=PACKETBLOBS/2+1;k<PACKETBLOBS-1;k++)
	  floor_posts[i][k]=
	    floor1_interpolate_fit(vb,b->flr[info->floorsubmap[submap]],
				   floor_posts[i][PACKETBLOBS/2],
				   floor_posts[i][PACKETBLOBS-1],
				   (k-PACKETBLOBS/2)*65536/(PACKETBLOBS/2));
      }
    }
  }
  vbi->ampmax=global_ampmax;

  /*
    the next phases are performed once for vbr-only and PACKETBLOB
    times for bitrate managed modes.
    
    1) encode actual mode being used
    2) encode the floor for each channel, compute coded mask curve/res
    3) normalize and couple.
    4) encode residue
    5) save packet bytes to the packetblob vector
    
  */

  /* iterate over the many masking curve fits we've created */

  {
    float **res_bundle=alloca(sizeof(*res_bundle)*vi->channels);
    float **couple_bundle=alloca(sizeof(*couple_bundle)*vi->channels);
    int *zerobundle=alloca(sizeof(*zerobundle)*vi->channels);
    int **sortindex=alloca(sizeof(*sortindex)*vi->channels);
    float **mag_memo;
    int **mag_sort;

    if(info->coupling_steps){
      mag_memo=_vp_quantize_couple_memo(vb,
					&ci->psy_g_param,
					psy_look,
					info,
					gmdct);    
      
      mag_sort=_vp_quantize_couple_sort(vb,
					psy_look,
					info,
#ifdef __SSE__												/* SSE Optimize */
					mag_memo,
					res_org[0]);    
#else														/* SSE Optimize */
					mag_memo);    
#endif														/* SSE Optimize */
    }

    memset(sortindex,0,sizeof(*sortindex)*vi->channels);
    if(psy_look->vi->normal_channel_p){
      for(i=0;i<vi->channels;i++){
	float *mdct    =gmdct[i];
	sortindex[i]=alloca(sizeof(**sortindex)*n/2);
#ifdef __SSE__												/* SSE Optimize */
	_vp_noise_normalize_sort(psy_look,mdct,sortindex[i],res_org[0]);
#else														/* SSE Optimize */
	_vp_noise_normalize_sort(psy_look,mdct,sortindex[i]);
#endif														/* SSE Optimize */
      }
    }

    for(k=(vorbis_bitrate_managed(vb)?0:PACKETBLOBS/2);
	k<=(vorbis_bitrate_managed(vb)?PACKETBLOBS-1:PACKETBLOBS/2);
	k++){
      oggpack_buffer *opb=vbi->packetblob[k];

      /* start out our new packet blob with packet type and mode */
      /* Encode the packet type */
      oggpack_write(opb,0,1);
      /* Encode the modenumber */
      /* Encode frame mode, pre,post windowsize, then dispatch */
      oggpack_write(opb,modenumber,b->modebits);
      if(vb->W){
	oggpack_write(opb,vb->lW,1);
	oggpack_write(opb,vb->nW,1);
      }

      /* encode floor, compute masking curve, sep out residue */
      for(i=0;i<vi->channels;i++){
	int submap=info->chmuxlist[i];
	float *mdct    =gmdct[i];
	float *mdct_org=gmdct_org[i];
	float *res     =vb->pcm[i];
	float *resorgch=res_org[i];
	int   *ilogmask=ilogmaskch[i]=
	  _vorbis_block_alloc(vb,n/2*sizeof(**gmdct));
      
	nonzero[i]=floor1_encode(opb,vb,b->flr[info->floorsubmap[submap]],
				 floor_posts[i][k],
				 ilogmask);
#if 0
	{
	  char buf[80];
	  sprintf(buf,"maskI%c%d",i?'R':'L',k);
	  float work[n/2];
	  for(j=0;j<n/2;j++)
	    work[j]=FLOOR1_fromdB_LOOKUP[ilogmask[j]];
	  _analysis_output(buf,seq,work,n/2,1,1,0);
	}
#endif
	_vp_remove_floor(psy_look,
			 mdct,
			 ilogmask,
			 res,
			 ci->psy_g_param.sliding_lowpass[vb->W][k]);

	/* stereo threshold */
	_vp_remove_floor(psy_look,
			 mdct_org,
			 ilogmask,
			 resorgch,
			 ci->psy_g_param.sliding_lowpass[vb->W][k]);
		 

	_vp_noise_normalize(psy_look,res,res+n/2,sortindex[i]);

	
#if 0
	{
	  char buf[80];
	  float work[n/2];
	  for(j=0;j<n/2;j++)
	    work[j]=FLOOR1_fromdB_LOOKUP[ilogmask[j]]*(res+n/2)[j];
	  sprintf(buf,"resI%c%d",i?'R':'L',k);
	  _analysis_output(buf,seq,work,n/2,1,1,0);

	}
#endif
      }
      
      /* our iteration is now based on masking curve, not prequant and
	 coupling.  Only one prequant/coupling step */
      
      /* quantize/couple */
      /* incomplete implementation that assumes the tree is all depth
         one, or no tree at all */
      if(info->coupling_steps){
	_vp_couple(k,
		   &ci->psy_g_param,
		   psy_look,
		   info,
		   vb->pcm,
		   mag_memo,
		   mag_sort,
		   ilogmaskch,
		   nonzero,
		   ci->psy_g_param.sliding_lowpass[vb->W][k],
		   gmdct, res_org);
      }
      
      /* classify and encode by submap */
      for(i=0;i<info->submaps;i++){
	int ch_in_bundle=0;
	long **classifications;
	int resnum=info->residuesubmap[i];

	for(j=0;j<vi->channels;j++){
	  if(info->chmuxlist[j]==i){
	    zerobundle[ch_in_bundle]=0;
	    if(nonzero[j])zerobundle[ch_in_bundle]=1;
	    res_bundle[ch_in_bundle]=vb->pcm[j];
	    couple_bundle[ch_in_bundle++]=vb->pcm[j]+n/2;
	  }
	}
	
	classifications=_residue_P[ci->residue_type[resnum]]->
	  class(vb,b->residue[resnum],couple_bundle,zerobundle,ch_in_bundle);
	
	_residue_P[ci->residue_type[resnum]]->
	  forward(opb,vb,b->residue[resnum],
		  couple_bundle,NULL,zerobundle,ch_in_bundle,classifications);
      }
      
      /* set last-window type & number */
      if((blocktype == b->lW_blocktype) && (modenumber == b->lW_modenumber)) b->lW_no++;
      else b->lW_no = 1;
      b->lW_blocktype = blocktype;
      b->lW_modenumber = modenumber;
      /* ok, done encoding.  Next protopacket. */
    }
    
  }

#if 0
  seq++;
  total+=ci->blocksizes[vb->W]/4+ci->blocksizes[vb->nW]/4;
#endif
#endif	/* for _OPENMP */
  return(0);
}

static int mapping0_inverse(vorbis_block *vb,vorbis_info_mapping *l){
  vorbis_dsp_state     *vd=vb->vd;
  vorbis_info          *vi=vd->vi;
  codec_setup_info     *ci=vi->codec_setup;
  private_state        *b=vd->backend_state;
  vorbis_info_mapping0 *info=(vorbis_info_mapping0 *)l;
  int hs=ci->halfrate_flag; 

  int                   i,j;
  long                  n=vb->pcmend=ci->blocksizes[vb->W];

  float **pcmbundle=alloca(sizeof(*pcmbundle)*vi->channels);
  int    *zerobundle=alloca(sizeof(*zerobundle)*vi->channels);

  int   *nonzero  =alloca(sizeof(*nonzero)*vi->channels);
  void **floormemo=alloca(sizeof(*floormemo)*vi->channels);
  
  /* recover the spectral envelope; store it in the PCM vector for now */
  for(i=0;i<vi->channels;i++){
    int submap=info->chmuxlist[i];
    floormemo[i]=_floor_P[ci->floor_type[info->floorsubmap[submap]]]->
      inverse1(vb,b->flr[info->floorsubmap[submap]]);
    if(floormemo[i])
      nonzero[i]=1;
    else
      nonzero[i]=0;      
    memset(vb->pcm[i],0,sizeof(*vb->pcm[i])*n/2);
  }

  /* channel coupling can 'dirty' the nonzero listing */
  for(i=0;i<info->coupling_steps;i++){
    if(nonzero[info->coupling_mag[i]] ||
       nonzero[info->coupling_ang[i]]){
      nonzero[info->coupling_mag[i]]=1; 
      nonzero[info->coupling_ang[i]]=1; 
    }
  }

  /* recover the residue into our working vectors */
  for(i=0;i<info->submaps;i++){
    int ch_in_bundle=0;
    for(j=0;j<vi->channels;j++){
      if(info->chmuxlist[j]==i){
	if(nonzero[j])
	  zerobundle[ch_in_bundle]=1;
	else
	  zerobundle[ch_in_bundle]=0;
	pcmbundle[ch_in_bundle++]=vb->pcm[j];
      }
    }

    _residue_P[ci->residue_type[info->residuesubmap[i]]]->
      inverse(vb,b->residue[info->residuesubmap[i]],
	      pcmbundle,zerobundle,ch_in_bundle);
  }

  /* channel coupling */
  for(i=info->coupling_steps-1;i>=0;i--){
#ifdef	__SSE__												/* SSE Optimize */
	{
		float	*PCMM	 = vb->pcm[info->coupling_mag[i]];
		float	*PCMA	 = vb->pcm[info->coupling_ang[i]];
		int	Lim	 = (n/2)&(~7);
		for(j=0;j<Lim;j+=8){
			__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5;
			XMM0	 = _mm_load_ps(PCMA+j  );
			XMM3	 = _mm_load_ps(PCMA+j+4);
			XMM1	 = _mm_load_ps(PCMM+j  );
			XMM4	 = _mm_load_ps(PCMM+j+4);
			XMM2	 = XMM0;
			XMM5	 = XMM3;
			XMM0	 = _mm_cmpnle_ps(XMM0, PM128(PFV_0));
			XMM3	 = _mm_cmpnle_ps(XMM3, PM128(PFV_0));
			XMM1	 = _mm_xor_ps(XMM1, XMM2);
			XMM4	 = _mm_xor_ps(XMM4, XMM5);
			XMM1	 = _mm_andnot_ps(XMM1, PM128(PCS_RRRR));
			XMM4	 = _mm_andnot_ps(XMM4, PM128(PCS_RRRR));
			XMM1	 = _mm_xor_ps(XMM1, XMM2);
			XMM4	 = _mm_xor_ps(XMM4, XMM5);
			XMM2	 = XMM1;
			XMM5	 = XMM4;
			XMM1	 = _mm_and_ps(XMM1, XMM0);
			XMM4	 = _mm_and_ps(XMM4, XMM3);
			XMM0	 = _mm_andnot_ps(XMM0, XMM2);
			XMM3	 = _mm_andnot_ps(XMM3, XMM5);
			XMM2	 = _mm_load_ps(PCMM+j  );
			XMM5	 = _mm_load_ps(PCMM+j+4);
			XMM1	 = _mm_add_ps(XMM1, XMM2);
			XMM4	 = _mm_add_ps(XMM4, XMM5);
			XMM0	 = _mm_add_ps(XMM0, XMM2);
			XMM3	 = _mm_add_ps(XMM3, XMM5);
			_mm_store_ps(PCMA+j  , XMM1);
			_mm_store_ps(PCMA+j+4, XMM4);
			_mm_store_ps(PCMM+j  , XMM0);
			_mm_store_ps(PCMM+j+4, XMM3);
		}
		Lim	 = (n/2)&(~3);
		for(;j<Lim;j+=4){
			__m128	XMM0, XMM1, XMM2;
			XMM0	 = _mm_load_ps(PCMA+j  );
			XMM1	 = _mm_load_ps(PCMM+j  );
			XMM2	 = XMM0;
			XMM0	 = _mm_cmpnle_ps(XMM0, PM128(PFV_0));
			XMM1	 = _mm_xor_ps(XMM1, XMM2);
			XMM1	 = _mm_andnot_ps(XMM1, PM128(PCS_RRRR));
			XMM1	 = _mm_xor_ps(XMM1, XMM2);
			XMM2	 = XMM1;
			XMM1	 = _mm_and_ps(XMM1, XMM0);
			XMM0	 = _mm_andnot_ps(XMM0, XMM2);
			XMM2	 = _mm_load_ps(PCMM+j  );
			XMM1	 = _mm_add_ps(XMM1, XMM2);
			XMM0	 = _mm_add_ps(XMM0, XMM2);
			_mm_store_ps(PCMA+j  , XMM1);
			_mm_store_ps(PCMM+j  , XMM0);
		}
		for(;j<n/2;j++){
			float mag=PCMM[j];
			float ang=PCMA[j];

			if(ang>0){
				PCMM[j]=mag;
				PCMA[j]=mag > 0 ? mag-ang : mag+ang;
			}else{
				PCMM[j]=mag > 0 ? mag+ang : mag-ang;
				PCMA[j]=mag;
			}
		}
	}
#else														/* SSE Optimize */
    float *pcmM=vb->pcm[info->coupling_mag[i]];
    float *pcmA=vb->pcm[info->coupling_ang[i]];

    for(j=0;j<n/2;j++){
      float mag=pcmM[j];
      float ang=pcmA[j];

      if(mag>0)
	if(ang>0){
	  pcmM[j]=mag;
	  pcmA[j]=mag-ang;
	}else{
	  pcmA[j]=mag;
	  pcmM[j]=mag+ang;
	}
      else
	if(ang>0){
	  pcmM[j]=mag;
	  pcmA[j]=mag+ang;
	}else{
	  pcmA[j]=mag;
	  pcmM[j]=mag-ang;
	}
    }
#endif														/* SSE Optimize */
  }

  /* compute and apply spectral envelope */
  for(i=0;i<vi->channels;i++){
    float *pcm=vb->pcm[i];
    int submap=info->chmuxlist[i];
    _floor_P[ci->floor_type[info->floorsubmap[submap]]]->
      inverse2(vb,b->flr[info->floorsubmap[submap]],
	       floormemo[i],pcm);
  }

  /* transform the PCM data; takes PCM vector, vb; modifies PCM vector */
  /* only MDCT right now.... */
  for(i=0;i<vi->channels;i++){
    float *pcm=vb->pcm[i];
    mdct_backward(b->transform[vb->W][0],pcm,pcm);
  }

  /* all done! */
  return(0);
}

/* export hooks */
vorbis_func_mapping mapping0_exportbundle={
#ifdef DECODE_ONLY
	NULL,
#else
  &mapping0_pack,
#endif
  &mapping0_unpack,
  &mapping0_free_info,
#ifdef DECODE_ONLY
	NULL,
#else
  &mapping0_forward,
#endif
  &mapping0_inverse
};

