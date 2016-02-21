
#include "tjsTypes.h"
#include "tvpgl.h"
#include "tvpgl_ia32_intf.h"
#include "simd_def_x86x64.h"


extern "C" {
extern unsigned char TVPNegativeMulTable65[65*256];
extern unsigned char TVPOpacityOnOpacityTable65[65*256];
};

// tshift = 6 : 65
// tshift = 8 : normal(255)
template<int tshift>
struct sse2_apply_color_map_xx_functor {
	__m128i mc_;
	__m128i color_;
	const __m128i zero_;
	inline sse2_apply_color_map_xx_functor( tjs_uint32 color ) : zero_(_mm_setzero_si128()) {
		mc_ = _mm_cvtsi32_si128( color );
		mc_ = _mm_shuffle_epi32( mc_, _MM_SHUFFLE( 0, 0, 0, 0 )  );
		color_ = mc_;
		mc_ = _mm_unpacklo_epi8( mc_, zero_ );
	}
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint8 s ) const {
		__m128i md = _mm_cvtsi32_si128( d );
		md = _mm_unpacklo_epi8( md, zero_ );
		__m128i mo = _mm_cvtsi32_si128( s );
		mo = _mm_shufflelo_epi16( mo, _MM_SHUFFLE( 0, 0, 0, 0 )  );	// 00oo00oo00oo00oo
		__m128i mc = mc_;
		mc = _mm_sub_epi16( mc, md );	// c - d
		mc = _mm_mullo_epi16( mc, mo );	// c *= opa
		mc = _mm_srai_epi16( mc, tshift );	// c >>= tshift
		md = _mm_add_epi16( md, mc );	// d += c
		md = _mm_packus_epi16( md, zero_ );
		return _mm_cvtsi128_si32( md );
	}
	inline __m128i operator()( __m128i md1, __m128i mo1 ) const {
		__m128i md2 = md1;
		md1 = _mm_unpacklo_epi8( md1, zero_ );
		__m128i mo2 = mo1;
		mo1 = _mm_unpacklo_epi16( mo1, mo1 );
		mo2 = _mm_unpackhi_epi16( mo2, mo2 );

		__m128i mc = mc_;
		mc = _mm_sub_epi16( mc, md1 );		// c - d
		mc = _mm_mullo_epi16( mc, mo1 );	// c *= opa
		mc = _mm_srai_epi16( mc, tshift );	// c >>= tshift
		md1 = _mm_add_epi16( md1, mc );		// d += c

		mc = mc_;
		md2 = _mm_unpackhi_epi8( md2, zero_ );
		mc = _mm_sub_epi16( mc, md2 );		// c - d
		mc = _mm_mullo_epi16( mc, mo2 );	// c *= opa
		mc = _mm_srai_epi16( mc, tshift );	// c >>= tshift
		md2 = _mm_add_epi16( md2, mc );		// d += c

		return _mm_packus_epi16( md1, md2 );
	}
};

template<typename tbase>
struct sse2_apply_color_map_xx_o_functor : tbase {
	tjs_int opa32_;
	__m128i opa_;
	inline sse2_apply_color_map_xx_o_functor( tjs_uint32 color, tjs_int opa ) : tbase(color), opa32_(opa) {
		opa_ = _mm_cvtsi32_si128( opa );
		opa_ = _mm_shufflelo_epi16( opa_, _MM_SHUFFLE( 0, 0, 0, 0 )  );
	}
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint8 s ) const {
		return tbase::operator()( d, (tjs_uint8)( (s*opa32_)>>8) );
	}
	inline __m128i operator()( __m128i md1, tjs_uint32 s ) const {
		__m128i mo = _mm_cvtsi32_si128( s );
		mo = _mm_unpacklo_epi8( mo, zero_ );	// 0000 0o 0o 0o 0o
		mo = _mm_mullo_epi16( mo, opa_ );
		mo = _mm_srli_epi16( mo, 8 );
		mo = _mm_unpacklo_epi16( mo, mo );	// 1 1 2 2 3 3 4 4
		return tbase::operator()( md1, mo );
	}
};
template<typename tbase>
struct sse2_apply_color_map_xx_straight_functor : tbase {
	inline sse2_apply_color_map_xx_straight_functor( tjs_uint32 color ) : tbase(color) {}
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint8 s ) const {
		return tbase::operator()( d, s );
	}
	inline __m128i operator()( __m128i md1, tjs_uint32 s ) const {
		__m128i mo = _mm_cvtsi32_si128( s );
		mo = _mm_unpacklo_epi8( mo, zero_ );	// 0000 0o 0o 0o 0o
		mo = _mm_unpacklo_epi16( mo, mo );		// 1 1 2 2 3 3 4 4
		return tbase::operator()( md1, mo );
	}
};
typedef sse2_apply_color_map_xx_straight_functor<sse2_apply_color_map_xx_functor<6> > sse2_apply_color_map65_functor;
typedef sse2_apply_color_map_xx_straight_functor<sse2_apply_color_map_xx_functor<8> > sse2_apply_color_map_functor;
typedef sse2_apply_color_map_xx_o_functor<sse2_apply_color_map_xx_functor<6> > sse2_apply_color_map65_o_functor;
typedef sse2_apply_color_map_xx_o_functor<sse2_apply_color_map_xx_functor<8> > sse2_apply_color_map_o_functor;

struct sse2_apply_color_map65_d_functor {
	__m128i mc_;
	__m128i color_;
	const __m128i zero_;
	const __m128i colormask_;
	inline sse2_apply_color_map65_d_functor( tjs_uint32 color ) : zero_(_mm_setzero_si128()), colormask_(_mm_set1_epi32(0x00ffffff)) {
		mc_ = _mm_cvtsi32_si128( color|0xff000000 );
		mc_ = _mm_shuffle_epi32( mc_, _MM_SHUFFLE( 0, 0, 0, 0 )  );
		color_ = mc_;
		mc_ = _mm_unpacklo_epi8( mc_, zero_ );
	}
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint8 s ) const {
		if( s == 0 ) return d;
		__m128i mc = mc_;	// color
		__m128i md = _mm_cvtsi32_si128( d );
		md = _mm_unpacklo_epi8( md, zero_ );
		tjs_int addr = (s << 8) | (d >> 24);
		tjs_uint32 dopa = TVPNegativeMulTable65[addr];		// dest opa table
		tjs_uint32 o = TVPOpacityOnOpacityTable65[addr];	// blend opa table
		dopa <<= 24;
		__m128i mo = _mm_cvtsi32_si128( o );
		mo = _mm_shufflelo_epi16( mo, _MM_SHUFFLE( 0, 0, 0, 0 )  );	// 00oo00oo00oo00oo
		mc = _mm_sub_epi16( mc, md );	// mc -= md
		mc = _mm_mullo_epi16( mc, mo );	// mc *= mo
		md = _mm_slli_epi16( md, 8 );	// md <<= 8
		md = _mm_add_epi16( md, mc );	// md += mc
		md = _mm_srli_epi16( md, 8 );	// md >>= 8
		md = _mm_packus_epi16( md, zero_ );
		tjs_uint32 ret = _mm_cvtsi128_si32( md );
		return (ret&0x00ffffff)|dopa;
	}
	inline __m128i operator()( __m128i md1, tjs_uint32 s ) const {
		__m128i da = md1;
		da = _mm_srli_epi32( da, 24 );	// >>= 24
		__m128i opa = _mm_cvtsi32_si128( s );
		opa = _mm_unpacklo_epi8( opa, zero_ );	// 0000 | 0 s 0 s 0 s 0 s
		__m128i dopa = opa;	// 4倍して飽和してって処理長いからテーブルの方がいいかも…
		dopa = _mm_slli_epi16( dopa, 2 );			// *= 4
		dopa = _mm_packus_epi16( dopa, zero_ );		// dopa > 255 ? 255 : dopa
		dopa = _mm_unpacklo_epi8( dopa, zero_ );	// 0000 | 0 s 0 s 0 s 0 s
		dopa = _mm_unpacklo_epi16( dopa, zero_ );	// 000s 000s 000s 000s
		dopa = _mm_slli_epi32( dopa, 8 );			// <<= 8
		dopa = _mm_or_si128( dopa, da );

		opa = _mm_unpacklo_epi16( opa, zero_ );	// 000s 000s 000s 000s
		opa = _mm_slli_epi32( opa, 8 );			// <<= 8
		da = _mm_or_si128( da, opa );
		__m128i ma1 = _mm_set_epi32(
			TVPOpacityOnOpacityTable65[da.m128i_u32[3]],
			TVPOpacityOnOpacityTable65[da.m128i_u32[2]],
			TVPOpacityOnOpacityTable65[da.m128i_u32[1]],
			TVPOpacityOnOpacityTable65[da.m128i_u32[0]]);

		ma1 = _mm_packs_epi32( ma1, ma1 );		// 0 1 2 3 0 1 2 3
		ma1 = _mm_unpacklo_epi16( ma1, ma1 );	// 0 0 1 1 2 2 3 3
		__m128i ma2 = ma1;
		ma1 = _mm_unpacklo_epi16( ma1, ma1 );	// 0 0 0 0 1 1 1 1

		__m128i mc = mc_;
		__m128i md2 = md1;
		md1 = _mm_unpacklo_epi8( md1, zero_ );
		mc = _mm_sub_epi16( mc, md1 );	// mc -= md
		mc = _mm_mullo_epi16( mc, ma1 );// mc *= mo
		md1 = _mm_slli_epi16( md1, 8 );	// md <<= 8
		md1 = _mm_add_epi16( md1, mc );	// md += mc
		md1 = _mm_srli_epi16( md1, 8 );	// md >>= 8

		mc = mc_;
		ma2 = _mm_unpackhi_epi16( ma2, ma2 );	// 2 2 2 2 3 3 3 3
		md2 = _mm_unpackhi_epi8( md2, zero_ );
		mc = _mm_sub_epi16( mc, md2 );	// mc -= md
		mc = _mm_mullo_epi16( mc, ma2 );// mc *= mo
		md2 = _mm_slli_epi16( md2, 8 );	// md <<= 8
		md2 = _mm_add_epi16( md2, mc );	// md += mc
		md2 = _mm_srli_epi16( md2, 8 );	// md >>= 8

		md1 = _mm_packus_epi16( md1, md2 );

		// ( 255 - (255-a)*(255-b)/ 255 ); 
		__m128i mask = colormask_;
		mask = _mm_srli_epi32( mask, 8 );	// 0x00ffffff >> 8 = 0x0000ffff
		dopa = _mm_xor_si128( dopa, mask );	// (a = 255-a, b = 255-b) : ^=xor : 普通に8bit単位で引いても一緒か……
		__m128i mtmp = dopa;
		dopa = _mm_slli_epi32( dopa, 8 );		// 00ff|ff00	上位 << 8
		mtmp = _mm_slli_epi16( mtmp, 8 );		// 0000|ff00	下位 << 8
		mtmp = _mm_slli_epi32( mtmp, 8 );		// 00ff|0000
		dopa = _mm_mullo_epi16( dopa, mtmp );	// 上位で演算、下位部分はごみ
		dopa = _mm_srli_epi32( dopa, 16 );		// addr >> 16 | 下位を捨てる
		dopa = _mm_andnot_si128( dopa, mask );	// ~addr&0x0000ffff
		dopa = _mm_srli_epi16( dopa, 8 );		// addr>>8
		dopa = _mm_slli_epi32( dopa, 24 );		// アルファ位置へ

		md1 = _mm_and_si128( md1, colormask_ );
		return _mm_or_si128( md1, dopa );
	}
};
template<int tshift>
struct sse2_apply_color_map_xx_a_functor {
	__m128i mc_;
	__m128i color_;
	const __m128i zero_;
	inline sse2_apply_color_map_xx_a_functor( tjs_uint32 color ) : zero_(_mm_setzero_si128()) {
		mc_ = _mm_cvtsi32_si128( color&0x00ffffff );
		__m128i tmp = _mm_cvtsi32_si128( 0x100 );
		tmp = _mm_slli_epi64( tmp, 48 );	// << 48
		mc_ = _mm_unpacklo_epi8( mc_, zero_ );
		mc_ = _mm_or_si128( mc_, tmp );		// 01 00 00 co 00 co 00 co
		mc_ = _mm_shuffle_epi32( mc_, _MM_SHUFFLE( 1, 0, 1, 0 )  );
		color_ = _mm_cvtsi32_si128( color|0xff000000 );
		color_ = _mm_shuffle_epi32( color_, _MM_SHUFFLE( 0, 0, 0, 0 )  );
	}
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint8 s ) const {
		__m128i mo = _mm_cvtsi32_si128( s );
		mo = _mm_shufflelo_epi16( mo, _MM_SHUFFLE( 0, 0, 0, 0 )  );	// 00 Sa 00 Sa 00 Sa 00 Sa
		__m128i ms = mo;
		ms = _mm_mullo_epi16( ms, mc_ );		// alpha * color
		ms = _mm_srli_epi16( ms, tshift );		// 00 Sa 00 Si 00 Si 00 Si
		__m128i md = _mm_cvtsi32_si128( d );	// (DaDiDiDi)
		md = _mm_unpacklo_epi8( md, zero_ );	// 00 Da 00 Di 00 Di 00 Di
		__m128i mds = md;
		mds = _mm_mullo_epi16( mds, mo );		// dest * alpha
		mds = _mm_srli_epi16( mds, tshift );	// 00 SaDa 00 SaDi 00 SaDi 00 SaDi
		md = _mm_sub_epi16( md, mds );			// Di - SaDi
		md = _mm_add_epi16( md, ms );			// Di - SaDi + Si
		md = _mm_packus_epi16( md, zero_ );
		return _mm_cvtsi128_si32( md );
	}
	inline __m128i operator()( __m128i md1, __m128i mo1 ) const {
		__m128i md2 = md1;
		md1 = _mm_unpacklo_epi8( md1, zero_ );	// 00 Da 00 Di 00 Di 00 Di
		__m128i mo2 = mo1;
		mo1 = _mm_unpacklo_epi16( mo1, mo1 );
		mo2 = _mm_unpackhi_epi16( mo2, mo2 );

		__m128i ms = mo1;
		ms = _mm_mullo_epi16( ms, mc_ );		// alpha * color
		ms = _mm_srli_epi16( ms, tshift );		// 00 Sa 00 Si 00 Si 00 Si
		__m128i mds = md1;
		mds = _mm_mullo_epi16( mds, mo1 );		// dest * alpha
		mds = _mm_srli_epi16( mds, tshift );			// 00 SaDa 00 SaDi 00 SaDi 00 SaDi
		md1 = _mm_sub_epi16( md1, mds );		// Di - SaDi
		md1 = _mm_add_epi16( md1, ms );			// Di - SaDi + Si
		
		md2 = _mm_unpackhi_epi8( md2, zero_ );	// 00 Da 00 Di 00 Di 00 Di
		ms = mo2;
		ms = _mm_mullo_epi16( ms, mc_ );		// alpha * color
		ms = _mm_srli_epi16( ms, tshift );		// 00 Sa 00 Si 00 Si 00 Si
		mds = md2;
		mds = _mm_mullo_epi16( mds, mo2 );		// dest * alpha
		mds = _mm_srli_epi16( mds, tshift );	// 00 SaDa 00 SaDi 00 SaDi 00 SaDi
		md2 = _mm_sub_epi16( md2, mds );		// Di - SaDi
		md2 = _mm_add_epi16( md2, ms );			// Di - SaDi + Si

		return _mm_packus_epi16( md1, md2 );
	}
};

typedef sse2_apply_color_map_xx_straight_functor<sse2_apply_color_map_xx_a_functor<6> > sse2_apply_color_map65_a_functor;
typedef sse2_apply_color_map_xx_straight_functor<sse2_apply_color_map_xx_a_functor<8> > sse2_apply_color_map_a_functor;
typedef sse2_apply_color_map_xx_o_functor<sse2_apply_color_map_xx_a_functor<6> > sse2_apply_color_map65_ao_functor;
typedef sse2_apply_color_map_xx_o_functor<sse2_apply_color_map_xx_a_functor<8> > sse2_apply_color_map_ao_functor;


template<typename functor,int topaque>
static inline void apply_color_map_branch_func_sse2( tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, const functor& func ) {
	if( len <= 0 ) return;

	tjs_int count = (tjs_int)((unsigned)dest & 0xF);
	if( count ) {
		count = (16 - count)>>2;
		count = count < len ? count : count - len;
		tjs_uint32* limit = dest + count;
		while( dest < limit ) {
			*dest = func( *dest, *src );
			dest++; src++;
		}
		len -= count;
	}
	tjs_uint32 rem = (len>>2)<<2;
	tjs_uint32* limit = dest + rem;
	while( dest < limit ) {
		tjs_uint32 s = *(const tjs_uint32*)src;
		if( s == topaque ) { // completely opaque
			_mm_store_si128( (__m128i*)dest, func.color_ );
		} else if( s != 0 ) {
			__m128i md = _mm_load_si128( (__m128i const*)dest );
			_mm_store_si128( (__m128i*)dest, func( md, s ) );
		} // else { // completely transparent
		dest+=4; src+=4;
	}
	limit += (len-rem);
	while( dest < limit ) {
		*dest = func( *dest, *src );
		dest++; src++;
	}
}

template<typename functor>
static inline void apply_color_map_func_sse2( tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, const functor& func ) {
	if( len <= 0 ) return;

	tjs_int count = (tjs_int)((unsigned)dest & 0xF);
	if( count ) {
		count = (16 - count)>>2;
		count = count < len ? count : count - len;
		tjs_uint32* limit = dest + count;
		while( dest < limit ) {
			*dest = func( *dest, *src );
			dest++; src++;
		}
		len -= count;
	}
	tjs_uint32 rem = (len>>2)<<2;
	tjs_uint32* limit = dest + rem;
	while( dest < limit ) {
		tjs_uint32 s = *(const tjs_uint32*)src;
		__m128i md = _mm_load_si128( (__m128i const*)dest );
		_mm_store_si128( (__m128i*)dest, func( md, s ) );
		dest+=4; src+=4;
	}
	limit += (len-rem);
	while( dest < limit ) {
		*dest = func( *dest, *src );
		dest++; src++;
	}
}

void TVPApplyColorMap65_sse2_c(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color) {
	sse2_apply_color_map65_functor func(color);
	apply_color_map_branch_func_sse2<sse2_apply_color_map65_functor,0x40404040>( dest, src, len , func );
}
void TVPApplyColorMap_sse2_c(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color) {
	sse2_apply_color_map_functor func(color);
	apply_color_map_branch_func_sse2<sse2_apply_color_map_functor,0xffffffff>( dest, src, len , func );
}
void TVPApplyColorMap65_d_sse2_c(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color) {
	sse2_apply_color_map65_d_functor func( color );
	apply_color_map_branch_func_sse2<sse2_apply_color_map65_d_functor,0x40404040>( dest, src, len , func );
}
void TVPApplyColorMap65_a_sse2_c(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color) {
	sse2_apply_color_map65_a_functor func( color );
	apply_color_map_branch_func_sse2<sse2_apply_color_map65_a_functor,0x40404040>( dest, src, len , func );
}
void TVPApplyColorMap_a_sse2_c(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color) {
	sse2_apply_color_map_a_functor func( color );
	apply_color_map_branch_func_sse2<sse2_apply_color_map_a_functor,0xffffffff>( dest, src, len , func );
}
void TVPApplyColorMap65_o_sse2_c(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa) {
	sse2_apply_color_map65_o_functor func(color,opa);
	apply_color_map_func_sse2( dest, src, len , func );
}
void TVPApplyColorMap_o_sse2_c(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa) {
	sse2_apply_color_map_o_functor func(color,opa);
	apply_color_map_func_sse2( dest, src, len , func );
}
void TVPApplyColorMap65_ao_sse2_c(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa) {
	sse2_apply_color_map65_ao_functor func(color,opa);
	apply_color_map_func_sse2( dest, src, len , func );
}
void TVPApplyColorMap_ao_sse2_c(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa) {
	sse2_apply_color_map_ao_functor func(color,opa);
	apply_color_map_func_sse2( dest, src, len , func );
}


/*
void TVPApplyColorMap(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color); // has SSE2
void TVPApplyColorMap_o(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa); // has SSE2
void TVPApplyColorMap65(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color); // has SSE2
void TVPApplyColorMap65_o(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa); // has SSE2
void TVPApplyColorMap_HDA(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color);
void TVPApplyColorMap_HDA_o(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa);
void TVPApplyColorMap65_HDA(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color);
void TVPApplyColorMap65_HDA_o(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa);
void TVPApplyColorMap_d(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color);
void TVPApplyColorMap65_d(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color); // has SSE2
void TVPApplyColorMap_a(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color); // has SSE2
void TVPApplyColorMap65_a(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color); // has SSE2
void TVPApplyColorMap_do(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa);
void TVPApplyColorMap65_do(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa);
void TVPApplyColorMap_ao(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa); // has SSE2
void TVPApplyColorMap65_ao(tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa); // has SSE2
*/

