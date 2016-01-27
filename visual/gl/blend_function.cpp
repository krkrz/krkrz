/******************************************************************************/
/**
 * テンプレートベースのGL
 * ----------------------------------------------------------------------------
 * 	Copyright (C) T.Imoto <http://www.kaede-software.com>
 * ----------------------------------------------------------------------------
 * @author		T.Imoto
 * @date		2014/02/11
 * @note		従来の TVP GL を置き換える
 *****************************************************************************/

#define TVPPS_USE_OVERLAY_TABLE

#include <math.h>

#include "tjsTypes.h"
#include "tvpgl.h"

extern "C" {
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
};

#include "blend_functor_c.h"


unsigned char ps_soft_light_table::TABLE[256][256];
unsigned char ps_color_dodge_table::TABLE[256][256];
unsigned char ps_color_burn_table::TABLE[256][256];
#ifdef TVPPS_USE_OVERLAY_TABLE
unsigned char ps_overlay_table::TABLE[256][256];
#endif

template<typename functor>
static inline void copy_func_c( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len ) {
	functor func;
	for( int i = 0; i < len; i++ ) {
		*dest = func( *dest, *src );
		dest++; src++;
	}
}
template<typename functor>
static inline void blend_func_c( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, const functor& func ) {
	for( int i = 0; i < len; i++ ) {
		*dest = func( *dest, *src );
		dest++; src++;
	}
}

#define DEFINE_BLEND_FUNCTION_VARIATION( FUNC ) \
static void TVP_##FUNC##( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len ) {						\
	copy_func_c<FUNC##_functor>( dest, src, len );														\
}																										\
static void TVP_##FUNC##_HDA( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len ) {					\
	copy_func_c<FUNC##_HDA_functor>( dest, src, len );													\
}																										\
static void TVP_##FUNC##_o( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa ) {		\
	FUNC##_o_functor func(opa);																			\
	blend_func_c( dest, src, len, func );																\
}																										\
static void TVP_##FUNC##_HDA_o( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa ) {	\
	FUNC##_HDA_o_functor func(opa);																		\
	blend_func_c( dest, src, len, func );																\
}																										\
static void TVP_##FUNC##_d( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len ) {					\
	copy_func_c<FUNC##_d_functor>( dest, src, len );													\
}																										\
static void TVP_##FUNC##_a( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len ) {					\
	copy_func_c<FUNC##_a_functor>( dest, src, len );													\
}																										\
static void TVP_##FUNC##_do( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa ) {		\
	FUNC##_do_functor func(opa);																		\
	blend_func_c( dest, src, len, func );																\
}																										\
static void TVP_##FUNC##_ao( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa ) {		\
	FUNC##_ao_functor func(opa);																		\
	blend_func_c( dest, src, len, func );																\
}

#define DEFINE_BLEND_FUNCTION_MIN_VARIATION( FUNC ) \
static void TVP_##FUNC##( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len ) {						\
	copy_func_c<FUNC##_functor>( dest, src, len );														\
}																										\
static void TVP_##FUNC##_HDA( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len ) {					\
	copy_func_c<FUNC##_HDA_functor>( dest, src, len );													\
}																										\
static void TVP_##FUNC##_o( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa ) {		\
	FUNC##_o_functor func(opa);																			\
	blend_func_c( dest, src, len, func );																\
}																										\
static void TVP_##FUNC##_HDA_o( tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa ) {	\
	FUNC##_HDA_o_functor func(opa);																		\
	blend_func_c( dest, src, len, func );																\
}																										\


DEFINE_BLEND_FUNCTION_VARIATION( alpha_blend );
DEFINE_BLEND_FUNCTION_MIN_VARIATION( add_blend );
DEFINE_BLEND_FUNCTION_MIN_VARIATION( sub_blend );
DEFINE_BLEND_FUNCTION_MIN_VARIATION( mul_blend );
DEFINE_BLEND_FUNCTION_MIN_VARIATION( color_dodge_blend );
DEFINE_BLEND_FUNCTION_MIN_VARIATION( darken_blend );
DEFINE_BLEND_FUNCTION_MIN_VARIATION( lighten_blend );
DEFINE_BLEND_FUNCTION_MIN_VARIATION( screen_blend );

/* --------------------------------------------------------------------
  Table initialize function
-------------------------------------------------------------------- */
static void TVPPsInitTable(void)
{
	int s, d;
	for (s=0; s<256; s++) {
		for (d=0; d<256; d++) {
			ps_soft_light_table::TABLE[s][d]  = (s>=128) ?
				( ((unsigned char)(pow(d/255.0, 128.0/s)*255.0)) ) :
				( ((unsigned char)(pow(d/255.0, (1.0-s/255.0)/0.5)*255.0)) );
			ps_color_dodge_table::TABLE[s][d] = ((255-s)<=d) ? (0xff) : ((d*255)/(255-s));
			ps_color_burn_table::TABLE[s][d]  = (s<=(255-d)) ? (0x00) : (255-((255-d)*255)/s);
#ifdef TVPPS_USE_OVERLAY_TABLE
			ps_overlay_table::TABLE[s][d]  = (d<128) ? ((s*d*2)/255) : (((s+d)*2)-((s*d*2)/255)-255);
#endif
		}
	}
}

#define SET_BLEND_FUNCTIONS( DEST_FUNC, FUNC )	\
TVP##DEST_FUNC = TVP_##FUNC;					\
TVP##DEST_FUNC##_HDA = TVP_##FUNC##_HDA;		\
TVP##DEST_FUNC##_o = TVP_##FUNC##_o;			\
TVP##DEST_FUNC##_HDA_o = TVP_##FUNC##_HDA_o;	\
TVP##DEST_FUNC##_d = TVP_##FUNC##_d;			\
TVP##DEST_FUNC##_a = TVP_##FUNC##_a;			\
TVP##DEST_FUNC##_do = TVP_##FUNC##_do;			\
TVP##DEST_FUNC##_ao = TVP_##FUNC##_ao;

#define SET_BLEND_MID_FUNCTIONS( DEST_FUNC, FUNC )	\
TVP##DEST_FUNC = TVP_##FUNC;					\
TVP##DEST_FUNC##_HDA = TVP_##FUNC##_HDA;		\
TVP##DEST_FUNC##_o = TVP_##FUNC##_o;			\
TVP##DEST_FUNC##_HDA_o = TVP_##FUNC##_HDA_o;	\
TVP##DEST_FUNC##_d = TVP_##FUNC##_d;			\
TVP##DEST_FUNC##_do = TVP_##FUNC##_do;

#define SET_BLEND_MIN_FUNCTIONS( DEST_FUNC, FUNC )	\
TVP##DEST_FUNC = TVP_##FUNC;					\
TVP##DEST_FUNC##_HDA = TVP_##FUNC##_HDA;		\
TVP##DEST_FUNC##_o = TVP_##FUNC##_o;			\
TVP##DEST_FUNC##_HDA_o = TVP_##FUNC##_HDA_o;


/**
 * GL初期化。関数ポインタを設定する
 */
void TVPGL_C_Init() {
#if 0
	// 以下未テスト、テスト後置き換え。
	// 各種ブレンドを関数オブジェクト化することでアフィン変換や拡縮に展開しやすくする
	SET_BLEND_FUNCTIONS( AlphaBlend, alpha_blend );
	SET_BLEND_MIN_FUNCTIONS( AddBlend, add_blend );
	SET_BLEND_MIN_FUNCTIONS( SubBlend, sub_blend );
	SET_BLEND_MIN_FUNCTIONS( MulBlend, mul_blend );
	SET_BLEND_MIN_FUNCTIONS( ColorDodgeBlend, color_dodge_blend );
	SET_BLEND_MIN_FUNCTIONS( DarkenBlend, darken_blend );
	SET_BLEND_MIN_FUNCTIONS( LightenBlend, lighten_blend );
	SET_BLEND_MIN_FUNCTIONS( ScreenBlend, screen_blend );
#endif
	TVPPsInitTable();
}

