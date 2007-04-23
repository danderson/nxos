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
