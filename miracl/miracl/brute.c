/*
 *   Program to factor numbers using brute force.
 */

#include <stdio.h>
#include "miracl.h"
#define LIMIT 15000

main()
{ /* find factors by brute force division */
    big x,y;
    int n,p;
    int *primes;
    mirsys(50,MAXBASE);
    x=mirvar(0);
    y=mirvar(0);
    primes=gprime(LIMIT);
    n=0;
    p=primes[0];
    printf("input number to be factored\n");
    cinnum(x,stdin);
    if (prime(x))
    {
        printf("this number is prime!\n");
        exit(0);
    }
    printf("factors are \n");
    while (size(x)>=p)
    { /* try division by each prime in turn */
        if (p==0)
        { /* run out of primes */
            printf("composite factor ");
            cotnum(x,stdout);
            exit(0);
        }
        if (subdiv(x,p,y)==0)
        { /* factor found */
            copy(y,x);
            printf("prime factor     ");
            printf("%d\n",p);
            if (prime(x)) break;
        }
        else p=primes[++n];
    }
    printf("prime factor     ");
    cotnum(x,stdout);
}

