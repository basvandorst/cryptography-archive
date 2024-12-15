/*
* pgpRndSeed.h -- read and write the randseed.bin file.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* This is a Public API Function Header.
*
* $Id: pgpRndSeed.h,v 1.5.2.1 1997/06/07 09:51:35 mhw Exp $
*/

#ifndef PGPRNDSEED_H
#define PGPRNDSEED_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct PgpCfbContext;
#ifndef TYPE_PGPCFBCONTEXT
#define TYPE_PGPCFBCONTEXT 1
typedef struct PgpCfbContext PgpCfbContext;
#endif

struct PgpRandomContext;
#ifndef TYPE_PGPRANDOMCONTEXT
#define TYPE_PGPRANDOMCONTEXT 1
typedef struct PgpRandomContext PgpRandomContext;
#endif

/* We want to see at least this many bytes in randseed */
#define PGP_SEED_MIN_BYTES		24

int PGPExport pgpRandSeedRead (FILE *file, struct PgpRandomContext const *rc);
void PGPExport pgpRandSeedWrite (FILE *file,
	struct PgpRandomContext const *rc, struct PgpCfbContext *cfb);

#ifdef __cplusplus
}
#endif

#endif /* PGPRNDSEED_H */
