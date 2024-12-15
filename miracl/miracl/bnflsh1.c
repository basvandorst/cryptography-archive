/*
 *    MIRACL flash roots and powers
 *    bnflsh1.c
 */

#include <stdio.h>
#include <math.h>
#include "miracl.h"

/* Access global variables */

extern int depth;     /* error tracing .. */
extern int trace[];   /* ... mechanism    */
extern int nib;       /* length of bigs   */
extern int lg2b;      /* no. bits in base */
extern small base;    /* number base      */
extern int workprec;
extern int stprec;    /* start precision  */

extern big w7,w8,w9,w10,w15; /* workspace variables */

static int RS,RD;

int quad(w,n)
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
    copy(x,w8);
    zero(w);
    if (ERNUM || size(w8)==0) return;
    convert(1,w);
    if (n==0) return;
    depth++;
    trace[depth]=51;
    if (TRACER) track();
    if (n<0)
    {
        n=(-n);
        frecip(w8,w8);
    }
    if (n==1)
    {
        copy(w8,w);
        depth--;
        return;
    }
    forever
    {
        if (n%2!=0) fmul(w,w8,w);
        n/=2;
        if (ERNUM || n==0) break;
        fmul(w8,w8,w8);
    }
    depth--;
}
 
bool froot(x,n,w)
flash w,x;
int n;
{ /* extract nth root of x  - w=x^(1/n) using Newtons method */
    bool minus,rn,rm,hack;
    int nm,dn,s,op[5];
    copy(x,w);
    if (ERNUM || n==1) return TRUE;
    if (n==(-1))
    {
        frecip(w,w);
        return TRUE;
    }
    depth++;
    trace[depth]=52;
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
        depth--;
        return FALSE;
    }
    insign(PLUS,w);
    numer(w,w8);
    denom(w,w9);
    rn=root(w8,n,w8);
    rm=root(w9,n,w9);
    if (rn && rm)
    {
        fpack(w8,w9,w);
        if (minus) frecip(w,w);
        insign(s,w);
        depth--;
        return TRUE;
    }
    nm=size(w8);
    dn=size(w9);
    if (n==2 && ((nm<TOOBIG) || rn) && ((dn<TOOBIG) || rm))
    {
        if (!rn && nm<TOOBIG)
        {
            multiply(w8,w8,w8);
            numer(w,w7);
            subtract(w7,w8,w8);
            RS=w8[1]+base*w8[2];
            RD=nm;
            build(w8,quad);
        }
        if (!rm && dn<TOOBIG)
        {
            multiply(w9,w9,w9);
            denom(w,w7);
            subtract(w7,w9,w9);
            RS=w9[1]+base*w9[2];
            RD=dn;
            build(w9,quad);
        }
        if (size(w9)==1) copy(w8,w);
        else             fdiv(w8,w9,w);
        if (minus) frecip(w,w);
        insign(s,w);
        depth--;
        return FALSE;
    }
    hack=FALSE;
    if (lent(w)<=2)
    { /* for 'simple' w only */
        hack=TRUE;
        fpi(w15);
        fpmul(w15,1,3,w10);
        fpower(w10,n,w10);
        fmul(w,w10,w);
    }
    op[0]=0x6C;  /* set up for [(n-1).x+y]/n */
    op[1]=n-1;
    op[2]=1;
    op[3]=n;
    op[4]=0;
    workprec=stprec;
    dconv(pow(fdsize(w),1.0/(double)n),w10);
    while (workprec!=nib)
    { /* Newtons iteration w10=(w/w10^(n-1)+(n-1)*w10)/n */
        if (workprec<nib) workprec*=2;
        if (workprec>=nib) workprec=nib;
        else if (workprec*2>nib) workprec=(nib+1)/2;
        fpower(w10,n-1,w9);
        fdiv(w,w9,w9);
        flop(w10,w9,op,w10);
    }
    copy(w10,w);
    op[0]=0x48;
    op[1]=3;
    op[3]=1;
    op[2]=op[4]=0;
    if (hack) flop(w,w15,op,w);
    if (minus) frecip(w,w);
    insign(s,w);
    depth--;
    return FALSE;
}
