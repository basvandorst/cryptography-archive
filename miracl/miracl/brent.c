/*
 *   Program to factor big numbers using Brent-Pollard method.
 *   See "An Improved Monte Carlo Factorization Algorithm"
 *   by Richard Brent in BIT Vol. 20 1980 pp 176-184
 */

#include <stdio.h>
#include "miracl.h"

#define min(a,b) ((a) < (b)? (a) : (b))

main()
{  /*  factoring program using Brents method */
    long k,r,i,m,iter;
    big x,y,z,n,q,ys,c3;
    mirsys(50,MAXBASE);
    x=mirvar(0);
    y=mirvar(0);
    ys=mirvar(0);
    z=mirvar(0);
    n=mirvar(0);
    q=mirvar(0);
    c3=mirvar(3);
    printf("input number to be factored\n");
    cinnum(n,stdin);
    if (prime(n))
    {
        printf("this number is prime!\n");
        exit(0);
    }
    m=10;
    r=1;
    iter=0;
    do
    {
        printf("iterations=%5ld",iter);
        convert(1,q);
        do
        {
            copy(y,x);
            for (i=1;i<=r;i++)
                mad(y,y,c3,n,n,y);
            k=0;
            do
            {
                iter++;
                if (iter%10==0) printf("\b\b\b\b\b%5ld",iter);
                copy(y,ys);
                for (i=1;i<=min(m,r-k);i++)
                {
                    mad(y,y,c3,n,n,y);
                    subtract(y,x,z);
                    mad(z,q,q,n,n,q);
                }
                gcd(q,n,z);
                k+=m;
            } while (k<r && size(z)==1);
            r*=2;
        } while (size(z)==1);
        if (compare(z,n)==0) do 
        { /* back-track */
            mad(ys,ys,c3,n,n,ys);
            subtract(ys,x,z);
        } while (gcd(z,n,z)==1);
        if (!prime(z))
             printf("\ncomposite factor ");
        else printf("\nprime factor     ");
        cotnum(z,stdout);
        if (compare(z,n)==0) exit(0);
        divide(n,z,n);
        divide(y,n,n);
    } while (!prime(n));      
    printf("prime factor     ");
    cotnum(n,stdout);
}

