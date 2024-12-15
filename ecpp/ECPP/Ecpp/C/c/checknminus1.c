
/* $Log:	checknminus1.c,v $
 * Revision 1.1  91/09/20  14:53:07  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:27:32  morain
 * Initial revision
 *  */

/**********************************************************************
                         Checkling the N-1 tests
**********************************************************************/

#include "exp.h"
#include "ifactor.h"
#include "pratt.h"

int CheckTheorem1(la, N, Nl, lF1)
ListOfBigNum la;
BigNum N;
int Nl;
ListOfFactors lF1;
{
    BigNum b = BmCreate(), c = BmCreate(), Nminus1 = BNC(Nl), w = BNC(Nl+1), a;
    ListOfFactors tmp;
    long nr = 1;
    int al = 1, bl, cl, wd, wl, pr = 1, N1l = Nl, is2;
    double tp, runtime();

    /* Nminus1 <- N-1 */
    BnnAssign(Nminus1, N, Nl);
    BnnSubtractBorrow(Nminus1, Nl, 0);
    tmp = lF1;
    tp = runtime();
    while(tmp != NULL){
#ifdef DEBUG
	if(debuglevel >= 2)
	    {printf("%% pi="); BnnPrint(tmp->p, tmp->pl); printf("\n");}
#endif
	BnnAssign(w, Nminus1, N1l);
	BnnSetDigit((w+N1l), 0);
	/* computes (N-1)/pi */
	BnnDivide(w, N1l+1, tmp->p, tmp->pl);
	if(!BnnIsZero(w, tmp->pl)){
	    printf("%%!%% Error in Theorem 1\n");
	    return(0);
	}
	wd = tmp->pl;
	wl = BnnNumDigits((w+wd), N1l+1-tmp->pl);
#ifdef DEBUG
	if(debuglevel >= 2)
	    {printf("%% (N-1)/pi="); BnnPrint(w+wd, wl); printf("\n");}
#endif
	a = la->n; la = la->cdr;
	BmExp(b, &bl, a, al, (w+wd), wl);
#ifdef DEBUG
	if(debuglevel >= 2)
	    {printf("%% a**((N-1)/pi)="); BnnPrint(b, bl); printf("\n");}
#endif
	if(BnnIsOne(b, bl)){
	    printf("%%!%% Error b=1\n");
	    pr = 0;
	    break;
	}
	/* checks that b**pi = 1 */
	BmExp(c, &cl, b, bl, tmp->p, tmp->pl);
#ifdef DEBUG
	if(debuglevel >= 2)
	    {printf("%% b**pi="); BnnPrint(c, cl); printf("\n");}
#endif
	if(!BnnIsOne(c, cl)){
	    printf("%%!%% N is not a psp ");BnnPrint(a, 1);printf("\n");
	    pr = 0;
	}
    	tmp = tmp->cdr;
    }
    BnFree(b); BnFree(c); BnFree(Nminus1); BnFree(w);
    return(pr);
}

int CheckTheorem2(la, N, Nl, lF1)
ListOfBigNum la;
BigNum N;
int Nl;
ListOfFactors lF1;
{
    BigNum b, c, Nminus1 = BNC(Nl), w = BNC(Nl+1), a;
    ListOfFactors tmp;
    long nr = 1;
    int al = 1, bl, cl, wd, wl, pr = 1, N1l = Nl, is2;
    double tp, runtime();

    /* Nminus1 <- (N-1)/2 */
    BnnAssign(Nminus1, N, Nl);
    BnnSubtractBorrow(Nminus1, Nl, 0);
    BnnShiftRight(Nminus1, Nl, 1);
    if(BnnIsDigitZero(*(Nminus1+N1l-1))) N1l--;
    b = BmCreate(); c = BmCreate();
    tmp = lF1;
    tp = runtime();
    while(tmp != NULL){
#ifdef DEBUG
	if(debuglevel >= 2)
	    {printf("%% pi="); BnnPrint(tmp->p, tmp->pl); printf("\n");}
#endif
	BnnAssign(w, Nminus1, N1l);
	BnnSetDigit((w+N1l), 0);
	if((is2 = !BnnCompare(tmp->p, tmp->pl, (CONSTANTES+2), 1))){
	    /* pi = 2 */
	    wd = 0; wl = N1l;
	}
	else{
	    /* computes (N-1)/2 pi */
	    BnnDivide(w, N1l+1, tmp->p, tmp->pl);
	    if(!BnnIsZero(w, tmp->pl)){
		printf("%%!%% Error in Theorem2\n");
		return(0);
	    }
	    wd = tmp->pl;
	    wl = BnnNumDigits((w+wd), N1l+1-tmp->pl);
#ifdef DEBUG
	    if(debuglevel >= 2)
		{printf("%% (N-1)/2 pi="); BnnPrint(w+wd, wl); printf("\n");}
#endif
	}
	a = la->n; la = la->cdr;
	BmExp(b, &bl, a, al, (w+wd), wl);
#ifdef DEBUG
	if(debuglevel >= 2)
	    {printf("%% a**((N-1)/2/pi)="); BnnPrint(b, bl); printf("\n");}
#endif
	if(BmIsMinusOne(b, bl) && !is2){
	    printf("%%!%% Error b=-1\n");
	    pr = 0;
	    break;
	}
	if(!is2){
	    /* checks that b**pi = -1 */
	    BmExp(c, &cl, b, bl, tmp->p, tmp->pl);
#ifdef DEBUG
	    if(debuglevel >= 2)
		{printf("%% b**pi="); BnnPrint(c, cl); printf("\n");}
#endif
	    if(!BmIsMinusOne(c, cl)){
		printf("%%!%% N is not a psp ");BnnPrint(a, 1);printf("\n");
		pr = 0;
	    }
	}
	tmp = tmp->cdr;
    }
    BnFree(b); BnFree(c); BnFree(Nminus1); BnFree(w);
    return(pr);
}

int CheckConditionI(la, N, Nl, lF1)
ListOfBigNum la;
BigNum N;
int Nl;
ListOfFactors lF1;
{
    BigNum b, c, prod, Nminus1 = BNC(Nl), w = BNC(Nl+1), a;
    ListOfFactors tmp;
    long nr = 1;
    int al = 1, bl, cl, prodl = 1, wl, pr = 1;
    double tp, runtime();

    BnnAssign(Nminus1, N, Nl);
    BnnSubtractBorrow(Nminus1, Nl, 0);
    b = BmCreate(), c = BmCreate(); prod = BmCreate();
    BnnSetDigit(prod, 1);
    tmp = lF1;
    tp = runtime();
    while(tmp != NULL){
	/* computes (N-1)/pi */
	BnnAssign(w, Nminus1, Nl);
	BnnSetDigit((w+Nl), 0);
	BnnDivide(w, Nl+1, tmp->p, tmp->pl);
	if(!BnnIsZero(w, tmp->pl)){
	    printf("%%!%% Error in ConditionI\n");
	    exit(0);
	}
	wl = BnnNumDigits((w+tmp->pl), Nl+1-tmp->pl);
#ifdef DEBUG
	printf("%% (N-1)/pi="); BnnPrint(w+tmp->pl, wl); printf("\n");
#endif
	a = la->n;
	BmExp(b, &bl, a, al, (w+tmp->pl), wl);
	if(BnnIsOne(b, bl)){
	    printf("%%!%% Error b=1\n");
	    pr = 0;
	    break;
	}
	/* checks that b*pi = 1 */
	BmExp(c, &cl, b, bl, tmp->p, tmp->pl);
	if(!BnnIsOne(c, cl)){
	    printf("%%!%% N is not a psp "); BnnPrint(a, 1); printf("\n");
	    pr = 0;
	    break;
	}
	BnnSubtractBorrow(b, bl, 0);
	if(BnnIsDigitZero(*(b+bl-1))) bl--;
	BmMultiply(prod, &prodl, prod, prodl, b, bl);
#ifdef DEBUG
	printf("%% prod="); BnnPrint(prod, prodl); printf("\n");
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
    BnFree(b); BnFree(c); BnFree(prod); BnFree(Nminus1); BnFree(w);
    return(pr);
}

/* Thm: N-1 = m * p with p an odd prime s.t. 2*p+1 > sqrt(N). If there 
        exists an a for which a**((N-1)/2) = -1 mod N, but 
        a**(m/2) <> -1 mod N, then N is prime

   a is supposed to have 1 digit */
int CheckTheorem3(a, N, Nl, p, pl)
BigNum a, N, p;
int Nl, pl;
{
    BigNum Nminus1 = BNC(Nl+1), m = BNC(Nl+1), b;
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
    BmExp(b, &bl, a, al, (m+pl), ml);
#ifdef DEBUG
    if(debuglevel >= 2)
	{printf("%% a^(m/2)="); BnnPrint(b, bl); printf("\n");}
#endif
    if(!BnnCompare(b, bl, Nminus1, Nl)){
	printf("%% a^(m/2) = -1\n");
	pr = 0;
    }
    else{
	BmExp(b, &bl, b, bl, p, pl);
#ifdef DEBUG
	if(debuglevel >= 2)
	    {printf("%% b="); BnnPrint(b, bl); printf("\n");}
#endif
	if(BnnCompare(b, bl, Nminus1, Nl)){
	    printf("N is not a epsp\n");
	    pr = 0;
	}
	else{
	    pr = 1;
	}
    }
    BnFree(Nminus1); BnFree(m); BmFree(b);
    return(pr);
}

#define NEXTL(lg, l) {(lg) = *((l)->n); (l) = (l)->cdr;}

int CheckPratt(lf)
ListOfBigNum lf;
{
    long tl[PRATTMAX];
    int i;

    i = 0;
    while(lf != NULL)
	NEXTL(tl[i++], lf);
    return(CheckPrattAux(tl, &i));
}

int CheckPrattAux(tl, p_i)
long tl[];
int *p_i;
{
    long p, g, tq[PRATTMAX];
    int nq, j, ok;

    p = tl[--(*p_i)];
    g = tl[--(*p_i)];
    if(p == 2) return(1);
    else{
	nq = tl[--(*p_i)];
	for(j = 0; j < nq; j++){
	    tq[j] = tl[*p_i-1];
	    ok = CheckPrattAux(tl, p_i);
	    if(!ok) break;
	}
	if(ok) ok = SmallCheckPratt(p, g, tq, nq);
    }
    return(ok);
}

/* Checks that g is a primitive root mod p */
int SmallCheckPratt(p, g, tq, nq)
long p, g, tq[];
int nq;
{
    long e;
    int i, q;

    if(SmallExpMod(g, p-1, p) != 1){
	printf("%%!%% %ld^(p-1) != 1 mod %ld\n", g, p);
	return(0);
    }
    else{
	for(i = 0; i < nq; i++){
	    q = tq[i];
	    if((p-1) % q){
		printf("%%!%% %ld != 0 mod %ld\n", p-1, q);
		return(0); 
	    }
	    else{
		e = (p-1)/q;
		if(SmallExpMod(g, e, p) == 1){
		    printf("%%!%% %ld^%ld != 1 mod %ld\n", g, e, p);
		    return(0);
		}
	    }
	}
	return(1);
    }
}
