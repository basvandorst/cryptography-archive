/* Copyright     Digital Equipment Corporation & INRIA     1988, 1989 */
/* testKerN.c: tests des primitives de KerN                           */
/* Last modified on Wed Feb 27 23:26:55 GMT+1:00 1991 by shand        */
/*      modified on Wed Feb 14 16:14:04 GMT+1:00 1990 by herve        */
/*      modified on 17-OCT-1989 20:35:55.91 by Jim Lawton             */

/* You can comment the line below if you want to test the C macro Package
   instead of C or Assembly functions. */

#define BNNMACROS_OFF 1


#include "BigNum.h"
#include "BntoBnn.h"


			/* structure d'un test */

struct testenv {
	char	*name;		/* Le nom de la fonction teste'e.	 */
	int	flag;		/* Pour savoir si l'on continue le Test. */
	char	hist[2048];	/* L'expression qui provoque l'erreur.   */
	char	*depend;	/* De quoi depend le Test.		 */
};


			/* Les nombres pre'de'finies. */

static BigNum 		NumbVect[5][2];
static BigNum 		NumbProto, Ntmp2, NtmpBig;

#define RN(n)		NumbVect[n][0]
#define SN(n) 		NumbVect[n][1]

			/* Taille des nombres utilise's. */
			/* de la forme 4(n + 1) */
#define TESTLENGTH 	16
#define DTL 		TESTLENGTH/2
#define QTL 		TESTLENGTH/4

/* Nombre de test. */
int TestCount, CallDummy = 0;

int dummy()
{
    /* a simple way to get control after <n> steps in the debugger */
    printf("TestCount = %d\n", TestCount);
};

int TestCountInc()
{
    TestCount++;
    if (TestCount == CallDummy)
        dummy();
}

ResetTest(n) int n; {
	/* Remet le nieme nombre a` la valeur prototype. */
	BnAssign(RN(n), 0, NumbProto, 0, TESTLENGTH);
	BnAssign(SN(n), 0, NumbProto, 0, TESTLENGTH);
}

Check(n) int n; {
	int i;
	/* Verifie que les n nombres calcules correspondent aux simule's. */
	for(i = 0; i < n; i++)
		if(CheckSubRange(i, 0, TESTLENGTH)) return(1);
	return(FALSE);
}

CheckSubRange(x, nd, nl) int x, nd, nl; {
	/* Verifie l'e'galite' des sous-nombres
           (RN(x), nd, nl) et (SN(x), nd, nl) */
	while(nl) {
		nl--;
		if(BnCompareDigits(RN(x), nd, SN(x), nd)) return(nd + 1);
		nd++;
	}
	return(FALSE);
}

ShowDiff0(e, r1, r2) struct testenv *e; int r1,r2; {
	ErrorPrint(e);
	if(r1 != r2)
		printf("---- Result is %d and should be %d----\n", r1, r2);
	return(e->flag);
}

ShowDiff1(e, r1, r2, n, nd, nl)
		struct testenv *e; char *n; int r1, r2, nd, nl; {
	ErrorPrint(e);
	if(r1 != r2)
		printf("---- Result is %d and should be %d----\n", r1, r2);
	ShowOutRange(0, n, nd, nl);
	ShowSubNumber(0, n, nd, nl);
	return(e->flag);
}

ShowDiff2(e, r1, r2, n, nd, nl, m, md, ml)
		struct testenv *e; char *n, *m; int r1, r2, nd, nl, md, ml; {
	ErrorPrint(e);
	if(r1 != r2)
		printf("---- Result is %d and should be %d----\n", r1, r2);
	ShowOutRange(0, n, nd, nl);
	ShowOutRange(1, m, md, ml);
	ShowSubNumber(0, n, nd, nl);
	ShowSubNumber(1, m, md, ml);
	return(e->flag);
}

ShowDiff3(e, r1, r2, n, nd, nl, m, md, ml, o, od, ol)
		struct testenv *e; char *n, *m, *o;
		int r1, r2, nd, nl, md, ml, od, ol; {
	ErrorPrint(e);
	if(r1 != r2)
		printf("---- Result is %d and should be %d----\n", r1, r2);
	ShowOutRange(0, n, nd, nl);
	ShowOutRange(1, m, md, ml);
	ShowOutRange(2, o, od, ol);
	ShowSubNumber(0, n, nd, nl);
	ShowSubNumber(1, m, md, ml);
	ShowSubNumber(2, o, od, ol);
	return(e->flag);
}

ShowDiff4(e, r1, r2, n, nd, nl, m, md, ml, o, od, ol, p, pd, pl)
		struct testenv *e; char *n, *m, *o, *p;
		int r1, r2, nd, nl, md, ml, od, ol, pd, pl; {
	ErrorPrint(e);
	if(r1 != r2)
		printf("---- Result is %d and should be %d----\n", r1, r2);
	ShowOutRange(0, n, nd, nl);
	ShowOutRange(1, m, md, ml);
	ShowOutRange(2, o, od, ol);
	ShowOutRange(3, p, pd, pl);
	ShowSubNumber(0, n, nd, nl);
	ShowSubNumber(1, m, md, ml);
	ShowSubNumber(2, o, od, ol);
	ShowSubNumber(3, p, pd, pl);
	return(e->flag);
}

ShowSubNumber(x, n, nd, nl) char *n; int x, nd, nl; {
	printf("[%s, %d, %d] =	", n, nd, nl);
	RangeNumberPrint("", RN(x), nd, nl);
	if(CheckSubRange(x, nd, nl)) {
		RangeNumberPrint(" Before:	", NumbProto, nd, nl);
		RangeNumberPrint(" Simulated:	", SN(x), nd, nl);
}	}

RangeNumberPrint(s, n, nd, nl) char *s; BigNum n; int nd, nl; {
	int first = 1;

	/* Ne marche que si BnGetDigit est garanti!!! */
	printf("%s {", s);
	while(nl) {
		nl--;
		if(!first) printf(", "); else first = 0;
		if(BN_DIGIT_SIZE <= 16)
			printf("%.4X", BnGetDigit(n, nd + nl));
		else	printf("%.8X", BnGetDigit(n, nd + nl));
	}
	printf("}\n");
}

char *msg = "---- Modification Out of Range of number ";
ShowOutRange(x, n, nd, nl) char *n; int x, nd, nl; {
	int i = 0, bol = 0;

	while(i = CheckSubRange(x, i, TESTLENGTH - i)) {
		if((i <= nd) || (i > nd + nl)) {
			if(!bol) {
				bol = 1;
				printf("%s %s at index: (%d", msg, n, i - 1);
			} else {
				printf(" %d", i - 1);
	}	}	}
	if(bol) printf(").\n");
}		

ErrorPrint(e) struct testenv *e; {
	printf("*** Error in compute : %s\n", e->hist);
	printf("  Depends on %s\n", e->depend);
}

/*
 *	Tests des fonctions non redefinisables
 */

int genlengthvec[] = {9, 8, 1, 0, 2000, 32000,};
BigNumType gentypevec[] = {0, 1, 2, 3, 4, 5,};

Generique(e) struct testenv *e; {
	int i;
	int length, length2;
	BigNumType type, type2;
	int fix;
	BigNum n;

	
   for(i=0; i < 6; i++) {
	type = gentypevec[i];
	length = genlengthvec[i];
	n = BnCreate(type, length);
	if((type2 = BnGetType(n)) != type) {
		sprintf(e->hist,"BnGetType(BnCreate(%d, %d));", type, length);
		if(ShowDiff0(e, type, type2)) return(TRUE);
	}
	if((length2 = BnGetSize(n)) != length) {
		sprintf(e->hist,"BnGetSize(BnCreate(%d, %d));", type, length);
		if(ShowDiff0(e, length, length2)) return(TRUE);
	}
	if(BnFree(n) == 0) {
		sprintf(e->hist, "BnFree(BnCreate(%d, %d));", type, length);
		if(ShowDiff0(e, 1, 0)) return(TRUE);
	}
	BnSetType((n = BnAlloc(length)), type);
	if((type2 = BnGetType(n)) != type) {
		sprintf(e->hist,"BnGetType(BnAlloc(%d, %d));", type, length);
		if(ShowDiff0(e, type, type2)) return(TRUE);
	}
	if((length2 = BnGetSize(n)) != length) {
		sprintf(e->hist,"BnGetSize(BnAlloc(%d, %d));", type, length);
		if(ShowDiff0(e, length, length2)) return(TRUE);
	}
	if(BnFree(n) == 0) {
		sprintf(e->hist, "BnFree(BnAlloc(%d, %d));", type, length);
		if(ShowDiff0(e, 1, 0)) return(TRUE);
	}
   }
   return(FALSE);
}

/*
 *	BnSetToZero
 */
___BnSetToZero___(n, nd, nl) register BigNum n; register int nd, nl; {
	register int i;
	for(i=0; i<nl; i++)
		BnSetDigit(n, nd + i, 0);
}

TestBnSetToZero(e) struct testenv *e; {
	int nd, nl;

	e->depend = "()";
	for(nd = 0; nd <= TESTLENGTH; nd++)
	   for(nl = 0; nl <= TESTLENGTH - nd; nl++) {
		TestCountInc();
		ResetTest(0);
		   BnSetToZero   (RN(0), nd, nl);
		___BnSetToZero___(SN(0), nd, nl);
		if(Check(1)) {
			sprintf(e->hist, "%s(n, %d, %d)", e->name, nd, nl);
			if(ShowDiff1(e, 0, 0, "n", nd, nl)) return(1);
	}	}
	return(FALSE);
}

/*
 *	BnAssign
 */
___BnAssign___(m, md, n, nd, nl) BigNum m, n; int md, nd, nl; {
	BnSetToZero(NtmpBig, 0, nl);
	BnAdd(NtmpBig, 0, nl, n, nd, nl, 0);
	BnSetToZero(m, md, nl);
	BnAdd(m, md, nl, NtmpBig, 0, nl, 0);
}

TestBnAssign(e) struct testenv *e; {
	int md, nd, nl;

	e->depend = "(BnSetToZero, BnAdd)";
	for(md = 0; md <= TESTLENGTH; md++)
	  for(nd = 0; nd <= TESTLENGTH; nd++)
	    for(nl=0; ((nl<=TESTLENGTH-nd) && (nl<=TESTLENGTH-md)); nl++) {
		TestCountInc();
		ResetTest(0);
		   BnAssign   (RN(0), md, RN(0), nd, nl);
		___BnAssign___(SN(0), md, SN(0), nd, nl);
		if(Check(1)) {
			sprintf(e->hist, "%s(n, %d, n, %d, %d)", e->name,
						md, nd, nl);
			if(ShowDiff1(e, 0, 0, "n", md, nl)) return(1);
	}	}
	return(FALSE);
}


/*
 *	BnNumDigits
 */
___BnNumDigits___(n, nd, nl) register BigNum n; register int nd, nl; {

	while(nl != 0) {
		nl--;
		if(!BnIsDigitZero(n, nd + nl)) break;
	}
	return(nl + 1);
}

TestBnNumDigits(e) struct testenv *e; {
	int nd0, nl0, nd, nl, l1, l2;

	e->depend = "(BnIsDigitZero)";
	for(nd0 = 0; nd0 <= TESTLENGTH; nd0++)
	  for(nl0 = 0; nl0 <= TESTLENGTH - nd0; nl0++)
	    for(nd = 0; nd <= TESTLENGTH; nd++)
	      for(nl = 0; nl <= TESTLENGTH - nd; nl++) {
		TestCountInc();
		ResetTest(0);
		BnSetToZero(RN(0), nd0, nl0);
		BnSetToZero(SN(0), nd0, nl0);
		l1 =    BnNumDigits   (RN(0), nd, nl);
		l2 = ___BnNumDigits___(SN(0), nd, nl);
		if(Check(1) || l1 != l2) {
			sprintf(e->hist, "%s(n, %d, %d)", e->name, nd, nl);
			if(ShowDiff1(e, l1, l2, "n", nd, nl)) return(1);
	}	}
	return(FALSE);
}

/*
 *	BnNumLeadingZeroBitsInDigit
 */
__BnNumLeadingZeroBitsInDigit__(n, nd) BigNum n; int nd; {
	int p = 0;

	if(BnIsDigitZero(n, nd)) return(BN_DIGIT_SIZE);
	BnAssign(Ntmp2, 0, n, nd, 1);
	BnShiftLeft(Ntmp2, 0, 1, Ntmp2, 1, 1);
	while(BnIsDigitZero(Ntmp2, 1)) {
		BnShiftLeft(Ntmp2, 0, 1, Ntmp2, 1, 1);
		p++;
	}
	return(p);
}

TestBnNumLeadingZeroBitsInDigit(e) struct testenv *e; {
	int nd; int l1, l2;


	e->depend = "(BnShiftLeft, BnIsDigitZero)";
	ResetTest(0);
	for(nd = 0; nd < TESTLENGTH; nd++) {
		TestCountInc();
		l1 =    BnNumLeadingZeroBitsInDigit   (RN(0), nd);
		l2 = __BnNumLeadingZeroBitsInDigit__(SN(0), nd);
		if(Check(1) || l1 != l2) {
			sprintf(e->hist, "%s(n, %d)", e->name, nd);
			if(ShowDiff1(e, l1, l2, "n", nd, 1)) return(1);
	}	}
	return(FALSE);
}

/*
 *	BnIsDigitZero
 */
___BnIsDigitZero___(n, nd) BigNum n; int nd; {
	if(BnGetDigit(n, nd) == 0) return(1);
	return(0);
}

TestBnIsDigitZero(e) struct testenv *e; {
	int nd; int l1, l2;

	e->depend = "()";
	ResetTest(0);
	for(nd = 0; nd < TESTLENGTH; nd++) {
		TestCountInc();
		l1 =    BnIsDigitZero   (RN(0), nd);
		l2 = ___BnIsDigitZero___(SN(0), nd);
		if(Check(1) || ((l1 == 0) && (l2 != 0)) ||
			       ((l1 != 0) && (l2 == 0))) {
			sprintf(e->hist, "%s(n, %d)", e->name, nd);
			if(ShowDiff1(e, l1, l2, "n", nd, 1)) return(1);
	}	}
	return(FALSE);
}

/*
 *	BnIsDigitNormalized
 */
___BnIsDigitNormalized___(n, nd) BigNum n; int nd; {
	BnAssign(Ntmp2, 0, n, nd, 1);
	BnShiftLeft(Ntmp2, 0, 1, Ntmp2, 1, 1);
	if(BnIsDigitZero(Ntmp2, 1)) return(0);
	return(1);
}

TestBnIsDigitNormalized(e) struct testenv *e; {
	int nd; int l1, l2;

	e->depend = "(BnShiftLeft, BnIsDigitZero)";
	ResetTest(0);
	for(nd = 0; nd < TESTLENGTH; nd++) {
		TestCountInc();
		l1 =    BnIsDigitNormalized   (RN(0), nd);
		l2 = ___BnIsDigitNormalized___(SN(0), nd);
		if(Check(1) || ((l1 == 0) && (l2 != 0)) ||
			       ((l1 != 0) && (l2 == 0))) {
			sprintf(e->hist, "%s(n, %d)", e->name, nd);
			if(ShowDiff1(e, l1, l2, "n", nd, 1)) return(1);
	}	}
	return(FALSE);
}

/*
 *	BnIsDigitOdd
 */
___BnIsDigitOdd___(n, nd) BigNum n; int nd; {
	BnAssign(Ntmp2, 0, n, nd, 1);
	BnShiftRight(Ntmp2, 0, 1, Ntmp2, 1, 1);
	if(BnIsDigitZero(Ntmp2, 1)) return(0);
	return(1);
}

TestBnIsDigitOdd(e) struct testenv *e; {
	int nd; int l1, l2;

	e->depend = "(BnShiftRight, BnIsDigitZero)";
	ResetTest(0);
	for(nd = 0; nd < TESTLENGTH; nd++) {
		TestCountInc();
		l1 =    BnIsDigitOdd   (RN(0), nd);
		l2 = ___BnIsDigitOdd___(SN(0), nd);
		if(Check(1) || ((l1 == 0) && (l2 != 0)) ||
			       ((l1 != 0) && (l2 == 0))) {
			sprintf(e->hist, "%s(n, %d)", e->name, nd);
			if(ShowDiff1(e, l1, l2, "n", nd, 1)) return(1);
	}	}
	return(FALSE);
}

/*
 *	BnCompareDigits
 */
___BnCompareDigits___(n, nd, m, md) BigNum n, m; int nd, md; {
	BnAssign(Ntmp2, 0, n, nd, 1);
	BnComplement(Ntmp2, 0, 1);
	if(BnAdd(Ntmp2, 0, 1, m, md, 1, 0)) return(-1);
	BnComplement(Ntmp2, 0, 1);
	if(BnIsDigitZero(Ntmp2, 0)) return(0);
	return(1);
}

TestBnCompareDigits(e) struct testenv *e; {
	int nd, md; int l1, l2;

	e->depend = "(BnComplement, BnAdd, BnIsDigitZero)";
	ResetTest(0);
	ResetTest(1);
	for(nd = 0; nd < TESTLENGTH; nd++)
	   for(md = 0; md < TESTLENGTH; md++) {
		TestCountInc();
		l1 =    BnCompareDigits   (RN(0), nd, RN(1), md);
		l2 = ___BnCompareDigits___(SN(0), nd, SN(1), md);
		if(Check(2) || l1 != l2) {
			sprintf(e->hist, "%s(n, %d, m, %d)", e->name, nd, md);
			if(ShowDiff2(e, l1, l2, "n", nd, 1, "m", md, 1))
				return(1);
	}	}
	return(FALSE);
}

/*
 *	BnComplement
 */
___BnComplement___(n, nd, nl) BigNum n; int nd, nl; {
	int i;

	BnSetDigit(Ntmp2, 0, 0);
	BnSubtractBorrow(Ntmp2, 0, 1, 0);
	for(i = 0; i < nl; i++)
		BnXorDigits(n, nd + i, Ntmp2, 0);
}

TestBnComplement(e) struct testenv *e; {
	int nd, nl;

	e->depend = "(BnSubtractBorrow, BnXorDigits)";
	for(nd = 0; nd <= TESTLENGTH; nd++)
	   for(nl = 0; nl <= TESTLENGTH - nd; nl++) {
		TestCountInc();
		ResetTest(0);
		   BnComplement   (RN(0), nd, nl);
		___BnComplement___(SN(0), nd, nl);
		if(Check(1)) {
			sprintf(e->hist, "%s(n, %d, %d)", e->name, nd, nl);
			if(ShowDiff1(e, 0, 0, "n", nd, nl)) return(1);
	}	}
	return(FALSE);
}

/*
 *	BnAndDigits
 */
___BnAndDigits___(n, nd, m, md) BigNum n, m; int nd, md; {
	BnAssign(Ntmp2, 0, n, nd, 1);
	BnOrDigits(Ntmp2, 0, m, md);
	BnXorDigits(Ntmp2, 0, m, md);
	BnXorDigits(n, nd, Ntmp2, 0);
}

TestBnAndDigits(e) struct testenv *e; {
	int nd, md;

	e->depend = "(BnOrDigits, BnXorDigits)";
	ResetTest(1);
	for(nd = 0; nd < TESTLENGTH; nd++)
	   for(md = 0; md < TESTLENGTH; md++) {
		TestCountInc();
		ResetTest(0);
		   BnAndDigits   (RN(0), nd, RN(1), md);
		___BnAndDigits___(SN(0), nd, SN(1), md);
		if(Check(2)) {
			sprintf(e->hist, "%s(n, %d, m, %d)", e->name, nd, md);
			if(ShowDiff2(e, 0, 0, "n", nd, 1, "m", md, 1))
				return(1);
	}	}
	return(FALSE);
}

/*
 *	BnOrDigits
 */
___BnOrDigits___(n, nd, m, md) BigNum n, m; int nd, md; {
	BnAssign(Ntmp2, 0, n, nd, 1);
	BnAndDigits(Ntmp2, 0, m, md);
	BnXorDigits(Ntmp2, 0, m, md);
	BnXorDigits(n, nd, Ntmp2, 0);
}

TestBnOrDigits(e) struct testenv *e; {
	int nd, md;

	e->depend = "(BnAndDigits, BnXorDigits)";
	ResetTest(1);
	for(nd = 0; nd < TESTLENGTH; nd++)
	   for(md = 0; md < TESTLENGTH; md++) {
		TestCountInc();
		ResetTest(0);
		   BnOrDigits   (RN(0), nd, RN(1), md);
		___BnOrDigits___(SN(0), nd, SN(1), md);
		if(Check(2)) {
			sprintf(e->hist, "%s(n, %d, m, %d)", e->name, nd, md);
			if(ShowDiff2(e, 0, 0, "n", nd, 1, "m", md, 1))
				return(1);
	}	}
	return(FALSE);
}

/*
 *	BnXorDigits
 */
___BnXorDigits___(n, nd, m, md) BigNum n, m; int nd, md; {
	BnAssign(Ntmp2, 0, n, nd, 1);
	BnAndDigits(Ntmp2, 0, m, md);
	BnComplement(Ntmp2, 0, 1);
	BnOrDigits(n, nd, m, md);
	BnAndDigits(n, nd, Ntmp2, 0);
}

TestBnXorDigits(e) struct testenv *e; {
	int nd, md;

	e->depend = "(BnAndDigits, BnComplement, BnOrDigits)";
	ResetTest(1);
	for(nd = 0; nd < TESTLENGTH; nd++)
	   for(md = 0; md < TESTLENGTH; md++) {
		TestCountInc();
		ResetTest(0);
		   BnXorDigits   (RN(0), nd, RN(1), md);
		___BnXorDigits___(SN(0), nd, SN(1), md);
		if(Check(2)) {
			sprintf(e->hist, "%s(n, %d, m, %d)", e->name, nd, md);
			if(ShowDiff2(e, 0, 0, "n", nd, 1, "m", md, 1))
				return(1);
	}	}
	return(FALSE);
}

/*
 *	BnShiftLeft
 */
___BnShiftLeft___(n, nd, nl, m, md, s) BigNum n, m; int nd, nl, md; int s; {
	BnSetDigit(m, md, 2);
	BnSetDigit(Ntmp2, 0, 1);
	while(s--) {
		BnSetToZero(NtmpBig, 0, 2);
		BnMultiplyDigit(NtmpBig, 0, 2, Ntmp2, 0, 1, m, md);
		BnAssign(Ntmp2, 0, NtmpBig, 0, 1);
	}
	BnSetToZero(NtmpBig, 0, nl + 1);
	BnMultiplyDigit(NtmpBig, 0, nl + 1, n, nd, nl, Ntmp2, 0);
	BnAssign(n, nd, NtmpBig, 0, nl);
	BnAssign(m, md, NtmpBig, nl, 1);
}

TestBnShiftLeft(e) struct testenv *e; {
	int nd, nl, md; int s;

	e->depend = "(BnSetToZero, BnMultiplyDigit)";
	ResetTest(1);
	for(nd = 0; nd <= TESTLENGTH; nd++)
	  for(nl = 0; nl <= TESTLENGTH - nd; nl++)
	    for(md = 0; md < 2; md++)
	      for(s = 0; s < BN_DIGIT_SIZE; s++) {
		TestCountInc();
		ResetTest(0);
		   BnShiftLeft   (RN(0), nd, nl, RN(1), md, s);
		___BnShiftLeft___(SN(0), nd, nl, SN(1), md, s);
		if(Check(2)) {
			sprintf(e->hist, "%s(n, %d, %d, m, %d, %d)",
					e->name, nd, nl, md, s);
			if(ShowDiff2(e, 0, 0, "n", nd, nl, "m", md, 1))
				return(1);
	}	}
	return(FALSE);
}

/*
 *	BnShiftRight
 */
___BnShiftRight___(n, nd, nl, m, md, s) BigNum n, m; int nd, nl, md; int s; {
	if((nl == 0) || (s == 0)) {
		BnSetDigit(m, md, 0);
		return;
	}
	BnAssign(NtmpBig, 0, n, nd, nl);
	BnShiftLeft(NtmpBig, 0, nl, NtmpBig, nl, BN_DIGIT_SIZE - s);
	BnAssign(n, nd, NtmpBig, 1, nl);
	BnAssign(m, md, NtmpBig, 0, 1);
}

TestBnShiftRight(e) struct testenv *e; {
	int nd, nl, md; int s;

	e->depend = "(BnShiftLeft)";
	ResetTest(1);
	for(nd = 0; nd <= TESTLENGTH; nd++)
	  for(nl = 0; nl <= TESTLENGTH - nd; nl++)
	    for(md = 0; md < 2; md++)
	      for(s = 0; s < BN_DIGIT_SIZE; s++) {
		TestCountInc();
		ResetTest(0);
		   BnShiftRight   (RN(0), nd, nl, RN(1), md, s);
		___BnShiftRight___(SN(0), nd, nl, SN(1), md, s);
		if(Check(2)) {
			sprintf(e->hist, "%s(n, %d, %d, m, %d, %d)",
					e->name, nd, nl, md, s);
			if(ShowDiff2(e, 0, 0, "n", nd, nl, "m", md, 1))
				return(1);
	}	}
	return(FALSE);
}

/*
 *	BnAddCarry
 */
___BnAddCarry___(n, nd, nl, r) BigNum n; int nd, nl; int r;{
	if(r == 0) return(0);
	BnComplement(n, nd, nl);
	r = BnSubtractBorrow(n, nd, nl, 0);
	BnComplement(n, nd, nl);
	if(r == 0) return(1);
	return(0);
}

TestBnAddCarry(e) struct testenv *e; {
	int nd, nl; int r, l1, l2;

	e->depend = "(BnComplement, BnSubtractBorrow)";
	for(nd = 0; nd <= TESTLENGTH; nd++)
	  for(nl = 0; nl <= TESTLENGTH - nd; nl++)
	    for(r = 0; r < 2; r++) {
		TestCountInc();
		ResetTest(0);
		l1 =    BnAddCarry   (RN(0), nd, nl, r);
		l2 = ___BnAddCarry___(SN(0), nd, nl, r);
		if(Check(1) || l1 != l2) {
			sprintf(e->hist, "%s(n, %d, %d, %d)",
					e->name, nd, nl, r);
			if(ShowDiff1(e, l1, l2, "n", nd, nl)) return(1);
	}	}
	return(FALSE);
}

/*
 *	BnAdd
 */
___BnAdd___(n, nd, nl, m, md, ml, r) BigNum n, m; int nd, nl, md, ml; int r;{
	BnComplement(m, md, ml);
	r = BnSubtract(n, nd, ml, m, md, ml, r);
	BnComplement(m, md, ml);
	return(BnAddCarry(n, nd + ml, nl - ml, r));
}

TestBnAdd(e) struct testenv *e; {
	int nd, nl, md, ml; int r, l1, l2;

	e->depend = "(BnComplement, BnSubtract, BnAddCarry)";
	ResetTest(1);
	for(nd = 0; nd <= TESTLENGTH; nd++)
	 for(nl = 0; nl <= TESTLENGTH - nd; nl++)
	  for(md = 0; md <= TESTLENGTH - nl; md++)
	   for(ml = 0; ml <= nl ; ml++)
	    for(r = 0; r < 2; r++) {
		TestCountInc();
		ResetTest(0);
		l1 =    BnAdd   (RN(0), nd, nl, RN(1), md, ml, r);
		l2 = ___BnAdd___(SN(0), nd, nl, SN(1), md, ml, r);
		if(Check(2) || l1 != l2) {
			sprintf(e->hist, "%s(n, %d, %d, m, %d, %d, %d)",
					e->name, nd, nl, md, ml, r);
			if(ShowDiff2(e, l1, l2, "n", nd, nl, "m", md, ml))
				return(1);
	}	}
	return(FALSE);
}

/*
 *	BnSubtractBorrow
 */
___BnSubtractBorrow___(n, nd, nl, r) BigNum n; int nd, nl; int r;{
	if(r == 1) return(1);
	BnComplement(n, nd, nl);
	r = BnAddCarry(n, nd, nl, 1);
	BnComplement(n, nd, nl);
	if(r == 0) return(1);
	return(0);
}

TestBnSubtractBorrow(e) struct testenv *e; {
	int nd, nl; int r, l1, l2;

	e->depend = "(BnComplement, BnAddCarry)";
	for(nd = 0; nd <= TESTLENGTH; nd++)
	  for(nl = 0; nl <= TESTLENGTH - nd; nl++)
	    for(r = 0; r < 2; r++) {
		TestCountInc();
		ResetTest(0);
		l1 =    BnSubtractBorrow   (RN(0), nd, nl, r);
		l2 = ___BnSubtractBorrow___(SN(0), nd, nl, r);
		if(Check(1) || l1 != l2) {
			sprintf(e->hist, "%s(n, %d, %d, %d)",
					e->name, nd, nl, r);
			if(ShowDiff1(e, l1, l2, "n", nd, nl)) return(1);
	}	}
	return(FALSE);
}

/*
 *	BnSubtract
 */
___BnSubtract___(n, nd, nl, m, md, ml, r) BigNum n, m; int nd, nl, md, ml; int r;{
	BnComplement(m, md, ml);
	r = BnAdd(n, nd, ml, m, md, ml, r);
	BnComplement(m, md, ml);
	return(BnSubtractBorrow(n, nd + ml, nl - ml, r));
}

TestBnSubtract(e) struct testenv *e; {
	int nd, nl, md, ml; int r, l1, l2;

	e->depend = "(BnComplement, BnAdd, BnSubtractBorrow)";
	ResetTest(1);
	for(nd = 0; nd <= TESTLENGTH; nd++)
	 for(nl = 0; nl <= TESTLENGTH - nd; nl++)
	  for(md = 0; md <= TESTLENGTH - nl; md++)
	   for(ml = 0; ml <= nl ; ml++)
	    for(r = 0; r < 2; r++) {	
		TestCountInc();
		ResetTest(0);
		l1 =    BnSubtract   (RN(0), nd, nl, RN(1), md, ml, r);
		l2 = ___BnSubtract___(SN(0), nd, nl, SN(1), md, ml, r);
		if(Check(2) || l1 != l2) {
			sprintf(e->hist, "%s(n, %d, %d, m, %d, %d, %d)",
					e->name, nd, nl, md, ml, r);
			if(ShowDiff2(e, l1, l2, "n", nd, nl, "m", md, ml))
				return(1);
	}	}
	return(FALSE);
}

/*
 *	BnMultiplyDigit
 */
___BnMultiplyDigit___(p, pd, pl, n, nd, nl, m, md) BigNum p, n, m; int pd, pl, nd, nl, md; {
	int r = 0, ret = 0;

	BnAssign(Ntmp2, 0, m, md, 1);
	BnAssign(NtmpBig, 0, n, nd, nl);
	BnSetToZero(NtmpBig, nl, 1);
	while(!BnIsDigitZero(Ntmp2, 0)) {
		if(BnIsDigitOdd(Ntmp2, 0)) {
			r = BnAdd(p, pd, pl, NtmpBig, 0, nl + 1, 0);
			if((ret == 0) && (r == 1)) ret = 1;
			else if((ret == 1) && (r == 1))	ret = 2;
		}
		BnShiftRight(Ntmp2, 0, 1, Ntmp2, 1, 1);
		BnShiftLeft(NtmpBig, 0, nl + 1, Ntmp2, 1, 1);
		if(!BnIsDigitZero(Ntmp2, 1)) ret = 3;
	}
	return(ret);
}

TestBnMultiplyDigit(e) struct testenv *e; {
	int pd, pl, nd, nl, md; int l1, l2;

	e->depend = "(BnSetToZero, BnIsDigitZero, BnIsDigitOdd, BnAdd, BnShiftRight, BnShiftLeft)";
	ResetTest(1);
	ResetTest(2);
	for(pd = 0; pd <= TESTLENGTH; pd++)
	 for(pl = 0; pl <= TESTLENGTH - pd; pl++)
	  for(nd = 0; nd <= TESTLENGTH - pl; nd++)
	   for(nl = 0; nl < pl ; nl++)
	    for(md = 0; md < TESTLENGTH; md++) {
		TestCountInc();
		ResetTest(0);
		l1 =    BnMultiplyDigit   (RN(0),pd,pl,RN(1),nd,nl,RN(2),md);
		l2 = ___BnMultiplyDigit___(SN(0),pd,pl,SN(1),nd,nl,SN(2),md);
		if(Check(3) || l1 != l2) {
			sprintf(e->hist,
			   "BnMultiplyDigit(p, %d, %d, n, %d, %d, m, %d)",
						pd, pl, nd, nl, md);
			if(ShowDiff3(e,l1,l2,"p",pd,pl,"n",nd,nl,"m",md,1))
				 return(1);
	}	}
	return(FALSE);
}

/*
 *	BnDivideDigit
 */
TestBnDivideDigit(e) struct testenv *e; {
	int nd, nl, md, qd, rd, l2;

	e->depend = "(BnSetToZero, BnMultiplyDigit, BnCompareDigits)";
	ResetTest(2);
	ResetTest(3);
	for(nd = 0; nd <= TESTLENGTH - 2; nd++)
	 for(nl = 2; nl <= TESTLENGTH - nd; nl++)
	  for(md = 0; md < TESTLENGTH; md++)
	   for(qd = 0; qd < TESTLENGTH - nl + 1 ; qd++)
	    for(rd = 0; rd < 2; rd++)
	     if((!BnIsDigitZero(RN(3), md)) &&
			(BnCompareDigits(RN(2), nd+nl-1, RN(3), md) == -1)) {
		TestCountInc();
		ResetTest(0);
		ResetTest(1);
		BnDivideDigit(RN(0), qd, RN(1), rd, RN(2), nd, nl, RN(3), md);
		BnAssign(SN(0), qd, RN(0), qd, nl - 1);
		BnAssign(SN(1), rd, RN(1), rd, 1);
		BnSetToZero(SN(2), nd, nl);
		BnAssign(SN(2), nd, SN(1), rd, 1);
		l2 = BnMultiplyDigit(SN(2),nd,nl, SN(0),qd,nl - 1, SN(3), md);
		if(Check(4) || l2 != 0) {
			sprintf(e->hist,
			   "BnDivideDigit(q, %d, r, %d, n, %d, %d, m, %d)",
						qd, rd, nd, nl, md);
			if(ShowDiff4(e, 0, l2, "q", qd, nl - 1, "r", rd, 1,
					"n", nd, nl, "m", md, 1))
				return(TRUE);
	}	}
	return(FALSE);
}

/*
 *	BnMultiply
 */
___BnMultiply___(p, pd, pl, m, md, ml, n, nd, nl) BigNum p, m, n; int pd, pl, md, ml, nd, nl; {
	int ret;

        for (ret = 0; nl-- > 0; pd++, nd++, pl--)
           ret += BnMultiplyDigit (p, pd, pl, m, md, ml, n, nd);
	return(ret);
}

TestBnMultiply(e) struct testenv *e; {
	int pd, pl, nd, nl, md, ml; int l1, l2;

	e->depend = "(BnSetToZero, BnMultiplyDigit)";
	ResetTest(1);
	ResetTest(2);
	for(pd = 0; pd <= TESTLENGTH; pd++)
	 for(pl = 0; pl <= TESTLENGTH - pd && pl <= TESTLENGTH/2; pl++)
	  for(nd = 0; nd <= TESTLENGTH - pl; nd++)
	   for(nl = 0; nl < pl && nl <= TESTLENGTH/3; nl++)
	   {
	    if (nl <= pl-nl)
	    {
	        /* Test squaring */
	        TestCountInc();
		ResetTest(0);
		l1 =    BnMultiply   (RN(0),pd,pl,RN(1),nd,nl,RN(1),nd,nl);
		l2 = ___BnMultiply___(SN(0),pd,pl,SN(1),nd,nl,SN(1),nd,nl);
		if(Check(3) || l1 != l2) {
			sprintf(e->hist,
			   "BnMultiply(p, %d, %d, n, %d, %d, n, %d, %d)",
						pd, pl, nd, nl, nd, nl);
			if(ShowDiff3(e,l1,l2,"p",pd,pl,"n",nd,nl,"n",nd,nl))
				 return(1);
		}

	    }
	    for(md = 0; md <= TESTLENGTH; md++) 
	     for (ml = 0; ml <= pl-nl && ml <= TESTLENGTH/3 && md+ml <= TESTLENGTH; ml++) {
		TestCountInc();
		ResetTest(0);
		l1 =    BnMultiply   (RN(0),pd,pl,RN(1),nd,nl,RN(2),md,ml);
		l2 = ___BnMultiply___(SN(0),pd,pl,SN(1),nd,nl,SN(2),md,ml);
		if(Check(3) || l1 != l2) {
			sprintf(e->hist,
			   "BnMultiply(p, %d, %d, n, %d, %d, m, %d, %d)",
						pd, pl, nd, nl, md, ml);
			if(ShowDiff3(e,l1,l2,"p",pd,pl,"n",nd,nl,"m",md,ml))
				 return(1);
	}  }	}
	return(FALSE);
}

/*
 *	Main
 */
typedef struct {
	int	(*TestFnt)();
	char	*NameFnt;
} TESTONE;
TESTONE AllTest[] = {
	Generique,				"Generic Functions",
	TestBnSetToZero,			"BnSetToZero",
	TestBnAssign,				"BnAssign",
	TestBnNumDigits,			"BnNumDigits",
	TestBnNumLeadingZeroBitsInDigit,	"BnNumLeadingZeroBitsInDigit",
	TestBnIsDigitZero,			"BnIsDigitZero",
	TestBnIsDigitNormalized,		"BnIsDigitNormalized",
	TestBnIsDigitOdd,			"BnIsDigitOdd",
	TestBnCompareDigits,			"BnCompareDigits",
	TestBnComplement,			"BnComplement",
	TestBnAndDigits,			"BnAndDigits",
	TestBnOrDigits,				"BnOrDigits",
	TestBnXorDigits,			"BnXorDigits",
	TestBnShiftLeft,			"BnShiftLeft",
	TestBnShiftRight,			"BnShiftRight",
	TestBnAddCarry,				"BnAddCarry",
	TestBnAdd,				"BnAdd",
	TestBnSubtractBorrow,			"BnSubtractBorrow",
	TestBnSubtract,				"BnSubtract",
	TestBnMultiplyDigit,			"BnMultiplyDigit",
	TestBnDivideDigit,			"BnDivideDigit",
	TestBnMultiply,				"BnMultiply",
};

main(n, s) int n; char **s; {
	struct testenv realenv, *e = &realenv;
	int i, j, nbtest, SizeAllTest;

	/* Initialisations de l'environnement de test. */
	e->flag = 1;
	e->depend = "()";
	/* Allocation des 2 nombres globaux. */
	Ntmp2 = BnAlloc(2);
	NtmpBig = BnAlloc(2 * TESTLENGTH);
	NumbProto = BnAlloc(TESTLENGTH);
	/* Creation du nombre prototype. */
	BnSetDigit(NumbProto, 0, 0);		/* Les 2 premiers a` ze'ro. */
	BnSetDigit(NumbProto, 1, 0);
	for(i=0; i < TESTLENGTH/4 - 1; i++)	/* Le premier quart est la */
		BnSetDigit(NumbProto, i + 2, i + 1);	/* suite 1, 2, 3, ...      */
	/* Le 2nd quart est le 1er shifte de BN_DIGIT_SIZE - 2. 0x4000 0x8000 ...*/
	BnAssign(NumbProto, QTL + 1, NumbProto, 2, QTL - 1);
	BnShiftLeft(NumbProto, QTL + 1, QTL - 1, NumbProto, 0, BN_DIGIT_SIZE - 2);
	/* La 2nd moitie est l'inverse logique de la 1ere */
	BnAssign(NumbProto, DTL, NumbProto, 0, DTL);
	BnComplement(NumbProto, DTL, DTL);
	/* Allocation des nombres utilise's */
	for(i=0; i < 5; i++) {
		RN(i) = BnAlloc(TESTLENGTH);
		SN(i) = BnAlloc(TESTLENGTH);
	}
	if(n > 1 && s[1][0] == '-') {
	    CallDummy = atoi(s[1]+1);
	    n--;
	    s++;
        }
	if(n == 1) {
		printf("%s [-CallDummy#] v|a|TestNum\n", s[0]);
	}
	/* On y va */
	SizeAllTest = (sizeof(AllTest)/sizeof(AllTest[0]));
	for(i = 1; i < n; i++) {
		if(s[i][0] == 'm') {
			/* 0 = No skip; 1 = skip to next; else STOP */
			e->flag = atoi(&s[i][1]);
		} else if(s[i][0] == 'a') {
			for(i = 0; i < SizeAllTest; i++)
				dotest(e, i);
		} else if(s[i][0] == 'v') {
			for(j = 0; j < SizeAllTest; j++)
				seetest(j);
		} else {
			nbtest = atoi(s[i]);
			if((nbtest < 0) || (nbtest >= SizeAllTest))
				printf("Test %d is invalid\n", nbtest);
			else	dotest(e, nbtest);
}	}	}

dotest(e, n) struct testenv *e; int n; {
	seetest(n);
	TestCount = 0;
	e->name = AllTest[n].NameFnt;
	if(((*(AllTest[n].TestFnt)) (e)) && e->flag > 1) exit(0);
	printf("%d tests were performed\n", TestCount);
}

seetest(n) int n; {
	printf("%d.	Testing %s\n", n, AllTest[n].NameFnt);
}

