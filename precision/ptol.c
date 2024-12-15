#include "pdefs.h"
#include "pcvt.h"
#include "precision.h"

/*
 *  Precision to long 
 */
long ptol(u)
   precision u;
{
   register digitPtr uPtr;
   register accumulator temp;
   int res;

   (void) pparm(u);
   uPtr = u->value + u->size;
   temp	= 0;
   do {
      if (temp > divBase(MAXLONG - *--uPtr)) {
         temp = (long) errorp(POVERFLOW, "ptol", "overflow");
	 break;
      }
      temp  = mulBase(temp);
      temp += *uPtr;
   } while (uPtr > u->value);

   if (u->sign) res = - (long) temp; else res = (long) temp;

   pdestroy(u);
   return res;
}
