/*
 * pgpFileType.h - classify files
 *
 * Copyright (C) 1993-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpFileType.h,v 1.3.2.1 1997/06/07 09:51:41 mhw Exp $
 */

#ifndef PGPFILETYPE_H
#define PGPFILETYPE_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpFileType {
	char const *typename;	 /* Noun/adjective to modify "file" */
		char const *ext;		/* .zip, .lha, .gz, etc. */
		int compressible;		/* Can it be further compressed (.tar) */
	};
#ifndef TYPE_PGPFILETYPE
#define TYPE_PGPFILETYPE 1
typedef struct PgpFileType PgpFileType;
#endif

struct PgpFileType const PGPExport *pgpFileType (byte const *prefix,
	unsigned len);
int PGPExport pgpFileTypeBinary (char const *lang, byte const *buf,
	unsigned len);
int PGPExport pgpFileTypePGP (byte const *prefix, unsigned len);

#ifdef __cplusplus
}
#endif

#endif /* PGPFILETYPE_H */
