
/* $Log:	ecppsieve.c,v $
 * Revision 1.1  91/09/20  14:53:46  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:28:21  morain
 * Initial revision
 *  */

/**********************************************************************

                       Special sieve for ECPP

**********************************************************************/

#include <stdio.h>
#include "ecpp.h"

/**********************************************************************
                  Sieving with precomputations
**********************************************************************/

/* 
   Finds all prime factors of n[0..nl][0] less than spmax using primes 
   taken from filename; n is replaced by n/product(p_i^e_i). All factors 
   are put in *p_lfact. Returns 1 if factorization is completed and 0
   otherwise. spmax is a 1-Digit number. *p_nbfact contains the number of
   factors found.
  
   If D > 0, we suppose A > 0 and 4 n = A^2 + D B^2, so m = n + 1 - A.
   all = 1 if all factors are to be tested and 0 otherwise.
*/
ecppsieve(n, nl, difnp_file, spmax, D, Az, p_lfm, p_lfp, all)
BigNum n, spmax;
int nl, all;
char *difnp_file[];
long D;
BigZ Az;
ListOfFactors *p_lfm, *p_lfp;
{
    FILE *fopen(), *ifile;
    BigNumDigit p[1], q[1], Amodp;
    BigNum A, m = BNC(nl+1);
    int Al;
    int dp, ip = -1;

    if(D <= 1){
	sievepm(n, nl, difnp_file, p_lfp, p_lfm, spmax, resp);
    }
    else{
	Al = BzGetSize(Az)+1; A = BNC(Al);
	BnnAssign(A, BzToBn(Az), Al-1);
#ifdef DEBUG
	if(debuglevel >= 4) printf("%% Entering ecppsieve\n");
#endif
	if(!BnnIsDigitOdd(*A)){
	    BnnSetDigit(p, 2); 
	    AppendFactor(p_lfm, p, 1, 0);
	    AppendFactor(p_lfp, p, 1, 0);
	}
	BnnSetDigit(p, 1);
	ifile = fopen(difnp_file, "r");
	while(fscanf(ifile, "%d", &dp) != EOF){
	    ip++;
	    BnnSetDigit(q, (dp << 1));
	    BnnAdd(p, 1, q, 1, 0);
	    if(BnnCompare(p, 1, spmax, 1) == 1) break;
	    if(!all && (LongJacobiLong(-D, p[0]) == -1)) continue;
	    /* computing A mod p */
	    Amodp = BnnDivideDigit(m, A, Al, p[0]);
	    if(resp[ip] == Amodp){
		/* N+1 = A mod p => p divides N+1-A */
#ifdef DEBUG
		if(debuglevel >= 4){
		    printf("%% Sieve- -> "); BnnPrint(p, 1); printf("\n");
		}
#endif
		AppendFactor(p_lfm, p, 1, 0);
	    }
	    Amodp = (Amodp+resp[ip]) % p[0];
	    if(!Amodp){
		/* N+1+A mod p = 0 => p divides N+1 */
#ifdef DEBUG
		if(debuglevel >= 4){
		    printf("%% Sieve+ -> "); BnnPrint(p, 1); printf("\n");
		}
#endif
		AppendFactor(p_lfp, p, 1, 0);
	    }
	}
	fclose(ifile);
	BnFree(A); BnFree(m);
    }
}
