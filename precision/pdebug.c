#include <stdio.h>
#include "pdefs.h"
#include "precision.h"

extern cacheType pcache[];

/*
 * For debugging
 */
void pshow(p)
   precision p;
{
   digitPtr dPtr;
  
   if (p == pUndef) {
      printf("undefined");
   } else {
      printf("%.*x[%d](%d) =>%d sign:%.*x @%.*x = ",
	 (sizeof (precision)) * 2, p,
	 p->alloc, p->size, p->refcount,
	 (sizeof p->sign) * 2 , p->sign,
	 (sizeof(digitPtr)) * 2, p->value
      );
      dPtr = p->value + p->size; 
      do {
	 printf("%.*x ", (sizeof(digit)) * 2, *--dPtr);
      } while (dPtr > p->value);
   }
   putchar('\n');
}

/*
 * phistogram - Show a histogram of the freespace cache
 */
void phistogram(stream)
   FILE *stream;
{
   register int size;

   for (size = 0; size < CACHESIZE; size++) {
      if (pcache[size].count != 0) {
	 fprintf(stream, "%3d: %d\n", size, pcache[size].count);
      }
   }
}
