/*
 * pgpMacFile.h -- Header for MacBinarizing PgpFile implementation
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpMacFile.h,v 1.4.2.1 1997/06/07 09:50:09 mhw Exp $
 */

#ifndef PGPMACFILE_H  /* [ */
#define PGPMACFILE_H

#include "pgpTypes.h"
#include "pgpFileRef.h"
#include "pgpFile.h"
#include "pgpFileMod.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The <flags> params below are only used to convey MacBinary mode:
 * kPGPFileOpenMaybeLocalEncode or kPGPFileOpenForceLocalEncode
 */
PgpFile PGPExport *   pgpFileRefMacWriteOpen(PGPFileRef const *fileRef,
                              PGPFileType fileType,
                              PGPFileOpenFlags flags, PGPError *error);
PgpFile PGPExport *   pgpFileRefMacReadOpen(PGPFileRef const *fileRef,
                              PGPFileOpenFlags flags, PGPError *error);
PGPError PGPExport    pgpMacCalcFileSize(PGPFileRef const *fileRef,
                              PGPFileOpenFlags flags, size_t *fileSize);

#ifdef __cplusplus
}
#endif

#endif  /* ] PGPMACFILE_H */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
