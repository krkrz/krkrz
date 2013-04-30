; -*- Mode: asm; tab-width: 12; -*-
; this is a part of TVP (KIRIKIRI) software source.
; see other sources for license.
; (C)2001-2009 W.Dee <dee@kikyou.info> and contributors


%include		"nasm.nah"


;;;; THIS IS A TEST ROUTINE!!!!(not so optimized)

globaldef		TVPMakeAlphaFromKey_cmovcc_a

	segment_code
;--------------------------------------------------------------------

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_CMOV] TVPMakeAlphaFromKey
;;void, TVPMakeAlphaFromKey_cmovcc_a, (tjs_uint32 *dest, tjs_int len, tjs_uint32 key)

	function_align
TVPMakeAlphaFromKey_cmovcc_a:			; make alpha from key
	push	edi
	push	esi
	push	ebx
	push	ecx
	push	edx
	mov	edi,	[esp + 24]		; dest
	mov	ecx,	[esp + 28]		; len
	mov	edx,	[esp + 32]		; key
	and	edx,	000ffffffh		; mask alpha
	or	edx,	0ff000000h
	xor	esi,	esi
	dec	ecx
	jmp	.loopstart

	loop_align
.loop:
	mov	eax, [edi]		; dest
	and	eax, 000ffffffh		; mask alpha
	mov	ebx, eax
	add	edi, byte 4
	or	eax, 0ff000000h		; full opacity
	cmp	eax, edx		; eax == key?
	cmove	eax, ebx		; eax = 0 if eax == key
	dec	ecx
	mov	[edi-4], eax		; store

.loopstart:
	jns	.loop

	pop	edx
	pop	ecx
	pop	ebx
	pop	esi
	pop	edi
	ret

;--------------------------------------------------------------------
