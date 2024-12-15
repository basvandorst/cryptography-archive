/**************************************************************************\
*                                                                          *
*                              QSORT.H (tm)                                *
*                       Implement a QuickSort Function                     *
*                                                                          *
*             SOURCE CODE REVISION  DEV-940821-14:49 EST                   *
*                                                                          *
*    DEVEL:[X]   ALPHA DEVEL: [ ]   BETA TESTING:[ ]    PRODUCTION:[ ]     *
*                                                                          *
*              WARNING!   UNTESTED CODE!  USE AT YOUR OWN RISK!            *
*                                                                          *
*==========================================================================*
*                                                                          *
*               Copyright (C) 1994 by Ray (Arsen) Arachelian,              *
*          All Rights Reserved, General Technical Data Restricted.         *
*                                                                          *
*                THIS IS PUBLISHED, PROPRIETARY SOURCE CODE                *
*                                                                          *
*==========================================================================*
*                                                                          *
*    WARNING: This source code is the sole property of the author and is   *
*    to be treated as such.  The use of this source code is subject to     *
*    certain limitations stated in the documentation and in the main files *
*    of this software package.  Using this source code or a machine        *
*    executable version of this source code is subject to these conditions.*
*                                                                          *
\**************************************************************************/

// Stack overflow prevention.  Need to experiment with this value until the
// machine doesn't do a stack overflow.  This is machine/OS dependant.
#define STACKTHRESHOLD 500


static void qs(int start,                        // indicates a new start
	       void *item,                       // pointer to item array to sort
	       long left, long right,            // boundaries of cur. partition

	       void *(getitem)(void *, long),        // get item
	       int (cmpitem) (void *, long, long),   //comparison function
	       int (cmpitem2)(void *, long, void *), //cmp itemidx w/item
	       void(swapitem)(void *, long, long))   //swap function

{
static   long recursive;           // check for stack overflow
static   int  stackoverflow;       // check for stack overflow

register long i,j;
void *middle;
int swapflag;

 // Initialize
 if (start) {recursive=0L; stackoverflow=0;}

 recursive++; // signal that we entered another stack level


// printf("Entering qs\n");

 /*========================================================================

  Since QuickSort is recursive, we need to check for stack overflow errors.
  If your OS/Compiler has a better way of checking for soon to be occur
  stack overflows, use it instead of this.  [Basically some function that
  returns how much free space is on the stack is all you need.  If that
  space drops under, say 128 bytes, you're in trouble.]

  If you have some source code to a NON-RECURSIVE version of QuickSort,
  and are willing to share it with me, please contact me and I'll replace
  this function with it.

  ========================================================================*/

 if (recursive>STACKTHRESHOLD) stackoverflow=1;  // Oops! Stack overflow!
 if (stackoverflow)      // If we overflowed the stack, unroll all the
   if (recursive)        // entries on the stack until the top one.
      { recursive--; return;}
   else                 // Once we unrolled the stack, switch to a bubble
   {                    // sort or some other non-recursive sort so that
			// this function will never fail to sort the items.
			// and in the worst case, it will just crawl.

bsort:
     puts("Switching to slow-sort");
     swapflag=0;
     do
      for (i=left; i<right; i++)
       if (cmpitem(item, i,i+1)>0) {swapitem(item, i,i+1); swapflag=1;}
     while(swapflag);
     return;
   }

 /*========= Standard QuickSort Stuff:====================================*/

 i=left; j=right; middle=getitem(item, (left+right)/2 );
 /* Index to left, right, middle items */

 do
  {
    while (cmpitem2(item, i,middle)<0  &&  i<right) i++;
    while (cmpitem2(item, j,middle)>0  &&  j>left)  j--;
    if (i<=j)
     {
      swapitem(item,i,j);
      i++;
      j--;
     }
  }
 while (i<=j);

 if (left<j    )  qs(0,item,  left,j      ,getitem,cmpitem,cmpitem2,swapitem);
 if (i   <right)  qs(0,item,  i   ,right  ,getitem,cmpitem,cmpitem2,swapitem);

 recursive--;

 if (recursive==0 && stackoverflow) goto bsort;

 free(middle);

}



// Main entry point.
void quicksort(void *item,                          // pointer to base item
	       long count,                          // number of items


	       void *(getitem)(void *, long),        // get item
	       int (cmpitem) (void *, long, long),   //comparison function
	       int (cmpitem2)(void *, long, void *), //cmp itemidx w/item
	       void(swapitem)(void *, long, long))   //swap function

{
 qs(1,item,0,count,getitem,cmpitem,cmpitem2,swapitem);
}

