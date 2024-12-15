#include "pdefs.h"
#include "pcvt.h"
#include "precision.h"

/*
 *  Precision to integer 
 */
int ptoi(u)
   precision u;
{
   register digitPtr uPtr;
   register accumulator temp;
   int res;

   (void) pparm(u);
   uPtr = u->value + u->size;
   temp	= 0;
   do {
      if (temp > divBase(MAXINT - *--uPtr)) {
         temp = (int) errorp(POVERFLOW, "ptoi", "overflow");
	 break;
      }
      temp  = mulBase(temp);
      temp += *uPtr;
   } while (uPtr > u->value);

   if (u->sign) res = - (int) temp; else res = (int) temp;

   pdestroy(u);
   return res;
}
