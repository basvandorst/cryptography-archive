/*
 *   MIRACL flash hyperbolic trig.
 *   mrflsh4.c
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <miracl.h>

#ifdef MR_FLASH

/* Access Global variables */

extern int  mr_depth;    /* error tracing... */
extern int  mr_trace[];  /* .. mechanism     */

extern big  mr_w11;  /* workspace variables */

void ftanh(x,y)
flash x,y;
{ /* calculates y=tanh(x) */
    int op[5];
    copy(x,y);
    if (MR_ERNUM || size(y)==0) return;
    mr_depth++;
    mr_trace[mr_depth]=63;
    if (TRACER) track();
    fexp(y,y);
    op[0]=0x33;
    op[1]=op[3]=op[4]=1;
    op[2]=(-1);
    flop(y,y,op,y);
    mr_depth--;
}

void fatanh(x,y)
flash x,y;
{ /* calculate y=atanh(x) */
    int op[5];
    copy(x,y);
    if (MR_ERNUM || size(y)==0) return;
    mr_depth++;
    mr_trace[mr_depth]=64;
    if (TRACER) track();
    fconv(1,1,mr_w11);
    op[0]=0x66;
    op[1]=op[2]=op[3]=1;
    op[4]=(-1);
    flop(mr_w11,y,op,y);
    flog(y,y);
    fpmul(y,1,2,y);
    mr_depth--;
}

void fsinh(x,y)
flash x,y;
{ /*  calculate y=sinh(x) */
    int op[5];
    copy(x,y);
    if (MR_ERNUM || size(y)==0) return;
    mr_depth++;
    mr_trace[mr_depth]=65;
    if (TRACER) track();
    fexp(y,y);
    op[0]=0xC6;
    op[2]=op[3]=op[4]=1;
    op[1]=(-1);
    flop(y,y,op,y);
    mr_depth--;
}

void fasinh(x,y)
flash x,y;
{ /* calculate y=asinh(x) */
    copy(x,y);
    if (MR_ERNUM || size(y)==0) return;
    mr_depth++;
    mr_trace[mr_depth]=66;
    if (TRACER) track();
    fmul(y,y,mr_w11);
    fincr(mr_w11,1,1,mr_w11);
    froot(mr_w11,2,mr_w11);
    fadd(y,mr_w11,y);
    flog(y,y);
    mr_depth--;
}

void fcosh(x,y)
flash x,y;
{ /* calculate y=cosh(x) */
    int op[5];
    copy(x,y);
    if (MR_ERNUM || size(y)==0)
    {
        convert(1,y);
        return;
    }
    mr_depth++;
    mr_trace[mr_depth]=67;
    if (TRACER) track();
    fexp(y,y);
    op[0]=0xC6;
    op[1]=op[2]=op[3]=op[4]=1;
    flop(y,y,op,y);
    mr_depth--;
}

void facosh(x,y)
flash x,y;
{ /* calculate y=acosh(x) */
    copy(x,y);
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=62;
    if (TRACER) track();
    fmul(y,y,mr_w11);
    fincr(mr_w11,(-1),1,mr_w11);
    froot(mr_w11,2,mr_w11);
    fadd(y,mr_w11,y);
    flog(y,y);
    mr_depth--;
}

#endif

