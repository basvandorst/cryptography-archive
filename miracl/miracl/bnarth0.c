/*
 *   MIRACL arithmetic routines 0 - Add and subtract routines 
 *   bnarth0.c
 */

#include <stdio.h>
#include "miracl.h"
#define sign(x) ((x)<0? (-1) : 1)

/* Access global variables */

extern small base;   /* number base      */  
extern int nib;      /* length of bigs   */
extern int depth;    /* error tracing .. */
extern int trace[];  /* ... mechanism    */
extern big w0;       /* workspace big    */
extern bool check;   /* overflow check   */

void padd(x,y,z)
big x;
big y;
big z;
{ /*  add two  big numbers, z=x+y where   *
   *  x and y are positive and  x>=y      */
    int lx,ly,i;
    small carry,psum;
    lx = x[0];
    ly = y[0];
    if (y!=z) copy(x,z);
    else ly=lx;
    carry=0;
    z[0]=lx;
    if (lx<nib || (!check && lx<2*nib)) z[0]++;
    for (i=1;i<=ly || carry>0;i++)
    { /* add by columns */
        if (i>nib && check)
        {
            berror(3);
            return;
        }
        psum=x[i]+y[i]+carry;
        carry=0;
        if (psum>=base)
        { /* set carry */
            carry=1;
            psum-=base;
        }
        z[i]=psum;
    }
    if (z[z[0]]==0) z[0]--;
}

void psub(x,y,z)
big x;
big y;
big z;
{  /*  subtract two big numbers z=x-y      *
    *  where x and y are positive and x>y  */
    int lx,ly,i;
    small borrow,pdiff;
    lx = x[0];
    ly = y[0];
    if (ly>lx)
    {
        berror(4);
        return;
    }
    if (y!=z) copy(x,z);
    else ly=lx;
    z[0]=lx;
    borrow=0;
    for (i=1;i<=ly || borrow>0;i++)
    { /* subtract by columns */
        if (i>lx)
        {
            berror(4);
            return;
        }
        pdiff=x[i]-y[i]-borrow;
        borrow=0;
        if (pdiff<0)
        { /* set borrow */
            pdiff+=base;
            borrow=1;
        }
        z[i]=pdiff;
    }
    lzero(z);
}

void select(x,d,y,z)
big x;
int d;
big y;
big z;
{ /* perform required add or subtract operation */
    int sx,sy,sz,jf,xgty;
    if (notint(x) || notint(y))
    {
        berror(12);
        return;
    }
    sx=sign(x[0]);
    sy=sign(y[0]);
    if (x[0]<0) x[0]=(-x[0]);  /* force operands to be positive */
    if (y[0]<0) y[0]=(-y[0]);
    xgty=compare(x,y);
    jf=(1+sx)+(1+d*sy)/2;
    switch (jf)
    { /* branch according to signs of operands */
    case 0:
        if (xgty>=0)
            padd(x,y,z);
        else
            padd(y,x,z);
        sz=MINUS;
        break;
    case 1:
        if (xgty<=0)
        {
            psub(y,x,z);
            sz=PLUS;
        }
        else
        {
            psub(x,y,z);
            sz=MINUS;
        }
        break;
    case 2:
        if (xgty>=0)
        {
            psub(x,y,z);
            sz=PLUS;
        }
        else
        {
            psub(y,x,z);
            sz=MINUS;
        }
        break;
    case 3:
        if (xgty>=0)
            padd(x,y,z);
        else
            padd(y,x,z);
        sz=PLUS;
        break;
    }
    if (sz<0) z[0]=(-z[0]);         /* set sign of result         */
    if (x!=z && sx<0) x[0]=(-x[0]); /* restore signs to operands  */
    if (y!=z && y!=x && sy<0) y[0]=(-y[0]);
}

void add(x,y,z)
big x;
big y;
big z;
{  /* add two signed big numbers together z=x+y */
    if (ERNUM) return;
    depth++;
    trace[depth]=27;
    if (TRACER) track();
    select(x,PLUS,y,z);
    depth--;
}

void subtract(x,y,z)
big x;
big y;
big z;
{ /* subtract two big signed numbers z=x-y */
    if (ERNUM) return;
    depth++;
    trace[depth]=28;
    if (TRACER) track();
    select(x,MINUS,y,z);
    depth--;
}

void incr(x,n,z)
big x;
big z;
small n;
{  /* add int to big number: z=x+n */
    if (ERNUM) return;
    depth++;
    trace[depth]=7;
    if (TRACER) track();
    convert(n,w0);
    select(x,PLUS,w0,z);
    depth--;
}

void decr(x,n,z)
big x;
big z;
small n;
{  /* subtract int from big number: z=x-n */   
    if (ERNUM) return;
    depth++;
    trace[depth]=8;
    if (TRACER) track();
    convert(n,w0);
    select(x,MINUS,w0,z);
    depth--;
}
