/*
 *   MIRACL random number routines 
 *   bnrand.c
 */

#include <stdio.h>
#include "miracl.h"

/* access global variables */

extern int depth;    /* error tracing ..*/
extern int trace[];  /* .. mechanism    */
extern int nib;      /* length of bigs  */
extern small base;   /* number base     */

extern big w1;       /* workspace variables */
extern big w2;
extern big w5;
extern big w6;

void bigrand(w,x)
big x;
big w;
{  /*  generate a big random number 1<x<w  */
    if (ERNUM) return;
    depth++;
    trace[depth]=20;
    if (TRACER) track();
    decr(w,2,w1);
    convert(brand(),x);
    while (compare(x,w1)<0)
    { /* create big rand piece by piece */
        shift(x,1,x);
        incr(x,brand(),x);
    }
    divide(x,w1,w1);
    incr(x,2,x);
    depth--;
}

void bigdig(x,n,b)
big x;
int n;
small b;
{ /* generate random number n digits long *
   * to base b                            */
    int i;
    if (ERNUM) return;
    depth++;
    trace[depth]=19;
    if (TRACER) track();
    do
    { /* repeat if x too small */
        convert(1,w2);
        for (i=1;i<=n;i++) premult(w2,b,w2);
        bigrand(w2,x);
        subdiv(w2,b,w2);
    } while (!ERNUM && compare(x,w2)<0);
    depth--;
}

void frand(x)
flash x;
{ /* generates random flash number 0<x<1 */
    if (ERNUM) return;
    depth++;
    trace[depth]=46;
    if (TRACER) track();
    bigdig(w6,nib-1,base);
    bigrand(w6,w5);
    round(w5,w6,x);
    depth--;
}

