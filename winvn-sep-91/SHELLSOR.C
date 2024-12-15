
/* Shellsort is a variation on Insertion Sort that is virtually unbeatable 
 * on small data sets.  Insertion Sort is slow because it only exchanges 
 * adjacent elements.  Shellsort circumvents this by allowing the exchange 
 * of elements that are farther apart.  It works by first performing Insertion 
 * Sort on subsets of the data.  For example, Shellsort might first sort 
 * the set of elements 1, 6, 11, 16... relative to each other--the effect 
 * is that the elements in that subset are moved much closer to their final 
 * positions.  At first the sets are wide-spread, perhaps every fortieth 
 * element.  Then it repeats using a tighter set, perhaps every fourteenth 
 * element, then every fourth element.  Each of these passes is much cheaper 
 * than a traditional Insertion Sort pass.  The effect of the passes is that 
 * the data set is mutated to be in "almost sorted" order.  The final insertion 
 * sort pass can then go very quickly because insertion sort does very well on 
 * almost sorted data.  In other words, at first the elements take large leaps 
 * to the general location they belong and successively smaller steps move the 
 * element to its precise place. I call this algorithm "inscrutable sort" 
 * because even after having coded the algorithm, I still have trouble 
 * following the animation.  No one has been able to analyze this algorithm 
 * rigorously; the functional form of the running time is conjectured to be 
 * O(N^1.25).  Shellsort may be a bit mysterious, but it is an good general 
 * purpose sort since it has excellent performance and the code you must write 
 * is only slightly more complex than Insertion Sort.
 *
 * Author: Julie Zelenski, NeXT Developer Support
 * You may freely copy, distribute and reuse the code in this example.  
 * NeXT disclaims any warranty of any kind, expressed or implied, as to 
 * its fitness for any particular use.      qsort
 */

#include <stdio.h>
#include <string.h>

#define memSwap(a,b,unused) tmp = *(char far * far *)(a); \
  *(char far * far *)(a) = *(char far * far *)(b); *(char far * far *)(b) = tmp;


void _FAR_ _cdecl
ShellSort(base, nElements, width, compare)
void far *base;
size_t nElements;
size_t width;
int (_FAR_ _cdecl *compare)( const void far *elem1, const void far *elem2 );
{
#define STRIDE_FACTOR 3
   int c,d, stride;
   char far *tmp;
   int found;

   stride = 1;
   while (stride <= nElements) stride = stride*STRIDE_FACTOR +1;
    
   while (stride>(STRIDE_FACTOR-1)) {
    	stride = stride / STRIDE_FACTOR;
	   for (c = stride; c < nElements; c++){
	      found = 0;
	      d = c-stride;
	      while ((d >= 0) && !found) {
	         if (compare((char far *)base+(d+stride)*width, (char far *)base+d*width) < 0) {
               memSwap((char far *)base+(d+stride)*width,(char far *)base+d*width,width);
		         d -= stride;
		      } else {
		         found = 1;
            }
	      }
	   }
   }
}
	
