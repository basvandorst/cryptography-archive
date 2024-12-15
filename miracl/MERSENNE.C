/*
 *   Program to calculate mersenne primes
 *   using Lucas-Lehmer test - Knuth p.391
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <miracl.h>
#define LIMIT 10000 

main()
{ /* calculate mersenne primes */
    big L,m,T;
    int i,k,q;
    mirsys(120,0);
    L=mirvar(0);
    m=mirvar(0);
    T=mirvar(0);
    gprime(LIMIT);
    for (k=1;;k++)
    { /* test only prime exponents */
        q=PRIMES[k];
        if (q==0) break;
        expb2(q,m);
        decr(m,1,m);
        convert(4,L);
        for(i=1;i<=q-2;i++)
        { /* Lucas-Lehmer test */
            fastmult(L,L,L);
            decr(L,2,L);
            sftbit(L,-q,T);
            add(L,T,L);
            sftbit(T,q,T);
            subtract(L,T,L);
            if (compare(L,m)>=0) subtract(L,m,L);
        }
        if (size(L)==0)
        { /* mersenne prime found! */
            printf("2^%d-1 is prime = \n",q);
            cotnum(m,stdout);
        }
    }
}

