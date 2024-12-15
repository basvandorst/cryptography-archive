/*
 *   MIRACL arithmetic routines 3 - powers and roots
 *   mrarth3.c
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
extern int   mr_lg2b;     /* bits in base    */
extern small mr_base2;    /* 2^mr_lg2b          */
extern bool  mr_check;    /* overflow check  */
extern big mr_w0;
extern big mr_w1;        /* workspace variables */
extern big mr_w2;
extern big mr_w3;
extern big mr_w4;
extern big mr_w5;
extern big mr_w6;

#ifdef FULLWIDTH

bool testbit(x,i)
big x;
int i;
{ /* return value of i-th bit of big */
    if ((x[1+i/MR_SBITS] & ((small)1<<(i%MR_SBITS))) >0) return TRUE;
    return FALSE;
}

#endif

int logb2(x)
big x;
{ /* returns number of bits in x */
    int xl,lg2;
    if (MR_ERNUM || size(x)==0) return 0;
    mr_depth++;
    mr_trace[mr_depth]=49;
    if (TRACER) track();
    copy(x,mr_w0);
    insign(PLUS,mr_w0);
    lg2=0;
    xl=(int)mr_w0[0];
    if (mr_base==mr_base2)
    {
        lg2=mr_lg2b*(xl-1);
        shift(mr_w0,1-xl,mr_w0);
    }
    else while (mr_w0[0]>1)
    {
        sdiv(mr_w0,mr_base2,mr_w0);
        lg2+=mr_lg2b;
    }
    while (mr_w0[1]>=1)
    {
        lg2++;
        mr_w0[1]/=2;
    }
    mr_depth--;
    return lg2;
}

void expb2(n,x)
int n;
big x;
{ /* sets x=2^n */
    int i,r,p;
    if (MR_ERNUM) return;
    convert(1,x);
    if (n==0) return;
    mr_depth++;
    mr_trace[mr_depth]=50;
    if (TRACER) track();
    if (n<0)
    {
        berror(10);
        mr_depth--;
        return;
    }
    r=n/mr_lg2b;
    p=n%mr_lg2b;
    if (mr_base==mr_base2)
    {
        shift(x,r,x);
        x[x[0]]<<=p;
    }
    else
    {
        for (i=1;i<=r;i++)
            pmul(x,mr_base2,x);
        pmul(x,((small)1<<p),x);
    }
    mr_depth--;
}   

void sftbit(x,n,z)
big x,z;
int n;
{ /* shift x by n bits */
    int m;
    if (MR_ERNUM) return;
    copy(x,z);
    if (n==0) return;
    mr_depth++;
    mr_trace[mr_depth]=47;
    if (TRACER) track();
    m=abs(n);
    if (n>0)
    { /* shift left */
        if (mr_base==mr_base2)
        {
            shift(z,n/mr_lg2b,z);
            pmul(z,(small)1<<(m%mr_lg2b),z);
        }
        else
        {
            expb2(m,mr_w1);
            multiply(z,mr_w1,z);
        }
    }
    else
    { /* shift right */
        if (mr_base==mr_base2)
        {
            shift(z,n/mr_lg2b,z);
            sdiv(z,(small)1<<(m%mr_lg2b),z);
        }
        else
        {
            expb2(m,mr_w1);
            divide(z,mr_w1,z);
        }
    }
    mr_depth--;
}

void power(x,n,z,w)
big w;
big x;
int n;
big z;
{ /* raise big number to int power  w=x^n *
   * (mod z if z and w distinct)          */
    small norm;
    copy(x,mr_w5);
    zero(w);
    if(MR_ERNUM || size(mr_w5)==0) return;
    convert(1,w);
    if (n==0) return;
    mr_depth++;
    mr_trace[mr_depth]=17;
    if (TRACER) track();
    if (n<0)
    {
        berror(10);
        mr_depth--;
        return;
    }
    if (w==z) forever
    { /* "Russian peasant" exponentiation */
        if (n%2!=0) multiply(w,mr_w5,w);
        n/=2;
        if (MR_ERNUM || n==0) break;
        multiply(mr_w5,mr_w5,mr_w5);
    }
    else
    { 
        norm=normalise(z,z);
        divide(mr_w5,z,z);
        forever
        {
            if (n%2!=0) mad(w,mr_w5,mr_w5,z,z,w);
            n/=2;
            if (MR_ERNUM || n==0) break;
            mad(mr_w5,mr_w5,mr_w5,z,z,mr_w5);
        }
        if (norm!=1)
        {
            sdiv(z,norm,z);
            divide(w,z,z);
        }
    }
    mr_depth--;
}

void powltr(x,y,z,w)
int x;
big y,z,w;
{ /* calculates w=x^y mod z using Left to Right Method   */
  /* uses only n^2 modular squarings, because x is small */
    int i,nb;
    small norm;
    if (MR_ERNUM) return;
    copy(y,mr_w1);
    zero(w);
    if (x==0) return;
    convert(1,w);
    if (size(mr_w1)==0) return;
    mr_depth++;
    mr_trace[mr_depth]=71;
    if (TRACER) track();
    if (size(mr_w1)<0) berror(10);
    if (w==z)       berror(7) ;
    if (MR_ERNUM)
    {
        mr_depth--;
        return;
    }
    norm=normalise(z,z);
#ifdef FULLWIDTH
    nb=logb2(mr_w1);
    convert(x,w);
    if (nb>1) for (i=nb-2;i>=0;i--)
    { /* Left to Right binary method */
        mad(w,w,w,z,z,w);
        if (testbit(mr_w1,i))
        { /* this is quick */
            premult(w,x,w);
            divide(w,z,z);
        }
    }    
#else
    expb2(logb2(mr_w1)-1,mr_w2);
    while (size(mr_w2)!=0)
    { /* Left to Right binary method */
        mad(w,w,w,z,z,w);
        if (compare(mr_w1,mr_w2)>=0)
        {
            premult(w,x,w);
            divide(w,z,z);
            subtract(mr_w1,mr_w2,mr_w1);
        }
        subdiv(mr_w2,2,mr_w2);
    }
#endif
    if (norm!=1) 
    {
        sdiv(z,norm,z);
        divide(w,z,z);
    }
    mr_depth--;
}

void powmod(x,y,z,w)
big w;
big x;
big y;
big z;
{  /*  calculates w=x^y mod z */
   /*  various 'faster' methods have been looked at.  *
    *  All require more memory/more elaborate control *
    *  None is really worth it - but see below        */ 
    small norm;
    int i,nb;
    if (MR_ERNUM) return;
    copy(y,mr_w1);
    copy(x,mr_w2);
    zero(w);
    if (size(mr_w2)==0) return;
    convert (1,w);
    if (size(mr_w1)==0) return;
    mr_depth++;
    mr_trace[mr_depth]=18;
    if (TRACER) track();
    if (size(mr_w1)<0) berror(10);
    if (w==z)       berror(7) ;
    if (MR_ERNUM)
    {
        mr_depth--;
        return;
    }
    norm=normalise(z,z);
    divide(mr_w2,z,z);

#ifdef FULLWIDTH
/* here we don't need mr_w1, so we can use it to
   obtain a modest 10% optimization. We precalculate
   x^3, and use it if we see two 1's in the exponent.
   This also improves worst case performance. This
   'sliding windows' idea can be extended, at the 
   cost of more precomputations, and more memory, with
   diminishing returns. Precalculate x^n, for n=3,5,7,9 etc
   and look ahead for these bit patterns in the exponent.
   A further 10% speed-up is possible, but it rather goes
   against the MIRACL philosophy of keeping things as small
   and transparent as possible */

    mad(mr_w2,mr_w2,mr_w2,z,z,mr_w1);
    mad(mr_w2,mr_w1,mr_w2,z,z,mr_w1); /* x^3 */
    nb=logb2(y);
    copy(mr_w2,w);
    if (nb>1) for (i=nb-2;i>=0;)
    { /* Left to Right method */
        mad(w,w,w,z,z,w);
        if (testbit(y,i--)) 
        { /* check if next bit is 1 */
            if (i>0 && testbit(y,i)) 
            { /* it is. */ 
                i--;
                mad(w,w,w,z,z,w);
                mad(w,mr_w1,w,z,z,w);
            }
            else mad(w,mr_w2,w,z,z,w);
        }
    }

#else
    forever
    { /* "Russian peasant" Right-to-Left exponentiation */
        if (subdiv(mr_w1,2,mr_w1)!=0)
            mad(w,mr_w2,mr_w2,z,z,w);
        if (MR_ERNUM || size(mr_w1)==0) break;
        mad(mr_w2,mr_w2,mr_w2,z,z,mr_w2);
    }
#endif
    if (norm!=1)
    {
        sdiv(z,norm,z);
        divide(w,z,z);
    }
    mr_depth--;
}

bool nroot(x,n,w)
big x,w;
int n;
{  /*  extract  lower approximation to nth root   *
    *  w=x^(1/n) returns TRUE for exact root      *
    *  uses Newtons method                        */
    int sx,dif,s,p,d,lg2,lgx,rem;
    bool full;
    if (MR_ERNUM) return FALSE;
    if (size(x)==0 || n==1)
    {
        copy(x,w);
        return TRUE;
    }
    mr_depth++;
    mr_trace[mr_depth]=16;
    if (TRACER) track();
    if (n<1) berror(11);
    sx=exsign(x);
    if (n%2==0 && sx==MINUS) berror(9);
    if (MR_ERNUM) 
    {
        mr_depth--;
        return FALSE;
    }
    insign(PLUS,x);
    lgx=logb2(x);
    if (n>=lgx)
    { /* root must be 1 */
        insign(sx,x);
        convert(sx,w);
        mr_depth--;
        if (lgx==1) return TRUE;
        else        return FALSE;
    }
    expb2(1+(lgx-1)/n,mr_w2);           /* guess root as 2^(log2(x)/n) */
    s=(-(((int)x[0]-1)/n)*n);
    shift(mr_w2,s/n,mr_w2);
    lg2=logb2(mr_w2)-1;
    full=FALSE;
    if (s==0) full=TRUE;
    d=0;
    p=1;
    while (!MR_ERNUM)
    { /* Newtons method */
        copy(mr_w2,mr_w3);
        shift(x,s,mr_w4);
        mr_check=OFF;
        power(mr_w2,n-1,mr_w6,mr_w6);
        mr_check=ON;
        divide(mr_w4,mr_w6,mr_w2);
        rem=size(mr_w4);
        subtract(mr_w2,mr_w3,mr_w2);
        dif=size(mr_w2);
        subdiv(mr_w2,n,mr_w2);
        add(mr_w2,mr_w3,mr_w2);
        p*=2;
        if(p<lg2+d*mr_lg2b) continue;
        if (full && abs(dif)<n)
        { /* test for finished */
            while (dif<0)
            {
                rem=0;
                decr(mr_w2,1,mr_w2);
                mr_check=OFF;
                power(mr_w2,n,mr_w6,mr_w6);
                mr_check=ON;
                dif=compare(x,mr_w6);
            }
            copy(mr_w2,w);
            insign(sx,w);
            insign(sx,x);
            mr_depth--;
            if (rem==0 && dif==0) return TRUE;
            else                  return FALSE;
        }
        else
        { /* adjust precision */
            d*=2;
            if (d==0) d=1;
            s+=d*n;
            if (s>=0)
            {
                d-=s/n;
                s=0;
                full=TRUE;
            }
            shift(mr_w2,d,mr_w2);
        }
        p/=2;
    }
    mr_depth--;
    return FALSE;
}

