/*
 * pgpRIPEMD160.h -- RIPEMD-160 message digest
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpRIPEMD160.h,v 1.2 1997/06/13 01:53:00 lloyd Exp $
 */

#ifndef Included_pgpRIPEMD160_h
#define Included_pgpRIPEMD160_h

#include "pgpHashPriv.h"

PGP_BEGIN_C_DECLARATIONS

extern PGPHashVTBL const HashRIPEMD160;

PGP_END_C_DECLARATIONS

#endif /* !Included_pgpRIPEMD160_h */
