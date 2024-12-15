/*
 * radix.c -- Define the armor Table for 6-bit integer to character
 * conversion.
 *
 * $Id: pgpRadix64.c,v 1.2 1997/05/02 22:45:25 lloyd Exp $
 */

#include "pgpConfig.h"

#include "pgpRadix64.h"

/* This is are table of 6-bit value -> character */
char const armorTable[65] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
