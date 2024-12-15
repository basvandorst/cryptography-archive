
/* $Log:	cllucas.c,v $
 * Revision 1.1  91/09/20  14:53:14  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:27:48  morain
 * Initial revision
 *  */

/**********************************************************************
                  N+1 test a` la Cohen-Lenstra
**********************************************************************/

#include "ifactor.h"
#include "exp.h"

#define TRIALMAX 50

#define CLIsOne(x0, x1) (BnnIsOne((x0), Modul) && BnnIsZero((x1), Modul))

/*
   We work over A = (Z/NZ)[T]/(T^2-u*T-a) where
    u = 0, (a/N) = -1        if N = 1 mod 4
    a = 1, ((u^2+4)/N) = -1  if N = 3 mod 4
*/

/* Assumes that N is odd */
int Theorem17or19(N, Nl, F2, F2l, R2, R2l, B2, B2l, lF2)
BigNum N, F2, R2, B2;
int Nl, F2l, R2l, B2l;
ListOfFactors lF2;
{
    BigNum r = BNC(Nl+1), s = BNC(Nl+1), w = BNC(Nl+2);
    BigZ t, aux;
    ListOfBigNum listofa;
    ListOfFactors tmp;
    int i, rl, sl, tl, wl;

    /* Computing bounds */
    /* F2 <- 2*F2 */
    BnnShiftLeft(F2, F2l, 1); if(!BnnIsDigitZero(*(F2+F2l))) F2l++;
    BnnAssign(w, R2, R2l);
    BnnDivide(w, R2l+1, F2, F2l);
    /* s <- R2 / (2*F2) */
    sl = R2l-F2l+1;
    BnnAssign(s, (w+F2l), sl); sl = BnnNumDigits(s, sl);
    /* r <- R2 mod (2*F2) with abs(r) < F2 */
    BnnAssign(r, w, F2l); rl = BnnNumDigits(r, F2l);
    /* F2 returns to its original value */
    BnnShiftRight(F2, F2l, 1); if(BnnIsDigitZero(*(F2+F2l-1))) F2l--;
    if(BnnCompare(r, rl, F2, F2l) == 1){
	/* r <- 2*F2 - r*/
	BnnSetToZero(w, Nl+2);
	BnnAssign(w, F2, F2l); wl = F2l;
	if(BnnAdd(w, wl, F2, F2l, 0)){
	    BnnSetDigit((w+wl), 1);
	    wl++;
	}
	BnnSubtract(w, wl, r, rl, 1);
	BnnSetToZero(r, Nl+1);
	rl = BnnNumDigits(w, wl);
	BnnAssign(r, w, rl);
	/* s <- s+1 */
	if(BnnAddCarry(s, sl, 1)){
	    BnnSetDigit((s+sl), 1);
	    sl++;
	}
    }
    printf("abs(r)="); BnnPrint(r, rl); printf("\n");
    printf("s="); BnnPrint(s, sl); printf("\n");
    /* t <- r^2 + 8 * s */
    t = BzFromBigNum(r, rl); t = BzMultiply(t, t);
    aux = BzFromString("8", 10); 
    aux = BzMultiply(aux, BzFromBigNum(s, sl));
    t = BzAdd(t, aux);
    BnnSetToZero(w, Nl+1);
    if(!BnnIsZero(s, sl) && BzIsSquare(w, t)){
	printf("N is composite\n");
	/* one has w >= r */
	wl = BnnNumDigits(w, Nl+1);
	/* w <- (w-r)/2 */
	BnnSubtract(w, wl, r, rl, 1);
	wl = BnnNumDigits(w, wl);
	BnnShiftRight(w, wl, 1);
	wl = BnnNumDigits(w, wl);
	/* r <- 1 + F2*w */
	rl = Nl+1;
	BnnSetToZero(r, rl);
	BnnSetDigit(r, 1);
	BnnMultiply(r, rl, w, wl, F2, F2l);
	rl = BnnNumDigits(r, rl);
	printf("Factor is "); BnnPrint(r, rl); printf("\n");
	return(0);
    }
    else{
	if(!Inequality2(N, Nl, F2, F2l, B2, B2l, r, rl)){
	    printf("Inequality not satisfied\n");
	}
	else{
	    if(!CLLucas(&listofa, N, Nl, lF2)){
		printf("N is not prime\n");
		return(0);
	    }
	    else return(1);
	}
    }
}

int Theorem13(N, Nl, lF2)
BigNum N;
int Nl;
ListOfFactors lF2;
{
    ListOfBigNum lm;

    return(CLLucas(&lm, N, Nl, lF2));
}

int Theorem17(N, Nl, F2, F2l, R2, R2l, lF2)
BigNum N, F2, R2;
int Nl, F2l, R2l;
ListOfFactors lF2;
{
    BigNum Un = BNC(1);

    BnnSetDigit(Un, 1);
    return(Theorem17or19(N, Nl, F2, F2l, R2, R2l, Un, 1, lF2));
}

/* Assumes that all prime factors of R2 are > B2 */
int Theorem19(N, Nl, F2, F2l, R2, R2l, B2, B2l, lF2)
BigNum N, F2, R2, B2;
int Nl, F2l, R2l, B2l;
ListOfFactors lF2;
{
    return(Theorem17or19(N, Nl, F2, F2l, R2, R2l, B2, B2l, lF2));
}

/* Based on p. 112--114
  *p_lm contains
    a if N = 1 mod 4
    u if N = 3 mod 4
  followed by the m_i s.t. (alpha+m_i)/(alphabar+m_i) satisfies the theorem.
*/
int CLLucas(p_lm, N, Nl, lF2)
ListOfBigNum *p_lm;
BigNum N;
int Nl;
ListOfFactors lF2;
{
    ListOfBigNum foo;
    BigNumDigit a[1], u[1];
    int ok;

    /* initializations */
    u[0] = 0; a[0] = 0;
    if(!CLInit(u, a, N, Nl)){
	printf("%%!%% N is composite\n");
	return(0);
    }
    else{
#ifdef DEBUG
	if(debuglevel >= 2) printf("%% u=%ld a=%ld\n", *u, *a);
#endif
	ok = CLConditionIII(p_lm, N, Nl, lF2, u, a);
	if(ok){
	    foo = ListOfBigNumAlloc();
	    foo->n = BNC(1);
	    if(BnnIsOne(a, 1))
		BnnAssign(foo->n, u, 1);
	    else
		BnnAssign(foo->n, a, 1);
	    foo->cdr = *p_lm;
	    *p_lm = foo;
	}
	return(ok);
    }
}

/* 
   lF2 = [[p1, a1], ..., [pk, ak]]
   Returns 1 if for all i, there exists xi in A s.t.
   xi**((N+1)/pi != 1 but xi**(N+1) == 1.
*/
int CLConditionIII(p_lm, N, Nl, lF2, u, a)
ListOfBigNum *p_lm;
BigNum N, u, a;
int Nl;
ListOfFactors lF2;
{
    BigNum Nplus1 = BNC(Nl), w = BNC(Nl+1), m = BNC(1);
    BigMod prod = BmCreate();
    BigMod x0 = BmCreate(), x1 = BmCreate();
    BigMod y0 = BmCreate(), y1 = BmCreate();
    BigMod z0 = BmCreate(), z1 = BmCreate();
    ListOfFactors tmp;
    ListOfBigNum foo = NULL, bar;
    int newx = 1, prodl = 1, wl, Nplus1l = Nl, pr = 1;
    double tp, runtime();

    BnnAssign(Nplus1, N, Nl);
    if(BnnAddCarry(Nplus1, Nl, 1)){
	BnnSetDigit((Nplus1+Nl), 1);
	Nplus1l++;
    }
    *p_lm = NULL;
    BnnSetDigit(m, 0);
    CLFindNorm1(x0, x1, u, a, m);
    BnnSetDigit(prod, 1);
    tmp = lF2;
    tp = runtime();
    while(tmp != NULL){
	/* computes (N+1)/pi */
#ifdef DEBUG
	if(debuglevel >= 4){
	    printf("%% Let's check for ");
	    BnnPrint(tmp->p, tmp->pl);
	    printf("\n");
	}
#endif
	BnnAssign(w, Nplus1, Nplus1l);
	if(Nplus1l == Nl) BnnSetDigit((w+Nl), 0);
	BnnDivide(w, Nl+1, tmp->p, tmp->pl);
	if(!BnnIsZero(w, tmp->pl)){
	    printf("%%!%% Error in ConditionIII: not a divisor\n");
	    pr = 0;
	    break;
	}
	wl = BnnNumDigits((w+tmp->pl), Nl+1-tmp->pl);
#ifdef DEBUG
	if(debuglevel >= 4){
	    printf("%% (N+1)/pi=");
	    BnnPrint(w+tmp->pl, wl);
	    printf("\n");
	}
#endif
	while(1){
	    CLExp(y0, y1, x0, x1, u, a, (w+tmp->pl), wl);
#ifdef DEBUG
	    if(debuglevel >= 4){
		printf("%% (y0, y1)=");
		CLPrint(y0, y1);
	    }
#endif
	    if(CLIsOne(y0, y1)){
		newx = 1;
#ifdef DEBUG
		if(debuglevel >= 4) printf("%% (y0, y1)=1\n");
#endif
		CLFindNorm1(x0, x1, u, a, m);
	    }
	    else break;
	}
	if(newx){
	    /* checks that (y0, y1)**pi = 1 */
	    CLExp(z0, z1, y0, y1, u, a, tmp->p, tmp->pl);
#ifdef DEBUG
	    if(debuglevel >= 4){
		printf("%% (z0, z1)=");
		CLPrint(z0, z1);
	    }
#endif
	    if(!CLIsOne(z0, z1)){
		printf("%%!%% N is not a Lucas-psp\n");
		pr = 0;
		break;
	    }
	    /* just to make sure that each time (y**(N+1)/p)**p=1 */
	    newx = 0;
	}
	bar = ListOfBigNumAlloc();
	bar->n = BNC(1);
	BnnAssign(bar->n, m, 1);
	bar->cdr = NULL;
	if(foo == NULL){
	    foo = bar;
	    *p_lm = foo;
	}
	else{
	    foo->cdr = bar;
	    foo = bar;
	}
#ifdef DEBUG
	if(debuglevel >= 4) printf("%% Check done at %lf s\n", runtime()/1000);
#endif
	ModSubtractDigit(y0, CONSTANTES, 1);
	if(BnnIsZero(y0, Modul))
	    ModMultiply(prod, prod, y1);
	else
	    ModMultiply(prod, prod, y0);
	tmp = tmp->cdr;
    }
    if(pr){
	/* final check on (prod, N) = 1 */
	prodl = BnnNumDigits(prod, Modul);
	wl = BnnGcdWithCopy(w, prod, prodl, N, Nl);
	if(BnnIsOne(w, wl)){
	    foo = NULL;
	    pr = 1;
	}
	else{
	    printf("%%!%% Gcd nontrivial, please check\n");
	    BnnPrint(w, wl);
	    printf("\n");
	    pr = 0;
	}
    }
    BnFree(Nplus1); BnFree(w);
    BmFree(prod);
    BmFree(x0); BmFree(x1); BmFree(y0); BmFree(y1); BmFree(z0); BmFree(z1);
    return(pr);
}

/* Returns 1 if N passes the initializations */
int CLInit(u, a, N, Nl)
BigNum u, a, N;
int Nl;
{
    BigMod e = BmCreate();
    long nr;
    int Nmod4, el;

    /* a = N & 3 = N mod 4 */
    BnnSetDigit(a, 3);
    BnnAndDigits(a, *N);
    Nmod4 = BnnGetDigit(a);
    if((Nmod4 != 1) && (Nmod4 != 3)) return(0);
    if(Nmod4 == 1){
	BigMod y = BmCreate();
	int yl, el;

	/* N <- N-1 */
	BnnSubtractBorrow(N, Nl, 0);
	BnnAssign(e, N, Nl);
	BnnShiftRight(e, Nl, 1);
	el = BnnNumDigits(e, Nl);
	BnnSetDigit(a, 1);
	while(1){
	    BnnAddCarry(a, 1, 1);
	    BmExp(y, &yl, a, 1, e, el);
	    if(!BnnCompare(y, yl, N, Nl))
		/* y = -1 */
		break;
	    else{
		if(!BnnIsOne(y, yl)){
		    printf("%%!%% ");
		    BnnPrint(a, 1);
		    printf("**((N-1)/2) <> +/-1\n");
		    return(0);
		}
	    }
	}
	BnnAddCarry(N, Nl, 1);
#ifdef DEBUG
	if(debuglevel >= 4)
	    {printf("%% a="); BnnPrint(a, 1); printf("\n");}
#endif
	BmFree(e); BmFree(y);
	return(1);
    }
    else{
	BigMod x0 = BmCreate(), x1 = BmCreate();
	BigMod y0 = BmCreate(), y1 = BmCreate();

	BnnSetDigit(a, 1);
	nr = 1;
	while(LongJacobiBn(nr*nr+4, N, Nl) != -1) nr++;
#ifdef DEBUG
	if(debuglevel >= 4) printf("%% u=%ld\n", nr);
#endif
	BnnSetDigit(u, nr);
	/* verify that alpha**(N+1) = -1 in A */
	/* alpha**2 = 1 + u * alpha */
	BnnSetDigit(x0, 1);
	BnnAssign(x1, u, 1);
	/* e <- (N+1)/2 */
	BnnAssign(e, N, Nl);
	el = Nl;
	if(BnnAddCarry(e, el, 1)){
	    BnnSetDigit((e+el), 1);
	    el++;
	}
	BnnShiftRight(e, el, 1);
	if(BnnIsDigitZero(*(e+el-1))) el--;
	CLExp(y0, y1, x0, x1, u, a, e, el);
	/* N <- N-1 */
	BnnSubtractBorrow(N, Nl, 0);
	/* we should have y0 = N-1 and y1 = 0 */
	if(BnnCompare(y0, Nl, N, Nl) || !BnnIsZero(y1, Modul)){
	    printf("%%!%% alpha**(N+1) != -1\n");
	    printf("%% (y0, y1)=(");
	    BnnPrint(y0, Nl);
	    printf(", ");
	    BnnPrint(y1, Nl);
	    printf(")\n");
	    return(0);
	}
	else{
	    BnnAddCarry(N, Nl, 1);
	    BmFree(x0); BmFree(x1); BmFree(y0); BmFree(y1);
	}
    }
    return(1);
}

/* looking for alpha of norm 1 */
CLFindNorm1(x0, x1, u, a, m)
BigMod x0, x1, u, a, m;
{
    BigMod norm = BmCreate();
    int i, ok = 1;

    for(i = 0; i < TRIALMAX; i++){
	BnnAddCarry(m, 1, 1);
#ifdef DEBUG
	if(debuglevel >= 4) printf("%% m=%ld\n", *m);
#endif
	if(!CLComputeXofNorm1(x0, x1, u, a, m)){
	    ok = 0;
	    break;
	}
#ifdef DEBUG
	if(debuglevel >= 10){
	    printf("%% Candidate=");
	    CLPrint(x0, x1);
	}
#endif
	CLNorm(norm, x0, x1, u, a);
#ifdef DEBUG
	if(debuglevel >= 4)
	    {printf("%% Norm="); BnnPrint(norm, Modul); printf("\n");}
#endif
	if(BnnIsOne(norm, Modul))
	    break;
    }
    if(i == TRIALMAX){
	printf("%%!%% No candidate found\n");
	ok = 0;
    }
    BmFree(norm);
    return(ok);
}

/* Computes (x0, x1) = (alpha+m)/(alphabar+m) */
/* x0 = (m^2+a)/den, x1 = (2*m+u)/den */
/* Uses Dbuf */
int CLComputeXofNorm1(x0, x1, u, a, m)
BigNum x0, x1, u, a, m;
{
    BigMod den = BmCreate();
    int denl, gl, dl;

    /* Dbuf <- m*(m+u)-a */
    ModSetToZero(Dbuf);
    BnnAssign(Dbuf, m, 1);
    ModAddDigit(Dbuf, u, 0);
    ModMultiplyDigit(Dbuf, m, 0);
    ModSubtractDigit(Dbuf, a, 0);
    dl = BnnNumDigits(Dbuf, Modul);
    /* den <- 1/Dbuf */
    if(!BnnEeaWithCopy(Gbuf,&gl,den,&denl,Modu,Modul,Dbuf,dl)){
	printf("den=%s is noninvertible\n");
	printf("Factor of N is "); BnnPrint(Gbuf, gl); printf("\n");
	return(0);
    }
    else{
	/* x0 <- (m^2+a)*den */
	ModSetToZero(x0);
	BnnAssign(x0, m, 1);
	ModMultiply(x0, x0, x0);
	ModAddDigit(x0, a, 0);
	ModMultiply(x0, x0, den);
#ifdef DEBUG
	if(debuglevel >= 10)
	    {printf("%% x0="); BnnPrint(x0, Modul); printf("\n");}
#endif
	/* x1 <- (2*m+u)*den */
	ModSetToZero(x1);
	BnnAssign(x1, m, 1);
	ModAdd(x1, x1);
	ModAddDigit(x1, u, 0);
	ModMultiply(x1, x1, den);
#ifdef DEBUG
	if(debuglevel >= 10) 
	    {printf("%% x1="); BnnPrint(x1, Modul); printf("\n");}
#endif
	return(1);
    }
}

/* Test whether N < (B2*F2-1)*[2*F2^2+(B2-abs(r))*F2+1] */
/* Warning: this time, r can be negative */
int Inequality2(N, Nl, F2, F2l, B2, B2l, r, rl)
BigNum N, F2, B2, r;
int Nl, F2l, B2l, rl;
{
    BigZ zN = BzFromBigNum(N, Nl), zF2 = BzFromBigNum(F2, F2l);
    BigZ zB2 = BzFromBigNum(B2, B2l), zr = BzFromBigNum(r, rl), res, aux;
    int comp;

#ifdef DEBUG
    if(debuglevel >= 10){
	printf("zN=%s\n", BzToString(zN, 10));
	printf("zF2=%s\n", BzToString(zF2, 10));
	printf("zB2=%s\n", BzToString(zB2, 10));
	printf("zr=%s\n", BzToString(zr, 10));
    }
#endif
    res = BzAdd(zF2, zF2);
    res = BzAdd(res, zB2);
    res = BzSubtract(res, zr);
    res = BzMultiply(res, zF2);
    res = BzAdd(res, BzFromString("1", 10));
    aux = BzMultiply(zB2, zF2);
    aux = BzSubtract(aux, BzFromString("1", 10));
    res = BzMultiply(res, aux);
    comp = BzCompare(zN, res);
    comp = (comp == -1);
    BzFree(zN); BzFree(zF2); BzFree(zB2);
    BzFree(res); BzFree(aux);
    return(comp);
}

