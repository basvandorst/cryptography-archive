
/* $Log:	binroot.c,v $
 * Revision 1.1  91/09/20  14:52:49  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:27:12  morain
 * Initial revision
 *  */

/**********************************************************************

                             Solving X^n = a mod p

**********************************************************************/

#include <math.h>

#include "mod.h"
#include "exp.h"
#include "gcd.h"
#include "poly.h"

#define LQMAX 6.90775 /* we want q**(k-1) <= 1000 */

int ModBinomialRoot(y, a, q)
BigNum y, a;
int q;
{
}

/* y <- sqrt(a, al) mod Modu */
/* returns 1 if a squareroot exists and 0 otherwise */
int MdSqrt(y, p_yl, a, al)
BigNum y, a;
int *p_yl, al;
{
    BigNum z = BmCreate();
    int ok, zl;

    ModSetToZero(z);
    zl = 1;
    ok = MdSqrtZ(y, p_yl, a, al, z, &zl);
    BmFree(z);
    return(ok);
}

/* Uses Shanks' algorithm */
int MdSqrtZ(y, p_yl, a, al, z, p_zl)
BigMod y, a, z;
int *p_yl, al, *p_zl;
{
    BigNum q = BmCreate(), s = BmCreate(), v = BmCreate();
    BigNum w = BmCreate(), yy = BmCreate();
    long nr = 2;
    int e = 0, j, k, ql, sl, vl = 1, wl = 0, yyl = 0, i, ok = 1;

#ifdef DEBUG
    if(debuglevel >= 10)
	{printf("Extracting squareroot of "); BnnPrint(a, al); printf("\n");}
#endif
    /* initialisations */
    BnnAssign(q, Modu, Modul);
    BnnSubtractBorrow(q, Modul, 0);
    while(!BnnIsDigitOdd(*q)){
	e++;
	BnnShiftRight(q, Modul, 1);
    }
    ql = BnnNumDigits(q, Modul);
#ifdef DEBUG
    if(debuglevel >= 10) {printf("q="); BnnPrint(q, ql); printf("\n");}
#endif
    if(BnnIsZero(z, *p_zl)){
	/* looking for a nonresidue */
	while(LongJacobiBn(nr, Modu, Modul) != -1) nr++;
#ifdef DEBUG
	if(debuglevel >= 10) printf("Nonresidue=%ld\n", nr);
#endif
	BnnSetDigit(z, nr);
	BmExp(z, p_zl, z, 1, q, ql);
    }
#ifdef DEBUG
    if(debuglevel>=10) {printf("z**q="); BnnPrint(z, *p_zl); printf("\n");}
#endif
    BnnAssign(yy, z, *p_zl);
    yyl = *p_zl; j = e;
    /* v <- a**(q-1)/2 mod Modu */
    if(!BnnIsGtOne(q, ql))
	/* q = 1 */
	BnnSetDigit(v, 1);
    else{
	/* q <- (q-1)/2 */
	BnnShiftRight(q, ql, 1);
	/* updating ql */
	if(BnnIsDigitZero(*(q+ql-1))) ql--;
	BmExp(v, &vl, a, al, q, ql);
    }
#ifdef DEBUG
    if(debuglevel >= 10) {printf("a**(q-1)/2="); BnnPrint(v, vl); printf("\n");}
#endif
    /* w <- v**2 = a**(q-1) */
    BmSquare(w, &wl, v, vl);
    /* w <- w*a = a**q */
    BmMultiply(w, &wl, w, wl, a, al);
    /* v <- v*a = a**(q+1)/2 */
    BmMultiply(v, &vl, v, vl, a, al);
    /* let us loop */
    while(BnnIsGtOne(w, wl)){
	MdAssign(s, w, wl);
	sl = wl;
	for(k = 0; k < j; k++){
	    if(BnnIsOne(s, sl))
		/* s = 1 */
		break;
	    else
		BmSquare(s, &sl, s, sl);
	}
	if(k == j){
	    printf("No sqrt mod "); BnnPrint(Modu, Modul); printf("\n");
	    ok = 0;
	    break;
	}
	MdAssign(s, yy, yyl);
	sl = yyl;
	for(i = 1; i < j-k; i++)
	    BmSquare(s, &sl, s, sl);
	j = k;
	BmMultiply(v, &vl, v, vl, s, sl);
	BmSquare(yy, &yyl, s, sl);
	BmMultiply(w, &wl, w, wl, yy, yyl);
    }
    if(ok){
	MdAssign(y, v, vl);
	*p_yl = vl;
#ifdef DEBUG
	if(debuglevel >= 10)
	    {printf("Squareroot is "); BnnPrint(v, vl); printf("\n");}
#endif
    }
    BmFree(q); BmFree(s); BmFree(v); BmFree(w); BmFree(yy);
    return(ok);
}

/**********************************************************************

                      Case n is an odd prime q

**********************************************************************/

/* y <- a ^ (1/q) mod p */
int ModBinRootOddPrime(y, p_yl, a, al, q)
BigNum y, a;
int *p_yl, al, q;
{
    BigNum z = BmCreate();
    int root, zl;

    root = ModBinRootOddPrimeZ(y, p_yl, a, al, q, z, &zl);
    BmFree(z);
    return(root);
}

/* y <- a^(1/q) mod p; z is a q-th nonresidue mod p */
int ModBinRootOddPrimeZ(y, p_yl, a, al, q, z, p_zl)
BigNum y, a, z;
int q, *p_yl, al, *p_zl;
{
    BigNum m = BmCreate(), mq = BmCreate(), tmp;
    BigNum x = BmCreate(), w = BmCreate(), ww = BmCreate();
    BigNum r = BmCreate();
    double log();
    long qk;
    int ml, xl, wl, yl, mql, zl, wwl, rl;
    int i, k, lam;

#ifdef DEBUG
    if(debuglevel >= 3){
	printf("%% Computing %d-th root of ", q); BnnPrint(a, al); printf("\n");
    }
#endif
    BnnAssign(mq, Modu, Modul);
    BnnSubtractBorrow(mq, Modul, 0);
    k = ComputeValuation(m, &ml, mq, Modul, (BigNumDigit)q);
#ifdef DEBUG
    if(debuglevel >= 3){
	printf("%% m=(p-1)/%d**%d=", q, k); BnnPrint(m, ml); printf("\n");
    }
#endif
    /* m = (p-1)/q**k */
    if(k <= 1){
	/*   p <> 1 mod q,   and y <- a^((1/q) mod p-1)
          or p <> 1 mod q^2, and y <- a^((1/q) mod (p-1)/q) */
	BnnSetDigit(y, q);
	/* x <- 1/q mod p-1 */
	BnnEeaWithCopy(w, &wl, x, &xl, m, ml, y, 1);
	BmExp(y, p_yl, a, al, x, xl);
	return(1);
    }
    else{
	if(((double)(k-1))*log((double)q) > LQMAX){
#ifdef DEBUG
	    if(debuglevel >= 3){
		printf("%% v_%d(p-1)=%d quite high\n", q, k);
	    }
#endif
	    return(ModBinRootOddPrimeLarge(y, p_yl, a, al, q));
	}
	/* k > 1 */
	qk = q;
	for(i = 2; i < k; i++)
	    qk *= q;
	/* mq <- q * m = (p-1)/q**(k-1) */
	ModSetToZero(mq);
	BnnMultiplyDigit(mq, ml+1, m, ml, q);
	mql = BnnNumDigits(mq, ml+1);
#ifdef DEBUG
	if(debuglevel >= 3){
	    printf("%% mq=(p-1)/%d**%d=",q,k-1); BnnPrint(mq,mql); printf("\n");
	}
#endif
	/* if z == 0, find a suitable z */
	if(BnnIsZero(z, *p_zl))
	    FindPrimitiveRootModp(z, p_zl, w, &wl, q);
	/* r <- z**mq */
	BmExp(r, &rl, z, *p_zl, mq, mql);
#ifdef DEBUG
	if(debuglevel >= 3){
	    printf("%% r=z**mq="); BnnPrint(r, rl); printf("\n");
	}
#endif
	/* w <- a**m */
	BmExp(w, &wl, a, al, m, ml);
#ifdef DEBUG
	if(debuglevel >= 3){
	    printf("%% a**m="); BnnPrint(w, wl); printf("\n");
	}
#endif
	ModSetToZero(x);
	BnnSetDigit(x, 1);
	xl = 1;
	for(lam = 0; lam <= qk; lam++){
	    if(!BnnCompare(x, xl, w, wl))
		break;
	    else{
#ifdef DEBUG
		if(debuglevel >= 6)
		    {printf("%% r**%d=", lam); BnnPrint(x, xl); printf("\n");}
#endif
		ModMultiply(x, x, r);
		xl = BnnNumDigits(x, Modul);
	    }
	}
	if(lam > qk){
	    printf("%%!%% Error in ModRoot, lambda > q**(k-1)\n");
	    return(0);
	}
#ifdef DEBUG
	if(debuglevel >= 3) printf("%% lambda=%d\n", lam);
#endif
	/* x <- 1/q mod m */
	BnnSetDigit(y, q);
	BnnEeaWithCopy(w, &wl, x, &xl, m, ml, y, 1);
	BmExp(y, &yl, a, al, x, xl);
	if(lam){
	    /* */
	    ModSetToZero(w);
	    BnnMultiplyDigit(w, xl+1, x, xl, q);
	    BnnSubtractBorrow(w, xl+1, 0);
	    wl = BnnNumDigits(w, xl+1);
	    /* ww <- 1/z mod Modu */
	    BnnEeaWithCopy(x, &xl, ww, &wwl, Modu, Modul, z, *p_zl);
	    BmExp(ww, &wwl, ww, wwl, w, wl);
	    BnnSetDigit(w, lam);
	    BmExp(ww, &wwl, ww, wwl, w, 1);
	    ModMultiply(y, y, ww);
	}
	*p_yl = BnnNumDigits(y, Modul);
	return(1);
    }
}

/* Same problem when k is large */
ModBinRootOddPrimeLarge(y, p_yl, a, al, q)
BigNum y, a;
int q, *p_yl, al;
{
    Poly mx, px;
    BigNum w = BNC(q*Modul+1), z = BNC(q*Modul+1), mq = BmCreate();
    BigNum b = BmCreate(), c = BmCreate(), d = BmCreate(), tmp;
    int i, cl, dl, bl, wl, zl, mql;

#ifdef DEBUG
    if(debuglevel >= 3) printf("%% Entering large mod root\n");
#endif
    BnnAssign(z, Modu, Modul);
    zl = Modul;
    /* z <- (p^q-1)/(p-1) = p^(q-1) + ... + 1 = ((...(p + 1) p) + 1)...+1 */
    for(i = 2; i < q; i++){
	if(BnnAddCarry(z, zl, 1)){
	    BnnSetDigit(z+zl, 1);
	    zl++;
	}
	wl = zl + Modul;
	BnnSetToZero(w, wl);
	BnnMultiply(w, wl, z, zl, Modu, Modul);
	wl = BnnNumDigits(w, wl);
	tmp = z; z = w; w = tmp;
	zl = wl;
    }
    if(BnnAddCarry(z, zl, 1)){
	BnnSetDigit(z+zl, 1);
	zl++;
    }
#ifdef DEBUG
    if(debuglevel >= 3)
	{printf("%% (p^q-1)/(p-1)="); BnnPrint(z, zl); printf("\n");}
#endif
    /* w <- z / q */
    if(BnnDivideDigit(w, z, zl+1, (BigNumDigit)q)){
	fprintf(stderr, "%% q does not divide w\n");
	return(0);
    }
    wl = BnnNumDigits(w, zl);
#ifdef DEBUG
    if(debuglevel >= 3)
	{printf("%% (p^q-1)/(p-1)/q="); BnnPrint(w, wl); printf("\n");}
#endif
    /* finds b s.t. d = a - b^q is a qth nonresidue mod p */
    BnnSetDigit(b, 1);
    bl = 1;
    ModAssign(z, Modu);
    BnnSubtractBorrow(z, Modul, 0);
    BnnSetDigit(z+Modul, 0);
    BnnDivideDigit(mq, z, Modul+1, q);
    mql = BnnNumDigits(mq, Modul);
    BnnSetDigit((z+Modul+1), q);
    do {
	BnnAddCarry(b, 1, 1);
	BmExp(c, &cl, b, 1, (z+Modul+1), 1);
	ModAssign(d, a);
	ModSubtract(d, c);
	dl = BnnNumDigits(d, Modul);
	BmExp(z, &zl, d, dl, mq, mql);
    } while(BnnIsOne(z, zl));
#ifdef DEBUG
    if(debuglevel >= 3){
	printf("%% b=%d\n", *b);
	printf("%% d="); BnnPrint(d, dl); printf("\n");
    }
#endif
    /* y <- (b+X)**w mod X^q-d */
    PolyInit(q);
    mx = PolyCreate(q);
    mx->deg = q;
    BnnSetDigit(mx->coeff[q], 1);
    ModNegate(d);
    BnnAssign(mx->coeff[0], d, BnnNumDigits(d, Modul));
#ifdef DEBUG
    if(debuglevel >= 3) {printf("MX="); PolyPrint(mx);}
#endif
    px = PolyCreate(q);
    px->deg = 1;
    BnnSetDigit(px->coeff[1], 1);
    BnnAssign(px->coeff[0], b, 1);
#ifdef DEBUG
    if(debuglevel >= 3) {printf("px="); PolyPrint(px);}
#endif
    PolyExpMod(px, px, w, wl, mx);
#ifdef DEBUG
    if(debuglevel >= 3) {printf("px**w="); PolyPrint(px);}
#endif
    ModAssign(y, px->coeff[0]);
    PolyClose();
    BnFree(w); BnFree(z); BmFree(b); BmFree(c); BmFree(d);
    BmFree(mq);
    *p_yl = BnnNumDigits(y, Modul);
    return(1);
}

/* Returns the maximum power k of q dividing n and m <- n/q**k. */
int ComputeValuation(m, p_ml, n, nl, q)
BigNum m, n;
int *p_ml, nl;
BigNumDigit q;
{
    BigNum m1, m2, tmp;
    int k, m1l;

    m1 = BNC(nl+1);
    m2 = BNC(nl+1);
    BnnAssign(m1, n, nl);
    m1l = nl;
    for(k = 0; ; k++){
	BnnSetDigit((m1+m1l), 0);
	if(BnnDivideDigit(m2, m1, m1l+1, q))
	    break;
	else{
	    tmp = m1; m1 = m2; m2 = tmp;
	    m1l = BnnNumDigits(m1, m1l);
	}
    }
    BnnAssign(m, m1, m1l);
    *p_ml = m1l;
    BnFree(m1); BnFree(m2);
    return(k);
}

/* Returns zz = q-th nonresidue mod p, z = zz^((p-1)/q) a primtive q-th
   root of 1 mod p. We suppose we are in the case p = 1 mod q and q is prime.
*/
FindPrimitiveRootModp(zz, p_zzl, z, p_zl, q)
BigNum z, zz;
int q, *p_zzl, *p_zl;
{
    BigNum m = BmCreate();
    int ml, zl;

    /* m <- (p-1)/q */
    ModAssign(zz, Modu);
    ml = Modul;
    BnnSubtractBorrow(zz, ml, 0);
    if(BnnDivideDigit(m, zz, ml+1, q))
	printf("Modu <> 1 mod p\n");
    ml = BnnNumDigits(m, ml);
#ifdef DEBUG
    if(debuglevel >= 3){printf("%% (p-1)/q="); BnnPrint(m, ml); printf("\n");}
#endif
    ModSetToZero(zz);
    BnnSetDigit(zz, 1);
    do {
	BnnAddCarry(zz, 1, 1);
	BmExp(z, &zl, zz, 1, m, ml);
    } while(BnnIsOne(z, zl));
#ifdef DEBUG
    if(debuglevel >= 3)
	{printf("%% %d-th nonresidue is ", q); BnnPrint(zz, 1); printf("\n");}
#endif
    *p_zl = zl; *p_zzl = 1;
    BmFree(m);
}

