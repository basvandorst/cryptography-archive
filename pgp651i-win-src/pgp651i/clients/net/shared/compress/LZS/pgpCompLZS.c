/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpCompLZS.c,v 1.9 1999/03/20 22:44:40 dgal Exp $
____________________________________________________________________________*/

#include "pgpCompLZS.h"
#include "pgpMem.h"
#include "LZSC.h"

typedef struct _PGPLZSContext
{
	LZSContextRef	context;
	PGPMemoryMgrRef	memoryMgr;
	PGPByte *		history;
} PGPLZSContext;

typedef PGPLZSContext *		PGPLZSContextRef;


PGPError pgpInitLZSCompressProc(PGPMemoryMgrRef memoryMgr,
			PGPUserValue *userValue)
{
	PGPLZSContextRef lzsContext;
	PGPError err = kPGPError_NoErr;

	if (IsNull(userValue))
		return kPGPError_BadParams;

	*userValue = NULL;

	lzsContext = (PGPLZSContextRef) PGPNewData(memoryMgr, 
										sizeof(PGPLZSContext),
										kPGPMemoryMgrFlags_Clear);

	if (IsNull(lzsContext))
		return kPGPError_OutOfMemory;

	lzsContext->memoryMgr = memoryMgr;
	
	lzsContext->context = (LZSContextRef) PGPNewData(memoryMgr, 
											LZS_GetContextSize(),
											kPGPMemoryMgrFlags_Clear);
	
	if (IsNull(lzsContext->context))
	{
		PGPFreeData(lzsContext);
		return kPGPError_OutOfMemory;
	}

	lzsContext->history = (PGPByte *) PGPNewData(memoryMgr, 
										LZS_HISTORY_SIZE,
										kPGPMemoryMgrFlags_Clear);
	
	if (IsNull(lzsContext->history))
	{
		PGPFreeData(lzsContext->context);
		PGPFreeData(lzsContext);
		return kPGPError_OutOfMemory;
	}

	LZS_InitHistory(lzsContext->context, lzsContext->history);

	*userValue = lzsContext;
	return err;
}


PGPError pgpLZSCompressProc(PGPUserValue userValue,
			PGPByte *inputBuffer, PGPSize inputBufferSize, 
			PGPByte **outputBuffer, PGPSize *outputBufferSize,
			PGPSize *actualOutputSize)
{
	PGPLZSContextRef lzsContext;
	PGPUInt16 returnCode;
	PGPUInt32 sourceCount;
	PGPUInt32 destCount;
	PGPByte *bufferPtr;
	PGPUInt16 perfMode = 2 << 3;		/* 0-2 << 3 */
	PGPUInt16 perfParam = 200;			/* 0-255 */
	PGPError err = kPGPError_NoErr;

	lzsContext = (PGPLZSContextRef) userValue;

	sourceCount = inputBufferSize;
	destCount = *outputBufferSize;
	bufferPtr = *outputBuffer;
	returnCode = 0;

	do
	{
		if (returnCode & LZS_DEST_EXHAUSTED)
		{
			err = PGPReallocData(lzsContext->memoryMgr,
					outputBuffer, (*outputBufferSize)*2, 0);

			if (IsntPGPError(err))
			{
				bufferPtr = &((*outputBuffer)[*outputBufferSize]);
				destCount = *outputBufferSize;
				(*outputBufferSize) *= 2;
			}
		}

		if (IsntPGPError(err))
			returnCode = LZS_Compress(lzsContext->context, &inputBuffer, 
							&bufferPtr, &sourceCount, &destCount, 
							lzsContext->history,
							(PGPUInt16) (LZS_SOURCE_FLUSH | perfMode), 
							perfParam);
	}
	while ((returnCode != LZS_INVALID) && !(returnCode & LZS_FLUSHED) && 
			IsntPGPError(err));
	
	if (returnCode == LZS_INVALID)
		err = kPGPError_CorruptData;

	if (IsntPGPError(err))
		*actualOutputSize = *outputBufferSize - destCount;

	return err;
}


PGPError pgpLZSContinueCompressProc(PGPUserValue userValue,
			PGPByte *inputBuffer, PGPSize inputBufferSize, 
			PGPByte *outputBuffer, PGPSize outputBufferSize,
			PGPSize *inputBytesUsed, PGPSize *outputBytesUsed)
{
	PGPLZSContextRef lzsContext;
	PGPUInt16 returnCode;
	PGPUInt32 sourceCount;
	PGPUInt32 destCount;
	PGPByte *bufferPtr;
	PGPUInt16 perfMode = 2 << 3;		/* 0-2 << 3 */
	PGPUInt16 perfParam = 200;			/* 0-255 */
	PGPError err = kPGPError_NoErr;

	lzsContext = (PGPLZSContextRef) userValue;

	if (outputBufferSize < LZS_DEST_MIN)
		return kPGPError_BufferTooSmall;

	sourceCount = inputBufferSize;
	destCount = outputBufferSize;
	bufferPtr = outputBuffer;

	returnCode = LZS_Compress(lzsContext->context, &inputBuffer, 
					&bufferPtr, &sourceCount, &destCount, 
					lzsContext->history, perfMode, perfParam);

	*inputBytesUsed = inputBufferSize - sourceCount;
	*outputBytesUsed = outputBufferSize - destCount;

	return err;
}


PGPError pgpLZSFinishCompressProc(PGPUserValue userValue,
			PGPByte *outputBuffer, PGPSize outputBufferSize,
			PGPSize *outputBytesUsed, PGPBoolean *moreOutputNeeded)
{
	PGPLZSContextRef lzsContext;
	PGPUInt16 returnCode;
	PGPUInt32 sourceCount;
	PGPUInt32 destCount;
	PGPByte *bufferPtr;
	PGPByte *nullPtr = NULL;
	PGPUInt16 perfMode = 2 << 3;		/* 0-2 << 3 */
	PGPUInt16 perfParam = 200;			/* 0-255 */
	PGPError err = kPGPError_NoErr;

	lzsContext = (PGPLZSContextRef) userValue;

	if (outputBufferSize < LZS_DEST_MIN)
		return kPGPError_BufferTooSmall;

	sourceCount = 0;
	destCount = outputBufferSize;
	bufferPtr = outputBuffer;

	returnCode = LZS_Compress(lzsContext->context, &nullPtr, 
					&bufferPtr, &sourceCount, &destCount, 
					lzsContext->history, 
					(PGPUInt16) (LZS_SOURCE_FLUSH | perfMode),
					perfParam);

	*outputBytesUsed = outputBufferSize - destCount;

	if (returnCode & LZS_DEST_EXHAUSTED)
		*moreOutputNeeded = TRUE;

	return err;
}


PGPError pgpCleanupLZSCompressProc(PGPUserValue *userValue)
{
	PGPError err = kPGPError_NoErr;

	if (IsNull(userValue))
		return kPGPError_BadParams;

	PGPFreeData(((PGPLZSContextRef) *userValue)->history);
	PGPFreeData(((PGPLZSContextRef) *userValue)->context);
	PGPFreeData(*userValue);

	*userValue = NULL;
	return err;
}


PGPError pgpInitLZSDecompressProc(PGPMemoryMgrRef memoryMgr,
			PGPUserValue *userValue)
{
	PGPLZSContextRef lzsContext;
	PGPError err = kPGPError_NoErr;

	if (IsNull(userValue))
		return kPGPError_BadParams;

	*userValue = NULL;

	lzsContext = (PGPLZSContextRef) PGPNewData(memoryMgr, 
										sizeof(PGPLZSContext),
										kPGPMemoryMgrFlags_Clear);

	if (IsNull(lzsContext))
		return kPGPError_OutOfMemory;

	lzsContext->memoryMgr = memoryMgr;
	
	lzsContext->context = (LZSContextRef) PGPNewData(memoryMgr, 
											LZS_GetContextSize(),
											kPGPMemoryMgrFlags_Clear);
	
	if (IsNull(lzsContext->context))
	{
		PGPFreeData(lzsContext);
		return kPGPError_OutOfMemory;
	}

	lzsContext->history = (PGPByte *) PGPNewData(memoryMgr, 
										LZS_HISTORY_SIZE,
										kPGPMemoryMgrFlags_Clear);
	
	if (IsNull(lzsContext->history))
	{
		PGPFreeData(lzsContext->context);
		PGPFreeData(lzsContext);
		return kPGPError_OutOfMemory;
	}

	LZS_InitHistory(lzsContext->context, lzsContext->history);

	*userValue = lzsContext;
	return err;
}


PGPError pgpLZSDecompressProc(PGPUserValue userValue,
			PGPByte *inputBuffer, PGPSize inputBufferSize, 
			PGPByte **outputBuffer, PGPSize *outputBufferSize,
			PGPSize *actualOutputSize)
{
	PGPLZSContextRef lzsContext;
	PGPUInt16 flags;
	PGPUInt16 returnCode;
	PGPUInt32 sourceCount;
	PGPUInt32 destCount;
	PGPByte *bufferPtr;
	PGPError err = kPGPError_NoErr;

	lzsContext = (PGPLZSContextRef) userValue;

	sourceCount = inputBufferSize;
	destCount = *outputBufferSize;
	bufferPtr = *outputBuffer;
	returnCode = 0;
	flags = LZS_RESET;

	do
	{
		if (returnCode & LZS_DEST_EXHAUSTED)
		{
			flags = 0;
			err = PGPReallocData(lzsContext->memoryMgr,
					outputBuffer, (*outputBufferSize)*2, 0);

			if (IsntPGPError(err))
			{
				bufferPtr = &((*outputBuffer)[*outputBufferSize]);
				destCount = *outputBufferSize;
				(*outputBufferSize) *= 2;
			}
		}

		if (IsntPGPError(err))
			returnCode = LZS_Decompress(lzsContext->context, &inputBuffer, 
							&bufferPtr, &sourceCount, &destCount, 
							lzsContext->history, flags);
	}
	while ((returnCode != LZS_INVALID) && !(returnCode & LZS_END_MARKER) && 
			IsntPGPError(err));

	if (returnCode == LZS_INVALID)
		err = kPGPError_CorruptData;

	if (IsntPGPError(err))
		*actualOutputSize = *outputBufferSize - destCount;

	return err;
}


PGPError pgpLZSContinueDecompressProc(PGPUserValue userValue,
			PGPByte *inputBuffer, PGPSize inputBufferSize, 
			PGPByte *outputBuffer, PGPSize outputBufferSize,
			PGPSize *inputBytesUsed, PGPSize *outputBytesUsed,
			PGPBoolean *finished)
{
	PGPLZSContextRef lzsContext;
	PGPUInt16 returnCode;
	PGPUInt32 sourceCount;
	PGPUInt32 destCount;
	PGPByte *bufferPtr;
	PGPError err = kPGPError_NoErr;

	lzsContext = (PGPLZSContextRef) userValue;

	sourceCount = inputBufferSize;
	destCount = outputBufferSize;
	bufferPtr = outputBuffer;

	returnCode = LZS_Decompress(lzsContext->context, &inputBuffer, 
					&bufferPtr, &sourceCount, &destCount, 
					lzsContext->history, 0);

	*inputBytesUsed = inputBufferSize - sourceCount;
	*outputBytesUsed = outputBufferSize - destCount;

	if ((returnCode & LZS_END_MARKER) && !(returnCode & LZS_DEST_EXHAUSTED))
		*finished = TRUE;

	return err;
}


PGPError pgpCleanupLZSDecompressProc(PGPUserValue *userValue)
{
	PGPError err = kPGPError_NoErr;

	if (IsNull(userValue))
		return kPGPError_BadParams;

	PGPFreeData(((PGPLZSContextRef) *userValue)->history);
	PGPFreeData(((PGPLZSContextRef) *userValue)->context);
	PGPFreeData(*userValue);

	*userValue = NULL;
	return err;
}



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
