


#include "tjsTypes.h"
#include "tvpgl.h"
#include "tvpgl_ia32_intf.h"
#include "simd_def_x86x64.h"

//#include <windows.h>

#include "blend_functor_sse2.h"
#include "blend_ps_functor_sse2.h"
#include "interpolation_functor_sse2.h"


extern "C" {
extern tjs_uint32 TVPCPUType;
extern unsigned char TVPDivTable[256*256];
extern unsigned char TVPOpacityOnOpacityTable[256*256];
extern unsigned char TVPNegativeMulTable[256*256];
extern unsigned char TVPOpacityOnOpacityTable65[65*256];
extern unsigned char TVPNegativeMulTable65[65*256];
extern unsigned char TVPDitherTable_5_6[8][4][2][256];
extern unsigned char TVPDitherTable_676[3][4][4][256];
extern unsigned char TVP252DitherPalette[3][256];
extern tjs_uint32 TVPRecipTable256[256];
extern tjs_uint16 TVPRecipTable256_16[256];
}
// 未テスト
void TVPMakeAlphaFromKey_sse2_c(tjs_uint32 *dest, tjs_int len, tjs_uint32 key) {
	if( len <= 0 ) return;

	key &= 0x00ffffff;
	tjs_int count = (tjs_int)((unsigned)dest & 0xF);
	if( count ) {	// ここで len > 3 としてしまった方がいいかな
		count = (16 - count)>>2;
		count = count < len ? count : count - len;
		tjs_uint32* limit = dest + count;
		while( dest < limit ) {
			tjs_uint32 c = (*dest)&0x00ffffff;
			if( c != key ) c |= 0xff000000;
			*dest = c;
			dest++;
		}
		len -= count;
	}

	tjs_uint32 rem = (len>>2)<<2;
	tjs_uint32* limit = dest + rem;
	if( dest < limit ) {
		const __m128i mmkey = _mm_set1_epi32( key );
		const __m128i mmask = _mm_set1_epi32( 0x00ffffff );
		const __m128i alpha = _mm_set1_epi32( 0xff000000 );
		do {
			__m128i md = _mm_load_si128( (__m128i const*)dest );
			md = _mm_and_si128( md, mmask );	// d &= mask アルファをクリア(透明に)
			__m128i mk = mmkey;
			mk = _mm_cmpeq_epi32( mk, md );		// d == key ? 1111 : 0000
			mk = _mm_andnot_si128( mk, alpha );	// maskalpha = (^cmpmask) & alpha
			md = _mm_or_si128( md, mk );		// d |= maskalpha
			_mm_stream_si128( (__m128i*)dest, md );
			dest += 4;
		} while( dest < limit );
	}

	limit += (len-rem);
	while( dest < limit ) {
		tjs_uint32 c = (*dest)&0x00ffffff;
		if( c != key ) c |= 0xff000000;
		*dest = c;
		dest++;
	}
}


template<typename functor>
static inline void blend_func_sse2( tjs_uint32 * __restrict dest, const tjs_uint32 * __restrict src, tjs_int len, const functor& func ) {
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
	if( (((unsigned)src)&0xF) == 0 ) {
		while( dest < limit ) {
			__m128i md = _mm_load_si128( (__m128i const*)dest );
			__m128i ms = _mm_load_si128( (__m128i const*)src );
			_mm_store_si128( (__m128i*)dest, func( md, ms ) );
			dest+=4; src+=4;
		}
	} else {
		while( dest < limit ) {
			__m128i md = _mm_load_si128( (__m128i const*)dest );
			__m128i ms = _mm_loadu_si128( (__m128i const*)src );
			_mm_store_si128( (__m128i*)dest, func( md, ms ) );
			dest+=4; src+=4;
		}
	}
	limit += (len-rem);
	while( dest < limit ) {
		*dest = func( *dest, *src );
		dest++; src++;
	}
}
template<typename functor>
static void copy_func_sse2( tjs_uint32 * __restrict dest, const tjs_uint32 * __restrict src, tjs_int len ) {
	functor func;
	blend_func_sse2<functor>( dest, src, len, func );
}

// src と dest が重複している可能性のあるもの
template<typename functor>
static inline void overlap_blend_func_sse2( tjs_uint32 * dest, const tjs_uint32 * src, tjs_int len, const functor& func ) {
	if( len <= 0 ) return;
	
	const tjs_uint32 *src_end = src + len;
	if( dest > src && dest < src_end ) {
		// backward オーバーラップするので後ろからコピー
		tjs_int remain = (len>>2)<<2;	// 4未満の端数カット
		len--;
		while( len >= remain ) {
			dest[len] = func( dest[len], src[len] );
			len--;
		}
		while( len >= 0 ) {
			// 4ピクセルずつコピー, オーバーラップのケースは少ないのとCore i3/5/7以降ならアライメント関係なく速いので、アライメント気にせずコピー
			__m128i md = _mm_loadu_si128( (__m128i const*)&(dest[len-3]) );
			__m128i ms = _mm_loadu_si128( (__m128i const*)&(src[len-3]) );
			md = func( md, ms );
			_mm_storeu_si128( (__m128i*)&(dest[len-3]), md );
			len -= 4;
		}
	} else {
		// forward
		blend_func_sse2<functor>( dest, src, len, func );
	}
}
template<typename functor>
static void overlap_copy_func_sse2( tjs_uint32 * __restrict dest, const tjs_uint32 * __restrict src, tjs_int len ) {
	functor func;
	overlap_blend_func_sse2<functor>( dest, src, len, func );
}
// dest = src1 * src2 となっているもの
template<typename functor>
static inline void sd_blend_func_sse2( tjs_uint32 *dest, const tjs_uint32 *src1, const tjs_uint32 *src2, tjs_int len, const functor& func ) {
	if( len <= 0 ) return;

	tjs_int count = (tjs_int)((unsigned)dest & 0xF);
	if( count ) {
		count = (16 - count)>>2;
		count = count < len ? count : count - len;
		tjs_uint32* limit = dest + count;
		while( dest < limit ) {
			*dest = func( *src1, *src2 );
			dest++; src1++; src2++;
		}
		len -= count;
	}
	tjs_uint32 rem = (len>>2)<<2;
	tjs_uint32* limit = dest + rem;
	if( (((unsigned)src1)&0xF) == 0 && (((unsigned)src2)&0xF) == 0 ) {
		while( dest < limit ) {
			__m128i ms1 = _mm_load_si128( (__m128i const*)src1 );
			__m128i ms2 = _mm_load_si128( (__m128i const*)src2 );
			_mm_store_si128( (__m128i*)dest, func( ms1, ms2 ) );
			dest+=4; src1+=4; src2+=4;
		}
	} else {
		while( dest < limit ) {
			__m128i ms1 = _mm_loadu_si128( (__m128i const*)src1 );
			__m128i ms2 = _mm_loadu_si128( (__m128i const*)src2 );
			_mm_store_si128( (__m128i*)dest, func( ms1, ms2 ) );
			dest+=4; src1+=4; src2+=4;
		}
	}
	limit += (len-rem);
	while( dest < limit ) {
		*dest = func( *src1, *src2 );
		dest++; src1++; src2++;
	}
}


// 完全透明ではコピーせず、完全不透明はそのままコピーする
template<typename functor>
static void blend_src_branch_func_sse2( tjs_uint32 * __restrict dest, const tjs_uint32 * __restrict src, tjs_int len, const functor& func ) {
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
	const __m128i alphamask = _mm_set1_epi32(0xff000000);
	const __m128i zero = _mm_setzero_si128();
	if( (((unsigned)src)&0xF) == 0 ) {
		while( dest < limit ) {
			__m128i ms = _mm_load_si128( (__m128i const*)src );
			__m128i ma = ms;
			ma = _mm_and_si128( ma, alphamask );
			ma = _mm_cmpeq_epi32( ma, alphamask );
			if( _mm_movemask_epi8(ma) == 0xffff ) {	// totally opaque
				_mm_store_si128( (__m128i*)dest, ms );
			} else {
				ma = ms;
				ma = _mm_and_si128( ma, alphamask );
				ma = _mm_cmpeq_epi32( ma, zero );
				if( _mm_movemask_epi8(ma) != 0xffff ) {
					__m128i md = _mm_load_si128( (__m128i const*)dest );
					_mm_store_si128( (__m128i*)dest, func( md, ms ) );
				}
			}
			dest+=4; src+=4;
		}
	} else {
		while( dest < limit ) {
			__m128i ms = _mm_loadu_si128( (__m128i const*)src );
			__m128i ma = ms;
			ma = _mm_and_si128( ma, alphamask );
			ma = _mm_cmpeq_epi32( ma, alphamask );
			if( _mm_movemask_epi8(ma) == 0xffff ) {	// totally opaque
				_mm_store_si128( (__m128i*)dest, ms );
			} else {
				ma = ms;
				ma = _mm_and_si128( ma, alphamask );
				ma = _mm_cmpeq_epi32( ma, zero );
				if( _mm_movemask_epi8(ma) != 0xffff ) {
					__m128i md = _mm_load_si128( (__m128i const*)dest );
					_mm_store_si128( (__m128i*)dest, func( md, ms ) );
				}
			}
			dest+=4; src+=4;
		}
	}
	limit += (len-rem);
	while( dest < limit ) {
		*dest = func( *dest, *src );
		dest++; src++;
	}
}

template<typename functor>
static void copy_src_branch_func_sse2( tjs_uint32 * __restrict dest, const tjs_uint32 * __restrict src, tjs_int len ) {
	functor func;
	blend_src_branch_func_sse2<functor>( dest, src, len, func );
}
// 補間コピー用 16bit固定小数点
// func_t : ブレンド関数
// interp_t : 補間関数
template<typename func_t,typename interp_t>
void sse2_interpolation_line_transform_copy(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch, const func_t& func, const interp_t& inter) {
	if( len <= 0 ) return;

	const tjs_uint8* s = (tjs_uint8*)src;

	__m128i mxy1 = _mm_cvtsi32_si128( sy );			// _mm_mul_epu32 のために下をsyとする
	__m128i mx = _mm_cvtsi32_si128( sx );
	mxy1 = _mm_unpacklo_epi32( mxy1, mx );			// 0 | 0 | sx | sy
	__m128i mstep = _mm_cvtsi32_si128( stepy );
	__m128i mstepx = _mm_cvtsi32_si128( stepx );
	mstep = _mm_unpacklo_epi32( mstep, mstepx );	// 0 | 0 | stepx | stepy
	__m128i mxy = mxy1;
	mxy1 = _mm_add_epi32( mxy1, mstep );			// 1進める
	mxy = _mm_unpacklo_epi64( mxy, mxy1 );			// ++sx | ++sy | sx | sy
	mstep = _mm_unpacklo_epi64( mstep, mstep );		// stepx | stepy | stepx | stepy
	__m128i mstep1 = mstep;
	mstep = _mm_add_epi32( mstep, mstep );			// stepx*2 | stepy*2 | stepx*2 | stepy*2
	__m128i mpitch = _mm_cvtsi32_si128( srcpitch );
	mpitch = _mm_unpacklo_epi64( mpitch, mpitch );	// 0000 srcpitch 0000 srcpitch
	
	tjs_int count = (tjs_int)(((unsigned)dest)&0xF);
	if( count ) {
		count = (16 - count)>>2;
		count = count < len ? count : count - len;
		tjs_uint32* limit = dest + count;
		while( dest < limit ) {
			__m128i mxyi = mxy;
			int sy = _mm_cvtsi128_si32( mxyi );
			mxyi = _mm_srli_epi64( mxyi, 32 );
			int sx = _mm_cvtsi128_si32( mxyi );
			*dest = func( *dest, inter( s, sy, sx, srcpitch ) );
			mxy = _mm_add_epi32( mxy, mstep1 );	// sx,sy += stepx,stepy
			dest++;
		}
	}

	tjs_uint32 rem = (len>>2)<<2;
	tjs_uint32* limit = dest + rem;
	const __m128i zero = _mm_setzero_si128();
	while( dest < limit ) {
		__m128i ms1 = inter( s, mxy, mpitch, srcpitch );
		mxy = _mm_add_epi32( mxy, mstep );	// sx,sy += stepx,stepy
		__m128i ms2 = inter( s, mxy, mpitch, srcpitch );
		mxy = _mm_add_epi32( mxy, mstep );	// sx,sy += stepx,stepy
		ms1 = inter.pack( ms1, ms2 );
		//ms1 = _mm_packus_epi16( ms1, ms2 );
		__m128i md = _mm_load_si128( (__m128i const*)dest );
		_mm_store_si128( (__m128i*)dest, func( md, ms1 ) );
		dest += 4;
	}
	// 端数
	limit += (len-rem);
	while( dest < limit ) {
		__m128i mxyi = mxy;
		int sy = _mm_cvtsi128_si32( mxyi );
		mxyi = _mm_srli_epi64( mxyi, 32 );
		int sx = _mm_cvtsi128_si32( mxyi );
		*dest = func( *dest, inter( s, sy, sx, srcpitch ) );
		mxy = _mm_add_epi32( mxy, mstep1 );	// sx,sy += stepx,stepy
		dest++;
	}
}
#define DEFINE_BLEND_FUNCTION_MIN_VARIATION( NAME, FUNC ) \
static void TVP##NAME##_sse2_c( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len ) {				\
	copy_func_sse2<sse2_##FUNC##_functor>( dest, src, len );											\
}																										\
static void TVP##NAME##_HDA_sse2_c( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len ) {			\
	copy_func_sse2<sse2_##FUNC##_hda_functor>( dest, src, len );										\
}																										\
static void TVP##NAME##_o_sse2_c( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa ) {	\
	sse2_##FUNC##_o_functor func(opa);																	\
	blend_func_sse2( dest, src, len, func );															\
}																										\
static void TVP##NAME##_HDA_o_sse2_c( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa ) {	\
	sse2_##FUNC##_hda_o_functor func(opa);																\
	blend_func_sse2( dest, src, len, func );															\
}

#define DEFINE_BLEND_FUNCTION_MIN3_VARIATION( NAME, FUNC ) \
static void TVP##NAME##_sse2_c( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len ) {				\
	copy_func_sse2<sse2_##FUNC##_functor>( dest, src, len );											\
}																										\
static void TVP##NAME##_HDA_sse2_c( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len ) {			\
	copy_func_sse2<sse2_##FUNC##_hda_functor>( dest, src, len );										\
}																										\
static void TVP##NAME##_o_sse2_c( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa ) {	\
	sse2_##FUNC##_o_functor func(opa);																	\
	blend_func_sse2( dest, src, len, func );															\
}

#define DEFINE_BLEND_FUNCTION_MIN2_VARIATION( NAME, FUNC ) \
static void TVP##NAME##_sse2_c( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len ) {				\
	copy_func_sse2<sse2_##FUNC##_functor>( dest, src, len );											\
}																										\
static void TVP##NAME##_HDA_sse2_c( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len ) {			\
	copy_func_sse2<sse2_##FUNC##_hda_functor>( dest, src, len );										\
}

//DEFINE_BLEND_FUNCTION_MIN_VARIATION( AlphaBlend, alpha_blend )
// AlphaBlendはソースが完全透明/不透明で分岐する特殊版を使うので、個別に書く
static void TVPAlphaBlend_sse2_c( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len ) {
	copy_src_branch_func_sse2<sse2_alpha_blend_functor>( dest, src, len );
}
static void TVPAlphaBlend_HDA_sse2_c( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len ) {
	copy_func_sse2<sse2_alpha_blend_hda_functor>( dest, src, len );
}
static void TVPAlphaBlend_o_sse2_c( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa ) {
	sse2_alpha_blend_o_functor func(opa);
	blend_func_sse2( dest, src, len, func );
}
static void TVPAlphaBlend_HDA_o_sse2_c( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa ) {
	sse2_alpha_blend_hda_o_functor func(opa);
	blend_func_sse2( dest, src, len, func );
}
static void TVPAlphaBlend_d_sse2_c( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len ) {
	copy_src_branch_func_sse2<sse2_alpha_blend_d_functor>( dest, src, len );
}

static void TVPConstAlphaBlend_SD_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src1, const tjs_uint32 *src2, tjs_int len, tjs_int opa){
	sse2_const_alpha_blend_functor func(opa);
	sd_blend_func_sse2( dest, src1, src2, len, func );
}
static void TVPCopyColor_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len) {
	overlap_copy_func_sse2<sse2_color_copy_functor>( dest, src, len );
}
static void TVPCopyMask_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len) {
	overlap_copy_func_sse2<sse2_alpha_copy_functor>( dest, src, len );
}
static void TVPCopyOpaqueImage_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len) {
	copy_func_sse2<sse2_color_opaque_functor>( dest, src, len );
}
static void TVPConstAlphaBlend_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa) {
	sse2_const_alpha_blend_functor func(opa);
	blend_func_sse2( dest, src, len, func );
}
static void TVPConstAlphaBlend_HDA_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa) {
	sse2_const_alpha_blend_hda_functor func(opa);
	blend_func_sse2( dest, src, len, func );
}
static void TVPConstAlphaBlend_d_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa) {
	sse2_const_alpha_blend_d_functor func(opa);
	blend_func_sse2( dest, src, len, func );
}
static void TVPConstAlphaBlend_a_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa) {
	sse2_const_alpha_blend_a_functor func(opa);
	blend_func_sse2( dest, src, len, func );
}


static void TVPAdditiveAlphaBlend_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len){
	copy_func_sse2<sse2_premul_alpha_blend_functor>( dest, src, len );
}
static void TVPAdditiveAlphaBlend_o_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa){
	sse2_premul_alpha_blend_o_functor func(opa);
	blend_func_sse2( dest, src, len, func );
}
static void TVPAdditiveAlphaBlend_HDA_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len){
	copy_func_sse2<sse2_premul_alpha_blend_hda_functor>( dest, src, len );
}
static void TVPAdditiveAlphaBlend_a_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len){
	copy_func_sse2<sse2_premul_alpha_blend_a_functor>( dest, src, len );
}

DEFINE_BLEND_FUNCTION_MIN3_VARIATION( AddBlend, add_blend )
DEFINE_BLEND_FUNCTION_MIN3_VARIATION( SubBlend, sub_blend )
DEFINE_BLEND_FUNCTION_MIN_VARIATION( MulBlend, mul_blend )
DEFINE_BLEND_FUNCTION_MIN2_VARIATION( LightenBlend, lighten_blend )
DEFINE_BLEND_FUNCTION_MIN2_VARIATION( DarkenBlend, darken_blend )
DEFINE_BLEND_FUNCTION_MIN_VARIATION( ScreenBlend, screen_blend )


DEFINE_BLEND_FUNCTION_MIN_VARIATION( PsAlphaBlend, ps_alpha_blend )
DEFINE_BLEND_FUNCTION_MIN_VARIATION( PsAddBlend, ps_add_blend )
DEFINE_BLEND_FUNCTION_MIN_VARIATION( PsSubBlend, ps_sub_blend )
DEFINE_BLEND_FUNCTION_MIN_VARIATION( PsMulBlend, ps_mul_blend )
DEFINE_BLEND_FUNCTION_MIN_VARIATION( PsScreenBlend, ps_screen_blend )
DEFINE_BLEND_FUNCTION_MIN_VARIATION( PsOverlayBlend, ps_overlay_blend )
DEFINE_BLEND_FUNCTION_MIN_VARIATION( PsHardLightBlend, ps_hardlight_blend )
DEFINE_BLEND_FUNCTION_MIN_VARIATION( PsSoftLightBlend, ps_softlight_blend )
DEFINE_BLEND_FUNCTION_MIN_VARIATION( PsColorDodgeBlend, ps_colordodge_blend )
DEFINE_BLEND_FUNCTION_MIN_VARIATION( PsColorBurnBlend, ps_colorburn_blend )
DEFINE_BLEND_FUNCTION_MIN_VARIATION( PsColorDodge5Blend, ps_colordodge5_blend )
DEFINE_BLEND_FUNCTION_MIN_VARIATION( PsLightenBlend, ps_lighten_blend )
DEFINE_BLEND_FUNCTION_MIN_VARIATION( PsDarkenBlend, ps_darken_blend )
DEFINE_BLEND_FUNCTION_MIN_VARIATION( PsDiffBlend, ps_diff_blend )
DEFINE_BLEND_FUNCTION_MIN_VARIATION( PsDiff5Blend, ps_diff5_blend )
DEFINE_BLEND_FUNCTION_MIN_VARIATION( PsExclusionBlend, ps_exclusion_blend )


/// アフィン変換
#define DEF_NEAREST( blend_func )	\
sse2_nearest_functor inter;	\
blend_func func;					\
sse2_interpolation_line_transform_copy( dest, len, src, sx, sy, stepx, stepy, srcpitch, func, inter);

#define DEF_NEAREST_OPA( blend_func )	\
sse2_nearest_functor inter;		\
blend_func func(opa);					\
sse2_interpolation_line_transform_copy( dest, len, src, sx, sy, stepx, stepy, srcpitch, func, inter);

void TVPLinTransAlphaBlend_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch){
	DEF_NEAREST( sse2_alpha_blend_functor );
}
void TVPLinTransAlphaBlend_HDA_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch){
	DEF_NEAREST( sse2_alpha_blend_hda_functor );
}
void TVPLinTransAlphaBlend_o_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch, tjs_int opa){
	DEF_NEAREST_OPA( sse2_alpha_blend_o_functor );
}
void TVPLinTransAlphaBlend_HDA_o_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch, tjs_int opa){
	DEF_NEAREST_OPA( sse2_alpha_blend_hda_o_functor );
}
void TVPLinTransAlphaBlend_d_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch){
	DEF_NEAREST( sse2_alpha_blend_d_functor )
}
void TVPLinTransAlphaBlend_a_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch){
	DEF_NEAREST( sse2_alpha_blend_a_functor )
}
//void TVPLinTransAlphaBlend_do_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch, tjs_int opa){}
//void TVPLinTransAlphaBlend_ao_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch, tjs_int opa){}
void TVPLinTransAdditiveAlphaBlend_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch){
	DEF_NEAREST( sse2_premul_alpha_blend_functor )
}
void TVPLinTransAdditiveAlphaBlend_HDA_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch){
	DEF_NEAREST( sse2_premul_alpha_blend_hda_functor )
}
void TVPLinTransAdditiveAlphaBlend_o_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch, tjs_int opa){
	DEF_NEAREST_OPA( sse2_premul_alpha_blend_o_functor );
}
//void TVPLinTransAdditiveAlphaBlend_HDA_o_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch, tjs_int opa){}
void TVPLinTransAdditiveAlphaBlend_a_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch){
	DEF_NEAREST( sse2_premul_alpha_blend_a_functor );
}
//void TVPLinTransAdditiveAlphaBlend_ao_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch, tjs_int opa){}

void TVPLinTransCopyOpaqueImage_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch) {
	DEF_NEAREST( sse2_color_opaque_functor );
}
void TVPLinTransCopy_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch) {
	DEF_NEAREST( sse2_const_copy_functor );
}
void TVPLinTransColorCopy_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch) {
	DEF_NEAREST( sse2_const_copy_functor );
}
void TVPLinTransConstAlphaBlend_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src,  tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch, tjs_int opa) {
	DEF_NEAREST_OPA( sse2_const_alpha_blend_functor );
}
// HDA版
void TVPLinTransConstAlphaBlend_HDA_sse2_c( tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch, tjs_int opa ) {
	DEF_NEAREST_OPA( sse2_const_alpha_blend_hda_functor );
}
// dest alpha を使う版
void TVPLinTransConstAlphaBlend_d_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch, tjs_int opa) {
	DEF_NEAREST_OPA( sse2_const_alpha_blend_d_functor );
}
// dest が premul alpha 版
void TVPLinTransConstAlphaBlend_a_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src, tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch, tjs_int opa) {
	DEF_NEAREST_OPA( sse2_const_alpha_blend_a_functor );
}
//sse2_premul_alpha_blend_a_functor( d, (s&0x00ffffff) | opa<<24 )

/*
TVPLinTransCopyOpaqueImage_c
TVPLinTransCopy_sse2_c
TVPLinTransColorCopy_sse2_c
opa
TVPLinTransConstAlphaBlend_sse2_c
TVPLinTransConstAlphaBlend_HDA_sse2_c
TVPLinTransConstAlphaBlend_d_sse2_c
TVPLinTransConstAlphaBlend_a_sse2_c
*/

#define DEF_BILINEAR( blend_func )	\
sse2_bilinear_functor inter;		\
blend_func func;					\
sse2_interpolation_line_transform_copy( dest, len, src, sx, sy, stepx, stepy, srcpitch, func, inter);

#define DEF_BILINEAR_OPA( blend_func )	\
sse2_bilinear_functor inter;			\
blend_func func(opa);					\
sse2_interpolation_line_transform_copy( dest, len, src, sx, sy, stepx, stepy, srcpitch, func, inter);

void TVPInterpLinTransCopy_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src,  tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch) {
	DEF_BILINEAR( sse2_const_copy_functor )
}
void TVPInterpLinTransConstAlphaBlend_sse2_c(tjs_uint32 *dest, tjs_int len, const tjs_uint32 *src,  tjs_int sx, tjs_int sy, tjs_int stepx, tjs_int stepy, tjs_int srcpitch, tjs_int opa) {
	DEF_BILINEAR_OPA( sse2_const_alpha_blend_functor )
}

extern void TVPUnivTransBlend_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src1, const tjs_uint32 *src2, const tjs_uint8 *rule, const tjs_uint32 *table, tjs_int len);
extern void TVPUnivTransBlend_switch_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src1, const tjs_uint32 *src2, const tjs_uint8 *rule, const tjs_uint32 *table, tjs_int len, tjs_int src1lv, tjs_int src2lv);
extern void TVPUnivTransBlend_d_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src1, const tjs_uint32 *src2, const tjs_uint8 *rule, const tjs_uint32 *table, tjs_int len);
extern void TVPUnivTransBlend_switch_d_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src1, const tjs_uint32 *src2, const tjs_uint8 *rule, const tjs_uint32 *table, tjs_int len, tjs_int src1lv, tjs_int src2lv);

extern void TVPInitGammaAdjustTempData_sse2_c( tTVPGLGammaAdjustTempData *temp, const tTVPGLGammaAdjustData *data );
extern void TVPAdjustGamma_a_sse2_c(tjs_uint32 *dest, tjs_int len, tTVPGLGammaAdjustTempData *param);

extern void TVPAddSubVertSum16_sse2_c(tjs_uint16 *dest, const tjs_uint32 *addline, const tjs_uint32 *subline, tjs_int len);
extern void TVPAddSubVertSum16_d_sse2_c(tjs_uint16 *dest, const tjs_uint32 *addline, const tjs_uint32 *subline, tjs_int len);
extern void TVPAddSubVertSum32_sse2_c(tjs_uint32 *dest, const tjs_uint32 *addline, const tjs_uint32 *subline, tjs_int len);
extern void TVPAddSubVertSum32_d_sse2_c(tjs_uint32 *dest, const tjs_uint32 *addline, const tjs_uint32 *subline, tjs_int len);
extern void TVPDoBoxBlurAvg16_sse2_c(tjs_uint32 *dest, tjs_uint16 *sum, const tjs_uint16 * add, const tjs_uint16 * sub, tjs_int n, tjs_int len);
extern void TVPDoBoxBlurAvg16_d_sse2_c(tjs_uint32 *dest, tjs_uint16 *sum, const tjs_uint16 * add, const tjs_uint16 * sub, tjs_int n, tjs_int len);
extern void TVPDoBoxBlurAvg32_sse2_c(tjs_uint32 *dest, tjs_uint32 *sum, const tjs_uint32 * add, const tjs_uint32 * sub, tjs_int n, tjs_int len);
extern void TVPDoBoxBlurAvg32_d_sse2_c(tjs_uint32 *dest, tjs_uint32 *sum, const tjs_uint32 * add, const tjs_uint32 * sub, tjs_int n, tjs_int len);



void TVPGL_SSE2_Init() {
	if( TVPCPUType & TVP_CPU_HAS_SSE2 ) {
		TVPAdditiveAlphaBlend = TVPAdditiveAlphaBlend_sse2_c;
		TVPAdditiveAlphaBlend_o = TVPAdditiveAlphaBlend_o_sse2_c;
		TVPAdditiveAlphaBlend_HDA = TVPAdditiveAlphaBlend_HDA_sse2_c;
		TVPAdditiveAlphaBlend_a = TVPAdditiveAlphaBlend_a_sse2_c;
		// TVPAdditiveAlphaBlend_ao

		TVPAddBlend =  TVPAddBlend_sse2_c;
		TVPAddBlend_HDA =  TVPAddBlend_HDA_sse2_c;
		TVPAddBlend_o =  TVPAddBlend_o_sse2_c;
		TVPAddBlend_HDA_o =  TVPAddBlend_o_sse2_c;

		TVPAlphaBlend =  TVPAlphaBlend_sse2_c;
		TVPAlphaBlend_o =  TVPAlphaBlend_o_sse2_c;
		TVPAlphaBlend_HDA =  TVPAlphaBlend_HDA_sse2_c;
		TVPAlphaBlend_d =  TVPAlphaBlend_d_sse2_c;
		// TVPAlphaBlend_a
		// TVPAlphaBlend_do
		// TVPAlphaBlend_ao
		TVPConstAlphaBlend =  TVPConstAlphaBlend_sse2_c;
		TVPConstAlphaBlend_HDA = TVPConstAlphaBlend_HDA_sse2_c;
		TVPConstAlphaBlend_d = TVPConstAlphaBlend_d_sse2_c;
		TVPConstAlphaBlend_a = TVPConstAlphaBlend_a_sse2_c;

		TVPConstAlphaBlend_SD =  TVPConstAlphaBlend_SD_sse2_c;
		TVPConstAlphaBlend_SD_a = TVPConstAlphaBlend_SD_sse2_c;

		TVPCopyColor = TVPCopyColor_sse2_c;
		TVPCopyMask = TVPCopyMask_sse2_c;
		TVPCopyOpaqueImage = TVPCopyOpaqueImage_sse2_c;

		TVPDarkenBlend =  TVPDarkenBlend_sse2_c;
		TVPDarkenBlend_HDA =  TVPDarkenBlend_HDA_sse2_c;

		TVPLightenBlend =  TVPLightenBlend_sse2_c;
		TVPLightenBlend_HDA =  TVPLightenBlend_HDA_sse2_c;

		TVPMulBlend =  TVPMulBlend_sse2_c;
		TVPMulBlend_HDA =  TVPMulBlend_HDA_sse2_c;
		TVPMulBlend_o =  TVPMulBlend_o_sse2_c;
		TVPMulBlend_HDA_o =  TVPMulBlend_HDA_o_sse2_c;
		
		TVPScreenBlend =  TVPScreenBlend_sse2_c;
		TVPScreenBlend_HDA =  TVPScreenBlend_HDA_sse2_c;
		TVPScreenBlend_o =  TVPScreenBlend_o_sse2_c;
		TVPScreenBlend_HDA_o =  TVPScreenBlend_HDA_o_sse2_c;

		TVPSubBlend = TVPSubBlend_sse2_c;
		TVPSubBlend_HDA = TVPSubBlend_HDA_sse2_c;
		TVPSubBlend_o = TVPSubBlend_o_sse2_c;
		TVPSubBlend_HDA_o = TVPSubBlend_o_sse2_c;

		TVPPsAlphaBlend =  TVPPsAlphaBlend_sse2_c;
		TVPPsAlphaBlend_o =  TVPPsAlphaBlend_o_sse2_c;
		TVPPsAlphaBlend_HDA =  TVPPsAlphaBlend_HDA_sse2_c;
		TVPPsAlphaBlend_HDA_o =  TVPPsAlphaBlend_HDA_o_sse2_c;
		TVPPsAddBlend =  TVPPsAddBlend_sse2_c;
		TVPPsAddBlend_o =  TVPPsAddBlend_o_sse2_c;
		TVPPsAddBlend_HDA =  TVPPsAddBlend_HDA_sse2_c;
		TVPPsAddBlend_HDA_o =  TVPPsAddBlend_HDA_o_sse2_c;	
		TVPPsSubBlend =  TVPPsSubBlend_sse2_c;
		TVPPsSubBlend_o =  TVPPsSubBlend_o_sse2_c;
		TVPPsSubBlend_HDA =  TVPPsSubBlend_HDA_sse2_c;
		TVPPsSubBlend_HDA_o =  TVPPsSubBlend_HDA_o_sse2_c;
		TVPPsMulBlend =  TVPPsMulBlend_sse2_c;
		TVPPsMulBlend_o =  TVPPsMulBlend_o_sse2_c;
		TVPPsMulBlend_HDA =  TVPPsMulBlend_HDA_sse2_c;
		TVPPsMulBlend_HDA_o =  TVPPsMulBlend_HDA_o_sse2_c;
		TVPPsScreenBlend =  TVPPsScreenBlend_sse2_c;
		TVPPsScreenBlend_o =  TVPPsScreenBlend_o_sse2_c;
		TVPPsScreenBlend_HDA =  TVPPsScreenBlend_HDA_sse2_c;
		TVPPsScreenBlend_HDA_o =  TVPPsScreenBlend_HDA_o_sse2_c;
		TVPPsOverlayBlend =  TVPPsOverlayBlend_sse2_c;
		TVPPsOverlayBlend_o =  TVPPsOverlayBlend_o_sse2_c;
		TVPPsOverlayBlend_HDA =  TVPPsOverlayBlend_HDA_sse2_c;
		TVPPsOverlayBlend_HDA_o =  TVPPsOverlayBlend_HDA_o_sse2_c;
		TVPPsHardLightBlend =  TVPPsHardLightBlend_sse2_c;
		TVPPsHardLightBlend_o =  TVPPsHardLightBlend_o_sse2_c;
		TVPPsHardLightBlend_HDA =  TVPPsHardLightBlend_HDA_sse2_c;
		TVPPsHardLightBlend_HDA_o =  TVPPsHardLightBlend_HDA_o_sse2_c;
		TVPPsSoftLightBlend =  TVPPsSoftLightBlend_sse2_c;
		TVPPsSoftLightBlend_o =  TVPPsSoftLightBlend_o_sse2_c;
		TVPPsSoftLightBlend_HDA =  TVPPsSoftLightBlend_HDA_sse2_c;
		TVPPsSoftLightBlend_HDA_o =  TVPPsSoftLightBlend_HDA_o_sse2_c;
		TVPPsColorDodgeBlend =  TVPPsColorDodgeBlend_sse2_c;
		TVPPsColorDodgeBlend_o =  TVPPsColorDodgeBlend_o_sse2_c;
		TVPPsColorDodgeBlend_HDA =  TVPPsColorDodgeBlend_HDA_sse2_c;
		TVPPsColorDodgeBlend_HDA_o =  TVPPsColorDodgeBlend_HDA_o_sse2_c;
		TVPPsColorDodge5Blend =  TVPPsColorDodge5Blend_sse2_c;
		TVPPsColorDodge5Blend_o =  TVPPsColorDodge5Blend_o_sse2_c;
		TVPPsColorDodge5Blend_HDA =  TVPPsColorDodge5Blend_HDA_sse2_c;
		TVPPsColorDodge5Blend_HDA_o =  TVPPsColorDodge5Blend_HDA_o_sse2_c;
		TVPPsColorBurnBlend =  TVPPsColorBurnBlend_sse2_c;
		TVPPsColorBurnBlend_o =  TVPPsColorBurnBlend_o_sse2_c;
		TVPPsColorBurnBlend_HDA =  TVPPsColorBurnBlend_HDA_sse2_c;
		TVPPsColorBurnBlend_HDA_o =  TVPPsColorBurnBlend_HDA_o_sse2_c;
		TVPPsLightenBlend =  TVPPsLightenBlend_sse2_c;
		TVPPsLightenBlend_o =  TVPPsLightenBlend_o_sse2_c;
		TVPPsLightenBlend_HDA =  TVPPsLightenBlend_HDA_sse2_c;
		TVPPsLightenBlend_HDA_o =  TVPPsLightenBlend_HDA_o_sse2_c;
		TVPPsDarkenBlend =  TVPPsDarkenBlend_sse2_c;
		TVPPsDarkenBlend_o =  TVPPsDarkenBlend_o_sse2_c;
		TVPPsDarkenBlend_HDA =  TVPPsDarkenBlend_HDA_sse2_c;
		TVPPsDarkenBlend_HDA_o =  TVPPsDarkenBlend_HDA_o_sse2_c;
		TVPPsDiffBlend =  TVPPsDiffBlend_sse2_c;
		TVPPsDiffBlend_o =  TVPPsDiffBlend_o_sse2_c;
		TVPPsDiffBlend_HDA =  TVPPsDiffBlend_HDA_sse2_c;
		TVPPsDiffBlend_HDA_o =  TVPPsDiffBlend_HDA_o_sse2_c;
		TVPPsDiff5Blend =  TVPPsDiff5Blend_sse2_c;
		TVPPsDiff5Blend_o =  TVPPsDiff5Blend_o_sse2_c;
		TVPPsDiff5Blend_HDA =  TVPPsDiff5Blend_HDA_sse2_c;
		TVPPsDiff5Blend_HDA_o =  TVPPsDiff5Blend_HDA_o_sse2_c;
		TVPPsExclusionBlend =  TVPPsExclusionBlend_sse2_c;
		TVPPsExclusionBlend_o =  TVPPsExclusionBlend_o_sse2_c;
		TVPPsExclusionBlend_HDA =  TVPPsExclusionBlend_HDA_sse2_c;
		TVPPsExclusionBlend_HDA_o =  TVPPsExclusionBlend_HDA_o_sse2_c;

		// SSE2版はアルファもブレンドしているので、どちらでも行ける
		TVPUnivTransBlend = TVPUnivTransBlend_sse2_c;
		TVPUnivTransBlend_a = TVPUnivTransBlend_sse2_c;
		TVPUnivTransBlend_d = TVPUnivTransBlend_d_sse2_c;
		TVPUnivTransBlend_switch = TVPUnivTransBlend_switch_sse2_c;
		TVPUnivTransBlend_switch_a = TVPUnivTransBlend_switch_sse2_c;
		TVPUnivTransBlend_switch_d = TVPUnivTransBlend_switch_d_sse2_c;

		if( TVPCPUType & TVP_CPU_HAS_SSE ) {
			TVPInitGammaAdjustTempData = TVPInitGammaAdjustTempData_sse2_c;
		}
		TVPAdjustGamma_a = TVPAdjustGamma_a_sse2_c;	// 逆数テーブルを使用しない方法にするとSSEも使う

		// アフィン変換用
		TVPLinTransAlphaBlend = TVPLinTransAlphaBlend_sse2_c;
		TVPLinTransAlphaBlend_HDA = TVPLinTransAlphaBlend_HDA_sse2_c;
		TVPLinTransAlphaBlend_o = TVPLinTransAlphaBlend_o_sse2_c;
		TVPLinTransAlphaBlend_HDA_o = TVPLinTransAlphaBlend_HDA_o_sse2_c;
		TVPLinTransAlphaBlend_d = TVPLinTransAlphaBlend_d_sse2_c;
		TVPLinTransAlphaBlend_a = TVPLinTransAlphaBlend_a_sse2_c;
		TVPLinTransAdditiveAlphaBlend = TVPLinTransAdditiveAlphaBlend_sse2_c;
		TVPLinTransAdditiveAlphaBlend_HDA = TVPLinTransAdditiveAlphaBlend_HDA_sse2_c;
		TVPLinTransAdditiveAlphaBlend_o = TVPLinTransAdditiveAlphaBlend_o_sse2_c;
		TVPLinTransAdditiveAlphaBlend_a = TVPLinTransAdditiveAlphaBlend_a_sse2_c;
		TVPLinTransCopyOpaqueImage = TVPLinTransCopyOpaqueImage_sse2_c;
		TVPLinTransCopy = TVPLinTransCopy_sse2_c;
		TVPLinTransColorCopy = TVPLinTransColorCopy_sse2_c;
		TVPLinTransConstAlphaBlend = TVPLinTransConstAlphaBlend_sse2_c;
		TVPLinTransConstAlphaBlend_HDA = TVPLinTransConstAlphaBlend_HDA_sse2_c;
		TVPLinTransConstAlphaBlend_d = TVPLinTransConstAlphaBlend_d_sse2_c;
		TVPLinTransConstAlphaBlend_a = TVPLinTransConstAlphaBlend_a_sse2_c;
		TVPInterpLinTransCopy = TVPInterpLinTransCopy_sse2_c;
		TVPInterpLinTransConstAlphaBlend = TVPInterpLinTransConstAlphaBlend_sse2_c;

		// Box Blur ( TODO 実環境で要テスト)
		TVPAddSubVertSum16 = TVPAddSubVertSum16_sse2_c;
		TVPAddSubVertSum16_d = TVPAddSubVertSum16_d_sse2_c;
		TVPAddSubVertSum32 = TVPAddSubVertSum32_sse2_c;
		TVPAddSubVertSum32_d = TVPAddSubVertSum32_d_sse2_c;
		TVPDoBoxBlurAvg16 = TVPDoBoxBlurAvg16_sse2_c;
		TVPDoBoxBlurAvg32 = TVPDoBoxBlurAvg32_sse2_c;
		if( TVPCPUType & TVP_CPU_HAS_SSE ) {
			TVPDoBoxBlurAvg16_d = TVPDoBoxBlurAvg16_d_sse2_c;	// SSE
			TVPDoBoxBlurAvg32_d = TVPDoBoxBlurAvg32_d_sse2_c;	// SSE
		}
	}
}

//#define SIMD_BLEND_TEST
#ifdef SIMD_BLEND_TEST

#include <stdio.h>
#include <math.h>
enum CheckType {
	CT_NOARMAL,
	CT_OPA,
	CT_SD,
};
void start_test( bool ischeckalpha, void* blend_a, void* blend_b, int check_type = CT_NOARMAL, tjs_int opa = 255 ) {
	tjs_uint32 aux;
	//unsigned long* src = _aligned_malloc( 256*256, 32 );
	tjs_uint32* src = (tjs_uint32*)_mm_malloc( 256*256*sizeof(tjs_uint32), 32 );
	tjs_uint32* src2 = (tjs_uint32*)_mm_malloc( 256*256*sizeof(tjs_uint32), 32 );
	tjs_uint32* dst = (tjs_uint32*)_mm_malloc( 256*256*sizeof(tjs_uint32), 32 );
	tjs_uint32* dst1 = (tjs_uint32*)_mm_malloc( 256*256*sizeof(tjs_uint32), 32 );
	tjs_uint32* dst2 = (tjs_uint32*)_mm_malloc( 256*256*sizeof(tjs_uint32), 32 );

	unsigned __int64 func_b_total = 0;
	unsigned __int64 func_a_total = 0;
	unsigned char da = 0;
	for( int dai = 0; dai < 256; dai++ ) {	// destination alpha
		da += 11;
		unsigned char sa = 0;
		int i = 0;
		for( int sai = 0; sai < 256; sai++ ) {	// source alpha
			sa += 7;
//			sa = 0;

			unsigned char sr = 0;
			unsigned char sg = 42;
			unsigned char sb = 84;
			unsigned char dr = 126;
			unsigned char dg = 168;
			unsigned char db = 210;
			for( int color = 0; color < 256; color++ ) {
				tjs_uint32 sc = ((tjs_uint32)sa) << 24 | ((tjs_uint32)sr) << 16 | ((tjs_uint32)sg) << 8 | ((tjs_uint32)sb);
				tjs_uint32 dc = ((tjs_uint32)da) << 24 | ((tjs_uint32)dr) << 16 | ((tjs_uint32)dg) << 8 | ((tjs_uint32)db);
				src[i] = src2[i] = sc;
				dst[i] = dst1[i] = dst2[i] = dc;

				sr += 1;
				sg += 3;
				sb += 5;
				dr += 7;
				dg += 11;
				db += 13;
				i++;
			}
		}
		if( check_type == CT_NOARMAL ) {
			void (*FUNC_A)(tjs_uint32*,const tjs_uint32*,tjs_int) = (void (*)(tjs_uint32*,const tjs_uint32*,tjs_int))blend_a;
			void (*FUNC_B)(tjs_uint32*,const tjs_uint32*,tjs_int) = (void (*)(tjs_uint32*,const tjs_uint32*,tjs_int))blend_b;
			// blend src + dst
			unsigned __int64 start = __rdtscp(&aux);
			FUNC_B( dst1, src, 256*256 );
			unsigned __int64 end0 = __rdtscp(&aux);
			FUNC_A( dst2, src2, 256*256 );
			unsigned __int64 end1 = __rdtscp(&aux);
			func_b_total += end0-start;
			func_a_total += end1-end0;
		} else if( check_type == CT_OPA ) {
			void (*FUNC_A_O)(tjs_uint32*,const tjs_uint32*,tjs_int,tjs_int) = (void (*)(tjs_uint32*,const tjs_uint32*,tjs_int,tjs_int))blend_a;
			void (*FUNC_B_O)(tjs_uint32*,const tjs_uint32*,tjs_int,tjs_int) = (void (*)(tjs_uint32*,const tjs_uint32*,tjs_int,tjs_int))blend_b;
			unsigned __int64 start = __rdtscp(&aux);
			FUNC_B_O( dst1, src, 256*256, opa );
			unsigned __int64 end0 = __rdtscp(&aux);
			FUNC_A_O( dst2, src2, 256*256, opa );
			unsigned __int64 end1 = __rdtscp(&aux);
			func_b_total += end0-start;
			func_a_total += end1-end0;
		} else if( check_type == CT_SD ) {
			void (*FUNC_A_O)(tjs_uint32*,const tjs_uint32*,const tjs_uint32*,tjs_int,tjs_int) = (void (*)(tjs_uint32*,const tjs_uint32*,const tjs_uint32*,tjs_int,tjs_int))blend_a;
			void (*FUNC_B_O)(tjs_uint32*,const tjs_uint32*,const tjs_uint32*,tjs_int,tjs_int) = (void (*)(tjs_uint32*,const tjs_uint32*,const tjs_uint32*,tjs_int,tjs_int))blend_b;
			unsigned __int64 start = __rdtscp(&aux);
			FUNC_B_O( dst1, dst1, src, 256*256, opa );
			unsigned __int64 end0 = __rdtscp(&aux);
			FUNC_A_O( dst2, dst2, src2, 256*256, opa );
			unsigned __int64 end1 = __rdtscp(&aux);
			func_b_total += end0-start;
			func_a_total += end1-end0;
		}
		
#ifdef _DEBUG
		// check
		for( int j = 0; j < 256*256; j++ ) {
			int a1 = (dst1[j]>>24)&0xff;
			int r1 = (dst1[j]>>16)&0xff;
			int g1 = (dst1[j]>> 8)&0xff;
			int b1 = (dst1[j]>> 0)&0xff;

			int a2 = (dst2[j]>>24)&0xff;
			int r2 = (dst2[j]>>16)&0xff;
			int g2 = (dst2[j]>> 8)&0xff;
			int b2 = (dst2[j]>> 0)&0xff;

			int adiff = abs( a1 - a2 );
			int rdiff = abs( r1 - r2 );
			int gdiff = abs( g1 - g2 );
			int bdiff = abs( b1 - b2 );
			// check color
			const int range = 3;
			if( /*(a1!=0)&&(a2!=0)&&*/(rdiff | gdiff | bdiff) ) {
				// src[j], dst[j] is faild.
				if( rdiff < range && gdiff < range && bdiff < range ) {	// warn. 1しか差がないので誤差レベル
				} else {
					printf("invalid color src : 0x%08x, dst : 0x%08x\n", src[j], dst[j] );
					printf( "  c 0x%08x\n", dst1[j] );
					printf( "asm 0x%08x\n", dst2[j] );
					int start = (j>>2)<<2;
					int limit = start + 4;
					printf("src : " );
					for( int k = start; k < limit; k++ ) {
						printf("0x%08x,", src[k] );
					}
					printf("\ndst : " );
					for( int k = start; k < limit; k++ ) {
						printf("0x%08x,", dst[k] );
					}
					printf("\noffset : %d\n", j - start );
				}
			}
			if( ischeckalpha && adiff ) {
				// src[j], dst[j] is faild.
				if( adiff < range) {	// warn. 誤差レベル
				} else {
					printf("invalid alpha src : 0x%08x, dst : 0x%08x\n", src[j], dst[j] );
					printf( "  c 0x%08x\n", dst1[j] );
					printf( "asm 0x%08x\n", dst2[j] );
				}
			}
		}
#endif
	}
	printf( "sse2 %lld : ", func_b_total );
	printf( "mmx %lld : ", func_a_total );
	printf( "rate %lld\n", (func_b_total)*100/(func_a_total) );

	_mm_free( src );
	_mm_free( src2 );
	_mm_free( dst );
	_mm_free( dst1 );
	_mm_free( dst2 );
}
#define TEST_FUNC( name ) \
	printf( "%s\n", #name );	\
	start_test(false,name##_mmx_a,name##_sse2_c); \
//	start_test(false,name##_o_mmx_a,name##_o_sse2_c,CT_OPA,0); \
//	start_test(false,name##_o_mmx_a,name##_o_sse2_c,CT_OPA,128); \
//	start_test(false,name##_o_mmx_a,name##_o_sse2_c,CT_OPA,77); \
//	start_test(false,name##_o_mmx_a,name##_o_sse2_c,CT_OPA,255);

#define TEST_FUNC_C( name ) \
	printf( "%s\n", #name );	\
	start_test(false,name##,name##_sse2_c);

/*
#define TEST_FUNC_O( name ) \
	printf( "%s\n", #name );	\
	for( tjs_int i = 0; i < 256; i++ ) {\
	start_test(false,name##_o_mmx_a,name##_o_sse2_c,CT_OPA,i); \
	}
*/
#define TEST_FUNC_O( name ) \
	printf( "%s_o\n", #name );	\
	start_test(false,name##_o_mmx_a,name##_o_sse2_c,CT_OPA,0); \
	start_test(false,name##_o_mmx_a,name##_o_sse2_c,CT_OPA,128); \
	start_test(false,name##_o_mmx_a,name##_o_sse2_c,CT_OPA,77); \
	start_test(false,name##_o_mmx_a,name##_o_sse2_c,CT_OPA,255);

#define TEST_FUNC_OA( name ) \
	printf( "%s_o\n", #name );	\
	start_test(true,name##_o_mmx_a,name##_o_sse2_c,CT_OPA,64); \
	start_test(true,name##_o_mmx_a,name##_o_sse2_c,CT_OPA,0); \
	start_test(true,name##_o_mmx_a,name##_o_sse2_c,CT_OPA,128); \
	start_test(true,name##_o_mmx_a,name##_o_sse2_c,CT_OPA,77); \
	start_test(true,name##_o_mmx_a,name##_o_sse2_c,CT_OPA,255);

#define TEST_FUNC_O2( name ) \
	printf( "%s\n", #name );	\
	start_test(false,name##_mmx_a,name##_sse2_c,CT_OPA,0); \
	start_test(false,name##_mmx_a,name##_sse2_c,CT_OPA,128); \
	start_test(false,name##_mmx_a,name##_sse2_c,CT_OPA,77); \
	start_test(false,name##_mmx_a,name##_sse2_c,CT_OPA,255);

#define TEST_FUNC_O2_C( name ) \
	printf( "%s\n", #name );	\
	start_test(false,name##,name##_sse2_c,CT_OPA,0); \
	start_test(false,name##,name##_sse2_c,CT_OPA,128); \
	start_test(false,name##,name##_sse2_c,CT_OPA,77); \
	start_test(false,name##,name##_sse2_c,CT_OPA,255);

#define TEST_FUNC_O_C( name ) \
	printf( "%s\n", #name );	\
	start_test(false,name##_o,name##_o_sse2_c,CT_OPA,0); \
	start_test(false,name##_o,name##_o_sse2_c,CT_OPA,128); \
	start_test(false,name##_o,name##_o_sse2_c,CT_OPA,77); \
	start_test(false,name##_o,name##_o_sse2_c,CT_OPA,255);

#define TEST_FUNC_O_CA( name ) \
	printf( "%s\n", #name );	\
	start_test(true,name##_o,name##_o_sse2_c,CT_OPA,64); \
	start_test(true,name##_o,name##_o_sse2_c,CT_OPA,0); \
	start_test(true,name##_o,name##_o_sse2_c,CT_OPA,128); \
	start_test(true,name##_o,name##_o_sse2_c,CT_OPA,77); \
	start_test(true,name##_o,name##_o_sse2_c,CT_OPA,255);

#define TEST_FUNC_SD( name ) \
	printf( "%s\n", #name );	\
	start_test(true,name##_mmx_a,name##_sse2_c,CT_SD,0); \
	start_test(true,name##_mmx_a,name##_sse2_c,CT_SD,128); \
	start_test(true,name##_mmx_a,name##_sse2_c,CT_SD,77); \
	start_test(true,name##_mmx_a,name##_sse2_c,CT_SD,255);

/*
static void TVPConstAlphaBlend_SD_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src1, const tjs_uint32 *src2, tjs_int len, tjs_int opa){
static void TVPCopyColor_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len) {
static void TVPCopyMask_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len) {
static void TVPCopyOpaqueImage_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len) {
static void TVPConstAlphaBlend_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa) {
static void TVPConstAlphaBlend_HDA_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa) {
static void TVPConstAlphaBlend_d_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa) {
static void TVPConstAlphaBlend_a_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa) {
*/
void BlendTest_sse2_c() {
	/*
	printf( "TVPConstAlphaBlend_SD_a\n" );
	start_test(true,TVPConstAlphaBlend_SD_a,TVPConstAlphaBlend_SD_sse2_c,CT_SD,0);
	start_test(true,TVPConstAlphaBlend_SD_a,TVPConstAlphaBlend_SD_sse2_c,CT_SD,128);
	start_test(true,TVPConstAlphaBlend_SD_a,TVPConstAlphaBlend_SD_sse2_c,CT_SD,77);
	start_test(true,TVPConstAlphaBlend_SD_a,TVPConstAlphaBlend_SD_sse2_c,CT_SD,255);
	*/

	//TEST_FUNC_SD( TVPConstAlphaBlend_SD );	// pass
	//TEST_FUNC_C( TVPCopyColor );	// pass
	//TEST_FUNC_C( TVPCopyMask );	// pass
	//TEST_FUNC_C( TVPCopyOpaqueImage );	// pass
	//TEST_FUNC_O2( TVPConstAlphaBlend );	// pass
	//TEST_FUNC_O2_C( TVPConstAlphaBlend_HDA );	// pass
	//TEST_FUNC_O2_C( TVPConstAlphaBlend_d );	// pass
	//TEST_FUNC_O2_C( TVPConstAlphaBlend_a );	// C版と比較すると 2 程度差が出るケースがある

	
	//TEST_FUNC( TVPAlphaBlend_d );
	//TEST_FUNC_C( TVPAlphaBlend_d );
	//TEST_FUNC_O_CA( TVPAdditiveAlphaBlend )

	//printf( "TVPAlphaBlend_d\n");
	//start_test(true,TVPAlphaBlend_d_mmx_a,TVPAlphaBlend_d_sse2_c);
	/*
	TEST_FUNC( TVPAlphaBlend );
	TEST_FUNC( TVPAlphaBlend_HDA );
	TEST_FUNC( TVPAlphaBlend_d );
	
	TEST_FUNC( TVPAdditiveAlphaBlend );
	TEST_FUNC( TVPAdditiveAlphaBlend_HDA );
	TEST_FUNC( TVPAddBlend );
	TEST_FUNC( TVPAddBlend_HDA );
	TEST_FUNC( TVPDarkenBlend );

	TEST_FUNC( TVPDarkenBlend_HDA );
	//printf( "TVPDarkenBlend_HDA\n");
	//start_test(false,TVPDarkenBlend_HDA_c,TVPDarkenBlend_HDA_sse2_c);

	TEST_FUNC( TVPLightenBlend );
	TEST_FUNC( TVPLightenBlend_HDA );
	TEST_FUNC( TVPMulBlend );
	TEST_FUNC( TVPMulBlend_HDA );
	TEST_FUNC( TVPScreenBlend );
	TEST_FUNC( TVPScreenBlend_HDA );
	TEST_FUNC( TVPSubBlend );
	TEST_FUNC( TVPSubBlend_HDA );
	*/
	
	// pass TEST_FUNC_O( TVPAlphaBlend );
	// pass TEST_FUNC_O_C( TVPAlphaBlend_HDA );
	//TEST_FUNC_O( TVPAdditiveAlphaBlend )
	//TEST_FUNC_O( TVPAddBlend )
	//TEST_FUNC_O( TVPMulBlend )
	//TEST_FUNC_O( TVPMulBlend_HDA )
	//TEST_FUNC_O( TVPScreenBlend )
	//TEST_FUNC_O_C( TVPScreenBlend_HDA )
	//TEST_FUNC_O( TVPSubBlend )
	//TEST_FUNC_O2( TVPConstAlphaBlend )

	// PS系 pass テーブル系のブレンド遅いから展開したいところ
	/*
	TEST_FUNC( TVPPsAlphaBlend );
	TEST_FUNC( TVPPsAlphaBlend_HDA );
	TEST_FUNC( TVPPsAddBlend );
	TEST_FUNC( TVPPsAddBlend_HDA );
	TEST_FUNC( TVPPsSubBlend );
	TEST_FUNC( TVPPsSubBlend_HDA );
	TEST_FUNC( TVPPsMulBlend );
	TEST_FUNC( TVPPsMulBlend_HDA );
	TEST_FUNC( TVPPsScreenBlend );
	TEST_FUNC( TVPPsScreenBlend_HDA );
	TEST_FUNC( TVPPsOverlayBlend );
	TEST_FUNC( TVPPsOverlayBlend_HDA );
	TEST_FUNC( TVPPsHardLightBlend );
	TEST_FUNC( TVPPsHardLightBlend_HDA );
	TEST_FUNC( TVPPsSoftLightBlend );
	TEST_FUNC( TVPPsSoftLightBlend_HDA );
	TEST_FUNC( TVPPsColorDodgeBlend );
	TEST_FUNC( TVPPsColorDodgeBlend_HDA );
	TEST_FUNC( TVPPsColorDodge5Blend );
	TEST_FUNC( TVPPsColorDodge5Blend_HDA );
	TEST_FUNC( TVPPsColorBurnBlend );
	TEST_FUNC( TVPPsColorBurnBlend_HDA );
	TEST_FUNC( TVPPsLightenBlend );
	TEST_FUNC( TVPPsLightenBlend_HDA );
	TEST_FUNC( TVPPsDarkenBlend );
	TEST_FUNC( TVPPsDarkenBlend_HDA );
	TEST_FUNC( TVPPsDiffBlend );
	TEST_FUNC( TVPPsDiffBlend_HDA );
	TEST_FUNC( TVPPsDiff5Blend );
	TEST_FUNC( TVPPsDiff5Blend_HDA );
	TEST_FUNC( TVPPsExclusionBlend );
	TEST_FUNC( TVPPsExclusionBlend_HDA );
	*/
}

#endif	// SIMD_BLEND_TEST
