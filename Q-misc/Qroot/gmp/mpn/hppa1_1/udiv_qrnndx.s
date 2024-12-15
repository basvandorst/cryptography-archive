	.SPACE $PRIVATE$
	.SPACE $TEXT$
	.SUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY

	.align 8
LC$0000
	; .double 1.84467440737095520000e+19
	.word 1139802112 ; = 0x43f00000
	.word 0 ; = 0x0

	.align 4
	.export __hppa_udiv_qrnnd,CODE
	.export __hppa_udiv_qrnnd,ENTRY,PRIV_LEV=3,ARGW0=GR,ARGW1=GR,ARGW2=GR,ARGW3=GR,RTNVAL=GR
__hppa_udiv_qrnnd
	.proc
	.callinfo frame=64,no_calls
	.entry
	ldo		64(%r30),%r30

	stws		%r25,-16(0,%r30)	; n_hi
	stws		%r24,-12(0,%r30)	; n_lo
	ldil		L'LC$0000,%r19
	ldo		R'LC$0000(%r19),%r19
	fldds		-16(0,%r30),%fr5
	stws		%r23,-12(0,%r30)
	comib,<=	0,%r25,L1
	fcnvxf,dbl,dbl	%fr5,%fr5
	fldds		0(0,%r19),%fr4
	fadd,dbl	%fr4,%fr5,%fr5
L1
	fcpy,sgl	%fr0,%fr6L
	fldws		-12(0,%r30),%fr6R
	fcnvxf,dbl,dbl	%fr6,%fr4

	fdiv,dbl	%fr5,%fr4,%fr5

	fcnvfx,dbl,dbl	%fr5,%fr4
	fstws		%fr4R,-16(%r30)
	xmpyu		%fr4R,%fr6R,%fr6
	ldws		-16(%r30),%r28
	fstds		%fr6,-16(0,%r30)
	ldws		-12(0,%r30),%r21
	ldws		-16(0,%r30),%r20
	sub		%r24,%r21,%r22
	subb		%r25,%r20,%r19
	comib,=		0,%r19,L2
	ldo		-64(%r30),%r30

	add		%r22,%r23,%r22
	ldo		-1(%r28),%r28
	bv		0(2)
	stws		%r22,0(0,%r26)

L2	comb,<<		%r22,%r23,L5
	nop
	.import	abort,code
	b abort

	sub		%r22,%r23,%r22
	ldo		1(%r28),%r28
L5	bv		0(2)
	stws		%r22,0(0,%r26)
	.exit
	.procend
