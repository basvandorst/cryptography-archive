/*
 *   Program to calculate mersenne primes
 *   using Lucas-Lehmer test - Knuth p.391
 */

#include <stdio.h>
#include "miracl.h"
#define LIMIT 128 

main()
{ /* calculate mersenne primes */
    big L,m;
    int i,k,q;
    int *primes;
    mirsys(100,MAXBASE);
    L=mirvar(0);
    m=mirvar(0);
    primes=gprime(LIMIT);
    for (k=1;;k++)
    { /* test only prime exponents */
        q=primes[k];
        if (q==0) break;
        expb2(m,q);
        decr(m,1,m);
        convert(4,L);
        for(i=1;i<=q-2;i++)
        { /* Lucas-Lehmer test */
            multiply(L,L,L);
            decr(L,2,L);
            divide(L,m,m);
        }
        if (size(L)==0)
        { /* mersenne prime found! */
            printf("2^%d-1 is prime = \n",q);
            cotnum(m,stdout);
        }
    }
}

