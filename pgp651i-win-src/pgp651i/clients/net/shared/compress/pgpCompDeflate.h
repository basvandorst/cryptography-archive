/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpCompDeflate.h,v 1.3 1999/03/20 22:43:23 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_pgpCompDeflate_h	/* [ */
#define Included_pgpCompDeflate_h

#include "pgpCompressPriv.h"

PGPError pgpInitDeflateCompressProc(PGPMemoryMgrRef memoryMgr,
			PGPUserValue *userValue);

PGPError pgpDeflateCompressProc(PGPUserValue userValue,
			PGPByte *inputBuffer, PGPSize inputBufferSize, 
			PGPByte **outputBuffer, PGPSize *outputBufferSize,
			PGPSize *actualOutputSize);

PGPError pgpDeflateContinueCompressProc(PGPUserValue userValue,
			PGPByte *inputBuffer, PGPSize inputBufferSize, 
			PGPByte *outputBuffer, PGPSize outputBufferSize,
			PGPSize *inputBytesUsed, PGPSize *outputBytesUsed);

PGPError pgpDeflateFinishCompressProc(PGPUserValue userValue,
			PGPByte *outputBuffer, PGPSize outputBufferSize,
			PGPSize *outputBytesUsed, PGPBoolean *moreOutputNeeded);

PGPError pgpCleanupDeflateCompressProc(PGPUserValue *userValue);

PGPError pgpInitDeflateDecompressProc(PGPMemoryMgrRef memoryMgr,
			PGPUserValue *userValue);

PGPError pgpDeflateDecompressProc(PGPUserValue userValue,
			PGPByte *inputBuffer, PGPSize inputBufferSize, 
			PGPByte **outputBuffer, PGPSize *outputBufferSize,
			PGPSize *actualOutputSize);

PGPError pgpDeflateContinueDecompressProc(PGPUserValue userValue,
			PGPByte *inputBuffer, PGPSize inputBufferSize, 
			PGPByte *outputBuffer, PGPSize outputBufferSize,
			PGPSize *inputBytesUsed, PGPSize *outputBytesUsed,
			PGPBoolean *finished);

PGPError pgpCleanupDeflateDecompressProc(PGPUserValue *userValue);

#endif /* ] Included_pgpCompDeflate_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
