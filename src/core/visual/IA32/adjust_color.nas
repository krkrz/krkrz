; -*- Mode: asm; tab-width: 12; -*-
; this is a part of TVP (KIRIKIRI) software source.
; see other sources for license.
; (C)2001-2009 W.Dee <dee@kikyou.info> and contributors

; Color adjustment routines (like gamma adjustment)

%include		"nasm.nah"



externdef		TVPRecipTable256_16
globaldef		TVPAdjustGamma_a_mmx_a


;--------------------------------------------------------------------
	segment_code
;--------------------------------------------------------------------
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPAdjustGamma_a
;;void, TVPAdjustGamma_a_mmx_a, (tjs_uint32 *dest, tjs_int len, tTVPGLGammaAdjustTempData *temp)
;--------------------------------------------------------------------

	function_align
proc_start	TVPAdjustGamma_a_mmx_a
	proc_arg	4,	dest
	proc_arg	4,	len
	proc_arg	4,	param

	local_var	4,	step
	local_var	4,	phase

	end_local_vars


	mov	edi,	[%$dest]
	mov	ecx,	[%$len]
	lea	esi,	[ecx*4+edi]		; limit
	mov	ecx,	[%$param]
	cmp	edi,	esi
	jae	near .pexit

	pxor	mm0,	mm0		; mm0 = 00 00 00 00 00 00 00 00

	jmp	.ploop

.popaque: ; completely opaque pixel
	movzx	ebx,	al
	movzx	edx,	byte[ebx + ecx]		; look table B up  (BB')

	movzx	ebx,	ah
	movzx	ebx,	byte[ebx + ecx + 256]		; look table G up  (GG')

	shl	ebx,	8
	or	edx,	ebx

	shr	eax,	16
	and	eax,	0xff

	movzx	ebx,	byte[eax + ecx + 512]		; look table R up  (RR')

	shl	ebx,	16
	or	edx,	ebx
	or	edx,	0xff000000

	mov	[edi],	edx		; store

.ptransp: ; completely tranaparent pixel
	add	edi,	4
	cmp	edi,	esi
	jae	near .pexit

.ploop:													; main loop

	mov	eax,	[edi]		; load

	cmp	eax,	0xff000000
	jae	.popaque		; completely opaque
	or	eax,	eax
	jz	.ptransp		; completely transparent

	movd	mm2,	eax		; mm2 = dest
	mov	ebx,	eax

	shr	ebx,	24		; ebx = alpha          (AA)
	movzx	edx,	word[TVPRecipTable256_16+ebx*2]		; edx = 65536/opacity (rcp)

	punpcklbw	mm2,	mm0		; mm2 = 00 AA 00 RR 00 GG 00 BB

	and	eax,	0xff000000		; extract alpha

	movd	mm1,	edx
	movd	mm3,	ebx
	punpcklwd	mm1,	mm1
	punpcklwd	mm3,	mm3
	punpcklwd	mm1,	mm1		; mm1 = rcp rcp rcp rcp
	punpcklwd	mm3,	mm3		; mm3 = 00 AA 00 AA 00 AA 00 AA

	movd	mm6,	eax

	movq	mm4,	mm2
	pmullw	mm1,	mm2
	pcmpgtw	mm4,	mm3
	psrlw	mm1,	8		; mm1 = rcp*AA>>8 rcp*RR>>8 rcp*GG>>8 rcp*BB>>8
	psrlw	mm4,	8

	por	mm1,	mm4

	packuswb	mm1,	mm0		; mm1 = 00 00 00 00 rcp*AA>>8 rcp*RR>>8 rcp*GG>>8 rcp*BB>>8

	psubusb	mm2,	mm3		; mm2 = 0000 RR-AA GG-AA BB-AA (unsigned lower saturated)

	movd	eax,	mm1

	movzx	ebx,	al
	movzx	edx,	byte[ebx + ecx]		; look table B up  (BB')

	movzx	ebx,	ah
	movzx	ebx,	byte[ebx + ecx + 256]		; look table G up  (GG')

	shl	ebx,	8
	or	edx,	ebx

	shr	eax,	16
	and	eax,	0xff

	movzx	ebx,	byte[eax + ecx + 512]		; look table R up  (RR')

	movq	mm5,	mm3

	shl	ebx,	16
	psrlw	mm5,	7
	or	edx,	ebx

	paddw	mm3,	mm5		; adjust alpha

	movd	mm1,	edx		; mm1 = 00 00 00 00 00 RR' GG' BB'
	add	edi,	4
	punpcklbw	mm1,	mm0		; mm1 = 00 00 00 RR' 00 GG' 00 BB'

	pmullw	mm1,	mm3
	psrlw	mm1,	8		; mm1 = 0000 alpha*RR'>>8 alpha*GG'>>8 alpha*BB'>>8

	paddw	mm1,	mm2

	cmp	edi,	esi

	packuswb	mm1,	mm0

	por	mm1,	mm6		; mm1 = final result

	movd	[edi-4],	mm1

	jb	.ploop


.pexit:
	emms
proc_end

