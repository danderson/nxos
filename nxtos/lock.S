.code 32
.text
.align 0

	.global spinlock_acquire_from_ref
spinlock_acquire_from_ref:
	mov r1, #1
spinlock_loop:
	swpb r1, r1, [r0]
	cmp r1, #0
	bxeq lr
	b spinlock_loop


	.global spinlock_try_acquire_from_ref
spinlock_try_acquire_from_ref:
	mov r1, #1
	swpb r1, r1, [r0]
	cmp r1, #0
	mvneq r0, #0
	movne r0, #0
	bx lr
