/*
 *   MIRACL floating-Slash arithmetic
 *   bnflash.c
 */

#include <stdio.h>
#include "miracl.h"
#define abs(x)  ((x)<0? (-(x)) : (x))

/* Access global variables */

extern int  depth;    /* error tracing ..*/
extern int  trace[];  /* .. mechanism    */
extern bool check;    /* overflow check  */
extern int  nib;      /* length of bigs  */

extern big w1;        /* workspace variables  */
extern big w2;
extern big w3;
extern big w4;
extern big w5;
extern big w6;
extern big w0;

void flop(x,y,op,z)
flash x,y,z;
int *op;
{ /* Do basic flash operation - depending on op[] *
   * Performs operations of the form

                 A.f1(x,y) + B.f2(x,y)
                 -------------------
                 C.f3(x,y) + D.f4(x,y)

   * Four functions f(x,y) are supported and      *
   * coded thus                                   *
   *              00 - Nx.Ny                      *
   *              01 - Nx.Dy                      *
   *              10 - Dx.Ny                      *
   *              11 - Dx.Dy                      *
   * where Nx is numerator of x, Dx denominator   *
   * of x, etc.                                   *
   * op[0] contains the codes for f1-f4 in bottom *
   * eight bits =  00000000f1f2f3f4               *
   * op[1], op[2], op[3], op[4] contain the       *
   * single precision multipliers A, B, C, D      */
    int i,code;
    if (ERNUM) return;
    depth++;
    trace[depth]=69;
    if (TRACER) track();
    numer(x,w1);
    denom(x,w2);
    numer(y,w3);
    denom(y,w4);
    check=OFF;
    for (i=1;i<=4;i++)
    {
        zero(w0);
        if (op[i]==0) continue;
        code=(op[0]>>(2*(4-i)))&3;
        switch (code)
        {
        case 0 : if (x==y) multiply(w1,w1,w0);
                 else      multiply(w1,w3,w0);
                 break;
        case 1 : multiply(w1,w4,w0);
                 break;
        case 2 : multiply(w2,w3,w0);
                 break;
        case 3 : if(x==y) multiply(w2,w2,w0);
                 else     multiply(w2,w4,w0);
                 break;
        }
        premult(w0,(small)op[i],w0);
        switch (i)
        {
        case 1 : copy(w0,w5);
                 break;
        case 2 : add(w5,w0,w5);
                 break;
        case 3 : copy(w0,w6);
                 break;
        case 4 : add(w6,w0,w6);
                 break;
        }
    }
    check=ON;
    round(w5,w6,z);
    depth--;
}

void fmul(x,y,z)
flash x,y,z;
{ /* Flash multiplication - z=x*y */
    int op[5];
    if (ERNUM) return;
    depth++;
    trace[depth]=35;
    if (TRACER) track();
    op[0]=0x0C;
    op[1]=op[3]=1;
    op[2]=op[4]=0;
    flop(x,y,op,z);
    depth--;
}

void fdiv(x,y,z)
flash x,y,z;
{ /* Flash divide - z=x/y */
    int op[5];
    if (ERNUM) return;
    depth++;
    trace[depth]=36;
    if (TRACER) track();
    op[0]=0x48;
    op[1]=op[3]=1;
    op[2]=op[4]=0;
    flop(x,y,op,z);
    depth--;
}

void fadd(x,y,z)
flash x,y,z;
{ /* Flash add - z=x+y */
    int op[5];
    if (ERNUM) return;
    depth++;
    trace[depth]=37;
    if (TRACER) track();
    op[0]=0x6C;
    op[1]=op[2]=op[3]=1;
    op[4]=0;   
    flop(x,y,op,z);
    depth--;
}

void fsub(x,y,z)
flash x,y,z;
{ /* Flash subtract - z=x-y */
    int op[5];
    if (ERNUM) return;
    depth++;
    trace[depth]=38;
    if (TRACER) track();
    op[0]=0x6C;
    op[1]=op[3]=1;
    op[2]=(-1);
    op[4]=0;
    flop(x,y,op,z);
    depth--;
}

int fcomp(x,y)
flash x,y;
{ /* compares two Flash numbers             *
   * returns -1 if y>x; +1 if x>y; 0 if x=y */
    if (ERNUM) return 0;
    depth++;
    trace[depth]=39;
    if (TRACER) track();
    numer(x,w1);
    denom(y,w2);
    check=OFF;
    multiply(w1,w2,w5);
    numer(y,w1);
    denom(x,w2);
    multiply(w1,w2,w0);
    check=ON;
    depth--;
    return (compare(w5,w0));
}

void ftrunc(x,y,z)
flash x,z;
big y;
{ /* sets y=int(x), z=rem(x) - returns *
   * y only for ftrunc(x,y,y)          */
    if (ERNUM) return;
    depth++;
    trace[depth]=45;
    if (TRACER) track();
    numer(x,w1);
    denom(x,w2);
    divide(w1,w2,w3);
    copy(w3,y);
    if (y!=z) fpack(w1,w2,z);
    depth--;
}

void fconv(n,d,x)
small n,d;
flash x;
{ /* convert simple fraction n/d to Flash form */
    small g;
    if (ERNUM) return;
    depth++;
    trace[depth]=40;
    if (TRACER) track();
    if (d<0)
    {
        d=(-d);
        n=(-n);
    }
    g=igcd(n,d);
    n/=g;
    d/=g;
    convert(n,w5);
    convert(d,w6);
    fpack(w5,w6,x);
    depth--;
}

void frecip(x,y)
flash x,y;
{ /* set y=1/x */
    if (ERNUM) return;
    depth++;
    trace[depth]=41;
    if (TRACER) track();
    numer(x,w1);
    denom(x,w2);
    fpack(w2,w1,y);
    depth--;
}

void fpmul(x,n,d,y)
flash x,y;
small n,d;
{ /* multiply x by small fraction n/d - y=x*n/d */
    small r,g;
    if (ERNUM) return;
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
    depth++;
    trace[depth]=42;
    if (TRACER) track();
    if (d<0)
    {
        d=(-d);
        n=(-n);
    }
    g=igcd(n,d);
    n/=g;
    d/=g;
    numer(x,w1);
    denom(x,w2);
    r=subdiv(w1,d,w3);
    g=igcd(d,r);
    r=subdiv(w2,n,w3);
    g*=igcd(n,r);
    check=OFF;
    premult(w1,n,w5);
    premult(w2,d,w6);
    subdiv(w5,g,w5);
    subdiv(w6,g,w6);
    check=ON;
    if (abs(w5[0])+abs(w6[0])<=nib)
        fpack(w5,w6,y);
    else
        round(w5,w6,y);
    depth--;
}

void fincr(x,n,d,y)
flash x,y;
small n,d;
{ /* increment x by small fraction n/d - y=x+(n/d) */
    if (ERNUM) return;
    depth++;
    trace[depth]=43;
    if (TRACER) track();
    if (d<0)
    {
        d=(-d);
        n=(-n);
    }
    numer(x,w1);
    denom(x,w2);
    check=OFF;
    premult(w1,d,w5);
    premult(w2,d,w6);
    premult(w2,n,w0);
    add(w5,w0,w5);
    check=ON;
    if (d==1 && (abs(w5[0])+abs(w6[0])<=nib))
        fpack(w5,w6,y);
    else
        round(w5,w6,y);
    depth--;
}
