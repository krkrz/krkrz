; -*- Mode: asm; tab-width: 12; -*-
; this is a part of TVP (KIRIKIRI) software source.
; see other sources for license.
; (C)2001-2009 W.Dee <dee@kikyou.info> and contributors

; pixel darken blender

%include		"nasm.nah"


globaldef		TVPDarkenBlend_mmx_a
globaldef		TVPDarkenBlend_HDA_mmx_a



; The basic algorithm is:
; dest = b - sat(b-a)
; where sat(x) = x<0?0:x (zero lower saturation)





	segment_code
;--------------------------------------------------------------------

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPDarkenBlend
;;void, TVPDarkenBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
	function_align
TVPDarkenBlend_mmx_a:			; pixel darken blender
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
			; align destination pointer to QWORD
		movd	mm1,	[edi]		; dest
		movq	mm2,	mm1
		movd	mm3,	[ebp]
		psubusb	mm2,	mm3
		psubb	mm1,	mm2
		movd	[edi],	mm1

		add	edi,	byte 4
		add	ebp,	byte 4
		cmp	edi,	esi

		jae	short .pfraction		; jump if edi >= esi
	ENDIF

	loop_align
.ploop:
	movq	mm1,	[edi]		; dest
	movq	mm3,	[edi+8]		; dest
	add	ebp,	byte 16
	movq	mm2,	mm1
	movq	mm4,	mm3
	add	edi,	byte 16
	psubusb	mm2,	[ebp-16]
	psubusb	mm4,	[ebp+8-16]
	cmp	edi,	esi
	psubb	mm1,	mm2
	psubb	mm3,	mm4
	movq	[edi-16],	mm1
	movq	[edi+8-16],	mm3

	jb	short .ploop

.pfraction:
	add	esi,	byte 16
	cmp	edi,	esi
	jae	.pexit		; jump if edi >= esi

.ploop2:	; fractions
	movd	mm1,	[edi]		; dest
	movq	mm2,	mm1
	movd	mm3,	[ebp]
	psubusb	mm2,	mm3
	psubb	mm1,	mm2
	movd	[edi],	mm1

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

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPDarkenBlend_HDA
;;void, TVPDarkenBlend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)

	function_align
TVPDarkenBlend_HDA_mmx_a:			; pixel darken blender (holding desitination alpha)
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
	mov	eax,	000ffffffh
	movd	mm7,	eax
	punpckldq	mm7,	mm7		; mask
	mov	edi,	[esp + 28]		; dest
	mov	ebp,	[esp + 32]		; src
	lea	esi,	[edi + ecx*4]		; limit
	sub	esi,	byte 16		; 4*4
	cmp	edi,	esi
	jae	near .pfraction		; jump if edi >= esi


	test	edi,	4
	IF	nz
			; align destination pointer to QWORD
		movd	mm1,	[edi]		; dest
		por	mm1,	mm7
		movq	mm2,	mm1
		movd	mm3,	[ebp]
		psubusb	mm2,	mm3
		pand	mm2,	mm7
		psubb	mm1,	mm2
		movd	[edi],	mm1

		add	edi,	byte 4
		add	ebp,	byte 4
		cmp	edi,	esi

		jae	short .pfraction		; jump if edi >= esi
	ENDIF

	loop_align
.ploop:
	movq	mm1,	[edi]		; dest
	movq	mm3,	[edi+8]		; dest
	add	ebp,	byte 16
	movq	mm2,	mm1
	movq	mm4,	mm3
	add	edi,	byte 16
	psubusb	mm2,	[ebp-16]
	psubusb	mm4,	[ebp+8-16]
	pand	mm2,	mm7
	pand	mm4,	mm7
	cmp	edi,	esi
	psubb	mm1,	mm2
	psubb	mm3,	mm4
	movq	[edi-16],	mm1
	movq	[edi+8-16],	mm3

	jb	short .ploop

.pfraction:
	add	esi,	byte 16
	cmp	edi,	esi
	jae	.pexit		; jump if edi >= esi

.ploop2:	; fractions
	movd	mm1,	[edi]		; dest
	por	mm1,	mm7
	movq	mm2,	mm1
	movd	mm3,	[ebp]
	psubusb	mm2,	mm3
	pand	mm2,	mm7
	psubb	mm1,	mm2
	movd	[edi],	mm1

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

