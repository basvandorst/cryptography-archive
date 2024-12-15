/*
 *   MIRACL calculate pi - by Gauss-Legendre method
 *   bnpi.c
 */
  
#include <stdio.h>
#include "miracl.h"

/* Access global variables */

extern  int depth;     /* error tracing .. */
extern  int trace[];   /* ... mechanism    */
extern  int nib;       /* length of bigs   */
extern  int lg2b;      /* bits in base     */

extern big  w11,w12,w13,w15; /* workspace variables */

void fpi(pi)
flash pi;
{ /* Calculate pi using Guass-Legendre method */
    int x,nits,op[5];
    if (ERNUM) return;
    depth++;
    trace[depth]=53;
    if (TRACER) track();
    if (size(w15)!=0)
    {
        copy(w15,pi);
        EXACT=FALSE;
        depth--;
        return;
    }
    fconv(1,2,pi);
    froot(pi,2,pi);
    fconv(1,1,w11);
    fconv(1,4,w12);
    x=1;
    op[0]=0x6C;
    op[1]=1;
    op[4]=0;
    nits=lg2b*nib/4;
    while (x<nits)
    {
        copy(w11,w13);
        op[2]=1;
        op[3]=2;
        flop(w11,pi,op,w11);
        fmul(pi,w13,pi);
        froot(pi,2,pi);
        fsub(w11,w13,w13);
        fmul(w13,w13,w13);
        op[3]=1;
        op[2]=(-x);
        flop(w12,w13,op,w12);  /* w12 = w12 - x.w13 */
        x*=2;
    }
    fadd(w11,pi,pi);
    fmul(pi,pi,pi);
    op[0]=0x48;
    op[2]=0;
    op[3]=4;
    flop(pi,w12,op,pi);   /* pi = pi/(4.w12) */
    copy(pi,w15);
    depth--;
}

