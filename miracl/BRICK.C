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

#include <stdio.h>
#include <miracl.h>

#define ALPHA 16
static char *modulus=
"295NZNjq8kIndq5Df0NDrA3qk4wxKpbXX4G5bC11A2lRKxcbnap2XDgE4X286glvmxDN66uSaeTjRMrelTY5WfLn";

main()
{
    static big table[200];
    static int digit[200];
    big e,n,a,b;
    int i,d,ndig,nb,best,time,store,base;
    mirsys(50,0);
    n=mirvar(0);
    e=mirvar(0);
    a=mirvar(0);
    b=mirvar(0);
    IOBASE=60;
    cinstr(n,modulus);
    IOBASE=10;
    printf("Enter size of exponent in bits = ");
    scanf("%d",&nb);
    getchar();
    best=10000;
    for (i=1,base=2;;base*=2,i++)
    { /* find best base as power of 2 */
        store=nb/i+1;
        time=store+base-3;  /* no floating point! */
        if (time<best) best=time;
        else break; 
    }
    printf("%d multiplications will be needed by naive method\n",nb*3/2);
    printf("%d multiplications will be needed by Brickell method\n",best);
    printf("%d big numbers will be precomputed and stored in table\n",store);

    table[0]=mirvar(ALPHA);
    printf("precomputation...... \n");
    for (i=1;i<store;i++) 
    { /* calculate look-up table */
        table[i]=mirvar(0);
        power(table[i-1],base,n,table[i]);
    }
    bigdig(nb,2,e);  /* random exponent */  

    printf("naive method\n");
    powltr(ALPHA,e,n,a);
    cotnum(a,stdout);

    printf("Brickell et al. method\n");
    for (ndig=0;size(e)>0;ndig++)
    { /* break up exponent into digits, using 'base' */
      /* (note base is a power of 2.) This is fast.  */
        digit[ndig]=subdiv(e,base,e);
    }

    convert(1,b);
    convert(1,a);
    for (d=base-1;d>0;d--)
    { /* brickell's method */
        for (i=0;i<ndig;i++)
            if (digit[i]==d) mad(b,table[i],b,n,n,b);
        mad(a,b,a,n,n,a);
    }
    cotnum(a,stdout);
}

