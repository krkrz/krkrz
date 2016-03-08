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

 function: residue backend 0, 1 and 2 implementation
 last mod: $Id: res0.c 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

/* Slow, slow, slow, simpleminded and did I mention it was slow?  The
   encode/decode loops are coded for clarity and performance is not
   yet even a nagging little idea lurking in the shadows.  Oh and BTW,
   it's slow. */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ogg/ogg.h>
#include "vorbis/codec.h"
#include "codec_internal.h"
#include "registry.h"
#include "codebook.h"
#include "misc.h"
#include "os.h"
#ifdef __SSE__												/* SSE Optimize */
#include "xmmlib.h"
#endif														/* SSE Optimize */

#if defined(TRAIN_RES) || defined (TRAIN_RESAUX)
#include <stdio.h>
#endif 

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

void res0_free_info(vorbis_info_residue *i){
  vorbis_info_residue0 *info=(vorbis_info_residue0 *)i;
  if(info){
    memset(info,0,sizeof(*info));
    _ogg_free(info);
  }
}

void res0_free_look(vorbis_look_residue *i){
  int j;
  if(i){

    vorbis_look_residue0 *look=(vorbis_look_residue0 *)i;

#ifdef TRAIN_RES
    {
      int j,k,l;
      for(j=0;j<look->parts;j++){
	/*fprintf(stderr,"partition %d: ",j);*/
	for(k=0;k<8;k++)
	  if(look->training_data[k][j]){
	    char buffer[80];
	    FILE *of;
	    codebook *statebook=look->partbooks[j][k];
	    
	    /* long and short into the same bucket by current convention */
	    sprintf(buffer,"res_part%d_pass%d.vqd",j,k);
	    of=fopen(buffer,"a");

	    for(l=0;l<statebook->entries;l++)
	      fprintf(of,"%d:%ld\n",l,look->training_data[k][j][l]);
	    
	    fclose(of);
	    
	    /*fprintf(stderr,"%d(%.2f|%.2f) ",k,
	      look->training_min[k][j],look->training_max[k][j]);*/

	    _ogg_free(look->training_data[k][j]);
	    look->training_data[k][j]=NULL;
	  }
	/*fprintf(stderr,"\n");*/
      }
    }
    fprintf(stderr,"min/max residue: %g::%g\n",look->tmin,look->tmax);

    /*fprintf(stderr,"residue bit usage %f:%f (%f total)\n",
	    (float)look->phrasebits/look->frames,
	    (float)look->postbits/look->frames,
	    (float)(look->postbits+look->phrasebits)/look->frames);*/
#endif


    /*vorbis_info_residue0 *info=look->info;

    fprintf(stderr,
	    "%ld frames encoded in %ld phrasebits and %ld residue bits "
	    "(%g/frame) \n",look->frames,look->phrasebits,
	    look->resbitsflat,
	    (look->phrasebits+look->resbitsflat)/(float)look->frames);
    
    for(j=0;j<look->parts;j++){
      long acc=0;
      fprintf(stderr,"\t[%d] == ",j);
      for(k=0;k<look->stages;k++)
	if((info->secondstages[j]>>k)&1){
	  fprintf(stderr,"%ld,",look->resbits[j][k]);
	  acc+=look->resbits[j][k];
	}

      fprintf(stderr,":: (%ld vals) %1.2fbits/sample\n",look->resvals[j],
	      acc?(float)acc/(look->resvals[j]*info->grouping):0);
    }
    fprintf(stderr,"\n");*/

    for(j=0;j<look->parts;j++)
      if(look->partbooks[j])_ogg_free(look->partbooks[j]);
    _ogg_free(look->partbooks);
    for(j=0;j<look->partvals;j++)
      _ogg_free(look->decodemap[j]);
    _ogg_free(look->decodemap);

    memset(look,0,sizeof(*look));
    _ogg_free(look);
  }
}

static int ilog(unsigned int v){
  int ret=0;
  while(v){
    ret++;
    v>>=1;
  }
  return(ret);
}

static int icount(unsigned int v){
  int ret=0;
  while(v){
    ret+=v&1;
    v>>=1;
  }
  return(ret);
}


void res0_pack(vorbis_info_residue *vr,oggpack_buffer *opb){
  vorbis_info_residue0 *info=(vorbis_info_residue0 *)vr;
  int j,acc=0;
  oggpack_write(opb,info->begin,24);
  oggpack_write(opb,info->end,24);

  oggpack_write(opb,info->grouping-1,24);  /* residue vectors to group and 
					     code with a partitioned book */
  oggpack_write(opb,info->partitions-1,6); /* possible partition choices */
  oggpack_write(opb,info->groupbook,8);  /* group huffman book */

  /* secondstages is a bitmask; as encoding progresses pass by pass, a
     bitmask of one indicates this partition class has bits to write
     this pass */
  for(j=0;j<info->partitions;j++){
    if(ilog(info->secondstages[j])>3){
      /* yes, this is a minor hack due to not thinking ahead */
      oggpack_write(opb,info->secondstages[j],3); 
      oggpack_write(opb,1,1);
      oggpack_write(opb,info->secondstages[j]>>3,5); 
    }else
      oggpack_write(opb,info->secondstages[j],4); /* trailing zero */
    acc+=icount(info->secondstages[j]);
  }
  for(j=0;j<acc;j++)
    oggpack_write(opb,info->booklist[j],8);

}

/* vorbis_info is for range checking */
vorbis_info_residue *res0_unpack(vorbis_info *vi,oggpack_buffer *opb){
  int j,acc=0;
  vorbis_info_residue0 *info=_ogg_calloc(1,sizeof(*info));
  codec_setup_info     *ci=vi->codec_setup;

  info->begin=oggpack_read(opb,24);
  info->end=oggpack_read(opb,24);
  info->grouping=oggpack_read(opb,24)+1;
  info->partitions=oggpack_read(opb,6)+1;
  info->groupbook=oggpack_read(opb,8);

  for(j=0;j<info->partitions;j++){
    int cascade=oggpack_read(opb,3);
    if(oggpack_read(opb,1))
      cascade|=(oggpack_read(opb,5)<<3);
    info->secondstages[j]=cascade;

    acc+=icount(cascade);
  }
  for(j=0;j<acc;j++)
    info->booklist[j]=oggpack_read(opb,8);

  if(info->groupbook>=ci->books)goto errout;
  for(j=0;j<acc;j++)
    if(info->booklist[j]>=ci->books)goto errout;

  return(info);
 errout:
  res0_free_info(info);
  return(NULL);
}

vorbis_look_residue *res0_look(vorbis_dsp_state *vd,
			       vorbis_info_residue *vr){
  vorbis_info_residue0 *info=(vorbis_info_residue0 *)vr;
  vorbis_look_residue0 *look=_ogg_calloc(1,sizeof(*look));
  codec_setup_info     *ci=vd->vi->codec_setup;

  int j,k,acc=0;
  int dim;
  int maxstage=0;
  look->info=info;

  look->parts=info->partitions;
  look->fullbooks=ci->fullbooks;
  look->phrasebook=ci->fullbooks+info->groupbook;
  dim=look->phrasebook->dim;

  look->partbooks=_ogg_calloc(look->parts,sizeof(*look->partbooks));

  for(j=0;j<look->parts;j++){
    int stages=ilog(info->secondstages[j]);
    if(stages){
      if(stages>maxstage)maxstage=stages;
      look->partbooks[j]=_ogg_calloc(stages,sizeof(*look->partbooks[j]));
      for(k=0;k<stages;k++)
	if(info->secondstages[j]&(1<<k)){
	  look->partbooks[j][k]=ci->fullbooks+info->booklist[acc++];
#ifdef TRAIN_RES
	  look->training_data[k][j]=_ogg_calloc(look->partbooks[j][k]->entries,
					   sizeof(***look->training_data));
#endif
	}
    }
  }

  look->partvals=rint(pow((float)look->parts,(float)dim));
  look->stages=maxstage;
  look->decodemap=_ogg_malloc(look->partvals*sizeof(*look->decodemap));
  for(j=0;j<look->partvals;j++){
    long val=j;
    long mult=look->partvals/look->parts;
    look->decodemap[j]=_ogg_malloc(dim*sizeof(*look->decodemap[j]));
    for(k=0;k<dim;k++){
      long deco=val/mult;
      val-=deco*mult;
      mult/=look->parts;
      look->decodemap[j][k]=deco;
    }
  }
#if defined(TRAIN_RES) || defined (TRAIN_RESAUX)
  {
    static int train_seq=0;
    look->train_seq=train_seq++;
  }
#endif
  return(look);
}

/* break an abstraction and copy some code for performance purposes */
static int local_book_besterror(codebook *book,float *a){
  int dim=book->dim,i,k,o;
  int best=0;
  encode_aux_threshmatch *tt=book->c->thresh_tree;

  /* find the quant val of each scalar */
  for(k=0,o=dim;k<dim;++k){
    float val=a[--o];
    i=tt->threshvals>>1;

    if(val<tt->quantthresh[i]){      
      if(val<tt->quantthresh[i-1]){
	for(--i;i>0;--i)
	  if(val>=tt->quantthresh[i-1])
	    break;
      }
    }else{
      
      for(++i;i<tt->threshvals-1;++i)
	if(val<tt->quantthresh[i])break;
      
    }

    best=(best*tt->quantvals)+tt->quantmap[i];
  }
  /* regular lattices are easy :-) */
  
  if(book->c->lengthlist[best]<=0){
    const static_codebook *c=book->c;
    int i,j;
    float bestf=0.f;
    float *e=book->valuelist;
    best=-1;
    for(i=0;i<book->entries;i++){
      if(c->lengthlist[i]>0){
	float this=0.f;
	for(j=0;j<dim;j++){
	  float val=(e[j]-a[j]);
	  this+=val*val;
	}
	if(best==-1 || this<bestf){
	  bestf=this;
	  best=i;
	}
      }
      e+=dim;
    }
  }

  {
#ifdef __SSE__												/* SSE Optimize */
	switch(dim)
	{
		case 2 :
			{
				float *ptr	 = book->valuelist+best*2;
				__m128	XMM0	 = _mm_load_ss(a  );
				__m128	XMM1	 = _mm_load_ss(a+1);
				XMM0	 = _mm_sub_ss(XMM0, PM128(ptr  ));
				XMM1	 = _mm_sub_ss(XMM1, PM128(ptr+1));
				_mm_store_ss(a  , XMM0);
				_mm_store_ss(a+1, XMM1);
			}
			break;
		case 4 :
			{
				float *ptr	 = book->valuelist+best*4;
				__m128	XMM0;
				XMM0	 = _mm_load_ps(a  );
				XMM0	 = _mm_sub_ps(XMM0, PM128(ptr  ));
				_mm_store_ps(a  , XMM0);
			}
			break;
		case 8 :
			{
				float *ptr	 = book->valuelist+best*8;
				__m128	XMM0, XMM1;
				XMM0	 = _mm_load_ps(a  );
				XMM1	 = _mm_load_ps(a+4);
				XMM0	 = _mm_sub_ps(XMM0, PM128(ptr  ));
				XMM1	 = _mm_sub_ps(XMM1, PM128(ptr+4));
				_mm_store_ps(a  , XMM0);
				_mm_store_ps(a+4, XMM1);
			}
			break;
		default :
			{
				float *ptr	 = book->valuelist+best*dim;
				for(i=0;i<dim;i++)
					*a++	-= *ptr++;
			}
			break;
	}
#else														/* SSE Optimize */
    float *ptr=book->valuelist+best*dim;
    for(i=0;i<dim;i++)
      *a++ -= *ptr++;
#endif														/* SSE Optimize */
  }

  return(best);
}

#ifdef __SSE__												/* SSE Optimize */
STIN int local_book_besterror_dim1x4(codebook *book,float *a,oggpack_buffer *opb, int* ia)
{
	int bits;
	encode_aux_threshmatch *tt	 = book->c->thresh_tree;
	__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
	int ctrl0, ctrl1, ctrl2, ctrl3;
	ctrl0	 = ia[0];
	ctrl1	 = ia[1];
	ctrl2	 = ia[2];
	ctrl3	 = ia[3];
	XMM0	 = _mm_load_ss(a  );
	XMM1	 = _mm_load_ss(a+1);
	XMM2	 = _mm_load_ss(a+2);
	XMM3	 = _mm_load_ss(a+3);
	ctrl0	 = tt->quantmap[ctrl0];
	ctrl1	 = tt->quantmap[ctrl1];
	ctrl2	 = tt->quantmap[ctrl2];
	ctrl3	 = tt->quantmap[ctrl3];
	if(book->c->lengthlist[ctrl0]<=0)
	{
		const static_codebook *c	 = book->c;
		int i;
		float bestf	 = 0.f;
		float *e	 = book->valuelist;
		ctrl0	 = -1;
		for(i=0;i<book->entries;i++)
		{
			if(c->lengthlist[i]>0)
			{
				float this;
				{
					float val	 = (e[0]-a[0]);
					this		 = val*val;
				}
				if(ctrl0==-1 || this<bestf)
				{
					bestf	 = this;
					ctrl0	 = i;
				}
			}
			e	++;
		}
	}
	if(book->c->lengthlist[ctrl1]<=0)
	{
		const static_codebook *c	 = book->c;
		int i;
		float bestf	 = 0.f;
		float *e	 = book->valuelist;
		ctrl1	 = -1;
		for(i=0;i<book->entries;i++)
		{
			if(c->lengthlist[i]>0)
			{
				float this;
				{
					float val	 = (e[0]-a[1]);
					this		 = val*val;
				}
				if(ctrl1==-1 || this<bestf)
				{
					bestf	 = this;
					ctrl1	 = i;
				}
			}
			e	++;
		}
	}
	if(book->c->lengthlist[ctrl2]<=0)
	{
		const static_codebook *c	 = book->c;
		int i;
		float bestf	 = 0.f;
		float *e	 = book->valuelist;
		ctrl2	 = -1;
		for(i=0;i<book->entries;i++)
		{
			if(c->lengthlist[i]>0)
			{
				float this;
				{
					float val	 = (e[0]-a[2]);
					this		 = val*val;
				}
				if(ctrl2==-1 || this<bestf)
				{
					bestf	 = this;
					ctrl2	 = i;
				}
			}
			e	++;
		}
	}
	if(book->c->lengthlist[ctrl3]<=0)
	{
		const static_codebook *c	 = book->c;
		int i;
		float bestf	 = 0.f;
		float *e	 = book->valuelist;
		ctrl3	 = -1;
		for(i=0;i<book->entries;i++)
		{
			if(c->lengthlist[i]>0)
			{
				float this;
				{
					float val	 = (e[0]-a[3]);
					this		 = val*val;
				}
				if(ctrl3==-1 || this<bestf)
				{
					bestf	 = this;
					ctrl3	 = i;
				}
			}
			e	++;
		}
	}
	XMM4	 = _mm_load_ss(book->valuelist+ctrl0);
	XMM5	 = _mm_load_ss(book->valuelist+ctrl1);
	XMM6	 = _mm_load_ss(book->valuelist+ctrl2);
	XMM7	 = _mm_load_ss(book->valuelist+ctrl3);
	XMM0	 = _mm_sub_ss(XMM0, XMM4);
	XMM1	 = _mm_sub_ss(XMM1, XMM5);
	XMM2	 = _mm_sub_ss(XMM2, XMM6);
	XMM3	 = _mm_sub_ss(XMM3, XMM7);
	_mm_store_ss(a  , XMM0);
	_mm_store_ss(a+1, XMM1);
	_mm_store_ss(a+2, XMM2);
	_mm_store_ss(a+3, XMM3);
	bits		 = vorbis_book_encode(book, ctrl0, opb);
	bits		+= vorbis_book_encode(book, ctrl1, opb);
	bits		+= vorbis_book_encode(book, ctrl2, opb);
	bits		+= vorbis_book_encode(book, ctrl3, opb);
	return(bits);
}

STIN int local_book_besterror_dim2x2(codebook *book,float *a,oggpack_buffer *opb, int* ia)
{
	int bits;
	encode_aux_threshmatch *tt	 = book->c->thresh_tree;
	__m128	XMM0, XMM1;
	int ctrl0, ctrl1, ctrl2, ctrl3;
	ctrl0	 = ia[1];
	ctrl1	 = ia[0];
	ctrl2	 = ia[3];
	ctrl3	 = ia[2];
	XMM0	 = _mm_load_ps(a);
	ctrl0	 = tt->quantmap[ctrl0];
	ctrl1	 = tt->quantmap[ctrl1];
	ctrl2	 = tt->quantmap[ctrl2];
	ctrl3	 = tt->quantmap[ctrl3];
	ctrl0	 = ctrl0*tt->quantvals+ctrl1;
	ctrl2	 = ctrl2*tt->quantvals+ctrl3;

	if(book->c->lengthlist[ctrl0]<=0)
	{
		const static_codebook *c	 = book->c;
		int i,j;
		float bestf	 = 0.f;
		float *e	 = book->valuelist;
		ctrl0	 = -1;
		for(i=0;i<book->entries;i++)
		{
			if(c->lengthlist[i]>0)
			{
				float this	 = 0.f;
				for(j=0;j<2;j++)
				{
					float val	 = (e[j]-a[j]);
					this		+= val*val;
				}
				if(ctrl0==-1 || this<bestf)
				{
					bestf	 = this;
					ctrl0	 = i;
				}
			}
			e	+= 2;
		}
	}
	if(book->c->lengthlist[ctrl2]<=0)
	{
		const static_codebook *c	 = book->c;
		int i,j;
		float bestf	 = 0.f;
		float *e	 = book->valuelist;
		ctrl2	 = -1;
		for(i=0;i<book->entries;i++)
		{
			if(c->lengthlist[i]>0)
			{
				float this	 = 0.f;
				for(j=0;j<2;j++)
				{
					float val	 = (e[j]-a[j+2]);
					this		+= val*val;
				}
				if(ctrl2==-1 || this<bestf)
				{
					bestf	 = this;
					ctrl2	 = i;
				}
			}
			e	+= 2;
		}
	}
#pragma warning(disable : 592)
	XMM1	 = _mm_loadl_pi(XMM1, (__m64*)(book->valuelist+ctrl0*2));
#pragma warning(default : 592)
	XMM1	 = _mm_loadh_pi(XMM1, (__m64*)(book->valuelist+ctrl2*2));
	XMM0	 = _mm_sub_ps(XMM0, XMM1);
	_mm_store_ps(a, XMM0);
	bits		 = vorbis_book_encode(book, ctrl0, opb);
	bits		+= vorbis_book_encode(book, ctrl2, opb);
	return(bits);
}

STIN int local_book_besterror_dim4(codebook *book,float *a, int* ia)
{
	encode_aux_threshmatch *tt	 = book->c->thresh_tree;
	__m128	XMM0;
	int ctrl0, ctrl1, ctrl2, ctrl3;
	ctrl0	 = ia[3];
	ctrl1	 = ia[2];
	ctrl2	 = ia[1];
	ctrl3	 = ia[0];
	XMM0	 = _mm_load_ps(a  );
	ctrl0	 = tt->quantmap[ctrl0];
	ctrl1	 = tt->quantmap[ctrl1];
	ctrl2	 = tt->quantmap[ctrl2];
	ctrl3	 = tt->quantmap[ctrl3];
	ctrl0	 = ctrl0 *tt->quantvals+ctrl1;
	ctrl0	 = ctrl0 *tt->quantvals+ctrl2;
	ctrl0	 = ctrl0 *tt->quantvals+ctrl3;

	if(book->c->lengthlist[ctrl0]<=0)
	{
		const static_codebook *c	 = book->c;
		int i;
		float bestf	 = 0.f;
		float *e	 = book->valuelist;
		ctrl0	 = -1;
		for(i=0;i<book->entries;i++)
		{
			if(c->lengthlist[i]>0)
			{
				float this;
				__m128	PVAL	 = _mm_load_ps(e);
				PVAL	 = _mm_sub_ps(PVAL, XMM0);
				PVAL	 = _mm_mul_ps(PVAL, PVAL);
				this	 = _mm_add_horz(PVAL);
				if(ctrl0==-1 || this<bestf)
				{
					bestf	 = this;
					ctrl0	 = i;
				}
			}
			e	+= 4;
		}
	}
	XMM0	 = _mm_sub_ps(XMM0, PM128(book->valuelist+ctrl0*4));
	_mm_store_ps(a, XMM0);
	return(ctrl0);
}

STIN int local_book_besterror_dim8(codebook *book,float *a, int* ia)
{
	int best;
	encode_aux_threshmatch *tt	 = book->c->thresh_tree;
	__m128	XMM0, XMM1;
	int ctrl0, ctrl1, ctrl2, ctrl3;
	XMM0	 = _mm_load_ps(a  );
	ctrl0	 = ia[7];
	ctrl1	 = ia[6];
	ctrl2	 = ia[5];
	ctrl3	 = ia[4];
	ctrl0	 = tt->quantmap[ctrl0];
	ctrl1	 = tt->quantmap[ctrl1];
	ctrl2	 = tt->quantmap[ctrl2];
	ctrl3	 = tt->quantmap[ctrl3];
	best	 = ctrl0*tt->quantvals+ctrl1;
	best	 = best *tt->quantvals+ctrl2;
	best	 = best *tt->quantvals+ctrl3;
	XMM1	 = _mm_load_ps(a+4);
	ctrl0	 = ia[3];
	ctrl1	 = ia[2];
	ctrl2	 = ia[1];
	ctrl3	 = ia[0];
	ctrl0	 = tt->quantmap[ctrl0];
	ctrl1	 = tt->quantmap[ctrl1];
	ctrl2	 = tt->quantmap[ctrl2];
	ctrl3	 = tt->quantmap[ctrl3];
	best	 = best *tt->quantvals+ctrl0;
	best	 = best *tt->quantvals+ctrl1;
	best	 = best *tt->quantvals+ctrl2;
	best	 = best *tt->quantvals+ctrl3;

	if(book->c->lengthlist[best]<=0)
	{
		const static_codebook *c	 = book->c;
		int i;
		float bestf	 = 0.f;
		float *e	 = book->valuelist;
		best	 = -1;
		for(i=0;i<book->entries;i++)
		{
			if(c->lengthlist[i]>0)
			{
				float this;
				__m128	PVAL0	 = _mm_load_ps(e  );
				__m128	PVAL1	 = _mm_load_ps(e+4);
				PVAL0	 = _mm_sub_ps(PVAL0, PM128(a  ));
				PVAL1	 = _mm_sub_ps(PVAL1, PM128(a+4));
				PVAL0	 = _mm_mul_ps(PVAL0, PVAL0);
				PVAL1	 = _mm_mul_ps(PVAL1, PVAL1);
				PVAL0	 = _mm_add_ps(PVAL0, PVAL1);
				this	 = _mm_add_horz(PVAL0);
				if(best==-1 || this<bestf)
				{
					bestf	 = this;
					best	 = i;
				}
			}
			e	+= 8;
		}
	}
	XMM0	 = _mm_sub_ps(XMM0, PM128(book->valuelist+best*8  ));
	XMM1	 = _mm_sub_ps(XMM1, PM128(book->valuelist+best*8+4));
	_mm_store_ps(a  , XMM0);
	_mm_store_ps(a+4, XMM1);
	return(best);
}
#endif														/* SSE Optimize */

static int _encodepart(oggpack_buffer *opb,float *vec, int n,
		       codebook *book,long *acc){
  int i,bits=0;
  int dim=book->dim;
#ifdef __SSE__												/* SSE Optimize */
	int*	TEMP	 = (int*)_ogg_alloca(sizeof(int)*n);
	__m128	PMIN	 = _mm_set1_ps(-(float)(book->c->thresh_tree->threshvals>>1));
	__m128	PMAX	 = _mm_set1_ps( (float)(book->c->thresh_tree->threshvals>>1));

	if(dim<=8)
	{
		if(book->c->thresh_tree->quantthresh[0]==-(float)(book->c->thresh_tree->threshvals>>1)+.5f)
		{
#if	defined(__SSE2__)
			for(i=0;i<n;i+=16)
			{
				__m128	XMM0	 = _mm_load_ps(vec+i   );
				__m128	XMM1	 = _mm_load_ps(vec+i+ 4);
				__m128	XMM2	 = _mm_load_ps(vec+i+ 8);
				__m128	XMM3	 = _mm_load_ps(vec+i+12);
				XMM0	 = _mm_min_ps(XMM0, PMAX);
				XMM1	 = _mm_min_ps(XMM1, PMAX);
				XMM2	 = _mm_min_ps(XMM2, PMAX);
				XMM3	 = _mm_min_ps(XMM3, PMAX);
				XMM0	 = _mm_max_ps(XMM0, PMIN);
				XMM1	 = _mm_max_ps(XMM1, PMIN);
				XMM2	 = _mm_max_ps(XMM2, PMIN);
				XMM3	 = _mm_max_ps(XMM3, PMIN);
				XMM0	 = _mm_add_ps(XMM0, PMAX);
				XMM1	 = _mm_add_ps(XMM1, PMAX);
				XMM2	 = _mm_add_ps(XMM2, PMAX);
				XMM3	 = _mm_add_ps(XMM3, PMAX);
				XMM0	 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM0));
				XMM1	 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM1));
				XMM2	 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM2));
				XMM3	 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM3));
				_mm_store_ps((__m128*)(TEMP+i   ), XMM0);
				_mm_store_ps((__m128*)(TEMP+i+ 4), XMM1);
				_mm_store_ps((__m128*)(TEMP+i+ 8), XMM2);
				_mm_store_ps((__m128*)(TEMP+i+12), XMM3);
			}
#else
			for(i=0;i<n;i+=16)
			{
				__m64	MM0, MM1, MM2, MM3, MM4, MM5, MM6, MM7;
				__m128	XMM0	 = _mm_load_ps(vec+i   );
				__m128	XMM1	 = _mm_load_ps(vec+i+ 4);
				__m128	XMM2	 = _mm_load_ps(vec+i+ 8);
				__m128	XMM3	 = _mm_load_ps(vec+i+12);
				XMM0	 = _mm_min_ps(XMM0, PMAX);
				XMM1	 = _mm_min_ps(XMM1, PMAX);
				XMM2	 = _mm_min_ps(XMM2, PMAX);
				XMM3	 = _mm_min_ps(XMM3, PMAX);
				XMM0	 = _mm_max_ps(XMM0, PMIN);
				XMM1	 = _mm_max_ps(XMM1, PMIN);
				XMM2	 = _mm_max_ps(XMM2, PMIN);
				XMM3	 = _mm_max_ps(XMM3, PMIN);
				XMM0	 = _mm_add_ps(XMM0, PMAX);
				XMM1	 = _mm_add_ps(XMM1, PMAX);
				XMM2	 = _mm_add_ps(XMM2, PMAX);
				XMM3	 = _mm_add_ps(XMM3, PMAX);
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
				PM64(TEMP+i   )	 = MM0;
				PM64(TEMP+i+ 4)	 = MM2;
				PM64(TEMP+i+ 8)	 = MM4;
				PM64(TEMP+i+12)	 = MM6;
				PM64(TEMP+i+ 2)	 = MM1;
				PM64(TEMP+i+ 6)	 = MM3;
				PM64(TEMP+i+10)	 = MM5;
				PM64(TEMP+i+14)	 = MM7;
			}
			_mm_empty();
#endif
		}
		else
		{
			__m128	PM	 = _mm_set1_ps(1.f/(
				(float)(book->c->thresh_tree->quantthresh[1]-book->c->thresh_tree->quantthresh[0])
					-1.0e-04));	/* for control of round */
			for(i=0;i<n;i+=16)
			{
#if	!defined(__SSE2__)
				__m64	MM0, MM1, MM2, MM3, MM4, MM5, MM6, MM7;
#endif
				__m128	XMM0, XMM1, XMM2, XMM3;
				XMM0	 = _mm_load_ps(vec+i   );
				XMM1	 = _mm_load_ps(vec+i+ 4);
				XMM2	 = _mm_load_ps(vec+i+ 8);
				XMM3	 = _mm_load_ps(vec+i+12);
				XMM0	 = _mm_mul_ps(XMM0, PM);
				XMM1	 = _mm_mul_ps(XMM1, PM);
				XMM2	 = _mm_mul_ps(XMM2, PM);
				XMM3	 = _mm_mul_ps(XMM3, PM);
				XMM0	 = _mm_max_ps(XMM0, PMIN);
				XMM1	 = _mm_max_ps(XMM1, PMIN);
				XMM2	 = _mm_max_ps(XMM2, PMIN);
				XMM3	 = _mm_max_ps(XMM3, PMIN);
				XMM0	 = _mm_min_ps(XMM0, PMAX);
				XMM1	 = _mm_min_ps(XMM1, PMAX);
				XMM2	 = _mm_min_ps(XMM2, PMAX);
				XMM3	 = _mm_min_ps(XMM3, PMAX);
				XMM0	 = _mm_add_ps(XMM0, PMAX);
				XMM1	 = _mm_add_ps(XMM1, PMAX);
				XMM2	 = _mm_add_ps(XMM2, PMAX);
				XMM3	 = _mm_add_ps(XMM3, PMAX);
#if	defined(__SSE2__)
				XMM0	 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM0));
				XMM1	 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM1));
				XMM2	 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM2));
				XMM3	 = _mm_castsi128_ps(_mm_cvtps_epi32(XMM3));
				_mm_store_ps((__m128*)(TEMP+i   ), XMM0);
				_mm_store_ps((__m128*)(TEMP+i+ 4), XMM1);
				_mm_store_ps((__m128*)(TEMP+i+ 8), XMM2);
				_mm_store_ps((__m128*)(TEMP+i+12), XMM3);
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
				PM64(TEMP+i   )	 = MM0;
				PM64(TEMP+i+ 4)	 = MM2;
				PM64(TEMP+i+ 8)	 = MM4;
				PM64(TEMP+i+12)	 = MM6;
				PM64(TEMP+i+ 2)	 = MM1;
				PM64(TEMP+i+ 6)	 = MM3;
				PM64(TEMP+i+10)	 = MM5;
				PM64(TEMP+i+14)	 = MM7;
			}
			_mm_empty();
#endif
		}
		switch(dim)
		{
			case 1:
				for(i=0;i<n;i+=4)
				{
					bits	+= local_book_besterror_dim1x4(book,vec+i, opb, TEMP+i);
				}
				break;
			case 2:
				for(i=0;i<n;i+=4)
				{
					bits	+= local_book_besterror_dim2x2(book,vec+i, opb, TEMP+i);
				}
				break;
			case 4:
				for(i=0;i<n;i+=4)
				{
					int entry	 = local_book_besterror_dim4(book,vec+i, TEMP+i);
					bits		+= vorbis_book_encode(book,entry,opb);
				}
				break;
			case 8:
				for(i=0;i<n;i+=8)
				{
					int entry	 = local_book_besterror_dim8(book,vec+i, TEMP+i);
					bits		+= vorbis_book_encode(book,entry,opb);
				}
				break;
			default:
				break;
		}
	}
	else
	{
#endif														/* SSE Optimize */
  int step=n/dim;

  for(i=0;i<step;i++){
    int entry=local_book_besterror(book,vec+i*dim);

#ifdef TRAIN_RES
    acc[entry]++;
#endif

    bits+=vorbis_book_encode(book,entry,opb);
  }
#if	defined(__SSE__)										/* SSE Optimize */
	}
#endif														/* SSE Optimize */

  return(bits);
}

static long **_01class(vorbis_block *vb,vorbis_look_residue *vl,
		       float **in,int ch){
  long i,j,k;
  vorbis_look_residue0 *look=(vorbis_look_residue0 *)vl;
  vorbis_info_residue0 *info=look->info;
  vorbis_info           *vi=vb->vd->vi;
  codec_setup_info      *ci=vi->codec_setup;

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int possible_partitions=info->partitions;
  int n=info->end-info->begin;
  
  int partvals=n/samples_per_partition;
  long **partword=_vorbis_block_alloc(vb,ch*sizeof(*partword));
  float scale=100./samples_per_partition;

  /* we find the partition type for each partition of each
     channel.  We'll go back and do the interleaved encoding in a
     bit.  For now, clarity */
 
  for(i=0;i<ch;i++){
    partword[i]=_vorbis_block_alloc(vb,n/samples_per_partition*sizeof(*partword[i]));
    memset(partword[i],0,n/samples_per_partition*sizeof(*partword[i]));
  }
  
  for(i=0;i<partvals;i++){
    int offset=i*samples_per_partition+info->begin;
    for(j=0;j<ch;j++){
      float max=0.;
      float ent=0.;
      for(k=0;k<samples_per_partition;k++){
	if(fabs(in[j][offset+k])>max)max=fabs(in[j][offset+k]);
	ent+=fabs(rint(in[j][offset+k]));
      }
      ent*=scale;
      
      for(k=0;k<possible_partitions-1;k++)
	if(max<=info->classmetric1[k] &&
	   (info->classmetric2[k]<0 || (int)ent<info->classmetric2[k]))
	  break;
      
      partword[j][i]=k;  
    }
  }

#ifdef TRAIN_RESAUX
  {
    FILE *of;
    char buffer[80];
  
    for(i=0;i<ch;i++){
      sprintf(buffer,"resaux_%d.vqd",look->train_seq);
      of=fopen(buffer,"a");
      for(j=0;j<partvals;j++)
	fprintf(of,"%ld, ",partword[i][j]);
      fprintf(of,"\n");
      fclose(of);
    }
  }
#endif
  look->frames++;

  return(partword);
}

/* designed for stereo or other modes where the partition size is an
   integer multiple of the number of channels encoded in the current
   submap */
static long **_2class(vorbis_block *vb,vorbis_look_residue *vl,float **in,
		      int ch){
  long i,j,k,l;
  vorbis_look_residue0 *look=(vorbis_look_residue0 *)vl;
  vorbis_info_residue0 *info=look->info;

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int possible_partitions=info->partitions;
  int n=info->end-info->begin;

  int partvals=n/samples_per_partition;
  long **partword=_vorbis_block_alloc(vb,sizeof(*partword));

#if defined(TRAIN_RES) || defined (TRAIN_RESAUX)
  FILE *of;
  char buffer[80];
#endif
  
  partword[0]=_vorbis_block_alloc(vb,n*ch/samples_per_partition*sizeof(*partword[0]));
#ifdef __SSE__												/* SSE Optimize */
	{
		int	pn	 = n*ch/samples_per_partition;
		__m128	XMM0	 = _mm_setzero_ps();
		int tn;
		float *d = (float*)(partword[0]);
		tn	 = pn&(~31);
		for(i=0;i<tn;i+=32)
		{
			_mm_store_ps(d+i   , XMM0);
			_mm_store_ps(d+i+ 4, XMM0);
			_mm_store_ps(d+i+ 8, XMM0);
			_mm_store_ps(d+i+12, XMM0);
			_mm_store_ps(d+i+16, XMM0);
			_mm_store_ps(d+i+20, XMM0);
			_mm_store_ps(d+i+24, XMM0);
			_mm_store_ps(d+i+28, XMM0);
		}
		tn	 = pn&(~15);
		for(;i<tn;i+=16)
		{
			_mm_store_ps(d+i   , XMM0);
			_mm_store_ps(d+i+ 4, XMM0);
			_mm_store_ps(d+i+ 8, XMM0);
			_mm_store_ps(d+i+12, XMM0);
		}
		tn	 = pn&(~7);
		for(;i<tn;i+=8)
		{
			_mm_store_ps(d+i   , XMM0);
			_mm_store_ps(d+i+ 4, XMM0);
		}
		tn	 = pn&(~3);
		for(;i<tn;i+=4)
		{
			_mm_store_ps(d+i   , XMM0);
		}
		for(;i<pn;i++)
		{
			*(d+i   )	 = 0;
		}
	}
#else														/* SSE Optimize */
  memset(partword[0],0,n*ch/samples_per_partition*sizeof(*partword[0]));
#endif														/* SSE Optimize */

  for(i=0,l=info->begin/ch;i<partvals;i++){
    float magmax=0.f;
    float angmax=0.f;
#ifdef __SSE__												/* SSE Optimize */
	if(ch==2&&possible_partitions==10)
	{
		register __m128 PMAGMAX	 = _mm_setzero_ps();
		register __m128 PANGMAX	 = _mm_setzero_ps();
		float	*pin0	 = in[0];
		float	*pin1	 = in[1];
		
		for(j=0;j<samples_per_partition;j+=16)
		{
			__m128	XMM0	 = _mm_load_ps(pin0+l  );
			__m128	XMM1	 = _mm_load_ps(pin1+l  );
			__m128	XMM2	 = _mm_load_ps(pin0+l+4);
			__m128	XMM3	 = _mm_load_ps(pin1+l+4);
			XMM0	 = _mm_and_ps(XMM0, PM128(PABSMASK));
			XMM1	 = _mm_and_ps(XMM1, PM128(PABSMASK));
			XMM2	 = _mm_and_ps(XMM2, PM128(PABSMASK));
			XMM3	 = _mm_and_ps(XMM3, PM128(PABSMASK));
			PMAGMAX	 = _mm_max_ps(PMAGMAX, XMM0);
			PANGMAX	 = _mm_max_ps(PANGMAX, XMM1);
			PMAGMAX	 = _mm_max_ps(PMAGMAX, XMM2);
			PANGMAX	 = _mm_max_ps(PANGMAX, XMM3);
			l	+= 8;
		}
		magmax	 = _mm_max_horz(PMAGMAX);
		angmax	 = _mm_max_horz(PANGMAX);
		PMAGMAX	 = _mm_set1_ps(magmax);
		PANGMAX	 = _mm_set1_ps(angmax);
		{
			static int jtable0[16]	 = 
			{
				 0, 1, 0, 2, 0, 1, 0, 3,
				 0, 1, 0, 2, 0, 1, 0,-1,
			};
			static int jtable1[16]	 = 
			{
				 4, 5, 4, 6, 4, 5, 4, 7,
				 4, 5, 4, 6, 5, 5, 4,-1,
			};
			__m128	XMM0	 = _mm_lddqu_ps(info->classmetric1);
			__m128	XMM1	 = _mm_lddqu_ps(info->classmetric2);
			XMM0	 = _mm_cmplt_ps(XMM0, PMAGMAX);
			XMM1	 = _mm_cmplt_ps(XMM1, PANGMAX);
			XMM0	 = _mm_or_ps(XMM0, XMM1);
			j	 = _mm_movemask_ps(XMM0);
			if(j!=15)
				j	 = jtable0[j];
			else
			{
				__m128	XMM0	 = _mm_lddqu_ps(info->classmetric1+4);
				__m128	XMM1	 = _mm_lddqu_ps(info->classmetric2+4);
				XMM0	 = _mm_cmplt_ps(XMM0, PMAGMAX);
				XMM1	 = _mm_cmplt_ps(XMM1, PANGMAX);
				XMM0	 = _mm_or_ps(XMM0, XMM1);
				j	 = _mm_movemask_ps(XMM0);
				if(j!=15)
					j	 = jtable1[j];
				else
				{
					if(magmax<=info->classmetric1[8] &&
						   angmax<=info->classmetric2[8])
						j	 = 8;
					else
						j	 = 9;
				}
			}
		}
	}
	else
	{
		for(j=0;j<samples_per_partition;j+=ch)
		{
			if(fabs(in[0][l])>magmax)
				magmax=fabs(in[0][l]);
			for(k=1;k<ch;k++)
				if(fabs(in[k][l])>angmax)
					angmax=fabs(in[k][l]);
				l++;
		}
		for(j=0;j<possible_partitions-1;j++)
			if(magmax<=info->classmetric1[j] &&
			   angmax<=info->classmetric2[j])
				break;
	}
#else														/* SSE Optimize */
    for(j=0;j<samples_per_partition;j+=ch){
      if(fabs(in[0][l])>magmax)magmax=fabs(in[0][l]);
      for(k=1;k<ch;k++)
	if(fabs(in[k][l])>angmax)angmax=fabs(in[k][l]);
      l++;
    }
    for(j=0;j<possible_partitions-1;j++)
      if(magmax<=info->classmetric1[j] &&
	 angmax<=info->classmetric2[j])
	break;
#endif														/* SSE Optimize */

    partword[0][i]=j;

  }  
  
#ifdef TRAIN_RESAUX
  sprintf(buffer,"resaux_%d.vqd",look->train_seq);
  of=fopen(buffer,"a");
  for(i=0;i<partvals;i++)
    fprintf(of,"%ld, ",partword[0][i]);
  fprintf(of,"\n");
  fclose(of);
#endif

  look->frames++;

  return(partword);
}

static int _01forward(oggpack_buffer *opb,
		      vorbis_block *vb,vorbis_look_residue *vl,
		      float **in,int ch,
		      long **partword,
		      int (*encode)(oggpack_buffer *,float *,int,
				    codebook *,long *)){
  long i,j,k,s;
  vorbis_look_residue0 *look=(vorbis_look_residue0 *)vl;
  vorbis_info_residue0 *info=look->info;

  vorbis_dsp_state      *vd=vb->vd;

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int possible_partitions=info->partitions;
  int partitions_per_word=look->phrasebook->dim;
  int n=info->end-info->begin;

  int partvals=n/samples_per_partition;
  long resbits[128];
  long resvals[128];

#ifdef TRAIN_RES
  for(i=0;i<ch;i++)
    for(j=info->begin;j<info->end;j++){
      if(in[i][j]>look->tmax)look->tmax=in[i][j];
      if(in[i][j]<look->tmin)look->tmin=in[i][j];
    }
#endif

  memset(resbits,0,sizeof(resbits));
  memset(resvals,0,sizeof(resvals));
  
  /* we code the partition words for each channel, then the residual
     words for a partition per channel until we've written all the
     residual words for that partition word.  Then write the next
     partition channel words... */

  for(s=0;s<look->stages;s++){

    for(i=0;i<partvals;){

      /* first we encode a partition codeword for each channel */
      if(s==0){
	for(j=0;j<ch;j++){
	  long val=partword[j][i];
	  for(k=1;k<partitions_per_word;k++){
	    val*=possible_partitions;
	    if(i+k<partvals)
	      val+=partword[j][i+k];
	  }	

	  /* training hack */
	  if(val<look->phrasebook->entries)
	    look->phrasebits+=vorbis_book_encode(look->phrasebook,val,opb);
#if 0 /*def TRAIN_RES*/
	  else
	    fprintf(stderr,"!");
#endif
	
	}
      }
      
      /* now we encode interleaved residual values for the partitions */
      for(k=0;k<partitions_per_word && i<partvals;k++,i++){
	long offset=i*samples_per_partition+info->begin;
	
	for(j=0;j<ch;j++){
	  if(s==0)resvals[partword[j][i]]+=samples_per_partition;
	  if(info->secondstages[partword[j][i]]&(1<<s)){
	    codebook *statebook=look->partbooks[partword[j][i]][s];
	    if(statebook){
	      int ret;
	      long *accumulator=NULL;

#ifdef TRAIN_RES
	      accumulator=look->training_data[s][partword[j][i]];
	      {
		int l;
		float *samples=in[j]+offset;
		for(l=0;l<samples_per_partition;l++){
		  if(samples[l]<look->training_min[s][partword[j][i]])
		    look->training_min[s][partword[j][i]]=samples[l];
		  if(samples[l]>look->training_max[s][partword[j][i]])
		    look->training_max[s][partword[j][i]]=samples[l];
		}
	      }
#endif
	      
	      ret=encode(opb,in[j]+offset,samples_per_partition,
			 statebook,accumulator);

	      look->postbits+=ret;
	      resbits[partword[j][i]]+=ret;
	    }
	  }
	}
      }
    }
  }

  /*{
    long total=0;
    long totalbits=0;
    fprintf(stderr,"%d :: ",vb->mode);
    for(k=0;k<possible_partitions;k++){
      fprintf(stderr,"%ld/%1.2g, ",resvals[k],(float)resbits[k]/resvals[k]);
      total+=resvals[k];
      totalbits+=resbits[k];
      }
    
    fprintf(stderr,":: %ld:%1.2g\n",total,(double)totalbits/total);
    }*/
  return(0);
}

/* a truncated packet here just means 'stop working'; it's not an error */
static int _01inverse(vorbis_block *vb,vorbis_look_residue *vl,
		      float **in,int ch,
		      long (*decodepart)(codebook *, float *, 
					 oggpack_buffer *,int)){

  long i,j,k,l,s;
  vorbis_look_residue0 *look=(vorbis_look_residue0 *)vl;
  vorbis_info_residue0 *info=look->info;

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int partitions_per_word=look->phrasebook->dim;
  int n=info->end-info->begin;
  
  int partvals=n/samples_per_partition;
  int partwords=(partvals+partitions_per_word-1)/partitions_per_word;
  int ***partword=alloca(ch*sizeof(*partword));

  for(j=0;j<ch;j++)
    partword[j]=_vorbis_block_alloc(vb,partwords*sizeof(*partword[j]));

  for(s=0;s<look->stages;s++){

    /* each loop decodes on partition codeword containing 
       partitions_pre_word partitions */
    for(i=0,l=0;i<partvals;l++){
      if(s==0){
	/* fetch the partition word for each channel */
	for(j=0;j<ch;j++){
	  int temp=vorbis_book_decode(look->phrasebook,&vb->opb);
	  if(temp==-1)goto eopbreak;
	  partword[j][l]=look->decodemap[temp];
	  if(partword[j][l]==NULL)goto errout;
	}
      }
      
      /* now we decode residual values for the partitions */
      for(k=0;k<partitions_per_word && i<partvals;k++,i++)
	for(j=0;j<ch;j++){
	  long offset=info->begin+i*samples_per_partition;
	  if(info->secondstages[partword[j][l][k]]&(1<<s)){
	    codebook *stagebook=look->partbooks[partword[j][l][k]][s];
	    if(stagebook){
	      if(decodepart(stagebook,in[j]+offset,&vb->opb,
			    samples_per_partition)==-1)goto eopbreak;
	    }
	  }
	}
    } 
  }
  
 errout:
 eopbreak:
  return(0);
}

#if 0
/* residue 0 and 1 are just slight variants of one another. 0 is
   interleaved, 1 is not */
long **res0_class(vorbis_block *vb,vorbis_look_residue *vl,
		  float **in,int *nonzero,int ch){
  /* we encode only the nonzero parts of a bundle */
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])
      in[used++]=in[i];
  if(used)
    /*return(_01class(vb,vl,in,used,_interleaved_testhack));*/
    return(_01class(vb,vl,in,used));
  else
    return(0);
}

int res0_forward(vorbis_block *vb,vorbis_look_residue *vl,
		 float **in,float **out,int *nonzero,int ch,
		 long **partword){
  /* we encode only the nonzero parts of a bundle */
  int i,j,used=0,n=vb->pcmend/2;
  for(i=0;i<ch;i++)
    if(nonzero[i]){
      if(out)
	for(j=0;j<n;j++)
	  out[i][j]+=in[i][j];
      in[used++]=in[i];
    }
  if(used){
    int ret=_01forward(vb,vl,in,used,partword,
		      _interleaved_encodepart);
    if(out){
      used=0;
      for(i=0;i<ch;i++)
	if(nonzero[i]){
	  for(j=0;j<n;j++)
	    out[i][j]-=in[used][j];
	  used++;
	}
    }
    return(ret);
  }else{
    return(0);
  }
}
#endif

int res0_inverse(vorbis_block *vb,vorbis_look_residue *vl,
		 float **in,int *nonzero,int ch){
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])
      in[used++]=in[i];
  if(used)
    return(_01inverse(vb,vl,in,used,vorbis_book_decodevs_add));
  else
    return(0);
}

int res1_forward(oggpack_buffer *opb,vorbis_block *vb,vorbis_look_residue *vl,
		 float **in,float **out,int *nonzero,int ch,
		 long **partword){
  int i,j,used=0,n=vb->pcmend/2;
  for(i=0;i<ch;i++)
    if(nonzero[i]){
      if(out)
#ifdef __SSE__												/* SSE Optimize */
	{
		float	*pin	 = in[i];
		float	*pout	 = out[i];
		for(j=0;j<n;j+=8)
		{
			__m128	XMM0	 = _mm_load_ps(pout+j  );
			__m128	XMM2	 = _mm_load_ps(pin+j  );
			__m128	XMM1	 = _mm_load_ps(pout+j+4);
			__m128	XMM3	 = _mm_load_ps(pin+j+4);
			XMM0	 = _mm_add_ps(XMM0, XMM2);
			XMM1	 = _mm_add_ps(XMM1, XMM3);
			_mm_store_ps(pout+j  , XMM0);
			_mm_store_ps(pout+j+4, XMM1);
				
		}
	}
#else														/* SSE Optimize */
	for(j=0;j<n;j++)
	  out[i][j]+=in[i][j];
#endif														/* SSE Optimize */
      in[used++]=in[i];
    }

  if(used){
    int ret=_01forward(opb,vb,vl,in,used,partword,_encodepart);
    if(out){
      used=0;
      for(i=0;i<ch;i++)
	if(nonzero[i]){
#ifdef __SSE__												/* SSE Optimize */
		{
			float	*pin	 = in[i];
			float	*pout	 = out[used];
			for(j=0;j<n;j+=8)
			{
				__m128	XMM0	 = _mm_load_ps(pout+j  );
				__m128	XMM2	 = _mm_load_ps(pin+j  );
				__m128	XMM1	 = _mm_load_ps(pout+j+4);
				__m128	XMM3	 = _mm_load_ps(pin+j+4);
				XMM0	 = _mm_sub_ps(XMM0, XMM2);
				XMM1	 = _mm_sub_ps(XMM1, XMM3);
				_mm_store_ps(pout+j  , XMM0);
				_mm_store_ps(pout+j+4, XMM1);
			}
		}
#else														/* SSE Optimize */
	  for(j=0;j<n;j++)
	    out[i][j]-=in[used][j];
#endif														/* SSE Optimize */
	  used++;
	}
    }
    return(ret);
  }else{
    return(0);
  }
}

long **res1_class(vorbis_block *vb,vorbis_look_residue *vl,
		  float **in,int *nonzero,int ch){
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])
      in[used++]=in[i];
  if(used)
    return(_01class(vb,vl,in,used));
  else
    return(0);
}

int res1_inverse(vorbis_block *vb,vorbis_look_residue *vl,
		 float **in,int *nonzero,int ch){
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])
      in[used++]=in[i];
  if(used)
    return(_01inverse(vb,vl,in,used,vorbis_book_decodev_add));
  else
    return(0);
}

long **res2_class(vorbis_block *vb,vorbis_look_residue *vl,
		  float **in,int *nonzero,int ch){
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])used++;
  if(used)
    return(_2class(vb,vl,in,ch));
  else
    return(0);
}

/* res2 is slightly more different; all the channels are interleaved
   into a single vector and encoded. */

int res2_forward(oggpack_buffer *opb,
		 vorbis_block *vb,vorbis_look_residue *vl,
		 float **in,float **out,int *nonzero,int ch,
		 long **partword){
  long i,j,k,n=vb->pcmend/2,used=0;

  /* don't duplicate the code; use a working vector hack for now and
     reshape ourselves into a single channel res1 */
  /* ugly; reallocs for each coupling pass :-( */
  float *work=_vorbis_block_alloc(vb,ch*n*sizeof(*work));
#ifdef __SSE__												/* SSE Optimize */
  for(i=0;i<ch;i++){
    if(nonzero[i])used++;
  }
  if(ch==2)
  {
	float *pcm0=in[0];
	float *pcm1=in[1];
	for(j=0;j<n;j+=16)
	{
		// ABCD    ABEF    AEBF
		// EFGH -> CDGH -> CGDH
		__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
		XMM0	 = _mm_load_ps(pcm0+j   );
		XMM2	 = _mm_load_ps(pcm0+j+ 4);
		XMM4	 = _mm_load_ps(pcm1+j   );
		XMM5	 = _mm_load_ps(pcm1+j+ 4);
		XMM1	 = XMM0;
		XMM3	 = XMM2;
		XMM0	 = _mm_unpacklo_ps(XMM0, XMM4);
		XMM1	 = _mm_unpackhi_ps(XMM1, XMM4);
		XMM6	 = _mm_load_ps(pcm0+j+ 8);
		XMM7	 = _mm_load_ps(pcm0+j+12);
		XMM2	 = _mm_unpacklo_ps(XMM2, XMM5);
		XMM3	 = _mm_unpackhi_ps(XMM3, XMM5);
		XMM4	 = _mm_load_ps(pcm1+j+ 8);
		XMM5	 = _mm_load_ps(pcm1+j+12);
		_mm_store_ps(work+j*2   , XMM0);
		_mm_store_ps(work+j*2+ 4, XMM1);
		XMM1	 = XMM6;
		_mm_store_ps(work+j*2+ 8, XMM2);
		_mm_store_ps(work+j*2+12, XMM3);
		XMM3	 = XMM7;
		XMM6	 = _mm_unpacklo_ps(XMM6, XMM4);
		XMM1	 = _mm_unpackhi_ps(XMM1, XMM4);
		XMM7	 = _mm_unpacklo_ps(XMM7, XMM5);
		XMM3	 = _mm_unpackhi_ps(XMM3, XMM5);
		_mm_store_ps(work+j*2+16, XMM6);
		_mm_store_ps(work+j*2+20, XMM1);
		_mm_store_ps(work+j*2+24, XMM7);
		_mm_store_ps(work+j*2+28, XMM3);
	}
  }
  else
  {
	for(i=0;i<ch;i++){
		float *pcm=in[i];
		for(j=0,k=i;j<n;j++,k+=ch)
			work[k]=pcm[j];
	}
  }
#else														/* SSE Optimize */
  for(i=0;i<ch;i++){
    float *pcm=in[i];
    if(nonzero[i])used++;
    for(j=0,k=i;j<n;j++,k+=ch)
      work[k]=pcm[j];
  }
#endif														/* SSE Optimize */
  
  if(used){
    int ret=_01forward(opb,vb,vl,&work,1,partword,_encodepart);
    /* update the sofar vector */
    if(out){
#ifdef __SSE__												/* SSE Optimize */
	if(ch==2)
	{
		float *pcm0		 = in[0];
		float *pcm1		 = in[1];
		float *sofar0	 = out[0];
		float *sofar1	 = out[1];
		for(j=0;j<n;j+=8)
		{
			__m128	XMM0, XMM1, XMM2, XMM3;
			__m128	XMM4, XMM5, XMM6, XMM7;
			XMM0	 = _mm_load_ps(work+j*2   );
			XMM4	 = _mm_load_ps(work+j*2+ 8);
			XMM6	 = _mm_load_ps(work+j*2+ 4);
			XMM7	 = _mm_load_ps(work+j*2+12);
			XMM1	 = XMM0;
			XMM5	 = XMM5;
			XMM0	 = _mm_shuffle_ps(XMM0, XMM6, _MM_SHUFFLE(2,0,2,0));
			XMM1	 = _mm_shuffle_ps(XMM1, XMM6, _MM_SHUFFLE(3,1,3,1));
			XMM4	 = _mm_shuffle_ps(XMM4, XMM7, _MM_SHUFFLE(2,0,2,0));
			XMM5	 = _mm_shuffle_ps(XMM5, XMM7, _MM_SHUFFLE(3,1,3,1));
			XMM2	 = _mm_load_ps(pcm0+j     );
			XMM3	 = _mm_load_ps(pcm1+j     );
			XMM6	 = _mm_load_ps(pcm0+j  + 4);
			XMM7	 = _mm_load_ps(pcm1+j  + 4);
			XMM2	 = _mm_sub_ps(XMM2, XMM0);
			XMM3	 = _mm_sub_ps(XMM3, XMM1);
			XMM6	 = _mm_sub_ps(XMM6, XMM4);
			XMM7	 = _mm_sub_ps(XMM7, XMM5);
			XMM0	 = _mm_load_ps(sofar0+j  );
			XMM1	 = _mm_load_ps(sofar0+j+4);
			XMM2	 = _mm_add_ps(XMM2, XMM0);
			XMM3	 = _mm_add_ps(XMM3, XMM0);
			XMM6	 = _mm_add_ps(XMM6, XMM1);
			XMM7	 = _mm_add_ps(XMM7, XMM1);
			_mm_store_ps(sofar0+j  , XMM2);
			_mm_store_ps(sofar1+j  , XMM3);
			_mm_store_ps(sofar0+j+4, XMM6);
			_mm_store_ps(sofar1+j+4, XMM7);
		}
	}
	else
	{
#endif														/* SSE Optimize */
      for(i=0;i<ch;i++){
	float *pcm=in[i];
	float *sofar=out[i];
	for(j=0,k=i;j<n;j++,k+=ch)
	  sofar[j]+=pcm[j]-work[k];

      }
#ifdef __SSE__												/* SSE Optimize */
	}
#endif														/* SSE Optimize */
    }
    return(ret);
  }else{
    return(0);
  }
}

/* duplicate code here as speed is somewhat more important */
int res2_inverse(vorbis_block *vb,vorbis_look_residue *vl,
		 float **in,int *nonzero,int ch){
  long i,k,l,s;
  vorbis_look_residue0 *look=(vorbis_look_residue0 *)vl;
  vorbis_info_residue0 *info=look->info;

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int partitions_per_word=look->phrasebook->dim;
  int n=info->end-info->begin;

  int partvals=n/samples_per_partition;
  int partwords=(partvals+partitions_per_word-1)/partitions_per_word;
#if	1
  int **partword=alloca(partwords*sizeof(*partword));
#else
  int **partword=_vorbis_block_alloc(vb,partwords*sizeof(*partword));
#endif

  for(i=0;i<ch;i++)if(nonzero[i])break;
  if(i==ch)return(0); /* no nonzero vectors */

  for(s=0;s<look->stages;s++){
    for(i=0,l=0;i<partvals;l++){

      if(s==0){
	/* fetch the partition word */
	int temp=vorbis_book_decode(look->phrasebook,&vb->opb);
	if(temp==-1)goto eopbreak;
	partword[l]=look->decodemap[temp];
	if(partword[l]==NULL)goto errout;
      }

      /* now we decode residual values for the partitions */
      for(k=0;k<partitions_per_word && i<partvals;k++,i++)
	if(info->secondstages[partword[l][k]]&(1<<s)){
	  codebook *stagebook=look->partbooks[partword[l][k]][s];
	  
	  if(stagebook){
	    if(vorbis_book_decodevv_add(stagebook,in,
					i*samples_per_partition+info->begin,ch,
					&vb->opb,samples_per_partition)==-1)
	      goto eopbreak;
	  }
	}
    } 
  }
  
 errout:
 eopbreak:
  return(0);
}

#if defined(_OPENMP)&&defined(__SSE__)
static long **_2class_mt(vorbis_block *vb,vorbis_look_residue *vl,float **in,
		      int ch, long ** partword, int thnum, int thmax){
  long i,j,k,l;
  vorbis_look_residue0 *look=(vorbis_look_residue0 *)vl;
  vorbis_info_residue0 *info=look->info;

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int possible_partitions=info->partitions;
  int n=info->end-info->begin;

  int partvals=n/samples_per_partition;
  int sn = _omp_get_start(partvals, 1, thnum, thmax);
  int en = _omp_get_end(partvals, 1, thnum, thmax);

  for(i=sn,l=(sn*samples_per_partition+info->begin)/ch;i<en;i++){
	float magmax=0.f;
	float angmax=0.f;
	if(ch==2&&possible_partitions==10)
	{
	  register __m128 PMAGMAX	 = _mm_setzero_ps();
	  register __m128 PANGMAX	 = _mm_setzero_ps();
	  float	*pin0	 = in[0];
	  float	*pin1	 = in[1];
	  
	  for(j=0;j<samples_per_partition;j+=16)
	  {
		__m128	XMM0	 = _mm_load_ps(pin0+l  );
		__m128	XMM1	 = _mm_load_ps(pin1+l  );
		__m128	XMM2	 = _mm_load_ps(pin0+l+4);
		__m128	XMM3	 = _mm_load_ps(pin1+l+4);
		XMM0	 = _mm_and_ps(XMM0, PM128(PABSMASK));
		XMM1	 = _mm_and_ps(XMM1, PM128(PABSMASK));
		XMM2	 = _mm_and_ps(XMM2, PM128(PABSMASK));
		XMM3	 = _mm_and_ps(XMM3, PM128(PABSMASK));
		PMAGMAX	 = _mm_max_ps(PMAGMAX, XMM0);
		PANGMAX	 = _mm_max_ps(PANGMAX, XMM1);
		PMAGMAX	 = _mm_max_ps(PMAGMAX, XMM2);
		PANGMAX	 = _mm_max_ps(PANGMAX, XMM3);
		l	+= 8;
	  }
	  magmax	 = _mm_max_horz(PMAGMAX);
	  angmax	 = _mm_max_horz(PANGMAX);
	  PMAGMAX	 = _mm_set1_ps(magmax);
	  PANGMAX	 = _mm_set1_ps(angmax);
	  {
		static int jtable0[16]	 = 
		{
		   0, 1, 0, 2, 0, 1, 0, 3,
		   0, 1, 0, 2, 0, 1, 0,-1,
		};
		static int jtable1[16]	 = 
		{
		   4, 5, 4, 6, 4, 5, 4, 7,
		   4, 5, 4, 6, 5, 5, 4,-1,
		};
		__m128	XMM0	 = _mm_lddqu_ps(info->classmetric1);
		__m128	XMM1	 = _mm_lddqu_ps(info->classmetric2);
		XMM0	 = _mm_cmplt_ps(XMM0, PMAGMAX);
		XMM1	 = _mm_cmplt_ps(XMM1, PANGMAX);
		XMM0	 = _mm_or_ps(XMM0, XMM1);
		j	 = _mm_movemask_ps(XMM0);
		if(j!=15)
		  j	 = jtable0[j];
		else
		{
		  __m128	XMM0	 = _mm_lddqu_ps(info->classmetric1+4);
		  __m128	XMM1	 = _mm_lddqu_ps(info->classmetric2+4);
		  XMM0	 = _mm_cmplt_ps(XMM0, PMAGMAX);
		  XMM1	 = _mm_cmplt_ps(XMM1, PANGMAX);
		  XMM0	 = _mm_or_ps(XMM0, XMM1);
		  j	 = _mm_movemask_ps(XMM0);
		  if(j!=15)
			j	 = jtable1[j];
		  else
		  {
			if(magmax<=info->classmetric1[8] &&
			  angmax<=info->classmetric2[8])
			  j	 = 8;
			else
			  j	 = 9;
		  }
		}
	  }
	}
	else
	{
	  for(j=0;j<samples_per_partition;j+=ch)
	  {
		if(fabs(in[0][l])>magmax)
		  magmax=fabs(in[0][l]);
		for(k=1;k<ch;k++)
		  if(fabs(in[k][l])>angmax)
			  angmax=fabs(in[k][l]);
		  l++;
	  }
	  for(j=0;j<possible_partitions-1;j++)
		if(magmax<=info->classmetric1[j] &&
		   angmax<=info->classmetric2[j])
		  break;
	}
	partword[0][i]=j;
  }
  if(thnum==0)
	look->frames++;
  return(partword);
}

long **res2mt_class(vorbis_block *vb,vorbis_look_residue *vl,
		  float **in,int *nonzero,int ch, long** partword,int thnum,int thmax){
  int i,used=0;
  for(i=0;i<ch;i++)
	if(nonzero[i])used++;
  if(used)
	return(_2class_mt(vb,vl,in,ch, partword,thnum,thmax));
  else
	return(0);
}
#endif

vorbis_func_residue residue0_exportbundle={
  NULL,
  &res0_unpack,
  &res0_look,
  &res0_free_info,
  &res0_free_look,
  NULL,
  NULL,
  &res0_inverse
};

vorbis_func_residue residue1_exportbundle={
  &res0_pack,
  &res0_unpack,
  &res0_look,
  &res0_free_info,
  &res0_free_look,
  &res1_class,
  &res1_forward,
  &res1_inverse
};

vorbis_func_residue residue2_exportbundle={
  &res0_pack,
  &res0_unpack,
  &res0_look,
  &res0_free_info,
  &res0_free_look,
  &res2_class,
  &res2_forward,
  &res2_inverse
};

#if defined(_OPENMP)&&defined(__SSE__)
vorbis_func_residue residue2mt_exportbundle={
  &res0_pack,
  &res0_unpack,
  &res0_look,
  &res0_free_info,
  &res0_free_look,
  &res2mt_class,
  &res2_forward,
  &res2_inverse
};
#endif
