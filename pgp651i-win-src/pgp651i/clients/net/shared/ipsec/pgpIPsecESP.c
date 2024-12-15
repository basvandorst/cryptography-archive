/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpIPsecESP.c,v 1.47.4.1 1999/06/17 15:28:47 dgal Exp $
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

#include "pgpIPsecESP.h"
#include "pgpIPheader.h"
#include "pgpIPsecErrors.h"
#include "pgpIPsecContextPriv.h"
#include "pgpHMAC.h"
#include "pgpEndianConversion.h"
#include "pgpMem.h"

#define ESP_PROTOCOL	50
#define ESP_HEADERSIZE	8

#define ESPHDR_SPI			0
#define ESPHDR_SEQNUMBER	4
#define ESPHDR_DATA			8

#define MAXHASHSIZE	32
#define TRUNCATED_HASH_SIZE	12

#define MAX_SLOP_BLOCK	16
#define MAX_PADDING		24

static PGPByte nullIV[] = {0,0,0,0,0,0,0,0};


PGPError pgpIPsecESPGetSPI(PGPIPsecBuffer *inPacket, PGPUInt32 *spi)
{
	PGPUInt32 espIndex;
	PGPIPsecBuffer tempPacket;
	PGPByte data[ESP_HEADERSIZE];
	PGPError err = kPGPError_NoErr;

	*spi = 0;
	
	tempPacket.data = data;
	tempPacket.dataSize = 0;
	tempPacket.allocatedSize = ESP_HEADERSIZE;
	tempPacket.next = NULL;

	err = pgpFindProtocolHeader(inPacket, ESP_PROTOCOL, &espIndex);

	if (IsntPGPError(err))
	{
		PGPCopyIPsecBuffer(NULL, inPacket, espIndex, &tempPacket, 0, 
			ESP_HEADERSIZE, ESP_HEADERSIZE);

		*spi = PGPEndianToUInt32(kPGPNetworkByteOrder, &(data[ESPHDR_SPI]));
	}

	return err;
}


PGPError pgpIPsecEncrypt(PGPIPsecContextRef ipsec, PGPUInt32 maxBufferSize,
			PGPIPsecBuffer *ipPacketIn, PGPBoolean tunnelMode, 
			PGPUInt32 gatewayIP, PGPCipherAlgorithm cipher, 
			const PGPByte *cryptKey, PGPByte *iv, PGPHashAlgorithm hash,
			PGPByte *authKey, PGPUInt32 spi, PGPUInt32 sequenceNumber, 
			PGPIPsecBuffer *ipPacketOut)
{
	PGPUInt32 espIndex;
	PGPUInt32 index;
	PGPUInt32 padStart;
	PGPUInt16 packetSize;
	PGPUInt32 inputStart;
	PGPUInt32 inDataIndex;
	PGPUInt32 outDataIndex;
	PGPUInt32 inSlopIndex;
	PGPUInt32 outSlopIndex;
	PGPUInt32 chunkSize;
	PGPSize total;
	PGPSize prevTotal;
	PGPByte nextHeader;
	PGPByte padLength;
	PGPSize keySize;
	PGPSize encryptSize;
	PGPSize blockSize;
	PGPSize inputSize;
	PGPByte hashSize;
	PGPByte authKeySize;
	PGPByte inSlopBlock[MAX_SLOP_BLOCK];
	PGPByte outSlopBlock[MAX_SLOP_BLOCK];
	PGPByte padding[MAX_PADDING];
	PGPByte authBuffer[MAXHASHSIZE];
	PGPByte headerData[ESP_HEADERSIZE + MAXHASHSIZE];
	PGPByte *inSlopPtr = NULL;
	PGPByte *outSlopPtr = NULL;
	PGPIPsecBuffer tempBuffer;
	PGPIPsecBuffer *inPtr = NULL;
	PGPIPsecBuffer *outPtr = NULL;
	PGPMemoryMgrRef memoryMgr = NULL;
	PGPHMACContextRef hmacRef = NULL;
	PGPSymmetricCipherContextRef cipherRef = NULL;
	PGPCBCContextRef cbcRef = NULL;
	PGPError err = kPGPError_NoErr;

	DBG_FUNC("pgpIPsecEncrypt")

	DBG_ENTER();

	if (IsNull(ipsec)) {
		DBG_PRINT(("!!!!! pgpIPsecEncrypt error: %d !!!!!!!!!!!\n", kPGPError_BadParams););
		DBG_LEAVE(kPGPError_BadParams);
		return kPGPError_BadParams;
	}
	
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

	switch (cipher)
	{
	case kPGPCipherAlgorithm_CAST5:
		keySize = 16;
		blockSize = 8;
		err = PGPNewSymmetricCipherContext(memoryMgr, 
				kPGPCipherAlgorithm_CAST5, 16, &cipherRef);

		if (IsntPGPError(err))
			err = PGPNewCBCContext(cipherRef, &cbcRef);
		break;

	case kPGPCipherAlgorithm_3DES:
		keySize = 24;
		blockSize = 8;
		err = PGPNewSymmetricCipherContext(memoryMgr, 
				kPGPCipherAlgorithm_3DES, 24, &cipherRef);

		if (IsntPGPError(err))
			err = PGPNewCBCContext(cipherRef, &cbcRef);
		break;

	case kPGPCipherAlgorithm_None:
		keySize = 0;
		blockSize = 0;
		break;

	default:
		err = kPGPError_BadCipherNumber;
		break;
	};

	if (IsntPGPError(err)) 
	{
		pgpGetIPHeaderNextProtocol(ipPacketIn, tunnelMode, &nextHeader);

		espIndex = pgpGetIPHeaderLength(ipPacketIn);
		err = pgpAddProtocolToIPHeader(ipPacketIn, ESP_PROTOCOL, ipPacketOut);

		if (tunnelMode)
			packetSize += espIndex;
	}

	if (IsntPGPError(err))
	{
		if ((maxBufferSize > 0) && 
			(maxBufferSize <= (espIndex + ESP_HEADERSIZE + blockSize)))
			err = kPGPError_BufferTooSmall;

		encryptSize = packetSize - espIndex;

		if (tunnelMode)
			inputStart = 0;
		else
			inputStart = espIndex;

		inputSize = encryptSize;
		outDataIndex = espIndex + ESPHDR_DATA + blockSize;
	}

	if (cipher != kPGPCipherAlgorithm_None)
	{
		if (IsntPGPError(err))
		{
			if (IsntNull(iv))
				err = PGPInitCBC(cbcRef, cryptKey, iv);
			else
				err = PGPInitCBC(cbcRef, cryptKey, nullIV);
		}
		
		if (IsntPGPError(err))
		{
			inPtr = ipPacketIn;
			outPtr = ipPacketOut;
			total = inPtr->dataSize;
			prevTotal = 0;
			inSlopIndex = 0;
			outSlopIndex = 0;
			
			while ((total < inputStart) && IsntNull(inPtr->next))
			{
				prevTotal = total;
				inPtr = inPtr->next;
				total += inPtr->dataSize;
			}
			
			inDataIndex = inputStart - prevTotal;
			
			if (total < inputStart)
				err = kPGPError_BufferTooSmall;
		}
		
		while (IsntPGPError(err) && (inputSize >= blockSize) &&
			IsntNull(inPtr))
		{
			inSlopIndex = 0;
			outSlopIndex = 0;
			
			if (((inPtr->dataSize - inDataIndex) >= blockSize) &&
				((outPtr->allocatedSize - outDataIndex) >= blockSize))
			{
				if ((inPtr->dataSize - inDataIndex) <= 
					(outPtr->allocatedSize - outDataIndex))
				{
					chunkSize = (inPtr->dataSize - inDataIndex) -
								((inPtr->dataSize - inDataIndex) % blockSize);
				}
				else
				{
					chunkSize = (outPtr->allocatedSize - outDataIndex) -
								((outPtr->allocatedSize - outDataIndex) % 
									blockSize);
				}
				
				err = PGPCBCEncrypt(cbcRef, &(inPtr->data[inDataIndex]), 
						chunkSize, &(outPtr->data[outDataIndex]));
				
				outDataIndex += chunkSize;
				inDataIndex += chunkSize;
				inputSize -= chunkSize;

				if (inDataIndex == inPtr->dataSize)
				{
					inPtr = inPtr->next;
					inDataIndex = 0;
				}

				if (outDataIndex == outPtr->allocatedSize)
				{
					outPtr->dataSize = outPtr->allocatedSize;
					outPtr = outPtr->next;
					outDataIndex = 0;
				}
			}
			else
			{
				if ((inPtr->dataSize - inDataIndex) < blockSize)
				{
					inSlopPtr = inSlopBlock;
					
					while ((inputSize >= (blockSize - inSlopIndex) ) && 
							(inSlopIndex < blockSize) && IsntNull(inPtr))
					{
						total = inPtr->dataSize - inDataIndex;
						if (total > (blockSize - inSlopIndex))
							total = blockSize - inSlopIndex;
						
						pgpCopyMemory(&(inPtr->data[inDataIndex]),
							&(inSlopBlock[inSlopIndex]), total);
						
						inSlopIndex += total;
						inDataIndex += total;
						inputSize -= total;
						
						if (inDataIndex == inPtr->dataSize)
						{
							inPtr = inPtr->next;
							inDataIndex = 0;
						}
					}
				}
				else
				{
					inSlopPtr = &(inPtr->data[inDataIndex]);
					inSlopIndex = blockSize;
					inDataIndex += blockSize;
					inputSize -= blockSize;
				}
				
				if ((outPtr->allocatedSize - outDataIndex) < blockSize)
				{
					err = PGPExpandIPsecBuffer(memoryMgr, outPtr, 
							maxBufferSize, blockSize + outDataIndex);
					
					if ((outPtr->allocatedSize - outDataIndex) < blockSize)
						outSlopPtr = outSlopBlock;
					else
						outSlopPtr = &(outPtr->data[outDataIndex]);
				}
				else
					outSlopPtr = &(outPtr->data[outDataIndex]);
				
				if (IsntPGPError(err) && (inSlopIndex == blockSize))
				{
					inSlopIndex = 0;
					err = PGPCBCEncrypt(cbcRef, inSlopPtr, blockSize, 
							outSlopPtr);
					
					if (IsntPGPError(err) && (outSlopPtr == outSlopBlock))
					{
						outSlopIndex = outPtr->allocatedSize - outDataIndex;
						
						pgpCopyMemory(outSlopBlock, 
							&(outPtr->data[outDataIndex]), outSlopIndex);
						
						outPtr->dataSize = outPtr->allocatedSize;
						outPtr = outPtr->next;
						
						pgpCopyMemory(&(outSlopBlock[outSlopIndex]),
							outPtr->data, blockSize - outSlopIndex);
						
						outDataIndex = blockSize - outSlopIndex;
					}
					else
						outDataIndex += blockSize;
				}
			}
		}
		
		if (IsntPGPError(err))
		{
			if (inputSize)
			{
				if ((inPtr->dataSize - inDataIndex) < inputSize)
				{
					pgpCopyMemory(&(inPtr->data[inDataIndex]),
						&(inSlopBlock[inSlopIndex]), 
						inPtr->dataSize - inDataIndex);

					inSlopIndex += inPtr->dataSize - inDataIndex;
					inputSize -= inPtr->dataSize - inDataIndex;
					inPtr = inPtr->next;

					pgpCopyMemory(inPtr->data, &(inSlopBlock[inSlopIndex]), 
						inputSize);
				}
				else
					pgpCopyMemory(&(inPtr->data[inDataIndex]),
						&(inSlopBlock[inSlopIndex]), inputSize);
				
				inSlopIndex += inputSize;
			}
			
			if ((outPtr->allocatedSize - outDataIndex) < blockSize)
			{
				err = PGPExpandIPsecBuffer(memoryMgr, outPtr, 
						maxBufferSize, blockSize + outDataIndex);
					
				if ((outPtr->allocatedSize - outDataIndex) < blockSize)
					outSlopPtr = outSlopBlock;
				else
					outSlopPtr = &(outPtr->data[outDataIndex]);
			}
			else
				outSlopPtr = &(outPtr->data[outDataIndex]);
				
			if (inSlopIndex < 7)
			{
				padLength = blockSize - inSlopIndex - 2;
				
				if (padLength)
					for (index=0; index<padLength; index++)
						inSlopBlock[index+inSlopIndex] = index + 1;
					
					inSlopBlock[inSlopIndex + padLength] = padLength;
					inSlopBlock[inSlopIndex + padLength + 1] = nextHeader;
					
					err = PGPCBCEncrypt(cbcRef, inSlopBlock, blockSize, 
							outSlopPtr);
					
					if (IsntPGPError(err) && (outSlopPtr == outSlopBlock))
					{
						outSlopIndex = outPtr->allocatedSize - outDataIndex;
						
						pgpCopyMemory(outSlopBlock, 
							&(outPtr->data[outDataIndex]), outSlopIndex);
						
						outPtr->dataSize = outPtr->allocatedSize;
						outPtr = outPtr->next;
						
						pgpCopyMemory(&(outSlopBlock[outSlopIndex]),
							outPtr->data, blockSize - outSlopIndex);
						
						outDataIndex = blockSize - outSlopIndex;
					}
					else
						outDataIndex += blockSize;
			}
			else
			{
				inSlopBlock[7] = 1;
				err = PGPCBCEncrypt(cbcRef, inSlopBlock, blockSize, 
						outSlopPtr);
				
				if (IsntPGPError(err) && (outSlopPtr == outSlopBlock))
				{
					outSlopIndex = outPtr->allocatedSize - outDataIndex;
					
					pgpCopyMemory(outSlopBlock, 
						&(outPtr->data[outDataIndex]), outSlopIndex);
					
					outPtr->dataSize = outPtr->allocatedSize;
					outPtr = outPtr->next;
					
					pgpCopyMemory(&(outSlopBlock[outSlopIndex]),
						outPtr->data, blockSize - outSlopIndex);
					
					outDataIndex = blockSize - outSlopIndex;
				}
				else
					outDataIndex += blockSize;
				
				if ((outPtr->allocatedSize - outDataIndex) < blockSize)
				{
					err = PGPExpandIPsecBuffer(memoryMgr, outPtr, 
							maxBufferSize, blockSize + outDataIndex);
					
					if ((outPtr->allocatedSize - outDataIndex) < blockSize)
						outSlopPtr = outSlopBlock;
					else
						outSlopPtr = &(outPtr->data[outDataIndex]);
				}
				else
					outSlopPtr = &(outPtr->data[outDataIndex]);
				
				if (IsntPGPError(err))
				{
					for (index=0; index<6; index++)
						inSlopBlock[index] = index + 2;
					
					padLength = 7;
					inSlopBlock[6] = padLength;
					inSlopBlock[7] = nextHeader;
					
					err = PGPCBCEncrypt(cbcRef, inSlopBlock, blockSize, 
							outSlopPtr);
				}
				
				if (IsntPGPError(err) && (outSlopPtr == outSlopBlock))
				{
					outSlopIndex = outPtr->allocatedSize - outDataIndex;
					
					pgpCopyMemory(outSlopBlock, 
						&(outPtr->data[outDataIndex]), outSlopIndex);
					
					outPtr->dataSize = outPtr->allocatedSize;
					outPtr = outPtr->next;
					
					pgpCopyMemory(&(outSlopBlock[outSlopIndex]),
						outPtr->data, blockSize - outSlopIndex);
					
					outDataIndex = blockSize - outSlopIndex;
				}
				else
					outDataIndex += blockSize;
			}
		}

		if (IsntPGPError(err))
		{
			packetSize += padLength + 2 + ESP_HEADERSIZE + blockSize;
			outPtr->dataSize = outDataIndex;
		}
	}
	else
	{
		if (tunnelMode)
			PGPCopyIPsecBuffer(memoryMgr, ipPacketIn, 0, ipPacketOut, 
				espIndex + ESP_HEADERSIZE + blockSize, packetSize - espIndex,
				maxBufferSize);
		else
			PGPCopyIPsecBuffer(memoryMgr, ipPacketIn, espIndex, ipPacketOut, 
				espIndex + ESP_HEADERSIZE + blockSize, packetSize - espIndex,
				maxBufferSize);
		
		padStart = espIndex + ESPHDR_DATA + blockSize + encryptSize;

		if (blockSize > 0)
		{
			padLength = blockSize - ((encryptSize + 2) % blockSize);
			if (padLength == 8)
				padLength = 0;
		}
		else
		{
			padLength = 4 - ((encryptSize + 2) % 4);
			if (padLength == 4)
				padLength = 0;
		}
		
		if (padLength)
		{
			for (index=0; index<padLength; index++)
				padding[index] = index + 1;
		}
		
		padding[padLength] = padLength;
		padding[padLength + 1] = nextHeader;

		tempBuffer.data = padding;
		tempBuffer.dataSize = padLength + 2;
		tempBuffer.allocatedSize = padLength + 2;
		tempBuffer.next = NULL;

		PGPCopyIPsecBuffer(memoryMgr, &tempBuffer, 0, ipPacketOut, padStart, 
			padLength + 2, maxBufferSize);

		packetSize += padLength + 2 + ESP_HEADERSIZE + blockSize;
	}

	if (IsntPGPError(err))
	{
		tempBuffer.data = headerData;
		tempBuffer.dataSize = ESP_HEADERSIZE + blockSize;
		tempBuffer.allocatedSize = ESP_HEADERSIZE + blockSize;
		tempBuffer.next = NULL;

		PGPUInt32ToEndian(spi, kPGPNetworkByteOrder, 
			&(headerData[ESPHDR_SPI]));
		PGPUInt32ToEndian(sequenceNumber, kPGPNetworkByteOrder, 
			&(headerData[ESPHDR_SEQNUMBER]));

		if (IsntNull(iv))
		{
			pgpCopyMemory(iv, &(headerData[ESPHDR_DATA]), blockSize);

			if (IsntNull(outSlopPtr))
				pgpCopyMemory(outSlopPtr, iv, blockSize);
		}
		else
			pgpCopyMemory(nullIV, &(headerData[ESPHDR_DATA]), blockSize);
			
		PGPCopyIPsecBuffer(memoryMgr, &tempBuffer, 0, ipPacketOut, espIndex, 
			ESP_HEADERSIZE + blockSize, ESP_HEADERSIZE + blockSize);

		err = pgpAddProtocolToIPHeader(ipPacketIn, ESP_PROTOCOL, 
				ipPacketOut);

		if (tunnelMode)
			pgpSetDestIPAddress(ipPacketOut, gatewayIP);

		pgpSetPacketSize(packetSize, ipPacketOut);
	}

	if (IsntPGPError(err) && (hash != kPGPHashAlgorithm_Invalid))
	{
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
		};

		if (hashSize > TRUNCATED_HASH_SIZE)
			hashSize = TRUNCATED_HASH_SIZE;

		err = PGPNewHMACContext(memoryMgr, hash, authKey, authKeySize, 
				&hmacRef);

		outPtr = ipPacketOut;
		outDataIndex = espIndex;
		total = outPtr->dataSize - outDataIndex;

		err = PGPContinueHMAC(hmacRef, &(outPtr->data[outDataIndex]), total);

		while (IsntPGPError(err) && (total < (packetSize - espIndex))) 
		{
			outPtr = outPtr->next;
			total += outPtr->dataSize;

			err = PGPContinueHMAC(hmacRef, outPtr->data, outPtr->dataSize);
		}

		if (IsntPGPError(err)) 
			err = PGPFinalizeHMAC(hmacRef, authBuffer);

		if (IsntPGPError(err))
		{
			tempBuffer.data = authBuffer;
			tempBuffer.dataSize = hashSize;
			tempBuffer.allocatedSize = hashSize;
			tempBuffer.next = NULL;

			PGPCopyIPsecBuffer(memoryMgr, &tempBuffer, 0, outPtr, 
				outPtr->dataSize, hashSize, maxBufferSize);

			packetSize += hashSize;
			pgpSetPacketSize(packetSize, ipPacketOut);
		} 
	}

	if (IsntPGPError(err))
		pgpSetIPHeaderChecksum(ipPacketOut);

	if (IsntNull(hmacRef))
		PGPFreeHMACContext(hmacRef);

	if (IsntNull(cbcRef))
		PGPFreeCBCContext(cbcRef);

	DBG_LEAVE(err);
	return err;
}


PGPError pgpIPsecDecrypt(PGPIPsecContextRef ipsec, PGPUInt32 maxBufferSize,
			PGPIPsecBuffer *ipPacketIn, PGPBoolean tunnelMode, 
			PGPCipherAlgorithm cipher, const PGPByte *cryptKey, 
			PGPHashAlgorithm hash, PGPByte *authKey, 
			PGPUInt32 *sequenceWindow, PGPUInt32 *upperSequence, 
			PGPUInt32 *lowerSequence, PGPIPsecBuffer *ipPacketOut)
{
	PGPUInt32 espIndex;
	PGPUInt32 sequenceNumber;
	PGPUInt32 sequenceBit;
	PGPUInt16 packetSize;
	PGPUInt32 inDataIndex;
	PGPUInt32 outDataIndex;
	PGPUInt32 inSlopIndex;
	PGPUInt32 outSlopIndex;
	PGPUInt32 chunkSize;
	PGPUInt32 inputStart;
	PGPSize total;
	PGPSize prevTotal;
	PGPSize keySize;
	PGPSize blockSize;
	PGPSize inputSize;
	PGPByte padLength;
	PGPByte nextHeader;
	PGPByte hashSize;
	PGPByte authKeySize;
	PGPByte *iv = NULL;
	PGPByte hashData[MAXHASHSIZE];
	PGPByte inSlopBlock[MAX_SLOP_BLOCK];
	PGPByte outSlopBlock[MAX_SLOP_BLOCK];
	PGPByte headerData[ESP_HEADERSIZE + MAXHASHSIZE];
	PGPByte *inSlopPtr = NULL;
	PGPByte *outSlopPtr = NULL;
	PGPIPsecBuffer tempBuffer;
	PGPIPsecBuffer *inPtr = NULL;
	PGPIPsecBuffer *outPtr = NULL;
	PGPIPsecBuffer *tempPtr = NULL;
	PGPMemoryMgrRef memoryMgr = NULL;
	PGPHMACContextRef hmacRef = NULL;
	PGPSymmetricCipherContextRef cipherRef = NULL;
	PGPCBCContextRef cbcRef = NULL;
	PGPError err = kPGPError_NoErr;

	DBG_FUNC("pgpIPsecDecrypt")

	DBG_ENTER();

	if (IsNull(ipsec)) {
		DBG_LEAVE(kPGPError_BadParams);
		return kPGPError_BadParams;
	}

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

	switch (cipher)
	{
	case kPGPCipherAlgorithm_CAST5:
		keySize = 16;
		blockSize = 8;
		err = PGPNewSymmetricCipherContext(memoryMgr, 
				kPGPCipherAlgorithm_CAST5, 16, &cipherRef);

		if (IsntPGPError(err))
			err = PGPNewCBCContext(cipherRef, &cbcRef);
		break;

	case kPGPCipherAlgorithm_3DES:
		keySize = 24;
		blockSize = 8;
		err = PGPNewSymmetricCipherContext(memoryMgr, 
				kPGPCipherAlgorithm_3DES, 24, &cipherRef);

		if (IsntPGPError(err))
			err = PGPNewCBCContext(cipherRef, &cbcRef);
		break;

	case kPGPCipherAlgorithm_None:
		keySize = 0;
		blockSize = 0;
		break;

	default:
		err = kPGPError_BadCipherNumber;
		break;
	};

	if (IsntPGPError(err))
		err = pgpFindProtocolHeader(ipPacketIn, ESP_PROTOCOL, &espIndex);

	if (IsntPGPError(err))
	{
		tempBuffer.data = headerData;
		tempBuffer.dataSize = ESP_HEADERSIZE + blockSize;
		tempBuffer.allocatedSize = ESP_HEADERSIZE + blockSize;
		tempBuffer.next = NULL;

		PGPCopyIPsecBuffer(memoryMgr, ipPacketIn, espIndex, &tempBuffer, 0,
			ESP_HEADERSIZE + blockSize, ESP_HEADERSIZE + blockSize);

		if (hash != kPGPHashAlgorithm_Invalid)
		{
			if (*upperSequence < ((sizeof(*sequenceWindow) << 3) - 1))
				*upperSequence = (sizeof(*sequenceWindow) << 3) - 1;

			*lowerSequence = *upperSequence - (sizeof(sequenceWindow) << 3)
								+ 1;

			sequenceNumber = PGPEndianToUInt32(kPGPNetworkByteOrder, 
								&(headerData[ESPHDR_SEQNUMBER]));

			if (sequenceNumber < *lowerSequence)
				err = kPGPIPsecError_PacketReceivedTooLate;
			else if (sequenceNumber <= *upperSequence)
			{
				sequenceBit = 0x00000001 << (sequenceNumber - *lowerSequence);
				if (sequenceBit & *sequenceWindow)
					err = kPGPIPsecError_PacketAlreadyReceived;
			}
		}
	}

	if (IsntPGPError(err))
	{
		if ((maxBufferSize > 0) && 
			(maxBufferSize < (espIndex + ESP_HEADERSIZE + blockSize)))
			err = kPGPError_BufferTooSmall;
	}

	if (IsntPGPError(err) && (hash != kPGPHashAlgorithm_Invalid))
	{
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
		};

		if (hashSize > TRUNCATED_HASH_SIZE)
			hashSize = TRUNCATED_HASH_SIZE;

		err = PGPNewHMACContext(memoryMgr, hash, authKey, authKeySize, 
				&hmacRef);
		
		inPtr = ipPacketIn;
		inDataIndex = espIndex;
		if (inPtr->dataSize > (PGPUInt32) (packetSize - hashSize))
			total = packetSize - espIndex - hashSize;
		else
			total = inPtr->dataSize - inDataIndex;

		err = PGPContinueHMAC(hmacRef, &(inPtr->data[inDataIndex]), total);
		inDataIndex += total;

		while (IsntPGPError(err) && 
				(total < (packetSize - espIndex - hashSize))) 
		{
			inPtr = inPtr->next;
			prevTotal = total;
			total += inPtr->dataSize;

			if (total <= (packetSize - espIndex - hashSize))
				inDataIndex = inPtr->dataSize;
			else
				inDataIndex = packetSize - espIndex - hashSize - prevTotal;

			err = PGPContinueHMAC(hmacRef, inPtr->data, inDataIndex);
		}

		if (IsntPGPError(err)) 
			err = PGPFinalizeHMAC(hmacRef, hashData);

		if (IsntPGPError(err))
		{
			tempBuffer.data = hashData;
			tempBuffer.dataSize = hashSize;
			tempBuffer.allocatedSize = hashSize;
			tempBuffer.next = NULL;

			if (!PGPIPsecBufferDataIsEqual(&tempBuffer, 0, ipPacketIn,
					packetSize - hashSize, hashSize))
			{
				err = kPGPIPsecError_PacketAuthenticationFailed;
			}
			else
			{
				packetSize -= hashSize;
				
				if ((inDataIndex + hashSize) > inPtr->dataSize)
					inPtr->next->dataSize = 0;

				inPtr->dataSize = inDataIndex;
			}
		}
	}

	if (cipher != kPGPCipherAlgorithm_None)
	{
		if (IsntPGPError(err))
		{
			iv = (PGPByte *) PGPNewData(memoryMgr, blockSize, 
								kPGPMemoryMgrFlags_Clear);
			
			if (IsNull(iv))
				err = kPGPError_OutOfMemory;
		}
		
		if (IsntPGPError(err))
		{
			pgpCopyMemory(&(headerData[ESPHDR_DATA]), iv, blockSize);
			err = PGPInitCBC(cbcRef, cryptKey, iv);
		}
		
		if (IsntPGPError(err))
		{
			inputStart = espIndex + ESPHDR_DATA + blockSize;
			inputSize = packetSize - inputStart;

			if (tunnelMode)
				outDataIndex = 0;
			else
				outDataIndex = espIndex;

			inPtr = ipPacketIn;
			outPtr = ipPacketOut;
			total = inPtr->dataSize;
			prevTotal = 0;
			inSlopIndex = 0;
			outSlopIndex = 0;
			
			while ((total < inputStart) && IsntNull(inPtr->next))
			{
				prevTotal = total;
				inPtr = inPtr->next;
				total += inPtr->dataSize;
			}
			
			inDataIndex = inputStart - prevTotal;
			
			if (total < inputStart)
				err = kPGPError_BufferTooSmall;
		}
		
		while (IsntPGPError(err) && (inputSize >= blockSize) &&
			IsntNull(inPtr))
		{
			inSlopIndex = 0;
			outSlopIndex = 0;
			
			if (((inPtr->dataSize - inDataIndex) >= blockSize) &&
				((outPtr->allocatedSize - outDataIndex) >= blockSize))
			{
				if ((inPtr->dataSize - inDataIndex) <= 
					(outPtr->allocatedSize - outDataIndex))
				{
					chunkSize = (inPtr->dataSize - inDataIndex) -
								((inPtr->dataSize - inDataIndex) % blockSize);
				}
				else
				{
					chunkSize = (outPtr->allocatedSize - outDataIndex) -
								((outPtr->allocatedSize - outDataIndex) % 
									blockSize);
				}
				
				err = PGPCBCDecrypt(cbcRef, &(inPtr->data[inDataIndex]), 
						chunkSize, &(outPtr->data[outDataIndex]));
				
				outDataIndex += chunkSize;
				inDataIndex += chunkSize;
				inputSize -= chunkSize;

				if (inDataIndex == inPtr->dataSize)
				{
					inPtr = inPtr->next;
					inDataIndex = 0;
				}

				if (outDataIndex == outPtr->allocatedSize)
				{
					outPtr->dataSize = outPtr->allocatedSize;
					tempPtr = outPtr;
					outPtr = outPtr->next;
					outDataIndex = 0;
				}
			}
			else
			{
				if ((inPtr->dataSize - inDataIndex) < blockSize)
				{
					inSlopPtr = inSlopBlock;
					
					while ((inSlopIndex < blockSize) && IsntNull(inPtr))
					{
						total = inPtr->dataSize - inDataIndex;
						if (total > (blockSize - inSlopIndex))
							total = blockSize - inSlopIndex;
						
						pgpCopyMemory(&(inPtr->data[inDataIndex]),
							&(inSlopBlock[inSlopIndex]), total);
						
						inSlopIndex += total;
						inDataIndex += total;
						inputSize -= total;
						
						if (inDataIndex == inPtr->dataSize)
						{
							inPtr = inPtr->next;
							inDataIndex = 0;
						}
					}
				}
				else
				{
					inSlopPtr = &(inPtr->data[inDataIndex]);
					inSlopIndex = blockSize;
					inDataIndex += blockSize;
					inputSize -= blockSize;
				}
				
				if ((outPtr->allocatedSize - outDataIndex) < blockSize)
				{
					err = PGPExpandIPsecBuffer(memoryMgr, outPtr, 
							maxBufferSize, blockSize + outDataIndex);
					
					if ((outPtr->allocatedSize - outDataIndex) < blockSize)
						outSlopPtr = outSlopBlock;
					else
						outSlopPtr = &(outPtr->data[outDataIndex]);
				}
				else
					outSlopPtr = &(outPtr->data[outDataIndex]);
				
				if (IsntPGPError(err) && (inSlopIndex == blockSize))
				{
					inSlopIndex = 0;
					err = PGPCBCDecrypt(cbcRef, inSlopPtr, blockSize, 
							outSlopPtr);
					
					if (IsntPGPError(err) && (outSlopPtr == outSlopBlock))
					{
						outSlopIndex = outPtr->allocatedSize - outDataIndex;
						
						pgpCopyMemory(outSlopBlock, 
							&(outPtr->data[outDataIndex]), outSlopIndex);
						
						outPtr->dataSize = outPtr->allocatedSize;
						tempPtr = outPtr;
						outPtr = outPtr->next;
						
						pgpCopyMemory(&(outSlopBlock[outSlopIndex]),
							outPtr->data, blockSize - outSlopIndex);
						
						outDataIndex = blockSize - outSlopIndex;
					}
					else
						outDataIndex += blockSize;
				}
			}
		}
	}
	else
	{
		if (tunnelMode)
			PGPCopyIPsecBuffer(memoryMgr, 
				ipPacketIn, espIndex + ESP_HEADERSIZE + blockSize, 
				ipPacketOut, 0, packetSize - espIndex - ESP_HEADERSIZE, 
				maxBufferSize);
		else
			PGPCopyIPsecBuffer(memoryMgr, 
				ipPacketIn, espIndex + ESP_HEADERSIZE + blockSize, 
				ipPacketOut, espIndex, packetSize - espIndex - ESP_HEADERSIZE,
				maxBufferSize);

		outPtr = ipPacketOut;
		tempPtr = NULL;
		while (IsntNull(outPtr->next))
		{
			if (outPtr->next->dataSize == 0)
				break;
			else
			{
				tempPtr = outPtr;
				outPtr = outPtr->next;
			}
		}
		outDataIndex = outPtr->dataSize;
	}
		
	if (IsntPGPError(err) && (hash != kPGPHashAlgorithm_Invalid))
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
	{
		if (outDataIndex > 1)
		{
			padLength = outPtr->data[outDataIndex - 2];
			nextHeader = outPtr->data[outDataIndex - 1];
		}
		else if (outDataIndex == 1)
		{
			padLength = tempPtr->data[tempPtr->dataSize - 1];
			nextHeader = outPtr->data[0];
		}
		else
		{
			padLength = tempPtr->data[tempPtr->dataSize - 2];
			nextHeader = tempPtr->data[tempPtr->dataSize - 1];
		}

		if (outDataIndex > (PGPUInt32) (padLength + 2))
		{
			outDataIndex -= padLength + 2;
			outPtr->dataSize = outDataIndex;
		}
		else
		{
			outPtr->dataSize = 0;
			tempPtr->dataSize -= padLength + 2 - outDataIndex;
			outDataIndex = tempPtr->dataSize;
			outPtr = tempPtr;
		}

		if ((nextHeader == IP_IN_IP) && !tunnelMode)
		{
			tunnelMode = TRUE;
			PGPCopyIPsecBuffer(memoryMgr, ipPacketOut, espIndex,
				ipPacketOut, 0, inputSize - (padLength + 2), maxBufferSize);
		}

		packetSize -= blockSize;

		if (tunnelMode)
			packetSize -= padLength + 2 + ESP_HEADERSIZE + espIndex;
		else
		{
			packetSize -= padLength + 2 + ESP_HEADERSIZE;
			err = pgpRemoveProtocolFromIPHeader(ipPacketIn, ESP_PROTOCOL, 
					nextHeader, ipPacketOut);
		}
	}

	if (IsntPGPError(err))
	{
		pgpSetPacketSize(packetSize, ipPacketOut);
		pgpSetIPHeaderChecksum(ipPacketOut);
	}

	if (IsntNull(iv))
		PGPFreeData(iv);

	if (IsntNull(cbcRef))
		PGPFreeCBCContext(cbcRef);

	if (IsntNull(hmacRef))
		PGPFreeHMACContext(hmacRef);

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