/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpIPsec.c,v 1.34.4.2 1999/06/16 22:09:04 dgal Exp $
____________________________________________________________________________*/

#ifndef USERLAND_TEST
#define USERLAND_TEST	0
#endif


#if USERLAND_TEST

#define DBG_FUNC(x)
#define DBG_ENTER()
#define DBG_PRINT(x)
#define DBG_LEAVE(x)

#include "pgpNetKernel.h"

#else

#include "pgpNetKernel.h"
#include "vpndbg.h"

#endif 


#include "pgpIPsecAH.h"
#include "pgpIPsecESP.h"
#include "pgpIPsecComp.h"
#include "pgpIPheader.h"
#include "pgpEndianConversion.h"
#include "pgpMem.h"
#include "pgpIPsecContextPriv.h"

#define ETHERNET_HDRLENGTH	14

PGPError
PGPnetIPsecGetAddrAndSPI(struct _PGPnetPMContext *pContext,
						 PGPNDIS_PACKET *packet, 
						 PGPUInt32 *ipAddress,
						 PGPUInt32 *spi)
{
	PGPIPsecBuffer inPacket;
	PGPUInt32 tempSPI;
	PGPError err = kPGPError_NoErr;

	if (IsNull(pContext) || IsNull(packet) || IsNull(ipAddress) || 
		IsNull(spi))
		return kPGPError_BadParams;

	*spi = 0;

	if (packet->xformBlockLen > 0)
	{
		inPacket.data = &(packet->xformBlock[ETHERNET_HDRLENGTH]);
		inPacket.dataSize = packet->xformBlockLen - ETHERNET_HDRLENGTH;
	}
	else
	{
		inPacket.data = &(packet->srcBlock[ETHERNET_HDRLENGTH]);
		inPacket.dataSize = packet->srcBlockLen - ETHERNET_HDRLENGTH;
	}

	inPacket.allocatedSize = MAX_DATASIZE + 20;
	inPacket.next = NULL;

	pgpGetSrcIPAddress(&inPacket, ipAddress);
	pgpIPsecESPGetSPI(&inPacket, &tempSPI);

	if (!tempSPI)
		pgpIPsecAHGetSPI(&inPacket, &tempSPI);

	*spi = tempSPI;
	return err;
}


PGPError
PGPnetIPsecApplySA(PVPN_ADAPTER adapter,
				   struct _PGPnetPMContext *pContext,
				   PGPNDIS_PACKET *packet,
				   PGPnetKernelSA *sa,
				   PGPBoolean incoming,
				   PGPBoolean tunnel)
{
	PGPMemoryMgrRef memoryMgr;
	PGPCipherAlgorithm espCipher = kPGPCipherAlgorithm_None;
	PGPHashAlgorithm espHash = kPGPHashAlgorithm_Invalid;
	PGPHashAlgorithm ahHash = kPGPHashAlgorithm_Invalid;
	PGPCompressionAlgorithm compAlg = kPGPCompAlgorithm_None;
	PGPUInt16 index;
	PGPByte *espCipherKey;
	PGPByte *espIV;
	PGPByte *espHashKey;
	PGPByte *ahHashKey;
	PGPUInt32 espSPI;
	PGPUInt32 ahSPI;
	PGPUInt32 gatewayIP;
	PGPUInt32 sequenceWindow;
	PGPUInt32 upperSequence;
	PGPUInt32 lowerSequence;
	PGPBoolean tunnelAH;
	PGPBoolean tunnelESP;
	PGPBoolean useESP;
	PGPBoolean useAH;
	PGPBoolean useIPCOMP;
	PGPBoolean didUseIPCOMP;
	PGPBoolean lastSrcBlock;
	PGPBoolean secondCall;
	PGPIPsecBuffer *inPacket = NULL;
	PGPIPsecBuffer *outPacket = NULL;
	PGPIPsecBuffer *inPtr = NULL;
	PGPIPsecBuffer *outPtr = NULL;
	PGPIPsecBuffer *tempPtr = NULL;
	PGPIPsecBuffer tempBuffer[2];
	PGPIPsecBuffer *tempBufferPtr1;
	PGPIPsecBuffer *tempBufferPtr2;
	PGPNDIS_PACKET *ndisPtr;
	NDIS_STATUS status;
	PGPError err = kPGPError_NoErr;

    DBG_FUNC("PGPnetIPsecApplySA")

    DBG_ENTER();

	if (IsNull(pContext) || IsNull(packet)) {
		DBG_LEAVE(kPGPError_BadParams);
		return kPGPError_BadParams;
	}

	memoryMgr = pContext->ipsecContext->memory;
	ndisPtr = packet;

	if (!ndisPtr->firstSrcBlock)
		return kPGPError_BadParams;

	if (packet->xformBlockLen > 0)
		secondCall = TRUE;
	else
		secondCall = FALSE;

	inPacket = (PGPIPsecBuffer *) PGPNewData(memoryMgr, 
									sizeof(PGPIPsecBuffer), 0);

	if (IsNull(inPacket))
		err = kPGPError_OutOfMemory;
	else
	{
		if (secondCall)
		{
			inPacket->data = (PGPByte *) PGPNewData(memoryMgr, 
												MAX_DATASIZE + 20, 
												0);
			
			if (IsNull(inPacket->data))
				err = kPGPError_OutOfMemory;
			else
			{
				pgpCopyMemory(&(ndisPtr->xformBlock[ETHERNET_HDRLENGTH]),
					inPacket->data, 
					ndisPtr->xformBlockLen - ETHERNET_HDRLENGTH);
			}

			inPacket->dataSize = ndisPtr->xformBlockLen - ETHERNET_HDRLENGTH;
		}
		else
		{
			inPacket->data = &(ndisPtr->srcBlock[ETHERNET_HDRLENGTH]);
			inPacket->dataSize = ndisPtr->srcBlockLen - ETHERNET_HDRLENGTH;
		}
		
		inPacket->allocatedSize = MAX_DATASIZE + 20;
		inPacket->next = NULL;
	}

	outPacket = (PGPIPsecBuffer *) PGPNewData(memoryMgr, 
										sizeof(PGPIPsecBuffer), 0);

	if (IsNull(outPacket))
		err = kPGPError_OutOfMemory;
	else
	{
		outPacket->data = &(ndisPtr->xformBlock[ETHERNET_HDRLENGTH]);
		outPacket->dataSize = 0;
		outPacket->allocatedSize = MAX_DATASIZE + 20;
		outPacket->next = NULL;
	}

	for (index=0; index<2; index++)
	{
		tempBuffer[index].data = (PGPByte *) PGPNewData(memoryMgr, 
												MAX_DATASIZE + 20, 
												0);

		if (IsNull(tempBuffer[index].data))
			err = kPGPError_OutOfMemory;
		else
		{
			tempBuffer[index].dataSize = 0;
			tempBuffer[index].allocatedSize = MAX_DATASIZE + 20;
			tempBuffer[index].next = NULL;
		}
	}

	inPtr = inPacket;
	outPtr = outPacket;
	tempBufferPtr1 = &(tempBuffer[0]);
	tempBufferPtr2 = &(tempBuffer[1]);
	
	if (secondCall)
		lastSrcBlock = ndisPtr->lastXformBlock;
	else
		lastSrcBlock = ndisPtr->lastSrcBlock;

	while (IsntNull(ndisPtr->link) && IsntPGPError(err))
	{
		if (!lastSrcBlock)
		{
			inPtr->next = (PGPIPsecBuffer *) PGPNewData(memoryMgr, 
												sizeof(PGPIPsecBuffer), 0);
			if (IsNull(inPtr->next))
				err = kPGPError_OutOfMemory;
		}
		else
			inPtr->next = NULL;

		outPtr->next = (PGPIPsecBuffer *) PGPNewData(memoryMgr, 
											sizeof(PGPIPsecBuffer), 0);

		if (IsNull(outPtr->next))
			err = kPGPError_OutOfMemory;
		
		tempBufferPtr1->next = (PGPIPsecBuffer *) PGPNewData(memoryMgr, 
													sizeof(PGPIPsecBuffer), 
													0);

		if (IsNull(tempBufferPtr1->next))
		{
			err = kPGPError_OutOfMemory;
			break;
		}

		tempBufferPtr1 = tempBufferPtr1->next;
		tempBufferPtr1->data = (PGPByte *) PGPNewData(memoryMgr, 
											MAX_DATASIZE, 
											0);
			
		if (IsNull(tempBufferPtr1->data))
			err = kPGPError_OutOfMemory;
		else
		{
			tempBufferPtr1->dataSize = 0;
			tempBufferPtr1->allocatedSize = MAX_DATASIZE;
			tempBufferPtr1->next = NULL;
		}
		
		tempBufferPtr2->next = (PGPIPsecBuffer *) PGPNewData(memoryMgr, 
													sizeof(PGPIPsecBuffer), 
													0);

		if (IsNull(tempBufferPtr2->next))
		{
			err = kPGPError_OutOfMemory;
			break;
		}

		tempBufferPtr2 = tempBufferPtr2->next;
		tempBufferPtr2->data = (PGPByte *) PGPNewData(memoryMgr, 
												MAX_DATASIZE, 
												0);
			
		if (IsNull(tempBufferPtr2->data))
			err = kPGPError_OutOfMemory;
		else
		{
			tempBufferPtr2->dataSize = 0;
			tempBufferPtr2->allocatedSize = MAX_DATASIZE;
			tempBufferPtr2->next = NULL;
		}
		
		if (IsntPGPError(err))
		{
			outPtr = outPtr->next;
			ndisPtr = ndisPtr->link;

			if (!lastSrcBlock)
			{
				inPtr = inPtr->next;

				if (secondCall)
				{
					inPtr->data = (PGPByte *) PGPNewData(memoryMgr,
												MAX_DATASIZE, 0);

					if (IsNull(inPtr->data))
						err = kPGPError_OutOfMemory;
					else
					{
						pgpCopyMemory(&(ndisPtr->
							xformBlock[ETHERNET_HDRLENGTH + 20]),
							inPtr->data,
							ndisPtr->xformBlockLen - ETHERNET_HDRLENGTH - 20);
					}

					inPtr->dataSize = ndisPtr->xformBlockLen - 
										ETHERNET_HDRLENGTH - 20;
					lastSrcBlock = ndisPtr->lastXformBlock;
				}
				else
				{
					inPtr->data = &(ndisPtr->
										srcBlock[ETHERNET_HDRLENGTH + 20]);

					inPtr->dataSize = ndisPtr->srcBlockLen - 
										ETHERNET_HDRLENGTH - 20;

					lastSrcBlock = ndisPtr->lastSrcBlock;
				}

				inPtr->allocatedSize = MAX_DATASIZE;
				inPtr->next = NULL;
			}

			outPtr->data = &(ndisPtr->xformBlock[ETHERNET_HDRLENGTH + 20]);
			outPtr->dataSize = 0;
			outPtr->allocatedSize = MAX_DATASIZE;
			outPtr->next = NULL;
		}
	}

	if (tunnel)
		gatewayIP = sa->ikeSA.ipAddress;
	else
		gatewayIP = 0;
	
	useESP = FALSE;
	useAH = FALSE;
	useIPCOMP = FALSE;

	for (index=0; index<sa->ikeSA.numTransforms; index++)
	{
		if (sa->ikeSA.transform[index].u.ipsec.protocol == kPGPike_PR_AH)
		{
			switch (sa->ikeSA.transform[index].u.ipsec.u.ah.t.authAlg)
			{
			case kPGPike_AH_MD5:
				ahHash = kPGPHashAlgorithm_MD5;
				useAH = TRUE;
				break;
				
			case kPGPike_AH_SHA:
				ahHash = kPGPHashAlgorithm_SHA;
				useAH = TRUE;
				break;
				
			default:
				err = kPGPError_BadHashNumber;
			}
			
			if (incoming)
			{
				ahHashKey = sa->ikeSA.transform[index].u.ipsec.u.ah.inAuthKey;
				
				ahSPI = PGPEndianToUInt32(kPGPBigEndian, 
							sa->ikeSA.transform[index].u.ipsec.inSPI);
			}
			else
			{
				ahHashKey = sa->
					ikeSA.transform[index].u.ipsec.u.ah.outAuthKey;

				ahSPI = PGPEndianToUInt32(kPGPBigEndian, 
							sa->ikeSA.transform[index].u.ipsec.outSPI);
			}
		}
		else if (sa->ikeSA.transform[index].u.ipsec.protocol == 
					kPGPike_PR_ESP)
		{
			DBG_PRINT(("Using kPGPike_PR_ESP\n"););
			switch (sa->ikeSA.transform[index].u.ipsec.u.esp.t.cipher)
			{
			case kPGPike_ET_3DES:
				espCipher = kPGPCipherAlgorithm_3DES;
				DBG_PRINT(("Using kPGPCipherAlgorithm_3DES\n"););
				useESP = TRUE;
				break;
				
			case kPGPike_ET_CAST:
				DBG_PRINT(("Using kPGPCipherAlgorithm_CAST5\n"););
				espCipher = kPGPCipherAlgorithm_CAST5;
				useESP = TRUE;
				break;
			
			case kPGPike_ET_DES_IV64:
				espCipher = kPGPCipherAlgorithm_3DES;
				DBG_PRINT(("Using kPGPCipherAlgorithm_3DES in 1DES mode\n"););
				useESP = TRUE;
				break;
			
			case kPGPike_ET_DES:
				espCipher = kPGPCipherAlgorithm_3DES;
				DBG_PRINT(("Using kPGPCipherAlgorithm_3DES as 1DES no IV\n"););
				useESP = TRUE;
				break;

			case kPGPike_ET_NULL:
				DBG_PRINT(("Using NULL algorithm\n"););
				espCipher = kPGPCipherAlgorithm_None;
				break;
				
			default:
				err = kPGPError_BadCipherNumber;
			}
			
			switch (sa->ikeSA.transform[index].u.ipsec.u.esp.t.authAttr)
			{
			case kPGPike_AA_HMAC_MD5:
				DBG_PRINT(("Using kPGPHashAlgorithm_MD5\n"););
				espHash = kPGPHashAlgorithm_MD5;
				useESP = TRUE;
				break;
				
			case kPGPike_AA_HMAC_SHA:
				DBG_PRINT(("Using kPGPHashAlgorithm_SHA\n"););
				espHash = kPGPHashAlgorithm_SHA;
				useESP = TRUE;
				break;
				
			case kPGPike_AA_None:
				DBG_PRINT(("Not using a hash algorithm\n"););
				espHash = kPGPHashAlgorithm_Invalid;
				break;
				
			default:
				err = kPGPError_BadHashNumber;
			}
			
			if (incoming)
			{
				espCipherKey = sa->
					ikeSA.transform[index].u.ipsec.u.esp.inESPKey;
				
				espHashKey = sa->
					ikeSA.transform[index].u.ipsec.u.esp.inAuthKey;
				
				espSPI = PGPEndianToUInt32(kPGPBigEndian, 
							sa->ikeSA.transform[index].u.ipsec.inSPI);
			}
			else
			{
				espCipherKey = sa->
					ikeSA.transform[index].u.ipsec.u.esp.outESPKey;

				espHashKey = sa->
					ikeSA.transform[index].u.ipsec.u.esp.outAuthKey;

				espSPI = PGPEndianToUInt32(kPGPBigEndian, 
							sa->ikeSA.transform[index].u.ipsec.outSPI);

				espIV = sa->
					ikeSA.transform[index].u.ipsec.u.esp.explicitIV;
			}
		}
		else if (sa->ikeSA.transform[index].u.ipsec.protocol == 
					kPGPike_PR_IPCOMP)
		{
			DBG_PRINT(("Using kPGPike_PR_IPCOMP\n"););
			switch (sa->ikeSA.transform[index].u.ipsec.u.ipcomp.t.compAlg)
			{
			case kPGPike_IC_Deflate:
				compAlg = kPGPCompAlgorithm_Deflate;
				DBG_PRINT(("Using kPGPCompAlgorithm_Deflate\n"););
				useIPCOMP = TRUE;
				break;
				
			case kPGPike_IC_LZS:
				DBG_PRINT(("Using kPGPCompAlgorithm_LZS\n"););
				compAlg = kPGPCompAlgorithm_LZS;
				useIPCOMP = TRUE;
				break;
				
			case kPGPike_IC_None:
				DBG_PRINT(("No compression\n"););
				compAlg = kPGPCompAlgorithm_None;
				break;
				
			default:
				err = kPGPError_BadParams;
			}
		}			
	}		

	if (!useESP && !useAH)
		useIPCOMP = FALSE;

	tunnelAH = !useESP && !useIPCOMP && tunnel;
	tunnelESP = !useIPCOMP && tunnel;

	inPtr = inPacket;

	if (useESP && (useAH || useIPCOMP))
		outPtr = &(tempBuffer[0]);
	else if (useAH && useIPCOMP)
		outPtr = &(tempBuffer[0]);
	else
		outPtr = outPacket;
	
	if (incoming)
	{
		if (useAH && IsntPGPError(err))
		{
			sequenceWindow = sa->sequenceWindow;
			upperSequence = sa->sequenceWindowUpper;
			lowerSequence = sa->sequenceWindowLower;

			err = pgpVerifyIPsecAuthentication(pContext->ipsecContext,
					MAX_DATASIZE, inPtr, tunnelAH, ahHash, ahHashKey, 
					&sequenceWindow, &upperSequence, &lowerSequence, 
					outPtr);

			inPtr = outPtr;
			if (useESP && useIPCOMP)
				outPtr = &(tempBuffer[1]);
			else
				outPtr = outPacket;
		}

		if (useESP && IsntPGPError(err))
		{
			sequenceWindow = sa->sequenceWindow;
			upperSequence = sa->sequenceWindowUpper;
			lowerSequence = sa->sequenceWindowLower;
	
			err = pgpIPsecDecrypt(pContext->ipsecContext, MAX_DATASIZE,
					inPtr, tunnelESP, espCipher, espCipherKey, espHash,
					espHashKey, &sequenceWindow, &upperSequence, 
					&lowerSequence, outPtr);

			inPtr = outPtr;
			outPtr = outPacket;
		}

		if (IsntPGPError(err) && (useESP || useAH))
		{
			sa->sequenceWindow = sequenceWindow;
			sa->sequenceWindowUpper = upperSequence;
			sa->sequenceWindowLower = lowerSequence;
		}

		if (useIPCOMP && IsntPGPError(err))
		{
			err = pgpIPsecDecompress(pContext->ipsecContext, MAX_DATASIZE,
					inPtr, tunnel, compAlg, outPtr);

			while (err == kPGPError_BufferTooSmall)
			{
				ndisPtr = packet;
				tempPtr = outPacket;
				tempPtr->dataSize = 0;

				while (IsntNull(tempPtr->next))
				{
					ndisPtr = ndisPtr->link;
					tempPtr = tempPtr->next;
					tempPtr->dataSize = 0;
				}

				tempPtr->next = (PGPIPsecBuffer *) PGPNewData(memoryMgr,
													sizeof(PGPIPsecBuffer),
													0);

				if (IsntNull(tempPtr->next))
				{
					ndisPtr->link = 
						PGPNdisPacketAllocWithSrcPacket(&status, adapter);

					if (IsntNull(ndisPtr->link))
					{
						tempPtr->next->data = 
							&(ndisPtr->link->
								xformBlock[ETHERNET_HDRLENGTH + 20]);

						tempPtr->next->dataSize = 0;
						tempPtr->next->allocatedSize = MAX_DATASIZE;
						tempPtr->next->next = NULL;
						ndisPtr->link->link = NULL;
					}
					else
						err = kPGPError_OutOfMemory;
				}
				else
					err = kPGPError_OutOfMemory;

				if (err == kPGPError_BufferTooSmall)
					err = pgpIPsecDecompress(pContext->ipsecContext, 
							MAX_DATASIZE, inPtr, tunnel, compAlg, outPtr);
			}
		}
	}
	else
	{
		if (secondCall)
		{
			pgpClearDFBit(inPtr);
			pgpSetIPHeaderChecksum(inPtr);
		}

		if (useIPCOMP && IsntPGPError(err))
		{
			err = pgpIPsecCompress(pContext->ipsecContext, MAX_DATASIZE,
					inPtr, tunnel, gatewayIP, compAlg, outPtr, &didUseIPCOMP);

			inPtr = outPtr;
			if (useESP && useAH)
				outPtr = &(tempBuffer[1]);
			else
				outPtr = outPacket;
			
			tunnelESP = tunnel && useESP && !didUseIPCOMP;
			tunnelAH = tunnel && useAH && !useESP && !didUseIPCOMP;
		}

		if (useESP && IsntPGPError(err))
		{
			err = pgpIPsecEncrypt(pContext->ipsecContext, MAX_DATASIZE,
					inPtr, tunnelESP, gatewayIP, espCipher, espCipherKey, 
					espIV, espHash, espHashKey, espSPI, sa->packetsSent + 1, 
					outPtr);

			inPtr = outPtr;
			outPtr = outPacket;
		}

		if (useAH && IsntPGPError(err))
		{
			err = pgpApplyIPsecAuthentication(pContext->ipsecContext,
					MAX_DATASIZE, inPtr, tunnelAH, gatewayIP, ahHash, 
					ahHashKey, ahSPI, sa->packetsSent + 1, outPtr);
		}
	}

	if (IsntPGPError(err))
	{
		packet->xformBlockLen = outPacket->dataSize + ETHERNET_HDRLENGTH;
		packet->firstXformBlock = TRUE;
		outPtr = outPacket->next;

		if (IsNull(packet->link) || IsNull(outPtr))
		{
			packet->lastXformBlock = TRUE;
			ndisPtr = NULL;
		}
		else if (outPtr->dataSize == 0)
		{
			packet->lastXformBlock = TRUE;
			ndisPtr = NULL;
		}
		else
		{
			ndisPtr = packet->link;
			packet->lastXformBlock = FALSE;
		}

		while (IsntNull(ndisPtr))
		{
			ndisPtr->xformBlockLen = ETHERNET_HDRLENGTH + 20 +
										outPtr->dataSize;

			ndisPtr->firstXformBlock = FALSE;
			ndisPtr->lastXformBlock = FALSE;

			if (IsNull(outPtr->next))
			{
				ndisPtr->lastXformBlock = TRUE;
				ndisPtr = NULL;
			}
			else if (outPtr->next->dataSize == 0)
			{
				ndisPtr->lastXformBlock = TRUE;
				ndisPtr = NULL;
			}
			else
			{
				outPtr = outPtr->next;
				ndisPtr = ndisPtr->link;
			}
		}
	}
	
	for (index=0; index<2; index++)
	{
		if (IsntNull(tempBuffer[index].data))
			PGPFreeData(tempBuffer[index].data);

		inPtr = tempBuffer[index].next;

		while (IsntNull(inPtr))
		{
			if (IsntNull(inPtr->data))
				PGPFreeData(inPtr->data);

			tempPtr = inPtr;
			inPtr = inPtr->next;
			PGPFreeData(tempPtr);
		}
	}

	inPtr = inPacket;
	while (IsntNull(inPtr))
	{
		if (secondCall)
			PGPFreeData(inPtr->data);

		tempPtr = inPtr;
		inPtr = inPtr->next;
		PGPFreeData(tempPtr);
	}

	outPtr = outPacket;
	while (IsntNull(outPtr))
	{
		tempPtr = outPtr;
		outPtr = outPtr->next;
		PGPFreeData(tempPtr);
	}

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
