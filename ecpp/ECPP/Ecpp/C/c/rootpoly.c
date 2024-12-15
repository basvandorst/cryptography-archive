
/* $Log:	rootpoly.c,v $
 * Revision 1.1  91/09/20  14:55:27  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:30:19  morain
 * Initial revision
 *  */

/**********************************************************************
             Computing roots of polynomials modulo p
**********************************************************************/

#include "mod.h"
#include "poly.h"

#define NBUMAX    100 /* limit on the number of splitting u's in Berlekamp */
#define DEGSWITCH   2 /* the degree for which we stop using Berlekamp */

/* Returns 1 if PX(x)=0 mod Modu and 0 otherwise */
int PolyIsRootOf(PX, x, xl)
Poly PX;
BigNum x;
{
    BigMod r = BmCreate();
    int rl, isz;

    rl = PolyHorner(r, x, xl, PX);
    isz = BnnIsZero(r, rl);
    BmFree(r);
    return(isz);
}
	 
/********** Berlekamp's "folk's method" **********/
    
/* Computing a root j of PX mod Modu. Returns 
   -1 if no root has been found (pb in Berlekamp),
    1 if j is a root,
    0 if j is not a root.
 */
int RootOfPolyModp(j, p_jl, PX)
BigMod j;
int *p_jl;
Poly PX;
{
    Poly PY = PolyCopy(PX), R = PolyCreate(PX->deg);
    BigNumDigit u[1];
    int fac = 1;

    BnnSetDigit(u, 0);
    R->deg = 0;
    while(PY->deg > DEGSWITCH){
	if(!(fac = PolyFactor(PY, R, u)))
	    break;
	else{
	    PolyMakeMonic(R);
	    if(R->deg >= 1)
		PolyAssign(PY, R);
	}
    }
    if(fac){
	switch(PY->deg){
	  case 1:
	    BnnAssign(j, Modu, Modul);
	    *p_jl = Modul;
	    BmSubtract(j, p_jl, PY->coeff[0], Modul);
	    break;
	  case 2:
	    fac = OneRootForDegree2(j, p_jl, PY);
	    break;
#if 0
	  case 3:
	    fac = OneRootForDegree3(j, p_jl, PY);
	    break;
	  case 4:
	    fac = OneRootForDegree4(j, p_jl, PY);
#endif
	}
    }
#ifdef DEBUG
    if(debuglevel >= 8)
	{printf("%% Root is "); BnnPrint(j, *p_jl); printf("\n");}
#endif
    PolyFree(PY); PolyFree(R);
    if(!fac)
	return(-1);
    else
	return(PolyIsRootOf(PX, j, *p_jl));
}

/* One step of Berlekamp: factorisation of P(X) modulo Modu, with */
/* result in R(X), of degree 1 < < deg(P). */
/* Returns 0 in case of problem */
int PolyFactor(P, R, u)
Poly P, R;
BigNum u;
{
    Poly Q = PolyCreate(P->deg);
    BigNum e = BmCreate();
    int el, fac = 1, nbu = 0, l;

    /* don't forget to update TP */
    TP = PolyCreate(P->deg);
    /* e <- (Modu-1)/2 */
    BnnAssign(e, Modu, Modul);
    BnnShiftRight(e, Modul, 1);
    el = BnnNumDigits(e, Modul);
    PolySetToZero(R, P->deg);
    /* if deg(R) = deg(P), then R = P */
    while((!R->deg) || (R->deg == P->deg)){
	PolySetToZero(R, P->deg);
	PolySetToZero(Q, P->deg);
	/* setting X + u */
	nbu++; if(nbu == NBUMAX){fac = 0; break;}
	BnnAddCarry(u, 1, 1);
	R->deg = 1;
	BnnAssign(R->coeff[0], u, 1);
	BnnSetDigit(R->coeff[1], 1);
	/* Q <- (X+u)**e mod P(X) mod Modu */
#ifdef DEBUG
	if(debuglevel >= 10) 
	    {printf("Ru=X+"); BnnPrint(u, 1); printf("\n");}
#endif
	PolyExpMod(Q, R, e, el, P);
#ifdef DEBUG
	if(debuglevel >= 10) {printf("R**e="); PolyPrint(Q);}
#endif
	/* Q(X) <- Q(X) - 1 */
	l = Modul;
	BmSubtract(Q->coeff[0], &l, (CONSTANTES+1), 1);
	/* R <- gcd(Q, P) */
	PolyAssign(R, P);
	PolyPgcd(TP, R, Q);
	PolyAssign(R, TP);
#ifdef DEBUG
	if(debuglevel >= 4) printf("#%d#\n", R->deg);
#endif
    }
    /* smallest factor if possible */
    if((R->deg << 1) > P->deg){
	/* degree(P/R) < degree(R) */
/* 	    (print "% J'ai mieux...")*/
	PolySetToZero(Q, P->deg);
/* 	    (print "% avant :" R)*/
	PolyQuoPoly(Q, P, R);
	/* 	    (print "% apres :" Q)*/
	PolyAssign(R, Q);
    }
    BmFree(e); PolyFree(Q);
    return(fac);
}

/********** Cantor-Zassenhaus **********/

/* Computing a root j of PX mod Modu. Returns 
   -1 if no root has been found (pb in Berlekamp),
    1 if j is a root,
    0 if j is not a root.
 */
int NewRootOfPolyModp(j, p_jl, PX)
BigMod j;
int *p_jl;
Poly PX;
{
    Poly PY = PolyCopy(PX), R = PolyCreate(PX->deg);
    BigNumDigit u[1];
    int fac;

    BnnSetDigit(u, 0);
    R->deg = 0;
    while(PY->deg > DEGSWITCH){
	if(!(fac = PolySeparate(PY, R, u)))
	    break;
	else{
	    /* 1 <= deg(R) <= deg(PY)/2 */
	    if(R->deg >= 1){
		PolyMakeMonic(R);
		PolyAssign(PY, R);
	    }
	    if(PY->deg > DEGSWITCH){
		BnnAddCarry(u, 1, 1);
		PolyIncr(PY);
	    }
	}
    }
    if(fac){
	switch(PY->deg){
	  case 1:
	    BnnAssign(j, Modu, Modul);
	    *p_jl = Modul;
	    BmSubtract(j, p_jl, PY->coeff[0], Modul);
	    break;
	  case 2:
	    fac = OneRootForDegree2(j, p_jl, PY);
	    break;
#if 0
	  case 3:
	    fac = OneRootForDegree3(j, p_jl, PY);
	    break;
	  case 4:
	    fac = OneRootForDegree4(j, p_jl, PY);
#endif
	}
    }
    PolyFree(PY); PolyFree(R);
    if(!fac)
	return(-1);
    else{
	/* coming back to a root of the original polynomial */
	if(!BnnIsDigitZero(u[0])){
	    *p_jl = Modul;
	    BmAdd(j, p_jl, u, 1);
	}
#ifdef DEBUG
	if(debuglevel >= 10){
	    printf("u="); BnnPrint(u, 1); printf("\n");
	    printf("j+u="); BnnPrint(j, *p_jl); printf("\n");
	}
#endif
	return(PolyIsRootOf(PX, j, *p_jl));
    }
}

/* 
   Computes R(X) = gcd(P(X+u), X**(p-1)/2-1) until R(X) is a nontrivial
   factor of P(X+u) of degree 1 < < deg(P). Returns 0 in case of problem.
*/
int PolySeparate(P, R, u)
Poly P, R;
BigNum u;
{
    Poly Q = PolyCreate(P->deg);
    BigNum e = BmCreate();
    int el, fac = 1, nbu = 0, l;

#ifdef DEBUG
    if(debuglevel >= 8) {printf("Operating on "); PolyPrint(P);}
#endif
    /* don't forget to update TP */
    TP = PolyCreate(P->deg);
    /* e <- (Modu-1)/2 */
    BnnAssign(e, Modu, Modul);
    BnnShiftRight(e, Modul, 1);
    el = BnnNumDigits(e, Modul);
    PolySetToZero(R, P->deg);
    /* if deg(R) = deg(P), then R = P */
    while(1){
	PolySetToZero(R, P->deg);
	PolySetToZero(Q, P->deg);
	/* Q <- X**e mod P(X) mod Modu */
	PolyXPowerMod(Q, e, el, P);
#ifdef DEBUG
	if(debuglevel >= 10) {printf("X**e="); PolyPrint(Q);}
#endif
	/* Q(X) <- Q(X) - 1 */
	l = Modul;
	BmSubtract(Q->coeff[0], &l, (CONSTANTES+1), 1);
	/* R <- gcd(Q, P) */
	PolyAssign(R, P);
	PolyPgcd(TP, R, Q);
	PolyAssign(R, TP);
#ifdef DEBUG
	printf("#%d#\n", R->deg);
#endif
	if((R->deg) && (R->deg < P->deg))
	    break;
	else{
	    /* P(X) <- P(X+1) */
	    nbu++; if(nbu == NBUMAX){fac = 0; break;}
	    BnnAddCarry(u, 1, 1);
	    PolyIncr(P);
	}
    }
    /* smallest factor if possible */
    if((R->deg << 1) > P->deg){
	/* degree(P/R) < degree(R) */
/* 	    (print "% J'ai mieux...")*/
	PolySetToZero(Q, P->deg);
/* 	    (print "% avant :" R)*/
	PolyQuoPoly(Q, P, R);
	/* 	    (print "% apres :" Q)*/
	PolyAssign(R, Q);
    }
    BmFree(e); PolyFree(Q);
    return(fac);
}
