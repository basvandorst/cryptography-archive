/*
 *   MIRACL Jacobi symbol routine
 *   mrjack.c
 *
 *   See "Prime numbers and Computer methods for Factorisation", 
 *   Hans Reisel, Birkhauser 1985, page 286
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <miracl.h>

/* Access global variables */

extern int   mr_depth;    /* error tracing ..*/
extern int   mr_trace[];  /* .. mechanism    */

extern big mr_w1;         /* workspace variables */
extern big mr_w2;
extern big mr_w3;

int jack(x,n)
big x,n;
{ /* find jacobi symbol for x wrt n. Only defined for *
   * positive x and n, n odd. Otherwise returns 0     */
    int m,n8,i2,u4;
    if (MR_ERNUM || size(x)<=0 || size(n) <1) return 0;
    copy(x,mr_w1);
    copy(n,mr_w2);
    mr_depth++;
    mr_trace[mr_depth]=3;
    if (TRACER) track();
    if (subdiv(mr_w2,2,mr_w3)==0)
    { /* n is even */
        mr_depth--;
        return 0;
    }
    m=0;
    while(!MR_ERNUM && size(mr_w2)>1)
    { /* main loop */
        if (size(mr_w1)==0)
        { /* gcd(x,n) > 0 */
            mr_depth--;
            return 0;
        }
        n8=subdiv(mr_w2,8,mr_w3);
        i2=0;
        while (subdiv(mr_w1,2,mr_w3)==0)
        { /* extract powers of 2 */
            i2++;
            copy(mr_w3,mr_w1);
        }
        if ((i2%2)==1) m+=(n8*n8-1)/8;
        u4=subdiv(mr_w1,4,mr_w3);
        m+=(n8-1)*(u4-1)/4;
        copy(mr_w2,mr_w3);
        divide(mr_w3,mr_w1,mr_w1);
        copy(mr_w1,mr_w2);
        copy(mr_w3,mr_w1);
        m%=2;
    }
    mr_depth--;
    if (m%2==0) return 1;
    else return (-1);
} 

