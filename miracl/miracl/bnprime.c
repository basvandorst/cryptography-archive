/*
 *   MIRACL prime number routines - test for and generate prime numbers
 *   bnprime.c
 */

#include <stdio.h>
#include "miracl.h"

/* access global variables */

extern int depth;    /* error tracing ..*/
extern int trace[];  /* .. mechanism    */

extern big w1;       /* workspace variables */
extern big w2;
extern big w3;
extern big w4;
extern big w5;
extern big w6;
extern big w7;


bool prime(x)
big x;
{  /*  test for primality (probably) ;TRUE if x is  *
    *  prime. test done NTRY times; chance of wrong * 
    *  identification << (1/4)^NTRY                 */
    int i,j,k,m,n,sx,nits;
    static char primes[]={2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,
                           61,67,71,73,79,83,89,97,101,103};
    static long pmult[]={223092870L,58642669L,600662303L,33984931L,89809099L};
    if (ERNUM) return TRUE;
    sx=size(x);
    if (sx<=1) return FALSE;
    if (sx<105) 
    {
        for (i=0;i<27;i++)
              if (sx==(int)primes[i]) return TRUE;
        return FALSE;
    }
    depth++;
    trace[depth]=22;
    if (TRACER) track();
    for (i=0;i<4;i++)
    { /* check if divisible by first few primes */
        lconv(pmult[i],w3); 
        if (gcd(w3,x,w4)!=1)
        { /* factor found... */
            depth--;
            return FALSE;
        }
    }
    nits=logb2(x)/5;        /* lot less work than a powmod */
    convert(5,w1);
    convert(2,w2);
    convert(1,w3);
    k=m=1;
    for (i=1;i<nits;i++)
    { /* try a few iterations of Pollard-Rho factoring method */
        k--;
        if (k==0)
        {
            copy(w1,w2);
            m*=2;
            k=m;
        }
        convert(1,w4);
        mad(w1,w1,w4,x,x,w1);
        subtract(w2,w1,w4);
        mad(w4,w3,w3,x,x,w3);
    }
    if (gcd(w3,x,w4)!=1)
    { /* factor found... */
        depth--;
        return FALSE;
    }
    decr(x,1,w1); /* calculate k and w3 ...    */
    k=0;
    while (subdiv(w1,2,w1)==0)
    {
        k++;
        copy(w1,w3);
    }              /* ... such that x=1+w3*2**k */
    for (n=0;n<NTRY;n++)
    { /* perform test NTRY times */
        convert((int)primes[n],w4);
        j=0;
        powmod(w4,w3,x,w4);
        decr(x,1,w2);
        while ((j>0 || size(w4)!=1) 
              && compare(w4,w2)!=0)
        {
            j++;
            if ((j>1 && size(w4)==1) || j==k)
            { /* definitely not prime */
                depth--;
                return FALSE;
            }
            mad(w4,w4,w4,x,x,w4);
        }
    }
    depth--;
    return TRUE;  /* probably prime */
}

void nxprime(w,x)
big w,x;
{  /*  find next highest prime from w using     * 
    *  probabilistic primality test NTRY times  */
    if (ERNUM) return;
    depth++;
    trace[depth]=21;
    if (TRACER) track();
    copy(w,x);
    if (size(x)<2) 
    {
        convert(2,x);
        depth--;
        return;
    }
    if (subdiv(x,2,w1)==0) incr(x,1,x);
    else                   incr(x,2,x);
    while (!prime(x)) incr(x,2,x);
    depth--;
}

void strongp(p,n)
big p;
int n;
{ /* generate strong prime number p suitable *
   * for encryption. p will be > n decimal   *
   * digits in length. See Gordon J. 'Strong *
   * primes are easy to find', Advances in   *
   * Cryptology, Proc Eurocrypt 84, Lecture  *
   * notes in Computer Science, Vol. 209     */
    int k,n1,n2;
    if (ERNUM) return;
    depth++;
    trace[depth]=47;
    if (TRACER) track();
    n2=n/3;
    n1=2*n2+n%3;
    bigdig(w6,n1,10);
    nxprime(w6,w6);
    bigdig(w5,n2,10);
    nxprime(w5,w5);  /* find w5 and w6 prime */
    k=0;
    do
    { /* find prime p = k.w6 + 1  with k even */
        k+=2;
        premult(w6,k,p);
        incr(p,1,p);
    } while (!ERNUM && !prime(p));
    multiply(p,w5,w7);
    decr(p,1,p);
    powmod(w5,p,w7,w6);
    incr(p,1,p);
    decr(w5,1,w5);
    powmod(p,w5,w7,w5);
    subtract(w6,w5,w6);     /* w6 = (w5^(p-1) - p^(w5-1)) mod w7  */
    if (subdiv(w6,2,w5)==0) add(w6,w7,w6);
    k=0;
    do
    { /* find prime p = w6 + k.w7  with k even and p=2 mod 3 */
        k+=2;
        if (ERNUM) break;
        premult(w7,k,p);
        add(p,w6,p);
    } while (subdiv(p,3,w5)!=2 || !prime(p));
    depth--;
}

