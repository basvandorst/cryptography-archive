/*
 * bn00.c - auto-size-detecting bn??.c file.
 *
 * Written in 1995 by Colin Plumb.  I'm not going to embarass myself
 * by claiming copyright on something this trivial.
 */

#include "bnsize00.h"

#if BNSIZE64

/* Include all of the C source file by reference */
#include "bn64.c"
#include "bninit64.c"

#elif BNSIZE32

/* Include all of the C source file by reference */
#include "bn32.c"
#include "bninit32.c"

#else /* BNSIZE16 */

/* Include all of the C source file by reference */
#include "bn16.c"
#include "bninit16.c"

#endif
