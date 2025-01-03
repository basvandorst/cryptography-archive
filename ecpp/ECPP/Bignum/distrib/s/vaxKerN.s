# Copyright     Digital Equipment Corporation & INRIA     1988, 1989, 1990
#
#		KerN for the VAX.
#		[Bepaul, Shand]
#		Last modified on Mon Apr  2 21:03:05 GMT+2:00 1990 by shand 
#		     modified on Mon Nov 20 13:51:10 GMT+1:00 1989 by herve 
#		     modified on 17-OCT-1989 20:37:48 by Jim Lawton 
#
# >>> IMPORTANT <<<     DO NOT MODIFY THIS LINE
#
# >>> READ THIS <<<
#
# This file is automatically converted from unix to VAX/VMS assembler format.
# On VMS it is the callee's rsponsiblity to save all modified registers
# other than r0 and r1.  On Ultrix r0-r5 are considered saved by caller.
# Specify procedure entry masks that save ALL modified registers (including
# r0 and r1) and "&" them with "callee_save" which is a predefined constant
# that eliminates the saves which are unnecessary under whichever calling
# convention the file is being assembler for.
.set callee_save,~63
	# WARNING: text after comment used in conversion to VMS format assembler
		.data	# vaxKerN_data
_copyright:	.ascii "@(#)vaxKerN.s: copyright Digital Equipment Corporation & INRIA 1988, 1989, 1990\12\0"
	# WARNING: text after comment used in conversion to VMS format assembler
		.text	# vaxKerN
		.globl	_BnnSetToZero
		.align	3
_BnnSetToZero:
		.word	0x3&callee_save	# mask<r0,r1>
		movl	4(ap),r0	# nn
		movl	8(ap),r1	# nl
		sobgeq	r1,LSTZ1	# if(nl--) goto LSTZ1
		ret			# return;
LSTZ1:		clrl	(r0)+		# *(nn++) = 0;
		sobgeq	r1,LSTZ1	# if(nl--) goto LSTZ1;
		ret

		.globl	_BnnAssign
		.align	3
_BnnAssign:
		.word	0x7&callee_save	# mask<r0,r1,r2>
		movl	4(ap),r0	# mm
		movl	8(ap),r1	# nn
		movl	12(ap),r2	# nl
		cmpl	r0,r1
		bgequ	LAG2		# if(mm >= nn) goto LAG2;
		sobgeq	r2,LAG1		# if(nl--) goto LAG1;
		ret			# return;
LAG1:		movl	(r1)+,(r0)+	# *(mm++) = *(nn++);
		sobgeq	r2,LAG1		# if(nl--) goto LAG1;
LAG2:		blequ	LAG4		# if(mm <= nn) goto LAG4;
		moval	(r0)[r2],r0	# mm = &mm[nl];
		moval	(r1)[r2],r1	# nn = &nn[nl];
		sobgeq	r2,LAG3		# if(nl--) goto LAG3;
		ret			# return;
LAG3:		movl	-(r1),-(r0)	# *(--mm) = *(--nn);
		sobgeq	r2,LAG3		# if(nl--) goto LAG3;
LAG4:		ret			# return;

		.globl	_BnnSetDigit
		.align	3
_BnnSetDigit:
		.word	0x0&callee_save	# mask<>
		movl	8(ap),*4(ap)	# *nn = d;
		ret

		.globl	_BnnGetDigit
		.align	3
_BnnGetDigit:
		.word	0x0&callee_save	# mask<>
		movl	*4(ap),r0	# return(*nn);
		ret

		.globl	_BnnNumDigits
		.align	3
_BnnNumDigits:
		.word	0x2&callee_save	# mask<r1>
		movl	8(ap),r0	# nl
		moval	*4(ap)[r0],r1	# nn = &nn[nd];
		sobgeq	r0,LND1		# if(nl-- != 0) goto LND1;
		movl	$1,r0
		ret			# return(1);
LND1:		tstl	-(r1)
		bneq	LND3		# if(*(--n) != 0) goto LND3;
		sobgeq	r0,LND1		# if(nl-- != 0) goto LND1;
		movl	$1,r0
		ret			# return(1);
LND3:		incl	r0
		ret			# return(nl + 1);

		.globl	_BnnNumLeadingZeroBitsInDigit
		.align	3
_BnnNumLeadingZeroBitsInDigit:
		.word	0x2&callee_save	# mask<r1>
		movl	4(ap),r1	# d
		movl	$31,r0
LLZ1:		bbs	r0,r1,LLZ2
		sobgeq	r0,LLZ1
LLZ2:		subl3	r0,$31,r0
		ret

		.globl	_BnnDoesDigitFitInWord
		.align	3
_BnnDoesDigitFitInWord:
		.word	0x0&callee_save	# mask<>
		movl	$1,r0		# C_VERSION
		ret

		.globl	_BnnIsDigitZero
		.align	3
_BnnIsDigitZero:
		.word	0x2&callee_save	# mask<r1>
		tstl	4(ap)		# d
		bneq	LDZ1		# if(d) goto LDZ1;
		movl	$1,r0
		ret			# return(1);
LDZ1:		clrl	r0
		ret			# return(0);

		.globl	_BnnIsDigitNormalized
# Boolean BnIsDigitNormalized(n, nd) BigNum n; int nd; {
		.align	3
_BnnIsDigitNormalized:
		.word	0x0&callee_save	# mask<>
		movl	4(ap),r0	# d
		extzv	$31,$1,r0,r0	# return(d >> 31);
		ret

		.globl	_BnnIsDigitOdd
		.align	3
_BnnIsDigitOdd:
		.word	0x0&callee_save	# mask<>
		bicl3	$-2,4(ap),r0	# return(d || 1);
		ret

		.globl	_BnnCompareDigits
		.align	3
_BnnCompareDigits:
		.word	0x0&callee_save	# mask<>
		cmpl	4(ap),8(ap)	# cmpl d1,d2
		beql	LCDeq		# if(d0 == d1) goto LCDeq
		blssu	LCDinf		# if(d0 < d1) goto LCDinf
		movl	$1,r0		# return(1);
		ret
LCDeq:		clrl	r0		# return(0);
		ret
LCDinf:		movl	$-1,r0		# return(-1);
		ret

		.globl	_BnnComplement
		.align	3
_BnnComplement:
		.word	0x2&callee_save	# mask<r1>
		movl	4(ap),r0	# nn
		movl	8(ap),r1	# nl
		sobgeq	r1,LCM1		# if(nl-- != 0) goto LCM1;
		ret
LCM1:		mcoml	(r0),(r0)+	# *(n++) ^= -1;
		sobgeq	r1,LCM1		# if(nl-- != 0) goto LCM1;
		ret

		.globl	_BnnAndDigits
		.align	3
_BnnAndDigits:
		.word	0x0&callee_save	# mask<>
		mcoml	8(ap),r0	# d = ~d;
		bicl2	r0,*4(ap)	# *nn &= ~d;
		ret

		.globl	_BnnOrDigits
		.align	3
_BnnOrDigits:
		.word	0x0&callee_save	# mask<>
		bisl2	8(ap),*4(ap)	# *nn |= d;
		ret

		.globl	_BnnXorDigits
		.align	3
_BnnXorDigits:
		.word	0x0&callee_save	# mask<>
		xorl2	8(ap),*4(ap)	# *nn ^= d;
		ret

		.globl	_BnnShiftLeft
		.align	3
_BnnShiftLeft:
		.word	0x7E&callee_save	# mask<r1,r2,r3,r4,r5,r6>
		clrl	r0		# res = 0;
		movl	12(ap),r3	# nbi
		bneq	LSL0		# if(nbi) goto LSL0
		ret			# return(res);
LSL0:		movl	4(ap),r2	# mm
		movl	8(ap),r1	# ml
		subl3	r3,$32,r4	# rnbi = BN_DIGIT_SIZE - nbi;
		sobgeq	r1,LSL1		# if(ml-- != 0) goto LSL1;
		ret			# return(res);
LSL1:		movl	(r2),r5		# save = *mm
		ashl	r3,r5,r6	# X = save << nbi;
		bisl3	r0,r6,(r2)+	# *(mm++) = X | res;
		extzv	r4,r3,r5,r0	# res = save >> rnbits;
		sobgeq	r1,LSL1		# if(ml-- != 0) goto LSL1;
		ret			# return(res);

		.globl	_BnnShiftRight
		.align	3
_BnnShiftRight:
		.word	0x7E&callee_save	# mask<r1,r2,r3,r4,r5,r6>
		clrl	r0		# res = 0;
		movl	12(ap),r3	# nbi
		bneq	LSR0		# if(nbi) goto LSR0;
		ret			# return(res);
LSR0:		movl	8(ap),r1	# ml
		moval	*4(ap)[r1],r2	# mm = &mm[ml];
		subl3	r3,$32,r4	# lnbi = BN_DIGIT_SIZE - nbi;
		sobgeq	r1,LSR1		# if(ml-- != 0) goto LSR1;
		ret			# return(res);
LSR1:		movl	-(r2),r5	# save = *(--mm);
		extzv	r3,r4,r5,r6	# X = save >> nbi;
		bisl3	r0,r6,(r2)	# *mm = X | res;
		ashl	r4,r5,r0	# res = save << lnbi;
		sobgeq	r1,LSR1		# if(ml-- != 0) goto LSR1;
		ret			# return(res);

		.globl	_BnnAddCarry
		.align	3
_BnnAddCarry:
		.word	0x2&callee_save	# mask<r1>
		movl	12(ap),r0	# car
		beql	LAC3		# if(car == 0) return(car);
		movl	8(ap),r0	# nl
		beql	LAC2		# if(nl == 0) return(1);
		movl	4(ap),r1	# nn
LAC1:		incl	(r1)+		# ++(*nn++);
		bcc	LAC4		# if(!Carry) goto LAC4
		sobgtr	r0,LAC1		# if(--nl > 0) goto LAC1;
LAC2:		movl	$1,r0		# return(1);
LAC3:		ret
LAC4:		clrl	r0		# return(0);
		ret

		.globl	_BnnAdd
		.align	3
_BnnAdd:
		.word	0x1E&callee_save	# mask<r1,r2,r3,r4>
LADDEntry:	movl	4(ap),r0	# mm
		movl	12(ap),r1	# nn
		movl	16(ap),r3	# nl
		bneq	LADD1		# if(nl) goto LADD1
		subl3	r3,8(ap),r2	# ml -= nl;
		tstl	20(ap)		# car
		bneq	LADD5		# if(car) goto LADD5
		clrl	r0
		ret			# return(0);
LADD1:		subl3	r3,8(ap),r2	# ml -= nl;
		addl3	20(ap),$-1,r4	# C = car

LADD2:		adwc	(r1)+,(r0)+	# *(m++) += *(n++) + C;
LADD3:		sobgtr	r3,LADD2	# if(--nl > 0) goto LADD2;
		bcs	LADD5		# if(C) goto LADD5;
LADD4:		clrl	r0
		ret

LADD6:		incl	(r0)+		# ++(*m++);
		bcc	LADD4		# if(!C) goto LADD4;
LADD5:		sobgeq	r2,LADD6	# if(--ml >= 0) goto LADD6;
LADD7:		movl	$1,r0
		ret

		.globl	_BnnSubtractBorrow
		.align	3
_BnnSubtractBorrow:
		.word	0x2&callee_save	# mask<r1>
		movl	12(ap),r0	# car
		bneq	LSB2		# if(car) return(car);
		movl	8(ap),r0	# nl
		beql	LSB20		# if(nl == 0) return(0);
		movl	4(ap),r1	# nn
LSB1:		decl	(r1)+		# (*nn++)--;
		bcc	LSB3		# if(!Carry) goto LSB3;
		sobgtr	r0,LSB1		# if(--nl > 0) goto LSB1;
LSB20:		# assert r0 == 0	  return(0);
LSB2:		ret
LSB3:		movl	$1,r0		# return(1);
		ret

		.globl	_BnnSubtract
		.align	3
_BnnSubtract:
		.word	0x1E&callee_save	# mask<r1,r2,r3,r4>
		movl	4(ap),r2	# mm
		movl	12(ap),r1	# nn
		movl	16(ap),r3	# nl
		bneq	LS1		# if(nl) goto LS1
		subl3	r3,8(ap),r0	# ml -= nl;
		tstl	20(ap)		# car
		beql	LS5		# if(car) goto LS5
		movl	$1,r0
		ret			# return(1);
LS1:		subl3	r3,8(ap),r0	# ml -= nl;
		tstl	20(ap)		# C = 0; Z = (car == 0)
		bneq	LS2		# if(!(Z = (car == 0))) goto LS2
		addl3	$1,$-1,r4	# C = 1;

LS2:		sbwc	(r1)+,(r2)+	# C..*m++ -= *n++ + C
		sobgtr	r3,LS2		# if(--nl > 0) goto LS2
		bcs	LS5
LS3:		movl	$1,r0
		ret
LS4:		decl	(r2)+
		bcc	LS3
LS5:		sobgeq	r0,LS4		# if (--ml >= 0) goto LS4
		clrl	r0
		ret

		.globl	_BnnMultiplyDigit
# note1: (2^32-1)*(2^32-1) = 2^64-1 - 2*(2^32-1)
# thus 64 bits accomodates a*b+c+d for all a,b,c,d < 2^32
# note2: inner loop is doubled to avoid unnecessary register moves.
		.align	3
_BnnMultiplyDigit:
		.word	0x1FE&callee_save	# mask<r1,r2,r3,r4,r5,r6,r7,r8>
		movl	20(ap),r2	# r2 = d
		blss	LMDNeg		# if (d<0) goto LMDNeg
		bneq	LMD1		# if (d) goto LMD1;
		clrl	r0
		ret
LMD1:		cmpl	$1,r2
		bneq	LMD2		# if (d != 1) goto LMD2
		clrl	20(ap)		# IN BnnAdd: car = 0
		brw	LADDEntry	# BnnAdd(pp,pl,mm,ml,0);

LMD2:		movl	4(ap),r3	# r3 = p
		movl	12(ap),r1	# r1 = m
		movl	16(ap),r7	# r7 = ml
		subl3	r7,8(ap),r8	# r8 = pl-ml
		ashl	$-1,r7,r0	# loop counter r0 = (ml+1)/2
		clrl	r5
		bitl	$1,r7
		bneq	LMDPOddLen	# if (ml is odd) goto LMDPOddLen
		clrl	r7
		brb	LMDPEvenLen	# if (ml is even) goto LMDPOddLen
LMDPLoop:	emul	(r1)+,r2,$0,r4	# r4:r5 = m[i]*d
		bgeq	LMDMPos1	# if (m[i] < 0) 
		addl2	r2,r5		#    r5 += d
LMDMPos1:	addl2	r7,r4		# r4 = (m[i]*d)%2^32+(m[i-1]*d)/2^32+C
		adwc	$0,r5		# r5 = (m[i]*d)/2^32 + carry1
		addl2	r4,(r3)+	# *p++ += r4
		adwc	$0,r5		# r5 = (m[i]*d)/2^32 + carry2
LMDPOddLen:	emul	(r1)+,r2,$0,r6	# r6:r7 = m[i+1]*d
		bgeq	LMDMPos2	# if (m[i+1] < 0)
		addl2	r2,r7		#    r7 += d
LMDMPos2:	addl2	r5,r6		# r6 = (m[i+1]*d)%2^32+(m[i]*d)/2^32+C
		adwc	$0,r7		# r7 = (m[i+1]*d)/2^32 + carry1
		addl2	r6,(r3)+	# *p++ += r6
		adwc	$0,r7		# r7 = (m[i+1]*d)/2^32 + carry2
LMDPEvenLen:	sobgeq	r0,LMDPLoop	# if ((i+=2)/2 < ml/2) repeat loop
		addl2	r7,(r3)+	# *p += (m[ml-1]*d)/2^32
		bcs	LMDTail
LMDRet0:	clrl	r0
		ret

LMDNeg:		movl	4(ap),r3	# r3 = p
		movl	12(ap),r1	# r1 = m
		movl	16(ap),r7	# r7 = ml
		subl3	r7,8(ap),r8	# r8 = pl-ml
		ashl	$-1,r7,r0	# loop counter r0 = (ml+1)/2
		clrl	r5
		bitl	$1,r7
		bneq	LMDNOddLen
		clrl	r7
		brb	LMDNEvenLen
LMDNLoop:	movl	(r1)+,r6	# r6 = m[i]
		emul	r6,r2,$0,r4	# r4:r5 = m[i]*d
		bleq	LMDMPos3	# if (m[i] < 0) 
		addl2	r2,r5		#    r5 += d
LMDMPos3:	addl2	r6,r5		# r5 += m[i]
		addl2	r7,r4		# r4 = (m[i]*d)%2^32+(m[i-1]*d)/2^32+C
		adwc	$0,r5		# r5 = (m[i]*d)/2^32 + carry1
		addl2	r4,(r3)+	# *p++ += r4
		adwc	$0,r5		# r5 = (m[i]*d)/2^32 + carry2
LMDNOddLen:	movl	(r1)+,r4	# r6 = m[i+1]
		emul	r4,r2,$0,r6	# r6:r7 = m[i+1]*d
		bleq	LMDMPos4	# if (m[i+1] < 0)
		addl2	r2,r7		#    r7 += d
LMDMPos4:	addl2	r4,r7		# r7 += m[i+1]
		addl2	r5,r6		# r6 = (m[i+1]*d)%2^32+(m[i]*d)/2^32+C
		adwc	$0,r7		# r7 = (m[i+1]*d)/2^32 + carry1
		addl2	r6,(r3)+	# *p++ += r6
		adwc	$0,r7		# r7 = (m[i+1]*d)/2^32 + carry2
LMDNEvenLen:	sobgeq	r0,LMDNLoop	# if ((i+=2)/2 < ml/2) repeat loop
		addl2	r7,(r3)+	# *p += (m[ml-1]*d)/2^32
		bcs	LMDTail
		clrl	r0		# r0 = carry
		ret

LMDTailLoop:	incl	(r3)+
		bcc	LMDRet0
LMDTail:	sobgtr	r8,LMDTailLoop
		movl	$1,r0		# r0 = carry
		ret

		.globl	_BnnDivideDigit
		.align	3
_BnnDivideDigit:
		.word	0x3FE&callee_save	# mask<r1,r2,r3,r4,r5,r6,r7,r8,r9>
		movl	12(ap),r2	# nl
		movl	16(ap),r3	# d
		moval	*8(ap)[r2],r0	# nn = &nn[nl];
		decl	r2		# nl--;
		moval	*4(ap)[r2],r1	# qq = &qq[nl];
		movl	-(r0),r5	#  X(hight) = *(--n);
		extzv	$1,$31,r3,r7	# r7 = D' <- D div 2
		tstl	r3
		bgeq	Lndivc2
		brw	Lndiv5		# D < 0!!

					# D < 2**31
		brb	Lndivc2		# N < D * 2**32
Lndivc1:	movl	-(r0),r4	# (bdivu dx3 ax1 dx1)
		cmpl	r5,r7
		blss	Lndivc11
		extzv	$0,$1,r4,r6	# r6 <- n0
		ashq	$-1,r4,r4	# N' = r4 = N quo 2 < D * 2**31
		ediv	r3,r4,r4,r5	# r4 <- Q' = N' quo D < 2**31
					# r5 <- R' = N' rem D < D
		ashq	$1,r4,r4	# r4 <- 2 * Q'        < 2**32
					# r5 <- 2 * R'        < 2 * D
		addl2	r6,r5		# r5 <- 2 * R' + n0   < 2 * D
		cmpl	r5,r3		# r5 < D -> Q = r4, R = r5
		blssu	Lndivc12		# sinon
		incl	r4		# Q = r4 + 1
		subl2	r3,r5		# R = r5 - D
		brb	Lndivc12
Lndivc11:	ediv	r3,r4,r4,r5	# Q = r4, R = r5
Lndivc12:	movl	r4,-(r1)	# range r4 en me'moire
Lndivc2:	sobgeq	r2,Lndivc1	# (sobgez dx2 Lndivc1)
		movl	r5,r0		# return(X(hight));
		ret

Lndiv3:		movl	-(r0),r4	# r4 poid faible de N
		extzv	$0,$1,r4,r9	# r9 <-  n0
		extzv	$1,$1,r4,r6	# r6 <-  n1
		extzv	$2,$1,r4,r8	# r8 <- n2
		ashq	$-3,r4,r4	# r4 <- N'' = N quo 4
		bicl2	$0xE0000000,r5	# Le ashq ne le fait pas
		ediv 	r7,r4,r4,r5	# r4 <- Q' = N''' quo D'
					# r5 <- R' = N''' rem D'
		ashl	$1,r5,r5	# r5 <- 2 * R'
		addl2	r8,r5		# r5 <- 2 * R' + n2
		bbc	$0,r3,Lndiv4	# si d0 = 0
		cmpl	r5,r4		# sinon r5 <- 2R' + n1 - Q'
		blssu	Lndiv30		# la diff est < 0
		subl2	r4,r5		# la diff est > 0
		brb	Lndiv4		# voila la diff!
Lndiv30:	subl2	r4,r5		# la diff!
		decl	r4		# r4 <- r4 - 1
		addl2	r3,r5		# r5 <- r5 + D
Lndiv4:		ashl	$1,r4,r4	# r4 <- 2Q'
		addl2	r5,r5		# r5 <- 2r5
		bisl2	r6,r5		# r5 <- r5 + n1 (flag C ok!)
		bcs	Lndiv40		# On deborde sur!
		cmpl	r5,r3
		blssu	Lndiv42		# depasse pas D
Lndiv40:	incl	r4		# Q = r4 + 1
		subl2	r3,r5		# R = r5 - D
Lndiv42:	ashl	$1,r4,r4	# r4 <- 2Q'
		addl2	r5,r5		# r5 <- 2r5
		bisl2	r9,r5		# r5 <- r5 + n0 (flag C ok!)
		bcs	Lndiv43		# On deborde sur!
		cmpl	r5,r3
		blssu	Lndiv44		# depasse pas D
Lndiv43:	incl	r4		# Q = r4 + 1
		subl2	r3,r5		# R = r5 - D
Lndiv44:	movl	r4,-(r1)	# range le quotient en memoire
Lndiv5:		sobgeq	r2,Lndiv3	# On continue!
		movl	r5,r0		# return(X(hight));
		ret

# BigNumCarry BnnMultiply (pp, pl, mm, ml, nn, nl)
# BigNum	pp, nn, mm;
# BigNumLength	pl, nl, ml;

.globl	_BnnMultiply
	.align	3
_BnnMultiply:
	.word	0xFFE&callee_save	# mask<r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11>
	movl	24(ap),r9		# r9 = nl
	bneq	LMM_nl_pos
LMM_Ret0a:
	clrl	r0
LMM_Ret:
	ret
LMM_nl_pos:
	movl	20(ap),r11		# nn
	cmpl	12(ap),r11		# if (nn == mm)
	beql	BMM_Sqr
LMM_NotSqr:
	movl	4(ap),r10		# pp
	clrl	r8			# c_hi
LMM_NLoop:
	  movl	16(ap),r7		# ml
	  movl	12(ap),r1		# mm
	  moval	(r10)+,r3		# pp
	  clrl	r5			# c_lo
	  movl	(r11)+,r2		# digit
	  bsbw	BMM_MultiplyDigit
	  sobgtr	r9,LMM_NLoop
	movl	r8,r0
	beql	LMM_Ret
	movl	16(ap),r7		# r7 = ml
	subl3	r7,8(ap),r1		# r7 = pl-ml
	subl2	24(ap),r1		# r7 = pl-ml-nl
	bleq	LMM_Ret
	moval	(r10)[r7],r10		# pp += ml
LMM_PLoop:
	incl	(r10)+
	bcc	LMM_Ret0a
	sobgtr	r1,LMM_PLoop
	ret
# Special squaring code based on:
# n[0..nl-1]*n[0..nl-1] = sum (i = 0..nl-1):
#                                B^2i * (n[i]*n[i] + 2*n[i] * n[i+1..nl-1] * B)
# the 2*n[i] is tricky because it may overflow, but ...
# suppose  L[i] = 2*n[i]%2^32
#    and   H[i] = 2*n[i]/2^32
# Then:
#   sum (i = 0..nl-1):
#         B^2i * (n[i]*n[i] + L[i]+H[i-1] * n[i+1..nl-1] * B + H[i-1]*n[i])
# notice that when i = nl-1 the final term is 2*n[nl-1] * n[nl..nl-1],
# n[nl..nl-1] is zero length -- i.e. we can ignore it!
# lastly we don't have quite enough registers to conveniently remember
# the top bit of n[i-1] we encode it in the PC by duplicating
# the loop--sometimes I love assembler.
LMMS_NNLoop:
	    # execute this version of loop if n[i-1] was >= 2^31
	    movl	(r11)+,r0	# d = r0 = *nn++
	    movl	r11,r1		# r1 = mm
	    emul	r0,r0,$0,r4	# r4:r5 = d*d
	    addl2	r0,r4		# r4 += (2*n[i-1])/2^32*n[i] (= d)
	    adwc	$0,r5
	    addl2	r4,(r10)+	# *pp++ += d*d%2^32
	    adwc	$0,r5		# r5 += C
	    moval	(r10)+,r3	# arg-p = pp++
	    movl	r9,r7		# arg-ml = ml
	    addl3	r0,r0,r2	# if (d >= 0)
	    bisl2	$1,r2
	    bcc		LMMS_DPos	# switch to < 2^31 loop
LMMS_DNeg:
	    addl2	r0,r5		# compensate for signed mul
	    addl2	r0,r5		# r4:r5 += 2*r2*2^32
	    tstl	r2		# set condition codes for entry to subr
	    # MultiplyDigit(pp=r3, mm=r1, ml=r7, d=r2, c_hi=r8, c_lo=r5)
	    bsbb	BMM_MultiplyDigit
	    sobgeq	r9,LMMS_NNLoop
	brb	LMMS_Post
# >>> ENTRY <<<
BMM_Sqr:
	# r9 = nl, r11 = nn
	movl	16(ap),r8		# r8 = ml
	bneq	LMMS_ml_pos
	clrl	r0
	ret				# return 0;
LMMS_ml_pos:
	cmpl	r8,r9			# if (ml != nl)
	bneq	LMM_NotSqr

	# r8 = 0, r9 = nl, r11 = nn, r10 = sgn(nn[nl-1])
	movl	4(ap),r10		# r10 = pp
					# r11 = nn
	clrl	r8			# r8 = high carry = 0
	decl	r9			# r9 = ml-1 = nl-1
LMMS_NLoop:
	    # execute this version of loop if n[i-1] was < 2^31
	    movl	(r11)+,r0	# d = r0 = *nn++
	    movl	r11,r1		# r1 = mm
	    emul	r0,r0,$0,r4	# r4:r5 = d*d
	    addl2	r4,(r10)+	# *pp++ += d*d%2^32
	    adwc	$0,r5		# r5 += C
	    moval	(r10)+,r3	# arg-p = pp++
	    movl	r9,r7		# arg-ml = ml
	    addl3	r0,r0,r2	# if (d < 0)
	    bcs		LMMS_DNeg	# switch to >= 2^31 loop
LMMS_DPos:
	    tstl	r2		# set condition codes for entry to subr
	    # MultiplyDigit(pp=r3, mm=r1, ml=r7, d=r2, c_hi=r8, c_lo=r5)
	    bsbb	BMM_MultiplyDigit
	    sobgeq	r9,LMMS_NLoop
	# r9 = 0, r10 = pp+2*ml, r2 = 2*nn[nl-1], r8 = carry_hi, r11 = nn+nl
LMMS_Post:
	movl	r8,r0
	bneq	LMMS_CProp		# if (c != 0)
LMMS_ret:
	ret				# return
LMMS_CProp:
	movl	16(ap),r7		# r7 = nl
	subl3	r7,8(ap),r2		# r2 = pl-nl (note nl == ml)
	subl2	r7,r2			# r2 = pl-nl-ml
	bleq	LMMS_ret		# if (pl-nl > ml)
					# ret = BnnAddCarry(pp+ml, pl-ml, c);
LMMS_CPLoop:
	incl	(r10)+			# (*pp++)++
	bcc	LMMS_Ret0
	sobgtr	r2,LMMS_CPLoop
	ret
LMMS_Ret0:
	clrl	r0
	ret

# Subroutine: MultiplyDigit(pp,mm,ml,d,c_hi,c_lo)
# returns:
# c_hi*base^(ml+1)+ pp[0..ml] = pp[0..ml]+(mm[0..ml-1]*d)+c_hi*base^ml+c_lo
#
# In:
#  ml_entry:r7
#  ml/2: r0
#  mm: r1
#  digit: r2
#  pp: r3
#  c_hi: r8
#  c_lo: r5
#
# multiply scratch: r4,r5 / r6,r7
#
# Out:
#  c_hi: r8
LMMD_C_hi:
		addl2	r8,(r3)[r7]	# p[ml] += c_hi
		clrl	r8
		adwc	$0,r8
		rsb
LMMD_Zero:
		tstl	r5		# Too complicated, return to
		beql	LMMD_ZC_lo	# normal case.
		tstl	r2
		brb	LMMD_Retry
LMMD_ZC_lo:
		tstl	r8
		bneq	LMMD_C_hi
		rsb
BMM_MultiplyDigit:
		beql	LMMD_Zero
LMMD_Retry:
		blss	LMMD_Neg	# if (d<0) goto LMMD_Neg
		ashl	$-1,r7,r0	# loop counter r0 = ml/2
		bitl	$1,r7
		bneq	LMMD_POddLen	# if (ml is odd) goto LMMD_POddLen
		movl	r5,r7
		brb	LMMD_PEvenLen	# if (ml is even) goto LMMD_POddLen
LMMD_PLoop:	emul	(r1)+,r2,$0,r4	# r4:r5 = m[i]*d
		bgeq	LMMD_MPos1	# if (m[i] < 0) 
		addl2	r2,r5		#    r5 += d
LMMD_MPos1:	addl2	r7,r4		# r4 = (m[i]*d)%2^32+(m[i-1]*d)/2^32+C
		adwc	$0,r5		# r5 = (m[i]*d)/2^32 + carry1
		addl2	r4,(r3)+	# *p++ += r4
		adwc	$0,r5		# r5 = (m[i]*d)/2^32 + carry2
LMMD_POddLen:	emul	(r1)+,r2,$0,r6	# r6:r7 = m[i+1]*d
		bgeq	LMMD_MPos2	# if (m[i+1] < 0)
		addl2	r2,r7		#    r7 += d
LMMD_MPos2:	addl2	r5,r6		# r6 = (m[i+1]*d)%2^32+(m[i]*d)/2^32+C
		adwc	$0,r7		# r7 = (m[i+1]*d)/2^32 + carry1
		addl2	r6,(r3)+	# *p++ += r6
		adwc	$0,r7		# r7 = (m[i+1]*d)/2^32 + carry2
LMMD_PEvenLen:	sobgeq	r0,LMMD_PLoop	# if ((i+=2)/2 < ml/2) repeat loop
		addl2	r8,r7
		clrl	r8
		adwc	$0,r8
		addl2	r7,(r3)		# *p += (m[ml-1]*d)/2^32
		adwc	$0,r8
		rsb
LMMD_Neg:
		ashl	$-1,r7,r0	# loop counter r0 = ml/2
		bitl	$1,r7
		bneq	LMMD_NOddLen
		movl	r5,r7
		brb	LMMD_NEvenLen
LMMD_NLoop:	movl	(r1)+,r6	# r6 = m[i]
		emul	r6,r2,$0,r4	# r4:r5 = m[i]*d
		bleq	LMMD_MPos3	# if (m[i] < 0) 
		addl2	r2,r5		#    r5 += d
LMMD_MPos3:	addl2	r6,r5		# r5 += m[i]
		addl2	r7,r4		# r4 = (m[i]*d)%2^32+(m[i-1]*d)/2^32+C
		adwc	$0,r5		# r5 = (m[i]*d)/2^32 + carry1
		addl2	r4,(r3)+	# *p++ += r4
		adwc	$0,r5		# r5 = (m[i]*d)/2^32 + carry2
LMMD_NOddLen:	movl	(r1)+,r4	# r6 = m[i+1]
		emul	r4,r2,$0,r6	# r6:r7 = m[i+1]*d
		bleq	LMMD_MPos4	# if (m[i+1] < 0)
		addl2	r2,r7		#    r7 += d
LMMD_MPos4:	addl2	r4,r7		# r7 += m[i+1]
		addl2	r5,r6		# r6 = (m[i+1]*d)%2^32+(m[i]*d)/2^32+C
		adwc	$0,r7		# r7 = (m[i+1]*d)/2^32 + carry1
		addl2	r6,(r3)+	# *p++ += r6
		adwc	$0,r7		# r7 = (m[i+1]*d)/2^32 + carry2
LMMD_NEvenLen:	sobgeq	r0,LMMD_NLoop	# if ((i+=2)/2 < ml/2) repeat loop
		addl2	r8,r7
		clrl	r8
		adwc	$0,r8
		addl2	r7,(r3)		# *p += (m[ml-1]*d)/2^32
		adwc	$0,r8
		rsb
