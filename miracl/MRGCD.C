/*
 *   MIRACL Greatest Common Divisor module.
 *   mrgcd.c
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <miracl.h>
#define abs(x)  ((x)<0? (-(x)) : (x))

/* Access global variables */

extern int   mr_depth;    /* error tracing ..*/
extern int   mr_trace[];  /* .. mechanism    */
extern small mr_base;     /* number base     */

extern big mr_w1;       /* workspace variables */
extern big mr_w2;
extern big mr_w0;

int egcd(x,y,z)
big x;
big y;
big z;
{ /* greatest common divisor z=gcd(x,y) by Euclids  *
   * method using Lehmers algorithm for big numbers */
    int q,r,a,b,c,d,n;
    small sr,m,st;
    unsigned long u,v,lq,lr;
    big t;
    if (MR_ERNUM) return 0;
    mr_depth++;
    mr_trace[mr_depth]=12;
    if (TRACER) track();
    copy(x,mr_w1);
    copy(y,mr_w2);
    insign(PLUS,mr_w1);
    insign(PLUS,mr_w2);
    a=b=c=d=0;
    while (size(mr_w2)!=0)
    {
        if (b==0)
        { /* update mr_w1 and mr_w2 */
            divide(mr_w1,mr_w2,mr_w2);
            t=mr_w1,mr_w1=mr_w2,mr_w2=t;   /* swap(mr_w1,mr_w2) */
        }
        else
        {
            premult(mr_w1,c,z);
            premult(mr_w1,a,mr_w1);
            premult(mr_w2,b,mr_w0);
            premult(mr_w2,d,mr_w2);
            add(mr_w1,mr_w0,mr_w1);
            add(mr_w2,z,mr_w2);
        }
        if (MR_ERNUM || size(mr_w2)==0) break;
        n=(int)mr_w1[0];
        if (mr_w2[0]==1)
        { /* special case if mr_w2 is now small */
            sr=sdiv(mr_w1,mr_w2[1],mr_w1);
            if (sr==0)
            {
                copy(mr_w2,mr_w1);
                break;
            }
            zero(mr_w1);
            mr_w1[0]=1;
            mr_w1[1]=sr;
            while ((sr=mr_w2[1]%mr_w1[1])!=0)
                  mr_w2[1]=mr_w1[1],mr_w1[1]=sr;
            break;
        }
        a=1;
        b=0;
        c=0;
        d=1;
        m=mr_w1[n]+1;
        if (mr_base==0)
        {
            if (sizeof(long)==2*sizeof(small))
            { /* use longs if they are double length */
                if (n>2 && m!=0)
                { /* squeeze out as much significance as possible */
                    u=muldvm(mr_w1[n],mr_w1[n-1],m,&sr);
                    st=muldvm(sr,mr_w1[n-2],m,&sr);
                    u=(u<<MR_SBITS)+st;
                    v=muldvm(mr_w2[n],mr_w2[n-1],m,&sr);
                    st=muldvm(sr,mr_w2[n-2],m,&sr);
                    v=(v<<MR_SBITS)+st;
                }
                else
                {
                    u=((unsigned long)mr_w1[n]<<MR_SBITS)+mr_w1[n-1];
                    v=((unsigned long)mr_w2[n]<<MR_SBITS)+mr_w2[n-1];
                }
            }
            else
            {
                if (m==0)
                {
                    u=mr_w1[n];
                    v=mr_w2[n];
                }
                else
                {
                    u=muldvm(mr_w1[n],mr_w1[n-1],m,&sr);
                    v=muldvm(mr_w2[n],mr_w2[n-1],m,&sr);
                }
            }
        }
        else
        {
            if (sizeof(long)==2*sizeof(small))
            { /* use longs if they are double length */
                if (n>2)
                { /* squeeze out as much significance as possible */
                    u=muldiv(mr_w1[n],mr_base,mr_w1[n-1],m,&sr);
                    u=u*mr_base+muldiv(sr,mr_base,mr_w1[n-2],m,&sr);
                    v=muldiv(mr_w2[n],mr_base,mr_w2[n-1],m,&sr);
                    v=v*mr_base+muldiv(sr,mr_base,mr_w2[n-2],m,&sr);
                }
                else
                {
                    u=(unsigned long)mr_base*mr_w1[n]+mr_w1[n-1];
                    v=(unsigned long)mr_base*mr_w2[n]+mr_w2[n-1];
                }
            }
            else
            {
                u=muldiv(mr_w1[n],mr_base,mr_w1[n-1],m,&sr);
                v=muldiv(mr_w2[n],mr_base,mr_w2[n-1],m,&sr);
            }
        }
        forever
        { /* work only with most significant piece */
            if (((v+c)==0) || ((v+d)==0)) break;
            lq=(u+a)/(v+c);
            if (lq!=(u+b)/(v+d)) break;
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
        }
    }
    copy(mr_w1,z);
    mr_depth--;
    return (size(mr_w1));
}

