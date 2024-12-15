/*
 * pgpGlobals.c -- A file containing global, constant values.
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by: Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpGlobals.c,v 1.4.2.3 1997/06/07 09:50:07 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpUsuals.h"

char const pgpLibVersionString[] =
#ifdef PGP_FREEWARE
    "PGPfreeware 5.0i for non-commercial use";
#else
    "PGP for Personal Privacy 5.0i";
#endif
