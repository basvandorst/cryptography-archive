/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpPassBuffer.h,v 1.2 1999/03/10 02:53:59 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpPassBuffer_h	/* [ */
#define Included_pgpPassBuffer_h

#include "pgpMemoryMgr.h"
#include "pgpPubTypes.h"

typedef struct PGPPassBuffer	*PGPPassBufferRef;

#define	kInvalidPGPPassBufferRef		((PGPPassBufferRef) NULL)
#define PGPPassBufferRefIsValid(ref)	((ref) != kInvalidPGPPassBufferRef)

enum PGPPassBufferType_
{
	kPGPPassBufferType_Invalid			= 0,
	kPGPPassBufferType_Passphrase		= 1,
	kPGPPassBufferType_PassKey			= 2,
	
	PGP_ENUM_FORCE( PGPPassBufferType_ )
} ;
PGPENUM_TYPEDEF( PGPPassBufferType_, PGPPassBufferType );

PGP_BEGIN_C_DECLARATIONS

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

PGPError			PGPNewPassBuffer(PGPMemoryMgrRef memoryMgr,
							PGPPassBufferRef *buffer);
void				PGPFreePassBuffer(PGPPassBufferRef buffer);
PGPError			PGPCopyPassBuffer(PGPPassBufferRef buffer,
							PGPPassBufferRef *bufferCopy);
	
PGPError			PGPPassBufferSetPassphrase(PGPPassBufferRef buffer,
							const char *passphrase);
PGPError			PGPPassBufferSetPassKey(PGPPassBufferRef buffer,
							const PGPByte *passKey, PGPSize passKeySize);
PGPError			PGPClearPassBuffer(PGPPassBufferRef buffer);

PGPError			PGPPassBufferGetPassphrasePtr(PGPPassBufferRef buffer,
							char **passphrasePtr);
PGPError			PGPPassBufferGetPassKeyPtr(PGPPassBufferRef buffer,
							PGPByte **passpKeyPtr, PGPSize *passKeySize);
	
PGPOptionListRef	PGPOPassBuffer(PGPContextRef context,
							PGPPassBufferRef buffer);
PGPPassBufferType	PGPGetPassBufferType(PGPPassBufferRef buffer);
						
#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpPassBuffer_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/