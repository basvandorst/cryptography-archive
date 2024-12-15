#include "precision.h"

/*
 * Return the floor af the log base b of a precision
 */
int plogb(p, b)
   precision p, b;
{
   register int res = 0;
   unsigned int i = 1;		    /* must be unsigned for >> to work */

   precision u = pUndef;
   precision stack[32];			/* 2 ^ (2 ^ 32) is pretty big! */
   register  precision *sp = stack;

   (void) pparm(b);
   (void) pparm(p);
   if (plez(p) || plt(b, ptwo)) {
      res = -1;
   } else {
      *sp = pUndef;			/* find highest successive square */
      pset(sp, b);
      while ple(*sp, p) {
	 pset(&u, pmul(*sp, *sp));
	 *++sp = pUndef;
	 pset(sp, u);
	 i <<= 1;
      }
      pset(&u, p);			/* unravel squares to get log */
      do {
	 if ple(*sp, u) {
	    pdivmod(u, *sp, &u, pNull);
	    res += i;
	 }
	 pdestroy(*sp);
	 --sp;
      } while ((i >>= 1) > 0);
   }
   pdestroy(u);
   pdestroy(p);
   pdestroy(b);
   return res;
}
