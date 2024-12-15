/*
 *   Program to factor big numbers using Pollards (p-1) method.
 *   Works when for some prime divisor p of n, p-1 has only
 *   small factors.
 *   See "Speeding the Pollard and Elliptic Curve Methods"
 *   by Peter Montgomery, Math. Comp. Vol. 48 Jan. 1987 pp243-264
 */

#include <stdio.h>
#include "miracl.h"

#define LIMIT1 10000        /* must be int, and > MULT/2 */
#define LIMIT2 300000L      /* may be long */
#define MULT   2310         /* must be int, product of small primes 2.3.. */

main()
{  /*  factoring program using Pollards (p-1) method */
    int phase,m,iv,pos,btch;
    int *primes;
    long i,p,pa;
    big b,q,n,t,bw,bvw,bd,bp;
    static big bu[1+MULT/2];
    static bool cp[1+MULT/2];
    mirsys(50,MAXBASE);
    b=mirvar(2);
    q=mirvar(0);
    n=mirvar(0);
    t=mirvar(0);
    bw=mirvar(0);
    bvw=mirvar(0);
    bd=mirvar(0);
    bp=mirvar(0);
    primes=gprime(LIMIT1);
    for (m=1;m<=MULT/2;m+=2)
        if (igcd(MULT,m)==1)
        {
            bu[m]=mirvar(0);
            cp[m]=TRUE;
        }
        else cp[m]=FALSE;
    printf("input number to be factored\n");
    cinnum(n,stdin);
    if (prime(n))
    {
        printf("this number is prime!\n");
        exit(0);
    }
    while (gcd(n,b,t)!=1) incr(b,1,b);
    phase=1;
    p=0;
    btch=50;
    i=0;
    printf("phase 1 - trying all primes less than %d\n",LIMIT1);
    printf("prime= %8ld",p);
    forever
    { /* main loop */
        if (phase==1)
        { /* looking for all factors of (p-1) < LIMIT1 */
            p=primes[i];
            if (primes[i+1]==0)
            { /* now change gear */
                phase=2;
                printf("\nphase 2 - trying last prime less than %ld\n",LIMIT2);
                printf("prime= %8ld",p);
                power(b,8,n,bw);
                convert(1,t);
                copy(b,bp);
                copy(b,bu[1]);
                for (m=3;m<=MULT/2;m+=2)
                { /* store bu[m] = b^(m*m) */
                    mad(bw,t,t,n,n,t);
                    mad(bp,t,t,n,n,bp);
                    if (!cp[m]) continue;
                    copy(bp,bu[m]);
                }
                power(b,MULT,n,t);
                power(t,MULT,n,t);
                mad(t,t,t,n,n,bd);     /* bd = b^(2*MULT*MULT) */
                iv=p/MULT;
                if (p%MULT>MULT/2) iv++,p=2*(long)iv*MULT-p;
                power(t,(2*iv-1),n,bw);
                power(t,iv,n,bvw);
                power(bvw,iv,n,bvw);   /* bvw = b^(MULT*MULT*iv*iv) */
                subtract(bvw,bu[p%MULT],q);
                btch*=10;
                i++;
                continue;
            }
            pa=p;
            while ((LIMIT1/p) > pa) pa*=p;
            power(b,(int)pa,n,b);      /* b = b^pa mod n  */
            decr(b,1,q);
        }
        else
        { /* phase 2 - looking for one last prime factor of (p-1) < LIMIT2 */
            p+=2;
            pos=p%MULT;
            if (pos>MULT/2)
            { /* increment giant step */
                iv++;
                p=(long)iv*MULT+1;
                pos=1;
                mad(bw,bd,bd,n,n,bw);
                mad(bvw,bw,bw,n,n,bvw);
            }
            if (!cp[pos]) continue;
            subtract(bvw,bu[pos],t);
            mad(q,t,t,n,n,q);  /* batching gcds q=q.t mod n */
        }
        if (i++%btch==0)
        { /* try for a solution */
            printf("\b\b\b\b\b\b\b\b%8ld",p);
            gcd(q,n,t);
            if (size(t)==1)
            {
                if (p>LIMIT2) break;
                else continue;
            }
            if (compare(t,n)==0)
            {
                printf("\ndegenerate case");
                break;
            }
            printf("\nfactors are\n");
            if (prime(t)) printf("prime factor     ");
            else          printf("composite factor ");
            cotnum(t,stdout);
            divide(n,t,n);
            if (prime(n)) printf("prime factor     ");
            else          printf("composite factor ");
            cotnum(n,stdout);
            exit(0);
        }
    } 
    printf("\nfailed to factor\n");
}

