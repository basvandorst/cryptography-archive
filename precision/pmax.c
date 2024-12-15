#include "precision.h"

/*
 * Return the maximum of two precisions
 */
precision pmax(u, v)
   precision u, v;
{
   precision res = pUndef;

   if (pge(pparm(u), pparm(v))) {
      pset(&res, u);
   } else {
      pset(&res, v);
   }
   pdestroy(u);
   pdestroy(v);
   return presult(res);
}

