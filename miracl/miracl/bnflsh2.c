/*
 *  MIRACL flash exponential and logs
 *  bnflsh2.c
 */

#include <stdio.h>
#include <math.h>
#include "miracl.h"
#define abs(x)  ((x)<0? (-(x)) : (x))

/* Access Global variables */

extern int  depth;    /* error tracing... */
extern int  trace[];  /* .. mechanism     */
extern int  nib;      /* length of bigs   */
extern int  workprec;
extern int  stprec;   /* start precision  */
extern int  lg2b;     /* no. bits in base */

extern big  w8,w9,w10,w11; /* workspace variables */
extern big  w12,w13;
extern big  w5,w6;

int expon(w,n)
big w;
int n;
{  /* generator for C.F. of e */ 
    if (n==0) return 2;
    if (n%3==2) return 2*(n/3)+2;
    else return 1;
}
 
void fexp(x,y)
flash x,y;
{ /* calculates y=exp(x) */
    int i,n,nsq,m,sqrn,op[5];
    bool minus;
    if (ERNUM) return;
    if (size(x)==0)
    {
        convert(1,y);
        return;
    }
    copy(x,y);
    depth++;
    trace[depth]=54;
    if (TRACER) track();
    minus=FALSE;
    if (size(y)<0)
    {
        minus=TRUE;
        negate(y,y);
    }
    ftrunc(y,y,w9);
    n=size(y);
    if (n==TOOBIG)
    {
        berror(13);
        depth--;
        return;
    }
    if (n==0) convert(1,y);
    else
    {
        build(y,expon);
        fpower(y,n,y);
    }
    if (size(w9)==0)
    {
        if (minus) frecip(y,y);
        depth--;
        return;
    }
    sqrn=isqrt(lg2b*workprec,lg2b);
    nsq=0;
    copy(w9,w8);
    frecip(w9,w9);
    ftrunc(w9,w9,w9);
    m=logb2(w9)+1;
    if (m<sqrn)
    { /* scale fraction down */
        nsq=sqrn-m;
        expb2(w9,nsq);
        fdiv(w8,w9,w8);
    }
    zero(w10);
    op[0]=0x4B;  /* set up for x/(C+y) */
    op[1]=1;
    op[2]=0;
    for (m=sqrn;m>0;m--)
    { /* Unwind C.F. expansion for exp(x)-1 */
        if (m%2==0) op[4]=2,op[3]=1;
        else        op[4]=m,op[3]=(-1);
        flop(w8,w10,op,w10);
    }
    op[0]=0x2C;  /* set up for (x+2).y */
    op[1]=op[3]=1;
    op[2]=2;
    op[4]=0;
    for (i=0;i<nsq;i++)
    { /* now square it back up again */
        flop(w10,w10,op,w10);
    }
    op[2]=1;
    flop(w10,y,op,y);
    if (minus) frecip(y,y);
    depth--;
}

void flog(x,y)
flash x,y;
{ /* calculate y=log(x) to base e */
    bool hack;
    int op[5];
    copy(x,y);
    if (ERNUM) return;
    if (size(y)==1)
    {
        zero(y);
        return;
    }
    depth++;
    trace[depth]=55;
    if (TRACER) track();
    if (size(y)<=0)
    {
        berror(15);
        depth--;
        return;
    }
    hack=FALSE;
    if (lent(y)<=2)
    { /* for 'simple' values of y */
        hack=TRUE;
        build(w11,expon);
        fdiv(y,w11,y);
    }
    op[0]=0x68;
    op[1]=op[3]=1;
    op[2]=(-1);
    op[4]=0;
    workprec=stprec;
    dconv(log(fdsize(y)),w11);
    while (workprec!=nib)
    { /* Newtons iteration w11=w11+(y-exp(w11))/exp(w11) */
        if (workprec<nib) workprec*=2;
        if (workprec>=nib) workprec=nib;
        else if (workprec*2>nib) workprec=(nib+1)/2;
        fexp(w11,w12);
        flop(y,w12,op,w12);
        fadd(w12,w11,w11);
    }
    copy(w11,y);
    if (hack) fincr(y,1,1,y);
    depth--;
}
    
void fpowf(x,y,z)
flash x,y,z;
{ /* raise flash number to flash power *
   *     z=x^y  -> z=exp(y.log(x))     */
    int n;
    if (ERNUM) return;
    depth++;
    trace[depth]=56;
    if (TRACER) track();
    n=size(y);
    if (abs(n)<TOOBIG)
    { /* y is small int */
        fpower(x,n,z);
        depth--;
        return;
    }
    frecip(y,w13);
    n=size(w13);
    if (abs(n)<TOOBIG)
    { /* 1/y is small int */
        froot(x,n,z);
        depth--;
        return;
    }
    copy(x,z);
    flog(z,z);
    fdiv(z,w13,z);
    fexp(z,z);
    depth--;
}
