/*
 * pgpMD5.h -- The MD-5 Message Digest.
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpMD5.h,v 1.1 1998/08/18 22:59:29 dgal Exp $
 */

#ifndef Included_pgpMD5_h
#define Included_pgpMD5_h

#include "pgpHashPriv.h"

PGP_BEGIN_C_DECLARATIONS

extern PGPHashVTBL const HashMD5;


void	pgpMD5Transform(PGPUInt32 *block, PGPUInt32 const *key);

PGP_END_C_DECLARATIONS

#endif /* !Included_pgpMD5_h */