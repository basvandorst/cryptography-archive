/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpCompDeflate.c,v 1.8 1999/03/20 22:44:16 dgal Exp $
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

	streamPtr = (z_streamp) userValue;
	
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

	deflateReset(streamPtr);
	return err;
}


PGPError pgpDeflateContinueCompressProc(PGPUserValue userValue,
			PGPByte *inputBuffer, PGPSize inputBufferSize, 
			PGPByte *outputBuffer, PGPSize outputBufferSize,
			PGPSize *inputBytesUsed, PGPSize *outputBytesUsed)
{
	z_streamp streamPtr = NULL;
	PGPUInt32 zErr = Z_OK;
	PGPError err = kPGPError_NoErr;

	streamPtr = (z_streamp) userValue;
	
	streamPtr->next_in = inputBuffer;
	streamPtr->avail_in = inputBufferSize;
	streamPtr->next_out = outputBuffer;
	streamPtr->avail_out = outputBufferSize;

	zErr = deflate(streamPtr, Z_NO_FLUSH);

	*inputBytesUsed = inputBufferSize - streamPtr->avail_in;
	*outputBytesUsed = outputBufferSize - streamPtr->avail_out;

	switch (zErr)
	{
	case Z_OK:
		err = kPGPError_NoErr;
		break;

	case Z_STREAM_ERROR:
	case Z_BUF_ERROR:
		err = kPGPError_BufferTooSmall;
		break;

	case Z_MEM_ERROR:
		err = kPGPError_OutOfMemory;
		break;
	}

	return err;
}


PGPError pgpDeflateFinishCompressProc(PGPUserValue userValue,
			PGPByte *outputBuffer, PGPSize outputBufferSize,
			PGPSize *outputBytesUsed, PGPBoolean *moreOutputNeeded)
{
	z_streamp streamPtr = NULL;
	PGPUInt32 zErr = Z_OK;
	PGPError err = kPGPError_NoErr;

	streamPtr = (z_streamp) userValue;
	
	streamPtr->next_in = NULL;
	streamPtr->avail_in = 0;
	streamPtr->next_out = outputBuffer;
	streamPtr->avail_out = outputBufferSize;

	zErr = deflate(streamPtr, Z_FINISH);

	*outputBytesUsed = outputBufferSize - streamPtr->avail_out;

	switch (zErr)
	{
	case Z_OK:
		err = kPGPError_NoErr;
		*moreOutputNeeded = TRUE;
		break;

	case Z_STREAM_END:
		err = kPGPError_NoErr;
		deflateReset(streamPtr);
		break;

	case Z_STREAM_ERROR:
	case Z_BUF_ERROR:
		err = kPGPError_BufferTooSmall;
		break;

	case Z_MEM_ERROR:
		err = kPGPError_OutOfMemory;
		break;
	}

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

	streamPtr = (z_streamp) userValue;

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

	inflateReset(streamPtr);
	return err;
}


PGPError pgpDeflateContinueDecompressProc(PGPUserValue userValue,
			PGPByte *inputBuffer, PGPSize inputBufferSize, 
			PGPByte *outputBuffer, PGPSize outputBufferSize,
			PGPSize *inputBytesUsed, PGPSize *outputBytesUsed,
			PGPBoolean *finished)
{
	z_streamp streamPtr = NULL;
	PGPUInt32 zErr = Z_OK;
	PGPError err = kPGPError_NoErr;

	streamPtr = (z_streamp) userValue;
	
	streamPtr->next_in = inputBuffer;
	streamPtr->avail_in = inputBufferSize;
	streamPtr->next_out = outputBuffer;
	streamPtr->avail_out = outputBufferSize;

	zErr = inflate(streamPtr, Z_NO_FLUSH);

	*inputBytesUsed = inputBufferSize - streamPtr->avail_in;
	*outputBytesUsed = outputBufferSize - streamPtr->avail_out;

	switch (zErr)
	{
	case Z_OK:
		err = kPGPError_NoErr;
		break;

	case Z_STREAM_END:
		err = kPGPError_NoErr;
		*finished = TRUE;
		inflateReset(streamPtr);
		break;

	case Z_STREAM_ERROR:
	case Z_BUF_ERROR:
		err = kPGPError_BufferTooSmall;
		break;

	case Z_MEM_ERROR:
		err = kPGPError_OutOfMemory;
		break;

	case Z_DATA_ERROR:
		err = kPGPError_CorruptData;
		break;
	}

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
