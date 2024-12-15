/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpCompLZS.h,v 1.3 1999/03/20 22:43:23 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_pgpCompLZS_h	/* [ */
#define Included_pgpCompLZS_h

#include "pgpCompressPriv.h"

PGPError pgpInitLZSCompressProc(PGPMemoryMgrRef memoryMgr,
			PGPUserValue *userValue);

PGPError pgpLZSCompressProc(PGPUserValue userValue,
			PGPByte *inputBuffer, PGPSize inputBufferSize, 
			PGPByte **outputBuffer, PGPSize *outputBufferSize,
			PGPSize *actualOutputSize);

PGPError pgpLZSContinueCompressProc(PGPUserValue userValue,
			PGPByte *inputBuffer, PGPSize inputBufferSize, 
			PGPByte *outputBuffer, PGPSize outputBufferSize,
			PGPSize *inputBytesUsed, PGPSize *outputBytesUsed);

PGPError pgpLZSFinishCompressProc(PGPUserValue userValue,
			PGPByte *outputBuffer, PGPSize outputBufferSize,
			PGPSize *outputBytesUsed, PGPBoolean *moreOutputNeeded);

PGPError pgpCleanupLZSCompressProc(PGPUserValue *userValue);

PGPError pgpInitLZSDecompressProc(PGPMemoryMgrRef memoryMgr,
			PGPUserValue *userValue);

PGPError pgpLZSDecompressProc(PGPUserValue userValue,
			PGPByte *inputBuffer, PGPSize inputBufferSize, 
			PGPByte **outputBuffer, PGPSize *outputBufferSize,
			PGPSize *actualOutputSize);

PGPError pgpLZSContinueDecompressProc(PGPUserValue userValue,
			PGPByte *inputBuffer, PGPSize inputBufferSize, 
			PGPByte *outputBuffer, PGPSize outputBufferSize,
			PGPSize *inputBytesUsed, PGPSize *outputBytesUsed,
			PGPBoolean *finished);

PGPError pgpCleanupLZSDecompressProc(PGPUserValue *userValue);

#endif /* ] Included_pgpCompLZS_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/