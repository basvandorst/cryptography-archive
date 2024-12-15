/*
 * pgpFileType.h - classify files
 *
 * $Id: pgpFileType.h,v 1.10 1997/12/01 19:35:17 lloyd Exp $
 */

#ifndef Included_pgpFileType_h
#define Included_pgpFileType_h

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS
	
int		pgpFileTypePGP (PGPByte const *prefix, unsigned len);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpFileType_h */
