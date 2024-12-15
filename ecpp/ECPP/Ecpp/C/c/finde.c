
/* $Log:	finde.c,v $
 * Revision 1.1  91/09/20  14:54:01  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:28:38  morain
 * Initial revision
 *  */

/**********************************************************************
          Finding the equation of the curve and a point on it
**********************************************************************/

#include "ecpp.h"
#include "gcd.h"
#include "ecmodp.h"

#define TRYMAX 200

int findE(p_E, D, h, g, A, B, sqrtd)
Ecm *p_E;
long D;
BigZ A, B;
BigNum sqrtd;
int h, g;
{
    BigNum j = BmCreate(), k = BmCreate(), a = BmCreate(), b = BmCreate();
    int jl, kl;

    if(D == 3)
	findEfor3(b, A, B);
    else{
	if(D == 4)
	    findEfor4(a, A, B);
	else{
	    findj(j, &jl, D, h, g, sqrtd);
#ifdef DEBUG
	    if(debuglevel >= 6) 
		{printf("%% j="); BnnPrint(j, jl); printf("\n");}
#endif
	    /* k <- j/(1728-j) */
	    BnnSetDigit(k, 1728);
	    kl = ModSubtract(k, j);
#ifdef DEBUG
	    if(debuglevel >= 6) 
		{printf("%% 1728-j="); BnnPrint(k, Modul); printf("\n");}
#endif
	    EcppEea(k, &kl, k, kl);
#ifdef DEBUG
	    if(debuglevel >= 6)
		{printf("%% 1/(1728-j)="); BnnPrint(k, kl); printf("\n");}
#endif
	    BnnSetToZero((k+kl), Modul-kl+1);
	    BmMultiply(k, &kl, k, kl, j, jl);
#ifdef DEBUG
	    if(debuglevel >= 6)
		{printf("%% k="); BnnPrint(k, Modul); printf("\n");}
#endif
	    /* b = 2 k */
	    ModAssign(b, k);
	    ModAdd(b, b);
	    /* a = 3 k */
	    ModAssign(a, k);
	    ModAdd(a, b);
	}
    }
    /* at this point, we have a candidate */
    p_E->a = a;
    p_E->b = b;
    if(verbose) printf("%% E found\n");
#ifdef DEBUG
    if(debuglevel >= 4){
	printf("%% E="); EcmPrint(*p_E); printf("\n");
    }
#endif
    BmFree(j); BmFree(k);
}

/* 4 Modu = A'2 + 3 B'^2 => pi = (A'+B'*sqrt(-3))/2 = (A'+B')/2 + B' rho. */
int findEfor3(eb, A, B)
BigNum eb;
BigZ A, B;
{
    BigZ u, v, bb, tmp, newA, dAB;
    BigNum nbb, q = BmCreate();
    BigNumDigit C[1];
    int r, s, dl, zl, i, nbbl;

    if(verbose) printf("%% Entering the D=3 business\n");
    /* A <- (A'+B')/2 */
    newA = BzAdd(A, B); 
    BnnShiftRight(BzToBn(newA), BzNumDigits(newA), 1);
    dAB = BzSubtract(newA, B);
#ifdef DEBUG
    if(debuglevel >= 8){
	printf("%% [A', B']=[%s, %s]\n", BzToString(A, 10), BzToString(B, 10));
	printf("%% [A, B]=[%s, %s]\n",BzToString(newA, 10), BzToString(B, 10));
    }
#endif
    /* (r, s) = (2(A-B) mod 3, B mod 3) = ((A'-B') mod 3, B' mod 3) */
    /* s <- B' mod 3 */
    BnnAssign(q, BzToBn(B), BzNumDigits(B));
    *C = BnnDivideDigit(q, q, Modul+1, *(CONSTANTES+3));
    s = (int)BnnGetDigit(C);
    if(s == 2) s = -1;
    if(BzGetSign(B) == -1) s = -s;
    /* r <- A' mod 3 */
    ModSetToZero(q);
    BnnAssign(q, BzToBn(A), BzNumDigits(A));
    *C = BnnDivideDigit(q, q, Modul+1, *(CONSTANTES+3));
    r = (int)BnnGetDigit(C);
    if(r == 2) r = -1;
    if(BzGetSign(A) == -1) r = -r;
    r -= s;
    if(r > 0) r %= 3; else r = 3 - ((-r) % 3);
    if(r > 1) r -= 3;
#ifdef DEBUG
    if(debuglevel >= 8) printf("%% r=%d s=%d\n", r, s);
#endif
    /* solving (A-B) v + B u = s */
    Bezout(&v, &u, dAB, B, s);
#ifdef DEBUG
    if(debuglevel >= 8){
	printf("%% u=");
	BzPrint(u); printf(" v=");
	BzPrint(v); printf("\n");
    }
#endif
    /* computing bb = -r + A u - B v mod Modu */
    bb = BzFromInteger(-r); /* <- valid ? */
    tmp = BzMultiply(newA, u);
    bb = BzAdd(bb, tmp);
    tmp = BzMultiply(B, v);
    bb = BzSubtract(bb, tmp);
    bb = BzMod(bb, BzFromBigNum(Modu, Modul));
#ifdef DEBUG
    if(debuglevel >= 8) printf("%% bb=%s\n", BzToString(bb, 10));
#endif
    nbb = BzToBn(bb); nbbl = BzNumDigits(bb);
    /* Dt <- (Modu-1)/6 */
    BnnAssign(q, Modu, Modul);
    BnnSetToZero(Dbuf, Modul);
    BnnDivideDigit(Dbuf, q, Modul+1, *(CONSTANTES+6));
    dl = BnnNumDigits(Dbuf, Modul);
    /* looking for b s.t. b**Dt = nbb */
    ModSetToZero(Zbuf);
    zl = 1;
    for(i = 0; i < TRYMAX; i++){
	if(!BnnCompare(nbb, nbbl, Zbuf, zl))
	    break;
	else{
	    BnnAddCarry(eb, 1, 1);
	    BmExp(Zbuf, &zl, eb, 1, Dbuf, dl);
#ifdef DEBUG
	    if(debuglevel >= 10){
		printf("b="); BnnPrint(eb, 1); printf("\n");
		printf("b^((p-1)/6)="); BnnPrint(Zbuf, zl); printf("\n");
	    }
#endif
	}
    }
    if(i == TRYMAX){
	printf("%%!%% b not found after %d trials\n", TRYMAX);
	exit(0);
    }
    /* eb <- b/4 */
    BmShiftRight(eb, Modul, 2);
    BzFree(u); BzFree(v); BzFree(bb); BzFree(newA); BzFree(dAB); BmFree(q);
}

/* Uses Zbuf and Dtampon */
/* 4 Modu = A'^2 + 4 B'^2 => Modu = (A'/2)^2 + B'^2 */
int findEfor4(ea, A, B)
BigNum ea;
BigZ A, B;
{
    BigZ u, v, aa, tmp;
    BigNum naa;
    BigNumDigit C[1];
    int r = 0, s = 0, rrr, sss, dl, zl, i, naal;

    if(verbose) printf("%% Entering the D=4 business\n");
    /* A <- A / 2 */
    BnnShiftRight(BzToBn(A), BzNumDigits(A), 1);
#ifdef DEBUG
    if(debuglevel >= 8){
	printf("%% A=%s\n", BzToString(A, 10));
	printf("%% B=%s\n", BzToString(B, 10));
    }
#endif
    /* (r, s) = (0, (B-A) mod 4) if A is even 
                ((A-B) mod 4, 0) if B is even */
    if(BnnIsDigitOdd(*(BzToBn(A))))
	r = 1;
    else s = 1;
    /* C <- A mod 4 */
    BnnAssign(C, BzToBn(A), 1);
    BnnAndDigits(C, (BigNumDigit)3);
    rrr = (int)BnnGetDigit(C);
    /* rrr <- A mod 4, 0 <= rrr < 4 */
    if((rrr) && (BzGetSign(A) == -1))
	rrr = -rrr;
    BnnAssign(C, BzToBn(B), 1);
    BnnAndDigits(C, (BigNumDigit)3);
    sss = (int)(BnnGetDigit(C));
    /* sss <- B mod 4, 0 <= sss < 4 */
    if((sss) && (BzGetSign(B) == -1))
	sss = -sss;
    rrr -= sss;
    /* rrr <- (A-B) mod 4, -2 <= rrr <= 2 */
    if(rrr > 0) rrr %= 4; else rrr = 4 - ((-rrr) % 4);
    if(rrr > 2) rrr -= 4;
    if(r)
	r = rrr;
    else s = -rrr;
#ifdef DEBUG
    if(debuglevel >= 8) printf("%% r=%d s=%d\n", r, s);
#endif
    /* solving A v + B u = s */
    Bezout(&v, &u, A, B, s);
#ifdef DEBUG
    if(debuglevel >= 8){
	printf("%% u=");
	BzPrint(u); printf(" v=");
	BzPrint(v); printf("\n");
    }
#endif
    /* computing aa = r + A u - B v mod Modu */
    aa = BzFromInteger(r); /* <- valid ? */
    tmp = BzMultiply(A, u);
    aa = BzAdd(aa, tmp);
    tmp = BzMultiply(B, v);
    aa = BzSubtract(aa, tmp);
    aa = BzMod(aa, BzFromBigNum(Modu, Modul));
#ifdef DEBUG
    if(debuglevel >= 8) printf("%% aa=%s\n", BzToString(aa, 10));
#endif
    naa = BzToBn(aa); naal = BzNumDigits(aa);
    /* Dt <- (Modu-1)/4 */
    BnnAssign(Dbuf, Modu, Modul);
    BnnShiftRight(Dbuf, Modul, 2);
    dl = BnnNumDigits(Dbuf, Modul);
    /* looking for a s.t. a**Dt = naa */
    ModSetToZero(Zbuf);
    zl = 1;
    for(i = 0; i < TRYMAX; i++){
	if(!BnnCompare(naa, naal, Zbuf, zl))
	    break;
	else{
	    BnnAddCarry(ea, 1, 1);
	    BmExp(Zbuf, &zl, ea, 1, Dbuf, dl);
	}
    }
    BzFree(u); BzFree(v); BzFree(aa);
    if(i == TRYMAX){
	printf("a not found\n");
	exit(0);
    }
    if(BnnIsDigitOdd(*Dbuf))
	ModNegate(ea);
}
