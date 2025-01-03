/*
 * bni00.c - auto-size-detecting bni??.c file.
 *
 * Written in 1995 by Colin Plumb.
 *
 * $Id: bni00.c,v 1.1 1997/04/30 08:36:09 mhw Exp $
 */

#include "bnsize00.h"

#if BNSIZE64

/* Include all of the C source file by reference */
#include "bni64.c"

#elif BNSIZE32

/* Include all of the C source file by reference */
#include "bni32.c"

#else /* BNSIZE16 */

/* Include all of the C source file by reference */
#include "bni16.c"

#endif
