
/* $Log:	fastsieve.c,v $
 * Revision 1.1  91/09/20  14:53:54  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:28:31  morain
 * Initial revision
 *  */

/**********************************************************************

                 Fast sieving for primality proving purposes

**********************************************************************/

#include <stdio.h>
#include "ifactor.h"

/* Computes (n0+1) mod p for all p <= spmax */
int presieve(n0, nl, difnp_file, spmax, resp, ipmax)
BigNum n0, spmax;
BigNumDigit resp[];
int nl, ipmax;
char *difnp_file;
{
    FILE *fopen(), *ifile;
    BigNum m = BNC(nl+1), n = BNC(nl+1);
    BigNumDigit p[1], q[1];
    int ip = 0, dp;

#ifdef DEBUG
    if(debuglevel >= 4) printf("%% Entering presieve\n");
#endif
    /* n <- n0+1 */
    BnnAssign(n, n0, nl);
    BnnSetDigit((n+nl), 0);
    BnnAddCarry(n, nl, 1);
    BnnSetDigit(p, 1);
    ifile = fopen(difnp_file, "r");
    while(fscanf(ifile, "%d", &dp) != EOF){
	BnnSetDigit(q, (dp << 1));
	BnnAdd(p, 1, q, 1, 0);
	if(BnnCompare(p, 1, spmax, 1) == 1) break;
	resp[ip++] = BnnDivideDigit(m, n, nl+1, p[0]);
	if(ip == ipmax){
	    printf("%%!%% Too many primes\n");
	    exit(0);
	}
    }
    fclose(ifile);
    BnFree(m); BnFree(n);
}

/* 
   p_lfactp contains the factors of n+1
   p_lfactm contains the factors of n-1
   resp[] contains the values of (n+1) mod p
*/
int sievepm(n, nl, difnp_file, p_lfactp, p_lfactm, spmax, resp)
BigNum n, spmax;
BigNumDigit resp[];
int nl;
char *difnp_file;
ListOfFactors *p_lfactp, *p_lfactm;
{
    FILE *fopen(), *ifile;
    BigNumDigit p[1], q[1];
    ListOfFactors lf;
    int dp, ip = -1;

#ifdef DEBUG
    if(debuglevel >= 4) printf("%% Entering sievepm\n");
#endif
    BnnSetDigit(p, 2); 
    AppendFactor(p_lfactp, p, 1, 0);
    AppendFactor(p_lfactm, p, 1, 0);
    BnnSetDigit(p, 1);
    ifile = fopen(difnp_file, "r");
    while(fscanf(ifile, "%d", &dp) != EOF){
	ip++;
	BnnSetDigit(q, (dp << 1));
	BnnAdd(p, 1, q, 1, 0);
	if(BnnCompare(p, 1, spmax, 1) == 1) break;
	if(resp[ip] == 2){
	    /* N+1 mod p = 2 => p divides N-1 */
#ifdef DEBUG
	    if(debuglevel >= 4){
		printf("%% Sieve- -> "); BnnPrint(p, 1);
		printf("^%d\n", 0);
	    }
#endif
	    AppendFactor(p_lfactm, p, 1, 0);
	}
	if(!resp[ip]){
	    /* N+1 mod p = 0 => p divides N+1 */
#ifdef DEBUG
	    if(debuglevel >= 4){
		printf("%% Sieve+ -> "); BnnPrint(p, 1);
		printf("^%d\n", 0);
	    }
#endif
	    AppendFactor(p_lfactp, p, 1, 0);
	}
    }
    fclose(ifile);
}

/* Returns 1 if factorisation finished (i.e., cofactor is 1) */
int DivideAndCheck(n, p_nl, p_lfact)
BigNum n;
int *p_nl;
ListOfFactors *p_lfact;
{
    BigNum m = BNC(*p_nl+1);
    BigNumDigit p[1];
    int done = 0;

    DivideMaxByList(n, p_nl, p_lfact);
    if(BnnIsOne(n, *p_nl))
	done = 1;
    else{
	if(*p_nl <= 2){
	    BnnSetToZero(m, 2);
	    p[0] = 0; /* tmp ?!? */
	    BnnMultiply(m, 2, p, 1, p, 1);
	    if(BnnCompare(n, *p_nl, m, 2) == -1){
		/* n < pmax^2, hence n is prime */
		AppendFactor(p_lfact, n, *p_nl, 1);
		BnnSetToZero(n, *p_nl);
		BnnSetDigit(n, 1);
		*p_nl = 1;
		done = 1;
	    }
	}
    }
    BnFree(m);
    return(done);
}

/* Divides n by the factors contained in *p_lfact */
int DivideMaxByList(n, p_nl, p_lfact)
BigNum n;
int *p_nl;
ListOfFactors *p_lfact;
{
    BigNum m = BNC(*p_nl+1);
    ListOfFactors foo = *p_lfact;
    int ml, pl, e;

    while(foo != NULL){
	e = DivideMax(n, p_nl, m, foo->p, foo->pl);
	if(!e){
	    printf("%%!%% Error in DivideMax?\n");
	    printf("%%!%% n="); BnnPrint(n, *p_nl); printf("\n");
	    printf("%%!%% p="); BnnPrint(foo->p, foo->pl); printf("\n");
/*	    exit(0);*/
	}
	else{
	    foo->e = e;
	}
	foo = foo->cdr;
    }
    BnFree(m);
}

int DivideMax(n, p_nl, m, f, fl)
BigNum n, m, f;
int *p_nl, fl;
{
    int e = 0, ml;

#ifdef DEBUG
    if(debuglevel >= 6)
	{printf("%% DM: p="); BnnPrint(f, fl); printf("\n");}
#endif
    while(1){
	ml = *p_nl;
	BnnAssign(m, n, ml);
	BnnSetDigit((m+ml), 0);
	BnnDivide(m, ml+1, f, fl);
	/* r = m[0..fl-1], q = m[fl..ml-fl+1] */
	if(!BnnIsZero(m, fl))
	    break;
	else{
	    e++;
	    BnnSetToZero(n, *p_nl);
	    BnnAssign(n, (m+fl), ml-fl+1);
	    *p_nl = BnnNumDigits(n, ml-fl+1);
	}
    }
    return(e);
}

/* Takes a list of factors and put it in the array tfact */
int ListToTab(lfact, tfact)
ListOfFactors lfact;
LBig tfact[];
{
    ListOfFactors foo = lfact;
    int i = 0;

    while(foo != NULL){
	tfact[i].n = BNC(foo->pl);
	BnnAssign(tfact[i].n, foo->p, foo->pl);
	tfact[i++].nl = foo->pl;
	foo = foo->cdr;
    }
    return(i);
}
