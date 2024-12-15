#include "pdefs.h"
#include "pcvt.h"
#include "precision.h"

/*
 *  Precision to unsigned long
 */
unsigned long ptoul(u)
   precision u;
{
   register digitPtr uPtr;
   register accumulator temp;

   (void) pparm(u);
   if (u->sign) {
      temp = (unsigned long) errorp(PDOMAIN, "ptoul", "negative argument");
   } else {
      uPtr = u->value + u->size;
      temp = 0;
      do {
	 if (temp > divBase(MAXUNSIGNEDLONG - *--uPtr)) {
	    temp = (unsigned long) errorp(POVERFLOW, "ptoul", "overflow");
	    break;
	 }
	 temp  = mulBase(temp);
	 temp += *uPtr;
      } while (uPtr > u->value);
   }
   pdestroy(u);
   return (unsigned long) temp;
}
