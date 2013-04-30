; -*- Mode: asm; tab-width: 12; -*-
; this is a part of TVP (KIRIKIRI) software source.
; see other sources for license.
; (C)2001-2009 W.Dee <dee@kikyou.info> and contributors

; pixel multiplicative blender

%include		"nasm.nah"


globaldef		TVPMulBlend_mmx_a
globaldef		TVPMulBlend_HDA_mmx_a
globaldef		TVPMulBlend_o_mmx_a
globaldef		TVPMulBlend_HDA_o_mmx_a


	segment_code
;--------------------------------------------------------------------

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPMulBlend
;;void, TVPMulBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
	function_align
TVPMulBlend_mmx_a:			; pixel multiplicative blender
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 36]		; len
	cmp	ecx,	byte 0
	jle	near .pexit
	pxor	mm0,	mm0
	mov	edi,	[esp + 28]		; dest
	mov	ebp,	[esp + 32]		; src
	lea	esi,	[edi + ecx*4]		; limit
	sub	esi,	byte 16		; 4*4
	cmp	edi,	esi
	jae	near .pfraction		; jump if edi >= esi

	test	edi,	4
	IF	nz
			;	align destination pointer to QWORD
		movd	mm1,	[edi]		; dest
		punpcklbw	mm1,	mm0		; 
		movd	mm3,	[ebp]		; src
		punpcklbw	mm3,	mm0		; 
		pmullw	mm1,	mm3		; multiply
		psrlw	mm1,	8		; shift
		packuswb	mm1,	mm0		; pack
		movd	[edi],	mm1		; store

		add	edi,	byte 4
		add	ebp,	byte 4
		cmp	edi,	esi

		jae	short .pfraction		; jump if edi >= esi

	ENDIF


	loop_align
.ploop:
	movq	mm1,	[edi]		; 1 dest
	movq	mm2,	mm1		; 1
	punpcklbw	mm1,	mm0		; 1
	punpckhbw	mm2,	mm0		; 1
	movq	mm3,	[ebp]		; 1 src
	movq	mm4,	mm3		; 1
	punpcklbw	mm3,	mm0		; 1
	punpckhbw	mm4,	mm0		; 1
	pmullw	mm1,	mm3		; 1 multiply
	pmullw	mm2,	mm4		; 1 multiply
	movq	mm3,	[edi+8]		; 2 dest
	psrlw	mm1,	8		; 1 shift
	psrlw	mm2,	8		; 1 shift
	movq	mm4,	mm3		; 2
	packuswb	mm1,	mm2		; 1 pack
	punpcklbw	mm3,	mm0		; 2
	movq	[edi],	mm1		; 1 store
	punpckhbw	mm4,	mm0		; 2
	movq	mm1,	[ebp+8]		; 2 src
	movq	mm2,	mm1		; 2
	punpcklbw	mm1,	mm0		; 2
	punpckhbw	mm2,	mm0		; 2
	pmullw	mm3,	mm1		; 2 multiply
	add	edi,	byte 16
	pmullw	mm4,	mm2		; 2 multiply
	add	ebp,	byte 16
	psrlw	mm3,	8		; 2 shift
	psrlw	mm4,	8		; 2 shift
	cmp	edi,	esi
	packuswb	mm3,	mm4		; 2 pack
	movq	[edi+8-16],	mm3		; 2 store

	jb	short .ploop

.pfraction:
	add	esi,	byte 16
	cmp	edi,	esi
	jae	.pexit		; jump if edi >= esi

.ploop2:	; fractions
	movd	mm1,	[edi]		; dest
	punpcklbw	mm1,	mm0		; 
	movd	mm3,	[ebp]		; src
	punpcklbw	mm3,	mm0		; 
	pmullw	mm1,	mm3		; multiply
	psrlw	mm1,	8		; shift
	packuswb	mm1,	mm0		; pack
	movd	[edi],	mm1		; store

	add	edi,	byte 4
	add	ebp,	byte 4
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

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPMulBlend_HDA
;;void, TVPMulBlend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)

	segment_data

	align 16
TVPMulBlendHDA_mulmask		dd	0ffffffffh
		dd	00000ffffh
TVPMulBlendHDA_100bit		dd	000000000h
		dd	001000000h

	segment_code

	function_align
TVPMulBlend_HDA_mmx_a:			; pixel multiplicative blender (holding desitination alpha)
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 36]		; len
	cmp	ecx,	byte 0
	jle	near .pexit
	pxor	mm0,	mm0
	movq	mm6,	[TVPMulBlendHDA_mulmask]
	movq	mm7,	[TVPMulBlendHDA_100bit]
	mov	edi,	[esp + 28]		; dest
	mov	ebp,	[esp + 32]		; src
	lea	esi,	[edi + ecx*4]		; limit
	sub	esi,	byte 16		; 4*4
	cmp	edi,	esi
	jae	near .pfraction		; jump if edi >= esi

	test	edi,	4
	IF	nz
			;	align destination pointer to QWORD
		movd	mm1,	[edi]		; dest
		punpcklbw	mm1,	mm0		; 
		movd	mm3,	[ebp]		; src
		punpcklbw	mm3,	mm0		; 
		pand	mm3,	mm6		; 1 mask
		por	mm3,	mm7		; 1
		pmullw	mm1,	mm3		; multiply
		psrlw	mm1,	8		; shift
		packuswb	mm1,	mm0		; pack
		movd	[edi],	mm1		; store

		add	edi,	byte 4
		add	ebp,	byte 4
		cmp	edi,	esi

		jae	near .pfraction		; jump if edi >= esi
	ENDIF


	loop_align
.ploop:
	movq	mm1,	[edi]		; 1 dest
	movq	mm2,	mm1		; 1
	punpcklbw	mm1,	mm0		; 1
	punpckhbw	mm2,	mm0		; 1
	movq	mm3,	[ebp]		; 1 src
	movq	mm4,	mm3		; 1
	punpcklbw	mm3,	mm0		; 1
	punpckhbw	mm4,	mm0		; 1
	pand	mm3,	mm6		; 1 mask
	por	mm3,	mm7		; 1
	pand	mm4,	mm6		; 1 mask
	por	mm4,	mm7		; 1
	pmullw	mm1,	mm3		; 1 multiply
	pmullw	mm2,	mm4		; 1 multiply
	psrlw	mm1,	8		; 1 shift
	psrlw	mm2,	8		; 1 shift
	packuswb	mm1,	mm2		; 1 pack
	movq	[edi],	mm1		; 1 store

	add	edi,	byte 8
	add	ebp,	byte 8

	movq	mm1,	[edi]		; 1 dest
	movq	mm2,	mm1		; 1
	punpcklbw	mm1,	mm0		; 1
	punpckhbw	mm2,	mm0		; 1
	movq	mm3,	[ebp]		; 1 src
	movq	mm4,	mm3		; 1
	punpcklbw	mm3,	mm0		; 1
	punpckhbw	mm4,	mm0		; 1
	pand	mm3,	mm6		; 1 mask
	por	mm3,	mm7		; 1
	pand	mm4,	mm6		; 1 mask
	por	mm4,	mm7		; 1
	pmullw	mm1,	mm3		; 1 multiply
	pmullw	mm2,	mm4		; 1 multiply
	psrlw	mm1,	8		; 1 shift
	psrlw	mm2,	8		; 1 shift
	packuswb	mm1,	mm2		; 1 pack
	movq	[edi],	mm1		; 1 store

	add	edi,	byte 8
	add	ebp,	byte 8

	cmp	edi,	esi
	jb	near .ploop

.pfraction:
	add	esi,	byte 16
	cmp	edi,	esi
	jae	.pexit		; jump if edi >= esi

.ploop2:	; fractions
	movd	mm1,	[edi]		; dest
	punpcklbw	mm1,	mm0		; 
	movd	mm3,	[ebp]		; src
	punpcklbw	mm3,	mm0		; 
	pand	mm3,	mm6		; 1 mask
	por	mm3,	mm7		; 1
	pmullw	mm1,	mm3		; multiply
	psrlw	mm1,	8		; shift
	packuswb	mm1,	mm0		; pack
	movd	[edi],	mm1		; store

	add	edi,	byte 4
	add	ebp,	byte 4
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

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPMulBlend_o
;;void, TVPMulBlend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)

	segment_data

	align 16
TVPMulBlendHDA_fullbit		dd	0ffffffffh
		dd	0ffffffffh

	segment_code

	function_align
TVPMulBlend_o_mmx_a:			; pixel multiplicative blender with opacity
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 36]		; len
	cmp	ecx,	byte 0
	jle	near .pexit
	movd	mm5,	[esp + 40]		; opa
	punpcklwd	mm5,	mm5
	punpcklwd	mm5,	mm5		; mm5 = 00oo00oo00oo00oo
	pxor	mm0,	mm0
	movq	mm6,	[TVPMulBlendHDA_fullbit]
	mov	edi,	[esp + 28]		; dest
	mov	ebp,	[esp + 32]		; src
	lea	esi,	[edi + ecx*4]		; limit
	sub	esi,	byte 16		; 4*4
	cmp	edi,	esi
	jae	near .pfraction		; jump if edi >= esi

	test	edi,	4
	IF	nz
			;	align destination pointer to QWORD
		movd	mm1,	[edi]		; dest
		punpcklbw	mm1,	mm0		; 
		movd	mm3,	[ebp]		; src
		pxor	mm3,	mm6		; 1 not src
		punpcklbw	mm3,	mm0		; 
		pmullw	mm3,	mm5		; 1 opa multiply
		pxor	mm3,	mm6		; 1 not src
		psrlw	mm3,	8		; 1 opa shift
		pmullw	mm1,	mm3		; multiply
		psrlw	mm1,	8		; shift
		packuswb	mm1,	mm0		; pack
		movd	[edi],	mm1		; store

		add	edi,	byte 4
		add	ebp,	byte 4
		cmp	edi,	esi

		jae	near .pfraction		; jump if edi >= esi
	ENDIF


	loop_align
.ploop:
	movq	mm1,	[edi]		; 1 dest
	movq	mm2,	mm1		; 1
	punpcklbw	mm1,	mm0		; 1
	punpckhbw	mm2,	mm0		; 1
	movq	mm3,	[ebp]		; 1 src
	pxor	mm3,	mm6		; 1 not src
	movq	mm4,	mm3		; 1
	punpcklbw	mm3,	mm0		; 1
	punpckhbw	mm4,	mm0		; 1
	pmullw	mm3,	mm5		; 1 opa multiply
	pmullw	mm4,	mm5		; 1 opa multiply
	pxor	mm3,	mm6		; 1 not src
	pxor	mm4,	mm6		; 1 not src
	psrlw	mm3,	8		; 1 opa shift
	psrlw	mm4,	8		; 1 opa shift
	pmullw	mm1,	mm3		; 1 multiply
	pmullw	mm2,	mm4		; 1 multiply
	psrlw	mm1,	8		; 1 shift
	psrlw	mm2,	8		; 1 shift
	packuswb	mm1,	mm2		; 1 pack
	movq	[edi],	mm1		; 1 store

	add	edi,	byte 8
	add	ebp,	byte 8

	movq	mm1,	[edi]		; 1 dest
	movq	mm2,	mm1		; 1
	punpcklbw	mm1,	mm0		; 1
	punpckhbw	mm2,	mm0		; 1
	movq	mm3,	[ebp]		; 1 src
	pxor	mm3,	mm6		; 1 not src
	movq	mm4,	mm3		; 1
	punpcklbw	mm3,	mm0		; 1
	punpckhbw	mm4,	mm0		; 1
	pmullw	mm3,	mm5		; 1 opa multiply
	pmullw	mm4,	mm5		; 1 opa multiply
	pxor	mm3,	mm6		; 1 not src
	pxor	mm4,	mm6		; 1 not src
	psrlw	mm3,	8		; 1 opa shift
	psrlw	mm4,	8		; 1 opa shift
	pmullw	mm1,	mm3		; 1 multiply
	pmullw	mm2,	mm4		; 1 multiply
	psrlw	mm1,	8		; 1 shift
	psrlw	mm2,	8		; 1 shift
	packuswb	mm1,	mm2		; 1 pack
	movq	[edi],	mm1		; 1 store

	add	edi,	byte 8
	add	ebp,	byte 8

	cmp	edi,	esi
	jb	near .ploop

.pfraction:
	add	esi,	byte 16
	cmp	edi,	esi
	jae	.pexit		; jump if edi >= esi

.ploop2:	; fractions
	movd	mm1,	[edi]		; dest
	punpcklbw	mm1,	mm0		; 
	movd	mm3,	[ebp]		; src
	pxor	mm3,	mm6		; 1 not src
	punpcklbw	mm3,	mm0		; 
	pmullw	mm3,	mm5		; 1 opa multiply
	pxor	mm3,	mm6		; 1 not src
	psrlw	mm3,	8		; 1 opa shift
	pmullw	mm1,	mm3		; multiply
	psrlw	mm1,	8		; shift
	packuswb	mm1,	mm0		; pack
	movd	[edi],	mm1		; store

	add	edi,	byte 4
	add	ebp,	byte 4
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

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPMulBlend_HDA_o
;;void, TVPMulBlend_HDA_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)

	segment_data

	align 16
		dd 0ffffffffh
		dd 0ffffffffh
		dd 0ffffffffh
		dd 0ffffffffh
TVPMulBlend_full_bit_aligned	dd 0ffffffffh
		dd 0ffffffffh

	segment_code

	function_align
TVPMulBlend_HDA_o_mmx_a:			; pixel multiplicative blender with opacity (HDA)
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 36]		; len
	cmp	ecx,	byte 0
	jle	near .pexit
	movd	mm5,	[esp + 40]		; opa
	punpcklwd	mm5,	mm5
	punpcklwd	mm5,	mm5		; mm5 = 00oo00oo00oo00oo
	pxor	mm0,	mm0
	movq	mm6,	[TVPMulBlendHDA_mulmask]
	movq	mm7,	[TVPMulBlendHDA_100bit]
	lea	edx,	[TVPMulBlend_full_bit_aligned]		; ptr to 0xffffffffffffffff
	and	edx,	0fffffff0h		; align to 16-bytes
			; Borland linker seems not to align except for 4bytes...
	mov	edi,	[esp + 28]		; dest
	mov	ebp,	[esp + 32]		; src
	lea	esi,	[edi + ecx*4]		; limit
	sub	esi,	byte 16		; 4*4
	cmp	edi,	esi
	jae	near .pfraction		; jump if edi >= esi

	test	edi,	4
	IF	nz
			;	align destination pointer to QWORD
		movd	mm1,	[edi]		; dest
		punpcklbw	mm1,	mm0		; 
		movd	mm3,	[ebp]		; src
		pxor	mm3,	[edx]		; 1 not src
		punpcklbw	mm3,	mm0		; 
		pmullw	mm3,	mm5		; opa multiply
		pxor	mm3,	[edx]		; 1 not src
		psrlw	mm3,	8		; opa shift
		pand	mm3,	mm6		; mask
		por	mm3,	mm7		; 
		pmullw	mm1,	mm3		; multiply
		psrlw	mm1,	8		; shift
		packuswb	mm1,	mm0		; pack
		movd	[edi],	mm1		; store

		add	edi,	byte 4
		add	ebp,	byte 4
		cmp	edi,	esi

		jae	near .pfraction		; jump if edi >= esi
	ENDIF


	loop_align
.ploop:
	movq	mm1,	[edi]		; 1 dest
	movq	mm2,	mm1		; 1
	punpcklbw	mm1,	mm0		; 1
	punpckhbw	mm2,	mm0		; 1
	movq	mm3,	[ebp]		; 1 src
	pxor	mm3,	[edx]		; 1 not src
	movq	mm4,	mm3		; 1
	punpcklbw	mm3,	mm0		; 1
	punpckhbw	mm4,	mm0		; 1
	pmullw	mm3,	mm5		; 1 opa multiply
	pmullw	mm4,	mm5		; 1 opa multiply
	pxor	mm3,	[edx]		; 1 not src
	pxor	mm4,	[edx]		; 1 not src
	psrlw	mm3,	8		; 1 opa shift
	psrlw	mm4,	8		; 1 opa shift
	pand	mm3,	mm6		; 1 mask
	por	mm3,	mm7		; 1
	pand	mm4,	mm6		; 1 mask
	por	mm4,	mm7		; 1
	pmullw	mm1,	mm3		; 1 multiply
	pmullw	mm2,	mm4		; 1 multiply
	psrlw	mm1,	8		; 1 shift
	psrlw	mm2,	8		; 1 shift
	packuswb	mm1,	mm2		; 1 pack
	movq	[edi],	mm1		; 1 store

	add	edi,	byte 8
	add	ebp,	byte 8

	movq	mm1,	[edi]		; 1 dest
	movq	mm2,	mm1		; 1
	punpcklbw	mm1,	mm0		; 1
	punpckhbw	mm2,	mm0		; 1
	movq	mm3,	[ebp]		; 1 src
	pxor	mm3,	[edx]		; 1 not src
	movq	mm4,	mm3		; 1
	punpcklbw	mm3,	mm0		; 1
	punpckhbw	mm4,	mm0		; 1
	pmullw	mm3,	mm5		; 1 opa multiply
	pmullw	mm4,	mm5		; 1 opa multiply
	pxor	mm3,	[edx]		; 1 not src
	pxor	mm4,	[edx]		; 1 not src
	psrlw	mm3,	8		; 1 opa shift
	psrlw	mm4,	8		; 1 opa shift
	pand	mm3,	mm6		; 1 mask
	por	mm3,	mm7		; 1
	pand	mm4,	mm6		; 1 mask
	por	mm4,	mm7		; 1
	pmullw	mm1,	mm3		; 1 multiply
	pmullw	mm2,	mm4		; 1 multiply
	psrlw	mm1,	8		; 1 shift
	psrlw	mm2,	8		; 1 shift
	packuswb	mm1,	mm2		; 1 pack
	movq	[edi],	mm1		; 1 store

	add	edi,	byte 8
	add	ebp,	byte 8

	cmp	edi,	esi
	jb	near .ploop

.pfraction:
	add	esi,	byte 16
	cmp	edi,	esi
	jae	.pexit		; jump if edi >= esi

.ploop2:	; fractions
	movd	mm1,	[edi]		; dest
	punpcklbw	mm1,	mm0		; 
	movd	mm3,	[ebp]		; src
	pxor	mm3,	[edx]		; 1 not src
	punpcklbw	mm3,	mm0		; 
	pmullw	mm3,	mm5		; opa multiply
	pxor	mm3,	[edx]		; 1 not src
	psrlw	mm3,	8		; opa shift
	pand	mm3,	mm6		; mask
	por	mm3,	mm7		; 
	pmullw	mm1,	mm3		; multiply
	psrlw	mm1,	8		; shift
	packuswb	mm1,	mm0		; pack
	movd	[edi],	mm1		; store

	add	edi,	byte 4
	add	ebp,	byte 4
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

