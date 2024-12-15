head	 1.9;
branch   ;
access   ;
symbols  V80:1.5;
locks    ; strict;
comment  @ * @;


1.9
date	 94.09.18.00.45.40;  author jcooper;  state Exp;
branches ;
next	 1.8;

1.8
date	 94.01.21.23.17.18;  author rushing;  state Exp;
branches ;
next	 1.7;

1.7
date	 93.12.08.01.27.21;  author rushing;  state Exp;
branches ;
next	 1.6;

1.6
date	 93.06.25.20.44.45;  author dumoulin;  state Exp;
branches ;
next	 1.5;

1.5
date	 93.06.10.18.29.34;  author rushing;  state Exp;
branches ;
next	 1.4;

1.4
date	 93.06.08.19.41.56;  author rushing;  state Exp;
branches ;
next	 1.3;

1.3
date	 93.06.05.03.18.25;  author rushing;  state Exp;
branches ;
next	 1.2;

1.2
date	 93.06.04.16.46.44;  author rushing;  state Exp;
branches ;
next	 1.1;

1.1
date	 93.06.01.18.15.12;  author rushing;  state Exp;
branches ;
next	 ;


desc
@routines for handling the header and thread index arrays
@


1.9
log
@rearranged headers to allow use of precompiled headers
@
text
@
/* headarry.c
 * author: Sam Rushing
 * $Id: headarry.c 1.8 1994/01/21 23:17:18 rushing Exp $
 * $Log: headarry.c $
 * Revision 1.8  1994/01/21  23:17:18  rushing
 * Documented the thread sort algorithm.
 *
 * Revision 1.7  1993/12/08  01:27:21  rushing
 * new version box and cr lf consistency
 * *
 * routines for handling the array of header information
 *
 * Revision 1.6  1993/06/25  20:44:45  dumoulin
 * Change dates from strings to Posix standard date formats
 *
 * Revision 1.5  1993/06/10  18:29:34  rushing
 * added set_index_to_identity to prepare for writing article ranges
 *
 * Revision 1.4  1993/06/08  19:41:56  rushing
 * changes to support Sort... menu
 *
 * Revision 1.3  1993/06/05  03:18:25  rushing
 * primitive functional threading.
 *
 * Revision 1.2  1993/06/04  16:46:44  rushing
 * stable version of shellsort
 *
 * Revision 1.1  1993/06/01  18:15:12  rushing
 * Initial revision
 *
 *
 */ 

#include <windows.h>
#include <windowsx.h>
#include "wvglob.h"
#include "winvn.h"
#pragma hdrstop

/* The header and thread arrays are set up as follows:
 * When we allocate space for the TypHeader array, we leave room for a
 * pointer at the very front of that space, which will either indicate 
 * that there is no thread index array (== NULL) or will point to that
 * array.  Since windows can move both of these arrays around, this
 * slot is updated whenever lock_headers is called.
 * 
 * After initialize_header_array is called, this sequence of
 * operations can be used to access the header array:
 * 1. header_p headers = lock_headers (header_handle, thread_handle);
 * 2. header_elt (headers, index);
 * 3. unlock_headers (header_handle, thread_handle);
 *
 * The thread_index array is an array of longs, each an index into the
 * the real header array.  If thread_index is allocated and filled,
 * header_elt will indirect through it.  Sorting the array of indices
 * will sort the headers accordingly.
 */ 

/* globals, yuck! */
header_p g_headers;
thread_array g_index, g_parent, g_parsort;
long g_length;

/* lock the headers and the thread index array in memory for access */

header_p
lock_headers (HANDLE header_handle,HANDLE thread_handle)
{
  thread_array_p indirect;
  header_p headers;

  /* lock the header array in position */
  indirect = (thread_array_p) GlobalLock (header_handle);

  headers = (header_p) ((char_p) indirect + sizeof (char_p));
  
  /* if we've a valid thread_handle, lock the thread_array, too */
  if (thread_handle) {
    *indirect = (thread_array) GlobalLock (thread_handle);
  }
  else
    *indirect = NULL;
  
  return (headers);
}

/* unlock the headers and thread index array */

void
unlock_headers (HANDLE header_handle, HANDLE thread_handle)
{
  GlobalUnlock (header_handle);
  if (thread_handle)
    GlobalUnlock (thread_handle);
}


/* return the header memory to windows */

void
free_headers (HANDLE header_handle, HANDLE thread_handle)
{
  GlobalFree (header_handle);
  if (thread_handle)
    GlobalFree (thread_handle);
}


void
set_index_to_identity (HANDLE header_handle, HANDLE thread_handle, long length)
{
  long i;
  header_p headers;
  thread_array thread_index;
  thread_array_p thread_index_p;

  headers = lock_headers (header_handle, thread_handle);

  if (thread_handle) {
    thread_index_p  = (thread_array_p) ((char_p) headers - sizeof (char_p));
    thread_index = *thread_index_p;
    
    /* Initialize with identity */
    for (i=0; i <length ; i++)
      thread_index[i] = i;
  }
}


/* set up the header array, and possibly the thread index array */

void
initialize_header_array (HANDLE header_handle, HANDLE thread_handle, long length)
{
  long i;
  header_p headers;
  thread_array thread_index;
  thread_array_p thread_index_p;

  headers = lock_headers (header_handle, thread_handle);

  if (thread_handle) {
    thread_index_p  = (thread_array_p) ((char_p) headers - sizeof (char_p));
    thread_index = *thread_index_p;
    
    /* Initialize with identity */
    for (i=0; i <length ; i++)
      thread_index[i] = i;
  }

  for (i=0; i < length ; i++) {
    headers[i].Seen = (char) 0;
    headers[i].Selected = (char) 0;
    headers[i].number = 0;
    headers[i].thread_depth = 0;
    headers[i].lines = 0;
    headers[i].date =  0;
    headers[i].subject[0] = (char) 0;
    headers[i].from[0] = (char) 0;
    headers[i].message_id[0] = (char) 0;
    headers[i].references[0] = (char) 0;
    headers[i].ArtDoc = NULL;
  }
  unlock_headers (header_handle, thread_handle);
}

/* Use this routine to get at an element of the header array - it  */
/* will automatically indirect through the thread index array if it's */
/* there. */

header_p
header_elt (header_p headers, long index)
{
  
  thread_array thread_index;
  thread_index  = *((thread_array_p) ((char_p) headers - sizeof (char_p)));  

  if (thread_index) {
    return (&(headers[thread_index[index]]));
  }
  else
    return (&(headers[index]));
}
      
int
compare_artnum (header_p headers,
		long elem1, long elem2)
{
  long e1,e2;
  e1=headers[elem1].number;
  e2=headers[elem2].number;
  if (e1 == e2)
    return 0;
  else if (e1 < e2)
    return -1;
  else
    return 1;
}

      
int
compare_lines (header_p headers,
		long elem1, long elem2)
{
  long e1,e2;
  e1=headers[elem1].lines;
  e2=headers[elem2].lines;
  if (e1 == e2)
    return 0;
  else if (e1 < e2)
    return -1;
  else
    return 1;
}

int  
compare_message_id (header_p headers,
		    long elem1, long elem2)
{
  return strcmp (headers[elem1].message_id , headers[elem2].message_id);
}

  
int
compare_subject (header_p headers,
		 long elem1, long elem2)
{
  return stricmp (headers[elem1].subject , headers[elem2].subject);
}

int
compare_from (header_p headers,
		 long elem1, long elem2)
{
  return stricmp (headers[elem1].from , headers[elem2].from);
}

int
compare_date (header_p headers,
	      long elem1, long elem2)
{
  long e1,e2;
  e1=headers[elem1].date;
  e2=headers[elem2].date;
  if (e1 == e2)
    return 0;
  else if (e1 < e2)
    return -1;
  else
    return 1;
}

/* this is the shell sort taken from shellsor.c and modified for */
/* threading purposes... The extra comparison is to make the sort */
/* stable w.r.t. article number */

void
shell_sort_index_array (header_p headers,
			thread_array index,
			long nElements,
			int (*compare) (header_p headers,
					long elem1,
					long elem2))
{
#define STRIDE_FACTOR 3
  int c, d, stride;
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
	      int comp = compare (headers, index[d + stride], index[d]);
	      if ((comp < 0) ||
		  ((comp == 0) &&
		   (compare_artnum (headers, index[d + stride], index[d]) < 0)))
		{
		  long tmp = index[d];
		  index[d] = index[d+stride];
		  index[d+stride] = tmp;
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


void
shell_sort_parent_array (thread_array index,
			 thread_array parents,
			 long n)
{
#define STRIDE_FACTOR 3
  int c, d, stride;
  int found;

  stride = 1;
  while (stride <= n)
    stride = stride * STRIDE_FACTOR + 1;

  while (stride > (STRIDE_FACTOR - 1))
    {
      stride = stride / STRIDE_FACTOR;
      for (c = stride; c < n ; c++)
	{
	  found = 0;
	  d = c - stride;
	  while ((d >= 0) && !found)
	    {
	      long p1 = parents[index[d+stride]];
	      long p2 = parents[index[d]];

	      if ((p1 < p2) || ((p1 == p2) && (index[d+stride] > index[d])))
		{
		  long tmp = index[d];
		  index[d] = index[d+stride];
		  index[d+stride] = tmp;
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

long
bsearch_parsort_table (thread_array parsort,
		       thread_array parents,
		       long looking_for,
		       long n)
{
  long p;  
  long high = n;
  long low = 0;
  
  while ((high - low) > 1) {
    p = (high + low) / 2;
    if (looking_for <= parents[parsort[p-1]])
      high = p;
    else
      low = p;
  }
  if (looking_for == parents[parsort[high-1]])
    return (high-1);
  else
    return -1;
}


long
bsearch_mid_table (header_p headers,
		   thread_array index, /* sorted index */
		   char * mid,	/* message_id we're looking for */
		   long n)
{
  long p;  
  long high = n;
  long low = 0;
  
  while ((high - low) > 1) {
    p = (high + low) / 2;
    if (strcmp (mid, headers[index[p-1]].message_id) <= 0)
      high = p;
    else
      low = p;
  }
  if (strcmp (mid, headers[index[high-1]].message_id) == 0)
    return (high-1);
  else
    return -1;
}


long
thread_sort (long root_index, long start, long end, int depth)
{
  long j;
  long num_children = 0;
  long child_start;

  if (start == end)
    return end;
  else {
    /* find the children of this node, pack them in the bottom */

    /* this will find the first child in the sorted table */
    child_start = bsearch_parsort_table (g_parsort,
					 g_parent,
					 root_index,
					 g_length);

    /* for each child, find its index and push on the stack in g_index */
    if (child_start != -1) {
      while ((child_start < g_length) &&
	     (g_parent[g_parsort[child_start]] == root_index)) {
	g_index[end-num_children-1] = g_parsort[child_start];
	child_start++;
	num_children++;
      }
    }
    /* no children found */
    else
      return (start);

    /* apply sort-me to each of the children */

    if (num_children == 0)
      return (start);

    for (j = num_children ; j > 0 ; j--) {
      g_index[start] = g_index[end-j];
      g_headers[g_index[start]].thread_depth = (char) depth;
      start = thread_sort (g_index[start], start+1, end-(j-1), depth+1);
    }
    return (start);
  }
}


/* setup for threading algorithm:
 * 1. allocate an extra thread_array for holding a sorted message-id table
 * 2. using mid_table, allocate and create another table, a map from
 *    article_index->parent_index  
 * 3. sort this table by parent_index (must be a stable sort)
 * 
 * the recursive thread_sort will use these tables to do a stable sort
 * by threads...
 */

void
sort_by_threads (HANDLE header_handle, HANDLE thread_handle, long length)
{
  long i;
  header_p headers;
  HANDLE mid_handle, parent_handle;
  thread_array thread_index,mid_table,parent_table;
  
  headers = lock_headers (header_handle, thread_handle);
  thread_index  = *((thread_array_p) ((char_p) headers - sizeof (char_p)));  

  if (!thread_index)
    return;

  mid_handle = GlobalAlloc (GMEM_MOVEABLE, (long) (sizeof(long) * length));
  if (!mid_handle)
    return;

  parent_handle = GlobalAlloc (GMEM_MOVEABLE, (long) (sizeof (long) * length));
  if (!parent_handle) {
    GlobalFree (mid_handle);
    return;
  }
    
  mid_table = (thread_array) GlobalLock (mid_handle);
  parent_table = (thread_array) GlobalLock (parent_handle);
  
  /* create the message_id table */
  for (i=0 ; i < length; i++) mid_table[i]=i;
  shell_sort_index_array (headers, mid_table, length, compare_message_id);

  /* create the parent table */
  
  for (i=0 ; i < length; i++) {
    long p = bsearch_mid_table (headers,mid_table,headers[i].references,length);
    parent_table[i] = (p == -1) ? -1 : mid_table[p];
  }

  /* clear it so we can debug */
  for (i=0 ; i< length; i++)
    thread_index[i]=0;

  /* re-use the mid_table as a sorted parent table */
  for (i=0 ; i< length; i++)
    mid_table[i]=i;

  shell_sort_parent_array (mid_table, parent_table, length);

  g_headers = headers;
  g_index = thread_index;
  g_parent = parent_table;
  g_parsort = mid_table;
  g_length = length;

  thread_sort (-1, 0, length,0);

  GlobalFree (parent_handle);
  GlobalFree (mid_handle);
}

/*
--------------------------------------------------------------------------

Thread sorting algorithm.

Here's an example, already threaded, showing the relationship between
parent, child, and original index.



0   5
1      2
2         3
3      0   
4   1   
5      4        

On the display, it may look like this:

5   Why my computer is better than yours...
2     Re: Why my computer is better than yours...
3       Why my _car_ is better than your computer (was Re: ...)
0     Re: Why my computer is better than yous...
1   What's the latest on the Amiga 6000???
4     What planet are you on ? (was Re: What's the latest...)

This shows that articles #2 and #0 are in response to article #5 (yes this can
and does happen), article #4 is in response to #1, etc...

This is the parent table.  It answers the question: "what is the index
of my parent".  '*' means root, or 'no' index - either there is no    
parent of this article, or we don't have it.  In the code, '*' == -1.

  |--|
0 |5 |
  |--|
1 |* | 
  |--|
2 |5 |
  |--|
3 |2 |
  |--|
4 |1 |
  |--|
5 |* |
  |--|

This table was computed by
1) sorting by Message-ID (by creating 'mid_table') with a shell sort.
2) use 'mid_table' to find each article's parent (using a binary search),
   thereby creating 'parent_table'.
3) mid_table's not needed any longer, so we now re-use it as a sorted
   index into parent_table.  More on this later.

What we want from thread_sort is for the empty thread_index to hold
an array with these articles indices in the correct order:
               	       	 	      
|--|
|5 |
|--|
|2 |
|--|
|3 |
|--|
|0 |
|--|
|1 |
|--|
|4 |
|--|

---------------------------------------------------------------------------
Now, for the actual algorithm.  The work is performed by the recursive
function thread_sort().

thread_sort (root_index, start, end, depth) { ... }
(depth is used to keep track of the depth of the recursion)

1) Start with an empty index table.  start = 0 (the beginning of the
   table), and end = length (the length of the table).

2) Find all the children of the current root, (which is '*' to start
   with), and pack them (in order) into the bottom of the table.
   If there are no children, return 'start'.
   If start == end, return 'start'.

3) Now, we will recurse [call thread_root()] for each of these
   children, using the empty portion of thread_index to work with.
   We do this by:
     
   3a) Move child #1 into the top slot.
   3b) calling thread_sort, with
       root_index = child #1
       start = start of the empty portion
       end = end of the empty portion
       depth = depth+1
   3c) After thread_sort() does its magic, it will return a 
       new value for 'start', indicating where the 'work area'
       can start.  thread_sort() may have filled in an arbitrary
       number of slots in this call, but will never overstep the
       free space.  Don't worry, it all works out.  8^)

   3d) Go to child #2, repeat 3(a-c), #3, #4, etc...

4) return 'start' (the start of the empty space).
			      
Here's a trace of the algorithm using our example articles.
---------------------------------------------------------------------------
The number above the stack of boxes indicates the parent that
we're finding the children of.

   *		              
  |--|  |--|                                                
0 |  |  |5 |   5                                            
  |--|  |--|  |--|  |--|                          |--|  |--|
1 |  |  |  |  |  |  |2 |   2                      |2 |  |2 |
  |--|  |--|  |--|  |--|  |--|  |--|        |--|  |--|  |--|
2 |  |  |  |  |  |  |  |  |  |  |3 |   3    |3 |  |3 |  |3 |    ==>
  |--|  |--|  |--|  |--|  |--|  |--|  |--|  |--|  |--|  |--|
3 |  |  |  |  |2 |  |  |  |3 |  |  |  |  |  |  |  |  |  |  |
  |--|  |--|  |--|  |--|  |--|  |--|  |--|  |--|  |--|  |--|
4 |5 |  |  |  |0 |  |0 |                                |0 |
  |--|  |--|  |--|  |--|                                |--|
5 |1 |  |1 |                                                
  |--|  |--|                                                
                        	                      	    

continued...
               	       	 	      
                    |--|                    |--|
0                   |5 |                    |5 |
              |--|  |--|                    |--|
1             |2 |  |2 |                    |2 |
              |--|  |--|                    |--|
2             |3 |  |3 |                    |3 |
  |--|        |--|  |--|                    |--|
3 |0 |   0    |0 |  |0 |                    |0 |
  |--|  |--|  |--|  |--|  |--|              |--|
4 |  |  |  |  |  |  |  |  |1 |   1          |1 |
  |--|  |--|  |--|  |--|  |--|  |--|        |--|
5                   |1 |  |  |  |4 |   4    |4 |
                    |--|  |--|  |--|  |--|  |--|


---------------------------------------------------------------------------
Now that you understand the algorithm 8^), we go back to parsort_table.  
At the start of each call to thread_sort(), we need to find all the
children of root_index.  We can do this quickly by using a sorted
version of parent_table, called parsort_table.  It contains a
convenient ordered list of children for us:

  x -+
  x  | all the children of 'x' 
  x  | 			      
  x -+                        
  y -+  	      
  y -+ all the children of 'y'
  z -+
  z  |
  z  | all the children of 'z' 
  z  | 
  z -+

A single call (order logn) to bsearch_parsort_table puts us at the
correct index for finding all the children we are looking for.

parsort_table
  |--|
0 |1 |
  |--|
1 |5 | 
  |--|
2 |4 |
  |--|
3 |3 |
  |--|
4 |0 |
  |--|
5 |2 |
  |--|

---------------------------------------------------------------------------

*/@


1.8
log
@Documented the thread sort algorithm.
@
text
@d4 1
a4 1
 * $Id: headarry.c 1.7 1993/12/08 01:27:21 rushing Exp rushing $
d6 3
d36 1
d39 1
@


1.7
log
@new version box and cr lf consistency
@
text
@d4 5
a8 2
 * $Id: headarry.c 1.6 1993/06/25 20:44:45 dumoulin Exp rushing $
 * $Log: headarry.c $ *
a35 1

a390 1
// long i;
a397 8
//
//    for (i = 0; i < g_length; i++) {
//      if (g_parent[g_length-i-1] == root_index) {
//	g_index[end-num_children-1] = g_length-i-1;
//	num_children++;
//      }
//    }
//#endif
d400 4
a403 1
    child_start = bsearch_parsort_table (g_parsort, g_parent, root_index, g_length);
d503 184
@


1.6
log
@Change dates from strings to Posix standard date formats
@
text
@d1 1
d4 6
a10 4
 * routines for handling the array of header information
 * 
 * $Id: headarry.c 1.5 1993/06/10 18:29:34 rushing Exp dumoulin $
 * $Log: headarry.c $
@


1.5
log
@added set_index_to_identity to prepare for writing article ranges
@
text
@d6 1
a6 1
 * $Id: headarry.c 1.4 1993/06/08 19:41:56 rushing Exp rushing $
d8 3
d148 1
a148 1
    headers[i].date[0] = (char) 0;
d233 9
a241 1
  return stricmp (headers[elem1].date , headers[elem2].date);
d385 2
a386 2

  long i,j;
d394 8
a401 8
#if 0
    for (i = 0; i < g_length; i++) {
      if (g_parent[g_length-i-1] == root_index) {
	g_index[end-num_children-1] = g_length-i-1;
	num_children++;
      }
    }
#endif
@


1.4
log
@changes to support Sort... menu
@
text
@d6 1
a6 1
 * $Id: headarry.c 1.3 1993/06/05 03:18:25 rushing Exp SOMEONE $
d8 3
d94 21
@


1.3
log
@primitive functional threading.
@
text
@d6 1
a6 1
 * $Id: headarry.c 1.2 1993/06/04 16:46:44 rushing Exp rushing $
d8 3
d46 1
a46 1
thread_array g_index, g_parent;
d119 1
d164 15
d192 8
a199 1
  return strcmp (headers[elem1].subject , headers[elem2].subject);
a201 1
  
a203 1
	      thread_array index,
d206 1
a206 1
  return strcmp (headers[elem1].date , headers[elem2].date);
d257 66
d353 1
d359 1
d366 18
a383 1
    
a444 1
/*    sprintf (headers[i].from, "%3.3ld m%3.3ld p%3.3ld", i,mid_table[i],parent_table[i]); */
d447 10
d460 1
a461 4

  /* clear it so we can debug */
  for (i=0 ; i< length; i++)
    g_index[i]=0;
@


1.2
log
@stable version of shellsort
@
text
@d6 1
a6 1
 * $Id: headarry.c 1.1 1993/06/01 18:15:12 rushing Exp rushing $
d8 3
d41 4
d233 67
d303 1
d305 3
a307 2
  thread_array thread_index;

d311 2
a312 3
  if (thread_index) {

    shell_sort_index_array (headers, thread_index, length, compare_subject); 
d314 8
d323 7
a329 1
}
d331 7
d339 8
d348 1
d350 3
a352 2


@


1.1
log
@Initial revision
@
text
@d2 1
d6 4
a9 2
 * $Id:$
 * $Log:$
d11 1
d38 1
d137 44
d182 43
a228 1
  long i;
d236 2
a237 4
    
    /* let's try reversing the array */
    for (i=0; i <length ; i++)
      thread_index[i] = (length - (i+1));
d241 7
@
