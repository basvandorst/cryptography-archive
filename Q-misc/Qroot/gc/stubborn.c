
/* 
 * Copyright 1988, 1989 Hans-J. Boehm, Alan J. Demers
 * Copyright (c) 1991-1993 by Xerox Corporation.  All rights reserved.
 *
 * THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
 * OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 *
 * Permission is hereby granted to copy this garbage collector for any purpose,
 * provided the above notices are retained on all copies.
 */


#include "gc_private.h"

# ifdef STUBBORN_ALLOC
/* Stubborn object (hard to change, nearly immutable) allocation. */


# ifdef ALL_INTERIOR_POINTERS
#   define SMALL_OBJ(bytes) ((bytes) < WORDS_TO_BYTES(MAXOBJSZ))
# else
#   define SMALL_OBJ(bytes) ((bytes) <= WORDS_TO_BYTES(MAXOBJSZ))
# endif

/* Data structure representing immutable objects that 	*/
/* are still being initialized.				*/
/* This is a bit baroque in order to avoid acquiring	*/
/* the lock twice for a typical allocation.		*/

extern_ptr_t * GC_changing_list_start;

# ifdef THREADS
  VOLATILE extern_ptr_t * VOLATILE GC_changing_list_current;
# else
  extern_ptr_t * GC_changing_list_current;
# endif
	/* Points at last added element.  Also (ab)used for		*/
	/* synchronization.  Updates and reads are assumed atomic.	*/

extern_ptr_t * GC_changing_list_limit;
	/* Points at the last word of the buffer, which is always 0	*/
	/* All entries in (GC_changing_list_current,			*/
	/* GC_changing_list_limit] are 0				*/


void GC_stubborn_init()
{
#   define INIT_SIZE 10

    GC_changing_list_start = (extern_ptr_t *)
    			GC_generic_malloc_inner(
    				(word)(INIT_SIZE * sizeof(extern_ptr_t)),
    				PTRFREE);
    bzero((char *)GC_changing_list_start,
    	  (int)(INIT_SIZE * sizeof(extern_ptr_t)));
    if (GC_changing_list_start == 0) {
        GC_err_printf0("Insufficient space to start up\n");
        ABORT("GC_stubborn_init: put of space");
    }
    GC_changing_list_current = GC_changing_list_start;
    GC_changing_list_limit = GC_changing_list_start + INIT_SIZE - 1;
    * GC_changing_list_limit = 0;
}

/* Compact and possibly grow GC_uninit_list.  The old copy is		*/
/* left alone.	Lock must be held.					*/
/* When called GC_changing_list_current == GC_changing_list_limit	*/
/* which is one past the current element.				*/
/* When we finish GC_changing_list_current again points one past last	*/
/* element.								*/
/* Invariant while this is running: GC_changing_list_current    	*/
/* points at a word containing 0.					*/
/* Returns FALSE on failure.						*/
bool GC_compact_changing_list()
{
    register extern_ptr_t *p, *q;
    register int count = 0;
    word old_size = GC_changing_list_limit-GC_changing_list_start+1;
    register word new_size = old_size;
    extern_ptr_t * new_list;
    
    for (p = GC_changing_list_start; p < GC_changing_list_limit; p++) {
        if (*p != 0) count++;
    }
    if (2 * count > old_size) new_size = 2 * count;
    new_list = (extern_ptr_t *)
    		GC_generic_malloc_inner(
    				new_size * sizeof(extern_ptr_t), PTRFREE);
    		/* PTRFREE is a lie.  But we don't want the collector to  */
    		/* consider these.  We do want the list itself to be  	  */
    		/* collectable.						  */
    if (new_list == 0) return(FALSE);
    bzero((char *)new_list, (int)(new_size * sizeof(extern_ptr_t)));
    q = new_list;
    for (p = GC_changing_list_start; p < GC_changing_list_limit; p++) {
        if (*p != 0) *q++ = *p;
    }
    GC_changing_list_start = new_list;
    GC_changing_list_limit = new_list + new_size - 1;
    GC_changing_list_current = q;
    return(TRUE);
}

/* Add p to changing list.  Clear p on failure.	*/
# define ADD_CHANGING(p) \
	{	\
	    register struct hblk * h = HBLKPTR(p);	\
	    register word index = PHT_HASH(h);	\
	    \
	    set_pht_entry_from_index(GC_changed_pages, index);	\
	}	\
	if (*GC_changing_list_current != 0 \
	    && ++GC_changing_list_current == GC_changing_list_limit) { \
	    if (!GC_compact_changing_list()) (p) = 0; \
	} \
	*GC_changing_list_current = p;

void GC_change_stubborn(p)
extern_ptr_t p;
{
    DCL_LOCK_STATE;
    
    DISABLE_SIGNALS();
    LOCK();
    ADD_CHANGING(p);
    UNLOCK();
    ENABLE_SIGNALS();
}

void GC_end_stubborn_change(p)
extern_ptr_t p;
{
#   ifdef THREADS
      register VOLATILE extern_ptr_t * my_current = GC_changing_list_current;
#   else
      register extern_ptr_t * my_current = GC_changing_list_current;
#   endif
    register bool tried_quick;
    DCL_LOCK_STATE;
    
    if (*my_current == p) {
        /* Hopefully the normal case.					*/
        /* Compaction could not have been running when we started.	*/
        *my_current = 0;
#	ifdef THREADS
          if (my_current == GC_changing_list_current) {
            /* Compaction can't have run in the interim. 	*/
            /* We got away with the quick and dirty approach.   */
            return;
          }
          tried_quick = TRUE;
#	else
	  return;
#	endif
    } else {
        tried_quick = FALSE;
    }
    DISABLE_SIGNALS();
    LOCK();
    my_current = GC_changing_list_current;
    for (; my_current >= GC_changing_list_start; my_current--) {
        if (*my_current == p) {
            *my_current = 0;
            UNLOCK();
            ENABLE_SIGNALS();
            return;
        }
    }
    if (!tried_quick) {
        GC_err_printf1("Bad arg to GC_end_stubborn_change: 0x%lx\n",
        	       (unsigned long)p);
        ABORT("Bad arg to GC_end_stubborn_change");
    }
    UNLOCK();
    ENABLE_SIGNALS();
}

/* Allocate lb bytes of composite (pointerful) data	*/
/* No pointer fields may be changed after a call to	*/
/* GC_end_stubborn_change(p) where p is the value	*/
/* returned by GC_malloc_stubborn.			*/
# ifdef __STDC__
    extern_ptr_t GC_malloc_stubborn(size_t lb)
# else
    extern_ptr_t GC_malloc_stubborn(lb)
    size_t lb;
# endif
{
register ptr_t op;
register ptr_t *opp;
register word lw;
extern_ptr_t result;
DCL_LOCK_STATE;

    if( SMALL_OBJ(lb) ) {
#       ifdef MERGE_SIZES
	  lw = GC_size_map[lb];
#	else
	  lw = ROUNDED_UP_WORDS(lb);
#       endif
	opp = &(GC_sobjfreelist[lw]);
	FASTLOCK();
        if( !FASTLOCK_SUCCEEDED() || (op = *opp) == 0 ) {
            FASTUNLOCK();
            result = GC_generic_malloc((word)lb, STUBBORN);
            goto record;
        }
        *opp = obj_link(op);
        obj_link(op) = 0;
        GC_words_allocd += lw;
        result = (extern_ptr_t) op;
        ADD_CHANGING(result);
        FASTUNLOCK();
        return(result);
   } else {
       result = (extern_ptr_t)
          	GC_generic_malloc((word)lb, STUBBORN);
   }
record:
   DISABLE_SIGNALS();
   LOCK();
   ADD_CHANGING(result);
   UNLOCK();
   ENABLE_SIGNALS();
   return(result);
}


/* Functions analogous to GC_read_dirty and GC_page_was_dirty.	*/
/* Report pages on which stubborn objects were changed.		*/
void GC_read_changed()
{
    register extern_ptr_t * p = GC_changing_list_start;
    register extern_ptr_t q;
    register struct hblk * h;
    register word index;
    
    if (p == 0) /* initializing */ return;
    bcopy((char *)GC_changed_pages, (char *)GC_prev_changed_pages,
          (int)(sizeof GC_changed_pages));
    bzero((char *)GC_changed_pages, (int)(sizeof GC_changed_pages));
    for (; p <= GC_changing_list_current; p++) {
        if ((q = *p) != 0) {
            h = HBLKPTR(q);
            index = PHT_HASH(h);
            set_pht_entry_from_index(GC_changed_pages, index);
        }
    }
}

bool GC_page_was_changed(h)
struct hblk * h;
{
    register word index = PHT_HASH(h);
    
    return(get_pht_entry_from_index(GC_prev_changed_pages, index));
}

/* Remove unreachable entries from changed list. Should only be	*/
/* called with mark bits consistent and lock held.		*/
void GC_clean_changing_list()
{
    register extern_ptr_t * p = GC_changing_list_start;
    register extern_ptr_t q;
    register ptr_t r;
    register unsigned long count = 0;
    register unsigned long dropped_count = 0;
    
    if (p == 0) /* initializing */ return;
    for (; p <= GC_changing_list_current; p++) {
        if ((q = *p) != 0) {
            count++;
            r = (ptr_t)GC_base(q);
            if (r == 0 || !GC_is_marked(r)) {
                *p = 0;
                dropped_count++;
	    }
        }
    }
#   ifdef PRINTSTATS
      if (count > 0) {
        GC_printf2("%lu entries in changing list: reclaimed %lu\n",
                  (unsigned long)count, (unsigned long)dropped_count);
      }
#   endif
}

#else /* !STUBBORN_ALLOC */

# ifdef __STDC__
    extern_ptr_t GC_malloc_stubborn(size_t lb)
# else
    extern_ptr_t GC_malloc_stubborn(lb)
    size_t lb;
# endif
{
    return(GC_malloc(lb));
}

/*ARGSUSED*/
void GC_end_stubborn_change(p)
extern_ptr_t p;
{
}

/*ARGSUSED*/
void GC_change_stubborn(p)
extern_ptr_t p;
{
}


#endif
