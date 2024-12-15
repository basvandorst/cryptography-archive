/*
 * lbn00.c - auto-size-detecting lbn??.c file.
 *
 * Written in 1995 by Colin Plumb.
 *
 * $Id: lbn00.c,v 1.1 1997/04/30 08:36:24 mhw Exp $
 */

#include "bnsize00.h"

#if BNSIZE64

/* Include all of the C source file by reference */
#include "lbn64.c"

#elif BNSIZE32

/* Include all of the C source file by reference */
#include "lbn32.c"

#else /* BNSIZE16 */

/* Include all of the C source file by reference */
#include "lbn16.c"

#endif
