
/* $Log:	prprime.c,v $
 * Revision 1.1  91/09/20  14:48:26  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:07:46  morain
 * Initial revision
 *  */

/**********************************************************************
                           PSEUDOPRIMALITY
**********************************************************************/

#include "mod.h"

int IsProbablePrime(n, nl)
BigNum n;
int nl;
{
    return(MillerRabin(n, nl, 2));
}

#define NBSMALLP 50

static long smallprimes[NBSMALLP] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
31, 37,41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107,
109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191,
193, 197, 199, 211, 223, 227, 229};

/* Returns 1 if n is a strong pseudoprime for e bases and 0 otherwise */
/* n is odd */
int MillerRabin(n, nl, e)
BigNum n;
int nl, e;
{
    BigNumDigit a[2];
    BigMod nminus1 = BmCreate(), m = BmCreate(), k = BmCreate();
    long random();
    int ml = 0, kl = nl, q = 0, i, prp = 1, ind, pused[NBSMALLP];

    /* computing n-1 */
    BnnAssign(nminus1, n, nl);
    BnnSubtractBorrow(nminus1, nl, 0);
    BnnAssign(k, nminus1, nl);
    /* take out powers of two */
    while(!BnnIsDigitOdd(*k)){
	q++;
	BnnShiftRight(k, kl, 1);
    }
    kl = BnnNumDigits(k, kl);
    for(i = 0; i < NBSMALLP; i++) pused[i] = 0;
    for( ; e > 0; e--){
	/* choose a */
	do {
	    ind = ((int)random()) % NBSMALLP;
	} while(pused[ind]);
	pused[ind] = 1;
	a[0] = (BigNumDigit)smallprimes[ind];
#ifdef DEBUG
	if(debuglevel >= 10) printf("random=%d\n", a[0]);
#endif
	if(BnnCompare(Modu, Modul, a, 1) != 1){
	    /* a <- a mod Modu */
	    a[1] = 0;
	    a[0] = BnnDivideDigit(m, a, 2, *Modu);
	}
	if(BnnIsZero(a, 1))
	    a[0] = 2;
	BmExp(m, &ml, a, 1, k, kl);
#ifdef DEBUG
	if(debuglevel >= 10) printf("m=%s\n", From(m, ml));
#endif
	if((!BnnIsOne(m, ml)) && BnnCompare(nminus1, nl, m, ml)){
	    for(i = 1; i < q; i++){
		BmSquare(m, &ml, m, ml);
#ifdef DEBUG
		if(debuglevel >= 10) 
		    printf("m**(2**%d)=%s\n", i, From(m, ml));
#endif
		if(!BnnCompare(nminus1, nl, m, ml))
		    break;
	    }
	    if(i == q){prp = 0;	break;}
	}
    }
    BmFree(nminus1); BmFree(m); BmFree(k);
    return(prp);
}

/* returns 0 if some small factor is found, 1 otherwise and *p_why contains
   some small factor if any. */
int IsProbablePrimeWithSieve(n, nl, difnp_file, pmax, p_why)
char difnp_file[];
BigNum n;
long pmax, *p_why;
int nl;
{
    long done;

    *p_why = 0;
    if(!(done = SievePrime(n, nl, difnp_file, pmax))){
	if((nl == 1) && (BnnDoesDigitFitInWord(*n)) && (BnnGetDigit(n) <= 3))
	    /* n <= 3 */
	    done = 1;
	else{
	    BmInit(n);
	    *p_why = 1;
	    done = MillerRabin(n, nl, 2);
	}
    }
    else{
	*p_why = done;
	done = 0;
    }
    return(done);
}

/* Stupid test of primality for small numbers: returns 0 if n has no small 
   prime factor from difnp_file and this factor otherwise. */
long SievePrime(n, nl, difnp_file, pmax)
BigNum n;
int nl;
char *difnp_file;
long pmax;
{
    FILE *fopen(), *ifile;
    BigNum m, p, q, sq;
    long done = 0;
    int dp, sql;

    if(BnnIsOne(n, nl)) return(1);
    if(!BnnIsDigitOdd(*n)){
	if((nl == 1) && (BnnDoesDigitFitInWord(*n)) && (BnnGetDigit(n) == 2))
	    /* n == 2 */
	    return(0);
	else
	    /* n is just even */
	    return(2);
    }
    m = BNC(nl+1); p = BNC(1); q = BNC(nl+1); sq = BNC(nl);
    if(nl == 1)
	sql = BnnSqrt(sq, n, nl);
    else{
	BnnSetDigit(sq, (BigNumDigit)pmax);
	sql = 1;
    }
    BnnSetDigit(p, 1);
    ifile = fopen(difnp_file, "r");
    done = 0;
    while(fscanf(ifile, "%d", &dp) != EOF){
	BnnSetDigit(q, (dp << 1));
	BnnAdd(p, 1, q, 1, 0);
	if(BnnCompare(sq, sql, p, 1) == -1)
	    /* p > sqrt(n), so n is prime */
	    break;
	else{
	    BnnAssign(m, n, nl);
	    BnnSetDigit((m+nl), 0);
	    if(!BnnDivideDigit(q, m, nl+1, *p)){
		done = *p;
		break;
	    }
	}
    }
    fclose(ifile);
    BnFree(m); BnFree(p); BnFree(q); BnFree(sq);
    return(done);
}

/* Check the primality of n < 100 */
int IsSmallPrime(n)
long n;
{
    long p, g;
    int i;

    if(n == 1) return(0);
    p = 1;
    for(i = 0; i < 5; i++) p *= smallprimes[i];
    g = LongGcdLong(n, p);
    if(g == 1) return(1);
    else{
	for(i = 0; i < 5; i++)
	    if(n == smallprimes[i]) return(1);
    }
    return(0);
}
