/*
 *   MIRACL arithmetic routines 0 - Add and subtract routines 
 *   mrarth0.c
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <miracl.h>

/* Access global variables */

extern small MR_MSBIT;  /* msb of small     */
extern small MR_OBITS;  /* other bits       */
extern small mr_base;   /* number base      */  
extern int mr_nib;      /* length of bigs   */
extern int mr_depth;    /* error tracing .. */
extern int mr_trace[];  /* ... mechanism    */
extern big mr_w0;       /* workspace big    */
extern bool mr_check;   /* overflow check   */

void padd(x,y,z)
big x;
big y;
big z;
{ /*  add two  big numbers, z=x+y where   *
   *  x and y are positive and  x>=y      */
    int lx,ly,i;
    small carry,psum;
    lx = (int)x[0];
    ly = (int)y[0];
    if (y!=z) copy(x,z);
    else ly=lx;
    carry=0;
    z[0]=lx;
    if (lx<mr_nib || !mr_check) z[0]++;
    if (mr_base==0) for (i=1;i<=ly || carry>0;i++)
    { /* add by columns */
        if (i>mr_nib && mr_check)
        {
            berror(3);
            return;
        }
        psum=x[i]+y[i]+carry;
        if (psum>x[i]) carry=0;
        if (psum<x[i]) carry=1;
        z[i]=psum;
    }
    else for (i=1;i<=ly || carry>0;i++)
    { /* add by columns */
        if (i>mr_nib && mr_check)
        {
            berror(3);
            return;
        }
        psum=x[i]+y[i]+carry;
        carry=0;
        if (psum>=mr_base)
        { /* set carry */
            carry=1;
            psum-=mr_base;
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
    lx = (int)x[0];
    ly = (int)y[0];
    if (ly>lx)
    {
        berror(4);
        return;
    }
    if (y!=z) copy(x,z);
    else ly=lx;
    z[0]=lx;
    borrow=0;
    if (mr_base==0) for (i=1;i<=ly || borrow>0;i++)
    { /* subtract by columns */
        if (i>lx)
        {
            berror(4);
            return;
        }
        pdiff=x[i]-y[i]-borrow;
        if (pdiff<x[i]) borrow=0;
        if (pdiff>x[i]) borrow=1;
        z[i]=pdiff;
    }
    else for (i=1;i<=ly || borrow>0;i++)
    { /* subtract by columns */
        if (i>lx)
        {
            berror(4);
            return;
        }
        pdiff=y[i]+borrow;
        borrow=0;
        if (x[i]>=pdiff) pdiff=x[i]-pdiff;
        else
        { /* set borrow */
            pdiff=mr_base+x[i]-pdiff;
            borrow=1;
        }
        z[i]=pdiff;
    }
    lzero(z);
}

static void select(x,d,y,z)
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
    sx=exsign(x);
    sy=exsign(y);
    x[0]&=MR_OBITS;  /* force operands to be positive */
    y[0]&=MR_OBITS;
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
    if (sz<0) z[0]^=MR_MSBIT;         /* set sign of result         */
    if (x!=z && sx<0) x[0]^=MR_MSBIT; /* restore signs to operands  */
    if (y!=z && y!=x && sy<0) y[0]^=MR_MSBIT;
}

void add(x,y,z)
big x;
big y;
big z;
{  /* add two signed big numbers together z=x+y */
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=27;
    if (TRACER) track();
    select(x,PLUS,y,z);
    mr_depth--;
}

void subtract(x,y,z)
big x;
big y;
big z;
{ /* subtract two big signed numbers z=x-y */
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=28;
    if (TRACER) track();
    select(x,MINUS,y,z);
    mr_depth--;
}

void incr(x,n,z)
big x;
big z;
int n;
{  /* add int to big number: z=x+n */
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=7;
    if (TRACER) track();
    convert(n,mr_w0);
    select(x,PLUS,mr_w0,z);
    mr_depth--;
}

void decr(x,n,z)
big x;
big z;
int n;
{  /* subtract int from big number: z=x-n */   
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=8;
    if (TRACER) track();
    convert(n,mr_w0);
    select(x,MINUS,mr_w0,z);
    mr_depth--;
}

