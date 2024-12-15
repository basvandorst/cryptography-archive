#ifndef mfloat_h
#define mfloat_h
#include <string.h>             /* strlen() */
#ifndef mfloatb
#include <stdlib.h>
#include <stdio.h>
#endif
/* ------------------------------------------------------------------------ */
/* mfloat types */
/* ------------------------------------------------------------------------ */

#ifndef mfloatwords
#define mfloatwords 17                /* maximum precision */
#endif

typedef int mfloata[mfloatwords];     /* mfloat array */
#define mfloat_ int far *             /* pointer to array */
#define mfloat mfloata                /* for use in 'C' programs */

/* ------------------------------------------------------------------------ */
/* compiler control */
/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
  extern "C" {                        /* do not extend names (C++) */
#endif

#ifdef __STDC__                       /* ANSI comatibility mode */
#ifndef __HUGE__
#error  ANSI compatibility only with model 'huge'
#endif
#define far
#undef mfloat_
#define mfloat_ int *
#pragma option -Ff            	 /* automatic far data */

#define pascal
#pragma option -p              	 /* pascal calling convention */
#pragma option -l-c-        	 /* no case sensitive link */

#pragma option -u-            	 /* do not generate underlines */

#define _fstrlen(v) strlen(v)
#endif

/* ------------------------------------------------------------------------ */
/* mfloat basic functions */
/* ------------------------------------------------------------------------ */

extern void      far pascal setmantissawords(int number);
extern int       far pascal getmantissawords(void);
extern void      far pascal mfloatError(int Nr);
extern void      far pascal mfloatTest80386(void);

extern mfloat_   far pascal equm(mfloat_ a,             /* a <- b           */
				const mfloat_ b);
extern mfloat_   far pascal addm(mfloat_ a,             /* a <- a + b       */
				const mfloat_ b);
extern mfloat_   far pascal subm(mfloat_ a,             /* a <- a - b       */
				const mfloat_ b);
extern mfloat_   far pascal multm(mfloat_ a,            /* a <- a * b       */
				const mfloat_ b);
extern mfloat_   far pascal divm(mfloat_ a,             /* a <- a / b       */
				const mfloat_ b);
extern mfloat_   far pascal multi(mfloat_ a, int b);    /* a <- a * b       */
extern mfloat_   far pascal divi(mfloat_ a, int b);     /* a <- a / b       */
extern mfloat_   far pascal inversm(mfloat_ a);         /* a <- 1/a         */
extern mfloat_   far pascal negm(mfloat_ a);            /* a <-  - a        */
extern char      far pascal eqzero(const mfloat_ a);    /* eqZero <- a = 0  */
extern char      far pascal gtzero(const mfloat_ a);    /* gtZero <- a > 0  */
extern char      far pascal gezero(const mfloat_ a);    /* geZero <- a >= 0 */
extern char      far pascal gtm(const mfloat_ a,        /* gtm <- a > b     */
				const mfloat_ b);
extern char      far pascal eqm(const mfloat_ a,        /* eqm <- a = b     */
				const mfloat_ b);
extern mfloat_   far pascal getzerom(mfloat_ a);        /* a <- 0           */
extern mfloat_   far pascal getonem(mfloat_ a);         /* a <- 1           */
extern mfloat_   far pascal getpim(mfloat_ a);          /* a <- pi          */
extern mfloat_   far pascal getln2m(mfloat_ a);         /* a <- ln(2)       */
extern mfloat_   far pascal getln10m(mfloat_ a);        /* a <- ln(10)      */
       int       far pascal strtomf(mfloat_ a,          /* a <- b           */
				const char far * b);
       char far * far pascal mftoa(char far * str,      /* str <- a         */
				const mfloat_ a,
				int len);
       char far * far pascal mftostr(char far * str,    /* str <- a         */
				const mfloat_ a,
				int len,
				const char far * format);
       double    far pascal mftod(const mfloat_ b);     /* mftod <- b       */
       long double far pascal mftold(const mfloat_ b);  /* mftold <- b      */
extern mfloat_   far pascal dtomf(mfloat_ a,            /* a <- b           */
				double b);
extern mfloat_   far pascal ldtomf(mfloat_ a,           /* a <- b           */
				long double b);
extern mfloat_   far pascal inttomf(mfloat_ a, int b);  /* a <- b           */

/* ------------------------------------------------------------------------ */
/* standard functions (Borland C: MATH.H) */
/* ------------------------------------------------------------------------ */
extern mfloat_   far pascal acosm(mfloat_ a);           /* a <- acos(a)     */
extern mfloat_   far pascal asinm(mfloat_ a);           /* a <- asin(a)     */
extern mfloat_   far pascal atanm(mfloat_ a);           /* a <- atan(a)     */
extern mfloat_   far pascal atan2m(mfloat_ a,           /* a <- atan2(a,b)  */
				const mfloat_ b);
/*                          atof                           see strtomf      */
extern mfloat_   far pascal ceilm(mfloat_ a);           /* a <- ceil(a)     */
extern mfloat_   far pascal cosm(mfloat_ a);            /* a <- cos(a)      */
extern mfloat_   far pascal coshm(mfloat_ a);           /* a <- cosh(a)     */
extern mfloat_   far pascal expm(mfloat_ a);            /* a <- exp(a)      */
extern mfloat_   far pascal fabsm(mfloat_ a);           /* a <- fabs(a)     */
extern mfloat_   far pascal floorm(mfloat_ a);          /* a <- floor(a)    */
extern mfloat_   far pascal fmodm(mfloat_ a,            /* a <- fmod(a,b)   */
				const mfloat_ b);
extern mfloat_   far pascal frexpm(mfloat_ a, int *b);  /* a <- frexp(a,b)  */
extern mfloat_   far pascal hypotm(mfloat_ a,           /* a <- hypot(a,b)  */
				const mfloat_ b);
extern mfloat_   far pascal ldexpm(mfloat_ a, int b);   /* a <- ldexp(a,b)  */
extern mfloat_   far pascal ldexpm1(mfloat_ a, int b);   /* a <- ldexp(a,b)  */
extern mfloat_   far pascal logm(mfloat_ a);            /* a <- log(a)      */
extern mfloat_   far pascal log10m(mfloat_ a);          /* a <- log10(a)    */
extern mfloat_   far pascal modfm(mfloat_ a,            /* a <- modf(a,b)   */
				  mfloat_ b);
extern mfloat_   far pascal powm(mfloat_ a,             /* a <- a^b         */
				const mfloat_ b);
extern mfloat_   far pascal powi(mfloat_ a, int b);     /* a <- a^b         */
extern mfloat_   far pascal pow10m(mfloat_ a, int b);   /* a <- 10^b        */
extern mfloat_   far pascal sinm(mfloat_ a);            /* a <- sin(a)      */
extern mfloat_   far pascal sinhm(mfloat_ a);           /* a <- sinh(a)     */
extern mfloat_   far pascal sqrtm(mfloat_ a);           /* a <- sqrt(a)     */
extern mfloat_   far pascal tanm(mfloat_ a);            /* a <- tan(a)      */
extern mfloat_   far pascal tanhm(mfloat_ a);           /* a <- tanh(a)     */

/* ------------------------------------------------------------------------ */
/* extended standard functions */
/* ------------------------------------------------------------------------ */

extern mfloat_   far pascal acoshm(mfloat_ a);          /* a <- acosh(a)    */
extern mfloat_   far pascal acotm(mfloat_ a);           /* a <- acot(a)     */
extern mfloat_   far pascal acothm(mfloat_ a);          /* a <- acoth(a)    */
extern mfloat_   far pascal asinhm(mfloat_ a);          /* a <- asinh(a)    */
extern mfloat_   far pascal atanhm(mfloat_ a);          /* a <- atanh(a)    */
extern mfloat_   far pascal cossinm(mfloat_ a,          /* a <- cos(a),     */
				    mfloat_ b);         /* b <- sin(a)      */
extern mfloat_   far pascal cotm(mfloat_ a);            /* a <- cot(a)      */
extern mfloat_   far pascal cothm(mfloat_ a);           /* a <- coth(a)     */
extern mfloat_   far pascal exp10m(mfloat_ a);          /* a <- 10^a        */
extern mfloat_   far pascal sqrm(mfloat_ a);            /* a <- sqr(a)      */
extern mfloat_   far pascal truncm(mfloat_ a);          /* a <- trunc(a)    */

/* ------------------------------------------------------------------------ */
/* internal functions */
/* ------------------------------------------------------------------------ */

extern void      far pascal setmantissawords_(int number);
extern char far * far pascal mftostr_(char far * a,     /* a <- b           */
				      const mfloat_ b, const mfloat_ bound,
				      int far * len, const char far * format);
extern int       far pascal strtomf_(mfloat_ a,         /* a <- b           */
				     mfloat_ bound,const char far * b,
				     const int len);
extern void      far pascal mftod_(double far * a,      /* a <- b           */
				   const mfloat_ b);
extern void      far pascal mftold_(long double far * a, /* a <- b          */
				    const mfloat_ b);

/* ------------------------------------------------------------------------ */
/* constants */
/* ------------------------------------------------------------------------ */

extern far const pascal mfloata onem;
extern far const pascal mfloata pim;
extern far const pascal mfloata ln2m;
extern far const pascal mfloata ln10m;
extern far const pascal mfloata powten;

/* ------------------------------------------------------------------------ */
/* static variables */
/* ------------------------------------------------------------------------ */


#ifdef mfloatb
/* for internal use only */
extern far char useInterruptAtError;
extern far int pascal mantissawords;
extern far char pascal use386instructions;
#else
/* normal use */
extern far char useInterruptAtError=1;
extern far int pascal mantissawords=1;
extern far char pascal use386instructions=0;
char calculationerror=0;

/* ------------------------------------------------------------------------ */
/* included functions */
/* ------------------------------------------------------------------------ */

extern void far pascal mfloatError(int Nr) {
  /* handling errors of mfloat */

  calculationerror=1;               /* set error flag */
  if (useInterruptAtError > 0) {
    /* The calculation is iterrupted */
    if (useInterruptAtError > 1) Nr=useInterruptAtError;
    printf("\nMFLOAT-error:\n");
    switch (Nr) {
      case 1  : printf("divide by zero\n"); break;
      case 2  : printf("floating point overflow\n"); break;
      case 3  : printf("sqrt DOMAIN error\n"); break;
      case 4  : printf("log DOMAIN error\n"); break;
      case 5  : printf("log SINGULARITY error\n"); break;
      case 6  : printf("log10 DOMAIN error\n"); break;
      case 7  : printf("log10 SINGULARITY error\n"); break;
      case 8  : printf("tan SINGULARITY error\n"); break;
      case 9  : printf("cot SINGULARITY error\n"); break;
      case 10 : printf("asin DOMAIN error\n"); break;
      case 11 : printf("acos DOMAIN error\n"); break;
      case 12 : printf("atan SINGULARITY error\n"); break;
      case 13 : printf("acot SINGULARITY error\n"); break;
      case 14 : printf("atan2 DOMAIN error\n"); break;
      case 15 : printf("tanh SINGULARITY error\n"); break;
      case 16 : printf("coth SINGULARITY error\n"); break;
      case 17 : printf("acosh DOMAIN error\n"); break;
      case 18 : printf("atanh DOMAIN error\n"); break;
      case 19 : printf("atanh SINGULARITY error\n"); break;
      case 20 : printf("acoth DOMAIN error\n"); break;
      case 21 : printf("acoth SINGULARITY error\n"); break;
      case 22 : printf("pow DOMAIN error\n"); break;
      case 23 : printf("pow SINGULARITY error\n"); break;
      case 24 : printf("error in sine series\n"); break;
      case 25 : printf("error in sine hyperbolic series\n"); break;
      case 26 : printf("error in arctan series\n"); break;
      case 27 : printf("error in Artanh series\n"); break;
      case 28 : printf("error at conversion of number-formats\n"); break;
      case 29 : printf("error at conversion of a string to mfloat\n"); break;
      default : printf("unknown\n");
    }
    exit(1);
  }
}

/* ------------------------------------------------------------------------ */

extern void far pascal mfloatStackOverflow(void) {
  /* mfloat stack overflow error in "MFLOATA.ASM" or "MFLOATB.C" */
  printf("\nMFLOAT-error:\n");
  printf("stack overflow\n");
  exit(1);
}

/* ------------------------------------------------------------------------ */

void far pascal setmantissawords(int number) {

  if (number < 1) number=1;
  if (use386instructions && (number & 1)) number++; /* use even number */
  if (number > mfloatwords-1) number=mfloatwords-1;
  setmantissawords_(number);
};

/* ------------------------------------------------------------------------ */

void pascal reseterror(void) {

  calculationerror=0;
}

/* ------------------------------------------------------------------------ */

char geterror(void) {

  return calculationerror;
}

/* ----------------------------------------------------------------------- */

int far pascal strtomf(mfloat_ a,const char far * b) {
  /* converts a string to a mfloat number                 */
  /* return value: character of the string at first error */
  /* if the conversion is correct: return value: 0        */
  mfloata bound; /* here only dummy */

  return strtomf_(a,bound,b,_fstrlen(b));
}

/* ------------------------------------------------------------------------ */

char far * far pascal mftoa(char far * str, const mfloat_ a, int len) {
  /* converts a mfloat number to a string using a standard format */
  mfloata bound;
  bound[0]=0;                          /* exact number assumed */
  mftostr_(str,a,bound,&len,".32767F");
  str[len]=0;                          /* terminate the string */
  return str;
}

/* ------------------------------------------------------------------------ */

char far * far pascal mftostr(char far * str,const mfloat_ a,int len,
		const char far * format) {
  /* converts a mfloat number to a string using a "C"-format string*/
  mfloata bound;
  bound[0]=0;                          /* exact number assumed */
  mftostr_(str,a,bound,&len,format);
  str[len]=0;                          /* terminate the string */
  return str;
};

/* ------------------------------------------------------------------------ */

double far pascal mftod(const mfloat_ b) {
  /* convert a mfloat number to a double number */
  double tmp;
  mftod_(&tmp,b);
  return tmp;
}

/* ------------------------------------------------------------------------ */

long double far pascal mftold(const mfloat_ b) {
  /* convert a mfloat number to a long double number */
  long double tmp;
  mftold_(&tmp,b);
  return tmp;
}

#endif /* ifdef mfloatb else */

/* ------------------------------------------------------------------------ */
/* compiler control */
/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}                                /* do not extend function names (C++) */
#endif

#ifdef __STDC__
#undef far                      /* delete some of the self made defines */
#undef pascal
#undef huge
#pragma option -u               /* generate underlines   */
#pragma option -p-              /* no pascal calling convention */
#endif

#endif /* ifndef mfloat_h */

/* ------------------------------------------------------------------------ */
/* End of MFLOAT.H */
/* ------------------------------------------------------------------------ */
