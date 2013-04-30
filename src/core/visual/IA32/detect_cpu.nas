; -*- Mode: asm; tab-width: 12; -*-
; this is a part of TVP (KIRIKIRI) software source.
; see other sources for license.
; (C)2001-2009 W.Dee <dee@kikyou.info> and contributors

; CPU type detection routine
; mostly taken from GOGO-NO-CODA but totally rewritten.

%include		"nasm.nah"

%include		"cpu_types.nah"

globaldef		TVPCheckCPU
globaldef		TVPCPUFeatures
globaldef		TVPCPUID1_EAX
globaldef		TVPCPUID1_EBX
globaldef		TVPCPUVendor
globaldef		TVPCPUName
globaldef		TVPGetTSC
externdef		TVPCPUType

	segment_data

;;[emit_c_h] extern tjs_uint32 TVPCPUFeatures;
;;[emit_c_h] extern tjs_uint32 TVPCPUID1_EAX;
;;[emit_c_h] extern tjs_uint32 TVPCPUID1_EBX;
;;[emit_c_h] extern tjs_nchar TVPCPUVendor[16];
;;[emit_c_h] extern tjs_nchar TVPCPUName[52];

		align 4
TVPCPUFeatures	dd	0
TVPCPUID1_EAX	dd	0		; type/family/model/stepping
TVPCPUID1_EBX	dd	0		; brand ID etc.
%define			TVPCPUVendor_buf_len	16
TVPCPUVendor	times TVPCPUVendor_buf_len		db		0
%define 		TVPCPUName_buf_len		52
TVPCPUName		times TVPCPUName_buf_len		db		0

	segment_code

;--------------------------------------------------------------------

;;[function] tjs_uint32, TVPCheckCPU, ()

%imacro		set_feat_bit 2
	xor	eax,	eax
	test	edx,	%1
	setz	al
	dec	eax
	and	eax,	%2
	or	esi,	eax
%endmacro

%imacro		check_vendor 2
	xor	eax,	eax
	cmp	ecx,	%1
	setne	al
	dec	eax
	or	edx,	eax
	and	eax,	%2
	or	esi,	eax
%endmacro

	function_align
TVPCheckCPU:			; check CPU type and feature
		;	clear result area
	pushfd
	push	edi
	push	ecx
	xor	eax,	eax
	cld
	mov	[TVPCPUFeatures],	eax
	mov	[TVPCPUID1_EAX],	eax
	mov	[TVPCPUID1_EBX],	eax
	lea	edi,	[TVPCPUVendor]
	mov	ecx,	TVPCPUVendor_buf_len
	rep	stosb
	lea	edi,	[TVPCPUName]
	mov	ecx,	TVPCPUName_buf_len
	rep	stosb
	pop	ecx
	pop	edi
	popfd

		;	check FPU existence
	push	esi

	mov	eax, 1
	fninit
	fnstsw	ax
	cmp	al, 0		; al changed ?
	jne	.invalid_cpu		; system does not have fpu
	fnstcw	word [esp-4]
	mov	ax, [esp-4]
	and	ax, 103fh
	cmp	ax, 3fh
	jz	.have_fpu

.invalid_cpu:
		; system does not have FPU or too old cpu
	xor	esi, esi
	jmp	.exit

.have_fpu:
	xor	esi, esi
	or	esi, TVP_CPU_HAS_FPU

		; check i486 or later
	pushfd
	pushfd
	pop	eax
	or	eax, (1<<21)
	push	eax
	popfd
	pushfd
	pop	eax
	popfd
	test	eax, (1<<21)
	jnz	.chk_i586

		; check cyrix 486
	push	ebx
	xor	ax, ax
	sahf
	mov	ax, 5
	mov	bx, 2
	div	bl		; do an operation that does not change flags
	lahf
	pop	ebx
	cmp	ah, 2		; check for change in flags
	jne	.is_486_intel

		; cpu is cyrix 486
	or	esi, TVP_CPU_IS_CYRIX
	or	esi, 4		; family 4
	jmp	.exit

.is_486_intel:
	or	esi, TVP_CPU_IS_INTEL
	or	esi, 4		; family 4
	jmp	.exit

.chk_i586:
		; check for 586 or later
	push	ebx
	push	ecx
	push	edx

		; get vendor string
	xor	eax,	eax
	cpuid
	mov	[TVPCPUVendor+ 0],	ebx
	mov	[TVPCPUVendor+ 4],	edx
	mov	[TVPCPUVendor+ 8],	ecx

		; check Intel, AMD, CYRIX, IDT, etc
	xor	edx,	edx

	check_vendor	06c65746eh,	TVP_CPU_IS_INTEL		; "letn" 'GenuineIntel'
	check_vendor	0444d4163h,	TVP_CPU_IS_AMD		; "DMAc" 'AuthenticAMD'
	check_vendor	052455454h,	TVP_CPU_IS_AMD		; "RETT" 'AMD ISBETTER' (old AMD)
	check_vendor	064616574h,	TVP_CPU_IS_CYRIX		; "daet" 'CyrixInstead'
	check_vendor	0736c7561h,	TVP_CPU_IS_IDT		; "slua" 'CentaurHauls'
	check_vendor	06e657669h,	TVP_CPU_IS_NEXGEN		; "nevi" 'NexGenDriven'
	check_vendor	065736952h,	TVP_CPU_IS_RISE		; "esiR" 'RiseRiseRise'
	check_vendor	020434d55h,	TVP_CPU_IS_UMC		; " CMU" 'UMC UMC UMC '
	check_vendor	03638784dh,	TVP_CPU_IS_TRANSMETA		; "68xM" 'GenuineTMx86'

	or	edx,	edx
	jnz	.known_vendor
	or	esi,	TVP_CPU_IS_UNKNOWN
.known_vendor:


.detect_fam:
		; detect family, model, stepping
	mov	eax,	1
	cpuid
	mov	[TVPCPUID1_EAX],	eax
	mov	[TVPCPUID1_EBX],	ebx
	and	ah,	00fh
	cmp	ah,	4
	jne	.store_fam
	or	esi,	4		; family 4
	jmp	.586exit		; extended features cannot be used

.store_fam:
	movzx	eax, ah
	or	esi, eax

.chk_feat:
		; check for special features

	mov	eax,	080000000h
	cpuid
	cmp	eax,	080000001h		; check maximum query number
	jb	.check_basic_feat

	mov	eax,	080000001h		; for non-intel processors
	cpuid

	set_feat_bit	(1<<31),	TVP_CPU_HAS_3DN
	set_feat_bit	(1<<15),	TVP_CPU_HAS_CMOV
	set_feat_bit	(1<<30),	TVP_CPU_HAS_E3DN
	set_feat_bit	(1<<22),	TVP_CPU_HAS_EMMX

.check_basic_feat:
	mov	eax, 1		; for most processors
	cpuid

	set_feat_bit	(1<<23),	TVP_CPU_HAS_MMX
	set_feat_bit	(1<<15),	TVP_CPU_HAS_CMOV
	set_feat_bit	(1<<25),	(TVP_CPU_HAS_EMMX|TVP_CPU_HAS_SSE)
	set_feat_bit	(1<<26),	TVP_CPU_HAS_SSE2
	set_feat_bit	(1<< 4),	TVP_CPU_HAS_TSC

.get_cpu_string:
	mov	eax,	080000000h
	cpuid
	cmp	eax,	080000004h		; check maximum query number
	jb	.586exit

	mov	eax,	080000002h
	cpuid
	mov	[TVPCPUName+ 0],	eax
	mov	[TVPCPUName+ 4],	ebx
	mov	[TVPCPUName+ 8],	ecx
	mov	[TVPCPUName+12],	edx

	mov	eax,	080000003h
	cpuid
	mov	[TVPCPUName+16],	eax
	mov	[TVPCPUName+20],	ebx
	mov	[TVPCPUName+24],	ecx
	mov	[TVPCPUName+28],	edx

	mov	eax,	080000004h
	cpuid
	mov	[TVPCPUName+32],	eax
	mov	[TVPCPUName+36],	ebx
	mov	[TVPCPUName+40],	ecx
	mov	[TVPCPUName+44],	edx

.586exit:
	pop	edx
	pop	ecx
	pop	ebx
.exit:
	mov	eax, esi
	mov	[TVPCPUFeatures], eax
	pop	esi
	ret

;--------------------------------------------------------------------

;;[function] tjs_uint64, TVPGetTSC, ()

	function_align
TVPGetTSC:			; get TSC
	test	dword [TVPCPUType],	TVP_CPU_HAS_TSC
	jnz	.have_tsc
	xor	eax,	eax
	xor	edx,	edx
	ret
.have_tsc:
	rdtsc		; read TSC (edx:eax)
	ret



;--------------------------------------------------------------------

