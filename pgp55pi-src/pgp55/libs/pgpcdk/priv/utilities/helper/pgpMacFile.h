/*
 * pgpMacFile.h -- Header for MacBinarizing PGPFile implementation
 *
 * $Id: pgpMacFile.h,v 1.11 1997/07/25 01:49:45 heller Exp $
 */

#ifndef Included_pgpMacFile_h	/* [ */
#define Included_pgpMacFile_h

#include "pgpTypes.h"
#include "pgpFileRef.h"
#include "pgpFile.h"
#include "pgpFileMod.h"

PGP_BEGIN_C_DECLARATIONS

/*
 * The <flags> params below are only used to convey MacBinary mode:
 *		kPGPFileOpenMaybeLocalEncode or kPGPFileOpenForceLocalEncode
 */
PGPFile  *		pgpFileRefMacWriteOpen( 
					PFLConstFileSpecRef fileRef,
					PGPFileType fileType,
					PGPFileOpenFlags flags, PGPError *error);
PGPFile  *		pgpFileRefMacReadOpen(
					PFLConstFileSpecRef fileRef,
					PGPFileOpenFlags flags, PGPError *error);
PGPError 		pgpMacCalcFileSize( 
					PFLConstFileSpecRef fileRef,
					PGPFileOpenFlags flags, size_t *fileSize);
OSType			pgpGetMacFileTypeFromPGPFileType( PGPFileType fileType );
OSType			pgpGetMacFileCreatorFromPGPFileType( PGPFileType fileType );
					
PGP_END_C_DECLARATIONS

#endif	/* ] Included_pgpMacFile_h */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
