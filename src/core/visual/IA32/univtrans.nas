; -*- Mode: asm; tab-width: 12; -*-
; this is a part of TVP (KIRIKIRI) software source.
; see other sources for license.
; (C)2001-2009 W.Dee <dee@kikyou.info> and contributors

; universal transition blender

%include		"nasm.nah"

; basic algorithm
;  opacity = table[*rule]
;  dest = blend(src1, src2, opacity)
; where blend() is:
;   return src1 * opacity + src2 * (1-opacity)

;--------------------------------------------------------------------

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPInitUnivTransBlendTable
;;void, TVPInitUnivTransBlendTable_mmx_c, (tjs_uint32 *table, tjs_int phase, tjs_int vague)

;;[emit_c_c]TVP_GL_IA32_FUNC_DECL(void, TVPInitUnivTransBlendTable_mmx_c, (tjs_uint32 *table, tjs_int phase, tjs_int vague))
;;[emit_c_c]{
;;[emit_c_c]	tjs_uint16 *t = (tjs_uint16*)table;
;;[emit_c_c]	tjs_int i;
;;[emit_c_c]	tjs_int phasemax = phase + vague;
;;[emit_c_c]	phase -= vague;
;;[emit_c_c]	for(i = 0; i<256; i++)
;;[emit_c_c]	{
;;[emit_c_c]		if(i<phase)  t[i] = 0<<7;
;;[emit_c_c]		else if(i>=phasemax) t[i] = 255<<7;
;;[emit_c_c]		else
;;[emit_c_c]		{
;;[emit_c_c]			int tmp = ( i - phase )*255 / vague;
;;[emit_c_c]			if(tmp<0) tmp = 0;
;;[emit_c_c]			if(tmp>255) tmp = 255;
;;[emit_c_c]			t[i] = tmp << 7;
;;[emit_c_c]		}
;;[emit_c_c]	}
;;[emit_c_c]}



globaldef		TVPUnivTransBlend_mmx_a
globaldef		TVPUnivTransBlend_emmx_a
globaldef		TVPUnivTransBlend_switch_mmx_a
globaldef		TVPUnivTransBlend_switch_emmx_a

	segment_code
;--------------------------------------------------------------------
%imacro		TVPUnivTransBlend_proto 3
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	mov	esi,	[esp + 44]		; len
	cmp	esi,	byte 0
	jle	near	.pexit2
	mov	edi,	[esp + 24]		; dest
	mov	ecx,	[esp + 28]		; src1
	mov	ebx,	[esp + 32]		; src2
	mov	eax,	[esp + 40]		; table
	pxor	mm0,	mm0		; mm0 = 0
	push	ebp
	mov	ebp,	[esp + 40]		; rule
	lea	esi,	[edi+esi*4]		; limit
	sub	esi,	byte 4
	cmp	edi,	esi
	jae	near	.pfraction		; jump if edi >= esi

	loop_align
.ploop:
	movzx	edx,	byte [ebp]		; 1 retrieve rule value
	%1
	movq	mm1,	[ebx]		; src2
	%2
	movq	mm2,	[ecx]		; src1
	%3
	movq	mm5,	mm1		; src2
	movq	mm6,	mm2		; src1
	punpcklbw	mm1,	mm0		; 1 unpack
	movd	mm3,	dword [eax + edx*2]		; 1 blend table
	punpckhbw	mm5,	mm0		; 2 unpack
	movzx	edx,	byte [ebp + 1]		; 2 retrieve rule value
	punpcklbw	mm2,	mm0		; 1 unpack
	movd	mm4,	dword [eax + edx*2]		; 2 blend table
	punpckhbw	mm6,	mm0		; 2 unpack
	punpcklwd	mm3,	mm3		; 1 unpack
	psubw	mm2,	mm1		; 1 mm2 -= mm1
	punpcklwd	mm3,	mm3		; 1 unpack
	add	ebp,	byte 2
	punpcklwd	mm4,	mm4		; 2 unpack
	psubw	mm6,	mm5		; 2 mm6 -= mm5
	punpcklwd	mm4,	mm4		; 2 unpack
	pmulhw	mm2,	mm3		; 1 mm2 *= mm3
	add	ecx,	byte 8
	add	edi,	byte 8
	pmulhw	mm6,	mm4		; 2 mm6 *= mm4
	add	ebx,	byte 8
	psllw	mm2,	1		; 1 mm2 <<= 1
	psllw	mm6,	1		; 2 mm6 <<= 1
	paddw	mm1,	mm2		; 1 mm1 += mm2
	paddw	mm5,	mm6		; 2 mm5 += mm6
	packuswb	mm1,	mm5		; 1 pack
	cmp	edi,	esi
	movq	[edi - 8], mm1		; 1 store

	jb	near .ploop		; jump if edi < esi

.pfraction:
	add	esi,	byte 4
	cmp	edi,	esi
	jae	.pexit		; jump if edi >= esi

.ploop2:
	movzx	edx,	byte [ebp]		; retrieve rule value
	movd	mm2,	dword [ecx]		; src1
	movd	mm4,	dword [eax + edx*2]		; blend table
	punpcklwd	mm4,	mm4		; unpack
	movd	mm1,	dword [ebx]		; src2
	punpcklbw	mm2,	mm0		; unpack
	punpcklbw	mm1,	mm0		; unpack
	punpcklwd	mm4,	mm4		; unpack
	psubw	mm2,	mm1		; mm2 -= mm1
	pmulhw	mm2,	mm4		; mm2 *= mm4
	psllw	mm2,	1		; mm2 <<= 1
	paddw	mm1,	mm2		; mm1 += mm2
	packuswb	mm1,	mm0		; pack
	movd	dword [edi], mm1		; store
	inc	ebp
	add	ecx,	byte 4
	add	edi,	byte 4
	add	ebx,	byte 4

	cmp	edi,	esi
	jb	.ploop2		; jump if edi < esi

.pexit:
	pop	ebp
.pexit2:
	pop	edx
	pop	ecx
	pop	ebx
	pop	esi
	pop	edi
	emms
	ret
%endmacro

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPUnivTransBlend
;;void, TVPUnivTransBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src1, const tjs_uint32 *src2, const tjs_uint8 *rule, const tjs_uint32 *table, tjs_int len)
	function_align
TVPUnivTransBlend_mmx_a:			; do universal transition blend
	TVPUnivTransBlend_proto	.dummy1:, .dummy2:, .dummy3:

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPUnivTransBlend
;;void, TVPUnivTransBlend_emmx_a, (tjs_uint32 *dest, const tjs_uint32 *src1, const tjs_uint32 *src2, const tjs_uint8 *rule, const tjs_uint32 *table, tjs_int len)
	function_align
TVPUnivTransBlend_emmx_a:			; do universal transition blend
	TVPUnivTransBlend_proto	prefetcht0	[ebx + 2], prefetcht0	[ebx + 16], prefetcht0	[ecx + 16]

;--------------------------------------------------------------------

%imacro		TVPUnivTransBlend_switch_proto 3
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	mov	esi,	[esp + 44]		; len
	cmp	esi,	byte 0
	jle	near	.pexit2
	mov	edi,	[esp + 24]		; dest
	mov	ecx,	[esp + 28]		; src1
	mov	ebx,	[esp + 32]		; src2
	mov	eax,	[esp + 40]		; table
	pxor	mm0,	mm0		; mm0 = 0
	push	ebp
	mov	ebp,	[esp + 40]		; rule
	lea	esi,	[edi+esi*4]		; limit
	sub	esi,	byte 4
	cmp	edi,	esi
	jae	near	.pfraction		; jump if edi >= esi
	jmp	.ploop

.psrc1lv1:
	movzx	edx,	byte [ebp]
	cmp	edx,	[esp + 52]		; src1lv
	jl	.pnormal
	movq	mm1,	[ecx]		; src1
	add	edi,	byte 8
	add	ebx,	byte 8
	add	ebp,	byte 2
	add	ecx,	byte 8
	movq	[edi-8], mm1
	cmp	edi,	esi
	jb	near .ploop		; jump if edi < esi
	jmp	.pfraction

.psrc2lv1:
	movzx	edx,	byte [ebp]
	cmp	edx,	[esp + 56]		; src2lv
	jge	.pnormal
	movq	mm1,	[ebx]		; src2
	add	edi,	byte 8
	add	ecx,	byte 8
	add	ebp,	byte 2
	add	ebx,	byte 8
	movq	[edi-8], mm1
	cmp	edi,	esi
	jb	near .ploop		; jump if edi < esi
	jmp	.pfraction

	loop_align
.ploop:
	movzx	edx,	byte [ebp  + 1]		; 2 retrieve rule value
	%1
	cmp	edx,	[esp + 52]		; src1lv
	jge	.psrc1lv1
	cmp	edx,	[esp + 56]		; src2lv
	jl	.psrc2lv1
	movzx	edx,	byte [ebp]		; 1 retrieve rule value

.pnormal:
	movd	mm1,	dword [ebx]		; 1 src2
	movd	mm3,	dword [eax + edx*2]		; 1 blend table
	punpcklbw	mm1,	mm0		; 1 unpack
	movd	mm2,	dword [ecx]		; 1 src1
	punpcklwd	mm3,	mm3		; 1 unpack
	movzx	edx,	byte [ebp + 1]		; 2 retrieve rule value
	punpcklbw	mm2,	mm0		; 1 unpack
	movd	mm4,	dword [eax + edx*2]		; 2 blend table
	movd	mm6,	dword [ecx + 4]		; 2 src1
	%2
	punpcklwd	mm4,	mm4		; 2 unpack
	movd	mm5,	dword [ebx + 4]		; 2 src2
	%3
	punpcklbw	mm6,	mm0		; 2 unpack
	punpcklbw	mm5,	mm0		; 2 unpack
	psubw	mm2,	mm1		; 1 mm2 -= mm1
	punpcklwd	mm3,	mm3		; 1 unpack
	psubw	mm6,	mm5		; 2 mm6 -= mm5
	punpcklwd	mm4,	mm4		; 2 unpack
	pmulhw	mm2,	mm3		; 1 mm2 *= mm3
	add	ebp,	byte 2
	add	ecx,	byte 8
	pmulhw	mm6,	mm4		; 2 mm6 *= mm4
	add	ebx,	byte 8
	psllw	mm2,	1		; 1 mm2 <<= 1
	add	edi,	byte 8
	psllw	mm6,	1		; 2 mm6 <<= 1
	paddw	mm1,	mm2		; 1 mm1 += mm2
	paddw	mm5,	mm6		; 2 mm5 += mm6
	packuswb	mm1,	mm5		; pack
	cmp	edi,	esi
	movq	[edi - 8], mm1		; store

	jb	near .ploop		; jump if edi < esi

.pfraction:
	add	esi,	byte 4
	cmp	edi,	esi
	jae	.pexit		; jump if edi >= esi

.ploop2:
	movzx	edx,	byte [ebp]		; retrieve rule value
	movd	mm2,	dword [ecx]		; src1
	movd	mm4,	dword [eax + edx*2]		; blend table
	punpcklwd	mm4,	mm4		; unpack
	movd	mm1,	dword [ebx]		; src2
	punpcklbw	mm2,	mm0		; unpack
	punpcklbw	mm1,	mm0		; unpack
	punpcklwd	mm4,	mm4		; unpack
	psubw	mm2,	mm1		; mm2 -= mm1
	pmulhw	mm2,	mm4		; mm2 *= mm4
	psllw	mm2,	1		; mm2 <<= 1
	paddw	mm1,	mm2		; mm1 += mm2
	packuswb	mm1,	mm0		; pack
	movd	dword [edi], mm1		; store
	inc	ebp
	add	ecx,	byte 4
	add	edi,	byte 4
	add	ebx,	byte 4

	cmp	edi,	esi
	jb	.ploop2		; jump if edi < esi

.pexit:
	pop	ebp
.pexit2:
	pop	edx
	pop	ecx
	pop	ebx
	pop	esi
	pop	edi
	emms
	ret
%endmacro

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPUnivTransBlend_switch
;;void, TVPUnivTransBlend_switch_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src1, const tjs_uint32 *src2, const tjs_uint8 *rule, const tjs_uint32 *table, tjs_int len, tjs_int src1lv, tjs_int src2lv)
	function_align
TVPUnivTransBlend_switch_mmx_a:			; do universal transition blend with switching
	TVPUnivTransBlend_switch_proto	.dummy1:, .dummy2:, .dummy3:

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPUnivTransBlend_switch
;;void, TVPUnivTransBlend_switch_emmx_a, (tjs_uint32 *dest, const tjs_uint32 *src1, const tjs_uint32 *src2, const tjs_uint8 *rule, const tjs_uint32 *table, tjs_int len, tjs_int src1lv, tjs_int src2lv)
	function_align
TVPUnivTransBlend_switch_emmx_a:			; do universal transition blend with switching
	TVPUnivTransBlend_switch_proto	prefetcht0	[ebp + 4], prefetcht0	[ecx + 16], prefetcht0	[ebx + 16]

