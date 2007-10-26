.code 32
.text
.align 0

#include "asm_decls.h"

	.global run_first_task
	.extern main_returned
run_first_task:
	stmfd sp!, {lr}

	/* We need to disable interrupts when first switching to system mode, to
	 * correctly set up the stack. Once it is set up, we switch to system mode
	 * "properly", preserving the interrupt state we were originally given.
	 *
	 * Note that the ORR below only works properly because the bitmask for
	 * system mode is 0b11111.
	 */
	mrs r2, cpsr
	orr r2, r2, #MODE_SYS
	msr cpsr_c, #(MODE_SYS | IRQ_FIQ_MASK)

	/* Set up the system stack, switch into system mode properly, and jump into
	 * the task.
	 */
	mov sp, r1
	msr cpsr_c, r2
	mov lr, pc
	bx r0

	/* Switch back to supervisor mode and return. */
	mrs r1, cpsr
	bic r1, r1, #MODE_SYS
	orr r1, r1, #MODE_SVC
	msr cpsr_c, r1
	ldmfd sp!, {pc}
