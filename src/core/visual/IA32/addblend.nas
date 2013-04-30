; -*- Mode: asm; tab-width: 12; -*-
; this is a part of TVP (KIRIKIRI) software source.
; see other sources for license.
; (C)2001-2009 W.Dee <dee@kikyou.info> and contributors

; pixel additive blender

%include		"nasm.nah"


globaldef		TVPAddBlend_mmx_a
globaldef		TVPAddBlend_HDA_mmx_a
globaldef		TVPAddBlend_o_mmx_a
globaldef		TVPAddBlend_HDA_o_mmx_a


	segment_code
;--------------------------------------------------------------------

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPAddBlend
;;void, TVPAddBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
	function_align
TVPAddBlend_mmx_a:			; pixel additive blender
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
	lea	esi,	[edi + ecx*4]		; limit
	sub	esi,	byte 16		; 4*4
	cmp	edi,	esi
	jae	near .pfraction		; jump if edi >= esi

	test	edi,	4
	IF	nz
			;	align destination pointer to QWORD

		movd	mm0,	[edi]		; dest
		movd	mm1,	[ebp]		; src
		paddusb	mm0,	mm1		; dest += src
		movd	[edi],	mm0		; store
		add	edi,	byte 4
		add	ebp,	byte 4
		cmp	edi,	esi

		jae	near .pfraction		; jump if edi >= esi
	ENDIF


	loop_align
.ploop:
	movq	mm0,	[edi]		; 1 dest
	movq	mm1,	[edi + 8]		; 2 dest
	paddusb	mm0,	[ebp]		; 1 dest += src
	add	edi,	byte 16
	paddusb	mm1,	[ebp + 8]		; 2 dest += src
	movq	[edi-16],	mm0		; 1 store
	add	ebp,	byte 16
	cmp	edi,	esi
	movq	[edi+8-16],	mm1		; 2 store

	jb	short .ploop

.pfraction:
	add	esi,	byte 16
	cmp	edi,	esi
	jae	.pexit		; jump if edi >= esi

.ploop2:	; fractions
	movd	mm0,	[edi]		; dest
	movd	mm1,	[ebp]		; src
	paddusb	mm0,	mm1		; dest += src
	movd	[edi],	mm0		; store
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

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPAddBlend_HDA
;;void, TVPAddBlend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
	function_align
TVPAddBlend_HDA_mmx_a:			; pixel additive blender (holding desitination alpha)
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 36]		; len
	cmp	ecx,	byte 0
	jle	near .pexit
	mov	edx,	000ffffffh
	movd	mm7,	edx
	punpckldq	mm7,	mm7		; mm7 = 0x00ffffff00ffffff
	mov	edi,	[esp + 28]		; dest
	mov	ebp,	[esp + 32]		; src
	lea	esi,	[edi + ecx*4]		; limit
	sub	esi,	byte 16		; 3*4
	cmp	edi,	esi
	jae	near .pfraction		; jump if edi >= esi


	test	edi,	4
	IF	nz
			;	align destination pointer to QWORD

		movd	mm0,	[edi]		; dest
		movd	mm1,	[ebp]		; src
		pand	mm1,	mm7		; mask
		paddusb	mm0,	mm1		; dest += src
		movd	[edi],	mm0		; store
		add	edi,	byte 4
		add	ebp,	byte 4
		cmp	edi,	esi

		jae	near .pfraction		; jump if edi >= esi

	ENDIF

	loop_align
.ploop:
	movq	mm0,	[ebp]		; 1 src
	movq	mm1,	[ebp+8]		; 2 src
	pand	mm0,	mm7		; 1 mask
	pand	mm1,	mm7		; 2 mask
	paddusb	mm0,	[edi]		; 1 src += dest
	paddusb	mm1,	[edi+8]		; 2 src += dest
	movq	[edi],	mm0		; 1 store
	movq	[edi+8],	mm1		; 2 store

	add	ebp,	byte 16
	add	edi,	byte 16
	cmp	edi,	esi
	jb	short .ploop

.pfraction:
	add	esi,	byte 16
	cmp	edi,	esi
	jae	.pexit		; jump if edi >= esi

.ploop2:	; fractions
	movd	mm0,	[edi]		; dest
	movd	mm1,	[ebp]		; src
	pand	mm1,	mm7		; mask
	paddusb	mm0,	mm1		; dest += src
	movd	[edi],	mm0		; store
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

; the routine below always holds the destination alpha

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPAddBlend_o
;;void, TVPAddBlend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPAddBlend_HDA_o
;;void, TVPAddBlend_HDA_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
	function_align
TVPAddBlend_o_mmx_a:			; pixel additive blender
TVPAddBlend_HDA_o_mmx_a:		; pixel additive blender
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 36]		; len
	cmp	ecx,	byte 0
	jle	near .pexit
	movd	mm7,	[esp + 40]		; opa
	punpcklwd	mm7,	mm7
	punpcklwd	mm7,	mm7		; mm7 = 00oo00oo00oo00oo
	psrlq	mm7,	16		; mm7 = 000000oo00oo00oo
	pxor	mm0,	mm0		; mm0 = 0
	mov	edi,	[esp + 28]		; dest
	mov	ebp,	[esp + 32]		; src
	lea	esi,	[edi + ecx*4]		; limit
	sub	esi,	byte 16		; 3*4
	cmp	edi,	esi
	jae	near .pfraction		; jump if edi >= esi

	test	edi,	4
	IF	nz
			;	align destination pointer to QWORD

		movd	mm1,	[edi]		; dest
		punpcklbw	mm1,	mm0		; mm1 = 00dd00dd00dd00dd
		movd	mm2,	[ebp]		; src
		punpcklbw	mm2,	mm0		; unpack mm2 = 00ss00ss00ss00ss
		pmullw	mm2,	mm7		; mm2 *= opa
		psrlw	mm2,	8		; mm2 >>=8
		paddw	mm1,	mm2		; mm1 += mm2
		packuswb	mm1,	mm0		; pack
		movd	[edi],	mm1		; store
		add	edi,	byte 4
		add	ebp,	byte 4
		cmp	edi,	esi

		jae	near .pfraction		; jump if edi >= esi

	ENDIF


	loop_align
.ploop:
	movq	mm1,	[edi]		; 1 1 dest
	movq	mm3,	mm1		; 1
	punpcklbw	mm1,	mm0		; 1 1 mm1 = 00dd00dd00dd00dd
	punpckhbw	mm3,	mm0		; 1 2 mm3 = 00dd00dd00dd00dd
	movq	mm2,	[ebp]		; 1 1 src
	movq	mm4,	mm2		; 1
	punpcklbw	mm2,	mm0		; 1 1 unpack mm2 = 00ss00ss00ss00ss
	movq	mm5,	[edi+8]		; 2 1 dest
	punpckhbw	mm4,	mm0		; 1 2 unpack mm4 = 00ss00ss00ss00ss
	pmullw	mm2,	mm7		; 1 1 mm2 *= opa
	movq	mm6,	mm5		; 2
	pmullw	mm4,	mm7		; 1 2 mm4 *= opa
	punpcklbw	mm5,	mm0		; 2 1 mm5 = 00dd00dd00dd00dd
	psrlw	mm2,	8		; 1 1 mm2 >>=8
	psrlw	mm4,	8		; 1 2 mm4 >>=8
	paddw	mm1,	mm2		; 1 1 mm1 += mm2
	paddw	mm3,	mm4		; 1 2 mm3 += mm4
	punpckhbw	mm6,	mm0		; 2 2 mm6 = 00dd00dd00dd00dd
	movq	mm2,	[ebp+8]		; 2 1 src
	packuswb	mm1,	mm3		; 1 1 pack
	movq	[edi],	mm1		; 1 1 store
	movq	mm4,	mm2		; 2
	punpcklbw	mm2,	mm0		; 2 1 unpack mm3 = 00ss00ss00ss00ss
	punpckhbw	mm4,	mm0		; 2 2 unpack mm4 = 00ss00ss00ss00ss
	pmullw	mm2,	mm7		; 2 1 mm3 *= opa
	add	edi,	byte 16
	pmullw	mm4,	mm7		; 2 2 mm4 *= opa
	add	ebp,	byte 16
	psrlw	mm2,	8		; 2 1 mm3 >>=8
	psrlw	mm4,	8		; 2 2 mm4 >>=8
	paddw	mm5,	mm2		; 2 1 mm5 += mm2
	cmp	edi,	esi
	paddw	mm6,	mm4		; 2 2 mm6 += mm4
	packuswb	mm5,	mm6		; 2 1 pack
	movq	[edi+8-16], mm5		; 2 1 store

	jb	short .ploop

.pfraction:
	add	esi,	byte 16
	cmp	edi,	esi
	jae	.pexit		; jump if edi >= esi

.ploop2:	; fractions
	movd	mm1,	[edi]		; dest
	punpcklbw	mm1,	mm0		; mm1 = 00dd00dd00dd00dd
	movd	mm2,	[ebp]		; src
	punpcklbw	mm2,	mm0		; unpack mm2 = 00ss00ss00ss00ss
	pmullw	mm2,	mm7		; mm2 *= opa
	psrlw	mm2,	8		; mm2 >>=8
	paddw	mm1,	mm2		; mm1 += mm2
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
