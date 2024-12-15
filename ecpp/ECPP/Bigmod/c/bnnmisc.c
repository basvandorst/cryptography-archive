
/* $Log:	bnnmisc.c,v $
 * Revision 1.1  91/09/20  14:47:40  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:07:00  morain
 * Initial revision
 *  */

#include "mod.h"

/* r <- floor(sqrt(a, al)). Assumes rl >= (al-1)/2. Returns the number of
 digits of r; r = 0 when entering.*/
int BnnSqrt(r, a, al)
BigMod r, a;
int al;
{
    if(BnnIsOne(a, al) || ((al == 1) && (BnnDoesDigitFitInWord(*a))
			   && (BnnGetDigit(a) <= 3))){
	BnnSetDigit(r, 1);
	return(1);
    }
    else{
	BigNum x0, x1, x2;
	int x0l = 1, x1l, x2l, b;

#ifdef DEBUG
	if(debuglevel >= 10)
	    printf("Computing square root of %s\n", From(a, al));
#endif
	x1 =BNC(al + 1);
	BnnAssign(x1, a, al);
	x1l = al; x2l = al + 1;
	x2 =BNC(x2l);
	/* overestimate r */
	x0 = BNC(al + 1);
	if(al == 1){
	    /* choose x0=a-1 */
	    BnnAssign(x0, a, al);
	    BnnSubtractBorrow(x0, 1, 0);
	}
	else{
	    /* choose x0=B^(al/2) */
	    b = BnNumBits(a, 0, al);
	    b = 1+ (b / 2);
	    x0l = b / BN_DIGIT_SIZE;
	    BnnSetDigit((x0+x0l), 1);
	    BnnShiftLeft((x0+x0l), 1, b % BN_DIGIT_SIZE);
	    x0l++;
	}
#ifdef DEBUG
	if(debuglevel >= 10) printf("Approx=%s\n", From(x0, x0l));
#endif
	while(BnnCompare(x0, x0l, x1, x1l) == -1){
#ifdef DEBUG
	    if(debuglevel >= 10){
		printf("x0=%s\n", From(x0, x0l));
		printf("x1=%s\n", From(x1, x1l));
	    }
#endif
	    /* x2 <- a ou a+x0 : au pire x2[al] contient 1 */
	    BnnAssign(x2, a, al);
	    BnnSetDigit((x2+al), 0);
	    if(BnnIsDigitOdd(*x0))
		BnnAdd(x2, x2l, x0, x0l, 0);
	    /* x1 <- x0 */
	    BnnAssign(x1, x0, x0l);
	    x1l = x0l;
	    /* x0 <- 2*x0 */
	    x0l++;
	    BnnAdd(x0, x0l, x0, x0l, 0);
	    if(BnnIsDigitZero(*(x0+x0l - 1)))
		x0l--;
	    /* x2 <- quomod(x2, x0) */
	    BnnDivide(x2, x2l, x0, x0l);
	    /* x0 <- x0/4 */
	    BnnShiftRight(x0, x0l, 2);
	    /* x0 <- x0+x2[x0l..x2l-1] */
	    BnnAdd(x0, x0l, (x2+x0l), x2l - x0l, 0);
	    x0l = BnnNumDigits(x0, x0l);
	}
	BnnAssign(r, x0, x0l);
#ifdef DEBUG
	if(debuglevel >= 10) printf("sqrt=%s\n", From(r, x0l));
#endif
	BnFree(x0); BnFree(x1); BnFree(x2);
	return(x0l);
    }
}

/* Returns 1 if a is a square and 0 otherwise. If it 1, then (r, *p_rl) 
   contains the squareroot.*/
int BnnIsSquare(r, p_rl, a, al)
BigMod r, a;
int *p_rl, al;
{
    int sq;
    BigMod r2 = BNC(al + 1);
    
    *p_rl = BnnSqrt(r, a, al);
    BnnMultiply(r2, (*p_rl) << 1, r, *p_rl, r, *p_rl);
    sq = (!BnnCompare(a, al, r2, al));
    BnFree(r2);
    return(sq);
}

/* r is large enough and contains sqrt(z) if z is a square */
int BzIsSquare(r, z)
BigNum r;
BigZ z;
{
    if(BzGetSign(z) == -1)
	return(0);
    else{
	BigNum n;
	int nl, rl, sq;

	n = BzToOldBigNum(z, &nl);
	sq = BnnIsSquare(r, &rl, n, nl);
	BnFree(n);
	return(sq);
    }
}

