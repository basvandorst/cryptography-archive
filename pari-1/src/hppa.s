	.SHORTDATA
	.EXPORT	hiremainder
	.EXPORT	overflow
	.WORD
	.ALIGN	8
hiremainder	.WORD
	.ALIGN	8
overflow	.WORD

        .CODE
	.EXPORT	addll,ENTRY
	.EXPORT	addllx,ENTRY
	.EXPORT	subll,ENTRY
	.EXPORT	subllx,ENTRY
	.EXPORT	shiftl,ENTRY
	.EXPORT	shiftlr,ENTRY
	.EXPORT	bfffo,ENTRY
	.EXPORT	mulll,ENTRY
 	.EXPORT	addmul,ENTRY
	.EXPORT	divll,ENTRY

	.PROC
	.CALLINFO
addll	.ENTER
	ADD	%arg0,%arg1,%ret0
	ADDC	0,0,%t1
	STW	%t1,overflow-$global$(%dp)
	.LEAVE
	.PROCEND

	.PROC
	.CALLINFO
addllx	.ENTER
	LDW	overflow-$global$(%dp),%t1
	ADDB,UV	%t1,%arg0,addllx2	
	ADD	%arg0,%arg1,%ret0
	ADDC	0,0,%t1
	STW	%t1,overflow-$global$(%dp)
	.LEAVE
addllx2	LDI	1,%t1
	STW	%t1,overflow-$global$(%dp)
	.LEAVE	
	.PROCEND

	.PROC
	.CALLINFO
subll	.ENTER
	SUB	%arg0,%arg1,%ret0
	ADDC	0,0,%t1
	SUBI	1,%t1,%t1
	STW	%t1,overflow-$global$(%dp)
	.LEAVE
	.PROCEND

	.PROC
	.CALLINFO
subllx	.ENTER
	LDW	overflow-$global$(%dp),%t1
	SUB,>>=	%arg0,%arg1,%ret0
	SUB,TR	%ret0,%t1,%ret0
	SUB,>>=	%ret0,%t1,%ret0
	ADDI,TR	1,0,%t1
	LDI	0,%t1
	STW	%t1,overflow-$global$(%dp)
	.LEAVE
	.PROCEND

	.PROC
	.CALLINFO
shiftl	.ENTER
	SUBI	32,%arg1,%arg1
L$30	MFCTL	11,%t1
	MTCTL	%arg1,11
	VSHD	%arg0,0,%ret0;
	VSHD	0,%arg0,%t2
	MTCTL	%t1,11
L$31	STW	%t2,hiremainder-$global$(%dp)
	.LEAVE
	.PROCEND

	.PROC
	.CALLINFO
shiftlr	.ENTER
L$40	MFCTL	11,%t1
	MTCTL	%arg1,11
	VSHD	0,%arg0,%ret0;
	VSHD	%arg0,0,%t2
	MTCTL	%t1,11
L$41	STW	%t2,hiremainder-$global$(%dp)
	.LEAVE
	.PROCEND

	.PROC
	.CALLINFO
bfffo	.ENTER
	COMB,=,N	%r0,%arg0,L$0
	LDI	31,%ret0
	EXTRU,<>	%arg0,15,16,%r0
	SHD,TR	%arg0,%r0,16,%arg0
	ADDI	-16,%ret0,%ret0
	EXTRU,<>	%arg0,7,8,%r0
	SHD,TR	%arg0,%r0,24,%arg0
	ADDI	-8,%ret0,%ret0
	EXTRU,<>	%arg0,3,4,%r0
	SHD,TR	%arg0,%r0,28,%arg0
	ADDI	-4,%ret0,%ret0
	EXTRU,<>	%arg0,1,2,%r0
	SHD,TR	%arg0,%r0,30,%arg0
	ADDI	-2,%ret0,%ret0
	EXTRU,=	%arg0,0,1,%r0
	ADDI	-1,%ret0,%ret0
	B,N	L$1
L$0	LDI	32,%ret0
L$1	.LEAVE
	.PROCEND
	
	.PROC
	.CALLINFO
mulll	.ENTER
	LDO	hiremainder-$global$(%dp),%r1
	STW	%arg0,0(%r1)
	FLDWS	0(%r1),%fr4
	STW	%arg1,0(%r1)
	FLDWS	0(%r1),%fr5
	XMPYU	4,5,6
	FSTDS	6,0(%r1)
	LDWS	4(%r1),%ret0
	.LEAVE
	.PROCEND

	.PROC
	.CALLINFO
addmul	.ENTER
	LDO	hiremainder-$global$(%dp),%r1
	LDW	0(%r1),%t1
	STW	%arg0,0(%r1)
	FLDWS	0(%r1),%fr4
	STW	%arg1,0(%r1)
	FLDWS	0(%r1),%fr5
	XMPYU	4,5,6
	FSTDS	6,0(%r1)
	LDWS	4(%r1),%ret0
	ADD,NUV	%t1,%ret0,%ret0
	B,N	suite
	.LEAVE
suite	LDW	0(%r1),%ret1
	ADDI	1,%ret1,%ret1
	STW	%ret1,0(%r1)
	.LEAVE
	.PROCEND

hirem	.REG	%t1
loquo	.REG	%ret0
div	.REG	%arg1

nibble	.MACRO
	DS	hirem,div,hirem
	ADDC	loquo,loquo,loquo
	DS	hirem,div,hirem
	ADDC	loquo,loquo,loquo
	DS	hirem,div,hirem
	ADDC	loquo,loquo,loquo
	DS	hirem,div,hirem
	ADDC	loquo,loquo,loquo
	.ENDM
			
divll	.PROC
	.CALLINFO
	.ENTER
	LDW	hiremainder-$global$(%dp),hirem

	COMB,<	div,0,L$50
	COPY	%arg0,loquo
	SUB	0,div,%t2
	DS	0,%t2,0
	ADDC	loquo,loquo,loquo
	nibble
	nibble
	nibble
	nibble
	nibble
	nibble
	nibble
	nibble
	ADD,>=	0,hirem,0
	ADD	hirem,div,hirem
	STW	hirem,hiremainder-$global$(%dp)
	.LEAVE
	
L$50	COPY	div,%arg0
	EXTRU,<>	div,31,1,%t3
	B	L$51
	EXTRU	div,30,31,div
	ADDB,NSV	%t3,div,L$51
	COPY	hirem,%t4
	COPY	loquo,hirem
	B	L$52
	COPY	%t4,loquo
	
L$51	EXTRU	loquo,31,1,%t4
	SHD	hirem,loquo,1,loquo
	EXTRU	hirem,30,31,hirem
	SUB	0,div,%t2
	DS	0,%t2,0
	ADDC	loquo,loquo,loquo
	nibble
	nibble
	nibble
	nibble
	nibble
	nibble
	nibble
	nibble
	ADD,>=	0,hirem,0
	ADD	hirem,div,hirem
	COMB,=	0,%t3,L$53
	SH1ADD	hirem,%t4,hirem
L$52	COPY	%arg0,div
	ADDB,NUV,N	loquo,hirem,L$54
	SUB	hirem,div,hirem
	ADDI	1,loquo,loquo
L$54	COMB,<<,N	hirem,div,L$53
	SUB	hirem,div,hirem
	ADDI	1,loquo,loquo
	
L$53	STW	hirem,hiremainder-$global$(%dp)
	.LEAVE	
	.PROCEND

	.END
