/*
 *  MIRACL flash hyperbolic trig.
 *  bnflsh4.c
 */

#include <stdio.h>
#include "miracl.h"

/* Access Global variables */

extern int  depth;    /* error tracing... */
extern int  trace[];  /* .. mechanism     */

extern big  w11,w12;  /* workspace variables */

void ftanh(x,y)
flash x,y;
{ /* calculates y=tanh(x) */
    int op[5];
    copy(x,y);
    if (ERNUM || size(y)==0) return;
    depth++;
    trace[depth]=63;
    if (TRACER) track();
    fexp(y,y);
    op[0]=0x33;
    op[1]=op[3]=op[4]=1;
    op[2]=(-1);
    flop(y,y,op,y);
    depth--;
}

void fatanh(x,y)
flash x,y;
{ /* calculate y=atanh(x) */
    int op[5];
    copy(x,y);
    if (ERNUM || size(y)==0) return;
    depth++;
    trace[depth]=64;
    if (TRACER) track();
    fconv(1,1,w11);
    op[0]=0x66;
    op[1]=op[2]=op[3]=1;
    op[4]=(-1);
    flop(w11,y,op,y);
    flog(y,y);
    fpmul(y,1,2,y);
    depth--;
}

void fsinh(x,y)
flash x,y;
{ /*  calculate y=sinh(x) */
    int op[5];
    copy(x,y);
    if (ERNUM || size(y)==0) return;
    depth++;
    trace[depth]=65;
    if (TRACER) track();
    fexp(y,y);
    op[0]=0xC6;
    op[2]=op[3]=op[4]=1;
    op[1]=(-1);
    flop(y,y,op,y);
    depth--;
}

void fasinh(x,y)
flash x,y;
{ /* calculate y=asinh(x) */
    copy(x,y);
    if (ERNUM || size(y)==0) return;
    depth++;
    trace[depth]=66;
    if (TRACER) track();
    fmul(y,y,w11);
    fincr(w11,1,1,w11);
    froot(w11,2,w11);
    fadd(y,w11,y);
    flog(y,y);
    depth--;
}

void fcosh(x,y)
flash x,y;
{ /* calculate y=cosh(x) */
    int op[5];
    copy(x,y);
    if (ERNUM || size(y)==0)
    {
        convert(1,y);
        return;
    }
    depth++;
    trace[depth]=67;
    if (TRACER) track();
    fexp(y,y);
    op[0]=0xC6;
    op[1]=op[2]=op[3]=op[4]=1;
    flop(y,y,op,y);
    depth--;
}

void facosh(x,y)
flash x,y;
{ /* calculate y=acosh(x) */
    copy(x,y);
    if (ERNUM) return;
    depth++;
    trace[depth]=62;
    if (TRACER) track();
    fmul(y,y,w11);
    fincr(w11,(-1),1,w11);
    froot(w11,2,w11);
    fadd(y,w11,y);
    flog(y,y);
    depth--;
}
