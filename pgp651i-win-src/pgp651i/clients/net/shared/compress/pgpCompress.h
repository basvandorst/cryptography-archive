/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpCompress.h,v 1.4 1999/03/20 22:43:24 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_pgpCompress_h	/* [ */
#define Included_pgpCompress_h

#include "pgpPubTypes.h"
#include "pgpMemoryMgr.h"
#include "pgpErrors.h"

typedef struct PGPCompContext		PGPCompContext;
typedef struct PGPCompContext *		PGPCompContextRef;

typedef enum _PGPCompressionAlgorithm
{
	kPGPCompAlgorithm_None		= 0,
	kPGPCompAlgorithm_Deflate	= 1,
	kPGPCompAlgorithm_LZS		= 2

} PGPCompressionAlgorithm;


PGP_BEGIN_C_DECLARATIONS

PGPError PGPNewCompContext(PGPMemoryMgrRef memoryMgr, 
			PGPCompressionAlgorithm algorithm, PGPCompContextRef *comp);

/* Caller must call PGPFreeData(outputBuffer) when finished with it */

PGPError PGPCompressBuffer(PGPCompContextRef comp, PGPByte *inputBuffer,
			PGPSize inputBufferSize, PGPBoolean secureOutputBuffer, 
			PGPByte **outputBuffer, PGPSize *outputBufferSize);

PGPError PGPContinueCompress(PGPCompContextRef comp, PGPByte *inputBuffer,
			PGPSize inputBufferSize, PGPByte *outputBuffer, 
			PGPSize outputBufferSize, PGPSize *inputBytesUsed,
			PGPSize *outputBytesUsed);

PGPError PGPFinishCompress(PGPCompContextRef comp, PGPByte *outputBuffer, 
			PGPSize outputBufferSize, PGPSize *outputBytesUsed,
			PGPBoolean *moreOutputNeeded);

/* Caller must call PGPFreeData(outputBuffer) when finished with it */

PGPError PGPDecompressBuffer(PGPCompContextRef comp, PGPByte *inputBuffer,
			PGPSize inputBufferSize, PGPBoolean secureOutputBuffer, 
			PGPByte **outputBuffer, PGPSize *outputBufferSize);

PGPError PGPContinueDecompress(PGPCompContextRef comp, PGPByte *inputBuffer,
			PGPSize inputBufferSize, PGPByte *outputBuffer, 
			PGPSize outputBufferSize, PGPSize *inputBytesUsed,
			PGPSize *outputBytesUsed, PGPBoolean *finished);

PGPError PGPFreeCompContext(PGPCompContextRef comp);

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpCompress_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
