
/* $Log:	cornacchia.c,v $
 * Revision 1.1  91/09/20  14:53:20  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:27:55  morain
 * Initial revision
 *  */

/**********************************************************************
                       Cornacchia's algorithm
**********************************************************************/

#include "mod.h"

/* Returns 1 if 4 Modu = A^2 + D B^2 and 0 otherwise; sqrtd contains 
   sqrt(-D) mod Modu. */
int cornacchia(A, B, D, sqrtd)
BigNum A, B, sqrtd;
long D;
{
    int norm;

    if(!(D % 4)){
	norm = cornac1(A, B, D/4, sqrtd);
	ModAdd(sqrtd, sqrtd);
    }
    else{
	if((D % 8) == 3)
	    /* (D+1)/4 is even */
	    norm = cornac4(A, B, D, sqrtd);
	else{
	    /* (D+1)/4 is odd */
	    norm = cornac1(A, B, D, sqrtd);
	    if(norm){
		/* no carry since A, B < sqrt(Modu) */
		BnnAdd(A, Modul, A, Modul, 0);
		BnnAdd(B, Modul, B, Modul, 0);
	    }
	}
    }
    if(!norm){
#ifdef DEBUG
	if(debuglevel >= 10) printf("This is not a norm\n");
#endif
    }
    else{
	if(!(D % 4))
	    BnnAdd(A, Modul, A, Modul, 0);
#ifdef DEBUG
	if(debuglevel >= 10){
	    printf("%% A="); BnnPrint(A, Modul); printf("\n");
	    printf("%% B="); BnnPrint(B, Modul); printf("\n");
	}
#endif
    }
    return(norm);
}

/* Solving A^2+d*B^2 = Modu */
int cornac1(A, B, d, sqrtd)
BigNum A, B, sqrtd;
long d;
{
    BigMod x0 = BmCreate();
    int x0l, Al, ispcp;

    /* solving x^2=-d mod n */
    ModAssign(A, Modu);
    BnnSetDigit(x0, d);
    BnnSubtract(A, Modul, x0, 1, 1);
    EcppMdSqrt(x0, &x0l, A, BnnNumDigits(A, Modul));
    ModAssign(sqrtd, x0);
    /* we want p/2 < x0 < p */
    BnnAssign(A, Modu, Modul);
    BnnShiftRight(A, Modul, 1);
    Al = BnnNumDigits(A, Modul);
    if(BnnCompare(x0, x0l, A, Al) != 1){
	BnnComplement(x0, Modul);
	BnnAdd(x0, Modul, Modu, Modul, 1);
	x0l = BnnNumDigits(x0, Modul);
    }
    ispcp = cornac(A, B, x0, x0l, d, 0);
    BmFree(x0);
    return(ispcp);
}

/* Case D=3 mod 8.*/
int cornac4(A, B, d, sqrtd)
BigNum A, B, sqrtd;
long d;
{
    BigMod x0 = BmCreate();
    int x0l, Al, ispcp;

    /* solving x^2=-d mod Modu */
    ModAssign(A, Modu);
    BnnSetDigit(x0, d);
    BnnSubtract(A, Modul, x0, 1, 1);
    EcppMdSqrt(x0, &x0l, A, BnnNumDigits(A, Modul));
    ModAssign(sqrtd, x0);
    /* we want x0=1 mod 2 */
    if(!BnnIsDigitOdd(*x0)){
	/* x0 is even */
	BnnComplement(x0, Modul);
	BnnAdd(x0, Modul, Modu, Modul, 1);
	x0l = BnnNumDigits(x0, Modul);
    }
    ispcp = cornac(A, B, x0, x0l, d, 1);
    BmFree(x0);
    return(ispcp);
}

/* We have ml <= Modul+1; flag=1 if using (4*Modu, 2*Modu).
   Does not assume that x0 > m/2 */
int cornac(A, B, x0, x0l, d, flag)
BigNum A, B, x0;
long d;
int x0l, flag;
{
    BigNum rac = BmCreate(), w = BmCreate(), x2, m, ww;
    int Bl, racl = 0, wl = Modul, x2l, ml = Modul, ispcp = 0;

    ww = w;
    /* set up */
    if(!flag){
	racl = BnnSqrt(rac, Modu, Modul);
	m = BnCopy(Modu);
	BnnAssign(w, Modu, Modul);
    }
    else{
	/* w <- 4*p */
	BnnAssign(w, Modu, Modul);
	BnnShiftLeft(w, Modul+1, 2);
	m = BmCreate(); BnnAssign(m, w, Modul+1);
	racl = BnnSqrt(rac, w, BnnNumDigits(w, Modul+1));
	/* w <- w/2, no update of wl since w[wl]<=1 and division follows */
	BnnShiftRight(w, Modul+1, 1);
	if(!BnnIsDigitZero(*(m+Modul))) ml++;
    }
#ifdef DEBUG
    if(debuglevel >= 10) 
	{printf("Sqrt="); BnnPrint(rac, racl); printf("\n");}
#endif
    /* Euclide's */
    while(BnnCompare(x0, x0l, rac, racl) == 1){
#ifdef DEBUG
	if(debuglevel >= 10){
	    printf(" w="); BnnPrint(w, wl); printf("\n");
	    printf("x0="); BnnPrint(x0, x0l); printf("\n");
	}
#endif
	BnnDivide(w, wl+1, x0, x0l);
	BnnSetToZero((w+x0l), wl-x0l+1);
	wl = BnnNumDigits(w, x0l);
	x2 = x0; x0 = w; w = x2; x2l = x0l; x0l = wl; wl = x2l;
    }
    /* conclusion */
    /* B <- x0^2 */
    Bl = x0l << 1;
    BnnMultiply(B, Bl, x0, x0l, x0, x0l);
    if(BnnIsDigitZero(*(B+Bl-1))) Bl--;
#ifdef DEBUG
    if(debuglevel >= 10) {printf("%% x0^2="); BnnPrint(B, Bl); printf("\n");}
#endif
    /* B <- Cpl(B) */
    BnnComplement(B, ml);
#ifdef DEBUG
    if(debuglevel >= 10) {printf("%% -x0^2="); BnnPrint(B, ml); printf("\n");}
#endif
    /* B <- m+B+1=m-x0^2 */
    BnnAdd(B, ml, m, ml, 1);
    Bl = BnnNumDigits(B, ml);
    BnnSetToZero((B+Bl), Modul-Bl+1);
#ifdef DEBUG
    if(debuglevel >= 10)
	{printf("%% m-x0^2="); BnnPrint(B, Bl); printf("\n");}
#endif
    /* note that ml >= Modul and that if Bl=Modul+1, then B[Bl-1]=1 */
    if(!BnnDivideDigit(A, B, Modul+1, d)){
	/* m-x0^2 is divisible by d, quotient=A[0..Modul] */
/*	BnnSetToZero((A+Bl), Modul-Bl+1);*/
#ifdef DEBUG
	if(debuglevel >= 10)
	    {printf("%% (m-x0^2)/d="); BnnPrint(A, Modul+1); printf("\n");}
#endif
	ModSetToZero(B);
	if(BnnIsSquare(B, &Bl, A, BnnNumDigits(A, Modul+1))){
	    ModAssign(A, x0);
	    ispcp = 1;
	}
    }
    else{
#ifdef DEBUG
	if(debuglevel >= 6) {printf("%% d doesn't divide m-x0^2\n");}
#endif
    }
    BmFree(rac); BmFree(ww); BnFree(m);
    return(ispcp);
}
