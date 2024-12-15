.rdata
	.align	3
$C34:
	.t_floating 1.84467440737095516160e19
	.align	3
$C35:
	.t_floating 3.40282366920938463463e38
.text
	.align	3
	.globl	invert_normalized_limb_with_umul_ppmm
	.ent	invert_normalized_limb_with_umul_ppmm 0
invert_normalized_limb_with_umul_ppmm:
	ldgp	$29,0($27)
invert_normalized_limb_with_umul_ppmm..ng:
	lda	$30,-16($30)
	.frame	$30,16,$26,0
	stq	$26,8($30)
	.mask	0x4000000,-8
	.prologue 1
	stq	$16,0($30)
	ldt	$f11,0($30)
	cvtqt	$f11,$f10
	bge	$16,$56
	lda	$1,$C34
	ldt	$f1,0($1)
	addt	$f10,$f1,$f10
$56:
	lda	$1,$C35
	ldt	$f1,0($1)
	divt	$f1,$f10,$f1
	cvttqc	$f1,$f1
	stt	$f1,0($30)
	addq	$16,$16,$1
	cmpult	$31,$1,$1
	ldq	$0,0($30)
	bis	$31,2,$3
	subq	$31,$1,$1
	and	$0,$1,$2
	ornot	$2,$1,$0
	umulh	$0,$0,$1
	addq	$1,$0,$1
	cmpule	$0,$1,$2
	addq	$1,$0,$1
	cmovne	$2,1,$3
	cmpult	$1,$0,$2
	addq	$3,$2,$3
	subq	$3,2,$2
	umulh	$1,$16,$4
	beq	$2,$68
	cmpule	$3,2,$1
	beq	$1,$72
	subq	$3,1,$1
	beq	$1,$66
	br	$31,$70
	.align	4
$72:
	subq	$3,3,$1
	beq	$1,$69
	br	$31,$70
	.align	4
$68:
	addq	$16,$16,$16
	br	$31,$66
	.align	4
$69:
	s4subq	$16,$16,$16
	br	$31,$66
	.align	4
$70:
	lda	$27,abort
	jsr	$26,($27),abort
	ldgp	$29,0($26)
	.align	4
$66:
	addq	$4,$16,$4
	addq	$0,$0,$0
	subq	$0,$4,$0
	ldq	$26,8($30)
	lda	$30,16($30)
	ret	$31,($26),1
	.end	invert_normalized_limb_with_umul_ppmm
