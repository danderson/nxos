.code 32
.text

/*
 * Definition of a few constants used in the interrupt dispatchers.
 */
#define AIC_BASE 0xFFFFF000 /* Base of the AIC register bank. */
#define AIC_IVR 0x100   /* Offset of the IVR register within the AIC */
#define AIC_FVR 0x104   /* Offset of the FVR register within the AIC. */
#define AIC_EOICR 0x130 /* Offset of the EOICR register within the AIC. */

#define MC_BASE 0xFFFFFF00 /* Base of the MC register bank. */
#define MC_REMAP 0x0       /* Offset of the RCR register within the MC. */

#define FIQ_MASK 0x40   /* FIQ disable control bit in the CPSR. */
#define IRQ_MASK 0x80   /* IRQ disable control bit in the CPSR. */

#define MODE_FIQ 0x11   /* FIQ mode. */
#define MODE_IRQ 0x12   /* IRQ mode. */
#define MODE_SVC 0x13   /* Supervisor mode. */

#define RAM_START 0x00200000 /* The first address of RAM. */
#define RAM_END 0x002FFFFF  /* The last address of RAM. */
#define IRQ_STACK_SIZE 0x60 /* The size of the IRQ mode stack: 3*4*8 bytes. */


.global init_reset
.global illegal_hdl
.global swi_hdl
.global prefetch_hdl
.global data_hdl
.global reserved_hdl
.global irq_hdl
.global fiq_hdl

illegal_hdl:	b illegal_hdl 	/* Illegal instruction */
swi_hdl:	b swi_hdl	/* Software interrupt */
prefetch_hdl:	b prefetch_hdl	/* Prefetch abort */
data_hdl:	b data_hdl	/* Data abort */
reserved_hdl:	b reserved_hdl	/* Reserved vector */

/*
 * This is the FIQ handler. It simply wraps the registered FIQ
 * handling routine in the required exception handling code.
 */
fiq_hdl:

	/* Save R0 to an FIQ banked register, replace it with the AIC
 	 * register base address, and switch into supervisor (SVC) mode.
	 */
	mov r9, r0
	ldr r0, [r8, #AIC_FVR]
	msr CPSR_c, #(IRQ_MASK | FIQ_MASK | MODE_SVC)

	/* Now that we have a real stack, save the other registers
	 * we're responsible for saving (according to the ATPCS), and
	 * branch to the FIQ handler.
	 */
	stmfd sp!, {r1-r3, ip, lr}
	mov r14, pc
	bx r0

	/* Restore stacked registers, switch back into FIQ mode and
	 * resume execution.
	 */
	ldmfd sp!, {r1-r3, ip, lr}
	msr CPSR_c, #(IRQ_MASK | FIQ_MASK | MODE_FIQ)
	mov r0, r9
	subs pc, lr, #4


/*
 * IRQ exception dispatcher. This code runs when the CPU detects an
 * interrupt coming from the AIC. It switches to supervisor mode and
 * dispatches executions to the registered handler.
 */
irq_hdl:

	/* Save the return address and return processor mode registers
	 * on the IRQ stack. This is needed to allow nested interrupts,
	 * where a higher-priority interrupt halts execution of a lower
	 * priority interrupt. Also save r0, as we're going to need a
	 * scratch register.
	 */
	sub lr, lr, #4
	stmfd sp!, {lr}
	mrs lr, SPSR
	stmfd sp!, {lr}
	stmfd sp!, {r0}

	/* Get the IVR value. */
	ldr lr, =AIC_BASE
	ldr r0, [lr, #AIC_IVR]

	/* If we're in Protected mode (usually for JTAG debugging), we
	 * need to write back to the IVR register to tell the AIC it
	 * can dispatch other higher priority interrupts again.
	 * In normal mode, this has no effect, so we can safely do it.
	 */
	str lr, [lr, #AIC_IVR]

	/* Switch to Supervisor mode and reenable IRQ and FIQ handling. */
	msr CPSR_c, #MODE_SVC

	/* Dispatch the IRQ to the registed handler. */
	stmfd sp!, {r1-r3, ip, lr}
	mov lr, pc
	bx r0

	/* Restore registers and switch back to IRQ mode. */
	ldmfd sp!, {r1-r3, ip, lr}
	msr CPSR_c, #(IRQ_MASK | MODE_IRQ)
	ldmfd sp!, {r0}

	/* Tell the AIC that the interrupt has been handled. */
	str lr, [lr, #AIC_EOICR]

	/* Restore the SPSR register from the IRQ stack. */
	ldmfd sp!, {lr}
	msr SPSR_cxsf, lr

	/* Return to the normal execution flow. */
	ldmfd sp!, {pc}^


/*
 * Low level service routines that inhibit and enable interrupts in
 * the CPU. These are provided as helpers to the OS, as C code cannot
 * directly touch the core mode register.
 */

/* Disable IRQ and FIQ handling in the CPU. */
.global interrupts_disable
interrupts_disable:
	/* Get the interrupt counter and increment it. */
	adr r1, interrupts_count
	ldr r2, [r1]
	adds r2, r2, #1
	str r2, [r1]

	/* If the add didn't cause an overflow, interrupts are already
	 * disabled. Return immediately.
	 */
	bxvc lr

	/* Otherwise, disable interrupts. */
	mrs r0, CPSR
	orr r0, r0, #(IRQ_MASK | FIQ_MASK)
	msr CPSR_c, r0
	bx lr

/* Enable IRQ and FIQ handling in the CPU. */
.global interrupts_enable
interrupts_enable:
	adr r1, interrupts_count
	ldr r2, [r1]
	subs r2, r2, #1
	str r2, [r1]

	/* If we are still inside a nested interrupt disable, don't
	 * reenable interrupts. Return immediately.
	 */
	bxpl lr

	/* Otherwise, reenable interrupts. */
	mrs r0, CPSR
	bic r0, r0, #(IRQ_MASK | FIQ_MASK)
	msr CPSR_c, r0
	bx lr

interrupts_count:
	/* The byte count starts at 0, which is interrupts disabled with 1 level of nesting. Interrupts enabled is 0xFF. */
	.int 0

/*
 * Default handler for the interrupt controller. The board is
 * initialized with interrupts disabled, and this handler set for all
 * interrupts.
 */
nxt_default_irq_handler:
	b nxt_default_irq_handler
.global nxt_default_irq_handler

end:	b end


/*
 * Macros and functions to do memory initialization.
 */

	/* Copy the region from source to source_end over to dest. */
	.macro mem_copy, source, source_end, dest
	ldr r0, =\source
	ldr r1, =\source_end
	ldr r2, =\dest
	bl mem_copy_func
	.endm

mem_copy_func:
	/* Stop if source and dest addresses are the same */
	cmp r0, r2
	bxeq lr

	/* test if all addresses are 16-byte aligned, if so use ldm/stm copy */
	mov r3, r0
	orr r3, r3, r1
	orr r3, r3, r2
	ands r3, r3, #15
	beq mcf_16_aligned

	/* else use 4-byte aligned loop */
mcf_loop:
	cmp r0,r1
	ldrlo r3,[r2],#4
	strlo r3,[r0],#4
	blo mcf_loop
	bx lr

mcf_16_aligned:
	cmp r0,r1
	ldmloia r0!,{r3-r6}
	stmloia r2!,{r3-r6}
	blo mcf_16_aligned
	bx lr

	/* Initialize the region from dest to dest_end with val. */
	.macro mem_initialize, dest, dest_end, val
	ldr r0,=\dest
	ldr r1,=\dest_end
	ldr r2,=\val
	bl mem_init_func
	.endm

mem_init_func:
	orr r3,r0,r1
	ands r3,r2,#15
	beq mif_16_aligned
mif_loop:
	cmp r0,r1
	strlo r2,[r0],#4
	blo mif_loop
	bx lr

mif_16_aligned:
	mov r3,r2
	mov r4,r2
	mov r5,r2
mif_16_loop:
	cmp r0,r1
	stmloia r0!,{r2-r5}
	blo mif_16_loop
	bx lr


/*
 * Main initialization. This is where the execution starts when the
 * board is powered on.
 */
init_reset:
	/* If we're booting from SAM-BA or rebooting, interrupts are on.
	 * Turn them off.
	 */
	mrs r0, cpsr
	orr r0, r0, #(FIQ_MASK | IRQ_MASK)
	msr cpsr_c, r0

	/* Test to see whether the bottom of memory is RAM or not. If
	 * it is not, execute a remap.
	 */
	ldr  r0, =0x000000
	ldr  r1, =RAM_START

	/* Read the reset vector, then invert the bottom byte of ram
	 * and see if the reset vector has changed. If so, we're already
	 * in RAM.
	 */
	ldr  r2, [r0]
	ldr  r3, [r1]
	ldr  r4, =0xFFFFFFFF
	eor  r4, r4, r3
	str  r4, [r1]
	ldr  r4, [r0]
	str  r3, [r1]

	cmp r2, r4

	/* If the bottom byte of memory didn't change, execute a remap
	 * to put the RAM there.
	 */
	ldreq r0, =MC_BASE
	moveq r1, #0x1
	streq r1, [r0, #MC_REMAP]

	/* Set up the supervisor mode stack at the top of RAM. */
	ldr sp, =RAM_END

	/* Call the low-level initialization code. Once this has run,
	 * we're at 48MHz, and the AIC has valid (if useless) exception
	 * handlers registered.
	 */
	bl nxt_low_level_init

	/* Copy the vectors, data and ramtext sections into ram if necessary,
	 * and initialize the BSS and stack regions.
	 */
	mem_copy __vectors_load_start, __vectors_load_end, __vectors_ram_start
	mem_copy __data_load_start, __data_load_end, __data_ram_start
	mem_copy __ramtext_load_start, __ramtext_load_end, __ramtext_ram_start
	mem_initialize __bss_start, __bss_end, 0
	mem_initialize __stack_start, __stack_end, 0

	/* Switch to FIQ mode and set the r8 register with the AIC
	 * register base address.
	 */
	msr CPSR_c, #(IRQ_MASK | FIQ_MASK | MODE_FIQ)
	ldr r8, =AIC_BASE

	/* Switch to IRQ mode and set up the IRQ stack at the very top
	 * of ram. Given our IRQ dispatch code, we need 3 words per
	 * priority level, so a stack of 3*4*8 bytes.
	 */
	msr CPSR_c, #(IRQ_MASK | FIQ_MASK | MODE_IRQ)
	ldr r0, =__irq_stack
	mov sp, r0

	/* Go back to Supervisor mode and set up the Supervisor stack
	 * to start at the top of the IRQ stack.
	 */
	msr CPSR_c, #(IRQ_MASK | FIQ_MASK | MODE_SVC)
	ldr r0, =__system_stack
	mov sp, r0

	/* Set up initial frame pointers and boot the main kernel code. */
	mov r0, #0
	mov fp, r0
	mov r7, r0

	ldr r5, =kernel_main
	ldr lr, =end
	bx r5
