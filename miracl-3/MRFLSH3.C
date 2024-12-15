/*
 *   MIRACL flash trig.
 *   mrflsh3.c
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <math.h>
#include <miracl.h>

#ifdef MR_FLASH

#define TAN 1
#define SIN 2
#define COS 3

/* Access Global variables */

extern int  mr_depth;    /* error tracing... */
extern int  mr_trace[];  /* .. mechanism     */
extern int  mr_nib;      /* length of bigs   */
extern int  mr_workprec;
extern int  mr_stprec;   /* start precision  */
extern int  mr_lg2b;     /* no. bits in base */

extern big  mr_w8,mr_w9,mr_w10,mr_w11; /* workspace variables */
extern big  mr_w12,mr_w15;
extern big  mr_w5,mr_w6;

static int norm(type,y)
flash y;
int type;
{ /* convert y to first quadrant angle *
   * and return sign of result         */
    int s;
    if (MR_ERNUM) return 0;
    s=PLUS;
    if (size(y)<0)
    {
        negate(y,y);
        if (type!=COS) s=(-s);
    }
    fpi(mr_w15);
    fpmul(mr_w15,1,2,mr_w8);
    if (fcomp(y,mr_w8)<=0) return s;
    fpmul(mr_w15,2,1,mr_w8);
    if (fcomp(y,mr_w8)>0)
    { /* reduce mod 2.pi */
        fdiv(y,mr_w8,mr_w9);
        ftrunc(mr_w9,mr_w9,mr_w9);
        fmul(mr_w9,mr_w8,mr_w9);
        fsub(y,mr_w9,y);
    }
    if (fcomp(y,mr_w15)>0)
    { /* if greater than pi */
        fsub(y,mr_w15,y);
        if (type!=TAN) s=(-s);
    }
    fpmul(mr_w15,1,2,mr_w8);
    if (fcomp(y,mr_w8)>0)
    { /* if greater than pi/2 */
        fsub(mr_w15,y,y);
        if (type!=SIN) s=(-s);
    }
    return s;
}

static int tan1(w,n)
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
    if (MR_ERNUM || size(y)==0) return;
    mr_depth++;
    mr_trace[mr_depth]=57;
    if (TRACER) track();
    sgn=norm(TAN,y);
    ftrunc(y,y,mr_w10);
    n=size(y);
    if (n!=0) build(y,tan1);
    if (size(mr_w10)==0)
    {
        insign(sgn,y);
        mr_depth--;
        return;
    }
    sqrn=isqrt(mr_lg2b*mr_workprec,mr_lg2b);
    nsq=0;
    copy(mr_w10,mr_w8);
    frecip(mr_w10,mr_w10);
    ftrunc(mr_w10,mr_w10,mr_w10);
    m=logb2(mr_w10);
    if (m<sqrn)
    { /* scale fraction down */
        nsq=sqrn-m;
        expb2(nsq,mr_w10);
        fdiv(mr_w8,mr_w10,mr_w8);
    }
    zero(mr_w10);
    fmul(mr_w8,mr_w8,mr_w9);
    negate(mr_w9,mr_w9);
    op[0]=0x4B;    /* set up for x/(y+C) */
    op[1]=op[3]=1;
    op[2]=0;
    for (m=sqrn;m>1;m--)
    { /* Unwind C.F for tan(x)=z/(1-z^2/(3-z^2/(5-...)))))) */
        op[4]=2*m-1;
        flop(mr_w9,mr_w10,op,mr_w10);
    }
    op[4]=1;
    flop(mr_w8,mr_w10,op,mr_w10);
    op[0]=0x6C;     /* set up for tan(x+y)=tan(x)+tan(y)/(1-tan(x).tan(y)) */
    op[1]=op[2]=op[3]=1;
    op[4]=(-1);
    for (i=0;i<nsq;i++)
    { /* now square it back up again */
        flop(mr_w10,mr_w10,op,mr_w10);
    }
    flop(y,mr_w10,op,y);
    insign(sgn,y);
    mr_depth--;
}

void fatan(x,y)
flash x,y;
{ /* calculate y=atan(x) */
    int s,c,op[5];
    bool inv,hack;
    copy(x,y);
    if (MR_ERNUM || size(y)==0) return;
    mr_depth++;
    mr_trace[mr_depth]=58;
    if (TRACER) track();
    s=exsign(y);
    insign(PLUS,y);
    inv=FALSE;
    fpi(mr_w15);
    fconv(1,1,mr_w11);
    c=fcomp(y,mr_w11);
    if (c==0)
    { /* atan(1)=pi/4 */
        fpmul(mr_w15,1,4,y);
        insign(s,y);
        mr_depth--;
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
        fconv(3,1,mr_w11);
        froot(mr_w11,2,mr_w11);
        op[0]=0xC6;
        op[2]=op[3]=op[4]=1;
        op[1]=(-1);
        flop(y,mr_w11,op,y);
    }
    op[0]=0x4B;
    op[1]=op[3]=op[4]=1;
    op[2]=0;
    mr_workprec=mr_stprec;
    dconv(atan(fdsize(y)),mr_w11);
    while (mr_workprec!=mr_nib)
    { /* Newtons iteration mr_w11=mr_w11+(y-tan(mr_w11))/(tan(mr_w11)^2+1) */
        if (mr_workprec<mr_nib) mr_workprec*=2;
        if (mr_workprec>=mr_nib) mr_workprec=mr_nib;
        else if (mr_workprec*2>mr_nib) mr_workprec=(mr_nib+1)/2;
        ftan(mr_w11,mr_w12);
        fsub(y,mr_w12,mr_w8);
        fmul(mr_w12,mr_w12,mr_w12);
        flop(mr_w8,mr_w12,op,mr_w12);  /* mr_w12=mr_w8/(mr_w12+1) */
        fadd(mr_w12,mr_w11,mr_w11);
    }
    copy(mr_w11,y);
    op[0]=0x6C;
    op[1]=op[3]=6;
    op[2]=1;
    op[4]=0;
    if (hack) flop(y,mr_w15,op,y);
    op[1]=(-2);
    op[3]=2;
    if (inv) flop(y,mr_w15,op,y);
    insign(s,y);
    mr_depth--;
}

void fsin(x,y)
flash x,y;
{ /*  calculate y=sin(x) */
    int sgn,op[5];
    copy(x,y);
    if (MR_ERNUM || size(y)==0) return;
    mr_depth++;
    mr_trace[mr_depth]=59;
    if (TRACER) track();
    sgn=norm(SIN,y);
    fpmul(y,1,2,y);
    ftan(y,y);
    op[0]=0x6C;
    op[1]=op[2]=op[3]=op[4]=1;
    flop(y,y,op,y);
    insign(sgn,y);
    mr_depth--;
}

void fasin(x,y)
flash x,y;
{ /* calculate y=asin(x) */
    int s,op[5];
    copy(x,y);
    if (MR_ERNUM || size(y)==0) return;
    mr_depth++;
    mr_trace[mr_depth]=60;
    if (TRACER) track();
    s=exsign(y);
    insign(PLUS,y);
    op[0]=0x3C;
    op[1]=(-1);
    op[2]=op[3]=1;
    op[4]=0;
    flop(y,y,op,mr_w11);  /* mr_w11 = -(y.y-1) */
    froot(mr_w11,2,mr_w11);
    if (size(mr_w11)==0)
    {
        fpi(mr_w15);
        fpmul(mr_w15,1,2,y);
    }
    else
    {
        fdiv(y,mr_w11,y);
        fatan(y,y);
    }
    insign(s,y);    
    mr_depth--;
}

void fcos(x,y)
flash x,y;
{ /* calculate y=cos(x) */
    int sgn,op[5];
    copy(x,y);
    if (MR_ERNUM || size(y)==0)
    {
        convert(1,y);
        return;
    }
    mr_depth++;
    mr_trace[mr_depth]=61;
    if (TRACER) track();
    sgn=norm(COS,y);
    fpmul(y,1,2,y);
    ftan(y,y);
    op[0]=0x33;
    op[1]=op[3]=op[4]=1;
    op[2]=(-1);
    flop(y,y,op,y);
    insign(sgn,y);
    mr_depth--;
}

void facos(x,y)
flash x,y;
{ /* calculate y=acos(x) */
    int op[5];
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=62;
    if (TRACER) track();
    fasin(x,y);
    fpi(mr_w15);
    op[0]=0x6C;
    op[1]=(-2);
    op[2]=1;
    op[3]=2;
    op[4]=0;
    flop(y,mr_w15,op,y);    /* y = pi/2 - y */
    mr_depth--;
}

#endif

