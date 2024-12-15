/*
 *   Program to factor numbers
 *   Note: This is VERY tight for space in a single data segment. It fits 
 *   using Turbo C++ compiler V1.0 but not using Borland C++ V3.0
 *   if 'out of space' error occurs in Quadratic sieve, some static arrays
 *   will need to be reduced in size.
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <miracl.h>

#define LIMIT 15000
#define BTRIES 1000
#define MULT 2310

#define min(a,b) ((a) < (b)? (a) : (b))

static big *fu;
static bool *cp;
big n,nn;

brute()
{ /* find factors by brute force division */
    big x,y;
    int m,p;
    gprime(LIMIT);
    if (isprime(n))
    {
        printf("this number is prime!\n");
        exit(0);
    }
    x=mirvar(0);
    y=mirvar(0);
    m=0;
    p=PRIMES[0];
    forever
    { /* try division by each prime in turn */
        if (subdiv(n,p,y)==0)
        { /* factor found */
            copy(y,n);
            printf("prime factor     ");
            printf("%d\n",p);
            if (size(n)==1) exit(0);
            continue;
        }
        if (size(y)<=p) 
        { /* must be prime */
            printf("prime factor     ");
            cotnum(n,stdout);
            exit(0);
        }
        p=PRIMES[++m];
        if (p==0) break;
    }
    if (isprime(n)) 
    {
        printf("prime factor     ");
        cotnum(n,stdout);
        exit(0);
    }
    free(x);
    free(y);
    gprime(0);
}

brent()
{  /*  factoring program using Brents method */
    long k,r,i,m,iter;
    big x,y,ys,z,q,c3;
    x=mirvar(0);
    y=mirvar(0);
    ys=mirvar(0);
    z=mirvar(0);
    q=mirvar(0);
    c3=mirvar(3);
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
                if (iter>BTRIES)
                {
                    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                    free(x);
                    free(y);
                    free(ys);
                    free(z);
                    free(q);
                    free(c3);             
                    return 0;
                }
                if (iter%10==0) printf("\b\b\b\b\b%5ld",iter);
                copy(y,ys);
                for (i=1;i<=min(m,r-k);i++)
                {
                    mad(y,y,c3,n,n,y);
                    subtract(y,x,z);
                    mad(z,q,q,n,n,q);
                }
                egcd(q,n,z);
                k+=m;
            } while (k<r && size(z)==1);
            r*=2;
        } while (size(z)==1);
        if (compare(z,n)==0) do 
        { /* back-track */
            mad(ys,ys,c3,n,n,ys);
            subtract(ys,x,z);
        } while (egcd(z,n,z)==1);
        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
        if (!isprime(z))
             printf("composite factor ");
        else printf("prime factor     ");
        cotnum(z,stdout);
        if (compare(z,n)==0) exit(0);
        divide(n,z,n);
        divide(y,n,n);
    } while (!isprime(n));      
    printf("prime factor     ");
    cotnum(n,stdout);
    exit(0);
}

pollard(lim1,lim2)
int lim1;
long lim2;
{ /* factoring program using Pollards (p-1) method */
    long i,p,pa;
    int phase,m,pos,btch,iv;
    big t,b,bw,bvw,bd,bp,q;
    t=mirvar(0);
    b=mirvar(0);
    q=mirvar(0);
    bw=mirvar(0);
    bvw=mirvar(0);
    bd=mirvar(0);
    bp=mirvar(0);
    gprime(lim1);
    for (m=1;m<=MULT/2;m+=2)
        if (igcd(MULT,m)==1)
        {
            fu[m]=mirvar(0);
            cp[m]=TRUE;
        }
        else cp[m]=FALSE;
    phase=1;
    p=0;
    btch=50;
    i=0;
    convert(3,b);
    printf("phase 1 - trying all primes less than %d\n",lim1);
    printf("prime= %8ld",p);
    forever
    {
        if (phase==1)
        { /* looking for all factors of (p-1) < lim1 */
            p=PRIMES[i];
            if (PRIMES[i+1]==0)
            {
                 phase=2;
                 printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                 printf("phase 2 - trying last prime less than %ld\n"
                        ,lim2);
                 printf("prime= %8ld",p);
                 power(b,8,n,bw);
                 convert(1,t);
                 copy(b,bp);
                 copy(b,fu[1]);
                 for (m=3;m<=MULT/2;m+=2)
                 { /* store fu[m] = b^(m*m) */
                     mad(t,bw,bw,n,n,t);
                     mad(bp,t,t,n,n,bp);
                     if (cp[m]) copy(bp,fu[m]);
                 }
                 power(b,MULT,n,t);
                 power(t,MULT,n,t);
                 mad(t,t,t,n,n,bd);        /* bd=b^(2*MULT*MULT) */
                 iv=p/MULT;
                 if (p%MULT>MULT/2) iv++,p=2*(long)iv*MULT-p;
                 power(t,2*iv-1,n,bw);
                 power(t,iv,n,bvw);
                 power(bvw,iv,n,bvw);      /* bvw = b^(MULT*MULT*iv*iv) */
                 subtract(bvw,fu[p%MULT],q);
                 btch*=10;
                 i++;
                 continue;
            }
            pa=p;
            while ((lim1/p) > pa) pa*=p;
            power(b,(int)pa,n,b);
            decr(b,1,q);
        }
        else
        { /* looking for last prime factor of (p-1) < lim2 */
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
            subtract(bvw,fu[pos],t);
            mad(q,t,t,n,n,q);  /* batching gcds */
        }
        if (i++%btch==0)
        { /* try for a solution */
            printf("\b\b\b\b\b\b\b\b%8ld",p);
            egcd(q,n,t);
            if (size(t)==1)
            {
                if (p>lim2) 
                {
                    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                    break;
                }
                else continue;
            }
            if (compare(t,n)==0)
            {
                printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                printf("degenerate case\n");
                break;
            }
            printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
            if (isprime(t)) printf("prime factor     ");
            else            printf("composite factor ");
            cotnum(t,stdout);
            divide(n,t,n);
            if (isprime(n))
            {
                printf("prime factor     ");
                cotnum(n,stdout);
                exit(0);
            }
            break;
        }
    }
    gprime(0);
    free(t);
    free(b);
    free(q);
    free(bw);
    free(bvw);
    free(bd);
    free(bp);
    for (m=1;m<=MULT/2;m+=2)
        if (igcd(MULT,m)==1) free(fu[m]);
}

static big t,t2;

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

williams(lim1,lim2,ntrys)
int lim1,ntrys;
long lim2;
{  /*  factoring program using Williams (p+1) method */
    int k,phase,m,nt,iv,pos,btch;
    long i,p,pa;
    big b,q,fp,fvw,fd,fn;
    b=mirvar(0);
    t2=mirvar((-2));
    q=mirvar(0);
    t=mirvar(0);
    fp=mirvar(0);
    fvw=mirvar(0);
    fd=mirvar(0);
    fn=mirvar(0);
    gprime(lim1);
    for (m=1;m<=MULT/2;m+=2)
        if (igcd(MULT,m)==1)
        {
            fu[m]=mirvar(0);
            cp[m]=TRUE;
        }
        else cp[m]=FALSE;
    normalise(n,nn);
    for (nt=0,k=3;k<10;k++)
    { /* try more than once for p+1 condition (may be p-1) */
        convert(k,b);              /* try b=3,4,5..        */
        convert((k*k-4),t);
        if (egcd(t,n,t)!=1) continue; /* check (b*b-4,n)!=0 */
        nt++;
        phase=1;
        p=0;
        btch=50;
        i=0;
        printf("phase 1 - trying all primes less than %d\n",lim1);
        printf("prime= %8ld",p);
        forever
        { /* main loop */
            if (phase==1)
            { /* looking for all factors of p+1 < lim1 */
                p=PRIMES[i];
                if (PRIMES[i+1]==0)
                { /* now change gear */
                    phase=2;
                    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                    printf("phase 2 - trying last prime less than %ld\n"
                           ,lim2);
                    printf("prime= %8ld",p);
                    copy(b,fu[1]);
                    copy(b,fp);
                    mad(b,b,t2,nn,nn,fd);
                    negate(b,t);
                    mad(fd,b,t,nn,nn,fn);
                    for (m=5;m<=MULT/2;m+=2)
                    { /* store fu[m] = Vm(b) */
                        negate(fp,t);
                        mad(fn,fd,t,nn,nn,t);
                        copy(fn,fp);
                        copy(t,fn);
                        if (!cp[m]) continue;
                        copy(t,fu[m]);
                    }
                    lucas(b,MULT,nn,fp,fd);
                    iv=p/MULT;
                    if (p%MULT>MULT/2) iv++,p=2*(long)iv*MULT-p;
                    lucas(fd,iv,nn,fp,fvw);
                    negate(fp,fp);
                    subtract(fvw,fu[p%MULT],q);
                    btch*=10;
                    i++;
                    continue;
                }
                pa=p;
                while ((lim1/p) > pa) pa*=p;
                lucas(b,(int)pa,nn,fp,q);
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
                    mad(fvw,fd,fp,nn,nn,fvw);
                    negate(t,fp);
                }
                if (!cp[pos]) continue;
                subtract(fvw,fu[pos],t);
                mad(q,t,t,nn,nn,q);  /* batching gcds */
            }
            if (i++%btch==0)
            { /* try for a solution */
                printf("\b\b\b\b\b\b\b\b%8ld",p);
                egcd(q,n,t);
                if (size(t)==1)
                {
                    if (p>lim2) 
                    {
                        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                        break;
                    }
                    else continue;
                }
                if (compare(t,n)==0)
                {
                    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                    printf("degenerate case\n");
                    break;
                }
                printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                if (isprime(t)) printf("prime factor     ");
                else          printf("composite factor ");
                cotnum(t,stdout);
                divide(n,t,n);
                if (isprime(n)) 
                {
                    printf("prime factor     ");
                    cotnum(n,stdout);
                    exit(0);
                }
                nt=ntrys;
                break;
            }
        } 
        if (nt>=ntrys) break;
    }
    gprime(0);
    free(b);
    free(t2);
    free(q);
    free(t);
    free(fp);
    free(fvw);
    free(fd);
    free(fn);
    for (m=1;m<=MULT/2;m+=2)
        if (igcd(MULT,m)==1) free(fu[m]);
}

static big ak,tt,s1,d1,s2,d2;

void duplication(sum,diff,n,x,z)
big sum,diff,n,x,z;
{ /* double a point on the curve P(x,z)=2.P(x1,z1) */
    mad(sum,sum,sum,n,n,tt);   
    mad(diff,diff,diff,n,n,z); 
    mad(tt,z,z,n,n,x);           /* x = sum^2.diff^2 */
    subtract(tt,z,tt);            /* tt = sum^2-diff^2 */
    mad(ak,tt,z,n,n,z);          /* z = ak*tt +diff^2 */
    mad(z,tt,tt,n,n,z);           /* z = z.tt          */
}

void addition(xd,zd,sm1,df1,sm2,df2,n,x,z)
big xd,zd,sm1,df1,sm2,df2,n,x,z;
{ /* add two points on the curve P(x,z)=P(x1,z1)+P(x2,z2) *
   * given their difference P(xd,zd)                      */
        mad(df2,sm1,sm1,n,n,x);
        mad(df1,sm2,sm2,n,n,z);
        add(z,x,tt);
        subtract(z,x,z);
        mad(tt,tt,tt,n,n,x);
        mad(zd,x,x,n,n,x);     /* x = zd.[df1.sm2+sm1.df2]^2 */
        mad(z,z,z,n,n,z);
        mad(xd,z,z,n,n,z);     /* z = xd.[df1.sm2-sm1.df2]^2 */
}

void ellipse(x,z,r,n,x1,z1,x2,z2)
big x,z,n,x1,z1,x2,z2;
int r;
{ /* calculate point r.P(x,z) on curve */
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

lenstra(lim1,lim2,ncurves)
int lim1,ncurves;
long lim2;
{  /*  factoring program using Lenstras Elliptic Curve method */
    int phase,m,k,nc,iv,pos,btch;
    long i,p,pa;
    big q,x,z,a,x1,z1,x2,z2,xt,zt,fvw;
    q=mirvar(0);
    x=mirvar(0);
    z=mirvar(0);
    a=mirvar(0);
    x1=mirvar(0);
    z1=mirvar(0);
    x2=mirvar(0);
    z2=mirvar(0);
    tt=mirvar(0);
    s1=mirvar(0);
    d1=mirvar(0);
    s2=mirvar(0);
    d2=mirvar(0);
    ak=mirvar(0);
    xt=mirvar(0);
    zt=mirvar(0);
    fvw=mirvar(0);
    gprime(lim1);
    for (m=1;m<=MULT/2;m+=2)
        if (igcd(MULT,m)==1)
        {
            fu[m]=mirvar(0);
            cp[m]=TRUE;
        }
        else cp[m]=FALSE;
    normalise(n,nn);
    for (nc=1,k=3;k<100;k++)
    { /* try a new curve */
        convert(2,x);         /* generating an elliptic curve */
        convert(1,z);
        convert((2*(k*k-1)),tt);
        if (xgcd(tt,n,tt,tt,tt)!=1) continue;
        convert((4-k*k),a);
        mad(a,tt,tt,nn,nn,tt);
        if (size(tt)<0) add(tt,nn,tt);
        copy(tt,a);        
        if (xgcd(tt,n,tt,tt,tt)!=1) continue;
        nc++;
        add(a,tt,a);
        divide(a,nn,nn);
        convert(4,ak);
        xgcd(ak,n,ak,ak,ak);
        incr(a,2,tt);
        mad(ak,tt,tt,nn,nn,ak);    /* ak = (a+2)/4 */
        phase=1;
        p=0;
        i=0;
        btch=50;
        printf("phase 1 - trying all primes less than %d\n",lim1);
        printf("prime= %8ld",p);
        forever
        { /* main loop */
            if (phase==1)
            {
                p=PRIMES[i];
                if (PRIMES[i+1]==0)
                { /* now change gear */
                    phase=2;
                    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                    printf("phase 2 - trying last prime less than %ld\n",
                            lim2);
                    printf("prime= %8ld",p);
                    copy(x,xt);
                    copy(z,zt);
                    add(x,z,s2);
                    subtract(x,z,d2);                    /*   P = (s2,d2) */
                    duplication(s2,d2,nn,x,z);
                    add(x,z,s1);
                    subtract(x,z,d1);                    /* 2.P = (s1,d1) */
                    xgcd(z1,n,fu[1],fu[1],fu[1]);        /* fu[1] = x1/z1 */
                    mad(x1,fu[1],x1,nn,nn,fu[1]);
                    addition(x1,z1,s1,d1,s2,d2,nn,x2,z2); /* 3.P = (x2,z2) */
                    for (m=5;m<=MULT/2;m+=2)
                    { /* calculate m.P = (x,z) and store fu[m] = x/z */
                        add(x2,z2,s2);
                        subtract(x2,z2,d2);
                        addition(x1,z1,s2,d2,s1,d1,nn,x,z);
                        copy(x2,x1);
                        copy(z2,z1);
                        copy(x,x2);
                        copy(z,z2);
                        if (!cp[m]) continue;
                        copy(z2,fu[m]);
                        xgcd(fu[m],n,fu[m],fu[m],fu[m]);
                        mad(x2,fu[m],x2,nn,nn,fu[m]);
                    }
                    ellipse(xt,zt,MULT,nn,x,z,x2,z2);
                    add(x,z,xt);
                    subtract(x,z,zt);              /* MULT.P = (xt,zt) */
                    iv=p/MULT;
                    if (p%MULT>MULT/2) iv++,p=2*(long)iv*MULT-p;
                    ellipse(x,z,iv,nn,x1,z1,x2,z2); /* (x1,z1) = iv.MULT.P */
                    xgcd(z1,n,fvw,fvw,fvw);
                    mad(x1,fvw,x1,nn,nn,fvw);        /* fvw = x1/z1 */
                    subtract(fvw,fu[p%MULT],q);
                    btch*=10;
                    i++;
                    continue;
                }
                pa=p;
                while ((lim1/p) > pa) pa*=p;
                ellipse(x,z,(int)pa,nn,x1,z1,x2,z2);
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
                    mad(x2,fvw,x2,nn,nn,fvw);
                    add(x2,z2,s2);
                    subtract(x2,z2,d2);
                    addition(x1,z1,s2,d2,xt,zt,nn,x,z);
                    copy(x2,x1);
                    copy(z2,z1);
                    copy(x,x2);
                    copy(z,z2);
                }
                if (!cp[pos]) continue;
                subtract(fvw,fu[pos],tt);
                mad(q,tt,tt,nn,nn,q);         /* batch gcds */
            }
            if (i++%btch==0)
            { /* try for a solution */
                printf("\b\b\b\b\b\b\b\b%8ld",p);
                egcd(q,n,tt);
                if (size(tt)==1)
                {
                    if (p>lim2) 
                    {
                        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                        break;
                    }
                    else continue;
                }
                if (compare(tt,n)==0)
                {
                    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                    printf("degenerate case\n");
                    break;
                }
                printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                if (isprime(tt)) printf("prime factor     ");
                else          printf("composite factor ");
                cotnum(tt,stdout);
                divide(n,tt,n);
                if (isprime(n))
                {
                    printf("prime factor     ");
                    cotnum(n,stdout);
                    exit(0);
                }
                nc=ncurves+1;
                break;
            }
        }
        if (nc>ncurves) break;
    } 
    gprime(0);
    free(q);
    free(x);
    free(z);
    free(a);
    free(x1);
    free(z1);
    free(x2);
    free(z2);
    free(tt);
    free(s1);
    free(d1);
    free(s2);
    free(d2);
    free(ak);
    free(xt);
    free(zt);
    free(fvw);
    for (m=1;m<=MULT/2;m+=2)
        if (igcd(MULT,m)==1) free(fu[m]);
}

#define MEM 240        /* Maximum size of factor base   */
#define MLF 80         /* Max. number of large factors  */
#define PAK 1+MEM/(MR_IBITS)
#define SSIZE 10000    /* Maximum sieve size            */

static big NN,TT,DD,RR,VV,PP,XX,YY,DG,IG,AA,BB;
static big x[MEM+1],y[MEM+1],z[MLF+1],w[MLF+1];
static unsigned int EE[MEM+1][PAK],G[MLF+1][PAK];
static int *ep,*r1,*r2,*rp,*b,*pr,*e,*hash;
static unsigned char *logp;
static int mm,jj,nbts,nlp,lp,hmod,hmod2;
static bool partial;

int knuth(d,ep,N,R,D,T)
int d;
int ep[];
big N,R,D,T;
{ /* Input number to be factored N, find best multiplier k  *
   * set D=k.N and calculate factor base ep[] and R=sqrt(D) */
    double fks,dp,top;
    bool found;
    int i,j,mm,bk,nk,m,kk,r,p,maxp;
    static int K[]={0,1,2,3,5,6,7,10,11,13,14,15,17,0};
    copy (n,N);
    
    if (d<10) m=d;
    else m=25;
    if (d>20) m=30*(d-20);
    if (m>MEM) m=MEM;
    dp=(double)2*(m+25);          /* number of primes to generate */
    if (dp<20.0) dp=20.0;
    maxp=(int)(dp*(log(dp*log(dp))-0.5)); /* Rossers upper bound */
    gprime(maxp);
    top=(-10.0e0);
    found=FALSE;
    nk=0;
    bk=0;
    ep[0]=1;
    ep[1]=2;
    do
    { /* search for best Knuth-Schroepel multiplier */
        mm=m;
        kk=K[++nk];
        if (kk==0)
        { /* finished */
            kk=K[bk];
            found=TRUE;
        }
        premult(N,kk,D);
        if (nroot(D,2,R))
        {
            printf("%dN is a perfect square!\n",kk);
            continue;
        }
        fks=log(2.0e0)/(2.0e0);
        r=subdiv(D,8,T);
        if (r==1) fks*=(4.0e0);
        if (r==5) fks*=(2.0e0);
        fks-=log((double)kk)/(2.0e0);
        i=0;
        j=1;
        while (j<m)
        { /* select small primes */
            p=PRIMES[++i];
            if (p==0)
            {
                mm=j;
                break;
            }
            r=subdiv(D,p,T);
            if (spmd(r,(p-1)/2,p)<=1)
            { /* use only if Jacobi symbol = 0 or 1 */
                ep[++j]=p;
                dp=(double)p;
                if (kk%p==0) fks+=log(dp)/dp;
                else         fks+=2*log(dp)/(dp-1.0e0);
            }
        }
        if (fks>top)
        { /* find biggest fks */
            top=fks;
            bk=nk;
        }
    } while (!found);
    printf("using multiplier k= %d",kk);
    printf(" and %d small primes as factor base\n",mm);
    gprime(0);   /* reclaim PRIMES space */
    return mm;
}

bool factored(lptr,T)
big T;
long lptr;
{ /* factor quadratic residue */
    bool facted;
    int i,j,r,st;  
    partial=FALSE;
    facted=FALSE;
    for (j=1;j<=mm;j++)
    { /* now attempt complete factorisation of T */
        r=lptr%ep[j];
        if (r<0) r+=ep[j];
        if (r!=r1[j] && r!=r2[j]) continue;
        while (subdiv(T,ep[j],XX)==0)
        { /* cast out ep[j] */
            e[j]++;
            copy(XX,T);
        }
        st=size(T);
        if (st==1)
        {
           facted=TRUE;
           break;
        }
        if (size(XX)<=ep[j])
        { /* st is prime < ep[mm]^2 */
            if (st>=TOOBIG || (st/ep[mm])>(1+MLF/50)) break;
            if (st<=ep[mm])
                for (i=j;i<=mm;i++)
                if (st==ep[i])
                {
                    e[i]++;
                    facted=TRUE;
                    break;
                }
            if (facted) break;
            lp=st;  /* factored with large prime */
            partial=TRUE;
            facted=TRUE;
            break;
        }
    }
    return facted;
}

bool gotcha()
{ /* use new factorisation */
    int r,j,i,k,n,rb,had,hp;
    bool found;
    found=TRUE;
    if (partial)
    { /* check partial factorisation for usefulness */
        had=lp%hmod;
        forever
        { /* hash search for matching large prime */
            hp=hash[had];
            if (hp<0)
            { /* failed to find match */
                found=FALSE;
                break;
            }
            if (pr[hp]==lp) break; /* hash hit! */
            had=(had+(hmod2-lp%hmod2))%hmod;
        }
        if (!found && nlp>=MLF) return FALSE;
    }
    copy(PP,XX);
    convert(1,YY);
    for (k=1;k<=mm;k++)
    { /* build up square part in YY  *
       * reducing e[k] to 0s and 1s */
        if (e[k]<2) continue;
        r=e[k]/2;
        e[k]%=2;
        convert(ep[k],TT);
        power(TT,r,TT,TT);
        multiply(TT,YY,YY);
    }
/* debug only - print out details of each factorization 
    cotnum(XX,stdout);
    cotnum(YY,stdout);
    if (e[0]==1) printf("-1");
    else printf("1");
    for (k=1;k<=mm;k++)
    {
        if (e[k]==0) continue;
        printf(".%d",ep[k]);
    }
    if (partial) printf(".%d\n",lp);
    else printf("\n");
*/
    if (partial)
    { /* factored with large prime */
        if (!found)
        { /* store new partial factorization */
            hash[had]=nlp;
            pr[nlp]=lp;
            copy(XX,z[nlp]);
            copy(YY,w[nlp]);
            for (n=0,rb=0,j=0;j<=mm;j++)
            {
                G[nlp][n]|=((e[j]&1)<<rb);
                if (++rb==nbts) n++,rb=0;
            }
            nlp++;
        }
        if (found)
        { /* match found so use as factorization */
            printf("\b\b\b\b\b*");
            mad(XX,z[hp],XX,NN,NN,XX);
            mad(YY,w[hp],YY,NN,NN,YY);
            for (n=0,rb=0,j=0;j<=mm;j++)
            {
                e[j]+=((G[hp][n]>>rb)&1);
                if (e[j]==2)
                {
                    premult(YY,ep[j],YY);
                    divide(YY,NN,NN);
                    e[j]=0;
                }
                if (++rb==nbts) n++,rb=0;
            }
            premult(YY,lp,YY);
            divide(YY,NN,NN);
        }
    }
    else printf("\b\b\b\b\b ");
    if (found)
    {
        for (k=mm;k>=0;k--)
        { /* use new factorization in search for solution */
            if (e[k]%2==0) continue;
            if (b[k]<0)
            { /* no solution this time */
                found=FALSE;
                break;
            }
            i=b[k];
            mad(XX,x[i],XX,NN,NN,XX);    /* This is very inefficient -  */
            mad(YY,y[i],YY,NN,NN,YY);    /* There must be a better way! */
            for (n=0,rb=0,j=0;j<=mm;j++)
            { /* Gaussian elimination */
                e[j]+=((EE[i][n]>>rb)&1);
                if (++rb==nbts) n++,rb=0;
            }
        }
        for (j=0;j<=mm;j++)
        { /* update YY */
            if (e[j]<2) continue;
            convert(ep[j],TT);
            power(TT,e[j]/2,NN,TT);
            mad(YY,TT,YY,NN,NN,YY);
        }
        if (!found)
        { /* store details in EE, x and y for later */
            b[k]=jj;
            copy(XX,x[jj]);
            copy(YY,y[jj]);
            for (n=0,rb=0,j=0;j<=mm;j++)
            {
                EE[jj][n]|=((e[j]&1)<<rb);
                if (++rb==nbts) n++,rb=0;
            }
            jj++;
            printf("%4d",jj);
        }
    }
    if (found)
    { /* check for false alarm */
        printf("\ntrying...\n");
        add(XX,YY,TT);
        if (compare(XX,YY)==0 || compare(TT,NN)==0) found=FALSE;
        if (!found) printf("working... %4d",jj);
    }
    return found;
}

void initv(d)
int d;
{ /* initialize big numbers and arrays */
    int i,j;
    ep=(int *)malloc((MEM+1)*sizeof(int));
    r1=(int *)malloc((MEM+1)*sizeof(int));
    r2=(int *)malloc((MEM+1)*sizeof(int));
    rp=(int *)malloc((MEM+1)*sizeof(int));
    b=(int *)malloc((MEM+1)*sizeof(int));
    e=(int *)malloc((MEM+1)*sizeof(int));
    pr=(int *)malloc((MLF+1)*sizeof(int));
    hash=(int *)malloc((2*MLF+1)*sizeof(int));
    logp=(unsigned char *)malloc(MEM+1);
    NN=mirvar(0); 
    TT=mirvar(0);
    DD=mirvar(0);
    RR=mirvar(0);
    VV=mirvar(0);
    PP=mirvar(0);
    XX=mirvar(0);
    YY=mirvar(0);
    DG=mirvar(0);
    IG=mirvar(0);
    AA=mirvar(0);
    BB=mirvar(0);
    for (i=0;i<=MEM;i++)
    {
        x[i]=mirvar(0);
        y[i]=mirvar(0);
    }
    for (i=0;i<=MLF;i++)
    {
        z[i]=mirvar(0);
        w[i]=mirvar(0);
    }
    nbts=8*sizeof(int);
    mm=knuth(d,ep,NN,RR,DD,TT);

    for (i=0;i<=mm;i++)
    { 
        b[i]=(-1);
        for (j=0;j<=mm/nbts;j++)
            EE[i][j]=0;
    }
    for (i=0;i<=MLF;i++)
        for (j=0;j<=mm/nbts;j++)
            G[i][j]=0;
}
 
qsieve(d)
int d;
{ /* factoring via quadratic sieve */
    unsigned int i,j,a,isz;
    int S,r,s1,s2,s,NS,logm,ptr,threshold;
    unsigned char *sieve;
    unsigned int  *isp;
    long M,la,lptr;
    sieve=(unsigned char *)malloc(SSIZE+1);

    initv(d);
    isz=SSIZE/sizeof(int);
    isp=(unsigned int *)sieve;  /* equivalence sieve to int array */

    hmod=2*MLF+1;               /* set up hash table */
    convert(hmod,TT);
    while (!isprime(TT)) decr(TT,2,TT);
    hmod=size(TT);
    hmod2=hmod-2;
    for (i=0;i<hmod;i++) hash[i]=(-1);

    M=50*(long)mm;
    NS=M/SSIZE;
    if (M%SSIZE!=0) NS++;
    M=SSIZE*(long)NS;
    logm=0;
    la=M;
    while ((la/=2)>0) logm++;   /* logm = log(M) */

    rp[0]=logp[0]=0;
    for (i=1;i<=mm;i++)
    { /* find root mod each prime, and approx log of each prime */
        r=subdiv(DD,ep[i],TT);
        rp[i]=sqrmp(r,ep[i]);
        logp[i]=0;
        r=ep[i];
        while((r/=2)>0) logp[i]++;
    }
    r=subdiv(DD,8,TT);    /* take special care of 2 */
    if (r==5) logp[1]++;
    if (r==1) logp[1]+=2;

    threshold=logm+logb2(RR)-2*logp[mm];

    jj=0;
    nlp=0;
    premult(DD,2,DG);
    nroot(DG,2,DG);
    lgconv(M,TT);
    divide(DG,TT,DG);
    nroot(DG,2,DG);
    if (subdiv(DG,2,TT)==0) incr(DG,1,DG);
    if (subdiv(DG,4,TT)==1) incr(DG,2,DG);
    printf("working...    0");
    forever
    { /* try a new polynomial */
        r=MR_NTRY;
        MR_NTRY=1;         /* speed up search for prime */
        do
        { /* looking for suitable prime DG = 3 mod 4 */
            do incr(DG,4,DG); while(!isprime(DG));
            decr(DG,1,TT);
            subdiv(TT,2,TT);
            powmod(DD,TT,DG,TT);  /* check D is quad residue */
        } while (size(TT)!=1);
        MR_NTRY=r;
        incr(DG,1,TT);
        subdiv(TT,4,TT);
        powmod(DD,TT,DG,BB);
        negate(DD,TT);
        mad(BB,BB,TT,DG,TT,TT);
        negate(TT,TT);
        premult(BB,2,AA);
        xgcd(AA,DG,AA,AA,AA);
        mad(AA,TT,TT,DG,DG,AA);
        multiply(AA,DG,TT);
        add(BB,TT,BB);        /* BB^2 = DD mod DG^2 */
        multiply(DG,DG,AA);   /* AA = DG*DG         */
        xgcd(DG,DD,IG,IG,IG); /* IG = 1/DG mod DD  */

        r1[0]=r2[0]=0;
        for (i=1;i<=mm;i++) 
        { /* find roots of quadratic mod each prime */
            s=subdiv(BB,ep[i],TT);
            r=subdiv(AA,ep[i],TT);
            r=invers(r,ep[i]);     /* r = 1/AA mod p */
            s1=(-s+rp[i]);
            s2=(-s-rp[i]);
            r1[i]=smul(s1,r,ep[i]);
            r2[i]=smul(s2,r,ep[i]);
        }

        for (ptr=(-NS);ptr<NS;ptr++)
        { /* sieve over next period */
            la=(long)ptr*SSIZE;
            for (i=0;i<isz;i++) isp[i]=0;
            for (i=1;i<=mm;i++)
            { /* sieving with each prime */
                r=la%ep[i];
                s1=(r1[i]-r)%ep[i];
                if (s1<0) s1+=ep[i];
                s2=(r2[i]-r)%ep[i];
                if (s2<0) s2+=ep[i];
           /* these loops are time critical */
                for (j=s1;j<SSIZE;j+=ep[i]) sieve[j]+=logp[i];
                if (s1==s2) continue;
                for (j=s2;j<SSIZE;j+=ep[i]) sieve[j]+=logp[i];
            }

            for (a=0;a<SSIZE;a++)
            { /* main loop - look for fully factored residues */
                if (sieve[a]<threshold) continue;
                lptr=la+a;
                lgconv(lptr,TT);
                S=0;
                multiply(AA,TT,TT);           /* TT = AAx + BB      */
                add(TT,BB,TT);
                mad(TT,IG,TT,DD,DD,PP);       /* PP = (AAx + BB)/G  */
                if (size(PP)<0) add(PP,DD,PP);
                mad(PP,PP,PP,DD,DD,VV);       /* VV = PP^2 mod kN  */
                absol(TT,TT);
                if (compare(TT,RR)<0) S=1;    /* check for -ve VV */
                if (S==1) subtract(DD,VV,VV);
                copy(VV,TT);
                e[0]=S;
                for (i=1;i<=mm;i++) e[i]=0;
                if (!factored(lptr,TT)) continue;
                if (gotcha())
                { /* factors found! */
                    egcd(TT,NN,PP);
                    if (isprime(PP)) printf("prime factor     ");
                    else             printf("composite factor ");
                    cotnum(PP,stdout);
                    divide(NN,PP,NN);
                    if (isprime(NN)) printf("prime factor     ");
                    else             printf("composite factor ");
                    cotnum(NN,stdout);
                    exit(0);
                }
            }
        }
    }
}

int digits()
{ /* size of n */
    int d;
    t=mirvar(0);
    d=0;
    copy(n,t);
    while (size(t)!=0)
    {
        subdiv(t,10,t);
        d++;
    }
    free(t);
    return d;
}

static char *s;

void eval_power (oldn,n,op)
big oldn,n;
char op;
{
        int i;
        if (op) power(oldn,size(n),n,n);
}

void eval_product (oldn,n,op)
big oldn,n;
char op;
{
        switch (op)
        {
        case '*':
                multiply(n,oldn,n);
                break;
        case '/':
                copy(oldn,t);
                divide(t,n,t);
                copy(t,n);
                break;
        case '%':
                copy(oldn,t);
                divide(t,n,n);
                copy(t,n);
        }
}

void eval_sum (oldn,n,op)
big oldn,n;
char op;
{
        switch (op)
        {
        case '+':
                add(n,oldn,n);
                break;
        case '-':
                subtract(oldn,n,n);
        }
}

void eval ()
{
        big oldn[3];
        big n;
        int i;
        char oldop[3];
        char op;
        char minus;
        n=mirvar(0);
        for (i=0;i<3;i++)
        {
            oldop[i]=0;
            oldn[i]=mirvar(0);
        }
LOOP:
        while (*s==' ')
        s++;
        if (*s=='-')    /* Unary minus */
        {
        s++;
        minus=1;
        }
        else
        minus=0;
        while (*s==' ')
        s++;
        if (*s=='(')    /* Number is subexpression */
        {
        s++;
        eval ();
        copy(t,n);     
        }
        else            /* Number is decimal value */
        {
        for (i=0;s[i]>='0' && s[i]<='9';i++)
                ;
        if (!i)         /* No digits found */
        {
                printf ("Error - invalid number\n");
                exit (20);
        }
        op=s[i];
        s[i]=0;
        lgconv(atol(s),n);
        s+=i;
        *s=op;
        }
        if (minus) negate(n,n);
        do
        op=*s++;
        while (op==' ');
        if (op==0 || op==')')
        {
        eval_power (oldn[2],n,oldop[2]);
        eval_product (oldn[1],n,oldop[1]);
        eval_sum (oldn[0],n,oldop[0]);
        copy(n,t);
        free(n);
        for (i=0;i<2;i++) free(oldn[i]);
        return;
        }
        else
        {
        if (op=='#')
        {
                eval_power (oldn[2],n,oldop[2]);
                copy(n,oldn[2]);
                oldop[2]='#';
        }
        else
        {
                if (op=='*' || op=='/' || op=='%')
                {
                eval_power (oldn[2],n,oldop[2]);
                oldop[2]=0;
                eval_product (oldn[1],n,oldop[1]);
                copy(n,oldn[1]);
                oldop[1]=op;
                }
                else
                {
                if (op=='+' || op=='-')
                {
                        eval_power (oldn[2],n,oldop[2]);
                        oldop[2]=0;
                        eval_product (oldn[1],n,oldop[1]);
                        oldop[1]=0;
                        eval_sum (oldn[0],n,oldop[0]);
                        copy(n,oldn[0]);
                        oldop[0]=op;
                }
                else    /* Error - invalid operator */
                {
                        printf ("Error - invalid operator\n");
                        exit (20);
                }
                }
        }
        }
        goto LOOP;
}

main(argc,argv)
int argc;
char **argv;
{
    argv++;argc--;
    if (argc<1 || argc>2 || (argc==2 && strcmp(argv[0],"-f")!=0))
    {
      printf("Incorrect Usage\n");
      printf("factor <number>\n");
      printf("OR\n");
      printf("factor -f <formula>\n");
      printf("e.g. factor 10000000000000000000000000000000009\n");
      printf("or   factor -f 10#34+9\n");
      printf("Freeware from Shamus Software, 94 Shangan rd, Dublin 9, Ireland\n");
      printf("Full C source code and MIRACL multiprecision library available\n");
      printf("Write to the above address for details\n");
      exit(0);
    }
    mirsys(-32,0);
    n=mirvar(0);
    nn=mirvar(0);
    if (argc==1) cinstr(n,argv[0]);
    else
    {
        s=argv[1];
        t=mirvar(0);
        eval();
        copy(t,n);
        free(t);
        printf("factoring ");
        cotnum(n,stdout);
    }

    printf("first trying brute force division by small primes\n");
    brute();
    printf("now trying %d iterations of brent's method\n",BTRIES);
    brent();
    fu= (big *)malloc((1+MULT/2)*sizeof(big));
    cp=(bool *)malloc((1+MULT/2)*sizeof(bool));
    if (digits()>25)
    {
        printf("now trying william's (p+1) method 3 times\n");
        williams(4000,160000L,3);
        printf("now trying pollard's (p-1) method\n");
        pollard(10000,300000L);
    }
    if (digits()>35)
    {
        printf("o.k. you asked for it - lenstra's method using 8 curves\n");
        lenstra(2000,100000L,8);
    }
    free(cp);
    free(fu);
    if (digits()<45)
    {
        printf("finally - the quadratic sieve\n");
        qsieve(digits());
    }
    printf("I give up - composite factor\n");
    cotnum(n,stdout);
}

