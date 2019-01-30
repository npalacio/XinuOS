/* setjmp.S - setjmp, longjmp					*/

/* This implementation of setjmp and longjmp are intended specifically	   */
/* for C programs compiled using gcc for the Xinu system.		   */
/*									   */
/*  The definition of jmp_buf is expected to be:			   */
/*									   */
/*	typedef int jmp_buf[6];						   */
/*									   */
/* When setjmp is invoked, the general purpose register values %eax, %ecx, */
/* %edx, %ebx, %esp, %ebp, %esi, and %edi are saved in the 8 elements of   */
/* the array passed as an argument.					   */

	.text
	.globl	setjmp
	.globl	longjmp

/*--------------------------
 * int32 setjmp(jmp_buf env)
 *--------------------------
 */
setjmp:
	xorl	%eax,%eax
	movl	4(%esp),%edx		/* get ptr to jmp_buf array base */
	movl	%ebx,0(%edx)		/* save %ebx register		 */
	movl	%esi,4(%edx)		/* save %esi register		 */
	movl	%edi,8(%edx)		/* save %edi register		 */
	leal	4(%esp),%ecx		/* get stack addr of jmp_buf arg */
	movl	%ecx,16(%edx)		/* save it			 */
	movl	(%esp),%ecx		/* get return address		 */
	movl	%ecx,20(%edx)		/* save it			 */
	movl	%ebp,12(%edx)
	ret

/* On entry to setjmp, the stack looks like this:			 */
/*									 */
/*	+------+							 */
/*	| addr |							 */
/*	+------+							 */
/*	| retn | <-- %esp						 */
/*	*------+							 */

/* The jmp_buf contents look like this:					 */
/*									 */
/*	+------+							 */
/*	| retn | 20							 */
/*	+------+							 */
/*	| stak | 16 (addr of original call's jmp_buf argument)		 */
/*	+------+							 */
/*	| %ebp | 12							 */
/*	+------+							 */
/*	| %edi | 8							 */
/*	+------+							 */
/*	| %esi | 4							 */
/*	+------+							 */
/*	| %ebx | 0 <-- %edx						 */
/*	+------+							 */

/*-------------------------------------
 * void longjmp(jmp_buf env, int32 val)
 *-------------------------------------
 */
longjmp:
	movl	4(%esp),%ecx		/* jmp_buf ptr to %ecx		 */
	movl	8(%esp),%eax		/* return value			 */
	movl	20(%ecx),%edx		/* return address tp %edx	 */
	movl	(%ecx),%ebx		/* restore %ebx			 */
	movl	4(%ecx),%esi		/* restore %esi			 */
	movl	8(%ecx),%edi		/* restore %edi			 */
	movl	12(%ecx),%ebp		/* restore %ebp			 */
	movl	16(%ecx),%esp		/* restore %esp			 */
	jmp	*%edx			/* reture to caller		 */
