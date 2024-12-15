#include "precision.h"

/*
 * Raise to integer power
 */
precision pipow(u, n)
   precision 	u;
   unsigned 	n;
{
   precision j = pUndef, i = pUndef;

   pset(&j, pone);
   pset(&i, pparm(u));

   do {
      if (n & 1) pset(&j, pmul(i, j));
      n >>= 1;
      if (n == 0) break;
      pset(&i, pmul(i, i));
   } while (1);

   pdestroy(i);
   pdestroy(u);
   return presult(j);
}
