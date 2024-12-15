/*
 * pgpRadix64.h -- The header for Radix64 encoding,
 *		   which is used by ASCII Armor.
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpRadix64.h,v 1.4 1997/05/13 01:19:01 mhw Exp $
 */

#ifndef Included_pgpRadix64_h
#define Included_pgpRadix64_h

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS

extern char const armorTable[65];

PGP_END_C_DECLARATIONS

#endif /* Included_pgpRadix64_h */