/*
 * pgpRadix64.c -- Define the armor Table for 6-bit integer to character
 * conversion.
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpRadix64.c,v 1.1.2.1 1997/06/07 09:50:58 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpRadix64.h"

/* This is are table of 6-bit value -> character */
char const armorTable[65] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
