/*
 *   MIRACL Core module - contains initialisation code and general purpose 
 *   utilities.
 *   mrcore.c
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <mirdef.h>

/* Marsaglia & Zaman Random number generator */
/*         constants      alternatives       */
#define NK   37           /* 21 */
#define NJ   24           /*  6 */
#define NV   14           /*  8 */

                   /* some useful definitions */
#define bool int
#define FALSE 0
#define TRUE 1
#define OFF 0
#define ON 1
#define PLUS 1
#define MINUS (-1)

typedef small *big;
typedef small *flash;

/* Declare global variables */

small MR_MSBIT;         /* msb of small    */
small MR_OBITS;         /* other bits      */

#ifdef MR_FLASH
int          MR_BTS;   
unsigned int MR_MSK;
#endif

small mr_base;          /* number base     */
small mr_apbase;        /* apparent base   */
int   mr_pack;          /* packing density */
int   mr_lg2b;          /* bits in base    */
small mr_base2;         /* 2^mr_lg2b          */
int   mr_nib;           /* length of bigs  */
int   mr_workprec;
int   mr_stprec;        /* start precision */
int   mr_depth;         /* error tracing ..*/
int   mr_trace[30];     /* .. mechanism    */
bool  mr_check;         /* overflow check  */
bool  mr_strout;        /* Output to string   */
bool  mr_strin;         /* Input from string  */
unsigned char  *mr_inbuff;    /* Input buffer       */
unsigned char  *mr_otbuff;    /* Output buffer      */
bool  miracl_active=OFF;

static unsigned long ira[NK];  /* random number...   */
static int           rndptr=0; /* ...array & pointer */
static unsigned long borrow=0L;

#ifndef MR_STRIPPED_DOWN

static char *names[] =
{"your program","innum","otnum","jack","normalise",
"multiply","divide","incr","decr","premult",
"subdiv","fdsize","egcd","cbase",
"cinnum","cotnum","nroot","power",
"powmod","bigdig","bigrand","nxprime","isprime",
"mirvar","mad","mirsys","putdig",
"add","subtract","select","xgcd",
"fpack","dconv","shift","round","fmul",
"fdiv","fadd","fsub","fcomp","fconv",
"frecip","fpmul","fincr","fsize","ftrunc",
"frand","sftbit","build","log2","exp2",
"fpower","froot","fpi","fexp","flog","fpowf",
"ftan","fatan","fsin","fasin","fcos","facos",
"ftanh","fatanh","fsinh","fasinh","fcosh",
"facosh","flop","gprime","powltr","fastmult",
"crt_init","crt","otstr","instr","cotstr","cinstr"}; /* 0 - 78 (79 in all) */

#endif

big mr_w0;            /* workspace bigs  */
big mr_w1,mr_w2,mr_w3,mr_w4;
big mr_w5,mr_w6,mr_w7;

#ifdef MR_FLASH
big mr_w8,mr_w9,mr_w10,mr_w11;
big mr_w12,mr_w13,mr_w15;
#endif

unsigned char MR_IOBUFF[MR_IOBSIZ+1]; /* i/o buffer    */
bool MR_ERCON;     /* error control   */
int  MR_ERNUM;     /* last error code */
int  MR_NTRY;      /* no. of tries for probablistic primality testing   */
int  TOOBIG;       /* bigger than int */
int  IOBASE;       /* base for input and output */
bool EXACT;        /* exact flag      */
bool POINT;        /* =ON for radix point, =OFF for fractions in output */
bool TRACER;       /* turns trace tracker on/off */
int  INPLEN;       /* input length               */
int *PRIMES;       /* small primes array         */

static void mputs(s)
char s[];
{ /* output a string */
    int i=0;
    while (s[i]!=0) putchar(s[i++]);
}

void berror(nerr)
int nerr;
{  /*  Big number error routine  */

int i;
if (MR_ERCON)
{
    MR_ERNUM=nerr;
    return;
}
#ifndef MR_STRIPPED_DOWN
mputs("\nMIRACL error from routine ");
mputs(names[mr_trace[mr_depth]]);
putchar('\n');

for (i=mr_depth-1;i>=0;i--)
{
    mputs("              called from ");
    mputs(names[mr_trace[i]]);
    putchar('\n');
}

switch (nerr)
{
case 1 :
mputs("Number base too big for representation\n");
exit(0);
case 2 :
mputs("Division by zero attempted\n");
exit(0);
case 3 : 
mputs("Overflow - Number too big\n");
exit(0);
case 4 :
mputs("Internal result is negative\n");
exit(0);
case 5 : 
mputs("Input format error\n");
exit(0);
case 6 :
mputs("Illegal number base\n");
exit(0);
case 7 : 
mputs("Illegal parameter usage\n");
exit(0);
case 8 :
mputs("Out of space\n");
exit(0);
case 9 :
mputs("Even root of a negative number\n");
exit(0);
case 10:
mputs("Raising integer to negative power\n");
exit(0);
case 11:
mputs("Attempt to take illegal root\n");
exit(0);
case 12:
mputs("Integer operation attempted on Flash number\n");
exit(0);
case 13:
mputs("Flash overflow\n");
exit(0);
case 14:
mputs("Numbers too big\n");
exit(0);
case 15:
mputs("Log of a non-positive number\n");
exit(0);
case 16:
mputs("Double to flash conversation failure\n");
exit(0);
case 17:
mputs("I/O buffer overflow\n");
exit(0);
case 18:
mputs("MIRACL not initialised - no call to mirsys()\n");
exit(0);
}
#else
mputs("MIRACL error\n");
exit(0);
#endif
}

void track()
{ /* track course of program execution *
   * through the MIRACL routines       */

#ifndef MR_STRIPPED_DOWN
    int i;
    for (i=0;i<mr_depth;i++) putchar('-');
    putchar('>');
    mputs(names[mr_trace[mr_depth]]);
    putchar('\n');
#endif

}

small brand()
{ /* Marsaglia & Zaman random number generator */
    int i,k;
    unsigned long pdiff,t;
    rndptr++;
    if (rndptr<NK) return (small)ira[rndptr];
    rndptr=0;
    for (i=0,k=NK-NJ;i<NK;i++,k++)
    { /* calculate next NK values */
        if (k==NK) k=0;
        t=ira[k];
        pdiff=t-ira[i]-borrow;
        if (pdiff<t) borrow=0;
        if (pdiff>t) borrow=1;
        ira[i]=pdiff; 
    }
    return (small)ira[0];
}

void irand(seed)
unsigned long seed;
{ /* initialise random number system */
    int i,in;
    unsigned long m=1L;
    borrow=0L;
    rndptr=0;
    ira[0]=seed;
    for (i=1;i<NK;i++)
    { /* fill initialisation vector */
        in=(NV*i)%NK;
        ira[in]=m;
        m=seed-m;
        seed=ira[in];
    }
    for (i=0;i<1000;i++) brand(); /* "warm-up" the generator */
}

void setbase(nb)
small nb;
{  /* set base. Pack as many digits as  *
    * possible into each computer word  */
    small temp;
#ifdef FULLWIDTH
    bool fits;
    int bits;
    fits=FALSE;
    bits=MR_SBITS;
    while (bits>1) 
    {
        bits/=2;
        temp=((small)1<<bits);
        if (temp==nb)
        {
            fits=TRUE;
            break;
        }
        if (temp<nb || (bits%2)!=0) break;
    }
    if (fits)
    {
        mr_apbase=nb;
        mr_pack=MR_SBITS/bits;
        mr_base=0;
        return;
    }
#endif
    mr_apbase=nb;
    mr_pack=1;
    mr_base=nb;
    if (mr_base==0) return;
    temp=MAXBASE/nb;
    while (temp>=nb)
    {
        temp=(temp/nb);
        mr_base*=nb;
        mr_pack++;
    }
}

#ifdef MR_FLASH

bool fit(x,y,f)
big x,y;
int f;
{ /* returns TRUE if x/y would fit flash format of length f */
    int n,d;
    n=(int)(x[0]&MR_OBITS);
    d=(int)(y[0]&MR_OBITS);
    if (n==1 && x[1]==1) n=0;
    if (d==1 && y[1]==1) d=0;
    if (n+d<=f) return TRUE;
    return FALSE;
}

#endif

int lent(x)
flash x;
{ /* return length of big or flash in words */
    small lx;
    lx=(x[0]&MR_OBITS);
#ifdef MR_FLASH
    return (int)((lx&MR_MSK)+((lx>>MR_BTS)&MR_MSK));
#else
    return lx;
#endif
}

void zero(x)
flash x;
{ /* set big/flash number to zero */
    int i,n;
    if (MR_ERNUM) return;
    n=lent(x);
    for (i=0;i<=n;i++)
        x[i]=0;
}

void convert(n,x)
big x;
int n;
{  /*  convert integer n to big number format  */
    int m;
    small s;
    zero(x);
    if (n==0) return;
    s=0;
    if (n<0)
    {
        s=MR_MSBIT;
        n=(-n);
    }
    m=0;
    if (mr_base==0)
    {
        m=1;
        x[m]=n;
    }
    else while (n>0)
    {
        m++;
        x[m]=n%mr_base;
        n/=mr_base;
    }
    x[0]=(m|s);
}

void lgconv(n,x)
big x;
long n;
{ /* convert long integer to big number format - rarely needed */
    int m;
    small s;
    long b;
    zero(x);
    if (n==0) return;
    s=0;
    if (n<0)
    {
        s=MR_MSBIT;
        n=(-n);
    }
    m=0;
    if (mr_base==0)
    {
        if (MR_LBITS>MR_SBITS)
        {
            b=(1L<<MR_SBITS);
            while (n>0)
            {
                m++;
                x[m]=(small)(n%b);
                n/=b;
            }
        }
        else
        {
            m=1;
            x[m]=(small)n;
        }
    }    
    else while (n>0)
    {
        m++;
        x[m]=(small)(n%mr_base);
        n/=mr_base;
    }
    x[0]=(m|s);
}

flash mirvar(iv)
int iv;
{ /* initialize big/flash number */
    flash x;
    if (MR_ERNUM) return NULL;
    mr_depth++;
    mr_trace[mr_depth]=23;
    if (TRACER) track();
    if (!miracl_active)
    {
        berror(18);
        mr_depth--;
        return NULL;
    }
    x=(small *)calloc(mr_nib+1,sizeof(small));
    if (x==NULL)
    {
        berror(8);
        mr_depth--;
        return x;
    }
    convert(iv,x);
    mr_depth--;
    return x;
}

void mirsys(nd,nb)
int nd;
small nb;
{  /*  Initialize MIRACL system to   *
    *  use numbers to base nb, and   *
    *  nd digits or (-nd) bytes long */
    small b;
    mr_depth=0;
    mr_trace[0]=0;
    mr_depth++;
    mr_trace[mr_depth]=25;
                                       /* digest hardware configuration */
    if (MR_SBITS>=MR_IBITS) TOOBIG =(1<<(MR_IBITS-2));
    else              TOOBIG =(1<<(MR_SBITS-1));

#ifdef MR_FLASH
    MR_BTS=MR_SBITS/2;
    MR_MSK=(1<<MR_BTS)-1;
#endif

    MR_MSBIT=((small)1<<(MR_SBITS-1));
    MR_OBITS=MR_MSBIT-1;
#ifndef FULLWIDTH
    if (nb==0)
    {
        berror(6);
        mr_depth--;
        return;
    }
#endif
    if (nb==1 || nb>MAXBASE)
    {
        berror(6);
        mr_depth--;
        return;
    }
    setbase(nb);
    b=mr_base;
    mr_lg2b=0;
    mr_base2=1;
    if (b==0)
    {
        mr_lg2b=MR_SBITS;
        mr_base2=0;
    }
    else while (b>1)
    {
        b/=2;
        mr_lg2b++;
        mr_base2*=2;
    }
    if (nd>0)
        mr_nib=(nd-1)/mr_pack+1;
    else
        mr_nib=(mr_lg2b-8*nd-1)/mr_lg2b;
    if (mr_nib<2) mr_nib=2;
    mr_workprec=mr_nib;
    mr_stprec=mr_nib;
    while (mr_stprec>MR_MBITS/mr_lg2b) mr_stprec=(mr_stprec+1)/2;
    if (mr_stprec<2) mr_stprec=2;
    mr_check=ON;
    IOBASE=10;   /* defaults */
    MR_ERCON=FALSE;
    MR_ERNUM=0;
    POINT=OFF;
    MR_NTRY=6;
    EXACT=TRUE;
    MR_IOBUFF[0]='\0';
    TRACER=OFF;
    INPLEN=0;
    PRIMES=NULL;
    irand(0L);     /* start rand number generator */
    mr_nib=2*mr_nib+1;
#ifdef MR_FLASH
    if (mr_nib!=(mr_nib&MR_MSK) || mr_nib>TOOBIG)
#else
    if (mr_nib!=(mr_nib&MR_OBITS) || mr_nib>TOOBIG)
#endif
    {
        berror(14);
        mr_nib=(mr_nib-1)/2;
        mr_depth--;
        return;
    }
    mr_strin=FALSE;
    mr_strout=FALSE;
    miracl_active=ON;
    mr_w0=mirvar((small)0); /* w0 is double length  */
    mr_nib=(mr_nib-1)/2;
    mr_w1=mirvar((small)0); /* initialize workspace */
    mr_w2=mirvar((small)0);
    mr_w3=mirvar((small)0);
    mr_w4=mirvar((small)0);
    mr_nib=2*mr_nib+1;             /* double length */
    mr_w5=mirvar((small)0);
    mr_w6=mirvar((small)0);
    mr_w7=mirvar((small)0);
    mr_nib=(mr_nib-1)/2;
#ifdef MR_FLASH
    mr_w8=mirvar((small)0);
    mr_w9=mirvar((small)0);
    mr_w10=mirvar((small)0);
    mr_w11=mirvar((small)0);
    mr_w12=mirvar((small)0);
    mr_w13=mirvar((small)0);
    mr_w15=mirvar((small)0); /* used to store pi */
#endif
    mr_depth--;
} 

void mirexit()
{ /* clean up after miracl */
    int i;
    miracl_active=OFF;
    zero(mr_w0);
    zero(mr_w1);
    zero(mr_w2);
    zero(mr_w3);
    zero(mr_w4);
    zero(mr_w5);
    zero(mr_w6);
    zero(mr_w7);
    free(mr_w0);
    free(mr_w1);
    free(mr_w2);
    free(mr_w3);
    free(mr_w4);
    free(mr_w5);
    free(mr_w6);
    free(mr_w7);
#ifdef MR_FLASH
    zero(mr_w8);
    zero(mr_w9);
    zero(mr_w10);
    zero(mr_w11);
    zero(mr_w12);
    zero(mr_w13);
    zero(mr_w15);
    free(mr_w8);
    free(mr_w9);
    free(mr_w10);
    free(mr_w11);
    free(mr_w12);
    free(mr_w13);
    free(mr_w15);
#endif
    for (i=0;i<MR_IOBSIZ;i++) MR_IOBUFF[i]=0;
    if (PRIMES!=NULL) free(PRIMES);
}

int exsign(x)
flash x;
{ /* extract sign of big/flash number */
    if ((x[0]&MR_MSBIT)==0) return PLUS;
    else                 return MINUS;    
}

void insign(s,x)
flash x;
int s;
{  /* assert sign of big/flash number */
    if (x[0]==0) return;
    if (s<0) x[0]|=MR_MSBIT;
    else     x[0]&=MR_OBITS;
}   

void lzero(x)
big x;
{  /*  strip leading zeros from big number  */
    small s;
    int m;
    s=(x[0]&MR_MSBIT);
    m=(int)(x[0]&MR_OBITS);
    while (m>0 && x[m]==0)
        m--;
    x[0]=m;
    if (m>0) x[0]|=s;
}

int getdig(x,i)
big x;
int i;
{  /* extract a packed digit */
    int k;
    small n;
    i--;
    n=x[i/mr_pack+1];
    if (mr_pack==1) return (int)n;
    k=i%mr_pack;
    for (i=1;i<=k;i++)
        n=(n/mr_apbase);
    return (int)(n%mr_apbase);
}

int numdig(x)
big x;
{  /* returns number of digits in x */
    int nd;
    nd=(int)(x[0]&MR_OBITS)*mr_pack;
    while (getdig(x,nd)==0)
        nd--;
    return nd;
} 

void putdig(n,x,i)  
big x;
int i,n;
{  /* insert a digit into a packed word */
    int j,k,lx;
    small s,m,p;
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=26;
    if (TRACER) track();
    s=(x[0]&MR_MSBIT);
    lx=(int)(x[0]&MR_OBITS);
    m=getdig(x,i);
    p=n;
    i--;
    j=i/mr_pack+1;
    k=i%mr_pack;
    for (i=1;i<=k;i++)
    {
        m*=mr_apbase;
        p*=mr_apbase;
    }
    if (j>mr_nib && (mr_check || j>2*mr_nib))
    {
        berror(3);
        mr_depth--;
        return;
    }
    x[j]=(x[j]-m)+p;
    if (j>lx) x[0]=(j|s);
    lzero(x);
    mr_depth--;
}

void copy(x,y)
flash x;
flash y;
{  /* copy x to y: y=x  */
    int i,nx,ny;
    if (MR_ERNUM || x==y) return;
    ny=lent(y);
    nx=lent(x);
    for (i=nx+1;i<=ny;i++)
        y[i]=0;
    for (i=0;i<=nx;i++)
        y[i]=x[i];
}

void negate(x,y)
flash x;
flash y;
{ /* negate a big/flash variable: y=-x */
    copy(x,y);
    if (y[0]!=0) y[0]^=MR_MSBIT;
}

void absol(x,y)
flash x;
flash y;
{ /* y=abs(x) */
    copy(x,y);
    y[0]&=MR_OBITS;
}

bool notint(x)
flash x;
{ /* returns TRUE if x is Flash */
#ifdef MR_FLASH
    if ((((x[0]&MR_OBITS)>>MR_BTS)&MR_MSK)!=0) return TRUE;
#endif
    return FALSE;
}

void shift(x,n,w)
big x,w;
int n;
{ /* set w=x.(mr_base^n) by shifting */
    small s;
    int i,bl;
    copy(x,w);
    if (MR_ERNUM || w[0]==0 || n==0) return;
    mr_depth++;
    mr_trace[mr_depth]=33;
    if (TRACER) track();
    if (notint(w)) berror(12);
    s=(w[0]&MR_MSBIT);
    bl=(int)(w[0]&MR_OBITS)+n;
    if (bl<=0)
    {
        zero(w);
        mr_depth--;
        return;
    }
    if (bl>mr_nib && mr_check) berror(3);
    if (MR_ERNUM)
    {
        mr_depth--;
        return;
    }
    if (n>0)
    {
        for (i=bl;i>n;i--)
            w[i]=w[i-n];
        for (i=1;i<=n;i++)
            w[i]=0;
    }
    else
    {
        n=(-n);
        for (i=1;i<=bl;i++)
            w[i]=w[i+n];
        for (i=1;i<=n;i++)
            w[bl+i]=0;
    }
    w[0]=(bl|s);
    mr_depth--;
}

int size(x)
big x;
{  /*  get size of big number;  convert to *
    *  integer - if possible               */
    int n;
    small s,m;
    s=(x[0]&MR_MSBIT);
    m=(x[0]&MR_OBITS);
    if (m==0) return 0;
    if (m==1 && x[1]<(small)TOOBIG) n=(int)x[1];
    else                            n=TOOBIG;
    if (s==MR_MSBIT) return (-n);
    return n;
}

int compare(x,y)
big x;
big y;
{  /* compare x and y: =1 if x>y  =-1 if x<y *
    *  =0 if x=y                             */
    int m,n,sig;
    small sx,sy;
    if (x==y) return 0;
    sx=(x[0]&MR_MSBIT);
    sy=(y[0]&MR_MSBIT);
    if (sx==0) sig=PLUS;
    else       sig=MINUS;
    if (sx!=sy) return sig;
    m=(int)(x[0]&MR_OBITS);
    n=(int)(y[0]&MR_OBITS);
    if (m>n) return sig;
    if (m<n) return -sig;
    while (m>0)
    { /* check digit by digit */
        if (x[m]>y[m]) return sig;
        if (x[m]<y[m]) return -sig;
        m--;
    }
    return 0;
}

#ifdef MR_FLASH

void fpack(n,d,x)
big n,d;
flash x;
{ /* create floating-slash number x=n/d from *
   * big integer numerator and denominator   */
    small s;
    int i,ld,ln;
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=31;
    if (TRACER) track();
    ld=(int)(d[0]&MR_OBITS);
    if (ld==0) berror(13);
    if (ld==1 && d[1]==1) ld=0;
    if (x==d) berror(7);
    if (notint(n) || notint(d)) berror(12);
    s=(n[0]&MR_MSBIT);
    ln=(int)(n[0]&MR_OBITS);
    if (ln==1 && n[1]==1) ln=0;
    if ((ld+ln>mr_nib) && (mr_check || ld+ln>2*mr_nib)) berror(13);
    if (MR_ERNUM)
    {
       mr_depth--;
       return;
    }
    copy(n,x);
    if (n[0]==0)
    {
        mr_depth--;
        return;
    }
    s^=(d[0]&MR_MSBIT);
    if (ld==0)
    {
        if (x[0]!=0) x[0]|=s;
        mr_depth--;
        return;
    }
    for (i=1;i<=ld;i++)
        x[ln+i]=d[i];
    x[0]=(s|(ln+((small)ld<<MR_BTS)));
    mr_depth--;
}

void numer(x,y)
flash x;
big y;
{ /* extract numerator of x */
    int i,ln,ld;
    small ly,s;
    if (MR_ERNUM) return;
    if (notint(x))
    {
        s=(x[0]&MR_MSBIT);
        ly=(x[0]&MR_OBITS);
        ln=(int)(ly&MR_MSK);
        if (ln==0)
        {
            if(s==MR_MSBIT) convert((-1),y);
            else         convert(1,y);
            return;
        }
        ld=(int)((ly>>MR_BTS)&MR_MSK);
        if (x!=y)
        {
            for (i=1;i<=ln;i++) y[i]=x[i];
            for (i=ln+1;i<=lent(y);i++) y[i]=0;
        }
        else for (i=1;i<=ld;i++) y[ln+i]=0;
        y[0]=(ln|s);
    }
    else copy(x,y);
}

void denom(x,y)
flash x;
big y;
{ /* extract denominator of x */
    int i,ln,ld;
    small ly;
    if (MR_ERNUM) return;
    if (!notint(x))
    {
        convert(1,y);
        return;
    }
    ly=(x[0]&MR_OBITS);
    ln=(int)(ly&MR_MSK);
    ld=(int)((ly>>MR_BTS)&MR_MSK);
    for (i=1;i<=ld;i++)
        y[i]=x[ln+i];
    if (x==y) for (i=1;i<=ln;i++) y[ld+i]=0;
    else for (i=ld+1;i<=lent(y);i++) y[i]=0;
    y[0]=ld;
}

#endif

int igcd(x,y)
int x,y;
{ /* integer GCD, returns GCD of x and y */
    int r;
    if (y==0) return x;
    while ((r=x%y)!=0)
        x=y,y=r;
    return y;
}

int isqrt(num,guess)
int num,guess;
{ /* square root of an integer */
    int sqr;
    for (;;)
    { /* Newtons iteration */
        sqr=guess+(((num/guess)-guess)/2);
        if (sqr==guess) return sqr;
        guess=sqr;
    }
}

