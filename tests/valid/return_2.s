.globl main
main:
	pushq %rbp
	movq %rsp, rbp
	subq $0, %rsp
	movl $2, %rax
	movq %rbp, %rsp
	popq %rbp
	ret

.section .note.GNU-stack,"",@progbits
