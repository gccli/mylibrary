	.file	"bitbype.c"
	.text
.globl func1
	.type	func1, @function
func1:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	$0, -4(%ebp)
	movl	$0, -8(%ebp)
	jmp	.L2
.L3:
	movl	-8(%ebp), %eax
	addl	%eax, -4(%ebp)
	addl	$1, -8(%ebp)
.L2:
	cmpl	$99, -8(%ebp)
	jle	.L3
	leave
	ret
	.size	func1, .-func1
	.section	.rodata
.LC0:
	.string	"0x%x\n"
.LC1:
	.string	"x=0x%x, y=0x%x, z=0x%x\n"
	.text
.globl main
	.type	main, @function
main:
	leal	4(%esp), %ecx
	andl	$-16, %esp
	pushl	-4(%ecx)
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	pushl	%ecx
	subl	$32, %esp
	movl	$-2147483647, -20(%ebp)
	movl	$-256, -16(%ebp)
	movl	$4096, -12(%ebp)
	movl	-16(%ebp), %eax
	andl	-20(%ebp), %eax
	movl	%eax, -24(%ebp)
	movl	-24(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC0, (%esp)
	call	printf
	movl	-20(%ebp), %eax
	movl	-16(%ebp), %ebx
	movl	-12(%ebp), %ecx
#APP
	shl $1, %eax
sal $16, %ecx
shr $16, %ebx

#NO_APP
	movl	%eax, -20(%ebp)
	movl	%ebx, -16(%ebp)
	movl	%ecx, -12(%ebp)
	movl	-12(%ebp), %eax
	movl	%eax, 12(%esp)
	movl	-16(%ebp), %eax
	movl	%eax, 8(%esp)
	movl	-20(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC1, (%esp)
	call	printf
	movl	$-2147483647, -20(%ebp)
	movl	$-256, -16(%ebp)
	movl	$4096, -12(%ebp)
	movl	-20(%ebp), %eax
	movl	-16(%ebp), %ebx
	movl	-12(%ebp), %ecx
#APP
	sal $1, %eax
shl $16, %ecx
sar $16, %ebx

#NO_APP
	movl	%eax, -20(%ebp)
	movl	%ebx, -16(%ebp)
	movl	%ecx, -12(%ebp)
	movl	-12(%ebp), %eax
	movl	%eax, 12(%esp)
	movl	-16(%ebp), %eax
	movl	%eax, 8(%esp)
	movl	-20(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC1, (%esp)
	call	printf
	movl	-12(%ebp), %eax
	addl	$32, %esp
	popl	%ecx
	popl	%ebx
	popl	%ebp
	leal	-4(%ecx), %esp
	ret
	.size	main, .-main
	.ident	"GCC: (GNU) 4.1.2 20080704 (Red Hat 4.1.2-48)"
	.section	.note.GNU-stack,"",@progbits
