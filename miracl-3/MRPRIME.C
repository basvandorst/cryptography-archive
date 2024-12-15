/*
 *   MIRACL prime number routines - test for and generate prime numbers
 *   mrprime.c
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <miracl.h>

/* access global variables */

extern int mr_depth;    /* error tracing ..*/
extern int mr_trace[];  /* .. mechanism    */

extern big mr_w1;       /* workspace variables */
extern big mr_w2;
extern big mr_w3;
extern big mr_w4;

void gprime(maxp)
int maxp;
{ /* generate all primes less than maxp into global PRIMES */
    char *sv;
    int pix,i,k,prime;
    if (MR_ERNUM) return;
    if (maxp<=0)
    {
        if (PRIMES!=NULL) free(PRIMES);
        PRIMES=NULL;
        return;
    }
    mr_depth++;
    mr_trace[mr_depth]=70;
    if (TRACER) track();
    if (maxp>=TOOBIG)
    {
         berror(8);
         mr_depth--;
         return;
    }
    if (maxp<1000) maxp=1000;
    maxp=(maxp+1)/2;
    sv=(char *)calloc(maxp,1);
    if (sv==NULL)
    {
        berror(8);
        mr_depth--;
        return;
    }
    pix=1;
    for (i=0;i<maxp;i++)
        sv[i]=TRUE;
    for (i=0;i<maxp;i++)
    if (sv[i])
    { /* using sieve of Eratosthenes */
        prime=i+i+3;
        for (k=i+prime;k<maxp;k+=prime)
            sv[k]=FALSE;
        pix++;
    }
    if (PRIMES!=NULL) free(PRIMES);
    PRIMES=(int *)calloc(pix+2,sizeof(int));
    if (PRIMES==NULL)
    {
        berror(8);
        mr_depth--;
        return;
    }
    PRIMES[0]=2;
    pix=1;
    for (i=0;i<maxp;i++)
        if (sv[i]) PRIMES[pix++]=i+i+3;
    PRIMES[pix]=0;
    free(sv);
    mr_depth--;
    return;
}

bool isprime(x)
big x;
{  /*  test for primality (probably); TRUE if x is prime. test done MR_NTRY *
    *  times; chance of wrong identification << (1/4)^MR_NTRY. Note however *
    *  that this is an extreme upper bound. For example for a 100 digit  *
    *  "prime" the chances of false witness are actually < (.00000003)^MR_NTRY *
    *  See Kim & Pomerance "The probability that a random probable prime *
    *  is Composite", Math. Comp. October 1989 pp.721-741                *
    *  The value of MR_NTRY is now adjusted internally to account for this. */
    int i,j,k,n,times,d;
    if (MR_ERNUM) return TRUE;
    if (size(x)<=1) return FALSE;
    mr_depth++;
    mr_trace[mr_depth]=22;
    if (TRACER) track();
    if (PRIMES==NULL) gprime(1000);
    for (i=0;PRIMES[i]!=0;i++)
    { /* test for divisible by first few primes */
        if (subdiv(x,PRIMES[i],mr_w1)==0)
        { 
            mr_depth--;
            if (size(mr_w1)==1) return TRUE;
            else return FALSE;
        }
        if (size(mr_w1)<=PRIMES[i])
        { /* must be prime */
            mr_depth--;
            return TRUE;
        }
    }
            
    decr(x,1,mr_w1); /* calculate k and mr_w3 ...    */
    k=0;
    while (subdiv(mr_w1,2,mr_w1)==0)
    {
        k++;
        copy(mr_w1,mr_w3);
    }              /* ... such that x=1+mr_w3*2**k */
    times=MR_NTRY;
    if (times>100) times=100;
    d=logb2(x);    /* for larger primes, reduce MR_NTRYs */
    if (d>220) times=1+((10*times)/(d-210));
    for (n=1;n<=times;n++)
    { /* perform test times times */
        j=0;
        powltr(PRIMES[n],mr_w3,x,mr_w4);
        decr(x,1,mr_w2);
        while ((j>0 || size(mr_w4)!=1) 
              && compare(mr_w4,mr_w2)!=0)
        {
            j++;
            if ((j>1 && size(mr_w4)==1) || j==k)
            { /* definitely not prime */
                mr_depth--;
                return FALSE;
            }
            mad(mr_w4,mr_w4,mr_w4,x,x,mr_w4);
        }
    }
    mr_depth--;
    return TRUE;  /* probably prime */
}

void nxprime(w,x)
big w,x;
{  /*  find next highest prime from w using     * 
    *  probabilistic primality test             */
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=21;
    if (TRACER) track();
    copy(w,x);
    if (size(x)<2) 
    {
        convert(2,x);
        mr_depth--;
        return;
    }
    if (subdiv(x,2,mr_w1)==0) incr(x,1,x);
    else                   incr(x,2,x);
    while (!isprime(x)) incr(x,2,x);
    mr_depth--;
}

