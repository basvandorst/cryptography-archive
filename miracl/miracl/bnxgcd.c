/*
 *   MIRACL Extended Greatest Common Divisor module.
 *   bnxgcd.c
 */

#include <stdio.h>
#include "miracl.h"
#define abs(x)  ((x)<0? (-(x)) : (x))

/* Access global variables */

extern int   depth;    /* error tracing ..*/
extern int   trace[];  /* .. mechanism    */
extern small base;     /* number base     */

extern big w1;       /* workspace variables  */
extern big w2;
extern big w3;
extern big w4;
extern big w5;
extern big w0;

int xgcd(x,y,xd,yd,z)
big x,y,xd,yd,z;
{ /* greatest common divisor by Euclids method  *
   * extended to also calculate xd and yd where *
   *      z = x.xd + y.yd = gcd(x,y)            *
   * if xd, yd not distinct, only xd calculated *
   * z only returned if distinct from xd and yd *
   * xd will always be positive, yd negative    */
    small q,r,a,b,c,d,m;
    long u,v,lq,lr;
    int s,n;
    bool last;
    big t;
    if (ERNUM) return 0;
    depth++;
    trace[depth]=30;
    if (TRACER) track();
    copy(x,w1);
    copy(y,w2);
    s=exsign(w1);
    insign(PLUS,w1);
    insign(PLUS,w2);
    copy(w1,w3);
    copy(w2,w4);
    convert((small)1,w3);
    zero(w4);
    last=FALSE;
    a=b=c=d=0;
    while (size(w2)!=0)
    {
        if (b==0)
        { /* update w1 and w2 */
            divide(w1,w2,w5);
            t=w1,w1=w2,w2=t;    /* swap(w1,w2) */
            multiply(w4,w5,w0);
            subtract(w3,w0,w3);
            t=w3,w3=w4,w4=t;    /* swap(xd,yd) */
        }
        else
        {
            premult(w1,c,w5);
            premult(w1,a,w1);
            premult(w2,b,w0);
            premult(w2,d,w2);
            add(w1,w0,w1);
            add(w2,w5,w2);
            premult(w3,c,w5);
            premult(w3,a,w3);
            premult(w4,b,w0);
            premult(w4,d,w4);
            add(w3,w0,w3);
            add(w4,w5,w4);
        }
        if (ERNUM || size(w2)==0) break;
        n=w1[0];
        a=1;
        b=0;
        c=0;
        d=1;
        if (n==1)
        {
            last=TRUE;
            u=w1[1];
            v=w2[1];
        }
        else
        {
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
                    last=TRUE;
                }
            }
            else
            {
                u=muldiv(w1[n],base,w1[n-1],m,&r);
                v=muldiv(w2[n],base,w2[n-1],m,&r);
            }
        }
        forever
        { /* work only with most significant piece */
            if (last)
            {
                if (v==0) break;
                lq=u/v;
            }
            else
            {
                if (((v+c)==0) || ((v+d)==0)) break;
                lq=(u+a)/(v+c);
                if (lq!=(u+b)/(v+d)) break;
            }
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
    if (s==MINUS) negate(w3,w3);
    if (exsign(w3)==MINUS) add(w3,y,w3);
    if (xd!=yd)
    {
        negate(x,w2);
        mad(w2,w3,w1,y,w4,w4);
        copy(w4,yd);
    }
    copy(w3,xd);
    if (z!=xd && z!=yd) copy(w1,z);
    depth--;
    return (size(w1));
}

