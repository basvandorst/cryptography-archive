/*
* pgpRnd.h - system-specific gathering of high-resolution timers for RNG.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* This is a PRIVATE header file, for use only within the PGP Library.
* You should not be using these functions in an application.
*
* The interface is system-independent, but the implementation should be
* highly system-dependent, to get at as much state as possible.
*
* $Id: pgpRnd.h,v 1.2.2.1 1997/06/07 09:51:33 mhw Exp $
*/

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpRandomContext;

word32 ranGetEntropy(struct PgpRandomContext const *rc);

#ifdef __cplusplus
}
#endif
