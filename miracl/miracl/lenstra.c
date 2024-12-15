/*
 *   Program to factor big numbers using Lenstras elliptic curve method.
 *   Works when for some prime divisor p of n, p+1+d has only
 *   small factors, where d depends on the particular curve used.
 *   See "Speeding the Pollard and Elliptic Curve Methods"
 *   by Peter Montgomery, Math. Comp. Vol. 48 Jan. 1987 pp243-264
 */

#include <stdio.h>
#include "miracl.h"

#define LIMIT1 2000        /* must be int, and > MULT/2 */
#define LIMIT2 100000L     /* may be long */
#define MULT   2310        /* must be int, product of small primes 2.3... */
#define NCURVES 20         /* no. of curves to try */

big ak,t,s1,d1,s2,d2;

void duplication(sum,diff,n,x,z)
big sum,diff,n,x,z;
{ /* double a point on the curve P(x,z)=2.P(x1,z1) */
    mad(sum,sum,sum,n,n,t);   
    mad(diff,diff,diff,n,n,z); 
    mad(t,z,z,n,n,x);           /* x = sum^2.diff^2 */
    subtract(t,z,t);            /* t = sum^2-diff^2 */
    mad(ak,t,z,n,n,z);          /* z = ak*t +diff^2 */
    mad(z,t,t,n,n,z);           /* z = z.t          */
}

void addition(xd,zd,sm1,df1,sm2,df2,n,x,z)
big xd,zd,sm1,df1,sm2,df2,n,x,z;
{ /* add two points on the curve P(x,z)=P(x1,z1)+P(x2,z2) *
   * given their difference P(xd,zd)                      */
        mad(df2,sm1,sm1,n,n,x);
        mad(df1,sm2,sm2,n,n,z);
        add(z,x,t);
        subtract(z,x,z);
        mad(t,t,t,n,n,x);
        mad(zd,x,x,n,n,x);     /* x = zd.[df1.sm2+sm1.df2]^2 */
        mad(z,z,z,n,n,z);
        mad(xd,z,z,n,n,z);     /* z = xd.[df1.sm2-sm1.df2]^2 */
}

void ellipse(x,z,r,n,x1,z1,x2,z2)
big x,z,n,x1,z1,x2,z2;
int r;
{ /* calculate point r.P(x,y) on curve */
    int k,rr;
    k=1;
    rr=r;
    copy(x,x1);            
    copy(z,z1);
    add(x1,z1,s1);
    subtract(x1,z1,d1);
    duplication(s1,d1,n,x2,z2);  /* generate 2.P */
    while ((rr/=2)>1) k*=2;
    while (k>0)
    { /* use binary method */
        add(x1,z1,s1);         /* form sums and differences */
        subtract(x1,z1,d1);    /* x+z and x-z for P1 and P2 */
        add(x2,z2,s2);
        subtract(x2,z2,d2);
        if ((r&k)==0)
        { /* double P(x1,z1) mP to 2mP */
            addition(x,z,s1,d1,s2,d2,n,x2,z2);
            duplication(s1,d1,n,x1,z1);
        }
        else
        { /* double P(x2,z2) (m+1)P to (2m+2)P */
            addition(x,z,s1,d1,s2,d2,n,x1,z1);
            duplication(s2,d2,n,x2,z2);
        }    
        k/=2;
    }
}

main()
{  /*  factoring program using Lenstras Elliptic Curve method */
    int phase,m,k,nc,iv,pos,btch;
    int *primes;
    long i,p,pa;
    big q,x,z,a,x1,z1,x2,z2,xt,zt,n,fvw;
    static big fu[1+MULT/2];
    static bool cp[1+MULT/2];
    mirsys(50,MAXBASE);
    q=mirvar(0);
    x=mirvar(0);
    z=mirvar(0);
    a=mirvar(0);
    x1=mirvar(0);
    z1=mirvar(0);
    x2=mirvar(0);
    z2=mirvar(0);
    n=mirvar(0);
    t=mirvar(0);
    s1=mirvar(0);
    d1=mirvar(0);
    s2=mirvar(0);
    d2=mirvar(0);
    ak=mirvar(0);
    xt=mirvar(0);
    zt=mirvar(0);
    fvw=mirvar(0);
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
    for (nc=1,k=3;k<100;k++)
    { /* try a new curve */
        convert(2,x);         /* generating an elliptic curve */
        convert(1,z);
        convert((2*(k*k-1)),t);
        if (xgcd(t,n,t,t,t)!=1) continue;
        convert((4-k*k),a);
        mad(a,t,t,n,n,t);
        if (size(t)<0) add(t,n,t);
        copy(t,a);        
        if (xgcd(t,n,t,t,t)!=1) continue;
        nc++;
        add(a,t,a);
        divide(a,n,n);
        convert(4,ak);
        xgcd(ak,n,ak,ak,ak);
        incr(a,2,t);
        mad(ak,t,t,n,n,ak);    /* ak = (a+2)/4 */
        phase=1;
        p=0;
        i=0;
        btch=50;
        printf("phase 1 - trying all primes less than %d\n",LIMIT1);
        printf("prime= %8ld",p);
        forever
        { /* main loop */
            if (phase==1)
            {
                p=primes[i];
                if (primes[i+1]==0)
                { /* now change gear */
                    phase=2;
                    printf("\nphase 2 - trying last prime less than %ld\n",
                            LIMIT2);
                    printf("prime= %8ld",p);
                    copy(x,xt);
                    copy(z,zt);
                    add(x,z,s2);
                    subtract(x,z,d2);                    /*   P = (s2,d2) */
                    duplication(s2,d2,n,x,z);
                    add(x,z,s1);
                    subtract(x,z,d1);                    /* 2.P = (s1,d1) */
                    xgcd(z1,n,fu[1],fu[1],fu[1]);        /* fu[1] = x1/z1 */
                    mad(x1,fu[1],x1,n,n,fu[1]);
                    addition(x1,z1,s1,d1,s2,d2,n,x2,z2); /* 3.P = (x2,z2) */
                    for (m=5;m<=MULT/2;m+=2)
                    { /* calculate m.P = (x,y) and store fu[m] = x/y */
                        add(x2,z2,s2);
                        subtract(x2,z2,d2);
                        addition(x1,z1,s2,d2,s1,d1,n,x,z);
                        copy(x2,x1);
                        copy(z2,z1);
                        copy(x,x2);
                        copy(z,z2);
                        if (!cp[m]) continue;
                        copy(z2,fu[m]);
                        xgcd(fu[m],n,fu[m],fu[m],fu[m]);
                        mad(x2,fu[m],x2,n,n,fu[m]);
                    }
                    ellipse(xt,zt,MULT,n,x,z,x2,z2);
                    add(x,z,xt);
                    subtract(x,z,zt);              /* MULT.P = (xt,zt) */
                    iv=p/MULT;
                    if (p%MULT>MULT/2) iv++,p=2*(long)iv*MULT-p;
                    ellipse(x,z,iv,n,x1,z1,x2,z2); /* (x1,z1) = iv.MULT.P */
                    xgcd(z1,n,fvw,fvw,fvw);
                    mad(x1,fvw,x1,n,n,fvw);        /* fvw = x1/z1 */
                    subtract(fvw,fu[p%MULT],q);
                    btch*=10;
                    i++;
                    continue;
                }
                pa=p;
                while ((LIMIT1/p) > pa) pa*=p;
                convert((int)pa,t);
                ellipse(x,z,(int)pa,n,x1,z1,x2,z2);
                copy(x1,x);
                copy(z1,z);
                copy(z,q);
            }
            else
            { /* phase 2 - looking for last large prime factor of (p+1+d) */
                p+=2;
                pos=p%MULT;
                if (pos>MULT/2)
                { /* increment giant step */
                    iv++;
                    p=(long)iv*MULT+1;
                    pos=1;
                    xgcd(z2,n,fvw,fvw,fvw);
                    mad(x2,fvw,x2,n,n,fvw);
                    add(x2,z2,s2);
                    subtract(x2,z2,d2);
                    addition(x1,z1,s2,d2,xt,zt,n,x,z);
                    copy(x2,x1);
                    copy(z2,z1);
                    copy(x,x2);
                    copy(z,z2);
                }
                if (!cp[pos]) continue;
                subtract(fvw,fu[pos],t);
                mad(q,t,t,n,n,q);         /* batch gcds */
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
        if (nc>NCURVES) break;
        printf("\ntrying a different curve %d\n",nc);
    } 
    printf("\nfailed to factor\n");
}

