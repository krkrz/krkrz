
#include "tjsTypes.h"
#include "tvpgl.h"
#include "tvpgl_ia32_intf.h"
#include "simd_def_x86x64.h"

#if 0	// 偏りが多い場合は速いが、通常ケースでは遅い
// MMX + SSE
tjs_int TVPTLG5DecompressSlide_sse_c( tjs_uint8 *out, const tjs_uint8 *in, tjs_int insize, tjs_uint8 *text, tjs_int initialr ) {
	tjs_int r = initialr;
	tjs_uint flags = 0;
	const tjs_uint8 *inlim = in + insize;
	const __m64 mask = _mm_set1_pi32(0xffffffff);
	while(in < inlim ) {
		if(((flags >>= 1) & 256) == 0) {
			flags = in[0];
			in++;
			if( flags == 0 && in < (inlim-8) ) {	// copy 8byte
				__m64 c = *(__m64 const*)in;
				*(__m64 *)out = c;
				*(__m64 *)&text[r] = c;
				r += 8;
				if( r > 4095 ) {
					r &= 0x0FFF;
					c = _mm_srli_pi64( c, (8 - r)*8 );
					__m64 t = *(__m64 const*)text;
					__m64 m = mask;
					m = _mm_srli_pi64( m, (8 - r)*8 );
					_mm_maskmove_si64( c, m, (char*)text );
				}
				in += 8;
				out += 8;
				continue;
			} else {
				flags |= 0xff00;
			}
		}
		if(flags & 1) {
			tjs_int mpos = in[0] | ((in[1] & 0xf) << 8);
			tjs_int mlen = (in[1] & 0xf0) >> 4;
			in += 2;
			mlen += 3;
			if(mlen == 18) {
				mlen += in[0];
				in++;
			}
			while( mlen ) {
				__m64 c = *(__m64 const*)&text[mpos];
				*(__m64 *)out = c;
				if( mlen < 8 ) {
					if( (4096-mpos) < mlen ) {
						tjs_int mrem = (4096-mpos);
						out += mrem;
						mlen -= mrem;
						__m64 m = mask;
						m = _mm_srli_pi64( m, (8-mrem)*8 );
						_mm_maskmove_si64( c, m, (char*)&text[r] );
						r += mrem;
						if( r > 4095 ) {
							r &= 0x0fff;
							c = _mm_srli_pi64( c, (mrem-r)*8 );
							m = mask;
							m = _mm_srli_pi64( m, (8-r)*8 );
							_mm_maskmove_si64( c, m, (char*)text );
						}
						mpos = 0;
					} else {
						out += mlen;
						__m64 m = mask;
						m = _mm_srli_pi64( m, (8-mlen)*8 );
						_mm_maskmove_si64( c, m, (char*)&text[r] );
						r += mlen;
						if( r > 4095 ) {
							r &= 0x0fff;
							c = _mm_srli_pi64( c, (mlen-r)*8 );
							m = mask;
							m = _mm_srli_pi64( m, (8-r)*8 );
							_mm_maskmove_si64( c, m, (char*)text );
						}
						mpos += mlen;
						mpos &= 0x0fff;
						mlen = 0;
					}
				} else if( (4096-mpos) < 8 ) {
					tjs_int mrem = (4096-mpos);
					out += mrem;
					mlen -= mrem;
					__m64 m = mask;
					m = _mm_srli_pi64( m, (8-mrem)*8 );
					_mm_maskmove_si64( c, m, (char*)&text[r] );
					r += mrem;
					if( r > 4095 ) {
						r &= 0x0fff;
						c = _mm_srli_pi64( c, (mrem-r)*8 );
						m = mask;
						m = _mm_srli_pi64( m, (8-r)*8 );
						_mm_maskmove_si64( c, m, (char*)text );
					}
					mpos = 0;
				} else {
					out += 8;
					mlen -= 8;
					*(__m64*)&text[r] = c;
					r += 8;
					if( r > 4095 ) {
						r &= 0x0fff;
						c = _mm_srli_pi64( c, (8-r)*8 );
						__m64 m = mask;
						m = _mm_srli_pi64( m, (8-r)*8 );
						_mm_maskmove_si64( c, m, (char*)text );
					}
					mpos += 8;
					mpos &= 0x0fff;
				}
			}
		} else {
			unsigned char c = in[0]; in++;
			out[0] = c; out++;
			text[r++] = c;
			r &= (4096 - 1);
		}
	}
	_mm_empty();
	return r;
}
#endif

tjs_int TVPTLG5DecompressSlide_sse2_c( tjs_uint8 *out, const tjs_uint8 *in, tjs_int insize, tjs_uint8 *text, tjs_int initialr ) {
	tjs_int r = initialr;
	tjs_uint flags = 0;
	const tjs_uint8 *inlim = in + insize;
	// text と out、in は、+16余分に確保して、はみ出してもいいようにしておく
	//const __m128i mask = _mm_set1_epi32(0xffffffff);
	const __m128i mask = _mm_set_epi32(0,0,0xffffffff,0xffffffff);
	while(in < inlim ) {
		if(((flags >>= 1) & 256) == 0) {
			flags = in[0] | 0xff00;
			in++;
#if 1
			if( flags == 0xff00 && r < (4096-8) && in < (inlim-8)  ) {	// copy 8byte
				__m128i c = _mm_loadl_epi64( (__m128i const*)in );
				_mm_storel_epi64( (__m128i *)out, c );
				_mm_storel_epi64( (__m128i *)&text[r], c );	// 末尾はみ出すのを気にせずコピー
				r += 8;
#if 0
				if( r > 4095 ) {
					r &= 0x0FFF;
					c = _mm_srli_epi64( c, (8 - r)*8 );
					__m128i t = _mm_loadl_epi64( (__m128i const*)text );
					__m128i m = mask;
					m = _mm_srli_epi64( m, (8 - r)*8 );
					_mm_maskmoveu_si128( c, m, (char*)text );
				}
#endif
				in += 8;
				out += 8;
				flags = 0;
				continue;
			}
#endif
		}
		if(flags & 1) {
			tjs_int mpos = in[0] | ((in[1] & 0xf) << 8);
			tjs_int mlen = (in[1] & 0xf0) >> 4;
			in += 2;
			mlen += 3;
			if(mlen == 18) {
				mlen += in[0]; in++;
				// 数バイトの細切れだと遅いと思うけど、そうじゃなければ速いかな？
				// 8バイト単位 128bitシフトはimmでないといけないので、64bitごとに処理
				// 頑張れば16バイト単位でも行けそうだけど、分岐が多くなりそうなので
				if( (mpos+mlen) < 4096 && (r+mlen) < 4096 ) {
					// 末尾気にせずコピーしていい時は16byte単位でまとめてコピー
					tjs_int count = mlen >> 4;
					while( count-- ) {
						__m128i c = _mm_loadu_si128( (__m128i const*)&text[mpos] );
						_mm_storeu_si128( (__m128i *)out, c );
						_mm_storeu_si128( (__m128i *)&text[r], c );
						mpos += 16; r += 16; out += 16;
					}
					mlen &= 0x0f;	// 余り
					while(mlen--) {
						out[0] = text[r++] = text[mpos++]; out++;
					}
					continue;
				}
#if 0
				while(mlen--) {
					out[0] = text[r++] = text[mpos++]; out++;
					mpos &= 0x0fff;
					r &= 0x0fff;
				}
#else
				while( mlen ) {
					__m128i c = _mm_loadl_epi64( (__m128i const*)&text[mpos] );
					_mm_storeu_si128( (__m128i *)out, c );
					if( mlen < 8 ) {
						if( (4096-mpos) < mlen ) {
							tjs_int mrem = (4096-mpos);
							out += mrem;
							mlen -= mrem;
							__m128i m = mask;
							m = _mm_srli_epi64( m, (8-mrem)*8 );
							_mm_maskmoveu_si128( c, m, (char*)&text[r] );
							r += mrem;
							if( r > 4095 ) {
								r &= 0x0fff;
								c = _mm_srli_epi64( c, (mrem-r)*8 );
								m = mask;
								m = _mm_srli_epi64( m, (8-r)*8 );
								_mm_maskmoveu_si128( c, m, (char*)text );
							}
							mpos = 0;
						} else {
							out += mlen;
							__m128i m = mask;
							m = _mm_srli_epi64( m, (8-mlen)*8 );
							_mm_maskmoveu_si128( c, m, (char*)&text[r] );
							r += mlen;
							if( r > 4095 ) {
								r &= 0x0fff;
								c = _mm_srli_epi64( c, (mlen-r)*8 );
								m = mask;
								m = _mm_srli_epi64( m, (8-r)*8 );
								_mm_maskmoveu_si128( c, m, (char*)text );
							}
							mpos += mlen;
							mpos &= 0x0fff;
							mlen = 0;
						}
					} else if( (4096-mpos) < 8 ) {
						tjs_int mrem = (4096-mpos);
						out += mrem;
						mlen -= mrem;
						__m128i m = mask;
						m = _mm_srli_epi64( m, (8-mrem)*8 );
						_mm_maskmoveu_si128( c, m, (char*)&text[r] );
						r += mrem;
						if( r > 4095 ) {
							r &= 0x0fff;
							c = _mm_srli_epi64( c, (mrem-r)*8 );
							m = mask;
							m = _mm_srli_epi64( m, (8-r)*8 );
							_mm_maskmoveu_si128( c, m, (char*)text );
						}
						mpos = 0;
					} else {
						out += 8;
						mlen -= 8;
						_mm_storel_epi64( (__m128i *)&text[r], c );
						r += 8;
						if( r > 4095 ) {
							r &= 0x0fff;
							c = _mm_srli_epi64( c, (8-r)*8 );
							__m128i m = mask;
							m = _mm_srli_epi64( m, (8-r)*8 );
							_mm_maskmoveu_si128( c, m, (char*)text );
						}
						mpos += 8;
						mpos &= 0x0fff;
					}
				}
#endif
			} else {
				while(mlen--) {
					out[0] = text[r++] = text[mpos++]; out++;
					mpos &= 0x0fff;
					r &= 0x0fff;
				}
			}
		} else {
			unsigned char c = in[0]; in++;
			out[0] = c; out++;
			text[r++] = c;
			r &= 0x0fff;
		}
	}
	return r;
}

void TVPTLG5ComposeColors3To4_sse2_c(tjs_uint8 *outp, const tjs_uint8 *upper, tjs_uint8 * const * buf, tjs_int width) {
	tjs_int len = (width>>2)<<2;
	// 4つずつ処理
	tjs_uint32* b0 = (tjs_uint32*)buf[0];
	tjs_uint32* b1 = (tjs_uint32*)buf[1];
	tjs_uint32* b2 = (tjs_uint32*)buf[2];
	__m128i pc = _mm_setzero_si128();
	const __m128i opa(_mm_set1_epi32( 0xff000000 ));
	const __m128i zero(_mm_setzero_si128());
	tjs_int x = 0;
	for( ; x < len; x+=4 ) {
		__m128i c0 = _mm_cvtsi32_si128( *b0 );
		__m128i c1 = _mm_cvtsi32_si128( *b1 );
		__m128i c2 = _mm_cvtsi32_si128( *b2 );
		c0 = _mm_unpacklo_epi8( c0, zero );
		c0 = _mm_unpacklo_epi16( c0, zero );
		c2 = _mm_unpacklo_epi8( c2, zero );
		__m128i tmp = zero;
		tmp = _mm_unpacklo_epi16( tmp, c2 );
		c0 = _mm_or_si128( c0, tmp );		// 0 X 2 X
		c1 = _mm_unpacklo_epi8( c1, c1 );	// XXXXXXXX 0 0 1 1 2 2 3 3
		c1 = _mm_unpacklo_epi16( c1, c1 );	// 0000 1111 2222 3333
		c0 = _mm_add_epi8( c0, c1 );
		pc = _mm_add_epi8( pc, c0 );
		tmp = pc;
		c0 = _mm_srli_si128( c0, 4 );
		pc = _mm_add_epi8( pc, c0 );
		tmp = _mm_unpacklo_epi32( tmp, pc );
		c0 = _mm_srli_si128( c0, 4 );
		pc = _mm_add_epi8( pc, c0 );
		__m128i tmp2 = pc;
		c0 = _mm_srli_si128( c0, 4 );
		pc = _mm_add_epi8( pc, c0 );
		tmp2 = _mm_unpacklo_epi32( tmp2, pc );
		tmp = _mm_unpacklo_epi64( tmp, tmp2 );
		__m128i mup = _mm_loadu_si128( (__m128i const*)upper );
		tmp = _mm_add_epi8( tmp, mup );
		tmp = _mm_or_si128( tmp, opa );
		_mm_storeu_si128( (__m128i*)outp, tmp );
		b0++; b1++; b2++; outp += 4*4; upper += 4*4;
	}
	tjs_uint8* bb0 = (tjs_uint8*)b0;
	tjs_uint8* bb1 = (tjs_uint8*)b1;
	tjs_uint8* bb2 = (tjs_uint8*)b2;
	for( ; x < width; x++ ) {
		__m128i c0 = _mm_cvtsi32_si128( *bb0 );
		__m128i c1 = _mm_cvtsi32_si128( (*bb1)*0x00010101 );
		__m128i c2 = _mm_cvtsi32_si128( *bb2 << 16 );
		c0 = _mm_or_si128( c0, c2 );
		c0 = _mm_add_epi8( c0, c1 );
		pc = _mm_add_epi8( pc, c0 );
		__m128i mup = _mm_cvtsi32_si128(*(tjs_uint32*)upper);
		__m128i tmp = pc;
		tmp = _mm_add_epi8( tmp, mup );
		tmp = _mm_or_si128( tmp, opa );
		*(tjs_uint32*)outp = _mm_cvtsi128_si32(tmp);
		bb0++; bb1++; bb2++;
		outp += 4; upper += 4;
	}
}
void TVPTLG5ComposeColors4To4_sse2_c(tjs_uint8 *outp, const tjs_uint8 *upper, tjs_uint8 * const * buf, tjs_int width) {
	tjs_int len = (width>>2)<<2;
	// 4つずつ処理
	tjs_uint32* b0 = (tjs_uint32*)buf[0];
	tjs_uint32* b1 = (tjs_uint32*)buf[1];
	tjs_uint32* b2 = (tjs_uint32*)buf[2];
	tjs_uint32* b3 = (tjs_uint32*)buf[3];
	__m128i pc = _mm_setzero_si128();
	const __m128i zero(_mm_setzero_si128());
	tjs_int x = 0;
	for( ; x < len; x+=4 ) {
		__m128i c0 = _mm_cvtsi32_si128( *b0 );
		__m128i c1 = _mm_cvtsi32_si128( *b1 );
		__m128i c2 = _mm_cvtsi32_si128( *b2 );
		__m128i c3 = _mm_cvtsi32_si128( *b3 );
		c0 = _mm_unpacklo_epi8( c0, zero );
		c0 = _mm_unpacklo_epi16( c0, zero );
		c2 = _mm_unpacklo_epi8( c2, zero );
		__m128i tmp = zero;
		tmp = _mm_unpacklo_epi16( tmp, c2 );
		c3 = _mm_unpacklo_epi8( c3, zero );
		c3 = _mm_unpacklo_epi16( c3, zero );
		c3 = _mm_slli_epi32( c3, 24 );
		c0 = _mm_or_si128( c0, tmp );		// 0 X 2 X
		c0 = _mm_or_si128( c0, c3 );		// 0 X 2 3
		c1 = _mm_unpacklo_epi8( c1, c1 );	// XXXXXXXX 0 0 1 1 2 2 3 3
		c1 = _mm_unpacklo_epi16( c1, c1 );	// 0000 1111 2222 3333
		c1 = _mm_srli_epi32( c1, 8 );		// X000 X111 X222 X333
		c0 = _mm_add_epi8( c0, c1 );
		pc = _mm_add_epi8( pc, c0 );
		tmp = pc;
		c0 = _mm_srli_si128( c0, 4 );
		pc = _mm_add_epi8( pc, c0 );
		tmp = _mm_unpacklo_epi32( tmp, pc );
		c0 = _mm_srli_si128( c0, 4 );
		pc = _mm_add_epi8( pc, c0 );
		__m128i tmp2 = pc;
		c0 = _mm_srli_si128( c0, 4 );
		pc = _mm_add_epi8( pc, c0 );
		tmp2 = _mm_unpacklo_epi32( tmp2, pc );
		tmp = _mm_unpacklo_epi64( tmp, tmp2 );
		__m128i mup = _mm_loadu_si128( (__m128i const*)upper );
		tmp = _mm_add_epi8( tmp, mup );
		_mm_storeu_si128( (__m128i*)outp, tmp );
		b0++; b1++; b2++; b3++; outp += 4*4; upper += 4*4;
	}
	tjs_uint8* bb0 = (tjs_uint8*)b0;
	tjs_uint8* bb1 = (tjs_uint8*)b1;
	tjs_uint8* bb2 = (tjs_uint8*)b2;
	tjs_uint8* bb3 = (tjs_uint8*)b3;
	for( ; x < width; x++ ) {
		__m128i c0 = _mm_cvtsi32_si128( *bb0 );
		__m128i c1 = _mm_cvtsi32_si128( (*bb1)*0x00010101 );
		__m128i c2 = _mm_cvtsi32_si128( *bb2 << 16 );
		__m128i c3 = _mm_cvtsi32_si128( *bb3 << 24 );
		c0 = _mm_or_si128( c0, c2 );
		c0 = _mm_or_si128( c0, c3 );
		c0 = _mm_add_epi8( c0, c1 );
		pc = _mm_add_epi8( pc, c0 );
		__m128i mup = _mm_cvtsi32_si128(*(tjs_uint32*)upper);
		__m128i tmp = pc;
		tmp = _mm_add_epi8( tmp, mup );
		*(tjs_uint32*)outp = _mm_cvtsi128_si32(tmp);
		bb0++; bb1++; bb2++; bb3++;
		outp += 4; upper += 4;
	}
}
//#define LZSS_TEST
#ifdef LZSS_TEST
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "tjsCommHead.h"
#include "tjsUtils.h"
extern "C" tjs_int TVPTLG5DecompressSlide_c(tjs_uint8 *out, const tjs_uint8 *in, tjs_int insize, tjs_uint8 *text, tjs_int initialr);
tjs_int TVPTLG5DecompressSlide_test( tjs_uint8 *out, const tjs_uint8 *in, tjs_int insize, tjs_uint8 *text, tjs_int initialr ) {
	tjs_uint8* text0 = (tjs_uint8*)TJSAlignedAlloc(4096+16, 4);
	tjs_uint8* text1 = (tjs_uint8*)TJSAlignedAlloc(4096+16, 4);
	tjs_uint8* out0 = (tjs_uint8*)TJSAlignedAlloc(2048 * 2048 + 10+16, 4);
	tjs_uint8* out1 = (tjs_uint8*)TJSAlignedAlloc(2048 * 2048 + 10+16, 4);
	memcpy( text0, text, 4096 );
	memcpy( text1, text, 4096 );
	tjs_int ret = TVPTLG5DecompressSlide_a( out, in, insize, text, initialr );

	tjs_uint32 aux;
	unsigned __int64 start = __rdtscp(&aux);
	tjs_int r0 = TVPTLG5DecompressSlide_sse2_c( out0, in, insize, text0, initialr );
	//tjs_int r0 = TVPTLG5DecompressSlide_sse_c( out0, in, insize, text0, initialr );
	unsigned __int64 end0 = __rdtscp(&aux);
	//tjs_int r1 = TVPTLG5DecompressSlide_a( out1, in, insize, text1, initialr );
	tjs_int r1 = TVPTLG5DecompressSlide_c( out1, in, insize, text1, initialr );
	unsigned __int64 end1 = __rdtscp(&aux);
	{
		unsigned __int64 func_b_total = end0-start;
		unsigned __int64 func_a_total = end1-end0;
		wchar_t buff[128];
		wsprintf( buff, L"LZSS SSE2 %I64d, ASM %I64d, rate %I64d\n", func_b_total, func_a_total, (func_b_total)*100/(func_a_total) );
		OutputDebugString( buff );
	}

	if( r0 != r1 ) {
		wchar_t buff[128];
		wsprintf( buff, L"LZSS ret, %d, %d\n", r0, r1 );
		OutputDebugString( buff );
	}
	for( int i = 0; i < 4096; i++ ) {
		if( text0[i] != text1[i] ) {
			wchar_t buff[128];
			wsprintf( buff, L"LZSS text index : %d, %02x, %02x\n", i, text0[i], text1[i] );
			OutputDebugString( buff );
		}
	}
	for( int i = 0; i < 1920*4; i++ ) {
		if( out0[i] != out1[i] ) {
			wchar_t buff[128];
			wsprintf( buff, L"LZSS output index : %d, %02x, %02x\n", i, out0[i], out1[i] );
			OutputDebugString( buff );
		}
	}

	TJSAlignedDealloc( text0 );
	TJSAlignedDealloc( text1 );
	TJSAlignedDealloc( out0 );
	TJSAlignedDealloc( out1 );
	return ret;
}
// tjs_int TVPTLG5DecompressSlide_sse2_c( tjs_uint8 *out, const tjs_uint8 *in, tjs_int insize, tjs_uint8 *text, tjs_int initialr );
// tjs_int TVPTLG5DecompressSlide_mmx_a( tjs_uint8 *out, const tjs_uint8 *in, tjs_int insize, tjs_uint8 *text, tjs_int initialr );

// void TVPTLG5ComposeColors3To4_sse2_c(tjs_uint8 *outp, const tjs_uint8 *upper, tjs_uint8 * const * buf, tjs_int width)
// TVPTLG5ComposeColors4To4_sse2_c(tjs_uint8 *outp, const tjs_uint8 *upper, tjs_uint8 * const * buf, tjs_int width)
void TVPTLG5ComposeColors3To4_test(tjs_uint8 *outp, const tjs_uint8 *upper, tjs_uint8 * const * buf, tjs_int width) {
	tjs_uint8 *outbuf0[4];
	tjs_uint8 *outbuf1[4];
	for(tjs_int i = 0; i < 3; i++) {
		outbuf0[i] = (tjs_uint8*)TJSAlignedAlloc(16 * width + 10, 4);
		outbuf1[i] = (tjs_uint8*)TJSAlignedAlloc(16 * width + 10, 4);
		memcpy( outbuf0[i], buf[i], width );
		memcpy( outbuf1[i], buf[i], width );
	}
	tjs_uint8 *upper0 = (tjs_uint8*)TJSAlignedAlloc(width*4, 4);
	tjs_uint8 *upper1 = (tjs_uint8*)TJSAlignedAlloc(width*4, 4);
	memcpy( upper0, upper, width*4 );
	memcpy( upper1, upper, width*4 );
	tjs_uint8 *outp0 = (tjs_uint8*)TJSAlignedAlloc(width*4, 4);
	tjs_uint8 *outp1 = (tjs_uint8*)TJSAlignedAlloc(width*4, 4);
	TVPTLG5ComposeColors3To4_mmx_a( outp, upper, buf, width );

	tjs_uint32 aux;
	unsigned __int64 start = __rdtscp(&aux);
	TVPTLG5ComposeColors3To4_sse2_c( outp0, upper0, outbuf0, width );
	unsigned __int64 end0 = __rdtscp(&aux);
	TVPTLG5ComposeColors3To4_mmx_a( outp1, upper1, outbuf1, width );
	unsigned __int64 end1 = __rdtscp(&aux);
	{
		unsigned __int64 func_b_total = end0-start;
		unsigned __int64 func_a_total = end1-end0;
		wchar_t buff[128];
		wsprintf( buff, L"ComposeColors3To4 SSE2 %I64d, ASM %I64d, rate %I64d\n", func_b_total, func_a_total, (func_b_total)*100/(func_a_total) );
		OutputDebugString( buff );
	}
	for( int i = 0; i < width; i++ ) {
		if( outp0[i*4+0] != outp1[i*4+0] ||
			outp0[i*4+1] != outp1[i*4+1] ||
			outp0[i*4+2] != outp1[i*4+2] ||
			outp0[i*4+3] != outp1[i*4+3]) {
			wchar_t buff[128];
			wsprintf( buff, L"LZSS color index : %d, 0x%02x%02x%02x%02x, 0x%02x%02x%02x%02x\n", i,
				outp0[i*4+3], outp0[i*4+2], outp0[i*4+1], outp0[i*4+0],
				outp1[i*4+3], outp1[i*4+2], outp1[i*4+1], outp1[i*4+0] );
			OutputDebugString( buff );
		}
	}

	for(tjs_int i = 0; i < 3; i++) {
		TJSAlignedDealloc( outbuf0[i] );
		TJSAlignedDealloc( outbuf1[i] );
	}
	TJSAlignedDealloc( upper0 );
	TJSAlignedDealloc( upper1 );
}
void TVPTLG5ComposeColors4To4_test(tjs_uint8 *outp, const tjs_uint8 *upper, tjs_uint8 * const * buf, tjs_int width) {
	tjs_uint8 *outbuf0[4];
	tjs_uint8 *outbuf1[4];
	for(tjs_int i = 0; i < 4; i++) {
		outbuf0[i] = (tjs_uint8*)TJSAlignedAlloc(16 * width + 10, 4);
		outbuf1[i] = (tjs_uint8*)TJSAlignedAlloc(16 * width + 10, 4);
		memcpy( outbuf0[i], buf[i], width );
		memcpy( outbuf1[i], buf[i], width );
	}
	tjs_uint8 *upper0 = (tjs_uint8*)TJSAlignedAlloc(width*4, 4);
	tjs_uint8 *upper1 = (tjs_uint8*)TJSAlignedAlloc(width*4, 4);
	memcpy( upper0, upper, width*4 );
	memcpy( upper1, upper, width*4 );
	tjs_uint8 *outp0 = (tjs_uint8*)TJSAlignedAlloc(width*4, 4);
	tjs_uint8 *outp1 = (tjs_uint8*)TJSAlignedAlloc(width*4, 4);
	TVPTLG5ComposeColors4To4_mmx_a( outp, upper, buf, width );
	memset( outp0, 0xdd, width*4 );
	memset( outp1, 0xdd, width*4 );

	tjs_uint32 aux;
	unsigned __int64 start = __rdtscp(&aux);
	TVPTLG5ComposeColors4To4_sse2_c( outp0, upper0, outbuf0, width );
	unsigned __int64 end0 = __rdtscp(&aux);
	TVPTLG5ComposeColors4To4_mmx_a( outp1, upper1, outbuf1, width );
	unsigned __int64 end1 = __rdtscp(&aux);
	{
		unsigned __int64 func_b_total = end0-start;
		unsigned __int64 func_a_total = end1-end0;
		wchar_t buff[128];
		wsprintf( buff, L"ComposeColors4To4 SSE2 %I64d, ASM %I64d, rate %I64d\n", func_b_total, func_a_total, (func_b_total)*100/(func_a_total) );
		OutputDebugString( buff );
	}
	for( int i = 0; i < width; i++ ) {
		if( outp0[i*4+0] != outp1[i*4+0] ||
			outp0[i*4+1] != outp1[i*4+1] ||
			outp0[i*4+2] != outp1[i*4+2] ||
			outp0[i*4+3] != outp1[i*4+3]) {
			wchar_t buff[128];
			wsprintf( buff, L"LZSS color index : %d, 0x%02x%02x%02x%02x, 0x%02x%02x%02x%02x\n", i,
				outp0[i*4+3], outp0[i*4+2], outp0[i*4+1], outp0[i*4+0],
				outp1[i*4+3], outp1[i*4+2], outp1[i*4+1], outp1[i*4+0] );
			OutputDebugString( buff );
		}
	}
	for(tjs_int i = 0; i < 4; i++) {
		TJSAlignedDealloc( outbuf0[i] );
		TJSAlignedDealloc( outbuf1[i] );
	}
	TJSAlignedDealloc( upper0 );
	TJSAlignedDealloc( upper1 );
	TJSAlignedDealloc( outp0 );
	TJSAlignedDealloc( outp1 );
}
/*
tlg6_golomb は、MMX 使っているが、一時変数として使われているのとプリフェッチのみ。SSE2 は意味なさげ
tlg6_chroma は、MMX(SSE)が使われている
*/
#endif
