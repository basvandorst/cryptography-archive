/*
 *   MIRACL I/O routines 1. 
 *   bnio1.c
 */

#include <stdio.h>
#include "miracl.h"

/* Access global variables */

extern small base;   /* number base        */
extern small apbase; /* apparent base      */
extern int depth;    /* error tracing ..   */
extern int trace[];  /* .. mechanism       */
extern int pack;     /* packing density    */
extern int nib;      /* length of bigs     */
extern bool check;   /* overflow check     */

extern big w1;        /* workspace variables */
extern big w2;
extern big w3;
extern big w4;
extern big w5;
extern big w6;
extern big w0;

int innum(x,filep)
flash x;
FILE *filep;
{  /*  input a big number       *
    *  returns length in digits */
    int i,ipt,n,s,e,ch;
    bool frac;
    if (ERNUM) return 0;
    depth++;
    trace[depth]=1;
    if (TRACER) track();
    if (apbase>256)
    {
        berror(1);
        depth--;
        return 0;
    }
    zero(x);
    if (INPLEN==0)
    { /* inputting ASCII bytes */
        if (IBUFF[0]==0) fgets(IBUFF,IBSIZ+1,filep);
        forever
        { /*  get input length  */
            ch=IBUFF[INPLEN];
            if (ch=='\0' || ch==EOF) break;
            if (apbase<=60 && ch=='\n') break;
            INPLEN++;
            if (INPLEN>=IBSIZ)
            {
                berror(17);
                depth--;
                return 0;
            }
        }
    }
    else
    { /* inputting BINARY bytes */
        if (INPLEN>=IBSIZ)
        {
            berror(17);
            depth--;
            return 0;
        }
        if (IBUFF[0]==0) for(i=0;i<INPLEN;i++) IBUFF[i]=fgetc(filep);
    }
    n=0;
    s=PLUS;
    e=0;
    frac=FALSE;
    if (INPLEN>0 && apbase<=60)
    { /* skip leading blanks and check sign */
        if (IBUFF[INPLEN-1]=='/') INPLEN--;
        while (IBUFF[e]==' ') e++;
        if (IBUFF[e]=='-')
        { /* check sign */
             s=MINUS;
             e++;
        }
        if (IBUFF[e]=='+') e++;
    }
    for (i=INPLEN-1;i>=e;i--)
    {
        ch=IBUFF[i];
        if (apbase<=60)
        { /* check for slash or dot and convert character to number */
            if (!frac)
            {
                if (ch=='/')
                {
                    frac=TRUE;
                    copy(x,w0);
                    zero(x);    
                    n=0;
                    continue;
                }
                if (ch=='.')
                {
                    frac=TRUE;
                    zero(w0);
                    putdig(1,w0,n+1);
                    continue;
                }
            }
            ch+=80;
            if (ch>127 && ch<138) ch-=128;
            if (ch>144 && ch<171) ch-=135;
            if (ch>176 && ch<203) ch-=141;
        }
        if (ch>=apbase)
        {
            berror(5);
            depth--;
            return 0;
        }
        n++;
        putdig(ch,x,n);
    }
    ipt=INPLEN;
    IBUFF[0]=0;
    INPLEN=0;
    insign(s,x);
    lzero(x);
    lzero(w0);
    if (frac) fpack(x,w0,x);
    depth--;
    return ipt;
}

int otnum(x,filep)
flash x;
FILE *filep;
{  /*  output a big number  */
    int s,i,n,ch,rp,nd,m,nw,dw;
    small lx;
    bool done;
    if (ERNUM) return 0;
    depth++;
    trace[depth]=2;
    if (TRACER) track();
    if (apbase>256)
    {
        berror(1);
        depth--;
        return 0;
    }
    n=0;
    s=exsign(x);
    insign(PLUS,x);
    lx = x[0];
    if (lx==0 && apbase<=60)
    {
        if (STROUT)
        {
            OBUFF[0]='0';
            OBUFF[1]='\0';
        }
        else
        {
            putc('0',filep);
            putc('\n',filep);
        }
        depth--;
        return 1;
    }
    done=FALSE;
    rp=0;
    numer(x,w1);
    if (s==MINUS && apbase<=60)
    {
        if (STROUT) OBUFF[n]='-';
        else putc('-',filep);
        n++;
    }
    if (POINT)
    { /* output with radix point */
        denom(x,w2);
        if (size(w2)>1)
        { /* multiply up numerator to get full precision in *
           * the output. Remember position of radix point.  */
            nw=(lx&MSK);
            dw=((lx>>BTS)&MSK);
            if (nw==0) nw++;
            if (nw>dw) shift(w2,nw-dw,w2);
            if (dw>nw) shift(w1,dw-nw,w1);
            nd=nib;
            if (compare(w1,w2)>=0) nd--;
            check=OFF;
            copy(w1,w0);
            shift(w0,nd,w0);
            divide(w0,w2,w1);
            check=ON;
            rp=pack*(nd+dw-nw);
        }
    }
    forever
    {
        nd=numdig(w1);
        m=nd;
        if (rp>m) m=rp;
        for (i=m;i>0;i--)
        { 
            if (STROUT && n>=OBSIZ-3)
            {
                berror(17);
                depth--;
                return n;
            }
            if (i==rp && apbase<=60)
            {
                if (STROUT) OBUFF[n]='.';
                else putc('.',filep);
                n++;
            }
            if (i>nd) ch='0';
            else
            {
                ch=getdig(w1,i);
                putdig(0,w1,i);
                if (apbase<=60)
                { /* convert number to character */
                    ch+=48;
                    if (ch>=58) ch+=7;
                    if (ch>=91) ch+=6;
                }
            }
            if (i<rp && apbase<=60 && ch=='0' && size(w1)==0) break;
            if (STROUT) OBUFF[n]=(char)ch;
            else putc((char)ch,filep);
            n++;
            if (!STROUT && apbase<=60 && WRAP && (n%(LINE-2)==0))
                                                       fputs("-\n",filep);
        }
        if (done) break;
        denom(x,w1);
        if (POINT || size(w1)==1) break;
        if (apbase<=60)
        {
            if (STROUT) OBUFF[n]='/';
            else putc('/',filep);
            n++;
        }
        done=TRUE;
    }
    if (STROUT) OBUFF[n]='\0';
    else if(apbase<=60) putc('\n',filep);
    insign(s,x);
    depth--;
    return n;
}

