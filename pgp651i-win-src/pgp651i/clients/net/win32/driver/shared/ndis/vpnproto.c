#include "version.h"
#include "vpn.h"
#include "vpndbg.h"

#include "ipdef.h"

#include "pgpNetKernel.h"

/*
 *
 */
extern NDIS_STRING DriverName;
extern NDIS_STRING MSTCPName;

extern BOOLEAN		VpnAdapterCreated;
extern PVPN_ADAPTER VpnAdapterGlobal;

static NDIS_PHYSICAL_ADDRESS HighestAcceptableAddress = NDIS_PHYSICAL_ADDRESS_CONST(-1, -1);

VOID ProtocolCloseAdapterComplete(
    IN NDIS_HANDLE    ProtocolBindingContext,
    IN NDIS_STATUS    status
    )
{
	DBG_FUNC("ProtocolCloseAdapterComplete")

	PVPN_ADAPTER	adapter;

	DBG_ENTER()

	adapter = (PVPN_ADAPTER)ProtocolBindingContext;

/*
	NdisCompleteCloseAdapter (
					adapter->NdisBindingContextFromProtocol,
					status
					);
*/
	DBG_LEAVE(0);

}

VOID ProtocolOpenAdapterComplete(
    IN NDIS_HANDLE		ProtocolBindingContext,
    IN NDIS_STATUS      status,
    IN NDIS_STATUS      OpenErrorStatus
    )
{
	DBG_FUNC("ProtocolOpenAdapterComplete")
	PVPN_ADAPTER adapter;

	DBG_ENTER();

	adapter = (PVPN_ADAPTER)ProtocolBindingContext;

	//NdisCompleteOpenAdapter(adapter->NdisBindingContextFromProtocol, status, OpenErrorStatus);
	if (status != NDIS_STATUS_SUCCESS)
	{
		DBG_PRINT(("!!!!! Red Alert! Unreoverable, OpenAdapter failure!\n"););
		// How to recover gracefully for this error?
	}



	DBG_LEAVE(status);
}

NDIS_STATUS ProtocolReceive(
    IN NDIS_HANDLE		ProtocolBindingContext,
    IN NDIS_HANDLE      MacReceiveContext,
    IN PVOID            HeaderBuffer,
    IN UINT             HeaderBufferSize,
    IN PVOID            LookAheadBuffer,
    IN UINT             LookAheadBufferSize,
    IN UINT             PacketSize
    )
{ 
	DBG_FUNC("ProtocolReceive")
	NDIS_STATUS status;
	PVPN_ADAPTER adapter;

	USHORT				eth_header_len;
	USHORT				eth_protocol;
	PETHERNET_HEADER	eth_header;
	PIP_HEADER			ip_header;
	PUDP_HEADER			udp_header = 0; /* EML 05/04/99 */
	PGPnetPMStatus		pmstatus;

	DBG_ENTER();

	adapter = (PVPN_ADAPTER)ProtocolBindingContext;

	if (LookAheadBufferSize < (sizeof(IP_HEADER)) ) {
		status = NDIS_STATUS_NOT_ACCEPTED;
		goto failout;
	}

	if (adapter->media != NdisMedium802_3 && adapter->media != NdisMediumWan)
	{
		status = NDIS_STATUS_NOT_ACCEPTED;
		goto failout;
	}

	eth_header   = (PETHERNET_HEADER) HeaderBuffer;
	eth_protocol = *((PUSHORT)(&eth_header->eth_protocolType[0]));

	if (eth_protocol != IPPROT_NET)
		goto passthrough;

	eth_header_len = sizeof(ETHERNET_HEADER);

	ip_header = (PIP_HEADER) ( (UCHAR*)LookAheadBuffer );

	if (ip_header->ip_prot == PROTOCOL_IGMP)
		goto passthrough;

	if (ip_header->ip_prot == PROTOCOL_ICMP)
	{
#ifdef KERNEL_MESSAGE_EMULATION
		PGPMESSAGE_CONTEXT	*kernelMessageContext;
		PGPnetMessageHeader *kernelMessageHeader;
		PGPnet_ICMP_Message *icmpMessage;

		kernelMessageContext = (PGPMESSAGE_CONTEXT*)(adapter->pgpMessage);
		
		kernelMessageHeader = &kernelMessageContext->header;
		

		NdisAcquireSpinLock(&adapter->general_lock);

		kernelMessageHeader->head++;

		if (kernelMessageHeader->head > kernelMessageHeader->maxSlots)
			kernelMessageHeader->head = 1;

		kernelMessageContext = &kernelMessageContext[kernelMessageHeader->head];

		kernelMessageContext->messageType = PGPnetMessageICMPType;

		icmpMessage = (PGPnet_ICMP_Message*)&kernelMessageContext->message;

		icmpMessage->ip_packets_received = adapter->ReceivePackets;

		NdisReleaseSpinLock(&adapter->general_lock);

		PgpEventSet(&adapter->pgpEvent);
#endif
	}

	if ( (ip_header->ip_prot == PROTOCOL_UDP) &&
			LookAheadBufferSize >= (sizeof(IP_HEADER) + sizeof(UDP_HEADER)) )
	{
		udp_header = (PUDP_HEADER) ( (UCHAR*)ip_header + sizeof(IP_HEADER) );
	}

	// Better yet, we got this packet, but it's not destined to us.
	// So it's a broadcast packet.

	// We have to check if it's WAN packet

	if (NdisMedium802_3 == adapter->media)
	{
		if (ip_header->ip_dest != adapter->ip_address)

			goto passthrough;
	}

	// Now we have an ordinary ip header with possible more data packet.
	// Ask policy manager for futher directions.
	if (ip_header->ip_foff)
		pmstatus = PGPnetPMNeedTransformLight(PGPnetDriver.PolicyManagerHandle,
			ip_header->ip_src,
			TRUE,
			adapter);
	else
		pmstatus = PGPnetPMNeedTransform(PGPnetDriver.PolicyManagerHandle, 
			ip_header->ip_src,
			(PGPUInt16)(udp_header ? udp_header->dest_port : 0), /* EML 05/04/99 */
			TRUE,
			LookAheadBuffer,
			LookAheadBufferSize,
			adapter);

	if ( kPGPNetPMPacketClear == pmstatus )
		goto passthrough;
	if ( kPGPNetPMPacketEncrypt != pmstatus )
	{
		status = NDIS_STATUS_NOT_ACCEPTED;
		goto failout;
	}
	
	// If we ever reach here. We have to do something to the packet

	status = QueueForTransferComplete(adapter,
					MacReceiveContext,
					HeaderBuffer,
					HeaderBufferSize,
					LookAheadBuffer,
					LookAheadBufferSize,
					PacketSize);

	DBG_LEAVE(status);
	return status;

	if (status != NDIS_STATUS_SUCCESS)
		goto failout;

passthrough:

	// Do we have to take a note here so the next ProtocolReceivComplete will do if it's a secured
	// packet or not? Yes!

	// We indicate up the same MacReceiveContext from NIC to the upper protocol. Secured packet will
	// be save in a list. Thus we can assume packet not in the list is not a secured packet.

	ProtocolIndicateReceive(adapter,
							MacReceiveContext,
							HeaderBuffer,
							HeaderBufferSize,
							LookAheadBuffer,
							LookAheadBufferSize,
							PacketSize);

	 status = NDIS_STATUS_SUCCESS;
	/*
	if ( PacketSize <= MAX_ETHER_FRAME_SIZE - MAX_IPSEC_PACKETSIZE_INCREASE - adapter->eth_hdr_len)
	{
		NdisIndicateReceive(&status, adapter->NdisBindingContextFromProtocol, MacReceiveContext,
                  HeaderBuffer, HeaderBufferSize, LookAheadBuffer,
                  LookAheadBufferSize, PacketSize);
		// Remember that we have indicated a passthrough packet and it is waiting for ReceiveComplete.
		NdisAcquireSpinLock(&adapter->general_lock);
		adapter->unsecuredPacketIndicated = TRUE;
		NdisReleaseSpinLock(&adapter->general_lock);

	}
	else
	{
		status = QueueForTransferComplete(adapter,
					MacReceiveContext,
					HeaderBuffer,
					HeaderBufferSize,
					LookAheadBuffer,
					LookAheadBufferSize,
					PacketSize);
	}
	*/

	adapter->ReceivePackets++;

failout:

	DBG_LEAVE(status);
	return status;
}

VOID ProtocolIndicateReceive(
    IN PVPN_ADAPTER		adapter,
    IN NDIS_HANDLE		MacContext,
    IN PUCHAR           HeaderBuffer,
    IN UINT             HeaderBufferSize,
    IN PVOID            LookAheadBuffer,
    IN UINT             LookAheadBufferSize,
    IN UINT             PacketSize
    )
{
    NDIS_STATUS         Status;

	PBINDING_CONTEXT	eachBinding;
    UINT                i;
    
	DBG_FUNC("ProtocolIndicateReceive")

    NdisAcquireSpinLock(&adapter->general_lock);
    
    eachBinding = (PBINDING_CONTEXT)adapter->Bindings.Flink;

    for (i = 0; i < adapter->BindingNumber; i++)
    {
        ASSERT(eachBinding);

        ASSERT(eachBinding->NdisBindingContextFromProtocol);
            
        NdisReleaseSpinLock(&adapter->general_lock);

        NdisIndicateReceive(&Status,
                            eachBinding->NdisBindingContextFromProtocol,
                            MacContext,
                            HeaderBuffer,
                            HeaderBufferSize,
                            LookAheadBuffer,
                            LookAheadBufferSize,
                            PacketSize
                            );
        
        NdisAcquireSpinLock(&adapter->general_lock);

		if (Status != NDIS_STATUS_NOT_ACCEPTED)
		{
			adapter->unsecuredPacketIndicated = TRUE;
		}
        
        eachBinding = (PBINDING_CONTEXT)eachBinding->Next.Flink;
    }

    NdisReleaseSpinLock(&adapter->general_lock);
}


VOID ProtocolReceiveComplete(
    IN NDIS_HANDLE		ProtocolBindingContext
    )
{
	DBG_FUNC("ProtocolReceiveComplete")
	PVPN_ADAPTER	adapter;
	PPGPNDIS_PACKET pgpPacket;
	UINT			indicated = 0;

	DBG_ENTER();

	adapter = (PVPN_ADAPTER)ProtocolBindingContext;

	NdisAcquireSpinLock(&adapter->general_lock);

	if (adapter->indicate_busy == FALSE) {
		adapter->indicate_busy = TRUE;
		NdisReleaseSpinLock(&adapter->general_lock);

		while ( (pgpPacket = PacketDequeue(adapter, &adapter->incoming_indicateComplete_wait_list)) != NULL ) {
			NDIS_STATUS status;
			status = TransformAndIndicate(adapter, pgpPacket);
			if (status == NDIS_STATUS_SUCCESS)
			{
				PGPNdisPacketFreeSrcPacket(adapter, pgpPacket);
			}
			if (status == NDIS_STATUS_SUCCESS)
				indicated++;
		}

		if (adapter->receive_fragmented)
		{
			NdisIndicateReceiveComplete(adapter->NdisBindingContextFromProtocol);
		}

		if ( indicated > 0 || adapter->unsecuredPacketIndicated)
		{
			// If we have an unsecured packet indicateComplete pending. Do a indicateReceiveComplete
			// Note, if we indicate a secured packet, we should also indicate a receiveComplete.
			// Cause that's the last chance we do it. Or is it?

			adapter->unsecuredPacketIndicated = FALSE;
			//NdisIndicateReceiveComplete(adapter->NdisBindingContextFromProtocol);
			ProtocolIndicateReceiveComplete(adapter);
		}

		if (adapter->receive_fragmented)
		{
			while ( (pgpPacket = PacketDequeue(adapter, &adapter->incoming_fragment_indicateComplete_wait_list)) != NULL ) {
				PGPNdisPacketFreeSrcPacket(adapter, pgpPacket);
			}
			adapter->receive_fragmented = FALSE;
		}
	
		NdisAcquireSpinLock(&adapter->general_lock);
		adapter->indicate_busy = FALSE;
	}

	NdisReleaseSpinLock(&adapter->general_lock);

	DBG_LEAVE(0);
}

VOID ProtocolIndicateReceiveComplete(
    IN PVPN_ADAPTER		adapter
    )
{
	PBINDING_CONTEXT	eachBinding;
    UINT                i;

	DBG_FUNC("ProtocolIndicateReceiveComplete");
    
    NdisAcquireSpinLock(&adapter->general_lock);
    
    eachBinding = (PBINDING_CONTEXT)adapter->Bindings.Flink;
    for (i= 0; i< adapter->BindingNumber; i++)
    {
        ASSERT(eachBinding);

        ASSERT(eachBinding->NdisBindingContextFromProtocol);
        
        NdisReleaseSpinLock(&adapter->general_lock);

        NdisIndicateReceiveComplete(eachBinding->NdisBindingContextFromProtocol);
    
        NdisAcquireSpinLock(&adapter->general_lock);

        eachBinding = (PBINDING_CONTEXT)eachBinding->Next.Flink;
    }
    NdisReleaseSpinLock(&adapter->general_lock);

}

VOID ProtocolRequestComplete(
    IN NDIS_HANDLE		ProtocolBindingContext,
    IN PNDIS_REQUEST    NdisRequest,
    IN NDIS_STATUS      Status
    )
{
	DBG_FUNC("ProtocolRequestComplete")

	PVPN_ADAPTER adapter;
	PPGPNDIS_REQUEST pgpRequest;

	DBG_ENTER();

	adapter = (PVPN_ADAPTER)ProtocolBindingContext;

	NdisRequestTrace(NdisRequest);

	pgpRequest = RequestRemoveByNdisRequest(adapter, &adapter->send_request_list, NdisRequest);

	if (pgpRequest != NULL)
	{
		NdisCompleteRequest (
					pgpRequest->Binding->NdisBindingContextFromProtocol,
					NdisRequest,
					Status
					);
		PGPNdisRequestFree(adapter, pgpRequest);

	}

	DBG_LEAVE(Status);
}

VOID ProtocolResetComplete(
    IN NDIS_HANDLE		ProtocolBindingContext,
    IN NDIS_STATUS      status
    )
{
	DBG_FUNC("ProtocolResetComplete")

	PVPN_ADAPTER adapter;
	PBINDING_CONTEXT	resetBinding;
	PBINDING_CONTEXT	eachBinding;
	UINT				bindingCount;

	DBG_ENTER();

	adapter = (PVPN_ADAPTER)ProtocolBindingContext;

	resetBinding = adapter->ResetBinding;

	eachBinding = (PBINDING_CONTEXT)adapter->Bindings.Flink;

	for (bindingCount = 0; bindingCount < adapter->BindingNumber; bindingCount++)
	{
		ASSERT(eachBinding);
		
		NdisIndicateStatus(eachBinding->NdisBindingContextFromProtocol,
					NDIS_STATUS_RESET_END,
					NULL,
					0
					);
		eachBinding = (PBINDING_CONTEXT)eachBinding->Next.Flink;
	}

	if (resetBinding != NULL)
		NdisCompleteReset(resetBinding->NdisBindingContextFromProtocol, status);
	
	NdisAcquireSpinLock(&adapter->general_lock);
	adapter->ResetBinding = NULL;
	NdisReleaseSpinLock(&adapter->general_lock);
	
	DBG_LEAVE(status);
}

VOID ProtocolSendComplete(
    IN NDIS_HANDLE		ProtocolBindingContext,
    IN PNDIS_PACKET     Packet,
    IN NDIS_STATUS      Status
    )
{
	DBG_FUNC("ProtocolSendComplete")
	PVPN_ADAPTER adapter;
	PPGPNDIS_PACKET pgpPacket;
	DBG_ENTER();

	adapter = (PVPN_ADAPTER)ProtocolBindingContext;

	if ( (pgpPacket = PacketRemoveBySrcPacket(adapter, &adapter->sent_plainpacket_list, Packet)) != NULL )
	{
		NdisCompleteSend(pgpPacket->Binding->NdisBindingContextFromProtocol, Packet, Status);
		PGPNdisPacketFreeWithBindingContext(adapter, pgpPacket);
	}
	else if ((pgpPacket = PacketRemoveByXformPacket(adapter, &adapter->sent_ipsecpacket_list, Packet)) != NULL)
	{
		if ( pgpPacket->fragmentNumber == 2)
		{
			NdisCompleteSend(pgpPacket->Binding->NdisBindingContextFromProtocol,
				pgpPacket->originalPacket,
				Status);
		}
		else if (pgpPacket->fragmentNumber == 1)
		{
			DBG_PRINT(("ProtocolSendComplete. fragment1 sent.\n"););
		}
		else
		{
			NdisCompleteSend(pgpPacket->Binding->NdisBindingContextFromProtocol,
				pgpPacket->srcPacket,
				Status);
		}
		PGPNdisPacketFree(adapter, pgpPacket);

	}
	else if ((pgpPacket = PacketRemoveByXformPacket(adapter, &adapter->outgoing_multiple_ipsecpacket_list, Packet)) != NULL )
	{
		if (pgpPacket->lastSrcBlock == TRUE)
			NdisCompleteSend(pgpPacket->Binding->NdisBindingContextFromProtocol, pgpPacket->srcPacket, Status);

		PGPNdisPacketFree(adapter, pgpPacket);
	}
	else
	{
		//ASSERT(FALSE);
		//NdisCompleteSend(adapter->NdisBindingContextFromProtocol, pgpPacket->srcPacket, Status);
		//PGPNdisPacketFree(adapter, pgpPacket);
		DBG_PRINT(("Packet Completed while adapter shutdown or Memory Leak!\n"););
	}

	DBG_LEAVE(0);
}

VOID ProtocolStatus(
    IN NDIS_HANDLE		ProtocolBindingContext,
    IN NDIS_STATUS      Status,
    IN PVOID            StatusBuffer,
    IN UINT             StatusBufferSize
    )
{
	DBG_FUNC("ProtocolStatus")
	PBINDING_CONTEXT eachBinding;
	UINT			 i;
	PVPN_ADAPTER adapter;

	DBG_ENTER();

	adapter = (PVPN_ADAPTER)ProtocolBindingContext;

    DBG_PRINT(("!!!!! ProtocolStatus Status=%Xh\n",
                   Status););

	if (Status == NDIS_STATUS_WAN_LINE_DOWN)
	{
		adapter->WanIPAddressDetected = FALSE;
		PGPnetRASdisconnect(adapter);
	}

	if (Status == NDIS_STATUS_WAN_LINE_UP)
	{
		//ULONG ipAddress = (ULONG)((PNDIS_WAN_LINE_UP)StatusBuffer)->LocalAddress;
		//PGPnetRASconnect(adapter, ipAddress);
		adapter->WanIPAddressDetected = FALSE;
	}

    NdisAcquireSpinLock(&adapter->general_lock);
    
    eachBinding = (PBINDING_CONTEXT)adapter->Bindings.Flink;
    for (i = 0; i < adapter->BindingNumber; i++)
    {
        ASSERT(eachBinding);

        ASSERT(eachBinding->NdisBindingContextFromProtocol);
            
        NdisReleaseSpinLock(&adapter->general_lock);

        NdisIndicateStatus(eachBinding->NdisBindingContextFromProtocol,
                           Status,
                           StatusBuffer,
                           StatusBufferSize);
        
        NdisAcquireSpinLock(&adapter->general_lock);

        eachBinding = (PBINDING_CONTEXT)eachBinding->Next.Flink;
    }
    NdisReleaseSpinLock(&adapter->general_lock);

	DBG_LEAVE(0);
}

VOID ProtocolStatusComplete(
    IN NDIS_HANDLE		ProtocolBindingContext
    )
{
	DBG_FUNC("ProtocolStatusComplete")
	PBINDING_CONTEXT eachBinding;
	UINT			 i;
	PVPN_ADAPTER adapter;

	DBG_ENTER();

	adapter = (PVPN_ADAPTER)ProtocolBindingContext;

    NdisAcquireSpinLock(&adapter->general_lock);
    
    eachBinding = (PBINDING_CONTEXT)adapter->Bindings.Flink;
    for (i = 0; i < adapter->BindingNumber; i++)
    {
        ASSERT(eachBinding);

        ASSERT(eachBinding->NdisBindingContextFromProtocol);
            
        NdisReleaseSpinLock(&adapter->general_lock);

        NdisIndicateStatusComplete(eachBinding->NdisBindingContextFromProtocol);
        
        NdisAcquireSpinLock(&adapter->general_lock);

        eachBinding = (PBINDING_CONTEXT)eachBinding->Next.Flink;
    }
    NdisReleaseSpinLock(&adapter->general_lock);

	DBG_LEAVE(0);
}

VOID ProtocolTransferDataComplete(
    IN NDIS_HANDLE		ProtocolBindingContext,
    IN PNDIS_PACKET     Packet,
    IN NDIS_STATUS      Status,
    IN UINT             BytesTransferred
    )
{
	DBG_FUNC("ProtocolTransferDataComplete")
	PVPN_ADAPTER	adapter;
	PPGPNDIS_PACKET	pgpPacket;
	DBG_ENTER();

	adapter = (PVPN_ADAPTER)ProtocolBindingContext;

	if ( (pgpPacket = PacketRemoveBySrcPacket(adapter, &adapter->sent_plainpacket_list, Packet)) != NULL )
	{
		NdisCompleteTransferData(pgpPacket->Binding->NdisBindingContextFromProtocol, Packet, Status, BytesTransferred);
		PGPNdisPacketFreeWithBindingContext(adapter, pgpPacket);
	}
	// Check to see if the packet is in the list.
	// If not, it'a unsecured one indicate it to the upper Protocol driver.
	// Add the checking later!
	else if ( (pgpPacket = PacketRemoveBySrcPacket(adapter, &adapter->incoming_ipsectransferComplete_wait_list, Packet)) != NULL)
	{
		if (Status == NDIS_STATUS_SUCCESS) {
			PGPnetAdjustTransferCompletePacket(pgpPacket);
			PacketEnqueue(adapter, &adapter->incoming_indicateComplete_wait_list, pgpPacket);
		}
		else {
			PGPNdisPacketFreeSrcPacket(adapter, pgpPacket);
		}
	}
	else {
		ASSERT(FALSE);
		//NdisCompleteTransferData(adapter->NdisBindingContextFromProtocol, Packet, Status, BytesTransferred);
	}

	DBG_LEAVE(Status);
}

VOID ProtocolBindAdapter(
    OUT PNDIS_STATUS    Status,
    IN NDIS_HANDLE      BindContext,
    IN PNDIS_STRING     DeviceName,
    IN PVOID            SystemSpecific1,
    IN PVOID            SystemSpecific2
    )
{
	DBG_FUNC("ProtocolBindAdapter")

	PVPN_ADAPTER	adapter;
	NDIS_STATUS		status;
	//NDIS_STATUS		OpenErrorStatus;


	NDIS_MEDIUM MediumArray[] = { NdisMedium802_3, NdisMediumWan };
	DBG_ENTER();

	/*
	 *	Check if the Mac instance already exists. FIX!
	 */

	if (NULL != VpnAdapterGlobal)
	{
		adapter = VpnAdapterGlobal;
		status = NDIS_STATUS_SUCCESS;

	    //adapter->NdisBindingContextFromProtocol = BindContext;
		//adapter->NdisAdapterRegistrationHandle = SystemSpecific1;

		*Status = status;

		DBG_LEAVE(0);

		return;
		
		
	}

	/*
	 *	First thing first. We have to call the Mac Driver Entry here.
	 */
	status = MacDriverEntry(NULL, NULL);

    if (status != NDIS_STATUS_SUCCESS)
    {
		*Status = status;
        DBG_PRINT(("!!!!! MacDriverEntry return %Xh\n", status););
		return;
    }


	/*
	 *	Since we don't have a way to disable the PGPnet protocol driver binding to the PGPnet
	 *	adapter itself. We have to detect the redundant binding in this BindAdapter Entry.
	 *	Hopefully by simply return SUCCESS will convince the NDIS LIB.
	 */
	// To Do


	/*
	 *	Allocate memory for the virtual adapter. Borrow some code from AddAdapter.
	 */

	adapter = AllocateVpnAdapter();

	if (adapter == NULL)
		goto failed;


	/*
	 *	Allocate memory for the adapter name and copy the name over.
	 */
	// Merge with AllocateVpnAdapter.
	if (adapter->RealMacName.Length == 0) {
		status = NdisAllocateMemory(&adapter->RealMacName.Buffer, DeviceName->Length, 0, HighestAcceptableAddress);
		if (status != NDIS_STATUS_SUCCESS)
		{
			*Status = status;
			goto failed;
		}
		NdisMoveMemory(adapter->RealMacName.Buffer, DeviceName->Buffer, DeviceName->Length);
		adapter->RealMacName.MaximumLength = DeviceName->Length;
		adapter->RealMacName.Length = DeviceName->Length;
	}

#ifdef MEMPHIS
	/*
	 *	Allocate the shared memory which is done in AddAdapter on NT.
	 */
	adapter->SharedMemorySize = 1024;  // Fix, should get from registry.

    if (adapter->SharedMemorySize)
    {
        NdisAllocateSharedMemory(adapter->NdisAdapterRegistrationHandle,
                                 adapter->SharedMemorySize,
                                 FALSE,
                                 &adapter->SharedMemoryPtr,
                                 &adapter->SharedMemoryPhysicalAddress
                                 );
        if (adapter->SharedMemoryPtr == NULL)
        {
            DBG_PRINT(("!!!!! Could not allocate shared memory.\n"););  
        }
        else
        {
            NdisZeroMemory(adapter->SharedMemoryPtr,
                           adapter->SharedMemorySize);
        }
    }
	
	status = AllocatePGPnetPacketPool(adapter);
	if (status != NDIS_STATUS_SUCCESS)
		goto failed;

	status = AllocatePGPnetRequestPool(adapter);

    InitializeListHead(&adapter->Bindings);

	if (status != NDIS_STATUS_SUCCESS)
		goto failed;


	/*
	 *	Open the binding adapter. See what we can borrow from the OpenAdapter in pgpmac.c
	 */
    if (adapter->NdisBindingHandleToRealMac == NULL) {
        UINT selected_index;

		NdisOpenAdapter(&status,
				&OpenErrorStatus,
				&adapter->NdisBindingHandleToRealMac,
				&selected_index,
				adapter->SupportedMediums,
				adapter->NumSupportedMediums,
				PGPnetDriver.NdisProtocolHandle,
				adapter,
				&adapter->RealMacName,
				0,
				NULL);


		adapter->media = adapter->SupportedMediums[selected_index];

    }

	if (adapter->media == NdisMedium802_3 || adapter->media == NdisMediumWan)
		adapter->eth_hdr_len = ETHER_HEADER_SIZE;

	/*
	for ( i = 0; i < adapter->NumSupportedMediums; i++ ) {
		if (adapter->media == MediumArray[i]) {
			*SelectedMediumIndex = i;
			break;
		}
	}
	*/

    adapter->open = TRUE;
	adapter->SendPackets = 0;
	adapter->ReceivePackets = 0;
#endif

	/*
	 *	Save the binding context. 
  	 */
    adapter->NdisBindingContextFromProtocol = BindContext;
	adapter->NdisAdapterRegistrationHandle = SystemSpecific1;


	if (NDIS_STATUS_PENDING == status)
		*Status = NDIS_STATUS_SUCCESS;
	else
		*Status = status;
	
failed:
	DBG_LEAVE(0);

}

VOID ProtocolUnbindAdapter(
    OUT PNDIS_STATUS    Status,
    IN NDIS_HANDLE      ProtocolBindingContext,
    IN NDIS_HANDLE      UnbindContext
    )
{
	DBG_FUNC("ProtocolUnbindAdapter")

	PVPN_ADAPTER	adapter;
	DBG_ENTER();

	adapter = ProtocolBindingContext;

	if (!VpnAdapterCreated)
	{
		DBG_LEAVE(0);
		return;
	}

	if (VpnAdapterGlobal == NULL)
	{
		DBG_LEAVE(0);
		return;
	}
	
	if (!PGPnetDriver.NdisMacHandle)
	{
		DBG_LEAVE(0);
		return;
	}
	if (!adapter->open)
	{
		DBG_LEAVE(0);
		return;
	}

    if (adapter->NdisBindingHandleToRealMac != NULL)
	{
        NdisCloseAdapter(Status, adapter->NdisBindingHandleToRealMac);
		// What if return PENDING status? TO DO.
		adapter->open = FALSE;
		adapter->NdisBindingHandleToRealMac = NULL;

	}
	else
	{
		*Status = NDIS_STATUS_SUCCESS;
	}


	DBG_LEAVE(0);
}


VOID ProtocolUnload(
    VOID
    )
{
    DBG_FUNC("ProtocolUnload")

    NDIS_STATUS status;

	DBG_ENTER();

    if (PGPnetDriver.NdisProtocolHandle)
    {
        NdisDeregisterProtocol(&status, PGPnetDriver.NdisProtocolHandle);
        PGPnetDriver.NdisProtocolHandle = NULL;
        ASSERT(status == NDIS_STATUS_SUCCESS);
    }
    
	DBG_LEAVE(status);
}


/*
 *
 */

NDIS_STATUS ProtocolDriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath 
    )
{
    DBG_FUNC("ProtocolDriverEntry")

    NDIS_STATUS status;

    NDIS_PROTOCOL_CHARACTERISTICS ProtocolChar;

	DBG_ENTER();
    NdisZeroMemory(&ProtocolChar, sizeof(ProtocolChar));
    ProtocolChar.MajorNdisVersion            = NDIS_MAJOR_VERSION;
    ProtocolChar.MinorNdisVersion            = NDIS_MINOR_VERSION;
    ProtocolChar.Reserved                    = 0;
	ProtocolChar.OpenAdapterCompleteHandler  = ProtocolOpenAdapterComplete;
    ProtocolChar.CloseAdapterCompleteHandler = ProtocolCloseAdapterComplete;
	ProtocolChar.ReceiveHandler              = ProtocolReceive;
    ProtocolChar.ReceiveCompleteHandler      = ProtocolReceiveComplete;
	ProtocolChar.TransferDataCompleteHandler = ProtocolTransferDataComplete;
    ProtocolChar.SendCompleteHandler         = ProtocolSendComplete;
    ProtocolChar.RequestCompleteHandler      = ProtocolRequestComplete;
    ProtocolChar.ResetCompleteHandler        = ProtocolResetComplete;
    ProtocolChar.StatusHandler               = ProtocolStatus;
    ProtocolChar.StatusCompleteHandler       = ProtocolStatusComplete;
    ProtocolChar.Name                        = DriverName;
#ifdef CHICAGO
    ProtocolChar.BindAdapterHandler          = ProtocolBindAdapter;
    ProtocolChar.UnbindAdapterHandler        = ProtocolUnbindAdapter;
    ProtocolChar.UnloadProtocolHandler       = ProtocolUnload;
#endif
    NdisRegisterProtocol(&status,
                         &PGPnetDriver.NdisProtocolHandle,
                         &ProtocolChar,
                         sizeof(ProtocolChar)
                         );
      
    if (status != NDIS_STATUS_SUCCESS)
    {
        DBG_PRINT(("!!!!!:NdisRegisterProtocol status=%Xh\n", status););
    }
	DBG_LEAVE(status);
    return (status);
}
