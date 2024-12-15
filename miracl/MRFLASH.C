/*
 *  MIRACL floating-Slash arithmetic
 *  mrflash.c
 *
 *  Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <miracl.h>

#ifdef MR_FLASH

/* Access global variables */

extern int  mr_depth;    /* error tracing ..*/
extern int  mr_trace[];  /* .. mechanism    */
extern bool mr_check;    /* overflow check  */
extern int  mr_nib;      /* length of bigs  */

extern big mr_w1;        /* workspace variables  */
extern big mr_w2;
extern big mr_w3;
extern big mr_w4;
extern big mr_w5;
extern big mr_w6;
extern big mr_w0;

void flop(x,y,op,z)
flash x,y,z;
int *op;
{ /* Do basic flash operation - depending on   *
   * op[]. Performs operations of the form

              A.f1(x,y) + B.f2(x,y)
              -------------------
              C.f3(x,y) + D.f4(x,y)

   * Four functions f(x,y) are supported and    *
   * coded thus                                 *
   *              00 - Nx.Ny                    *
   *              01 - Nx.Dy                    *
   *              10 - Dx.Ny                    *
   *              11 - Dx.Dy                    *
   * where Nx is numerator of x, Dx denominator *
   * of x, etc.                                 *
   * op[0] contains the codes for f1-f4 in last *
   * eight bits =  00000000f1f2f3f4             *
   * op[1], op[2], op[3], op[4] contain the     *
   * single precision multipliers A, B, C, D    */
    int i,code;
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=69;
    if (TRACER) track();
    numer(x,mr_w1);
    denom(x,mr_w2);
    numer(y,mr_w3);
    denom(y,mr_w4);

    mr_check=OFF;
    for (i=1;i<=4;i++)
    {
        zero(mr_w0);
        if (op[i]==0) continue;
        code=(op[0]>>(2*(4-i)))&3;
        switch (code)
        {
        case 0 : if (x==y) multiply(mr_w1,mr_w1,mr_w0);
                 else      multiply(mr_w1,mr_w3,mr_w0);
                 break;
        case 1 : multiply(mr_w1,mr_w4,mr_w0);
                 break;
        case 2 : multiply(mr_w2,mr_w3,mr_w0);
                 break;
        case 3 : if(x==y) multiply(mr_w2,mr_w2,mr_w0);
                 else     multiply(mr_w2,mr_w4,mr_w0);
                 break;
        }
        premult(mr_w0,op[i],mr_w0);
        switch (i)
        {
        case 1 : copy(mr_w0,mr_w5);
                 break;
        case 2 : add(mr_w5,mr_w0,mr_w5);
                 break;
        case 3 : copy(mr_w0,mr_w6);
                 break;
        case 4 : add(mr_w6,mr_w0,mr_w6);
                 break;
        }
    }
    mr_check=ON;
    round(mr_w5,mr_w6,z);
    mr_depth--;
}

void fmul(x,y,z)
flash x,y,z;
{ /* Flash multiplication - z=x*y */
    int op[5];
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=35;
    if (TRACER) track();
    op[0]=0x0C;
    op[1]=op[3]=1;
    op[2]=op[4]=0;
    flop(x,y,op,z);
    mr_depth--;
}

void fdiv(x,y,z)
flash x,y,z;
{ /* Flash divide - z=x/y */
    int op[5];
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=36;
    if (TRACER) track();
    op[0]=0x48;
    op[1]=op[3]=1;
    op[2]=op[4]=0;
    flop(x,y,op,z);
    mr_depth--;
}

void fadd(x,y,z)
flash x,y,z;
{ /* Flash add - z=x+y */
    int op[5];
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=37;
    if (TRACER) track();
    op[0]=0x6C;
    op[1]=op[2]=op[3]=1;
    op[4]=0;   
    flop(x,y,op,z);
    mr_depth--;
}

void fsub(x,y,z)
flash x,y,z;
{ /* Flash subtract - z=x-y */
    int op[5];
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=38;
    if (TRACER) track();
    op[0]=0x6C;
    op[1]=op[3]=1;
    op[2]=(-1);
    op[4]=0;
    flop(x,y,op,z);
    mr_depth--;
}

int fcomp(x,y)
flash x,y;
{ /* compares two Flash numbers             *
   * returns -1 if y>x; +1 if x>y; 0 if x=y */
    if (MR_ERNUM) return 0;
    mr_depth++;
    mr_trace[mr_depth]=39;
    if (TRACER) track();
    numer(x,mr_w1);
    denom(y,mr_w2);
    mr_check=OFF;
    multiply(mr_w1,mr_w2,mr_w5);
    numer(y,mr_w1);
    denom(x,mr_w2);
    multiply(mr_w1,mr_w2,mr_w0);
    mr_check=ON;
    mr_depth--;
    return (compare(mr_w5,mr_w0));
}

void ftrunc(x,y,z)
flash x,z;
big y;
{ /* sets y=int(x), z=rem(x) - returns *
   * y only for ftrunc(x,y,y)          */
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=45;
    if (TRACER) track();
    numer(x,mr_w1);
    denom(x,mr_w2);
    divide(mr_w1,mr_w2,mr_w3);
    copy(mr_w3,y);
    if (y!=z) fpack(mr_w1,mr_w2,z);
    mr_depth--;
}

void fconv(n,d,x)
int n,d;
flash x;
{ /* convert simple fraction n/d to Flash form */
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=40;
    if (TRACER) track();
    if (d<0)
    {
        d=(-d);
        n=(-n);
    }
    convert(n,mr_w5);
    convert(d,mr_w6);
    fpack(mr_w5,mr_w6,x);
    mr_depth--;
}

void frecip(x,y)
flash x,y;
{ /* set y=1/x */
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=41;
    if (TRACER) track();
    numer(x,mr_w1);
    denom(x,mr_w2);
    fpack(mr_w2,mr_w1,y);
    mr_depth--;
}

void fpmul(x,n,d,y)
flash x,y;
int n,d;
{ /* multiply x by small fraction n/d - y=x*n/d */
    int r,g;
    if (MR_ERNUM) return;
    if (n==0 || size(x)==0)
    {
        zero(y);
        return;
    }
    if (n==d)
    {
        copy(x,y);
        return;
    }
    mr_depth++;
    mr_trace[mr_depth]=42;
    if (TRACER) track();
    if (d<0)
    {
        d=(-d);
        n=(-n);
    }
    numer(x,mr_w1);
    denom(x,mr_w2);
    r=subdiv(mr_w1,d,mr_w3);
    g=igcd(d,r);
    r=subdiv(mr_w2,n,mr_w3);
    g*=igcd(n,r);
    mr_check=OFF;
    premult(mr_w1,n,mr_w5);
    premult(mr_w2,d,mr_w6);
    subdiv(mr_w5,g,mr_w5);
    subdiv(mr_w6,g,mr_w6);
    mr_check=ON;
    if (fit(mr_w5,mr_w6,mr_nib))
        fpack(mr_w5,mr_w6,y);
    else
        round(mr_w5,mr_w6,y);
    mr_depth--;
}

void fincr(x,n,d,y)
flash x,y;
int n,d;
{ /* increment x by small fraction n/d - y=x+(n/d) */
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=43;
    if (TRACER) track();
    if (d<0)
    {
        d=(-d);
        n=(-n);
    }
    numer(x,mr_w1);
    denom(x,mr_w2);
    mr_check=OFF;
    premult(mr_w1,d,mr_w5);
    premult(mr_w2,d,mr_w6);
    premult(mr_w2,n,mr_w0);
    add(mr_w5,mr_w0,mr_w5);
    mr_check=ON;
    if (d==1 && fit(mr_w5,mr_w6,mr_nib))
        fpack(mr_w5,mr_w6,y);
    else
        round(mr_w5,mr_w6,y);
    mr_depth--;
}

#endif
