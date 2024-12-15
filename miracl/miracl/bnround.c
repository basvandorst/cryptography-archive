/*
 *   MIRACL euclidean mediant rounding routine
 *   bnround.c
 */

#include <stdio.h>
#include "miracl.h"
#define abs(x)  ((x)<0? (-(x)) : (x))

/* Access global variables */

extern int  depth;    /* error tracing .. */
extern int  trace[];  /* .. mechanism     */
extern int  nib;      /* length of bigs   */
extern small base;    /* number base      */
extern bool check;    /* overflow check   */

extern big w0;        /* workspace variables */
extern big w5;
extern big w6;
extern big w7;

int euclid(x,num)
big x;
int num;
{ /* outputs next c.f. quotient from gcd(w5,w6) */
    static long u,v;
    static small a,b,c,d;
    static int   n,oldn,q;
    static bool last,carryon;
    small r,m;
    long lr,lq;
    big t;
    if (num==0)
    {
        oldn=(-1);
        carryon=FALSE;
        last=FALSE;
        if (compare(w6,w5)>0)
        { /* ensure w5>w6 */
            t=w5,w5=w6,w6=t;
            return (q=0);
        }
    }
    else if (num==oldn || q<0) return q;
    oldn=num;
    if (carryon) goto middle;
start:
    if (size(w6)==0) return (q=(-1));
    n=w5[0];
    carryon=TRUE;
    a=1;
    b=0;
    c=0;
    d=1;
    if (n==1)
    {
        last=TRUE;
        u=w5[1];
        v=w6[1];
    }
    else
    {
        m=w5[n]+1;
        if (sizeof(long)==2*sizeof(small))
        { /* use longs if they are double length */
            if (n>2)
            { /* squeeze out as much significance as possible */
                u=muldiv(w5[n],base,w5[n-1],m,&r);
                u=u*base+muldiv(r,base,w5[n-2],m,&r);
                v=muldiv(w6[n],base,w6[n-1],m,&r);
                v=v*base+muldiv(r,base,w6[n-2],m,&r);
            }
            else
            {
                u=(long)base*w5[n]+w5[n-1];
                v=(long)base*w6[n]+w6[n-1];
                last=TRUE;
            }
        }
        else
        {
            u=muldiv(w5[n],base,w5[n-1],m,&r);
            v=muldiv(w6[n],base,w6[n-1],m,&r);
        }
    }
middle:
    forever
    { /* work only with most significant piece */
        if (last)
        {
            if (v==0) return (q=(-1));
            lq=u/v;
        }
        else
        {
            if (((v+c)==0) || ((v+d)==0)) break;
            lq=(u+a)/(v+c);
            if (lq!=(u+b)/(v+d)) break;
        }
        if (lq>=TOOBIG || lq>=MAXBASE/abs(d)) break;
        q=lq;
        r=a-q*c;
        a=c;
        c=r;
        r=b-q*d;
        b=d;
        d=r;
        lr=u-lq*v;
        u=v;
        v=lr;
        return q;
    }
    carryon=FALSE;
    if (b==0)
    { /* update w5 and w6 */
        check=OFF;
        divide(w5,w6,w7);
        check=ON;
        if (lent(w7)>nib) return (q=(-2));
        t=w5,w5=w6,w6=t;   /* swap(w5,w6) */
        copy(w7,x);
        return (q=size(x));
    }
    else
    {
        check=OFF;
        premult(w5,c,w7);
        premult(w5,a,w5);
        premult(w6,b,w0);
        premult(w6,d,w6);
        add(w5,w0,w5);
        add(w6,w7,w6);
        check=ON;
    }
    goto start;
}


void round(num,den,z)
big num,den;
flash z;
{ /* reduces and rounds the fraction num/den into z */
    int s;
    if (ERNUM) return;
    if (size(num)==0) 
    {
        zero(z);
        return;
    }
    depth++;
    trace[depth]=34;
    if (TRACER) track();
    if (size(den)==0)
    {
        berror(13);
        depth--;
        return;
    }
    copy(num,w5);
    copy(den,w6);
    s=exsign(w5)*exsign(w6);
    insign(PLUS,w5);
    insign(PLUS,w6);
    if (compare(w5,w6)==0)
    {
        convert((small)s,z);
        depth--;
        return;
    }
    if (size(w6)==1)
    {
        if (w5[0]>nib)
        {
            berror(13);
            depth--;
            return;
        }
        copy(w5,z);
        insign(s,z);
        depth--;
        return;
    }
    build(z,euclid);
    insign(s,z);
    depth--;
}
SHAR_EOF
chmod 0664 ./miracl/bnround.c || echo "restore of ./miracl/bnround.c fails"
set `wc -c ./miracl/bnround.c`;Sum=$1
if test "$Sum" != "3907"
then echo original size 3907, current size $Sum;fi
sed 's/^X//' << 'SHAR_EOF' > ./miracl/bnsmall.c &&
/*
 * MIRACL small number theoretic routines 
 * bnsmall.c
 */

#include <stdio.h>
#include <math.h>
#include "miracl.h"

/* access global variables */

extern int depth;     /* error tracing... */
extern int trace[];   /* ....mechanism    */
extern char *calloc();

int *gprime(maxp)
int maxp;
{ /* generate all primes less than maxp */
    int *primes;
    double dn;
    char *sv;
    int pix,i,k,prime;
    if (ERNUM) return NULL;
    if (maxp<0)
    { /* generate (-maxp) primes */
        dn=(-maxp);
        if (dn<20.0) dn=20.0;
        maxp=(int)(dn*(log(dn*log(dn))-0.5)); /* Rossers upper bound */
    }
    depth++;
    trace[depth]=70;
    if (TRACER) track();
    if (maxp>=TOOBIG)
    {
         berror(8);
         depth--;
         return NULL;
    }
    maxp=(maxp+1)/2;
    sv=(char *)calloc(maxp,1);
    if (sv==NULL)
    {
        berror(8);
        depth--;
        return NULL;
    }
    pix=1;
    for (i=0;i<maxp;i++)
        sv[i]=TRUE;
    for (i=0;i<maxp;i++)
    if (sv[i])
    { /* using sieve of Eratosthenes */
        prime=i+i+3;
        for (k=i+prime;k<maxp;k+=prime)
            sv[k]=FALSE;
        pix++;
    }
    primes=(int *)calloc(pix+2,sizeof(int));
    if (primes==NULL)
    {
        berror(8);
        depth--;
        return NULL;
    }
    primes[0]=2;
    pix=1;
    for (i=0;i<maxp;i++)
        if (sv[i]) primes[pix++]=i+i+3;
    primes[pix]=0;
    free(sv);
    depth--;
    return primes;
}

small smul(x,y,n)
small x,y,n;
{ /* returns x*y mod n */
    small r;
    x%=n;
    y%=n;
    if (x<0) x+=n;
    if (y<0) y+=n;
    muldiv(x,y,(small)0,n,&r);
    return r;
}

small spmd(x,n,m)
small x,n,m;
{ /*  returns x^n mod m  */
    small r;
    x%=m;
    if (x<0) x+=m;
    r=0;
    if (x==0) return r;
    r=1;
    if (n==0) return r;
    forever
    {
        if (n%2!=0) muldiv(r,x,(small)0,m,&r);
        n/=2;
        if (n==0) return r;
        muldiv(x,x,(small)0,m,&x);
    }
}

small inverse(x,y)
small x,y;
{ /* returns inverse of x mod y */
    small r,s,q,t,p;
    x%=y;
    if (x<0) x+=y;
    r=1;
    s=0;
    p=y;
    while (p!=0)
    { /* main euclidean loop */
        q=x/p;
        t=r-s*q;
        r=s;
        s=t;
        t=x-p*q;
        x=p;
        p=t;
    }
    if (r<0) r+=y;
    return r;
}

small sqrmp(x,m)
small x,m;
{ /* square root mod a small prime by Shanks method  *
   * returns 0 if root does not exist or m not prime */
    small q,z,y,v,w,t;
    int i,r,e,n;
    bool pp;
    x%=m;
    if (x==0) return 0;
    if (x<0) x+=m;
    if (x==1) return 1;
    if (spmd(x,(m-1)/2,m)!=1) return 0;    /* Legendre symbol not 1   */
    if (m%4==3) return spmd(x,(m+1)/4,m);  /* easy case for m=4.k+3   */
    q=m-1;
    e=0;
    while (q%2==0) 
    {
        q/=2;
        e++;
    }
    if (e==0) return 0;      /* even m */
    for (r=2;;r++)
    { /* find suitable z */
        z=spmd((small)r,q,m);
        if (z==1) continue;
        t=z;
        pp=FALSE;
        for (i=1;i<e;i++)
        { /* check for composite m */
            if (t==(m-1)) pp=TRUE;
            muldiv(t,t,(small)0,m,&t);
            if (t==1 && !pp) return 0;
        }
        if (t==(m-1)) break;
        if (!pp) return 0;   /* m is not prime */
    }
    y=z;
    r=e;
    v=spmd(x,(q+1)/2,m);
    w=spmd(x,q,m);
    while (w!=1)
    {
        t=w;
        for (n=0;t!=1;n++) muldiv(t,t,(small)0,m,&t);
        if (n>=r) return 0;
        y=spmd(y,(small)(1<<(r-n-1)),m);
        muldiv(v,y,(small)0,m,&v);
        muldiv(y,y,(small)0,m,&y);
        muldiv(w,y,(small)0,m,&w);
        r=n;
    }
    return v;
}

