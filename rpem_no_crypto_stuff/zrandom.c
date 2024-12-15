#include <stdio.h>
#include "zbigint.h"
#include "zproto.h"

/*--- function zrandom ---------------------------------------------------
 *
 *  Return a pseudo-random BIGINT.
 *  Not cryptologically strong, because zrandombits() isn't.
 *
 *  Entry:  n  is a BIGINT (the modulus).
 *
 *  Exit:   s  is a pseudo-random BIGINT such that 0 <= s < n.
 *
 *  By Mark Riordan, based on a similar routine by Marc Ringuette.
 *  This code is in the public domain.
 */

void
zrandom(s,n)
BIGINT *s;
BIGINT *n;
{
   long int i, j, bits = zlog(n);
   BIGINT   shift, randdigit;

   do {
      MFREE(s);
      MSET(0,s);
      MINIT(&randdigit);
      MSET(1 << (MRANDBITS - 1),&shift);
      zadd(&shift,&shift,&shift);
      j = bits/MRANDBITS;
      for (i = 0; i < j; i++) {
         zmult3(s,&shift,s);
         MFREE(&randdigit);
         MSET((zrandombits()&(TWO_TO_THE_MRANDBITS-1)),&randdigit);
         zadd(&randdigit,s,s);
      }
      j = bits % MRANDBITS;
      if (j) {
         j = 1 << j;
         MFREE(&shift);
         MSET(j,&shift);
         zmult3(s,&shift,s);
         MFREE(&randdigit);
         MSET(zrandombits()&(j-1),&randdigit);
         zadd(&randdigit,s,s);
      }
      MFREE(&shift);
      MFREE(&randdigit);
   } while (zcompare(s,n) >= 0);
}
