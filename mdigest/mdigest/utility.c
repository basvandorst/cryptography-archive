
/* -------------------------------------------------------------------- */
/*                                                                      */
/*  Secure Asyncronous Communications Subsystem (SACS)                  */
/*  (C) 1994 Matthew Gream <M.Gream@uts.edu.au>                         */
/*                                                                      */
/*  You have been given a limited license to use and distribute this    */
/*  library. You should have received this license with the original    */
/*  release package of this library, named "license.txt" and entitled   */
/*  "SACS SOFTWARE LICENSE". In the event that you have not read or     */
/*  understood this license, regardless of reason, you may do nothing   */
/*  with this library other than read it for self-awareness.            */
/*                                                                      */
/* -------------------------------------------------------------------- */

/* $Id: memory.c%v 0.81 1994/02/12 10:08:43 matt Exp $
 */

/*
 *  File:         utility.c
 *  Description:  misc utility stuff
 */

#include <mdigest.h>


/* -------------------------------------------------------------------- */
/* function    : mem_get                                                */
/* description : allocate and return 'req_size' bytes of memory, clear  */
/*               if MEMOPT_CLEAR flag is present. Failures are hard.    */
/* -------------------------------------------------------------------- */
GLOBAL opaque
mem_get (req_size, req_options)
    word            req_size;           /* size of memory requested     */
    bitstring       req_options;        /* options for allocated mem    */
{
    opaque          seg_addr;

    DEBUG_FUNCTION_ENTER("mem_get");

    /* ---------------------------------------------------------------- */
    /* code should never request zero size segments -- deal with it by  */
    /* returning NULL pointer.                                          */
    /* ---------------------------------------------------------------- */

    if (req_size == 0)
      return ((opaque)NULL);

    /* ---------------------------------------------------------------- */
    /* if allocation request fails, call error handler and indicate a   */
    /* fatal error with appropriate error code                          */
    /* ---------------------------------------------------------------- */

    seg_addr = (opaque)malloc (req_size);
    if (seg_addr == NULL)
      {
        /*sacs_hard_error (err_no_memory);*/
        fprintf (stderr, "malloc failed.\n");
        exit (1);
        /* not reached */
      }

    /* ---------------------------------------------------------------- */
    /* clear segment if we were specifically asked to do so             */
    /* ---------------------------------------------------------------- */

    if (bit_chk (req_options, MEMOPT_CLEAR) == TRUE)
      memset (seg_addr, '\0', req_size);

    return (seg_addr);
}


/* -------------------------------------------------------------------- */
/* function    : mem_free                                               */
/* description : release memory segment previously allocated            */
/* -------------------------------------------------------------------- */
GLOBAL void
mem_free (seg_addr)
    opaque          seg_addr;           /* address of mem segment       */
{
    DEBUG_FUNCTION_ENTER("mem_free");

    if (seg_addr != NULL)
      free (seg_addr);
}

/* -------------------------------------------------------------------- */

