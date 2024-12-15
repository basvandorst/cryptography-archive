/*
 *   MIRACL arithmetic routines 3 - powers and roots
 *   bnarth3.c
 */

#include <stdio.h>
#include "miracl.h"
#define abs(x)  ((x)<0? (-(x)) : (x))

/* Access global variables */

extern int   depth;    /* error tracing ..*/
extern int   trace[];  /* .. mechanism    */
extern small base;     /* number base     */
extern int   lg2b;     /* bits in base    */
extern small base2;    /* 2^lg2b          */
extern bool  check;    /* overflow check  */

extern big w0;
extern big w1;        /* workspace variables */
extern big w2;
extern big w3;
extern big w4;
extern big w5;
extern big w6;

int logb2(x)
big x;
{ /* returns number of bits in x */
    int xl,lg2;
    if (ERNUM || size(x)==0) return 0;
    depth++;
    trace[depth]=49;
    if (TRACER) track();
    copy(x,w0);
    insign(PLUS,w0);
    lg2=0;
    xl=w0[0];
    if (base==base2)
    {
        lg2=lg2b*(xl-1);
        shift(w0,1-xl,w0);
    }
    else while (w0[0]>1)
    {
        subdiv(w0,base2,w0);
        lg2+=lg2b;
    }
    while (w0[1]>1)
    {
        lg2++;
        w0[1]/=2;
    }
    depth--;
    return lg2;
}

void expb2(x,n)
big x;
int n;
{ /* sets x=2^n */
    int i,r;
    if (ERNUM) return;
    convert(1,x);
    if (n==0) return;
    depth++;
    trace[depth]=50;
    if (TRACER) track();
    if (n<0)
    {
        berror(10);
        depth--;
        return;
    }
    r=n/lg2b;
    if (base==base2)
    {
        shift(x,r,x);
        x[x[0]]<<=(n%lg2b);
    }
    else
    {
        for (i=1;i<=r;i++)
            premult(x,base2,x);
        premult(x,((small)1<<(n%lg2b)),x);
    }
    depth--;
}   

void power(x,n,z,w)
big w;
big x;
int n;
big z;
{ /* raise big number to int power  w=x^n *
   * (mod z if z and w distinct)          */
    copy(x,w5);
    zero(w);
    if(ERNUM || size(w5)==0) return;
    convert(1,w);
    if (n==0) return;
    depth++;
    trace[depth]=17;
    if (TRACER) track();
    if (n<0)
    {
        berror(10);
        depth--;
        return;
    }
    if (w!=z) divide(w5,z,z);
    if (w==z) forever
    { /* "Russian peasant" exponentiation */
        if (n%2!=0) multiply(w,w5,w);
        n/=2;
        if (ERNUM || n==0) break;
        multiply(w5,w5,w5);
    }
    else forever
    {
        if (n%2!=0) mad(w,w5,w5,z,z,w);
        n/=2;
        if (ERNUM || n==0) break;
        mad(w5,w5,w5,z,z,w5);
    }
    depth--;
    return;
}

void powmod(x,y,z,w)
big w;
big x;
big y;
big z;
{  /*  calculates w=x^y mod z */
    if (ERNUM) return;
    copy(y,w1);
    copy(x,w2);
    zero(w);
    if (size(w2)==0) return;
    convert (1,w);
    if (size(w1)==0) return;
    depth++;
    trace[depth]=18;
    if (TRACER) track();
    if (size(w1)<0) berror(10);
    if (w==z)       berror(7) ;
    if (ERNUM)
    {
        depth--;
        return;
    }
    divide(w2,z,z);
    forever
    { /* "Russian peasant" exponentiation */
        if(subdiv(w1,2,w1)!=0)
            mad(w,w2,w2,z,z,w);
        if(ERNUM || size(w1)==0) break;
        mad(w2,w2,w2,z,z,w2);
    }
    depth--;
}

bool root(x,n,w)
big x,w;
int n;
{  /*  extract  lower approximation to nth root   *
    *  w=x^(1/n) returns TRUE for exact root      *
    *  using Newtons method                       */
    int sx,dif,s,p,d,lg2,lgx,rem;
    bool full;
    if (ERNUM) return FALSE;
    if (size(x)==0 || n==1)
    {
        copy(x,w);
        return TRUE;
    }
    depth++;
    trace[depth]=16;
    if (TRACER) track();
    if (n<1) berror(11);
    sx=exsign(x);
    if (n%2==0 && sx==MINUS) berror(9);
    if (ERNUM) 
    {
        depth--;
        return FALSE;
    }
    insign(PLUS,x);
    lgx=logb2(x);
    if (n>lgx)
    { /* root must be 1 */
        insign(sx,x);
        convert(sx,w);
        depth--;
        if (lgx==0) return TRUE;
        else        return FALSE;
    }
    expb2(w2,1+lgx/n);           /* guess root as 2^(log2(x)/n) */
    s=(-((x[0]-1)/n)*n);
    shift(w2,s/n,w2);
    lg2=logb2(w2);
    full=FALSE;
    if (s==0) full=TRUE;
    d=0;
    p=1;
    while (!ERNUM)
    { /* Newtons method */
        copy(w2,w3);
        shift(x,s,w4);
        check=OFF;
        power(w2,n-1,w6,w6);
        check=ON;
        divide(w4,w6,w2);
        rem=size(w4);
        subtract(w2,w3,w2);
        dif=size(w2);
        subdiv(w2,n,w2);
        add(w2,w3,w2);
        p*=2;
        if(p<lg2+d*lg2b) continue;
        if (full && abs(dif)<n)
        { /* test for finished */
            while (dif<0)
            {
                rem=0;
                decr(w2,1,w2);
                check=OFF;
                power(w2,n,w6,w6);
                check=ON;
                dif=compare(x,w6);
            }
            copy(w2,w);
            insign(sx,w);
            insign(sx,x);
            depth--;
            if (rem==0 && dif==0) return TRUE;
            else                  return FALSE;
        }
        else
        { /* adjust precision */
            d*=2;
            if (d==0) d=1;
            s+=d*n;
            if (s>=0)
            {
                d-=s/n;
                s=0;
                full=TRUE;
            }
            shift(w2,d,w2);
        }
        p/=2;
    }
    depth--;
    return FALSE;
}
