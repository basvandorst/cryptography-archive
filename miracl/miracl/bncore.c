/*
 *   MIRACL Core module - contains initialisation code and general purpose 
 *   utilities.
 *   bncore.c
 */

#include <stdio.h>
#include "mirdef.h"
#define abs(x)  ((x)<0? (-(x)) : (x))
#define sign(x) ((x)<0? (-1) : 1)

#define NK      55
#define NJ      24
#define NV      21
                   /* Constants for random number   *
                    * generator. See Knuth Chapt. 3 */
typedef small *big;
typedef small *flash;

/* Declare global variables */

small base;        /* number base     */
small apbase;      /* apparent base   */
int pack;          /* packing density */
int lg2b;          /* bits in base    */
small base2;       /* 2^lg2b          */
int nib;           /* length of bigs  */
int workprec;
int stprec;        /* start precision */
int depth;         /* error tracing ..*/
int trace[20];     /* .. mechanism    */
bool check;        /* overflow check  */
long ira[NK];      /* random number   */
int rndptr;        /* array & pointer */
char *names[] =
{"your program","innum","otnum","cinbin","cotbin",
"multiply","divide","incr","decr","premult",
"subdiv","fdsize","gcd","cbase",
"cinnum","cotnum","root","power",
"powmod","bigdig","bigrand","nxprime","prime",
"mirvar","mad","mirsys","putdig",
"add","subtract","select","xgcd",
"fpack","dconv","shift","round","fmul",
"fdiv","fadd","fsub","fcomp","fconv",
"frecip","fpmul","fincr","fsize","ftrunc",
"frand","strongp","build","log2","exp2",
"fpower","froot","fpi","fexp","flog","fpowf",
"ftan","fatan","fsin","fasin","fcos","facos",
"ftanh","fatanh","fsinh","fasinh","fcosh",
"facosh","flop","gprime"}; /* 70 */

big w0;            /* workspace bigs  */
big w1,w2,w3,w4;
big w5,w6,w7;
big w8,w9,w10,w11;
big w12,w13,w14,w15;

unsigned char IBUFF[IBSIZ+1]; /* input buffer    */
unsigned char OBUFF[OBSIZ+1]; /* output buffer   */
bool ERCON;        /* error control   */
int  ERNUM;        /* last error code */
bool STROUT;       /* redirect output */
int  LINE;         /* Output line length */
int  IOBASE;       /* base for input and output */
bool WRAP;         /* =ON for wrapround on output, otherwise =OFF       */
bool EXACT;        /* exact flag      */
bool POINT;        /* =ON for radix point, =OFF for fractions in output */
int  NTRY;         /* no. of tries for probablistic primality testing   */
bool TRACER;       /* turns trace tracker on/off */
int  INPLEN;       /* input length               */

extern char *calloc();

void berror(nerr)
int nerr;
{  /*  Big number error routine  */
int i;
if (ERCON)
{
    ERNUM=nerr;
    return;
}
printf("MIRACL error from routine %s\n",
names[trace[depth]]);
for (i=depth-1;i>=0;i--)
printf("              called from %s\n",
names[trace[i]]);
switch (nerr)
{
case 1 :
printf("Number base too big for representation\n");
exit(0);
case 2 :
printf("Division by zero attempted\n");
exit(0);
case 3 : 
printf("Overflow\n");
exit(0);
case 4 :
printf("Internal result is negative\n");
exit(0);
case 5 : 
printf("Input format error\n");
exit(0);
case 6 :
printf("Illegal number base\n");
exit(0);
case 7 : 
printf("Illegal parameter usuage\n");
exit(0);
case 8 :
printf("Out of space\n");
exit(0);
case 9 :
printf("Even root of a negative number\n");
exit(0);
case 10:
printf("Raising integer to negative power\n");
exit(0);
case 11:
printf("Attempt to take illegal root\n");
exit(0);
case 12:
printf("Integer operation attempted on Flash number\n");
exit(0);
case 13:
printf("Flash overflow\n");
exit(0);
case 14:
printf("Numbers too big\n");
exit(0);
case 15:
printf("Log of a non-positive number\n");
exit(0);
case 16:
printf("Double to flash conversation failure\n");
exit(0);
case 17:
printf("I/O buffer overflow\n");
exit(0);
}
}

void track()
{ /* track course of program execution *
   * through the MIRACL routines       */
    int i;
    for (i=0;i<depth;i++) putchar('-');
    putchar('>');
    printf("%s\n",names[trace[depth]]);
}

small brand()
{ /* random number generator - uses Knuth's *
   * subtractive method                     */
    int i;
    small t;
    long k;   
    rndptr++;
    t=(small)ira[rndptr];
    if (rndptr<NK) return abs(t);
    rndptr=0;
    for (i=0;i<NJ;i++)
    {
        k=ira[i]-ira[i+NK-NJ];
        if (k<0) k+=MAXNUM;
        ira[i]=k;
    }
    for (i=NJ;i<NK;i++)
    {
        k=ira[i]-ira[i-NJ];
        if (k<0) k+=MAXNUM;
        ira[i]=k;
    }
    t=(small)ira[0];
    return abs(t);
}

void irand(seed)
long seed;
{ /* initialise random number system */
    int i,in;
    long k;
    k=1L;
    ira[0]=seed;
    for (i=1;i<NK;i++)
    {
        in=(NV*i)%NK;
        ira[in]=k;
        k=seed-k;
        if (k<0) k+=MAXNUM;
        seed=ira[in];
    }
    for (i=1;i<4;i++)
    { /* warm-up the generator */
        rndptr=NK;
        brand();
    }
}

void setbase(nb)
small nb;
{  /* set base. Pack as many digits as  *
    * possible into each computer word  */
    small temp;
    apbase=nb;
    pack=1;
    base=nb;
    temp=MAXBASE/nb;
    while (temp>=nb)
    {
        temp/=nb;
        base*=nb;
        pack++;
    }
}

bool fit(x,y,f)
big x,y;
int f;
{ /* returns TRUE if x/y would fit flash format of length f */
    int n,d;
    n=abs(x[0]);
    d=abs(y[0]);
    if (n==1 && x[1]==1) n=0;
    if (d==1 && y[1]==1) d=0;
    if (n+d<=f) return TRUE;
    return FALSE;
}

int lent(x)
flash x;
{ /* return length of big or flash in words */
    small lx;
    lx=abs(x[0]);
    return (int)((lx&MSK)+((lx>>BTS)&MSK));
}

void zero(x)
flash x;
{ /* set big/flash number to zero */
    int i,n;
    if (ERNUM) return;
    n=lent(x);
    for (i=0;i<=n;i++)
        x[i]=0;
}

void convert(n,x)
big x;
small n;
{  /*  convert integer n to big number format  */
    int m,s;
    zero(x);
    if (n==0) return;
    s=sign(n);
    n=abs(n);
    m=0;
    while (n>0)
    {
        m++;
        x[m]=n%base;
        n/=base;
    }
    x[0]=m*s;
}

void lconv(n,x)
big x;
long n;
{ /* convert long integer to big number format */
    int m,s;
    zero(x);
    if (n==0) return;
    s=sign(n);
    n=abs(n);
    m=0;
    while (n>0)
    {
        m++;
        x[m]=n%base;
        n/=base;
    }
    x[0]=m*s;
}

flash mirvar(iv)
small iv;
{ /* initialize big/flash number */
    flash x;
    if (ERNUM) return x;
    depth++;
    trace[depth]=23;
    if (TRACER) track();
    x=(small *)calloc(nib+1,sizeof(small));
    if (x==NULL)
    {
        berror(8);
        depth--;
        return x;
    }
    convert(iv,x);
    depth--;
    return x;
}

void mirsys(nd,nb)
int nd;
small nb;
{  /*  Initialize MIRACL system to   *
    *  use numbers to base nb, and   *
    *  nd digits or (-nd) bytes long */
    small b;
    depth=0;
    trace[0]=0;
    depth++;
    trace[depth]=25;
    if (nb<2 || nb>MAXBASE)
    {
        berror(6);
        depth--;
        return;
    }
    setbase(nb);
    b=base;
    lg2b=0;
    base2=1;
    while (b>1)
    {
        b/=2;
        lg2b++;
        base2*=2;
    }
    if (nd>0)
        nib=(nd-1)/pack+1;
    else
        nib=(lg2b-8*nd-1)/lg2b;
    if (nib<2) nib=2;
    workprec=nib;
    stprec=nib;
    while (stprec>WPERD) stprec=(stprec+1)/2;
    check=ON;
    IOBASE=10;   /* defaults */
    WRAP=ON;
    STROUT=FALSE;
    ERCON=FALSE;
    ERNUM=0;
    POINT=OFF;
    NTRY=6;           /* <=27 */
    EXACT=TRUE;
    IBUFF[0]='\0';
    TRACER=OFF;
    LINE=80;
    INPLEN=0;
    irand(0L);     /* start rand number generator */
    nib*=2;
    if (nib!=(nib&MSK) || nib>TOOBIG)
    {
        berror(14);
        nib/=2;
        depth--;
        return;
    }
    w0=mirvar((small)0); /* w0 is double length  */
    nib/=2;
    w1=mirvar((small)0); /* initialize workspace */
    w2=mirvar((small)0);
    w3=mirvar((small)0);
    w4=mirvar((small)0);
    nib*=2;              /* double length */
    w5=mirvar((small)0);
    w6=mirvar((small)0);
    w7=mirvar((small)0);
    nib/=2;
    w8=mirvar((small)0);
    w9=mirvar((small)0);
    w10=mirvar((small)0);
    w11=mirvar((small)0);
    w12=mirvar((small)0);
    w13=mirvar((small)0);
    w14=mirvar((small)0);
    w15=mirvar((small)0); /* used to store pi */
    depth--;
} 

int exsign(x)
flash x;
{ /* extract sign of big/flash number */
    return sign(x[0]);
}

void insign(s,x)
flash x;
int s;
{  /* assert sign of big/flash number */
    x[0]=s*abs(x[0]);
}   

void lzero(x)
big x;
{  /*  strip leading zeros from big number  */
    int m;
    bool neg;
    if (x[0]<0)
    {
        m=(-x[0]);
        neg=TRUE;
    }
    else
    {
        m=x[0];
        neg=FALSE;
    }
    while (m>0 && x[m]==0)
        m--;
    if (neg) x[0]=(-m);
    else     x[0]=m;
}

int getdig(x,i)
big x;
int i;
{  /* extract a packed digit */
    int k;
    small n;
    i--;
    n=x[i/pack+1];
    k=i%pack;
    for (i=1;i<=k;i++)
        n/=apbase;
    return n%apbase;
}

int numdig(x)
big x;
{  /* returns number of digits in x */
    int nd;
    nd=abs(x[0])*pack;
    while (getdig(x,nd)==0)
        nd--;
    return nd;
} 

void putdig(n,x,i)  
big x;
int i;
int n;
{  /* insert a digit into a packed word */
    int j,k,lx;
    small m,p;
    if (ERNUM) return;
    depth++;
    trace[depth]=26;
    if (TRACER) track();
    lx=abs(x[0]);
    m=getdig(x,i);
    p=n;
    i--;
    j=i/pack+1;
    k=i%pack;
    for (i=1;i<=k;i++)
    {
        m*=apbase;
        p*=apbase;
    }
    if (check && j>nib)
    {
        berror(3);
        depth--;
        return;
    }
    x[j]=x[j]-m+p;
    if (j>lx) x[0]=j*exsign(x);
    lzero(x);
    depth--;
}

void copy(x,y)
flash x;
flash y;
{  /* copy x to y: y=x  */
    int i,nx,ny;
    if (ERNUM || x==y) return;
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
    y[0]=(-y[0]);
}

void absol(x,y)
flash x;
flash y;
{ /* y=abs(x) */
    copy(x,y);
    y[0]=abs(y[0]);
}

bool notint(x)
flash x;
{ /* returns TRUE if x is Flash */
    if (((abs(x[0])>>BTS)&MSK)!=0) return TRUE;
    return FALSE;
}

void shift(x,n,w)
big x,w;
int n;
{ /* set w=x.(base^n) by shifting */
    int i,bl;
    copy(x,w);
    if (ERNUM || w[0]==0 || n==0) return;
    depth++;
    trace[depth]=33;
    if (TRACER) track();
    if (notint(w)) berror(12);
    bl=abs(w[0])+n;
    if (check && bl>nib) berror(3);
    if (ERNUM)
    {
       depth--;
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
    w[0]=bl*exsign(w);
    depth--;
}

int size(x)
big x;
{  /*  get size of big number;  convert to *
    *  integer - if possible               */
    int s;
    small m;
    s=sign(x[0]);
    m=abs(x[0]);
    if (m==0) return 0;
    if (m==1 && x[1]<TOOBIG) return s*x[1];
    return s*TOOBIG;
}

int compare(x,y)
big x;
big y;
{  /* compare x and y: =1 if x>y  =-1 if x<y *
    *  =0 if x=y                             */
    int m,s;
    if (x==y) return 0;
    if (x[0]>y[0]) return 1;
    if (x[0]<y[0]) return -1;
    s=sign(x[0]);
    m=abs(x[0]);
    while (m>0)
    { /* check digit by digit */
        if (x[m]>y[m]) return s;
        if (x[m]<y[m]) return -s;
        m--;
    }
    return 0;
}

void fpack(n,d,x)
big n,d;
flash x;
{ /* create floating-slash number x=n/d from *
   * big integer numerator and denominator   */
    int i,s,ld,ln;
    if (ERNUM) return;
    depth++;
    trace[depth]=31;
    if (TRACER) track();
    ld=abs(d[0]);
    if (ld==0) berror(13);
    if (x==d) berror(7);
    if (notint(n) || notint(d)) berror(12);
    s=exsign(n);
    ln=abs(n[0]);
    if (abs(size(n))==1) ln=0;
    if (ld+ln>nib) berror(13);
    if (ERNUM)
    {
       depth--;
       return;
    }
    copy(n,x);
    if (size(n)==0)
    {
        depth--;
        return;
    }
    s*=exsign(d);
    if (abs(size(d))==1)
    {
        insign(s,x);
        depth--;
        return;
    }
    for (i=1;i<=ld;i++)
        x[ln+i]=d[i];
    x[0]=s*(ln+((small)ld<<BTS));
    depth--;
}

void numer(x,y)
flash x;
big y;
{ /* extract numerator of x */
    int i,s,ln,ld;
    small ly;
    if (ERNUM) return;
    copy(x,y);
    if (notint(y))
    {
        s=exsign(y);
        ly=abs(y[0]);
        ln=(ly&MSK);
        if (ln==0)
        {
            convert((small)s,y);
            return;
        }
        ld=((ly>>BTS)&MSK);
        for (i=1;i<=ld;i++)
            y[ln+i]=0;
        y[0]=ln*s;
    }
}

void denom(x,y)
flash x;
big y;
{ /* extract denominator of x */
    int i,ln,ld;
    small ly;
    if (ERNUM) return;
    if (!notint(x))
    {
        convert((small)1,y);
        return;
    }
    copy(x,y);
    ly=abs(y[0]);
    ln=(ly&MSK);
    ld=((ly>>BTS)&MSK);
    y[0]=ld;
    if (ln==0) return;
    for (i=1;i<=ld;i++)
        y[i]=y[ln+i];
    for (i=1;i<=ln;i++)
        y[ld+i]=0;
}

small igcd(x,y)
small x,y;
{ /* integer GCD, returns GCD of x and y */
    small r;
    if (y==0) return x;
    while ((r=x%y)!=0)
        x=y,y=r;
    return y;
}

small isqrt(num,guess)
small num,guess;
{ /* square root of an integer */
    small sqr;
    forever
    { /* Newtons iteration */
        sqr=guess+(((num/guess)-guess)/2);
        if (sqr==guess) return sqr;
        guess=sqr;
    }
}

