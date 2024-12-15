#include "pdefs.h"
#include "pcvt.h"
#include "precision.h"

/*
 * Unsigned long to Precision
 */
precision ultop(ul)
   register unsigned long ul;
{
   register digitPtr  uPtr;
   register precision u = palloc(LONGSIZE);

   if (u == pUndef) return u;

   u->sign    = false;
   uPtr	      = u->value;
   do {
      *uPtr++ = modBase(ul);
      ul      = divBase(ul);
   } while (ul != 0);

   u->size = (uPtr - u->value);			/* normalize */
   return presult(u);
}
