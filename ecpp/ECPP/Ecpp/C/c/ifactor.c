
/* $Log:	ifactor.c,v $
 * Revision 1.1  91/09/20  14:54:23  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:29:02  morain
 * Initial revision
 *  */

/**********************************************************************
              Main file for the factoring process
**********************************************************************/

#include "ifactor.h"
#include "exp.h"

/* Attempts to factor n[0..nl-1][0]
   Returns 
   0: if factorization not completed
   1: if factorization is complete (Modu=1)
   2: if factors found and cofactor is a probable prime
   3: if the original number is a pseudoprime (no factor found)
   Cofactor is put back in n0, being prprime or composite
   Uses first a sieve, then rho if userho != 0, then p-1 if usepollard != 0.
*/
int ifactor(n0, p_n0l, p_lfact, difnp_file, spmax, userho, usepollard)
BigNum n0, spmax;
int *p_n0l;
ListOfFactors *p_lfact;
char *difnp_file;
int userho, usepollard;
{
    BigNum n = BNC((*p_n0l)+1);
    long b1, b2, itmax;
    int done = 0, nbgcd, nl, nbf;

    BnnAssign(n, n0, *p_n0l);
    nl = *p_n0l;
    done = sieve(n, &nl, difnp_file, p_lfact, spmax, &nbf);
    if(done == 1){
	printf("%% Factorization completed using sieve only\n");
	BnnSetToZero(n0, *p_n0l);
	BnnAssign(n0, n, nl);
	*p_n0l = nl;
    }
    else{
	BmInit(n);
	if(IsProbablePrime(Modu, Modul)){
	    printf("%% Cofactor after sieve is a probable prime\n");
	    if(nbf) return(2); else return(3);
	}
	else{
	    if(userho){
/*		itmax = 1000; nbgcd = 10;*/
		itmax = 3800; nbgcd = 10;
		done = rho(p_lfact, itmax, nbgcd);
		if(done)
		    printf("%% Factorization completed using Rho\n");
	    }
	    if(!done && usepollard){
		/* the number we try to factor is composite... */
		/*		    b1 = 2000; b2 = 100000; nbgcd = 10;*/
		b1 = 100000; b2 = 1000000; nbgcd = 100;
		done = pollard(p_lfact, b1, b2, nbgcd, difnp_file);
		if(done)
		    printf("%% Factorization completed using p-1\n");
	    }
	    BnnSetToZero(n0, *p_n0l);
	    BnnAssign(n0, Modu, Modul);
	    *p_n0l = Modul;
	}
    }
    BnFree(n);
    return(done);
}

/* Does some useful things when a factor (g, gl) of (Modu, Modul) is found:
   1. checks that g divides Modu;
   2. computes e s.t. g^e || Modu and append (g, e) to *p_lfact;
   3. puts back in Modu the least of g^e and Modu;
   4. uses Zbuf as buffer;
   5. returns 1 if the cofactor in Modu is 1,
              2 if the cofactor in Modu is a probable prime,
	      0 otherwise
*/
int FactorCheck(g, gl, p_lfact)
BigMod g;
int gl;
ListOfFactors *p_lfact;
{
    int e = 0, zl = Modul+1, ml;

    /*  g < Modu */
    BnnAssign(Zbuf, Modu, Modul);
    BnnSetDigit((Zbuf+Modul), 0);
    BnnDivide(Zbuf, zl, g, gl);
    /* the remainder is in Zt[0..gl-1] */
    if(!BnnIsZero(Zbuf, gl)){
	printf("Error in FactorCheck: not a divisor\n");
	exit(1);
    }
    zl = BnnNumDigits((Zbuf+gl), zl-gl);
    if(BnnCompare((Zbuf+gl), zl, g, gl) == -1){
	/* Modu / g < g, on e'change Modu/g et g */
	BnnAssign(Modu, (Zbuf+gl), zl);
	BnnAssign((Zbuf+zl), g, gl);
	BnnAssign(g, Modu, zl);
	ml = gl; gl = zl; zl = ml;
    }
    /* at this point, we have Modu >= g */
    printf("%% Factor="); BnnPrint(g, gl); printf("\n");
    while(BnnIsZero(Zbuf, gl)){
	/* Modu = Zt[gl..gl+ml-1] */
	e++; ml = zl;
	BnnAssign(Modu, (Zbuf+gl), ml);
	BnnAssign(Zbuf, Modu, ml);
	BnnSetDigit((Zbuf+ml), 0);
	BnnDivide(Zbuf, ml+1, g, gl);
	zl = BnnNumDigits((Zbuf+gl), ml-gl+1);
    }
    /* quotient is in [gl..gl+ml-1] */
    if(e > 1) printf("%% Multiple factor found -> %d\n", e);
    AppendFactor(p_lfact, g, gl, e);
    /* pbs si g compose', e > 1 et M/g^e < g ??? */
    if(debuglevel >= 9)
	{printf("New Modu="); BnnPrint(Modu, Modul); printf("\n");}
    if(BnnIsOne(Modu, Modul)) return(1);
    BmInit(BnConvert(Modu, 0, ml));
    if(IsProbablePrime(Modu, Modul)) return(2);
    else return(0);
}

/* returns 1 if g=gcd(a, n) is 1 or n and 0 otherwise */
int IsGcdTrivial(g, p_gl, a, al, n, nl)
BigNum g, a, n;
int *p_gl, al, nl;
{
    *p_gl = BnnGcdWithCopy(g, a, al, n, nl);
#ifdef DEBUG 
    if(debuglevel >= 7) 
	{printf("gcd is "); BnnPrint(g, *p_gl); printf("\n");}
#endif
    return(BnnIsOne(g,*p_gl) || !(BnnCompare(g, *p_gl, n, nl)));
}

