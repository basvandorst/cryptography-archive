/*
 *   MIRACL calculate pi - by Gauss-Legendre method
 *   mrpi.c
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */


#include <stdio.h>
#include <miracl.h>

#ifdef MR_FLASH  

/* Access global variables */

extern  int mr_depth;     /* error tracing .. */
extern  int mr_trace[];   /* ... mechanism    */
extern  int mr_nib;       /* length of bigs   */
extern  int mr_lg2b;      /* bits in base     */

extern big  mr_w11,mr_w12,mr_w13,mr_w15; /* workspace variables */

void fpi(pi)
flash pi;
{ /* Calculate pi using Guass-Legendre method */
    int x,nits,op[5];
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=53;
    if (TRACER) track();
    if (size(mr_w15)!=0)
    {
        copy(mr_w15,pi);
        EXACT=FALSE;
        mr_depth--;
        return;
    }
    fconv(1,2,pi);
    froot(pi,2,pi);
    fconv(1,1,mr_w11);
    fconv(1,4,mr_w12);
    x=1;
    op[0]=0x6C;
    op[1]=1;
    op[4]=0;
    nits=mr_lg2b*mr_nib/4;
    while (x<nits)
    {
        copy(mr_w11,mr_w13);
        op[2]=1;
        op[3]=2;
        flop(mr_w11,pi,op,mr_w11);
        fmul(pi,mr_w13,pi);
        froot(pi,2,pi);
        fsub(mr_w11,mr_w13,mr_w13);
        fmul(mr_w13,mr_w13,mr_w13);
        op[3]=1;
        op[2]=(-x);
        flop(mr_w12,mr_w13,op,mr_w12);  /* mr_w12 = mr_w12 - x.mr_w13 */
        x*=2;
    }
    fadd(mr_w11,pi,pi);
    fmul(pi,pi,pi);
    op[0]=0x48;
    op[2]=0;
    op[3]=4;
    flop(pi,mr_w12,op,pi);   /* pi = pi/(4.mr_w12) */
    copy(pi,mr_w15);
    mr_depth--;
}

#endif

