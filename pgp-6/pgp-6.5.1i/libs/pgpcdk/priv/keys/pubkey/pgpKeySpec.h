/*
 * $Id: pgpKeySpec.h,v 1.9 1997/06/25 19:40:57 lloyd Exp $
 */

#ifndef Included_pgpKeySpec_h
#define PGP_KEYSPEC_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

PGPKeySpec  *pgpKeySpecCreate(PGPEnv const *env);
PGPKeySpec  *pgpKeySpecCopy(PGPKeySpec const *ks);
void  pgpKeySpecDestroy(PGPKeySpec *ks);

PgpVersion  pgpKeySpecVersion(PGPKeySpec const *ks);

PGPUInt32  pgpKeySpecCreation(PGPKeySpec const *ks);
PGPUInt16  pgpKeySpecValidity(PGPKeySpec const *ks);

int  pgpKeySpecSetVersion(PGPKeySpec *ks, PgpVersion ver);
int  pgpKeySpecSetCreation(PGPKeySpec *ks, PGPUInt32 creation);
int  pgpKeySpecSetValidity(PGPKeySpec *ks, PGPUInt16 validity);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpKeySpec_h */
