/*
 * pgpMD2.h -- The MD-5 Message Digest.
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpMD2.h,v 1.1 1998/03/14 00:23:24 hal Exp $
 */

#ifndef Included_pgpMD2_h
#define Included_pgpMD2_h

#include "pgpHashPriv.h"

PGP_BEGIN_C_DECLARATIONS

extern PGPHashVTBL const HashMD2;

PGP_END_C_DECLARATIONS

#endif /* !Included_pgpMD2_h */