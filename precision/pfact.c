#include "precision.h"

precision factorial(n)
   precision n;
{
   precision res = pUndef;

   pparm(n);
   if (peqz(n)) {
      pset(&res, pone);
   } else {
      pset(&res, pmul(n, factorial(psub(n, pone))));
   }
   pdestroy(n);
   return presult(res);
}
