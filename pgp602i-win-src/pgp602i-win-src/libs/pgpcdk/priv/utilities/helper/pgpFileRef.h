/*
 * pgpFileRef.h
 * Platform independent filename handling module
 *
 * Copyright (C) 1997 Network Associates, Inc. and its affiliates.
 * All rights reserved
 *
 * $Id: pgpFileRef.h,v 1.33.4.1.2.1 1998/11/12 03:23:20 heller Exp $
 */

#ifndef Included_pgpFileRef_h	/* [ */
#define Included_pgpFileRef_h

#include <stdio.h>
#include "pflTypes.h"
#include "pgpFile.h"
#include "pgpFileMod.h"
#include "pgpErrors.h"

/* This must be kept in sync with the tables in pgpFileRef.c and pgpMacFile.c*/
enum PGPFileType_
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
	kPGPFileTypeGroups,
	kPGPFileTypeDecryptedText,
	kPGPFileTypeDecryptedBin,
	kPGPFileTypeSignedData,
	kPGPFileTypeNumTypes,
	
	PGP_ENUM_FORCE( PGPFileType )
} ;
PGPENUM_TYPEDEF( PGPFileType_, PGPFileType );


typedef PGPInt16 PGPFileOpenFlags;

#define kPGPFileOpenReadPerm			0x001
#define kPGPFileOpenWritePerm			0x002
#define kPGPFileOpenReadWritePerm		(kPGPFileOpenReadPerm		\
										| kPGPFileOpenWritePerm)
#define kPGPFileOpenAppend				0x004
#define kPGPFileOpenTruncate			0x008
#define kPGPFileOpenCreate				0x010
#define kPGPFileOpenTextMode			0x020	/* XXX To be phased out */
#define kPGPFileOpenMaybeLocalEncode	0x040	/* eg, MacBin on Mac */
#define kPGPFileOpenForceLocalEncode	0x080	/* eg, MacBin on Mac */
/*
 * The following flag modifies one of the two flags above to strip out
 * volatile information such as dates, filenames, icon colors, etc.
 * It is not necessarily suitable for recreating a file, and should only
 * be used for detached signatures.
 */
#define kPGPFileOpenLocalEncodeHashOnly	0x100
/*
 * The following flag modifies the behavior of LocalEncode on the
 * Mac (MacBinary) to accept a CRC of 0 in the header.
 */
#define kPGPFileOpenNoMacBinCRCOkay		0x200
/*
 * The following flag causes additional file locking to be used, if
 * the normal platform file locking is lacking.  Currently, this only
 * affects Un*x systems, which perform no locking by default.
 */
#define kPGPFileOpenLockFile			0x400

#define kPGPFileOpenStdWriteFlags	(kPGPFileOpenWritePerm			\
									| kPGPFileOpenTruncate			\
									| kPGPFileOpenCreate)
#define kPGPFileOpenStdAppendFlags	(kPGPFileOpenWritePerm			\
									| kPGPFileOpenAppend			\
									| kPGPFileOpenCreate)
#define kPGPFileOpenStdUpdateFlags	(kPGPFileOpenStdWriteFlags		\
									| kPGPFileOpenReadPerm)

PGP_BEGIN_C_DECLARATIONS

/* get publicly available routines */
#include "pgpUtilities.h"


PGPMemoryMgrRef	pgpGetFileRefMemoryMgr( PFLConstFileSpecRef fileRef );

uchar  *		pgpExportFileRef(PFLConstFileSpecRef fileRef,
						size_t *size);
PGPError 		pgpImportFileRef( PGPContextRef context,
					uchar const *buffer, size_t size, PFLFileSpecRef *ref );

/*
 * pgpGetFileRefName returns a string allocated with pgpContextMemAlloc(),
 * which the caller is responsible for freeing.
 */
char  *			pgpGetFileRefName(PFLConstFileSpecRef fileRef);
PGPError 		pgpSetFileRefName(PFLFileSpecRef fileRef,
						char const *newName);



PGPError 		pgpCalcFileSize(PFLConstFileSpecRef fileRef,
						PGPFileOpenFlags flags, size_t *fileSize);
						
PGPError 		pgpCreateFile(PFLConstFileSpecRef fileRef,
						PGPFileType fileType);

/* depracated, do *not* use. Use PFLFileSpecRename() */
PGPError		pgpRenameFile( PFLConstFileSpecRef oldRef,
					PFLConstFileSpecRef	newRef);

/*
 * Create and open a temporary file in a platform-saavy manner.
 * The caller is responsible for deleting the file when finished.
 */
FILE  *			pgpStdIOOpenTempFile( PGPContextRef	context,
					PFLFileSpecRef *tempFileRef, PGPError *error);

PGPFile  *		pgpFileRefOpen(
						PGPContextRef context,
						PFLConstFileSpecRef fileRef,
						PGPFileOpenFlags flags,
						PGPFileType fileType, PGPError *error);
						
PGPFileRead  *	pgpFileRefReadCreate(
						PGPContextRef context,
						PFLConstFileSpecRef fileRef,
						PGPFileOpenFlags flags, PGPError *error,
						PGPFileDataType	*fileDataType);

/* XXX Compatibility routines follow, intended to be phased out */

PGPError		pgpFileRefStdIOOpen(PFLConstFileSpecRef fileRef,
					PGPFileOpenFlags flags,
					PGPFileType fileType, FILE **file);
					
PGPError 		pgpStdIOClose(FILE *stdIOFile);

#if 0	/* [ */

PGPFile *		pgpFileRefReadOpen(PFLConstFileSpecRef fileRef,
								PGPUICb const *ui, void *ui_arg,
								PGPError *error );
PGPFile *		pgpFileRefWriteOpen(PFLConstFileSpecRef fileRef,
								PGPFileType fileType,
								PGPCFBContext *cfb,
								PGPError *error);
PGPFile *		pgpFileRefProcWriteOpen(PFLConstFileSpecRef fileRef,
								PGPFileType fileType,
				      			int (*doClose) (FILE *file, void *arg),
				      			void *arg, PGPError *error);
#endif	/* ] */

PGP_END_C_DECLARATIONS

#endif	/* ] Included_pgpFileRef_h */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */