/*
 *   MIRACL arithmetic routines 1 - multiplying and dividing BIG NUMBERS by  
 *   integer numbers.
 *   bnarth1.c
 */

#include <stdio.h>
#include "miracl.h"
#define abs(x)  ((x)<0? (-(x)) : (x))
#define sign(x) ((x)<0? (-1) : 1)

/* access global variables */

extern small base;   /* number base     */
extern int nib;      /* length of bigs  */
extern int depth;    /* error tracing ..*/
extern int trace[];  /* .. mechanism    */
extern bool check;   /* overflow check  */

void premult(x,n,z)
big x;
big z;
small n;
{ /* premultiply a big number by an int z=x.n */
    int s,xl,m;
    small carry;
    if (ERNUM) return;
    depth++;
    trace[depth]=9;
    if (TRACER) track();
    if (notint(x))
    {
        berror(12);
        depth--;
        return;
    }
    if (n==0)  /* test for some special cases  */
    {
        zero(z);
        depth--;
        return;
    }
    if (x[0]>=0)
    {
        s=PLUS;
        xl=x[0];
    }
    else
    {
        s=MINUS;
        xl=(-x[0]);
    }
    if (n<0)
    {
        n=(-n);
        s=(-s);
    }
    if (n==1)
    {
        copy(x,z);
        insign(s,z);
        depth--;
        return;
    }
    if (x!=z) zero(z);
    m=0;
    carry=0;
    while (m<xl || carry>0)
    { /* multiply each digit of x by n */
        m++;
        if (m>nib && check)
        {
            berror(3);
            depth--;
            return;
        }
        carry=muldiv(x[m],n,carry,base,&z[m]);
        z[0]=m;
    }
    if (s<0) z[0]=(-z[0]);
    depth--;  
}
         
small subdiv(x,n,z)
big x;
big z;
small n;
{  /*  subdivide a big number by an int   z=x/n  *
    *  returns int remainder                     */ 
    int i,s,sx,xl;
    small r;
    if (ERNUM) return 0;
    depth++;
    trace[depth]=10;
    if (TRACER) track();
    if (notint(x)) berror(12);
    r=0;
    if (n==0) berror(2);
    if (ERNUM) 
    {
        depth--;
        return 0;
    }
    sx=sign(x[0]);
    s=sx*sign(n);  /* set sign of result */
    if (n<0) n=(-n);
    if (n==1) /* special case */
    {
        copy(x,z);
        insign(s,z);
        depth--;
        return r;
    }
    xl=abs(x[0]);
    if (x!=z) zero(z);
    for (i=xl;i>0;i--)
    { /* divide each digit of x by n */
        z[i]=muldiv(r,base,x[i],n,&r);
    }
    z[0]=xl;
    lzero(z);
    if (s<0) z[0]=(-z[0]);
    depth--;
    if (sx<0) return (-r);
    else      return r;
}

