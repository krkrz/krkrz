; -*- Mode: asm; tab-width: 12; -*-
; this is a part of TVP (KIRIKIRI) software source.
; see other sources for license.
; (C)2001-2009 W.Dee <dee@kikyou.info> and contributors

; pixel lighten blender

%include		"nasm.nah"


globaldef		TVPLightenBlend_mmx_a
globaldef		TVPLightenBlend_HDA_mmx_a



; The basic algorithm is:
; dest = b + sat(a-b)
; where sat(x) = x<0?0:x (zero lower saturation)





	segment_code
;--------------------------------------------------------------------

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPLightenBlend
;;void, TVPLightenBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
	function_align
TVPLightenBlend_mmx_a:			; pixel lighten blender
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
		movd	mm2,	[ebp]		; dest
		psubusb	mm2,	[edi]
		paddb	mm2,	[edi]
		movd	[edi],	mm2

		add	edi,	byte 4
		add	ebp,	byte 4
		cmp	edi,	esi

		jae	short .pfraction		; jump if edi >= esi
	ENDIF

	loop_align
.ploop:
	movq	mm2,	[ebp]
	movq	mm4,	[ebp+8]
	add	edi,	byte 16
	add	ebp,	byte 16
	psubusb	mm2,	[edi-16]
	psubusb	mm4,	[edi+8-16]
	paddb	mm2,	[edi-16]
	paddb	mm4,	[edi+8-16]
	cmp	edi,	esi
	movq	[edi-16],	mm2
	movq	[edi+8-16],	mm4

	jb	short .ploop

.pfraction:
	add	esi,	byte 16
	cmp	edi,	esi
	jae	.pexit		; jump if edi >= esi

.ploop2:	; fractions
	movd	mm2,	[ebp]		; dest
	psubusb	mm2,	[edi]
	paddb	mm2,	[edi]
	movd	[edi],	mm2

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

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPLightenBlend_HDA
;;void, TVPLightenBlend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)

	function_align
TVPLightenBlend_HDA_mmx_a:			; pixel lighten blender (holding desitination alpha)
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
		movq	mm2,	[ebp]
		pand	mm2,	mm7
		psubusb	mm2,	[edi]
		paddb	mm2,	[edi]
		movd	[edi],	mm2

		add	edi,	byte 4
		add	ebp,	byte 4
		cmp	edi,	esi

		jae	short .pfraction		; jump if edi >= esi
	ENDIF

	loop_align
.ploop:
	movq	mm2,	[ebp]
	movq	mm4,	[ebp+8]
	add	edi,	byte 16
	pand	mm2,	mm7
	pand	mm4,	mm7
	add	ebp,	byte 16
	psubusb	mm2,	[edi-16]
	psubusb	mm4,	[edi+8-16]
	paddb	mm2,	[edi-16]
	paddb	mm4,	[edi+8-16]
	cmp	edi,	esi
	movq	[edi-16],	mm2
	movq	[edi+8-16],	mm4

	jb	short .ploop

.pfraction:
	add	esi,	byte 16
	cmp	edi,	esi
	jae	.pexit		; jump if edi >= esi

.ploop2:	; fractions
	movq	mm2,	[ebp]
	pand	mm2,	mm7
	psubusb	mm2,	[edi]
	paddb	mm2,	[edi]
	movd	[edi],	mm2

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

