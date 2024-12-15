
/* $Log:	sieve.c,v $
 * Revision 1.1  91/09/20  14:55:33  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:30:33  morain
 * Initial revision
 *  */

/**********************************************************************
                       Just a crude ordinary sieve
**********************************************************************/

#include <stdio.h>
#include "ifactor.h"

/* 
   Finds all prime factors of n[0..nl][0] less than spmax using primes 
   taken from filename; n is replaced by n/product(p_i^e_i). All factors 
   are put in *p_lfact. Returns 1 if factorization is completed and 0
   otherwise. spmax is a 1-Digit number. *p_nbfact contains the number of
   factors found.
*/
int sieve(n, p_nl, difnp_file, p_lfact, spmax, p_nbfact)
BigNum n, spmax;
int *p_nl, *p_nbfact;
char *difnp_file;
ListOfFactors *p_lfact;
{
    FILE *fopen(), *ifile;
    BigNum m = BNC(*p_nl+1), p = BNC(1), q = BNC(1);
    ListOfFactors lf;
    int e = 0, nbf = 0, dp, tmpl, done = 0;

#ifdef DEBUG
    if(debuglevel >= 4) printf("%% Entering sieve\n");
#endif
    /* Taking out powers of 2 */
    while(!BnnIsDigitOdd(*n)){
	e++; BnnShiftRight(n, *p_nl, 1);
    }
    if(e){
	BnnSetDigit(p, 2); AppendFactor(p_lfact, p, 1, e);
	*p_nl = BnnNumDigits(n, *p_nl); nbf++;
    }
    BnnSetDigit(p, 1);
    ifile = fopen(difnp_file, "r");
    while(fscanf(ifile, "%d", &dp) != EOF){
	BnnSetDigit(q, (dp << 1));
	BnnAdd(p, 1, q, 1, 0);
	if(BnnCompare(p, 1, spmax, 1) == 1) break;
	e = 0;
	while(1){
	    BnnSetDigit((n+*p_nl), 0);
	    BnnDivideDigit(m, q, n, *p_nl+1, *p);
	    if(BnnIsDigitZero(*q)){
		e++;
		BnnAssign(n, m, *p_nl);
		tmpl = BnnNumDigits(n, *p_nl);
		BnnSetToZero((n+tmpl), *p_nl-tmpl);
		*p_nl = tmpl;
	    }
	    else{
		if(e){
		    nbf++;
#ifdef DEBUG
		    if(debuglevel >= 4){
			printf("%% Sieve -> "); BnnPrint(p, 1);
			printf("^%d\n", e);
		    }
#endif
		    AppendFactor(p_lfact, p, 1, e);
		}
		break;
	    }
	}
    }
    fclose(ifile);
    if(BnnIsOne(n, *p_nl))
	done = 1;
    else{
	if(*p_nl <= 2){
	    BnnSetToZero(m, 2);
	    BnnMultiply(m, 2, p, 1, p, 1);
	    if(BnnCompare(n, *p_nl, m, 2) == -1){
		/* n < pmax^2, hence n is prime */
		nbf++;
		AppendFactor(p_lfact, n, *p_nl, 1);
		BnnSetToZero(n, *p_nl);
		BnnSetDigit(n, 1);
		*p_nl = 1;
		done = 1;
	    }
	}
    }
    BnFree(m); BnFree(p); BnFree(q);
    *p_nbfact = nbf;
    return(done);
}
