/*
 * precision vector operations
 */
#include "precision.h"
#include <string.h>

/*
 * Set all elements of an uninitialized precision vector to pUndef
 *
 * This is used to initialize automatic variables
 *
 * Do NOT use this on precision vectors which has valid precision values in it!
 */
pvector pvundef(pv, size)
   register pvector pv;
   register unsigned size;
{
   memset((char *) pv, '\0', size * sizeof (precision));
   return pv;
}

/*
 * Destroy all elements of a precision vector of the specified size
 *
 * (inverse of pvundef) used to clear out automatic storage
 */
void pvdestroy(pv, size)
   register pvector pv;
   register unsigned size;
{
   if (size != 0) do {
      pdestroy(*pv);
      *pv++ = pUndef;
   } while (--size);
}

/*
 * Create a new precision vector (returned vector is already pvundef'd)
 */
pvector pvalloc(size)
   unsigned size;
{
   pvector pv = (pvector) malloc(size * sizeof (precision));

   if (pv != (pvector) 0) {
      pvundef(pv, size);
   }
   return pv;
}

/*
 * Deallocate a precision vector
 */
void pvfree(pv, size)
   register pvector pv;
   register unsigned size;
{
   if (pv != (pvector) 0) {
      pvdestroy(pv, size);
      free(pv);
   }
}

/*
 * Set all the elements of a precision vector to the given value
 */
pvector pvset(pv, size, value)
   register pvector pv;
   register unsigned size;
   register precision value;
{
   if (pv != (pvector) 0 && size != 0) do {
      pset(pv++, value);
   } while (--size);
   return pv;
}
