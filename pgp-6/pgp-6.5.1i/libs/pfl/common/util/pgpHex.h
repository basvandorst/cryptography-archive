/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpHex.h,v 1.2 1999/03/10 02:51:40 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpHex_h	/* [ */
#define Included_pgpHex_h

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS


PGPBoolean	pgpIsValidHexChar( char	theChar );

PGPByte		pgpHexCharToNibble( char nibble );

void		pgpBytesToHex( PGPByte const * keyBytes,
				PGPSize	numBytes, PGPBoolean add0x, char *outString );

void		pgpHexToBytes( const char * hex,
				PGPUInt32 numBinaryBytes, PGPByte * buffer );

PGPUInt32	pgpHexToPGPUInt32( const char *hex );
	
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpHex_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
