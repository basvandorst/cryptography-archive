/*
 * pgpFileType.h - classify files
 *
 * $Id: pgpFileType.h,v 1.9 1997/06/25 19:42:02 lloyd Exp $
 */

#ifndef Included_pgpFileType_h
#define Included_pgpFileType_h

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS

struct PGPFileTypeStruct {
	char const *	typename;	/* Noun/adjective to modify "file" */
	char const *	ext;	/* .zip, .lha, .gz, etc. */
	PGPBoolean		compressible;/* Can it be further compressed (.tar) */
};



			
int		pgpFileTypePGP (PGPByte const *prefix, unsigned len);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpFileType_h */
