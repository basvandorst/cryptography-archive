/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpIPsecComp.c,v 1.14 1999/05/24 20:21:56 dgal Exp $
____________________________________________________________________________*/

#ifndef USERLAND_TEST
#define USERLAND_TEST	0
#endif

#if USERLAND_TEST

#define DBG_FUNC(x)
#define DBG_ENTER()
#define DBG_PRINT(x)
#define DBG_LEAVE(x)

#else

#if PGP_WIN32

#include "vpn.h"
#include "vpndbg.h"
#include "ipdef.h"

#elif PGP_MACINTOSH

#define DBG_FUNC(x)
#define DBG_ENTER()
#define DBG_PRINT(x)
#define DBG_LEAVE(x)

#endif

#endif

#include "pgpIPsecComp.h"
#include "pgpIPheader.h"
#include "pgpIPsecErrors.h"
#include "pgpIPsecContextPriv.h"
#include "pgpEndianConversion.h"
#include "pgpMem.h"

#define COMP_PROTOCOL		108
#define COMP_HEADERSIZE		4

#define COMPHDR_NEXTHDR		0
#define COMPHDR_FLAGS		1
#define COMPHDR_CPI			2
#define COMPHDR_DATA		4

#define IPCOMP_DEFLATE		2
#define IPCOMP_LZS			3

#define MIN_PACKETSIZE		90


PGPError pgpIPsecCompress(PGPIPsecContextRef ipsec, PGPUInt32 maxBufferSize,
			PGPIPsecBuffer *ipPacketIn, PGPBoolean tunnelMode, 
			PGPUInt32 gatewayIP, PGPCompressionAlgorithm alg, 
			PGPIPsecBuffer *ipPacketOut, PGPBoolean *usedIPCOMP)
{
	PGPError err = kPGPError_NoErr;
	PGPUInt32 compIndex;
	PGPUInt32 inDataIndex;
	PGPUInt32 outDataIndex;
	PGPSize bytesIn;
	PGPSize bytesOut;
	PGPUInt32 total;
	PGPUInt32 originalSize;
	PGPUInt32 compressedSize;
	PGPUInt16 packetSize;
	PGPUInt16 cpi;
	PGPByte nextHeader;
	PGPBoolean moreOutput = FALSE;
	PGPIPsecBuffer *inPtr = NULL;
	PGPIPsecBuffer *outPtr = NULL;
	PGPIPsecBuffer *endPtr = NULL;
	PGPMemoryMgrRef memoryMgr = NULL;
	PGPCompContextRef compRef = NULL;

	DBG_FUNC("pgpIPsecCompress")

	DBG_ENTER();

	if (IsNull(ipsec)) {
		DBG_PRINT(("!!!!! pgpIPsecCompress error: %d !!!!!!!!!!!\n", kPGPError_BadParams););
		DBG_LEAVE(kPGPError_BadParams);
		return kPGPError_BadParams;
	}

	memoryMgr = ipsec->memory;
	*usedIPCOMP = FALSE;

	packetSize = pgpGetPacketSize(ipPacketIn);

	if (packetSize < MIN_PACKETSIZE)
	{
		PGPCopyIPsecBuffer(memoryMgr, ipPacketIn, 0, ipPacketOut, 0, 
			packetSize, maxBufferSize);

		return kPGPError_NoErr;
	}

	if (((PGPUInt32)(packetSize + 32) < maxBufferSize) || 
		(maxBufferSize == 0))
	{
		if (ipPacketOut->allocatedSize < (PGPUInt32) (packetSize + 32))
		{
			if (IsntNull(ipPacketOut->data))
				PGPFreeData(ipPacketOut->data);
			
			ipPacketOut->data = (PGPByte *) PGPNewData(memoryMgr,
												packetSize + 32,
												kPGPMemoryMgrFlags_Clear);
			
			ipPacketOut->allocatedSize = packetSize + 32;
			if (maxBufferSize == 0)
				maxBufferSize = packetSize + 32;
		}
	}
	else
	{
		if (ipPacketOut->allocatedSize < maxBufferSize)
		{
			if (IsntNull(ipPacketOut->data))
				PGPFreeData(ipPacketOut->data);
			
			ipPacketOut->data = (PGPByte *) PGPNewData(memoryMgr,
												maxBufferSize,
												kPGPMemoryMgrFlags_Clear);
			
			ipPacketOut->allocatedSize = maxBufferSize;
		}
	}
	
	if (IsNull(ipPacketOut->data))
		err = kPGPError_OutOfMemory;

	switch (alg)
	{
	case kPGPCompAlgorithm_Deflate:
		err = PGPNewCompContext(memoryMgr, alg, &compRef);
		cpi = IPCOMP_DEFLATE;
		break;

	case kPGPCompAlgorithm_LZS:
		err = PGPNewCompContext(memoryMgr, alg, &compRef);
		cpi = IPCOMP_LZS;
		break;

	case kPGPCompAlgorithm_None:
		PGPCopyIPsecBuffer(memoryMgr, ipPacketIn, 0, ipPacketOut, 0, 
			packetSize, maxBufferSize);
		return kPGPError_NoErr;

	default:
		err = kPGPError_BadParams;
		break;
	};

	if (IsntPGPError(err))
	{
		compIndex = pgpGetIPHeaderLength(ipPacketIn);

		if (tunnelMode)
			inDataIndex = 0;
		else
			inDataIndex = compIndex;

		inPtr = ipPacketIn;
		outPtr = ipPacketOut;
		outDataIndex = compIndex + COMP_HEADERSIZE;
		originalSize = packetSize - inDataIndex;
		compressedSize = 0;
		total = 0;

		while (IsntPGPError(err) && IsntNull(inPtr) && IsntNull(outPtr) &&
				(total < originalSize))
		{
			err = PGPContinueCompress(compRef, &(inPtr->data[inDataIndex]), 
					inPtr->dataSize - inDataIndex, 
					&(outPtr->data[outDataIndex]), 
					outPtr->allocatedSize - outDataIndex, &bytesIn, &bytesOut);

			if (IsntPGPError(err))
			{
				inDataIndex += bytesIn;
				total += bytesIn;
				outDataIndex += bytesOut;
				compressedSize += bytesOut;
				
				if ((compressedSize >= originalSize) && IsNull(endPtr))
					endPtr = outPtr;
				
				if (inDataIndex == inPtr->dataSize)
				{
					inPtr = inPtr->next;
					inDataIndex = 0;
				}
				
				if (outDataIndex == outPtr->allocatedSize)
				{
					if (outPtr->allocatedSize < maxBufferSize)
					{
						err = PGPReallocData(memoryMgr, &(outPtr->data), 
								maxBufferSize, 0);
						
						if (IsntPGPError(err))
							outPtr->allocatedSize = maxBufferSize;
					}
					else
					{
						outPtr->dataSize = outPtr->allocatedSize;
						outPtr = outPtr->next;
						outDataIndex = 0;
					}
				}
			}
		}

		moreOutput = TRUE;
		while (IsntPGPError(err) && moreOutput && IsntNull(outPtr))
		{
			err = PGPFinishCompress(compRef, &(outPtr->data[outDataIndex]),
					outPtr->allocatedSize - outDataIndex, &bytesOut,
					&moreOutput);

			if (IsntPGPError(err))
			{
				outDataIndex += bytesOut;
				compressedSize += bytesOut;
				
				if ((compressedSize >= originalSize) && IsNull(endPtr))
					endPtr = outPtr;
				
				if ((outDataIndex == outPtr->allocatedSize) && moreOutput)
				{
					if (outPtr->allocatedSize < maxBufferSize)
					{
						err = PGPReallocData(memoryMgr, &(outPtr->data), 
								maxBufferSize, 0);
						
						if (IsntPGPError(err))
							outPtr->allocatedSize = maxBufferSize;
					}
					else
					{
						outPtr->dataSize = outPtr->allocatedSize;
						outPtr = outPtr->next;
						outDataIndex = 0;
					}
				}
			}
		}

		if (moreOutput && IsntPGPError(err))
			err = kPGPError_BufferTooSmall;
	}
	
	if (IsntPGPError(err))
	{
		if (compressedSize >= originalSize)
		{
			endPtr->dataSize = 0;
			PGPCopyIPsecBuffer(memoryMgr, ipPacketIn, 0, ipPacketOut, 0, 
				packetSize, maxBufferSize);

			PGPFreeCompContext(compRef);
			return kPGPError_NoErr;
		}

		err = pgpAddProtocolToIPHeader(ipPacketIn, COMP_PROTOCOL, 
				ipPacketOut);
	}

	if (IsntPGPError(err))
	{
		pgpGetIPHeaderNextProtocol(ipPacketIn, tunnelMode, &nextHeader);

		ipPacketOut->data[compIndex + COMPHDR_NEXTHDR] = nextHeader;
		ipPacketOut->data[compIndex + COMPHDR_FLAGS] = 0;
		PGPUInt16ToEndian(cpi, kPGPNetworkByteOrder, 
			&(ipPacketOut->data[compIndex + COMPHDR_CPI]));

		outPtr->dataSize = outDataIndex;
		packetSize = compIndex + COMP_HEADERSIZE + compressedSize;

		if (tunnelMode)
			pgpSetDestIPAddress(ipPacketOut, gatewayIP);

		pgpSetPacketSize(packetSize, ipPacketOut);
		pgpSetIPHeaderChecksum(ipPacketOut);
		*usedIPCOMP = TRUE;
	} 

	if (IsntNull(compRef))
		PGPFreeCompContext(compRef);
		
	DBG_LEAVE(err);
	return err;
}


PGPError pgpIPsecDecompress(PGPIPsecContextRef ipsec, PGPUInt32 maxBufferSize, 
			PGPIPsecBuffer *ipPacketIn, PGPBoolean tunnelMode,
			PGPCompressionAlgorithm alg, PGPIPsecBuffer *ipPacketOut)
{
	PGPError err = kPGPError_NoErr;
	PGPUInt32 compIndex;
	PGPUInt32 inDataIndex;
	PGPUInt32 outDataIndex;
	PGPSize bytesIn;
	PGPSize bytesOut;
	PGPUInt32 decompressedSize;
	PGPUInt16 packetSize;
	PGPUInt16 cpi;
	PGPBoolean done = FALSE;
	PGPIPsecBuffer *inPtr = NULL;
	PGPIPsecBuffer *outPtr = NULL;
	PGPMemoryMgrRef memoryMgr = NULL;
	PGPCompContextRef compRef = NULL;

	DBG_FUNC("pgpIPsecDecompress")

	DBG_ENTER();

	if (IsNull(ipsec)) {
		DBG_LEAVE(kPGPError_BadParams);
		return kPGPError_BadParams;
	}

	memoryMgr = ipsec->memory;

	packetSize = pgpGetPacketSize(ipPacketIn);

	if (((PGPUInt32) (packetSize * 3) < maxBufferSize) || (maxBufferSize == 0))
	{
		if (ipPacketOut->allocatedSize < (PGPUInt32) (packetSize * 3))
		{
			if (IsntNull(ipPacketOut->data))
				PGPFreeData(ipPacketOut->data);
			
			ipPacketOut->data = (PGPByte *) PGPNewData(memoryMgr,
												packetSize * 3,
												kPGPMemoryMgrFlags_Clear);
			
			ipPacketOut->allocatedSize = packetSize * 3;

			if (maxBufferSize == 0)
				maxBufferSize = packetSize * 3;
		}
	}
	else
	{
		if (ipPacketOut->allocatedSize < maxBufferSize)
		{
			if (IsntNull(ipPacketOut->data))
				PGPFreeData(ipPacketOut->data);
			
			ipPacketOut->data = (PGPByte *) PGPNewData(memoryMgr,
												maxBufferSize,
												kPGPMemoryMgrFlags_Clear);
			
			ipPacketOut->allocatedSize = maxBufferSize;
		}
	}
	
	if (IsNull(ipPacketOut->data))
		err = kPGPError_OutOfMemory;

	switch (alg)
	{
	case kPGPCompAlgorithm_Deflate:
		err = PGPNewCompContext(memoryMgr, alg, &compRef);
		cpi = IPCOMP_DEFLATE;
		break;

	case kPGPCompAlgorithm_LZS:
		err = PGPNewCompContext(memoryMgr, alg, &compRef);
		cpi = IPCOMP_LZS;
		break;

	case kPGPCompAlgorithm_None:
		PGPCopyIPsecBuffer(memoryMgr, ipPacketIn, 0, ipPacketOut, 0, 
			packetSize, maxBufferSize);
		return kPGPError_NoErr;

	default:
		err = kPGPError_BadParams;
		break;
	};

	if (IsntPGPError(err))
	{
		err = pgpFindProtocolHeader(ipPacketIn, COMP_PROTOCOL, 
				&compIndex);
		
		if (err == kPGPIPsecError_NoIPsecHeaderFound)
		{
			PGPCopyIPsecBuffer(memoryMgr, ipPacketIn, 0, ipPacketOut, 0, 
				packetSize, maxBufferSize);

			if (IsntNull(compRef))
				PGPFreeCompContext(compRef);

			return kPGPError_NoErr;
		}
	}
	
	if (IsntPGPError(err))
	{
		if (tunnelMode)
			outDataIndex = 0;
		else
			outDataIndex = compIndex;

		decompressedSize = 0;
		inPtr = ipPacketIn;
		inDataIndex = compIndex + COMP_HEADERSIZE;
		outPtr = ipPacketOut;

		while (IsntPGPError(err) && IsntNull(inPtr) && IsntNull(outPtr)
				&& !done)
		{
			err = PGPContinueDecompress(compRef, &(inPtr->data[inDataIndex]), 
					inPtr->dataSize - inDataIndex, 
					&(outPtr->data[outDataIndex]), 
					outPtr->allocatedSize - outDataIndex, &bytesIn, &bytesOut,
					&done);

			if (IsntPGPError(err))
			{
				inDataIndex += bytesIn;
				outDataIndex += bytesOut;
				decompressedSize += bytesOut;
				
				if (inDataIndex == inPtr->dataSize)
				{
					inPtr = inPtr->next;
					inDataIndex = 0;
				}
				
				if (outDataIndex == outPtr->allocatedSize)
				{
					if (outPtr->allocatedSize < maxBufferSize)
					{
						err = PGPReallocData(memoryMgr, &(outPtr->data), 
								maxBufferSize, 0);
						
						if (IsntPGPError(err))
							outPtr->allocatedSize = maxBufferSize;
					}
					else
					{
						outPtr->dataSize = outPtr->allocatedSize;
						outPtr = outPtr->next;
						outDataIndex = 0;
					}
				}
			}
		}

		if (!done && IsntPGPError(err))
			err = kPGPError_BufferTooSmall;
	}
	
	if (IsntPGPError(err))
	{
		if (tunnelMode)
			packetSize = decompressedSize;
		else
		{
			packetSize = compIndex + decompressedSize;
			err = pgpRemoveProtocolFromIPHeader(ipPacketIn, COMP_PROTOCOL, 
					ipPacketIn->data[compIndex + COMPHDR_NEXTHDR], 
					ipPacketOut);
		}
	}
	
	if (IsntPGPError(err))
	{
		outPtr->dataSize = outDataIndex;
		pgpSetPacketSize(packetSize, ipPacketOut);
		pgpSetIPHeaderChecksum(ipPacketOut);
	}

	if (IsntNull(compRef))
		PGPFreeCompContext(compRef);

	DBG_LEAVE(err);
	return err;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
