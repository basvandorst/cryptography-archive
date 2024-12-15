/*
 *  MIRACL Double to Flash conversion routines - use with care
 *  mrdouble.c
 *
 *  Copyright (c) 1988-1993 Shamus Software Ltd.
 *
 *  NOTE: The occasional use of (double)(unsigned long) instead of simply 
 *        (double) is to circumvent a bug in Zortech C & C++ V2.0
 */

#include <stdio.h>
#include <math.h>
#include <miracl.h>

#ifdef MR_FLASH

#define abs(x)  ((x)<0? (-(x)) : (x))
#define sign(x) ((x)<0? (-1) : 1)

/* Access global variables */

extern int  mr_depth;    /* error tracing .. */
extern int  mr_trace[];  /* .. mechanism     */
extern small mr_base;    /* number base      */
extern int  mr_nib;      /* length of bigs   */

extern big mr_w1;       /* workspace variables  */

static double D;

static int dquot(x,num)
big x;
int num;
{ /* generate c.f. for a double D */
    static double b,n,p;
    static int q,oldn;
    int m;
    if (num==0)
    {
        oldn=(-1);
        if (mr_base==0) b=pow(2.0,(double)MR_SBITS);
        else b=(double)(unsigned long)mr_base;
        if (D<1.0)
        {
            D=(1.0/D);
            return (q=0);
        }
    }
    else if (q<0 || num==oldn) return q;
    oldn=num;
    if (D==0.0) return (q=(-1));
    D=modf(D,&n);  /* n is whole number part */
    m=0;           /* D is fractional part (or guard digits!) */
    zero(x);
    while (n>0.0)
    { /* convert n to big */
        m++;
        if (m>mr_nib) return (q=(-2));
        p=n/b;
        modf(p,&p);
        x[m]=(small)(n-b*p);
        n=p;
    }
    x[0]=m;
    if (D>0.0) D=(1.0/D);
    return (q=size(x));
}

void dconv(d,w)
double d;
flash w;
{ /* convert double to rounded flash */
    int s;
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=32;
    if (TRACER) track();
    zero(w);
    if (d==0.0)
    {
        mr_depth--;
        return;
    }
    D=d;
    s=sign(D);
    D=abs(D);
    build(w,dquot);
    insign(s,w);
    mr_depth--;
}
        
double fdsize(w)
flash w;
{ /* express flash number as double. */
    int i,s,en,ed;
    double n,d,b,BIGGEST;
    if (MR_ERNUM || size(w)==0) return (0.0);
    mr_depth++;
    mr_trace[mr_depth]=11;
    if (TRACER) track();
    BIGGEST=pow(2.0,(double)(1<<(MR_EBITS-3)));
    EXACT=FALSE;
    n=0.0;
    d=0.0;
    if (mr_base==0) b=pow(2.0,(double)MR_SBITS);
    else b=(double)(unsigned long)mr_base;
    numer(w,mr_w1);
    s=exsign(mr_w1);
    insign(PLUS,mr_w1);
    en=(int)mr_w1[0];
    for (i=1;i<=en;i++)
        n=(double)(unsigned long)mr_w1[i]+(n/b);
    denom(w,mr_w1);
    ed=(int)mr_w1[0];
    for (i=1;i<=ed;i++)
        d=(double)(unsigned long)mr_w1[i]+(d/b);
    n=(s*(n/d));
    while (en!=ed)
    {
        if (en>ed)
        {
            ed++;
            if (BIGGEST/b<n)
            {
                berror(16);
                mr_depth--;
                return (0.0);
            }
            n*=b;
        }
        else
        {
            en++;
            if (n<b/BIGGEST)
            {
                berror(16);
                mr_depth--;
                return (0.0);
            }
            n/=b;
        }
    }
    mr_depth--;
    return n;
}

#endif
