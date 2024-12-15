/*
 *	High Precision Math Library
 *
 *	Written by Dave Barrett					2/23/83
 *	Translated from modcal to pascal			4/30/84
 *	Mod portability fixed; removed floor function		5/14/84
 *	Fixed numerous bugs and improved robustness		5/21/84
 *	Translated to C						6/14/84
 *	Changed precision to be determined at run-time		5/19/85
 *	Added dynamic allocation				7/21/85
 *	Combined unsigned math and integer math			8/01/85
 *	Fixed Bug in pcmp					7/20/87 
 *	Fixed handling of dynamic storage (refcount added)	7/20/87
 *	Final debugging of current version             		8/22/87
 *	Fixed many bugs in various routines, wrote atop		2/07/89
 *	Tuned for speed, fixed overflow problems		3/01/89
 *	Removed refcounts, more tuning, removed pcreate		3/16/89
 *	Added cmp macros, change name of pzero, added pshift	4/29/89
 *	Repaired operation order bugs in pdiv, calc.c		5/15/91
 *      Added pdiv macro, split out pcmp, pabs, much cleanup	5/21/91
 *
 *	warning! The mod operation with negative arguments not portable.
 *		 I have therefore avoided it completely with much pain.
 *	
 *	The following identities have proven useful:
 *
 *	given:	a %  b = a - floor(a/b) * b
 *	then : -a % -b = -(a % b)
 *	       -a %  b = -( a % -b) = b - a % b (a % b != 0)
 *		a % -b = -(-a % b) = a % b - b	(a % b != 0)
 *
 *	given:	a %  b =  a - a / b * b
 *	then : -a % -b = -a % b = -(a % b)
 *		a % -b =  a % b
 *
 *	Also, be very careful of computations in the inner loops.  Much
 *	work has been done to make sure the compiler does not re-arrange
 *	expressions to cause an overflow. The compiler may still be doing
 *	unnecessary type conversions.
 *  
 * NOTES:
 *
 *	The ptoa routine creates storage which is likely to be forgotton.
 *
 *	A function returning a result must use the result.  If it doesn't
 *	the storage is never freed.  For example: itop(2); by itself
 *	You must make sure to pdestroy the result.
 *
 *	An error (out of storage) fails to deallocate u and v.
 *
 *	psub, pcmp, pdiv, and pmul all assume normalized arguments.
 *
 * This file contains the storage management-specific code:
 *     palloc, pfree, pset -- together these account for 45% of execution time
 */
#include <string.h>
#include "pdefs.h"	/* private include file */
#include "precision.h"	/* public include file for forward refs */

cacheType pcache[CACHESIZE];
static char ident[] = 
   " @(#) libprecision.a version 1.50 24-Apr-91 by Dave Barrett\n";

/*
 * normalize (used by div and sub)
 *   remove all leading zero's 
 *   force positive sign if result is zero
 */
void pnorm(u)
   register precision u;
{
   register digitPtr uPtr;

   uPtr = u->value + u->size;
   do {
      if (*--uPtr != 0) break;
   } while (uPtr > u->value);

   if (uPtr == u->value && *uPtr == 0) u->sign = false;

   u->size = (uPtr - u->value) + 1;		/* normalize */
}

/*
 * Create a number with the given size (private) (very heavily used)
 */
precision palloc(size)
   register posit size;
{
   register precision w;
   register cacheType *kludge = pcache + size;	/* for shitty compilers */

   if (size < CACHESIZE && (w = kludge->next) != pUndef) {
      kludge->next = ((cacheType *) w)->next;
      --kludge->count;
   } else {
      w = (precision) allocate(PrecisionSize + sizeof(digit) * size);
      if (w == pUndef)  {
	 w = errorp(PNOMEM, "palloc", "out of memory");
	 return w;
      }
   }
   w->refcount = 1;
   w->size     = w->alloc = size;
   return w;
}

/*
 * (Very heavily used: Called conditionally pdestroy)
 * (should be void, but some compilers can't handle it with the macro)
 */
int pfree(u)
   register precision u;
{
   register posit size;
   register cacheType *kludge; 	/* for shitty compilers */

   if (u != pUndef) {
      if (u->refcount == 0) {
	 size = u->alloc;

	 kludge = pcache + size;
	 if (size < CACHESIZE && kludge->count < CACHELIMIT) {
	    ((cacheType *) u)->next = kludge->next;
	    kludge->next = u;
	    kludge->count++;
	 } else {
	    deallocate(u);
	 }
      } else {
	 errorp(PREFCOUNT, "pfree", "refcount nonzero");
      }
   }
   return 0;
}

int pdestroyf(u)
   register precision u;
{
   if (u != pUndef && --u->refcount <= 0) {
      pfree(u);
   }
   return 0;
}

precision pparmf(u)
   register precision u;
{
   u->refcount++;
   return u;
}

precision presultf(u)
   register precision u;
{
   --(u->refcount);
   return u;
}

/*
 * assignment (u <- v)   
 *
 * This rountine is THE most called in virtually any code
 * It amounts to 15% of total execution time (slightly more than palloc/free)
 *
 * usage: pset(&i, j);
 */
precision pset(up, v)
   register precision *up, v;
{
   register precision u;

   if (v != pUndef) (void) pparm(v);
   if (up == pNull) {		/* useful voiding parameters (pdiv) */
      pdestroy(v);
      return pUndef;
   }
   u = *up;
   if (u != pUndef) {		/* useful to force initial creation */
      pdestroy(u);
   }
   *up = v;		/* notice that v may be pUndef which is OK! */
   return v;			  /* no presult! This is a variable */
}
