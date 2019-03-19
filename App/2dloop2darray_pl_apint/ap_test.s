	.text
	.intel_syntax noprefix
	.file	"ap_test.cc"
	.globl	_Z1f11clang_apint_317_i11clang_apint_17_i # -- Begin function _Z1f11clang_apint_317_i11clang_apint_17_i
	.p2align	4, 0x90
	.type	_Z1f11clang_apint_317_i11clang_apint_17_i,@function
_Z1f11clang_apint_317_i11clang_apint_17_i: # @_Z1f11clang_apint_317_i11clang_apint_17_i
	.cfi_startproc
# %bb.0:                                # %entry
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset rbp, -16
	mov	rbp, rsp
	.cfi_def_cfa_register rbp
	push	rbx
	.cfi_offset rbx, -24
	mov	qword ptr [rbp - 48], rdi
	mov	qword ptr [rbp - 40], rsi
	mov	qword ptr [rbp - 32], rdx
	mov	qword ptr [rbp - 24], rcx
	movabs	rcx, 2305843009213693951
	and	r8, rcx
	mov	qword ptr [rbp - 16], r8
	mov	eax, r9d
	shr	eax, 16
	and	eax, 1
	mov	r10b, al
	mov	byte ptr [rbp - 50], r10b
	mov	r11w, r9w
	mov	word ptr [rbp - 52], r11w
	mov	rdx, qword ptr [rbp - 16]
	mov	rsi, qword ptr [rbp - 24]
	mov	rdi, qword ptr [rbp - 32]
	mov	r8, qword ptr [rbp - 48]
	mov	rbx, qword ptr [rbp - 40]
	add	r8, 1
	adc	rbx, 0
	adc	rdi, 0
	adc	rsi, 0
	adc	rdx, 0
	mov	qword ptr [rbp - 48], r8
	mov	qword ptr [rbp - 40], rbx
	mov	qword ptr [rbp - 32], rdi
	mov	qword ptr [rbp - 24], rsi
	and	rdx, rcx
	mov	qword ptr [rbp - 16], rdx
	movzx	eax, byte ptr [rbp - 50]
	shl	eax, 16
	movzx	r9d, word ptr [rbp - 52]
	or	r9d, eax
	mov	eax, dword ptr [rbp - 48]
	add	r9d, eax
	mov	r11w, r9w
	mov	word ptr [rbp - 52], r11w
	shr	r9d, 16
	and	r9d, 1
	mov	r10b, r9b
	mov	byte ptr [rbp - 50], r10b
	movzx	eax, byte ptr [rbp - 50]
	shl	eax, 16
	movzx	r9d, word ptr [rbp - 52]
	or	r9d, eax
	shl	r9d, 15
	sar	r9d, 15
	mov	eax, r9d
	pop	rbx
	pop	rbp
	.cfi_def_cfa rsp, 8
	ret
.Lfunc_end0:
	.size	_Z1f11clang_apint_317_i11clang_apint_17_i, .Lfunc_end0-_Z1f11clang_apint_317_i11clang_apint_17_i
	.cfi_endproc
                                        # -- End function
	.globl	main                    # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:                                # %entry
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset rbp, -16
	mov	rbp, rsp
	.cfi_def_cfa_register rbp
	sub	rsp, 32
	mov	dword ptr [rbp - 4], 0
	xor	eax, eax
	mov	ecx, eax
	mov	edi, 312
	mov	r9d, 5
	mov	rsi, rcx
	mov	rdx, rcx
	mov	qword ptr [rbp - 16], rcx # 8-byte Spill
	mov	r8, qword ptr [rbp - 16] # 8-byte Reload
	call	_Z1f11clang_apint_317_i11clang_apint_17_i
	mov	dword ptr [rbp - 8], eax
	mov	esi, dword ptr [rbp - 8]
	movabs	rdi, offset .L.str
	mov	al, 0
	call	printf
	xor	esi, esi
	mov	dword ptr [rbp - 20], eax # 4-byte Spill
	mov	eax, esi
	add	rsp, 32
	pop	rbp
	.cfi_def_cfa rsp, 8
	ret
.Lfunc_end1:
	.size	main, .Lfunc_end1-main
	.cfi_endproc
                                        # -- End function
	.type	.L.str,@object          # @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	"%d\n"
	.size	.L.str, 4


	.ident	"clang version 9.0.0 "
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym _Z1f11clang_apint_317_i11clang_apint_17_i
	.addrsig_sym printf
