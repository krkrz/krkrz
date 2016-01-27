


#include "tjsTypes.h"
#include "tvpgl.h"
#include "tvpgl_ia32_intf.h"
#include "simd_def_x86x64.h"

//#include <windows.h>

#include "blend_functor_sse2.h"
#include "blend_ps_functor_sse2.h"


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


template<typename functor>
static inline void blend_func_sse2( tjs_uint32 * __restrict dest, const tjs_uint32 * __restrict src, tjs_int len, const functor& func ) {
	if( len <= 0 ) return;

	tjs_int count = (tjs_int)((unsigned)dest & 0x15);
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

// 完全透明ではコピーせず、完全不透明はそのままコピーする
template<typename functor>
static void blend_src_branch_func_sse2( tjs_uint32 * __restrict dest, const tjs_uint32 * __restrict src, tjs_int len, const functor& func ) {
	if( len <= 0 ) return;
	
	tjs_int count = (tjs_int)((unsigned)dest & 0x15);
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

static void TVPConstAlphaBlend_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa) {
	sse2_const_alpha_blend_functor func(opa);
	blend_func_sse2( dest, src, len, func );
}
static void TVPConstAlphaBlend_SD_sse2_c(tjs_uint32 *dest, const tjs_uint32 *src1, const tjs_uint32 *src2, tjs_int len, tjs_int opa){
	if( len <= 0 ) return;

	sse2_const_alpha_blend_functor func(opa);
	tjs_uint32 rem = (len>>2)<<2;
	tjs_uint32* limit = dest + rem - 2;
	while( dest < limit ) {
		__m128i ms1 = _mm_loadu_si128( (__m128i const*)src1 );
		__m128i ms2 = _mm_loadu_si128( (__m128i const*)src2 );
		_mm_storeu_si128( (__m128i*)dest, func( ms1, ms2 ) );
		dest+=4; src1+=4; src2+=4;
	}
	limit += (len-rem);
	while( dest < limit ) {
		*dest = func( *src1, *src2 );
		dest++; src1++; src2++;
	}
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
		//TVPConstAlphaBlend =  TVPConstAlphaBlend_sse2_c; 未テスト
		//TVPConstAlphaBlend_SD =  TVPConstAlphaBlend_SD_sse2_c; 未テスト

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
	}
}

