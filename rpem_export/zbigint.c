/* zbigint.c - Basic operations on large integers.
 *
 * History and credits:
 *
 * This code was orginally written by Arjen Lenstra
 * (lenstra@flash.bellcore.com).  We appreciate his placing it
 * in the public domain.  It is understood that the code comes
 * with no warranty, of course.
 * 
 * Thanks go to Mark Manasse of DEC-SRC, who used the code in a 
 * distributed-factoring experiment in which the two M. R.'s
 * below participated (as donors of compute cycles) independently.
 * Mark M. was helpful in tracking down Arjen Lenstra and obtaining 
 * his permission.
 *
 * A few additional routines, at the end of the file, were written
 * by Marc Ringuette (in June 90) and Mark Riordan (Dec 90).
 * Code documentation, cleanup, and final packaging was done
 * by Mark Riordan.
 *
 * All work by Lenstra, Ringuette, and Riordan has been placed
 * in the public domain.  No restrictions are placed on its use.
 * However, it's only polite to cite at least Lenstra as the
 * original author in any derivative works.
 *
 * I would appreciate being sent any improvements to this code.
 *
 * Mark Riordan   riordanmr@clvax1.cl.msu.edu   17362mrr@MSU.BITNET
 * February 1991.
 */
 
/* Some notes on these routines:
 *
 * By and large, these routines operate only on positive numbers.
 * I call the data structure on which these routines operate BIGINTs.
 * The routines here operate only on statically-allocated BIGINTs.
 * Arjen may soon release a version of his code that
 * operates on dynamically-allocated BIGINTS, but this isn't it.
 * A BIGINT is an array of MYINTs and looks like this:
 *   MYINT #0:  Number of MYINTs to follow.
 *   MYINT #1:  Least significant MYINT of the number.
 *   MYINT #2:  Next most significant MYINT of the number.
 *    ... and so on.
 *
 * MYINT is a #define'd symbol which should be "long int" for
 * most machines.  However, on some machines it may be more
 * efficient to make MYINT just be an "int".
 *   
 * The BIGINT typedef structure is used to ease compatibility
 * with the Unix multiple precision package "mp" (as well as
 * Carnegie Mellon's own version, CMU-MP), and to maintain a 
 * level of data abstraction.
 * However, the original code in this file was not written using
 * this typedef and I say in this case, if it ain't broke, don't
 * fix it.
 * Hence, the header file zbigintp.h, which contains function
 * prototypes for the functions in this file, is not #included
 * in this file.
 *
 * Note that BIGINTs are never passed directly, despite the
 * wording of my comments.  What are passed are pointers to BIGINTs.
 * 
 * Be sure to examine the file "zbigint.h" to ensure that all
 * symbols have been defined properly for your machine and for the
 * size of integers you will be using.
 *
 * This file was originally called "basic.c" in Arjen's distribution.
 */

#define Z_DEFINE 1
#include <stdio.h>
#include <stdlib.h>
#include "zbigint.h"
#include "zbigintp.h"

/* There are several symbols you can define to compile additional
 * routines beyond the basic math functions.
 * By default many of these symbols are not defined, in most cases
 * resulting in a smaller object file.
 *
 * COMPILE_SQRT         compiles integer square root routines
 * COMPILE_PRIMES       compiles routines to compute prime numbers.
 *                      zprime, which determines whether a bigint
 *                      is a prime, is included under
 *                      COMPILE_MYINT_RANDOM.
 * COMPILE_MONTGOMERY   compiles routines to deal with Montgomery numbers,
 *                      whatever those are.
 * COMPILE_MYINT_RANDOM compiles the MYINT random number routines,
 *                      plus zprime.
 *
 * COMPILE_FUNCS        compiles the function equivalents of a few
 *                      simple functions that Arjen has also 
 *                      implemented as macros for the sake of
 *                      efficiency.  By default as distributed,
 *                      the macro versions are used.
 */

#define COMPILE_MYINT_RANDOM  1

/*========  Special functions and their macro equivalents ============ */

/* If you prefer you can replace the macros for zaddmulp, zaddmul, and zmmulp,
 * defined in zbigint.h, by the following three C-routines.
 */
 
#ifdef COMPILE_FUNCS
/*--- function zaddmulp -----------------------------------------------
 *
 *  This routine apparently does a partial multiply.
 *  a = a+t+b*d % RADIX; t = b*d / RADIX
 *  Internal routine.
 */
static void
zaddmulp(a,b,d,t)
MYINT *a;
MYINT b,d;
MYINT *t;
{ register MYINT b1 = b & RADIXROOTM, d1 = d & RADIXROOTM;
  register MYINT aa = *a + b1 * d1;

  b1 = b1* ( d >>= NBITSH ) + d1* ( b >>= NBITSH ) + (aa >> NBITSH);
  aa = (aa & RADIXROOTM) + ((b1 & RADIXROOTM) << NBITSH) + *t;
  *t = d*b + (b1 >> NBITSH) + (aa >> NBITS);
  *a = (aa & RADIXM);
}

/*--- function zaddmul ------------------------------------------------
 *
 *  a += d*b
 *  a and b not at overlapping addresses
 *  (except for d=1)
 *  Internal routine.
 */
static void
zaddmul(d,a,b)
MYINT d;
MYINT *a,*b;
{ register MYINT i;
  MYINT carry=0;

  for (i=(*b++);i>0;i--) zaddmulp(a++,*b++,d,&carry);
  *a += carry;
}

/*--- function zmmulp ----------------------------------------------------
 *
 *  Do a partial Montgomery multiplication.
 *  a[s:] += (a[s]*zminv)*b so that a[s] = 0
 *  Internal routine.
 */
static void
zmmulp(pa,pb)  
MYINT *pa,*pb;
{ register MYINT i = (*pa) >> NBITSH, d = (*pa) & RADIXROOTM;
  MYINT temp = 0;
  d=(zminv1*d+(((zminv1*i+zminv2*d)&RADIXROOTM)<<NBITSH))&RADIXM;
  for (i = *pb++;i>0;i--) zaddmulp(pa++,*pb++,d,&temp);
  if (((*pa += temp) & RADIX) > 0 ) { (*pa++)&=RADIXM; (*pa)++; }
}

#else

/* Compile the macro versions of these functions.  */

/* a = a+t+b*d % RADIX; t = b*d / RADIX */
/* should be optimized */
 
#define     zaddmulp(_a,_b,_d,_t) \
{ register MYINT lb = (_b), ld = (_d), \
      b1 = (_b) & RADIXROOTM, d1 = (_d) & RADIXROOTM; \
  register MYINT aa = *(_a) + b1 * d1; \
  b1 = b1* ( ld >>= NBITSH ) + d1* ( lb >>= NBITSH ) + (aa >> NBITSH); \
  aa = (aa & RADIXROOTM) + ((b1 & RADIXROOTM) << NBITSH) + *(_t); \
  *(_t) = ld*lb + (b1 >> NBITSH) + (aa >> NBITS); \
  *(_a) = (aa & RADIXM); \
} 

/* a += d*b, optimize this one */
/* a and b not at overlapping addresses */
/* (except for d=1) */
 
#define  zaddmul(ams,ama,amb) \
{ register MYINT lami, lams = (ams), *lama = (ama), *lamb = (amb); \
  MYINT lamcarry=0; \
  for (lami=(*lamb++);lami>0;lami--) \
   { zaddmulp(lama,*lamb,lams,&lamcarry); lama++; *lamb++; } \
  *lama += lamcarry; \
}
/* Be careful, the last lama is unnormalized */

/* a[s:] += (a[s]*zminv)*b so that a[s] = 0 */
/* only in montgomery multiplication zmont */
/* should be optimized. The meaning of s is clear from zmont, */
/* someting like zmtop-i +- 1 */

#define     zmmulp(mmpa,mmpb) \
{ register MYINT *lmmpa = (mmpa), *lmmpb =(mmpb); \
  register MYINT lmmi = (*lmmpa) >> NBITSH, lmmd = (*lmmpa) & RADIXROOTM; \
  MYINT zmmtemp = 0; \
  lmmd=(zminv1*lmmd+(((zminv1*lmmi+zminv2*lmmd)&RADIXROOTM)<<NBITSH))&RADIXM; \
  for (lmmi = *lmmpb++;lmmi>0;lmmi--) \
   { zaddmulp(lmmpa,*lmmpb,lmmd,&zmmtemp); lmmpa++; *lmmpb++; } \
  if (((*lmmpa += zmmtemp) & RADIX) > 0 ) { (*lmmpa++)&=RADIXM; (*lmmpa)++; } \
}
#endif

/*==== End of special functions and their macro equivalents ============ */


/*== global variables ============================================*/

/* The doubles below are used for MYINT division. */
static double epsilon, fradix, fudge;  

#ifdef COMPILE_MYINT_RANDOM
/* Below BIGINTs are used in pseudo-random number generation. */

static MYINT zseed[ZSIZEP], zranp[ZSIZEP], zprroot[ZSIZEP];
#endif

#ifdef COMPILE_MONTGOMERY
MYINT zr[ZSIZEP], zrr[ZSIZEP], zrrr[ZSIZEP], znm[ZSIZEP],
    zmtop, zminv1, zminv2;    /* for montgomery multiplication */
#endif

#ifdef COMPILE_PRIMES
/* to generate primes */
static short int xxprimes[PRIMBND], interval[PRIMBND];   
MYINT pshift = -2;
MYINT pindex;
#endif

/*== end global variables =======================================*/

/*--- function zstart ----------------------------------------------------
 *
 *  Initialize the BIGINT system.
 */

void
zstart()    /* set values of global variables */
{ double one = (double)1, half = 1/(double)2;

  z_one.len = 1;  z_one.val[0] = 1;
  fradix = (double)RADIX;  epsilon = one;
  fudge = one + epsilon; /* this is sick, but we have to */
  while ( one != fudge ) { /* because the test one != (one+epsilon) */
   epsilon = epsilon * half; /* leads to disasters on some machines */
   fudge = one + epsilon;
   }
  epsilon += epsilon;
  if ((epsilon*RADIX) > one) printf("decrease RADIX and recompile\n");
  else epsilon *= 3;
  fudge = fradix+epsilon*fradix;
  zseed[0] = -1;
}

/*--- function zsubmul ----------------------------------------------------
 *
 *  Internal routine used in zdiv and zmod.
 */

void
zsubmul(r,a,b)   /* a[s:] -= d*b, optimize this one */
                 /* a and b not at overlapping addresses */
MYINT r;
MYINT *a,*b;
{ register MYINT rd = RADIX-r, i;
  MYINT carry = RADIX;

  for (i=(*b++);i>0;i--)
    { zaddmulp(a,*b,rd,&carry); a++;
      carry += RADIXM-(*b++);
    }
  *a += carry-RADIX; /* unnormalized */
}

/*--- function zcopy ----------------------------------------------------
 *
 *  Make a copy of a BIGINT.
 *
 *  Entry:   a is a BIGINT.
 *
 *  Exit:    b is a copy of a.
 */

void
zcopy(a,b)  /* b = a */
MYINT *a, *b;
{ register MYINT i;

  for (i= *a; i>=0; i--) *b++ = *a++;
}

/*--- function zintoz ----------------------------------------------------
 *
 *  Convert a MYINTeger to a BIGINT.
 *
 *  Entry:  d is a MYINT.
 *  
 *  Exit:   a is a BIGINT with the same value as d.
 */

void
zintoz(d,a) /* a = d, d MYINT */
MYINT d;
MYINT a[];
{ register MYINT i=0;

  a[1]=0;
  while (d > 0)
   { i++; a[i] = d&RADIXM; d >>= NBITS; }
  if (i>0) a[0] = i;
  else a[0] = 1; 
}

/*--- function ztoint ----------------------------------------------------
 *
 *  Convert a BIGINT to a MYINT, with no overflow check.
 *
 *  Entry:  a is a BIGINT
 *
 *  Exit:   Returns the value of "a" as a BIGINT.
 */

MYINT
ztoint(a)   /* returns (MYINT)a, no overflow check */
MYINT *a;
{ register MYINT d, i, sa;

  sa = *a; a += sa; d = *a;
  for (i=sa-1; i>0; i--)
   { d <<= NBITS; d += *(--a); }
  return(d);
}
  
/*--- function zcompare ----------------------------------------------------
 *
 *  Compare two BIGINTs.
 *
 *  Entry:  a and b are BIGINTs.
 *
 *  Exit:   Returns 1 if a>b, 0 if a==b, or -1 if a<b.
 */

int
zcompare(a,b)  /* a>b:1; a=b:0; a<b:-1 */
MYINT *a, *b;
{ register MYINT sa = *a, sb = *b, i;

  if (sa > sb ) return(1);
  if (sa < sb ) return(-1);
  a += sa; b += sa;
  for (i=sa;i>0;i--)
    { if (*a > *b) return(1);
      if (*a-- < *b-- ) return(-1);
    }
  return(0);
}

/*--- function zsadd ----------------------------------------------------
 *
 *  Limited signed addition function.  Probably mostly for internal use.
 *
 *  Entry:   a  is a BIGINT.
 *           d  is a signed MYINT.
 *
 *  Exit:    b = a+d.  Result must be positive.
 */
void
zsadd(a,d,b)   /* b = a+d, abs(d)<RADIX, output can be input */
      /* assume result positive */
MYINT a[];
MYINT d;
MYINT b[];
{ MYINT x[2];

  x[0] = 1;
  if (d >= 0) { x[1] = d; zadd(a,x,b); }
  else { x[1] = -d; zsub(a,x,b); }
}

/*--- function zaddls ----------------------------------------------------
 *
 *  Limited BIGINT addition function.  Mostly for internal use.
 *
 *  Entry:  a, b are BIGINTs.  b must consist of no more MYINTs than a.
 * 
 *  Exit:   c = a+b.   OK if &c = &a or &c = &b on input.
 */

void
zaddls(a,b,c)  /* c = a+b, b shorter than a, output can be input */
MYINT *a,*b,c[];
{ register MYINT sa = *a, sb = *b, carry=0, i, *pc;

  pc = &c[0];
  for (i=1; i<=sb; i++)
    { if (( *(++pc) = (*(++a))+(*(++b))+carry) < RADIX ) carry = 0;
      else  { *pc -= RADIX; carry = 1; }
    }
  for (; i<=sa; i++)
    { if (( *(++pc) = (*(++a))+carry) < RADIX ) carry = 0;
      else  { *pc -= RADIX; carry = 1; }
    }
  if ( carry == 1 )
    { c[0] = sa+1; *(++pc) = 1; }
  else c[0] = sa;
}

/*--- function zadd ----------------------------------------------------
 *
 *  General-purpose BIGINT addition routine.
 *
 *  Entry:  a, b are BIGINTs.
 *
 *  Exit:   c = a + b.   OK if &c = &a or &c = &b.
 */

void
zadd(a,b,c) /* c = a+b, output can be input */
MYINT *a,*b,c[];
{ register MYINT sa = *a, carry=0, i, *pc;

  pc = &c[0];
  if ( sa == *b )
    {
    for (i=sa; i>0; i-- )
      { if (( *(++pc) = (*(++a))+(*(++b))+carry) < RADIX ) carry = 0;
        else   { *pc -= RADIX; carry = 1; }
      }
    if ( carry == 1 )
      { c[0] = sa+1; *(++pc) = 1; }
    else c[0] = sa;
    }
  else if ( sa > *b ) zaddls(a,b,c);
  else zaddls(b,a,c);
}

/*--- function zsub ----------------------------------------------------
 *
 *  General purpose BIGINT subtraction routine.
 *
 *  Entry:  a, b are BIGINTs; a>=b.
 *          OK if &c=&a or &c=&b
 *
 *  Exit:   c = a-b.
 */

void
zsub(a,b,c) /* c = a-b for a >= b, output can be input */
MYINT *a,*b,c[];
{ register MYINT sa = *a, sb = *b, carry=0, i, *pc;

#ifdef DEBUG
   if(zcompare(a,b) < 0) {
      zhalt("zsub: a<b");
   }
#endif
  pc = &c[0];
  for (i=1; i<= sb; i++)
    { if ((*(++pc) = (*(++a))-(*(++b))-carry) >= 0 ) carry = 0;
      else { *pc += RADIX; carry = 1; };
    }
  for (; i<=sa; i++)
    { if ((*(++pc) = (*(++a))-carry) >= 0 ) carry = 0;
      else { *pc += RADIX; carry = 1; };
    }
  i = sa;
  while ((i>1) && (*pc == 0 )) { i--; pc--; }
  c[0] = i;
}

/*--- function zsmul ----------------------------------------------------
 *
 *  BIGINT * MYINT multiply, mostly for internal use.
 *  
 *  Entry:  a is a BIGINT.
 *          d is a MYINTeger.
 *
 *  Exit:   b = d*a.
 */

void
zsmul(a,d,b)   /* b = d*a, d < RADIX, output can be input */
MYINT *a;
MYINT d;
MYINT b[];
{ register MYINT sa = *a, i, *pb;

  pb = &b[0];
  if (d == 0 )
    { *pb++ = 1; *pb = 0; }
  else {
    for (i=sa;i>=0;i--) *pb++ = *a++;
    sa ++; *pb = 0;
    zaddmul(d-1,&b[1],&b[0]);
    while ((sa>1) && (b[sa] == 0)) sa --;
    b[0] = sa;
  }
}

/*--- function zmul ----------------------------------------------------
 *
 *  General-purpose BIGINT multiple routine.
 *
 *  Entry:  a, b are BIGINTS.
 *          &a != &c and &b != &c.
 *
 *  Exit:   c = a*b
 */

void
zmul(a,b,c) /* c =a*b, output cannot be input */
MYINT *a,*b,c[];
{ register MYINT i, *pc, sc = *a + *b;

  pc = &c[1];
  for (i= sc; i>0; i--) *pc++ = 0;
  pc = &c[1];
  if (*a <= *b ) for (i= *a;i>0;i--) { zaddmul(*(++a),pc++,b); }
  else   for (i= *b;i>0;i--) { zaddmul(*(++b),pc++,a); }
  while ((sc>1) && (c[sc] == 0)) sc --;
  c[0] = sc;
}

/*--- function zsdiv ----------------------------------------------------
 *
 *  BIGINT / MYINT divide routine.   Mostly for internal use.
 *
 *  Entry:  a is a BIGINT.  d is a MYINT.
 *          &a != &b.
 *
 *  Exit:   b = a div d.   Returns the remainder (a mod d).
 */

MYINT
zsdiv(a,d,b)   /* b = a/d, returns a%ld, output can be input */
MYINT *a;
MYINT d;
MYINT *b;
{ register MYINT sa, rmodd, rdivd, carry=0, temp, i, *pb;
MYINT zd[ZSIZEP];

if (d < ZSINT) {
   sa = *a; rdivd = RADIX/d; rmodd = RADIX - d*rdivd;
   pb = b; a += sa; b += sa;
   for (i=sa; i>0; i--)
      { temp = (*a--)+rmodd*carry;
      (*b--) = rdivd*carry + temp/d;
      carry = temp % d;
      }
   while ((sa>1) && (*(pb+sa) == 0)) sa--;
   *pb = sa;
   return(carry);
   }
else {   zintoz(d,zd); zdiv(a,zd,b,zd); return(zd[1]); }
}

/*--- function zread ----------------------------------------------------
 *
 *  Read an ASCII decimal BIGINT from standard input.
 *
 *  Entry:  There's an ASCII decimal integer on standard input.
 *
 *  Exit:   a is a BIGINT with the value of that integer.
 */

void
zread(a)    /* read a from standard input */
MYINT a[];
{ char in[ZDECSIZE];
  register MYINT d=0;
  scanf("%s",in);
  a[0] = 1; a[1] = 0;
  while (in[d] != '\0')
   { if (in[d] == '\\') 
      { scanf("%s",in); d = 0; }
     else
      { zsmul(a,(MYINT)10,a);
        zsadd(a,(MYINT)(in[d++]-'0'),a);
      };
   }
}  

/*--- function fzread ----------------------------------------------------
 *
 *  Read an ASCII decimal BIGINT from a stream.
 *
 *  Entry:  fil  is a stream containing an ASCII decimal integer.
 *
 *  Exit:   a  is a BIGINT with the value of that integer.
 */

void
fzread(fil,a)     /* read a from file fil */
FILE *fil;
MYINT a[];
{ char in[ZDECSIZE];
  register MYINT d=0;
  fscanf(fil,"%s",in);
  a[0] = 1; a[1] = 0;
  while (in[d] != '\0')
   { if (in[d] == '\\') 
      { fscanf(fil,"%s",in); d = 0; }
     else
      { zsmul(a,(MYINT)10,a);
        zsadd(a,(MYINT)(in[d++]-'0'),a);
      };
   }
}

/*--- function zwrite ----------------------------------------------------
 *
 *  Write a BIGINT in decimal ASCII form into standard output.
 *
 *  Entry:  a  is a BIGINT.
 *
 *  Exit:   The decimal ASCII digits representing "a" have been 
 *          written to standard output.  Lines are wrapped at
 *          70 characters, with a \ appended to all but the last
 *          line.  (No backslash if the number takes <= 70 digits.)
 *          Returns the number of digits written.
 */

MYINT
zwrite(a)   /* write a on standard output */
MYINT a[];
{ MYINT ca[ZZSIZEP], out[ZDECSIZE], zsdiv();
  register MYINT sa = a[0], i;
  MYINT result;
  for (i=0; i<=sa;i++) ca[i] = a[i];
  i = -1;
  do out[++i] = zsdiv(ca,(MYINT)10,ca) ;
  while (ca[0] > 1 || ca[1] != 0 );
  sa = 0;
  if (i < 0 ) i=0;
  result = i+1;
  while (i >= 0 )
   { printf("%ld",out[i--]);
     sa ++;
     if ((i >= 0) && (sa == 70 ))
      { sa = 0; printf("\\\n"); }
   }
  return(result);
}

/*--- function fzwrite ----------------------------------------------------
 *
 *  Write a BIGINT in decimal ASCII form to a specified file.
 *
 *  Entry:  fil     is an output stream.
 *          a       is a BIGINT
 *          lll     is the maximum number of digits per line.
 *          eol     is the string to use at the end of MYINT lines
 *                  to indicate continuation.  I recommend "\".
 *          bol     is the string to use at the beginning of
 *                  new, continued lines.  I recommend "".
 *
 *  Exit:   Returns the number of digits written.
 */

MYINT
fzwrite(fil,a,lll,eol,bol)
      /* write a on file fil, at most lll digits per line */
      /* end line with eol, begin next line with bol */
FILE *fil;
MYINT a[];
MYINT lll;
char *eol, *bol;
{ MYINT ca[ZZSIZEP], out[ZDECSIZE], zsdiv();
  register MYINT sa = a[0], i;
  MYINT result;
  for (i=0; i<=sa;i++) ca[i] = a[i];
  i = -1;
  do out[++i] = zsdiv(ca,(MYINT)10,ca) ;
  while (ca[0] > 1 || ca[1] != 0 );
  sa = 0;
  if (i < 0 ) i=0;
  result = i+1;
  while (i >= 0 )
   { fprintf(fil,"%ld",out[i--]);
     sa ++;
     if ((i >= 0) && (sa == lll ))
      { sa = 0; fprintf(fil,"%s\n%s",eol,bol); }
   }
  return(result);
}

/*--- function zdiv ----------------------------------------------------
 *
 *  Divide two BIGINTS and return quotient and remainder.
 *
 *  Entry:   a is a BIGINT (the dividend).
 *           b is a BIGINT (the divisor).
 *
 *  Exit:    q is a BIGINT; a/b (integer portion, obviously).
 *           r is the remainder of the division.
 */

void
zdiv(a,b,q,r)  /* q = a/b, r = a%b, output can be input */
MYINT a[];
MYINT b[];
MYINT q[];
MYINT r[];
{ register MYINT i, qq;
  MYINT c[ZZSIZEP], d[ZZSIZEP], sa, sb = b[0], sq, *p, *pc, zsdiv();
  double btopinv, aux;
  p = &b[sb];
  if ((sb == 1) && (*p < ZSINT)) zintoz(zsdiv(a,*p,q),r);
  else
   { sa = a[0]; sq = sa-sb; btopinv = (double)(*p)*fradix;
     if (sb > 1) btopinv += (*(--p));
     btopinv *= fradix;
     if (sb > 2) btopinv += (*(p-1));
     btopinv = fudge / btopinv;
     p = &a[1]; pc = &c[0];
     for (i=sa;i>0;i--) *pc++ = *p++;
     p = pc; sa = 1-sb;
     for (i=(-sq);i>0;i--) *p++ = 0;
     *pc = 0; d[1] = 0; p = &d[sq+1];
     for (i=sq;i>=0;i--)
      { aux = fradix*(fradix*(*pc)+(*(pc-1)))+1.0;
        if (i > sa) aux += (*(pc-2));
        qq = (MYINT)(btopinv*aux+0.5);
      /* dirty, but safe. On most machines (MYINT)(btopinv*aux) */
      /* is correct, or one too big; on some however it becomes*/
      /* too small. Could change zstart, but +0.5 and a while */
      /* instead of one single if is safer */
        if (qq > 0)
         { zsubmul(qq,&c[i],&b[0]); 
           while (*pc < 0) 
            { qq --;
              { zaddmul(1,&c[i],&b[0]); }
            }
         }
        pc--; *p-- = qq;
      } /* loop on i */
   sb --;
   while ((sb > 0) && (c[sb] == 0)) sb --;
   sb ++;r[0] = sb; p = &r[1]; pc = &c[0];
   for (i=sb;i>0;i--) *p++ = *pc++;
   if (sq < 0)
      { q[0] = 1; q[1] = d[1]; }
   else
      { sq ++;
        while ((sq > 1) && (d[sq] == 0)) sq --;
        q[0] = sq; p = &q[1]; pc = &d[1];
        for (i=sq;i>0;i--) *p++ = *pc++;
      }
   } /* non-trivial case */

}

/*--- function zmod ----------------------------------------------------
 *
 *  Compute a BIGINT modulo another BIGINT.
 *
 *  Entry:   a and b are BIGINTs.
 *
 *  Exit:    r   is  a mod b.
 */

void
zmod(a,b,r) /* r = a%b, output can be input */
MYINT a[];
MYINT b[];
MYINT r[];
{ register MYINT i, qq;
  MYINT c[ZZSIZEP], sa, sb = b[0], sq, *p, *pc, zsdiv();
  double btopinv, aux;
  p = &b[sb];
  if ((sb == 1) && (*p < ZSINT)) zintoz(zsdiv(a,*p,c),r);
  else
   { sa = a[0]; sq = sa-sb; btopinv = (double)(*p)*fradix;
     if (sb > 1) btopinv += (*(--p));
     btopinv *= fradix;
     if (sb > 2) btopinv += (*(p-1));
     btopinv = fudge / btopinv;
     p = &a[1]; pc = &c[0];
     for (i=sa;i>0;i--) *pc++ = *p++;
     p = pc; sa = 1-sb;
     for (i=(-sq);i>0;i--) *p++ = 0;
     *pc = 0;
     for (i=sq;i>=0;i--)
      { aux = fradix*(fradix*(*pc)+(*(pc-1)))+1.0;
        if (i > sa) aux += (*(pc-2));
        qq = (MYINT)(btopinv*aux+0.5);
      /* see comment in zdiv */
        if (qq > 0)
         { zsubmul(qq,&c[i],&b[0]);
           while (*pc < 0) 
            { { zaddmul(1,&c[i],&b[0]); }
            }
         }
        pc--;
      } /* loop on i */
     sb --;
     while ((sb > 0) && (c[sb] == 0)) sb --;
     sb ++;r[0] = sb; p = &r[1]; pc = &c[0];
     for (i=sb;i>0;i--) *p++ = *pc++;
   } /* non-trivial case */

/* MRR */
   if(zcompare(b,r) < 0) zhalt("Error in zmod: result >= modulus");

}

/*--- function zaddmod ----------------------------------------------------
 *
 *  Compute (a + b) mod n.  Note limitations below.
 *
 *  Entry:  a, b, n all BIGINTs.  a and b must be < n.
 *
 *  Exit:   c = (a+b) mod n.
 */

void
zaddmod(a,b,n,c)  /* c = (a+b)%n, with 0<=a,b<n */
MYINT a[];
MYINT b[];
MYINT n[];
MYINT c[];
{
  zadd(a,b,c);
  if (zcompare(c,n) >= 0 ) zsub(c,n,c);
}

/*--- function zsubmod ----------------------------------------------------
 *
 *  Compute (a - b) mod n.  Note limitations below.
 *
 *  Entry:  a, b, n all BIGINTs.  a and b must be < n.
 *
 *  Exit:   c = (a-b) mod n.
 *
 */

void
zsubmod(a,b,n,c)  /* c = (a-b)%n, with 0<=a,b<n */
MYINT a[];
MYINT b[];
MYINT n[];
MYINT c[];
{ MYINT d[ZSIZEP];
  if (zcompare(a,b) >= 0) zsub(a,b,c);
  else { zadd(a,n,d); zsub(d,b,c); }
}

/*--- function zsmulmod ----------------------------------------------------
 *
 *  Compute the product of a BIGINT and a MYINT modulo a BIGINT
 *
 *  Entry:  a is a BIGINT
 *          d is a MYINT.
 *          n is a BIGINT (the modulus).
 *
 *  Exit:   c is (a * d) modulo n
 */

void
zsmulmod(a,d,n,c) /* c = (a*d)%n */
MYINT a[];
MYINT d;
MYINT n[];
MYINT c[];
{ MYINT dd[ZSIZEP];
  zintoz(d,dd);      zmulmod(a,dd,n,c);
}

/*--- function zmulmod ----------------------------------------------------
 *
 *  Compute the product of two BIGINTs modulus a BIGINT.
 *
 *  Entry:  a, b, n are all BIGINTs.
 *
 *  Exit:   c is (a * b) modulo n
 */

void
zmulmod(a,b,n,c)  /* c = (a*b)%n */
MYINT a[];
MYINT b[];
MYINT n[];
MYINT c[];
{ MYINT mem[ZZSIZEP];

  zmul(a,b,mem);  zmod(mem,n,c);
}

#ifdef COMPILE_MONTGOMERY

/*--- function zmstart ----------------------------------------------------
 *
 *  Initialize globals needed for Montgomery routines.
 */

void
zmstart(n)  /* set up global values for Montgomery's multiplication */
MYINT n[];
{ MYINT x[ZZSIZEP], i, zsdiv();
  if ((n[1]&1) == 0) zhalt("zmstart: n is even");
  zmtop = n[0];
  if (n[zmtop] >= (RADIX >>1)) zmtop ++;
  if (zmtop >= ZSIZEP ) zhalt("zmstart: n too big");
  zsadd(n,(MYINT)1,zrr); i=zsdiv(zrr,(MYINT)2,x);  /* x = (n+1)/2 */
  zrr[0]=1; zrr[1]=NBITS; zexp(x,zrr,n,x);   /* x = 2^-NBITS %n   */
  for (i=x[0]; i>0; i--) x[i+1] = x[i];
  x[1] = 0; x[0] ++;
  zsadd(x,(MYINT)-1,x);
  zdiv(x,n,x,zrr); zminv1 = x[1];   /* zminv1=n[1]^-1 modulo 2^NBITS */
  zminv2 = zminv1 >> NBITSH;  zminv1 &= RADIXROOTM;
  for (i=1; i<=zmtop; i++) x[i] = 0; x[zmtop+1] = 1; x[0] = zmtop+1;
  zdiv(x,n,zrr,zr);     zmulmod(zr,zr,n,zrr);
  zmulmod(zr,zrr,n,zrrr);  zsubmod(n,zr,n,znm);
}

/*--- function ztom ----------------------------------------------------
 *
 *  Compute the "Montgomery representation" of a BIGINT.
 */

void
ztom(a,n,b) /* b is montgomery representation of a */
MYINT a[];
MYINT n[];
MYINT b[];
{ MYINT zcompare();
  if (zcompare(n,a) < 0)
   { zmod(a,n,b); zmont(b,zrr,n,b);}
  else   zmont(a,zrr,n,b);
}

/*--- function zmtoz ----------------------------------------------------
 *
 *  Convert a number in Montgomery form to normal BIGINT form.
 */

void
zmtoz(a,n,b)   /* montgomery a is backtransformed to normal b */
MYINT a[];
MYINT n[];
MYINT b[];
{ MYINT x[2];
  x[0] = 1; x[1] = 1;
  zmont(a,x,n,b);
}

/*--- function zmont ----------------------------------------------------
 *
 *  Compute the Montgomery product of two numbers.
 */

void
zmont(a,b,n,c) /* c is mont product of mont's a and b */
      /* output can be input */
MYINT *a;
MYINT *b;
MYINT *n;
MYINT c[];
{ register MYINT i;
  MYINT x[ZZSIZEP], *px, *pc, zcompare();
  px = &x[0];
  for (i=ZZSIZEP;i>0;i--) *px++ = 0;
  px = &x[0];
  if (*a <= *b ) for (i= *a;i>0;i--) { zaddmul(*(++a),px++,b); }
  else   for (i= *b;i>0;i--) { zaddmul(*(++b),px++,a); }
  px = &x[0];
  for (i=zmtop; i>0; i--) zmmulp(px++,n);
  pc = &c[1];
  for (i=zmtop; i>0; i--) *pc++ = *px++;
  i = zmtop;
  while ((i > 1) && (*--pc == 0)) i --;
  c[0] = i;
  if (zcompare(n,c) < 0) zsub(c,n,c);
}
#endif

/*--- function zinv ----------------------------------------------------
 *
 *  Compute the inverse of a BIGINT modulus another BIGINT,
 *  if possible, or computes the greatest common divisor of
 *  these two numbers.
 *
 *  Entry:  ain, nin are BIGINTs.
 *
 *  Exit:   If there is an inverse of ain mod nin, returns 0
 *            and computes inv is such that (ain * inv) mod nin = 1
 *          else returns 1
 *            and computes inv such that gcd(ain,nin) = inv.
 */

MYINT
zinv(ain,nin,inv) /* if 0, then ain*inv%nin=1 */
         /* if 1, then gcd(ain,nin)=inv > 1 */
MYINT ain[];
MYINT nin[];
MYINT inv[];
{ MYINT a[ZZSIZEP], n[ZZSIZEP], q[ZZSIZEP], u[ZZSIZEP],
      w[ZZSIZEP], x[ZZSIZEP], y[ZZSIZEP], z[ZZSIZEP], 
      diff, ilo, sa, sn, temp, e,
      try11, try12, try21, try22, got11, got12, got21, got22,
      fast, parity, gotthem, *pin, *p, i;
  double hi, lo, dt, fhi, flo, num, den;
  fhi = 1.0 + epsilon; flo = 1.0 - epsilon;
  pin = &ain[0]; p = &a[0];
  for (i=(*pin); i>=0; i--) *p++ = *pin++;
  pin = &nin[0]; p = &n[0]; 
  for (i=(*pin); i>=0; i--) *p++ = *pin++;
  u[0] = 1; u[1] = 1; w[0] = 1; w[1] = 0;
  while (n[0] > 1 || n[1] > 0)
   { gotthem = 0; sa = a[0]; sn = n[0]; diff = sa-sn;
     if (diff == 0 || diff == 1 )
      { sa = a[0]; p = &a[sa];
        num = (double)(*p)*fradix;
        if (sa > 1) num += (*(--p));
        num *= fradix;
        if (sa > 2) num += (*(p-1));
        sn = n[0]; p = &n[sn];
        den = (double)(*p)*fradix;
        if (sn > 1) den += (*(--p));
        den *= fradix;
        if (sn > 2) den += (*(p-1));
        hi = fhi*(num+1.0)/den;
        lo = flo*num/(den+1.0);
        if (diff > 0) { hi *= fradix; lo *= fradix; }
        try11 = 1; try12 = 0; try21 = 0; try22 = 1;
        parity = 1; fast = 1;
        while (fast > 0)
         { parity ^= 1;
/* be careful, some mailers might change the previous line into */
/* open curly bracket parity tilde equal one semicolon */
/* the correct line is */
/* open curly bracket hat equal one semicolon */
           if (hi >= fradix )
            { fast = 0; }
           else   { ilo = (MYINT)lo;
              if (ilo == 0 || ilo < (MYINT)hi)
               { fast = 0; }
              else   { dt = lo; lo = flo/(hi-ilo);
                 if (dt > ilo) hi = fhi/(dt-ilo);
                 else hi = fradix;
                 temp = try11; try11 = try21;
                 if ((RADIX-temp)/ilo < try21)
                  fast = 0;
                 else try21 = temp+ilo*try21;
                 temp = try12; try12 = try22;
                 if ((RADIX-temp)/ilo < try22)
                  fast = 0;
                 else try22 = temp+ilo*try22;
                 if ((fast > 0) && (parity >0))
                  { gotthem = 1;
                    got11 = try11; got12 = try12;
                    got21 = try21; got22 = try22;
                  }
               }
            }
         }
      }
     if (gotthem > 0)
      { zsmul(u,got11,x); zsmul(w,got12,y);
        zsmul(u,got21,z); zsmul(w,got22,w);
        zadd(x,y,u);     zadd(z,w,w);
        zsmul(a,got11,x); zsmul(n,got12,y);
        zsmul(a,got21,z); zsmul(n,got22,n);
        zsub(x,y,a);     zsub(n,z,n);
      }
     else   { zdiv(a,n,q,a); zmul(q,w,x); zadd(u,x,u);
        if (a[0] > 1 || a[1] > 0)
         { zdiv(n,a,q,n); zmul(q,u,x); zadd(w,x,w); }
        else   { p = &a[0]; pin = &n[0];
           for (i=(*pin); i>=0; i--) *p++ = *pin++;
           n[0] = 1; n[1] = 0; zsub(nin,w,u);
         }
      }
   }
  p = &inv[0];
  if (a[0] != 1) e = 1;
  else { if (a[1] == 1) e = 0; else e = 1; }
  if (e == 0) pin = &u[0];
  else   { pin = &a[0]; }
  for (i=(*pin); i>=0; i--) *p++ = *pin++;
  return(e);
}

#ifdef COMPILE_MYINT_RANDOM

/*--- function zrstart ----------------------------------------------------
 *
 *  Initialize globals needed for pseudo-random number generation.
 *
 *  Entry:  s  is a MYINT used as a seed for the generator.
 */

void
zrstart(s)  /* initalize globals zseed = s, zranp = 2^89-1, */
      /* zprroot = 3^101 mod zranp */
MYINT s;
{ register MYINT i, ones, res;
  MYINT e[2];
  zintoz(s,zseed);
  ones = 89/NBITS; res = 89%NBITS;
  for (i=1; i<=ones; i++) zranp[i] = RADIXM;
  if (res != 0) zranp[++ones] = ( RADIXM >> (NBITS - res) );
  zranp[0] = ones;
  zintoz((MYINT)3,zprroot); e[0] = 1; e[1] = 101;
  zexp(zprroot,e,zranp,zprroot);
}

/*--- function zrandomint -------------------------------------------------
 *
 *  Returns a MYINT pseudo-random number in a given range.
 *  Uses the usual linear congruential technique
 *  (not cryptologically strong).
 *
 *  Entry:  b  is a MYINT specifying the desired range.
 *          zrstart must have been called.
 *
 *  Exit:   Returns a MYINT pseudo-random number in the range [0,b).
 */

MYINT
zrandomint(b) /* returns random in [0,b) */
MYINT b;
{ MYINT zb[ZSIZEP];
  if (zseed[0] < 0) zhalt("zrandomint: zseed bad");
  if (b <= 0) b = 1;
  zmulmod(zseed,zprroot,zranp,zseed);
  zintoz(b,zb);
  zmod(zseed,zb,zb);
  return(ztoint(zb));
}

/*--- function zrandombits ----------------------------------------------------
 *
 *  Return a pseudo-random MYINTeger.
 *  Not cryptologically strong.
 *
 *  Entry:   none  (except globals must be set up).
 *
 *  Exit:    Returns a pseudo-random MYINT in the range
 *           [0,RADIX).
 *
 *  Apparently written by Marc Ringuette, with fixes by MRR.
 */

MYINT
zrandombits()
{ MYINT ztoint();
  if (zseed[0] < 0) zhalt("zrandombits: zseed bad");
  zmulmod(zseed,zprroot,zranp,zseed);
  return(ztoint(zseed) & RADIXM);
}

/*--- function zprime ----------------------------------------------------
 *
 *  Determine whether a given number is likely to be a prime.
 *
 *  Entry:  m  is a BIGINT.
 *          t  is the number of tests to run to deterine whether
 *             m is a prime.  I believe that each test that m
 *             passes decreases by at least 50% the chance
 *             that m is composite.  Not sure of the exact
 *             mathematics of this.  I would say that 100 is
 *             probably an adequate value to use.
 *
 *  Exit:   Returns 1 if m is believed to be prime, else 0.
 *          If 0 is returned, we know that m is composite.
 */

MYINT
zprime(m,t)    /* 0 if m not prime, 1 if m passed t */
         /* pseudo prime tests */
MYINT m[];
MYINT t;
{ MYINT sm, i, j, s, ok, u[ZSIZEP], m1[ZSIZEP], a[ZSIZEP];

 if ((m[0] == 1) && (m[1] <= ZSINT))
   { sm = m[1];
     if (sm < 2) return(0);
     if (sm == 2) return(1);
     if ((sm&1) == 0) return(0);
     i = 3;
     while (i*i <= sm )
      { if ((sm%i) == 0) return(0);
        i++; i++;
      }
     return(1);
   }
 else { if ((m[1]&1) == 0) return(0);
     zsadd(m,(MYINT)-1,m1); i=zsdiv(m1,(MYINT)2,u); s = 0;
     while ((u[1]&1) == 0) { i=zsdiv(u,(MYINT)2,u); s ++; }
     if (m[1] <= ZSINT ) sm = RADIXM-2;
     else sm = m[1];
     for (i=t; i>0; i--)
      { zintoz(2+zrandomint(sm),a);
        zexp(a,u,m,a);
        if ( (a[0] != 1 || a[1] != 1) && (zcompare(m1,a) != 0))
         { j = s; ok = 0;
           while ((j > 0) && (ok == 0))
            { zmulmod(a,a,m,a);
              if (zcompare(a,m1) == 0) ok = 1;
              j--;
            }
           if (ok == 0) return(0);
         }
      }
     return(1);
   }
}
#endif

#ifdef COMPILE_PRIMES

/*--- function zpstart ----------------------------------------------------
 *
 *  Initialize the routines to compute primes by setting certain
 *  globals.
 */

void
zpstart()
{ MYINT i, j, jstep, jstart;
  short int *p;
  if (pshift == -2)
   { p = &xxprimes[0]; for (i=PRIMBND; i>0; i--) *p++ = 1;
     jstep = 1; jstart = -1;
     for (i=0; i<=INDBND; i++)
        { jstep += 2; jstart += 2*(jstep-1);
          if (xxprimes[i] > 0)
           { for (j=jstart; j<PRIMBND; j+=jstep) xxprimes[j] = 0;
           }
        }
   }
  pshift = 0; pindex = -1;
}

/*--- function zpshift ----------------------------------------------------
 *
 *  Internal routine used to help compute primes.
 */

void
zpshift()
{ MYINT i, j, jstep, jstart, ibound;
  short int *p, *pi;
  pi = &interval[0];
  if (pshift == 0)
   { p = &xxprimes[0]; pi = &interval[0];
     for (i=PRIMBND; i>0; i--) *pi++ = *p++;
   }
 else { for (i=PRIMBND; i>0; i--) *pi++ = 1;
     jstep = 3; ibound = pshift+2*PRIMBND+1;
     for (i=0; jstep*jstep<=ibound; i++)
      { if (xxprimes[i] > 0)
         { jstart = (pshift+2)/jstep+1;
           if ((jstart&1) == 0) jstart++;
           if (jstart <= jstep) jstart = jstep;
           jstart = (jstart*jstep-pshift-3)/2;
           for (j=jstart;j<PRIMBND; j+=jstep)
            interval[j] = 0;
         }
        jstep += 2;
      }
   }
}

/*--- function zpnext ----------------------------------------------------
 *
 *  Compute the next prime number.  Probably not of general interest.
 *
 *  Exit:  Returns a MYINT prime number--the next prime
 *         after the previous one.
 */

MYINT
zpnext() /* returns next prime number, starting at 2 */
      /* restart with pshift = -1 (get 2 first) */
      /* or with zpstart() (get 3 first) */
{ if (pshift < 0) { zpstart(); return(2); }
  if (pindex < 0) { pindex = 0; zpshift(); return(3); }
  while ((++pindex) < PRIMBND )
   { if (interval[pindex] > 0) return(pshift+2*pindex+3); }
  pshift += 2*PRIMBND;
  if (pshift > 2*PRIMBND*(2*PRIMBND+1)) { zpstart(); return(2); }
  zpshift();
  pindex = -1;
  while ((++pindex) < PRIMBND )     /* assuming interval non-empty */
   { if (interval[pindex] > 0) return(pshift+2*pindex+3); }
}
#endif

#ifdef COMPILE_SQRT

/*--- function isqrt ----------------------------------------------------
 *
 *  Compute the (integer truncated) square root of a MYINT.
 *  Probably not of general interest.
 *
 *  Entry:  n  is a MYINT.
 *
 *  Exit:   Returns trunc(sqrt(n))
 */

MYINT
isqrt(n)
MYINT n;
{ MYINT a, ndiva, newa;
  if (n <= 0) return (0);
  if (n <= 3) return (1);
  if (n <= 8) return (2);
  a = n / 2;
  while (1) {
   ndiva = n/a; newa = (ndiva+a)/2;
   if (newa-ndiva <= 1) 
      if (newa*newa <= n) return(newa);
      else return(ndiva);
   a = newa;
   }
}

/*--- function zsqrt ----------------------------------------------------
 *
 *  Compute the (integer truncated) square root of a BIGINT.
 *
 *  Entry:  n  is a BIGINT.
 *
 *  Exit:   r  is trunc(sqrt(n))
 */

void
zsqrt(n,r)
MYINT n[];
MYINT r[];
{ MYINT a[ZZSIZEP], ndiva[ZZSIZEP], diff[ZZSIZEP], i, isqrt(), zcompare(),
   zsdiv();
  if (n[0] == 1)
   zintoz(isqrt(n[1]),r);
  else   {
   a[0]=(n[0]+1)/2;
   for (i=1;i<a[0];i++) a[i]=0;
   a[a[0]] = isqrt(n[n[0]]) + 1;
   if ((n[0]&1) ==0) a[a[0]] <<= NBITSH;
   while (1) { 
      zdiv(n,a,ndiva,r);
      zadd(a,ndiva,r);
      i=zsdiv(r,(MYINT)2,r);
      if (zcompare(r,ndiva) <= 0) break;
      zsub(r,ndiva,diff);
      if ((diff[0] == 1) && (diff[1] <= 1)) {
         zmul(r,r,diff);
         if (zcompare(diff,n) > 0) zcopy(ndiva,r);
         break;
         }
      zcopy(r,a);
      }
   }
}
#endif

/*--- function zexp ----------------------------------------------------
 *
 *  Computes a BIGINT raised to a BIGINT power, mod a BIGINT.
 *
 *  Entry:  a, e, n are BIGINTs
 *
 *  Exit:   b  is (a^e) mod n.
 */

void
zexp(a,e,n,b)     /* b = (a^e)&n, b and a can be identical, but */
         /* both unequal to n and e */
         /* only if RADIX is power of two */
MYINT a[];
MYINT e[];
MYINT n[];
MYINT b[];
{ register MYINT i,j, cntr, ei;
  MYINT sq[ZZSIZEP], *pae;
  if ((e[0] == 1) && (e[1] == 0 ))
   { b[0] = 1; b[1] = 1; }
  else
   { zmod(a,n,sq);
     b[0] = 1; b[1] = 1; cntr = 0; pae = &e[0];
     for (i=(*pae); i>0; i--)
      { for (j=cntr; j>0; j--) zmulmod(sq,sq,n,sq);
        ei = *(++pae); cntr = NBITS;
        if (ei > 0)
         { while (ei > 1)
            { if ( (ei&1) == 1 ) zmulmod(b,sq,n,b);
              ei >>= 1;
              zmulmod(sq,sq,n,sq); cntr--;
            }
           zmulmod(b,sq,n,b);
         }
      }
   }
}

/*--- function zhalt ----------------------------------------------------
 *
 *  Abort with an error message.
 *
 *  Entry:  msg  is an error message (character string).
 *
 *  Exit:   Does not return.
 */

void
zhalt(msg)  /* error handling routine */
char *msg;
{ 
   fputs("Fatal error occurred in zbigint.c.\n",stderr);
   fputs(msg,stderr);
   fputc('\n',stderr);
   fflush(stderr);
   exit(1);
}


/*--- Last line of ZBIGINT.C --------------------------------------*/
