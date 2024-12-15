
/* $Log:	nminus1.c,v $
 * Revision 1.1  91/09/20  14:54:42  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:29:25  morain
 * Initial revision
 *  */

/**********************************************************************
                               N-1 tests
**********************************************************************/

#include "exp.h"
#include "ifactor.h"

#define TRYMAX 500

extern int verbose, hyperquiet;

/* Thm: N-1 = m * p with p an odd prime s.t. 2*p+1 > sqrt(N). If there 
        exists an a for which a**((N-1)/2) = -1 mod N, but 
        a**(m/2) <> -1 mod N, then N is prime

   a is supposed to have 1 digit */
int Theorem3(a, N, Nl, p, pl)
BigNum a, N, p;
int Nl, pl;
{
    BigNum Nminus1 = BNC(Nl+1), m = BNC(Nl+1), b;
    long nr = 1;
    int ml = Nl-pl+1, al = 1, bl, pr;

    BnnAssign(Nminus1, N, Nl);
    BnnSubtractBorrow(Nminus1, Nl, 0);
    BnnAssign(m, Nminus1, Nl);
    BnnDivide(m, Nl+1, p, pl);
    if(!BnnIsZero(m, pl)){
	printf("Error, p does not divide N-1\n");
	exit(0);
    }
    /* m = m[pl..Nl-pl+1] */
    ml = BnnNumDigits((m+pl), ml);
#ifdef DEBUG
    if(debuglevel >= 2)
	{printf("%% m="); BnnPrint((m+pl), ml); printf("\n");}
#endif
    /* since m is even, no pb */
    BnnShiftRight((m+pl), ml, 1);
    ml = BnnNumDigits((m+pl), ml);
#ifdef DEBUG
    if(debuglevel >= 2)
	{printf("%% m/2="); BnnPrint(m+pl, ml); printf("\n");}
#endif
    b = BmCreate();
    while(1){
	if(!FindNonresidue(&nr, N, Nl, TRYMAX)){
	    printf("No nonresidue found in Theorem3\n");
	    return(0);
	}
#ifdef DEBUG
	if(debuglevel >= 2) printf("%% nonresidue=%ld\n", nr);
#endif
	BnnSetDigit(a, nr);
	BmExp(b, &bl, a, al, (m+pl), ml);
#ifdef DEBUG
	if(debuglevel >= 2)
	    {printf("%% b="); BnnPrint(b, bl); printf("\n");}
#endif
	if(!BnnCompare(b, bl, Nminus1, Nl)){
#ifdef DEBUG
        if(debuglevel >= 2)
	    {printf("%% b = -1\n");}
#endif
	    nr++;
	}
	else{
	    BmExp(b, &bl, b, bl, p, pl);
#ifdef DEBUG
	    if(debuglevel >= 2)
		{printf("%% b="); BnnPrint(b, bl); printf("\n");}
#endif
	    if(BnnCompare(b, bl, Nminus1, Nl)){
		printf("N is not a epsp\n");
		pr = 0; break;
	    }
	    else{
		pr = 1; break;
	    }
	}
    }
    BnFree(Nminus1); BnFree(m); BnFree(b);
    return(pr);
}

/* lF1 = [[p1, e1], ..., [pk, ek]]
   Returns 1 if for all i, there exists ai s.t.
   1. gcd(ai**((N-1)/pi)-1, N) = 1
   2. ai**(N-1) = 1
   Puts the ai in *p_la, in the right order.
*/
int ConditionI(p_la, N, Nl, lF1)
ListOfBigNum *p_la;
BigNum N;
int Nl;
ListOfFactors lF1;
{
    BigNum b, c, prod, Nminus1 = BNC(Nl), w = BNC(Nl+1), a = BNC(1);
    ListOfFactors tmp;
    ListOfBigNum foo = NULL, bar;
    long nr = 1;
    int newa = 1, al = 1, bl, cl, prodl = 1, wl, pr = 1;
    double tp, runtime();

    BnnAssign(Nminus1, N, Nl);
    BnnSubtractBorrow(Nminus1, Nl, 0);
    b = BmCreate(), c = BmCreate(); prod = BmCreate();
    BnnSetDigit(prod, 1);
    if(!FindNonresidue(&nr, N, Nl, TRYMAX)){
	printf("No nonresidue found\n");
	exit(0);
    }
#ifdef DEBUG
    if(debuglevel >= 2) printf("%% Nonresidue is %ld\n", nr);
#endif
    BnnSetDigit(a, nr);
    tmp = lF1;
    *p_la = NULL;
    tp = runtime();
    while(tmp != NULL){
	/* computes (N-1)/pi */
#ifdef DEBUG
	if(debuglevel >= 2){
	    printf("%% Let's check for ");
	    BnnPrint(tmp->p,tmp->pl); 
	    printf("\n");
	}
#endif
	BnnAssign(w, Nminus1, Nl);
	BnnSetDigit((w+Nl), 0);
	BnnDivide(w, Nl+1, tmp->p, tmp->pl);
	if(!BnnIsZero(w, tmp->pl)){
	    printf("Error in ConditionI\n");
	    exit(0);
	}
	wl = BnnNumDigits((w+tmp->pl), Nl+1-tmp->pl);
#ifdef DEBUG
	if(debuglevel >= 2)
	    {printf("%% (N-1)/pi="); BnnPrint(w+tmp->pl, wl); printf("\n");}
#endif
	while(1){
	    BmExp(b, &bl, a, al, (w+tmp->pl), wl);
	    if(BnnIsOne(b, bl)){
		newa = 1;
		if(verbose) printf("%% b=1\n");
		FindNonresidue(&nr, N, Nl, TRYMAX);
		if(verbose) printf("%% Nonresidue is %ld\n", nr);
		BnnSetDigit(a, nr);
	    }
	    else break;
	}
#ifdef DEBUG
	if(debuglevel >= 2){
	    printf("%% Taking a=");
	    BnnPrint(a, al);
	    printf(" at %lf s\n", runtime()/1000);
	    printf("%% b="); BnnPrint(b, bl); printf("\n");
	}
#endif
	if(newa){
	    /* checks that b**pi = 1 */
	    BmExp(c, &cl, b, bl, tmp->p, tmp->pl);
	    if(!BnnIsOne(c, cl)){
		if(!hyperquiet){
		    printf("%%!%% N is not a psp ");
		    BnnPrint(a, 1); printf("\n");
		}
		pr = 0;
		break;
	    }
	    newa = 0;
	}
	bar = ListOfBigNumAlloc();
	bar->n = BNC(1);
	BnnAssign(bar->n, a, 1);
	bar->cdr = NULL;
	if(foo == NULL){
	    foo = bar;
	    *p_la = foo;
	}
	else{
	    foo->cdr = bar;
	    foo = bar;
	}
#ifdef DEBUG
	if(debuglevel >= 2) printf("%% Check done at %lf s\n", runtime()/1000);
#endif
	BnnSubtractBorrow(b, bl, 0);
	if(BnnIsDigitZero(*(b+bl-1))) bl--;
	BmMultiply(prod, &prodl, prod, prodl, b, bl);
#ifdef DEBUG
	if(debuglevel >= 2)
	    {printf("%% prod="); BnnPrint(prod, prodl); printf("\n");}
#endif
	tmp = tmp->cdr;
    }
    if(pr){
	/* final check on (prod, N) = 1 */
	wl = BnnGcdWithCopy(w, prod, prodl, N, Nl);
	if(BnnIsOne(w, wl)){
	    pr = 1;
	}
	else{
	    printf("Gcd nontrivial with "); BnnPrint(prod, prodl);
	    printf("\nPlease check\n");
	    pr = 0;
	}
    }
    BnFree(b); BnFree(c); BnFree(prod); BnFree(Nminus1); 
    BnFree(w); BnFree(a);
    return(pr);
}

/* Test whether N < (B1*F1+1)*[2*F1^2+(r-B1)*F1+1] */
int Inequality(N, Nl, F1, F1l, B1, B1l, r, rl)
BigNum N, F1, B1, r;
int Nl, F1l, B1l, rl;
{
    BigZ zN = BzFromBigNum(N, Nl), zF1 = BzFromBigNum(F1, F1l);
    BigZ zB1 = BzFromBigNum(B1, B1l), zr = BzFromBigNum(r, rl), res, aux;
    int comp;

#ifdef DEBUG
    printf("%% zN="); BzPrint(zN); printf("\n");
    printf("%% zF1="); BzPrint(zF1); printf("\n");
    printf("%% zB1="); BzPrint(zB1); printf("\n");
    printf("%% zr="); BzPrint(zr); printf("\n");
#endif
    res = BzAdd(zF1, zF1);
    res = BzAdd(res, zr);
    res = BzSubtract(res, zB1);
    res = BzMultiply(res, zF1);
    res = BzAdd(res, BzFromString("1", 10));
    aux = BzMultiply(zB1, zF1);
    aux = BzAdd(aux, BzFromString("1", 10));
    res = BzMultiply(res, aux);
    comp = BzCompare(zN, res);
    comp = (comp == -1);
    BzFree(zN); BzFree(zF1); BzFree(zB1); BzFree(zr); 
    BzFree(res); BzFree(aux);
    return(comp);
}

/* Assumes that N is odd */
int Theorem5or7(N, Nl, F1, F1l, R1, R1l, B1, B1l, lF1)
BigNum N, F1, R1, B1;
int Nl, F1l, R1l, B1l;
ListOfFactors lF1;
{
    BigNum r = BNC(Nl+1), s = BNC(Nl+1), w = BNC(Nl+1);
    BigZ t, aux;
    ListOfBigNum listofa;
    int i, rl, sl, tl, wl, pr;

    /* Computing bounds */
    /* F1 <- 2*F1 */
    BnnShiftLeft(F1, F1l, 1); if(!BnnIsDigitZero(*(F1+F1l))) F1l++;
    BnnAssign(w, R1, R1l);
    BnnDivide(w, R1l+1, F1, F1l);
    /* r <- R1 mod (2*F1) */
    BnnAssign(r, w, F1l); rl = BnnNumDigits(r, F1l);
    printf("%% r="); BnnPrint(r, rl); printf("\n");
    /* s <- R1 / (2*F1) */
    sl = R1l-F1l+1;
    BnnAssign(s, (w+F1l), sl); sl = BnnNumDigits(s, sl);
    printf("%% s="); BnnPrint(s, sl); printf("\n");
    /* t <- r^2 - 8 * s */
    t = BzFromBigNum(r, rl); t = BzMultiply(t, t);
    aux = BzFromString("8", 10); 
    aux = BzMultiply(aux, BzFromBigNum(s, sl));
    t = BzSubtract(t, aux);
    BnnSetToZero(w, Nl+1);
    if(!BnnIsZero(s, sl) && BzIsSquare(w, t)){
	printf("%% N is composite\n");
	/* one has r >= w */
	wl = BnnNumDigits(w, Nl+1);
	/* r <- (r-w)/2 */
	BnnSubtract(r, rl, w, wl, 1);
	rl = BnnNumDigits(r, rl);
	BnnShiftRight(r, rl, 1);
	rl = BnnNumDigits(r, rl);
	/* w <- 1 + F1*r */
	wl = Nl+1;
	BnnSetToZero(w, wl);
	BnnSetDigit(w, 1);
	BnnMultiply(w, wl, r, rl, F1, F1l);
	wl = BnnNumDigits(w, wl);
	printf("%% Factor is "); BnnPrint(w, wl); printf("\n");
	pr = 0;
    }
    else{
	/* F1 returns to its original value */
	BnnShiftRight(F1, F1l, 1); if(BnnIsDigitZero(*(F1+F1l-1))) F1l--;
	if(!Inequality(N, Nl, F1, F1l, B1, B1l, r, rl)){
	    printf("%% Inequality not satisfied\n");
	}
	else{
	    if(!ConditionI(&listofa, N, Nl, lF1)){
		printf("N is not prime\n");
		pr = 0;
	    }
	    else pr = 1;
	}
    }
    BnFree(r); BnFree(s); BnFree(w);
    BzFree(t); BzFree(aux);
    return(pr);
}

int Theorem1(p_la, N, Nl, lF1)
BigNum N;
int Nl;
ListOfFactors lF1;
ListOfBigNum *p_la;
{
    return(ConditionI(p_la, N, Nl, lF1));
}

int Theorem5(N, Nl, F1, F1l, R1, R1l, lF1)
BigNum N, F1, R1;
int Nl, F1l, R1l;
ListOfFactors lF1;
{
    BigNumDigit Un[1];

    BnnSetDigit(Un, 1);
    return(Theorem5or7(N, Nl, F1, F1l, R1, R1l, Un, 1, lF1));
}

/* Assumes that all prime factors of R1 are > B1 */
int Theorem7(N, Nl, F1, F1l, R1, R1l, B1, B1l, lF1)
BigNum N, F1, R1, B1;
int Nl, F1l, R1l, B1l;
ListOfFactors lF1;
{
    return(Theorem5or7(N, Nl, F1, F1l, R1, R1l, B1, B1l, lF1));
}

