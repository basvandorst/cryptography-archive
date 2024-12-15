/*
 *   MIRACL flash exponential and logs
 *   mrflsh2.c
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <math.h>
#include <miracl.h>

#ifdef MR_FLASH

#define abs(x)  ((x)<0? (-(x)) : (x))

/* Access Global variables */

extern int  mr_depth;    /* error tracing... */
extern int  mr_trace[];  /* .. mechanism     */
extern int  mr_nib;      /* length of bigs   */
extern int  mr_workprec;
extern int  mr_stprec;   /* start precision  */
extern int  mr_lg2b;     /* no. bits in base */

extern big  mr_w8,mr_w9,mr_w10,mr_w11; /* workspace variables */
extern big  mr_w12,mr_w13;
extern big  mr_w5,mr_w6;

static int expon(w,n)
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
    if (MR_ERNUM) return;
    if (size(x)==0)
    {
        convert(1,y);
        return;
    }
    copy(x,y);
    mr_depth++;
    mr_trace[mr_depth]=54;
    if (TRACER) track();
    minus=FALSE;
    if (size(y)<0)
    {
        minus=TRUE;
        negate(y,y);
    }
    ftrunc(y,y,mr_w9);
    n=size(y);
    if (n==TOOBIG)
    {
        berror(13);
        mr_depth--;
        return;
    }
    if (n==0) convert(1,y);
    else
    {
        build(y,expon);
        fpower(y,n,y);
    }
    if (size(mr_w9)==0)
    {
        if (minus) frecip(y,y);
        mr_depth--;
        return;
    }
    sqrn=isqrt(mr_lg2b*mr_workprec,mr_lg2b);
    nsq=0;
    copy(mr_w9,mr_w8);
    frecip(mr_w9,mr_w9);
    ftrunc(mr_w9,mr_w9,mr_w9);
    m=logb2(mr_w9);
    if (m<sqrn)
    { /* scale fraction down */
        nsq=sqrn-m;
        expb2(nsq,mr_w9);
        fdiv(mr_w8,mr_w9,mr_w8);
    }
    zero(mr_w10);
    op[0]=0x4B;  /* set up for x/(C+y) */
    op[1]=1;
    op[2]=0;
    for (m=sqrn;m>0;m--)
    { /* Unwind C.F. expansion for exp(x)-1 */
        if (m%2==0) op[4]=2,op[3]=1;
        else        op[4]=m,op[3]=(-1);
        flop(mr_w8,mr_w10,op,mr_w10);
    }
    op[0]=0x2C;  /* set up for (x+2).y */
    op[1]=op[3]=1;
    op[2]=2;
    op[4]=0;
    for (i=0;i<nsq;i++)
    { /* now square it back up again */
        flop(mr_w10,mr_w10,op,mr_w10);
    }
    op[2]=1;
    flop(mr_w10,y,op,y);
    if (minus) frecip(y,y);
    mr_depth--;
}

void flog(x,y)
flash x,y;
{ /* calculate y=log(x) to base e */
    bool hack;
    int op[5];
    copy(x,y);
    if (MR_ERNUM) return;
    if (size(y)==1)
    {
        zero(y);
        return;
    }
    mr_depth++;
    mr_trace[mr_depth]=55;
    if (TRACER) track();
    if (size(y)<=0)
    {
        berror(15);
        mr_depth--;
        return;
    }
    hack=FALSE;
    if (lent(y)<=2)
    { /* for 'simple' values of y */
        hack=TRUE;
        build(mr_w11,expon);
        fdiv(y,mr_w11,y);
    }
    op[0]=0x68;
    op[1]=op[3]=1;
    op[2]=(-1);
    op[4]=0;
    mr_workprec=mr_stprec;
    dconv(log(fdsize(y)),mr_w11);
    while (mr_workprec!=mr_nib)
    { /* Newtons iteration mr_w11=mr_w11+(y-exp(mr_w11))/exp(mr_w11) */
        if (mr_workprec<mr_nib) mr_workprec*=2;
        if (mr_workprec>=mr_nib) mr_workprec=mr_nib;
        else if (mr_workprec*2>mr_nib) mr_workprec=(mr_nib+1)/2;
        fexp(mr_w11,mr_w12);
        flop(y,mr_w12,op,mr_w12);
        fadd(mr_w12,mr_w11,mr_w11);
    }
    copy(mr_w11,y);
    if (hack) fincr(y,1,1,y);
    mr_depth--;
}
    
void fpowf(x,y,z)
flash x,y,z;
{ /* raise flash number to flash power *
   *     z=x^y  -> z=exp(y.log(x))     */
    int n;
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=56;
    if (TRACER) track();
    n=size(y);
    if (abs(n)<TOOBIG)
    { /* y is small int */
        fpower(x,n,z);
        mr_depth--;
        return;
    }
    frecip(y,mr_w13);
    n=size(mr_w13);
    if (abs(n)<TOOBIG)
    { /* 1/y is small int */
        froot(x,n,z);
        mr_depth--;
        return;
    }
    copy(x,z);
    flog(z,z);
    fdiv(z,mr_w13,z);
    fexp(z,z);
    mr_depth--;
}

#endif

