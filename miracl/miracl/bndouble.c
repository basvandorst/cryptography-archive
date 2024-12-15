/*
 *   MIRACL Double to Flash conversion routines - use with care
 *   bndouble.c
 */

#include <stdio.h>
#include <math.h>
#include "miracl.h"
#define abs(x)  ((x)<0? (-(x)) : (x))
#define sign(x) ((x)<0? (-1) : 1)

/* Access global variables */

extern int  depth;    /* error tracing .. */
extern int  trace[];  /* .. mechanism     */
extern small base;    /* number base      */
extern int  nib;      /* length of bigs   */
extern int  workprec;
extern bool check;    /* overflow check   */

extern big w1;       /* workspace variables  */
extern big w2;
extern big w3;
extern big w4;
extern big w5;
extern big w6;

extern double modf();
double D;

int dquot(x,num)
flash x;
int num;
{ /* generate c.f. for a double D */
    static double b,n,p;
    static int q,oldn;
    int m;
    if (num==0)
    {
        oldn=(-1);
        b=(double)base;
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
        if (m>nib) return (q=(-2));
        p=n/b;
        modf(p,&p);
        x[m]=n-b*p;
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
    if (ERNUM) return;
    depth++;
    trace[depth]=32;
    if (TRACER) track();
    zero(w);
    if (d==0.0)
    {
        depth--;
        return;
    }
    D=d;
    s=sign(D);
    D=abs(D);
    build(w,dquot);
    insign(s,w);
    depth--;
}
        
double fdsize(w)
flash w;
{ /* express flash number as double. */
    int i,s,en,ed;
    double n,d,b;
    if (ERNUM || size(w)==0) return (0.0);
    depth++;
    trace[depth]=11;
    if (TRACER) track();
    EXACT=FALSE;
    n=0.0;
    d=0.0;
    b=(double)base;
    numer(w,w1);
    s=exsign(w1);
    insign(PLUS,w1);
    en=w1[0];
    for (i=1;i<=en;i++)
        n=n/b+w1[i];
    denom(w,w1);
    ed=w1[0];
    for (i=1;i<=ed;i++)
        d=d/b+w1[i];
    n=(s*(n/d));
    while (en!=ed)
    {
        if (en>ed)
        {
            ed++;
            if (BIGGEST/b<n)
            {
                berror(16);
                depth--;
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
                depth--;
                return (0.0);
            }
            n/=b;
        }
    }
    depth--;
    return n;
}
