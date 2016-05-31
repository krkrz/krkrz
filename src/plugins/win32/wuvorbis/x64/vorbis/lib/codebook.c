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

 function: basic codebook pack/unpack/code/decode operations
 last mod: $Id: codebook.c 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ogg/ogg.h>
#include "vorbis/codec.h"
#include "codebook.h"
#include "scales.h"
#include "misc.h"
#include "os.h"
#ifdef __SSE__												/* SSE Optimize */
#include "xmmlib.h"
#endif														/* SSE Optimize */

#define BUFFER_INCREMENT 256

#if 1
static const unsigned char bitrev8[256] = {
	0x00,	0x80,	0x40,	0xC0,	0x20,	0xA0,	0x60,	0xE0,
	0x10,	0x90,	0x50,	0xD0,	0x30,	0xB0,	0x70,	0xF0,
	0x08,	0x88,	0x48,	0xC8,	0x28,	0xA8,	0x68,	0xE8,
	0x18,	0x98,	0x58,	0xD8,	0x38,	0xB8,	0x78,	0xF8,
	0x04,	0x84,	0x44,	0xC4,	0x24,	0xA4,	0x64,	0xE4,
	0x14,	0x94,	0x54,	0xD4,	0x34,	0xB4,	0x74,	0xF4,
	0x0C,	0x8C,	0x4C,	0xCC,	0x2C,	0xAC,	0x6C,	0xEC,
	0x1C,	0x9C,	0x5C,	0xDC,	0x3C,	0xBC,	0x7C,	0xFC,
	0x02,	0x82,	0x42,	0xC2,	0x22,	0xA2,	0x62,	0xE2,
	0x12,	0x92,	0x52,	0xD2,	0x32,	0xB2,	0x72,	0xF2,
	0x0A,	0x8A,	0x4A,	0xCA,	0x2A,	0xAA,	0x6A,	0xEA,
	0x1A,	0x9A,	0x5A,	0xDA,	0x3A,	0xBA,	0x7A,	0xFA,
	0x06,	0x86,	0x46,	0xC6,	0x26,	0xA6,	0x66,	0xE6,
	0x16,	0x96,	0x56,	0xD6,	0x36,	0xB6,	0x76,	0xF6,
	0x0E,	0x8E,	0x4E,	0xCE,	0x2E,	0xAE,	0x6E,	0xEE,
	0x1E,	0x9E,	0x5E,	0xDE,	0x3E,	0xBE,	0x7E,	0xFE,
	0x01,	0x81,	0x41,	0xC1,	0x21,	0xA1,	0x61,	0xE1,
	0x11,	0x91,	0x51,	0xD1,	0x31,	0xB1,	0x71,	0xF1,
	0x09,	0x89,	0x49,	0xC9,	0x29,	0xA9,	0x69,	0xE9,
	0x19,	0x99,	0x59,	0xD9,	0x39,	0xB9,	0x79,	0xF9,
	0x05,	0x85,	0x45,	0xC5,	0x25,	0xA5,	0x65,	0xE5,
	0x15,	0x95,	0x55,	0xD5,	0x35,	0xB5,	0x75,	0xF5,
	0x0D,	0x8D,	0x4D,	0xCD,	0x2D,	0xAD,	0x6D,	0xED,
	0x1D,	0x9D,	0x5D,	0xDD,	0x3D,	0xBD,	0x7D,	0xFD,
	0x03,	0x83,	0x43,	0xC3,	0x23,	0xA3,	0x63,	0xE3,
	0x13,	0x93,	0x53,	0xD3,	0x33,	0xB3,	0x73,	0xF3,
	0x0B,	0x8B,	0x4B,	0xCB,	0x2B,	0xAB,	0x6B,	0xEB,
	0x1B,	0x9B,	0x5B,	0xDB,	0x3B,	0xBB,	0x7B,	0xFB,
	0x07,	0x87,	0x47,	0xC7,	0x27,	0xA7,	0x67,	0xE7,
	0x17,	0x97,	0x57,	0xD7,	0x37,	0xB7,	0x77,	0xF7,
	0x0F,	0x8F,	0x4F,	0xCF,	0x2F,	0xAF,	0x6F,	0xEF,
	0x1F,	0x9F,	0x5F,	0xDF,	0x3F,	0xBF,	0x7F,	0xFF
};
#endif

static const unsigned long mask[]=
{0x00000000,0x00000001,0x00000003,0x00000007,0x0000000f,
 0x0000001f,0x0000003f,0x0000007f,0x000000ff,0x000001ff,
 0x000003ff,0x000007ff,0x00000fff,0x00001fff,0x00003fff,
 0x00007fff,0x0000ffff,0x0001ffff,0x0003ffff,0x0007ffff,
 0x000fffff,0x001fffff,0x003fffff,0x007fffff,0x00ffffff,
 0x01ffffff,0x03ffffff,0x07ffffff,0x0fffffff,0x1fffffff,
 0x3fffffff,0x7fffffff,0xffffffff };

#if	!defined(_USRDLL)
/* Takes only up to 32 bits. */
STIN vorbis_oggpack_write(oggpack_buffer *b, unsigned long value, int bits)
{
	unsigned long lvalue, hvalue;
	if(b->endbyte+4>=b->storage){
		b->buffer=realloc(b->buffer,b->storage+BUFFER_INCREMENT+4);
		b->storage+=BUFFER_INCREMENT;
		b->ptr=b->buffer+b->endbyte;
	}

	value&=mask[bits]; 
	bits+=b->endbit;

	if(bits<24)
	{
		lvalue = value<<b->endbit;
		lvalue |= (b->ptr[0]&mask[b->endbit]);
		*(unsigned long*)(b->ptr) = lvalue;
	}
	else
	{
		lvalue = value<<b->endbit;
		hvalue = value>>(32-b->endbit);
		lvalue |= (b->ptr[0]&mask[b->endbit]);
		b->ptr[4] = hvalue;
		*(unsigned long*)(b->ptr) = lvalue;
	}

	b->endbyte+=bits/8;
	b->ptr+=bits/8;
	b->endbit=bits&7;
}
#endif

#if defined(_OPENMP)
void vorbis_oggpack_writecache(oggpack_writecache *c, unsigned long value, int bits)
{
	c->data[c->count].size  = bits;
	c->data[c->count].value = value;
	c->count ++;
}

int vorbis_book_encode_cache(codebook *book, int a, oggpack_writecache *c){
  vorbis_oggpack_writecache(c,book->codelist[a],book->c->lengthlist[a]);
  return(book->c->lengthlist[a]);
}

void vorbis_oggpack_cacheflush(oggpack_writecache *c, oggpack_buffer *b)
{
	int i;
	for(i=0;i<c->count;i++)
		oggpack_write(b, c->data[i].value, c->data[i].size);
	c->count = 0;
}
#endif

/* Read in bits without advancing the bitptr; bits <= 32 */
STIN long vorbis_oggpack_look(oggpack_buffer *b,int bits){
#if defined(_MSC_VER)
  unsigned __int64 t;
#else
  unsigned long ret;
#endif
  unsigned long m=mask[bits];

  bits+=b->endbit;

  if(b->endbyte+4>=b->storage){
    /* not the main path */
    if(b->endbyte*8+bits>b->storage*8)return(-1);
  }
  
#if defined(_MSC_VER)
	t = *(unsigned __int64*)(b->ptr);
	return ((long)(t >> b->endbit))&m;
#else
  ret=b->ptr[0]>>b->endbit;
  if(bits>8){
    ret|=b->ptr[1]<<(8-b->endbit);  
    if(bits>16){
      ret|=b->ptr[2]<<(16-b->endbit);  
      if(bits>24){
	ret|=b->ptr[3]<<(24-b->endbit);  
	if(bits>32 && b->endbit)
	  ret|=b->ptr[4]<<(32-b->endbit);
      }
    }
  }
  return(m&ret);
#endif
}

STIN void vorbis_oggpack_adv(oggpack_buffer *b,int bits){
  bits+=b->endbit;
  b->ptr+=bits/8;
  b->endbyte+=bits/8;
  b->endbit=bits&7;
}

/* packs the given codebook into the bitstream **************************/

int vorbis_staticbook_pack(const static_codebook *c,oggpack_buffer *opb){
  long i,j;
  int ordered=0;

  /* first the basic parameters */
  oggpack_write(opb,0x564342,24);
  oggpack_write(opb,c->dim,16);
  oggpack_write(opb,c->entries,24);

  /* pack the codewords.  There are two packings; length ordered and
     length random.  Decide between the two now. */
  
  for(i=1;i<c->entries;i++)
    if(c->lengthlist[i-1]==0 || c->lengthlist[i]<c->lengthlist[i-1])break;
  if(i==c->entries)ordered=1;
  
  if(ordered){
    /* length ordered.  We only need to say how many codewords of
       each length.  The actual codewords are generated
       deterministically */

    long count=0;
    oggpack_write(opb,1,1);  /* ordered */
    oggpack_write(opb,c->lengthlist[0]-1,5); /* 1 to 32 */

    for(i=1;i<c->entries;i++){
      long this=c->lengthlist[i];
      long last=c->lengthlist[i-1];
      if(this>last){
	for(j=last;j<this;j++){
	  oggpack_write(opb,i-count,_ilog(c->entries-count));
	  count=i;
	}
      }
    }
    oggpack_write(opb,i-count,_ilog(c->entries-count));
    
  }else{
    /* length random.  Again, we don't code the codeword itself, just
       the length.  This time, though, we have to encode each length */
    oggpack_write(opb,0,1);   /* unordered */
    
    /* algortihmic mapping has use for 'unused entries', which we tag
       here.  The algorithmic mapping happens as usual, but the unused
       entry has no codeword. */
    for(i=0;i<c->entries;i++)
      if(c->lengthlist[i]==0)break;

    if(i==c->entries){
      oggpack_write(opb,0,1); /* no unused entries */
      for(i=0;i<c->entries;i++)
	oggpack_write(opb,c->lengthlist[i]-1,5);
    }else{
      oggpack_write(opb,1,1); /* we have unused entries; thus we tag */
      for(i=0;i<c->entries;i++){
	if(c->lengthlist[i]==0){
	  oggpack_write(opb,0,1);
	}else{
	  oggpack_write(opb,1,1);
	  oggpack_write(opb,c->lengthlist[i]-1,5);
	}
      }
    }
  }

  /* is the entry number the desired return value, or do we have a
     mapping? If we have a mapping, what type? */
  oggpack_write(opb,c->maptype,4);
  switch(c->maptype){
  case 0:
    /* no mapping */
    break;
  case 1:case 2:
    /* implicitly populated value mapping */
    /* explicitly populated value mapping */
    
    if(!c->quantlist){
      /* no quantlist?  error */
      return(-1);
    }
    
    /* values that define the dequantization */
    oggpack_write(opb,c->q_min,32);
    oggpack_write(opb,c->q_delta,32);
    oggpack_write(opb,c->q_quant-1,4);
    oggpack_write(opb,c->q_sequencep,1);
    
    {
      int quantvals;
      switch(c->maptype){
      case 1:
	/* a single column of (c->entries/c->dim) quantized values for
	   building a full value list algorithmically (square lattice) */
	quantvals=_book_maptype1_quantvals(c);
	break;
      case 2:
	/* every value (c->entries*c->dim total) specified explicitly */
	quantvals=c->entries*c->dim;
	break;
      default: /* NOT_REACHABLE */
	quantvals=-1;
      }

      /* quantized values */
      for(i=0;i<quantvals;i++)
	oggpack_write(opb,labs(c->quantlist[i]),c->q_quant);

    }
    break;
  default:
    /* error case; we don't have any other map types now */
    return(-1);
  }

  return(0);
}

/* unpacks a codebook from the packet buffer into the codebook struct,
   readies the codebook auxiliary structures for decode *************/
int vorbis_staticbook_unpack(oggpack_buffer *opb,static_codebook *s){
  long i,j;
  memset(s,0,sizeof(*s));
  s->allocedp=1;

  /* make sure alignment is correct */
  if(oggpack_read(opb,24)!=0x564342)goto _eofout;

  /* first the basic parameters */
  s->dim=oggpack_read(opb,16);
  s->entries=oggpack_read(opb,24);
  if(s->entries==-1)goto _eofout;

  /* codeword ordering.... length ordered or unordered? */
  switch((int)oggpack_read(opb,1)){
  case 0:
    /* unordered */
    s->lengthlist=_ogg_malloc(sizeof(*s->lengthlist)*s->entries);

    /* allocated but unused entries? */
    if(oggpack_read(opb,1)){
      /* yes, unused entries */

      for(i=0;i<s->entries;i++){
	if(oggpack_read(opb,1)){
	  long num=oggpack_read(opb,5);
	  if(num==-1)goto _eofout;
	  s->lengthlist[i]=num+1;
	}else
	  s->lengthlist[i]=0;
      }
    }else{
      /* all entries used; no tagging */
      for(i=0;i<s->entries;i++){
	long num=oggpack_read(opb,5);
	if(num==-1)goto _eofout;
	s->lengthlist[i]=num+1;
      }
    }
    
    break;
  case 1:
    /* ordered */
    {
      long length=oggpack_read(opb,5)+1;
      s->lengthlist=_ogg_malloc(sizeof(*s->lengthlist)*s->entries);

      for(i=0;i<s->entries;){
	long num=oggpack_read(opb,_ilog(s->entries-i));
	if(num==-1)goto _eofout;
	for(j=0;j<num && i<s->entries;j++,i++)
	  s->lengthlist[i]=length;
	length++;
      }
    }
    break;
  default:
    /* EOF */
    return(-1);
  }
  
  /* Do we have a mapping to unpack? */
  switch((s->maptype=oggpack_read(opb,4))){
  case 0:
    /* no mapping */
    break;
  case 1: case 2:
    /* implicitly populated value mapping */
    /* explicitly populated value mapping */

    s->q_min=oggpack_read(opb,32);
    s->q_delta=oggpack_read(opb,32);
    s->q_quant=oggpack_read(opb,4)+1;
    s->q_sequencep=oggpack_read(opb,1);

    {
      int quantvals=0;
      switch(s->maptype){
      case 1:
	quantvals=_book_maptype1_quantvals(s);
	break;
      case 2:
	quantvals=s->entries*s->dim;
	break;
      }
      
      /* quantized values */
      s->quantlist=_ogg_malloc(sizeof(*s->quantlist)*quantvals);
      for(i=0;i<quantvals;i++)
	s->quantlist[i]=oggpack_read(opb,s->q_quant);
      
      if(quantvals&&s->quantlist[quantvals-1]==-1)goto _eofout;
    }
    break;
  default:
    goto _errout;
  }

  /* all set */
  return(0);
  
 _errout:
 _eofout:
  vorbis_staticbook_clear(s);
  return(-1); 
}

/* returns the number of bits ************************************************/
int vorbis_book_encode(codebook *book, int a, oggpack_buffer *b){
#if	!defined(_USRDLL)
  vorbis_oggpack_write(b,book->codelist[a],book->c->lengthlist[a]);
#else
  oggpack_write(b,book->codelist[a],book->c->lengthlist[a]);
#endif
  return(book->c->lengthlist[a]);
}

/* One the encode side, our vector writers are each designed for a
specific purpose, and the encoder is not flexible without modification:

The LSP vector coder uses a single stage nearest-match with no
interleave, so no step and no error return.  This is specced by floor0
and doesn't change.

Residue0 encoding interleaves, uses multiple stages, and each stage
peels of a specific amount of resolution from a lattice (thus we want
to match by threshold, not nearest match).  Residue doesn't *have* to
be encoded that way, but to change it, one will need to add more
infrastructure on the encode side (decode side is specced and simpler) */

/* floor0 LSP (single stage, non interleaved, nearest match) */
/* returns entry number and *modifies a* to the quantization value *****/
int vorbis_book_errorv(codebook *book,float *a){
  int dim=book->dim,k;
  int best=_best(book,a,1);
  for(k=0;k<dim;k++)
    a[k]=(book->valuelist+best*dim)[k];
  return(best);
}

/* returns the number of bits and *modifies a* to the quantization value *****/
int vorbis_book_encodev(codebook *book,int best,float *a,oggpack_buffer *b){
  int k,dim=book->dim;
  for(k=0;k<dim;k++)
    a[k]=(book->valuelist+best*dim)[k];
  return(vorbis_book_encode(book,best,b));
}

/* the 'eliminate the decode tree' optimization actually requires the
   codewords to be MSb first, not LSb.  This is an annoying inelegancy
   (and one of the first places where carefully thought out design
   turned out to be wrong; Vorbis II and future Ogg codecs should go
   to an MSb bitpacker), but not actually the huge hit it appears to
   be.  The first-stage decode table catches most words so that
   bitreverse is not in the main execution path. */

static ogg_uint32_t bitreverse(ogg_uint32_t x){
#if	0
  x=    ((x>>16)&0x0000ffff) | ((x<<16)&0xffff0000);
  x=    ((x>> 8)&0x00ff00ff) | ((x<< 8)&0xff00ff00);
  x=    ((x>> 4)&0x0f0f0f0f) | ((x<< 4)&0xf0f0f0f0);
  x=    ((x>> 2)&0x33333333) | ((x<< 2)&0xcccccccc);
  return((x>> 1)&0x55555555) | ((x<< 1)&0xaaaaaaaa);
#else
	ogg_uint32_t x1, x2, x3;
	x3 = x;
	x2 = x;
	x1 = x;
	x3 = (x3 >> 24);
	x2 = (x2 >> 16)&0xFF;
	x1 = (x1 >>  8)&0xFF;
	x  = x&0xFF;
	x3 = bitrev8[x3];
	x2 = bitrev8[x2];
	x1 = bitrev8[x1];
	x  = bitrev8[x ];
	x2 = x2 <<  8;
	x1 = x1 << 16;
	x  = x  << 24;
	x  = x  | x1;
	x2 = x2 | x3;
	x  = x  | x2;
	return x;
#endif
}

STIN long decode_packed_entry_number(codebook *book, oggpack_buffer *b){
  int  read=book->dec_maxlength;
  long lo,hi;
  long lok = vorbis_oggpack_look(b,book->dec_firsttablen);
 
  if (lok >= 0) {
    long entry = book->dec_firsttable[lok];
    if(entry&0x80000000UL){
      lo=(entry>>15)&0x7fff;
      hi=book->used_entries-(entry&0x7fff);
    }else{
      vorbis_oggpack_adv(b, book->dec_codelengths[entry-1]);
      return(entry-1);
    }
  }else{
    lo=0;
    hi=book->used_entries;
  }

  lok = vorbis_oggpack_look(b, read);

  while(lok<0 && read>1)
    lok = vorbis_oggpack_look(b, --read);
  if(lok<0)return -1;

  /* bisect search for the codeword in the ordered list */
  {
    ogg_uint32_t testword=bitreverse((ogg_uint32_t)lok);

    while(hi-lo>1){
      long p=(hi-lo)>>1;
      long test=book->codelist[lo+p]>testword;    
      lo+=p&(test-1);
      hi-=p&(-test);
    }

    if(book->dec_codelengths[lo]<=read){
      vorbis_oggpack_adv(b, book->dec_codelengths[lo]);
      return(lo);
    }
  }
  
  vorbis_oggpack_adv(b, read);
  return(-1);
}

/* Decode side is specced and easier, because we don't need to find
   matches using different criteria; we simply read and map.  There are
   two things we need to do 'depending':
   
   We may need to support interleave.  We don't really, but it's
   convenient to do it here rather than rebuild the vector later.

   Cascades may be additive or multiplicitive; this is not inherent in
   the codebook, but set in the code using the codebook.  Like
   interleaving, it's easiest to do it here.  
   addmul==0 -> declarative (set the value)
   addmul==1 -> additive
   addmul==2 -> multiplicitive */

/* returns the [original, not compacted] entry number or -1 on eof *********/
long vorbis_book_decode(codebook *book, oggpack_buffer *b){
  long packed_entry=decode_packed_entry_number(book,b);
  if(packed_entry>=0)
    return(book->dec_index[packed_entry]);
  
  /* if there's no dec_index, the codebook unpacking isn't collapsed */
  return(packed_entry);
}

/* returns 0 on OK or -1 on eof *************************************/
long vorbis_book_decodevs_add(codebook *book,float *a,oggpack_buffer *b,int n){
  int step=n/book->dim;
  long *entry = alloca(sizeof(*entry)*step);
  float **t = alloca(sizeof(*t)*step);
  int i,j,o;

  for (i = 0; i < step; i++) {
    entry[i]=decode_packed_entry_number(book,b);
    if(entry[i]==-1)return(-1);
    t[i] = book->valuelist+entry[i]*book->dim;
  }
  for(i=0,o=0;i<book->dim;i++,o+=step)
    for (j=0;j<step;j++)
      a[o+j]+=t[j][i];
  return(0);
}

long vorbis_book_decodev_add(codebook *book,float *a,oggpack_buffer *b,int n){
  int i,j,entry;
  float *t;

  if(book->dim>8){
    for(i=0;i<n;){
      entry = decode_packed_entry_number(book,b);
      if(entry==-1)return(-1);
      t     = book->valuelist+entry*book->dim;
      for (j=0;j<book->dim;)
	a[i++]+=t[j++];
    }
  }else{
    for(i=0;i<n;){
      entry = decode_packed_entry_number(book,b);
      if(entry==-1)return(-1);
      t     = book->valuelist+entry*book->dim;
      j=0;
      switch((int)book->dim){
      case 8:
	a[i++]+=t[j++];
      case 7:
	a[i++]+=t[j++];
      case 6:
	a[i++]+=t[j++];
      case 5:
	a[i++]+=t[j++];
      case 4:
	a[i++]+=t[j++];
      case 3:
	a[i++]+=t[j++];
      case 2:
	a[i++]+=t[j++];
      case 1:
	a[i++]+=t[j++];
      case 0:
	break;
      }
    }
  }    
  return(0);
}

long vorbis_book_decodev_set(codebook *book,float *a,oggpack_buffer *b,int n){
  int i,j,entry;
  float *t;

  for(i=0;i<n;){
    entry = decode_packed_entry_number(book,b);
    if(entry==-1)return(-1);
    t     = book->valuelist+entry*book->dim;
    for (j=0;j<book->dim;)
      a[i++]=t[j++];
  }
  return(0);
}

long vorbis_book_decodevv_add(codebook *book,float **a,long offset,int ch,
			      oggpack_buffer *b,int n){
#ifdef __SSE__												/* SSE Optimize */
	long i,j;
	int chptr=0;

	if(ch==2)
	{
		int mid0 = (offset/2+3)&(~3);
		int mid1 = ((offset+n)/2)&(~3);
		float *bvl = book->valuelist;
		float *a0 = a[0];
		float *a1 = a[1];
		switch(book->dim)
		{
			default :
				for(i=offset/2;i<(offset+n)/2;)
				{
					long entry = decode_packed_entry_number(book,b);
					if(entry==-1)
						return(-1);
					{
						const float *t	 = bvl+entry*book->dim;
						for (j=0;j<book->dim;j++)
						{
							a[chptr++][i]	+= t[j];
							if(chptr==2)
							{
								chptr	 = 0;
								i	++;
							}
						}
					}
				}
				break;
			case 2:
				for(i=offset/2;i<mid0;)
				{
					long entry = decode_packed_entry_number(book,b);
					if(entry==-1)
						return(-1);
					{
						const float *t	 = bvl+entry*2;
						__m128	XMM0 = _mm_load_ss(t  );
						__m128	XMM1 = _mm_load_ss(a0+i);
						__m128	XMM2 = _mm_load_ss(t  );
						__m128	XMM3 = _mm_load_ss(a1+i);
						XMM0	 = _mm_add_ss(XMM0, XMM1);
						XMM2	 = _mm_add_ss(XMM2, XMM3);
						_mm_store_ss(a0+i  , XMM0);
						_mm_store_ss(a1+i++, XMM2);
					}
				}
				for(;i<mid1;)
				{
					/*
						XMM0	(T11 T10 T01 T00)
						XMM2	(T31 T30 T21 T20)
					*/
					__m128	XMM0, XMM1, XMM2, XMM3, XMM4;
					const float *t0, *t1,*t2, *t3;
					long entry = decode_packed_entry_number(book,b);
					if(entry==-1)
						return(-1);
					t0	 = bvl+entry*2;
					entry = decode_packed_entry_number(book,b);
					if(entry==-1)
						return(-1);
					t1	 = bvl+entry*2;
					entry = decode_packed_entry_number(book,b);
					if(entry==-1)
						return(-1);
					t2	 = bvl+entry*2;
					entry = decode_packed_entry_number(book,b);
					if(entry==-1)
						return(-1);
					t3	 = bvl+entry*2;
#pragma warning(disable : 592)
					XMM0	 = _mm_loadl_pi(XMM0, (__m64*)t0);
					XMM2	 = _mm_loadl_pi(XMM2, (__m64*)t2);
					XMM3	 = _mm_load_ps(a0+i);
					XMM0	 = _mm_loadh_pi(XMM0, (__m64*)t1);
					XMM2	 = _mm_loadh_pi(XMM2, (__m64*)t3);
#pragma warning(default : 592)
					/*
						XMM0	(T30 T20 T10 T00)
						XMM2	(T31 T21 T11 T01)
					*/
					XMM4	 = _mm_load_ps(a1+i);
					XMM1	 = XMM0;
					XMM0	 = _mm_shuffle_ps(XMM0, XMM2, _MM_SHUFFLE(2,0,2,0));
					XMM1	 = _mm_shuffle_ps(XMM1, XMM2, _MM_SHUFFLE(3,1,3,1));
					XMM0	 = _mm_add_ps(XMM0, XMM3);
					XMM1	 = _mm_add_ps(XMM1, XMM4);
					_mm_store_ps(a0+i, XMM0);
					_mm_store_ps(a1+i, XMM1);
					i	+= 4;
				}
				for(;i<(offset+n)/2;)
				{
					long entry = decode_packed_entry_number(book,b);
					if(entry==-1)
						return(-1);
					{
						const float *t	 = bvl+entry*2;
						__m128	XMM0 = _mm_load_ss(t  );
						__m128	XMM1 = _mm_load_ss(a0+i);
						__m128	XMM2 = _mm_load_ss(t  );
						__m128	XMM3 = _mm_load_ss(a1+i);
						XMM0	 = _mm_add_ss(XMM0, XMM1);
						XMM2	 = _mm_add_ss(XMM2, XMM3);
						_mm_store_ss(a0+i  , XMM0);
						_mm_store_ss(a1+i++, XMM2);
					}
				}
				break;
			case 4:
				for(i=offset/2;i<mid0;)
				{
					long entry = decode_packed_entry_number(book,b);
					if(entry==-1)
						return(-1);
					{
						const float *t	 = bvl+entry*4;
						__m128	XMM0 = _mm_load_ss(t  );
						__m128	XMM1 = _mm_load_ss(a0+i  );
						__m128	XMM2 = _mm_load_ss(t+1);
						__m128	XMM3 = _mm_load_ss(a1+i  );
						__m128	XMM4 = _mm_load_ss(t+2);
						__m128	XMM5 = _mm_load_ss(a0+i+1);
						__m128	XMM6 = _mm_load_ss(t+3);
						__m128	XMM7 = _mm_load_ss(a1+i+1);
						XMM0	 = _mm_add_ss(XMM0, XMM1);
						XMM2	 = _mm_add_ss(XMM2, XMM3);
						XMM4	 = _mm_add_ss(XMM4, XMM5);
						XMM6	 = _mm_add_ss(XMM6, XMM7);
						_mm_store_ss(a0+i  , XMM0);
						_mm_store_ss(a1+i  , XMM2);
						_mm_store_ss(a0+i+1, XMM4);
						_mm_store_ss(a1+i+1, XMM6);
						i	+= 2;
					}
				}
				for(;i<mid1;)
				{
					/*
						XMM0	(T03 T02 T01 T00)
						XMM1	(T13 T12 T11 T10)
						XMM2	(T23 T22 T21 T20)
						XMM3	(T33 T32 T31 T30)
					*/
					__m128	XMM0, XMM1, XMM2, XMM3;
					__m128	XMM4, XMM5, XMM6, XMM7;
					long entry = decode_packed_entry_number(book,b);
					if(entry==-1)
						return(-1);
					XMM0	 = _mm_lddqu_ps(bvl+entry*4);
					entry = decode_packed_entry_number(book,b);
					if(entry==-1)
						return(-1);
					XMM1	 = _mm_lddqu_ps(bvl+entry*4);
					entry = decode_packed_entry_number(book,b);
					if(entry==-1)
						return(-1);
					XMM2	 = _mm_lddqu_ps(bvl+entry*4);
					entry = decode_packed_entry_number(book,b);
					if(entry==-1)
						return(-1);
					XMM3	 = _mm_lddqu_ps(bvl+entry*4);
					/*
						XMM0	(T12 T10 T02 T00)
						XMM4	(T13 T11 T03 T01)
						XMM2	(T32 T20 T12 T10)
						XMM5	(T33 T21 T13 T11)
					*/
					XMM4	 = XMM0;
					XMM5	 = XMM2;
					XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(2,0,2,0));
					XMM4	 = _mm_shuffle_ps(XMM4, XMM1, _MM_SHUFFLE(3,1,3,1));
					XMM1	 = _mm_load_ps(a0+i  );
					XMM2	 = _mm_shuffle_ps(XMM2, XMM3, _MM_SHUFFLE(2,0,2,0));
					XMM5	 = _mm_shuffle_ps(XMM5, XMM3, _MM_SHUFFLE(3,1,3,1));
					XMM3	 = _mm_load_ps(a1+i  );
					XMM6	 = _mm_load_ps(a0+i+4);
					XMM7	 = _mm_load_ps(a1+i+4);
					XMM0	 = _mm_add_ps(XMM0, XMM1);
					XMM4	 = _mm_add_ps(XMM4, XMM3);
					XMM2	 = _mm_add_ps(XMM2, XMM6);
					XMM5	 = _mm_add_ps(XMM5, XMM7);
					_mm_store_ps(a0+i  , XMM0);
					_mm_store_ps(a1+i  , XMM4);
					_mm_store_ps(a0+i+4, XMM2);
					_mm_store_ps(a1+i+4, XMM5);
					i	+= 8;
				}
				for(;i<(offset+n)/2;)
				{
					long entry = decode_packed_entry_number(book,b);
					if(entry==-1)
						return(-1);
					{
						const float *t	 = bvl+entry*4;
						__m128	XMM0 = _mm_load_ss(t  );
						__m128	XMM1 = _mm_load_ss(a0+i  );
						__m128	XMM2 = _mm_load_ss(t+1);
						__m128	XMM3 = _mm_load_ss(a1+i  );
						__m128	XMM4 = _mm_load_ss(t+2);
						__m128	XMM5 = _mm_load_ss(a0+i+1);
						__m128	XMM6 = _mm_load_ss(t+3);
						__m128	XMM7 = _mm_load_ss(a1+i+1);
						XMM0	 = _mm_add_ss(XMM0, XMM1);
						XMM2	 = _mm_add_ss(XMM2, XMM3);
						XMM4	 = _mm_add_ss(XMM4, XMM5);
						XMM6	 = _mm_add_ss(XMM6, XMM7);
						_mm_store_ss(a0+i  , XMM0);
						_mm_store_ss(a1+i  , XMM2);
						_mm_store_ss(a0+i+1, XMM4);
						_mm_store_ss(a1+i+1, XMM6);
						i	+= 2;
					}
				}
				break;
			case 8:
				for(i=offset/2;i<mid0;)
				{
					long entry = decode_packed_entry_number(book,b);
					if(entry==-1)
						return(-1);
					{
						const float *t	 = bvl+entry*8;
						__m128	XMM0	 = _mm_lddqu_ps(t  );
						__m128	XMM1	 = _mm_lddqu_ps(t+4);
						__m128	XMM2	 = _mm_load_ps(a0+i);
						__m128	XMM3	 = _mm_load_ps(a1+i);
						__m128	XMM4	 = XMM0;
						XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(2,0,2,0));
						XMM4	 = _mm_shuffle_ps(XMM4, XMM1, _MM_SHUFFLE(3,1,3,1));
						XMM0	 = _mm_add_ps(XMM0, XMM2);
						XMM4	 = _mm_add_ps(XMM4, XMM3);
						_mm_store_ps(a0+i  , XMM0);
						_mm_store_ps(a1+i  , XMM4);
						i	+= 4;
					}
				}
				for(;i<mid1;)
				{
					/*
						XMM0	(T03 T02 T01 T00)
						XMM1	(T13 T12 T11 T10)
						XMM2	(T07 T06 T05 T04)
						XMM2	(T17 T16 T15 T14)
					*/
					__m128	XMM0, XMM1, XMM2, XMM3;
					__m128	XMM4, XMM5, XMM6, XMM7;
					const float *t;
					long entry = decode_packed_entry_number(book,b);
					if(entry==-1)
						return(-1);
					t	 = bvl+entry*8;
					XMM0	 = _mm_lddqu_ps(t  );
					XMM1	 = _mm_lddqu_ps(t+4);
					entry = decode_packed_entry_number(book,b);
					if(entry==-1)
						return(-1);
					t	 = bvl+entry*8;
					XMM2	 = _mm_lddqu_ps(t  );
					XMM3	 = _mm_lddqu_ps(t+4);
					/*
						XMM0	(T12 T10 T02 T00)
						XMM4	(T13 T11 T03 T01)
						XMM2	(T16 T14 T06 T04)
						XMM5	(T17 T15 T07 T05)
					*/
					XMM4	 = XMM0;
					XMM5	 = XMM2;
					XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(2,0,2,0));
					XMM4	 = _mm_shuffle_ps(XMM4, XMM1, _MM_SHUFFLE(3,1,3,1));
					XMM2	 = _mm_shuffle_ps(XMM2, XMM3, _MM_SHUFFLE(2,0,2,0));
					XMM5	 = _mm_shuffle_ps(XMM5, XMM3, _MM_SHUFFLE(3,1,3,1));
					XMM1	 = _mm_load_ps(a0+i  );
					XMM3	 = _mm_load_ps(a1+i  );
					XMM6	 = _mm_load_ps(a0+i+4);
					XMM7	 = _mm_load_ps(a1+i+4);
					XMM0	 = _mm_add_ps(XMM0, XMM1);
					XMM4	 = _mm_add_ps(XMM4, XMM3);
					XMM2	 = _mm_add_ps(XMM2, XMM6);
					XMM5	 = _mm_add_ps(XMM5, XMM7);
					_mm_store_ps(a0+i  , XMM0);
					_mm_store_ps(a1+i  , XMM4);
					_mm_store_ps(a0+i+4, XMM2);
					_mm_store_ps(a1+i+4, XMM5);
					i	+= 8;
				}
				for(;i<(offset+n)/2;)
				{
					long entry = decode_packed_entry_number(book,b);
					if(entry==-1)
						return(-1);
					{
						const float *t	 = bvl+entry*8;
						__m128	XMM0	 = _mm_lddqu_ps(t  );
						__m128	XMM1	 = _mm_lddqu_ps(t+4);
						__m128	XMM4	 = XMM0;
						__m128	XMM2	 = _mm_load_ps(a0+i);
						__m128	XMM3	 = _mm_load_ps(a1+i);
						XMM0	 = _mm_shuffle_ps(XMM0, XMM1, _MM_SHUFFLE(2,0,2,0));
						XMM4	 = _mm_shuffle_ps(XMM4, XMM1, _MM_SHUFFLE(3,1,3,1));
						XMM0	 = _mm_add_ps(XMM0, XMM2);
						XMM4	 = _mm_add_ps(XMM4, XMM3);
						_mm_store_ps(a0+i  , XMM0);
						_mm_store_ps(a1+i  , XMM4);
						i	+= 4;
					}
				}
				break;
		}
	}
	else
	{
		for(i=offset/ch;i<(offset+n)/ch;)
		{
			long entry = decode_packed_entry_number(book,b);
			if(entry==-1)
				return(-1);
			{
				const float *t	 = book->valuelist+entry*book->dim;
				for (j=0;j<book->dim;j++)
				{
					a[chptr++][i]	+= t[j];
					if(chptr==ch)
					{
						chptr	 = 0;
						i	++;
					}
				}
			}
		}
	}
#else														/* SSE Optimize */
  long i,j,entry;
  int chptr=0;

  for(i=offset/ch;i<(offset+n)/ch;){
    entry = decode_packed_entry_number(book,b);
    if(entry==-1)return(-1);
    {
      const float *t = book->valuelist+entry*book->dim;
      for (j=0;j<book->dim;j++){
	a[chptr++][i]+=t[j];
	if(chptr==ch){
	  chptr=0;
	  i++;
	}
      }
    }
  }
#endif														/* SSE Optimize */
  return(0);
}

#ifdef _V_SELFTEST
/* Simple enough; pack a few candidate codebooks, unpack them.  Code a
   number of vectors through (keeping track of the quantized values),
   and decode using the unpacked book.  quantized version of in should
   exactly equal out */

#include <stdio.h>

#include "vorbis/book/lsp20_0.vqh"
#include "vorbis/book/res0a_13.vqh"
#define TESTSIZE 40

float test1[TESTSIZE]={
  0.105939f,
  0.215373f,
  0.429117f,
  0.587974f,

  0.181173f,
  0.296583f,
  0.515707f,
  0.715261f,

  0.162327f,
  0.263834f,
  0.342876f,
  0.406025f,

  0.103571f,
  0.223561f,
  0.368513f,
  0.540313f,

  0.136672f,
  0.395882f,
  0.587183f,
  0.652476f,

  0.114338f,
  0.417300f,
  0.525486f,
  0.698679f,

  0.147492f,
  0.324481f,
  0.643089f,
  0.757582f,

  0.139556f,
  0.215795f,
  0.324559f,
  0.399387f,

  0.120236f,
  0.267420f,
  0.446940f,
  0.608760f,

  0.115587f,
  0.287234f,
  0.571081f,
  0.708603f,
};

float test3[TESTSIZE]={
  0,1,-2,3,4,-5,6,7,8,9,
  8,-2,7,-1,4,6,8,3,1,-9,
  10,11,12,13,14,15,26,17,18,19,
  30,-25,-30,-1,-5,-32,4,3,-2,0};

static_codebook *testlist[]={&_vq_book_lsp20_0,
			     &_vq_book_res0a_13,NULL};
float   *testvec[]={test1,test3};

int main(){
  oggpack_buffer write;
  oggpack_buffer read;
  long ptr=0,i;
  oggpack_writeinit(&write);
  
  fprintf(stderr,"Testing codebook abstraction...:\n");

  while(testlist[ptr]){
    codebook c;
    static_codebook s;
    float *qv=alloca(sizeof(*qv)*TESTSIZE);
    float *iv=alloca(sizeof(*iv)*TESTSIZE);
    memcpy(qv,testvec[ptr],sizeof(*qv)*TESTSIZE);
    memset(iv,0,sizeof(*iv)*TESTSIZE);

    fprintf(stderr,"\tpacking/coding %ld... ",ptr);

    /* pack the codebook, write the testvector */
    oggpack_reset(&write);
    vorbis_book_init_encode(&c,testlist[ptr]); /* get it into memory
                                                  we can write */
    vorbis_staticbook_pack(testlist[ptr],&write);
    fprintf(stderr,"Codebook size %ld bytes... ",oggpack_bytes(&write));
    for(i=0;i<TESTSIZE;i+=c.dim){
      int best=_best(&c,qv+i,1);
      vorbis_book_encodev(&c,best,qv+i,&write);
    }
    vorbis_book_clear(&c);
    
    fprintf(stderr,"OK.\n");
    fprintf(stderr,"\tunpacking/decoding %ld... ",ptr);

    /* transfer the write data to a read buffer and unpack/read */
    oggpack_readinit(&read,oggpack_get_buffer(&write),oggpack_bytes(&write));
    if(vorbis_staticbook_unpack(&read,&s)){
      fprintf(stderr,"Error unpacking codebook.\n");
      exit(1);
    }
    if(vorbis_book_init_decode(&c,&s)){
      fprintf(stderr,"Error initializing codebook.\n");
      exit(1);
    }

    for(i=0;i<TESTSIZE;i+=c.dim)
      if(vorbis_book_decodev_set(&c,iv+i,&read,c.dim)==-1){
	fprintf(stderr,"Error reading codebook test data (EOP).\n");
	exit(1);
      }
    for(i=0;i<TESTSIZE;i++)
      if(fabs(qv[i]-iv[i])>.000001){
	fprintf(stderr,"read (%g) != written (%g) at position (%ld)\n",
		iv[i],qv[i],i);
	exit(1);
      }
	  
    fprintf(stderr,"OK\n");
    ptr++;
  }

  /* The above is the trivial stuff; now try unquantizing a log scale codebook */

  exit(0);
}

#endif
