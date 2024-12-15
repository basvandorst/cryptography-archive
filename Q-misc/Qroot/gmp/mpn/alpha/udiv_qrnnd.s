        .set noreorder
        .set noat

.text
        .align 3
        .globl __udiv_qrnnd
        .ent __udiv_qrnnd 0
__udiv_qrnnd:
__udiv_qrnnd..ng:
        .frame $30,0,$26,0
        .prologue 0
#define cnt	$2
#define tmp	$3
#define rem_ptr	$16
#define n1	$17
#define n0	$18
#define d	$19
#define qb	$20

	ldiq	cnt,16
Loop:	cmplt	n0,0,tmp
	addq	n1,n1,n1
	bis	n1,tmp,n1
	addq	n0,n0,n0
	cmpule	d,n1,qb
	subq	n1,d,tmp
	cmovne	qb,tmp,n1
	bis	n0,qb,n0
	cmplt	n0,0,tmp
	addq	n1,n1,n1
	bis	n1,tmp,n1
	addq	n0,n0,n0
	cmpule	d,n1,qb
	subq	n1,d,tmp
	cmovne	qb,tmp,n1
	bis	n0,qb,n0
	cmplt	n0,0,tmp
	addq	n1,n1,n1
	bis	n1,tmp,n1
	addq	n0,n0,n0
	cmpule	d,n1,qb
	subq	n1,d,tmp
	cmovne	qb,tmp,n1
	bis	n0,qb,n0
	cmplt	n0,0,tmp
	addq	n1,n1,n1
	bis	n1,tmp,n1
	addq	n0,n0,n0
	cmpule	d,n1,qb
	subq	n1,d,tmp
	cmovne	qb,tmp,n1
	bis	n0,qb,n0

	subq	cnt,1,cnt
	bgt	cnt,Loop
	stq	n1,0(rem_ptr)
	bis	$31,n0,$0
	ret	$31,($26),1
	.end
