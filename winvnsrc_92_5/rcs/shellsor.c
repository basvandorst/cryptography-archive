head	1.8;
access;
symbols
	V80:1.1
	V76d:1.1;
locks; strict;
comment	@ * @;


1.8
date	94.09.03.01.08.03;	author rushing;	state Exp;
branches;
next	1.7;

1.7
date	94.09.02.23.50.14;	author rushing;	state Exp;
branches;
next	1.6;

1.6
date	93.12.08.01.28.38;	author rushing;	state Exp;
branches;
next	1.5;

1.5
date	93.07.08.19.41.06;	author rushing;	state Exp;
branches;
next	1.4;

1.4
date	93.07.06.21.09.09;	author cnolan;	state Exp;
branches;
next	1.3;

1.3
date	93.06.29.20.06.34;	author rushing;	state Exp;
branches;
next	1.2;

1.2
date	93.06.28.17.53.24;	author rushing;	state Exp;
branches;
next	1.1;

1.1
date	93.02.16.20.53.50;	author rushing;	state Exp;
branches;
next	;


desc
@winvn version 0.76 placed into RCS
@


1.8
log
@define huge away for win32
@
text
@
/*
 *
 * $Id: shellsor.c 1.7 1994/09/02 23:50:14 rushing Exp rushing $
 * $Log: shellsor.c $
 * Revision 1.7  1994/09/02  23:50:14  rushing
 * shellsort is now huge
 *
 * Revision 1.6  1993/12/08  01:28:38  rushing
 * new version box and cr lf consistency
 *
 * Revision 1.5  1993/07/08  19:41:06  rushing
 * fixed unsigned bug again.  strange.
 *
 * Revision 1.4  1993/07/06  21:09:09  cnolan
 * win32 support
 *
 * Revision 1.2  1993/06/28  17:53:24  rushing
 * fixed compiler warnings
 *
 * Revision 1.1  1993/02/16  20:53:50  rushing
 * Initial revision
 *
 *
 */

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

#ifdef WIN32
#include <windef.h>
#endif

#include <stdio.h>
#include <string.h>

#ifdef WIN32
  #define __far far
  #define huge far
  #define       __near near
#endif


#define memSwap(a,b,unused) tmp = *(char huge * huge *)(a); \
  *(char huge * huge *)(a) = *(char huge * huge *)(b); *(char huge * huge *)(b) = tmp;

void
ShellSort (void huge * huge * base,
	   size_t nElements,
	   size_t width,
	   int (*compare) (void const huge *elem1, void const huge * elem2))
{
#define STRIDE_FACTOR 3
  unsigned int c, stride;
  int d;
  char far *tmp;
  int found;

  stride = 1;
  while (stride <= nElements)
    stride = stride * STRIDE_FACTOR + 1;

  while (stride > (STRIDE_FACTOR - 1))
    {
      stride = stride / STRIDE_FACTOR;
      for (c = stride; c < nElements; c++)
	{
	  found = 0;
	  d = c - stride;
	  while ((d >= 0) && !found)
	    {
	      if (compare ((char huge *) base + (d + stride) * width, (char huge *) base + d * width) < 0)
		{
		  memSwap ((char huge *) base + (d + stride) * width, (char huge *) base + d * width, width);
		  d -= stride;
		}
	      else
		{
		  found = 1;
		}
	    }
	}
    }
}
@


1.7
log
@shellsort is now huge
@
text
@d4 1
a4 1
 * $Id: shellsor.c 1.6 1993/12/08 01:28:38 rushing Exp rushing $
d6 3
d64 3
a66 1
#  define __far far
d68 1
@


1.6
log
@new version box and cr lf consistency
@
text
@d4 1
a4 1
 * $Id: shellsor.c 1.5 1993/07/08 19:41:06 rushing Exp rushing $
d6 3
d64 2
a65 3
#define memSwap(a,b,unused) tmp = *(char far * far *)(a); \
  *(char far * far *)(a) = *(char far * far *)(b); *(char far * far *)(b) = tmp;

d67 5
a71 6
void __far
ShellSort (base, nElements, width, compare)
     void far *base;
     size_t nElements;
     size_t width;
     int (__far * compare) (void const far * elem1, void const far * elem2);
d92 1
a92 1
	      if (compare ((char far *) base + (d + stride) * width, (char far *) base + d * width) < 0)
d94 1
a94 1
		  memSwap ((char far *) base + (d + stride) * width, (char far *) base + d * width, width);
@


1.5
log
@fixed unsigned bug again.  strange.
@
text
@d1 1
d4 1
a4 1
 * $Id: shellsor.c 1.4 1993/07/06 21:09:09 cnolan Exp rushing $
d6 3
@


1.4
log
@win32 support
@
text
@d3 1
a3 1
 * $Id: shellsor.c 1.2 1993/06/28 17:53:24 rushing Exp $
d5 3
d69 2
a70 1
  unsigned int c, d, stride;
@


1.3
log
@fixed unsigned/signed bug
.,
@
text
@d3 1
a3 1
 * $Id: shellsor.c 1.2 1993/06/28 17:53:24 rushing Exp rushing $
d43 4
d50 4
d66 1
a66 2
  unsigned int c, stride;
  int d;
@


1.2
log
@fixed compiler warnings
@
text
@d3 1
a3 1
 * $Id: shellsor.c 1.1 1993/02/16 20:53:50 rushing Exp rushing $
d5 3
d58 2
a59 1
  unsigned int c, d, stride;
@


1.1
log
@Initial revision
@
text
@d3 4
a6 2
 * $Id$
 * $Log$
d8 1
d55 1
a55 1
  int c, d, stride;
@
