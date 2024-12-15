/*
 *   main MIRACL header - miracl.h.
 *   Use this file unmodified as main 'miracl.h' header if  using
 *   an ANSI enhanced type C compiler. This will cause
 *   the compiler to generate a warning if you call  a
 *   function with the wrong type of parameter - very useful.
 *
 *   If using an older compiler, delete the parameter declarations
 *   e.g. change  - extern void berror(int);
 *            to  - extern void berror();
 */

#include "mirdef.h"

/* big and flash variables consist of an array of smalls */

typedef small *big;
typedef small *flash;

extern bool WRAP;    /* =ON for wraparound on output, else =OFF      */
extern bool ERCON;   /* error control, TRUE for user processing      */
extern int  ERNUM;   /* last recorded error       */
extern bool STROUT;  /* redirect output to OBUFF  */
extern int  IOBASE;  /* base for I/O       */
extern int  LINE;    /* Output line length */ 
extern unsigned char OBUFF[]; /* Output buffer      */
extern unsigned char IBUFF[]; /* Input buffer       */
extern bool POINT;   /* =ON for radix point, =OFF for fractions      */
extern bool EXACT;   /* exact flag         */
extern int  NTRY;    /* no. of tries for prob. primality testing     */
extern bool TRACER;  /* turns track tracer on/off                    */
extern int  INPLEN;  /* input length (for binary input)              */

/* Function definition  */

/* Group 0 - Internal routines */

extern void  berror(int);                  /* MIRACL error routine           */
extern void  setbase(small);               /* set internal number base       */
extern void  track(void);                  /* routine tracker                */
extern void  lzero(big);                   /* suppress leading zeros         */
extern bool  notint(flash);                /* tests for flash or big         */
extern int   lent(flash);                  /* length of big/flash            */
extern void  padd(big,big,big);            /* primitive add                  */
extern void  psub(big,big,big);            /* primitive subtract             */
extern void  select(big,int,big,big);      /* selects padd or psub           */
extern void  shift(big,int,big);           /* shift left or right            */ 
extern small muldiv(small,small,small,small,small *);
                                           /* int multiply, add and divide   */
extern small igcd(small,small);            /* integer GCD                    */
extern small isqrt(small,small);           /* integer square root            */

/* Group 1 - General purpose, I/O and basic arithmetic routines  */

extern void  irand(long);                  /* initialize random numbers      */
extern small brand(void);                  /* integer random numbers         */
extern void  zero(flash);                  /* set flash number to zero       */
extern void  convert(small,big);           /* convert int number to big      */
extern void  lconv(long,big);              /* convert long to big            */
extern flash mirvar(small);                /* initialize big/flash number    */
extern void  mirsys(int,small);            /* initialize system              */
extern int   exsign(flash);                /* extract sign of flash number   */
extern void  insign(int,flash);            /* assert sign of flash number    */
extern int   getdig(big,int);              /* extract digit from big number  */
extern int   numdig(big);                  /* return number of digits in big */
extern void  putdig(int,big,int);          /* insert digit into big number   */
extern void  copy(flash,flash);            /* copy one flash to another      */
extern void  negate(flash,flash);          /* negate flash number            */
extern void  absol(flash,flash);           /* absolute value of flash number */
extern int   size(big);                    /* convert big number to int      */
extern void  fpack(big,big,flash);         /* make flash number from bigs    */
extern void  numer(flash,big);             /* extract flash numerator        */
extern void  denom(flash,big);             /* extract flash denominator      */
extern int   compare(big,big);             /* compare two big numbers        */
extern void  add(big,big,big);             /* add two big numbers            */
extern void  subtract(big,big,big);        /* subtract two big numbers       */
extern void  incr(big,small,big);          /* add int number to big          */
extern void  decr(big,small,big);          /* subtract int number from big   */
extern void  premult(big,small,big);       /* multiply big number by int     */
extern small subdiv(big,small,big);        /* divide big number by int       */
extern void  multiply(big,big,big);        /* multiply two big numbers       */
extern void  divide(big,big,big);          /* divide big number by another   */
extern void  mad(big,big,big,big,big,big); /* multiply, add and divide       */
extern void  build(flash,int (*)());       /* build flash from c.f generator */
extern void  round(big,big,flash);         /* make rounded flash from 2 bigs */
extern int   innum(flash,FILE *);          /* input  flash number            */
extern int   otnum(flash,FILE *);          /* output flash number            */
extern int   cinnum(flash,FILE *);         /* input  flash number - any base */
extern int   cotnum(flash,FILE *);         /* output flash number - any base */

/* Group 2 - Advanced arithmetic routines */

extern small smul(small,small,small);      /* multiply two ints mod int      */
extern small spmd(small,small,small);      /* raise int to int power mod int */
extern small inverse(small,small);         /* inverse mod a small            */
extern small sqrmp(small,small);           /* square root mod a prime        */
extern int  *gprime(int);                  /* prime number generator         */
extern int   gcd(big,big,big);             /* find GCD of two big numbers    */
extern int   xgcd(big,big,big,big,big);    /* extended GCD routine           */
extern int   logb2(big);                   /* returns no. of bits in a big   */
extern void  expb2(big,int);               /* raises 2 to int power          */
extern void  power(big,int,big,big);       /* raise big to int power mod big */
extern void  powmod(big,big,big,big);      /* raise big to big power mod big */
extern bool  root(big,int,big);            /* find root of big number        */
extern void  bigrand(big,big);             /* generate big random number     */
extern void  bigdig(big,int,small);        /* find random big of fixed size  */
extern bool  prime(big);                   /* test big number for primality  */
extern void  nxprime(big,big);             /* find next big prime number     */
extern void  strongp(big,int);             /* generate 'strong' prime        */

/* Group 3 - Floating-slash routines      */

extern void  flop(flash,flash,int *,flash);/* primitive flash operations     */
extern void  fmul(flash,flash,flash);      /* multiply flash nos.            */
extern void  fdiv(flash,flash,flash);      /* divide flash numbers           */
extern void  fadd(flash,flash,flash);      /* add flash numbers              */
extern void  fsub(flash,flash,flash);      /* subtract flash nos.            */
extern int   fcomp(flash,flash);           /* compare flash numbers          */
extern void  fconv(small,small,flash);     /* convert fraction to flash      */
extern void  frecip(flash,flash);          /* flash reciprical               */
extern void  ftrunc(flash,big,flash);      /* truncate flash number          */
extern void fpmul(flash,small,small,flash);/* multiply flash by fraction     */
extern void fincr(flash,small,small,flash);/* increment flash by fraction    */
extern void  dconv(double,flash);          /* convert double to flash        */
extern double fdsize(flash);               /* convert flash to double        */
extern void  frand(flash);                 /* generates flash random 0<x<1   */

/* Group 4 - Advanced Flash routines */ 

extern void  fpower(flash,int,flash);      /* raises flash to integer power  */
extern bool  froot(flash,int,flash);       /* find root of flash number      */
extern void  fpi(flash);                   /* returns pi by Guass-Legendre   */
extern void  fexp(flash,flash);            /* exponentiate flash number      */
extern void  flog(flash,flash);            /* log of flash number to base e  */
extern void  fpowf(flash,flash,flash);     /* flash raised to flash power    */
extern void  ftan(flash,flash);            /* tan of flash number            */
extern void  fatan(flash,flash);           /* arctan of flash number         */
extern void  fsin(flash,flash);            /* sine   of flash number         */
extern void  fasin(flash,flash);           /* arcsin of flash number         */
extern void  fcos(flash,flash);            /* cosine of flash number         */
extern void  facos(flash,flash);           /* arccos of flash number         */
extern void  ftanh(flash,flash);           /* hyperbolic tan of flash number */
extern void  fatanh(flash,flash);          /* hyperbolic arctan              */
extern void  fsinh(flash,flash);           /* hyperbolic sin                 */
extern void  fasinh(flash,flash);          /* hyperbolic arcsin              */
extern void  fcosh(flash,flash);           /* hyperbolic cos                 */
extern void  facosh(flash,flash);          /* hyperbolic arccos              */

