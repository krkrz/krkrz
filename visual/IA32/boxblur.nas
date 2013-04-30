; -*- Mode: asm; tab-width: 12; -*-
; this is a part of TVP (KIRIKIRI) software source.
; see other sources for license.
; (C)2001-2009 W.Dee <dee@kikyou.info> and contributors

; blur related functions

%ifndef	GEN_CODE

%include		"nasm.nah"

externdef		TVPDivTable
globaldef		TVPAddSubVertSum16_mmx_a
globaldef		TVPAddSubVertSum16_emmx_a
globaldef		TVPAddSubVertSum16_d_mmx_a
globaldef		TVPAddSubVertSum16_d_emmx_a
globaldef		TVPAddSubVertSum32_mmx_a
globaldef		TVPAddSubVertSum32_emmx_a
globaldef		TVPAddSubVertSum32_d_mmx_a
globaldef		TVPAddSubVertSum32_d_emmx_a
globaldef		TVPDoBoxBlurAvg16_mmx_a
globaldef		TVPDoBoxBlurAvg16_emmx_a
globaldef		TVPDoBoxBlurAvg16_sse_a
globaldef		TVPDoBoxBlurAvg16_d_mmx_a
globaldef		TVPDoBoxBlurAvg16_d_emmx_a
globaldef		TVPDoBoxBlurAvg16_d_sse_a
globaldef		TVPDoBoxBlurAvg32_mmx_a
globaldef		TVPDoBoxBlurAvg32_emmx_a
globaldef		TVPDoBoxBlurAvg32_d_mmx_a
globaldef		TVPDoBoxBlurAvg32_d_emmx_a


%define GEN_CODE

;--------------------------------------------------------------------
; MMX stuff
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPAddSubVertSum16
;;void, TVPAddSubVertSum16_mmx_a, (tjs_uint16 *dest, const tjs_uint32 *addline, const tjs_uint32 *subline, tjs_int len)
%define TVPAddSubVertSum16_name TVPAddSubVertSum16_mmx_a

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPAddSubVertSum16_d
;;void, TVPAddSubVertSum16_d_mmx_a, (tjs_uint16 *dest, const tjs_uint32 *addline, const tjs_uint32 *subline, tjs_int len)
%define TVPAddSubVertSum16_d_name TVPAddSubVertSum16_d_mmx_a

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPAddSubVertSum32
;;void, TVPAddSubVertSum32_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *addline, const tjs_uint32 *subline, tjs_int len)
%define TVPAddSubVertSum32_name TVPAddSubVertSum32_mmx_a

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPAddSubVertSum32_d
;;void, TVPAddSubVertSum32_d_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *addline, const tjs_uint32 *subline, tjs_int len)
%define TVPAddSubVertSum32_d_name TVPAddSubVertSum32_d_mmx_a

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPDoBoxBlurAvg16
;;void, TVPDoBoxBlurAvg16_mmx_a, (tjs_uint32 *dest, tjs_uint16 *sum, const tjs_uint16 * add, const tjs_uint16 * sub, tjs_int n, tjs_int len)
%define TVPDoBoxBlurAvg16_name TVPDoBoxBlurAvg16_mmx_a

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPDoBoxBlurAvg16_d
;;void, TVPDoBoxBlurAvg16_d_mmx_a, (tjs_uint32 *dest, tjs_uint16 *sum, const tjs_uint16 * add, const tjs_uint16 * sub, tjs_int n, tjs_int len)
%define TVPDoBoxBlurAvg16_d_name TVPDoBoxBlurAvg16_d_mmx_a

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPDoBoxBlurAvg32
;;void, TVPDoBoxBlurAvg32_mmx_a, (tjs_uint32 *dest, tjs_uint32 *sum, const tjs_uint32 * add, const tjs_uint32 * sub, tjs_int n, tjs_int len)
%define TVPDoBoxBlurAvg32_name TVPDoBoxBlurAvg32_mmx_a

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPDoBoxBlurAvg32_d
;;void, TVPDoBoxBlurAvg32_d_mmx_a, (tjs_uint32 *dest, tjs_uint32 *sum, const tjs_uint32 * add, const tjs_uint32 * sub, tjs_int n, tjs_int len)
%define TVPDoBoxBlurAvg32_d_name TVPDoBoxBlurAvg32_d_mmx_a
;--------------------------------------------------------------------
	%include "boxblur.nas"
;--------------------------------------------------------------------

;--------------------------------------------------------------------
; EMMX stuff
;--------------------------------------------------------------------
%define USE_EMMX
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPAddSubVertSum16
;;void, TVPAddSubVertSum16_emmx_a, (tjs_uint16 *dest, const tjs_uint32 *addline, const tjs_uint32 *subline, tjs_int len)
%define TVPAddSubVertSum16_name TVPAddSubVertSum16_emmx_a

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPAddSubVertSum16_d
;;void, TVPAddSubVertSum16_d_emmx_a, (tjs_uint16 *dest, const tjs_uint32 *addline, const tjs_uint32 *subline, tjs_int len)
%define TVPAddSubVertSum16_d_name TVPAddSubVertSum16_d_emmx_a

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPAddSubVertSum32
;;void, TVPAddSubVertSum32_emmx_a, (tjs_uint32 *dest, const tjs_uint32 *addline, const tjs_uint32 *subline, tjs_int len)
%define TVPAddSubVertSum32_name TVPAddSubVertSum32_emmx_a

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPAddSubVertSum32_d
;;void, TVPAddSubVertSum32_d_emmx_a, (tjs_uint32 *dest, const tjs_uint32 *addline, const tjs_uint32 *subline, tjs_int len)
%define TVPAddSubVertSum32_d_name TVPAddSubVertSum32_d_emmx_a

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPDoBoxBlurAvg16
;;void, TVPDoBoxBlurAvg16_emmx_a, (tjs_uint32 *dest, tjs_uint16 *sum, const tjs_uint16 * add, const tjs_uint16 * sub, tjs_int n, tjs_int len)
%define TVPDoBoxBlurAvg16_name TVPDoBoxBlurAvg16_emmx_a

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPDoBoxBlurAvg16_d
;;void, TVPDoBoxBlurAvg16_d_emmx_a, (tjs_uint32 *dest, tjs_uint16 *sum, const tjs_uint16 * add, const tjs_uint16 * sub, tjs_int n, tjs_int len)
%define TVPDoBoxBlurAvg16_d_name TVPDoBoxBlurAvg16_d_emmx_a

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPDoBoxBlurAvg32
;;void, TVPDoBoxBlurAvg32_emmx_a, (tjs_uint32 *dest, tjs_uint32 *sum, const tjs_uint32 * add, const tjs_uint32 * sub, tjs_int n, tjs_int len)
%define TVPDoBoxBlurAvg32_name TVPDoBoxBlurAvg32_emmx_a

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPDoBoxBlurAvg32_d
;;void, TVPDoBoxBlurAvg32_d_emmx_a, (tjs_uint32 *dest, tjs_uint32 *sum, const tjs_uint32 * add, const tjs_uint32 * sub, tjs_int n, tjs_int len)
%define TVPDoBoxBlurAvg32_d_name TVPDoBoxBlurAvg32_d_emmx_a
;--------------------------------------------------------------------
	%include "boxblur.nas"
;--------------------------------------------------------------------

%define GEN_SSE


;--------------------------------------------------------------------
	start_const_aligned
;--------------------------------------------------------------------
c_0000ffffffffffff		dd	0xffffffff, 0x0000ffff
c_ffff000000000000		dd	0x00000000, 0xffff0000
c_0100000000000000		dd	0x00000000, 0x01000000
;--------------------------------------------------------------------
	end_const_aligned
;--------------------------------------------------------------------


%else


;--------------------------------------------------------------------
	segment_code
;--------------------------------------------------------------------
	function_align
TVPAddSubVertSum16_name:			; vertical addition of pixel valus, 16bit precision
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 40]		; len
	cmp	ecx,	byte 0
	jle	near .pexit
	pxor	mm0,	mm0
	mov	edi,	[esp + 28]		; dest
	mov	eax,	[esp + 32]		; addline
	mov	ebx,	[esp + 36]		; subline
	shl	ecx,	3
	lea	esi,	[edi + ecx]		; limit
	sub	esi,	byte 24		; 3*8
	cmp	edi,	esi

	jae	near .pfraction		; jump if edi >= esi

	loop_align
.ploop:
	movq	mm1,	[edi]		; 1 A|R|G|B
	movq	mm6,	[edi+8]		; 2 A|R|G|B
	movd	mm3,	[eax]		; 1 00|00|00|00|A+|R+|G+|B+
	movd	mm2,	[eax+4]		; 2 00|00|00|00|A+|R+|G+|B+
	movd	mm4,	[ebx]		; 1 00|00|00|00|A-|R-|G-|B-
	movd	mm5,	[ebx+4]		; 2 00|00|00|00|A-|R-|G-|B-
	punpcklbw	mm3,	mm0		; 1 00|A+|00|R+|00|G+|00|B+
	punpcklbw	mm2,	mm0		; 2 00|A+|00|R+|00|G+|00|B+
%ifdef USE_EMMX
	prefetcht0	[eax + 64]
%endif
	punpcklbw	mm4,	mm0		; 1 00|A-|00|R-|00|G-|00|B-
	punpcklbw	mm5,	mm0		; 2 00|A-|00|R-|00|G-|00|B-
	paddd	mm1,	mm3		; 1
	paddd	mm6,	mm2		; 2
	psubd	mm1,	mm4		; 1
	psubd	mm6,	mm5		; 2
	movq	[edi],	mm1		; 1
	add	eax,	byte 4*4
	movq	[edi+8],	mm6		; 2
	movq	mm1,	[edi+16]		; 3 A|R|G|B
	add	ebx,	byte 4*4
	movq	mm6,	[edi+24]		; 4 A|R|G|B
	movd	mm3,	[eax+8-4*4]		; 3 00|00|00|00|A+|R+|G+|B+
	movd	mm2,	[eax+12-4*4]	; 4 00|00|00|00|A+|R+|G+|B+
	movd	mm4,	[ebx+8-4*4]		; 3 00|00|00|00|A-|R-|G-|B-
	add	edi,	byte 8*4
	movd	mm5,	[ebx+12-4*4]	; 4 00|00|00|00|A-|R-|G-|B-
	punpcklbw	mm3,	mm0		; 3 00|A+|00|R+|00|G+|00|B+
	punpcklbw	mm2,	mm0		; 4 00|A+|00|R+|00|G+|00|B+
%ifdef USE_EMMX
	prefetcht0	[ebx + 64]
%endif
	punpcklbw	mm4,	mm0		; 3 00|A-|00|R-|00|G-|00|B-
	cmp	edi,	esi
	punpcklbw	mm5,	mm0		; 4 00|A-|00|R-|00|G-|00|B-
	paddd	mm1,	mm3		; 3
	paddd	mm6,	mm2		; 4
	psubd	mm1,	mm4		; 3
	psubd	mm6,	mm5		; 4
	movq	[edi+16-8*4],mm1		; 3
	movq	[edi+24-8*4],mm6		; 4

	jb	near .ploop

.pfraction:
	add	esi,	byte 24
	cmp	edi,	esi
	jae	.pexit			; jump if edi >= esi

.ploop2:	; fractions
	movq	mm1,	[edi]		; A|R|G|B
	movd	mm3,	[eax]		; 00|00|00|00|A+|R+|G+|B+
	movd	mm4,	[ebx]		; 00|00|00|00|A-|R-|G-|B-

	punpcklbw	mm3,	mm0		; 00|A+|00|R+|00|G+|00|B+
	punpcklbw	mm4,	mm0		; 00|A-|00|R-|00|G-|00|B-

	paddd	mm1,	mm3
	psubd	mm1,	mm4

	movq	[edi],	mm1

	add	edi,	byte 8
	add	eax,	byte 4
	add	ebx,	byte 4

	cmp	edi,	esi
	jb	short .ploop2		; jump if edi < esi

.pexit:
	pop	ebp
	pop	edx
	pop	ecx
	pop	ebx
	pop	esi
	pop	edi
	emms
	ret

;--------------------------------------------------------------------
	function_align
TVPAddSubVertSum16_d_name:			; vertical addition of pixel valus with alpha, 16bit precision
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 40]		; len
	cmp	ecx,	byte 0
	jle	near .pexit
	pxor	mm0,	mm0
	mov	edi,	[esp + 28]		; dest
	mov	eax,	[esp + 32]		; addline
	mov	ebx,	[esp + 36]		; subline
	shl	ecx,	3
	lea	esi,	[edi + ecx]		; limit
	sub	esi,	byte 24		; 3*8
	cmp	edi,	esi

	jae	near .pfraction		; jump if edi >= esi

	loop_align
.ploop:
	movd	mm3,	[eax]		; 00|00|00|00|A+|R+|G+|B+
	movd	mm4,	[ebx]		; 00|00|00|00|A-|R-|G-|B-
	movq	mm1,	[edi]		; A|R|G|B

	punpcklbw	mm3,	mm0		; 00|A+|00|R+|00|G+|00|B+
	punpcklbw	mm4,	mm0		; 00|A-|00|R-|00|G-|00|B-

	movq	mm5,	mm3		; addalpha:
	movq	mm6,	mm4		; addalpha:
	psrlw	mm5,	7		; addalpha:
	psrlw	mm6,	7		; addalpha:
%ifdef USE_EMMX
	prefetcht0	[eax + 64]
%endif
	paddw	mm5,	mm3		; addalpha: adjust alpha
	paddw	mm6,	mm4		; addalpha: adjust alpha
	psrlq	mm5,	48		; addalpha: 0000|0000|0000|A+
	psrlq	mm6,	48		; addalpha: 0000|0000|0000|A-
	punpcklwd	mm5,	mm5		; addalpha: 0000|0000|A+|A+
	punpcklwd	mm6,	mm6		; addalpha: 0000|0000|A-|A-
	punpcklwd	mm5,	mm5		; addalpha: A+|A+|A+|A+
	punpcklwd	mm6,	mm6		; addalpha: A-|A-|A-|A-
	pand	mm5,	[c_0000ffffffffffff]	; addalpha:
	pand	mm6,	[c_0000ffffffffffff]	; addalpha:
	por	mm5,	[c_0100000000000000]	; addalpha: 0100|A+|A+|A+
	por	mm6,	[c_0100000000000000]	; addalpha: 0100|A-|A-|A-
	pmullw	mm3,	mm5		; addalpha:
	pmullw	mm4,	mm6		; addalpha:
	psrlw	mm3,	8		; addalpha:
	psrlw	mm4,	8		; addalpha:

	paddd	mm1,	mm3		; add
	add	eax,	byte 8
	psubd	mm1,	mm4		; sub

	movq	[edi],	mm1		; store

	movd	mm3,	[eax+4-8]		; 00|00|00|00|A+|R+|G+|B+
	movd	mm4,	[ebx+4]		; 00|00|00|00|A-|R-|G-|B-
	movq	mm1,	[edi+8]		; A|R|G|B

	punpcklbw	mm3,	mm0		; 00|A+|00|R+|00|G+|00|B+
	punpcklbw	mm4,	mm0		; 00|A-|00|R-|00|G-|00|B-

	movq	mm5,	mm3		; addalpha:
	movq	mm6,	mm4		; addalpha:
	psrlw	mm5,	7		; addalpha:
	psrlw	mm6,	7		; addalpha:
%ifdef USE_EMMX
	prefetcht0	[ebx + 64]
%endif
	paddw	mm5,	mm3		; addalpha: adjust alpha
	paddw	mm6,	mm4		; addalpha: adjust alpha
	psrlq	mm5,	48		; addalpha: 0000|0000|0000|A+
	psrlq	mm6,	48		; addalpha: 0000|0000|0000|A-
	punpcklwd	mm5,	mm5		; addalpha: 0000|0000|A+|A+
	punpcklwd	mm6,	mm6		; addalpha: 0000|0000|A-|A-
	punpcklwd	mm5,	mm5		; addalpha: A+|A+|A+|A+
	punpcklwd	mm6,	mm6		; addalpha: A-|A-|A-|A-
	pand	mm5,	[c_0000ffffffffffff]	; addalpha:
	pand	mm6,	[c_0000ffffffffffff]	; addalpha:
	por	mm5,	[c_0100000000000000]	; addalpha: 0100|A+|A+|A+
	por	mm6,	[c_0100000000000000]	; addalpha: 0100|A-|A-|A-
	pmullw	mm3,	mm5		; addalpha:
	pmullw	mm4,	mm6		; addalpha:
	psrlw	mm3,	8		; addalpha:
	psrlw	mm4,	8		; addalpha:

	add	edi,	byte 16
	paddd	mm1,	mm3		; add
	add	ebx,	byte 8
	psubd	mm1,	mm4		; sub

	cmp	edi,	esi
	movq	[edi+8-16],	mm1		; store

	jb	near .ploop

.pfraction:
	add	esi,	byte 24
	cmp	edi,	esi
	jae	.pexit			; jump if edi >= esi

.ploop2:	; fractions
	movq	mm1,	[edi]		; A|R|G|B
	movd	mm3,	[eax]		; 00|00|00|00|A+|R+|G+|B+
	movd	mm4,	[ebx]		; 00|00|00|00|A-|R-|G-|B-

	punpcklbw	mm3,	mm0		; 00|A+|00|R+|00|G+|00|B+
	punpcklbw	mm4,	mm0		; 00|A-|00|R-|00|G-|00|B-

	movq	mm5,	mm3		; addalpha:
	movq	mm6,	mm4		; addalpha:
	psrlw	mm5,	7		; addalpha:
	psrlw	mm6,	7		; addalpha:
	paddw	mm5,	mm3		; addalpha: adjust alpha
	paddw	mm6,	mm4		; addalpha: adjust alpha
	psrlq	mm5,	48		; addalpha: 0000|0000|0000|A+
	psrlq	mm6,	48		; addalpha: 0000|0000|0000|A-
	punpcklwd	mm5,	mm5		; addalpha: 0000|0000|A+|A+
	punpcklwd	mm6,	mm6		; addalpha: 0000|0000|A-|A-
	punpcklwd	mm5,	mm5		; addalpha: A+|A+|A+|A+
	punpcklwd	mm6,	mm6		; addalpha: A-|A-|A-|A-
	pand	mm5,	[c_0000ffffffffffff]	; addalpha:
	pand	mm6,	[c_0000ffffffffffff]	; addalpha:
	por	mm5,	[c_0100000000000000]	; addalpha: 0100|A+|A+|A+
	por	mm6,	[c_0100000000000000]	; addalpha: 0100|A-|A-|A-
	pmullw	mm3,	mm5		; addalpha:
	pmullw	mm4,	mm6		; addalpha:
	psrlw	mm3,	8		; addalpha:
	psrlw	mm4,	8		; addalpha:

	paddd	mm1,	mm3
	psubd	mm1,	mm4

	movq	[edi],	mm1

	add	edi,	byte 8
	add	eax,	byte 4
	add	ebx,	byte 4

	cmp	edi,	esi
	jb	short .ploop2		; jump if edi < esi

.pexit:
	pop	ebp
	pop	edx
	pop	ecx
	pop	ebx
	pop	esi
	pop	edi
	emms
	ret

;--------------------------------------------------------------------

	function_align
TVPAddSubVertSum32_name:			; vertical addition of pixel valus, 32bit precision
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 40]		; len
	cmp	ecx,	byte 0
	jle	near .pexit
	pxor	mm0,	mm0
	mov	edi,	[esp + 28]		; dest
	mov	eax,	[esp + 32]		; addline
	mov	ebx,	[esp + 36]		; subline
	shl	ecx,	4
	lea	esi,	[edi + ecx]		; limit
	sub	esi,	byte 16		; 1*16
	cmp	edi,	esi

	jae	near .pfraction		; jump if edi >= esi

	loop_align
.ploop:
	movq	mm1,	[edi]		; G|B
	movq	mm2,	[edi+8]		; A|R
	movd	mm3,	[eax]		; 00|00|00|00|A+|R+|G+|B+
	movd	mm4,	[ebx]		; 00|00|00|00|A-|R-|G-|B-

	punpcklbw	mm3,	mm0		; 00|A+|00|R+|00|G+|00|B+
	punpcklbw	mm4,	mm0		; 00|A-|00|R-|00|G-|00|B-

	movq	mm5,	mm3
	movq	mm6,	mm4

%ifdef USE_EMMX
	prefetcht0	[eax + 64]
%endif
	punpcklwd	mm3,	mm0		; 00|00|00|G+|00|00|00|B+
	punpcklwd	mm4,	mm0		; 00|00|00|G-|00|00|00|B-

	paddd	mm1,	mm3

	punpckhwd	mm5,	mm0		; 00|00|00|A+|00|00|00|R+
	psubd	mm1,	mm4
	paddd	mm2,	mm5
	punpckhwd	mm6,	mm0		; 00|00|00|A-|00|00|00|R-

	psubd	mm2,	mm6

	movq	[edi],	mm1
	movq	[edi+8],	mm2

	add	eax,	byte 8
	add	ebx,	byte 8

	movq	mm1,	[edi+16]		; G|B
	movq	mm2,	[edi+8+16]		; A|R
	movd	mm3,	[eax+4-8]		; 00|00|00|00|A+|R+|G+|B+
	movd	mm4,	[ebx+4-8]		; 00|00|00|00|A-|R-|G-|B-

	punpcklbw	mm3,	mm0		; 00|A+|00|R+|00|G+|00|B+
	punpcklbw	mm4,	mm0		; 00|A-|00|R-|00|G-|00|B-

	add	edi,	byte 32

	movq	mm5,	mm3
	movq	mm6,	mm4

%ifdef USE_EMMX
	prefetcht0	[ebx + 64]
%endif

	punpcklwd	mm3,	mm0		; 00|00|00|G+|00|00|00|B+
	punpcklwd	mm4,	mm0		; 00|00|00|G-|00|00|00|B-

	paddd	mm1,	mm3
	cmp	edi,	esi

	punpckhwd	mm5,	mm0		; 00|00|00|A+|00|00|00|R+
	psubd	mm1,	mm4
	paddd	mm2,	mm5
	punpckhwd	mm6,	mm0		; 00|00|00|A-|00|00|00|R-

	psubd	mm2,	mm6

	movq	[edi+16-32],	mm1
	movq	[edi+8+16-32],	mm2

	jb	near .ploop

.pfraction:
	add	esi,	byte 16
	cmp	edi,	esi
	jae	.pexit			; jump if edi >= esi

.ploop2:	; fractions
	movq	mm1,	[edi]		; G|B
	movq	mm2,	[edi+8]		; A|R
	movd	mm3,	[eax]		; 00|00|00|00|A+|R+|G+|B+
	movd	mm4,	[ebx]		; 00|00|00|00|A-|R-|G-|B-

	punpcklbw	mm3,	mm0		; 00|A+|00|R+|00|G+|00|B+
	punpcklbw	mm4,	mm0		; 00|A-|00|R-|00|G-|00|B-

	movq	mm5,	mm3
	movq	mm6,	mm4

	punpcklwd	mm3,	mm0		; 00|00|00|G+|00|00|00|B+
	punpcklwd	mm4,	mm0		; 00|00|00|G-|00|00|00|B-

	paddd	mm1,	mm3
	psubd	mm1,	mm4

	punpckhwd	mm5,	mm0		; 00|00|00|A+|00|00|00|R+
	punpckhwd	mm6,	mm0		; 00|00|00|A-|00|00|00|R-

	paddd	mm2,	mm5
	psubd	mm2,	mm6

	movq	[edi],	mm1
	movq	[edi+8],	mm2

	add	edi,	byte 16
	add	eax,	byte 4
	add	ebx,	byte 4
	cmp	edi,	esi

	jb	short .ploop2		; jump if edi < esi

.pexit:
	pop	ebp
	pop	edx
	pop	ecx
	pop	ebx
	pop	esi
	pop	edi
	emms
	ret

;--------------------------------------------------------------------
	function_align
TVPAddSubVertSum32_d_name:			; vertical addition of pixel valus with alpha, 32bit precision
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 40]		; len
	cmp	ecx,	byte 0
	jle	near .pexit
	pxor	mm0,	mm0
	mov	edi,	[esp + 28]		; dest
	mov	eax,	[esp + 32]		; addline
	mov	ebx,	[esp + 36]		; subline
	shl	ecx,	4
	lea	esi,	[edi + ecx]		; limit
	sub	esi,	byte 16		; 1*16
	cmp	edi,	esi

	jae	near .pfraction		; jump if edi >= esi

	loop_align
.ploop:
	movq	mm1,	[edi]		; load: G|B
	movq	mm2,	[edi+8]		; load: A|R
	movd	mm3,	[eax]		; load: 00|00|00|00|A+|R+|G+|B+
	movd	mm4,	[ebx]		; load: 00|00|00|00|A-|R-|G-|B-
	punpcklbw	mm3,	mm0		; load: 00|A+|00|R+|00|G+|00|B+
	punpcklbw	mm4,	mm0		; load: 00|A-|00|R-|00|G-|00|B-

	movq	mm5,	mm3		; addalpha:
	movq	mm6,	mm4		; addalpha:
%ifdef USE_EMMX
	prefetcht0	[eax + 64]
%endif
	psrlw	mm5,	7		; addalpha:
	psrlw	mm6,	7		; addalpha:
	paddw	mm5,	mm3		; addalpha: adjust alpha
	paddw	mm6,	mm4		; addalpha: adjust alpha
	psrlq	mm5,	48		; addalpha: 0000|0000|0000|A+
	psrlq	mm6,	48		; addalpha: 0000|0000|0000|A-
	punpcklwd	mm5,	mm5		; addalpha: 0000|0000|A+|A+
	punpcklwd	mm6,	mm6		; addalpha: 0000|0000|A-|A-
	punpcklwd	mm5,	mm5		; addalpha: A+|A+|A+|A+
	punpcklwd	mm6,	mm6		; addalpha: A-|A-|A-|A-
	pand	mm5,	[c_0000ffffffffffff]	; addalpha:
	pand	mm6,	[c_0000ffffffffffff]	; addalpha:
	por	mm5,	[c_0100000000000000]	; addalpha: 0100|A+|A+|A+
	por	mm6,	[c_0100000000000000]	; addalpha: 0100|A-|A-|A-
	pmullw	mm3,	mm5		; addalpha:
	pmullw	mm4,	mm6		; addalpha:
	psrlw	mm3,	8		; addalpha:
	psrlw	mm4,	8		; addalpha:

	movq	mm5,	mm3		; sum:
	movq	mm6,	mm4		; sum:
	punpcklwd	mm3,	mm0		; sum: 00|00|00|G+|00|00|00|B+
	punpcklwd	mm4,	mm0		; sum: 00|00|00|G-|00|00|00|B-
	punpckhwd	mm5,	mm0		; sum: 00|00|00|A+|00|00|00|R+
	punpckhwd	mm6,	mm0		; sum: 00|00|00|A-|00|00|00|R-
	paddd	mm1,	mm3		; sum:
	paddd	mm2,	mm5		; sum:
	psubd	mm1,	mm4		; sum:
	psubd	mm2,	mm6		; sum:

	movq	[edi],	mm1		; store:
	movq	[edi+8],	mm2		; store:

	movq	mm1,	[edi+16]		; load: G|B
	movq	mm2,	[edi+8+16]		; load: A|R
	movd	mm3,	[eax+4]		; load: 00|00|00|00|A+|R+|G+|B+
	movd	mm4,	[ebx+4]		; load: 00|00|00|00|A-|R-|G-|B-
	punpcklbw	mm3,	mm0		; load: 00|A+|00|R+|00|G+|00|B+
	punpcklbw	mm4,	mm0		; load: 00|A-|00|R-|00|G-|00|B-

	movq	mm5,	mm3		; addalpha:
	movq	mm6,	mm4		; addalpha:
%ifdef USE_EMMX
	prefetcht0	[ebx + 64]
%endif
	psrlw	mm5,	7		; addalpha:
	psrlw	mm6,	7		; addalpha:
	paddw	mm5,	mm3		; addalpha: adjust alpha
	paddw	mm6,	mm4		; addalpha: adjust alpha
	psrlq	mm5,	48		; addalpha: 0000|0000|0000|A+
	psrlq	mm6,	48		; addalpha: 0000|0000|0000|A-
	punpcklwd	mm5,	mm5		; addalpha: 0000|0000|A+|A+
	punpcklwd	mm6,	mm6		; addalpha: 0000|0000|A-|A-
	punpcklwd	mm5,	mm5		; addalpha: A+|A+|A+|A+
	punpcklwd	mm6,	mm6		; addalpha: A-|A-|A-|A-
	pand	mm5,	[c_0000ffffffffffff]	; addalpha:
	pand	mm6,	[c_0000ffffffffffff]	; addalpha:
	por	mm5,	[c_0100000000000000]	; addalpha: 0100|A+|A+|A+
	por	mm6,	[c_0100000000000000]	; addalpha: 0100|A-|A-|A-
	pmullw	mm3,	mm5		; addalpha:
	pmullw	mm4,	mm6		; addalpha:
	psrlw	mm3,	8		; addalpha:
	psrlw	mm4,	8		; addalpha:

	add	edi,	byte 32
	add	eax,	byte 8
	add	ebx,	byte 8
	cmp	edi,	esi

	movq	mm5,	mm3		; sum:
	movq	mm6,	mm4		; sum:
	punpcklwd	mm3,	mm0		; sum: 00|00|00|G+|00|00|00|B+
	punpcklwd	mm4,	mm0		; sum: 00|00|00|G-|00|00|00|B-
	punpckhwd	mm5,	mm0		; sum: 00|00|00|A+|00|00|00|R+
	punpckhwd	mm6,	mm0		; sum: 00|00|00|A-|00|00|00|R-
	paddd	mm1,	mm3		; sum:
	paddd	mm2,	mm5		; sum:
	psubd	mm1,	mm4		; sum:
	psubd	mm2,	mm6		; sum:

	movq	[edi+16-32],	mm1	; store:
	movq	[edi+8+16-32],	mm2	; store:

	jb	near .ploop

.pfraction:
	add	esi,	byte 16
	cmp	edi,	esi
	jae	near .pexit			; jump if edi >= esi

.ploop2:	; fractions
	movq	mm1,	[edi]		; load: G|B
	movq	mm2,	[edi+8]		; load: A|R
	movd	mm3,	[eax]		; load: 00|00|00|00|A+|R+|G+|B+
	movd	mm4,	[ebx]		; load: 00|00|00|00|A-|R-|G-|B-
	punpcklbw	mm3,	mm0		; load: 00|A+|00|R+|00|G+|00|B+
	punpcklbw	mm4,	mm0		; load: 00|A-|00|R-|00|G-|00|B-

	movq	mm5,	mm3		; addalpha:
	movq	mm6,	mm4		; addalpha:
	psrlw	mm5,	7		; addalpha:
	psrlw	mm6,	7		; addalpha:
	paddw	mm5,	mm3		; addalpha: adjust alpha
	paddw	mm6,	mm4		; addalpha: adjust alpha
	psrlq	mm5,	48		; addalpha: 0000|0000|0000|A+
	psrlq	mm6,	48		; addalpha: 0000|0000|0000|A-
	punpcklwd	mm5,	mm5		; addalpha: 0000|0000|A+|A+
	punpcklwd	mm6,	mm6		; addalpha: 0000|0000|A-|A-
	punpcklwd	mm5,	mm5		; addalpha: A+|A+|A+|A+
	punpcklwd	mm6,	mm6		; addalpha: A-|A-|A-|A-
	pand	mm5,	[c_0000ffffffffffff]	; addalpha:
	pand	mm6,	[c_0000ffffffffffff]	; addalpha:
	por	mm5,	[c_0100000000000000]	; addalpha: 0100|A+|A+|A+
	por	mm6,	[c_0100000000000000]	; addalpha: 0100|A-|A-|A-
	pmullw	mm3,	mm5		; addalpha:
	pmullw	mm4,	mm6		; addalpha:
	psrlw	mm3,	8		; addalpha:
	psrlw	mm4,	8		; addalpha:

	movq	mm5,	mm3		; sum:
	movq	mm6,	mm4		; sum:
	punpcklwd	mm3,	mm0		; sum: 00|00|00|G+|00|00|00|B+
	punpcklwd	mm4,	mm0		; sum: 00|00|00|G-|00|00|00|B-
	paddd	mm1,	mm3		; sum:
	psubd	mm1,	mm4		; sum:
	punpckhwd	mm5,	mm0		; sum: 00|00|00|A+|00|00|00|R+
	punpckhwd	mm6,	mm0		; sum: 00|00|00|A-|00|00|00|R-
	paddd	mm2,	mm5		; sum:
	psubd	mm2,	mm6		; sum:

	movq	[edi],	mm1		; store:
	movq	[edi+8],	mm2		; store:

	add	edi,	byte 16
	add	eax,	byte 4
	add	ebx,	byte 4

	cmp	edi,	esi
	jb	near .ploop2		; jump if edi < esi

.pexit:
	pop	ebp
	pop	edx
	pop	ecx
	pop	ebx
	pop	esi
	pop	edi
	emms
	ret

;--------------------------------------------------------------------

	function_align
TVPDoBoxBlurAvg16_name:			; do blur using box-blur algorithm, 16bit precision
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp

	mov	eax,	[esp + 44]		; n
	cmp	eax,	3
	IF	ge
	  cmp	eax,	128
	  IF	l
	    ; if n>=3 && n < 128
	    jmp near  .TVPDoBoxBlurAvg15		; do 15bit precition routine
	  ENDIF
	ENDIF

	mov	ecx,	[esp + 48]		; len
	cmp	ecx,	byte 0
	jle	near .pexit2

	shr	eax,	1		; eax = n / 2
	movd	mm7,	eax
	punpcklwd	mm7,	mm7		; mm7 = 0000|0000|half_n|half_n
	punpcklwd	mm7,	mm7		; mm7 = half_n|half_n|half_n|half_n

	pxor	mm0,	mm0		; mm0 = 0

	mov	ebx,	[esp + 44]		; n
	mov	eax,	(1<<16)
	xor	edx,	edx
	idiv	ebx			; eax = rcp = (1<<16) / n
	shl	eax,	16
	mov	esi,	eax		; esi = rcp

	mov	edi,	[esp + 28]		; dest

	mov	ebp,	[esp + 32]		; sum
	movq	mm5,	[ebp]		; mm5 = As|Rs|Gs|Bs

	mov	ebx,	[esp + 36]		; addptr
	mov	ebp,	[esp + 40]		; subptr

	lea	eax,	[edi + ecx*4]	; limit
	mov	[esp-8],	eax		; [esp-8] = limit

	sub	dword[esp-8],	byte 4	; 1*4
	cmp	edi,	[esp-8]

	jae	near .pfraction		; jump if edi >= limit

	loop_align
.ploop:

	movq	mm1,	mm5		; mm1 = As|Rs|Gs|Bs
	paddw	mm1,	mm7		; mm1 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN

	movq	mm2,	mm1
	punpcklwd	mm1,	mm0		; mm1 = 0000|Gs+HalfN|0000|Bs+HalfN
	punpckhwd	mm2,	mm0		; mm1 = 0000|As+HalfN|0000|Rs+HalfN

	movd	eax,	mm1
	mul	esi
	paddw	mm5,	[ebx]		; sum += *addptr
	mov	ecx,	edx

	psrlq	mm1,	32
	movd	eax,	mm1
	mul	esi
	psubw	mm5,	[ebp]		; sum -= *subptr
	shl	edx,	8
	add	ecx,	edx

	movd	eax,	mm2
	mul	esi
	movq	mm1,	mm5		; mm1 = As|Rs|Gs|Bs
	shl	edx,	16
	add	ecx,	edx

	psrlq	mm2,	32
	movd	eax,	mm2
	mul	esi
	paddw	mm1,	mm7		; mm1 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN
	shl	edx,	24
	movq	mm2,	mm1
	add	ecx,	edx

	mov	[edi],	ecx

	punpcklwd	mm1,	mm0		; mm1 = 0000|Gs+HalfN|0000|Bs+HalfN
	punpckhwd	mm2,	mm0		; mm1 = 0000|As+HalfN|0000|Rs+HalfN

	movd	eax,	mm1
	add	edi,	byte 8
	mul	esi
	paddw	mm5,	[ebx+8]		; sum += *addptr
	mov	ecx,	edx

	psrlq	mm1,	32
	movd	eax,	mm1
	mul	esi
	psubw	mm5,	[ebp+8]		; sum -= *subptr
	shl	edx,	8
	add	ecx,	edx

	movd	eax,	mm2
	mul	esi
	add	ebx,	byte 16
	shl	edx,	16
	add	ecx,	edx

	psrlq	mm2,	32
	movd	eax,	mm2
	mul	esi
	add	ebp,	byte 16
	shl	edx,	24
	add	ecx,	edx

	cmp	edi,	[esp-8]
	mov	[edi+4-8],	ecx

	jb	near .ploop

.pfraction:
	add	dword[esp-8],	byte 4
	cmp	edi,	[esp-8]
	jae	.pexit			; jump if edi >= limit

.ploop2:	; fractions
	movq	mm1,	mm5		; mm1 = As|Rs|Gs|Bs
	paddw	mm1,	mm7		; mm1 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN

	movq	mm2,	mm1
	punpcklwd	mm1,	mm0		; mm1 = 0000|Gs+HalfN|0000|Bs+HalfN
	punpckhwd	mm2,	mm0		; mm1 = 0000|As+HalfN|0000|Rs+HalfN

	movd	eax,	mm1
	mul	esi
	mov	ecx,	edx

	psrlq	mm1,	32
	movd	eax,	mm1
	mul	esi
	shl	edx,	8
	add	ecx,	edx

	movd	eax,	mm2
	mul	esi
	shl	edx,	16
	add	ecx,	edx

	psrlq	mm2,	32
	movd	eax,	mm2
	mul	esi
	shl	edx,	24
	add	ecx,	edx

	mov	[edi],	ecx

	paddw	mm5,	[ebx]		; sum += *addptr
	psubw	mm5,	[ebp]		; sum -= *subptr

	add	ebx,	byte 8
	add	ebp,	byte 8
	add	edi,	byte 4

	cmp	edi,	[esp-8]
	jb	short .ploop2		; jump if edi < limit

.pexit:
	mov	ebp,	[esp + 32]		; sum
	movq	[ebp],	mm5		; store sum

.pexit2:
	pop	ebp
	pop	edx
	pop	ecx
	pop	ebx
	pop	esi
	pop	edi
	emms
	ret

.TVPDoBoxBlurAvg15:				; 15bit precision routine
	mov	ecx,	[esp + 48]		; len
	cmp	ecx,	byte 0
	jle	near .epexit2

	pxor	mm0,	mm0		; mm0 = 0

	mov	eax,	[esp + 44]		; n
	shr	eax,	1		; eax = n / 2
	movd	mm7,	eax
	punpcklwd	mm7,	mm7		; mm7 = 0000|0000|half_n|half_n
	punpcklwd	mm7,	mm7		; mm7 = half_n|half_n|half_n|half_n

	mov	ebx,	[esp + 44]		; n
	mov	eax,	(1<<16)
	xor	edx,	edx
	idiv	ebx			; eax = rcp = (1<<16) / n
	movd	mm6,	eax
	punpcklwd	mm6,	mm6		; mm6 = 0000|0000|rcp|rcp
	punpcklwd	mm6,	mm6		; mm6 = rcp|rcp|rcp|rcp

	mov	edi,	[esp + 28]		; dest

	mov	ebp,	[esp + 32]		; sum
	movq	mm5,	[ebp]		; mm5 = As|Rs|Gs|Bs

	mov	ebx,	[esp + 36]		; addptr
	mov	ebp,	[esp + 40]		; subptr

	lea	eax,	[edi + ecx*4]	; limit
	mov	esi,	eax		; esi = limit

	sub	esi,	byte 12	; 3*4
	cmp	edi,	esi

	jae	near .epfraction		; jump if edi >= limit

	loop_align
.eploop:
	movq	mm1,	mm5		; 1 mm1 = As|Rs|Gs|Bs
	add	ebp,	byte 32
	add	edi,	byte 16
	paddw	mm1,	mm7		; 1 mm1 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN
	paddw	mm5,	[ebx]		; 1 sum += *addptr
	pmulhw	mm1,	mm6		; 1 mm1 = Aavg|Ravg|Gavg|Bavg
	psubw	mm5,	[ebp-32]		; 1 sum -= *subptr
	packuswb	mm1,	mm1		; 1 pack mm1
	movq	mm2,	mm5		; 2 mm2 = As|Rs|Gs|Bs
	movd	[edi-16],	mm1
	paddw	mm2,	mm7		; 2 mm2 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN
	paddw	mm5,	[ebx+8]		; 2 sum += *addptr
	pmulhw	mm2,	mm6		; 2 mm2 = Aavg|Ravg|Gavg|Bavg
	psubw	mm5,	[ebp+8-32]		; 2 sum -= *subptr
	packuswb	mm2,	mm2		; 2 pack mm2
	movq	mm3,	mm5		; 3 mm3 = As|Rs|Gs|Bs
	movd	[edi+4-16],	mm2
	paddw	mm3,	mm7		; 3 mm3 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN
	paddw	mm5,	[ebx+16]		; 3 sum += *addptr
	pmulhw	mm3,	mm6		; 3 mm3 = Aavg|Ravg|Gavg|Bavg
	psubw	mm5,	[ebp+16-32]		; 3 sum -= *subptr
	packuswb	mm3,	mm3		; 3 pack mm3
	add	ebx,	byte 32
	movq	mm4,	mm5		; 4 mm4 = As|Rs|Gs|Bs
	movd	[edi+8-16],	mm3
	paddw	mm4,	mm7		; 4 mm4 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN
	paddw	mm5,	[ebx+24-32]		; 4 sum += *addptr
	pmulhw	mm4,	mm6		; 4 mm4 = Aavg|Ravg|Gavg|Bavg
	psubw	mm5,	[ebp+24-32]		; 4 sum -= *subptr
	packuswb	mm4,	mm4		; 4 pack mm4
	cmp	edi,	esi
	movd	[edi+12-16],	mm4
	jb	near .eploop

.epfraction:
	add	esi,	byte 12
	cmp	edi,	esi
	jae	.epexit			; jump if edi >= limit

.eploop2:	; fractions
	movq	mm1,	mm5		; mm1 = As|Rs|Gs|Bs
	paddw	mm1,	mm7		; mm1 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN
	pmulhw	mm1,	mm6		; mm1 = Aavg|Ravg|Gavg|Bavg
	packuswb	mm1,	mm1		; pack mm1
	movd	[edi],	mm1

	paddw	mm5,	[ebx]		; sum += *addptr
	psubw	mm5,	[ebp]		; sum -= *subptr

	add	ebx,	byte 8
	add	ebp,	byte 8
	add	edi,	byte 4

	cmp	edi,	esi
	jb	short .eploop2		; jump if edi < limit

.epexit:
	mov	ebp,	[esp + 32]		; sum
	movq	[ebp],	mm5		; store sum

.epexit2:
	pop	ebp
	pop	edx
	pop	ecx
	pop	ebx
	pop	esi
	pop	edi
	emms
	ret

;--------------------------------------------------------------------

	function_align
TVPDoBoxBlurAvg16_d_name:			; do blur using box-blur algorithm with alpha, 16bit precision
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp

	mov	eax,	[esp + 44]		; n
	cmp	eax,	3
	IF	ge
	  cmp	eax,	128
	  IF	l
	    ; if n>=3 && n < 128
	    jmp near  .TVPDoBoxBlurAvg15_d		; do 15bit precition routine
	  ENDIF
	ENDIF

	mov	ecx,	[esp + 48]		; len
	cmp	ecx,	byte 0
	jle	near .pexit2

	pxor	mm0,	mm0		; mm0 = 0

	mov	eax,	[esp + 44]		; n
	shr	eax,	1		; eax = n / 2
	movd	mm7,	eax
	punpcklwd	mm7,	mm7		; mm7 = 0000|0000|half_n|half_n
	punpcklwd	mm7,	mm7		; mm7 = half_n|half_n|half_n|half_n

	mov	ebx,	[esp + 44]		; n
	mov	eax,	(1<<16)
	xor	edx,	edx
	idiv	ebx			; eax = rcp = (1<<16) / n
	shl	eax,	16
	mov	esi,	eax		; esi = rcp

	mov	edi,	[esp + 28]		; dest

	mov	ebp,	[esp + 32]		; sum
	movq	mm5,	[ebp]		; mm5 = As|Rs|Gs|Bs

	mov	ebp,	[esp + 40]		; subptr

	lea	eax,	[edi + ecx*4]	; limit
	mov	[esp-8],	eax		; [esp-8] = limit

	sub	dword[esp-8],	byte 4	; 1*4
	cmp	edi,	[esp-8]

	jae	near .pfraction		; jump if edi >= limit

	loop_align
.ploop:
	movq	mm1,	mm5		; mm1 = As|Rs|Gs|Bs
	paddw	mm1,	mm7		; mm1 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN

	movq	mm2,	mm1
	mov	ebx,	[esp + 36]		; addptr
	punpcklwd	mm1,	mm0		; mm1 = 0000|Gs+HalfN|0000|Bs+HalfN
	paddw	mm5,	[ebx]		; sum += *addptr
	punpckhwd	mm2,	mm0		; mm1 = 0000|As+HalfN|0000|Rs+HalfN

	movq	mm3,	mm2
	psubw	mm5,	[ebp]		; sum -= *subptr
	psrlq	mm3,	32
%ifdef USE_EMMX
	prefetcht0	[ebx + 64]
%endif
	movd	eax,	mm3
	mul	esi
	mov	ebx,	edx
	shl	edx,	24
	shl	ebx,	8
	add	ebx,	TVPDivTable		; ebx = TVPDivTable[alpha][]
	mov	ecx,	edx

	movd	eax,	mm1
	mul	esi
	movzx	edx,	byte[ebx+edx]	; look up the table
	add	ecx,	edx

	psrlq	mm1,	32
	movd	eax,	mm1
	mul	esi
	movzx	edx,	byte[ebx+edx]	; look up the table
	shl	edx,	8
	add	ecx,	edx

	movd	eax,	mm2
	mul	esi
	movzx	edx,	byte[ebx+edx]	; look up the table
	shl	edx,	16
	add	ecx,	edx

	mov	[edi],	ecx

	movq	mm1,	mm5		; mm1 = As|Rs|Gs|Bs
	paddw	mm1,	mm7		; mm1 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN

	movq	mm2,	mm1
	mov	ebx,	[esp + 36]		; addptr
	punpcklwd	mm1,	mm0		; mm1 = 0000|Gs+HalfN|0000|Bs+HalfN
	paddw	mm5,	[ebx+8]		; sum += *addptr
	punpckhwd	mm2,	mm0		; mm1 = 0000|As+HalfN|0000|Rs+HalfN

	movq	mm3,	mm2
	psubw	mm5,	[ebp+8]		; sum -= *subptr
	psrlq	mm3,	32
	movd	eax,	mm3
	add	edi,	byte 8
	mul	esi
	add	dword[esp + 36],	byte 16
	mov	ebx,	edx
	shl	edx,	24
	shl	ebx,	8
	add	ebx,	TVPDivTable		; ebx = TVPDivTable[alpha][]
	mov	ecx,	edx

	movd	eax,	mm1
	add	ebp,	byte 16
	mul	esi
	movzx	edx,	byte[ebx+edx]	; look up the table
	add	ecx,	edx

	psrlq	mm1,	32
	movd	eax,	mm1
%ifdef USE_EMMX
	prefetcht0	[ebp + 64]
%endif
	mul	esi
	movzx	edx,	byte[ebx+edx]	; look up the table
	shl	edx,	8
	add	ecx,	edx

	movd	eax,	mm2
	mul	esi
	movzx	edx,	byte[ebx+edx]	; look up the table
	shl	edx,	16
	add	ecx,	edx
	cmp	edi,	[esp-8]

	mov	[edi+4-8],	ecx

	jb	near .ploop

.pfraction:
	add	dword[esp-8],	byte 4
	cmp	edi,	[esp-8]
	jae	.pexit			; jump if edi >= limit

.ploop2:	; fractions
	movq	mm1,	mm5		; mm1 = As|Rs|Gs|Bs
	paddw	mm1,	mm7		; mm1 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN

	movq	mm2,	mm1
	punpcklwd	mm1,	mm0		; mm1 = 0000|Gs+HalfN|0000|Bs+HalfN
	punpckhwd	mm2,	mm0		; mm1 = 0000|As+HalfN|0000|Rs+HalfN

	movq	mm3,	mm2
	psrlq	mm3,	32
	movd	eax,	mm3
	mul	esi
	mov	ebx,	edx
	shl	edx,	24
	shl	ebx,	8
	add	ebx,	TVPDivTable		; ebx = TVPDivTable[alpha][]
	mov	ecx,	edx

	movd	eax,	mm1
	mul	esi
	movzx	edx,	byte[ebx+edx]	; look up the table
	add	ecx,	edx

	psrlq	mm1,	32
	movd	eax,	mm1
	mul	esi
	movzx	edx,	byte[ebx+edx]	; look up the table
	shl	edx,	8
	add	ecx,	edx

	movd	eax,	mm2
	mul	esi
	movzx	edx,	byte[ebx+edx]	; look up the table
	shl	edx,	16
	add	ecx,	edx

	mov	[edi],	ecx

	mov	ebx,	[esp + 36]		; addptr
	paddw	mm5,	[ebx]		; sum += *addptr
	psubw	mm5,	[ebp]		; sum -= *subptr

	add	dword[esp + 36],	byte 8
	add	ebp,	byte 8
	add	edi,	byte 4

	cmp	edi,	[esp-8]
	jb	short .ploop2		; jump if edi < limit

.pexit:
	mov	ebp,	[esp + 32]		; sum
	movq	[ebp],	mm5		; store sum

.pexit2:
	pop	ebp
	pop	edx
	pop	ecx
	pop	ebx
	pop	esi
	pop	edi
	emms
	ret

.TVPDoBoxBlurAvg15_d:				; do 15bit precision
	mov	ecx,	[esp + 48]		; len
	cmp	ecx,	byte 0
	jle	near .epexit2

	pxor	mm0,	mm0		; mm0 = 0

	mov	eax,	[esp + 44]		; n
	shr	eax,	1		; eax = n / 2
	movd	mm7,	eax
	punpcklwd	mm7,	mm7		; mm7 = 0000|0000|half_n|half_n
	punpcklwd	mm7,	mm7		; mm7 = half_n|half_n|half_n|half_n

	mov	ebx,	[esp + 44]		; n
	mov	eax,	(1<<16)
	xor	edx,	edx
	idiv	ebx			; eax = rcp = (1<<16) / n
	movd	mm6,	eax
	punpcklwd	mm6,	mm6		; mm6 = 0000|0000|rcp|rcp
	punpcklwd	mm6,	mm6		; mm6 = rcp|rcp|rcp|rcp

	mov	edi,	[esp + 28]		; dest

	mov	ebp,	[esp + 32]		; sum
	movq	mm5,	[ebp]		; mm5 = As|Rs|Gs|Bs

	mov	ebx,	[esp + 36]		; addptr
	mov	ebp,	[esp + 40]		; subptr

	lea	eax,	[edi + ecx*4]	; limit
	mov	esi,	eax		; esi = limit

	sub	esi,	byte 12	; 3*4
	cmp	edi,	esi

	jae	near .epfraction		; jump if edi >= limit

	loop_align
.eploop:
	movq	mm1,	mm5		; mm1 = As|Rs|Gs|Bs
	paddw	mm1,	mm7		; mm1 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN
	pmulhw	mm1,	mm6		; mm1 = Aavg|Ravg|Gavg|Bavg
	packuswb	mm1,	mm0
	movd	ecx,	mm1
	shr	ecx,	16
	and	ecx,	0xff00
	paddw	mm5,	[ebx]		; sum += *addptr
	add	ecx,	TVPDivTable		; ecx = TVPDivTable[Alpha][]
	movd	edx,	mm1
	and	edx,	0xff000000
	movd	eax,	mm1
	psubw	mm5,	[ebp]		; sum -= *subptr
	and	eax,	0xff		; eax = 00|00|00|B
	movzx	eax,byte	[ecx+eax]		; look up the table
	add	edx,	eax		; edx = alpha|00|00|[B]
	movd	eax,	mm1
	movq	mm4,	mm5		; mm4 = As|Rs|Gs|Bs
	shr	eax,	8
	and	eax,	0xff		; eax = Gavg
	paddw	mm4,	mm7		; mm4 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN
	movzx	eax,byte	[ecx+eax]		; look up the table
	shl	eax,	8		; eax = 00|00|[G]|00
	pmulhw	mm4,	mm6		; mm4 = Aavg|Ravg|Gavg|Bavg
	add	edx,	eax		; edx = alpha|00|[G]|[B]
	movd	eax,	mm1
	packuswb	mm4,	mm0
	shr	eax,	16
	and	eax,	0xff		; eax = Ravg
	movzx	eax,byte	[ecx+eax]		; look up the table
	shl	eax,	16		; eax = 00|[R]|00|00
	movd	ecx,	mm4
	add	edx,	eax		; edx = alpha|[R]|[G]|[B]
	shr	ecx,	16
	mov	[edi],	edx
	and	ecx,	0xff00
	paddw	mm5,	[ebx+8]		; sum += *addptr
	add	ecx,	TVPDivTable		; ecx = TVPDivTable[Alpha][]
	movd	edx,	mm4
	and	edx,	0xff000000
	movd	eax,	mm4
	psubw	mm5,	[ebp+8]		; sum -= *subptr
	and	eax,	0xff		; eax = 00|00|00|B
	movzx	eax,byte	[ecx+eax]		; look up the table
	add	edx,	eax		; edx = alpha|00|00|[B]
	movd	eax,	mm4
	add	ebx,	byte 16
	shr	eax,	8
	and	eax,	0xff		; eax = Gavg
	movzx	eax,byte	[ecx+eax]		; look up the table
	add	ebp,	byte 16
	shl	eax,	8		; eax = 00|00|[G]|00
	add	edx,	eax		; edx = alpha|00|[G]|[B]
	movd	eax,	mm4
	add	edi,	byte 8
	shr	eax,	16
	and	eax,	0xff		; eax = Ravg
	movzx	eax,byte	[ecx+eax]		; look up the table
	shl	eax,	16		; eax = 00|[R]|00|00
	add	edx,	eax		; edx = alpha|[R]|[G]|[B]
	cmp	edi,	esi
	mov	[edi+4-8],	edx
	jb	near .eploop

.epfraction:
	add	esi,	byte 12
	cmp	edi,	esi
	jae	.epexit			; jump if edi >= limit

.eploop2:	; fractions
	movq	mm1,	mm5		; mm1 = As|Rs|Gs|Bs
	paddw	mm1,	mm7		; mm1 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN
	pmulhw	mm1,	mm6		; mm1 = Aavg|Ravg|Gavg|Bavg
	packuswb	mm1,	mm0

	movd	ecx,	mm1
	shr	ecx,	16
	and	ecx,	0xff00
	add	ecx,	TVPDivTable		; ecx = TVPDivTable[Alpha][]
	movd	edx,	mm1
	and	edx,	0xff000000

	movd	eax,	mm1
	and	eax,	0xff		; eax = 00|00|00|B
	movzx	eax,byte	[ecx+eax]		; look up the table
	add	edx,	eax		; edx = alpha|00|00|[B]

	movd	eax,	mm1
	shr	eax,	8
	and	eax,	0xff		; eax = Gavg
	movzx	eax,byte	[ecx+eax]		; look up the table
	shl	eax,	8		; eax = 00|00|[G]|00
	add	edx,	eax		; edx = alpha|00|[G]|[B]

	movd	eax,	mm1
	shr	eax,	16
	and	eax,	0xff		; eax = Ravg
	movzx	eax,byte	[ecx+eax]		; look up the table
	shl	eax,	16		; eax = 00|[R]|00|00
	add	edx,	eax		; edx = alpha|[R]|[G]|[B]

	mov	[edi],	edx

	paddw	mm5,	[ebx]		; sum += *addptr
	psubw	mm5,	[ebp]		; sum -= *subptr

	add	ebx,	byte 8
	add	ebp,	byte 8
	add	edi,	byte 4

	cmp	edi,	esi
	jb	short .eploop2		; jump if edi < limit

.epexit:
	mov	ebp,	[esp + 32]		; sum
	movq	[ebp],	mm5		; store sum

.epexit2:
	pop	ebp
	pop	edx
	pop	ecx
	pop	ebx
	pop	esi
	pop	edi
	emms
	ret

;--------------------------------------------------------------------

	function_align
TVPDoBoxBlurAvg32_name:			; do blur using box-blur algorithm, 32bit precision
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 48]		; len
	cmp	ecx,	byte 0
	jle	near .pexit2
	pxor	mm0,	mm0		; mm0 = 0

	mov	eax,	[esp + 44]		; n
	shr	eax,	1		; eax = n / 2
	movd	mm7,	eax
	punpckldq	mm7,	mm7		; mm7 = half_n|half_n

	mov	ebx,	[esp + 44]		; n
	xor	eax,	eax
	mov	edx,	1
	idiv	ebx			; eax = (1<<32) / n
	mov	esi,	eax		; store to esi

	mov	edi,	[esp + 28]		; dest
	mov	ebp,	[esp + 32]		; sum

	movq	mm1,	[ebp]		; mm1 = Gs|Bs
	movq	mm2,	[ebp+8]		; mm2 = As|Rs

	mov	ebx,	[esp + 36]		; addptr
	lea	eax,	[edi + ecx*4]	; limit
	mov	ebp,	[esp + 40]		; subptr
	mov	[esp-8],	eax		; esp-8 = limit

	sub	dword[esp-8],	byte 4		; 1*4
	cmp	edi,	[esp-8]

	jae	near .pfraction		; jump if edi >= limit

	loop_align
.ploop:
	movq	mm3,	mm1		; 1: halfn: mm3 = Gs|Bs
%ifdef USE_EMMX
	prefetcht0	[ebp + 64]
%endif
	movq	mm4,	mm2		; 1: halfn: mm4 = As|Rs
	paddd	mm3,	mm7		; 1: halfn: mm3 = Gs+HalfN|Bs+HalfN
	paddd	mm4,	mm7		; 1: halfn: mm4 = As+HalfN|Rs+HalfN
	movd	eax,	mm3		; 1: B: eax = Bs+HalfN
	imul	esi			; 1: B: edx = (Bs+HalfN) / n
	mov	ecx,	edx		; 1: B:
	psrlq	mm3,	32		; 1:
	paddd	mm1,	[ebx]		; 1: addsub:
	movd	eax,	mm3		; 1: G: eax = Gs+HalfN
	paddd	mm2,	[ebx+8]		; 1: addsub:
	imul	esi			; 1: G: edx = (Gs+HalfN) / n

	shl	edx,	8		; 1: G:
	add	ecx,	edx		; 1: G:
	movd	eax,	mm4		; 1: R: eax = Rs+HalfN
	psubd	mm1,	[ebp]		; 1: addsub:
	imul	esi			; 1: R: edx = (Rs+HalfN) / n
	psubd	mm2,	[ebp+8]		; 1: addsub:
%ifdef USE_EMMX
	prefetcht0	[ebx + 64]
%endif
	shl	edx,	16		; 1: R:
	psrlq	mm4,	32		; 1: A:
	add	ecx,	edx		; 1: R:
	movd	eax,	mm4		; 1: A: eax = As+HalfN
	movq	mm3,	mm1		; 2: halfn: mm3 = Gs|Bs
	imul	esi			; 1: A: edx = (As+HalfN) / n
	movq	mm4,	mm2		; 2: halfn: mm4 = As|Rs
	shl	edx,	24		; 1: A:
	paddd	mm3,	mm7		; 2: halfn: mm3 = Gs+HalfN|Bs+HalfN
	add	ecx,	edx		; 1: A:
	paddd	mm4,	mm7		; 2: halfn: mm4 = As+HalfN|Rs+HalfN
	mov	[edi],	ecx		; 1: store result
	movd	eax,	mm3		; 2: B: eax = Bs+HalfN
	imul	esi			; 2: B: edx = (Bs+HalfN) / n
	mov	ecx,	edx		; 2: B:
	psrlq	mm3,	32		; 2:
	paddd	mm1,	[ebx+16]		; 2: addsub:
	movd	eax,	mm3		; 2: G: eax = Gs+HalfN
	paddd	mm2,	[ebx+8+16]		; 2: addsub:
	imul	esi			; 2: G: edx = (Gs+HalfN) / n
	shl	edx,	8		; 2: G:
	add	ecx,	edx		; 2: G:
	movd	eax,	mm4		; 2: R: eax = Rs+HalfN
	psubd	mm1,	[ebp+16]		; 2: addsub:
	imul	esi			; 2: R: edx = (Rs+HalfN) / n
	psubd	mm2,	[ebp+8+16]		; 2: addsub:
	shl	edx,	16		; 2: R:
	psrlq	mm4,	32		; 2: A:
	add	ecx,	edx		; 2: R:
	add	edi,	byte 8
	movd	eax,	mm4		; 2: A: eax = As+HalfN
	add	ebx,	byte 32
	imul	esi			; 2: A: edx = (As+HalfN) / n
	shl	edx,	24		; 2: A:
	add	ebp,	byte 32
	cmp	edi,	[esp-8]
	add	ecx,	edx		; 2: A:
	mov	[edi+4-8],	ecx		; 2: store result

	jb	near .ploop

.pfraction:
	add	dword[esp-8],	byte 4
	cmp	edi,	[esp-8]
	jae	.pexit			; jump if edi >= limit

.ploop2:	; fractions
	movq	mm3,	mm1		; mm3 = Gs|Bs
	movq	mm4,	mm2		; mm4 = As|Rs

	paddd	mm3,	mm7		; mm3 = Gs+HalfN|Bs+HalfN
	paddd	mm4,	mm7		; mm4 = As+HalfN|Rs+HalfN

	movd	eax,	mm3		; eax = Bs+HalfN
	imul	esi			; edx = (Bs+HalfN) / n
	mov	ecx,	edx

	psrlq	mm3,	32
	movd	eax,	mm3		; eax = Gs+HalfN
	imul	esi			; edx = (Gs+HalfN) / n
	shl	edx,	8
	add	ecx,	edx

	movd	eax,	mm4		; eax = Rs+HalfN
	imul	esi			; edx = (Rs+HalfN) / n
	shl	edx,	16
	add	ecx,	edx

	psrlq	mm4,	32
	movd	eax,	mm4		; eax = As+HalfN
	imul	esi			; edx = (As+HalfN) / n
	shl	edx,	24
	add	ecx,	edx

	mov	[edi],	ecx		; store result

	paddd	mm1,	[ebx]
	paddd	mm2,	[ebx+8]

	psubd	mm1,	[ebp]
	psubd	mm2,	[ebp+8]

	add	ebx,	byte 16
	add	ebp,	byte 16

	add	edi,	byte 4
	cmp	edi,	[esp-8]

	jb	short .ploop2		; jump if edi < limit

.pexit:
	mov	ebp,	[esp + 32]		; sum

	movq	[ebp],	mm1		; store sum
	movq	[ebp+8],	mm2		; store sum

.pexit2:
	pop	ebp
	pop	edx
	pop	ecx
	pop	ebx
	pop	esi
	pop	edi
	emms
	ret

;--------------------------------------------------------------------

	function_align
TVPDoBoxBlurAvg32_d_name:			; do blur using box-blur algorithm with alpha, 32bit precision
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 48]		; len
	cmp	ecx,	byte 0
	jle	near .pexit2
	pxor	mm0,	mm0		; mm0 = 0

	mov	eax,	[esp + 44]		; n
	shr	eax,	1		; eax = n / 2
	movd	mm7,	eax
	punpckldq	mm7,	mm7		; mm7 = half_n|half_n

	mov	ebx,	[esp + 44]		; n
	xor	eax,	eax
	mov	edx,	1
	idiv	ebx			; eax = (1<<32) / n
	mov	esi,	eax		; store to esi

	mov	edi,	[esp + 28]		; dest
	mov	ebp,	[esp + 32]		; sum

	movq	mm1,	[ebp]		; mm1 = Gs|Bs
	movq	mm2,	[ebp+8]		; mm2 = As|Rs

	lea	eax,	[edi + ecx*4]	; limit
	mov	ebp,	[esp + 40]		; subptr
	mov	[esp-8],	eax		; esp-8 = limit

	sub	dword[esp-8],	byte 4	; 1*4
	cmp	edi,	[esp-8]

	jae	near .pfraction		; jump if edi >= limit

	loop_align
.ploop:
	movq	mm3,	mm1		; load: mm3 = Gs|Bs
	movq	mm4,	mm2		; load: mm4 = As|Rs
%ifdef USE_EMMX
	prefetcht0	[ebp + 64]
%endif
	paddd	mm3,	mm7		; halfn: mm3 = Gs+HalfN|Bs+HalfN
	paddd	mm4,	mm7		; halfn: mm4 = As+HalfN|Rs+HalfN
	movq	mm5,	mm4		; a:
	psrlq	mm5,	32		; a:
	movd	eax,	mm5		; a: eax = As+HalfN
	imul	esi			; a: edx = (As+HalfN) / n
	mov	ebx,	edx		; a:
	shl	ebx,	8		; a:
	shl	edx,	24		; a:
	mov	ecx,	edx		; a:
	movd	eax,	mm3		; b: eax = Bs+HalfN
	imul	esi			; b: edx = (Bs+HalfN) / n
	movzx	edx,byte	[TVPDivTable+ebx+edx]	; b: lookup the table
	add	ecx,	edx		; b:
	psrlq	mm3,	32		; g:
	movd	eax,	mm3		; g: eax = Gs+HalfN
	imul	esi			; g: edx = (Gs+HalfN) / n
	movzx	edx,byte	[TVPDivTable+ebx+edx]	; g: lookup the table
	shl	edx,	8		; g:
	add	ecx,	edx		; g:
	movd	eax,	mm4		; r: eax = Rs+HalfN
	imul	esi			; r: edx = (Rs+HalfN) / n
	movzx	edx,byte	[TVPDivTable+ebx+edx]	; r: lookup the table
	shl	edx,	16		; r:
	mov	ebx,	[esp + 36]		; add: load addptr
	add	ecx,	edx		; r:
%ifdef USE_EMMX
	prefetcht0	[ebx + 64]
%endif
	psubd	mm1,	[ebp]		; sub:
	mov	[edi],	ecx		; store: store result
	psubd	mm2,	[ebp+8]		; sub:
	paddd	mm1,	[ebx]		; add:
	paddd	mm2,	[ebx+8]		; add:
	movq	mm3,	mm1		; load: mm3 = Gs|Bs
	movq	mm4,	mm2		; load: mm4 = As|Rs
	paddd	mm3,	mm7		; halfn: mm3 = Gs+HalfN|Bs+HalfN
	paddd	mm4,	mm7		; halfn: mm4 = As+HalfN|Rs+HalfN
	movq	mm5,	mm4		; a:
	psrlq	mm5,	32		; a:
	movd	eax,	mm5		; a: eax = As+HalfN
	imul	esi			; a: edx = (As+HalfN) / n
	add	dword[esp + 36],	byte 32
	mov	ebx,	edx		; a:
	shl	ebx,	8		; a:
	shl	edx,	24		; a:
	mov	ecx,	edx		; a:
	movd	eax,	mm3		; b: eax = Bs+HalfN
	imul	esi			; b: edx = (Bs+HalfN) / n
	add	ebp,	byte 32
	movzx	edx,byte	[TVPDivTable+ebx+edx]	; b: lookup the table
	add	ecx,	edx		; b:
	psrlq	mm3,	32		; g:
	add	edi,	byte 8
	movd	eax,	mm3		; g: eax = Gs+HalfN
	imul	esi			; g: edx = (Gs+HalfN) / n
	movzx	edx,byte	[TVPDivTable+ebx+edx]	; g: lookup the table
	shl	edx,	8		; g:
	add	ecx,	edx		; g:
	movd	eax,	mm4		; r: eax = Rs+HalfN
	imul	esi			; r: edx = (Rs+HalfN) / n
	movzx	edx,byte	[TVPDivTable+ebx+edx]	; r: lookup the table
	shl	edx,	16		; r:
	mov	ebx,	[esp + 36]		; add: load addptr
	add	ecx,	edx		; r:
	cmp	edi,	[esp-8]
	psubd	mm1,	[ebp+16-32]		; sub:
	mov	[edi+4-8],	ecx		; store: store result
	psubd	mm2,	[ebp+8+16-32]	; sub:
	paddd	mm1,	[ebx+16-32]		; add:
	paddd	mm2,	[ebx+8+16-32]	; add:

	jb	near .ploop

.pfraction:
	add	dword[esp-8],	byte 4
	cmp	edi,	[esp-8]
	jae	near .pexit			; jump if edi >= limit

.ploop2:	; fractions
	movq	mm3,	mm1		; load: mm3 = Gs|Bs
	movq	mm4,	mm2		; load: mm4 = As|Rs
	paddd	mm3,	mm7		; halfn: mm3 = Gs+HalfN|Bs+HalfN
	paddd	mm4,	mm7		; halfn: mm4 = As+HalfN|Rs+HalfN

	movq	mm5,	mm4		; a:
	psrlq	mm5,	32		; a:
	movd	eax,	mm5		; a: eax = As+HalfN
	imul	esi			; a: edx = (As+HalfN) / n
	mov	ebx,	edx		; a:
	shl	ebx,	8		; a:
	shl	edx,	24		; a:
	mov	ecx,	edx		; a:

	movd	eax,	mm3		; b: eax = Bs+HalfN
	imul	esi			; b: edx = (Bs+HalfN) / n
	movzx	edx,byte	[TVPDivTable+ebx+edx]	; b: lookup the table
	add	ecx,	edx		; b:

	psrlq	mm3,	32		; g:
	movd	eax,	mm3		; g: eax = Gs+HalfN
	imul	esi			; g: edx = (Gs+HalfN) / n
	movzx	edx,byte	[TVPDivTable+ebx+edx]	; g: lookup the table
	shl	edx,	8		; g:
	add	ecx,	edx		; g:

	movd	eax,	mm4		; r: eax = Rs+HalfN
	imul	esi			; r: edx = (Rs+HalfN) / n
	movzx	edx,byte	[TVPDivTable+ebx+edx]	; r: lookup the table
	shl	edx,	16		; r:
	add	ecx,	edx		; r:

	mov	[edi],	ecx		; store: store result

	psubd	mm1,	[ebp]		; sub:
	psubd	mm2,	[ebp+8]		; sub:

	mov	ebx,	[esp + 36]		; add: load addptr
	paddd	mm1,	[ebx]		; add:
	paddd	mm2,	[ebx+8]		; add:

	add	dword[esp + 36],	byte 16
	add	ebp,	byte 16
	add	edi,	byte 4

	cmp	edi,	[esp-8]
	jb	near .ploop2		; jump if edi < limit

.pexit:
	mov	ebp,	[esp + 32]		; sum

	movq	[ebp],	mm1		; store sum
	movq	[ebp+8],	mm2		; store sum

.pexit2:
	pop	ebp
	pop	edx
	pop	ecx
	pop	ebx
	pop	esi
	pop	edi
	emms
	ret


;--------------------------------------------------------------------
%endif


%ifdef GEN_SSE

;--------------------------------------------------------------------
; SSE stuff
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_SSE && TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPDoBoxBlurAvg16
;;void, TVPDoBoxBlurAvg16_sse_a, (tjs_uint32 *dest, tjs_uint16 *sum, const tjs_uint16 * add, const tjs_uint16 * sub, tjs_int n, tjs_int len)
;--------------------------------------------------------------------

	function_align
TVPDoBoxBlurAvg16_sse_a:			; do blur using box-blur algorithm, 16bit precision
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 48]		; len
	cmp	ecx,	byte 0
	jle	near .pexit2

	pxor	mm0,	mm0		; mm0 = 0

	mov	eax,	[esp + 44]		; n
	shr	eax,	1		; eax = n / 2
	movd	mm7,	eax
	punpcklwd	mm7,	mm7		; mm7 = 0000|0000|half_n|half_n
	punpcklwd	mm7,	mm7		; mm7 = half_n|half_n|half_n|half_n

	mov	ebx,	[esp + 44]		; n
	mov	eax,	(1<<16)
	xor	edx,	edx
	idiv	ebx			; eax = rcp = (1<<16) / n
	movd	mm6,	eax
	punpcklwd	mm6,	mm6		; mm6 = 0000|0000|rcp|rcp
	punpcklwd	mm6,	mm6		; mm6 = rcp|rcp|rcp|rcp

	mov	edi,	[esp + 28]		; dest

	mov	ebp,	[esp + 32]		; sum
	movq	mm5,	[ebp]		; mm5 = As|Rs|Gs|Bs

	mov	ebx,	[esp + 36]		; addptr
	mov	ebp,	[esp + 40]		; subptr

	lea	eax,	[edi + ecx*4]	; limit
	mov	esi,	eax		; esi = limit

	sub	esi,	byte 12	; 3*4
	cmp	edi,	esi

	jae	near .pfraction		; jump if edi >= limit

	loop_align
.ploop:
	movq	mm1,	mm5		; 1 mm1 = As|Rs|Gs|Bs
	add	ebp,	byte 32
	add	edi,	byte 16
	paddw	mm1,	mm7		; 1 mm1 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN
	paddw	mm5,	[ebx]		; 1 sum += *addptr
	pmulhuw	mm1,	mm6		; 1 mm1 = Aavg|Ravg|Gavg|Bavg   (pmulhuw = SSE inst.)
	psubw	mm5,	[ebp-32]		; 1 sum -= *subptr
	packuswb	mm1,	mm1		; 1 pack mm1
	movq	mm2,	mm5		; 2 mm2 = As|Rs|Gs|Bs
	movd	[edi-16],	mm1
	paddw	mm2,	mm7		; 2 mm2 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN
	paddw	mm5,	[ebx+8]		; 2 sum += *addptr
	pmulhuw	mm2,	mm6		; 2 mm2 = Aavg|Ravg|Gavg|Bavg   (pmulhuw = SSE inst.)
	psubw	mm5,	[ebp+8-32]		; 2 sum -= *subptr
	packuswb	mm2,	mm2		; 2 pack mm2
	movq	mm3,	mm5		; 3 mm3 = As|Rs|Gs|Bs
	movd	[edi+4-16],	mm2
	paddw	mm3,	mm7		; 3 mm3 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN
	paddw	mm5,	[ebx+16]		; 3 sum += *addptr
	pmulhuw	mm3,	mm6		; 3 mm3 = Aavg|Ravg|Gavg|Bavg   (pmulhuw = SSE inst.)
	psubw	mm5,	[ebp+16-32]		; 3 sum -= *subptr
	packuswb	mm3,	mm3		; 3 pack mm3
	add	ebx,	byte 32
	movq	mm4,	mm5		; 4 mm4 = As|Rs|Gs|Bs
	movd	[edi+8-16],	mm3
	paddw	mm4,	mm7		; 4 mm4 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN
	paddw	mm5,	[ebx+24-32]		; 4 sum += *addptr
	pmulhuw	mm4,	mm6		; 4 mm4 = Aavg|Ravg|Gavg|Bavg   (pmulhuw = SSE inst.)
	psubw	mm5,	[ebp+24-32]		; 4 sum -= *subptr
	packuswb	mm4,	mm4		; 4 pack mm4
	cmp	edi,	esi
	movd	[edi+12-16],	mm4
	jb	near .ploop

.pfraction:
	add	esi,	byte 12
	cmp	edi,	esi
	jae	.pexit			; jump if edi >= limit

.ploop2:	; fractions
	movq	mm1,	mm5		; mm1 = As|Rs|Gs|Bs
	paddw	mm1,	mm7		; mm1 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN
	pmulhuw	mm1,	mm6		; mm1 = Aavg|Ravg|Gavg|Bavg   (pmulhuw = SSE inst.)
	packuswb	mm1,	mm1		; pack mm1
	movd	[edi],	mm1

	paddw	mm5,	[ebx]		; sum += *addptr
	psubw	mm5,	[ebp]		; sum -= *subptr

	add	ebx,	byte 8
	add	ebp,	byte 8
	add	edi,	byte 4

	cmp	edi,	esi
	jb	short .ploop2		; jump if edi < limit

.pexit:
	mov	ebp,	[esp + 32]		; sum
	movq	[ebp],	mm5		; store sum

.pexit2:
	pop	ebp
	pop	edx
	pop	ecx
	pop	ebx
	pop	esi
	pop	edi
	emms
	ret
;--------------------------------------------------------------------

;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_SSE && TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPDoBoxBlurAvg16_d
;;void, TVPDoBoxBlurAvg16_d_sse_a, (tjs_uint32 *dest, tjs_uint16 *sum, const tjs_uint16 * add, const tjs_uint16 * sub, tjs_int n, tjs_int len)
;--------------------------------------------------------------------

	function_align
TVPDoBoxBlurAvg16_d_sse_a:			; do blur using box-blur algorithm with alpha, 16bit precision
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 48]		; len
	cmp	ecx,	byte 0
	jle	near .pexit2

	pxor	mm0,	mm0		; mm0 = 0

	mov	eax,	[esp + 44]		; n
	shr	eax,	1		; eax = n / 2
	movd	mm7,	eax
	punpcklwd	mm7,	mm7		; mm7 = 0000|0000|half_n|half_n
	punpcklwd	mm7,	mm7		; mm7 = half_n|half_n|half_n|half_n

	mov	ebx,	[esp + 44]		; n
	mov	eax,	(1<<16)
	xor	edx,	edx
	idiv	ebx			; eax = rcp = (1<<16) / n
	movd	mm6,	eax
	punpcklwd	mm6,	mm6		; mm6 = 0000|0000|rcp|rcp
	punpcklwd	mm6,	mm6		; mm6 = rcp|rcp|rcp|rcp

	mov	edi,	[esp + 28]		; dest

	mov	ebp,	[esp + 32]		; sum
	movq	mm5,	[ebp]		; mm5 = As|Rs|Gs|Bs

	mov	ebx,	[esp + 36]		; addptr
	mov	ebp,	[esp + 40]		; subptr

	lea	eax,	[edi + ecx*4]	; limit
	mov	esi,	eax		; esi = limit

	sub	esi,	byte 12	; 3*4
	cmp	edi,	esi

	jae	near .pfraction		; jump if edi >= limit

	loop_align
.ploop:
	movq	mm1,	mm5		; mm1 = As|Rs|Gs|Bs
	paddw	mm1,	mm7		; mm1 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN
	pmulhuw	mm1,	mm6		; mm1 = Aavg|Ravg|Gavg|Bavg   (pmulhuw = SSE inst.)
	packuswb	mm1,	mm0
	movd	ecx,	mm1
	shr	ecx,	16
	and	ecx,	0xff00
	paddw	mm5,	[ebx]		; sum += *addptr
	add	ecx,	TVPDivTable		; ecx = TVPDivTable[Alpha][]
	movd	edx,	mm1
	and	edx,	0xff000000
	movd	eax,	mm1
	psubw	mm5,	[ebp]		; sum -= *subptr
	and	eax,	0xff		; eax = 00|00|00|B
	movzx	eax,byte	[ecx+eax]		; look up the table
	add	edx,	eax		; edx = alpha|00|00|[B]
	movd	eax,	mm1
	movq	mm4,	mm5		; mm4 = As|Rs|Gs|Bs
	shr	eax,	8
	and	eax,	0xff		; eax = Gavg
	paddw	mm4,	mm7		; mm4 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN
	movzx	eax,byte	[ecx+eax]		; look up the table
	shl	eax,	8		; eax = 00|00|[G]|00
	pmulhuw	mm4,	mm6		; mm4 = Aavg|Ravg|Gavg|Bavg   (pmulhuw = SSE inst.)
	add	edx,	eax		; edx = alpha|00|[G]|[B]
	movd	eax,	mm1
	packuswb	mm4,	mm0
	shr	eax,	16
	and	eax,	0xff		; eax = Ravg
	movzx	eax,byte	[ecx+eax]		; look up the table
	shl	eax,	16		; eax = 00|[R]|00|00
	movd	ecx,	mm4
	add	edx,	eax		; edx = alpha|[R]|[G]|[B]
	shr	ecx,	16
	mov	[edi],	edx
	and	ecx,	0xff00
	paddw	mm5,	[ebx+8]		; sum += *addptr
	add	ecx,	TVPDivTable		; ecx = TVPDivTable[Alpha][]
	movd	edx,	mm4
	and	edx,	0xff000000
	movd	eax,	mm4
	psubw	mm5,	[ebp+8]		; sum -= *subptr
	and	eax,	0xff		; eax = 00|00|00|B
	movzx	eax,byte	[ecx+eax]		; look up the table
	add	edx,	eax		; edx = alpha|00|00|[B]
	movd	eax,	mm4
	add	ebx,	byte 16
	shr	eax,	8
	and	eax,	0xff		; eax = Gavg
	movzx	eax,byte	[ecx+eax]		; look up the table
	add	ebp,	byte 16
	shl	eax,	8		; eax = 00|00|[G]|00
	add	edx,	eax		; edx = alpha|00|[G]|[B]
	movd	eax,	mm4
	add	edi,	byte 8
	shr	eax,	16
	and	eax,	0xff		; eax = Ravg
	movzx	eax,byte	[ecx+eax]		; look up the table
	shl	eax,	16		; eax = 00|[R]|00|00
	add	edx,	eax		; edx = alpha|[R]|[G]|[B]
	cmp	edi,	esi
	mov	[edi+4-8],	edx
	jb	near .ploop

.pfraction:
	add	esi,	byte 12
	cmp	edi,	esi
	jae	.pexit			; jump if edi >= limit

.ploop2:	; fractions
	movq	mm1,	mm5		; mm1 = As|Rs|Gs|Bs
	paddw	mm1,	mm7		; mm1 = As+HalfN|Rs+HalfN|Gs+HalfN|Bs+HalfN
	pmulhuw	mm1,	mm6		; mm1 = Aavg|Ravg|Gavg|Bavg   (pmulhuw = SSE inst.)
	packuswb	mm1,	mm0

	movd	ecx,	mm1
	shr	ecx,	16
	and	ecx,	0xff00
	add	ecx,	TVPDivTable		; ecx = TVPDivTable[Alpha][]
	movd	edx,	mm1
	and	edx,	0xff000000

	movd	eax,	mm1
	and	eax,	0xff		; eax = 00|00|00|B
	movzx	eax,byte	[ecx+eax]		; look up the table
	add	edx,	eax		; edx = alpha|00|00|[B]

	movd	eax,	mm1
	shr	eax,	8
	and	eax,	0xff		; eax = Gavg
	movzx	eax,byte	[ecx+eax]		; look up the table
	shl	eax,	8		; eax = 00|00|[G]|00
	add	edx,	eax		; edx = alpha|00|[G]|[B]

	movd	eax,	mm1
	shr	eax,	16
	and	eax,	0xff		; eax = Ravg
	movzx	eax,byte	[ecx+eax]		; look up the table
	shl	eax,	16		; eax = 00|[R]|00|00
	add	edx,	eax		; edx = alpha|[R]|[G]|[B]

	mov	[edi],	edx

	paddw	mm5,	[ebx]		; sum += *addptr
	psubw	mm5,	[ebp]		; sum -= *subptr

	add	ebx,	byte 8
	add	ebp,	byte 8
	add	edi,	byte 4

	cmp	edi,	esi
	jb	short .ploop2		; jump if edi < limit

.pexit:
	mov	ebp,	[esp + 32]		; sum
	movq	[ebp],	mm5		; store sum

.pexit2:
	pop	ebp
	pop	edx
	pop	ecx
	pop	ebx
	pop	esi
	pop	edi
	emms
	ret
;--------------------------------------------------------------------


%endif

