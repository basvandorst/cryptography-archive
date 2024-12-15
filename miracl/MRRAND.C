/*
 *   MIRACL random number routines 
 *   mrrand.c
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <miracl.h>

/* access global variables */

extern int mr_depth;    /* error tracing ..*/
extern int mr_trace[];  /* .. mechanism    */
extern small mr_base;   /* number base     */

extern big mr_w0;  /* work space variables */
extern big mr_w1;

void bigrand(w,x)
big x;
big w;
{  /*  generate a big random number 1<x<w  */
    int m;
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=20;
    if (TRACER) track();
    decr(w,2,w);
    m=0;
    zero(mr_w0);
    do
    { /* create big rand piece by piece */
        m++;
        mr_w0[0]=m;
        if (mr_base==0) mr_w0[m]=brand();
        else            mr_w0[m]=brand()%mr_base;
    } while (compare(mr_w0,w)<0);
    divide(mr_w0,w,w);
    copy(mr_w0,x);
    incr(x,2,x);
    if (w!=x) incr(w,2,w);
    mr_depth--;
}

void bigdig(n,b,x)
int n;
small b;
big x;
{ /* generate random number n digits long *
   * to base b                            */
    int i;
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=19;
    if (TRACER) track();
    do
    { /* repeat if x too small */
        convert(1,mr_w1);
        for (i=1;i<=n;i++) pmul(mr_w1,b,mr_w1);
        bigrand(mr_w1,x);
        sdiv(mr_w1,b,mr_w1);
    } while (!MR_ERNUM && compare(x,mr_w1)<0);
    mr_depth--;
}

