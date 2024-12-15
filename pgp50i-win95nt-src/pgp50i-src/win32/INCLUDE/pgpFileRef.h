/*
 * pgpFileRef.h -- Platform independent filename handling module
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpFileRef.h,v 1.20.2.2 1997/06/07 09:50:06 mhw Exp $
 */

#ifndef PGPFILEREF_H /* [ */
#define PGPFILEREF_H

#include <stdio.h>
#include "pgpFile.h"
#include "pgpFileMod.h"
#include "pgpErr.h"

#if MACINTOSH /* [ */

#define kPGPMacFileTypeArmorFile     'TEXT'
#define kPGPMacFileTypeEncryptedData 'pgEF'
#define kPGPMacFileTypeSignedData    'pgSF'
#define kPGPMacFileTypeDetachedSig   'pgDS'

#endif  /* ] MACINTOSH */

/* This must be kept in sync with the tables in pgpFileRef.c */
typedef enum PGPFileType_
{
    kPGPFileTypeNone,
    kPGPFileTypePrefs,
    kPGPFileTypePubRing,
    kPGPFileTypePrivRing,
    kPGPFileTypeDetachedSig,
    kPGPFileTypeRandomSeed,
    kPGPFileTypeExportedKeys,
    kPGPFileTypeArmorFile,
    kPGPFileTypeEncryptedData,
    kPGPFileTypeDecryptedText,
    kPGPFileTypeDecryptedBin,
    kPGPFileTypeSignedData,
    kPGPFileTypeNumTypes
} PGPFileType;

typedef struct PGPFileRef_  PGPFileRef;

typedef int PGPFileOpenFlags;

#define kPGPFileOpenReadPerm            0x001
#define kPGPFileOpenWritePerm           0x002
#define kPGPFileOpenReadWritePerm (kPGPFileOpenReadPerm \
                                 | kPGPFileOpenWritePerm)
#define kPGPFileOpenAppend              0x004
#define kPGPFileOpenTruncate            0x008
#define kPGPFileOpenCreate              0x010
#define kPGPFileOpenTextMode            0x020 /* XXX To be phased out */
#define kPGPFileOpenMaybeLocalEncode    0x040 /* eg, MacBin on Mac */
#define kPGPFileOpenForceLocalEncode    0x080 /* eg, MacBin on Mac */
/*
 * The following flag modifies one of the two flags above to strip out
 * volatile information such as dates, filenames, icon colors, etc.
 * It is not necessarily suitable for recreating a file, and should only
 * be used for detached signatures.
 */
#define kPGPFileOpenLocalEncodeHashOnly 0x100
/*
 * The following flag modifies the behavior of LocalEncode on the
 * Mac (MacBinary) to accept a CRC of 0 in the header.
 */
#define kPGPFileOpenNoMacBinCRCOkay     0x200

#define kPGPFileOpenStdWriteFlags (kPGPFileOpenWritePerm  \
                                 | kPGPFileOpenTruncate   \
                                 | kPGPFileOpenCreate)
#define kPGPFileOpenStdAppendFlags (kPGPFileOpenWritePerm \
                                 | kPGPFileOpenAppend     \
                                 | kPGPFileOpenCreate)
#define kPGPFileOpenStdUpdateFlags (kPGPFileOpenStdWriteFlags \
                                 | kPGPFileOpenReadPerm)

#ifdef __cplusplus
extern "C" {
#endif

PGPFileRef PGPExport * pgpNewFileRefFromFullPath(char const *path);
PGPError PGPExport pgpFullPathFromFileRef(PGPFileRef const *fileRef,
                                          char **fullPathPtr);
PGPFileRef PGPExport * pgpCopyFileRef(PGPFileRef const *fileRef);
void PGPExport pgpFreeFileRef(PGPFileRef *fileRef);

#if MACINTOSH /* [ */
PGPFileRef PGPExport * pgpNewFileRefFromFSSpec(FSSpec const *spec);
PGPError PGPExport pgpFSSpecFromFileRef(PGPFileRef const *fileRef,
                                        FSSpec *spec);
#endif /* ] MACINTOSH */

uchar PGPExport * pgpExportFileRef(PGPFileRef const *fileRef,
                                   size_t *size);
PGPFileRef PGPExport * pgpImportFileRef(uchar const *buffer,
                                        size_t size);

/*
 * pgpGetFileRefName returns a string allocated with pgpAlloc(),
 * which the caller is responsible for freeing.
 */
char PGPExport * pgpGetFileRefName(PGPFileRef const *fileRef);
PGPError PGPExport pgpSetFileRefName(PGPFileRef **fileRef,
                                     char const *newName);

PGPError PGPExport pgpCalcFileSize(PGPFileRef const *fileRef,
                       PGPFileOpenFlags flags, size_t *fileSize);
PGPError PGPExport pgpCreateFile(PGPFileRef const *fileRef,
                       PGPFileType fileType);
PGPError PGPExport pgpDeleteFile(PGPFileRef const *fileRef);

/* oldRef and newRef must be the same PGPFileRefType and directory */
PGPError PGPExport    pgpRenameFile(PGPFileRef const *oldRef,
                                    PGPFileRef const *newRef);

/*
 * Create and open a temporary file in a platform-saavy manner.
 * The caller is responsible for deleting the file when finished.
 */
FILE PGPExport * pgpStdIOOpenTempFile(PGPFileRef **tempFileRef,
                       PGPError *error);

PgpFile PGPExport * pgpFileRefOpen(PGPFileRef const *fileRef,
                       PGPFileOpenFlags flags,
                       PGPFileType fileType,
                       PGPError *error);
PgpFileRead PGPExport * pgpFileRefReadCreate(PGPFileRef const *fileRef,
                       PGPFileOpenFlags flags, PGPError *error);

/* XXX Compatibility routines follow, intended to be phased out */

FILE PGPExport * pgpFileRefStdIOOpen(PGPFileRef const *fileRef,
                       PGPFileOpenFlags flags,
                       PGPFileType fileType, PGPError *error);
PGPError PGPExport pgpStdIOClose(FILE *stdIOFile);
PgpFile PGPExport * pgpFileRefReadOpen(PGPFileRef const *fileRef,
                       PgpUICb const *ui, void *ui_arg,
                       PGPError *error);
PgpFile PGPExport * pgpFileRefWriteOpen(PGPFileRef const *fileRef,
                       PGPFileType fileType,
                       PgpCfbContext *cfb, PGPError *error);
PgpFile PGPExport * pgpFileRefProcWriteOpen(PGPFileRef const *fileRef,
                       PGPFileType fileType,
                       int (*doClose) (FILE *file, void *arg),
                       void *arg, PGPError *error);

#ifdef __cplusplus
}
#endif

#endif /* ] PGPFILEREF_H */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
