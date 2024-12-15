
/* $Log:	factorm.c,v $
 * Revision 1.1  91/09/20  14:53:51  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:28:27  morain
 * Initial revision
 *  */

/**********************************************************************

                    Main file for ECPP-factoring process

**********************************************************************/

#include "ecpp.h"

/* Returns 1 if m is probably factored and 0 otherwise */
int factorm(p_lfact, m, ml, n, nl, A, B, D, difnp_file, all, gasp)
ListOfFactors *p_lfact;
BigNum m, n;
char *difnp_file;
int ml, all, nl, gasp;
long D;
BigZ A, B;
{
    BigNum m1 = BNC(nl+1), m2 = BNC(ml+1), smin = BNC(nl+1);
    int done, m1l, sminl, m2l;

    BnnAssign(m2, m, ml);
    m2l = ml;
/* WARNING: is m large enough (sz=Modul+1 enough for sieving ?) */
    done=ecppfactorm(m,&ml,p_lfact,difnp_file,Sievepmax,D,A,B,all,gasp);
    if(done == 3){
	if(verbose) printf("%% m is prime, we forget about it\n");
	done = 0;
    }
    if(D > 1){
	ComputeMaximalSmallestOrder(m1, &m1l, n, nl, *p_lfact);
	BnnDivide(m2, m2l+1, m1, m1l);
	m2l = BnnNumDigits((m2+m1l), m2l-m1l+1);
	EcmComputeBound(smin, &sminl, n, nl);
	if((BnnCompare(m1, m1l, smin, sminl) == -1) &&
	   (BnnCompare((m2+m1l), m2l, smin, sminl) == -1)){
	    if(verbose)
		printf("%%!%% Risk of too high noncyclicity, I give up\n");
#ifdef DEBUG
	    if(debuglevel >= 2){
		printf("%% m1(max)="); BnnPrint(m1, m1l); printf("\n");
		printf("%% m2(max)="); BnnPrint(m2+m1l, m2l); printf("\n");
		printf("%% smin   ="); BnnPrint(smin, sminl); printf("\n");
	    }
#endif
	    done = 0;
	}
    }
    if(done && cyclic && (!depth) && !IsSquareFree(*p_lfact)){
       /* for Eurocrypt only... */
	printf("%%!%% CYCLIC: non squarefree\n");
	done = 0;
    }
    BnFree(m1); BnFree(m2); BnFree(smin);
    return(done);
}

IsSquareFree(lfact)
ListOfFactors lfact;
{
    ListOfFactors foo;
    int sqf = 1;

    foo = lfact;
    while(foo != NULL){
	if(foo->e > 1){
	    sqf = 0;
	    break;
	}
	foo = foo->cdr;
    }
    return(sqf);
}

ComputeMaximalSmallestOrder(m1, p_m1l, n, nl, lfact)
BigNum m1, n;
int *p_m1l, nl;
ListOfFactors lfact;
{
    ListOfFactors foo;
    BigNum r = BNC(nl+1), rr = BNC(nl+1), tmp, mm1 = BNC(nl+1);
    int e, ee, rrl, m1l = 1, i;

    foo = lfact;
    BnnSetDigit(mm1, 1);
    BnnSubtractBorrow(n, nl, 0);
    while(foo != NULL){
	if(foo->e > 1){
	    BnnAssign(rr, n, nl);
	    rrl = nl;
	    if(e = DivideMax(rr, &rrl, r, foo->p, foo->pl)){
		/* mm1 <- mm1 * p^(min(e, foo->e/2)) */
		ee = (foo->e) >> 1;
		e = (e < ee) ? e : ee;
		for(i = 1; i <= e; i++){
		    BnnSetToZero(rr, nl+1);
		    BnnMultiply(rr, nl+1, mm1, m1l, foo->p, foo->pl);
		    tmp = mm1; mm1 = rr; rr = tmp;
		    m1l = BnnNumDigits(mm1, nl+1);
		}
	    }
	}
	foo = foo->cdr;
    }
    BnnAddCarry(n, nl, 1);
    *p_m1l = m1l;
    BnnAssign(m1, mm1, m1l);
    BnFree(r); BnFree(rr); BnFree(mm1);
}

/* Attempts to factor n[0..nl-1][0]
   Returns 
   0: if factorization not completed
   1: if factorization is complete (Modu=1)
   2: if factors found and cofactor is a probable prime
   3: if the original number is a pseudoprime (no factor found)
   Cofactor is put back in n0, being prprime or composite
   Uses first a sieve, then rho if userho != 0, then p-1 if usepollard != 0.
*/
int ecppfactorm(m0,p_m0l,p_lfact,difnp_file,spmax,D,Az,Bz,all,gasp)
BigNum m0, spmax;
int *p_m0l, all, gasp;
ListOfFactors *p_lfact;
char *difnp_file;
long D;
BigZ Az, Bz;
{
    BigNum m = BNC((*p_m0l)+1), g, mm;
    long b1, b2, itmax;
    int done = 0, nbgcd, ml, nbf, gl, userho, usepollard, e;

    BnnAssign(m, m0, *p_m0l);
    ml = *p_m0l;
    IfactorMethods(&userho, &usepollard, gasp);
    done = DivideAndCheck(m, &ml, p_lfact);
    if((D > 1) && !all && (done != 1)){
	g = BNC(ml+1); mm = BNC(ml+1);
	FindNonResPrimes(g, &gl, Az, Bz);
	if(!BnnIsOne(g, gl)){
	    /* get some extra factors ? */
#ifdef DEBUG
	    if(debuglevel >= 4){
		printf("%% (g="); BnnPrint(g, gl); printf(")^2 divides (m=");
		BnnPrint(m, ml); printf(")\n");
	    }
#endif
	    gl = BnnGcdWithCopy(g, g, gl, m, ml);
	    if(!BnnIsOne(g, gl)){
		/* yep */
#ifdef DEBUG
		if(debuglevel >= 4){
		    printf("%% (f="); BnnPrint(g, gl); 
		    printf(")^2 divides (m=");
		    BnnPrint(m, ml); printf(")\n");
		}
#endif
		if(!(e = DivideMax(m, &ml, mm, g, gl)))
		   printf("%%!%% g does not divide m\n");
		else AppendFactor(p_lfact, g, gl, e);
#ifdef DEBUG
		if(debuglevel >= 4)
		    {printf("%% m/f^%d=", e); BnnPrint(m, ml); printf("\n");}
#endif
		done = BnnIsOne(m, ml);
	    }
	}
	BnFree(g); BnFree(mm);
    }
    if(done == 1){
	if(verbose) printf("%% Factorization completed using sieve only\n");
	BnnSetToZero(m0, *p_m0l);
	BnnAssign(m0, m, ml);
	*p_m0l = ml;
    }
    else{
	BmInit(m);
	if(IsProbablePrime(Modu, Modul)){
	    if(verbose) 
		printf("%% Cofactor after sieve is a probable prime\n");
	    if(*p_lfact != NULL) return(2); else return(3);
	}
	else{
	    SetFactorParameters(&itmax, &nbgcd, &b1, &b2, Modul);
	    if(userho){
		done = rho(p_lfact, itmax, nbgcd);
		if(done){
		    if(verbose)
			printf("%% Factorization completed using Rho\n");
		}
	    }
	    if(!done && usepollard){
		/* the number we try to factor is composite... */
		done = pollard(p_lfact, b1, b2, nbgcd, difnp_file);
		if(done){
		    if(verbose)
			printf("%% Factorization completed using p-1\n");
		}
	    }
	    BnnSetToZero(m0, *p_m0l);
	    BnnAssign(m0, Modu, Modul);
	    *p_m0l = Modul;
	}
    }
    BnFree(m);
    return(done);
}

/* If 4 p = A^2 + D B^2, then 4 m = (A-2)^2 + D B^2. Then g contains the
   odd part of the gcd of A-2 and B.
*/
int FindNonResPrimes(g, p_gl, Az, Bz)
BigNum g;
int *p_gl;
BigZ Az, Bz;
{
    BigNum AA, A = BmCreate(), B;
    int Al, Bl, fact, signofB;

#ifdef DEBUG
    if(debuglevel >= 1) printf("%% Activating the gcd stuff\n");
#endif
    signofB = BzGetSign(Bz); BzSetSign(Bz, 1);
    B = BzToOldBigNum(Bz, &Bl); BzSetSign(Bz, signofB);
    /* What if A = 1 ? */
    if(BzGetSign(Az) == 1){
	AA = BzToOldBigNum(Az, &Al);
	BnnAssign(A, AA, Al);
	BnnSetDigit((A+Al), 0);
	/* A <- A-2 */
	BnnSubtract(A, Al, (CONSTANTES+2), 1, 1);
    }
    else{
	BzSetSign(Az, 1);
	AA = BzToOldBigNum(Az, &Al);
	BzSetSign(Az, -1);
	BnnAssign(A, AA, Al);
	BnnSetDigit((A+Al), 0);
	/* A <- A+2 */
	BnnAdd(A, Al+1, (CONSTANTES+2), 1, 0);
    }
    Al = BnnNumDigits(A, Al+1);
#ifdef DEBUG
    if(debuglevel >= 4)
	{printf("%% A-2="); BnnPrint(A, Al); printf("\n");}
#endif
    *p_gl = BnnGcdWithCopy(g, A, Al, B, Bl);
#ifdef DEBUG
    if(debuglevel >= 4)
        {printf("%% gcd(A-2, B)="); BnnPrint(g, *p_gl); printf("\n");}
#endif
    /* remove any power of 2 */
    while(!BnnIsDigitOdd(*g))
	BnnShiftRight(g, *p_gl, 1);
    *p_gl = BnnNumDigits(g, *p_gl);
    BnFree(AA); BmFree(A); BnFree(B);
}
