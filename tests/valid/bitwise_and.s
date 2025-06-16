.globl main
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $4, %rsp
	movl $5, -4(%rbp)
	andl $3, -4(%rbp)
	movl -4(%rbp), %eax
	movq %rbp, %rsp
	popq %rbp
	ret

.section .note.GNU-stack,"",@progbits
