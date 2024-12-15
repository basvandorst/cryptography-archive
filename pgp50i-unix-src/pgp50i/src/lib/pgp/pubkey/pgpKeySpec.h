/*
 * pgpKeySpec.h
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpKeySpec.h,v 1.3.2.1 1997/06/07 09:51:27 mhw Exp $
 */

#ifndef PGPKEYSPEC_H
#define PGP_KEYSPEC_h

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpKeySpec;
#ifndef TYPE_PGPKEYSPEC
#define TYPE_PGPKEYSPEC 1
typedef struct PgpKeySpec PgpKeySpec;
#endif

struct PgpEnv;
#ifndef TYPE_PGPENV
#define TYPE_PGPENV 1
typedef struct PgpEnv PgpEnv;
#endif

struct PgpKeySpec PGPExport *pgpKeySpecCreate(struct PgpEnv const *env);
struct PgpKeySpec PGPExport *pgpKeySpecCopy(struct PgpKeySpec const *ks);
void PGPExport pgpKeySpecDestroy(struct PgpKeySpec *ks);

PgpVersion PGPExport pgpKeySpecVersion(struct PgpKeySpec const *ks);

word32 PGPExport pgpKeySpecCreation(struct PgpKeySpec const *ks);
word16 PGPExport pgpKeySpecValidity(struct PgpKeySpec const *ks);

int PGPExport pgpKeySpecSetVersion(struct PgpKeySpec *ks, PgpVersion ver);
int PGPExport pgpKeySpecSetCreation(struct PgpKeySpec *ks, word32 creation);
int PGPExport pgpKeySpecSetValidity(struct PgpKeySpec *ks, word16 validity);

#ifdef __cplusplus
}
#endif

#endif /* PGPKEYSPEC_H */
