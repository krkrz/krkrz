; -*- Mode: asm; tab-width: 12; -*-
; this is a part of TVP (KIRIKIRI) software source.
; see other sources for license.
; (C)2001-2009 W.Dee <dee@kikyou.info> and contributors

; constant color alpha mapping

%ifndef		GEN_CODE

%include		"nasm.nah"


globaldef		TVPApplyColorMap65_mmx_a
globaldef		TVPApplyColorMap65_emmx_a
globaldef		TVPApplyColorMap65_d_mmx_a
globaldef		TVPApplyColorMap65_d_emmx_a
globaldef		TVPApplyColorMap65_a_mmx_a
globaldef		TVPApplyColorMap65_a_emmx_a
externdef		TVPNegativeMulTable65
externdef		TVPOpacityOnOpacityTable65

%define		GEN_CODE


;--------------------------------------------------------------------
; MMX stuff
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPApplyColorMap65
;;void, TVPApplyColorMap65_mmx_a, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color)
%define TVPApplyColorMap65_name TVPApplyColorMap65_mmx_a
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPApplyColorMap65_d
;;void, TVPApplyColorMap65_d_mmx_a, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color)
%define TVPApplyColorMap65_d_name TVPApplyColorMap65_d_mmx_a
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPApplyColorMap65_a
;;void, TVPApplyColorMap65_a_mmx_a, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color)
%define TVPApplyColorMap65_a_name TVPApplyColorMap65_a_mmx_a
;--------------------------------------------------------------------
	%include "colormap.nas"
;--------------------------------------------------------------------

;--------------------------------------------------------------------
; EMMX stuff
;--------------------------------------------------------------------
%define USE_EMMX
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPApplyColorMap65
;;void, TVPApplyColorMap65_emmx_a, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color)
%define TVPApplyColorMap65_name TVPApplyColorMap65_emmx_a
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPApplyColorMap65_d
;;void, TVPApplyColorMap65_d_emmx_a, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color)
%define TVPApplyColorMap65_d_name TVPApplyColorMap65_d_emmx_a
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_EMMX && TVPCPUType & TVP_CPU_HAS_MMX] TVPApplyColorMap65_a
;;void, TVPApplyColorMap65_a_emmx_a, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color)
%define TVPApplyColorMap65_a_name TVPApplyColorMap65_a_emmx_a
;--------------------------------------------------------------------
	%include "colormap.nas"
;--------------------------------------------------------------------


%else

;--------------------------------------------------------------------
	segment_code
;--------------------------------------------------------------------
	function_align
TVPApplyColorMap65_name:
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 36]		; len
	cmp	ecx,	byte 0
	jle	near .pexit		; jump if ecx <= 0 
	mov	edi,	[esp + 28]		; dest
	mov	ebp,	[esp + 32]		; src
	pxor	mm0,	mm0		; mm0 = 0
	mov	esi,	[esp + 40]		; color
	movd	mm7,	esi		; color
	lea	edx,	[edi + ecx*4]		; limit
	sub	edx,	byte 4		; 1*4
	punpcklbw	mm7,	mm0		; unpack
	cmp	edi,	edx
	jae	near .pfraction		; jump if edi >= edx
	jmp	.ploop


.pcopa:
	mov	[edi], esi		; color
	add	ebp, byte 2
	mov	[edi + 4],	esi
	add	edi, byte 8
	cmp	edi,	edx
	jb	.ploop		; jump if edi < edx
	jmp	.pfraction

.pctransp:
	add	ebp, byte 2
	add	edi, byte 8
	cmp	edi,	edx
	jb	.ploop		; jump if edi < edx
	jmp	.pfraction

	loop_align
.ploop:
	movzx	eax,	word [ebp]		; opacity
%ifdef	USE_EMMX
	prefetcht0	[ebp + 8]
%endif
	cmp	eax,	0ffffh
	je	.pcopa		; completely opaque
	cmp	eax,	byte 0
	je	.pctransp		; completely transparent

	movd	mm1,	[edi]		; 1 dest
	movd	mm4,	[edi+4]		; 2 dest
	punpcklbw	mm1,	mm0		; 1 unpack
	mov	ebx,	eax
	punpcklbw	mm4,	mm0		; 2 unpack
	and	eax,	0ffh
	shr	ebx,	8
	movd	mm3,	eax		; 1 opacity
	movd	mm6,	ebx		; 2 opacity
	punpcklwd	mm3,	mm3		; 1 unpack
	punpcklwd	mm6,	mm6		; 2 unpack
	movq	mm2,	mm7		; 1
	punpcklwd	mm3,	mm3		; 1 unpack
	movq	mm5,	mm7		; 2
	punpcklwd	mm6,	mm6		; 2 unpack
	psubw	mm2,	mm1		; 1
	psubw	mm5,	mm4		; 2
	pmullw	mm2,	mm3		; 1 mm2 *= mm3
	add	edi,	byte 8
	add	ebp,	byte 2
	pmullw	mm5,	mm6		; 2 mm5 *= mm6
	psraw	mm2,	6		; 1 mm2 >>= 6
	psraw	mm5,	6		; 2 mm5 >>= 6
	paddw	mm1,	mm2		; 1 mm1 += mm2
	paddw	mm4,	mm5		; 2 mm4 += mm5
	packuswb	mm1,	mm0		; 1 pack
	packuswb	mm4,	mm0		; 2 pack
	movd	[edi-8], mm1		; 1 store
	cmp	edi,	edx
	movd	[edi+4-8], mm4		; 2 store

	jb	.ploop		; jump if edi < edx

.pfraction:
	add	edx,	byte 4
	cmp	edi,	edx
	jae	.pexit		; jump if edi >= edx

.ploop2:	; fractions

	movzx	eax,	byte [ebp]		; opacity
	movd	mm4,	eax
	punpcklwd	mm4,	mm4		; unpack
	punpcklwd	mm4,	mm4		; unpack
	movd	mm1,	[edi]		; dest
	punpcklbw	mm1,	mm0		; unpack
	movq	mm2,	mm7
	psubw	mm2,	mm1
	pmullw	mm2,	mm4		; mm2 *= mm4
	psraw	mm2,	6		; mm2 >>= 6
	paddw	mm1,	mm2		; mm1 += mm2
	packuswb	mm1,	mm0		; pack
	movd	[edi], mm1		; store
	add	edi,	byte 4
	inc	ebp

	cmp	edi,	edx
	jb	.ploop2		; jump if edi < edx

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
TVPApplyColorMap65_d_name:
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 36]		; len
	cmp	ecx,	byte 0
	jle	near .pexit		; jump if ecx <= 0
	mov	edi,	[esp + 28]		; dest
	mov	ebp,	[esp + 32]		; src
	lea	esi,	[edi + ecx *4]		; limit
	pxor	mm0,	mm0		; mm0 = 0
	mov	eax,	[esp + 40]		; color
	and	eax,	000ffffffh		; transparent
	or	dword [esp+40],	0ff000000h		; opaque
	movd	mm7,	eax
	punpcklbw	mm7,	mm0		; unpack
	sub	esi,	byte 4		; 1*4
	cmp	edi,	esi
	jae	near .pfraction		; jump if edi >= esi
	jmp .ploop

.pcopa:
	mov	eax,	[esp + 40]		; color
	add	ebp,	byte 2
	mov	[edi],	eax
	mov	[edi+4],eax
	add	edi,	byte 8
	cmp	edi,	esi
	jb	.ploop		; jump if edi < esi
	jmp	.pfraction

.pctransp:
	add	ebp, byte 2
	add	edi, byte 8
	cmp	edi,	esi
	jb	.ploop		; jump if edi < esi
	jmp	.pfraction

	loop_align
.ploop:
	movzx	eax,	word [ebp]		; opacity
%ifdef	USE_EMMX
	prefetcht0	[ebp + 8]
%endif
	cmp	eax,	0ffffh
	je	.pcopa		; completely opaque
	cmp	eax,	byte 0
	je	.pctransp		; completely transparent

	mov	ecx,	eax
	mov	ebx,	[edi]		; 1 dest
	and	eax,	0ffh
	mov	edx,	[edi+4]		; 2 dest
	and	ecx,	0ff00h
	shl	eax,	8		; 1
	movq	mm2,	mm7		; 1 color
	movq	mm5,	mm7		; 2 color
	movd	mm1,	ebx		; 1 dest
	movd	mm4,	edx		; 2 dest
	psllq	mm1,	40		; 1
	shr	ebx,	24		; 1
	psllq	mm4,	40		; 2
	shr	edx,	24		; 2
	add	eax,	ebx		; 1
	add	ecx,	edx		; 2
	movzx	ebx,	byte [TVPNegativeMulTable65 + eax]		; 1 dest opa table
	psrlq	mm1,	40		; 1 mask
	movzx	edx,	byte [TVPNegativeMulTable65 + ecx]		; 2 dest opa table
	psrlq	mm4,	40		; 2 mask
	movzx	eax,	byte [TVPOpacityOnOpacityTable65 + eax]		; 1 blend opa table
	punpcklbw	mm1,	mm0		; 1 unpack
	movzx	ecx,	byte [TVPOpacityOnOpacityTable65 + ecx]		; 2 blend opa table
	punpcklbw	mm4,	mm0		; 2 unpack
	movd	mm3,	eax		; 1
	movd	mm6,	ecx		; 2
	punpcklwd	mm3,	mm3		; 1 unpack
	punpcklwd	mm6,	mm6		; 2 unpack
	psubw	mm2,	mm1		; 1 mm2 -= mm1
	punpcklwd	mm3,	mm3		; 1 unpack
	psubw	mm5,	mm4		; 2 mm5 -= mm
	punpcklwd	mm6,	mm6		; 2 unpack
	pmullw	mm2,	mm3		; 1 mm2 *= mm3
	shl	ebx,	24		; 1
	psllw	mm1,	8
	psllw	mm4,	8
	add	edi,	byte 8
	shl	edx,	24		; 2
	pmullw	mm5,	mm6		; 2 mm5 *= mm6
	add	ebp,	byte 2
	paddw	mm1,	mm2		; 1 mm1 += mm2
	paddw	mm4,	mm5		; 2 mm4 += mm5
	psrlw	mm1,	8
	psrlw	mm4,	8
	packuswb	mm1,	mm0		; 1 pack
	packuswb	mm4,	mm0		; 2 pack
	movd	eax,	mm1		; 1
	movd	ecx,	mm4		; 2
	or	eax,	ebx		; 1
	or	ecx,	edx		; 2
	mov	[edi-8],eax		; store
	cmp	edi,	esi
	mov	[edi+4-8],ecx		; 2 store

	jb	near .ploop		; jump if edi < esi

.pfraction:
	add	esi,	byte 4
	cmp	edi,	esi
	jae	.pexit		; jump if edi >= esi

.ploop2:	; fraction
	movzx	eax,	byte [ebp]		; opacity
	mov	ebx,	[edi]		; dest
	movq	mm2,	mm7		; color
	movd	mm1,	ebx		; dest
	psllq	mm1,	40
	psrlq	mm1,	40		; mask
	shl	eax,	8
	shr	ebx,	24
	add	eax,	ebx
	punpcklbw	mm1,	mm0		; unpack
	mov	bl,	byte [TVPNegativeMulTable65 + eax]		; dest opa table
	movzx	eax,	byte [TVPOpacityOnOpacityTable65 + eax]		; blend opa table
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
	movd	eax,	mm1
	or	eax,	ebx
	mov	[edi],	eax		; store
	add	ebp,	byte 1
	add	edi,	byte 4

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
TVPApplyColorMap65_a_name:
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ecx,	[esp + 36]		; len
	cmp	ecx,	byte 0
	jle	near .pexit		; jump if ecx <= 0
	mov	edi,	[esp + 28]		; dest
	mov	ebp,	[esp + 32]		; src
	lea	esi,	[edi + ecx *4]		; limit
	pxor	mm0,	mm0		; mm0 = 0
	mov	eax,	[esp + 40]		; color   (co)
	and	eax,	000ffffffh		; transparent
	movd	mm7,	eax
	mov	eax,	0x100
	movd	mm1,	eax
	psllq	mm1,	48
	punpcklbw	mm7,	mm0
	por	mm7,	mm1		; mm7 = 01 00 00 co 00 co 00 co
	sub	esi,	byte 8		; 2*4
	cmp	edi,	esi
	jae	near .pfraction		; jump if edi >= esi


	test	edi,	4
	IF	nz

			; align destination pointer to QWORD

		movzx	eax,	byte [ebp]		; opacity   (Sa)
		movd	mm3,	eax
		punpcklwd	mm3,	mm3
		punpcklwd	mm3,	mm3		; mm3 = 00 Sa 00 Sa 00 Sa 00 Sa
		movq	mm4,	mm3
		pmullw	mm4,	mm7
		psrlw	mm4,	6		; mm4 = 00 Sa 00 Si 00 Si 00 Si
		movd	mm1,	[edi]		; dest     (DaDiDiDi)
		punpcklbw	mm1,	mm0		; mm1 = 00 Da 00 Di 00 Di 00 Di
		movq	mm2,	mm1
		pmullw	mm2,	mm3
		psrlw	mm2,	6		; mm2 = 00 SaDa 00 SaDi 00 SaDi 00 SaDi
		psubw	mm1,	mm2
		paddw	mm1,	mm4
		packuswb	mm1,	mm0
		movd	[edi],	mm1		; store

		add	ebp,	byte 1
		add	edi,	byte 4

		cmp	edi,	esi
		jae	near	.pfraction

	ENDIF


	jmp .ploop

.pcopa:
	mov	eax,	[esp + 40]		; color
	add	ebp,	byte 2
	mov	[edi],	eax
	mov	[edi+4],eax
	add	edi,	byte 8
	cmp	edi,	esi
	jb	.ploop		; jump if edi < esi
	jmp	.pfraction

.pctransp:
	add	ebp, byte 2
	add	edi, byte 8
	cmp	edi,	esi
	jb	.ploop		; jump if edi < esi
	jmp	.pfraction

	loop_align
.ploop:
	movzx	eax,	word [ebp]		; opacity
%ifdef	USE_EMMX
	prefetcht0	[ebp + 8]
%endif
	cmp	eax,	0ffffh
	je	.pcopa		; completely opaque
	cmp	eax,	byte 0
	je	.pctransp		; completely transparent

	mov	ebx,	eax		; 1
	and	eax,	0xff		; 1
	shr	ebx,	8		; 2
	movd	mm3,	eax		; 1
	movd	mm5,	ebx		; 2
	punpcklwd	mm3,	mm3		; 1
	punpcklwd	mm5,	mm5		; 2
	punpcklwd	mm3,	mm3		; 1 mm3 = 00 Sa 00 Sa 00 Sa 00 Sa
	punpcklwd	mm5,	mm5		; 2 mm5 = 00 Sa 00 Sa 00 Sa 00 Sa
	movq	mm4,	mm3		; 1
	movq	mm6,	mm5		; 2
	pmullw	mm4,	mm7		; 1
	pmullw	mm6,	mm7		; 2
	psrlw	mm4,	6		; 1 mm4 = 00 Sa 00 Si 00 Si 00 Si
	psrlw	mm6,	6		; 2 mm6 = 00 Sa 00 Si 00 Si 00 Si
	movd	mm1,	[edi]		; 1 dest     (DaDiDiDi)
	punpcklbw	mm1,	mm0		; 1 mm1 = 00 Da 00 Di 00 Di 00 Di
	movq	mm2,	mm1		; 1
	pmullw	mm2,	mm3		; 1
	paddw	mm1,	mm4		; 1
	psrlw	mm2,	6		; 1 mm2 = 00 SaDa 00 SaDi 00 SaDi 00 SaDi
	psubw	mm1,	mm2		; 1
	movd	mm3,	[edi+4]		; 2 dest     (DaDiDiDi)
;		packuswb	mm1,		mm0			; 1
	punpcklbw	mm3,	mm0		; 2 mm3 = 00 Da 00 Di 00 Di 00 Di
;		movd		[edi],		mm1			; 1 store
	movq	mm2,	mm3		; 2
	pmullw	mm2,	mm5		; 2
	add	edi,	byte 8
	psrlw	mm2,	6		; 2 mm2 = 00 SaDa 00 SaDi 00 SaDi 00 SaDi
	add	ebp,	byte 2
	psubw	mm3,	mm2		; 2
	paddw	mm3,	mm6		; 2
	cmp	edi,	esi
	packuswb	mm1,	mm3		; 2
	movq	[edi-8],	mm1		; 2 store

	jb	near .ploop		; jump if edi < esi

.pfraction:
	add	esi,	byte 8
	cmp	edi,	esi
	jae	.pexit		; jump if edi >= esi

.ploop2:	; fraction
	movzx	eax,	byte [ebp]		; opacity   (Sa)
	movd	mm3,	eax
	punpcklwd	mm3,	mm3
	punpcklwd	mm3,	mm3		; mm3 = 00 Sa 00 Sa 00 Sa 00 Sa
	movq	mm4,	mm3
	pmullw	mm4,	mm7
	psrlw	mm4,	6		; mm4 = 00 Sa 00 Si 00 Si 00 Si
	movd	mm1,	[edi]		; dest     (DaDiDiDi)
	punpcklbw	mm1,	mm0		; mm1 = 00 Da 00 Di 00 Di 00 Di
	movq	mm2,	mm1
	pmullw	mm2,	mm3
	psrlw	mm2,	6		; mm2 = 00 SaDa 00 SaDi 00 SaDi 00 SaDi
	psubw	mm1,	mm2
	paddw	mm1,	mm4
	packuswb	mm1,	mm0
	movd	[edi],	mm1		; store

	add	ebp,	byte 1
	add	edi,	byte 4

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


%endif
