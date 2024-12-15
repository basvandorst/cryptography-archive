/*
 * bn00.c - auto-size-detecting bn??.c file.
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written in 1995 by Colin Plumb.
 *
 * $Id: bn00.c,v 1.5.4.1 1997/06/07 09:49:26 mhw Exp $
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
