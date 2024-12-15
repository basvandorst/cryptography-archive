/*
 *   Program to calculate mersenne primes
 *   using Lucas-Lehmer test - Knuth p.391
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <big.h>

#define LIMIT 1000

miracl precision=100; 

void main()
{ /* calculate mersenne primes */
    Big L,m,T;
    int i,k,q;
    gprime(LIMIT);
    for (k=1;;k++)
    { /* test only prime exponents */
        q=PRIMES[k];
        if (q==0) break;
        m=pow((Big)2,q)-1;
        L=4;
        for(i=1;i<=q-2;i++)
        { /* Lucas-Lehmer test */
            L=L*L-2;
            T=(L>>q);       /* find remainder mod m */
            L=L-(T<<q)+T;
            if (L>=m) L-=m;
        }
        if (L==0)
        { /* mersenne prime found! */
            cout << "\n2^" << q << "-1 is prime\n";
            cout << m << flush;
        }
    }
}

