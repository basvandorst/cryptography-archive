/*
 *   Test program to implement Brickell et al's method for fast
 *   computation of g^x mod n, for fixed g and n, using precomputation. 
 *   This idea can be used to substantially speed up certain phases 
 *   of the Digital Signature Standard (DSS).
 *
 *   See "Fast Exponentiation with Precomputation"
 *   by E. Brickell et al. in Proceedings Eurocrypt 1992
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <big.h>   /* include MIRACL system */

miracl precision=50;

#define ALPHA 16
static char *modulus=
"295NZNjq8kIndq5Df0NDrA3qk4wxKpbXX4G5bC11A2lRKxcbnap2XDgE4X286glvmxDN66uSaeTjRMrelTY5WfLn";

void main()
{
    static Big table[200];
    static int digit[200];
    Big e,n,a,b;
    int i,d,ndig,nb,best,time,store,base;
    IOBASE=60;
    n=modulus;
    IOBASE=10;
    cout << "Enter size of exponent in bits = ";
    cin >> nb;
    best=10000;
    for (i=1,base=2;;base*=2,i++)
    { /* find best base as power of 2 */
        store=nb/i+1;
        time=store+base-3;  /* no floating point! */
        if (time<best) best=time;
        else break; 
    }
    cout << nb*3/2 << " multiplications will be needed by naive method\n";
    cout << best << " multiplications will be needed by Brickell method\n";
    cout << store << " big numbers will be precomputed and stored in table\n";
    table[0]=ALPHA;
    cout << "precomputation...... " << endl;
    for (i=1;i<store;i++) 
    { /* calculate look-up table */
        table[i]=pow(table[i-1],base,n);
    }
    e=rand(nb,2); /* random exponent */

    cout << "naive method" << endl;
    a=pow(ALPHA,e,n);
    cout << a << endl;

    cout << "Brickell et al. method" << endl;
    for (ndig=0;e>0;ndig++)
    { /* break up exponent into digits, using 'base' */
      /* (Note base is a power of 2.) This is fast.  */
        digit[ndig]=e%base;
        e/=base;
    }

    b=a=1;
    for (d=base-1;d>0;d--)
    { /* brickell's fast method */
        for (i=0;i<ndig;i++)
            if (digit[i]==d) b=modmult(b,table[i],n);
        a=modmult(a,b,n);
    }
    cout << a << endl;
}

