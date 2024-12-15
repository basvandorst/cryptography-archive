/*--- zmisc.c -- Miscellaneous BIGINT routines.
 *
 *  Written by Marc Ringuette in June 1990, with some additions and
 *  corrections by Mark Riordan  Dec 1990 - Feb 1991.
 *  This code is hereby placed in the public domain.
 */

#include <stdio.h>
#include "zbigint.h"
#include "zproto.h"
#include "zmiscpro.h"

/*--- function ztest ---------------------------------------------------
 *
 *  Determine whether a BIGINT is zero.
 *
 *  Entry:  x  is a BIGINT.
 *
 *  Exit:   Returns 1 if the input value is non-zero; 0 if 0.
 */

int
ztest(x) /* 1 if non-zero */
BIGINT *x;
{
   return x->len>1 || x->val[0]!=0;
}

/*--- function zparity ---------------------------------------------------
 *
 *  Report the oddness/evenness of a BIGINT.
 *
 *  Entry:  x  is a BIGINT.
 *
 *  Exit:   Returns 1 if the number is odd, else 0.
 */

int
zparity(x)
BIGINT *x;
{
   return ((int) x->val[0]&1);
}

/*--- function zmult3 ------------------------------------------------
 *
 *  Multiply two BIGINTs; output can be input.
 *
 *  Entry:  x, y  are BIGINTs.
 *
 *  Exit:   z = x*y; &z can be &x or &y if you want
 */

void
zmult3(x,y,z)  /* output can be input */
BIGINT *x;
BIGINT *y;
BIGINT *z;
{
   BIGINT temp;

   zmul(x,y,&temp);
   MCOPY(&temp,z);
}

/*--- function zlog ---------------------------------------------------
 *
 *  Compute the logarithm base 2 of a BIGINT (essentially).
 *
 *  Entry:  x  is a BIGINT.
 *
 *  Exit:   Returns the number of bits required to represent
 *          the number.
 */

MYINT
zlog(x)  /* number of bits in x */
BIGINT *x;
{
   MYINT i,j,k=0;

   for (j=0,i=1; j<NBITS; ++j,i<<=1) if (x->val[x->len-1]&i) k=j+1;
   return NBITS*(x->len-1)+k;
}

/*--- function zshiftr ---------------------------------------------------
 *
 *  Shift a BIGINT right a given number of bits.
 *
 *  Entry:  x  is a BIGINT.
 *          n  is the number of bits to shift by.
 *
 *  Exit:   x  is the BIGINT, shifted right n bits.
 */

void
zshiftr(x,n)
BIGINT *x;
MYINT n;
{
   BIGINT divisor,dummy;
   MYINT two=2;
   MYINT j;

   MSET(1,&divisor);
   for (j=0;j<n;++j) zsmul(&divisor,two,&divisor);
   zdiv(x,&divisor,x,&dummy);
}

/*--- function zshiftl ---------------------------------------------------
 *
 *  Shift a BIGINT left a given number of bits.
 *
 *  Entry:  x  is a BIGINT.
 *          n  is the number of bits to shift by.
 *
 *  Exit:   x  is the BIGINT, shifted left n bits.
 */

void
zshiftl(x,n)
BIGINT *x;
MYINT n;
{
   BIGINT multiplier;
   MYINT two = 2;
   MYINT j;

   MSET(1,&multiplier);
   for (j=0;j<n;++j) zsmul(&multiplier,two,&multiplier);
   zmult3(x,&multiplier,x);
}

/*--- function zdec2z ----------------------------------------------------
 *
 *  Convert an ASCII decimal string to BIGINT format.
 *
 *  Entry:  str   is a zero-terminated ASCII decimal integer.
 *
 *  Exit:   a     is a BIGINT with the value of that integer.
 */

void
zdec2z(str,a)
char *str;
BIGINT *a;
{
  zset((MYINT)0,a);

  while (*str) {
     zsmul(a,(MYINT)10,a);
     zsadd(a,(MYINT)(*(str++)-'0'),a);
  }
}  

/*--- function zset ---------------------------------------------------
 *
 *  Set a BIGINT to a MYINT value.
 *
 *  Entry:  c   is a MYINT.
 *          x   is a BIGINT.
 *
 *  Exit:   x = c.
 */

void
zset(c,x)
MYINT c;
BIGINT *x;
{
   x->len = 1;
   x->val[0] = c;
}

/*--- function zlowbits ---------------------------------------------------
 *
 *  Returns the least significant NBITS of a BIGINT.
 *
 *  Entry:  x  is a BIGINT.
 *
 *  Exit:   Returns the lowermost NBITS of x.
 */

MYINT
zlowbits(x)
BIGINT *x;
{
   return x->val[0];
}



/*--- function zsignadd ---------------------------------------------
 *
 *   Compute the sum of two bignums, taking into account sign.
 *
 *   Mark Riordan  16 Dec 1990
 */

void
zsignadd(a,asign,b,bsign,c,csign)
BIGINT *a;
int *asign;
BIGINT *b;
int *bsign;
BIGINT *c;
int *csign;
{
   if(*asign>=0 && *bsign>=0) {
      *csign = 1;
      zadd(a,b,c);
   } else if(*asign<0 && *bsign<0) {
      *csign = -1;
      zadd(a,b,c);
   } else {
      /* The terms have different signs, so subtract the one with
       * the smaller magnitude from the one with the larger magnitude.
       */
      if(zcompare(a,b) >= 0) {
         /* a has greater magnitude, so the sum will have its sign. */
         zsub(a,b,c);
         *csign = *asign;
      } else {
         zsub(b,a,c);
         *csign = *bsign;
      }
   }
}

/*--- function zsignsub ---------------------------------------------
 *
 *   Compute the difference of two bignums, taking into account sign.
 *
 *   Mark Riordan  16 Dec 1990
 */

void
zsignsub(a,asign,b,bsign,c,csign)
BIGINT *a;
int *asign;
BIGINT *b;
int *bsign;
BIGINT *c;
int *csign;
{
   if(*asign != *bsign) {
      /* For example, 2 - (-3)  or -2 - 3  */
      *csign = *asign;
      zadd(a,b,c);
   } else {
      /* The terms have the same sign, so subtract the one with
       * the smaller magnitude from the one with the larger magnitude.
       */
      if(zcompare(a,b) >= 0) {
         /* a has greater magnitude.
          * For example, 3 - 2  or -3 - (-2)
          */
         zsub(a,b,c);
         *csign = *asign;
      } else {
         /* b has greater magnitude.
          * For example,  2 - 3   or   -2 - (-3)
          */
         zsub(b,a,c);
         *csign = -(*bsign);
      }
   }
}

/*--- function zsignmult ---------------------------------------------
 *
 *   Compute the product of two bignums, taking into account sign.
 *
 *   Mark Riordan  16 Dec 1990
 */

void
zsignmult(a,asign,b,bsign,c,csign)
BIGINT *a;
int *asign;
BIGINT *b;
int *bsign;
BIGINT *c;
int *csign;
{
   zmult3(a,b,c);
   *csign = *asign * *bsign;
}

/*--- function zsigndiv ---------------------------------------------
 *
 *   Compute the quotient of two bignums, taking into account sign.
 *
 *   Mark Riordan  16 Dec 1990
 */

void
zsigndiv(a,asign,b,bsign,c,csign,r,rsign)
BIGINT *a;
int *asign;
BIGINT *b;
int *bsign;
BIGINT *c;
int *csign;
BIGINT *r;
int *rsign;
{
   zdiv(a,b,c,r);
   *rsign = *bsign;
   *csign = *asign * *bsign;
}


/*--- function zjacobi ---------------------------------------------
 *
 *    Computes the Jacobi symbol J(a,n)
 *
 *    For integers a and odd integers n, J(a,n) is defined as:
 *      1  if n does not divide a evenly, and a is a quadratic
 *         residue mod n.
 *      0  if n divides a evenly.
 *     -1  if n does not divide a evenly, and a is not a
 *         quadratic residue mod n.
 *
 *    Recall that a is a quadratic residue mod n if
 *    x^2 = a mod n      has an integer solution x.
 *
 *    As we all know,
 *    J(a,n) = if a==1 then 1 else
 *             if a is even then J(a/2,n) * ((-1)^(n^2-1)/8))
 *             else J(n mod a, a) * ((-1)^((a-1)*(n-1)/4)))
 *
 *    Entry    a, n are numbers
 *
 *    Exit     Returns 0, -1 or 1
 *
 *    Written by Mark Riordan    Fall 1990
 */

int
zjacobi(a,n)
BIGINT *a;
BIGINT *n;
{
#define USE_SHIFT 0
   int power;
   BIGINT temp;
   static BIGINT z_two, z_four, z_eight, z_temp, z_temp2;

   if(!ztest(a)) {
      return(0);
   } else if(zcompare(&z_one,a) == 0) {
      return(1);
   } else if(!zparity(a)) {
      /* a is even */
      zmul(n,n,&temp);
      zsadd(&temp,(MYINT)-1,&temp);
#if !USE_SHIFT
      zset((MYINT)8,&z_eight);
      zdiv(&temp,&z_eight,&temp,&z_temp);
#else
      zshiftr(&temp,3);
#endif
      /* Now temp = (n^2 - 1) / 8
       * -1 to that power = 1 if even, else -1
       */
      if(zparity(&temp)) {
         power = -1;
      } else {
         power = 1;
      }
#if !USE_SHIFT
      zset((MYINT)2,&z_two);
      zdiv(a,&z_two,&temp,&z_temp);
#else
      zcopy(a,&temp);
      zshiftr(&temp,1);
#endif
      /* Now temp = a/2 */
      return(zjacobi(&temp,n) * power);
   } else {
#if !USE_SHIFT
      zsub(a,&z_one,&temp);
      zsub(n,&z_one,&z_temp);
#else
      zsadd(a,(MYINT)-1,&temp);
      zsadd(n,(MYINT)-1,&z_temp);
#endif
      zmul(&temp,&z_temp,&z_temp2);

      /* Now z_temp2 = (a-1) * (n-1) */
#if !USE_SHIFT
      zset((MYINT)4,&z_four);
      zdiv(&z_temp2,&z_four,&temp,&z_temp);
#else
      zshiftr(&z_temp2,2);
      zcopy(&z_temp2,&z_temp);
#endif
      /* Now temp = ((a-1) * (n-1)) / 4 */
      if(zparity(&temp)) {
         power = -1;
      } else {
         power = 1;
      }
      zmod(n,a,&temp);  /* temp = n mod a */
      return(zjacobi(&temp,a) * power);
   }
}
