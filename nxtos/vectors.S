	.code 32
	.align 	0
	.section ".vectors"

@ What follows is the exception vectors for. They are placed at the
@ bottom of memory at system start up, and just call into exception
@ handlers in crt0_s.S.
@
@ LDR is used instead of branching because LDR jumps can be relocated.

	ldr   pc, v0		@ Reset
        ldr   pc, v1		@ Undefined Instruction
        ldr   pc, v2		@ Software Interrupt
        ldr   pc, v3		@ Prefetch Abort
        ldr   pc, v4		@ Data Abort
        ldr   pc, v5		@ Reserved
	ldr   pc, v6		@ IRQ
	ldr   pc, v7		@ FIQ

v0:	.long init_reset
v1:	.long illegal_hdl
v2:     .long swi_hdl
v3:     .long prefetch_hdl
v4:	.long data_hdl
v5:	.long reserved_hdl
v6:	.long irq_hdl
v7:	.long fiq_hdl
