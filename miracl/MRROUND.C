/*
 *   MIRACL euclidean mediant rounding routine
 *   mrround.c
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <miracl.h>

#ifdef MR_FLASH

#define abs(x)  ((x)<0? (-(x)) : (x))

/* Access global variables */

extern int  mr_depth;    /* error tracing .. */
extern int  mr_trace[];  /* .. mechanism     */
extern int  mr_nib;      /* length of bigs   */
extern small mr_base;    /* number base      */
extern bool mr_check;    /* overflow check   */

extern big mr_w0;        /* workspace variables */
extern big mr_w5;
extern big mr_w6;
extern big mr_w7;

static int euclid(x,num)
big x;
int num;
{ /* outputs next c.f. quotient from gcd(mr_w5,mr_w6) */
    static unsigned long u,v,ku,kv;
    static int a,b,c,d,r,n,oldn,q;
    static bool last,carryon;
    small sr,m,st;
    unsigned long lr,lq;
    big t;
    if (num==0)
    {
        oldn=(-1);
        carryon=FALSE;
        last=FALSE;
        if (compare(mr_w6,mr_w5)>0)
        { /* ensure mr_w5>mr_w6 */
            t=mr_w5,mr_w5=mr_w6,mr_w6=t;
            return (q=0);
        }
    }
    else if (num==oldn || q<0) return q;
    oldn=num;
    if (carryon) goto middle;
start:
    if (size(mr_w6)==0) return (q=(-1));
    n=(int)mr_w5[0];
    carryon=TRUE;
    a=1;
    b=0;
    c=0;
    d=1;
    if (n==1)
    {
        last=TRUE;
        u=mr_w5[1];
        v=mr_w6[1];
    }
    else
    {
        m=mr_w5[n]+1;
        if (mr_base==0)
        {
            if (sizeof(long)==2*sizeof(small))
            { /* use longs if they are double length */
                if (n>2 && m!=0)
                { /* squeeze out as much significance as possible */
                    u=muldvm(mr_w5[n],mr_w5[n-1],m,&sr);
                    st=muldvm(sr,mr_w5[n-2],m,&sr);
                    u=(u<<MR_SBITS)+st;
                    v=muldvm(mr_w6[n],mr_w6[n-1],m,&sr);
                    st=muldvm(sr,mr_w6[n-2],m,&sr);
                    v=(v<<MR_SBITS)+st;
                }
                else
                {
                    u=((unsigned long)mr_w5[n]<<MR_SBITS)+mr_w5[n-1];
                    v=((unsigned long)mr_w6[n]<<MR_SBITS)+mr_w6[n-1];
                    if (n==2) last=TRUE;
                }
            }     
            else
            {
                if (m==0)
                {
                    u=mr_w5[n];
                    v=mr_w6[n];
                }
                else
                {
                    u=muldvm(mr_w5[n],mr_w5[n-1],m,&sr);
                    v=muldvm(mr_w6[n],mr_w6[n-1],m,&sr);
                }
            }
        }
        else
        {
            if (sizeof(long)==2*sizeof(small))
            { /* use longs if they are double length */
                if (n>2)
                { /* squeeze out as much significance as possible */
                    u=muldiv(mr_w5[n],mr_base,mr_w5[n-1],m,&sr);
                    u=u*mr_base+muldiv(sr,mr_base,mr_w5[n-2],m,&sr);
                    v=muldiv(mr_w6[n],mr_base,mr_w6[n-1],m,&sr);
                    v=v*mr_base+muldiv(sr,mr_base,mr_w6[n-2],m,&sr);
                }
                else
                {
                    u=(unsigned long)mr_base*mr_w5[n]+mr_w5[n-1];
                    v=(unsigned long)mr_base*mr_w6[n]+mr_w6[n-1];
                    last=TRUE;
                }
            }
            else
            {
                
                u=muldiv(mr_w5[n],mr_base,mr_w5[n-1],m,&sr);
                v=muldiv(mr_w6[n],mr_base,mr_w6[n-1],m,&sr);
            }
        }
    }
    ku=u;
    kv=v;
middle:
    forever
    { /* work only with most significant piece */
        if (last)
        {
            if (v==0) return (q=(-1));
            lq=u/v;
        }
        else
        {
            if (((v+c)==0) || ((v+d)==0)) break;
            lq=(u+a)/(v+c);
            if (lq!=(u+b)/(v+d)) break;
        }
        if (lq>=(unsigned long)(TOOBIG/abs(d))) break;
        q=(int)lq;
        r=a-q*c;
        a=c;
        c=r;
        r=b-q*d;
        b=d;
        d=r;
        lr=u-lq*v;
        u=v;
        v=lr;
        return q;
    }
    carryon=FALSE;
    if (b==0)
    { /* update mr_w5 and mr_w6 */
        mr_check=OFF;
        divide(mr_w5,mr_w6,mr_w7);
        mr_check=ON;
        if (lent(mr_w7)>mr_nib) return (q=(-2));
        t=mr_w5,mr_w5=mr_w6,mr_w6=t;   /* swap(mr_w5,mr_w6) */
        copy(mr_w7,x);
        return (q=size(x));
    }
    else
    {
        mr_check=OFF;
        premult(mr_w5,c,mr_w7);
        premult(mr_w5,a,mr_w5);
        premult(mr_w6,b,mr_w0);
        premult(mr_w6,d,mr_w6);
        add(mr_w5,mr_w0,mr_w5);
        add(mr_w6,mr_w7,mr_w6);
        mr_check=ON;
    }
    goto start;
}


void round(num,den,z)
big num,den;
flash z;
{ /* reduces and rounds the fraction num/den into z */
    int s;
    if (MR_ERNUM) return;
    if (size(num)==0) 
    {
        zero(z);
        return;
    }
    mr_depth++;
    mr_trace[mr_depth]=34;
    if (TRACER) track();
    if (size(den)==0)
    {
        berror(13);
        mr_depth--;
        return;
    }
    copy(num,mr_w5);
    copy(den,mr_w6);
    s=exsign(mr_w5)*exsign(mr_w6);
    insign(PLUS,mr_w5);
    insign(PLUS,mr_w6);
    if (compare(mr_w5,mr_w6)==0)
    {
        convert(s,z);
        mr_depth--;
        return;
    }
    if (size(mr_w6)==1)
    {
        if ((int)mr_w5[0]>mr_nib)
        {
            berror(13);
            mr_depth--;
            return;
        }
        copy(mr_w5,z);
        insign(s,z);
        mr_depth--;
        return;
    }
    build(z,euclid);
    insign(s,z);
    mr_depth--;
}

#endif

