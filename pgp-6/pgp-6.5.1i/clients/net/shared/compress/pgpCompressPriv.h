/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpCompressPriv.h,v 1.4 1999/03/20 22:43:24 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_pgpCompressPriv_h	/* [ */
#define Included_pgpCompressPriv_h

#include "pgpCompress.h"

typedef PGPError (*PGPCompInitCompressionProc)(PGPMemoryMgrRef memoryMgr,
						PGPUserValue *userValue);

typedef	PGPError (*PGPCompCompressionProc)(PGPUserValue userValue,
						PGPByte *inputBuffer, PGPSize inputBufferSize, 
						PGPByte **outputBuffer, PGPSize *outputBufferSize,
						PGPSize *actualOutputSize);

typedef	PGPError (*PGPCompContinueCompressProc)(PGPUserValue userValue,
						PGPByte *inputBuffer, PGPSize inputBufferSize, 
						PGPByte *outputBuffer, PGPSize outputBufferSize,
						PGPSize *inputBytesUsed, PGPSize *outputBytesUsed);

typedef	PGPError (*PGPCompFinishCompressProc)(PGPUserValue userValue,
						PGPByte *outputBuffer, PGPSize outputBufferSize,
						PGPSize *outputBytesUsed, 
						PGPBoolean *moreOutputNeeded);

typedef PGPError (*PGPCompCleanupCompressionProc)(PGPUserValue *userValue);

typedef PGPError (*PGPCompInitDecompressionProc)(PGPMemoryMgrRef memoryMgr,
						PGPUserValue *userValue);

typedef	PGPError (*PGPCompDecompressionProc)(PGPUserValue userValue,
						PGPByte *inputBuffer, PGPSize inputBufferSize, 
						PGPByte **outputBuffer, PGPSize *outputBufferSize,
						PGPSize *actualOutputSize);

typedef	PGPError (*PGPCompContinueDecompressProc)(PGPUserValue userValue,
						PGPByte *inputBuffer, PGPSize inputBufferSize, 
						PGPByte *outputBuffer, PGPSize outputBufferSize,
						PGPSize *inputBytesUsed, PGPSize *outputBytesUsed,
						PGPBoolean *finished);

typedef PGPError (*PGPCompCleanupDecompressionProc)(PGPUserValue *userValue);

#endif /* ] Included_pgpCompressPriv_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
