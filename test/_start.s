// for testing the output of czakoc
// remove this in the future
.text
.globl _start
_start:
	xor %rbp, %rbp
	andq $-16, %rsp
	movq (%rsp), %rdi
	movq 8(%rsp), %rsi
	// It will be crashed in [CoolPotOS](https://github.com/plos-clan/CoolPotOS)
	movq 8(%rsp, %rdi, 8), %rdx
	xorq %rax, %rax
	call main
	movq %rax, %rdi
	movq $60, %rax
	syscall
