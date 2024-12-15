/*--- mcrt.c -- Routine to apply Chinese Remainder Theorem.   */
#include <stdio.h>
#include "zbigint.h"
#include "zproto.h"
#include "zmiscpro.h"
#include "megcdpro.h"

#include "p.h"

static int mcrt2 P((BIGINT *r1,BIGINT *m1,BIGINT *r2,BIGINT *m2,
  BIGINT *cr,int *crsign));

/*
 * Apply Chinese Remainder Theorem.  6/23/89 bsy@cs.cmu.edu
 *
 * NEW CODE.  NOT DERIVED FROM MP
 *
 * precodition:
 *  All BIGINTs initialized.
 *  (mods_i,mods_j) = 1 for all i != j.  [independence of congruence eqns]
 * postcondition:
 *  for all i cr == resids_i mod mods_i if such exists,
 *  cr == 0 otherwise.
 */

int
mcrt(num,resids,mods,prodmod,cr)
int   num;
BIGINT   *resids, *mods, /* out */ *prodmod, *cr;
{
   BIGINT   r;
   int crsign, rsign=1;

   if (!num) {
      MFREE(cr); MSET(0,cr); return 0;
   }

   MINIT(&r);
   zcopy(resids,&r); zcopy(mods,prodmod);
   ++resids; ++mods; --num;

   while (--num >= 0) {
      if (!mcrt2(&r,prodmod,resids,mods,cr,&crsign)) {
         MFREE(&r);
         return 0;
      }
      zmult3(prodmod,mods,prodmod);

      /* canonicalize */
      if (crsign < 0) {
         zmod(cr,prodmod,cr);
         zsub(prodmod,cr,cr);
      } else if(zcompare(prodmod,cr) < 0) {
         zmod(cr,prodmod,cr);
      }
      if (num) {
         MMOVEFREE(cr,&r);
         MINIT(cr);
      }
      ++resids; ++mods;
   }
   MFREE(&r);
   return 1;
}

static int
mcrt2(r1,m1,r2,m2,cr,crsign)
BIGINT   *r1,*m1,*r2,*m2,*cr;
int *crsign;
{
   BIGINT   inv1,inv2,gcd,one;
   int   sign1=1, sign2=1, s_r1m2inv2, s_r2m1inv1;

   MINIT(&inv1); MINIT(&inv2); MINIT(&gcd); MSET(1,&one);
   megcd(m1,m2,&inv1,&sign1,&inv2,&sign2,&gcd);
   if (zcompare(&gcd,&one) != 0) {
      MFREE(&inv1); MFREE(&inv2); MFREE(&gcd); MFREE(&one);
      return 0;
   }
   zmult3(r1,m2,cr);
   zmult3(cr,&inv2,cr);
   s_r1m2inv2 = sign2;

   zmult3(r2,m1,&one);
   zmult3(&one,&inv1,&one);
   s_r2m1inv1 = sign1;

   /* Add the terms "cr" and "one", taking into account sign. */
   /* MRR */
   zsignadd(&one,&s_r2m1inv1,cr,&s_r1m2inv2,cr,crsign);

   MFREE(&inv1); MFREE(&inv2); MFREE(&gcd); MFREE(&one);
   return 1;
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
