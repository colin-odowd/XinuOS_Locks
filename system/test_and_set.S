/* test_and_set.S - test and set (for x86) */

		.text
		.globl	test_and_set

/*------------------------------------------------------------------------------------------
 * test_and_set -  X86 test and set; the call is test_and_set(uint32 *ptr, uint32 new_value)
 *------------------------------------------------------------------------------------------
 */
test_and_set:
		pushl %ebp				/* save the old base pointer */
		movl  %esp, %ebp		/* set the new base pointer to address of stack pointer */
		pushl %ebx				/* save the old ebx register */
		pushl %edi				/* save the old edi register */
		pushl %esi				/* save the old esi register */

		/* uint32 old_value = *ptr; */
		/* *ptr = new_value; */ 

		movl  12(%ebp), %eax    /* set eax to value from second argument (new_value) */
		movl  8(%ebp), %ecx     /* set ecx to address from first argument (ptr) */
		xchgl %eax, (%ecx)		/* exchange the values from new_value and ptr */

		popl %esi				/* restore the old esi register */
		popl %edi				/* restore the old edi register */
		popl %ebx				/* restore the old ebx register */
		popl %ebp				/* restore the old ebp regsiter */

		/* return old_value; */ 
		ret						/* return with the old_value stored in eax */