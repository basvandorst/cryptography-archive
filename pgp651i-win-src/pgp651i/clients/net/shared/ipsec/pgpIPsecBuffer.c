/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.

	$Id: pgpIPsecBuffer.c,v 1.2 1999/03/19 22:38:51 dgal Exp $
____________________________________________________________________________*/
#include "pgpIPsecBuffer.h"
#include "pgpMem.h"

PGPError PGPCopyIPsecBuffer(PGPMemoryMgrRef memoryMgr,
			PGPIPsecBuffer *inBuffer, PGPUInt32 inStart,
			PGPIPsecBuffer *outBuffer, PGPUInt32 outStart,
			PGPUInt32 numBytes, PGPUInt32 maxBufferSize)
{
	PGPUInt32 inDataIndex = 0;
	PGPUInt32 outDataIndex = 0;
	PGPUInt32 total = 0;
	PGPUInt32 prevTotal = 0;
	PGPUInt32 inCopy;
	PGPUInt32 outCopy;
	PGPIPsecBuffer *inPtr = NULL;
	PGPIPsecBuffer *outPtr = NULL;
	PGPError err = kPGPError_NoErr;

	if (IsNull(inBuffer) || IsNull(outBuffer))
		return kPGPError_BadParams;

	if (IsNull(inBuffer->data) || IsNull(outBuffer->data))
		return kPGPError_BadParams;

	inPtr = inBuffer;
	outPtr = outBuffer;
	outDataIndex = outStart;

	total = inPtr->dataSize;
	while ((total < inStart) && IsntNull(inPtr))
	{
		inPtr = inPtr->next;
		prevTotal = total;
		total += inPtr->dataSize;
	}

	inDataIndex = inStart - prevTotal;

	prevTotal = 0;
	total = outPtr->allocatedSize;
	while ((total < outStart) && IsntPGPError(err))
	{
		if (IsNull(outPtr->next))
		{
			if (maxBufferSize > 0)
				err = PGPExpandIPsecBuffer(memoryMgr, outPtr, maxBufferSize, 
						maxBufferSize);
			else
				err = kPGPError_BufferTooSmall;

			if (IsntPGPError(err))
				total = prevTotal + outPtr->allocatedSize;
		}
		else
		{
			outPtr = outPtr->next;
			prevTotal = total;
			total += outPtr->allocatedSize;
		}
	}

	outDataIndex = outStart - prevTotal;

	if (&(inPtr->data[inDataIndex]) == &(outPtr->data[outDataIndex]))
		return err;

	while ((numBytes > 0) && IsntNull(inPtr) && IsntPGPError(err))
	{
		inCopy = inPtr->dataSize - inDataIndex;
		outCopy = outPtr->allocatedSize - outDataIndex;

		if (numBytes < inCopy)
			inCopy = numBytes;
		if (numBytes < outCopy)
			outCopy = numBytes;

		if (inCopy <= outCopy)
		{
			pgpCopyMemory(&(inPtr->data[inDataIndex]), 
				&(outPtr->data[outDataIndex]), inCopy);

			inPtr = inPtr->next;
			inDataIndex = 0;
			outDataIndex += inCopy;
			numBytes -= inCopy;
			if (outPtr->dataSize < outDataIndex)
				outPtr->dataSize = outDataIndex;
		}
		else
		{
			if (IsNull(outPtr->next))
			{
				if (maxBufferSize > 0)
					err = PGPExpandIPsecBuffer(memoryMgr, outPtr, 
							maxBufferSize, maxBufferSize);
				else
					err = kPGPError_BufferTooSmall;
			}
			else
			{
				pgpCopyMemory(&(inPtr->data[inDataIndex]), 
					&(outPtr->data[outDataIndex]), outCopy);

				outPtr->dataSize = outPtr->allocatedSize;
				outPtr = outPtr->next;
				outDataIndex = 0;
				inDataIndex += outCopy;
				numBytes -= outCopy;
			}
		}
	}

	if (IsntPGPError(err) && (numBytes > 0))
		err = kPGPError_BufferTooSmall;

	return err;
}


PGPError PGPExpandIPsecBuffer(PGPMemoryMgrRef memoryMgr, 
			PGPIPsecBuffer *buffer, PGPUInt32 maxBufferSize, 
			PGPUInt32 expandSize)
{
	PGPError err = kPGPError_NoErr;
	
	if ((buffer->allocatedSize < maxBufferSize) && 
		(maxBufferSize >= expandSize))
	{
		err = PGPReallocData(memoryMgr, &(buffer->data), maxBufferSize, 0);
		
		if (IsntPGPError(err))
			buffer->allocatedSize = maxBufferSize;
	}
	else if (maxBufferSize == 0)
	{
		err = PGPReallocData(memoryMgr, &(buffer->data), expandSize, 0);
		
		if (IsntPGPError(err))
			buffer->allocatedSize = expandSize;
	}
	else
	{
		if (IsNull(buffer->next))
		{
			buffer->next = (PGPIPsecBuffer *) PGPNewData(memoryMgr,
												sizeof(PGPIPsecBuffer),
												kPGPMemoryMgrFlags_Clear);
			
			if (IsNull(buffer->next))
				err = kPGPError_OutOfMemory;
			else
			{
				buffer->next->data = (PGPByte *) PGPNewData(memoryMgr,
													maxBufferSize, 
													kPGPMemoryMgrFlags_Clear);
				
				if (IsNull(buffer->next->data))
					err = kPGPError_OutOfMemory;
			}
			
			if (IsntPGPError(err))
				buffer->next->allocatedSize = maxBufferSize;
		}
	}
	
	return err;
}


PGPBoolean PGPIPsecBufferDataIsEqual(PGPIPsecBuffer *inBuffer, 
			PGPUInt32 inStart, PGPIPsecBuffer *outBuffer, PGPUInt32 outStart,
			PGPUInt32 numBytes)
{
	PGPUInt32 inDataIndex = 0;
	PGPUInt32 outDataIndex = 0;
	PGPUInt32 total = 0;
	PGPUInt32 prevTotal = 0;
	PGPUInt32 inCompare;
	PGPUInt32 outCompare;
	PGPIPsecBuffer *inPtr = NULL;
	PGPIPsecBuffer *outPtr = NULL;

	if (IsNull(inBuffer) || IsNull(outBuffer))
		return FALSE;

	if (IsNull(inBuffer->data) || IsNull(outBuffer->data))
		return FALSE;

	inPtr = inBuffer;
	outPtr = outBuffer;

	total = inPtr->dataSize;
	while ((total < inStart) && IsntNull(inPtr))
	{
		inPtr = inPtr->next;
		prevTotal = total;
		total += inPtr->dataSize;
	}

	inDataIndex = inStart - prevTotal;

	prevTotal = 0;
	total = outPtr->dataSize;
	while ((total < outStart) && IsntNull(outPtr))
	{
		outPtr = outPtr->next;
		prevTotal = total;
		total += outPtr->dataSize;
	}

	outDataIndex = outStart - prevTotal;

	if (&(inPtr->data[inDataIndex]) == &(outPtr->data[outDataIndex]))
		return TRUE;

	while ((numBytes > 0) && IsntNull(inPtr) && IsntNull(outPtr))
	{
		inCompare = inPtr->dataSize - inDataIndex;
		outCompare = outPtr->dataSize - outDataIndex;

		if (numBytes < inCompare)
			inCompare = numBytes;
		if (numBytes < outCompare)
			outCompare = numBytes;

		if (inCompare <= outCompare)
		{
			if (!pgpMemoryEqual(&(inPtr->data[inDataIndex]), 
					&(outPtr->data[outDataIndex]), inCompare))
				return FALSE;

			inPtr = inPtr->next;
			inDataIndex = 0;
			outDataIndex += inCompare;
			numBytes -= inCompare;
		}
		else
		{
			if (!pgpMemoryEqual(&(inPtr->data[inDataIndex]), 
					&(outPtr->data[outDataIndex]), outCompare))
				return FALSE;

			outPtr = outPtr->next;
			outDataIndex = 0;
			inDataIndex += outCompare;
			numBytes -= outCompare;
		}
	}

	if (numBytes > 0)
		return FALSE;
	else
		return TRUE;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
