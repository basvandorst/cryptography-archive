#include "precision.h"

/*
 * Raise to precision power
 */
precision ppow(u, v)
   precision 	u, v;
{
   precision j = pUndef, i = pUndef, n = pUndef;

   pset(&i,   pparm(u));
   pset(&n,   pparm(v));
   pset(&j,   pone);

   do {
      if (podd(n)) pset(&j, pmul(i, j));
      pset(&n, phalf(n));
      if (peqz(n)) break;
      pset(&i, pmul(i, i));
   } while (1);

   pdestroy(i); pdestroy(n); pdestroy(u); pdestroy(v);
   return presult(j);
}
