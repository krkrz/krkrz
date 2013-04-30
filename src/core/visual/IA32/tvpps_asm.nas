; Photoshop-like layer blender for KIRIKIRI
; (c)2004-2005 Kengo Takagi (Kenjo) <kenjo@ceres.dti.ne.jp>
;
; TODO:
;   - Reduce penalty (MMX register pairing, mul result timing, etc.)
;   - Overlay/HardLight and table calc are little bit slow than other
;   - Not good alphablending accuracy (to simplify the operation, src alpha is cut to 7bit)
;
; Change log:
; 050727 Kenjo : - Used lightenblend.nas/darkenblend.nas algorithm for Lighten/Darken blend (2clock/pixel faster)
;                - Optimized AlphaBlend (2clk/pixel), Mul(1clk), Screen(1clk), SoftLight/ColorBurn(1clk?),
;                  ColorDodge(7clk?), Diff(1clk)
; 050804 Kenjo : - Fixed Add/Sub/ColorDodge/Diff as Photoshop7 style
;                - Separated Photoshop5.x style ColorDodge/Diff as different functions
;                - Fixed Diff(PS5) calculation

%include "nasm.nah"

%define IS_OPA  1
%define IS_HDA  2

	EXTERN  _TVPPsTableSoftLight
	EXTERN  _TVPPsTableColorDodge
	EXTERN  _TVPPsTableColorBurn

	BITS 32
	SECTION .text

;--------------------------------------------------------------------
; Base function macro
;
; void, TVPPs???Blend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;    Blend src and dst by specified operation, destroy dst alpha
;
; void, TVPPs???Blend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
;    Blend src and dst by specified operation, and alphablend the result and dst with (opa*100/255)%, destroy dst alpha
;
; void, TVPPs???Blend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;    Blend src and dst by specified operation, keep dst alpha
;
; void, TVPPs???Blend_HDA_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
;    Blend src and dst by specified operation, and alphablend the result and dst with (opa*100/255)%, keep dst alpha
;--------------------------------------------------------------------
%macro MACRO_MAINFUNC 2
	function_align

	push       ebp
	push       esi
	push       edi
	push       ebx
	push       ecx
	push       edx

	mov        ecx, [esp+36]         ; len
	cmp        ecx, byte 0
	jle        near %%exit           ; return if len==0
	mov        edi, [esp+28]         ; dst
	mov        esi, [esp+32]         ; src
	pxor       mm5, mm5              ; mm5 = 0000000000000000
%if %2&IS_OPA
	movd       mm4, [esp+40]         ; opa
%endif
	mov        eax, 0ffffffffh       ;
	movd       mm3, eax              ;
	lea        ebp, [esi+ecx*4]      ; src limit
	punpcklbw  mm3, mm5              ; mm3 = 00FF00FF00FF00FF (for Overlay/HardLight)
	mov        eax, 080808080h       ; eax = 80808080 (for Overlay/HardLight)
	sub        ebp, byte 16          ; 8*2 bytes pad (for loop expansion)
	cmp        esi, ebp
	jae        near %%remain         ; jump if src>=limit-16

	test       esi, 4
	jz         near %%loop
	%1.1       %2
	cmp        esi, ebp
	jae        near %%remain         ; jump if src>=limit-16

	loop_align
%%loop:
	%1.2       %2                    ; loop expansion QWORD x 2
	%1.2       %2
	cmp        esi, ebp
	jl         near %%loop           ; loop if src<limit-16

%%remain:
	add        ebp, byte 16          ; reminder
%%remloop:
	%1.1       %2
	cmp        esi, ebp
	jl         near %%remloop        ; loop if src<limit

%%exit:
	pop        edx
	pop        ecx
	pop        ebx
	pop        edi
	pop        esi
	pop        ebp
	emms
	ret
%endmacro

%macro MACRO_FUNCDEF 1
	GLOBAL _TVPPs%1_mmx_a
	GLOBAL _TVPPs%1_o_mmx_a
	GLOBAL _TVPPs%1_HDA_mmx_a
	GLOBAL _TVPPs%1_HDA_o_mmx_a

_TVPPs%1_mmx_a:
	MACRO_MAINFUNC %1,0

_TVPPs%1_o_mmx_a:
	MACRO_MAINFUNC %1,IS_OPA

_TVPPs%1_HDA_mmx_a:
	MACRO_MAINFUNC %1,IS_HDA

_TVPPs%1_HDA_o_mmx_a:
	MACRO_MAINFUNC %1,IS_OPA|IS_HDA
%endmacro


;--------------------------------------------------------------------
; Pixel blend macros
;  Input : esi=src pointer, edi=dst pointer, mm3=00ff00ff00ff00ff, mm4=opa, mm5=0
;
; xxx.1 is DWORD operation (1pixel) macro, xxx.2 is QWORD (2pixel) macro
;--------------------------------------------------------------------
; ############### Table calc blending
; XXX Too slow ...
%macro MACRO_TABLE_BLEND.1 2
	movd       mm2, [esi]           ; src
	movd       mm1, [edi]           ; dst
	xor        eax, eax             ;
	movd       ebx, mm2             ; ebx = src
	movd       edx, mm1             ; edx = dst
	mov        ah, bl               ;
	mov        al, dl               ;
	mov        bl, [%2+eax]         ; bl = tbl[src][dst] (blue)
	mov        ah, bh               ;
	mov        al, dh               ; XXX penalty?
	shr        edx, 16              ;
	mov        bh, [%2+eax]         ; bh = tbl[src][dst] (green)
	ror        ebx, 16              ; ebx = GGBBAA00    XXX penalty? (register access size mismatch)
	mov        ah, bl               ;
	mov        al, dl               ; XXX penalty?
	mov        bl, [%2+eax]         ; bl = tbl[src][dst] (red)
	ror        ebx, 16              ; ebx = AARRGGBB    XXX penalty? (register access size mismatch)
	psrld      mm2, 25              ; 7bit alpha
	movd       mm0, ebx             ;
%if %1&IS_OPA
	pmullw     mm2, mm4             ; mm2 = alpha*opa
%endif
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
%endif
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ; mm0 = (src-dst)*a
	add        esi, 4               ; src ptr++
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm1, mm0             ; mm1 = (((src-dst)*a)>>7)+dst
	add        edi, 4               ; dst ptr++
	packuswb   mm1, mm5             ; mm1 = AARRGGBB
	movd       [edi-4], mm1         ; store mm1 to dst ptr
%endmacro

%macro MACRO_TABLE_BLEND.2 2
	movq       mm6, [esi]           ; src
	movq       mm1, [edi]           ; dst
	movq       mm2, mm6             ; mm2 = src
	movq       mm3, mm1             ; mm3 = dst
	xor        eax, eax             ;
	movd       ebx, mm6             ; ebx = src
	movd       edx, mm1             ; edx = dst
	mov        ah, bl               ;
	mov        al, dl               ;
	mov        bl, [%2+eax]         ; bl = tbl[src][dst] (blue)
	mov        ah, bh               ;
	mov        al, dh               ; XXX penalty?
	shr        edx, 16              ;
	mov        bh, [%2+eax]         ; bh = tbl[src][dst] (green)
	ror        ebx, 16              ; ebx = GGBBAA00    XXX penalty? (register access size mismatch)
	mov        ah, bl               ;
	mov        al, dl               ; XXX penalty?
	mov        bl, [%2+eax]         ; bl = tbl[src][dst] (red)
	ror        ebx, 16              ; ebx = AARRGGBB    XXX penalty? (register access size mismatch)
	psrld      mm6, 25              ; 7bit alpha
	movd       mm0, ebx             ;
%if %1&IS_OPA
	pmullw     mm6, mm4             ; mm6 = alpha*opa
%endif
	psrlq      mm2, 32              ; mm2 = src (h)
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
	psrlq      mm3, 32              ; mm3 = dst (h)
%if %1&IS_OPA
	psrld      mm6, 8               ; mm6 = (alpha*opa)>>8
%endif
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
%if %1&IS_HDA
	movq       mm7, mm6             ; mm7 = 00000000000000AA
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm6, mm7             ; mm6 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm6, mm6             ; mm6 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm6             ; mm0 = (src-dst)*a
	movd       ebx, mm2             ; ebx = src
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm1, mm0             ; mm1 = (((src-dst)*a)>>7)+dst
	movd       edx, mm3             ; edx = dst
	mov        ah, bl               ;
	mov        al, dl               ;
	mov        bl, [%2+eax]         ; bl = tbl[src][dst] (blue)
	mov        ah, bh               ;
	mov        al, dh               ; XXX penalty?
	shr        edx, 16              ;
	mov        bh, [%2+eax]         ; bh = tbl[src][dst] (green)
	ror        ebx, 16              ; ebx = GGBBAA00    XXX penalty? (register access size mismatch)
	mov        ah, bl               ;
	mov        al, dl               ; XXX penalty?
	mov        bl, [%2+eax]         ; bl = tbl[src][dst] (red)
	ror        ebx, 16              ; ebx = AARRGGBB    XXX penalty? (register access size mismatch)
	psrld      mm2, 25              ; 7bit alpha
	movd       mm0, ebx             ;
%if %1&IS_OPA
	pmullw     mm2, mm4             ; mm2 = alpha*opa
%endif
	punpcklbw  mm3, mm5             ; mm3 = 00AA00RR00GG00BB
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
%endif
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm3             ; mm0 = src-dst
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm3             ; mm0 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ; mm0 = (src-dst)*a
	add        esi, 8               ; src ptr++
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm3, mm0             ; mm3 = (((src-dst)*a)>>7)+dst
	add        edi, 8               ; dst ptr++
	packuswb   mm1, mm3             ; mm1 = AARRGGBBAARRGGBB
	movq       [edi-8], mm1         ; store mm1 to dst ptr
%endmacro


; calc src*alpha BEFORE table reference (for PS5 style ColorDodge)
%macro MACRO_TABLE_BLEND2.1 2
	movd       mm2, [esi]           ; src
	movq       mm0, mm2             ;
	psrld      mm2, 25              ; 7bit alpha
	movd       mm1, [edi]           ; dst
%if %1&IS_OPA
	pmullw     mm2, mm4             ; mm2 = alpha*opa
%endif
	movd       edx, mm1             ; edx = dst
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
%endif
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
	pmullw     mm0, mm2             ; mm0 = src*a
	add        esi, 4               ; src ptr++
	psraw      mm0, 7               ; mm0 = ((src*a)>>7)
	xor        eax, eax             ;
	packuswb   mm0, mm5             ; mm0 = AARRGGBB
	movd       ebx, mm0             ; ebx = src
	mov        ah, bl               ;
	mov        al, dl               ; XXX penalty?
	mov        bl, [%2+eax]         ; bl = tbl[src][dst] (blue)
	mov        ah, bh               ;
	add        edi, 4               ; dst ptr++
	mov        al, dh               ;
	shr        edx, 16              ;
	mov        bh, [%2+eax]         ; bh = tbl[src][dst] (green)
	ror        ebx, 16              ; ebx = GGBBAA00    XXX penalty? (register access size mismatch)
	mov        ah, bl               ; 
%if %1&IS_HDA
	mov        bh, dh               ; msk = dst msk
%endif
	mov        al, dl               ;
	mov        bl, [%2+eax]         ; bl = tbl[src][dst] (red)
	ror        ebx, 16              ; ebx = AARRGGBB    XXX penalty? (register access size mismatch)
	mov        [edi-4], ebx         ; store ebx to dst ptr
%endmacro

%macro MACRO_TABLE_BLEND2.2 2
	movq       mm6, [esi]           ; src
	movq       mm0, mm6             ;
	movq       mm2, mm6             ;
	psrld      mm6, 25              ; 7bit alpha
	movq       mm1, [edi]           ; dst
%if %1&IS_OPA
	pmullw     mm6, mm4             ; mm6 = alpha*opa
%endif
	movd       edx, mm1             ; edx = dst
	psrlq      mm1, 32              ; mm1 = dst (h)
%if %1&IS_OPA
	psrld      mm6, 8               ; mm6 = (alpha*opa)>>8
%endif
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
	punpckldq  mm6, mm6             ; mm6 = 00AA00AA00AA00AA
	pmullw     mm0, mm6             ; mm0 = src*a
	movq       mm6, mm2             ;
	psraw      mm0, 7               ; mm0 = ((src*a)>>7)
	xor        eax, eax             ;
	packuswb   mm0, mm5             ; mm0 = AARRGGBB
	movd       ebx, mm0             ; ebx = src
	mov        ah, bl               ;
	mov        al, dl               ; XXX penalty?
	mov        bl, [%2+eax]         ; bl = tbl[src][dst] (blue)
	mov        ah, bh               ;
	psrlq      mm6, 57              ; 7bit alpha
	mov        al, dh               ;
	shr        edx, 16              ;
	mov        bh, [%2+eax]         ; bh = tbl[src][dst] (green)
	ror        ebx, 16              ; ebx = GGBBAA00    XXX penalty? (register access size mismatch)
	mov        ah, bl               ; 
%if %1&IS_HDA
	mov        bh, dh               ; msk = dst msk
%endif
	mov        al, dl               ;
	mov        bl, [%2+eax]         ; bl = tbl[src][dst] (red)
	ror        ebx, 16              ; ebx = AARRGGBB    XXX penalty? (register access size mismatch)
	movd       mm7, ebx             ; store ebx to mm7
%if %1&IS_OPA
	pmullw     mm6, mm4             ; mm6 = alpha*opa
%endif
	movd       edx, mm1             ; edx = dst (h)
%if %1&IS_OPA
	psrld      mm6, 8               ; mm6 = (alpha*opa)>>8
%endif
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	punpckhbw  mm2, mm5             ; mm2 = 00AA00RR00GG00BB
	punpckldq  mm6, mm6             ; mm6 = 00AA00AA00AA00AA
	pmullw     mm2, mm6             ; mm2 = src*a
	add        esi, 8               ; src ptr++
	psraw      mm2, 7               ; mm2 = ((src*a)>>7)
	packuswb   mm2, mm5             ; mm2 = AARRGGBB
	movd       ebx, mm2             ; ebx = src
	mov        ah, bl               ;
	mov        al, dl               ; XXX penalty?
	mov        bl, [%2+eax]         ; bl = tbl[src][dst] (blue)
	mov        ah, bh               ;
	mov        al, dh               ;
	shr        edx, 16              ;
	mov        bh, [%2+eax]         ; bh = tbl[src][dst] (green)
	ror        ebx, 16              ; ebx = GGBBAA00    XXX penalty? (register access size mismatch)
	mov        ah, bl               ; 
%if %1&IS_HDA
	mov        bh, dh               ; msk = dst msk
%endif
	mov        al, dl               ;
	mov        bl, [%2+eax]         ; bl = tbl[src][dst] (red)
	ror        ebx, 16              ; ebx = AARRGGBB    XXX penalty? (register access size mismatch)
	movd       mm6, ebx             ; store ebx to mm6
	psllq      mm6, 32              ;
	por        mm7, mm6             ;
	movq       [edi], mm7           ; store mm7 to dst ptr
	add        edi, 8               ; dst ptr++
%endmacro


; ############### AlphaBlend
%macro AlphaBlend.1 1
	movd       mm2, [esi]           ; src
	movd       mm1, [edi]           ; dst
	movq       mm0, mm2             ; mm0 = src
	psrld      mm2, 25              ; 7bit alpha
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm2, mm4             ; mm2 = alpha*opa
%endif
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
%endif
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ; mm0 = (src-dst)*a
	add        esi, 4               ; src ptr++
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm1, mm0             ; mm1 = (((src-dst)*a)>>7)+dst
	add        edi, 4               ; dst ptr++
	packuswb   mm1, mm5             ; mm1 = AARRGGBB
	movd       [edi-4], mm1         ; store mm1 to dst ptr
%endmacro

%macro AlphaBlend.2 1
	movq       mm0, [esi]           ; src
	movq       mm1, [edi]           ; dst
	movq       mm2, mm0             ; mm2 = src
	movq       mm6, mm2             ; mm6 = src
	movq       mm3, mm1             ; mm3 = dst
	psrld      mm6, 25              ; XXX 7bit
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB (src l)
%if %1&IS_OPA
	pmullw     mm6, mm4             ; mm6 = alpha*opa (l)
%endif
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB (dst l)
%if %1&IS_OPA
	psrld      mm6, 8               ; mm6 = (alpha*opa)>>8 (l)
%endif
%if %1&IS_HDA
	movq       mm7, mm6             ; mm7 = 00000000000000AA
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst (l)
	punpckldq  mm6, mm7             ; mm6 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst (l)
	punpckldq  mm6, mm6             ; mm6 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm6             ; mm0 = (src-dst)*a (l)
	add        esi, 8               ; src ptr++
	psraw      mm0, 7               ; mm0 = ((src-dst)*a)>>7
	movq       mm6, mm2             ; mm6 = src
	paddw      mm1, mm0             ; mm1 = (((src-dst)*a)>>7)+dst
	psrlq      mm6, 57              ; XXX 7bit
	punpckhbw  mm2, mm5             ; mm2 = 00AA00RR00GG00BB (src h)
%if %1&IS_OPA
	pmullw     mm6, mm4             ; mm6 = alpha*opa (h)
%endif
	punpckhbw  mm3, mm5             ; mm3 = 00AA00RR00GG00BB (dst h)
%if %1&IS_OPA
	psrld      mm6, 8               ; mm6 = (alpha*opa)>>8 (h)
%endif
%if %1&IS_HDA
	movq       mm7, mm6             ; mm7 = 00000000000000AA
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm2, mm3             ; mm2 = src-dst (h)
	punpckldq  mm6, mm7             ; mm6 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm2, mm3             ; mm2 = src-dst (h)
	punpckldq  mm6, mm6             ; mm6 = 00AA00AA00AA00AA
%endif
	pmullw     mm2, mm6             ; mm2 = (src-dst)*a (h)
	psraw      mm2, 7               ; mm2 = ((src-dst)*a)>>7
	paddw      mm3, mm2             ; mm3 = (((src-dst)*a)>>7)+dst
	packuswb   mm1, mm3             ; mm1 = AARRGGBBAARRGGBB
	movq       [edi], mm1           ; store mm1 to dst ptr
	add        edi, 8               ; dst ptr++
%endmacro


; -------------------- Fade src BEFORE add/sub
%if 0

; ############### Add
%macro AddBlend.1 1
	movd       mm2, [esi]           ; src
	movq       mm0, mm2             ; mm0 = src
	psrld      mm2, 25              ; 7bit alpha
	movd       mm1, [edi]           ; dst
%if %1&IS_OPA
	pmullw     mm2, mm4             ; mm2 = alpha*opa
%endif
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8     XXX penalty (mul result)
%endif
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA (keep dst alpha)   XXX penalty
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA   XXX penalty
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA   XXX penalty
%endif
	pmullw     mm0, mm2             ; mm0 = src*a
	add        esi, 4               ; src ptr++
	psraw      mm0, 7               ; mm0 = ((src*a)>>7)
	add        edi, 4               ; dst ptr++
	packuswb   mm0, mm5             ; mm0 = AARRGGBB
	paddusb    mm1, mm0             ; mm1 = dst+src (saturate)
	movd       [edi-4], mm1         ; store mm1 to dst ptr
%endmacro

%macro AddBlend.2 1
	movq       mm2, [esi]           ; src
	movq       mm0, mm2             ; mm0 = src
	movq       mm3, mm2             ; mm3 = src
	movq       mm6, mm2             ; mm6 = src
	psrld      mm2, 25              ; 7bit alpha
	psrlq      mm3, 57              ; 7bit alpha
	movq       mm1, [edi]           ; dst
%if %1&IS_OPA
	pmullw     mm2, mm4             ; mm2 = alpha*opa
	pmullw     mm3, mm4             ; mm3 = alpha*opa
%endif
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB (l)
	punpckhbw  mm6, mm5             ; mm6 = 00AA00RR00GG00BB (h)
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
	psrld      mm3, 8               ; mm3 = (alpha*opa)>>8
%endif
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA (keep dst alpha)   XXX penalty
	movq       mm7, mm3             ; mm7 = 00000000000000AA
	punpcklwd  mm3, mm3             ; mm3 = 0000000000AA00AA
	punpckldq  mm3, mm7             ; mm3 = 000000AA00AA00AA (keep dst alpha)   XXX penalty
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	punpcklwd  mm3, mm3             ; mm3 = 0000000000AA00AA
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
	punpckldq  mm3, mm3             ; mm3 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ; mm0 = src*a (l)
	pmullw     mm6, mm3             ; mm6 = src*a (h)
	add        esi, 8               ; src ptr++
	psraw      mm0, 7               ; mm0 = ((src*a)>>7) (l)
	psraw      mm6, 7               ; mm6 = ((src*a)>>7) (h)
	add        edi, 8               ; dst ptr++
	packuswb   mm0, mm6             ; mm0 = AARRGGBB
	paddusb    mm1, mm0             ; mm1 = dst+src (saturate)
	movq       [edi-8], mm1         ; store mm1 to dst ptr
%endmacro


; ############### Sub
%macro SubBlend.1 1
	movd       mm2, [esi]           ; src
	movq       mm0, mm2             ; mm0 = src
	psrld      mm2, 25              ; 7bit alpha
	movd       mm1, [edi]           ; dst
%if %1&IS_OPA
	pmullw     mm2, mm4             ; mm2 = alpha*opa
%endif
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8     XXX penalty (mul result)
%endif
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA (keep dst alpha)   XXX penalty
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA   XXX penalty
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA   XXX penalty
%endif
	pmullw     mm0, mm2             ; mm0 = src*a
	add        esi, 4               ; src ptr++
	psraw      mm0, 7               ; mm0 = ((src*a)>>7)
	add        edi, 4               ; dst ptr++
	packuswb   mm0, mm5             ; mm0 = AARRGGBB
	psubusb    mm1, mm0             ; mm1 = dst-src (saturate)
	movd       [edi-4], mm1         ; store mm1 to dst ptr
%endmacro

%macro SubBlend.2 1
	movq       mm2, [esi]           ; src
	movq       mm0, mm2             ; mm0 = src
	movq       mm3, mm2             ; mm3 = src
	movq       mm6, mm2             ; mm6 = src
	psrld      mm2, 25              ; 7bit alpha
	psrlq      mm3, 57              ; 7bit alpha
	movq       mm1, [edi]           ; dst
%if %1&IS_OPA
	pmullw     mm2, mm4             ; mm2 = alpha*opa
	pmullw     mm3, mm4             ; mm3 = alpha*opa
%endif
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB (l)
	punpckhbw  mm6, mm5             ; mm6 = 00AA00RR00GG00BB (h)
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
	psrld      mm3, 8               ; mm3 = (alpha*opa)>>8
%endif
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA (keep dst alpha)   XXX penalty
	movq       mm7, mm3             ; mm7 = 00000000000000AA
	punpcklwd  mm3, mm3             ; mm3 = 0000000000AA00AA
	punpckldq  mm3, mm7             ; mm3 = 000000AA00AA00AA (keep dst alpha)   XXX penalty
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	punpcklwd  mm3, mm3             ; mm3 = 0000000000AA00AA
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
	punpckldq  mm3, mm3             ; mm3 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ; mm0 = src*a (l)
	pmullw     mm6, mm3             ; mm6 = src*a (h)
	add        esi, 8               ; src ptr++
	psraw      mm0, 7               ; mm0 = ((src*a)>>7) (l)
	psraw      mm6, 7               ; mm6 = ((src*a)>>7) (h)
	add        edi, 8               ; dst ptr++
	packuswb   mm0, mm6             ; mm0 = AARRGGBB
	psubusb    mm1, mm0             ; mm1 = dst+src (saturate)
	movq       [edi-8], mm1         ; store mm1 to dst ptr
%endmacro


; -------------------- Blend src and dst AFTER add/sub
%else

; ############### Add
%macro AddBlend.1 1
	movd       mm2, [esi]           ; src
	movd       mm1, [edi]           ; dst
	movq       mm0, mm2             ; mm0 = src
	psrld      mm2, 25              ;
	paddusb    mm0, mm1             ; mm0 = dst+src (saturate)
%if %1&IS_OPA
	pmullw     mm2, mm4             ;
%endif
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*alv)>>8
%endif
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ;
	add        esi, 4               ; src ptr++
	psraw      mm0, 7               ;
	paddw      mm1, mm0             ; mm1 = ((src-dst)*a)>>7)+dst
	add        edi, 4               ; dst ptr++
	packuswb   mm1, mm5             ; mm1 = AARRGGBB
	movd       [edi-4], mm1         ; store mm1 to dst ptr
%endmacro

%macro AddBlend.2 1
	movq       mm2, [esi]           ; src
	movq       mm1, [edi]           ; dst
	movq       mm0, mm2             ; mm0 = src
	movq       mm6, mm2             ; mm6 = src
	psrld      mm2, 25              ;
	psrlq      mm6, 57              ;
	movd       eax, mm6             ; eax = alpha (h)
	movq       mm6, mm1             ; mm6 = dst
	paddusb    mm0, mm1             ; mm0 = dst+src (saturate)
	movq       mm3, mm0             ; mm3 = dst+src (saturate)
%if %1&IS_OPA
	pmullw     mm2, mm4             ;
%endif
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*alv)>>8
%endif
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ;
	movd       mm2, eax             ; mm2 = alpha (h)
	psraw      mm0, 7               ;
	paddw      mm1, mm0             ; mm1 = ((src-dst)*a)>>7)+dst
%if %1&IS_OPA
	pmullw     mm2, mm4             ;
%endif
	punpckhbw  mm6, mm5             ; mm6 = 00AA00RR00GG00BB
	punpckhbw  mm3, mm5             ; mm3 = 00AA00RR00GG00BB
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*alv)>>8
%endif
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm3, mm6             ; mm3 = src-dst
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm3, mm6             ; mm3 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm3, mm2             ;
	add        esi, 8               ; src ptr++
	psraw      mm3, 7               ;
	paddw      mm6, mm3             ; mm6 = ((src-dst)*a)>>7)+dst
	add        edi, 8               ; dst ptr++
	packuswb   mm1, mm6             ; mm1 = AARRGGBB
	movq       [edi-8], mm1         ; store mm1 to dst ptr
%endmacro


; ############### Sub
%macro SubBlend.1 1
	pxor       mm3, mm3
	movd       mm2, [esi]           ; src
	movd       mm1, [edi]           ; dst
	pcmpeqd    mm3, mm3             ; mm3 = FFFFFFFFFFFFFFFF
	movq       mm7, mm2             ; mm7 = src
	psrld      mm2, 25              ;
	pxor       mm7, mm3             ; mm7 = ~src
	movq       mm0, mm1             ; mm0 = dst
%if %1&IS_OPA
	pmullw     mm2, mm4             ;
%endif
	psubusb    mm0, mm7             ; mm0 = dst-src (saturate)
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*alv)>>8
%endif
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ;
	add        esi, 4               ; src ptr++
	psraw      mm0, 7               ;
	paddw      mm1, mm0             ; mm1 = ((src-dst)*a)>>7)+dst
	add        edi, 4               ; dst ptr++
	packuswb   mm1, mm5             ; mm1 = AARRGGBB
	movd       [edi-4], mm1         ; store mm1 to dst ptr
%endmacro

%macro SubBlend.2 1
	pxor       mm3, mm3
	movq       mm2, [esi]           ; src
	movq       mm1, [edi]           ; dst
	pcmpeqd    mm3, mm3             ; mm3 = FFFFFFFFFFFFFFFF
	movq       mm7, mm2             ; mm7 = src
	movq       mm6, mm2             ; mm6 = src
	psrld      mm2, 25              ;
	psrlq      mm6, 57              ;
	movd       eax, mm6             ; eax = alpha (h)
	movq       mm6, mm1             ; mm6 = dst
	pxor       mm7, mm3             ; mm7 = ~src
	movq       mm0, mm1             ; mm0 = dst
%if %1&IS_OPA
	pmullw     mm2, mm4             ;
%endif
	psubusb    mm0, mm7             ; mm0 = dst-src (saturate)
	movq       mm3, mm0             ; mm7 = dst-src (saturate)
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*alv)>>8
%endif
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ;
	movd       mm2, eax             ; mm2 = alpha (h)
	psraw      mm0, 7               ;
	paddw      mm1, mm0             ; mm1 = ((src-dst)*a)>>7)+dst
%if %1&IS_OPA
	pmullw     mm2, mm4             ;
%endif
	punpckhbw  mm6, mm5             ; mm6 = 00AA00RR00GG00BB
	punpckhbw  mm3, mm5             ; mm3 = 00AA00RR00GG00BB
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*alv)>>8
%endif
%if %1&IS_HDA
	movq       mm0, mm2             ; mm0 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm3, mm6             ; mm3 = src-dst
	punpckldq  mm2, mm0             ; mm2 = 000000AA00AA00AA
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm3, mm6             ; mm3 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm3, mm2             ;
	add        esi, 8               ; src ptr++
	psraw      mm3, 7               ;
	paddw      mm6, mm3             ; mm6 = ((src-dst)*a)>>7)+dst
	add        edi, 8               ; dst ptr++
	packuswb   mm1, mm6             ; mm1 = AARRGGBBAARRGGBB
	movq       [edi-8], mm1         ; store mm1 to dst ptr
%endmacro

%endif


; ############### Mul
%macro MulBlend.1 1
	movd       mm6, [esi]           ; src
	movd       mm1, [edi]           ; dst
	movq       mm0, mm6             ; mm0 = src
	psrld      mm6, 25              ; 7bit alpha
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm6, mm4             ; mm6 = alpha*opa
%endif
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
	pmullw     mm0, mm1             ; mm0 = dst*src
%if %1&IS_OPA
	psrld      mm6, 8               ; mm6 = (alpha*opa)>>8
%endif
	psrlw      mm0, 8               ; mm0 >>= 8  (/255)                XXX penalty
%if %1&IS_HDA
	movq       mm7, mm6             ; mm7 = 00000000000000AA
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm6, mm7             ; mm6 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm6, mm6             ; mm6 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm6             ; mm0 = (src-dst)*a
	add        esi, 4               ; src ptr++
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm1, mm0             ; mm1 = (((src-dst)*a)>>7)+dst
	add        edi, 4               ; dst ptr++
	packuswb   mm1, mm5             ; mm1 = AARRGGBB
	movd       [edi-4], mm1         ; store mm1 to dst ptr
%endmacro

%macro MulBlend.2 1
	movq       mm6, [esi]           ; src
	movq       mm1, [edi]           ; dst
	movq       mm0, mm6             ; mm0 = src
	movq       mm3, mm1             ; mm3 = dst
	movq       mm2, mm6             ; mm2 = src
	psrld      mm6, 25              ; 7bit alpha
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm6, mm4             ; mm6 = alpha*opa
%endif
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
	pmullw     mm0, mm1             ; mm0 = dst*src
%if %1&IS_OPA
	psrld      mm6, 8               ; mm6 = (alpha*opa)>>8
%endif
	psrlw      mm0, 8               ; mm0 >>= 8  (/255)                XXX penalty
%if %1&IS_HDA
	movq       mm7, mm6             ; mm7 = 00000000000000AA
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm6, mm7             ; mm6 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm6, mm6             ; mm6 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm6             ; mm0 = (src-dst)*a
	movq       mm6, mm2             ; mm6 = src
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm1, mm0             ; mm1 = (((src-dst)*a)>>7)+dst
	psrlq      mm6, 57              ; 7bit alpha
	punpckhbw  mm2, mm5             ; mm2 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm6, mm4             ; mm6 = alpha*opa
%endif
	punpckhbw  mm3, mm5             ; mm3 = 00AA00RR00GG00BB
	pmullw     mm2, mm3             ; mm2 = dst*src
%if %1&IS_OPA
	psrld      mm6, 8               ; mm6 = (alpha*opa)>>8
%endif
	psrlw      mm2, 8               ; mm2 >>= 8  (/255)                XXX penalty
%if %1&IS_HDA
	movq       mm7, mm6             ; mm7 = 00000000000000AA
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm2, mm3             ; mm2 = src-dst
	punpckldq  mm6, mm7             ; mm6 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm2, mm3             ; mm2 = src-dst
	punpckldq  mm6, mm6             ; mm6 = 00AA00AA00AA00AA
%endif
	pmullw     mm2, mm6             ; mm2 = (src-dst)*a
	add        esi, 8               ; src ptr++
	psraw      mm2, 7               ; mm2 = (((src-dst)*a)>>7)
	paddw      mm3, mm2             ; mm3 = (((src-dst)*a)>>7)+dst
	packuswb   mm1, mm3             ; mm1 = AARRGGBB
	movq       [edi], mm1           ; store mm1 to dst ptr
	add        edi, 8               ; dst ptr++
%endmacro


; ############### Screen
%macro ScreenBlend.1 1
	movd       mm2, [esi]           ; src
	movd       mm6, [edi]           ; dst
	movq       mm1, mm2             ; mm1 = src
	psrld      mm2, 25              ; 7bit alpha
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
	punpcklbw  mm6, mm5             ; mm6 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm2, mm4             ; mm2 = alpha*opa
%endif
	movq       mm0, mm1             ; mm0 = src backup
	pmullw     mm1, mm6             ; mm1 = dst*src
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
%endif
	psrlw      mm1, 8               ; mm1 >>= 8 (/255)                XXX penalty
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ; mm0 = (src-dst)*a
	add        esi, 4               ; src ptr++
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm6, mm0             ; mm6 = (((src-dst)*a)>>7)+dst
	add        edi, 4               ; dst ptr++
	packuswb   mm6, mm5             ; mm6 = AARRGGBB
	movd       [edi-4], mm6         ; store mm6 to dst ptr
%endmacro

%macro ScreenBlend.2 1
	movq       mm2, [esi]           ; src
	movq       mm3, [edi]           ; dst
	movq       mm1, mm2             ; mm1 = src
	movq       mm6, mm2             ; mm6 = src
	movq       mm7, mm3             ; mm7 = dst
	psrld      mm2, 25              ; 7bit alpha
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
	punpcklbw  mm3, mm5             ; mm3 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm2, mm4             ; mm2 = alpha*opa
%endif
	movq       mm0, mm1             ; mm0 = src backup
	pmullw     mm1, mm3             ; mm1 = dst*src
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
%endif
	psrlw      mm1, 8               ; mm1 >>= 8 (/255)                XXX penalty
%if %1&IS_HDA
	psubw      mm0, mm1             ; mm0 = src-dst
	movq       mm1, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	punpckldq  mm2, mm1             ; mm2 = 000000AA00AA00AA (keep dst alpha)  XXX penalty
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ; mm0 = (src-dst)*a
	movq       mm2, mm6             ; mm2 = src
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm3, mm0             ; mm3 = (((src-dst)*a)>>7)+dst
	psrlq      mm2, 57              ; 7bit alpha
	punpckhbw  mm6, mm5             ; mm6 = 00AA00RR00GG00BB
	punpckhbw  mm7, mm5             ; mm7 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm2, mm4             ; mm2 = alpha*opa
%endif
	movq       mm0, mm6             ; mm0 = src backup
	pmullw     mm6, mm7             ; mm6 = dst*src
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
%endif
	psrlw      mm6, 8               ; mm6 >>= 8 (/255)                XXX penalty
%if %1&IS_HDA
	psubw      mm0, mm6             ; mm0 = src-dst
	movq       mm6, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	punpckldq  mm2, mm6             ; mm2 = 000000AA00AA00AA (keep dst alpha)  XXX penalty
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm6             ; mm0 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ; mm0 = (src-dst)*a
	add        esi, 8               ; src ptr++
	add        edi, 8               ; dst ptr++
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm7, mm0             ; mm7 = (((src-dst)*a)>>7)+dst
	packuswb   mm3, mm7             ; mm3 = AARRGGBBAARRGGBB
	movq       [edi-8], mm3         ; store mm3 to dst ptr
%endmacro


; ############### Overlay
%macro OverlayBlend.1 1
	movd       mm2, [esi]           ; src
	movd       mm1, [edi]           ; dst
	movq       mm0, mm2             ; mm0 = src
	psrld      mm2, 25              ;
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm2, mm4             ; mm2 = alpha*opa
%endif
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
	movq       mm6, mm0             ; mm6 = src backup
	pmullw     mm0, mm1             ; mm0 = dst*src
	paddw      mm6, mm1             ; mm6 = dst+src
	psrlw      mm0, 7               ; mm0 = s*d*2/255
	psllw      mm6, 1               ; mm6 *= 2
	movd       mm7, eax             ; mm7 = eax(80808080)
	psubw      mm6, mm0             ; mm6 = (d+s)*2-(d*s*2)/255
	punpcklbw  mm7, mm5             ; mm7 = 0080008000800080
	psubw      mm6, mm3             ; mm6 = (d+s)*2-(d*s*2)/255-255  XXX could be overflow because of >>8 error
	pcmpgtw    mm7, mm1             ; mm7 = (128>d)?0xffff:0
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
%endif
	pand       mm0, mm7             ;
	pandn      mm7, mm6             ;
	por        mm0, mm7             ;
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ; mm0 = (src-dst)*a
	add        esi, 4               ; src ptr++
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm1, mm0             ; mm1 = (((src-dst)*a)>>7)+dst
	add        edi, 4               ; dst ptr++
	packuswb   mm1, mm5             ; mm1 = AARRGGBB
	movd       [edi-4], mm1         ; store mm1 to dst ptr
%endmacro

%macro OverlayBlend.2 1
	movq       mm2, [esi]           ; src
	movq       mm1, [edi]           ; dst
	movq       mm6, mm2             ; mm6 = src
	movq       mm7, mm1             ; mm7 = dst
	psrlq      mm6, 32              ;
	psrlq      mm7, 32              ;
	movd       ebx, mm6             ; ebx = src(h)
	movd       ecx, mm7             ; ecx = dst(h)
	movq       mm0, mm2             ; mm0 = src
	psrld      mm2, 25              ;
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm2, mm4             ; mm2 = alpha*opa
%endif
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
	movq       mm6, mm0             ; mm6 = src backup
	pmullw     mm0, mm1             ; mm0 = dst*src
	paddw      mm6, mm1             ; mm6 = dst+src
	psrlw      mm0, 7               ; mm0 = s*d*2/255
	psllw      mm6, 1               ; mm6 *= 2
	movd       mm7, eax             ; mm7 = eax(80808080)
	psubw      mm6, mm0             ; mm6 = (d+s)*2-(d*s*2)/255
	punpcklbw  mm7, mm5             ; mm7 = 0080008000800080
	psubw      mm6, mm3             ; mm6 = (d+s)*2-(d*s*2)/255-255  XXX could be overflow because of >>8 error
	pcmpgtw    mm7, mm1             ; mm7 = (128>d)?0xffff:0
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
%endif
	pand       mm0, mm7             ;
	pandn      mm7, mm6             ;
	por        mm0, mm7             ;
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ; mm0 = (src-dst)*a
	movd       mm2, ebx             ; src (h)
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm1, mm0             ; mm1 = (((src-dst)*a)>>7)+dst
	movq       mm0, mm2             ; mm0 = src
	packuswb   mm1, mm5             ; mm1 = AARRGGBB
	movd       edx, mm1             ; edx = result(l)
	movd       mm1, ecx             ; dst (h)
	psrld      mm2, 25              ;
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm2, mm4             ; mm2 = alpha*opa
%endif
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
	movq       mm6, mm0             ; mm6 = src backup
	pmullw     mm0, mm1             ; mm0 = dst*src
	paddw      mm6, mm1             ; mm6 = dst+src
	psrlw      mm0, 7               ; mm0 = s*d*2/255
	psllw      mm6, 1               ; mm6 *= 2
	movd       mm7, eax             ; mm7 = eax(80808080)
	psubw      mm6, mm0             ; mm6 = (d+s)*2-(d*s*2)/255
	punpcklbw  mm7, mm5             ; mm7 = 0080008000800080
	psubw      mm6, mm3             ; mm6 = (d+s)*2-(d*s*2)/255-255  XXX could be overflow because of >>8 error
	pcmpgtw    mm7, mm1             ; mm7 = (128>d)?0xffff:0
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
%endif
	pand       mm0, mm7             ;
	pandn      mm7, mm6             ;
	por        mm0, mm7             ;
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ; mm0 = (src-dst)*a
	add        esi, 8               ; src ptr++
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm1, mm0             ; mm1 = (((src-dst)*a)>>7)+dst
	add        edi, 8               ; dst ptr++
	packuswb   mm1, mm5             ; mm1 = AARRGGBB
	movd       mm0, edx             ; edx = result(l)
	psllq      mm1, 32              ;
	por        mm0, mm1             ;
	movq       [edi-8], mm0         ; store mm0 to dst ptr
%endmacro


; ############### HardLight
%macro HardLightBlend.1 1
	movd       mm2, [esi]           ; src
	movd       mm1, [edi]           ; dst
	movd       mm7, eax             ; mm7 = eax(80808080)
	movq       mm0, mm2             ; mm0 = src
	psrld      mm2, 25              ; 7bit alpha
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm2, mm4             ; mm2 = alpha*opa
%endif
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
	punpcklbw  mm7, mm5             ; mm7 = 0080008000800080
	movq       mm6, mm0             ; mm6 = src backup
	pcmpgtw    mm7, mm6             ; mm7 = (128>s)?0xffff:0
	paddw      mm6, mm1             ; mm6 = dst+src
	pmullw     mm0, mm1             ; mm0 = dst*src
	psllw      mm6, 1               ; mm6 *= 2
	psrlw      mm0, 7               ; mm0 = s*d*2/255
	psubw      mm6, mm0             ; mm6 = (d+s-d*s)*2
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
%endif
	psubw      mm6, mm3             ; mm6 = (d+s-d*s)*2-255  XXX could be overflow because of >>8 error
	pand       mm0, mm7             ;
	pandn      mm7, mm6             ;
	por        mm0, mm7             ;
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ; mm0 = (src-dst)*a
	add        esi, 4               ; src ptr++
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm1, mm0             ; mm1 = (((src-dst)*a)>>7)+dst
	add        edi, 4               ; dst ptr++
	packuswb   mm1, mm5             ; mm1 = AARRGGBB
	movd       [edi-4], mm1         ; store mm1 to dst ptr
%endmacro

%macro HardLightBlend.2 1
	movq       mm2, [esi]           ; src
	movq       mm1, [edi]           ; dst
	movq       mm6, mm2             ; mm6 = src
	movq       mm7, mm1             ; mm7 = dst
	psrlq      mm6, 32              ;
	psrlq      mm7, 32              ;
	movd       ebx, mm6             ; ebx = src(h)
	movd       ecx, mm7             ; ecx = dst(h)
	movd       mm7, eax             ; mm7 = eax(80808080)
	movq       mm0, mm2             ; mm0 = src
	psrld      mm2, 25              ; 7bit alpha
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm2, mm4             ; mm2 = alpha*opa
%endif
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
	punpcklbw  mm7, mm5             ; mm7 = 0080008000800080
	movq       mm6, mm0             ; mm6 = src backup
	pcmpgtw    mm7, mm6             ; mm7 = (128>s)?0xffff:0
	paddw      mm6, mm1             ; mm6 = dst+src
	pmullw     mm0, mm1             ; mm0 = dst*src
	psllw      mm6, 1               ; mm6 *= 2
	psrlw      mm0, 7               ; mm0 = s*d*2/255
	psubw      mm6, mm0             ; mm6 = (d+s-d*s)*2
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
%endif
	psubw      mm6, mm3             ; mm6 = (d+s-d*s)*2-255  XXX could be overflow because of >>8 error
	pand       mm0, mm7             ;
	pandn      mm7, mm6             ;
	por        mm0, mm7             ;
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ; mm0 = (src-dst)*a
	movd       mm2, ebx             ; src (h)
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm1, mm0             ; mm1 = (((src-dst)*a)>>7)+dst
	movq       mm0, mm2             ; mm0 = src
	packuswb   mm1, mm5             ; mm1 = AARRGGBB
	movd       edx, mm1             ; edx = result(l)
	movd       mm1, ecx             ; dst (h)
	movd       mm7, eax             ; mm7 = eax(80808080)
	psrld      mm2, 25              ; 7bit alpha
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm2, mm4             ; mm2 = alpha*opa
%endif
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
	punpcklbw  mm7, mm5             ; mm7 = 0080008000800080
	movq       mm6, mm0             ; mm6 = src backup
	pcmpgtw    mm7, mm6             ; mm7 = (128>s)?0xffff:0
	paddw      mm6, mm1             ; mm6 = dst+src
	pmullw     mm0, mm1             ; mm0 = dst*src
	psllw      mm6, 1               ; mm6 *= 2
	psrlw      mm0, 7               ; mm0 = s*d*2/255
	psubw      mm6, mm0             ; mm6 = (d+s-d*s)*2
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
%endif
	psubw      mm6, mm3             ; mm6 = (d+s-d*s)*2-255  XXX could be overflow because of >>8 error
	pand       mm0, mm7             ;
	pandn      mm7, mm6             ;
	por        mm0, mm7             ;
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ; mm0 = (src-dst)*a
	add        esi, 8               ; src ptr++
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm1, mm0             ; mm1 = (((src-dst)*a)>>7)+dst
	add        edi, 8               ; dst ptr++
	packuswb   mm1, mm5             ; mm1 = AARRGGBB
	movd       mm0, edx             ; edx = result(l)
	psllq      mm1, 32              ;
	por        mm0, mm1             ;
	movq       [edi-8], mm0         ; store mm0 to dst ptr
%endmacro


; ############### SoftLight
%macro SoftLightBlend.1 1
	MACRO_TABLE_BLEND.1 %1,_TVPPsTableSoftLight
%endmacro

%macro SoftLightBlend.2 1
	MACRO_TABLE_BLEND.2 %1,_TVPPsTableSoftLight
%endmacro


; ############### ColorDodge
%macro ColorDodgeBlend.1 1
	MACRO_TABLE_BLEND.1 %1,_TVPPsTableColorDodge
%endmacro

%macro ColorDodgeBlend.2 1
	MACRO_TABLE_BLEND.2 %1,_TVPPsTableColorDodge
%endmacro


; ############### ColorDodge (Photoshop5 style)
%macro ColorDodge5Blend.1 1
	MACRO_TABLE_BLEND2.1 %1,_TVPPsTableColorDodge
%endmacro

%macro ColorDodge5Blend.2 1
	MACRO_TABLE_BLEND2.2 %1,_TVPPsTableColorDodge
%endmacro


; ############### ColorBurn
%macro ColorBurnBlend.1 1
	MACRO_TABLE_BLEND.1 %1,_TVPPsTableColorBurn
%endmacro

%macro ColorBurnBlend.2 1
	MACRO_TABLE_BLEND.2 %1,_TVPPsTableColorBurn
%endmacro


; ############### Lighten
%macro LightenBlend.1 1
	movd       mm6, [esi]           ; src
	movq       mm0, mm6             ; mm0 = src
	psrld      mm6, 25              ; 7bit alpha
	movd       mm1, [edi]           ; dst
%if %1&IS_OPA
	pmullw     mm6, mm4             ; mm6 = alpha*opa
%endif
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
	movq       mm7, mm1             ; mm7 = dst
%if %1&IS_OPA
	psrld      mm6, 8               ; mm6 = (alpha*opa)>>8
%endif
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
	psubusb    mm7, mm0             ; mm7 = dst-src (saturate)
	paddb      mm0, mm7             ; mm0 = src+diff = lighten
%if %1&IS_HDA
	movq       mm7, mm6             ; mm7 = 00000000000000AA
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm6, mm7             ; mm6 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm6, mm6             ; mm6 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm6             ; mm0 = (src-dst)*a
	add        esi, 4               ; src ptr++
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm1, mm0             ; mm1 = (((src-dst)*a)>>7)+dst
	add        edi, 4               ; dst ptr++
	packuswb   mm1, mm5             ; mm1 = AARRGGBB
	movd       [edi-4], mm1         ; store mm1 to dst ptr
%endmacro

%macro LightenBlend.2 1
	movq       mm6, [esi]           ; src
	movq       mm1, [edi]           ; dst
	movq       mm0, mm6             ; mm0 = src
	movq       mm2, mm6             ; mm2 = src
	psrld      mm6, 25              ; 7bit alpha
	movq       mm3, mm1             ; mm3 = dst
%if %1&IS_OPA
	pmullw     mm6, mm4             ; mm6 = alpha*opa
%endif
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
	movq       mm7, mm1             ; mm7 = dst
%if %1&IS_OPA
	psrld      mm6, 8               ; mm6 = (alpha*opa)>>8
%endif
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
	psubusb    mm7, mm0             ; mm7 = dst-src (saturate)
	paddb      mm0, mm7             ; mm0 = lighten
%if %1&IS_HDA
	movq       mm7, mm6             ; mm7 = 00000000000000AA
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm6, mm7             ; mm6 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm6, mm6             ; mm6 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm6             ; mm0 = (src-dst)*a
	movq       mm6, mm2             ; mm6 = src
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	psrlq      mm6, 57              ; 7bit alpha
	paddw      mm1, mm0             ; mm1 = (((src-dst)*a)>>7)+dst
%if %1&IS_OPA
	pmullw     mm6, mm4             ; mm6 = alpha*opa
%endif
	punpckhbw  mm3, mm5             ; mm3 = 00AA00RR00GG00BB
	movq       mm7, mm3             ; mm7 = dst
	punpckhbw  mm2, mm5             ; mm2 = 00AA00RR00GG00BB
%if %1&IS_OPA
	psrld      mm6, 8               ; mm6 = (alpha*opa)>>8
%endif
	psubusb    mm7, mm2             ; mm7 = dst-src (saturate)
	paddb      mm2, mm7             ; mm0 = lighten
%if %1&IS_HDA
	movq       mm7, mm6             ; mm7 = 00000000000000AA
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm2, mm3             ; mm2 = src-dst
	punpckldq  mm6, mm7             ; mm6 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm2, mm3             ; mm2 = src-dst
	punpckldq  mm6, mm6             ; mm6 = 00AA00AA00AA00AA
%endif
	pmullw     mm2, mm6             ; mm2 = (src-dst)*a
	add        esi, 8               ; src ptr++
	psraw      mm2, 7               ; mm2 = (((src-dst)*a)>>7)
	paddw      mm3, mm2             ; mm3 = (((src-dst)*a)>>7)+dst
	add        edi, 8               ; dst ptr++
	packuswb   mm1, mm3             ; mm1 = AARRGGBBAARRGGBB
	movq       [edi-8], mm1         ; store mm1 to dst ptr
%endmacro


; ############### Darken
%macro DarkenBlend.1 1
	movd       mm6, [esi]           ; src
	movd       mm1, [edi]           ; dst
	movq       mm0, mm6             ; mm0 = src
	psrld      mm6, 25              ; 7bit alpha
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm6, mm4             ; mm6 = alpha*opa
%endif
	movq       mm7, mm0             ; mm7 = src
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
%if %1&IS_OPA
	psrld      mm6, 8               ; mm6 = (alpha*opa)>>8
%endif
	psubusb    mm7, mm1             ; mm7 = src-dst (saturate)
	psubb      mm0, mm7             ; mm0 = darken
%if %1&IS_HDA
	movq       mm7, mm6             ; mm7 = 00000000000000AA
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm6, mm7             ; mm6 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm6, mm6             ; mm6 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm6             ; mm0 = (src-dst)*a
	add        esi, 4               ; src ptr++
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm1, mm0             ; mm1 = (((src-dst)*a)>>7)+dst
	add        edi, 4               ; dst ptr++
	packuswb   mm1, mm5             ; mm1 = AARRGGBB
	movd       [edi-4], mm1         ; store mm1 to dst ptr
%endmacro

%macro DarkenBlend.2 1
	movq       mm6, [esi]           ; src
	movq       mm1, [edi]           ; dst
	movq       mm0, mm6             ; mm0 = src
	movq       mm2, mm6             ; mm2 = src
	psrld      mm6, 25              ; 7bit alpha
	movq       mm3, mm1             ; mm3 = dst
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm6, mm4             ; mm6 = alpha*opa
%endif
	movq       mm7, mm0             ; mm7 = src
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
%if %1&IS_OPA
	psrld      mm6, 8               ; mm6 = (alpha*opa)>>8
%endif
	psubusb    mm7, mm1             ; mm7 = src-dst (saturate)
	psubb      mm0, mm7             ; mm0 = darken
%if %1&IS_HDA
	movq       mm7, mm6             ; mm7 = 00000000000000AA
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm6, mm7             ; mm6 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm6, mm6             ; mm6 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm6             ; mm0 = (src-dst)*a
	movq       mm6, mm2             ; mm6 = src
	punpckhbw  mm2, mm5             ; mm2 = 00AA00RR00GG00BB
	psrlq      mm6, 57              ; 7bit alpha
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm1, mm0             ; mm1 = (((src-dst)*a)>>7)+dst
%if %1&IS_OPA
	pmullw     mm6, mm4             ; mm6 = alpha*opa
%endif
	movq       mm7, mm2             ; mm7 = src
	punpckhbw  mm3, mm5             ; mm3 = 00AA00RR00GG00BB
%if %1&IS_OPA
	psrld      mm6, 8               ; mm6 = (alpha*opa)>>8
%endif
	psubusb    mm7, mm3             ; mm7 = src-dst (saturate)
	psubb      mm2, mm7             ; mm2 = darken
%if %1&IS_HDA
	movq       mm7, mm6             ; mm7 = 00000000000000AA
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm2, mm3             ; mm2 = src-dst
	punpckldq  mm6, mm7             ; mm6 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm2, mm3             ; mm2 = src-dst
	punpckldq  mm6, mm6             ; mm6 = 00AA00AA00AA00AA
%endif
	pmullw     mm2, mm6             ; mm2 = (src-dst)*a
	add        esi, 8               ; src ptr++
	psraw      mm2, 7               ; mm2 = (((src-dst)*a)>>7)
	paddw      mm3, mm2             ; mm3 = (((src-dst)*a)>>7)+dst
	packuswb   mm1, mm3             ; mm1 = AARRGGBBAARRGGBB
	movq       [edi], mm1           ; store mm1 to dst ptr
	add        edi, 8               ; dst ptr++
%endmacro


; ############### Diff
%macro DiffBlend.1 1
	movd       mm2, [esi]           ; src
	movd       mm1, [edi]           ; dst
	movq       mm0, mm1             ; mm0 = dst
	movq       mm3, mm2             ; mm3 = src
	psrld      mm2, 25              ;
	psubusb    mm0, mm3             ; mm0 = dst-src (saturate)
%if %1&IS_OPA
	pmullw     mm2, mm4             ;
%endif
	psubusb    mm3, mm1             ; mm3 = src-dst (saturate)
	paddb      mm0, mm3             ; mm0 = Diff
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
%endif
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
%if %1&IS_HDA
	movq       mm3, mm2             ; mm3 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm3             ; mm2 = 000000AA00AA00AA
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 000000AA00AA00AA
%endif
	pmullw     mm0, mm2             ;
	add        esi, 4               ; src ptr++
	psraw      mm0, 7               ;
	paddw      mm1, mm0             ; mm1 = ((src-dst)*a)>>7)+dst
	add        edi, 4               ; dst ptr++
	packuswb   mm1, mm5             ; mm1 = AARRGGBB
	movd       [edi-4], mm1         ; store mm1 to dst ptr
%endmacro

%if 0
%macro DiffBlend.2 1
	movq       mm2, [esi]           ; src
	movq       mm1, [edi]           ; dst
	movq       mm6, mm2             ; mm6 = src
	movq       mm7, mm1             ; mm7 = dst
	movq       mm0, mm1             ; mm0 = dst
	movq       mm3, mm2             ; mm3 = src
	psrld      mm2, 25              ; 7bit alpha
	psubusb    mm0, mm3             ; mm0 = dst-src (saturate)
%if %1&IS_OPA
	pmullw     mm2, mm4             ;
%endif
	psubusb    mm3, mm1             ; mm3 = src-dst (saturate)
	paddb      mm0, mm3             ; mm0 = Diff
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
%endif
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
%if %1&IS_HDA
	movq       mm3, mm2             ; mm3 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm3             ; mm2 = 000000AA00AA00AA
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 000000AA00AA00AA
%endif
	pmullw     mm0, mm2             ;
	movq       mm3, mm6             ; mm3 = src
	psraw      mm0, 7               ;
	paddw      mm1, mm0             ; mm1 = ((src-dst)*a)>>7)+dst
	movq       mm0, mm7             ; mm0 = dst
	psrlq      mm6, 57              ; 7bit alpha
	psubusb    mm0, mm3             ; mm0 = dst-src (saturate)
%if %1&IS_OPA
	pmullw     mm6, mm4             ;
%endif
	psubusb    mm3, mm7             ; mm3 = src-dst (saturate)
	paddb      mm0, mm3             ; mm0 = Diff
%if %1&IS_OPA
	psrld      mm6, 8               ; mm6 = (alpha*opa)>>8
%endif
	punpckhbw  mm7, mm5             ; mm7 = 00AA00RR00GG00BB
	punpckhbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
%if %1&IS_HDA
	movq       mm3, mm6             ; mm3 = 00000000000000AA
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm7             ; mm0 = src-dst
	punpckldq  mm6, mm3             ; mm6 = 000000AA00AA00AA
%else
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm7             ; mm0 = src-dst
	punpckldq  mm6, mm6             ; mm6 = 000000AA00AA00AA
%endif
	pmullw     mm0, mm6             ;
	add        esi, 8               ; src ptr++
	psraw      mm0, 7               ;
	paddw      mm7, mm0             ; mm7 = ((src-dst)*a)>>7)+dst
	add        edi, 8               ; dst ptr++
	packuswb   mm1, mm7             ; mm1 = AARRGGBB
	movq       [edi-8], mm1         ; store mm1 to dst ptr
%endmacro


%else


%macro DiffBlend.2 1
	movq       mm2, [esi]           ; src
	movq       mm1, [edi]           ; dst
	movq       mm3, mm2             ; mm3 = src
	movq       mm7, mm1             ; mm7 = dst
	psrlq      mm3, 57              ; 7bit alpha
	movd       eax, mm3             ; eax = alpha (h)
	movq       mm0, mm1             ; mm0 = dst
	movq       mm3, mm2             ; mm3 = src
	psrld      mm2, 25              ; 7bit alpha
	psubusb    mm0, mm3             ; mm0 = dst-src (saturate)
%if %1&IS_OPA
	pmullw     mm2, mm4             ;
%endif
	psubusb    mm3, mm1             ; mm3 = src-dst (saturate)
	paddb      mm0, mm3             ; mm0 = Diff
	movq       mm6, mm0             ; mm6 = Diff
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
%endif
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
%if %1&IS_HDA
	movq       mm3, mm2             ; mm3 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm3             ; mm2 = 000000AA00AA00AA
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm1             ; mm0 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 000000AA00AA00AA
%endif
	pmullw     mm0, mm2             ;
	movd       mm2, eax             ; mm2 = 7bit alpha (h)
	psraw      mm0, 7               ;
%if %1&IS_OPA
	pmullw     mm2, mm4             ;
%endif
	paddw      mm1, mm0             ; mm1 = ((src-dst)*a)>>7)+dst
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
%endif
	punpckhbw  mm7, mm5             ; mm7 = 00AA00RR00GG00BB
	punpckhbw  mm6, mm5             ; mm6 = 00AA00RR00GG00BB
%if %1&IS_HDA
	movq       mm3, mm2             ; mm3 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm6, mm7             ; mm6 = src-dst
	punpckldq  mm2, mm3             ; mm2 = 000000AA00AA00AA
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm6, mm7             ; mm6 = src-dst
	punpckldq  mm2, mm2             ; mm2 = 000000AA00AA00AA
%endif
	pmullw     mm6, mm2             ;
	add        esi, 8               ; src ptr++
	psraw      mm6, 7               ;
	paddw      mm7, mm6             ; mm7 = ((src-dst)*a)>>7)+dst
	add        edi, 8               ; dst ptr++
	packuswb   mm1, mm7             ; mm1 = AARRGGBBAARRGGBB
	movq       [edi-8], mm1         ; store mm1 to dst ptr
%endmacro


%endif


; Photoshop5 works :
;   1. s = (*src) * alpha
;   2. diff = abs(s-(*dst))
%macro Diff5Blend.1 1
	movd       mm6, [esi]           ; src
	movq       mm0, mm6             ; mm0 = src
	psrld      mm6, 25              ; 7bit alpha
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm6, mm4             ; mm6 = alpha*opa
%endif
	movd       mm1, [edi]           ; dst
%if %1&IS_OPA
	psrld      mm6, 8               ; mm6 = (alpha*opa)>>8
%endif
%if %1&IS_HDA
	movq       mm7, mm6             ; mm7 = 00000000000000AA
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
	punpckldq  mm6, mm7             ; mm6 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
	punpckldq  mm6, mm6             ; mm6 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm6             ; mm0 = src*a
	movq       mm7, mm1             ; mm7 = dst
	psraw      mm0, 7               ; mm0 = ((src*a)>>7)              XXX penalty (mul result)
	psubusw    mm7, mm0             ; mm7 = dst-src (saturate)
	psubusw    mm0, mm1             ; mm0 = src-dst (saturate)
	paddw      mm7, mm0             ; mm7 = diff
	add        esi, 4               ; src ptr++
	add        edi, 4               ; dst ptr++
	packuswb   mm7, mm5             ; mm1 = AARRGGBB
	movd       [edi-4], mm7         ; store mm7 to dst ptr
%endmacro

%macro Diff5Blend.2 1
	movq       mm6, [esi]           ; src
	movq       mm0, mm6             ; mm0 = src
	movq       mm2, mm6             ; mm2 = src
	psrld      mm6, 25              ; 7bit alpha
	punpcklbw  mm0, mm5             ; mm0 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm6, mm4             ; mm6 = alpha*opa
%endif
	movq       mm1, [edi]           ; dst
	movq       mm3, mm1             ; mm3 = dst
%if %1&IS_OPA
	psrld      mm6, 8               ; mm6 = (alpha*opa)>>8
%endif
%if %1&IS_HDA
	movq       mm7, mm6             ; mm7 = 00000000000000AA
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
	punpckldq  mm6, mm7             ; mm6 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
	punpckldq  mm6, mm6             ; mm6 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm6             ; mm0 = src*a
	movq       mm7, mm1             ; mm7 = dst
	movq       mm6, mm2             ; src
	psraw      mm0, 7               ; mm0 = ((src*a)>>7)
	psrlq      mm6, 57              ; 7bit alpha
	psubusw    mm7, mm0             ; mm7 = dst-src (saturate)
	psubusw    mm0, mm1             ; mm0 = src-dst (saturate)
%if %1&IS_OPA
	pmullw     mm6, mm4             ; mm6 = alpha*opa
%endif
	paddw      mm7, mm0             ; mm7 = diff
	punpckhbw  mm2, mm5             ; mm2 = 00AA00RR00GG00BB
%if %1&IS_OPA
	psrld      mm6, 8               ; mm6 = (alpha*opa)>>8
%endif
%if %1&IS_HDA
	movq       mm0, mm6             ; mm7 = 00000000000000AA
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	punpckhbw  mm3, mm5             ; mm3 = 00AA00RR00GG00BB
	punpckldq  mm6, mm0             ; mm6 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	punpckhbw  mm3, mm5             ; mm3 = 00AA00RR00GG00BB
	punpckldq  mm6, mm6             ; mm6 = 00AA00AA00AA00AA
%endif
	pmullw     mm2, mm6             ; mm2 = src*a
	movq       mm1, mm3             ; mm1 = dst
	psraw      mm2, 7               ; mm2 = ((src*a)>>7)              XXX penalty (mul result)
	psubusw    mm1, mm2             ; mm1 = dst-src (saturate)
	psubusw    mm2, mm3             ; mm2 = src-dst (saturate)
	paddw      mm1, mm2             ; mm1 = diff
	packuswb   mm7, mm1             ; mm1 = AARRGGBBAARRGGBB
	movq       [edi], mm7           ; store mm7 to dst ptr
	add        esi, 8               ; src ptr++
	add        edi, 8               ; dst ptr++
%endmacro


; ############### Exclusion
%macro ExclusionBlend.1 1
	movd       mm2, [esi]           ; src
	movd       mm1, [edi]           ; dst
	movq       mm6, mm2             ; mm6 = src
	psrld      mm2, 25              ; 7bit alpha
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm2, mm4             ; mm2 = alpha*opa
%endif
	punpcklbw  mm6, mm5             ; mm6 = 00AA00RR00GG00BB
	movq       mm0, mm6             ; mm0 = src backup
	pmullw     mm6, mm1             ; mm6 = dst*src
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
%endif
	psrlw      mm6, 7               ; mm6 = s*d*2/255
%if %1&IS_HDA
	movq       mm7, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm6             ; mm0 = (d+s)-d*s*2
	punpckldq  mm2, mm7             ; mm2 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm6             ; mm0 = (d+s)-d*s*2
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ; mm0 = (src-dst)*a
	add        esi, 4               ; src ptr++
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm1, mm0             ; mm1 = (((src-dst)*a)>>7)+dst
	add        edi, 4               ; dst ptr++
	packuswb   mm1, mm5             ; mm1 = AARRGGBB
	movd       [edi-4], mm1         ; store mm1 to dst ptr
%endmacro

%macro ExclusionBlend.2 1
	movq       mm2, [esi]           ; src
	movq       mm1, [edi]           ; dst
	movq       mm3, mm2             ; mm3 = src
	movq       mm6, mm2             ; mm6 = src
	movq       mm7, mm1             ; mm7 = dst
	psrld      mm2, 25              ; 7bit alpha
	punpcklbw  mm1, mm5             ; mm1 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm2, mm4             ; mm2 = alpha*opa
%endif
	punpcklbw  mm3, mm5             ; mm3 = 00AA00RR00GG00BB
	movq       mm0, mm3             ; mm0 = src backup
	pmullw     mm3, mm1             ; mm3 = dst*src
%if %1&IS_OPA
	psrld      mm2, 8               ; mm2 = (alpha*opa)>>8
%endif
	psrlw      mm3, 7               ; mm3 = s*d*2/255
%if %1&IS_HDA
	psubw      mm0, mm3             ; mm0 = (d+s)-d*s*2
	movq       mm3, mm2             ; mm7 = 00000000000000AA
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	punpckldq  mm2, mm3             ; mm2 = 000000AA00AA00AA (keep dst alpha)  XXX penalty
%else
	punpcklwd  mm2, mm2             ; mm2 = 0000000000AA00AA
	psubw      mm0, mm3             ; mm0 = (d+s)-d*s*2
	punpckldq  mm2, mm2             ; mm2 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm2             ; mm0 = (src-dst)*a
	movq       mm3, mm6             ; mm3 = src
	psrlq      mm6, 57              ; 7bit alpha
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm1, mm0             ; mm1 = (((src-dst)*a)>>7)+dst
	punpckhbw  mm7, mm5             ; mm7 = 00AA00RR00GG00BB
%if %1&IS_OPA
	pmullw     mm6, mm4             ; mm6 = alpha*opa
%endif
	punpckhbw  mm3, mm5             ; mm3 = 00AA00RR00GG00BB
	movq       mm0, mm3             ; mm0 = src backup
	pmullw     mm3, mm7             ; mm3 = dst*src
%if %1&IS_OPA
	psrld      mm6, 8               ; mm6 = (alpha*opa)>>8
%endif
	psrlw      mm3, 7               ; mm3 = s*d*2/255
%if %1&IS_HDA
	movq       mm2, mm6             ; mm7 = 00000000000000AA
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm3             ; mm0 = (d+s)-d*s*2
	punpckldq  mm6, mm2             ; mm6 = 000000AA00AA00AA (keep dst alpha)
%else
	punpcklwd  mm6, mm6             ; mm6 = 0000000000AA00AA
	psubw      mm0, mm3             ; mm0 = (d+s)-d*s*2
	punpckldq  mm6, mm6             ; mm6 = 00AA00AA00AA00AA
%endif
	pmullw     mm0, mm6             ; mm0 = (src-dst)*a
	add        esi, 8               ; src ptr++
	psraw      mm0, 7               ; mm0 = (((src-dst)*a)>>7)
	paddw      mm7, mm0             ; mm7 = (((src-dst)*a)>>7)+dst
	add        edi, 8               ; dst ptr++
	packuswb   mm1, mm7             ; mm7 = AARRGGBB
	movq       [edi-8], mm1         ; store mm1 to dst ptr
%endmacro


;--------------------------------------------------------------------
; Substances for functions
;--------------------------------------------------------------------
MACRO_FUNCDEF AlphaBlend
MACRO_FUNCDEF AddBlend
MACRO_FUNCDEF SubBlend
MACRO_FUNCDEF MulBlend
MACRO_FUNCDEF ScreenBlend
MACRO_FUNCDEF OverlayBlend
MACRO_FUNCDEF HardLightBlend
MACRO_FUNCDEF SoftLightBlend
MACRO_FUNCDEF ColorDodgeBlend
MACRO_FUNCDEF ColorDodge5Blend
MACRO_FUNCDEF ColorBurnBlend
MACRO_FUNCDEF LightenBlend
MACRO_FUNCDEF DarkenBlend
MACRO_FUNCDEF DiffBlend
MACRO_FUNCDEF Diff5Blend
MACRO_FUNCDEF ExclusionBlend

;--------------------------------------------------------------------
; function replace directives
;--------------------------------------------------------------------


;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsAlphaBlend
;;void, TVPPsAlphaBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsAlphaBlend_o
;;void, TVPPsAlphaBlend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsAlphaBlend_HDA
;;void, TVPPsAlphaBlend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsAlphaBlend_HDA_o
;;void, TVPPsAlphaBlend_HDA_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsAddBlend
;;void, TVPPsAddBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsAddBlend_o
;;void, TVPPsAddBlend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsAddBlend_HDA
;;void, TVPPsAddBlend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsAddBlend_HDA_o
;;void, TVPPsAddBlend_HDA_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsSubBlend
;;void, TVPPsSubBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsSubBlend_o
;;void, TVPPsSubBlend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsSubBlend_HDA
;;void, TVPPsSubBlend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsSubBlend_HDA_o
;;void, TVPPsSubBlend_HDA_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsMulBlend
;;void, TVPPsMulBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsMulBlend_o
;;void, TVPPsMulBlend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsMulBlend_HDA
;;void, TVPPsMulBlend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsMulBlend_HDA_o
;;void, TVPPsMulBlend_HDA_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsScreenBlend
;;void, TVPPsScreenBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsScreenBlend_o
;;void, TVPPsScreenBlend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsScreenBlend_HDA
;;void, TVPPsScreenBlend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsScreenBlend_HDA_o
;;void, TVPPsScreenBlend_HDA_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsOverlayBlend
;;void, TVPPsOverlayBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsOverlayBlend_o
;;void, TVPPsOverlayBlend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsOverlayBlend_HDA
;;void, TVPPsOverlayBlend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsOverlayBlend_HDA_o
;;void, TVPPsOverlayBlend_HDA_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsHardLightBlend
;;void, TVPPsHardLightBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsHardLightBlend_o
;;void, TVPPsHardLightBlend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsHardLightBlend_HDA
;;void, TVPPsHardLightBlend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsHardLightBlend_HDA_o
;;void, TVPPsHardLightBlend_HDA_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsSoftLightBlend
;;void, TVPPsSoftLightBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsSoftLightBlend_o
;;void, TVPPsSoftLightBlend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsSoftLightBlend_HDA
;;void, TVPPsSoftLightBlend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsSoftLightBlend_HDA_o
;;void, TVPPsSoftLightBlend_HDA_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsColorDodgeBlend
;;void, TVPPsColorDodgeBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsColorDodgeBlend_o
;;void, TVPPsColorDodgeBlend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsColorDodgeBlend_HDA
;;void, TVPPsColorDodgeBlend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsColorDodgeBlend_HDA_o
;;void, TVPPsColorDodgeBlend_HDA_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsColorDodge5Blend
;;void, TVPPsColorDodge5Blend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsColorDodge5Blend_o
;;void, TVPPsColorDodge5Blend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsColorDodge5Blend_HDA
;;void, TVPPsColorDodge5Blend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsColorDodge5Blend_HDA_o
;;void, TVPPsColorDodge5Blend_HDA_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsColorBurnBlend
;;void, TVPPsColorBurnBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsColorBurnBlend_o
;;void, TVPPsColorBurnBlend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsColorBurnBlend_HDA
;;void, TVPPsColorBurnBlend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsColorBurnBlend_HDA_o
;;void, TVPPsColorBurnBlend_HDA_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsLightenBlend
;;void, TVPPsLightenBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsLightenBlend_o
;;void, TVPPsLightenBlend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsLightenBlend_HDA
;;void, TVPPsLightenBlend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsLightenBlend_HDA_o
;;void, TVPPsLightenBlend_HDA_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsDarkenBlend
;;void, TVPPsDarkenBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsDarkenBlend_o
;;void, TVPPsDarkenBlend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsDarkenBlend_HDA
;;void, TVPPsDarkenBlend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsDarkenBlend_HDA_o
;;void, TVPPsDarkenBlend_HDA_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsDiffBlend
;;void, TVPPsDiffBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsDiffBlend_o
;;void, TVPPsDiffBlend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsDiffBlend_HDA
;;void, TVPPsDiffBlend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsDiffBlend_HDA_o
;;void, TVPPsDiffBlend_HDA_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsDiff5Blend
;;void, TVPPsDiff5Blend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsDiff5Blend_o
;;void, TVPPsDiff5Blend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsDiff5Blend_HDA
;;void, TVPPsDiff5Blend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsDiff5Blend_HDA_o
;;void, TVPPsDiff5Blend_HDA_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)

;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsExclusionBlend
;;void, TVPPsExclusionBlend_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsExclusionBlend_o
;;void, TVPPsExclusionBlend_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsExclusionBlend_HDA
;;void, TVPPsExclusionBlend_HDA_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len)
;;[function_replace_by TVPCPUType & TVP_CPU_HAS_MMX] TVPPsExclusionBlend_HDA_o
;;void, TVPPsExclusionBlend_HDA_o_mmx_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa)



; [EOF]
