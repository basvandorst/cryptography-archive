/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpIPsecAH.c,v 1.29 1999/05/24 20:21:56 dgal Exp $
____________________________________________________________________________*/

#include "pgpIPsecAH.h"
#include "pgpIPheader.h"
#include "pgpIPsecErrors.h"
#include "pgpIPsecContextPriv.h"
#include "pgpHMAC.h"
#include "pgpEndianConversion.h"
#include "pgpMem.h"

#define AH_PROTOCOL		51
#define AH_HEADERSIZE	12

#define AHHDR_NEXTHEADER	0
#define AHHDR_LENGTH		1
#define AHHDR_SPI			4
#define AHHDR_SEQNUMBER		8
#define AHHDR_DATA			12

#define MAX_HASH_SIZE		32
#define TRUNCATED_HASH_SIZE	12


PGPError pgpIPsecAHGetSPI(PGPIPsecBuffer *inPacket, PGPUInt32 *spi)
{
	PGPError err = kPGPError_NoErr;
	PGPIPsecBuffer tempPacket;
	PGPByte data[AH_HEADERSIZE];
	PGPUInt32 ahIndex;

	*spi = 0;
	
	tempPacket.data = data;
	tempPacket.dataSize = 0;
	tempPacket.allocatedSize = AH_HEADERSIZE;
	tempPacket.next = NULL;

	err = pgpFindProtocolHeader(inPacket, AH_PROTOCOL, &ahIndex);

	if (IsntPGPError(err))
	{
		PGPCopyIPsecBuffer(NULL, inPacket, ahIndex, &tempPacket, 0, 
			AH_HEADERSIZE, AH_HEADERSIZE);

		*spi = PGPEndianToUInt32(kPGPNetworkByteOrder, &(data[AHHDR_SPI]));
	}

	return err;
}


PGPError pgpApplyIPsecAuthentication(PGPIPsecContextRef ipsec,
			PGPUInt32 maxBufferSize, PGPIPsecBuffer *ipPacketIn, 
			PGPBoolean tunnelMode, PGPUInt32 gatewayIP, PGPHashAlgorithm hash,
			PGPByte *authKey, PGPUInt32 spi, PGPUInt32 sequenceNumber, 
			PGPIPsecBuffer *ipPacketOut)
{
	PGPError err = kPGPError_NoErr;
	PGPByte	ipHeader[IPHDR_MAXSIZE];
	PGPByte hashData[MAX_HASH_SIZE];
	PGPByte headerData[AH_HEADERSIZE + MAX_HASH_SIZE];
	PGPByte ahHeaderSize = AH_HEADERSIZE;
	PGPUInt32 ahIndex;
	PGPUInt32 index;
	PGPUInt32 total;
	PGPUInt16 packetSize;
	PGPByte nextHeader;
	PGPByte hashSize;
	PGPByte authKeySize;
	PGPIPsecBuffer *outPtr;
	PGPIPsecBuffer tempBuffer;
	PGPMemoryMgrRef memoryMgr = NULL;
	PGPHMACContextRef hmacRef = NULL;

	if (IsNull(ipsec))
		return kPGPError_BadParams;

	memoryMgr = ipsec->memory;

	packetSize = pgpGetPacketSize(ipPacketIn);

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

	switch (hash)
	{
	case kPGPHashAlgorithm_MD5:
		hashSize = 16;
		authKeySize = 16;
		break;

	case kPGPHashAlgorithm_SHA:
		hashSize = 20;
		authKeySize = 20;
		break;

	case kPGPHashAlgorithm_RIPEMD160:
		hashSize = 20;
		authKeySize = 20;
		break;

	default:
		err = kPGPError_BadHashNumber;
		break;
	};

	if (hashSize > TRUNCATED_HASH_SIZE)
		hashSize = TRUNCATED_HASH_SIZE;

	ahHeaderSize += hashSize;
	packetSize += ahHeaderSize;

	if (IsntPGPError(err))
	{
		pgpGetIPHeaderNextProtocol(ipPacketIn, tunnelMode, &nextHeader);

		ahIndex = pgpGetIPHeaderLength(ipPacketIn);
		err = pgpAddProtocolToIPHeader(ipPacketIn, AH_PROTOCOL, 
				ipPacketOut);

		if (tunnelMode)
			packetSize += ahIndex;
	}

	if (IsntPGPError(err))
	{
		if (tunnelMode)
		{
			PGPCopyIPsecBuffer(memoryMgr, ipPacketIn, 0, ipPacketOut, 
				ahIndex + ahHeaderSize, packetSize - ahIndex - ahHeaderSize,
				maxBufferSize);
			
			pgpSetDestIPAddress(ipPacketOut, gatewayIP);
		}
		else
			PGPCopyIPsecBuffer(memoryMgr, ipPacketIn, ahIndex, ipPacketOut,
				ahIndex + ahHeaderSize, packetSize - ahIndex - ahHeaderSize,
				maxBufferSize);

		pgpSetPacketSize(packetSize, ipPacketOut);
		err = pgpClearMutableIPHeaderFields(ipPacketOut, ipHeader);
	}

	if (IsntPGPError(err))
	{
		tempBuffer.data = headerData;
		tempBuffer.dataSize = AH_HEADERSIZE + hashSize;
		tempBuffer.allocatedSize = AH_HEADERSIZE + hashSize;
		tempBuffer.next = NULL;

		headerData[AHHDR_NEXTHEADER] = nextHeader;
		headerData[AHHDR_LENGTH] = (ahHeaderSize / 4) - 2;
		PGPUInt32ToEndian(spi, kPGPNetworkByteOrder, 
			&(headerData[AHHDR_SPI]));
		PGPUInt32ToEndian(sequenceNumber, kPGPNetworkByteOrder, 
			&(headerData[AHHDR_SEQNUMBER]));

		for (index=AHHDR_DATA; index<(PGPUInt32)(AHHDR_DATA+hashSize); index++)
			headerData[index] = 0;

		PGPCopyIPsecBuffer(memoryMgr, &tempBuffer, 0, ipPacketOut, ahIndex, 
			AH_HEADERSIZE + hashSize, AH_HEADERSIZE + hashSize);

		err = PGPNewHMACContext(memoryMgr, hash, authKey, authKeySize, 
				&hmacRef);
	}

	if (IsntPGPError(err))
	{
		outPtr = ipPacketOut;
		total = outPtr->dataSize;

		err = PGPContinueHMAC(hmacRef, outPtr->data, total);

		while (IsntPGPError(err) && (total < packetSize)) 
		{
			outPtr = outPtr->next;
			total += outPtr->dataSize;

			err = PGPContinueHMAC(hmacRef, outPtr->data, outPtr->dataSize);
		}
	}

	if (IsntPGPError(err))
		err = PGPFinalizeHMAC(hmacRef, hashData);

	if (IsntPGPError(err))
	{
		tempBuffer.data = hashData;
		tempBuffer.dataSize = hashSize;
		tempBuffer.allocatedSize = hashSize;
		tempBuffer.next = NULL;

		PGPCopyIPsecBuffer(memoryMgr, &tempBuffer, 0, ipPacketOut, 
			ahIndex + AHHDR_DATA, hashSize, maxBufferSize);

		err = pgpRestoreIPHeader(ipHeader, ipPacketOut);
	}

	if (IsntPGPError(err))
	{
		pgpSetPacketSize(packetSize, ipPacketOut);
		pgpSetIPHeaderChecksum(ipPacketOut);
	}

	if (IsntNull(hmacRef))
		PGPFreeHMACContext(hmacRef);

	return err;
}


PGPError pgpVerifyIPsecAuthentication(PGPIPsecContextRef ipsec,
			PGPUInt32 maxBufferSize, PGPIPsecBuffer *ipPacketIn, 
			PGPBoolean tunnelMode, PGPHashAlgorithm hash, PGPByte *authKey, 
			PGPUInt32 *sequenceWindow, PGPUInt32 *upperSequence, 
			PGPUInt32 *lowerSequence, PGPIPsecBuffer *ipPacketOut)
{
	PGPError err = kPGPError_NoErr;
	PGPByte	ipHeader[IPHDR_MAXSIZE];
	PGPByte hashData[MAX_HASH_SIZE];
	PGPByte receivedData[MAX_HASH_SIZE];
	PGPByte headerData[AH_HEADERSIZE + MAX_HASH_SIZE];
	PGPByte ahHeaderSize = AH_HEADERSIZE;
	PGPUInt32 ahIndex;
	PGPUInt32 index;
	PGPUInt32 total;
	PGPUInt32 sequenceNumber;
	PGPUInt32 sequenceBit;
	PGPUInt16 packetSize;
	PGPByte hashSize;
	PGPByte authKeySize;
	PGPByte nextHeader;
	PGPIPsecBuffer *inPtr;
	PGPIPsecBuffer tempBuffer;
	PGPMemoryMgrRef memoryMgr = NULL;
	PGPHMACContextRef hmacRef = NULL;

	if (IsNull(ipsec))
		return kPGPError_BadParams;

	memoryMgr = ipsec->memory;

	packetSize = pgpGetPacketSize(ipPacketIn);

	if ((packetSize < maxBufferSize) || (maxBufferSize == 0))
	{
		if (ipPacketOut->allocatedSize < packetSize)
		{
			if (IsntNull(ipPacketOut->data))
				PGPFreeData(ipPacketOut->data);
			
			ipPacketOut->data = (PGPByte *) PGPNewData(memoryMgr,
												packetSize,
												kPGPMemoryMgrFlags_Clear);
			
			ipPacketOut->allocatedSize = packetSize;

			if (maxBufferSize == 0)
				maxBufferSize = packetSize;
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

	switch (hash)
	{
	case kPGPHashAlgorithm_MD5:
		hashSize = 16;
		authKeySize = 16;
		break;

	case kPGPHashAlgorithm_SHA:
		hashSize = 20;
		authKeySize = 20;
		break;

	case kPGPHashAlgorithm_RIPEMD160:
		hashSize = 20;
		authKeySize = 20;
		break;

	default:
		err = kPGPError_BadHashNumber;
		break;
	};

	if (hashSize > TRUNCATED_HASH_SIZE)
		hashSize = TRUNCATED_HASH_SIZE;

	ahHeaderSize += hashSize;

	if (IsntPGPError(err))
		err = pgpFindProtocolHeader(ipPacketIn, AH_PROTOCOL, &ahIndex);

	if (IsntPGPError(err))
	{
		tempBuffer.data = headerData;
		tempBuffer.dataSize = AH_HEADERSIZE + hashSize;
		tempBuffer.allocatedSize = AH_HEADERSIZE + hashSize;
		tempBuffer.next = NULL;

		PGPCopyIPsecBuffer(memoryMgr, ipPacketIn, ahIndex, &tempBuffer, 0,
			AH_HEADERSIZE + hashSize, AH_HEADERSIZE + hashSize);

		nextHeader = headerData[AHHDR_NEXTHEADER];

		if (*upperSequence < ((sizeof(*sequenceWindow) << 3) - 1))
			*upperSequence = (sizeof(*sequenceWindow) << 3) - 1;

		*lowerSequence = *upperSequence - (sizeof(sequenceWindow) << 3)
							+ 1;

		sequenceNumber = PGPEndianToUInt32(kPGPNetworkByteOrder, 
							&(headerData[AHHDR_SEQNUMBER]));

		if (sequenceNumber < *lowerSequence)
			err = kPGPIPsecError_PacketReceivedTooLate;
		else if (sequenceNumber <= *upperSequence)
		{
			sequenceBit = 0x00000001 << (sequenceNumber - *lowerSequence);
			if (sequenceBit & *sequenceWindow)
				err = kPGPIPsecError_PacketAlreadyReceived;
		}
	}

	if (IsntPGPError(err))
		err = pgpClearMutableIPHeaderFields(ipPacketIn, ipHeader);

	if (IsntPGPError(err))
	{
		pgpCopyMemory(&(headerData[AHHDR_DATA]), receivedData, hashSize);

		for (index=AHHDR_DATA; index<(PGPUInt32)(AHHDR_DATA+hashSize); index++)
			headerData[index] = 0;

		PGPCopyIPsecBuffer(memoryMgr, &tempBuffer, 0, ipPacketIn, ahIndex,
			AH_HEADERSIZE + hashSize, AH_HEADERSIZE + hashSize);

		err = PGPNewHMACContext(memoryMgr, hash, authKey, authKeySize, 
				&hmacRef);

		if (IsPGPError(err))
		{
			pgpRestoreIPHeader(ipHeader, ipPacketIn);			
			
			pgpCopyMemory(receivedData, &(headerData[AHHDR_DATA]), hashSize);
			PGPCopyIPsecBuffer(memoryMgr, &tempBuffer, 0, ipPacketIn, ahIndex,
				AH_HEADERSIZE + hashSize, AH_HEADERSIZE + hashSize);
		}
	}

	if (IsntPGPError(err))
	{
		inPtr = ipPacketIn;
		total = inPtr->dataSize;

		err = PGPContinueHMAC(hmacRef, inPtr->data, total);
		pgpCopyMemory(receivedData, &(headerData[AHHDR_DATA]), hashSize);
		PGPCopyIPsecBuffer(memoryMgr, &tempBuffer, 0, ipPacketIn, ahIndex,
			AH_HEADERSIZE + hashSize, AH_HEADERSIZE + hashSize);

		while (IsntPGPError(err) && (total < packetSize)) 
		{
			inPtr = inPtr->next;
			total += inPtr->dataSize;

			err = PGPContinueHMAC(hmacRef, inPtr->data, inPtr->dataSize);
		}
	}

	if (IsntPGPError(err))
		err = PGPFinalizeHMAC(hmacRef, hashData);

	if (IsntPGPError(err))
	{
		if (!pgpMemoryEqual(receivedData, hashData, hashSize))
		{
			err = kPGPIPsecError_PacketAuthenticationFailed;
		}
	}

	if (IsntPGPError(err))
	{
		if (sequenceNumber > *upperSequence)
		{
			PGPUInt32 sequenceMask = 0x7fffffff;

			sequenceMask >>= sequenceNumber - *upperSequence - 1;
			*sequenceWindow >>= sequenceNumber - *upperSequence;
			*sequenceWindow &= sequenceMask;

			*upperSequence = sequenceNumber;
			*lowerSequence = *upperSequence - (sizeof(sequenceWindow) << 3)
								+ 1;
		}

		sequenceBit = 0x00000001 << (sequenceNumber - *lowerSequence);
		*sequenceWindow |= sequenceBit;
	}

	if (IsntPGPError(err))
		err = pgpRestoreIPHeader(ipHeader, ipPacketIn);

	if (IsntPGPError(err))
	{
		if ((nextHeader == IP_IN_IP) && !tunnelMode)
			tunnelMode = TRUE;

		if (tunnelMode)
		{
			PGPCopyIPsecBuffer(memoryMgr, ipPacketIn, ahIndex + ahHeaderSize, 
				ipPacketOut, 0, packetSize - ahIndex - ahHeaderSize,
				maxBufferSize);

			packetSize -= ahIndex;
		}
		else
			PGPCopyIPsecBuffer(memoryMgr, ipPacketIn, ahIndex + ahHeaderSize, 
				ipPacketOut, ahIndex, packetSize - ahIndex - ahHeaderSize,
				maxBufferSize);

		packetSize -= ahHeaderSize;
	}

	if (IsntPGPError(err))
		if (!tunnelMode)
			err = pgpRemoveProtocolFromIPHeader(ipPacketIn, AH_PROTOCOL, 
					ipPacketIn->data[ahIndex], ipPacketOut);

	if (IsntPGPError(err))
	{
		pgpSetPacketSize(packetSize, ipPacketOut);
		pgpSetIPHeaderChecksum(ipPacketOut);
	}

	if (IsntNull(hmacRef))
		PGPFreeHMACContext(hmacRef);

	return err;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/