/*
 *   MIRACL Greatest Common Divisor module.
 *   bngcd.c
 */

#include <stdio.h>
#include "miracl.h"
#define abs(x)  ((x)<0? (-(x)) : (x))

/* Access global variables */

extern int   depth;    /* error tracing ..*/
extern int   trace[];  /* .. mechanism    */
extern bool  check;    /* overflow check  */
extern small base;     /* number base     */

extern big w1;       /* workspace variables */
extern big w2;
extern big w0;

int gcd(x,y,z)
big x;
big y;
big z;
{ /* greatest common divisor z=gcd(x,y) by Euclids  *
   * method using Lehmers algorithm for big numbers */
    small q,r,a,b,c,d,m;
    long u,v,lq,lr;
    int n;
    big t;
    if (ERNUM) return 0;
    depth++;
    trace[depth]=12;
    if (TRACER) track();
    copy(x,w1);
    copy(y,w2);
    insign(PLUS,w1);
    insign(PLUS,w2);
    a=b=c=d=0;
    while (size(w2)!=0)
    {
        if (b==0)
        { /* update w1 and w2 */
            divide(w1,w2,w2);
            t=w1,w1=w2,w2=t;   /* swap(w1,w2) */
        }
        else
        {
            premult(w1,c,z);
            premult(w1,a,w1);
            premult(w2,b,w0);
            premult(w2,d,w2);
            add(w1,w0,w1);
            add(w2,z,w2);
        }
        if (ERNUM || size(w2)==0) break;
        n=w1[0];
        if (w2[0]==1)
        { /* special case if w2 is now small */
            r=subdiv(w1,w2[1],w1);
            convert(igcd(w2[1],r),w1);
            break;
        }
        a=1;
        b=0;
        c=0;
        d=1;
        m=w1[n]+1;
        if (sizeof(long)==2*sizeof(small))
        { /* use longs if they are double length */
            if (n>2)
            { /* squeeze out as much significance as possible */
                u=muldiv(w1[n],base,w1[n-1],m,&r);
                u=u*base+muldiv(r,base,w1[n-2],m,&r);
                v=muldiv(w2[n],base,w2[n-1],m,&r);
                v=v*base+muldiv(r,base,w2[n-2],m,&r);
            }
            else
            {
                u=(long)base*w1[n]+w1[n-1];
                v=(long)base*w2[n]+w2[n-1];
            }
        }
        else
        {
            u=muldiv(w1[n],base,w1[n-1],m,&r);
            v=muldiv(w2[n],base,w2[n-1],m,&r);
        }
        forever
        { /* work only with most significant piece */
            if (((v+c)==0) || ((v+d)==0)) break;
            lq=(u+a)/(v+c);
            if (lq!=(u+b)/(v+d)) break;
            if (lq>=MAXBASE/abs(d)) break;
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
        }
    }
    copy(w1,z);
    depth--;
    return (size(w1));
}

