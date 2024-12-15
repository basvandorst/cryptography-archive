/*
 *   MIRACL I/O routines 2. 
 *   bnio2.c
 */

#include <stdio.h>
#include "miracl.h"

/* Access global variables */

extern small  base;     /* number base      */
extern small  apbase;   /* apparent base    */
extern int    depth;    /* error tracing .. */
extern int    trace[];  /* .. mechanism     */

extern big w1;        /* workspace variables */
extern big w2;
extern big w3;
extern big w4;
extern big w5;
extern big w6;

void cbase(x,oldbase)
big x;
small oldbase;
{  /*  change radix of x from oldbase to base  */
    int i,s;
    bool done;
    if (ERNUM || base==oldbase) return;
    depth++;
    trace[depth]=13;
    if (TRACER) track();
    s=exsign(x);
    numer(x,w1);
    insign(PLUS,w1);
    done=FALSE;
    forever
    {
        zero(w6);
        convert((small)1,w3);
        for (i=1;i<=w1[0];i++)  
        {
            premult(w3,w1[i],w2);
            add(w6,w2,w6);
            premult(w3,oldbase,w3);
        }
        if (ERNUM || done) break;
        denom(x,w1);
        copy(w6,w5);
        done=TRUE;
    }
    round(w5,w6,x);
    insign(s,x);
    depth--;
}

int cinnum(x,filep)
flash x;
FILE *filep;
{  /*  input big number in base IOBASE  */
    small newb,oldb,b;
    int ipt;
    if (ERNUM) return 0;
    depth++;
    trace[depth]=14;
    if (TRACER) track();
    newb=IOBASE;
    oldb=apbase;
    setbase(newb); /* temporarily change base ... */
    b=base;
    ipt=innum(x,filep); /* ... and get number  */
    setbase(oldb);      /* restore original base */
    cbase(x,b);
    depth--;
    return ipt;
}   

int cotnum(x,filep)
flash x;
FILE *filep;
{  /*  output a big number in base IOBASE  */
    small newb,oldb,b;
    int ipt;
    if (ERNUM) return 0;
    depth++;
    trace[depth]=15;
    if (TRACER) track();
    newb=IOBASE;
    oldb=apbase;
    b=base;
    setbase(newb);  /* temporarily change base ... */
    cbase(x,b);
    ipt=otnum(x,filep); /* ..X    b=base;
    setbase(oldb);
    cbase(x,b);     /* change back */
    depth--;
    return ipt;
}

