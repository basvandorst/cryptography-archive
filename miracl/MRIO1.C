/*
 *   MIRACL I/O routines 1. 
 *   mrio1.c
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <miracl.h>

/* Access global variables */

extern small mr_apbase;    /* apparent base      */
extern int mr_depth;       /* error tracing ..   */
extern int mr_trace[];     /* .. mechanism       */
extern int mr_pack;        /* packing density    */
extern int mr_nib;         /* length of bigs     */
extern bool mr_check;      /* overflow check     */
extern bool mr_strout;  /* Output to string   */
extern bool mr_strin;   /* Input from string  */
extern unsigned char *mr_inbuff; /* Input buffer       */
extern unsigned char *mr_otbuff; /* Output buffer      */
extern bool miracl_active;

#ifdef MR_FLASH
extern int MR_BTS;  
extern unsigned int MR_MSK;
#endif

extern big mr_w5;        /* workspace variables */
extern big mr_w6;
extern big mr_w0;

int innum(x,filep)
flash x;
FILE *filep;
{  /*  input a big number       *
    *  returns length in digits */
    int i,ipt,n,s,e,ch;
    bool frac;
    if (MR_ERNUM) return 0;
    mr_depth++;
    mr_trace[mr_depth]=1;
    if (TRACER) track();
    if (mr_apbase==0 || mr_apbase>256)
    {
        berror(1);
        mr_depth--;
        return 0;
    }
    if (!miracl_active)
    {
        berror(18);
        mr_depth--;
        return 0;
    }
    zero(x);
    if (!mr_strin) mr_inbuff=MR_IOBUFF;
    if (INPLEN==0)
    { /* inputting ASCII bytes */
        if (!mr_strin)
        { /* read in characters */
            i=0;
            do
            {
                ch=fgetc(filep);
                if (ch==EOF) break;
                mr_inbuff[i++]=(unsigned char)ch;
                if (i>=MR_IOBSIZ)
                {
                    berror(17);
                    mr_depth--;
                    return 0;
                }
            } while (ch!='\n' && ch!='\0'); 
            mr_inbuff[i]='\0';
        }
        forever
        { /*  get input length  */
            ch=mr_inbuff[INPLEN];
            if (ch=='\0') break;
            if (mr_apbase<=60 && ch=='\n') break;
            INPLEN++;
            if (mr_inbuff==MR_IOBUFF && INPLEN>=MR_IOBSIZ)
            {
                berror(17);
                mr_depth--;
                return 0;
            }
        }
    }
    else
    { /* inputting BINARY bytes */
        if (mr_inbuff==MR_IOBUFF && INPLEN>=MR_IOBSIZ)
        {
            berror(17);
            mr_depth--;
            return 0;
        }
        if (!mr_strin) for(i=0;i<INPLEN;i++) 
        {
            if ((ch=fgetc(filep))==EOF)
            {
                INPLEN=i;
                break;
            }
            mr_inbuff[i]=(char)ch;
        }
    }
    n=0;
    s=PLUS;
    e=0;
#ifdef MR_FLASH
    frac=FALSE;
#endif
    if (INPLEN>0 && mr_apbase<=60)
    { /* skip leading blanks and check sign */
#ifdef MR_FLASH
        if (mr_inbuff[INPLEN-1]=='/') INPLEN--;
#endif
        while (mr_inbuff[e]==' ') e++;
        if (mr_inbuff[e]=='-')
        { /* check sign */
             s=MINUS;
             e++;
        }
        if (mr_inbuff[e]=='+') e++;
    }
    for (i=INPLEN-1;i>=e;i--)
    {
        ch=mr_inbuff[i];
        if (mr_apbase<=60)
        { /* check for slash or dot and convert character to number */
#ifdef MR_FLASH
            if (!frac)
            {
                if (ch=='/')
                {
                    frac=TRUE;
                    copy(x,mr_w0);
                    zero(x);    
                    n=0;
                    continue;
                }
                if (ch=='.')
                {
                    frac=TRUE;
                    zero(mr_w0);
                    putdig(1,mr_w0,n+1);
                    continue;
                }
            }
#endif
            ch+=80;
            if (ch>127 && ch<138) ch-=128;
            if (ch>144 && ch<171) ch-=135;
            if (ch>176 && ch<203) ch-=141;
        }
        if ((small)ch>=mr_apbase)
        {
            berror(5);
            mr_depth--;
            return 0;
        }
        n++;
        putdig(ch,x,n);
    }
    ipt=INPLEN;
    INPLEN=0;
    insign(s,x);
    lzero(x);
#ifdef MR_FLASH
    lzero(mr_w0);
    if (frac) fpack(x,mr_w0,x);
#endif
    mr_depth--;
    return ipt;
}

int otnum(x,filep)
flash x;
FILE *filep;
{  /*  output a big number  */
    int s,i,n,ch,rp,nd,m,nw,dw;
    small lx;
    bool done;
    if (MR_ERNUM) return 0;
    mr_depth++;
    mr_trace[mr_depth]=2;
    if (TRACER) track();
    if (mr_apbase==0 || mr_apbase>256)
    {
        berror(1);
        mr_depth--;
        return 0;
    }
    if (!miracl_active)
    {
        berror(18);
        mr_depth--;
        return 0;
    }
    n=0;
    s=exsign(x);
    insign(PLUS,x);
    lx = x[0];
    if (lx==0 && mr_apbase<=60)
    {
        if (mr_strout)
        {
            mr_otbuff[0]='0';
            mr_otbuff[1]='\0';
        }
        else
        {
            putc('0',filep);
            putc('\n',filep);
        }
        mr_depth--;
        return 1;
    }
    rp=0;
    if (s==MINUS && mr_apbase<=60)
    {
        if (mr_strout) mr_otbuff[n]='-';
        else putc('-',filep);
        n++;
    }
#ifdef MR_FLASH
    done=FALSE;
    numer(x,mr_w6);
    if (POINT)
    { /* output with radix point */
        denom(x,mr_w5);
        if (size(mr_w5)>1)
        { /* multiply up numerator to get full precision in *
           * the output. Remember position of radix point.  */
            nw=(int)(lx&MR_MSK);
            dw=(int)((lx>>MR_BTS)&MR_MSK);
            if (nw==0) nw++;
            mr_check=OFF;
            if (nw>dw) shift(mr_w5,nw-dw,mr_w5);
            if (dw>nw) shift(mr_w6,dw-nw,mr_w6);
            nd=mr_nib;
            if (compare(mr_w6,mr_w5)>=0) nd--;
            copy(mr_w6,mr_w0);
            if (((int)mr_w0[0]+nd)>2*mr_nib) nd=2*mr_nib-(int)mr_w0[0];
            shift(mr_w0,nd,mr_w0);
            divide(mr_w0,mr_w5,mr_w6);
            mr_check=ON;
            rp=mr_pack*(nd+dw-nw);
        }
    }
#else
    copy(x,mr_w6);
    done=TRUE;
#endif

    forever
    {
        nd=numdig(mr_w6);
        m=nd;
        if (rp>m) m=rp;
        for (i=m;i>0;i--)
        { 
            if (mr_strout && mr_otbuff==MR_IOBUFF && n>=MR_IOBSIZ-3)
            {
                berror(17);
                mr_depth--;
                return n;
            }
#ifdef MR_FLASH
            if (i==rp && mr_apbase<=60)
            {
                if (mr_strout) mr_otbuff[n]='.';
                else putc('.',filep);
                n++;
            }
#endif
            if (i>nd) ch='0';
            else
            {
                ch=getdig(mr_w6,i);
                mr_check=OFF;
                putdig(0,mr_w6,i);
                mr_check=ON;
                if (mr_apbase<=60)
                { /* convert number to character */
                    ch+=48;
                    if (ch>=58) ch+=7;
                    if (ch>=91) ch+=6;
                }
            }
            if (i<rp && mr_apbase<=60 && ch=='0' && size(mr_w6)==0) break;
            if (mr_strout) mr_otbuff[n]=(char)ch;
            else putc((char)ch,filep);
            n++;
        }
        if (done || POINT) break;
#ifdef MR_FLASH
        denom(x,mr_w6);
        if (size(mr_w6)==1) break;
        if (mr_apbase<=60)
        {
            if (mr_strout) mr_otbuff[n]='/';
            else putc('/',filep);
            n++;
        }
        done=TRUE;
#endif
    }
    if (mr_strout) mr_otbuff[n]='\0';
    else if(mr_apbase<=60) putc('\n',filep);
    insign(s,x);
    mr_depth--;
    return n;
}

int instr(x,string)
flash x;
char *string;
{ /* convert from string to flash x */
    int n;
    if (MR_ERNUM) return 0;
    mr_depth++;
    mr_trace[mr_depth]=76;
    if (TRACER) track();
    mr_inbuff=(unsigned char *)string;
    mr_strin=TRUE;
    n=innum(x,NULL);
    mr_strin=FALSE;
    mr_depth--;
    return n;
}

int otstr(x,string)
flash x;
char *string;
{ /* convert flash to string */
    int n;
    if (MR_ERNUM) return 0;
    mr_depth++;
    mr_trace[mr_depth]=75;
    if (TRACER) track();
    mr_otbuff=(unsigned char *)string;
    mr_strout=TRUE;
    n=otnum(x,NULL);
    mr_strout=FALSE;
    mr_depth--;
    return n;
}

