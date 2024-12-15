#include "pdefs.h"
#include "precision.h"

extern unsigned short rand16();
extern unsigned long  srand16();

/*
 * Generate a random precision of n digits (for debugging and profiling)
 */
precision prandnum(n)
   register posit n;
{
   register digitPtr uPtr;
   precision u = palloc(n);

   uPtr = u->value;
   if (n > 0) do {
      *uPtr++ = modBase(rand16());
   } while (--n);

   u->sign = (rand16() > 32767);

   pnorm(u);
   return presult(u);
}

/*
 * Generate a random precision { w | zero <= w < u }
 */
precision prand(u)
   precision u;
{
   precision w = pUndef;

   (void) pparm(u);
   pset(&w, pmod(pabs(prandnum(u->size)), u));

   pdestroy(u);
   return presult(w);
}
