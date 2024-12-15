/*
 * pgpSHADouble.c - Double width version of the NIST SHA-1 hash
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpSHADouble.h,v 1.1 1997/07/02 23:18:25 hal Exp $
 */

#ifndef Included_pgpSHADouble_h
#define Included_pgpSHADouble_h

#include "pgpHashPriv.h"


PGP_BEGIN_C_DECLARATIONS

extern PGPHashVTBL const HashSHADouble;

PGP_END_C_DECLARATIONS

#endif /* !Included_pgpSHADouble_h */
