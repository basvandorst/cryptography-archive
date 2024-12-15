 #
 # This is quick hackup of the routines found in bn_mulw.c
 # 20 Dec 95 - eay
 #
	.verstamp 3 11 2 7 0
	.set noreorder
	.set volatile
	.set noat
	.file	1 "alpha2.s"
gcc2_compiled.:
__gnu_compiled_c:
.text
	.align 3
	.globl bn_mul_add_word
	.ent bn_mul_add_word
bn_mul_add_word:
bn_mul_add_word..ng:
	.frame $30,0,$26,0
	.prologue 0
	subq $18,2,$25	# num=-2
	bis $31,$31,$0
	blt $25,$42
	.align 5
$142:
	subq $18,2,$18	# num-=2
	subq $25,2,$25	# num-=2

	ldq $1,0($17)	# a[0]
	ldq $2,8($17)	# a[1]

	mulq $19,$1,$3	# a[0]*w low part	r3
 	umulh $19,$1,$1 # a[0]*w high part	r1
	mulq $19,$2,$4	# a[1]*w low part	r4
 	umulh $19,$2,$2 # a[1]*w high part	r2

	ldq $22,0($16)	# r[0]			r22
	ldq $23,8($16)	# r[1]			r23

	addq $3,$22,$3	# a0 low part + r[0]	
	addq $4,$23,$4	# a1 low part + r[1]	
	cmpult $3,$22,$5 # overflow?
	cmpult $4,$23,$6 # overflow?
	addq $5,$1,$1	# high part + overflow 
	addq $6,$2,$2	# high part + overflow 

	addq $3,$0,$3	# add c
	cmpult $3,$0,$5 # overflow?
	stq $3,0($16)
	addq $5,$1,$0	# c=high part + overflow 

	addq $4,$0,$4	# add c
	cmpult $4,$0,$5 # overflow?
	stq $4,8($16)
	addq $5,$2,$0	# c=high part + overflow 

	ble $18,$43

 	addq $16,16,$16
 	addq $17,16,$17
	blt $25,$42

 	br $31,$142
$42:
	ldq $1,0($17)	# a[0]
 	umulh $19,$1,$3 # a[0]*w high part
	mulq $19,$1,$1	# a[0]*w low part
	ldq $2,0($16)	# r[0]
	addq $1,$2,$1	# low part + r[0]
	cmpult $1,$2,$4 # overflow?
	addq $4,$3,$3	# high part + overflow 
	addq $1,$0,$1	# add c
	cmpult $1,$0,$4 # overflow?
	addq $4,$3,$0	# c=high part + overflow 
	stq $1,0($16)

	.align 4
$43:
	ret $31,($26),1
	.end bn_mul_add_word
	.align 3
	.globl bn_mul_word
	.ent bn_mul_word
bn_mul_word:
bn_mul_word..ng:
	.frame $30,0,$26,0
	.prologue 0
	subq $18,2,$25	# num=-2
	bis $31,$31,$0
	blt $25,$242
	.align 5
$342:
	subq $18,2,$18	# num-=2
	subq $25,2,$25	# num-=2

	ldq $1,0($17)	# a[0]
	ldq $2,8($17)	# a[1]

	mulq $19,$1,$3	# a[0]*w low part	r3
 	umulh $19,$1,$1 # a[0]*w high part	r1
	mulq $19,$2,$4	# a[1]*w low part	r4
 	umulh $19,$2,$2 # a[1]*w high part	r2

	addq $3,$0,$3	# add c
	cmpult $3,$0,$5 # overflow?
	stq $3,0($16)
	addq $5,$1,$0	# c=high part + overflow 

	addq $4,$0,$4	# add c
	cmpult $4,$0,$5 # overflow?
	stq $4,8($16)
	addq $5,$2,$0	# c=high part + overflow 

	ble $18,$243

 	addq $16,16,$16
 	addq $17,16,$17
	blt $25,$242

 	br $31,$342
$242:
	ldq $1,0($17)	# a[0]
 	umulh $19,$1,$3 # a[0]*w high part
	mulq $19,$1,$1	# a[0]*w low part
	addq $1,$0,$1	# add c
	cmpult $1,$0,$4 # overflow?
	addq $4,$3,$0	# c=high part + overflow 
	stq $1,0($16)
$243:
	ret $31,($26),1
	.end bn_mul_word
	.align 3
	.globl bn_sqr_words
	.ent bn_sqr_words
bn_sqr_words:
bn_sqr_words..ng:
	.frame $30,0,$26,0
	.prologue 0
	
	subq $18,2,$25	# num=-2
	blt $25,$442
	.align 5
$542:
	subq $18,2,$18	# num-=2
	subq $25,2,$25	# num-=2

	ldq $1,0($17)	# a[0]
	ldq $4,8($17)	# a[1]

	mulq $1,$1,$2	# a[0]*w low part	r2
 	umulh $1,$1,$3 # a[0]*w high part	r3
	mulq $4,$4,$5	# a[1]*w low part	r5
 	umulh $4,$4,$6 # a[1]*w high part	r6

	stq $2,0($16)	# r[0]
	stq $3,8($16)	# r[1]
	stq $5,16($16)	# r[3]
	stq $6,24($16)	# r[4]

	ble $18,$443

 	addq $16,32,$16
 	addq $17,16,$17
	blt $25,$442
 	br $31,$542

$442:
	ldq $1,0($17)   # a[0]
	mulq $1,$1,$2   # a[0]*w low part       r2
        umulh $1,$1,$3  # a[0]*w high part       r3
	stq $2,0($16)   # r[0]
        stq $3,8($16)   # r[1]

	.align 4
$443:
	ret $31,($26),1
	.end bn_sqr_words
