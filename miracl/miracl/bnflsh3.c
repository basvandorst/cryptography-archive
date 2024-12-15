/*
 *  MIRACL flash trig.
 *  bnflsh3.c
 */

#include <stdio.h>
#include <math.h>
#include "miracl.h"
#define TAN 1
#define SIN 2
#define COS 3

/* Access Global variables */

extern int  depth;    /* error tracing... */
extern int  trace[];  /* .. mechanism     */
extern int  nib;      /* length of bigs   */
extern int  workprec;
extern int  stprec;   /* start precision  */
extern int  lg2b;     /* no. bits in base */

extern big  w8,w9,w10,w11; /* workspace variables */
extern big  w12,w13,w14,w15;
extern big  w5,w6;

int norm(type,y)
flash y;
int type;
{ /* convert y to first quadrant angle *
   * and return sign of result         */
    int s;
    if (ERNUM) return 0;
    s=PLUS;
    if (size(y)<0)
    {
        negate(y,y);
        if (type!=COS) s=(-s);
    }
    fpi(w15);
    fpmul(w15,1,2,w8);
    if (fcomp(y,w8)<=0) return s;
    fpmul(w15,2,1,w8);
    if (fcomp(y,w8)>0)
    { /* reduce mod 2.pi */
        fdiv(y,w8,w9);
        ftrunc(w9,w9,w9);
        fmul(w9,w8,w9);
        fsub(y,w9,y);
    }
    if (fcomp(y,w15)>0)
    { /* if greater than pi */
        fsub(y,w15,y);
        if (type!=TAN) s=(-s);
    }
    fpmul(w15,1,2,w8);
    if (fcomp(y,w8)>0)
    { /* if greater than pi/2 */
        fsub(w15,y,y);
        if (type!=SIN) s=(-s);
    }
    return s;
}

int tan1(w,n)
big w;
int n;
{  /* generator for C.F. of tan(1) */ 
    if (n==0) return 1;
    if (n%2==1) return 2*(n/2)+1;
    else return 1;
}

void ftan(x,y)
flash x,y;
{ /* calculates y=tan(x) */
    int i,n,nsq,m,sqrn,sgn,op[5];
    copy(x,y);
    if (ERNUM || size(y)==0) return;
    depth++;
    trace[depth]=57;
    if (TRACER) track();
    sgn=norm(TAN,y);
    ftrunc(y,y,w10);
    n=size(y);
    if (n!=0) build(y,tan1);
    if (size(w10)==0)
    {
        insign(sgn,y);
        depth--;
        return;
    }
    sqrn=isqrt(lg2b*workprec,lg2b);
    nsq=0;
    copy(w10,w8);
    frecip(w10,w10);
    ftrunc(w10,w10,w10);
    m=logb2(w10);
    if (m<sqrn)
    { /* scale fraction down */
        nsq=sqrn-m;
        expb2(w10,nsq);
        fdiv(w8,w10,w8);
    }
    zero(w10);
    fmul(w8,w8,w9);
    negate(w9,w9);
    op[0]=0x4B;    /* set up for x/(y+C) */
    op[1]=op[3]=1;
    op[2]=0;
    for (m=sqrn;m>1;m--)
    { /* Unwind C.F for tan(x)=z/(1-z^2/(3-z^2/(5-...)))))) */
        op[4]=2*m-1;
        flop(w9,w10,op,w10);
    }
    op[4]=1;
    flop(w8,w10,op,w10);
    op[0]=0x6C;     /* set up for tan(x+y)=tan(x)+tan(y)/(1-tan(x).tan(y)) */
    op[1]=op[2]=op[3]=1;
    op[4]=(-1);
    for (i=0;i<nsq;i++)
    { /* now square it back up again */
        flop(w10,w10,op,w10);
    }
    flop(y,w10,op,y);
    insign(sgn,y);
    depth--;
}

void fatan(x,y)
flash x,y;
{ /* calculate y=atan(x) */
    int s,c,op[5];
    bool inv,hack;
    copy(x,y);
    if (ERNUM || size(y)==0) return;
    depth++;
    trace[depth]=58;
    if (TRACER) track();
    s=exsign(y);
    insign(PLUS,y);
    inv=FALSE;
    fpi(w15);
    fconv(1,1,w11);
    c=fcomp(y,w11);
    if (c==0)
    { /* atan(1)=pi/4 */
        fpmul(w15,1,4,y);
        insign(s,y);
        depth--;
        return;
    }
    if (c>0)
    { /* atan(x)=pi/2 - atan(1/x) for x>1 */
        inv=TRUE;
        frecip(y,y);
    }
    hack=FALSE;
    if (lent(y)<=2)
    { /* for 'simple' values of y */
        hack=TRUE;
        fconv(3,1,w11);
        froot(w11,2,w11);
        op[0]=0xC6;
        op[2]=op[3]=op[4]=1;
        op[1]=(-1);
        flop(y,w11,op,y);
    }
    op[0]=0x4B;
    op[1]=op[3]=op[4]=1;
    op[2]=0;
    workprec=stprec;
    dconv(atan(fdsize(y)),w11);
    while (workprec!=nib)
    { /* Newtons iteration w11=w11+(y-tan(w11))/(tan(w11)^2+1) */
        if (workprec<nib) workprec*=2;
        if (workprec>=nib) workprec=nib;
        else if (workprec*2>nib) workprec=(nib+1)/2;
        ftan(w11,w12);
        fsub(y,w12,w8);
        fmul(w12,w12,w12);
        flop(w8,w12,op,w12);  /* w12=w8/(w12+1) */
        fadd(w12,w11,w11);
    }
    copy(w11,y);
    op[0]=0x6C;
    op[1]=op[3]=6;
    op[2]=1;
    op[4]=0;
    if (hack) flop(y,w15,op,y);
    op[1]=(-2);
    op[3]=2;
    if (inv) flop(y,w15,op,y);
    insign(s,y);
    depth--;
}

void fsin(x,y)
flash x,y;
{ /*  calculate y=sin(x) */
    int sgn,op[5];
    copy(x,y);
    if (ERNUM || size(y)==0) return;
    depth++;
    trace[depth]=59;
    if (TRACER) track();
    sgn=norm(SIN,y);
    fpmul(y,1,2,y);
    ftan(y,y);
    op[0]=0x6C;
    op[1]=op[2]=op[3]=op[4]=1;
    flop(y,y,op,y);
    insign(sgn,y);
    depth--;
}

void fasin(x,y)
flash x,y;
{ /* calculate y=asin(x) */
    int s,op[5];
    copy(x,y);
    if (ERNUM || size(y)==0) return;
    depth++;
    trace[depth]=60;
    if (TRACER) track();
    s=exsign(y);
    insign(PLUS,y);
    op[0]=0x3C;
    op[1]=(-1);
    op[2]=op[3]=1;
    op[4]=0;
    flop(y,y,op,w11);  /* w11 = -(y.y-1) */
    froot(w11,2,w11);
    if (size(w11)==0)
    {
        fpi(w15);
        fpmul(w15,1,2,y);
    }
    else
    {
        fdiv(y,w11,y);
        fatan(y,y);
    }
    insign(s,y);    
    depth--;
}

void fcos(x,y)
flash x,y;
{ /* calculate y=cos(x) */
    int sgn,op[5];
    copy(x,y);
    if (ERNUM || size(y)==0)
    {
        convert(1,y);
        return;
    }
    depth++;
    trace[depth]=61;
    if (TRACER) track();
    sgn=norm(COS,y);
    fpmul(y,1,2,y);
    ftan(y,y);
    op[0]=0x33;
    op[1]=op[3]=op[4]=1;
    op[2]=(-1);
    flop(y,y,op,y);
    insign(sgn,y);
    depth--;
}

void facos(x,y)
flash x,y;
{ /* calculate y=acos(x) */
    int op[5];
    if (ERNUM) return;
    depth++;
    trace[depth]=62;
    if (TRACER) track();
    fasin(x,y);
    fpi(w15);
    op[0]=0x6C;
    op[1]=(-2);
    op[2]=1;
    op[3]=2;
    op[4]=0;
    flop(y,w15,op,y);    /* y = pi/2 - y */
    depth--;
}
