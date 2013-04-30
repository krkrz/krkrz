; -*- Mode: asm; tab-width: 12; -*-
; this is a part of TVP (KIRIKIRI) software source.
; see other sources for license.
; (C)2001-2009 W.Dee <dee@kikyou.info> and contributors

; pixel format conversion

%include		"nasm.nah"


globaldef		TVPConvert24BitTo32Bit_mmx_a
globaldef		TVPBLConvert24BitTo32Bit_mmx_a
globaldef		TVPDither32BitTo16Bit565_mmx_a
globaldef		TVPDither32BitTo16Bit555_mmx_a
externdef		TVPDitherTable_5_6

	segment_code
;--------------------------------------------------------------------

; these two are actually the same

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPConvert24BitTo32Bit
;;void, TVPConvert24BitTo32Bit_mmx_a, (tjs_uint32 *dest, const tjs_uint8 *buf, tjs_int len)

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPBLConvert24BitTo32Bit
;;void, TVPBLConvert24BitTo32Bit_mmx_a, (tjs_uint32 *dest, const tjs_uint8 *buf, tjs_int len)


	function_align
TVPConvert24BitTo32Bit_mmx_a:					; 24bpp RGB -> 32bpp ARGB
TVPBLConvert24BitTo32Bit_mmx_a:					; 24bpp RGB -> 32bpp ARGB
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	mov	esi,	[esp + 32]		; len
	cmp	esi,	byte 0
	jle	near .pexit
	mov	edi,	[esp + 24]		; dest
	mov	ecx,	[esp + 28]		; buf
	lea	ebx,	[edi + esi *4]		; ebx = limit

	sub	ebx,	byte 28		; 28 = 7*4
	cmp	edi,	ebx
	jae	near .pfraction		; jump if edi >= ebx

	xor	eax,	eax
	dec	eax		; eax = 0ffffffffh
	movd	mm5,	eax		; 00 00 00 00 ff ff ff ff
	movq	mm6,	mm5
	psllq	mm6,	32		; ff ff ff ff 00 00 00 00
	mov	eax,	0ff000000h
	movd	mm4,	eax
	movq	mm7,	mm4
	psllq	mm4,	32
	por	mm7,	mm4		; ff 00 00 00 ff 00 00 00

	loop_align
.ploop:
	movq	mm0,	[ecx]		; g2 b2 r1 g1 b1 r0 g0 b0
	movq	mm1,	[ecx + 8]		; b5 r4 g4 b4 r3 g3 b3 r2
	movq	mm3,	mm0
	movq	mm4,	mm0
	psllq	mm3,	8		; b2 r1 g1 b1 r0 g0 b0 00
	pand	mm4,	mm5		; 00 00 00 00 b1 r0 g0 b0
	pand	mm3,	mm6		; b2 r1 g1 b1 00 00 00 00
	psrlq	mm0,	48		; 00 00 00 00 00 00 g2 b2
	por	mm3,	mm4		; b2 r1 g1 b1 b1 r0 g0 b0
	movq	mm2,	mm1
	por	mm3,	mm7		; ff r1 g1 b1 ff r0 g0 b0
	movq	mm4,	mm1
	movq	[edi],	mm3		; store
	psllq	mm4,	16		; g4 b4 r3 g3 b3 r2 00 00
	movq	mm3,	mm1
	por	mm0,	mm4		; g4 b4 r3 g3 b3 r2 g2 b2
	psrlq	mm1,	32		; 00 00 00 00 b5 r4 g4 b4
	pand	mm0,	mm5		; 00 00 00 00 b3 r2 g2 b2
	psllq	mm3,	24		; b4 r3 g3 b3 r2 00 00 00
	por	mm0,	mm3		; b4 r3 g3 b3 xx r2 g2 b2
	psrlq	mm2,	24		; 00 00 00 b5 r4 g4 b4 r3
	movq	mm3,	[ecx + 16]		; r7 g7 b7 r6 g6 b6 r5 g5
	por	mm0,	mm7		; ff r3 g3 b3 ff r2 g2 b2
	pand	mm2,	mm6		; 00 00 00 b5 00 00 00 00
	movq	[edi+8], mm0		; store
	movq	mm4,	mm3
	movq	mm0,	mm3
	psllq	mm4,	40		; b6 r5 g5 00 00 00 00 00
	por	mm1,	mm7		; ff 00 00 00 ff r4 g4 b4
	por	mm2,	mm4		; b6 r5 g5 b5 00 00 00 00
	psrlq	mm0,	8		; 00 r7 g7 b7 r6 g6 b6 r5
	por	mm2,	mm1		; ff r5 g5 b5 ff r4 g4 b4
	pand	mm0,	mm6		; 00 r7 g7 b7 00 00 00 00
	movq	[edi+16], mm2		; store
	psrlq	mm3,	16		; 00 00 r7 g7 b7 r6 g6 b6
	add	edi,	byte 32
	pand	mm3,	mm5		; 00 00 00 00 b7 r6 g6 b6
	add	ecx,	byte 24
	por	mm3,	mm0		; 00 r7 g7 b7 b7 r6 g6 b6
	cmp	edi,	ebx
	por	mm3,	mm7		; ff r7 g7 b7 ff r6 g6 b6
	movq	[edi+24-32], mm3		; store

	jb	near .ploop		; jump if edi < ebx

.pfraction:
	add	ebx,	byte 28
	cmp	edi,	ebx
	jae	.pexit		; jump if edi >= ebx

.ploop2:
	movzx	eax,	byte [ecx]		; b
	movzx	edx,	byte [ecx+1]		; g
	shl	edx,	8
	add	eax,	edx
	movzx	edx,	byte [ecx+2]		; r
	shl	edx,	16
	add	eax,	edx
	or	eax,	0ff000000h		; opaque
	mov	[edi],	eax		; store

	add	edi,	byte 4
	add	ecx, 	byte 3
	cmp	edi,	ebx
	jb	.ploop2		; jump if edi < ebx

.pexit:
	pop	edx
	pop	ecx
	pop	ebx
	pop	esi
	pop	edi
	emms
	ret


;--------------------------------------------------------------------

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPDither32BitTo16Bit565
;;void, TVPDither32BitTo16Bit565_mmx_a, (tjs_uint16 *dest, const tjs_uint32 *src, tjs_int len, tjs_int xofs, tjs_int yofs)

	function_align
TVPDither32BitTo16Bit565_mmx_a:					; 32bpp (A)RGB -> 16bpp with ordered dithering
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
	lea	esi,	[ebp + ecx *4]		; esi = limit
	mov	eax,	[esp + 44]		; yofs
	and	eax,	3
	shl	eax,	11		; eax *= 4*2*256
	lea	edx,	[TVPDitherTable_5_6+eax] 	; line
	mov	ecx,	[esp + 40]		; xofs
	and	ecx,	3
	shl	ecx,	9		; ecx *= 2*256

	sub	esi,	byte 12		; 12 = 3*4
	cmp	ebp,	esi
	jae	near .pfraction		; jump if esi >= esi

	add	edx,	ecx		; edx = offset to table

	loop_align
.ploop:

	movq	mm0,	[ebp]		; a1 r1 g1 b1 a0 r0 g0 b0
	movq	mm3,	mm0
	psrlq	mm3,	32		; 00 00 00 00 a1 r1 g1 b1

	movd	eax,	mm0		; 0 a0 r0 g0 b0
	movd	ebx,	mm3		; 1 a1 r1 g1 b1
	and	eax,	0ffh		; 0 b
	and	ebx,	0ffh		; 1 b
	movzx	eax,	byte [edx+eax]		; 0 5bit dither table
	movzx	ebx,	byte [edx+ebx+512]  	; 1 5bit dither table
	movd	mm1,	eax		; 0
	movd	mm4,	ebx		; 1
	movd	eax,	mm0		; 0
	movd	ebx,	mm3		; 1
	shr	eax,	8		; 0
	psllq	mm4,	16		; 1
	shr	ebx,	8		; 1
	and	eax,	0ffh		; 0 g
	and	ebx,	0ffh		; 1 g
	movzx	eax,	byte [edx+eax+256]		; 0 6bit dither table
	movzx	ebx,	byte [edx+ebx+256+512] 	; 1 6bit dither table
	movd	mm2,	eax		; 0
	movd	mm5,	ebx		; 1
	psllq	mm2,	5		; 0
	movd	eax,	mm0		; 0
	psllq	mm5,	21		; 1
	por	mm1,	mm2		; 0 gb
	shr	eax,	16		; 0
	movd	ebx,	mm3		; 1
	por	mm4,	mm5		; 1 gb
	shr	ebx,	16		; 1
	and	eax,	0ffh		; 0 r
	and	ebx,	0ffh		; 1 r
	movzx	eax,	byte [edx+eax]		; 0 5bit dither table
	movzx	ebx,	byte [edx+ebx+512]		; 1 5bit dither table
	movd	mm2,	eax		; 0
	movd	mm5,	ebx		; 1
	psllq	mm2,	11		; 0
	movq	mm0,	[ebp+8]		; 2nd a1 r1 g1 b1 a0 r0 g0 b0
	psllq	mm5,	27		; 1


	por	mm1,	mm2		; 0 16(rgb)

	por	mm4,	mm5		; 1 16(rgb)

	movq	mm3,	mm0		; 2nd 

	por	mm1,	mm4		; 16(rgb) 16(rgb)

	psrlq	mm3,	32		; 2nd 00 00 00 00 a1 r1 g1 b1

		; 2nd -- 

	movd	eax,	mm0		; 0 a0 r0 g0 b0
	movd	ebx,	mm3		; 1 a1 r1 g1 b1
	and	eax,	0ffh		; 0 b
	and	ebx,	0ffh		; 1 b
	movzx	eax,	byte [edx+eax+1024]		; 0 5bit dither table
	movzx	ebx,	byte [edx+ebx+512+1024]  	; 1 5bit dither table
	movd	mm6,	eax		; 0
	movd	mm4,	ebx		; 1
	psllq	mm4,	48		; 1
	movd	ebx,	mm3		; 1
	movd	eax,	mm0		; 0
	shr	ebx,	8		; 1
	psllq	mm6,	32		; 0
	shr	eax,	8		; 0
	and	ebx,	0ffh		; 1 g
	and	eax,	0ffh		; 0 g
	movzx	ebx,	byte [edx+ebx+256+512+1024] 	; 1 6bit dither table
	movzx	eax,	byte [edx+eax+256+1024]		; 0 6bit dither table
	movd	mm5,	ebx		; 1
	movd	mm2,	eax		; 0
	psllq	mm5,	53		; 1
	movd	eax,	mm0		; 0
	psllq	mm2,	37		; 0
	movd	ebx,	mm3		; 1
	por	mm6,	mm2		; 0 gb
	shr	eax,	16		; 0
	por	mm4,	mm5		; 1 gb
	shr	ebx,	16		; 1
	and	eax,	0ffh		; 0 r
	and	ebx,	0ffh		; 1 r
	movzx	eax,	byte [edx+eax+1024]		; 0 5bit dither table
	movzx	ebx,	byte [edx+ebx+512+1024]		; 1 5bit dither table
	movd	mm2,	eax		; 0
	movd	mm5,	ebx		; 1

	psllq	mm2,	43		; 0
	psllq	mm5,	59		; 1
	por	mm6,	mm2		; 0 16(rgb)
	por	mm4,	mm5		; 1 16(rgb)

	add	ebp,	byte 16

	por	mm6,	mm4		; 16(rgb) 16(rgb)

	add	edi,	byte 8

	por	mm1,	mm6		; 16(rgb) 16(rgb) 16(rgb) 16(rgb)

	cmp	ebp,	esi

	movq	[edi-8],mm1		; store

	jb	near .ploop		; jump if ebp < esi

	sub	edx,	ecx

.pfraction:
	add	esi,	byte 12
	cmp	ebp,	esi
	jae	.pexit		; jump if ebp >= esi

.ploop2:
	add	edx,	ecx
	movzx	eax,	byte [ebp+2]		; r
	movzx	ebx,	byte [eax+edx]		; 5bit dither table
	shl	ebx,	11
	movzx	eax,	byte [ebp+1]		; g
	movzx	eax,	byte [eax+edx+256]		; 6bit dither table
	shl	eax,	5
	or	ebx,	eax
	movzx	eax,	byte [ebp]		; b
	movzx	eax,	byte [eax+edx]		; 5bit dither table
	or	ebx,	eax
	mov	[edi],	bx		; store
	add	ebp,	byte 4
	add	edi,	byte 2
	sub	edx,	ecx
	add	ecx,	0200h		; ecx += 0x200
	and	ecx,	0600h		; ecx &= 0x600

	cmp	ebp,	esi
	jb	.ploop2		; jump if ebp < esi

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

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPDither32BitTo16Bit555
;;void, TVPDither32BitTo16Bit555_mmx_a, (tjs_uint16 *dest, const tjs_uint32 *src, tjs_int len, tjs_int xofs, tjs_int yofs)

	function_align
TVPDither32BitTo16Bit555_mmx_a:					; 32bpp (A)RGB -> 16bpp with ordered dithering
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
	lea	esi,	[ebp + ecx *4]		; esi = limit
	mov	eax,	[esp + 44]		; yofs
	and	eax,	3
	shl	eax,	11		; eax *= 4*2*256
	lea	edx,	[TVPDitherTable_5_6+eax] 	; line
	mov	ecx,	[esp + 40]		; xofs
	and	ecx,	3
	shl	ecx,	9		; ecx *= 2*256

	sub	esi,	byte 12		; 12 = 3*4
	cmp	ebp,	esi
	jae	near .pfraction		; jump if ebp >= esi

	add	edx,	ecx		; edx = offset to table

	loop_align
.ploop:

	movq	mm0,	[ebp]		; a1 r1 g1 b1 a0 r0 g0 b0
	movq	mm3,	mm0
	psrlq	mm3,	32		; 00 00 00 00 a1 r1 g1 b1

	movd	eax,	mm0		; 0 a0 r0 g0 b0
	movd	ebx,	mm3		; 1 a1 r1 g1 b1
	and	eax,	0ffh		; 0 b
	and	ebx,	0ffh		; 1 b
	movzx	eax,	byte [edx+eax]		; 0 5bit dither table
	movzx	ebx,	byte [edx+ebx+512]  	; 1 5bit dither table
	movd	mm1,	eax		; 0
	movd	mm4,	ebx		; 1
	movd	eax,	mm0		; 0
	psllq	mm4,	16		; 1
	shr	eax,	8		; 0
	movd	ebx,	mm3		; 1
	shr	ebx,	8		; 1
	and	eax,	0ffh		; 0 g
	and	ebx,	0ffh		; 1 g
	movzx	eax,	byte [edx+eax]		; 0 5bit dither table
	movzx	ebx,	byte [edx+ebx+512]		; 1 5bit dither table
	movd	mm2,	eax		; 0
	movd	mm5,	ebx		; 1
	psllq	mm5,	21		; 1
	por	mm4,	mm5		; 1 gb
	movd	eax,	mm0		; 0
	psllq	mm2,	5		; 0
	movd	ebx,	mm3		; 1
	shr	eax,	16		; 0
	por	mm1,	mm2		; 0 gb
	shr	ebx,	16		; 1
	and	eax,	0ffh		; 0 r
	and	ebx,	0ffh		; 1 r
	movzx	eax,	byte [edx+eax]		; 0 5bit dither table
	movzx	ebx,	byte [edx+ebx+512]		; 1 5bit dither table
	movd	mm2,	eax		; 0
	movd	mm5,	ebx		; 1
	psllq	mm2,	10		; 0
	movq	mm0,	[ebp+8]		; 2nd a1 r1 g1 b1 a0 r0 g0 b0
	psllq	mm5,	26		; 1


	por	mm1,	mm2		; 0 15(rgb)

	por	mm4,	mm5		; 1 15(rgb)

	movq	mm3,	mm0		; 2nd 

	por	mm1,	mm4		; 15(rgb) 15(rgb)

	psrlq	mm3,	32		; 2nd 00 00 00 00 a1 r1 g1 b1

		; 2nd -- 

	movd	eax,	mm0		; 0 a0 r0 g0 b0
	movd	ebx,	mm3		; 1 a1 r1 g1 b1
	and	eax,	0ffh		; 0 b
	and	ebx,	0ffh		; 1 b
	movzx	eax,	byte [edx+eax+1024]		; 0 5bit dither table
	movzx	ebx,	byte [edx+ebx+512+1024]  	; 1 5bit dither table
	movd	mm6,	eax		; 0
	movd	mm4,	ebx		; 1
	psllq	mm6,	32		; 0
	movd	eax,	mm0		; 0
	movd	ebx,	mm3		; 1
	shr	eax,	8		; 0
	psllq	mm4,	48		; 1
	shr	ebx,	8		; 1
	and	eax,	0ffh		; 0 g
	and	ebx,	0ffh		; 1 g
	movzx	eax,	byte [edx+eax+1024]		; 0 5bit dither table
	movzx	ebx,	byte [edx+ebx+512+1024] 	; 1 5bit dither table
	movd	mm2,	eax		; 0
	movd	mm5,	ebx		; 1
	psllq	mm2,	37		; 0
	psllq	mm5,	53		; 1
	por	mm6,	mm2		; 0 gb
	movd	eax,	mm0		; 0
	movd	ebx,	mm3		; 1
	shr	eax,	16		; 0
	por	mm4,	mm5		; 1 gb
	shr	ebx,	16		; 1
	and	eax,	0ffh		; 0 r
	and	ebx,	0ffh		; 1 r
	movzx	eax,	byte [edx+eax+1024]		; 0 5bit dither table
	movzx	ebx,	byte [edx+ebx+512+1024]		; 1 5bit dither table
	movd	mm2,	eax		; 0
	movd	mm5,	ebx		; 1

	psllq	mm2,	42		; 0
	psllq	mm5,	58		; 1
	por	mm6,	mm2		; 0 15(rgb)
	por	mm4,	mm5		; 1 15(rgb)

	add	ebp,	byte 16

	por	mm6,	mm4		; 15(rgb) 15(rgb)

	add	edi,	byte 8

	por	mm1,	mm6		; 15(rgb) 15(rgb) 15(rgb) 15(rgb)

	cmp	ebp,	esi

	movq	[edi-8],mm1		; store

	jb	near .ploop		; jump if ebp < esi

	sub	edx,	ecx

.pfraction:
	add	esi,	byte 12
	cmp	ebp,	esi
	jae	.pexit		; jump if ebp >= esi

.ploop2:
	add	edx,	ecx
	movzx	eax,	byte [ebp+2]		; r
	movzx	ebx,	byte [eax+edx]		; 5bit dither table
	shl	ebx,	10
	movzx	eax,	byte [ebp+1]		; g
	movzx	eax,	byte [eax+edx]		; 5bit dither table
	shl	eax,	5
	or	ebx,	eax
	movzx	eax,	byte [ebp]		; b
	movzx	eax,	byte [eax+edx]		; 5bit dither table
	or	ebx,	eax
	mov	[edi],	bx		; store
	add	ebp,	byte 4
	add	edi,	byte 2
	sub	edx,	ecx
	add	ecx,	0200h		; ecx += 0x200
	and	ecx,	0600h		; ecx &= 0x600

	cmp	ebp,	esi
	jb	.ploop2		; jump if ebp < esi

.pexit:
	pop	ebp
	pop	edx
	pop	ecx
	pop	ebx
	pop	esi
	pop	edi
	emms
	ret


