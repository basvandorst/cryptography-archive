/*
* lbn00.c - auto-size-detecting lbn??.c file.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written in 1995 by Colin Plumb.
*
* $Id: lbn00.c,v 1.5.4.1 1997/06/07 09:49:43 mhw Exp $
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
