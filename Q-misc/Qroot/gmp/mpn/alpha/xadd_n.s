 # This code works only when size a multiple of 8.
 # It should run with 5 cycles/limb, but for some reason it takes 7.

	.set noreorder
	.set noat

.text
	.align	3
	.globl	mpn_add_n
	.ent	mpn_add_n 2
mpn_add_n:
	.frame	$30,0,$26
	.align 4
	bis	$31,$31,$0
Loop:
	ldq	$1,0($18)
	subq	$19,8,$19
	ldq	$2,0($17)

	ldq	$3,8($18)
	addq	$1,$0,$1
	ldq	$4,8($17)
	addq	$1,$2,$2
	cmpult	$1,$0,$0
	cmpult	$2,$1,$1
	stq	$2,0($16)
	addq	$0,$1,$0	# combine carries

	ldq	$1,16($18)
	addq	$3,$0,$3
	ldq	$2,16($17)
	addq	$3,$4,$4
	cmpult	$3,$0,$0
	cmpult	$4,$3,$3
	stq	$4,8($16)
	addq	$0,$3,$0	# combine carries

	ldq	$3,24($18)
	addq	$1,$0,$1
	ldq	$4,24($17)
	addq	$1,$2,$2
	cmpult	$1,$0,$0
	cmpult	$2,$1,$1
	stq	$2,16($16)
	addq	$0,$1,$0	# combine carries

	ldq	$1,32($18)
	addq	$3,$0,$3
	ldq	$2,32($17)
	addq	$3,$4,$4
	cmpult	$3,$0,$0
	cmpult	$4,$3,$3
	stq	$4,24($16)
	addq	$0,$3,$0	# combine carries

	ldq	$3,40($18)
	addq	$1,$0,$1
	ldq	$4,40($17)
	addq	$1,$2,$2
	cmpult	$1,$0,$0
	cmpult	$2,$1,$1
	stq	$2,32($16)
	addq	$0,$1,$0	# combine carries

	ldq	$1,48($18)
	addq	$3,$0,$3
	ldq	$2,48($17)
	addq	$3,$4,$4
	cmpult	$3,$0,$0
	cmpult	$4,$3,$3
	stq	$4,40($16)
	addq	$0,$3,$0	# combine carries

	ldq	$3,56($18)
	addq	$1,$0,$1
	ldq	$4,56($17)
	addq	$1,$2,$2
	cmpult	$1,$0,$0
	cmpult	$2,$1,$1
	stq	$2,48($16)
	addq	$0,$1,$0	# combine carries

	addq	$3,$0,$3
	addq	$3,$4,$4
	cmpult	$3,$0,$0
	cmpult	$4,$3,$3
	stq	$4,56($16)
	addq	$0,$3,$0	# combine carries

	addq	$16,64,$16
	addq	$17,64,$17
	addq	$18,64,$18
	bgt	$19,Loop

	ret	$31,($26),1
	.end	mpn_add_n
