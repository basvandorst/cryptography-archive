#include "pdefs.h"
#include "precision.h"

/*
 * convert a given base to precision
 *
 * Input:
 *    src    - the character radix representation
 *    size   - the number of characters to attempt to convert
 *    digmap - a 256 element integer array with the
 *             value corresponding to each digit.  
 *             -1 should be stored for invalid characters.
 *    radix  - The radix to use for conversion
 *    p      - the result
 *
 * Returns: 
 *   >= 0 - The number of characters consumed
 *   -1   - if invalid argument:
 */
int btop(p, src, size, digmap, radix)
   precision *p;
   char *src;
   register unsigned int size;
   int digmap[];
   unsigned int radix;
{
   register unsigned char *chp = (unsigned char *) src;
   register unsigned int i; 
   register int d;
   register accumulator temp;
   accumulator x;
   unsigned int lgclump;
   precision clump = pUndef;

   if (radix < 2 || radix > 256) {
      return -1;
   }
   if (size == 0) {
      return 0;
   }

   temp = radix;
   i    = 1;
   while (temp * radix > temp) {
     temp *= radix;
     i++;
   }
   lgclump = i;
   pset(&clump, utop(temp));

   pset(p, pzero);
   do {
      i    = lgclump;
      temp = 0;
      do {
	 d = digmap[*chp++];
	 if (size-- == 0 || d < 0) goto btoplast;
	 temp = temp * radix + d;
      } while (--i > 0);
      pset(p, padd(pmul(*p, clump), utop(temp)));
   } while(1);

btoplast:
   x = 1;
   while (i++ < lgclump) {
      x *= radix;
   }
   pset(p, padd(pmul(*p, utop(x)), utop(temp)));
   pdestroy(clump);
   return chp - (unsigned char *) src;
}
