; -*- Mode: asm; tab-width: 12; -*-
; this is a part of TVP (KIRIKIRI) software source.
; see other sources for license.
; (C)2001-2009 W.Dee <dee@kikyou.info> and contributors

; pixel alpha blender

%ifndef GEN_CODE

%include		"nasm.nah"

globaldef		TVPAlphaBlend_mmx_a
globaldef		TVPAlphaBlend_emmx_a
globaldef		TVPAlphaBlend_o_mmx_a
globaldef		TVPAlphaBlend_o_emmx_a
globaldef		TVPAlphaBlend_HDA_mmx_a
globaldef		TVPAlphaBlend_HDA_emmx_a
globaldef		TVPAlphaBlend_d_mmx_a
globaldef		TVPAlphaBlend_d_emmx_a
globaldef		TVPConstAlphaBlend_mmx_a
globaldef		TVPConstAlphaBlend_emmx_a
globaldef		TVPConstAlphaBlend_SD_mmx_a
globaldef		TVPConstAlphaBlend_SD_emmx_a
externdef		TVPOpacityOnOpacityTable
externdef		TVPNegativeMulTable


%define GEN_CODE

;--------------------------------------------------------------------
; MMX stuff
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPAlphaBlend
;;void, TVPAlphaBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
%define TVPAlphaBlend_name TVPAlphaBlend_mmx_a
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPAlphaBlend_o
;;void, TVPAlphaBlend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
%define TVPAlphaBlend_o_name TVPAlphaBlend_o_mmx_a
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPAlphaBlend_HDA
;;void, TVPAlphaBlend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
%define TVPAlphaBlend_HDA_name TVPAlphaBlend_HDA_mmx_a
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPAlphaBlend_d
;;void, TVPAlphaBlend_d_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
%define TVPAlphaBlend_d_name TVPAlphaBlend_d_mmx_a
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPConstAlphaBlend
;;void, TVPConstAlphaBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
%define TVPConstAlphaBlend_name TVPConstAlphaBlend_mmx_a
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPConstAlphaBlend_SD
;;void, TVPConstAlphaBlend_SD_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src1, const tjs_uint32 *src2, tjs_int len, tjs_int opa)
%define TVPConstAlphaBlend_SD_name TVPConstAlphaBlend_SD_mmx_a
;--------------------------------------------------------------------
	%include "alphablend.nas"
;--------------------------------------------------------------------


;--------------------------------------------------------------------
; EMMX stuff
;--------------------------------------------------------------------
%define USE_EMMX
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPAlphaBlend
;;void, TVPAlphaBlend_emmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
%define TVPAlphaBlend_name TVPAlphaBlend_emmx_a
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPAlphaBlend_o
;;void, TVPAlphaBlend_o_emmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
%define TVPAlphaBlend_o_name TVPAlphaBlend_o_emmx_a
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPAlphaBlend_HDA
;;void, TVPAlphaBlend_HDA_emmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
%define TVPAlphaBlend_HDA_name TVPAlphaBlend_HDA_emmx_a
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPAlphaBlend_d
;;void, TVPAlphaBlend_d_emmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
%define TVPAlphaBlend_d_name TVPAlphaBlend_d_emmx_a
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPConstAlphaBlend
;;void, TVPConstAlphaBlend_emmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
%define TVPConstAlphaBlend_name TVPConstAlphaBlend_emmx_a
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPConstAlphaBlend_SD
;;void, TVPConstAlphaBlend_SD_emmx_a, (tjs_uint32 *dest, const tjs_uint32 *src1, const tjs_uint32 *src2, tjs_int len, tjs_int opa)
%define TVPConstAlphaBlend_SD_name TVPConstAlphaBlend_SD_emmx_a
;--------------------------------------------------------------------
	%include "alphablend.nas"
;--------------------------------------------------------------------


%else

;--------------------------------------------------------------------
	segment_code
;--------------------------------------------------------------------
	function_align
TVPAlphaBlend_name:
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 36]		; len
	cmp	ecx,	byte 0
	jle	near .pexit
	mov	edi,	[esp + 28]		; dest
	mov	ebp,	[esp + 32]		; src
	pxor	mm0,	mm0		; mm0 = 0
	lea	esi,	[edi + ecx*4]		; limit
	sub	esi,	byte 16		; 4*4
	cmp	edi,	esi
	jae	near .pfraction		; jump if edi >= esi



	test	edi,	4
	IF	nz
			; align destination pointer to QWORD

		mov	eax,	[ebp]		; src
		cmp	eax,	0ff000000h
		IF	ae
			; totally opaque
			mov	[edi],	eax	; intact copy
		ELSE
			; not totally opaque
			movd	mm2,	eax
			shr	eax,	24		; eax >>= 24
			movd	mm1,	[edi]		; dest
			movd	mm4,	eax
			punpcklbw	mm2,	mm0		; unpack
			punpcklwd	mm4,	mm4		; unpack
			punpcklbw	mm1,	mm0		; unpack
			punpcklwd	mm4,	mm4		; unpack
			psubw	mm2,	mm1		; mm2 -= mm1
			pmullw	mm2,	mm4		; mm2 *= mm4
			psrlw	mm2,	8		; mm2 >>= 8
			paddb	mm1,	mm2		; mm1 += mm2
			packuswb	mm1,	mm0		; pack
			movd	[edi],	mm1		; store
		ENDIF
		add	edi,	byte 4
		add	ebp,	byte 4
		cmp	edi,	esi
		jae	near .pfraction		; jump if edi >= esi

	ENDIF


	mov	eax,	[ebp]		; (for next loop) 1 src
	mov	edx,	[ebp+4]		; (for next loop) 2 src
	mov	ecx,	eax
	mov	ebx,	eax

	jmp	.ploop

.popaque:
	movq	mm1,	[ebp]		; load
	movq	[edi],	mm1		; store
.ptransp:
	add	ebp,	byte 8
	add	edi,	byte 8
	mov	eax,	[ebp]		; (for next loop) 1 src
	mov	edx,	[ebp+4]		; (for next loop) 2 src
	mov	ecx,	eax
	mov	ebx,	eax
	cmp	edi,	esi
	jb	near .ploop
	jmp	.pfraction

	loop_align
.ploop:
;	 full transparent/opaque checking will make the routine faster in usual usage.
	and	ecx,	edx
	or	ebx,	edx
	cmp	ecx,	0ff000000h
	jae	.popaque		; totally opaque
	cmp	ebx,	000ffffffh
	jbe	.ptransp		; totally transparent

	movd	mm2,	eax		; 1
	movd	mm6,	edx		; 2
	punpcklbw	mm2,	mm0		; 1 unpack
	movd	mm1,	[edi]		; 1 dest
	punpcklbw	mm6,	mm0		; 2 unpack
	shr	edx,	24		; 2 eax >>= 24
	punpcklbw	mm1,	mm0		; 1 unpack
	shr	eax,	24		; 1 eax >>= 24
	movd	mm4,	edx		; 2
	movd	mm3,	eax		; 1
	punpcklwd	mm4,	mm4		; 2 unpack
	movd	mm5,	[edi+4]		; 2 dest
	punpcklwd	mm3,	mm3		; 1 unpack
	punpcklbw	mm5,	mm0		; 2 unpack
	psubw	mm2,	mm1		; 1 mm2 -= mm1
	punpcklwd	mm3,	mm3		; 1 unpack
	psubw	mm6,	mm5		; 2 mm6 -= mm5
	punpcklwd	mm4,	mm4		; 2 unpack
	pmullw	mm2,	mm3		; 1 mm2 *= mm3
	mov	eax,	[ebp+8]		; (for next loop) 1 src
	pmullw	mm6,	mm4		; 2 mm6 *= mm4
	psrlw	mm2,	8		; 1 mm2 >>= 8
	psrlw	mm6,	8		; 2 mm6 >>= 8
	mov	edx,	[ebp+12]		; (for next loop) 2 src
%ifdef USE_EMMX
	prefetcht0	[ebp + 16]
%endif
	paddb	mm1,	mm2		; 1 mm1 += mm2
	paddb	mm5,	mm6		; 2 mm5 += mm2
	add	edi,	byte 8
	packuswb	mm1,	mm5		; pack
	add	ebp,	byte 8
	movq	[edi-8], mm1		; 1 store
	mov	ecx,	eax		; (for next loop)
	cmp	edi,	esi
	mov	ebx,	eax		; (for next loop)

	jb	near .ploop

.pfraction:
	add	esi,	byte 16
	cmp	edi,	esi
	jae	.pexit		; jump if edi >= esi

.ploop2:	; fractions
	mov	eax,	[ebp]		; src
	cmp	eax,	0ff000000h
	IF	ae
		; totally opaque
		mov	[edi],	eax	; intact copy
	ELSE
		; not totally opaque
		movd	mm2,	eax
		shr	eax,	24		; eax >>= 24
		movd	mm1,	[edi]		; dest
		movd	mm4,	eax
		punpcklbw	mm2,	mm0		; unpack
		punpcklwd	mm4,	mm4		; unpack
		punpcklbw	mm1,	mm0		; unpack
		punpcklwd	mm4,	mm4		; unpack
		psubw	mm2,	mm1		; mm2 -= mm1
		pmullw	mm2,	mm4		; mm2 *= mm4
		psrlw	mm2,	8		; mm2 >>= 8
		paddb	mm1,	mm2		; mm1 += mm2
		packuswb	mm1,	mm0		; pack
		movd	[edi],	mm1		; store
	ENDIF
	add	edi,	byte 4
	add	ebp,	byte 4
	cmp	edi,	esi

	jb	.ploop2		; jump if edi < esi

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
TVPAlphaBlend_o_name:
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 36]		; len
	cmp	ecx,	byte 0
	jle	near .pexit
	mov	edi,	[esp + 28]		; dest
	mov	ebp,	[esp + 32]		; src
	movd	mm6,	[esp + 40]		; opa
	punpckldq	mm6,	mm6		; mm6 |= (mm6 << 32)
	pxor	mm0,	mm0		; mm0 = 0
	lea	esi,	[edi + ecx*4]		; limit
	sub	esi,	byte 8		; 2*4
	cmp	edi,	esi
	jae	near .pfraction		; jump if edi >= esi

	test	edi,	4
	IF	nz
			; align destination pointer to QWORD
		mov	ebx,	[esp + 40]		; opa

		mov	eax,	[ebp]		; src
		movd	mm2,	eax
		mul	ebx		; edx:eax = eax * ebx
		movd	mm1,	[edi]		; dest
		movd	mm4,	edx
		punpcklbw	mm2,	mm0		; unpack
		punpcklwd	mm4,	mm4		; unpack
		punpcklbw	mm1,	mm0		; unpack
		punpcklwd	mm4,	mm4		; unpack
		psubw	mm2,	mm1		; mm2 -= mm1
		pmullw	mm2,	mm4		; mm2 *= mm4
		psllw	mm1,	8
		paddw	mm1,	mm2		; mm1 += mm2
		psrlw	mm1,	8
		packuswb	mm1,	mm0		; pack
		movd	[edi],	mm1		; store
		add	edi,	byte 4
		add	ebp,	byte 4
		cmp	edi,	esi

		jae	near .pfraction
	ENDIF

	jmp	short	.ploop

.ptransp:
	add	edi,	byte 8
	add	ebp,	byte 8
	cmp	edi,	esi
	jb	near	.ploop
	jmp	near	.pfraction

	loop_align
.ploop:
	mov	ebx,	[ebp]		; 1 src1
	or	ebx,	[ebp + 4]		; 2 src2
	test	ebx,	0ff000000h		; test fully transparent
	jz	.ptransp		; jump to .ptransp if fully transparent

	movd	mm7,	[edi]		; 1 dest
	movq	mm2,	[ebp]		; src
	punpcklbw	mm7,	mm0		; 1 unpack
	movq	mm3,	[ebp]		; src
%ifdef	USE_EMMX
	prefetcht0	[ebp+16]
%endif
	psrld	mm2,	24
	movd	mm1,	[edi+4]		; 2 dest
	pmullw	mm2,	mm6		; multiply by opacity
	movq	mm5,	mm3		; 1
	psrld	mm2,	8
	punpcklbw	mm1,	mm0		; 2 unpack
	movq	mm4,	mm2		; 1
	punpckhdq	mm3,	mm0		; 2 mm3 >>= 32
	punpckhdq	mm2,	mm0		; 2 mm2 >>= 32
	punpcklbw	mm5,	mm0		; 1 unpack
	punpcklbw	mm3,	mm0		; 2 unpack
	psubw	mm5,	mm7		; 1 mm5 -= mm7
	punpcklwd	mm4,	mm4		; 1 unpack
	punpcklwd	mm2,	mm2		; 2 unpack
	psubw	mm3,	mm1		; 2 mm3 -= mm1
	punpcklwd	mm4,	mm4		; 1 unpack
	punpcklwd	mm2,	mm2		; 2 unpack
	pmullw	mm5,	mm4		; 1 mm5 *= mm4
	add	edi,	byte 8
	pmullw	mm3,	mm2		; 2 mm3 *= mm2
	psrlw	mm5,	8
	psrlw	mm3,	8
	add	ebp,	byte 8
	paddb	mm7,	mm5		; 1 mm7 += mm5
	paddb	mm1,	mm3		; 2 mm1 += mm3
	cmp	edi,	esi
	packuswb	mm7,	mm1		; pack
	movq	[edi-8],mm7		; store

	jb	near .ploop

.pfraction:
	add	esi,	byte 8
	cmp	edi,	esi
	jae	.pexit		; jump if edi >= esi

	mov	ebx,	[esp + 40]		; opa

.ploop2:	; fractions
	mov	eax,	[ebp]		; src
	movd	mm2,	eax
	mul	ebx		; edx:eax = eax * ebx
	movd	mm1,	[edi]		; dest
	movd	mm4,	edx
	punpcklbw	mm2,	mm0		; unpack
	punpcklwd	mm4,	mm4		; unpack
	punpcklbw	mm1,	mm0		; unpack
	punpcklwd	mm4,	mm4		; unpack
	psubw	mm2,	mm1		; mm2 -= mm1
	pmullw	mm2,	mm4		; mm2 *= mm4
	psllw	mm1,	8
	paddw	mm1,	mm2		; mm1 += mm2
	psrlw	mm1,	8
	packuswb	mm1,	mm0		; pack
	movd	[edi],	mm1		; store
	add	edi,	byte 4
	add	ebp,	byte 4
	cmp	edi,	esi

	jb	.ploop2		; jump if edi < esi

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
TVPAlphaBlend_HDA_name:
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	mov	ecx,	[esp + 32]		; len
	dec	ecx
	js	.pexit
	shl	ecx,	2
	mov	edi,	[esp + 24]		; dest
	mov	esi,	[esp + 28]		; src
	pxor	mm0,	mm0		; mm0 = 0

	loop_align
.ploop:
	mov	eax,	[esi + ecx]		; src
%ifdef	USE_EMMX
	prefetcht0	[esi + ecx - 8]
%endif
	mov	edx,	eax
	shr	eax,	24		; eax >>= 24
	movd	mm4,	eax
	movd	mm2,	edx
	punpcklwd	mm4,	mm4		; unpack
	mov	eax,	[edi + ecx]		; dest
	movd	mm1,	eax
	punpcklbw	mm2,	mm0		; unpack
	punpcklbw	mm1,	mm0		; unpack
	punpcklwd	mm4,	mm4		; unpack
	psubw	mm2,	mm1		; mm2 -= mm1
	and	eax,	0ff000000h		; mask alpha
	pmullw	mm2,	mm4		; mm2 *= mm4
	psllw	mm1,	8
	paddw	mm1,	mm2		; mm1 += mm2
	psrlw	mm1,	8
	packuswb	mm1,	mm0		; pack
	movd	edx,	mm1
	and	edx,	0ffffffh		; mask color
	or	edx,	eax		; store destination
	mov	[edi + ecx], edx		; store

	sub	ecx,	byte 4
	jns	.ploop

.pexit:
	pop	edx
	pop	ecx
	pop	ebx
	pop	esi
	pop	edi
	emms
	ret


;--------------------------------------------------------------------
	function_align
TVPAlphaBlend_d_name:
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 36]		; len
	cmp	ecx,	byte 0
	jle	near .pexit
	mov	edi,	[esp + 28]		; dest
	mov	ebp,	[esp + 32]		; src
	mov	eax,	0ffffffh
	movd	mm7,	eax		; mm7 = 0ffffffh
	pxor	mm0,	mm0		; mm0 = 0
	lea	esi,	[edi + ecx*4]		; limit
	sub	esi,	byte 4		; 1*4
	cmp	edi,	esi
	jae	near .pfraction		; jump if edi >= esi
	jmp	.ploop

.popaque:
	movq	mm1,	[ebp]		; load
	movq	[edi],	mm1		; store
.ptransp:
	add	ebp,	byte 8
	add	edi,	byte 8
	cmp	edi,	esi
	jb	near .ploop
	jmp	.pfraction

	loop_align
.ploop:
;	 full transparent/opaque checking will make the routine faster in usual usage.
	mov	eax,	[ebp]		; 1 src
	mov	ecx,	[ebp+4]		; 2 src
%ifdef	USE_EMMX
	prefetcht0	[ebp + 16]
%endif
	mov	ebx,	eax
	mov	edx,	eax
	and	ebx,	ecx
	or	edx,	ecx
	cmp	ebx,	0ff000000h
	jae	.popaque		; totally opaque
	cmp	edx,	000ffffffh
	jbe	.ptransp		; totally transparent

	mov	ebx,	[edi]		; 1 dest
	movd	mm2,	eax		; 1 src
	mov	edx,	[edi+4]		; 2 dest
%ifdef	USE_EMMX
	prefetcht0	[edi + 16]
%endif
	movd	mm5,	ecx		; 2 src
	pand	mm2,	mm7		; 1 mask
	pand	mm5,	mm7		; 2 mask
	shr	eax,	16		; 1 eax >>= 16
	movd	mm1,	ebx		; 1 dest
	shr	ecx,	16		; 2 ecx >>= 16
	movd	mm4,	edx		; 2 dest
	shr	ebx,	24		; 1 ebx >>= 24
	and	eax,	0ff00h		; 1 eax &= 0xff00
	shr	edx,	24		; 2 edx >>= 24
	and	ecx,	0ff00h		; 2 ecx &= 0xff00
	add	eax,	ebx		; 1 eax += ebx
	add	ecx,	edx		; 2 ecx += edx
	pand	mm1,	mm7		; 1 mask
	pand	mm4,	mm7		; 2 mask
	movzx	ebx,	byte [TVPOpacityOnOpacityTable + eax]		; 1 blend opa table
	punpcklbw	mm1,	mm0		; 1 unpack
	movzx	edx,	byte [TVPOpacityOnOpacityTable + ecx]		; 2 blend opa table
	punpcklbw	mm4,	mm0		; 2 unpack
	punpcklbw	mm2,	mm0		; 1 unpack
	punpcklbw	mm5,	mm0		; 2 unpack
	movd	mm3,	ebx		; 1 mm3 = eax
	movd	mm6,	edx		; 2 mm6 = ecx
	punpcklwd	mm3,	mm3		; 1 unpack
	psubw	mm2,	mm1		; 1 mm2 -= mm1
	punpcklwd	mm6,	mm6		; 2 unpack
	psubw	mm5,	mm4		; 2 mm5 -= mm4
	punpcklwd	mm3,	mm3		; 1 unpack
	xor	eax,	0ffffh		; 1 eax ^= 0ffffh (a = 255-a, b = 255-b)
	xor	ecx,	0ffffh		; 2 ecx ^= 0ffffh (a = 255-a, b = 255-b)
	mov	edx,	ecx		; 2 edx = ecx
	mov	ebx,	eax		; 1 ebx = eax
	and	eax,	0ffh		; 1 mask eax
	and	ecx,	0ffh		; 2 mask ecx
	shr	ebx,	8		; 1 ebx >>= 8
	punpcklwd	mm6,	mm6		; 2 unpack
	imul	eax,	ebx		; 1 eax *= ebx
	pmullw	mm2,	mm3		; 1 mm2 *= mm3
	add	edi,	byte 8
	pmullw	mm5,	mm6		; 2 mm5 *= mm6
	shr	edx,	8		; 2 edx >>= 8
	not	eax		; 1 result = 255-result
	psllw	mm1,	8
	add	ebp,	byte 8
	psllw	mm4,	8
	imul	ecx,	edx		; 2 ecx *= edx
	and	eax,	0ff00h		; 1 mask eax
	paddw	mm1,	mm2		; 1 mm1 += mm2
	not	ecx		; 2 result = 255-result
	paddw	mm4,	mm5		; 2 mm4 += mm5
	shl	eax,	16		; 1 shift eax
	psrlw	mm1,	8
	psrlw	mm4,	8
	and	ecx,	0ff00h		; 2 mask ecx
	packuswb	mm1,	mm0		; 1 pack
	shl	ecx,	16		; 2 shift ecx
	packuswb	mm4,	mm0		; 2 pack
	movd	ebx,	mm1		; 1 store
	movd	edx,	mm4		; 2 store
	or	ebx,	eax		; 1 destination opa
	or	edx,	ecx		; 2 destination opa
	mov	[edi-8], ebx		; 1 store
	cmp	edi,	esi
	mov	[edi+4-8], edx		; 2 store

	jb	near .ploop

.pfraction:
	add	esi,	byte 4
	cmp	edi,	esi
	jae	.pexit		; jump if edi >= esi

.ploop2:	; fractions
	mov	eax,	[ebp]		; src
	mov	ebx,	[edi]		; dest
	movd	mm2,	eax		; src
	movd	mm1,	ebx		; dest
	pand	mm2,	mm7		; mask
	pand	mm1,	mm7		; mask
	shr	eax,	16
	shr	ebx,	24
	and	eax,	0ff00h
	add	eax,	ebx
	punpcklbw	mm1,	mm0		; unpack
	movzx	ebx,	byte [TVPNegativeMulTable + eax]		; dest opa table
	movzx	eax,	byte [TVPOpacityOnOpacityTable + eax]		; blend opa table
	punpcklbw	mm2,	mm0		; unpack
	shl	ebx,	24
	movd	mm4,	eax
	punpcklwd	mm4,	mm4		; unpack
	punpcklwd	mm4,	mm4		; unpack
	psubw	mm2,	mm1		; mm2 -= mm1
	pmullw	mm2,	mm4		; mm2 *= mm4
	psllw	mm1,	8
	paddw	mm1,	mm2		; mm1 += mm2
	psrlw	mm1,	8
	packuswb	mm1,	mm0		; pack
	movd	eax,	mm1		; store
	or	eax,	ebx
	mov	[edi],	eax
	add	edi,	byte 4
	add	ebp,	byte 4

	cmp	edi,	esi
	jb	near .ploop2

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
TVPConstAlphaBlend_name:
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 36]		; len
	cmp	ecx,	byte 0
	jle	near .pexit
	mov	edi,	[esp + 28]		; dest
	mov	esi,	[esp + 32]		; src
	movd	mm4,	[esp + 40]		; opa
	xor	ebx,	ebx		; counter
	pxor	mm0,	mm0		; mm0 = 0
	lea	ebp,	[ecx*4]		; limit
	punpcklwd	mm4,	mm4		; unpack
	sub	ebp,	byte 8		; 2*4
	punpcklwd	mm4,	mm4		; unpack
	movq	mm3,	mm4
	cmp	ebx,	ebp
	jge	near .pfraction		; jump if ebx >= ebp

	test	edi,	4
	IF	nz
			; align destination pointer to QWORD
		movd	mm2,	[esi+ebx]		; src
		movd	mm1,	[edi+ebx]		; dest
		punpcklbw	mm2,	mm0		; unpack
		punpcklbw	mm1,	mm0		; unpack
		psubw	mm2,	mm1		; mm2 -= mm1
		add	ebx,	byte 4
		pmullw	mm2,	mm4		; mm2 *= mm4
		psllw	mm1,	8
		paddw	mm1,	mm2		; mm1 += mm2
		cmp	ebx,	ebp
		psrlw	mm1,	8
		packuswb	mm1,	mm0		; pack
		movd	[edi-4+ebx], mm1		; store
		jge	.pfraction
	ENDIF


	loop_align
.ploop:
	movq	mm2,	[esi+ebx]		; src
%ifdef	USE_EMMX
	prefetcht0	[esi + ebx + 16]
%endif
	movq	mm1,	[edi+ebx]		; dest
	movq	mm6,	mm2
	movq	mm5,	mm1
	punpcklbw	mm2,	mm0		; 1 unpack
	punpckhbw	mm6,	mm0		; 2 unpack
	punpcklbw	mm1,	mm0		; 1 unpack
	punpckhbw	mm5,	mm0		; 2 unpack
	psubw	mm2,	mm1		; 1 mm2 -= mm1
	psubw	mm6,	mm5		; 2 mm6 -= mm5
	pmullw	mm2,	mm3		; 1 mm2 *= mm3
	add	ebx,	byte 8
	psllw	mm1,	8
	pmullw	mm6,	mm4		; 2 mm6 *= mm4
	psllw	mm5,	8
	cmp	ebx,	ebp
	paddw	mm1,	mm2		; 1 mm1 += mm2
	paddw	mm5,	mm6		; 2 mm5 += mm2
	psrlw	mm1,	8
	psrlw	mm5,	8
	packuswb	mm1,	mm5		; pack
	movq	[edi-8+ebx], mm1		; 1 store

	jl	near .ploop

.pfraction:
	add	ebp,	byte 8
	cmp	ebx,	ebp
	jge	.pexit		; jump if ebx >= ebp

.ploop2:	; fractions
	movd	mm2,	[esi+ebx]		; src
	movd	mm1,	[edi+ebx]		; dest
	punpcklbw	mm2,	mm0		; unpack
	punpcklbw	mm1,	mm0		; unpack
	psubw	mm2,	mm1		; mm2 -= mm1
	add	ebx,	byte 4
	pmullw	mm2,	mm4		; mm2 *= mm4
	psllw	mm1,	8
	paddw	mm1,	mm2		; mm1 += mm2
	cmp	ebx,	ebp
	psrlw	mm1,	8
	packuswb	mm1,	mm0		; pack
	movd	[edi-4+ebx], mm1		; store

	jl	.ploop2		; jump if ebx < ebp

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
TVPConstAlphaBlend_SD_name:
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 40]		; len
	cmp	ecx,	byte 0
	jle	near .pexit
	mov	edi,	[esp + 28]		; dest
	mov	esi,	[esp + 36]		; src2
	mov	eax,	[esp + 32]		; src1
	movd	mm4,	[esp + 44]		; opa
	pxor	mm0,	mm0		; mm0 = 0
	xor	ebx,	ebx		; counter
	lea	ebp,	[ecx*4]		; limit
	punpcklwd	mm4,	mm4		; unpack
	sub	ebp,	byte 16		; 4*4
	punpcklwd	mm4,	mm4		; unpack
	cmp	ebx,	ebp
	jge	near .pfraction		; jump if ebx >= ebp

	test	edi,	4
	IF	nz
			; align destination pointer to QWORD
		movd	mm2,	[esi+ebx]		; src1
		movd	mm1,	[eax+ebx]		; src2
		punpcklbw	mm2,	mm0		; unpack
		punpcklbw	mm1,	mm0		; unpack
		psubw	mm2,	mm1		; mm2 -= mm1
		pmullw	mm2,	mm4		; mm2 *= mm4
		psllw	mm1,	8
		paddw	mm1,	mm2		; mm1 += mm2
		psrlw	mm1,	8
		add	ebx,	byte 4
		packuswb	mm1,	mm0		; pack
		cmp	ebx,	ebp
		movd	[edi-4+ebx], mm1		; store

		jge	near .pfraction		; jump if ebx >= ebp
	ENDIF

	loop_align
.ploop:
	movq	mm2,	[esi+ebx]		; 0 src2
	movq	mm3,	[esi+ebx+8]		; 1 src2
	movq	mm1,	[eax+ebx]		; 0 src1
	movq	mm7,	[eax+ebx+8]		; 1 src1
%ifdef	USE_EMMX
	prefetcht0	[esi+ebx+32]
%endif
%ifdef	USE_EMMX
	prefetcht0	[eax+ebx+32]
%endif
	movq	mm6,	mm2		; 0
	movq	mm5,	mm1		; 0
	punpcklbw	mm2,	mm0		; 0 1 unpack
	punpcklbw	mm1,	mm0		; 0 1 unpack
	punpckhbw	mm6,	mm0		; 0 2 unpack
	punpckhbw	mm5,	mm0		; 0 2 unpack
	psubw	mm2,	mm1		; 0 1 mm2 -= mm1
	psubw	mm6,	mm5		; 0 2 mm6 -= mm5
	pmullw	mm2,	mm4		; 0 1 mm2 *= mm4
	psllw	mm1,	8
	pmullw	mm6,	mm4		; 0 2 mm6 *= mm4
	psllw	mm5,	8
	paddw	mm1,	mm2		; 0 1 mm1 += mm2
	paddw	mm5,	mm6		; 0 2 mm5 += mm2
	psrlw	mm1,	8
	psrlw	mm5,	8
	add	ebx,	byte 16
	packuswb	mm1,	mm5		; 0 pack
	movq	mm6,	mm3		; 1
	movq	mm5,	mm7		; 1
	punpcklbw	mm3,	mm0		; 1 1 unpack
	punpckhbw	mm6,	mm0		; 1 2 unpack
	punpcklbw	mm7,	mm0		; 1 1 unpack
	punpckhbw	mm5,	mm0		; 1 2 unpack
	psubw	mm3,	mm7		; 1 1 mm3 -= mm7
	psubw	mm6,	mm5		; 1 2 mm6 -= mm5
	pmullw	mm3,	mm4		; 1 1 mm3 *= mm4
	psllw	mm7,	8
	pmullw	mm6,	mm4		; 1 2 mm6 *= mm4
	psllw	mm5,	8
	movq	[edi+ebx-16], mm1		; 0 store
	paddw	mm7,	mm3		; 1 mm7 += mm3
	paddw	mm5,	mm6		; 1 2 mm5 += mm3
	psrlw	mm7,	8
	psrlw	mm5,	8
	cmp	ebx,	ebp
	packuswb	mm7,	mm5		; 1 pack
	movq	[edi+ebx+8-16], mm7		; 1 store

	jl	near .ploop

.pfraction:
	add	ebp,	byte 16
	cmp	ebx,	ebp
	jge	.pexit		; jump if ebx >= ebp

.ploop2:	; fractions
	movd	mm2,	[esi+ebx]		; src1
	movd	mm1,	[eax+ebx]		; src2
	punpcklbw	mm2,	mm0		; unpack
	punpcklbw	mm1,	mm0		; unpack
	psubw	mm2,	mm1		; mm2 -= mm1
	pmullw	mm2,	mm4		; mm2 *= mm4
	psllw	mm1,	8
	paddw	mm1,	mm2		; mm1 += mm2
	psrlw	mm1,	8
	add	ebx,	byte 4
	packuswb	mm1,	mm0		; pack
	cmp	ebx,	ebp
	movd	[edi-4+ebx], mm1		; store

	jl	.ploop2		; jump if ebx < ebp

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
%endif
