/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpCompDeflate.c,v 1.2 1999/03/10 02:49:05 heller Exp $
____________________________________________________________________________*/

#include "pgpCompDeflate.h"
#include "pgpMem.h"
#include "zlib.h"

static void *sDeflateAlloc(PGPMemoryMgrRef memoryMgr, PGPUInt32 numItems, 
				PGPUInt32 itemSize);

static void sDeflateFree(PGPMemoryMgrRef memoryMgr, void *address);


PGPError pgpInitDeflateCompressProc(PGPMemoryMgrRef memoryMgr,
			PGPUserValue *userValue)
{
	z_streamp streamPtr = NULL;
	PGPError err = kPGPError_NoErr;

	if (IsNull(userValue))
		return kPGPError_BadParams;

	*userValue = NULL;

	streamPtr = (z_streamp) PGPNewData(memoryMgr, sizeof(z_stream),
								kPGPMemoryMgrFlags_Clear);

	if (IsNull(streamPtr))
		return kPGPError_OutOfMemory;

	streamPtr->zalloc = (alloc_func) sDeflateAlloc;
	streamPtr->zfree = (free_func) sDeflateFree;
	streamPtr->opaque = memoryMgr;

	switch (deflateInit(streamPtr, Z_BEST_SPEED))
	{
	case Z_OK:
		break;

	case Z_MEM_ERROR:
		err = kPGPError_OutOfMemory;
		PGPFreeData(streamPtr);
		streamPtr = NULL;
		break;
	}

	*userValue = streamPtr;
	return err;
}


PGPError pgpDeflateCompressProc(PGPUserValue userValue,
			PGPByte *inputBuffer, PGPSize inputBufferSize, 
			PGPByte **outputBuffer, PGPSize *outputBufferSize,
			PGPSize *actualOutputSize)
{
	z_streamp streamPtr = NULL;
	PGPUInt32 zErr = Z_OK;
	PGPError err = kPGPError_NoErr;

	if (IsNull(inputBuffer) || IsNull(outputBuffer) || 
		IsNull(actualOutputSize) || IsNull(userValue) || 
		IsNull(outputBufferSize))
		return kPGPError_BadParams;

	*actualOutputSize = 0;

	if (!inputBufferSize || !(*outputBufferSize) || IsNull(*outputBuffer))
		return kPGPError_BadParams;

	streamPtr = (z_streamp) userValue;
	
	deflateReset(streamPtr);

	streamPtr->next_in = inputBuffer;
	streamPtr->avail_in = inputBufferSize;
	streamPtr->next_out = *outputBuffer;
	streamPtr->avail_out = *outputBufferSize;

	while (streamPtr->avail_in && (zErr == Z_OK) && IsntPGPError(err))
	{
		zErr = deflate(streamPtr, Z_NO_FLUSH);

		if ((zErr == Z_OK) && (!streamPtr->avail_out))
		{
			err = PGPReallocData((PGPMemoryMgrRef) streamPtr->opaque,
					outputBuffer, (*outputBufferSize)*2, 0);

			if (IsntPGPError(err))
			{
				streamPtr->next_out = &((*outputBuffer)[*outputBufferSize]);
				streamPtr->avail_out = *outputBufferSize;
				(*outputBufferSize) *= 2;
			}
		}
	}

	if ((zErr != Z_STREAM_END) && IsntPGPError(err))
	{
		if (zErr != Z_OK)
			err = kPGPError_CorruptData;

		while ((zErr == Z_OK) && IsntPGPError(err))
		{
			zErr = deflate(streamPtr, Z_FINISH);

			if (zErr == Z_OK)
			{
				err = PGPReallocData((PGPMemoryMgrRef) streamPtr->opaque,
						outputBuffer, (*outputBufferSize)*2, 0);
				
				if (IsntPGPError(err))
				{
					streamPtr->next_out = 
						&((*outputBuffer)[*outputBufferSize]) - 
						streamPtr->avail_out;

					streamPtr->avail_out += *outputBufferSize;
					(*outputBufferSize) *= 2;
				}
			}
		}
	}

	if ((zErr != Z_STREAM_END) && IsntPGPError(err))
		err = kPGPError_CorruptData;

	if (IsntPGPError(err))
		*actualOutputSize = streamPtr->total_out;

	return err;
}


PGPError pgpCleanupDeflateCompressProc(PGPUserValue *userValue)
{
	PGPError err = kPGPError_NoErr;

	if (IsNull(userValue))
		return kPGPError_BadParams;

	deflateEnd((z_streamp) *userValue);
	PGPFreeData(*userValue);
	*userValue = NULL;

	return err;
}


PGPError pgpInitDeflateDecompressProc(PGPMemoryMgrRef memoryMgr,
			PGPUserValue *userValue)
{
	z_streamp streamPtr = NULL;
	PGPError err = kPGPError_NoErr;

	if (IsNull(userValue))
		return kPGPError_BadParams;

	*userValue = NULL;

	streamPtr = (z_streamp) PGPNewData(memoryMgr, sizeof(z_stream),
								kPGPMemoryMgrFlags_Clear);

	if (IsNull(streamPtr))
		return kPGPError_OutOfMemory;

	streamPtr->zalloc = (alloc_func) sDeflateAlloc;
	streamPtr->zfree = (free_func) sDeflateFree;
	streamPtr->opaque = memoryMgr;

	switch (inflateInit(streamPtr))
	{
	case Z_OK:
		break;

	case Z_MEM_ERROR:
		err = kPGPError_OutOfMemory;
		PGPFreeData(streamPtr);
		streamPtr = NULL;
		break;
	}

	*userValue = streamPtr;
	return err;
}


PGPError pgpDeflateDecompressProc(PGPUserValue userValue,
			PGPByte *inputBuffer, PGPSize inputBufferSize, 
			PGPByte **outputBuffer, PGPSize *outputBufferSize,
			PGPSize *actualOutputSize)
{
	z_streamp streamPtr = NULL;
	PGPUInt32 zErr = Z_OK;
	PGPError err = kPGPError_NoErr;

	if (IsNull(inputBuffer) || IsNull(outputBuffer) || 
		IsNull(actualOutputSize) || IsNull(userValue) || 
		IsNull(outputBufferSize))
		return kPGPError_BadParams;

	*actualOutputSize = 0;

	if (!inputBufferSize || !(*outputBufferSize) || IsNull(*outputBuffer))
		return kPGPError_BadParams;

	streamPtr = (z_streamp) userValue;

	inflateReset(streamPtr);
	
	streamPtr->next_in = inputBuffer;
	streamPtr->avail_in = inputBufferSize;
	streamPtr->next_out = *outputBuffer;
	streamPtr->avail_out = *outputBufferSize;

	while (streamPtr->avail_in && (zErr == Z_OK) && IsntPGPError(err))
	{
		zErr = inflate(streamPtr, Z_NO_FLUSH);

		if ((zErr == Z_OK) && (!streamPtr->avail_out))
		{
			err = PGPReallocData((PGPMemoryMgrRef) streamPtr->opaque,
					outputBuffer, (*outputBufferSize)*2, 0);

			if (IsntPGPError(err))
			{
				streamPtr->next_out = &((*outputBuffer)[*outputBufferSize]);
				streamPtr->avail_out = *outputBufferSize;
				(*outputBufferSize) *= 2;
			}
		}
	}

	if ((zErr != Z_STREAM_END) && IsntPGPError(err))
	{
		if (zErr != Z_OK)
		{
			if (zErr == Z_MEM_ERROR)
				err = kPGPError_OutOfMemory;
			else
				err = kPGPError_CorruptData;
		}

		while ((zErr == Z_OK) && IsntPGPError(err))
		{
			zErr = inflate(streamPtr, Z_FINISH);

			if (zErr == Z_OK)
			{
				err = PGPReallocData((PGPMemoryMgrRef) streamPtr->opaque,
						outputBuffer, (*outputBufferSize)*2, 0);
				
				if (IsntPGPError(err))
				{
					streamPtr->next_out = 
						&((*outputBuffer)[*outputBufferSize]) - 
						streamPtr->avail_out;

					streamPtr->avail_out += *outputBufferSize;
					(*outputBufferSize) *= 2;
				}
			}
		}
	}

	if ((zErr != Z_STREAM_END) && IsntPGPError(err))
		err = kPGPError_CorruptData;

	if (IsntPGPError(err))
		*actualOutputSize = streamPtr->total_out;

	return err;
}


PGPError pgpCleanupDeflateDecompressProc(PGPUserValue *userValue)
{
	PGPError err = kPGPError_NoErr;

	if (IsNull(userValue))
		return kPGPError_BadParams;

	inflateEnd((z_streamp) *userValue);
	PGPFreeData(*userValue);
	*userValue = NULL;

	return err;
}


static void *sDeflateAlloc(PGPMemoryMgrRef memoryMgr, PGPUInt32 numItems, 
				PGPUInt32 itemSize)
{
	void *address;

	address = PGPNewData(memoryMgr, numItems * itemSize, 
					kPGPMemoryMgrFlags_Clear);

	if (IsNull(address))
		return Z_NULL;

	return address;
}


static void sDeflateFree(PGPMemoryMgrRef memoryMgr, void *address)
{
	(void) memoryMgr;
	
	if (IsntNull(address))
		PGPFreeData(address);

	return;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/