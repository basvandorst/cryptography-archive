/*
 * bntest00.c - auto-size-detecting bntest??.c file.
 *
 * Written in 1995 by Colin Plumb.  I'm not going to embarass myself
 * by claiming copyright on something this trivial.
 */

#include "bnsize00.h"

#if BNSIZE64

#include "bntest64.c"

#elif BNSIZE32

#include "bntest32.c"

#else /* BNSIZE16 */

#include "bntest16.c"

#endif
