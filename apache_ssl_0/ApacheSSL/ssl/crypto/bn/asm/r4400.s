	.file	1 "bn_mulw.c"
	.set	nobopt
	.option pic2

 # GNU C 2.6.3 [AL 1.1, MM 40] SGI running IRIX 5.0 compiled by GNU C

 # Cc1 defaults:
 # -mabicalls

 # Cc1 arguments (-G value = 0, Cpu = 6000, ISA = 2):
 # -mfp32 -mgp32 -quiet -dumpbase -mips2 -O2 -o

gcc2_compiled.:
__gnu_compiled_c:
	.rdata

	.byte	0x24,0x52,0x65,0x76,0x69,0x73,0x69,0x6f
	.byte	0x6e,0x3a,0x20,0x31,0x2e,0x34,0x39,0x20
	.byte	0x24,0x0

	.byte	0x24,0x52,0x65,0x76,0x69,0x73,0x69,0x6f
	.byte	0x6e,0x3a,0x20,0x31,0x2e,0x33,0x34,0x20
	.byte	0x24,0x0

	.byte	0x24,0x52,0x65,0x76,0x69,0x73,0x69,0x6f
	.byte	0x6e,0x3a,0x20,0x31,0x2e,0x35,0x20,0x24
	.byte	0x0

	.byte	0x24,0x52,0x65,0x76,0x69,0x73,0x69,0x6f
	.byte	0x6e,0x3a,0x20,0x31,0x2e,0x38,0x20,0x24
	.byte	0x0

	.byte	0x24,0x52,0x65,0x76,0x69,0x73,0x69,0x6f
	.byte	0x6e,0x3a,0x20,0x31,0x2e,0x32,0x33,0x20
	.byte	0x24,0x0

	.byte	0x24,0x52,0x65,0x76,0x69,0x73,0x69,0x6f
	.byte	0x6e,0x3a,0x20,0x31,0x2e,0x37,0x38,0x20
	.byte	0x24,0x0

	.byte	0x24,0x52,0x65,0x76,0x69,0x73,0x69,0x6f
	.byte	0x6e,0x3a,0x20,0x33,0x2e,0x37,0x30,0x20
	.byte	0x24,0x0

	.byte	0x24,0x52,0x65,0x76,0x69,0x73,0x69,0x6f
	.byte	0x6e,0x3a,0x20,0x31,0x2e,0x32,0x20,0x24
	.byte	0x0

	.byte	0x24,0x52,0x65,0x76,0x69,0x73,0x69,0x6f
	.byte	0x6e,0x3a,0x20,0x31,0x2e,0x34,0x20,0x24
	.byte	0x0

	.byte	0x24,0x52,0x65,0x76,0x69,0x73,0x69,0x6f
	.byte	0x6e,0x3a,0x20,0x31,0x2e,0x38,0x20,0x24
	.byte	0x0
	.text
	.align	2
	.globl	bn_mul_add_word
	.ent	bn_mul_add_word
bn_mul_add_word:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	move	$12,$4
	move	$14,$5
	move	$9,$6
	move	$13,$7
	move	$8,$0
	addu	$10,$12,12
	addu	$11,$14,12
$L2:
	lw	$6,0($14)
	#nop
	multu	$13,$6
	mfhi	$6
	mflo	$7
	#nop
	lw	$5,0($12)
	move	$3,$8
	move	$2,$0
	addu	$9,$9,-1
	move	$4,$0
	addu	$7,$7,$5
	sltu	$8,$7,$5
	addu	$6,$6,$4
	addu	$6,$6,$8
	addu	$7,$7,$3
	sltu	$4,$7,$3
	addu	$6,$6,$2
	addu	$6,$6,$4
	srl	$3,$6,0
	move	$2,$0
	move	$8,$3
	.set	noreorder
	.set	nomacro
	beq	$9,$0,$L3
	sw	$7,0($12)
	.set	macro
	.set	reorder

	lw	$6,-8($11)
	#nop
	multu	$13,$6
	mfhi	$6
	mflo	$7
	#nop
	lw	$5,-8($10)
	move	$3,$8
	move	$2,$0
	addu	$9,$9,-1
	move	$4,$0
	addu	$7,$7,$5
	sltu	$8,$7,$5
	addu	$6,$6,$4
	addu	$6,$6,$8
	addu	$7,$7,$3
	sltu	$4,$7,$3
	addu	$6,$6,$2
	addu	$6,$6,$4
	srl	$3,$6,0
	move	$2,$0
	move	$8,$3
	.set	noreorder
	.set	nomacro
	beq	$9,$0,$L3
	sw	$7,-8($10)
	.set	macro
	.set	reorder

	lw	$6,-4($11)
	#nop
	multu	$13,$6
	mfhi	$6
	mflo	$7
	#nop
	lw	$5,-4($10)
	move	$3,$8
	move	$2,$0
	addu	$9,$9,-1
	move	$4,$0
	addu	$7,$7,$5
	sltu	$8,$7,$5
	addu	$6,$6,$4
	addu	$6,$6,$8
	addu	$7,$7,$3
	sltu	$4,$7,$3
	addu	$6,$6,$2
	addu	$6,$6,$4
	srl	$3,$6,0
	move	$2,$0
	move	$8,$3
	.set	noreorder
	.set	nomacro
	beq	$9,$0,$L3
	sw	$7,-4($10)
	.set	macro
	.set	reorder

	lw	$6,0($11)
	#nop
	multu	$13,$6
	mfhi	$6
	mflo	$7
	#nop
	lw	$5,0($10)
	move	$3,$8
	move	$2,$0
	addu	$9,$9,-1
	move	$4,$0
	addu	$7,$7,$5
	sltu	$8,$7,$5
	addu	$6,$6,$4
	addu	$6,$6,$8
	addu	$7,$7,$3
	sltu	$4,$7,$3
	addu	$6,$6,$2
	addu	$6,$6,$4
	srl	$3,$6,0
	move	$2,$0
	move	$8,$3
	.set	noreorder
	.set	nomacro
	beq	$9,$0,$L3
	sw	$7,0($10)
	.set	macro
	.set	reorder

	addu	$11,$11,16
	addu	$14,$14,16
	addu	$10,$10,16
	.set	noreorder
	.set	nomacro
	j	$L2
	addu	$12,$12,16
	.set	macro
	.set	reorder

$L3:
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$8
	.set	macro
	.set	reorder

	.end	bn_mul_add_word
	.align	2
	.globl	bn_mul_word
	.ent	bn_mul_word
bn_mul_word:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	move	$11,$4
	move	$12,$5
	move	$8,$6
	move	$6,$0
	addu	$10,$11,12
	addu	$9,$12,12
$L10:
	lw	$4,0($12)
	#nop
	multu	$7,$4
	mfhi	$4
	mflo	$5
	#nop
	move	$3,$6
	move	$2,$0
	addu	$8,$8,-1
	addu	$5,$5,$3
	sltu	$6,$5,$3
	addu	$4,$4,$2
	addu	$4,$4,$6
	srl	$3,$4,0
	move	$2,$0
	move	$6,$3
	.set	noreorder
	.set	nomacro
	beq	$8,$0,$L11
	sw	$5,0($11)
	.set	macro
	.set	reorder

	lw	$4,-8($9)
	#nop
	multu	$7,$4
	mfhi	$4
	mflo	$5
	#nop
	move	$3,$6
	move	$2,$0
	addu	$8,$8,-1
	addu	$5,$5,$3
	sltu	$6,$5,$3
	addu	$4,$4,$2
	addu	$4,$4,$6
	srl	$3,$4,0
	move	$2,$0
	move	$6,$3
	.set	noreorder
	.set	nomacro
	beq	$8,$0,$L11
	sw	$5,-8($10)
	.set	macro
	.set	reorder

	lw	$4,-4($9)
	#nop
	multu	$7,$4
	mfhi	$4
	mflo	$5
	#nop
	move	$3,$6
	move	$2,$0
	addu	$8,$8,-1
	addu	$5,$5,$3
	sltu	$6,$5,$3
	addu	$4,$4,$2
	addu	$4,$4,$6
	srl	$3,$4,0
	move	$2,$0
	move	$6,$3
	.set	noreorder
	.set	nomacro
	beq	$8,$0,$L11
	sw	$5,-4($10)
	.set	macro
	.set	reorder

	lw	$4,0($9)
	#nop
	multu	$7,$4
	mfhi	$4
	mflo	$5
	#nop
	move	$3,$6
	move	$2,$0
	addu	$8,$8,-1
	addu	$5,$5,$3
	sltu	$6,$5,$3
	addu	$4,$4,$2
	addu	$4,$4,$6
	srl	$3,$4,0
	move	$2,$0
	move	$6,$3
	.set	noreorder
	.set	nomacro
	beq	$8,$0,$L11
	sw	$5,0($10)
	.set	macro
	.set	reorder

	addu	$9,$9,16
	addu	$12,$12,16
	addu	$10,$10,16
	.set	noreorder
	.set	nomacro
	j	$L10
	addu	$11,$11,16
	.set	macro
	.set	reorder

$L11:
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$6
	.set	macro
	.set	reorder

	.end	bn_mul_word
	.align	2
	.globl	bn_sqr_words
	.ent	bn_sqr_words
bn_sqr_words:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	move	$9,$4
	addu	$7,$9,28
	addu	$8,$5,12
$L18:
	lw	$2,0($5)
	#nop
	multu	$2,$2
	mfhi	$2
	mflo	$3
	#nop
	addu	$6,$6,-1
	sw	$3,0($9)
	srl	$3,$2,0
	move	$2,$0
	.set	noreorder
	.set	nomacro
	beq	$6,$0,$L19
	sw	$3,-24($7)
	.set	macro
	.set	reorder

	lw	$2,-8($8)
	#nop
	multu	$2,$2
	mfhi	$2
	mflo	$3
	#nop
	addu	$6,$6,-1
	sw	$3,-20($7)
	srl	$3,$2,0
	move	$2,$0
	.set	noreorder
	.set	nomacro
	beq	$6,$0,$L19
	sw	$3,-16($7)
	.set	macro
	.set	reorder

	lw	$2,-4($8)
	#nop
	multu	$2,$2
	mfhi	$2
	mflo	$3
	#nop
	addu	$6,$6,-1
	sw	$3,-12($7)
	srl	$3,$2,0
	move	$2,$0
	.set	noreorder
	.set	nomacro
	beq	$6,$0,$L19
	sw	$3,-8($7)
	.set	macro
	.set	reorder

	lw	$2,0($8)
	#nop
	multu	$2,$2
	mfhi	$2
	mflo	$3
	#nop
	addu	$6,$6,-1
	sw	$3,-4($7)
	srl	$3,$2,0
	move	$2,$0
	.set	noreorder
	.set	nomacro
	beq	$6,$0,$L19
	sw	$3,0($7)
	.set	macro
	.set	reorder

	addu	$8,$8,16
	addu	$5,$5,16
	addu	$7,$7,32
	.set	noreorder
	.set	nomacro
	j	$L18
	addu	$9,$9,32
	.set	macro
	.set	reorder

$L19:
	j	$31
	.end	bn_sqr_words
