/*
 *   MIRACL flash roots and powers
 *   mrflsh1.c
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <math.h>
#include <miracl.h>

#ifdef MR_FLASH

/* Access global variables */

extern int mr_depth;     /* error tracing .. */
extern int mr_trace[];   /* ... mechanism    */
extern int mr_nib;       /* length of bigs   */
extern small mr_base;    /* number base      */
extern int mr_workprec;
extern int mr_stprec;    /* start precision  */

extern big mr_w7,mr_w8,mr_w9,mr_w10,mr_w15; /* workspace variables */

static int RS,RD;

static int quad(w,n)
big w;
int n;
{ /* generator for C.F. of square root of small integer */
    static int v,u,ud,vd,a,oldn;
    int t;
    if (n==0)
    {
        u=2*RD;
        ud=u;
        v=1;
        vd=RS;
        a=RD;
        if (a>=TOOBIG)
        {
            convert(a,w);
            return TOOBIG;
        }
        return a;
    }
    else if (n==oldn) return a;
    t=v;
    v=a*(ud-u)+vd;
    vd=t;
    a=u/v;
    ud=u;
    u=2*RD-u%v;
    oldn=n;
    if (a>=TOOBIG)
    {
        convert(a,w);
        return TOOBIG;
    }
    return a;
}

void fpower(x,n,w)
flash x,w;
int n;
{ /* raise floating-slash number to integer power w=x^n */
    copy(x,mr_w8);
    zero(w);
    if (MR_ERNUM || size(mr_w8)==0) return;
    convert(1,w);
    if (n==0) return;
    mr_depth++;
    mr_trace[mr_depth]=51;
    if (TRACER) track();
    if (n<0)
    {
        n=(-n);
        frecip(mr_w8,mr_w8);
    }
    if (n==1)
    {
        copy(mr_w8,w);
        mr_depth--;
        return;
    }
    forever
    {
        if (n%2!=0) fmul(w,mr_w8,w);
        n/=2;
        if (MR_ERNUM || n==0) break;
        fmul(mr_w8,mr_w8,mr_w8);
    }
    mr_depth--;
}
 
bool froot(x,n,w)
flash w,x;
int n;
{ /* extract nth root of x  - w=x^(1/n) using Newtons method */
    bool minus,rn,rm,hack;
    int nm,dn,s,op[5];
    copy(x,w);
    if (MR_ERNUM || n==1) return TRUE;
    if (n==(-1))
    {
        frecip(w,w);
        return TRUE;
    }
    mr_depth++;
    mr_trace[mr_depth]=52;
    if (TRACER) track();
    minus=FALSE;
    if (n<0)
    {
        minus=TRUE;
        n=(-n);
    }
    s=exsign(w);
    if (n%2==0 && s==MINUS)
    {
        berror(9);
        mr_depth--;
        return FALSE;
    }
    insign(PLUS,w);
    numer(w,mr_w8);
    denom(w,mr_w9);
    rn=nroot(mr_w8,n,mr_w8);
    rm=nroot(mr_w9,n,mr_w9);
    if (rn && rm)
    {
        fpack(mr_w8,mr_w9,w);
        if (minus) frecip(w,w);
        insign(s,w);
        mr_depth--;
        return TRUE;
    }
    nm=size(mr_w8);
    dn=size(mr_w9);
    if (n==2 && ((nm<TOOBIG) || rn) && ((dn<TOOBIG) || rm))
    {
        if (!rn && nm<TOOBIG)
        {
            multiply(mr_w8,mr_w8,mr_w8);
            numer(w,mr_w7);
            subtract(mr_w7,mr_w8,mr_w8);
            RS=(int)(mr_w8[1]+mr_base*mr_w8[2]);
            RD=nm;
            build(mr_w8,quad);
        }
        if (!rm && dn<TOOBIG)
        {
            multiply(mr_w9,mr_w9,mr_w9);
            denom(w,mr_w7);
            subtract(mr_w7,mr_w9,mr_w9);
            RS=(int)(mr_w9[1]+mr_base*mr_w9[2]);
            RD=dn;
            build(mr_w9,quad);
        }
        if (size(mr_w9)==1) copy(mr_w8,w);
        else             fdiv(mr_w8,mr_w9,w);
        if (minus) frecip(w,w);
        insign(s,w);
        mr_depth--;
        return FALSE;
    }
    hack=FALSE;
    if (lent(w)<=2)
    { /* for 'simple' w only */
        hack=TRUE;
        fpi(mr_w15);
        fpmul(mr_w15,1,3,mr_w10);
        fpower(mr_w10,n,mr_w10);
        fmul(w,mr_w10,w);
    }
    op[0]=0x6C;  /* set up for [(n-1).x+y]/n */
    op[1]=n-1;
    op[2]=1;
    op[3]=n;
    op[4]=0;
    mr_workprec=mr_stprec;
    dconv(pow(fdsize(w),1.0/(double)n),mr_w10);
    while (mr_workprec!=mr_nib)
    { /* Newtons iteration mr_w10=(w/mr_w10^(n-1)+(n-1)*mr_w10)/n */
        if (mr_workprec<mr_nib) mr_workprec*=2;
        if (mr_workprec>=mr_nib) mr_workprec=mr_nib;
        else if (mr_workprec*2>mr_nib) mr_workprec=(mr_nib+1)/2;
        fpower(mr_w10,n-1,mr_w9);
        fdiv(w,mr_w9,mr_w9);
        flop(mr_w10,mr_w9,op,mr_w10);
    }
    copy(mr_w10,w);
    op[0]=0x48;
    op[1]=3;
    op[3]=1;
    op[2]=op[4]=0;
    if (hack) flop(w,mr_w15,op,w);
    if (minus) frecip(w,w);
    insign(s,w);
    mr_depth--;
    return FALSE;
}

#endif

