#include "version.h"

#include "vpn.h"
#include "vpndbg.h"

#include "ipdef.h"

#include "pgpNetKernel.h"

#include "pgpIPheader.h"

#ifdef PM_EMULATION
#include "dummypm.h"
#endif

#ifdef CHICAGO
extern unsigned long PgpKernelGetSystemTime(void);
#else
unsigned long PgpKernelGetSystemTime(void)
{
  LARGE_INTEGER t;
  KeQuerySystemTime(&t);
  return ((unsigned long)t.LowPart/10000);
}
#endif

void PacketEnqueue(
    PVPN_ADAPTER adapter,
    PPGPNDIS_PACKET *packet_list,
    PPGPNDIS_PACKET packet)
{
    NdisAcquireSpinLock(&adapter->general_lock);

    if (*packet_list == (PPGPNDIS_PACKET) NULL) {
        packet->link = packet;
    } else {
        packet->link = (*packet_list)->link;
        (*packet_list)->link = packet;
    }
    *packet_list = packet;

    NdisReleaseSpinLock(&adapter->general_lock);
}

PPGPNDIS_PACKET PacketDequeue(
    PVPN_ADAPTER adapter,
    PPGPNDIS_PACKET *packet_list)
{
	PPGPNDIS_PACKET packet;

    NdisAcquireSpinLock(&adapter->general_lock);

    if (*packet_list == (PPGPNDIS_PACKET) NULL) {
        packet = (PPGPNDIS_PACKET) NULL;
    } else {
		packet = (*packet_list)->link;
        if (packet == *packet_list) {
            *packet_list = (PPGPNDIS_PACKET) NULL;
        } else {
            (*packet_list)->link = packet->link;
        }
    }

    NdisReleaseSpinLock(&adapter->general_lock);

    return packet;
}

PPGPNDIS_PACKET PacketRemoveByXformPacket(
		PVPN_ADAPTER		adapter,
		PPGPNDIS_PACKET		*packet_list,
		PNDIS_PACKET		packet)
{
	PPGPNDIS_PACKET pgpPacket;

	NdisAcquireSpinLock(&adapter->general_lock);

	if ( (pgpPacket = *packet_list) != NULL ) 
	{
		do 
		{
			PPGPNDIS_PACKET next = pgpPacket->link;
			if (next->xformPacket == packet) 
			{
				if (next == pgpPacket)
					*packet_list = NULL;
				else
				{
					pgpPacket->link = next->link;
					if (*packet_list == next)
						*packet_list = pgpPacket;
				}
				NdisReleaseSpinLock(&adapter->general_lock);
				return next;
			}
			pgpPacket = next;
		} while (pgpPacket != *packet_list);
	};
	
	NdisReleaseSpinLock(&adapter->general_lock);
	
	return NULL;

}

PPGPNDIS_PACKET PacketRemoveBySrcPacket(
		PVPN_ADAPTER		adapter,
		PPGPNDIS_PACKET		*packet_list,
		PNDIS_PACKET		packet)
{
	PPGPNDIS_PACKET pgpPacket;

	NdisAcquireSpinLock(&adapter->general_lock);

	if ( (pgpPacket = *packet_list) != NULL ) 
	{
		do 
		{
			PPGPNDIS_PACKET next = pgpPacket->link;
			if (next->srcPacket == packet) 
			{
				if (next == pgpPacket)
					*packet_list = NULL;
				else
				{
					pgpPacket->link = next->link;
					if (*packet_list == next)
						*packet_list = pgpPacket;
				}
				NdisReleaseSpinLock(&adapter->general_lock);
				return next;
			}
			pgpPacket = next;
		} while (pgpPacket != *packet_list);
	};
	
	NdisReleaseSpinLock(&adapter->general_lock);
	
	return NULL;

}

NDIS_STATUS AllocatePGPnetPacketPool(PVPN_ADAPTER adapter)
{
	NDIS_STATUS status;

	int i;
	UINT block_size;
	NDIS_PHYSICAL_ADDRESS HighestAcceptableAddress = NDIS_PHYSICAL_ADDRESS_CONST(-1, -1);

	DBG_FUNC("AllocatePGPnetPacketPool")
	
	DBG_ENTER();

	adapter->free_packet_list	= (PPGPNDIS_PACKET)NULL;
	adapter->sent_ipsecpacket_list	= (PPGPNDIS_PACKET)NULL;
	adapter->sent_plainpacket_list = (PPGPNDIS_PACKET)NULL;
	adapter->incoming_plaintransferComplete_wait_list	= (PPGPNDIS_PACKET)NULL;
	adapter->incoming_ipsectransferComplete_wait_list	= (PPGPNDIS_PACKET)NULL;
	adapter->incoming_indicateComplete_wait_list = (PPGPNDIS_PACKET)NULL;
	adapter->incoming_fragment_indicateComplete_wait_list = (PPGPNDIS_PACKET)NULL;

	adapter->outgoing_multiple_ipsecpacket_list = (PPGPNDIS_PACKET)NULL;

	adapter->free_packet_head_list = (PPGPNDIS_PACKET_HEAD)NULL;
	adapter->outgoing_packet_head_list = (PPGPNDIS_PACKET_HEAD)NULL;
	adapter->incoming_packet_head_list = (PPGPNDIS_PACKET_HEAD)NULL;
	adapter->sent_packet_head_list = (PPGPNDIS_PACKET_HEAD)NULL;

	adapter->free_request_list	= (PPGPNDIS_REQUEST)NULL;
	adapter->send_request_list	= (PPGPNDIS_REQUEST)NULL;
	adapter->wait_request_list	= (PPGPNDIS_REQUEST)NULL;

	block_size = BLOCK_SIZE;					// FIX

	for (i = 0; i < PACKET_PENDING_MAXIMUM; i++) {
		PPGPNDIS_PACKET packet;
		status = NdisAllocateMemory(&packet, sizeof(PGPNDIS_PACKET), 0, HighestAcceptableAddress);
		if (status != NDIS_STATUS_SUCCESS)
		{
			DBG_LEAVE(status);
			return status;
		}
		NdisZeroMemory(packet, sizeof(PGPNDIS_PACKET));
		status = NdisAllocateMemory(&packet->srcBlock, block_size, 0, HighestAcceptableAddress);
		if (status != NDIS_STATUS_SUCCESS)
		{
			DBG_LEAVE(status);
			return status;
		}
		status = NdisAllocateMemory(&packet->xformBlock, block_size, 0, HighestAcceptableAddress);
		if (status != NDIS_STATUS_SUCCESS)
		{
			DBG_LEAVE(status);
			return status;
		}
		PacketEnqueue(adapter, &adapter->free_packet_list, packet);
	}

	for (i = 0; i < PACKET_HEAD_PENDING_MAXIMUM; i++) {
		PPGPNDIS_PACKET_HEAD packetHead;
		status = NdisAllocateMemory(&packetHead, sizeof(PGPNDIS_PACKET_HEAD), 0, HighestAcceptableAddress);
		if (status != NDIS_STATUS_SUCCESS)
		{
			DBG_LEAVE(status);
			return status;
		}
		NdisZeroMemory(packetHead, sizeof(PGPNDIS_PACKET_HEAD));
		PacketHeadEnqueue(adapter, &adapter->free_packet_head_list, packetHead);
	}

	for (i = 0; i < REQUEST_PENDING_MAXIMUM; i++) {
		PPGPNDIS_REQUEST request;
		status = NdisAllocateMemory(&request, sizeof(PGPNDIS_REQUEST), 0, HighestAcceptableAddress);
		if (status != NDIS_STATUS_SUCCESS)
		{
			DBG_LEAVE(status);
			return status;
		}
		NdisZeroMemory(request, sizeof(PGPNDIS_REQUEST));

		RequestEnqueue(adapter, &adapter->free_request_list, request);
	}


	DBG_LEAVE(status);

	return status;
}

PPGPNDIS_PACKET PGPNdisPacketAllocWithBindingContext(
    PNDIS_STATUS status,
    PVPN_ADAPTER adapter)
{
    PPGPNDIS_PACKET packet;

	DBG_FUNC("PGPNdisPacketAllocWithBindingContext")

	packet = (PPGPNDIS_PACKET) PacketDequeue( adapter, &adapter->free_packet_list);

	if (packet == (PPGPNDIS_PACKET) NULL) {
		DBG_PRINT(("!!!!! FATAL!!! PreAllocated PGPPacket Exhausted!!!\n"););
		ASSERT(FALSE);
		*status = NDIS_STATUS_RESOURCES;
		return NULL;
	}

	*status = NDIS_STATUS_SUCCESS;
	packet->Binding = NULL;
	packet->link = NULL;

    return packet;
}

PPGPNDIS_PACKET PGPNdisPacketAllocWithXformPacket(
    PNDIS_STATUS status,
    PVPN_ADAPTER adapter)
{
    PPGPNDIS_PACKET packet;

	DBG_FUNC("PGPNdisPacketAllocWithXformPacket")

	packet = (PPGPNDIS_PACKET) PacketDequeue( adapter, &adapter->free_packet_list);

	if (packet == (PPGPNDIS_PACKET) NULL) {
		DBG_PRINT(("!!!!! FATAL!!! PreAllocated PGPPacket Exhausted!!!\n"););
		ASSERT(FALSE);
		*status = NDIS_STATUS_RESOURCES;
		return NULL;
	}

	packet->srcPacket = NULL;
	packet->srcBuffer = NULL;

	NdisAllocatePacket(status, &packet->xformPacket, adapter->packet_pool);
    if (*status != NDIS_STATUS_SUCCESS) {
		PacketEnqueue(adapter, &adapter->free_packet_list, packet);
		DBG_PRINT(("!!!!! FATAL!!! PreAllocated Packet Pool Exhausted!!!\n"););
		ASSERT(FALSE);
		return NULL;
	}

	NdisAllocateBuffer(status, &packet->xformBuffer, adapter->buffer_pool, packet->xformBlock, BLOCK_SIZE);

	if (*status != NDIS_STATUS_SUCCESS) {
		NdisFreePacket(packet->xformPacket);
		PacketEnqueue(adapter, &adapter->free_packet_list, packet);
		DBG_PRINT(("!!!!! FATAL!!! PreAllocated Buffer Pool Exhausted!!!\n"););
		ASSERT(FALSE);
		return NULL;
	}

    NdisChainBufferAtFront(packet->xformPacket, packet->xformBuffer);

	packet->link = NULL;

	packet->firstSrcBlock = 0;
	packet->firstXformBlock = 0;
	packet->lastSrcBlock = 0;
	packet->lastXformBlock = 0;

    return packet;
}

PPGPNDIS_PACKET PGPNdisPacketAllocWithSrcPacket(
    PNDIS_STATUS status,
    PVPN_ADAPTER adapter)
{
    PPGPNDIS_PACKET packet;

	DBG_FUNC("PGPNdisPacketAllocSrcPacket")

	DBG_ENTER();

	*status = NDIS_STATUS_SUCCESS;

	packet = (PPGPNDIS_PACKET) PacketDequeue( adapter, &adapter->free_packet_list);

	if (packet == (PPGPNDIS_PACKET) NULL) {
		DBG_PRINT(("!!!!! FATAL!!! PreAllocated PGPPacket Exhausted!!!\n"););
		ASSERT(FALSE);
		*status = NDIS_STATUS_RESOURCES;
		return NULL;
	}

	NdisAllocatePacket(status, &packet->srcPacket, adapter->packet_pool);
    if (*status != NDIS_STATUS_SUCCESS) {
		PacketEnqueue(adapter, &adapter->free_packet_list, packet);
		DBG_PRINT(("!!!!! FATAL!!! PreAllocated Packet Pool Exhausted!!!\n"););
		ASSERT(FALSE);
		return NULL;
	}

	NdisAllocateBuffer(status, &packet->srcBuffer, adapter->buffer_pool, ((UCHAR*)packet->srcBlock + adapter->eth_hdr_len), BLOCK_SIZE);

	if (*status != NDIS_STATUS_SUCCESS) {
		NdisFreePacket(packet->srcPacket);
		PacketEnqueue(adapter, &adapter->free_packet_list, packet);
		DBG_PRINT(("!!!!! FATAL!!! PreAllocated Buffer Pool Exhausted!!!\n"););
		ASSERT(FALSE);
		return NULL;
	}

    NdisChainBufferAtFront(packet->srcPacket, packet->srcBuffer);


	packet->xformPacket = NULL;
	packet->xformBuffer = NULL;

	packet->link = NULL;

	packet->firstSrcBlock = 0;
	packet->firstXformBlock = 0;
	packet->lastSrcBlock = 0;
	packet->lastXformBlock = 0;

	DBG_LEAVE(0);

    return packet;
}

VOID PGPNdisPacketFree(
		PVPN_ADAPTER adapter,
		PPGPNDIS_PACKET packet)
{
	PNDIS_BUFFER buffer = NULL;

	DBG_FUNC("PGPNdisPacketFree")

	DBG_ENTER();

	if (packet == NULL)
	{
		DBG_LEAVE(1);
		return;
	}


	if (packet->xformPacket)
	{
		NdisUnchainBufferAtFront(packet->xformPacket, &buffer);

		if (buffer)
		{
			//NdisFreeBuffer(packet->xformBuffer); //?
			NdisFreeBuffer(buffer);
		}
		packet->xformBuffer = NULL;
		
		NdisFreePacket(packet->xformPacket);
		packet->xformPacket = (PNDIS_PACKET)NULL;
	}

	packet->fragmentNumber = 0;

    PacketEnqueue(adapter, &adapter->free_packet_list, packet);

	DBG_LEAVE(0);
}

VOID PGPNdisPacketFreeWithBindingContext(
		PVPN_ADAPTER adapter,
		PPGPNDIS_PACKET packet)
{
	PNDIS_BUFFER buffer = NULL;

	DBG_FUNC("PGPNdisPacketFreeWithBindingContext")

	DBG_ENTER();
	if (packet == NULL)
	{
		DBG_LEAVE(1);
		return;
	}

	packet->fragmentNumber = 0;

    PacketEnqueue(adapter, &adapter->free_packet_list, packet);

	DBG_LEAVE(0);
}

VOID PGPNdisPacketFreeSrcPacket(
		PVPN_ADAPTER adapter,
		PPGPNDIS_PACKET packet)
{
	PNDIS_BUFFER buffer = NULL;

	DBG_FUNC("PGPNdisPacketFreeSrcPacket")

	DBG_ENTER();
	if (packet == NULL)
	{
		DBG_LEAVE(1);
		return;
	}

	if (packet->srcPacket)
	{
		NdisUnchainBufferAtFront(packet->srcPacket, &buffer);

		if (buffer)
		{
			//NdisFreeBuffer(packet->xformBuffer); //?
			NdisFreeBuffer(buffer);
		}
		packet->srcBuffer = NULL;
		
		NdisFreePacket(packet->srcPacket);
		packet->srcPacket = (PNDIS_PACKET)NULL;
	}

	packet->fragmentNumber = 0;

    PacketEnqueue(adapter, &adapter->free_packet_list, packet);

	DBG_LEAVE(0);
}

VOID PGPCopyPacketToBlock(
		PNDIS_PACKET	packet,
		PUCHAR			block,
		PUINT			BytesCopied)
{

	UINT			numNdisBuffer;
	PNDIS_BUFFER	CurrentBuffer;
	PVOID			CurrentBlock;
	UINT			CurrentLength;

	DBG_FUNC("PGPCopyPacketToBlock")

	*BytesCopied = 0;
	NdisQueryPacket(packet, NULL, &numNdisBuffer, &CurrentBuffer, NULL);

	if (numNdisBuffer == 0)
		return;

	NdisQueryBuffer(CurrentBuffer, &CurrentBlock, &CurrentLength);

	while(CurrentBuffer != NULL)
	{
		NdisMoveMemory(block, CurrentBlock, CurrentLength);
		block = (PUCHAR)block + CurrentLength;
		*BytesCopied = *BytesCopied + CurrentLength;


		NdisGetNextBuffer(CurrentBuffer, &CurrentBuffer);
		if (CurrentBuffer == NULL)
			break;
		NdisQueryBuffer(CurrentBuffer, &CurrentBlock, &CurrentLength);
	}
}

VOID PGPnetAdjustTransferCompletePacket(PPGPNDIS_PACKET pgpPacket)
{
	PGPCopyPacketToBlock(pgpPacket->srcPacket, pgpPacket->xformBlock, &pgpPacket->xformBlockLen);
	NdisMoveMemory(pgpPacket->srcBlock, pgpPacket->xformBlock, pgpPacket->xformBlockLen);
	pgpPacket->srcBlockLen = pgpPacket->xformBlockLen;
	pgpPacket->xformBlockLen = 0;
}

VOID PGPnetAdjustPacket(PPGPNDIS_PACKET packet)
{
	PNDIS_BUFFER ndisBuffer;

	NdisUnchainBufferAtFront(packet->xformPacket, &ndisBuffer);
	NdisAdjustBufferLength(ndisBuffer, packet->xformBlockLen);
	NdisChainBufferAtFront(packet->xformPacket, ndisBuffer);
}

void RequestEnqueue(
    PVPN_ADAPTER adapter,
    PPGPNDIS_REQUEST *request_list,
    PPGPNDIS_REQUEST request)
{
    NdisAcquireSpinLock(&adapter->general_lock);

    if (*request_list == (PPGPNDIS_REQUEST) NULL) {
        request->link = request;
    } else {
        request->link = (*request_list)->link;
        (*request_list)->link = request;
    }
    *request_list = request;

    NdisReleaseSpinLock(&adapter->general_lock);
}

PPGPNDIS_REQUEST RequestDequeue(
    PVPN_ADAPTER adapter,
    PPGPNDIS_REQUEST *reqeust_list)
{
	PPGPNDIS_REQUEST request;

    NdisAcquireSpinLock(&adapter->general_lock);

    if (*reqeust_list == (PPGPNDIS_REQUEST) NULL) {
        request = (PPGPNDIS_REQUEST) NULL;
    } else {
		request = (*reqeust_list)->link;
        if (request == *reqeust_list) {
            *reqeust_list = (PPGPNDIS_REQUEST) NULL;
        } else {
            (*reqeust_list)->link = request->link;
        }
    }

    NdisReleaseSpinLock(&adapter->general_lock);

    return request;
}

PPGPNDIS_REQUEST RequestRemoveByNdisRequest(
		PVPN_ADAPTER		adapter,
		PPGPNDIS_REQUEST		*reqeust_list,
		PNDIS_REQUEST		packet)
{
	PPGPNDIS_REQUEST pgpRequest;

	NdisAcquireSpinLock(&adapter->general_lock);

	if ( (pgpRequest = *reqeust_list) != NULL ) 
	{
		do 
		{
			PPGPNDIS_REQUEST next = pgpRequest->link;
			if (next->NdisRequest == packet) 
			{
				if (next == pgpRequest)
					*reqeust_list = NULL;
				else
				{
					pgpRequest->link = next->link;
					if (*reqeust_list == next)
						*reqeust_list = pgpRequest;
				}
				NdisReleaseSpinLock(&adapter->general_lock);
				return next;
			}
			pgpRequest = next;
		} while (pgpRequest != *reqeust_list);
	};
	
	NdisReleaseSpinLock(&adapter->general_lock);
	
	return NULL;

}

PPGPNDIS_REQUEST PGPNdisRequestAlloc(
    PNDIS_STATUS status,
    PVPN_ADAPTER adapter)
{
    PPGPNDIS_REQUEST request;

	DBG_FUNC("PGPNdisRequestAlloc")

	request = (PPGPNDIS_REQUEST) RequestDequeue( adapter, &adapter->free_request_list);

	if (request == (PPGPNDIS_REQUEST) NULL) {
		DBG_PRINT(("!!!!! FATAL!!! PreAllocated PGPRequest Exhausted!!!\n"););
		ASSERT(FALSE);
		*status = NDIS_STATUS_RESOURCES;
		return NULL;
	}

	*status = NDIS_STATUS_SUCCESS;
	request->NdisRequest = NULL;
	request->Binding = NULL;

    return request;
}

VOID PGPNdisRequestFree(
		PVPN_ADAPTER adapter,
		PPGPNDIS_REQUEST request)
{

	DBG_FUNC("PGPNdisRequestFree")

	DBG_ENTER();

    RequestEnqueue(adapter, &adapter->free_request_list, request);

	DBG_LEAVE(0);
}

NDIS_STATUS QueueForTransferComplete(
		PVPN_ADAPTER	adapter,
		NDIS_HANDLE		MacReceiveContext,
		PUCHAR			HeaderBuffer,
		UINT			HeaderBufferLength,
		PUCHAR			LookAheadBuffer,
		UINT			LookAheadBufferSize,
		UINT			PacketSize)
{
	DBG_FUNC("QueueForTransferComplete")

	NDIS_STATUS status;
	PPGPNDIS_PACKET		pgpPacket;
	UINT				BytesTransferred;
	PIP_HEADER			ip_header;
	UINT				ip_len;

	DBG_ENTER();

	// If we have to decrypt the packet, we can not indicate it to protocol. Cause it is not complete yet.
	pgpPacket = PGPNdisPacketAllocWithSrcPacket(&status, adapter);

	if (status != NDIS_STATUS_SUCCESS)
	{
		DBG_LEAVE(status);
		return status;
	}

	if (HeaderBuffer != NULL)
		NdisMoveMemory(pgpPacket->srcBlock, HeaderBuffer, adapter->eth_hdr_len);

	pgpPacket->HeaderBuffer = pgpPacket->srcBlock;
	pgpPacket->HeaderBufferSize = adapter->eth_hdr_len;
	pgpPacket->LookaheadBuffer = pgpPacket->srcBlock + adapter->eth_hdr_len;

	ip_header = (PIP_HEADER) ( (UCHAR*)LookAheadBuffer);

    ip_len = ntohs(ip_header->ip_len);

	if (ip_len <= LookAheadBufferSize)
	{
		NdisMoveMemory(pgpPacket->LookaheadBuffer, ip_header, ip_len);
		pgpPacket->srcBlockLen = ip_len + adapter->eth_hdr_len;
		status = NDIS_STATUS_SUCCESS;
		PacketEnqueue(adapter, &adapter->incoming_indicateComplete_wait_list, pgpPacket);

	}
	else
	{
		PacketEnqueue(adapter, &adapter->incoming_ipsectransferComplete_wait_list, pgpPacket);

		NdisTransferData(&status, adapter->NdisBindingHandleToRealMac, MacReceiveContext, 0, //adapter->eth_hdr_len,
			PacketSize, pgpPacket->srcPacket, &BytesTransferred);

		if (status == NDIS_STATUS_PENDING) {
			status = NDIS_STATUS_SUCCESS;
			DBG_LEAVE(status);
			return status;
		}

		pgpPacket = PacketDequeue(adapter, &adapter->incoming_ipsectransferComplete_wait_list);

		if (status == NDIS_STATUS_SUCCESS) {
			PGPnetAdjustTransferCompletePacket(pgpPacket);
			PacketEnqueue(adapter, &adapter->incoming_indicateComplete_wait_list, pgpPacket);
		}
		else {
			PGPNdisPacketFreeSrcPacket(adapter, pgpPacket);
		}
	}
			

	DBG_LEAVE(status);

	return status;

#if 0
	pgpPacket->ipAddress = ntohl(ip_header->ip_src);

	PGPnetPMDoTransform(PGPnetDriver.PolicyManagerHandle, pgpPacket, TRUE); //&srcPGPnetPacket, &dstPGPnetPacket);

	// No need to adjust packet.
	// PGPnetAdjustPacket(pgpPacket);

	NdisIndicateReceive(&status, adapter->NdisBindingContextFromProtocol, MacReceiveContext,
					pgpPacket->xformBlock, pgpPacket->xformBlockLen,
					pgpPacket->xformBlock, pgpPacket->xformBlockLen,
					pgpPacket->xformBlockLen);

	PGPNdisPacketFree(adapter, pgpPacket);

	DBG_LEAVE(status);
#endif
}

NDIS_STATUS TransformAndIndicate(
		PVPN_ADAPTER	adapter,
		PPGPNDIS_PACKET	pgpPacket)
{

	DBG_FUNC("TransformAndIndicate")

	NDIS_STATUS status;
	PIP_HEADER			ip_header;
	PUDP_HEADER			udp_header = 0; /* EML 04/05/99 */
	UINT				ip_len;
	PGPnetPMStatus		pmstatus;
	PPGPNDIS_PACKET_HEAD	packetHead;
	BOOLEAN					newHead = FALSE;
	DBG_ENTER()

	status = NDIS_STATUS_SUCCESS;

	pgpPacket->HeaderBuffer = pgpPacket->srcBlock;
	pgpPacket->HeaderBufferSize = adapter->eth_hdr_len;
	pgpPacket->LookaheadBuffer = pgpPacket->srcBlock + adapter->eth_hdr_len;

	ip_header = (PIP_HEADER) ( (UCHAR*)pgpPacket->LookaheadBuffer );

    ip_len = ntohs(ip_header->ip_len);

	pgpPacket->srcBlockLen = ip_len + adapter->eth_hdr_len;

	pgpPacket->ipAddress = ntohl(ip_header->ip_src);

	/* Start EML 05/04/99 */
	if (ip_header->ip_prot == PROTOCOL_UDP)
	{
		udp_header = (PUDP_HEADER) ( (UCHAR*)ip_header + sizeof(IP_HEADER) );
	}

	pgpPacket->port = udp_header ? udp_header->dest_port : 0;
	/* End EML */

	pmstatus = PGPnetPMNeedTransformLight(PGPnetDriver.PolicyManagerHandle,
		ip_header->ip_src,
		TRUE,
		adapter);

	if ( kPGPNetPMPacketEncrypt == pmstatus )
	{
		// Now we have the whole secured incoming packet.
		NdisMoveMemory(pgpPacket->xformBlock, pgpPacket->srcBlock, adapter->eth_hdr_len);

		pgpPacket->offset = ntohs(ip_header->ip_foff & IP_OFFSET) << 3;

		if (pgpPacket->offset == 0)
			pgpPacket->firstSrcBlock = TRUE;

		// Check to see if it's in the incoming fragment list.
		packetHead = PacketHeadListQuery(adapter,
					&adapter->incoming_packet_head_list,
					ip_header->ip_id,
					pgpPacket->ipAddress);
		// If there is no incoming fragment list for this one. Create one.
		if (packetHead == NULL)
		{
			newHead = TRUE;
			packetHead = PGPNdisPacketHeadAlloc(&status, adapter);
		}

		if (status != NDIS_STATUS_SUCCESS)
			goto failout;

		// Add timestamp, update head information.
		if (packetHead->id == 0)
		{
			// Initialize packetHead
			packetHead->ipAddress = pgpPacket->ipAddress;
			packetHead->id = ip_header->ip_id;
			packetHead->timeStamp = PgpKernelGetSystemTime();
		}

		if (packetHead->numFragments ==0)
			packetHead->accumulatedLength = htons(ip_header->ip_len);
		else
			packetHead->accumulatedLength += htons(ip_header->ip_len) - IP_HEADER_SIZE;
		packetHead->numFragments++;
		
		if (IP_LAST_FRAGMENT(ip_header->ip_foff))
		{
			ASSERT(packetHead->totalLength == 0);
			pgpPacket->lastSrcBlock = TRUE;
			packetHead->totalLength = htons(ip_header->ip_len) + pgpPacket->offset;
		}

		// Insert this pgpPacket to the packet list 
		InsertPGPNdisPacket(adapter, packetHead, pgpPacket);

		// Check status, if finished fire up the send sequence.

		if ((packetHead->totalLength) && (packetHead->totalLength == packetHead->accumulatedLength))
		{
			// Got them all, send them all.
			PGPnetPMStatus pm_status;
			PPGPNDIS_PACKET extraPacket;

			extraPacket = PGPNdisPacketAllocWithSrcPacket(&status, adapter);

			AppendPGPNdisPacket(adapter, packetHead, extraPacket);
			if ( !(packetHead->link)->lastSrcBlock )
			{
				// More fragment. Adjust packet length.
				PIP_HEADER first_ip_hdr;
				PUCHAR		first_srcBlock;


				first_srcBlock = (packetHead->link)->srcBlock;
				first_ip_hdr = (PIP_HEADER)(first_srcBlock + ETHER_HEADER_SIZE);

				first_ip_hdr->ip_len = htons(packetHead->totalLength);

			}
		

			pm_status = PGPnetPMDoTransform(PGPnetDriver.PolicyManagerHandle,
								packetHead->link,
								TRUE,
								adapter);

			if (pm_status != kPGPNetPMPacketSent)
			{
				DBG_PRINT(("!!!!! Yellow Alert! PGPnetPMDoTransform Error!\n"););
				PGPNdisPacketHeadFreeList(adapter, packetHead, FALSE);
				PacketHeadListRemove(adapter, &adapter->incoming_packet_head_list, packetHead);
				PGPNdisPacketHeadFree(adapter, packetHead);

				status = NDIS_STATUS_FAILURE;
				goto failout;
			}
			ProtocolIndicatePackets(adapter, packetHead);


		}
		else
		{
			// Not finished, add to the incoming list
			if (newHead)
				PacketHeadEnqueue(adapter, &adapter->incoming_packet_head_list, packetHead);

		}

		status = NDIS_STATUS_PENDING;

		goto bailout;

#if 0
		PGPnetPMStatus		pm_status;
		pm_status = PGPnetPMDoTransform(PGPnetDriver.PolicyManagerHandle,
			pgpPacket,
			TRUE,
			adapter);
		if (pm_status != kPGPNetPMPacketSent)
		{
			DBG_PRINT(("!!!!! Yellow Alert! PGPnetPMDoTransform Error!\n"););
			status = NDIS_STATUS_FAILURE;
			goto failout;
		}
#endif

	}
	else if ( kPGPNetPMPacketClear == pmstatus )
	{
		CopySrcBlockToXformBlock(pgpPacket);
		IndicateReceiveClear(adapter, pgpPacket);
		goto bailout;
	}
	else
	{
		// Shouldn't get here in the first place. But this did happen.
		DBG_PRINT(("!!!!!FATAL!!!!!, Invalid status!\n"););
		//ASSERT(FALSE);
		status = NDIS_STATUS_FAILURE;
		goto failout;
	}

	pgpPacket->HeaderBuffer = pgpPacket->xformBlock;
	pgpPacket->HeaderBufferSize = adapter->eth_hdr_len;
	pgpPacket->LookaheadBuffer = pgpPacket->xformBlock + adapter->eth_hdr_len;

	ip_header = (PIP_HEADER) ( (UCHAR*)pgpPacket->LookaheadBuffer );

    ip_len = ntohs(ip_header->ip_len);


	//ip_len = pgpPacket->xformBlockLen - adapter->eth_hdr_len;

	ProtocolIndicateReceive(adapter,
							pgpPacket->srcPacket,
							pgpPacket->HeaderBuffer,
							pgpPacket->HeaderBufferSize,
							pgpPacket->LookaheadBuffer,
							ip_len,
							ip_len);

#ifdef INCOMING_FRAGMENT
	if ( pgpPacket->xformBlockLen <= MAX_ETHER_FRAME_SIZE - MAX_IPSEC_PACKETSIZE_INCREASE)
	{
		NdisIndicateReceive(&status, adapter->NdisBindingContextFromProtocol, pgpPacket->srcPacket,
				pgpPacket->HeaderBuffer, pgpPacket->HeaderBufferSize,
				pgpPacket->LookaheadBuffer, ip_len,
				ip_len);
	}
	else
	{
		FragmentAndIndicate(adapter, pgpPacket);
		adapter->receive_fragmented = TRUE;
	}
#endif
bailout:
failout:

	DBG_LEAVE(status); // We do care about the status
	return status;
}

USHORT iphdr_cksum(USHORT *iph)
{
    USHORT i;
    ULONG sum;

    i = 0;
    sum = 0;

    while(i++ < (sizeof(IP_HEADER)/2))
        sum += *iph++;

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    sum = ~sum;

    return (USHORT)sum;
}

void CopySrcBlockToXformBlock(PGPNDIS_PACKET *packet)
{

	if (packet->srcBlockLen)
		NdisMoveMemory(packet->xformBlock, packet->srcBlock, packet->srcBlockLen);

	packet->xformBlockLen = packet->srcBlockLen;

}

void GetIPAddressFromARP(PVPN_ADAPTER adapter, PVOID block)
{
	PARP_HEADER arp_header;

	arp_header = (PARP_HEADER)block;

	if ( (arp_header->arp_src_ip_addr == arp_header->arp_dst_ip_addr) &&
		arp_header->arp_src_ip_addr != 0L)
	{
		adapter->ip_address = arp_header->arp_src_ip_addr;
	}

}

/*******************************************************************************
// New fragmentation related routines.
*******************************************************************************/

void PacketHeadEnqueue(
    PVPN_ADAPTER adapter,
    PPGPNDIS_PACKET_HEAD *packet_head_list,
    PPGPNDIS_PACKET_HEAD packetHead)
{
    NdisAcquireSpinLock(&adapter->general_lock);

    if (*packet_head_list == (PPGPNDIS_PACKET_HEAD) NULL) {
        packetHead->next = packetHead;
    } else {
        packetHead->next = (*packet_head_list)->next;
        (*packet_head_list)->next = packetHead;
    }
    *packet_head_list = packetHead;

    NdisReleaseSpinLock(&adapter->general_lock);
}

PPGPNDIS_PACKET_HEAD PacketHeadDequeue(
    PVPN_ADAPTER adapter,
    PPGPNDIS_PACKET_HEAD *packet_head_list)
{
	PPGPNDIS_PACKET_HEAD packetHead;

    NdisAcquireSpinLock(&adapter->general_lock);

    if (*packet_head_list == (PPGPNDIS_PACKET_HEAD) NULL) {
        packetHead = (PPGPNDIS_PACKET_HEAD) NULL;
    } else {
		packetHead = (*packet_head_list)->next;
        if (packetHead == *packet_head_list) {
            *packet_head_list = (PPGPNDIS_PACKET_HEAD) NULL;
        } else {
            (*packet_head_list)->next = packetHead->next;
        }
    }

    NdisReleaseSpinLock(&adapter->general_lock);

    return packetHead;
}

VOID PacketHeadListRemove(
		PVPN_ADAPTER			adapter,
		PPGPNDIS_PACKET_HEAD	*packet_list,
		PPGPNDIS_PACKET_HEAD	packetHead)
{
	PPGPNDIS_PACKET_HEAD pgpPacket;

	NdisAcquireSpinLock(&adapter->general_lock);

	if ( (pgpPacket = *packet_list) != NULL ) 
	{
		do 
		{
			PPGPNDIS_PACKET_HEAD next = pgpPacket->next;
			if (next == packetHead) 
			{
				if (next == pgpPacket)
					*packet_list = NULL;
				else
				{
					pgpPacket->next = next->next;
					if (*packet_list == next)
						*packet_list = pgpPacket;
				}
				NdisReleaseSpinLock(&adapter->general_lock);
				return;
			}
			pgpPacket = next;
		} while (pgpPacket != *packet_list);
	}
	
	NdisReleaseSpinLock(&adapter->general_lock);
	
	return;

}

PPGPNDIS_PACKET_HEAD PacketHeadListQuery(
	PVPN_ADAPTER adapter,
	PPGPNDIS_PACKET_HEAD *packet_head_list,
	USHORT		 id,
	ULONG		 ipAddress)
{
	PPGPNDIS_PACKET_HEAD packetHead;

	NdisAcquireSpinLock(&adapter->general_lock);

	if ( (packetHead = *packet_head_list) != NULL )
	{
		do
		{
			PPGPNDIS_PACKET_HEAD next = packetHead->next;
			if ( (next->id == id) && (next->ipAddress == ipAddress) )
			{
				//if (next == packetHead)
				//{
					NdisReleaseSpinLock(&adapter->general_lock);
					return next;
				//}
			}
			packetHead = next;
		} while (packetHead != *packet_head_list);
	}
	NdisReleaseSpinLock(&adapter->general_lock);
	return NULL;
}

PPGPNDIS_PACKET_HEAD PGPNdisPacketHeadAlloc(
    PNDIS_STATUS status,
    PVPN_ADAPTER adapter)
{
    PPGPNDIS_PACKET_HEAD packetHead;

	DBG_FUNC("PGPNdisPacketHeadAlloc")

	packetHead = (PPGPNDIS_PACKET_HEAD) PacketHeadDequeue( adapter, &adapter->free_packet_head_list);

	if (packetHead == (PPGPNDIS_PACKET_HEAD) NULL) {
		DBG_PRINT(("!!!!! FATAL!!! PreAllocated PGPPacketHead Exhausted!!!\n"););
		ASSERT(FALSE);
		*status = NDIS_STATUS_RESOURCES;
		return NULL;
	}

	NdisZeroMemory(packetHead, sizeof(PGPNDIS_PACKET_HEAD));

	*status = NDIS_STATUS_SUCCESS;
	
    return packetHead;
}

VOID PGPNdisPacketHeadFreeList(
		PVPN_ADAPTER adapter,
		PPGPNDIS_PACKET_HEAD packetHead,
		BOOLEAN outgoing)
{

	DBG_FUNC("PGPNdisPacketHeadFreeList")

	PPGPNDIS_PACKET packet, next;

	DBG_ENTER();

	packet = packetHead->link;

	while (packet != NULL)
	{
		next = packet->link;
		if (outgoing)
			PGPNdisPacketFree(adapter, packet);
		else
			PGPNdisPacketFreeSrcPacket(adapter, packet);

		packet = next;
			
	}

	DBG_LEAVE(0);
}

VOID PGPNdisPacketFreeList(
		PVPN_ADAPTER adapter,
		PPGPNDIS_PACKET packet,
		BOOLEAN outgoing)
{

	DBG_FUNC("PGPNdisPacketFreeList")

	PPGPNDIS_PACKET next;

	DBG_ENTER();

	while (packet != NULL)
	{
		next = packet->link;
		if (outgoing)
			PGPNdisPacketFree(adapter, packet);
		else
			PGPNdisPacketFreeSrcPacket(adapter, packet);

		packet = next;
			
	}

	DBG_LEAVE(0);
}

VOID PGPNdisPacketHeadFree(
		PVPN_ADAPTER adapter,
		PPGPNDIS_PACKET_HEAD packetHead)
{

	DBG_FUNC("PGPNdisPacketHeadFree")

	DBG_ENTER();

	packetHead->timeOut = 0;
    PacketHeadEnqueue(adapter, &adapter->free_packet_head_list, packetHead);

	DBG_LEAVE(0);
}

VOID InsertPGPNdisPacket(
	PVPN_ADAPTER adapter,
	PPGPNDIS_PACKET_HEAD packetHead,
	PPGPNDIS_PACKET pgpPacket)
{
	PPGPNDIS_PACKET next;

	NdisAcquireSpinLock(&adapter->general_lock);

	next = packetHead->link;

	if (next == NULL)
	{
		packetHead->link = pgpPacket;
		pgpPacket->link = NULL;
		NdisReleaseSpinLock(&adapter->general_lock);
		return;
	}
	

	while (next != NULL)
	{
		if (next->link == NULL)
		{
			next->link = pgpPacket;
			pgpPacket->link = NULL;
			NdisReleaseSpinLock(&adapter->general_lock);
			return;
		}
		else if (pgpPacket->offset < next->link->offset)
		{
			pgpPacket->link = next->link;
			next->link = pgpPacket;
			NdisReleaseSpinLock(&adapter->general_lock);
			return;
		}

		next = next->link;

	}
	NdisReleaseSpinLock(&adapter->general_lock);

}

VOID AppendPGPNdisPacket(
	PVPN_ADAPTER adapter,
	PPGPNDIS_PACKET_HEAD packetHead,
	PPGPNDIS_PACKET pgpPacket)
{
	PPGPNDIS_PACKET next;

	NdisAcquireSpinLock(&adapter->general_lock);

	next = packetHead->link;

	if (next == NULL)
		packetHead->link = pgpPacket;

	while (next != NULL)
	{
		if (next->link == NULL)
		{
			next->link = pgpPacket;
			break;
		}

		next = next->link;
	}

	pgpPacket->link = NULL;

	NdisReleaseSpinLock(&adapter->general_lock);
}

void PacketHeadCollect(
		PVPN_ADAPTER			adapter,
		PPGPNDIS_PACKET_HEAD	*packet_list,
		BOOLEAN					outgoing)
{
	DBG_FUNC("PacketHeadCollect")

	PPGPNDIS_PACKET_HEAD current;
	PPGPNDIS_PACKET_HEAD next;
	
	BOOLEAN done = FALSE;

	if ((current = *packet_list) == NULL) {	
		return;
	}

	NdisAcquireSpinLock(&adapter->general_lock);

	do {
		next = current->next;
		
		next->timeOut += 1;

		if (next->timeOut > 1)
		{
			DBG_PRINT(("Caught one overdue fragment\n"););
			if (next == current) {
				*packet_list = NULL;
				done = TRUE;
			}
			else
			{
				current->next = next->next;
				if (next == *packet_list) {
					*packet_list = next->next;
					done = TRUE;
					break;
				}
			}
			PGPNdisPacketHeadFreeList(adapter, next, outgoing);
			PGPNdisPacketHeadFree(adapter, next);
		}
		else
			current = next;
	}while ( (next != *packet_list) && !done );

	NdisReleaseSpinLock(&adapter->general_lock);
}

VOID FragmentCollection(
    PVOID SystemArg1,
    PVOID Context,
    PVOID SystemArg2,
    PVOID SystemArg3)
{
// Hard coded timeout, to be changed to an adaptive value.
#define FRAGMENTATION_TIMEOUT 15000

	DBG_FUNC("FragmentCollection")

	PVPN_ADAPTER adapter = (PVPN_ADAPTER)Context;

	DBG_ENTER();

	PacketHeadCollect(adapter, &adapter->outgoing_packet_head_list, TRUE);
	PacketHeadCollect(adapter, &adapter->incoming_packet_head_list, FALSE);
	NdisSetTimer(&adapter->collection_timer, FRAGMENTATION_TIMEOUT);

	DBG_LEAVE(0);

}

VOID IndicateReceiveClear(
		PVPN_ADAPTER adapter,
		PPGPNDIS_PACKET pgpPacket)
{
	PIP_HEADER			ip_header;
	UINT				ip_len;

	pgpPacket->HeaderBuffer = pgpPacket->xformBlock;
	pgpPacket->HeaderBufferSize = adapter->eth_hdr_len;
	pgpPacket->LookaheadBuffer = pgpPacket->xformBlock + adapter->eth_hdr_len;

	ip_header = (PIP_HEADER) ( (UCHAR*)pgpPacket->LookaheadBuffer );

    ip_len = ntohs(ip_header->ip_len);

	ProtocolIndicateReceive(adapter,
							pgpPacket->srcPacket,
							pgpPacket->HeaderBuffer,
							pgpPacket->HeaderBufferSize,
							pgpPacket->LookaheadBuffer,
							ip_len,
							ip_len);
}

NDIS_STATUS MacSendPackets(
		PVPN_ADAPTER adapter,
		PPGPNDIS_PACKET_HEAD packetHead)
{
	DBG_FUNC("MacSendPackets")

	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
	NDIS_STATUS returnStatus = NDIS_STATUS_SUCCESS;

	USHORT	IPDataTotalLength;
	UCHAR	ip_prot; // Should always be 0x32?

	USHORT	offset;

	PPGPNDIS_PACKET packet, next;
	PIP_HEADER first_ip_hdr;
	PIP_HEADER ip_hdr;
	PUCHAR		first_xformBlock;
	PUCHAR		first_srcBlock;


	DBG_ENTER();

	// Get Information from the first packet.

	packet = packetHead->link;
	ASSERT(packet != NULL);

	first_srcBlock = packet->srcBlock;
	first_xformBlock = packet->xformBlock;
	first_ip_hdr = (PIP_HEADER)((UCHAR*)packet->xformBlock + ETHER_HEADER_SIZE);
	IPDataTotalLength = htons(first_ip_hdr->ip_len) - IP_HEADER_SIZE;
	ip_prot = first_ip_hdr->ip_prot;

	NdisMoveMemory(first_xformBlock, packet->srcBlock, ETHER_HEADER_SIZE);
	offset = 0;

	while (packet != NULL)
	{
		//NdisMoveMemory(packet->xformBlock, first_xformBlock, ETHER_HEADER_SIZE);
		// Wait for Damon's fix.
		// For every packet adjust the xformBlockLen, wait for Damon's fix
		packet->HeaderBuffer = packet->xformBlock;
		packet->HeaderBufferSize = adapter->eth_hdr_len;
		NdisMoveMemory(packet->xformBlock, first_xformBlock, ETHER_HEADER_SIZE);

		//packet->xformBlockLen -= ETHER_HEADER_SIZE + IP_HEADER_SIZE;
		NdisMoveMemory(packet->xformBlock, first_srcBlock, ETHER_HEADER_SIZE);

		ip_hdr = (PIP_HEADER)((UCHAR*)packet->xformBlock + ETHER_HEADER_SIZE);

		NdisMoveMemory(ip_hdr, first_ip_hdr, IP_HEADER_SIZE);

		ip_hdr->ip_len = htons(packet->xformBlockLen - ETHER_HEADER_SIZE);
		ip_hdr->ip_foff = htons(offset >> 3);
		if (packet->lastXformBlock)
			ip_hdr->ip_foff = ~(IP_MF) & ip_hdr->ip_foff;
		else
			ip_hdr->ip_foff = IP_MF | ip_hdr->ip_foff;
		ip_hdr->ip_prot = ip_prot;
		ip_hdr->ip_chksum = 0;
		ip_hdr->ip_chksum = iphdr_cksum((USHORT*)ip_hdr);

		if (packet->firstSrcBlock)
			packet->xformBlockLen += 0;//ETHER_HEADER_SIZE;
		else
			packet->xformBlockLen += 0;//ETHER_HEADER_SIZE + IP_HEADER_SIZE;

		PGPnetAdjustPacket(packet);

		offset += htons(ip_hdr->ip_len) - IP_HEADER_SIZE;

		if (packet->lastXformBlock)
		{
			PGPNdisPacketFreeList(adapter, packet->link, TRUE);
			next = NULL;
			packet->link = NULL;
		}
		else
			next = packet->link;

		PacketEnqueue(adapter, &adapter->outgoing_multiple_ipsecpacket_list, packet);

		NdisSend(&status, adapter->NdisBindingHandleToRealMac, packet->xformPacket);
		if (status == NDIS_STATUS_PENDING)
		{
			//status = NDIS_STATUS_SUCCESS;
		}
		else
		{
			packet = PacketRemoveByXformPacket(adapter, &adapter->outgoing_multiple_ipsecpacket_list, packet->xformPacket);
			PGPNdisPacketFree(adapter, packet);
		}
		if (packet->lastSrcBlock)
			returnStatus = status;


		packet = next;
		

	}

	PacketHeadListRemove(adapter, &adapter->outgoing_packet_head_list, packetHead);
	PGPNdisPacketHeadFree(adapter, packetHead);

	adapter->SendPackets++;

	DBG_LEAVE(returnStatus);
	return returnStatus;
}

VOID ProtocolIndicatePackets(
		PVPN_ADAPTER adapter,
		PPGPNDIS_PACKET_HEAD packetHead)
{
	DBG_FUNC("ProtocolIndicatePackets")

	NDIS_STATUS status = NDIS_STATUS_SUCCESS;

	USHORT	IPDataTotalLength;
	UCHAR	ip_prot; // Should always be 0x32?

	USHORT	offset;

	USHORT ip_len;

	PPGPNDIS_PACKET packet, next;
	PIP_HEADER first_ip_hdr;
	PIP_HEADER ip_hdr;
	PUCHAR		first_xformBlock;
	PUCHAR		first_srcBlock;


	DBG_ENTER();

	packet = packetHead->link;
	ASSERT(packet != NULL);

	first_srcBlock = packet->srcBlock;
	first_xformBlock = packet->xformBlock;
	first_ip_hdr = (PIP_HEADER)((UCHAR*)packet->xformBlock + ETHER_HEADER_SIZE);
	IPDataTotalLength = htons(first_ip_hdr->ip_len) - IP_HEADER_SIZE;
	ip_prot = first_ip_hdr->ip_prot;

	offset = 0;

	while (packet != NULL)
	{
		//packet->xformBlockLen -= ETHER_HEADER_SIZE + IP_HEADER_SIZE;

		packet->HeaderBuffer = packet->xformBlock;
		packet->HeaderBufferSize = adapter->eth_hdr_len;
		NdisMoveMemory(packet->xformBlock, first_srcBlock, packet->HeaderBufferSize);

		packet->LookaheadBuffer = packet->xformBlock + adapter->eth_hdr_len;

		ip_hdr = (PIP_HEADER)((UCHAR*)packet->xformBlock + ETHER_HEADER_SIZE);

		NdisMoveMemory(ip_hdr, first_ip_hdr, IP_HEADER_SIZE);

		ip_hdr->ip_len = htons(packet->xformBlockLen - ETHER_HEADER_SIZE);
		ip_hdr->ip_foff = htons(offset >> 3);
		if (packet->lastXformBlock)
			ip_hdr->ip_foff = ~(IP_MF) & ip_hdr->ip_foff;
		else
			ip_hdr->ip_foff = IP_MF | ip_hdr->ip_foff;
		ip_hdr->ip_prot = ip_prot;
		ip_hdr->ip_chksum = 0;
		ip_hdr->ip_chksum = iphdr_cksum((USHORT*)ip_hdr);

		if (packet->firstSrcBlock)
			packet->xformBlockLen += 0;//ETHER_HEADER_SIZE;
		else
			packet->xformBlockLen += 0; //ETHER_HEADER_SIZE + IP_HEADER_SIZE;

	    ip_len = ntohs(ip_hdr->ip_len);

		offset += ip_len - IP_HEADER_SIZE;
		if (packet->lastXformBlock)
		{
			PGPNdisPacketFreeList(adapter, packet->link, FALSE);
			next = NULL;
			packet->link = NULL;
		}
		else
			next = packet->link;

	
		ProtocolIndicateReceive(adapter,
							packet->srcPacket,
							packet->HeaderBuffer,
							packet->HeaderBufferSize,
							packet->LookaheadBuffer,
							ip_len,
							ip_len);

		ProtocolIndicateReceiveComplete(adapter);

		packet = next;


	}

	PGPNdisPacketHeadFreeList(adapter, packetHead, FALSE);
	PacketHeadListRemove(adapter, &adapter->incoming_packet_head_list, packetHead);
	PGPNdisPacketHeadFree(adapter, packetHead);

	DBG_LEAVE(0);
}

BOOLEAN BroadcastEthernetAddress(UCHAR* eth_dstAddress)
{
	int i;
	for (i = 0; i < 6; i ++)
	{
		if (eth_dstAddress[i] != 0xFF)
			return FALSE;
	}

	return TRUE;
}