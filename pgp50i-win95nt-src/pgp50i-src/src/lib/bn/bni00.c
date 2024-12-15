/*
* bni00.c - auto-size-detecting bni??.c file.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written in 1995 by Colin Plumb.
*
* $Id: bni00.c,v 1.2.4.1 1997/06/07 09:49:29 mhw Exp $
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
