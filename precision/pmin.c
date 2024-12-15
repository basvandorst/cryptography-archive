#include "precision.h"

/*
 * Return the minimum of two precisions
 */
precision pmin(u, v)
   precision u, v;
{
   precision res = pUndef;

   if (ple(pparm(u), pparm(v))) {
      pset(&res, u);
   } else {
      pset(&res, v);
   }
   pdestroy(u);
   pdestroy(v);
   return presult(res);
}

