/*
 *   MIRACL flash random number routine 
 *   mrfrnd.c
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <miracl.h>

#ifdef MR_FLASH

/* access global variables */

extern int mr_depth;    /* error tracing ..*/
extern int mr_trace[];  /* .. mechanism    */
extern int mr_nib;      /* length of bigs  */
extern small mr_base;   /* number base     */
extern bool mr_check;   /* error checking  */

extern big mr_w5;
extern big mr_w6;

void frand(x)
flash x;
{ /* generates random flash number 0<x<1 */
    int i;
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=46;
    if (TRACER) track();
    zero(mr_w6);
    mr_w6[0]=mr_nib;
    for (i=1;i<=mr_nib;i++) 
    { /* generate a full width random number */
        if (mr_base==0) mr_w6[i]=brand();
        else            mr_w6[i]=brand()%mr_base;
    }
    mr_check=OFF;
    bigrand(mr_w6,mr_w5);
    mr_check=ON;
    round(mr_w5,mr_w6,x);
    mr_depth--;
}

#endif
