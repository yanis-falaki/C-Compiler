.globl main
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $12, %rsp
	movl $0, -4(%rbp)
	movl $1, -8(%rbp)
	cmpl $0, -4(%rbp)
	jne .Lif_else.0
	movl $1, -8(%rbp)
	jmp .Lif_end.0
	.Lif_else.0:
	movl -8(%rbp), %r10d
	movl %r10d, -12(%rbp)
	notl -12(%rbp)
	cmpl $0, -12(%rbp)
	jne .Lif_end.1
	movl $2, -8(%rbp)
	.Lif_end.1:
	.Lif_end.0:
	movl -8(%rbp), %eax
	movq %rbp, %rsp
	popq %rbp
	ret
	movl $0, %eax
	movq %rbp, %rsp
	popq %rbp
	ret

.section .note.GNU-stack,"",@progbits
