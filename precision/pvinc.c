#include "precision.h"

#define SIZE 10

/*
 * Example to illustrate vector usage
 *
 * This is not meant to be efficient or elegent.  It is designed to illustate
 * how to us pvectors.
 * 
 * This function sets each element of the pvector r to the one more than
 * the corresponding element of the pvector v.  Only "size" elements are
 * effected.
 */
void pvinc(r, v, size)
   pvector r, v;
   unsigned size;
{
   unsigned i;
   precision plocal[SIZE];		/* local variable and static size */
   pvector pvone = pvalloc(size);	/* dynamic variable and size */

   pvundef(plocal, SIZE);		/* initialize local variables only */
					/* done exactly once per variable */
   /*
    * Notice that we did not need to do a pparm for arguments of type pvector
    */
   if (pvone == pNull) {
      errorp(PNOMEM, "pvaddone", "out of memory");
   } else {
      (void) pvset(pvone, size, pone);	/* set all elements to 1 */
      if (v != pNull) {
	 for (i=0; i< size; i++) {
	    /*
	     * plocal[i] = v[i] + 1;  Assumes that size <= SIZE
	     */
	    pset(&plocal[i], padd(v[i], pvone[i]));
	 }
      }
   }

   for (i = 0; i < size; i++) {
      pset(&r[i], plocal[i]);		/* r must be an initialized pvector */
   }

   pvdestroy(plocal, SIZE);		/* finalize local variables only */
   pvfree(pvone, size);			/* don't forget to use correct size! */
   /*
    * Notice that we did not need to do a presult for r.
    */
}
