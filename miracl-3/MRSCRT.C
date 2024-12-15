/*
 *   MIRACL Chinese Remainder Thereom routines (for use with small moduli) 
 *   mrscrt.c
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <miracl.h>

/* access global variables */

extern big mr_w1,mr_w2;    /* workspace       */

static utype *C;
static utype *V;
static utype *M;
static int NP=0;

bool scrt_init(r,moduli)
int r;
utype *moduli;
{ /* calculate CRT constants - returns FALSE if there is a problem */
    int i,j,k;
    if (r<2) return FALSE;
    M=moduli;
    for (i=0;i<r;i++) if (M[i]<2) return FALSE;
    NP=r;
    C=(utype *)calloc(NP*(NP-1)/2,sizeof(utype));
    if (C==NULL)
    { /* no room */
        NP=0;
        return FALSE;
    }
    V=(utype *)calloc(NP,sizeof(utype));
    if (V==NULL)
    { /* no room */
        free(C);
        NP=0;
        return FALSE;
    }
    for (k=0,i=0;i<NP;i++)
        for (j=0;j<i;j++,k++)
            C[k]=invers(M[j],M[i]);
    return TRUE;
}

void scrt(u,x)
utype *u;
big x;
{ /* Chinese Remainder Thereom                  *
   * Calculate x given remainders u[i] mod M[i] */
    int i,j,k;
    if (NP<2) return;
    V[0]=u[0];
    for (k=0,i=1;i<NP;i++)
    { /* Knuth P. 274 */
        V[i]=u[i]-V[0];
        V[i]=smul(V[i],C[k],M[i]);
        k++;
        for (j=1;j<i;j++,k++)
        {
            V[i]-=V[j];
            V[i]=smul(V[i],C[k],M[i]);
        }
    }
    zero(x);
    convert(1,mr_w1);
    for (i=0;i<NP;i++)
    {
        pmul(mr_w1,(small)V[i],mr_w2);
        add(x,mr_w2,x);         
        pmul(mr_w1,(small)M[i],mr_w1);
    }   
}

void scrt_end()
{ /* clean up after CRT */
    if (NP<2) return;
    free(V);
    free(C);
    NP=0;
}

