/*
 *   MIRACL arithmetic routines 2 - multiplying and dividing BIG NUMBERS.
 *   bnarth2.c
 */

#include <stdio.h>
#include "miracl.h"
#define abs(x)  ((x)<0? (-(x)) : (x))
#define sign(x) ((x)<0? (-1) : 1)

/* Access global variables */

extern small base;    /* number base     */
extern int  nib;      /* length of bigs  */
extern int  depth;    /* error tracing ..*/
extern int  trace[];  /* .. mechanism    */
extern big  w0;       /* workspace big   */
extern bool check;    /* error checking  */

void multiply(x,y,z)
big x;
big y;
big z;
{  /*  multiply two big numbers: z=x.y  */
    int i,sz,xl,yl,j,ti;
    small carry;
    if (ERNUM) return;
    if (size(y)==0 || size(x)==0) 
    {
        zero(z);
        return;
    }
    depth++;
    trace[depth]=5;
    if (TRACER) track();
    if (notint(x) || notint(y))
    {
        berror(12);
        depth--;
        return;
    }
    sz=sign(x[0])*sign(y[0]);
    xl=abs(x[0]);
    yl=abs(y[0]);
    if (yl==1)
    {
        premult(x,y[1],z);
        insign(sz,z);
        depth--;
        return;
    }
    zero(w0);
    if (xl+yl>nib && (check || (xl+yl)>2*nib))
    {
        berror(3);
        depth--;
        return;
    }
    if (x==y)
    { /* squaring can be done nearly twice as fast */
        for (i=1;i<xl;i++)
        { /* long multiplication */
            carry=0;
            for (j=i+1;j<=xl;j++)
            { /* Only do above the diagonal */
                carry=muldiv(x[i],x[j],w0[i+j-1]+carry,base,&w0[i+j-1]);
            }
            w0[xl+i]=carry;
        }
        w0[0]=xl+xl-1;
        padd(w0,w0,w0);     /* double it */
        carry=0;
        for (i=1;i<=xl;i++)
        { /* add in squared elements */
            ti=i+i;
            carry=muldiv(x[i],x[i],w0[ti-1]+carry,base,&w0[ti-1]);
            w0[ti]+=carry;
            carry=0;
            if (w0[ti]>=base)
            {
                carry=1;
                w0[ti]-=base;
            }
        }
    }
    else for (i=1;i<=xl;i++)
    { /* long multiplication */
        carry=0;
        for (j=1;j<=yl;j++)
        { /* multiply each digit of y by x[i] */
            carry=muldiv(x[i],y[j],w0[i+j-1]+carry,base,&w0[i+j-1]);
        }
        w0[yl+i]=carry;
    }
    w0[0]=(xl+yl)*sz; /* set length and sign of result */
    lzero(w0);
    copy(w0,z);
    depth--;
}

void divide(x,y,z)
big x;
big y;
big z;
{  /*  divide two big numbers  z=x/y : x=x mod y  *
    *  returns quotient only if  divide(x,y,x)    *
    *  returns remainder only if divide(x,y,y)    */
    small carry,borrow,try,r,ldy,ra,d,tst,dig,pdiff,psum;
    int i,k,m,sz,sx,sy,x0,y0,w00;
    if (ERNUM) return;
    depth++;
    trace[depth]=6;
    if (TRACER) track();
    if (x==y) berror(7);
    if (notint(x) || notint(y)) berror(12);
    if (size(y)==0) berror(2);
    if (ERNUM)
    {
        depth--;
        return;
    }
    sx=sign(x[0]);    /* extract signs ... */
    sy=sign(y[0]);
    sz=sx*sy;
    x[0]=abs(x[0]);  /* ... and force operands to positive */
    y[0]=abs(y[0]);
    x0=x[0];
    y0=y[0];
    copy(x,w0);
    w00=w0[0];
    if (check && (w00-y0+1>nib))
    {
        berror(3);
        depth--;
        return;
    }
    d=0;
    if (x0==y0)
    {
        if (x0==1) /* special case - x and y are both ints */
        { 
            d=w0[1]/y[1];
            w0[1]%=y[1];
            lzero(w0);
        }
        else if (w0[x0]/4<y[x0])
        while (compare(w0,y)>=0)
        {  /* small quotient - so do up to four subtracts instead */
            psub(w0,y,w0);
            d++;
        }
    }
    if (compare(w0,y)<0)
    {  /*  x less than y - so x becomes remainder */
        if (x!=z)  /* testing parameters */
        {
            copy(w0,x);
            insign(sx,x);
        }
        if (y!=z) convert(sz*d,z);
        insign(sy,y);
        depth--;
        return;
    }
    if (y0==1)
    {  /* y is small - so use subdiv instead */
        r=subdiv(w0,y[1],w0);
        if (y!=z)
        {
            copy(w0,z);
            insign(sz,z);
        }
        if (x!=z)
        {
            convert(r,x);
            insign(sx,x);
        }
        insign(sy,y);
        depth--;
        return;
    }
    if (y!=z) zero(z);
    d=base/(y[y0]+1);   /* have to do it the hard way */
    premult(w0,d,w0);
    premult(y,d,y);
    ldy=y[y0];
    for (k=w00;k>=y0;k--)
    {  /* long division */
        if (w0[k+1]==ldy) /* guess next quotient digit */
        {
            try=base-1;
            ra=ldy+w0[k];
        }
        else try=muldiv(w0[k+1],base,w0[k],ldy,&ra);
        while (ra<base)
        {
            tst=muldiv(y[y0-1],try,(small)0,base,&r); 
            if (tst< ra || (tst==ra && r<=w0[k-1])) break;
            try--;  /* refine guess */
            ra+=ldy;
        }
        m=k-y0;
        if (try>0)
        { /* do partial subtraction */
            borrow=0;
            for (i=1;i<=y0;i++)
            {
              borrow=muldiv(try,y[i],borrow,base,&dig);
              pdiff=w0[m+i]-dig;
              if (pdiff<0)
              { /* set borrow */
                  borrow++;
                  pdiff+=base;
              }
              w0[m+i]=pdiff;
            }
            if (w0[k+1]<borrow)
            {  /* whoops! - over did it */
                w0[k+1]=0;
                carry=0;
                for (i=1;i<=y0;i++)
                {  /* compensate for error ... */
                    psum=w0[m+i]+y[i]+carry;
                    carry=0;
                    if (psum>=base)
                    {
                        carry=1;
                        psum-=base;
                    }
                    w0[m+i]=psum;
                }
                try--;  /* ... and adjust guess */
            }
            else w0[k+1]-=borrow;
        }
        if (k==w00 && try==0) w00--;
        else if (y!=z) z[m+1]=try;
    }
    if (y!=z) z[0]=(w00-y0+1)*sz; /* set sign and length of result */
    w0[0]=y0;
    lzero(y);
    lzero(z);
    if (x!=z)
    {
        lzero(w0);
        subdiv(w0,d,x);
        insign(sx,x);
    }
    subdiv(y,d,y);
    insign(sy,y);
    depth--;
}

void mad(x,y,z,w,q,r)
big x,y,z,w,q,r;
{ /* Multiply, Add and Divide; q=(x*y+z)/w remainder r   *
   * returns remainder only if w=q, quotient only if q=r *
   * add done only if x, y and z are distinct.           */
    if (ERNUM) return;
    depth++;
    trace[depth]=24;
    if (TRACER) track();
    check=OFF;           /* turn off some error checks */
    if (w==r)
    {
        berror(7);
        depth--;
        return;
    }
    multiply(x,y,w0);
    if (x!=z && y!=z)add(w0,z,w0);
    divide(w0,w,q);
    if (q!=r) copy(w0,r);
    check=ON;
    depth--;
}
