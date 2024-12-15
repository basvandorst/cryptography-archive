/*
 * pgpUsuals.h - Typedefs and #defines used widely.
 *
 * $Id: pgpUsuals.h,v 1.1 1998/12/15 18:40:29 wjb Exp $
 */
#ifndef Included_pgpUsuals_h
#define Included_pgpUsuals_h

#include "pgpBase.h"

#include "pgpOpaqueStructs.h"

PGP_BEGIN_C_DECLARATIONS


#if PGP_HAVE64
typedef PGPUInt64	bnword64;
#endif


/* The PGP Library Cipher IV Length */
#define IVLEN	10

PGP_END_C_DECLARATIONS

#endif /* Included_pgpUsuals_h */
