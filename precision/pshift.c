#include <string.h>
#include "pdefs.h"
#include "precision.h"

/*
 * Shift a precision left by count digits (0's into lsd)
 * Count may be negative, which shifts right.
 * The shift occurs using radix BASE.
 */
precision pshift(u, count)
   register precision u;
   register int count;
{
   precision w = pUndef;

   if (count == 0) {
      return u;
   }
   (void) pparm(u);
   if (count > 0) {
      w = palloc(u->size + count);
      w->sign = u->sign;
      (void) memset(w->value, 0, count * sizeof(digit));
      (void) memcpy(w->value + count, u->value, u->size * sizeof(digit));
   } else {
      if (count > u->size) {	/* if all sig digits shifted off */
	 pset(&w, pzero);
      } else {
	 w = palloc(u->size - count);
	 w->sign = u->sign;
	 (void) memcpy(
	    w->value, u->value + count, (u->size - count) * sizeof(digit) 
	 );
      }
   }
   pdestroy(u);
   return presult(w);
}
