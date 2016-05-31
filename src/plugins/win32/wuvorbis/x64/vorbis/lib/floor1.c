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

 function: floor backend 1 implementation
 last mod: $Id: floor1.c 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ogg/ogg.h>
#include "vorbis/codec.h"
#include "codec_internal.h"
#include "registry.h"
#include "codebook.h"
#include "misc.h"
#include "scales.h"
#ifdef __SSE__												/* SSE Optimize */
#include "xmmlib.h"
#endif														/* SSE Optimize */
#if	defined(__INTEL_COMPILER)
#include <ia32intrin.h>
#endif

#include <stdio.h>

#define floor1_rangedB 140 /* floor 1 fixed at -140dB to 0dB range */

typedef struct {
  int sorted_index[VIF_POSIT+2];
  int forward_index[VIF_POSIT+2];
  int reverse_index[VIF_POSIT+2];
  
  int hineighbor[VIF_POSIT];
  int loneighbor[VIF_POSIT];
  int posts;

  int n;
  int quant_q;
  vorbis_info_floor1 *vi;

  long phrasebits;
  long postbits;
  long frames;
} vorbis_look_floor1;

typedef struct lsfit_acc{
  long x0;
  long x1;

  long xa;
  long ya;
  long x2a;
  long y2a;
  long xya; 
  long an;
} lsfit_acc;

/***********************************************/
 
static void floor1_free_info(vorbis_info_floor *i){
  vorbis_info_floor1 *info=(vorbis_info_floor1 *)i;
  if(info){
    memset(info,0,sizeof(*info));
    _ogg_free(info);
  }
}

static void floor1_free_look(vorbis_look_floor *i){
  vorbis_look_floor1 *look=(vorbis_look_floor1 *)i;
  if(look){
    /*fprintf(stderr,"floor 1 bit usage %f:%f (%f total)\n",
	    (float)look->phrasebits/look->frames,
	    (float)look->postbits/look->frames,
	    (float)(look->postbits+look->phrasebits)/look->frames);*/

    memset(look,0,sizeof(*look));
    _ogg_free(look);
  }
}

#if	defined(__INTEL_COMPILER)
static int ilog(unsigned int v){
	return(_bit_scan_reverse(v) + 1);
}

static int ilog2(unsigned int v){
	if(v)--v;
	return(_bit_scan_reverse(v) + 1);
}
#else
static int ilog(unsigned int v){
  int ret=0;
  while(v){
    ret++;
    v>>=1;
  }
  return(ret);
}

static int ilog2(unsigned int v){
  int ret=0;
  if(v)--v;
  while(v){
    ret++;
    v>>=1;
  }
  return(ret);
}
#endif

static void floor1_pack (vorbis_info_floor *i,oggpack_buffer *opb){
  vorbis_info_floor1 *info=(vorbis_info_floor1 *)i;
  int j,k;
  int count=0;
  int rangebits;
  int maxposit=info->postlist[1];
  int maxclass=-1;

  /* save out partitions */
  oggpack_write(opb,info->partitions,5); /* only 0 to 31 legal */
  for(j=0;j<info->partitions;j++){
    oggpack_write(opb,info->partitionclass[j],4); /* only 0 to 15 legal */
    if(maxclass<info->partitionclass[j])maxclass=info->partitionclass[j];
  }

  /* save out partition classes */
  for(j=0;j<maxclass+1;j++){
    oggpack_write(opb,info->class_dim[j]-1,3); /* 1 to 8 */
    oggpack_write(opb,info->class_subs[j],2); /* 0 to 3 */
    if(info->class_subs[j])oggpack_write(opb,info->class_book[j],8);
    for(k=0;k<(1<<info->class_subs[j]);k++)
      oggpack_write(opb,info->class_subbook[j][k]+1,8);
  }

  /* save out the post list */
  oggpack_write(opb,info->mult-1,2);     /* only 1,2,3,4 legal now */ 
  oggpack_write(opb,ilog2(maxposit),4);
  rangebits=ilog2(maxposit);

  for(j=0,k=0;j<info->partitions;j++){
    count+=info->class_dim[info->partitionclass[j]]; 
    for(;k<count;k++)
      oggpack_write(opb,info->postlist[k+2],rangebits);
  }
}


static vorbis_info_floor *floor1_unpack (vorbis_info *vi,oggpack_buffer *opb){
  codec_setup_info     *ci=vi->codec_setup;
  int j,k,count=0,maxclass=-1,rangebits;

  vorbis_info_floor1 *info=_ogg_calloc(1,sizeof(*info));
  /* read partitions */
  info->partitions=oggpack_read(opb,5); /* only 0 to 31 legal */
  for(j=0;j<info->partitions;j++){
    info->partitionclass[j]=oggpack_read(opb,4); /* only 0 to 15 legal */
    if(maxclass<info->partitionclass[j])maxclass=info->partitionclass[j];
  }

  /* read partition classes */
  for(j=0;j<maxclass+1;j++){
    info->class_dim[j]=oggpack_read(opb,3)+1; /* 1 to 8 */
    info->class_subs[j]=oggpack_read(opb,2); /* 0,1,2,3 bits */
    if(info->class_subs[j]<0)
      goto err_out;
    if(info->class_subs[j])info->class_book[j]=oggpack_read(opb,8);
    if(info->class_book[j]<0 || info->class_book[j]>=ci->books)
      goto err_out;
    for(k=0;k<(1<<info->class_subs[j]);k++){
      info->class_subbook[j][k]=oggpack_read(opb,8)-1;
      if(info->class_subbook[j][k]<-1 || info->class_subbook[j][k]>=ci->books)
	goto err_out;
    }
  }

  /* read the post list */
  info->mult=oggpack_read(opb,2)+1;     /* only 1,2,3,4 legal now */ 
  rangebits=oggpack_read(opb,4);

  for(j=0,k=0;j<info->partitions;j++){
    count+=info->class_dim[info->partitionclass[j]]; 
    for(;k<count;k++){
      int t=info->postlist[k+2]=oggpack_read(opb,rangebits);
      if(t<0 || t>=(1<<rangebits))
	goto err_out;
    }
  }
  info->postlist[0]=0;
  info->postlist[1]=1<<rangebits;

  return(info);
  
 err_out:
  floor1_free_info(info);
  return(NULL);
}

static int icomp(const void *a,const void *b){
  return(**(int **)a-**(int **)b);
}

static vorbis_look_floor *floor1_look(vorbis_dsp_state *vd,
				      vorbis_info_floor *in){

  int *sortpointer[VIF_POSIT+2];
  vorbis_info_floor1 *info=(vorbis_info_floor1 *)in;
  vorbis_look_floor1 *look=_ogg_calloc(1,sizeof(*look));
  int i,j,n=0;

  look->vi=info;
  look->n=info->postlist[1];
 
  /* we drop each position value in-between already decoded values,
     and use linear interpolation to predict each new value past the
     edges.  The positions are read in the order of the position
     list... we precompute the bounding positions in the lookup.  Of
     course, the neighbors can change (if a position is declined), but
     this is an initial mapping */

  for(i=0;i<info->partitions;i++)n+=info->class_dim[info->partitionclass[i]];
  n+=2;
  look->posts=n;

  /* also store a sorted position index */
  for(i=0;i<n;i++)sortpointer[i]=info->postlist+i;
  qsort(sortpointer,n,sizeof(*sortpointer),icomp);

  /* points from sort order back to range number */
  for(i=0;i<n;i++)look->forward_index[i]=sortpointer[i]-info->postlist;
  /* points from range order to sorted position */
  for(i=0;i<n;i++)look->reverse_index[look->forward_index[i]]=i;
  /* we actually need the post values too */
  for(i=0;i<n;i++)look->sorted_index[i]=info->postlist[look->forward_index[i]];
  
  /* quantize values to multiplier spec */
  switch(info->mult){
  case 1: /* 1024 -> 256 */
    look->quant_q=256;
    break;
  case 2: /* 1024 -> 128 */
    look->quant_q=128;
    break;
  case 3: /* 1024 -> 86 */
    look->quant_q=86;
    break;
  case 4: /* 1024 -> 64 */
    look->quant_q=64;
    break;
  }

  /* discover our neighbors for decode where we don't use fit flags
     (that would push the neighbors outward) */
  for(i=0;i<n-2;i++){
    int lo=0;
    int hi=1;
    int lx=0;
    int hx=look->n;
    int currentx=info->postlist[i+2];
    for(j=0;j<i+2;j++){
      int x=info->postlist[j];
      if(x>lx && x<currentx){
	lo=j;
	lx=x;
      }
      if(x<hx && x>currentx){
	hi=j;
	hx=x;
      }
    }
    look->loneighbor[i]=lo;
    look->hineighbor[i]=hi;
  }

  return(look);
}

static int render_point(int x0,int x1,int y0,int y1,int x){
  y0&=0x7fff; /* mask off flag */
  y1&=0x7fff;
    
  {
    int dy=y1-y0;
    int adx=x1-x0;
    int ady=abs(dy);
    int err=ady*(x-x0);
    
    int off=err/adx;
    if(dy<0)return(y0-off);
    return(y0+off);
  }
}
#if	defined(__SSE__)										/* SSE Optimize */
static _MM_ALIGN16 const float pfv0[4]	 = 
	{7.3142857f, 7.3142857f, 7.3142857f, 7.3142857f};
static _MM_ALIGN16 const float pfv1[4]	 = 
	{1023.5f, 1023.5f, 1023.5f, 1023.5f};
static _MM_ALIGN16 const float pfv2[4]	 = 
	{1023.f, 1023.f, 1023.f, 1023.f};
#endif														/* SSE Optimize */

static int vorbis_dBquant(const float *x){
#if	defined(__SSE__)										/* SSE Optimize */
	__m128	XMM0	 = _mm_load_ss(x);
	XMM0	 = _mm_mul_ss(XMM0, PM128(pfv0));
	XMM0	 = _mm_add_ss(XMM0, PM128(pfv1));
	XMM0	 = _mm_max_ss(XMM0, PM128(PFV_0));
	XMM0	 = _mm_min_ss(XMM0, PM128(pfv2));
	return	_mm_cvttss_si32(XMM0);
#else														/* SSE Optimize */
  int i= *x*7.3142857f+1023.5f;
  if(i>1023)return(1023);
  if(i<0)return(0);
  return i;
#endif														/* SSE Optimize */
}

#if 0
#if	defined(__SSE__)										/* SSE Optimize */
static __m128 vorbis_dBquant_ps(float *x)
{
#if	defined(__SSE2__)
	register __m128	pi;
	pi	 = _mm_load_ps(x);
	pi	 = _mm_mul_ps(pi, PM128(pfv0));
	pi	 = _mm_add_ps(pi, PM128(pfv1));
	pi	 = _mm_max_ps(pi, PM128(PFV_0));
	pi	 = _mm_min_ps(pi, PM128(pfv2));
	pi	 = _mm_cvtepi32_ps(_mm_cvttps_epi32(pi));
	return	pi;
#else
	register __m128	pi	 = PM128(x);
	register __m64	MM0, MM1;
	pi	 = _mm_mul_ps(pi, PM128(pfv0));
	pi	 = _mm_add_ps(pi, PM128(pfv1));
	pi	 = _mm_max_ps(pi, PM128(PFV_0));
	pi	 = _mm_min_ps(pi, PM128(pfv2));
	MM0	 = _mm_cvttps_pi32(pi);
	pi	 = _mm_movehl_ps(pi, pi);
	MM1	 = _mm_cvttps_pi32(pi);
	pi	 = _mm_cvtpi32_ps(pi, MM1);
	pi	 = _mm_movelh_ps(pi, pi);
	pi	 = _mm_cvtpi32_ps(pi, MM0);
	return	pi;
#endif
}
#endif														/* SSE Optimize */
#endif

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

void scale_FLOOR1_fromdB_LOOKUP(float fac)
{
	int i;
	for(i = 0; i < sizeof(FLOOR1_fromdB_LOOKUP)/sizeof(FLOOR1_fromdB_LOOKUP[0]); i++)
			FLOOR1_fromdB_LOOKUP[i] *= fac;
}

static void render_line(int x, int x2,int y, int y2, float *d)
{
	int shortLen = y2-y;
	int longLen = x2-x;
	int decInc;
	int j;
	if(shortLen==0)
	{
#if defined(__SSE__)										/* SSE Optimize */
		__m128	XMM0	 = _mm_set1_ps(FLOOR1_fromdB_LOOKUP[y]);
		decInc	 = (longLen&(~7));
		j		 = (longLen&(~3));
		for(;x<decInc;x+=8)
		{
			__m128	XMM1	 = _mm_lddqu_ps(d+x  );
			__m128	XMM2	 = _mm_lddqu_ps(d+x+4);
			XMM1	 = _mm_mul_ps(XMM1, XMM0);
			XMM2	 = _mm_mul_ps(XMM2, XMM0);
			_mm_storeu_ps(d+x  , XMM1);
			_mm_storeu_ps(d+x+4, XMM2);
		}
		for(;x<j;x+=4)
		{
			__m128	XMM1	 = _mm_lddqu_ps(d+x  );
			XMM1	 = _mm_mul_ps(XMM1, XMM0);
			_mm_storeu_ps(d+x  , XMM1);
		}
#endif														/* SSE Optimize */
		for(;x<x2;x++)
			d[x] *= FLOOR1_fromdB_LOOKUP[y];
	}
	else
	{
		decInc = (shortLen << 21) / longLen;
		if(shortLen>=0)
			j = 0x200   + (y<<21);
		else
			j = 0x1FF800 + (y<<21);
	
		for (;x<x2;x++)
		{
			d[x] *= FLOOR1_fromdB_LOOKUP[j >> 21];
			j += decInc;
		}
	}
	return;
}

static void render_line0(int x, int x2,int y, int y2, int *d)
{
	int shortLen = y2-y;
	int longLen = x2-x;
	int decInc = (shortLen << 21) / longLen;
	int j;
	if(shortLen>=0)
		j = 0x200   + (y<<21);
	else
		j = 0x1FF800 + (y<<21);

#if defined(__SSE2__)
	if(longLen>=4)
	{
		__m128i PJ0 = _mm_set_epi32(
			j+decInc*3, j+decInc*2, j+decInc  , j
		);
		__m128i PJ1 = _mm_set_epi32(
			j+decInc*7, j+decInc*6, j+decInc*5, j+decInc*4
		);
		__m128i	PDECINC = _mm_set1_epi32(decInc*8);
		int x1 = x+(longLen&(~7));
		for(;x<x1;x+=8)
		{
			__m128i XMM0 = PJ0;
			__m128i XMM1 = PJ1;
			XMM0 = _mm_srai_epi32(XMM0, 21);
			XMM1 = _mm_srai_epi32(XMM1, 21);
			_mm_storeu_si128((__m128i*)(d+x  ), XMM0);
			_mm_storeu_si128((__m128i*)(d+x+4), XMM1);
			PJ0 = _mm_add_epi32(PJ0, PDECINC);
			PJ1 = _mm_add_epi32(PJ1, PDECINC);
		}
		if(x2-x>=4)
		{
			__m128i XMM0 = PJ0;
			XMM0 = _mm_srai_epi32(XMM0, 21);
			_mm_storeu_si128((__m128i*)(d+x  ), XMM0);
			PJ0 = PJ1;
			x += 4;
		}
		j = _mm_cvtsi128_si32(PJ0);
	}
#elif defined(__SSE__)
	if(longLen>=4)
	{
		__m64 PJ0 = _mm_set_pi32(j+decInc  , j         );
		__m64 PJ1 = _mm_set_pi32(j+decInc*3, j+decInc*2);
		__m64 PDECINC = _mm_set1_pi32(decInc*4);
		int x1 = x+(longLen&(~3));
		for(;x<x1;x+=4)
		{
			__m64 MM0 = PJ0;
			__m64 MM1 = PJ1;
			MM0 = _mm_srai_pi32(MM0, 21);
			MM1 = _mm_srai_pi32(MM1, 21);
			PM64(d+x  ) = MM0;
			PM64(d+x+2) = MM1;
			PJ0 = _mm_add_pi32(PJ0, PDECINC);
			PJ1 = _mm_add_pi32(PJ1, PDECINC);
		}
		j = _mm_cvtsi64_si32(PJ0);
	}
#endif
	for (;x<x2;x++)
	{
		d[x] = j >> 21;
		j += decInc;
	}
	return;
}
#ifdef DECODE_ONLY
_MM_ALIGN16 float findex[2048];
_MM_ALIGN16 float findex2[2048];
#endif
/* the floor has already been filtered to only include relevant sections */
static int accumulate_fit(const float *flr,const float *mdct,
			  int x0, int x1,lsfit_acc *a,
#if	defined(__SSE__)										/* SSE Optimize */
			  int n,vorbis_info_floor1 *info, const float *tflr,
			  const float *tmask, const int *tcres){
#else														/* SSE Optimize */
			  int n,vorbis_info_floor1 *info){
#endif														/* SSE Optimize */
  long i;
#ifdef __SSE__												/* SSE Optimize */
  long xa=0,ya=0,x2a=0,y2a=0,xya=0,na=0, xb=0,yb=0,x2b=0,y2b=0,xyb=0,nb=0;
	int	x05;
	int j;

	extern float findex[2048];
	extern float findex2[2048];
#else														/* SSE Optimize */
  int quantized=vorbis_dBquant(flr+x0);

  long xa=0,ya=0,x2a=0,y2a=0,xya=0,na=0, xb=0,yb=0,x2b=0,y2b=0,xyb=0,nb=0;
#endif														/* SSE Optimize */

  memset(a,0,sizeof(*a));
  a->x0=x0;
  a->x1=x1;
  if(x1>=n)x1=n-1;

#ifdef __SSE__												/* SSE Optimize */
	x1	++;
	{
		static const int parm0[16] = {
			0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 4, 4, 5, 5, 6, 6
		};
		static const int parm3[16] = {
			6, 6, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1, 0, 0
		};
		__m128	PYA;
		__m128	PY2A;
		__m128	PX2A;
		__m128	PXYA;
		__m128	PYB;
		__m128	PY2B;
		__m128	PX2B;
		__m128	PXYB;
		x05	 = (x0+3)&(~3);
		x05	 = (x05>x1)?x1:x05;
		if(x1-x05<4)
		{
			for(i=x0;i<x1;i++)
			{
				int quantized	 = (int)tflr[i];
				if(quantized)
				{
					if(mdct[i]+info->twofitatten>=flr[i])
					{
						xa	+= i;
						ya	+= quantized;
						x2a	+= i*i;
						y2a	+= quantized*quantized;
						xya	+= i*quantized;
						na	++;
					}
					else
					{
						xb	+= i;
						yb	+= quantized;
						x2b	+= i*i;
						y2b	+= quantized*quantized;
						xyb	+= i*quantized;
						nb	++;
					}
				}
			}
		}
		else
		{
			_mm_prefetch((const float*)(findex +x0)  , _MM_HINT_NTA);
			_mm_prefetch((const float*)(findex2+x0)  , _MM_HINT_NTA);
			PYA		 = _mm_setzero_ps();
			PY2A	 = _mm_setzero_ps();
			PX2A	 = _mm_setzero_ps();
			PXYA	 = _mm_setzero_ps();
			PYB		 = _mm_setzero_ps();
			PY2B	 = _mm_setzero_ps();
			PX2B	 = _mm_setzero_ps();
			PXYB	 = _mm_setzero_ps();
#if	1
			j = 16>>(x05-x0);
			for(i=x0;i<x05;i++)
			{
				__m128	XMM0,  XMM1, XMM2, XMM3;
				XMM0	 = _mm_load_ss(tflr+i);
				XMM3	 = _mm_load_ss(findex+i);
				XMM1	 = XMM0;
				XMM2	 = XMM0;
				XMM1	 = _mm_mul_ss(XMM1, XMM1);
				XMM2	 = _mm_mul_ss(XMM2, XMM3);
				if((tcres[x05-4]&j)!=0)	/*	Type-A 1 unit burst mode */
				{
					xa	+= i;
					PYA		 = _mm_add_ss(PYA,  XMM0);
					x2a += i*i;
					PY2A	 = _mm_add_ss(PY2A, XMM1);
					PXYA	 = _mm_add_ss(PXYA, XMM2);
					na	++;
				}
				else					/*	Type-B 1 unit burst mode */
				{
					xb	+= i;
					PYB		 = _mm_add_ss(PYB,  XMM0);
					x2b += i*i;
					PY2B	 = _mm_add_ss(PY2B, XMM1);
					PXYB	 = _mm_add_ss(PXYB, XMM2);
					nb	++;
				}
				j = j << 1;
			}
#else
			for(i=x0;i<x05;i++)
			{
				int quantized	 = (int)tflr[i];
				if(quantized)
				{
					if(mdct[i]+info->twofitatten>=flr[i])
					{
						xa	+= i;
						ya	+= quantized;
						x2a	+= i*i;
						y2a	+= quantized*quantized;
						xya	+= i*quantized;
						na	++;
					}
					else
					{
						xb	+= i;
						yb	+= quantized;
						x2b	+= i*i;
						y2b	+= quantized*quantized;
						xyb	+= i*quantized;
						nb	++;
					}
				}
			}
#endif
			x05	 = ((x1-i)&(~3))+i;
			for(;i<x05;i+=4)
			{
				__m128	XMM0,  XMM1, XMM2;
				XMM0	 = _mm_load_ps(tflr+i);
				_mm_prefetch((const float*)(findex +i+16)  , _MM_HINT_NTA);
				_mm_prefetch((const float*)(findex2+i+16)  , _MM_HINT_NTA);
				XMM1	 = XMM0;
				XMM2	 = XMM0;
				XMM1	 = _mm_mul_ps(XMM1, XMM1);
				XMM2	 = _mm_mul_ps(XMM2, PM128(findex+i));
				if(tcres[i]==0xF)	/*	Type-A 4 unit burst mode */
				{
					xa	+= (i*4+6);
					PYA		 = _mm_add_ps(PYA,  XMM0);
					x2a += 4*i*(i+3)+14;
					PY2A	 = _mm_add_ps(PY2A, XMM1);
					PXYA	 = _mm_add_ps(PXYA, XMM2);
					na	+= 4;
				}
				else if(tcres[i]==0x0)	/*	Type-B 4 unit burst mode */
				{
					xb	+= (i*4+6);
					PYB		 = _mm_add_ps(PYB,  XMM0);
					x2b += 4*i*(i+3)+14;
					PY2B	 = _mm_add_ps(PY2B, XMM1);
					PXYB	 = _mm_add_ps(PXYB, XMM2);
					nb	+= 4;
				}
				else
				{
					int p = bitCountTable[tcres[i]];
					int q = 4 - p;
					__m128	PMASKA	 = _mm_load_ps(tmask+i);
					__m128	PMASKB	 = _mm_xor_ps(PMASKA, PM128(PMASKTABLE+4*15));
					xa		+= i*p+parm0[tcres[i]];
					PYA		 = _mm_add_ps(PYA , _mm_and_ps(XMM0, PMASKA));
					PX2A	 = _mm_add_ps(PX2A, _mm_and_ps(PM128(findex2+i), PMASKA));
					PY2A	 = _mm_add_ps(PY2A, _mm_and_ps(XMM1, PMASKA));
					PXYA	 = _mm_add_ps(PXYA, _mm_and_ps(XMM2, PMASKA));
					na		+= p;
					xb		+= i*q+parm3[tcres[i]];
					PYB		 = _mm_add_ps(PYB , _mm_and_ps(XMM0, PMASKB));
					PX2B	 = _mm_add_ps(PX2B, _mm_and_ps(PM128(findex2+i), PMASKB));
					PY2B	 = _mm_add_ps(PY2B, _mm_and_ps(XMM1, PMASKB));
					PXYB	 = _mm_add_ps(PXYB, _mm_and_ps(XMM2, PMASKB));
					nb		+= q;
				}
			}
			{
				__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5;
				__m128x	TMA, TMB;
				XMM0	 = XMM1	 = PYA;
				XMM3	 = XMM2	 = PY2A;
				XMM0	 = _mm_shuffle_ps(XMM0, PX2A, _MM_SHUFFLE(1,0,1,0));
				XMM1	 = _mm_shuffle_ps(XMM1, PX2A, _MM_SHUFFLE(3,2,3,2));
				XMM2	 = _mm_shuffle_ps(XMM2, PXYA, _MM_SHUFFLE(1,0,1,0));
				XMM3	 = _mm_shuffle_ps(XMM3, PXYA, _MM_SHUFFLE(3,2,3,2));
				XMM4	 = XMM0;
				XMM5	 = XMM1;
				XMM0	 = _mm_shuffle_ps(XMM0, XMM2, _MM_SHUFFLE(2,0,2,0));
				XMM4	 = _mm_shuffle_ps(XMM4, XMM2, _MM_SHUFFLE(3,1,3,1));
				XMM1	 = _mm_shuffle_ps(XMM1, XMM3, _MM_SHUFFLE(2,0,2,0));
				XMM5	 = _mm_shuffle_ps(XMM5, XMM3, _MM_SHUFFLE(3,1,3,1));
				XMM0	 = _mm_add_ps(XMM0, XMM4);
				XMM1	 = _mm_add_ps(XMM1, XMM5);
				XMM0	 = _mm_add_ps(XMM0, XMM1);
	
				TMA.ps	 = XMM0;
	
				XMM0	 = XMM1	 = PYB;
				XMM3	 = XMM2	 = PY2B;
				XMM0	 = _mm_shuffle_ps(XMM0, PX2B, _MM_SHUFFLE(1,0,1,0));
				XMM1	 = _mm_shuffle_ps(XMM1, PX2B, _MM_SHUFFLE(3,2,3,2));
				XMM2	 = _mm_shuffle_ps(XMM2, PXYB, _MM_SHUFFLE(1,0,1,0));
				XMM3	 = _mm_shuffle_ps(XMM3, PXYB, _MM_SHUFFLE(3,2,3,2));
				XMM4	 = XMM0;
				XMM5	 = XMM1;
				XMM0	 = _mm_shuffle_ps(XMM0, XMM2, _MM_SHUFFLE(2,0,2,0));
				XMM4	 = _mm_shuffle_ps(XMM4, XMM2, _MM_SHUFFLE(3,1,3,1));
				XMM1	 = _mm_shuffle_ps(XMM1, XMM3, _MM_SHUFFLE(2,0,2,0));
				XMM5	 = _mm_shuffle_ps(XMM5, XMM3, _MM_SHUFFLE(3,1,3,1));
				XMM0	 = _mm_add_ps(XMM0, XMM4);
				XMM1	 = _mm_add_ps(XMM1, XMM5);
				XMM0	 = _mm_add_ps(XMM0, XMM1);
	
				TMB.ps	 = XMM0;
	
#if	defined(__SSE2__)
				TMA.pi	 = _mm_cvttps_epi32(TMA.ps);
				TMB.pi	 = _mm_cvttps_epi32(TMB.ps);
#else
				{
					__m64	MM0, MM1, MM2, MM3;
					MM0	 = _mm_cvttps_pi32(TMA.ps);
					MM2	 = _mm_cvttps_pi32(TMB.ps);
					TMA.ps	 = _mm_movehl_ps(TMA.ps, TMA.ps);
					TMB.ps	 = _mm_movehl_ps(TMB.ps, TMB.ps);
					MM1	 = _mm_cvttps_pi32(TMA.ps);
					MM3	 = _mm_cvttps_pi32(TMB.ps);
					TMA.pi64[0]	 = MM0;
					TMB.pi64[0]	 = MM2;
					TMA.pi64[1]	 = MM1;
					TMB.pi64[1]	 = MM3;
				}
				_mm_empty();
#endif
				
				ya	+= TMA.si32[0];
				x2a	+= TMA.si32[1];
				y2a	+= TMA.si32[2];
				xya	+= TMA.si32[3];
				yb	+= TMB.si32[0];
				x2b	+= TMB.si32[1];
				y2b	+= TMB.si32[2];
				xyb	+= TMB.si32[3];
			}
			for(;i<x1;i++)
			{
				int quantized	 = (int)tflr[i];
				if(quantized)
				{
					if(mdct[i]+info->twofitatten>=flr[i])
					{
						xa	+= i;
						ya	+= quantized;
						x2a	+= i*i;
						y2a	+= quantized*quantized;
						xya	+= i*quantized;
						na	++;
					}
					else
					{
						xb	+= i;
						yb	+= quantized;
						x2b	+= i*i;
						y2b	+= quantized*quantized;
						xyb	+= i*quantized;
						nb	++;
					}
				}
			}
		}
	}
#else														/* SSE Optimize */
  for(i=x0;i<=x1;i++){
    int quantized=vorbis_dBquant(flr+i);
    if(quantized){
      if(mdct[i]+info->twofitatten>=flr[i]){
	xa  += i;
	ya  += quantized;
	x2a += i*i;
	y2a += quantized*quantized;
	xya += i*quantized;
	na++;
      }else{
	xb  += i;
	yb  += quantized;
	x2b += i*i;
	y2b += quantized*quantized;
	xyb += i*quantized;
	nb++;
      }
    }
  }
#endif														/* SSE Optimize */

  xb+=xa;
  yb+=ya;
  x2b+=x2a;
  y2b+=y2a;
  xyb+=xya;
  nb+=na;

  /* weight toward the actually used frequencies if we meet the threshhold */
  {
    int weight=nb*info->twofitweight/(na+1);

    a->xa=xa*weight+xb;
    a->ya=ya*weight+yb;
    a->x2a=x2a*weight+x2b;
    a->y2a=y2a*weight+y2b;
    a->xya=xya*weight+xyb;
    a->an=na*weight+nb;
  }

  return(na);
}

static void fit_line(lsfit_acc *a,int fits,int *y0,int *y1){
#ifdef __SSE__												/* SSE Optimize */
	long	x, y, x2,y2 ,xy ,an ,i;
	long	x0	 = a[0].x0;
	long	x1	 = a[fits-1].x1;
//#if	defined(__SSE2__)&&!defined(__PROF__)
#if	defined(__SSE2__)
	__m128i	XMM0, XMM1, XMM2, XMM3;
	__m128x T;
	__m128i *PA = (__m128i*)a;

	XMM0 = XMM1 = XMM2 = XMM3 = _mm_setzero_si128();
	for(i=0;i<fits&(~1);i+=2)
	{
		__m128i XMM4 = _mm_load_si128(PA+i*2  );
		__m128i XMM5 = _mm_load_si128(PA+i*2+1);
		__m128i XMM6 = _mm_load_si128(PA+i*2+2);
		__m128i XMM7 = _mm_load_si128(PA+i*2+3);
		XMM0 = _mm_add_epi32(XMM0, XMM4);
		XMM1 = _mm_add_epi32(XMM1, XMM5);
		XMM2 = _mm_add_epi32(XMM2, XMM6);
		XMM3 = _mm_add_epi32(XMM3, XMM7);
	}
	for(;i<fits;i++)
	{
		__m128i XMM4 = _mm_load_si128(PA+i*2  );
		__m128i XMM5 = _mm_load_si128(PA+i*2+1);
		XMM0 = _mm_add_epi32(XMM0, XMM4);
		XMM1 = _mm_add_epi32(XMM1, XMM5);
	}
	XMM0 = _mm_add_epi32(XMM0, XMM2);
	XMM1 = _mm_add_epi32(XMM1, XMM3);
	T.pi = XMM0;
	x	 = T.si32[2];
	y	 = T.si32[3];
	T.pi = XMM1;
	x2	 = T.si32[0];
	y2	 = T.si32[1];
	xy	 = T.si32[2];
	an	 = T.si32[3];
#else
	__m64	XY, X2Y2, XYAN;
	__m64	*PA	 = (__m64*)a;
	XY		 = 
	X2Y2	 = 
	XYAN	 = _mm_setzero_si64();

	for(i=0;i<fits&(~1);i+=2)
	{
		__m64	MM0	 = *(PA+1);
		__m64	MM1	 = *(PA+2);
		__m64	MM2	 = *(PA+3);
		XY		 = _mm_add_pi32(XY,   MM0);
		X2Y2	 = _mm_add_pi32(X2Y2, MM1);
		XYAN	 = _mm_add_pi32(XYAN, MM2);
		MM0	 = *(PA+5);
		MM1	 = *(PA+6);
		MM2	 = *(PA+7);
		XY		 = _mm_add_pi32(XY,   MM0);
		X2Y2	 = _mm_add_pi32(X2Y2, MM1);
		XYAN	 = _mm_add_pi32(XYAN, MM2);
		PA	+= 8;
	}
	for(;i<fits;i++)
	{
		__m64	MM0	 = *(PA+1);
		__m64	MM1	 = *(PA+2);
		__m64	MM2	 = *(PA+3);
		XY		 = _mm_add_pi32(XY,   MM0);
		X2Y2	 = _mm_add_pi32(X2Y2, MM1);
		XYAN	 = _mm_add_pi32(XYAN, MM2);
		PA	+= 4;
	}
	{
		__m64x	M0X, M1X, M2X;
		M0X.pi64	 = XY;
		M1X.pi64	 = X2Y2;
		M2X.pi64	 = XYAN;
		x	 = M0X.ssi32[0];
		y	 = M0X.ssi32[1];
		x2	 = M1X.ssi32[0];
		y2	 = M1X.ssi32[1];
		xy	 = M2X.ssi32[0];
		an	 = M2X.ssi32[1];
	}
	_mm_empty();
#endif
#else														/* SSE Optimize */
  long x=0,y=0,x2=0,y2=0,xy=0,an=0,i;
  long x0=a[0].x0;
  long x1=a[fits-1].x1;

  for(i=0;i<fits;i++){
    x+=a[i].xa;
    y+=a[i].ya;
    x2+=a[i].x2a;
    y2+=a[i].y2a;
    xy+=a[i].xya;
    an+=a[i].an;
  }
#endif														/* SSE Optimize */

  if(*y0>=0){
    x+=   x0;
    y+=  *y0;
    x2+=  x0 *  x0;
    y2+= *y0 * *y0;
    xy+= *y0 *  x0;
    an++;
  }

  if(*y1>=0){
    x+=   x1;
    y+=  *y1;
    x2+=  x1 *  x1;
    y2+= *y1 * *y1;
    xy+= *y1 *  x1;
    an++;
  }
  
  if(an){
    /* need 64 bit multiplies, which C doesn't give portably as int */
    double fx=x;
    double fy=y;
    double fx2=x2;
    double fxy=xy;
    double denom=1./(an*fx2-fx*fx);
    double a=(fy*fx2-fxy*fx)*denom;
    double b=(an*fxy-fx*fy)*denom;
    *y0=rint(a+b*x0);
    *y1=rint(a+b*x1);
    
    /* limit to our range! */
    if(*y0>1023)*y0=1023;
    if(*y1>1023)*y1=1023;
    if(*y0<0)*y0=0;
    if(*y1<0)*y1=0;
    
  }else{
    *y0=0;
    *y1=0;
  }
}

/*static void fit_line_point(lsfit_acc *a,int fits,int *y0,int *y1){
  long y=0;
  int i;

  for(i=0;i<fits && y==0;i++)
    y+=a[i].ya;
  
  *y0=*y1=y;
  }*/

static int inspect_error(int x0,int x1,int y0,int y1,const float *mask,
			 const float *mdct,
#if	defined(__SSE__)										/* SSE Optimize */
			 vorbis_info_floor1 *info, const float *tflr,
			  const float *tmask, const int *tcres){
#else														/* SSE Optimize */
			 vorbis_info_floor1 *info){
#endif														/* SSE Optimize */
#if	defined(__SSE__)										/* SSE Optimize */
	int x = x0;
	int y = y0;
	int val = vorbis_dBquant(mask+x);
	int mse = 0;
	int n = 0;
	int shortLen = y1-y;
	int longLen = x1-x;
	int decInc = (shortLen << 21) / longLen;
	int j;

	if(shortLen>=0)
		j = 0x200   + (y<<21);
	else
		j = 0x1FF800 + (y<<21);
	{
		int	x05;
		x05	 = (x+3)&(~3);
		x05	 = (x05>x1)?x1:x05;
		for(;x<x05;x++)
		{
			y = j >> 21;
			val	 = tflr[x];
			mse	+= ((y-val)*(y-val));
			n++;
			if(mdct[x]+info->twofitatten>=mask[x])
			{
				if(y+info->maxover<val)return(1);
				if(y-info->maxunder>val)return(1);
			}
			j += decInc;
		}
	}
	{
		register __m128	PMSE;
		__m128	PIMOVER		 = _mm_set1_ps(info->maxover);
		__m128	PIMUNDER	 = _mm_set1_ps(info->maxunder);
#if defined(__SSE2__)
		__m128i PJ0 = _mm_set_epi32(
			j+decInc*3, j+decInc*2, j+decInc  , j
		);
		__m128i	PDECINC = _mm_set1_epi32(decInc*4);
#else
		__m64 PJ0 = _mm_set_pi32(j+decInc  , j         );
		__m64 PJ1 = _mm_set_pi32(j+decInc*3, j+decInc*2);
		__m64 PDECINC = _mm_set1_pi32(decInc*4);
#endif
		int	x05	 = x1&(~3);

		x05	 = (x05>x1)?x1:x05;
		PMSE	 = _mm_setzero_ps();
		for(;x<x05;x+=4)
		{
			__m128	PY;
			register __m128	PVAL, PDMSE;
#if defined(__SSE2__)
			{
				__m128i XMM0 = PJ0;
				XMM0 = _mm_srai_epi32(XMM0, 21);
				PY = _mm_cvtepi32_ps(XMM0);
				PJ0 = _mm_add_epi32(PJ0, PDECINC);
			}
#else
			{
				__m64 MM1 = PJ1;
				__m64 MM0 = PJ0;
				MM1 = _mm_srai_pi32(MM1, 21);
				MM0 = _mm_srai_pi32(MM0, 21);
#pragma warning(disable : 592)
				PY  = _mm_cvtpi32_ps(PY, MM1);
#pragma warning(default : 592)
				PJ1 = _mm_add_pi32(PJ1, PDECINC);
				PY  = _mm_movelh_ps(PY, PY);
				PJ0 = _mm_add_pi32(PJ0, PDECINC);
				PY  = _mm_cvtpi32_ps(PY, MM0);
			}
#endif

			PVAL	 = _mm_load_ps(tflr+x);
			PDMSE	 = PY;
			PDMSE	 = _mm_sub_ps(PDMSE, PVAL);
			PDMSE	 = _mm_mul_ps(PDMSE, PDMSE);
			PMSE	 = _mm_add_ps(PMSE, PDMSE);
			n	+= 4;
			if(tcres[x]){
				register __m128	PMASK1, PMASK2;
				PMASK1	 = PY;
				PMASK2	 = PY;
				PMASK1	 = _mm_add_ps(PMASK1, PIMOVER);
				PMASK2	 = _mm_sub_ps(PMASK2, PIMUNDER);
				PMASK1	 = _mm_cmplt_ps(PMASK1, PVAL);
				PMASK2	 = _mm_cmpgt_ps(PMASK2, PVAL);
				PMASK1	 = _mm_or_ps(PMASK1, PMASK2);
				if(_mm_movemask_ps(PMASK1)&tcres[x])
				{
#if	!defined(__SSE2__)
					_mm_empty();
#endif
					return(1);
				}
			}
		}
#if	defined(__SSE2__)
		j = _mm_cvtsi128_si32(PJ0);
#else
		j = _mm_cvtsi64_si32(PJ0);
		_mm_empty();
#endif
		mse	+= (int)_mm_add_horz(PMSE);
	}
	{
		for(;x<x1;x++)
		{
			y = j >> 21;
			val	 = tflr[x];
			mse	+= ((y-val)*(y-val));
			n++;
			if(mdct[x]+info->twofitatten>=mask[x])
			{
				if(y+info->maxover<val)return(1);
				if(y-info->maxunder>val)return(1);
			}
			j += decInc;
		}
	}
#else														/* SSE Optimize */
  int dy=y1-y0;
  int adx=x1-x0;
  int ady=abs(dy);
  int base=dy/adx;
  int sy=(dy<0?base-1:base+1);
  int x=x0;
  int y=y0;
  int err=0;
  int val=vorbis_dBquant(mask+x);
  int mse=0;
  int n=0;

  ady-=abs(base*adx);
  
  mse=(y-val);
  mse*=mse;
  n++;
  if(mdct[x]+info->twofitatten>=mask[x]){
    if(y+info->maxover<val)return(1);
    if(y-info->maxunder>val)return(1);
  }

  while(++x<x1){
    err=err+ady;
    if(err>=adx){
      err-=adx;
      y+=sy;
    }else{
      y+=base;
    }

    val=vorbis_dBquant(mask+x);
    mse+=((y-val)*(y-val));
    n++;
    if(mdct[x]+info->twofitatten>=mask[x]){
      if(val){
	if(y+info->maxover<val)return(1);
	if(y-info->maxunder>val)return(1);
      }
    }
  }
#endif														/* SSE Optimize */
  
  if(info->maxover*info->maxover/n>info->maxerr)return(0);
  if(info->maxunder*info->maxunder/n>info->maxerr)return(0);
  if(mse/n>info->maxerr)return(1);
  return(0);
}

static int post_Y(int *A,int *B,int pos){
  if(A[pos]<0)
    return B[pos];
  if(B[pos]<0)
    return A[pos];

  return (A[pos]+B[pos])>>1;
}

static int seq=0;

int *floor1_fit(vorbis_block *vb,vorbis_look_floor1 *look,
			  const float *logmdct,   /* in */
#if	defined(_OPENMP)
			  const float *logmask,
			  int *in_output){
#else
			  const float *logmask){
#endif
  long i,j;
  vorbis_info_floor1 *info=look->vi;
  long n=look->n;
  long posts=look->posts;
  long nonzero=0;
  lsfit_acc fits[VIF_POSIT+1];
  int fit_valueA[VIF_POSIT+2]; /* index by range list position */
  int fit_valueB[VIF_POSIT+2]; /* index by range list position */

  int loneighbor[VIF_POSIT+2]; /* sorted index of range list position (+2) */
  int hineighbor[VIF_POSIT+2]; 
  int *output=NULL;
  int memo[VIF_POSIT+2];

#if	defined(__SSE__)										/* SSE Optimize */
	float *tflr = (float*)_ogg_alloca(sizeof(float)*n);
	float *tmask = (float*)_ogg_alloca(sizeof(float)*n);
	int *tcres = (int*)_ogg_alloca(sizeof(int)*n);
	__m128	PIT	 = _mm_set1_ps(info->twofitatten);
	
	/*
		preprocess (vorbis_dbQuant)
	*/
	for(i=0;i<n;i+=16)
	{
		__m128	XMM0, XMM1, XMM2, XMM3;
#if	!defined(__SSE2__)
		register __m64	MM0, MM1, MM2, MM3, MM4, MM5, MM6, MM7;
#endif
		XMM0	 = _mm_load_ps(logmask+i   );
		XMM1	 = _mm_load_ps(logmask+i+ 4);
		XMM2	 = _mm_load_ps(logmask+i+ 8);
		XMM3	 = _mm_load_ps(logmask+i+12);
		XMM0	 = _mm_mul_ps(XMM0, PM128(pfv0));
		XMM1	 = _mm_mul_ps(XMM1, PM128(pfv0));
		XMM2	 = _mm_mul_ps(XMM2, PM128(pfv0));
		XMM3	 = _mm_mul_ps(XMM3, PM128(pfv0));
		XMM0	 = _mm_add_ps(XMM0, PM128(pfv1));
		XMM1	 = _mm_add_ps(XMM1, PM128(pfv1));
		XMM2	 = _mm_add_ps(XMM2, PM128(pfv1));
		XMM3	 = _mm_add_ps(XMM3, PM128(pfv1));
		XMM0	 = _mm_max_ps(XMM0, PM128(PFV_0));
		XMM1	 = _mm_max_ps(XMM1, PM128(PFV_0));
		XMM2	 = _mm_max_ps(XMM2, PM128(PFV_0));
		XMM3	 = _mm_max_ps(XMM3, PM128(PFV_0));
		XMM0	 = _mm_min_ps(XMM0, PM128(pfv2));
		XMM1	 = _mm_min_ps(XMM1, PM128(pfv2));
		XMM2	 = _mm_min_ps(XMM2, PM128(pfv2));
		XMM3	 = _mm_min_ps(XMM3, PM128(pfv2));
#if	defined(__SSE2__)
		XMM0	 = _mm_cvtepi32_ps(_mm_cvttps_epi32(XMM0));
		XMM1	 = _mm_cvtepi32_ps(_mm_cvttps_epi32(XMM1));
		XMM2	 = _mm_cvtepi32_ps(_mm_cvttps_epi32(XMM2));
		XMM3	 = _mm_cvtepi32_ps(_mm_cvttps_epi32(XMM3));
#else
		MM0	 = _mm_cvttps_pi32(XMM0);
		MM2	 = _mm_cvttps_pi32(XMM1);
		MM4	 = _mm_cvttps_pi32(XMM2);
		MM6	 = _mm_cvttps_pi32(XMM3);
		XMM0	 = _mm_movehl_ps(XMM0, XMM0);
		XMM1	 = _mm_movehl_ps(XMM1, XMM1);
		XMM2	 = _mm_movehl_ps(XMM2, XMM2);
		XMM3	 = _mm_movehl_ps(XMM3, XMM3);
		MM1	 = _mm_cvttps_pi32(XMM0);
		MM3	 = _mm_cvttps_pi32(XMM1);
		MM5	 = _mm_cvttps_pi32(XMM2);
		MM7	 = _mm_cvttps_pi32(XMM3);
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
		_mm_store_ps(tflr+i   , XMM0);
		_mm_store_ps(tflr+i+ 4, XMM1);
		_mm_store_ps(tflr+i+ 8, XMM2);
		_mm_store_ps(tflr+i+12, XMM3);
	}
#if	!defined(__SSE2__)
	_mm_empty();
#endif
	/*
		preprocess (mdct+info->twofitatten>=flr)
	*/
	for(i=0;i<n;i+=64)
	{
		__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6;
		XMM0	 = _mm_load_ps(logmdct+i   );
		XMM1	 = _mm_load_ps(logmdct+i+ 4);
		XMM2	 = _mm_load_ps(logmask+i   );
		XMM3	 = _mm_load_ps(logmask+i+ 4);
		XMM0	 = _mm_add_ps(XMM0, PIT);
		XMM1	 = _mm_add_ps(XMM1, PIT);
		XMM4	 = _mm_load_ps(logmdct+i+ 8);
		XMM5	 = _mm_load_ps(logmdct+i+12);
		XMM2	 = _mm_cmplt_ps(XMM2, XMM0);
		XMM3	 = _mm_cmplt_ps(XMM3, XMM1);
		XMM0	 = _mm_load_ps(logmask+i+ 8);
		XMM1	 = _mm_load_ps(logmask+i+12);
		XMM4	 = _mm_add_ps(XMM4, PIT);
		XMM5	 = _mm_add_ps(XMM5, PIT);
		_mm_store_ps(tmask+i   , XMM2);
		_mm_store_ps(tmask+i+ 4, XMM3);
		XMM6	 = _mm_load_ps(logmdct+i+16);
		XMM0	 = _mm_cmplt_ps(XMM0, XMM4);
		XMM4	 = _mm_load_ps(logmdct+i+20);
		XMM1	 = _mm_cmplt_ps(XMM1, XMM5);
		XMM5	 = _mm_load_ps(logmask+i+16);
		tcres[i   ]	 = _mm_movemask_ps(XMM2);
		XMM6	 = _mm_add_ps(XMM6, PIT);
		XMM2	 = _mm_load_ps(logmask+i+20);
		tcres[i+ 4]	 = _mm_movemask_ps(XMM3);
		XMM4	 = _mm_add_ps(XMM4, PIT);
		XMM3	 = _mm_load_ps(logmdct+i+24);
		_mm_store_ps(tmask+i+ 8, XMM0);
		tcres[i+ 8]	 = _mm_movemask_ps(XMM0);
		_mm_store_ps(tmask+i+12, XMM1);
		XMM0	 = _mm_load_ps(logmdct+i+28);
		tcres[i+12]	 = _mm_movemask_ps(XMM1);
		XMM1	 = _mm_load_ps(logmdct+i+32);
		XMM5	 = _mm_cmplt_ps(XMM5, XMM6);
		XMM2	 = _mm_cmplt_ps(XMM2, XMM4);
		XMM6	 = _mm_load_ps(logmask+i+24);
		XMM4	 = _mm_load_ps(logmask+i+28);
		XMM3	 = _mm_add_ps(XMM3, PIT);
		XMM0	 = _mm_add_ps(XMM0, PIT);
		_mm_store_ps(tmask+i+16, XMM5);
		_mm_store_ps(tmask+i+20, XMM2);
		XMM6	 = _mm_cmplt_ps(XMM6, XMM3);
		XMM3	 = _mm_load_ps(logmdct+i+36);
		XMM4	 = _mm_cmplt_ps(XMM4, XMM0);
		XMM0	 = _mm_load_ps(logmask+i+32);
		tcres[i+16]	 = _mm_movemask_ps(XMM5);
		XMM5	 = _mm_load_ps(logmask+i+36);
		XMM1	 = _mm_add_ps(XMM1, PIT);
		XMM3	 = _mm_add_ps(XMM3, PIT);
		tcres[i+20]	 = _mm_movemask_ps(XMM2);
		XMM2	 = _mm_load_ps(logmdct+i+40);
		_mm_store_ps(tmask+i+24, XMM6);
		tcres[i+24]	 = _mm_movemask_ps(XMM6);
		XMM6	 = _mm_load_ps(logmdct+i+44);
		_mm_store_ps(tmask+i+28, XMM4);
		XMM0	 = _mm_cmplt_ps(XMM0, XMM1);
		tcres[i+28]	 = _mm_movemask_ps(XMM4);
		XMM5	 = _mm_cmplt_ps(XMM5, XMM3);
		XMM1	 = _mm_load_ps(logmask+i+40);
		XMM3	 = _mm_load_ps(logmask+i+44);
		XMM2	 = _mm_add_ps(XMM2, PIT);
		XMM6	 = _mm_add_ps(XMM6, PIT);
		_mm_store_ps(tmask+i+32, XMM0);
		_mm_store_ps(tmask+i+36, XMM5);
		XMM4	 = _mm_load_ps(logmdct+i+48);
		XMM1	 = _mm_cmplt_ps(XMM1, XMM2);
		XMM2	 = _mm_load_ps(logmdct+i+52);
		XMM3	 = _mm_cmplt_ps(XMM3, XMM6);
		XMM6	 = _mm_load_ps(logmask+i+48);
		tcres[i+32]	 = _mm_movemask_ps(XMM0);
		XMM4	 = _mm_add_ps(XMM4, PIT);
		XMM0	 = _mm_load_ps(logmask+i+52);
		tcres[i+36]	 = _mm_movemask_ps(XMM5);
		XMM2	 = _mm_add_ps(XMM2, PIT);
		XMM5	 = _mm_load_ps(logmdct+i+56);
		_mm_store_ps(tmask+i+40, XMM1);
		tcres[i+40]	 = _mm_movemask_ps(XMM1);
		_mm_store_ps(tmask+i+44, XMM3);
		XMM1	 = _mm_load_ps(logmdct+i+60);
		tcres[i+44]	 = _mm_movemask_ps(XMM3);
		XMM6	 = _mm_cmplt_ps(XMM6, XMM4);
		XMM0	 = _mm_cmplt_ps(XMM0, XMM2);
		XMM4	 = _mm_load_ps(logmask+i+56);
		XMM2	 = _mm_load_ps(logmask+i+60);
		XMM5	 = _mm_add_ps(XMM5, PIT);
		XMM1	 = _mm_add_ps(XMM1, PIT);
		_mm_store_ps(tmask+i+48, XMM6);
		_mm_store_ps(tmask+i+52, XMM0);
		XMM4	 = _mm_cmplt_ps(XMM4, XMM5);
		XMM2	 = _mm_cmplt_ps(XMM2, XMM1);
		tcres[i+48]	 = _mm_movemask_ps(XMM6);
		tcres[i+52]	 = _mm_movemask_ps(XMM0);
		_mm_store_ps(tmask+i+56, XMM4);
		tcres[i+56]	 = _mm_movemask_ps(XMM4);
		_mm_store_ps(tmask+i+60, XMM2);
		tcres[i+60]	 = _mm_movemask_ps(XMM2);
	}
#endif														/* SSE Optimize */

  for(i=0;i<posts;i++)fit_valueA[i]=-200; /* mark all unused */
  for(i=0;i<posts;i++)fit_valueB[i]=-200; /* mark all unused */
  for(i=0;i<posts;i++)loneighbor[i]=0; /* 0 for the implicit 0 post */
  for(i=0;i<posts;i++)hineighbor[i]=1; /* 1 for the implicit post at n */
  for(i=0;i<posts;i++)memo[i]=-1;      /* no neighbor yet */

  /* quantize the relevant floor points and collect them into line fit
     structures (one per minimal division) at the same time */
#if	defined(__SSE__)										/* SSE Optimize */
	if(posts==0)
	{
		nonzero+=accumulate_fit(logmask,logmdct,0,n,fits,n,info, tflr, tmask, tcres);
	}
	else
	{
		for(i=0;i<posts-1;i++)
			nonzero+=accumulate_fit(logmask,logmdct,look->sorted_index[i],
				look->sorted_index[i+1],fits+i,
				n,info, tflr, tmask, tcres);
	}
#else														/* SSE Optimize */
  if(posts==0){
    nonzero+=accumulate_fit(logmask,logmdct,0,n,fits,n,info);
  }else{
    for(i=0;i<posts-1;i++)
      nonzero+=accumulate_fit(logmask,logmdct,look->sorted_index[i],
			      look->sorted_index[i+1],fits+i,
			      n,info);
  }
#endif														/* SSE Optimize */
  
  if(nonzero){
    /* start by fitting the implicit base case.... */
    int y0=-200;
    int y1=-200;
    fit_line(fits,posts-1,&y0,&y1);

    fit_valueA[0]=y0;
    fit_valueB[0]=y0;
    fit_valueB[1]=y1;
    fit_valueA[1]=y1;

    /* Non degenerate case */
    /* start progressive splitting.  This is a greedy, non-optimal
       algorithm, but simple and close enough to the best
       answer. */
    for(i=2;i<posts;i++){
      int sortpos=look->reverse_index[i];
      int ln=loneighbor[sortpos];
      int hn=hineighbor[sortpos];
      
      /* eliminate repeat searches of a particular range with a memo */
      if(memo[ln]!=hn){
	/* haven't performed this error search yet */
	int lsortpos=look->reverse_index[ln];
	int hsortpos=look->reverse_index[hn];
	memo[ln]=hn;
		
	{
	  /* A note: we want to bound/minimize *local*, not global, error */
	  int lx=info->postlist[ln];
	  int hx=info->postlist[hn];	  
	  int ly=post_Y(fit_valueA,fit_valueB,ln);
	  int hy=post_Y(fit_valueA,fit_valueB,hn);
	  
	  if(ly==-1 || hy==-1){
	    exit(1);
	  }

#if	defined(__SSE__)										/* SSE Optimize */
	  if(inspect_error(lx,hx,ly,hy,logmask,logmdct,info, tflr, tmask, tcres)){
#else														/* SSE Optimize */
	  if(inspect_error(lx,hx,ly,hy,logmask,logmdct,info)){
#endif														/* SSE Optimize */
	    /* outside error bounds/begin search area.  Split it. */
	    int ly0=-200;
	    int ly1=-200;
	    int hy0=-200;
	    int hy1=-200;
	    fit_line(fits+lsortpos,sortpos-lsortpos,&ly0,&ly1);
	    fit_line(fits+sortpos,hsortpos-sortpos,&hy0,&hy1);
	    
	    /* store new edge values */
	    fit_valueB[ln]=ly0;
	    if(ln==0)fit_valueA[ln]=ly0;
	    fit_valueA[i]=ly1;
	    fit_valueB[i]=hy0;
	    fit_valueA[hn]=hy1;
	    if(hn==1)fit_valueB[hn]=hy1;
	    
	    if(ly1>=0 || hy0>=0){
	      /* store new neighbor values */
	      for(j=sortpos-1;j>=0;j--)
		if(hineighbor[j]==hn)
		  hineighbor[j]=i;
		else
		  break;
	      for(j=sortpos+1;j<posts;j++)
		if(loneighbor[j]==ln)
		  loneighbor[j]=i;
		else
		  break;
	      
	    }
	  }else{
	    
	    fit_valueA[i]=-200;
	    fit_valueB[i]=-200;
	  }
	}
      }
    }
  
#if	defined(_OPENMP)
    output=in_output;
#else
    output=_vorbis_block_alloc(vb,sizeof(*output)*posts);
#endif
    
    output[0]=post_Y(fit_valueA,fit_valueB,0);
    output[1]=post_Y(fit_valueA,fit_valueB,1);
    
    /* fill in posts marked as not using a fit; we will zero
       back out to 'unused' when encoding them so long as curve
       interpolation doesn't force them into use */
    for(i=2;i<posts;i++){
      int ln=look->loneighbor[i-2];
      int hn=look->hineighbor[i-2];
      int x0=info->postlist[ln];
      int x1=info->postlist[hn];
      int y0=output[ln];
      int y1=output[hn];
      
      int predicted=render_point(x0,x1,y0,y1,info->postlist[i]);
      int vx=post_Y(fit_valueA,fit_valueB,i);
      
      if(vx>=0 && predicted!=vx){ 
	output[i]=vx;
      }else{
	output[i]= predicted|0x8000;
      }
    }
  }

  return(output);
  
}
		
int *floor1_interpolate_fit(vorbis_block *vb,vorbis_look_floor1 *look,
			  int *A,int *B,
#if	defined(_OPENMP)
			  int del,
			  int *in_output){
#else
			  int del){
#endif

  long i;
  long posts=look->posts;
  int *output=NULL;
  
  if(A && B){
#if	defined(_OPENMP)
    output=in_output;
#else
    output=_vorbis_block_alloc(vb,sizeof(*output)*posts);
#endif
    
    for(i=0;i<posts;i++){
      output[i]=((65536-del)*(A[i]&0x7fff)+del*(B[i]&0x7fff)+32768)>>16;
      if(A[i]&0x8000 && B[i]&0x8000)output[i]|=0x8000;
    }
  }

  return(output);
}


int floor1_encode(oggpack_buffer *opb,vorbis_block *vb,
		  vorbis_look_floor1 *look,
#if	defined(_OPENMP)
		   int *post,int *ilogmask, oggpack_writecache *opwc){
#else
		   int *post,int *ilogmask){
#endif

  long i,j;
  vorbis_info_floor1 *info=look->vi;
  long n=look->n;
  long posts=look->posts;
  codec_setup_info *ci=vb->vd->vi->codec_setup;
  int out[VIF_POSIT+2];
  static_codebook **sbooks=ci->book_param;
  codebook *books=ci->fullbooks;
  static long seq=0; 

  /* quantize values to multiplier spec */
  if(post){
#if	defined(__SSE2__)
	int	posts4	 = posts&(~3);
	int	posts8	 = posts&(~7);
	_MM_ALIGN16 unsigned long PIV0[4]	 = 
		{0x00007FFF, 0x00007FFF, 0x00007FFF, 0x00007FFF};
	_MM_ALIGN16 unsigned long PIV1[4]	 = 
		{0xFFFF8000, 0xFFFF8000, 0xFFFF8000, 0xFFFF8000};
	
	i	 = 0;
	switch(info->mult)
	{
		case 1:
			for(;i<posts8;i+=8)
			{
				__m128i	PV0	 = PM128I(post+i  );
				__m128i	PV1	 = PM128I(post+i+4);
				__m128i	PV2	 = PV0;
				__m128i	PV3	 = PV1;
				PV0	 = _mm_and_si128(PV0, PM128I(PIV0));
				PV1	 = _mm_and_si128(PV1, PM128I(PIV0));
				PV2	 = _mm_and_si128(PV2, PM128I(PIV1));
				PV3	 = _mm_and_si128(PV3, PM128I(PIV1));
				PV0	 = _mm_srli_epi32(PV0, 2);
				PV1	 = _mm_srli_epi32(PV1, 2);
				PV0	 = _mm_or_si128(PV0, PV2);
				PV1	 = _mm_or_si128(PV1, PV3);
				PM128I(post+i  )	 = PV0;
				PM128I(post+i+4)	 = PV1;
			}
			for(;i<posts4;i+=4)
			{
				__m128i	PV0	 = PM128I(post+i  );
				__m128i	PV2	 = PV0;
				PV0	 = _mm_and_si128(PV0, PM128I(PIV0));
				PV2	 = _mm_and_si128(PV2, PM128I(PIV1));
				PV0	 = _mm_srli_epi32(PV0, 2);
				PV0	 = _mm_or_si128(PV0, PV2);
				PM128I(post+i  )	 = PV0;
			}
			break;
		case 2:
			for(;i<posts8;i+=8)
			{
				__m128i	PV0	 = PM128I(post+i  );
				__m128i	PV1	 = PM128I(post+i+4);
				__m128i	PV2	 = PV0;
				__m128i	PV3	 = PV1;
				PV0	 = _mm_and_si128(PV0, PM128I(PIV0));
				PV1	 = _mm_and_si128(PV1, PM128I(PIV0));
				PV2	 = _mm_and_si128(PV2, PM128I(PIV1));
				PV3	 = _mm_and_si128(PV3, PM128I(PIV1));
				PV0	 = _mm_srli_epi32(PV0, 3);
				PV1	 = _mm_srli_epi32(PV1, 3);
				PV0	 = _mm_or_si128(PV0, PV2);
				PV1	 = _mm_or_si128(PV1, PV3);
				PM128I(post+i  )	 = PV0;
				PM128I(post+i+4)	 = PV1;
			}
			for(;i<posts4;i+=4)
			{
				__m128i	PV0	 = PM128I(post+i  );
				__m128i	PV2	 = PV0;
				PV0	 = _mm_and_si128(PV0, PM128I(PIV0));
				PV2	 = _mm_and_si128(PV2, PM128I(PIV1));
				PV0	 = _mm_srli_epi32(PV0, 3);
				PV0	 = _mm_or_si128(PV0, PV2);
				PM128I(post+i  )	 = PV0;
			}
			break;
		case 3:
			for(;i<posts8;i+=8)
			{
				__m128i	PV0	 = PM128I(post+i  );
				__m128i	PV1	 = PM128I(post+i+4);
				__m128i	PV2	 = PV0;
				__m128i	PV3	 = PV1;
				__m128i	PV4, PV5;
				PV0	 = _mm_and_si128(PV0, PM128I(PIV0));
				PV1	 = _mm_and_si128(PV1, PM128I(PIV0));
				PV2	 = _mm_and_si128(PV2, PM128I(PIV1));
				PV3	 = _mm_and_si128(PV3, PM128I(PIV1));
				PV0	 = _mm_srli_epi32(PV0, 4);
				PV1	 = _mm_srli_epi32(PV1, 4);
				PV4	 = PV0;
				PV5	 = PV1;
				PV0	 = _mm_add_epi32(PV0, PV0);
				PV1	 = _mm_add_epi32(PV1, PV1);
				PV0	 = _mm_add_epi32(PV0, PV4);
				PV1	 = _mm_add_epi32(PV1, PV5);
				PV0	 = _mm_or_si128(PV0, PV2);
				PV1	 = _mm_or_si128(PV1, PV3);
				PM128I(post+i  )	 = PV0;
				PM128I(post+i+4)	 = PV1;
			}
			for(;i<posts4;i+=4)
			{
				__m128i	PV0	 = PM128I(post+i  );
				__m128i	PV2	 = PV0;
				__m128i	PV4;
				PV0	 = _mm_and_si128(PV0, PM128I(PIV0));
				PV2	 = _mm_and_si128(PV2, PM128I(PIV1));
				PV0	 = _mm_srli_epi32(PV0, 4);
				PV4	 = PV0;
				PV0	 = _mm_add_epi32(PV0, PV0);
				PV0	 = _mm_add_epi32(PV0, PV4);
				PV0	 = _mm_or_si128(PV0, PV2);
				PM128I(post+i  )	 = PV0;
			}
			break;
		case 4:
			for(;i<posts8;i+=8)
			{
				__m128i	PV0	 = PM128I(post+i  );
				__m128i	PV1	 = PM128I(post+i+4);
				__m128i	PV2	 = PV0;
				__m128i	PV3	 = PV1;
				PV0	 = _mm_and_si128(PV0, PM128I(PIV0));
				PV1	 = _mm_and_si128(PV1, PM128I(PIV0));
				PV2	 = _mm_and_si128(PV2, PM128I(PIV1));
				PV3	 = _mm_and_si128(PV3, PM128I(PIV1));
				PV0	 = _mm_srli_epi32(PV0, 4);
				PV1	 = _mm_srli_epi32(PV1, 4);
				PV0	 = _mm_or_si128(PV0, PV2);
				PV1	 = _mm_or_si128(PV1, PV3);
				PM128I(post+i  )	 = PV0;
				PM128I(post+i+4)	 = PV1;
			}
			for(;i<posts4;i+=4)
			{
				__m128i	PV0	 = PM128I(post+i  );
				__m128i	PV2	 = PV0;
				PV0	 = _mm_and_si128(PV0, PM128I(PIV0));
				PV2	 = _mm_and_si128(PV2, PM128I(PIV1));
				PV0	 = _mm_srli_epi32(PV0, 4);
				PV0	 = _mm_or_si128(PV0, PV2);
				PM128I(post+i  )	 = PV0;
			}
			break;
	}
	for(;i<posts;i++)
	{
		int val	 = post[i]&0x7fff;
		switch(info->mult)
		{
			case 1: /* 1024 -> 256 */
				val>>=2;
				break;
			case 2: /* 1024 -> 128 */
				val>>=3;
				break;
			case 3: /* 1024 -> 86 */
				val/=12;
				break;
			case 4: /* 1024 -> 64 */
				val>>=4;
				break;
		}
		post[i]=val | (post[i]&0x8000);
	}
#elif	defined(__SSE__)
	int	posts2	 = posts&(~1);
	int	posts4	 = posts&(~3);
	static unsigned long PIV0[2]	 = {0x00007FFF, 0x00007FFF};
	static unsigned long PIV1[2]	 = {0xFFFF8000, 0xFFFF8000};
	
	i	 = 0;
	switch(info->mult)
	{
		case 1:
			for(;i<posts4;i+=4)
			{
				__m64	PV0	 = PM64(post+i  );
				__m64	PV1	 = PM64(post+i+2);
				__m64	PV2	 = PV0;
				__m64	PV3	 = PV1;
				PV0	 = _mm_and_si64(PV0, PM64(PIV0));
				PV1	 = _mm_and_si64(PV1, PM64(PIV0));
				PV2	 = _mm_and_si64(PV2, PM64(PIV1));
				PV3	 = _mm_and_si64(PV3, PM64(PIV1));
				PV0	 = _mm_srli_pi32(PV0, 2);
				PV1	 = _mm_srli_pi32(PV1, 2);
				PV0	 = _mm_or_si64(PV0, PV2);
				PV1	 = _mm_or_si64(PV1, PV3);
				PM64(post+i  )	 = PV0;
				PM64(post+i+2)	 = PV1;
			}
			for(;i<posts2;i+=2)
			{
				__m64	PV0	 = PM64(post+i  );
				__m64	PV2	 = PV0;
				PV0	 = _mm_and_si64(PV0, PM64(PIV0));
				PV2	 = _mm_and_si64(PV2, PM64(PIV1));
				PV0	 = _mm_srli_pi32(PV0, 2);
				PV0	 = _mm_or_si64(PV0, PV2);
				PM64(post+i  )	 = PV0;
			}
			break;
		case 2:
			for(;i<posts4;i+=4)
			{
				__m64	PV0	 = PM64(post+i  );
				__m64	PV1	 = PM64(post+i+2);
				__m64	PV2	 = PV0;
				__m64	PV3	 = PV1;
				PV0	 = _mm_and_si64(PV0, PM64(PIV0));
				PV1	 = _mm_and_si64(PV1, PM64(PIV0));
				PV2	 = _mm_and_si64(PV2, PM64(PIV1));
				PV3	 = _mm_and_si64(PV3, PM64(PIV1));
				PV0	 = _mm_srli_pi32(PV0, 3);
				PV1	 = _mm_srli_pi32(PV1, 3);
				PV0	 = _mm_or_si64(PV0, PV2);
				PV1	 = _mm_or_si64(PV1, PV3);
				PM64(post+i  )	 = PV0;
				PM64(post+i+2)	 = PV1;
			}
			for(;i<posts2;i+=2)
			{
				__m64	PV0	 = PM64(post+i  );
				__m64	PV2	 = PV0;
				PV0	 = _mm_and_si64(PV0, PM64(PIV0));
				PV2	 = _mm_and_si64(PV2, PM64(PIV1));
				PV0	 = _mm_srli_pi32(PV0, 3);
				PV0	 = _mm_or_si64(PV0, PV2);
				PM64(post+i  )	 = PV0;
			}
			break;
		case 3:
			for(;i<posts4;i+=4)
			{
				__m64	PV0	 = PM64(post+i  );
				__m64	PV1	 = PM64(post+i+2);
				__m64	PV2	 = PV0;
				__m64	PV3	 = PV1;
				__m64	PV4, PV5;
				PV0	 = _mm_and_si64(PV0, PM64(PIV0));
				PV1	 = _mm_and_si64(PV1, PM64(PIV0));
				PV2	 = _mm_and_si64(PV2, PM64(PIV1));
				PV3	 = _mm_and_si64(PV3, PM64(PIV1));
				PV0	 = _mm_srli_pi32(PV0, 4);
				PV1	 = _mm_srli_pi32(PV1, 4);
				PV4	 = PV0;
				PV5	 = PV1;
				PV0	 = _mm_add_pi32(PV0, PV0);
				PV1	 = _mm_add_pi32(PV1, PV1);
				PV0	 = _mm_add_pi32(PV0, PV4);
				PV1	 = _mm_add_pi32(PV1, PV5);
				PV0	 = _mm_or_si64(PV0, PV2);
				PV1	 = _mm_or_si64(PV1, PV3);
				PM64(post+i  )	 = PV0;
				PM64(post+i+2)	 = PV1;
			}
			for(;i<posts2;i+=2)
			{
				__m64	PV0	 = PM64(post+i  );
				__m64	PV2	 = PV0;
				__m64	PV4;
				PV0	 = _mm_and_si64(PV0, PM64(PIV0));
				PV2	 = _mm_and_si64(PV2, PM64(PIV1));
				PV0	 = _mm_srli_pi32(PV0, 4);
				PV4	 = PV0;
				PV0	 = _mm_add_pi32(PV0, PV0);
				PV0	 = _mm_add_pi32(PV0, PV4);
				PV0	 = _mm_or_si64(PV0, PV2);
				PM64(post+i  )	 = PV0;
			}
			break;
		case 4:
			for(;i<posts4;i+=4)
			{
				__m64	PV0	 = PM64(post+i  );
				__m64	PV1	 = PM64(post+i+2);
				__m64	PV2	 = PV0;
				__m64	PV3	 = PV1;
				PV0	 = _mm_and_si64(PV0, PM64(PIV0));
				PV1	 = _mm_and_si64(PV1, PM64(PIV0));
				PV2	 = _mm_and_si64(PV2, PM64(PIV1));
				PV3	 = _mm_and_si64(PV3, PM64(PIV1));
				PV0	 = _mm_srli_pi32(PV0, 4);
				PV1	 = _mm_srli_pi32(PV1, 4);
				PV0	 = _mm_or_si64(PV0, PV2);
				PV1	 = _mm_or_si64(PV1, PV3);
				PM64(post+i  )	 = PV0;
				PM64(post+i+2)	 = PV1;
			}
			for(;i<posts2;i+=2)
			{
				__m64	PV0	 = PM64(post+i  );
				__m64	PV2	 = PV0;
				PV0	 = _mm_and_si64(PV0, PM64(PIV0));
				PV2	 = _mm_and_si64(PV2, PM64(PIV1));
				PV0	 = _mm_srli_pi32(PV0, 4);
				PV0	 = _mm_or_si64(PV0, PV2);
				PM64(post+i  )	 = PV0;
			}
			break;
	}
	_mm_empty();
	for(;i<posts;i++)
	{
		int val	 = post[i]&0x7fff;
		switch(info->mult)
		{
			case 1: /* 1024 -> 256 */
				val>>=2;
				break;
			case 2: /* 1024 -> 128 */
				val>>=3;
				break;
			case 3: /* 1024 -> 86 */
				val/=12;
				break;
			case 4: /* 1024 -> 64 */
				val>>=4;
				break;
		}
		post[i]=val | (post[i]&0x8000);
	}
#else														/* SSE Optimize */
    for(i=0;i<posts;i++){
      int val=post[i]&0x7fff;
      switch(info->mult){
      case 1: /* 1024 -> 256 */
	val>>=2;
	break;
      case 2: /* 1024 -> 128 */
	val>>=3;
	break;
      case 3: /* 1024 -> 86 */
	val/=12;
	break;
      case 4: /* 1024 -> 64 */
	val>>=4;
	break;
      }
      post[i]=val | (post[i]&0x8000);
    }
#endif														/* SSE Optimize */

    out[0]=post[0];
    out[1]=post[1];

    /* find prediction values for each post and subtract them */
    for(i=2;i<posts;i++){
      int ln=look->loneighbor[i-2];
      int hn=look->hineighbor[i-2];
      int x0=info->postlist[ln];
      int x1=info->postlist[hn];
      int y0=post[ln];
      int y1=post[hn];
      
      int predicted=render_point(x0,x1,y0,y1,info->postlist[i]);
      
      if((post[i]&0x8000) || (predicted==post[i])){
	post[i]=predicted|0x8000; /* in case there was roundoff jitter
				     in interpolation */
	out[i]=0;
      }else{
	int headroom=(look->quant_q-predicted<predicted?
		      look->quant_q-predicted:predicted);
	
	int val=post[i]-predicted;
	
	/* at this point the 'deviation' value is in the range +/- max
	   range, but the real, unique range can always be mapped to
	   only [0-maxrange).  So we want to wrap the deviation into
	   this limited range, but do it in the way that least screws
	   an essentially gaussian probability distribution. */
	
	if(val<0)
	  if(val<-headroom)
	    val=headroom-val-1;
	  else
	    val=-1-(val<<1);
	else
	  if(val>=headroom)
	    val= val+headroom;
	  else
	    val<<=1;
	
	out[i]=val;
	post[ln]&=0x7fff;
	post[hn]&=0x7fff;
      }
    }
    
    /* we have everything we need. pack it out */
    /* mark nontrivial floor */
#if	defined(_OPENMP)
    vorbis_oggpack_writecache(opwc,1,1);
#else
    oggpack_write(opb,1,1);
#endif
      
    /* beginning/end post */
    look->frames++;
    look->postbits+=ilog(look->quant_q-1)*2;
#if	defined(_OPENMP)
    vorbis_oggpack_writecache(opwc,out[0],ilog(look->quant_q-1));
    vorbis_oggpack_writecache(opwc,out[1],ilog(look->quant_q-1));
#else
    oggpack_write(opb,out[0],ilog(look->quant_q-1));
    oggpack_write(opb,out[1],ilog(look->quant_q-1));
#endif
      
      
    /* partition by partition */
    for(i=0,j=2;i<info->partitions;i++){
      int class=info->partitionclass[i];
      int cdim=info->class_dim[class];
      int csubbits=info->class_subs[class];
      int csub=1<<csubbits;
      int bookas[8]={0,0,0,0,0,0,0,0};
      int cval=0;
      int cshift=0;
      int k,l;

      /* generate the partition's first stage cascade value */
      if(csubbits){
	int maxval[8];
	for(k=0;k<csub;k++){
	  int booknum=info->class_subbook[class][k];
	  if(booknum<0){
	    maxval[k]=1;
	  }else{
	    maxval[k]=sbooks[info->class_subbook[class][k]]->entries;
	  }
	}
	for(k=0;k<cdim;k++){
	  for(l=0;l<csub;l++){
	    int val=out[j+k];
	    if(val<maxval[l]){
	      bookas[k]=l;
	      break;
	    }
	  }
	  cval|= bookas[k]<<cshift;
	  cshift+=csubbits;
	}
	/* write it */
#if	defined(_OPENMP)
	look->phrasebits+=
	  vorbis_book_encode_cache(books+info->class_book[class],cval,opwc);
#else
	look->phrasebits+=
	  vorbis_book_encode(books+info->class_book[class],cval,opb);
#endif
	
#ifdef TRAIN_FLOOR1
	{
	  FILE *of;
	  char buffer[80];
	  sprintf(buffer,"line_%dx%ld_class%d.vqd",
		  vb->pcmend/2,posts-2,class);
	  of=fopen(buffer,"a");
	  fprintf(of,"%d\n",cval);
	  fclose(of);
	}
#endif
      }
	
      /* write post values */
      for(k=0;k<cdim;k++){
	int book=info->class_subbook[class][bookas[k]];
	if(book>=0){
	  /* hack to allow training with 'bad' books */
	  if(out[j+k]<(books+book)->entries)
#if	defined(_OPENMP)
	    look->postbits+=vorbis_book_encode_cache(books+book,
					       out[j+k],opwc);
#else
	    look->postbits+=vorbis_book_encode(books+book,
					       out[j+k],opb);
#endif
	  /*else
	    fprintf(stderr,"+!");*/
	  
#ifdef TRAIN_FLOOR1
	  {
	    FILE *of;
	    char buffer[80];
	    sprintf(buffer,"line_%dx%ld_%dsub%d.vqd",
		    vb->pcmend/2,posts-2,class,bookas[k]);
	    of=fopen(buffer,"a");
	    fprintf(of,"%d\n",out[j+k]);
	    fclose(of);
	  }
#endif
	}
      }
      j+=cdim;
    }
    
    {
      /* generate quantized floor equivalent to what we'd unpack in decode */
      /* render the lines */
      int hx=0;
      int lx=0;
      int ly=post[0]*info->mult;
      for(j=1;j<look->posts;j++){
	int current=look->forward_index[j];
	int hy=post[current]&0x7fff;
	if(hy==post[current]){
	  
	  hy*=info->mult;
	  hx=info->postlist[current];
	
	  render_line0(lx,hx,ly,hy,ilogmask);
	
	  lx=hx;
	  ly=hy;
	}
      }
#if	defined(__SSE__)&&!defined(__SSE2__)					/* SSE Optimize */
	_mm_empty();
#endif														/* SSE Optimize */
      for(j=hx;j<vb->pcmend/2;j++)ilogmask[j]=ly; /* be certain */    
      seq++;
      return(1);
    }
  }else{
#if	defined(_OPENMP)
    vorbis_oggpack_writecache(opwc,0,1);
#else
    oggpack_write(opb,0,1);
#endif
    memset(ilogmask,0,vb->pcmend/2*sizeof(*ilogmask));
    seq++;
    return(0);
  }
}

static void *floor1_inverse1(vorbis_block *vb,vorbis_look_floor *in){
  vorbis_look_floor1 *look=(vorbis_look_floor1 *)in;
  vorbis_info_floor1 *info=look->vi;
  codec_setup_info   *ci=vb->vd->vi->codec_setup;
  
  int i,j,k;
  codebook *books=ci->fullbooks;   

  /* unpack wrapped/predicted values from stream */
  if(oggpack_read(&vb->opb,1)==1){
    int *fit_value=_vorbis_block_alloc(vb,(look->posts)*sizeof(*fit_value));

    fit_value[0]=oggpack_read(&vb->opb,ilog(look->quant_q-1));
    fit_value[1]=oggpack_read(&vb->opb,ilog(look->quant_q-1));

    /* partition by partition */
    for(i=0,j=2;i<info->partitions;i++){
      int class=info->partitionclass[i];
      int cdim=info->class_dim[class];
      int csubbits=info->class_subs[class];
      int csub=1<<csubbits;
      int cval=0;

      /* decode the partition's first stage cascade value */
      if(csubbits){
	cval=vorbis_book_decode(books+info->class_book[class],&vb->opb);

	if(cval==-1)goto eop;
      }

      for(k=0;k<cdim;k++){
	int book=info->class_subbook[class][cval&(csub-1)];
	cval>>=csubbits;
	if(book>=0){
	  if((fit_value[j+k]=vorbis_book_decode(books+book,&vb->opb))==-1)
	    goto eop;
	}else{
	  fit_value[j+k]=0;
	}
      }
      j+=cdim;
    }

    /* unwrap positive values and reconsitute via linear interpolation */
    for(i=2;i<look->posts;i++){
      int predicted=render_point(info->postlist[look->loneighbor[i-2]],
				 info->postlist[look->hineighbor[i-2]],
				 fit_value[look->loneighbor[i-2]],
				 fit_value[look->hineighbor[i-2]],
				 info->postlist[i]);
      int hiroom=look->quant_q-predicted;
      int loroom=predicted;
      int room=(hiroom<loroom?hiroom:loroom)<<1;
      int val=fit_value[i];

      if(val){
	if(val>=room){
	  if(hiroom>loroom){
	    val = val-loroom;
	  }else{
	    val = -1-(val-hiroom);
	  }
	}else{
	  if(val&1){
	    val= -((val+1)>>1);
	  }else{
	    val>>=1;
	  }
	}

	fit_value[i]=val+predicted;
	fit_value[look->loneighbor[i-2]]&=0x7fff;
	fit_value[look->hineighbor[i-2]]&=0x7fff;

      }else{
	fit_value[i]=predicted|0x8000;
      }
	
    }

    return(fit_value);
  }
 eop:
  return(NULL);
}

static int floor1_inverse2(vorbis_block *vb,vorbis_look_floor *in,void *memo,
			  float *out){
  vorbis_look_floor1 *look=(vorbis_look_floor1 *)in;
  vorbis_info_floor1 *info=look->vi;

  codec_setup_info   *ci=vb->vd->vi->codec_setup;
  int                  n=ci->blocksizes[vb->W]/2;
  int j;

  if(memo){
    /* render the lines */
    int *fit_value=(int *)memo;
    int hx=0;
    int lx=0;
    int ly=fit_value[0]*info->mult;
    for(j=1;j<look->posts;j++){
      int current=look->forward_index[j];
      int hy=fit_value[current]&0x7fff;
      if(hy==fit_value[current]){
	
	hy*=info->mult;
	hx=info->postlist[current];
	
	render_line(lx,hx,ly,hy,out);
	
	lx=hx;
	ly=hy;
      }
    }
    for(j=hx;j<n;j++)out[j]*=FLOOR1_fromdB_LOOKUP[ly]; /* be certain */    
    return(1);
  }
#if	defined(__SSE__)										/* SSE Optimize */
  {
	__m128 XMM0 = _mm_setzero_ps();
	for(j=0;j<n;j+=16)
	{
	  _mm_store_ps(out+j   , XMM0);
	  _mm_store_ps(out+j+ 4, XMM0);
	  _mm_store_ps(out+j+ 8, XMM0);
	  _mm_store_ps(out+j+12, XMM0);
	}
  }
#else														/* SSE Optimize */
  memset(out,0,sizeof(*out)*n);
#endif														/* SSE Optimize */

  return(0);
}

/* export hooks */
vorbis_func_floor floor1_exportbundle={
  &floor1_pack,&floor1_unpack,&floor1_look,&floor1_free_info,
  &floor1_free_look,&floor1_inverse1,&floor1_inverse2
};


