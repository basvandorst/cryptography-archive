/*
 *   MIRACL I/O routines 2. 
 *   mrio2.c
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <miracl.h>

/* Access global variables */

extern small  mr_base;       /* number base      */
extern small  mr_apbase;     /* apparent base    */
extern int    mr_depth;      /* error tracing .. */
extern int    mr_trace[];    /* .. mechanism     */
extern bool   mr_check;      /* error checking   */
extern bool   mr_strout;  /* Output to string   */
extern bool   mr_strin;   /* Input from string  */
extern unsigned char *mr_inbuff; /* Input buffer       */
extern unsigned char *mr_otbuff; /* Output buffer      */
extern int mr_nib;
extern small MR_OBITS;

#ifdef MR_FLASH
extern unsigned int MR_MSK;
extern int MR_BTS;
#endif

extern big mr_w0;
extern big mr_w1;        /* workspace variables */
extern big mr_w5;
extern big mr_w6;
extern big mr_w7;

static void cbase(x,oldbase,y)
big x,y;
small oldbase;
{  /*  change radix of x from oldbase to base  */
    int i,s;
    small n;
    bool done;
    if (MR_ERNUM) return;
    if (mr_base==oldbase)
    {
        copy(x,y);
        return;
    }
    mr_depth++;
    mr_trace[mr_depth]=13;
    if (TRACER) track();
    s=exsign(x);
#ifdef MR_FLASH
    numer(x,mr_w1);
    done=FALSE;
#else
    copy(x,mr_w1);
    done=TRUE;
#endif
    insign(PLUS,mr_w1);
    forever
    {
        zero(mr_w6);
        convert(1,mr_w0);
        for (i=1;i<=(int)mr_w1[0];i++)  
        {
            pmul(mr_w0,mr_w1[i],mr_w7);
            add(mr_w6,mr_w7,mr_w6);
            if (oldbase==0)
            { /* bit of a frig! */
                n=((small)1<<(MR_SBITS/2));
                pmul(mr_w0,n,mr_w0);
                pmul(mr_w0,n,mr_w0);
            }
            else pmul(mr_w0,oldbase,mr_w0);
        }
        if (MR_ERNUM || done) break;
#ifdef MR_FLASH
        denom(x,mr_w1);
        copy(mr_w6,mr_w5);
        done=TRUE;
#endif
    }
#ifdef MR_FLASH
    fpack(mr_w5,mr_w6,y);
#else
    copy(mr_w6,y);
#endif
    insign(s,y);
    mr_depth--;
}

int cinnum(x,filep)
flash x;
FILE *filep;
{  /*  input big number in base IOBASE  */
    small newb,oldb,b,lx;
    int ipt;
    if (MR_ERNUM) return 0;
    mr_depth++;
    mr_trace[mr_depth]=14;
    if (TRACER) track();
    newb=IOBASE;
    oldb=mr_apbase;
    setbase(newb); /* temporarily change base ... */
    b=mr_base;
    mr_check=OFF;
    ipt=innum(mr_w5,filep); /* ... and get number  */
    mr_check=ON;
    lx=(mr_w5[0]&MR_OBITS);
#ifdef MR_FLASH
    if ((int)(lx&MR_MSK)>mr_nib || (int)((lx>>MR_BTS)&MR_MSK)>mr_nib)
#else
    if (lx>mr_nib)
#endif
    { /* numerator or denominator too big */
        berror(3);
        mr_depth--;
        return 0;
    }
    setbase(oldb);      /* restore original base */
    cbase(mr_w5,b,x);
    mr_depth--;
    return ipt;
}   

int cotnum(x,filep)
flash x;
FILE *filep;
{  /*  output a big number in base IOBASE  */
    small newb,oldb,b;
    int ipt;
    if (MR_ERNUM) return 0;
    mr_depth++;
    mr_trace[mr_depth]=15;
    if (TRACER) track();
    newb=IOBASE;
    oldb=mr_apbase;
    b=mr_base;
    setbase(newb);   /* temporarily change base ... */
    mr_check=OFF;
    cbase(x,b,mr_w5);   /* number may get bigger !     */   
    mr_check=ON;
    ipt=otnum(mr_w5,filep); /*..... and output number  */ 
    zero(mr_w5);
    setbase(oldb);
    mr_depth--;
    return ipt;
}

int cinstr(x,string)
flash x;
char *string;
{ /* convert from string to flash x */
    int n;
    if (MR_ERNUM) return 0;
    mr_depth++;
    mr_trace[mr_depth]=78;
    if (TRACER) track();
    mr_inbuff=(unsigned char *)string;
    mr_strin=TRUE;
    n=cinnum(x,NULL);
    mr_strin=FALSE;
    mr_depth--;
    return n;
}

int cotstr(x,string)
flash x;
char *string;
{ /* convert flash to string */
    int n;
    if (MR_ERNUM) return 0;
    mr_depth++;
    mr_trace[mr_depth]=77;
    if (TRACER) track();
    mr_otbuff=(unsigned char *)string;
    mr_strout=TRUE;
    n=cotnum(x,NULL);
    mr_strout=FALSE;
    mr_depth--;
    return n;
}

