	.file	"perf_16byte.c"
	.text
	.p2align 4
	.globl	bypointer
	.type	bypointer, @function
bypointer:
.LFB22:
	.cfi_startproc
	movdqu	(%rdi), %xmm0
	paddq	.LC0(%rip), %xmm0
	movups	%xmm0, (%rdi)
	ret
	.cfi_endproc
.LFE22:
	.size	bypointer, .-bypointer
	.p2align 4
	.globl	byvalue
	.type	byvalue, @function
byvalue:
.LFB23:
	.cfi_startproc
	leaq	1(%rsi), %rdx
	leaq	1(%rdi), %rax
	ret
	.cfi_endproc
.LFE23:
	.size	byvalue, .-byvalue
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC1:
	.string	"%lld\n%lld"
	.section	.text.startup,"ax",@progbits
	.p2align 4
	.globl	main
	.type	main, @function
main:
.LFB24:
	.cfi_startproc
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	movl	%edi, %r12d
	xorl	%edi, %edi
	pushq	%rbp
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	pushq	%rbx
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	movq	%rsi, %rbx
	subq	$16, %rsp
	.cfi_def_cfa_offset 48
	call	time@PLT
	movl	%eax, %edi
	call	srand@PLT
	movq	8(%rbx), %rdi
	movl	$10, %edx
	xorl	%esi, %esi
	call	strtol@PLT
	movq	%rax, %rbp
	call	rand@PLT
	movq	8(%rbx), %rdi
	movl	$10, %edx
	xorl	%esi, %esi
	addl	%ebp, %eax
	movq	%rsp, %rbp
	cltq
	movq	%rax, (%rsp)
	call	strtol@PLT
	movq	%rax, %rbx
	call	rand@PLT
	addl	%ebx, %eax
	movl	$33554432, %ebx
	cltq
	movq	%rax, 8(%rsp)
	cmpl	$2, %r12d
	je	.L6
	.p2align 4,,10
	.p2align 3
.L5:
	movq	%rbp, %rdi
	call	bypointer
	subq	$1, %rbx
	jne	.L5
.L7:
	movq	8(%rsp), %rdx
	movq	(%rsp), %rsi
	leaq	.LC1(%rip), %rdi
	xorl	%eax, %eax
	call	printf@PLT
	addq	$16, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 32
	xorl	%eax, %eax
	popq	%rbx
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L6:
	.cfi_restore_state
	movq	(%rsp), %rdi
	movq	8(%rsp), %rsi
	call	byvalue
	movq	%rax, (%rsp)
	movq	%rdx, 8(%rsp)
	subq	$1, %rbx
	jne	.L6
	jmp	.L7
	.cfi_endproc
.LFE24:
	.size	main, .-main
	.section	.rodata.cst16,"aM",@progbits,16
	.align 16
.LC0:
	.quad	1
	.quad	1
	.ident	"GCC: (Debian 12.2.0-14) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
