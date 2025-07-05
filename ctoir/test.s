	.text
	.file	"test.c"
	.globl	testFunctionOne                 # -- Begin function testFunctionOne
	.p2align	4, 0x90
	.type	testFunctionOne,@function
testFunctionOne:                        # @testFunctionOne
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	popq	%rbp
	.cfi_def_cfa %rsp, 8
	retq
.Lfunc_end0:
	.size	testFunctionOne, .Lfunc_end0-testFunctionOne
	.cfi_endproc
                                        # -- End function
	.globl	testFunctionTwo                 # -- Begin function testFunctionTwo
	.p2align	4, 0x90
	.type	testFunctionTwo,@function
testFunctionTwo:                        # @testFunctionTwo
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	popq	%rbp
	.cfi_def_cfa %rsp, 8
	retq
.Lfunc_end1:
	.size	testFunctionTwo, .Lfunc_end1-testFunctionTwo
	.cfi_endproc
                                        # -- End function
	.globl	main                            # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	movl	$0, -4(%rbp)
	xorl	%eax, %eax
	popq	%rbp
	.cfi_def_cfa %rsp, 8
	retq
.Lfunc_end2:
	.size	main, .Lfunc_end2-main
	.cfi_endproc
                                        # -- End function
	.ident	"Debian clang version 18.1.8 (++20240731024826+3b5b5c1ec4a3-1~exp1~20240731144843.145)"
	.section	".note.GNU-stack","",@progbits
