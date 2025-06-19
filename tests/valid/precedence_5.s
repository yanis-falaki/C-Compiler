.globl main
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $24, %rsp
	movl $0, %r10d
	
	movl $0, -4(%rbp)
	
	movl $2, -8(%rbp)
	addl $1, -8(%rbp)
	movl $1, %r10d
	
	movl $0, -12(%rbp)
	
	
	movl $0, -16(%rbp)
	
	
	
	
	
	movl $1, -20(%rbp)
	
	
	movl $0, -20(%rbp)
	
	movl -20(%rbp), %r10d
	movl %r10d, -24(%rbp)
	addl $1, -24(%rbp)
	movl -24(%rbp), %eax
	movq %rbp, %rsp
	popq %rbp
	ret

.section .note.GNU-stack,"",@progbits
