/*
 *   MIRACL Chinese Remainder Thereom routines (for use with big moduli) 
 *   mrcrt.c
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <miracl.h>

/* access global variables */

extern int mr_depth;    /* error tracing ..*/
extern int mr_trace[];  /* .. mechanism    */
extern big mr_w1,mr_w2;    /* workspace       */

static big *C;
static big *V;
static big *M;
static int NP=0;

bool crt_init(r,moduli)
int r;
big *moduli;
{ /* calculate CRT constants */
    int i,j,k;
    if (r<2 || MR_ERNUM) return FALSE;
    M=moduli;
    for (i=0;i<r;i++) if (size(M[i])<2) return FALSE;
    NP=r;
    mr_depth++;
    mr_trace[mr_depth]=72;
    if (TRACER) track();
    C=(big *)calloc(NP*(NP-1)/2,sizeof(big));
    if (C==NULL)
    {
        NP=0;
        berror(8);
        mr_depth--;
        return FALSE;
    }
    V=(big *)calloc(NP,sizeof(big));
    if (V==NULL)
    {
        NP=0;
        free(C);
        berror(8);
        mr_depth--;
        return FALSE;
    }
    for (k=0,i=0;i<NP;i++)
    { 
        V[i]=mirvar(0);
        for (j=0;j<i;j++,k++)
        {
            C[k]=mirvar(0);
            xgcd(M[j],M[i],C[k],C[k],C[k]);
        }
    }    
    mr_depth--;
    return TRUE;
}

void crt(u,x)
big *u;
big x;
{ /* Chinese Remainder Thereom                  *
   * Calculate x given remainders u[i] mod M[i] */
    int i,j,k;
    if (NP<2 || MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=73;
    if (TRACER) track();
    copy(u[0],V[0]);
    for (k=0,i=1;i<NP;i++)
    { /* Knuth P. 274 */
        subtract(u[i],V[0],V[i]);
        mad(V[i],C[k],C[k],M[i],M[i],V[i]);
        k++;
        for (j=1;j<i;j++,k++)
        {
            subtract(V[i],V[j],V[i]);
            mad(V[i],C[k],C[k],M[i],M[i],V[i]);
        }
        if (size(V[i])<0) add(V[i],M[i],V[i]);
    }
    zero(x);
    convert(1,mr_w1);
    for (i=0;i<NP;i++)
    {
        multiply(mr_w1,V[i],mr_w2);
        add(x,mr_w2,x);         
        multiply(mr_w1,M[i],mr_w1);
    }   
    mr_depth--;
}

void crt_end()
{ /* clean up after CRT */
    int i,j,k;
    if (NP<2) return;
    for (k=0,i=0;i<NP;i++)
    {
        for (j=0;j<i;j++)
            free(C[k++]); 
        free(V[i]);
    }   
    free(V);
    free(C);
    NP=0;
}

