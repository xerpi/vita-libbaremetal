	.cpu cortex-a9
	.align 4
	.code 32

	.text

	.global _start

# r0 = Sysroot buffer paddr
_start:
	# Disable interrupts and enter System mode
	cpsid	aif, #0x1F

	# Get CPU ID
	mrc	p15, 0, r1, c0, c0, 5
	ands	r1, #0xF

	# Setup the SP at the end of the scratchpad
	mov	sp, #0x00008000
	sub	sp, r1, lsl #13

	# CPU0 clears BSS, others wait
	cmp	r1, #0
	bne	wait_bss_init

	# Clear BSS
	ldr	r2, =_bss_start
	ldr	r3, =_bss_end
	mov	r4, #0
1:
	cmp	r2, r3
	beq	bss_clear_done
	str	r4, [r2], #4
	b	1b

bss_clear_done:
	mov	r2, #1
	ldr	r3, =bss_clear_done_flag
	str	r2, [r3]
	sev
	b	b_main

wait_bss_init:
	ldr	r2, =bss_clear_done_flag
2:
	wfe
	ldr	r3, [r2]
	cmp	r3, #0
	beq	2b

b_main:
	# Jump to main(sysroot, cpu_id)
	blx	main

cpu_park:
	wfi
	b	cpu_park

	.data

bss_clear_done_flag:
	.word 0
