/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpCompress.c,v 1.7 1999/03/20 22:43:24 dgal Exp $
____________________________________________________________________________*/

#include "pgpCompressPriv.h"
#include "pgpCompDeflate.h"
#include "pgpCompLZS.h"
#include "pgpMem.h"

struct PGPCompContext
{
	PGPMemoryMgrRef memoryMgr;

	PGPCompInitCompressionProc		initCompressProc;
	PGPCompCompressionProc			compressProc;
	PGPCompContinueCompressProc		continueCompressProc;
	PGPCompFinishCompressProc		finishCompressProc;
	PGPCompCleanupCompressionProc	cleanupCompressProc;

	PGPCompInitDecompressionProc		initDecompressProc;
	PGPCompDecompressionProc			decompressProc;
	PGPCompContinueDecompressProc		continueDecompressProc;
	PGPCompCleanupDecompressionProc		cleanupDecompressProc;

	PGPUserValue	compressUserValue;
	PGPUserValue	decompressUserValue;
};


static PGPError sInitNullCompressProc(PGPMemoryMgrRef memoryMgr,
					PGPUserValue *userValue);

static PGPError sNullCompressProc(PGPUserValue userValue,
					PGPByte *inputBuffer, PGPSize inputBufferSize, 
					PGPByte **outputBuffer, PGPSize *outputBufferSize,
					PGPSize *actualOutputSize);

static PGPError sNullContinueCompressProc(PGPUserValue userValue,
					PGPByte *inputBuffer, PGPSize inputBufferSize, 
					PGPByte *outputBuffer, PGPSize outputBufferSize,
					PGPSize *inputBytesUsed, PGPSize *outputBytesUsed);

static PGPError sNullFinishCompressProc(PGPUserValue userValue,
					PGPByte *outputBuffer, PGPSize outputBufferSize,
					PGPSize *outputBytesUsed, PGPBoolean *moreOutputNeeded);

static PGPError sCleanupNullCompressProc(PGPUserValue *userValue);

static PGPError sInitNullDecompressProc(PGPMemoryMgrRef memoryMgr,
					PGPUserValue *userValue);

static PGPError sNullDecompressProc(PGPUserValue userValue,
					PGPByte *inputBuffer, PGPSize inputBufferSize, 
					PGPByte **outputBuffer, PGPSize *outputBufferSize,
					PGPSize *actualOutputSize);

static PGPError sNullContinueDecompressProc(PGPUserValue userValue,
					PGPByte *inputBuffer, PGPSize inputBufferSize, 
					PGPByte *outputBuffer, PGPSize outputBufferSize,
					PGPSize *inputBytesUsed, PGPSize *outputBytesUsed,
					PGPBoolean *finished);

static PGPError sCleanupNullDecompressProc(PGPUserValue *userValue);


PGPError PGPNewCompContext(PGPMemoryMgrRef memoryMgr, 
			PGPCompressionAlgorithm algorithm, PGPCompContextRef *comp)
{
	PGPCompContextRef newComp;
	PGPError err = kPGPError_NoErr;

	if (IsNull(comp))
		return kPGPError_BadParams;

	*comp = NULL;

	if (!PGPMemoryMgrRefIsValid(memoryMgr))
		return kPGPError_BadParams;

	newComp = (PGPCompContextRef) PGPNewData(memoryMgr, 
									sizeof(PGPCompContext),
									kPGPMemoryMgrFlags_Clear);

	if (IsNull(newComp))
		return kPGPError_OutOfMemory;

	newComp->memoryMgr	= memoryMgr;

	switch (algorithm)
	{
	case kPGPCompAlgorithm_None:
		newComp->initCompressProc = sInitNullCompressProc;
		newComp->compressProc = sNullCompressProc;
		newComp->continueCompressProc = sNullContinueCompressProc;
		newComp->finishCompressProc = sNullFinishCompressProc;
		newComp->cleanupCompressProc = sCleanupNullCompressProc;

		newComp->initDecompressProc = sInitNullDecompressProc;
		newComp->decompressProc = sNullDecompressProc;
		newComp->continueDecompressProc = sNullContinueDecompressProc;
		newComp->cleanupDecompressProc = sCleanupNullDecompressProc;
		break;

	case kPGPCompAlgorithm_Deflate:
		newComp->initCompressProc = pgpInitDeflateCompressProc;
		newComp->compressProc = pgpDeflateCompressProc;
		newComp->continueCompressProc = pgpDeflateContinueCompressProc;
		newComp->finishCompressProc = pgpDeflateFinishCompressProc;
		newComp->cleanupCompressProc = pgpCleanupDeflateCompressProc;

		newComp->initDecompressProc = pgpInitDeflateDecompressProc;
		newComp->decompressProc = pgpDeflateDecompressProc;
		newComp->continueDecompressProc = pgpDeflateContinueDecompressProc;
		newComp->cleanupDecompressProc = pgpCleanupDeflateDecompressProc;
		break;

	case kPGPCompAlgorithm_LZS:
		newComp->initCompressProc = pgpInitLZSCompressProc;
		newComp->compressProc = pgpLZSCompressProc;
		newComp->continueCompressProc = pgpLZSContinueCompressProc;
		newComp->finishCompressProc = pgpLZSFinishCompressProc;
		newComp->cleanupCompressProc = pgpCleanupLZSCompressProc;

		newComp->initDecompressProc = pgpInitLZSDecompressProc;
		newComp->decompressProc = pgpLZSDecompressProc;
		newComp->continueDecompressProc = pgpLZSContinueDecompressProc;
		newComp->cleanupDecompressProc = pgpCleanupLZSDecompressProc;
		break;

	default:
		PGPFreeData(newComp);
		newComp = NULL;
		err = kPGPError_BadParams;
		break;
	}

	if (IsntPGPError(err))
		err = newComp->initCompressProc(memoryMgr, 
				&(newComp->compressUserValue));

	if (IsntPGPError(err))
		err = newComp->initDecompressProc(memoryMgr, 
				&(newComp->decompressUserValue));

	if (IsntPGPError(err))
		*comp = newComp;

	return err;
}


PGPError PGPCompressBuffer(PGPCompContextRef comp, PGPByte *inputBuffer,
			PGPSize inputBufferSize, PGPBoolean secureOutputBuffer, 
			PGPByte **outputBuffer, PGPSize *outputBufferSize)
{
	PGPByte *tempBuffer = NULL;
	PGPSize tempBufferSize = 0;
	PGPSize	actualBufferSize = 0;
	PGPError err = kPGPError_NoErr;

	if (IsNull(outputBuffer) || IsNull(outputBufferSize))
		return kPGPError_BadParams;

	*outputBuffer = NULL;
	*outputBufferSize = 0;

	if (IsNull(inputBuffer))
		return kPGPError_BadParams;

	tempBufferSize = inputBufferSize * 2;

	if (secureOutputBuffer)
		tempBuffer = (PGPByte *) PGPNewSecureData(comp->memoryMgr, 
									tempBufferSize,
									kPGPMemoryMgrFlags_Clear);
	else
		tempBuffer = (PGPByte *) PGPNewData(comp->memoryMgr, 
									tempBufferSize,
									kPGPMemoryMgrFlags_Clear);

	if (IsNull(tempBuffer))
		return kPGPError_OutOfMemory;

	err = comp->compressProc(comp->compressUserValue, inputBuffer, 
			inputBufferSize, &tempBuffer, &tempBufferSize, 
			&actualBufferSize);

	*outputBuffer = tempBuffer;
	*outputBufferSize = actualBufferSize;
	return err;
}


PGPError PGPContinueCompress(PGPCompContextRef comp, PGPByte *inputBuffer,
			PGPSize inputBufferSize, PGPByte *outputBuffer, 
			PGPSize outputBufferSize, PGPSize *inputBytesUsed,
			PGPSize *outputBytesUsed)
{
	PGPError err = kPGPError_NoErr;

	if (IsNull(inputBytesUsed) || IsNull(outputBytesUsed))
		return kPGPError_BadParams;

	*outputBytesUsed = 0;
	*inputBytesUsed = 0;

	if (IsNull(inputBuffer) || IsNull(outputBuffer))
		return kPGPError_BadParams;

	err = comp->continueCompressProc(comp->compressUserValue, inputBuffer, 
			inputBufferSize, outputBuffer, outputBufferSize,
			inputBytesUsed, outputBytesUsed);

	return err;
}


PGPError PGPFinishCompress(PGPCompContextRef comp, PGPByte *outputBuffer, 
			PGPSize outputBufferSize, PGPSize *outputBytesUsed,
			PGPBoolean *moreOutputNeeded)
{
	PGPError err = kPGPError_NoErr;

	if (IsNull(moreOutputNeeded) || IsNull(outputBytesUsed))
		return kPGPError_BadParams;

	*outputBytesUsed = 0;
	*moreOutputNeeded = FALSE;

	if (IsNull(outputBuffer))
		return kPGPError_BadParams;

	err = comp->finishCompressProc(comp->compressUserValue, outputBuffer, 
			outputBufferSize, outputBytesUsed, moreOutputNeeded);

	return err;
}


PGPError PGPDecompressBuffer(PGPCompContextRef comp, PGPByte *inputBuffer,
			PGPSize inputBufferSize, PGPBoolean secureOutputBuffer, 
			PGPByte **outputBuffer, PGPSize *outputBufferSize)
{
	PGPByte *tempBuffer = NULL;
	PGPSize tempBufferSize = 0;
	PGPSize	actualBufferSize = 0;
	PGPError err = kPGPError_NoErr;

	if (IsNull(outputBuffer) || IsNull(outputBufferSize))
		return kPGPError_BadParams;

	*outputBuffer = NULL;
	*outputBufferSize = 0;

	if (IsNull(inputBuffer))
		return kPGPError_BadParams;

	tempBufferSize = inputBufferSize * 10;

	if (secureOutputBuffer)
		tempBuffer = (PGPByte *) PGPNewSecureData(comp->memoryMgr, 
									tempBufferSize,
									kPGPMemoryMgrFlags_Clear);
	else
		tempBuffer = (PGPByte *) PGPNewData(comp->memoryMgr, 
									tempBufferSize,
									kPGPMemoryMgrFlags_Clear);

	if (IsNull(tempBuffer))
		return kPGPError_OutOfMemory;

	err = comp->decompressProc(comp->decompressUserValue, inputBuffer, 
			inputBufferSize, &tempBuffer, &tempBufferSize, 
			&actualBufferSize);

	*outputBuffer = tempBuffer;
	*outputBufferSize = actualBufferSize;
	return err;
}


PGPError PGPContinueDecompress(PGPCompContextRef comp, PGPByte *inputBuffer,
			PGPSize inputBufferSize, PGPByte *outputBuffer, 
			PGPSize outputBufferSize, PGPSize *inputBytesUsed,
			PGPSize *outputBytesUsed, PGPBoolean *finished)
{
	PGPError err = kPGPError_NoErr;

	if (IsNull(inputBytesUsed) || IsNull(outputBytesUsed) || IsNull(finished))
		return kPGPError_BadParams;

	*outputBytesUsed = 0;
	*inputBytesUsed = 0;
	*finished = FALSE;

	if (IsNull(inputBuffer) || IsNull(outputBuffer))
		return kPGPError_BadParams;

	err = comp->continueDecompressProc(comp->decompressUserValue, inputBuffer, 
			inputBufferSize, outputBuffer, outputBufferSize,
			inputBytesUsed, outputBytesUsed, finished);

	return err;
}


PGPError PGPFreeCompContext(PGPCompContextRef comp)
{
	PGPError err = kPGPError_NoErr;

	if (IsNull(comp))
		return kPGPError_BadParams;

	comp->cleanupCompressProc(&(comp->compressUserValue));
	comp->cleanupDecompressProc(&(comp->decompressUserValue));
	
	PGPFreeData(comp);	
	return err;
}


static PGPError sInitNullCompressProc(PGPMemoryMgrRef memoryMgr,
					PGPUserValue *userValue)
{
	(void) memoryMgr;
	(void) userValue;
	
	return kPGPError_NoErr;
}


static PGPError sNullCompressProc(PGPUserValue userValue,
					PGPByte *inputBuffer, PGPSize inputBufferSize, 
					PGPByte **outputBuffer, PGPSize *outputBufferSize,
					PGPSize *actualOutputSize)
{
	PGPError err = kPGPError_NoErr;
	
	(void) userValue;

	if (*outputBufferSize < inputBufferSize)
		return kPGPError_BadParams;

	pgpCopyMemory(inputBuffer, *outputBuffer, inputBufferSize);
	*actualOutputSize = inputBufferSize;
	return err;
}


static PGPError sNullContinueCompressProc(PGPUserValue userValue,
					PGPByte *inputBuffer, PGPSize inputBufferSize, 
					PGPByte *outputBuffer, PGPSize outputBufferSize,
					PGPSize *inputBytesUsed, PGPSize *outputBytesUsed)
{
	PGPError err = kPGPError_NoErr;
	PGPSize copySize;
	
	(void) userValue;

	if (outputBufferSize < inputBufferSize)
		copySize = outputBufferSize;
	else
		copySize = inputBufferSize;

	pgpCopyMemory(inputBuffer, outputBuffer, copySize);
	
	*inputBytesUsed = copySize;
	*outputBytesUsed = copySize;

	return err;
}


static PGPError sNullFinishCompressProc(PGPUserValue userValue,
					PGPByte *outputBuffer, PGPSize outputBufferSize,
					PGPSize *outputBytesUsed, PGPBoolean *moreOutputNeeded)
{
	PGPError err = kPGPError_NoErr;
	
	(void) userValue;
	(void) outputBuffer;
	(void) outputBufferSize;

	*outputBytesUsed = 0;
	*moreOutputNeeded = FALSE;

	return err;
}

static PGPError sCleanupNullCompressProc(PGPUserValue *userValue)
{
	(void) userValue;
	
	return kPGPError_NoErr;
}


static PGPError sInitNullDecompressProc(PGPMemoryMgrRef memoryMgr,
					PGPUserValue *userValue)
{
	(void) memoryMgr;
	(void) userValue;
	
	return kPGPError_NoErr;
}


static PGPError sNullDecompressProc(PGPUserValue userValue,
					PGPByte *inputBuffer, PGPSize inputBufferSize, 
					PGPByte **outputBuffer, PGPSize *outputBufferSize,
					PGPSize *actualOutputSize)
{
	PGPError err = kPGPError_NoErr;

	(void) userValue;
	
	if (*outputBufferSize < inputBufferSize)
		return kPGPError_BadParams;

	pgpCopyMemory(inputBuffer, *outputBuffer, inputBufferSize);
	*actualOutputSize = inputBufferSize;
	return err;
}


static PGPError sNullContinueDecompressProc(PGPUserValue userValue,
					PGPByte *inputBuffer, PGPSize inputBufferSize, 
					PGPByte *outputBuffer, PGPSize outputBufferSize,
					PGPSize *inputBytesUsed, PGPSize *outputBytesUsed,
					PGPBoolean *finished)
{
	PGPError err = kPGPError_NoErr;
	PGPSize copySize;
	
	(void) userValue;

	if (outputBufferSize < inputBufferSize)
		copySize = outputBufferSize;
	else
		copySize = inputBufferSize;

	pgpCopyMemory(inputBuffer, outputBuffer, copySize);
	
	*inputBytesUsed = copySize;
	*outputBytesUsed = copySize;
	*finished = FALSE;

	return err;
}


static PGPError sCleanupNullDecompressProc(PGPUserValue *userValue)
{
	(void) userValue;
	
	return kPGPError_NoErr;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/