#ifndef MIRACL
#define MIRACL

/*
 *   main MIRACL header - miracl.h.
 *   Use this file unmodified as main 'miracl.h' header if  using
 *   an ANSI enhanced type C compiler. This will cause
 *   the compiler to generate a warning if you call a
 *   function with the wrong type of parameter - very useful.
 *
 *   IMPORTANT NOTE!
 *
 *   If using an older compiler, delete the parameter declarations
 *   e.g. change  - extern void berror(int);
 *            to  - extern void berror();
 *   (A parameter declaration in file mrbuild.c will also have to be removed)
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <mirdef.h>

               /* some useful definitions */

#define bool int
#define forever for(;;)   
#define FALSE 0
#define TRUE 1
#define OFF 0
#define ON 1
#define PLUS 1
#define MINUS (-1)

/* big and flash variables consist of an array of smalls */

typedef small *big;
typedef small *flash;

/* Globals. Do not use these names for any other purpose in your program */

extern bool TRACER;  /* turns track tracer on/off                    */
extern int  TOOBIG;  /* Number too big to be an int */
extern int  IOBASE;  /* base for I/O       */
extern bool POINT;   /* =ON for radix point, =OFF for fractions      */
extern bool EXACT;   /* exact flag         */
extern int  INPLEN;  /* input length (for binary input)              */
extern int *PRIMES;  /* array of small primes           */
extern bool MR_ERCON;/* error control, TRUE for user processing      */
extern int  MR_ERNUM;/* last recorded error       */
extern int  MR_NTRY; /* no. of tries for prob. primality testing     */
extern unsigned char MR_IOBUFF[];  /* I/O buffer        */

/* Function definitions  */

/* Group 0 - Internal routines */

extern void  berror(int);                  /* MIRACL error routine           */
extern void  setbase(small);               /* set internal number base       */
extern void  track(void);                  /* routine tracker                */
extern void  lzero(big);                   /* suppress leading zeros         */
extern bool  notint(flash);                /* tests for flash or big         */
extern int   lent(flash);                  /* length of big/flash            */
extern void  padd(big,big,big);            /* primitive add                  */
extern void  psub(big,big,big);            /* primitive subtract             */
extern void  pmul(big,small,big);          /* primitive multiply             */
extern small sdiv(big,small,big);          /* primitive divide               */
extern void  shift(big,int,big);           /* shift left or right by words   */ 
extern small muldiv(small,small,small,small,small *);
                                           /* int multiply, add and divide   */
extern small muldvm(small,small,small,small *); /* special routines for ..   */
extern small muldvd(small,small,small,small *); /* ..full-width base working */
extern int   igcd(int,int);                /* integer GCD                    */
extern int   isqrt(int,int);               /* integer square root            */
extern small normalise(big,big);           /* normalise divisor              */

/* Group 1 - General purpose, I/O and basic arithmetic routines  */

extern void  irand(long);                  /* initialize random numbers      */
extern small brand(void);                  /* integer random numbers         */
extern void  zero(flash);                  /* set flash number to zero       */
extern void  convert(int,big);             /* convert int number to big      */
extern void  lgconv(long,big);             /* convert long to big            */
extern flash mirvar(int);                  /* initialize big/flash number    */
extern void  mirsys(int,small);            /* initialize miracl system       */
extern void  mirexit(void);                /* exit miracl system             */
extern int   exsign(flash);                /* extract sign of flash number   */
extern void  insign(int,flash);            /* assert sign of flash number    */
extern int   getdig(big,int);              /* extract digit from big number  */
extern int   numdig(big);                  /* return number of digits in big */
extern void  putdig(int,big,int);          /* insert digit into big number   */
extern void  copy(flash,flash);            /* copy one flash to another      */
extern void  negate(flash,flash);          /* negate flash number            */
extern void  absol(flash,flash);           /* absolute value of flash number */
extern int   size(big);                    /* convert big number to int      */
extern int   compare(big,big);             /* compare two big numbers        */
extern void  add(big,big,big);             /* add two big numbers            */
extern void  subtract(big,big,big);        /* subtract two big numbers       */
extern void  incr(big,int,big);            /* add int number to big          */
extern void  decr(big,int,big);            /* subtract int number from big   */
extern void  premult(big,int,big);         /* multiply big number by int     */
extern int   subdiv(big,int,big);          /* divide big number by int       */
extern void  multiply(big,big,big);        /* multiply two big numbers       */
extern void  fastmult(big,big,big);        /* fast fourier multiplication    */
extern void  divide(big,big,big);          /* divide big number by another   */
extern void  mad(big,big,big,big,big,big); /* multiply, add and divide       */
extern int   innum(flash,FILE *);          /* input  flash number            */
extern int   otnum(flash,FILE *);          /* output flash number            */
extern int   instr(flash,char *);          /* input from string              */
extern int   otstr(flash,char *);          /* output to string               */
extern int   cinnum(flash,FILE *);         /* input  flash number - any base */
extern int   cotnum(flash,FILE *);         /* output flash number - any base */
extern int   cinstr(flash,char *);         /* input from string   - any base */
extern int   cotstr(flash,char *);         /* output to string    - any base */

/* Group 2 - Advanced arithmetic routines */

extern utype smul(utype,utype,utype);      /* multiply two ints mod int      */
extern utype spmd(utype,utype,utype);      /* raise int to int power mod int */
extern utype invers(utype,utype);          /* inverse mod a int              */
extern utype sqrmp(utype,utype);           /* square root mod a prime        */
extern void  gprime(int);                  /* prime number generator         */
extern int   jack(big,big);                /* jacobi symbol                  */
extern int   egcd(big,big,big);            /* find GCD of two big numbers    */
extern int   xgcd(big,big,big,big,big);    /* extended GCD routine           */
extern int   logb2(big);                   /* returns no. of bits in a big   */
extern void  expb2(int,big);               /* raises 2 to int power          */
extern void  sftbit(big,int,big);          /* shift left or right by bits    */
extern void  power(big,int,big,big);       /* raise big to int power mod big */
extern void  powmod(big,big,big,big);      /* raise big to big power mod big */
extern void  powltr(int,big,big,big);      /* raise int to big power mod big */
extern bool  nroot(big,int,big);           /* find n-th root of big number   */
extern void  bigrand(big,big);             /* generate big random number     */
extern void  bigdig(int,small,big);        /* find random big of fixed size  */
extern bool  isprime(big);                 /* test big number for primality  */
extern void  nxprime(big,big);             /* find next big prime number     */
extern bool  crt_init(int,big *);          /* Initialize for CRT  (bigs)     */
extern void  crt(big *,big);               /* Chinese Remainder Thereom      */
extern void  crt_end(void);                /* finish with CRT                */
extern bool  scrt_init(int,utype *);       /* Initialize for CRT  (smalls)   */
extern void  scrt(utype *,big);            /* Chinese Remainder Thereom      */
extern void  scrt_end(void);               /* finish with CRT                */

/* Group 3 - Floating-slash routines      */

#ifdef MR_FLASH
extern void  fpack(big,big,flash);         /* make flash number from bigs    */
extern void  numer(flash,big);             /* extract flash numerator        */
extern void  denom(flash,big);             /* extract flash denominator      */
extern bool  fit(big,big,int);             /* Test if two bigs fit a flash   */
extern void  build(flash,int (*)(big,int));/* build flash from c.f generator */
extern void  round(big,big,flash);         /* make rounded flash from 2 bigs */
extern void  flop(flash,flash,int *,flash);/* primitive flash operations     */
extern void  fmul(flash,flash,flash);      /* multiply flash nos.            */
extern void  fdiv(flash,flash,flash);      /* divide flash numbers           */
extern void  fadd(flash,flash,flash);      /* add flash numbers              */
extern void  fsub(flash,flash,flash);      /* subtract flash nos.            */
extern int   fcomp(flash,flash);           /* compare flash numbers          */
extern void  fconv(int,int,flash);         /* convert fraction to flash      */
extern void  frecip(flash,flash);          /* flash reciprical               */
extern void  ftrunc(flash,big,flash);      /* truncate flash number          */
extern void  fpmul(flash,int,int,flash);   /* multiply flash by fraction     */
extern void  fincr(flash,int,int,flash);   /* increment flash by fraction    */
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
#endif
#endif

