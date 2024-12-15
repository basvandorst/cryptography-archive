	.SPACE	$TEXT$
	.SUBSPA	$CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.SPACE	$TEXT$
	.SUBSPA	$CODE$

	.align	4
	.EXPORT	count_leading_zeros,CODE
	.EXPORT	count_leading_zeros,ENTRY,PRIV_LEV=3,ARGW0=GR,RTNVAL=GR
count_leading_zeros
	.PROC
	.CALLINFO	FRAME=0,NO_CALLS
	.ENTRY

	ldi		1,%r28

	extru,=		%r26,15,16,%r0		; Bits 31..16 zero?
	extru,tr	%r26,15,16,%r26		; No.  Shift down, skip add.
	ldo		16(%r28),%r28		; Yes.  Perform add.

	extru,=		%r26,23,8,%r0		; Bits 15..8 zero?
	extru,tr	%r26,23,8,%r26		; No.  Shift down, skip add.
	ldo		8(%r28),%r28		; Yes.  Perform add.

	extru,=		%r26,27,4,%r0		; Bits 7..4 zero?
	extru,tr	%r26,27,4,%r26		; No.  Shift down, skip add.
	ldo		4(%r28),%r28		; Yes.  Perform add.

	extru,=		%r26,29,2,%r0		; Bits 3..2 zero?
	extru,tr	%r26,29,2,%r26		; No.  Shift down, skip add.
	ldo		2(%r28),%r28		; Yes.  Perform add.

	extru		%r26,30,1,%r26		; Extract bit 1.
	bv		0(2)
	 sub		%r28,%r26,%r28		; Subtract it.

	.EXIT
	.PROCEND
