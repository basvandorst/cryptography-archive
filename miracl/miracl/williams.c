/*
 *   Program to factor big numbers using Williams (p+1) method.
 *   Works when for some prime divisor p of n, p+1 has only
 *   small factors.
 *   See "Speeding the Pollard and Elliptic Curve Methods"
 *   by Peter Montgomery, Math. Comp. Vol. 48. Jan. 1987 pp243-264
 */

#include <stdio.h>
#include "miracl.h"

#define LIMIT1 4000       /* must be int, and > MULT/2 */
#define LIMIT2 200000L    /* may be long */
#define MULT   2310       /* must be int, product of small primes 2.3.. */
#define NTRYS  3          /* number of attempts */
big t,t2;

void lucas(p,r,n,vp,v)
big p,v,n,vp;
int r;
{ /* calculate r-th elements of lucas sequence mod n */
    int k,rr;
    convert(2,vp);
    copy(p,v);
    subtract(n,p,p);
    k=1;
    rr=r;
    while ((rr/=2)>1) k*=2;
    while (k>0)
    { /* use binary method */
        mad(v,vp,p,n,n,vp);
        mad(v,v,t2,n,n,v);
        if ((r&k)>0)
        {
            mad(p,v,vp,n,n,t);
            copy(v,vp);
            negate(t,v);
        }
        k/=2;
    }
    subtract(n,p,p);
}

main()
{  /*  factoring program using Williams (p+1) method */
    int k,phase,m,nt,iv,pos,btch;
    int *primes;
    long i,p,pa;
    big b,q,n,fp,fvw,fd,fn;
    static big fu[1+MULT/2];
    static bool cp[1+MULT/2];
    mirsys(50,MAXBASE);
    b=mirvar(0);
    t2=mirvar((-2));
    q=mirvar(0);
    n=mirvar(0);
    t=mirvar(0);
    fp=mirvar(0);
    fvw=mirvar(0);
    fd=mirvar(0);
    fn=mirvar(0);
    primes=gprime(LIMIT1);
    for (m=1;m<=MULT/2;m+=2)
        if (igcd(MULT,m)==1)
        {
            fu[m]=mirvar(0);
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
    for (nt=0,k=3;k<10;k++)
    { /* try more than once for p+1 condition (may be p-1) */
        convert(k,b);              /* try b=3,4,5..        */
        convert((k*k-4),t);
        if (gcd(t,n,t)!=1) continue; /* check (b*b-4,n)!=0 */
        nt++;
        phase=1;
        p=0;
        btch=50;
        i=0;
        printf("phase 1 - trying all primes less than %d\n",LIMIT1);
        printf("prime= %8ld",p);
        forever
        { /* main loop */
            if (phase==1)
            { /* looking for all factors of p+1 < LIMIT1 */
                p=primes[i];
                if (primes[i+1]==0)
                { /* now change gear */
                    phase=2;
                    printf("\nphase 2 - trying last prime less than %ld\n"
                           ,LIMIT2);
                    printf("prime= %8ld",p);
                    copy(b,fu[1]);
                    copy(b,fp);
                    mad(b,b,t2,n,n,fd);
                    negate(b,t);
                    mad(fd,b,t,n,n,fn);
                    for (m=5;m<=MULT/2;m+=2)
                    { /* store fu[m] = Vm(b) */
                        negate(fp,t);
                        mad(fn,fd,t,n,n,t);
                        copy(fn,fp);
                        copy(t,fn);
                        if (!cp[m]) continue;
                        copy(t,fu[m]);
                    }
                    lucas(b,MULT,n,fp,fd);
                    iv=p/MULT;
                    if (p%MULT>MULT/2) iv++,p=2*(long)iv*MULT-p;
                    lucas(fd,iv,n,fp,fvw);
                    negate(fp,fp);
                    subtract(fvw,fu[p%MULT],q);
                    btch*=10;
                    i++;
                    continue;
                }
                pa=p;
                while ((LIMIT1/p) > pa) pa*=p;
                lucas(b,(int)pa,n,fp,q);
                copy(q,b);
                decr(q,2,q);
            }
            else
            { /* phase 2 - looking for last large prime factor of (p+1) */
                p+=2;
                pos=p%MULT;
                if (pos>MULT/2)
                { /* increment giant step */
                    iv++;
                    p=(long)iv*MULT+1;
                    pos=1;
                    copy(fvw,t);
                    mad(fvw,fd,fp,n,n,fvw);
                    negate(t,fp);
                }
                if (!cp[pos]) continue;
                subtract(fvw,fu[pos],t);
                mad(q,t,t,n,n,q);  /* batching gcds */
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
        if (nt>=NTRYS) break;
        printf("\ntrying again\n");
    }
    printf("\nfailed to factor\n");
}

