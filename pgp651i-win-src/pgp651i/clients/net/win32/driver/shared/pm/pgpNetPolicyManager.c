/*
 * Copyright (c) 1998, Network Associates, Inc.
 * All rights reserved.
 *
 * $Id: pgpNetPolicyManager.c,v 1.63.4.2 1999/06/17 21:15:20 elowe Exp $
 *
 */
#include <ndis.h>

#include "ipdef.h"
#include "vpn.h"
#include "vpndbg.h"

#include "pgpMem.h"
#include "pgpNetKernel.h"
#include "pgpNetKernelXChng.h"
#include "pgpNetPMConfig.h"
#include "pgpNetPMHost.h"
#include "pgpNetPMSA.h"
#include "pgpNetPMEvents.h"

/* some IPSEC helper functions */
#include "pgpIPsecBuffer.h"
#include "pgpIPsecESP.h"
#include "pgpIPsecAH.h"
#include "pgpIPheader.h"

#define ESP_HEADERSIZE	8
#define AH_HEADERSIZE	12

#ifndef PM_EMULATION

static void sCheckRekey(PVPN_ADAPTER adapter, PGPnetKernelSA *pSA);
static void sCopySrcToxForm(PGPNDIS_PACKET *start);
static void sCopyPacketXformToSrc(const PGPNDIS_PACKET *source, PGPNDIS_PACKET *dest,PVPN_ADAPTER adapter);
static PGPBoolean sCheckExpiration(PVPN_ADAPTER	adapter,
								   PGPnetKernelSA *pSA);
static PGPnetPMStatus sDoTransformOutgoing(PGPnetPMContext *pContext,
										   PGPNDIS_PACKET *packet,
										   PVPN_ADAPTER adapter);
static PGPnetPMStatus sDoTransformIncoming(PGPnetPMContext *pContext,
										   PGPNDIS_PACKET *packet,
										   PVPN_ADAPTER adapter);
static PGPnetPMStatus sAcquireSA(PGPnetPMContext *pContext,
								 PGPUInt32 ipAddress,
								 PGPUInt32 ipAddrStart,
								 PGPUInt32 ipMaskEnd,
								 PGPBoolean incoming,
								 void * ipBuffer,
								 PGPUInt32 ipBufferSize,
								 PVPN_ADAPTER adapter,
								 PGPnetKernelSA **pSA);
static PGPnetPMStatus sLookHardForSA(PGPnetPMContext *pContext, 
			   PVPN_ADAPTER adapter,
			   PGPNetHostEntry *host,
			   PGPUInt32 ipAddress,
			   PGPUInt16 port,
			   void *ipBuffer,
			   PGPUInt32 ipBufferSize,
			   PGPnetKernelSA **ppTransport,
			   PGPnetKernelSA **ppTunnel,
			   PGPBoolean incoming);



void
PGPnetRASdisconnect(PVPN_ADAPTER adapter)
{
	/* fire event */
	DBG_FUNC("PGPnetRASdisconnect")

	PGPMESSAGE_CONTEXT	*	kernelMessageContext;
	PGPnetMessageHeader *	kernelMessageHeader;
	PGPUInt32				head = 0;

	DBG_ENTER();

	if ((adapter != NULL) && (adapter->pgpMessage != NULL)) {
		kernelMessageContext = (PGPMESSAGE_CONTEXT*)(adapter->pgpMessage);
		kernelMessageHeader = &kernelMessageContext->header;
		
		NdisAcquireSpinLock(&adapter->general_lock);

		head = kernelMessageHeader->head + 1;

		if (head > kernelMessageHeader->maxSlots)
			head = 1;

		kernelMessageContext = &kernelMessageContext[head];
		kernelMessageContext->messageType = PGPnetMessageRASdisconnect;

		kernelMessageHeader->head = head;

		NdisReleaseSpinLock(&adapter->general_lock);

		PgpEventSet(&adapter->pgpEvent);
	}

	DBG_LEAVE(0);
}

void
PGPnetRASconnect(PVPN_ADAPTER adaptor, ULONG ipAddress)
{
	DBG_FUNC("PGPnetRASconnect")

	DBG_ENTER();

	DBG_PRINT(("New IP address Report, ipAddress: %xh\n", ipAddress););
	DBG_LEAVE(0);

}

/*
 * This is the Heavy (or default) NeedTransform. It starts
 * the SA acquisition process.
 */
PGPnetPMStatus
PGPnetPMNeedTransform(PGPnetPMContext *pContext, 
					  PGPUInt32 ipAddress, 
					  PGPUInt16 port,
					  PGPBoolean incoming,
					  void * ipBuffer,
					  PGPUInt32 ipBufferSize,
					  void * reserved)
{
	PGPNetHostEntry *	host 		= PMFindHost(pContext, ipAddress);
	PVPN_ADAPTER		adapter		= (PVPN_ADAPTER) reserved;
	PGPnetPMStatus		ret;
	PGPnetKernelSA *	pSA;
	
    DBG_FUNC("PGPnetPMNeedTransform")

	DBG_ENTER();

	ret = PGPnetPMNeedTransformLight(pContext, ipAddress, incoming, adapter);

	if (ret == kPGPNetPMPacketEncrypt) {
		ret = sLookHardForSA(pContext, 
			adapter,
			host,
			ipAddress,
			port,
			ipBuffer,
			ipBufferSize,
			&pSA,
			&pSA,
			incoming);

		if (ret != kPGPNetPMPacketEncrypt && port == UDP_PORT_IKE_NET)
			ret = kPGPNetPMPacketClear;

		DBG_LEAVE(ret);
		return ret;
	}

	DBG_LEAVE(ret);

	return ret;
}

/*
 * This is the lightweight version of NeedTransform. It does not
 * start the SA acquisition process.
 */
PGPnetPMStatus
PGPnetPMNeedTransformLight(PGPnetPMContext *pContext, 
					  PGPUInt32 ipAddress, 
					  PGPBoolean incoming,
					  void * reserved)
{
	PGPNetHostEntry *	host 		= PMFindHost(pContext, ipAddress);
	PGPNetHostEntry *	secureGW	= 0;
	PVPN_ADAPTER		adapter		= (PVPN_ADAPTER) reserved;
	
    DBG_FUNC("PGPnetPMNeedTransformLight")

	DBG_ENTER();
	/* configuration not here yet, default to clear */
	if (!pContext->pConfig) {
		DBG_PRINT(("kPGPNetPMPacketClear because no config yet\n"););
		DBG_LEAVE(kPGPNetPMPacketClear);
		return kPGPNetPMPacketClear;
	}

	if (!pContext->pConfig->bPGPnetEnabled){
		DBG_PRINT(("kPGPNetPMPacketClear because PGPnet disabled by user\n"););
		DBG_LEAVE(kPGPNetPMPacketClear);
		return kPGPNetPMPacketClear;
	}
	
	if (host) {
		/* Check for insecure host and no gateway */
		if (host->hostType == kPGPnetInsecureHost && host->childOf == -1) {
			DBG_PRINT(("kPGPNetPMPacketClear because host is insecure\n"););
			DBG_LEAVE(kPGPNetPMPacketClear);
			return kPGPNetPMPacketClear;
		}

		DBG_PRINT(("kPGPNetPMPacketEncrypt\n"););
		DBG_LEAVE(kPGPNetPMPacketEncrypt);
		return kPGPNetPMPacketEncrypt;
	}

	if (pContext->pConfig->bAllowUnconfigHost && 
		pContext->pConfig->bAttemptUnconfigHost) {
		DBG_PRINT(("kPGPNetPMPacketEncrypt\n"););
		DBG_LEAVE(kPGPNetPMPacketEncrypt);
		return kPGPNetPMPacketEncrypt;
	}

	if (pContext->pConfig->bAllowUnconfigHost && 
		!pContext->pConfig->bAttemptUnconfigHost) {
		/* 
		 * There may be an SA for this host that was initiated by the peer.
		 * So we need to look for an SA, before sending it in the clear
		 */
		if (PMFindSA(pContext, ipAddress, 0, incoming)) {
			DBG_PRINT(("kPGPNetPMPacketEncrypt because SA exists\n"););
			DBG_LEAVE(kPGPNetPMPacketEncrypt);
			return kPGPNetPMPacketEncrypt;
		} else {
			DBG_PRINT(("kPGPNetPMPacketClear because bAllow && !bAttempt\n"););
			DBG_LEAVE(kPGPNetPMPacketClear);
			return kPGPNetPMPacketClear;
		}
	}

	if (!pContext->pConfig->bAllowUnconfigHost) {
		DBG_PRINT(("kPGPNetPMPacketDrop because !bAllow\n"););
		DBG_LEAVE(kPGPNetPMPacketDrop);
		return kPGPNetPMPacketDrop;
	}

	DBG_PRINT(("kPGPNetPMPacketDrop because default\n"););
	DBG_LEAVE(kPGPNetPMPacketDrop);

	return kPGPNetPMPacketDrop;
}

PGPnetPMStatus
PGPnetPMDoTransform(PGPnetPMContext *pContext, 
					PGPNDIS_PACKET *packet,
					PGPBoolean incoming,
					PVPN_ADAPTER adapter)
{
	PGPnetPMStatus retCode;
	PGPUInt32 i = 0;

    DBG_FUNC("PGPnetPMDoTransform")

    DBG_ENTER();

	packet->xformBlockLen = 0;
	if (!incoming) {
		retCode = sDoTransformOutgoing(pContext, packet, adapter);
	} else {
		retCode = sDoTransformIncoming(pContext, packet, adapter);
	}

#if/*def*/ 0 /*DBG*/
	/* write out packet src info */
	DBG_PRINT(("Incoming: %s\n", incoming ? "TRUE" : "FALSE"););
	DBG_PRINT(("packet->srcBlockLen: %d\n", packet->srcBlockLen););
	DBG_PRINT(("packet->srcBlock: %X\n", packet->srcBlock););
	for (i = 0; i<packet->srcBlockLen; i++) {
		DbgPrint("%X ", packet->srcBlock[i]);
		if (((i+1) % 16) == 0) {
			DbgPrint("\n");
		}
	}
	DbgPrint("\n");

	/* write out packet xform info */
	DBG_PRINT(("packet->xformBlockLen: %d\n", packet->xformBlockLen););
	DBG_PRINT(("packet->xformBlock: %X\n", packet->xformBlock););
	for (i = 0; i<packet->xformBlockLen; i++) {
		DbgPrint("%X ", packet->xformBlock[i]);
		if (((i+1) % 16) == 0) {
			DbgPrint("\n");
		}
	}
	DbgPrint("\n");
#endif

	return retCode;
}

#endif // !PM_EMULATION

PGPnetPMStatus
sAcquireSA(PGPnetPMContext *pContext, 
		   PGPUInt32 ipAddress,
		   PGPUInt32 ipAddrStart,
		   PGPUInt32 ipMaskEnd,
		   PGPBoolean incoming,
		   void *ipBuffer,
		   PGPUInt32 ipBufferSize,
		   PVPN_ADAPTER adapter,
		   PGPnetKernelSA **pSA)
{
	PGPnetPMStatus		ret;
	PGPUInt32 spi = 0;
	PGPError err = kPGPError_NoErr;
	PGPIPsecBuffer inPacket;

	DBG_FUNC("sAcquireSA")

	DBG_ENTER();

	if (incoming) {
		
		PGPUInt32 unused;

		if (ipBufferSize == 0) {
			/* assume ipBuffer is an PGPNDIS_PACKET */
			/* Bad Assumption? XXX */
			PGPNDIS_PACKET *packet = (PGPNDIS_PACKET*)ipBuffer;

			packet->xformBlockLen = 0;
			err = PGPnetIPsecGetAddrAndSPI(pContext, 
				packet,
				&unused,
				&spi);
		} else {
			// look for spi in the ipBuffer, hopefully we have
			// read in enough data. If not, then say we will encrypt
			// and deal with any problems later

			if (ipBufferSize < (sizeof(IP_HEADER) + ESP_HEADERSIZE) ) {
				DBG_LEAVE(kPGPNetPMPacketEncrypt);
				return kPGPNetPMPacketEncrypt;
			}	

			inPacket.data = ipBuffer;
			inPacket.dataSize = ipBufferSize;
			inPacket.allocatedSize = ipBufferSize;
			inPacket.next = NULL;

			pgpIPsecESPGetSPI(&inPacket, &spi);

			if (!spi) {
				if (ipBufferSize < (sizeof(IP_HEADER) + AH_HEADERSIZE) ) {
					DBG_LEAVE(kPGPNetPMPacketEncrypt);
					return kPGPNetPMPacketEncrypt;
				}
				pgpIPsecAHGetSPI(&inPacket, &spi);
			}
		}
	}
	
	/* try to find existing SA */
	if (IsntPGPError(err) && spi) {
		*pSA = PMFindSAspi(pContext, ipAddress, spi);
		if (!*pSA) {
			/* Ok, this is odd, the packet had an SPI, but we can't
			 * find a matching SA. So we check to see if we have a
			 * different SA with this machine. If so, we don't ask
			 * for a new one. In any event, we must drop the packet
			 * since we can't process it.
			 */
			/* generate error for now */
			PMfireErrorEvent(adapter, ipAddress, kPGPIPsecError_NoSAFound);
			if (PMFindSA(pContext, ipAddress, ipAddrStart, incoming)) {
				DBG_PRINT(("kPGPNetPMPacketDrop because we could not find SA\n"););
				return kPGPNetPMPacketDrop;
			}
		}
	} else 
		*pSA = PMFindSA(pContext, ipAddress, ipAddrStart, incoming);

	if (*pSA && !(*pSA)->pending) {
		/* An SA is ready to go */
		DBG_PRINT(("kPGPNetPMPacketEncrypt because host is secure\n"););
		ret = kPGPNetPMPacketEncrypt;
	} else if (*pSA && (*pSA)->pending) {
		/* already waiting for one */
		DBG_PRINT(("kPGPNetPMPacketWaiting because no SA yet\n"););
		ret = kPGPNetPMPacketWaiting;
	} else {
		/* no SA */
		DBG_PRINT(("Asking for SA\n"););
		/* fire kernel event and ask service for SA */
		ret = PMfireSARequestEvent(adapter, ipAddress, ipAddrStart, ipMaskEnd);
		if (ret == kPGPNetPMPacketDrop) {
			DBG_LEAVE(kPGPNetPMPacketDrop);
			return kPGPNetPMPacketDrop;
		}

		if (!PMAddPendingSA(pContext, ipAddress, ipAddrStart, ipMaskEnd)) {
			DBG_LEAVE(kPGPNetPMPacketDrop);
			return kPGPNetPMPacketDrop;
		}
	}

	DBG_LEAVE(ret);
	return ret;
}

PGPBoolean
sCheckExpiration(PVPN_ADAPTER adapter, PGPnetKernelSA *pSA)
{
	DBG_FUNC("sCheckExpiration")
	if (!pSA)
		return FALSE;
	/* check for expiration */
	if ((pSA->ikeSA.kbLifeTime != 0 && 
			pSA->bytesSent >= (pSA->ikeSA.kbLifeTime * 1024)) || 
		(pSA->packetsSent >= (MAX_PGPUInt32-1))) {
		/*(pSA->packetsSent == 8)) {*/
		/* SA expired so generate SADied */
		PMfireSADiedEvent(adapter, pSA->ikeSA.transform[0].u.ipsec.inSPI);
		DBG_LEAVE(TRUE);
		return TRUE;
	}
	return FALSE;
}

void
sCheckRekey(PVPN_ADAPTER adapter, PGPnetKernelSA *pSA)
{
	DBG_FUNC("sCheckRekey")

	PGPBoolean	bRekey = FALSE;
	PGPUInt32	kbSent = 1;
	
	if (!pSA)
		return;
	
	/* OK, this is a little verbose, but I can understand it */

	/* 1st: already rekeying */
	if (pSA->reKeyInProgress)
		return;

	/* 2nd: packet count overload */
	if (pSA->packetsSent >= 0xFFFF7FFF) {
		bRekey = TRUE;
		goto rekey;
	}

	/* 3rd: no rekey timeout or only a few bytes sent */
	if ((pSA->ikeSA.kbLifeTime == 0) || (pSA->bytesSent < 1024)) 
		return;

	/* Ok calculating whether or not we should rekey */
	kbSent = pSA->bytesSent / 1024;

#ifdef CHICAGO

	//NdisGetCurrentSystemTime(&nowLarge);
//	KeQuerySystemTime(&nowLarge);
/*
VxDCall( Get_Last_Updated_System_Time );     //puts return value in eax
_asm mov timeVal, eax     ;get return value into variable
*/
#if 0
	pgpCopyMemory(&(nowLarge.HighPart) /*src*/,
		&now64 /*dest*/,
		4);
	pgpCopyMemory(&(nowLarge.LowPart) /*src*/,
		(&now64) + 4 /*dest*/,
		4);

	avgkb = kbSent / ((now64 - pSA->birthTime) / 10000000);
#endif

#else
	{
		PGPInt64	avgkb = 0;
		LARGE_INTEGER nowLarge;
		PGPInt64 now64 = 0, t1 = 0;

		KeQuerySystemTime(&nowLarge);
		now64 = nowLarge.QuadPart;
		t1 = (now64 - pSA->birthTime) / 10000000;

		if (t1 != 0)
			avgkb = kbSent / t1;

#define kPGPnetRekeyKBSeconds		30

		/* 4th: going to hit the limit in 30 seconds, better rekey */
		if ((avgkb != 0) && 
			(((pSA->ikeSA.kbLifeTime - kbSent) / avgkb) 
			<= kPGPnetRekeyKBSeconds)) { 
			bRekey = TRUE;
			goto rekey;
		}
	}
#endif

	/* 5th: only 1 meg left, better rekey */
	if (kbSent >= (pSA->ikeSA.kbLifeTime - 2048)) { 
		bRekey = TRUE;
		goto rekey;
	}

rekey:
	if (bRekey) {
		DBG_PRINT(("Firing Rekey event\n"););
		PMfireSARekeyEvent(adapter, pSA->ikeSA.transform[0].u.ipsec.inSPI);

		pSA->reKeyInProgress = TRUE;
	}
}

PGPnetPMStatus
sDoTransformOutgoing(PGPnetPMContext *pContext, 
					 PGPNDIS_PACKET *packet, 
					 PVPN_ADAPTER adapter)
{
	PGPUInt32		ipAddress		= htonl(packet->ipAddress);
	PGPUInt32		i				= 0;
	PGPNetHostEntry *host 			= PMFindHost(pContext, ipAddress);
	PGPnetKernelSA 	*transportSA	= 0, *tunnelSA = 0;
	PGPError		err				= kPGPError_NoErr;
	PGPBoolean		bRet			= FALSE;
	PGPnetPMStatus	ret;

    DBG_FUNC("sDoTransformOutgoing")

    DBG_ENTER();

retry:
	ret = sLookHardForSA(pContext, 
		adapter, 
		host, 
		ipAddress, 
		packet->port,
		packet,
		0,
		&transportSA, 
		&tunnelSA, 
		FALSE);

	if (ret != kPGPNetPMPacketEncrypt && ret != kPGPNetPMPacketClear) {
		DBG_PRINT(("Could not find SA\n"););
		DBG_PRINT(("Happens w/ fragmented packets and other cases\n"););
		goto end;
	}
	
	bRet = sCheckExpiration(adapter, transportSA);
	if (bRet) {
		/* now set activeOut to false  */
		transportSA->ikeSA.activeOut = FALSE;
		goto retry;
	}

	bRet = sCheckExpiration(adapter, tunnelSA);
	if (bRet) {
		/* now set activeOut to false */
		transportSA->ikeSA.activeOut = FALSE;
		goto retry;
	}

	sCheckRekey(adapter, transportSA);
	sCheckRekey(adapter, tunnelSA);

	if (transportSA && packet->port != UDP_PORT_IKE_NET) {
		err = PGPnetIPsecApplySA(adapter, pContext, packet, transportSA, FALSE, FALSE);
		if (IsntPGPError(err)) {
			transportSA->packetsSent++;
			DBG_PRINT(("Packets sent: %d\n", transportSA->packetsSent););		
			/* XXX Is BlockLen the right thing to add? */
			transportSA->bytesSent += packet->srcBlockLen /* - header size */;
		} else {
			goto fail;
		}

#if/*def*/ 0 /*DBG*/
	/* write out SA info */
	DBG_PRINT(("inESPKey\n"););
	for (i = 0; i< kPGPike_ESPMaximumKeySize; i++) {
		DbgPrint("%X ", transportSA->ikeSA.transform[0].u.ipsec.u.esp.inESPKey[i]);
	}
	DBG_PRINT(("\noutESPKey\n"););
	for (i = 0; i< kPGPike_ESPMaximumKeySize; i++) {
		DbgPrint("%X ", transportSA->ikeSA.transform[0].u.ipsec.u.esp.outESPKey[i]);
	}
	DBG_PRINT(("\ninAuthKey\n"););
	for (i = 0; i< kPGPike_AuthMaximumKeySize; i++) {
		DbgPrint("%X ", transportSA->ikeSA.transform[0].u.ipsec.u.esp.inAuthKey[i]);
	}
	DBG_PRINT(("\noutAuthKey\n"););
	for (i = 0; i< kPGPike_AuthMaximumKeySize; i++) {
		DbgPrint("%X ", transportSA->ikeSA.transform[0].u.ipsec.u.esp.outAuthKey[i]);
	}
	DBG_PRINT(("\nESP Cipher: %d\n", transportSA->ikeSA.transform[0].u.ipsec.u.esp.t.cipher););
	DBG_PRINT(("ESP Auth Attr: %d\n", transportSA->ikeSA.transform[0].u.ipsec.u.esp.t.authAttr););
#endif

	} else if (packet->port == UDP_PORT_IKE_NET && !tunnelSA) {
		// copy the chained buffer
		sCopySrcToxForm(packet);
	}

	if (tunnelSA) {
		err = PGPnetIPsecApplySA(adapter, pContext, packet, tunnelSA, FALSE, TRUE);		
	
		if (IsntPGPError(err)) {
			/* XXX this doesn't seem right */
			tunnelSA->packetsSent++;
			DBG_PRINT(("Packets sent: %d\n", tunnelSA->packetsSent););		
			tunnelSA->bytesSent += packet->srcBlockLen /* - something */;
		} else {
			goto fail;
		}
	}

	if (!tunnelSA && !transportSA && packet->port != UDP_PORT_IKE_NET) {
		err = kPGPIPsecError_NoSAFound;
		goto fail;
	}

	DBG_LEAVE(kPGPNetPMPacketSent);
	return kPGPNetPMPacketSent;

fail:
	PMfireErrorEvent(adapter, ipAddress, err);
	DBG_LEAVE(kPGPNetPMPacketDrop);
	return kPGPNetPMPacketDrop;

end:
	DBG_LEAVE(ret);
	return ret;
}

PGPnetPMStatus
sDoTransformIncoming(PGPnetPMContext *pContext, PGPNDIS_PACKET *packet, PVPN_ADAPTER adapter)
{
	PGPUInt32		ipAddress		= htonl(packet->ipAddress);
	PGPUInt32		i				= 0;
	PGPUInt32		spi				= 0;
	PGPUInt32		tunnelIPsrc		= 0;
	PGPNetHostEntry *host 			= PMFindHost(pContext, ipAddress);
	PGPnetKernelSA	*pSA			= 0;
	PGPnetKernelSA	*unused			= 0;
	PGPError		err				= kPGPError_NoErr;
	PGPBoolean		tunnelMode		= FALSE;
	PGPNDIS_PACKET	*pgpPacket		= 0;
	NDIS_STATUS		status;

    DBG_FUNC("sDoTransformIncoming")

    DBG_ENTER();

	/* retrieve the outermost spi (tunnelIPsrc ignored) */
	err = PGPnetIPsecGetAddrAndSPI(pContext, packet, &tunnelIPsrc, &spi);
	if (IsPGPError(err))
		goto fail;

	if (spi) {
		/* try to find SA */
		if (!(pSA = PMFindSAspi(pContext, ipAddress, spi))) {
			/* No SA found with this SPI
			 * Try to find another SA, and possibly
			 * acquire a new SA. Must drop this packet, 
			 * since there isn't anyway I can decrypt it.
			 */
			sLookHardForSA(pContext, 
				adapter, 
				host, 
				ipAddress, 
				packet->port,
				packet,
				0,
				&pSA,
				&unused,
				TRUE);
			/* silent drop */
			DBG_LEAVE(kPGPNetPMPacketDrop);
			return kPGPNetPMPacketDrop;
		}
		if (host && host->hostType == kPGPnetSecureGateway) {
			/* tunnels require phaseII */
			tunnelMode = TRUE;
		} else {
			tunnelMode = FALSE;
		}

		/* Ok, time to apply SA */
		err = PGPnetIPsecApplySA(adapter, pContext, packet, pSA, TRUE, tunnelMode);
		if (IsPGPError(err)) {
			DBG_PRINT(("Returning Drop: PGPnetIPsecApplySA error: %d\n", err););
			goto fail;
		}
	} else {
		if (packet->port == UDP_PORT_IKE_NET) {
			sCopySrcToxForm(packet);
		} else {
			/* Ok the packet did not have an SPI. But it should, since we can't get here unless we
			* have an SA for the machine. Although it may have been deleted in the meantime
			so check again. Should it? only if it came from a configured host. If it did, then we start an SA acquisition.
			if we already have an SA, which is the most likely case, then what?
			*/
			if (!(pSA = PMFindSAspi(pContext, ipAddress, spi))) {
				/* No SA found with this SPI
				* Try to find another SA, and possibly
				* acquire a new SA. Must drop this packet, 
				* since there isn't anyway I can decrypt it.
				*/
				sLookHardForSA(pContext,
					adapter,
					host,
					ipAddress,
					packet->port,
					packet,
					0,
					&pSA,
					&unused,
					TRUE);
				/* silent drop */
				DBG_LEAVE(kPGPNetPMPacketDrop);
				return kPGPNetPMPacketDrop;
			} else {
				err = kPGPIPsecError_NoSPIFound;
				goto fail;
			}
		}
	}
	
	/* phase II */
	if (tunnelMode) {
		DBG_PRINT(("Entering Phase II\n"););

#define ETHERNET_HDRLENGTH	14

		if (pgpIsFragmented(packet->xformBlock + ETHERNET_HDRLENGTH)) {

			// Allocate the pgpPacket
			pgpPacket = PGPNdisPacketAllocWithSrcPacket(&status, adapter);

			if (status != NDIS_STATUS_SUCCESS)
			{
				DBG_LEAVE(kPGPNetPMPacketDrop);
				return kPGPNetPMPacketDrop;
			}

			sCopyPacketXformToSrc(packet /*src*/, pgpPacket, adapter);

			status = TransformAndIndicate(adapter, pgpPacket);
			if (status == NDIS_STATUS_SUCCESS)
			{
				PGPNdisPacketFreeSrcPacket(adapter, pgpPacket);
			}

			return kPGPNetPMPacketDrop;
		}

		err = PGPnetIPsecGetAddrAndSPI(pContext, packet, &tunnelIPsrc, &spi); 
		if (IsPGPError(err)) {
			goto fail;
		}

		/* 
		 * If we don't find an SPI, that's fine. It just means that
		 * was a normal IP packet within the tunnel, rather than
		 * an IPSEC packet
		 */
		if (spi) {
			if (!(pSA = PMFindSAspi(pContext, tunnelIPsrc, spi))) {			
				err = kPGPIPsecError_NoSAFound;
				goto fail;
			}
			tunnelMode = FALSE;
			err = PGPnetIPsecApplySA(adapter, pContext, packet, pSA, TRUE, tunnelMode);
			if (IsPGPError(err)) {
				DBG_PRINT(("Returning Drop: PGPnetIPsecApplySA error: %d\n", err););
				goto fail;
			}
		}
	}

#if/*def*/ 0 /*DBG*/
	/* write out SA info */
	DBG_PRINT(("inESPKey\n"););
	for (i = 0; i< kPGPike_ESPMaximumKeySize; i++) {
		DbgPrint("%X ", pSA->ikeSA.transform[0].u.ipsec.u.esp.inESPKey[i]);
	}
	DBG_PRINT(("\noutESPKey\n"););
	for (i = 0; i< kPGPike_ESPMaximumKeySize; i++) {
		DbgPrint("%X ", pSA->ikeSA.transform[0].u.ipsec.u.esp.outESPKey[i]);
	}
	DBG_PRINT(("\ninAuthKey\n"););
	for (i = 0; i< kPGPike_AuthMaximumKeySize; i++) {
		DbgPrint("%X ", pSA->ikeSA.transform[0].u.ipsec.u.esp.inAuthKey[i]);
	}
	DBG_PRINT(("\noutAuthKey\n"););
	for (i = 0; i< kPGPike_AuthMaximumKeySize; i++) {
		DbgPrint("%X ", pSA->ikeSA.transform[0].u.ipsec.u.esp.outAuthKey[i]);
	}
	DBG_PRINT(("\nESP Cipher: %d\n", pSA->ikeSA.transform[0].u.ipsec.u.esp.t.cipher););
	DBG_PRINT(("ESP Auth Attr: %d\n", pSA->ikeSA.transform[0].u.ipsec.u.esp.t.authAttr););
#endif

	DBG_LEAVE(kPGPNetPMPacketSent);
	return kPGPNetPMPacketSent;

fail:
	PMfireErrorEvent(adapter, ipAddress, err);
	DBG_LEAVE(kPGPNetPMPacketDrop);
	return kPGPNetPMPacketDrop;
}

void
sCopySrcToxForm(PGPNDIS_PACKET *start)
{
	PGPNDIS_PACKET *current = start;
	PGPBoolean moreLinks = TRUE;

	DBG_FUNC("sCopySrcToxForm")

	DBG_ENTER();

	if (!current->firstSrcBlock)
		return;

	do {
		DBG_PRINT(("current->srcBlockLen: %d\n", current->srcBlockLen););
		
		pgpCopyMemory(current->srcBlock /*src*/, current->xformBlock, current->srcBlockLen);
		current->xformBlockLen = current->srcBlockLen;
		current->firstXformBlock = current->firstSrcBlock;
		current->lastXformBlock = current->lastSrcBlock;

		if (current->lastSrcBlock)
			moreLinks = FALSE;

		current = current->link;
	} while (moreLinks);

	DBG_LEAVE(0);
}

void
sCopyPacketXformToSrc(const PGPNDIS_PACKET *source, PGPNDIS_PACKET *dest, PVPN_ADAPTER adapter)
{
	DBG_FUNC("sCopyPacketXformToSrc")

	DBG_ENTER();

	if (!source->firstXformBlock)
		return;

	if (!source->lastXformBlock) {
		DBG_BREAK();
	}

	DBG_PRINT(("source->xformBlockLen: %d\n", source->xformBlockLen););
		
	pgpCopyMemory(source->xformBlock /*src*/, dest->srcBlock, source->xformBlockLen);
	dest->srcBlockLen = source->xformBlockLen;
	dest->firstSrcBlock = TRUE;
	dest->lastSrcBlock = FALSE;

	DBG_LEAVE(0);
}

PGPnetPMStatus
sLookHardForSA(PGPnetPMContext *pContext, 
			   PVPN_ADAPTER adapter,
			   PGPNetHostEntry *host,
			   PGPUInt32 ipAddress,
			   PGPUInt16 port,
			   void * ipBuffer,
			   PGPUInt32 ipBufferSize,
			   PGPnetKernelSA **ppTransport,
			   PGPnetKernelSA **ppTunnel,
			   PGPBoolean incoming)
{
	PGPnetPMStatus ret = kPGPNetPMPacketDrop;
	PGPnetPMStatus tunnelRet, transportRet;
	PGPNetHostEntry *secureGW = 0;

	if (host) {
		if (host->hostType == kPGPnetSecureGateway && 
			port != UDP_PORT_IKE_NET &&
			!incoming) {
			/* we always talk to Gateways in Tunnel mode */
			/* this should be right, but doesn't work with GVPN NT */
			ret = sAcquireSA(pContext, 
				host->ipAddress, 
				host->ipAddress,
				host->ipMask,
				incoming,
				ipBuffer,
				ipBufferSize,
				adapter,
				ppTunnel);

			return ret;
		}

		if (host->hostType == kPGPnetSecureHost && 
			host->childOf == -1 && 
			port != UDP_PORT_IKE_NET) {
			/* we need only the transport SA */
			ret = sAcquireSA(pContext,
				ipAddress,
				0,
				0,
				incoming,
				ipBuffer,
				ipBufferSize,
				adapter,
				ppTransport);
			return ret;
		}

		if (host->hostType == kPGPnetInsecureHost && host->childOf != -1) {
			/* we need only the tunnel SA */
			secureGW = &(pContext->pKernelHostList[host->childOf]);
			ret = sAcquireSA(pContext, 
				secureGW->ipAddress,
				host->ipAddress,
				host->ipMask,
				incoming,
				ipBuffer,
				ipBufferSize,
				adapter,
				ppTunnel);
			return ret;
		}

		if (host->hostType == kPGPnetSecureHost && host->childOf != -1) {
			/* we need transport SA and tunnel SA */

			/* first get tunnel SA */
			secureGW = &(pContext->pKernelHostList[host->childOf]);

			tunnelRet = sAcquireSA(pContext, 
				secureGW->ipAddress,
				host->ipAddress,
				host->ipMask,
				incoming,
				ipBuffer,
				ipBufferSize,
				adapter,
				ppTunnel);

			if (port == UDP_PORT_IKE_NET) {
				return tunnelRet;
			}

			/* next get the transport SA */
			transportRet = sAcquireSA(pContext, 
				ipAddress,
				0,
				0,
				incoming,
				ipBuffer,
				ipBufferSize,
				adapter,
				ppTransport);

			if (tunnelRet == kPGPNetPMPacketEncrypt && 
				transportRet == kPGPNetPMPacketEncrypt) {
				/* both SAs are ready to go */
				ret = kPGPNetPMPacketEncrypt;
			} else {
				/* something isn't ready yet */
				ret = kPGPNetPMPacketWaiting;
			}
			return ret;
		}
	}

	if (pContext->pConfig->bAllowUnconfigHost && 
		!pContext->pConfig->bAttemptUnconfigHost) {
		/* 
		 * There may be an SA for this host that was initiated by the peer.
		 * So we need to look for an SA, before sending it in the clear
		 */
		if ((*ppTransport = PMFindSA(pContext, ipAddress, 0, incoming))) {
			return kPGPNetPMPacketEncrypt;
		} else {
			return kPGPNetPMPacketClear;
		}
	}

	if (pContext->pConfig->bAllowUnconfigHost && 
		pContext->pConfig->bAttemptUnconfigHost && 
		port != UDP_PORT_IKE_NET) {

		ret = sAcquireSA(pContext, 
			ipAddress,
			0,
			0,
			incoming,
			ipBuffer,
			ipBufferSize,
			adapter,
			ppTransport);

		return ret;
	}

	if (ret != kPGPNetPMPacketEncrypt && port == UDP_PORT_IKE_NET)
		ret = kPGPNetPMPacketClear;

	return ret;
}
	