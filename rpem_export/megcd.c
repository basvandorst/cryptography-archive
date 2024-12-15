/*--- megcd.c -- Routine to find the Greatest Common Divisor of two #s */

#include <stdio.h>
#include <stdlib.h>
#include "zbigint.h"
#include "zproto.h"
#include "zmiscpro.h"
#include "megcdpro.h"

#include "p.h"  

static void
megcd_recurse P((BIGINT *x,BIGINT *y,BIGINT *w,int *wsign,
  BIGINT *z,int *zsign,BIGINT *gcd));

/*--- function megcd --------------------------------------------
 *
 *  Compute the Greatest Common Divisor of two numbers.
 *
 *  Extended Euclidean Algorithm.  6/23/89 bsy@cs.cmu.edu
 *  NEW CODE.  NOT DERIVED FROM MP
 *
 *  Entry:  p and q are BIGINTs
 *
 *  Exit:   gcd     is the Greatest Common Divisor of p and q.
 *          m and n are signed BIGINTs such that m * p + n * q = gcd.
 *          msign and nsign are integers that give the sign of
 *                  m and n; msign and nsign are each 1 or -1.
 */

void
megcd(p,q,m,msign,n,nsign,gcd)
BIGINT   *p,*q, /* out */ *m, *n, *gcd;
int *msign, *nsign;
{
   BIGINT   x, y;

   MINIT(&x); MINIT(&y);
   zcopy(p,&x); zcopy(q,&y);
   megcd_recurse(&x,&y,m,msign,n,nsign,gcd);
}
/*
 * postcondition:
 *
 * m * p + n * q = gcd
 */

static void
megcd_recurse(x,y,w,wsign,z,zsign,gcd)
BIGINT   *x,*y,*w,*z,*gcd;
int *wsign,*zsign;
{
/* Change by MRR to allocate memory on the heap rather than the
 * stack when running under MS-DOS (or OS/2).  Otherwise,
 * recursion can blow the stack, which is limited in size
 * even in Large Model programs.
 */
   int  dsign=1;  /* MRR */
#ifdef MSDOS
   BIGINT *dp = malloc(sizeof(BIGINT));

   if(!dp) zhalt("Cannot allocate memory in megcd.");
#else
   BIGINT   d, *dp;

   dp = &d;
#endif

   if (ztest(x) == 0) {
      zcopy(y,gcd);
      MFREE(w); MFREE(z);
      MSET(0,w); *wsign=1;
      MSET(1,z); *zsign=1;
      return;
   }
   MINIT(dp);
   zdiv(y,x,dp,y);
   megcd_recurse(y,x,z,zsign,w,wsign,gcd);
   zsignmult(dp,&dsign,z,zsign,dp,&dsign);
   zsignsub(w,wsign,dp,&dsign,w,wsign);

   MFREE(dp);
#ifdef MSDOS
   free(dp);
#endif
}
/*
 * Copyright (C) 1987, Bennet Yee.  You may freely distribute this code as long
 * as this notice is intact.  If you make any modifications in your
 * distribution, you must clearly document them.  If you distribute this code
 * or any derivative thereof, it must be licensed at no charge to all parties
 * on terms identical to those described herein.  All distributions must
 * contain source code.
 *
 * This software is provided with absolutely no warranty.
 *
 * If you have questions, you can contact me at bsy@a.cs.cmu.edu
 */
