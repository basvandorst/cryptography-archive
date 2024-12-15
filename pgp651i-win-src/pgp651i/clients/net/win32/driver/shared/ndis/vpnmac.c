#include "version.h"

#include "vpn.h"

#ifndef DBG_MESSAGE
//#ifndef CHICAGO
#define DBG_MESSAGE 1
//#endif
#endif

#include "vpndbg.h"

#include "ipdef.h"

#include "pgpNetKernel.h"

#include "pgpIPheader.h"

BOOLEAN		 VpnAdapterCreated = FALSE;
PVPN_ADAPTER VpnAdapterGlobal = NULL;

extern NDIS_STRING DriverName;
extern NDIS_STRING MSTCPName;

static NDIS_PHYSICAL_ADDRESS HighestAcceptableAddress = NDIS_PHYSICAL_ADDRESS_CONST(-1, -1);

NDIS_MEDIUM MediumArray[] = {
		NdisMediumWan,
        NdisMedium802_3
        };
UINT MediumArraySize = sizeof( MediumArray ) / sizeof( NDIS_MEDIUM );

#if DBG_MESSAGE
char *
GetOidString(NDIS_OID Oid);
#endif


PVPN_ADAPTER AllocateVpnAdapter()
{
	DBG_FUNC("AllocateVpnAdapter")
    NDIS_STATUS         status;

	PVPN_ADAPTER		VpnAdapter;

	if (VpnAdapterCreated)
	{
		VpnAdapter = NULL;
		DBG_LEAVE(0);
		return (VpnAdapter);
	}

    status = NdisAllocateMemory(&VpnAdapter,
                                sizeof(VPN_ADAPTER),
                                0,
                                HighestAcceptableAddress
                                );

    if (status != NDIS_STATUS_SUCCESS)
    {
        DBG_PRINT(("!!!!! NdisAllocateMemory failed status=%Xh\n", status););
        VpnAdapter = NULL;
		DBG_LEAVE(0);
		return (VpnAdapter);
    }

    NdisZeroMemory(VpnAdapter, sizeof(VPN_ADAPTER));
    
	VpnAdapter->MacContext = VpnAdapter;
	VpnAdapter->NdisBindingHandleToRealMac = NULL;
    
	VpnAdapterCreated = TRUE;
	VpnAdapterGlobal = VpnAdapter;

    // Any of the supported types.
    VpnAdapter->NumSupportedMediums = MediumArraySize;
    VpnAdapter->SupportedMediums = &MediumArray[0];
    VpnAdapter->media = (UINT) -1;

	NdisAllocateSpinLock(&VpnAdapter->general_lock);
	DBG_LEAVE(0);
    return (VpnAdapter);

}

VOID FreeVpnAdapter(
    IN PVPN_ADAPTER    VpnAdapter
    )
{
    DBG_FUNC("FreeVpnAdapter")
    

	DBG_PRINT( ("||||| VpnAdapter Freed\n"); );


	NdisFreeSpinLock(&VpnAdapter->general_lock);

    VpnAdapterGlobal = NULL;
    VpnAdapterCreated = FALSE;

	//PGPnetDriver.NdisMacHandle = NULL;

    NdisFreeMemory((PVOID)VpnAdapter, sizeof(VPN_ADAPTER), 0);
}


VOID PGPnetShutdown(PVOID adapter)
{
	DBG_FUNC("PGPnetShutdown")
    PVPN_ADAPTER a = (PVPN_ADAPTER) adapter;

}

NDIS_STATUS MacAddAdapter(
    NDIS_HANDLE MacMacContext,
    NDIS_HANDLE ConfigurationHandle,
    PNDIS_STRING AdapterName)
{
    NDIS_ADAPTER_INFORMATION adapter_info;
    PVPN_ADAPTER adapter;
    NDIS_STATUS status;

    NDIS_HANDLE regKeyHandle;
    PNDIS_STRING String;
    int found = 0;
    NDIS_STRING RealMacString = NDIS_STRING_CONST("RealMac");
	PNDIS_CONFIGURATION_PARAMETER ConfigurationValue;


	DBG_FUNC("MacAddAdapter")

	DBG_ENTER();

    NdisOpenConfiguration(&status, &regKeyHandle, ConfigurationHandle);
    if (status != NDIS_STATUS_SUCCESS) {
		DBG_PRINT(("NdisOpenConfiguration failed, status = %Xh\n", status););
        goto failed;
    }

	NdisReadConfiguration(
        &status,
        &ConfigurationValue,
        regKeyHandle,
        &RealMacString,
        NdisParameterString
        );

    if (status == NDIS_STATUS_SUCCESS)
        String = &ConfigurationValue->ParameterData.StringData;
	else 
	{

#ifndef CHICAGO
		NdisReadBindingInformation (
    				&status,
    				&String,
    				regKeyHandle
    				);
		if (NDIS_STATUS_SUCCESS != status) {
			DBG_PRINT(("!!!!! NdisReadBindingInformation failed, status = %Xh\n", status););
			NdisCloseConfiguration(regKeyHandle);
			goto failed;
		}
#endif
	}

#ifndef CHICAGO
	// Allocate memory for the new instance of the pgpmac driver.
	adapter = AllocateVpnAdapter();

	if (adapter == NULL)
		goto failed;
#else // !CHICAGO
	if (VpnAdapterGlobal == NULL)
		adapter = AllocateVpnAdapter();

	adapter = VpnAdapterGlobal;

#endif // CHICAGO

	status = NdisAllocateMemory(&adapter->RealMacName.Buffer, String->Length, 0, HighestAcceptableAddress);
    if (status != NDIS_STATUS_SUCCESS)
	{
		NdisCloseConfiguration(regKeyHandle);
        goto failed;
	}

	NdisMoveMemory(adapter->RealMacName.Buffer, String->Buffer, String->Length);
	adapter->RealMacName.MaximumLength = String->Length;
	adapter->RealMacName.Length = String->Length;

	NdisCloseConfiguration(regKeyHandle);

    NdisAllocatePacketPool(&status, &adapter->packet_pool, PACKET_POOL_SIZE,
        PROTOCOL_RESERVED_LENGTH);
    if (status != NDIS_STATUS_SUCCESS)
        goto failed;

    NdisAllocateBufferPool(&status, &adapter->buffer_pool, BUFFER_POOL_SIZE);
    if (status != NDIS_STATUS_SUCCESS)
        goto failed;
    
    NdisZeroMemory(&adapter_info, sizeof(NDIS_ADAPTER_INFORMATION));
    adapter_info.Master = TRUE;
    adapter_info.Dma32BitAddresses = TRUE;
    adapter_info.PhysicalMapRegistersNeeded = 1;
    adapter_info.MaximumPhysicalMapping = 0x1000;
    adapter_info.AdapterType = NdisInterfaceInternal;

    status = NdisRegisterAdapter(
				&adapter->NdisAdapterRegistrationHandle,
				PGPnetDriver.NdisMacHandle,
				(NDIS_HANDLE) adapter,
				ConfigurationHandle,
				AdapterName,
				&adapter_info);
    if (status != NDIS_STATUS_SUCCESS)
        goto failed;

    NdisRegisterAdapterShutdownHandler(
					adapter->NdisAdapterRegistrationHandle,
					adapter,
					PGPnetShutdown);

	adapter->SharedMemorySize = 1024 * 4;  // Fix, should get from registry.

    if (adapter->SharedMemorySize)
    {
        NdisAllocateSharedMemory(adapter->NdisAdapterRegistrationHandle,
                                 adapter->SharedMemorySize,
                                 TRUE,//FALSE,
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

    InitializeListHead(&adapter->Bindings);

	if (status != NDIS_STATUS_SUCCESS)
		goto failed;

	NdisInitializeTimer(&adapter->collection_timer,
		FragmentCollection,
		adapter);

#ifdef CHICAGO
	NdisInitializeTimer(&adapter->event_timer,
		EventTimerRoutine,
		adapter);
#endif
	NdisInitializeTimer(&adapter->request_timer,
		RequestTimerRoutine,
		adapter);

	DBG_LEAVE(status);
    return status;

failed:
    if (adapter) {
        if (adapter->buffer_pool)
            NdisFreeBufferPool(adapter->buffer_pool);
        if (adapter->packet_pool)
            NdisFreePacketPool(adapter->packet_pool);
    }

	DBG_LEAVE(status);
    return status;
}

NDIS_STATUS MacOpenAdapter(
    OUT PNDIS_STATUS OpenErrorStatus,   // extra status from token ring
    OUT NDIS_HANDLE *MacBindingHandle,  // VPN's id for this protocol binding
    OUT PUINT SelectedMediumIndex,      // media type of this intermedate mac driver
    IN PNDIS_MEDIUM MediumArray,        // vpn supported media list
    IN UINT MediumArraySize,            // number of supported media
    IN NDIS_HANDLE NdisBindingContext,  // ???
    IN NDIS_HANDLE MacAdapterContext,   // ???
    IN UINT OpenOptions,                // flags controlling this binding
    IN PSTRING AddressingInformation OPTIONAL) // haradware specific info
//  Called for every instance after NDIS add the adapter.
//  Find the real device we are suppose to bound to, and open the real
//	adapter. Initialize and allocate the resources which are supposed to be
//	allocated after open adapter.
//  Returns include status, media type and a handle to the adapter itself.
{
    NDIS_STATUS status;
    unsigned int i;
    PVPN_ADAPTER adapter;
	PBINDING_CONTEXT bindingContext;
#ifdef CHICAGO
	PNDIS_PROTOCOL_CHARACTERISTICS PProtocolChar;
#endif

	DBG_FUNC("MacOpenAdapter")

	DBG_ENTER();
	// Shall we care about possibilities of multiple open on the same adapter?

	adapter = (PVPN_ADAPTER)MacAdapterContext;

    *MacBindingHandle = (NDIS_HANDLE) adapter;
	// Save this to the new binding context instead.

	// Here we are assuming there is only one protocol binding to us. So there
	// is only one NdisBindingContextFromProtocol. Need change the structure
	// later to accomodate multiple protocol bindings.
    adapter->NdisBindingContextFromProtocol = NdisBindingContext;
	// Now it's the time to change this. 

	// Here raise a question. Do we need to open the physical adapter everytime
	// when the MacOpenAdapter get called? Maybe not.

	// So what we do here would be: open the real adapter, passing forth and
	// back some information. Since the vpn adapter should be ready to do real
	// job as soon as this function successfully finished its' job(either 
	// syncronous or asyncronous), we might want to do some configuration job
	// here.
    if (adapter->NdisBindingHandleToRealMac == NULL) {
        UINT selected_index;

#ifdef CHICAGO
		PProtocolChar = ( PNDIS_PROTOCOL_CHARACTERISTICS)&
			(((PWRAPPER_PROTOCOL_BLOCK)PGPnetDriver.NdisProtocolHandle)->ProtocolCharacteristics);
		PProtocolChar->Name = MSTCPName;
#endif
		NdisOpenAdapter(&status,
				OpenErrorStatus,
				&adapter->NdisBindingHandleToRealMac,
				&selected_index,
				adapter->SupportedMediums,
				adapter->NumSupportedMediums,
				PGPnetDriver.NdisProtocolHandle,
				adapter,
				&adapter->RealMacName,
				0,
				NULL);
#ifdef CHICAGO
		PProtocolChar->Name = DriverName;
#endif

		if ( (status == NDIS_STATUS_SUCCESS) || (status == NDIS_STATUS_PENDING) )
			adapter->media = adapter->SupportedMediums[selected_index];
		
    }

	if ( (status != NDIS_STATUS_SUCCESS) && (status != NDIS_STATUS_PENDING) )
	{
		DBG_PRINT(("!!!!! Red Alert! Unreoverable, OpenAdapter failure!\n"););
		goto failed;
	}

	if (adapter->media == NdisMedium802_3 || adapter->media == NdisMediumWan)
		adapter->eth_hdr_len = ETHER_HEADER_SIZE;

	for ( i = 0; i < adapter->NumSupportedMediums; i++ ) {
		if (adapter->media == MediumArray[i]) {
			*SelectedMediumIndex = i;
			break;
		}
	}

    adapter->open = TRUE;
	adapter->SendPackets = 0;
	adapter->ReceivePackets = 0;

    status = NdisAllocateMemory(&bindingContext,
                                sizeof(BINDING_CONTEXT),
                                0,
                                HighestAcceptableAddress
                                );

    if (status == NDIS_STATUS_SUCCESS)
    {
        NdisZeroMemory(bindingContext, sizeof(BINDING_CONTEXT));

        *MacBindingHandle = bindingContext;
        bindingContext->NdisBindingContextFromProtocol = NdisBindingContext;
        bindingContext->adapter = adapter;

        NdisAcquireSpinLock(&adapter->general_lock);
        InsertTailList(&adapter->Bindings, &bindingContext->Next);
        bindingContext->InstanceNumber = adapter->BindingNumber++;
        NdisReleaseSpinLock(&adapter->general_lock);
            
    }

	NdisSetTimer(&adapter->collection_timer, 60000/* 1 minute?*/);


failed:

	DBG_LEAVE(status);
    
    return status;
}

NDIS_STATUS MacCloseAdapter(
    IN NDIS_HANDLE BindingHandle) 
{
	DBG_FUNC("MacCloseAdapter")

	NDIS_STATUS		status;
	PBINDING_CONTEXT binding;
    PVPN_ADAPTER adapter;

	DBG_ENTER();

	binding = (PBINDING_CONTEXT) BindingHandle;

	adapter = binding->adapter;

	status = NDIS_STATUS_SUCCESS;

	if (VpnAdapterGlobal == NULL)
	{
		DBG_LEAVE(status);
		return status;
	}

    if (adapter) {
		if (adapter->NdisBindingHandleToRealMac != NULL)
		{
			/*
			PPGPNDIS_PACKET pgpPacket;
			while ( (pgpPacket = PacketDequeue(adapter, &adapter->sent_plainpacket_list)) != NULL)
			{
				DBG_PRINT(("||||| Complete Send plain packet.\n"););
				NdisCompleteSend(pgpPacket->Binding->NdisBindingContextFromProtocol,
					pgpPacket->srcPacket, NDIS_STATUS_SUCCESS);
				PGPNdisPacketFreeWithBindingContext(adapter, pgpPacket);
			}
			*/

			NdisCloseAdapter (
					&status,
	 				adapter->NdisBindingHandleToRealMac
	 				);

	        adapter->open = FALSE;
			adapter->NdisBindingHandleToRealMac = NULL;
		}
    }

	if (status == NDIS_STATUS_PENDING)
		status = NDIS_STATUS_SUCCESS;

	DBG_LEAVE(status)

	return status;

}


VOID MacUnload(
	IN NDIS_HANDLE MacContext)
{
    DBG_FUNC("MacUnload")

    NDIS_STATUS status;

	DBG_ENTER();

	ASSERT(MacContext == PGPnetDriver.NdisMacHandle);
    if (PGPnetDriver.NdisMacHandle)
    {
        NdisDeregisterMac(&status, PGPnetDriver.NdisMacHandle);
        PGPnetDriver.NdisMacHandle = NULL;
        ASSERT(status == NDIS_STATUS_SUCCESS);
    }

	DBG_LEAVE(status);
}

NDIS_STATUS MacQueryGlobalStatistics(
    IN NDIS_HANDLE MacContext,
    IN PNDIS_REQUEST NdisRequest
    )
{
	DBG_FUNC("MacQueryGlobalStatistics")
	NDIS_STATUS	status;

    PVPN_ADAPTER adapter;
    UINT BytesWritten = 0;
    UINT BytesNeeded = 0;
    UINT BytesLeft = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PUCHAR InfoBuffer = (PUCHAR)(NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer);
	UINT MoveBytes = sizeof(UINT) * 2 + sizeof(NDIS_OID);

	DBG_ENTER();


    status = NDIS_STATUS_SUCCESS;

	adapter = (PVPN_ADAPTER)MacContext;

#if DBG_MESSAGE
	DBG_PRINT(("MacQueryGlobalStatistics: %s\n", GetOidString(NdisRequest->DATA.QUERY_INFORMATION.Oid)););
#endif
    //
    // Make sure that int is 4 bytes.  Else GenericULong must change
    // to something of size 4.
    //
    ASSERT(sizeof(UINT) == 4);
	ASSERT(NdisRequest->RequestType == NdisRequestQueryStatistics);

#ifdef GENERIC_QUERY
    switch (NdisRequest->DATA.QUERY_INFORMATION.Oid) {
    case OID_GEN_SUPPORTED_LIST:

		status = NDIS_STATUS_NOT_SUPPORTED;

        break;
    case OID_GEN_MAC_OPTIONS:

		status = NDIS_STATUS_NOT_SUPPORTED;

        break;
	case OID_GEN_WIN32_IOCTL:
		status = ConfigurationIoctl(adapter,
                    NdisRequest->DATA.QUERY_INFORMATION.Oid,
                    NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer,
                    NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength,
                    &(NdisRequest->DATA.QUERY_INFORMATION.BytesWritten),
                    &(NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded)
                    );


		break;
	default:
		status = NDIS_STATUS_NOT_SUPPORTED;

	}
    NdisRequest->DATA.QUERY_INFORMATION.BytesWritten = BytesWritten;
    NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded = BytesNeeded;

#else
	status = ConfigurationIoctl(adapter,
                    NdisRequest->DATA.QUERY_INFORMATION.Oid,
                    NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer,
                    NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength,
                    &(NdisRequest->DATA.QUERY_INFORMATION.BytesWritten),
                    &(NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded)
                    );
  
#endif

	DBG_LEAVE(status);

	return status;
}

VOID MacRemoveAdapter(
    IN NDIS_HANDLE MacContext
    )
{
	DBG_FUNC("MacRemoveAdapter")
	NDIS_STATUS		status;
	PVPN_ADAPTER	adapter;

	DBG_ENTER()

	adapter = (PVPN_ADAPTER)MacContext;

	/*
	if (adapter->open)
	{
		PPGPNDIS_PACKET	pgpPacket;
		while ( (pgpPacket = PacketDequeue(adapter, &adapter->sent_plainpacket_list)) != NULL)
		{
			DBG_PRINT(("||||| Complete Send plain packet.\n"););
			NdisCompleteSend(pgpPacket->Binding->NdisBindingContextFromProtocol,
				pgpPacket->srcPacket, NDIS_STATUS_SUCCESS);
			PGPNdisPacketFreeWithBindingContext(adapter, pgpPacket);
		}
	}
	*/

    if (adapter->SharedMemorySize)
    {
        NdisFreeSharedMemory(adapter->NdisAdapterRegistrationHandle,
                                 adapter->SharedMemorySize,
                                 FALSE,
                                 adapter->SharedMemoryPtr,
                                 adapter->SharedMemoryPhysicalAddress
                                 );
		adapter->SharedMemoryPtr = NULL;
    }

	status = NdisDeregisterAdapter (adapter->NdisAdapterRegistrationHandle);

    if (adapter->buffer_pool)
        NdisFreeBufferPool(adapter->buffer_pool);
    if (adapter->packet_pool)
        NdisFreePacketPool(adapter->packet_pool);
	{
		BOOLEAN success;
		NdisCancelTimer(&adapter->collection_timer, &success);
#ifdef CHICAGO
		NdisCancelTimer(&adapter->event_timer, &success);
#endif
		NdisCancelTimer(&adapter->request_timer, &success);
	}

	FreeVpnAdapter(adapter);

	DBG_LEAVE(status);

	return;
}

NDIS_STATUS MacReset(
    IN NDIS_HANDLE Context
    )
{
	DBG_FUNC("MacReset")
	NDIS_STATUS status;
	PBINDING_CONTEXT binding;
    PVPN_ADAPTER adapter;

	DBG_ENTER();

	binding = (PBINDING_CONTEXT) Context;

	adapter = binding->adapter;

	adapter = (PVPN_ADAPTER)Context;

    NdisAcquireSpinLock(&adapter->general_lock);

    if (adapter->ResetBinding != NULL)
    {
        status = NDIS_STATUS_RESET_IN_PROGRESS;
    }
	else
	{
		UINT i;
		PBINDING_CONTEXT	eachBinding;

		adapter->ResetBinding = binding;

		eachBinding = (PBINDING_CONTEXT)adapter->Bindings.Flink;
		for (i = 0; i < adapter->BindingNumber; i++)
		{
			ASSERT(eachBinding);

			NdisIndicateStatus(eachBinding->NdisBindingContextFromProtocol,
                                   NDIS_STATUS_RESET_START,
                                   NULL,
                                   0
								   );

			eachBinding = (PBINDING_CONTEXT)eachBinding->Next.Flink;

		}

		NdisReset(&status, adapter->NdisBindingHandleToRealMac);

		if (status != NDIS_STATUS_PENDING)
		{
			eachBinding = (PBINDING_CONTEXT)adapter->Bindings.Flink;
			for (i = 0; i < adapter->BindingNumber; i++)
			{
				ASSERT(eachBinding);

				if (eachBinding != binding)
				{
					NdisIndicateStatus(eachBinding->NdisBindingContextFromProtocol,
										   NDIS_STATUS_RESET_END,
										   NULL,
										   0
										  );

				}
				eachBinding = (PBINDING_CONTEXT)eachBinding->Next.Flink;
			}
			adapter->ResetBinding = NULL;
		}
	}

	NdisReleaseSpinLock(&adapter->general_lock);

	DBG_LEAVE(status);
	return status;
}

NDIS_STATUS PgpMacQueryInformation(
    IN PBINDING_CONTEXT binding,
    IN PNDIS_REQUEST Request,
    IN NDIS_OID Oid,         
    IN PVOID InformationBuffer,
    IN ULONG BufferLength,     
    OUT PULONG BytesWritten,   
    OUT PULONG BytesNeeded)    
{

	DBG_FUNC("PgpMacQueryInformation")

    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
	PVPN_ADAPTER adapter = binding->adapter;
	PPGPNDIS_REQUEST pgpRequest;

	DBG_ENTER();


    switch (Oid) {
	case OID_GEN_VENDOR_DESCRIPTION:
		if (!adapter->DescriptionQueried)
		{
			adapter->DescriptionQueried = TRUE;
			*BytesNeeded = 128;
			status = NDIS_STATUS_INVALID_LENGTH;
			break;
		}

    case OID_GEN_MAXIMUM_FRAME_SIZE:
    case OID_GEN_TRANSMIT_BUFFER_SPACE:
    case OID_GEN_RECEIVE_BUFFER_SPACE:
    case OID_GEN_TRANSMIT_BLOCK_SIZE:
    case OID_GEN_RECEIVE_BLOCK_SIZE:
	/*
        if (BufferLength < sizeof(UINT)) {
            *BytesNeeded = sizeof(UINT);
            status = NDIS_STATUS_INVALID_LENGTH;
        } else {
			UINT frame_size = MAX_ETHER_FRAME_SIZE - ETHER_HEADER_SIZE - MAX_IPSEC_PACKETSIZE_INCREASE;
            NdisMoveMemory(InformationBuffer,
                &frame_size, sizeof(frame_size));
            *BytesWritten = sizeof(frame_size);
        }
        break;
	*/

    default:
		{
			pgpRequest = PGPNdisRequestAlloc(&status, adapter);

			if (status != NDIS_STATUS_SUCCESS)
				return status;

			pgpRequest->NdisRequest = Request;
			pgpRequest->Binding = binding;

#ifdef MEMPHIS
			RequestEnqueue(adapter, &adapter->send_request_list, pgpRequest);

			NdisRequest (
				&status,
				adapter->NdisBindingHandleToRealMac,
				Request
				);

			if (status != NDIS_STATUS_PENDING)
			{
				pgpRequest = RequestRemoveByNdisRequest(adapter, &adapter->send_request_list, Request);
				PGPNdisRequestFree(adapter, pgpRequest);

			}
			else
			{
				status = NDIS_STATUS_SUCCESS;
			}
#else
			RequestEnqueue(adapter, &adapter->wait_request_list, pgpRequest);
			status = NDIS_STATUS_PENDING;
			NdisSetTimer(&adapter->request_timer, 1000);
#endif


			break;
		}
    }

    DBG_LEAVE(status);

    return status;
}

NDIS_STATUS PgpMacSetInformation(
    IN PBINDING_CONTEXT binding,
    PNDIS_REQUEST Request,
    IN NDIS_OID Oid,      
    IN PVOID InformationBuffer)

{
    NDIS_STATUS status;
	PPGPNDIS_REQUEST pgpRequest;

	PVPN_ADAPTER adapter = binding->adapter;

	pgpRequest = PGPNdisRequestAlloc(&status, adapter);

	if (status != NDIS_STATUS_SUCCESS)
		return status;

	pgpRequest->NdisRequest = Request;
	pgpRequest->Binding = binding;

#ifdef MEMPHIS
	RequestEnqueue(adapter, &adapter->send_request_list, pgpRequest);

	NdisRequest (
			&status,
			adapter->NdisBindingHandleToRealMac,
			Request
			);

	if (status != NDIS_STATUS_PENDING)
	{
		pgpRequest = RequestRemoveByNdisRequest(adapter, &adapter->send_request_list, Request);
		PGPNdisRequestFree(adapter, pgpRequest);

	}
	else
	{
		status = NDIS_STATUS_SUCCESS;
	}
#else
	RequestEnqueue(adapter, &adapter->wait_request_list, pgpRequest);
	status = NDIS_STATUS_PENDING;
	NdisSetTimer(&adapter->request_timer, 1000);
#endif


    return status;
}


NDIS_STATUS MacRequest(
    IN NDIS_HANDLE Context,
    IN PNDIS_REQUEST NdisRequest
    )
{
	DBG_FUNC("MacRequest")
	NDIS_STATUS status;
	PBINDING_CONTEXT	binding;
	PVPN_ADAPTER		adapter;

	DBG_ENTER();

	binding = (PBINDING_CONTEXT) Context;
	adapter = binding->adapter;

	if (!adapter->open)
	{
		status = NDIS_STATUS_CLOSING;
		DBG_LEAVE(status);

		return status;

	}

    switch (NdisRequest->RequestType) {

    case NdisRequestSetInformation:
#if DBG_MESSAGE
		DBG_PRINT(("NdisRequestSetInformation: %s\n", GetOidString(NdisRequest->DATA.SET_INFORMATION.Oid)););
#endif
        status = PgpMacSetInformation(
                    binding,
                    NdisRequest,
                    NdisRequest->DATA.SET_INFORMATION.Oid,
                    NdisRequest->DATA.SET_INFORMATION.InformationBuffer);
        break;

    case NdisRequestQueryInformation:
#if DBG_MESSAGE
		DBG_PRINT(("NdisRequestQueryInformation: %s\n", GetOidString(NdisRequest->DATA.QUERY_INFORMATION.Oid)););
#endif
        status = PgpMacQueryInformation(
                    binding,
                    NdisRequest,
                    NdisRequest->DATA.QUERY_INFORMATION.Oid,
                    NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer,
                    NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength,
                    &(NdisRequest->DATA.QUERY_INFORMATION.BytesWritten),
                    &(NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded));

        break;

    default:
        status = NDIS_STATUS_NOT_SUPPORTED;
        break;
    }

	DBG_LEAVE(status);

	return status;
}

NDIS_STATUS MacSend(
    IN NDIS_HANDLE Context,
    IN PNDIS_PACKET Packet
    )
{
	DBG_FUNC("MacSend")

	NDIS_STATUS		status;
	PBINDING_CONTEXT binding;
	PVPN_ADAPTER	adapter;
	PNDIS_BUFFER	src_buffer;
	UINT			src_len;
	PNDIS_BUFFER	working_buffer;
	PVOID			working_block;
	UINT			working_block_len;

	PETHERNET_HEADER	eth_header;
	USHORT				eth_protocol;

	USHORT				eth_header_len;

	PIP_HEADER			ip_header;

	PUDP_HEADER			udp_header = 0; /* EML 05/04/99 */

	PPGPNDIS_PACKET		pgpPacket;
	PPGPNDIS_PACKET_HEAD	packetHead;
	BOOLEAN					newHead = FALSE;

	PGPnetPMStatus		pmstatus;

	BOOLEAN				assembleComplete = FALSE;

	DBG_ENTER();

	binding = (PBINDING_CONTEXT)Context;
	adapter = binding->adapter;

	/*
	 *	Check the Packet with the policy manager. Need some design work to be
	 *	done.
	 */

	NdisQueryPacket(Packet, NULL, NULL, &src_buffer, &src_len);
	NdisQueryBuffer(src_buffer, &working_block, &working_block_len);

	/*
	 *	An outgoing src packet would go through up to 4 stages in this stage before it is eventually 
	 *	sent out.
	 */

	/*
	 *	Stage 1. Get the ethernet header. Determines if this is an ip packet. If not bail out early,
	 *	otherwise go to stage 2.
	 */
	eth_header   = (PETHERNET_HEADER) working_block;
	eth_protocol = *((PUSHORT)(&eth_header->eth_protocolType[0]));
	eth_header_len = sizeof(ETHERNET_HEADER);
	if (eth_protocol != IPPROT_NET)
	{
		if (eth_protocol == ARPPROT_NET && adapter->media != NdisMediumWan)
		{
			GetIPAddressFromARP(adapter, (PVOID)((UCHAR*)eth_header + eth_header_len));
		}
		goto bailout;
	}

	if (BroadcastEthernetAddress(eth_header->eth_dstAddress))
		goto bailout;

	if (adapter->media != NdisMedium802_3 && adapter->media != NdisMediumWan)
	{
		status = NDIS_STATUS_NOT_ACCEPTED;
		goto failout;
	}

	/*
	 *	Stage 2 Get the ip header.
	 */
	
	if (working_block_len >= eth_header_len + sizeof(IP_HEADER))
	{
		ip_header = (PIP_HEADER) ( (PCHAR)working_block + eth_header_len);
		working_block = (PCHAR)working_block + eth_header_len;
		working_block_len -= eth_header_len;
	}
	else if (working_block_len == eth_header_len)
	{

		NdisGetNextBuffer(src_buffer, &working_buffer);
		if (working_buffer == NULL)
		{
			status = NDIS_STATUS_NOT_ACCEPTED;
			goto failout;
		}
		NdisQueryBuffer(working_buffer, &working_block, &working_block_len);

		ip_header = (PIP_HEADER)working_block;
	}
	else
	{
		status = NDIS_STATUS_NOT_ACCEPTED;
		goto failout;
	}

	/*
	 *	For Win98 WAN only
	 */
#ifdef CHICAGO
	if (!adapter->WanIPAddressDetected)
	{
		adapter->ip_address = ip_header->ip_src;
		adapter->WanIPAddressDetected = TRUE;
		PGPnetRASconnect(adapter, adapter->ip_address);
	}
#endif

	/*
	 *	Stage 3 Policy on IGMP, ICMP to be determined.
	 */

	if (ip_header->ip_prot == PROTOCOL_IGMP)
		goto bailout;


	/*
	 *	Stage 4. If it's UDP, Get the UDP header.
	 *	Determine if this is a UDP 500 packet. If it is, bail out.
	 */

	if (ip_header->ip_prot == PROTOCOL_UDP)
	{
		if( working_block_len <= sizeof(struct tag_IP_HEADER) )		// FIX!!! ONLY work for ordinary ipv4 header.
		{
			NdisGetNextBuffer(working_buffer, &working_buffer);
			if (working_buffer == NULL)
			{
				status = NDIS_STATUS_NOT_ACCEPTED;
				goto failout;
			}
			NdisQueryBuffer(working_buffer, &working_block, &working_block_len);
			udp_header = (PUDP_HEADER)working_block;
		}
		else
			udp_header = (PUDP_HEADER)( (UCHAR*)working_block + sizeof(IP_HEADER));
	}

#if 0

		if (PGPnetPMIKEPassthrough(PGPnetDriver.PolicyManagerHandle,
			ip_header->ip_dest,
			udp_header->dest_port))

//		if (udp_header->dest_port == UDP_PORT_IKE_NET)
		{
			goto bailout;
		}
		else if (ip_header->ip_foff != 0)
		{
			goto bailout;

		}

	}
#endif

	/*
	 *	Now we have an normal ip packet, ask the policy manager for more directions.
	 */

	if (ip_header->ip_foff)
		pmstatus = PGPnetPMNeedTransformLight(PGPnetDriver.PolicyManagerHandle,
			ip_header->ip_dest,
			FALSE,
			adapter);
	else
		pmstatus = PGPnetPMNeedTransform(PGPnetDriver.PolicyManagerHandle,
			ip_header->ip_dest,
			(PGPUInt16)(udp_header ? udp_header->dest_port : 0), /* EML 05/04/99 */
			FALSE,
			0,
			0,
			adapter);

	if ( kPGPNetPMPacketSent == pmstatus)
		goto dropout;
	if ( kPGPNetPMPacketWaiting == pmstatus)
		goto dropout;
	if ( kPGPNetPMPacketDrop == pmstatus)
		goto dropout;
	if ( kPGPNetPMPacketClear == pmstatus)
		goto bailout;
	if ( kPGPNetPMPacketEncrypt != pmstatus)
	{
		status = NDIS_STATUS_FAILURE;
		goto failout;
	}

	// Now we have a packet to be secured.

	// Allocate the pgpPakcet

	pgpPacket = PGPNdisPacketAllocWithXformPacket(&status, adapter);

	if (status != NDIS_STATUS_SUCCESS)
		goto failout;

	pgpPacket->Binding = binding;
	pgpPacket->srcPacket = Packet;

	PGPCopyPacketToBlock(pgpPacket->srcPacket, pgpPacket->srcBlock, &pgpPacket->srcBlockLen);

	pgpPacket->ipAddress = ntohl(ip_header->ip_dest);

	/* Start EML 05/04/99 */
	pgpPacket->port = udp_header ? udp_header->dest_port : 0;
	/* End ELM */

	pgpPacket->offset = ntohs(ip_header->ip_foff & IP_OFFSET) << 3;
	if (pgpPacket->offset == 0)
		pgpPacket->firstSrcBlock = TRUE;
	// Check to see if it's in the outgoing fragment list.
	packetHead = PacketHeadListQuery(adapter,
				&adapter->outgoing_packet_head_list,
				ip_header->ip_id,
				pgpPacket->ipAddress);
	// If there is no outgoing fragment list. Create one.
	if (packetHead == NULL)
	{
		packetHead = PGPNdisPacketHeadAlloc(&status, adapter);
		newHead = TRUE;
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
		// Have them all, send them all.
		PGPnetPMStatus pm_status;
		PPGPNDIS_PACKET extraPacket;

		// Put an extra buffer there.
		extraPacket = PGPNdisPacketAllocWithXformPacket(&status, adapter);

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
							FALSE,
							adapter);

		if (pm_status != kPGPNetPMPacketSent)
		{
			DBG_PRINT(("!!!!! Yellow Alert! PGPnetPMDoTransform Error!\n"););
			//PGPNdisPacketFree(adapter, pgpPacket);
			PGPNdisPacketHeadFreeList(adapter, packetHead, TRUE);
			PacketHeadListRemove(adapter, &adapter->outgoing_packet_head_list, packetHead);
			PGPNdisPacketHeadFree(adapter, packetHead);

			goto dropout;
		}

		status = MacSendPackets(adapter, packetHead);		

		assembleComplete = TRUE;

	}
	else
	{
		// Not finished, add to the outgoing list
		if (newHead)
			PacketHeadEnqueue(adapter, &adapter->outgoing_packet_head_list, packetHead);

	}
	
	goto dropout;
	// Either way, return successful.
	DBG_LEAVE(status);
	return status;

	


bailout:

	pgpPacket = PGPNdisPacketAllocWithBindingContext(&status, adapter);

	if (status != NDIS_STATUS_SUCCESS)
		goto failout;

	pgpPacket->srcPacket = Packet;
	pgpPacket->Binding = binding;

	PacketEnqueue(adapter, &adapter->sent_plainpacket_list, pgpPacket);

	NdisSend(&status,
				adapter->NdisBindingHandleToRealMac,
				Packet);

	if (status != NDIS_STATUS_PENDING)
	{
		pgpPacket = PacketRemoveBySrcPacket(adapter, &adapter->sent_plainpacket_list, Packet);
		PGPNdisPacketFreeWithBindingContext(adapter, pgpPacket);
	}

	adapter->SendPackets++;

failout:

	DBG_LEAVE(status);
	return status;

dropout:
	if (assembleComplete == FALSE)
		status = NDIS_STATUS_SUCCESS;
	DBG_LEAVE(status);
	return status;
}

NDIS_STATUS MacTransferData(
    IN NDIS_HANDLE MacBindingHandle,
    IN NDIS_HANDLE MacReceiveContext,
    IN UINT ByteOffset,              
    IN UINT BytesToTransfer,         
    OUT PNDIS_PACKET Packet,         
    OUT PUINT BytesTransferred       
    )
{
	DBG_FUNC("MacTransferData")
	NDIS_STATUS			status;
	PBINDING_CONTEXT	binding;
	PVPN_ADAPTER		adapter;
	PPGPNDIS_PACKET		pgpPacket;

	BOOLEAN			fragment = TRUE;

	DBG_ENTER();

	binding = (PBINDING_CONTEXT)MacBindingHandle;
	adapter = binding->adapter;

	// Check to see if the packet is in the list.
	// If not, it'a unsecured one pass it to the NIC driver.
	pgpPacket = PacketRemoveBySrcPacket(adapter, &adapter->incoming_indicateComplete_wait_list, MacReceiveContext);

	if (pgpPacket == NULL)
	{
		pgpPacket = PacketRemoveBySrcPacket(adapter, &adapter->incoming_fragment_indicateComplete_wait_list, MacReceiveContext);
	}
	else
	{
		fragment = FALSE;
	}
	
	if (pgpPacket == NULL)
	{
		pgpPacket = PGPNdisPacketAllocWithBindingContext(&status, adapter);

		pgpPacket->srcPacket = Packet;
		pgpPacket->Binding = binding;

		PacketEnqueue(adapter, &adapter->incoming_plaintransferComplete_wait_list, pgpPacket);

		NdisTransferData(&status,
						adapter->NdisBindingHandleToRealMac,
						MacReceiveContext,
						ByteOffset,
						BytesToTransfer,
						Packet,
						BytesTransferred);

		if (status != NDIS_STATUS_PENDING)
		{
			pgpPacket = PacketRemoveBySrcPacket(adapter, &adapter->incoming_plaintransferComplete_wait_list, Packet);
			PGPNdisPacketFreeWithBindingContext(adapter, pgpPacket);
		}
	}
	else
	{
		ASSERT(FALSE);
	}

	DBG_LEAVE(status);
	return NDIS_STATUS_SUCCESS;
}

/*
 *
 */

NDIS_STATUS MacDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    DBG_FUNC("MacDriverEntry")

    NDIS_STATUS     status;

    NDIS_MAC_CHARACTERISTICS MacChar;

	DBG_ENTER();
	VpnAdapterGlobal = NULL;
    VpnAdapterCreated = FALSE;

    NdisZeroMemory(&MacChar,sizeof(MacChar));
    MacChar.MajorNdisVersion            = NDIS_MAJOR_VERSION;
    MacChar.MinorNdisVersion            = NDIS_MINOR_VERSION;
    MacChar.AddAdapterHandler           = MacAddAdapter;
    MacChar.CloseAdapterHandler         = MacCloseAdapter;
    MacChar.OpenAdapterHandler          = MacOpenAdapter;
    MacChar.QueryGlobalStatisticsHandler= MacQueryGlobalStatistics;
    MacChar.RemoveAdapterHandler        = MacRemoveAdapter;
    MacChar.ResetHandler                = MacReset;
    MacChar.RequestHandler              = MacRequest;
    MacChar.SendHandler                 = MacSend;
    MacChar.TransferDataHandler         = MacTransferData;
    MacChar.UnloadMacHandler            = MacUnload;
    MacChar.Name                        = DriverName;

    ASSERT(PGPnetDriver.NdisWrapperHandle);
    
    NdisRegisterMac(&status,
                    &PGPnetDriver.NdisMacHandle,
                    PGPnetDriver.NdisWrapperHandle,
                    (NDIS_HANDLE)&PGPnetDriver,
                    &MacChar,
                    sizeof(MacChar)
                    );

    if (status != NDIS_STATUS_SUCCESS)
    {
        DBG_PRINT(("!!!!! %s: NdisRegisterMac status=%Xh\n",
                   status););
    }
	
    return (status);
}


#if DBG_MESSAGE
static struct _SupportedOidArray {

	PCHAR		OidName;
	NDIS_OID	Oid;

} SupportedOidArray[ ] = {

	"OID_GEN_SUPPORTED_LIST",				0x00010101,
	"OID_GEN_HARDWARE_STATUS",				0x00010102,
	"OID_GEN_MEDIA_SUPPORTED",				0x00010103,
	"OID_GEN_MEDIA_IN_USE",					0x00010104,
	"OID_GEN_MAXIMUM_LOOKAHEAD",			0x00010105,
	"OID_GEN_MAXIMUM_FRAME_SIZE",			0x00010106,
	"OID_GEN_LINK_SPEED",					0x00010107,
	"OID_GEN_TRANSMIT_BUFFER_SPACE",		0x00010108,
	"OID_GEN_RECEIVE_BUFFER_SPACE",			0x00010109,
	"OID_GEN_TRANSMIT_BLOCK_SIZE",			0x0001010A,
	"OID_GEN_RECEIVE_BLOCK_SIZE",			0x0001010B,
	"OID_GEN_VENDOR_ID",					0x0001010C,
	"OID_GEN_VENDOR_DESCRIPTION",			0x0001010D,
	"OID_GEN_CURRENT_PACKET_FILTER",		0x0001010E,
	"OID_GEN_CURRENT_LOOKAHEAD",			0x0001010F,
	"OID_GEN_DRIVER_VERSION",				0x00010110,
	"OID_GEN_MAXIMUM_TOTAL_SIZE",			0x00010111,
	"OID_GEN_PROTOCOL_OPTIONS",				0x00010112,
	"OID_GEN_MAC_OPTIONS",					0x00010113,
	"OID_GEN_MEDIA_CONNECT_STATUS",			0x00010114,
	"OID_GEN_XMIT_OK",						0x00020101,
	"OID_GEN_RCV_OK",						0x00020102,
	"OID_GEN_XMIT_ERROR",					0x00020103,
	"OID_GEN_RCV_ERROR",					0x00020104,
	"OID_GEN_RCV_NO_BUFFER",				0x00020105,
	"OID_GEN_DIRECTED_BYTES_XMIT",			0x00020201,
	"OID_GEN_DIRECTED_FRAMES_XMIT",			0x00020202,
	"OID_GEN_MULTICAST_BYTES_XMIT",			0x00020203,
	"OID_GEN_MULTICAST_FRAMES_XMIT",		0x00020204,
	"OID_GEN_BROADCAST_BYTES_XMIT",			0x00020205,
	"OID_GEN_BROADCAST_FRAMES_XMIT",		0x00020206,
	"OID_GEN_DIRECTED_BYTES_RCV",			0x00020207,
	"OID_GEN_DIRECTED_FRAMES_RCV",			0x00020208,
	"OID_GEN_MULTICAST_BYTES_RCV",			0x00020209,
	"OID_GEN_MULTICAST_FRAMES_RCV",			0x0002020A,
	"OID_GEN_BROADCAST_BYTES_RCV",			0x0002020B,
	"OID_GEN_BROADCAST_FRAMES_RCV",			0x0002020C,
	"OID_GEN_RCV_CRC_ERROR",				0x0002020D,
	"OID_GEN_TRANSMIT_QUEUE_LENGTH",		0x0002020E,
	"OID_802_3_PERMANENT_ADDRESS",			0x01010101,
	"OID_802_3_CURRENT_ADDRESS",			0x01010102,
	"OID_802_3_MULTICAST_LIST",				0x01010103,
	"OID_802_3_MAXIMUM_LIST_SIZE",			0x01010104,
	"OID_802_3_RCV_ERROR_ALIGNMENT",		0x01020101,
	"OID_802_3_XMIT_ONE_COLLISION",			0x01020102,
	"OID_802_3_XMIT_MORE_COLLISIONS",		0x01020103,
	"OID_802_3_XMIT_DEFERRED",				0x01020201,
	"OID_802_3_XMIT_MAX_COLLISIONS",		0x01020202,
	"OID_802_3_RCV_OVERRUN",				0x01020203,
	"OID_802_3_XMIT_UNDERRUN",				0x01020204,
	"OID_802_3_XMIT_HEARTBEAT_FAILURE",		0x01020205,
	"OID_802_3_XMIT_TIMES_CRS_LOST",		0x01020206,
	"OID_802_3_XMIT_LATE_COLLISIONS",		0x01020207,
	"OID_802_3_PRIORITY",					0x01020208,
	"OID_802_5_PERMANENT_ADDRESS",			0x02010101,
	"OID_802_5_CURRENT_ADDRESS",			0x02010102,
	"OID_802_5_CURRENT_FUNCTIONAL",			0x02010103,
	"OID_802_5_CURRENT_GROUP",				0x02010104,
	"OID_802_5_LAST_OPEN_STATUS",			0x02010105,
	"OID_802_5_CURRENT_RING_STATUS",		0x02010106,
	"OID_802_5_CURRENT_RING_STATE",			0x02010107,
	"OID_802_5_LINE_ERRORS",				0x02020101,
	"OID_802_5_LOST_FRAMES",				0x02020102,
	"OID_802_5_BURST_ERRORS",				0x02020201,
	"OID_802_5_AC_ERRORS",					0x02020202,
	"OID_802_5_ABORT_DELIMETERS",			0x02020203,
	"OID_802_5_FRAME_COPIED_ERRORS",		0x02020204,
	"OID_802_5_FREQUENCY_ERRORS",			0x02020205,
	"OID_802_5_TOKEN_ERRORS",				0x02020206,
	"OID_802_5_INTERNAL_ERRORS",			0x02020207,
	"OID_FDDI_LONG_PERMANENT_ADDR",			0x03010101,
	"OID_FDDI_LONG_CURRENT_ADDR",			0x03010102,
	"OID_FDDI_LONG_MULTICAST_LIST",			0x03010103,
	"OID_FDDI_LONG_MAX_LIST_SIZE",			0x03010104,
	"OID_FDDI_SHORT_PERMANENT_ADDR",		0x03010105,
	"OID_FDDI_SHORT_CURRENT_ADDR",			0x03010106,
	"OID_FDDI_SHORT_MULTICAST_LIST",		0x03010107,
	"OID_FDDI_SHORT_MAX_LIST_SIZE",			0x03010108,
	"OID_FDDI_ATTACHMENT_TYPE",				0x03020101,
	"OID_FDDI_UPSTREAM_NODE_LONG",			0x03020102,
	"OID_FDDI_DOWNSTREAM_NODE_LONG",		0x03020103,
	"OID_FDDI_FRAME_ERRORS",				0x03020104,
	"OID_FDDI_FRAMES_LOST",					0x03020105,
	"OID_FDDI_RING_MGT_STATE",				0x03020106,
	"OID_FDDI_LCT_FAILURES",				0x03020107,
	"OID_FDDI_LEM_REJECTS",					0x03020108,
	"OID_FDDI_LCONNECTION_STATE",			0x03020109,
	"OID_FDDI_SMT_STATION_ID",				0x03030201,
	"OID_FDDI_SMT_OP_VERSION_ID",			0x03030202,
	"OID_FDDI_SMT_HI_VERSION_ID",			0x03030203,
	"OID_FDDI_SMT_LO_VERSION_ID",			0x03030204,
	"OID_FDDI_SMT_MANUFACTURER_DATA",		0x03030205,
	"OID_FDDI_SMT_USER_DATA",				0x03030206,
	"OID_FDDI_SMT_MIB_VERSION_ID",			0x03030207,
	"OID_FDDI_SMT_MAC_CT",					0x03030208,
	"OID_FDDI_SMT_NON_MASTER_CT",			0x03030209,
	"OID_FDDI_SMT_MASTER_CT",				0x0303020A,
	"OID_FDDI_SMT_AVAILABLE_PATHS",			0x0303020B,
	"OID_FDDI_SMT_CONFIG_CAPABILITIES",		0x0303020C,
	"OID_FDDI_SMT_CONFIG_POLICY",			0x0303020D,
	"OID_FDDI_SMT_CONNECTION_POLICY",		0x0303020E,
	"OID_FDDI_SMT_T_NOTIFY",				0x0303020F,
	"OID_FDDI_SMT_STAT_RPT_POLICY",			0x03030210,
	"OID_FDDI_SMT_TRACE_MAX_EXPIRATION",	0x03030211,
	"OID_FDDI_SMT_PORT_INDEXES",			0x03030212,
	"OID_FDDI_SMT_MAC_INDEXES",				0x03030213,
	"OID_FDDI_SMT_BYPASS_PRESENT",			0x03030214,
	"OID_FDDI_SMT_ECM_STATE",				0x03030215,
	"OID_FDDI_SMT_CF_STATE",				0x03030216,
	"OID_FDDI_SMT_HOLD_STATE",				0x03030217,
	"OID_FDDI_SMT_REMOTE_DISCONNECT_FLAG",	0x03030218,
	"OID_FDDI_SMT_STATION_STATUS",			0x03030219,
	"OID_FDDI_SMT_PEER_WRAP_FLAG",			0x0303021A,
	"OID_FDDI_SMT_MSG_TIME_STAMP",			0x0303021B,
	"OID_FDDI_SMT_TRANSITION_TIME_STAMP",	0x0303021C,
	"OID_FDDI_SMT_SET_COUNT",				0x0303021D,
	"OID_FDDI_SMT_LAST_SET_STATION_ID",		0x0303021E,
	"OID_FDDI_MAC_FRAME_STATUS_FUNCTIONS",	0x0303021F,
	"OID_FDDI_MAC_BRIDGE_FUNCTIONS",		0x03030220,
	"OID_FDDI_MAC_T_MAX_CAPABILITY",		0x03030221,
	"OID_FDDI_MAC_TVX_CAPABILITY",			0x03030222,
	"OID_FDDI_MAC_AVAILABLE_PATHS",			0x03030223,
	"OID_FDDI_MAC_CURRENT_PATH",			0x03030224,
	"OID_FDDI_MAC_UPSTREAM_NBR",			0x03030225,
	"OID_FDDI_MAC_DOWNSTREAM_NBR",			0x03030226,
	"OID_FDDI_MAC_OLD_UPSTREAM_NBR",		0x03030227,
	"OID_FDDI_MAC_OLD_DOWNSTREAM_NBR",		0x03030228,
	"OID_FDDI_MAC_DUP_ADDRESS_TEST",		0x03030229,
	"OID_FDDI_MAC_REQUESTED_PATHS",			0x0303022A,
	"OID_FDDI_MAC_DOWNSTREAM_PORT_TYPE",	0x0303022B,
	"OID_FDDI_MAC_INDEX",					0x0303022C,
	"OID_FDDI_MAC_SMT_ADDRESS",				0x0303022D,
	"OID_FDDI_MAC_LONG_GRP_ADDRESS",		0x0303022E,
	"OID_FDDI_MAC_SHORT_GRP_ADDRESS",		0x0303022F,
	"OID_FDDI_MAC_T_REQ",					0x03030230,
	"OID_FDDI_MAC_T_NEG",					0x03030231,
	"OID_FDDI_MAC_T_MAX",					0x03030232,
	"OID_FDDI_MAC_TVX_VALUE",				0x03030233,
	"OID_FDDI_MAC_T_PRI0",					0x03030234,
	"OID_FDDI_MAC_T_PRI1",					0x03030235,
	"OID_FDDI_MAC_T_PRI2",					0x03030236,
	"OID_FDDI_MAC_T_PRI3",					0x03030237,
	"OID_FDDI_MAC_T_PRI4",					0x03030238,
	"OID_FDDI_MAC_T_PRI5",					0x03030239,
	"OID_FDDI_MAC_T_PRI6",					0x0303023A,
	"OID_FDDI_MAC_FRAME_CT",				0x0303023B,
	"OID_FDDI_MAC_COPIED_CT",				0x0303023C,
	"OID_FDDI_MAC_TRANSMIT_CT",				0x0303023D,
	"OID_FDDI_MAC_TOKEN_CT",				0x0303023E,
	"OID_FDDI_MAC_ERROR_CT",				0x0303023F,
	"OID_FDDI_MAC_LOST_CT",					0x03030240,
	"OID_FDDI_MAC_TVX_EXPIRED_CT",			0x03030241,
	"OID_FDDI_MAC_NOT_COPIED_CT",			0x03030242,
	"OID_FDDI_MAC_LATE_CT",					0x03030243,
	"OID_FDDI_MAC_RING_OP_CT",				0x03030244,
	"OID_FDDI_MAC_FRAME_ERROR_THRESHOLD",	0x03030245,
	"OID_FDDI_MAC_FRAME_ERROR_RATIO",		0x03030246,
	"OID_FDDI_MAC_NOT_COPIED_THRESHOLD",	0x03030247,
	"OID_FDDI_MAC_NOT_COPIED_RATIO",		0x03030248,
	"OID_FDDI_MAC_RMT_STATE",				0x03030249,
	"OID_FDDI_MAC_DA_FLAG",					0x0303024A,
	"OID_FDDI_MAC_UNDA_FLAG",				0x0303024B,
	"OID_FDDI_MAC_FRAME_ERROR_FLAG",		0x0303024C,
	"OID_FDDI_MAC_NOT_COPIED_FLAG",			0x0303024D,
	"OID_FDDI_MAC_MA_UNITDATA_AVAILABLE",	0x0303024E,
	"OID_FDDI_MAC_HARDWARE_PRESENT",		0x0303024F,
	"OID_FDDI_MAC_MA_UNITDATA_ENABLE",		0x03030250,
	"OID_FDDI_PATH_INDEX",					0x03030251,
	"OID_FDDI_PATH_RING_LATENCY",			0x03030252,
	"OID_FDDI_PATH_TRACE_STATUS",			0x03030253,
	"OID_FDDI_PATH_SBA_PAYLOAD",			0x03030254,
	"OID_FDDI_PATH_SBA_OVERHEAD",			0x03030255,
	"OID_FDDI_PATH_CONFIGURATION",			0x03030256,
	"OID_FDDI_PATH_T_R_MODE",				0x03030257,
	"OID_FDDI_PATH_SBA_AVAILABLE",			0x03030258,
	"OID_FDDI_PATH_TVX_LOWER_BOUND",		0x03030259,
	"OID_FDDI_PATH_T_MAX_LOWER_BOUND",		0x0303025A,
	"OID_FDDI_PATH_MAX_T_REQ",				0x0303025B,
	"OID_FDDI_PORT_MY_TYPE",				0x0303025C,
	"OID_FDDI_PORT_NEIGHBOR_TYPE",			0x0303025D,
	"OID_FDDI_PORT_CONNECTION_POLICIES",	0x0303025E,
	"OID_FDDI_PORT_MAC_INDICATED",			0x0303025F,
	"OID_FDDI_PORT_CURRENT_PATH",			0x03030260,
	"OID_FDDI_PORT_REQUESTED_PATHS",		0x03030261,
	"OID_FDDI_PORT_MAC_PLACEMENT",			0x03030262,
	"OID_FDDI_PORT_AVAILABLE_PATHS",		0x03030263,
	"OID_FDDI_PORT_MAC_LOOP_TIME",			0x03030264,
	"OID_FDDI_PORT_PMD_CLASS",				0x03030265,
	"OID_FDDI_PORT_CONNECTION_CAPABILITIES", 0x03030266,
	"OID_FDDI_PORT_INDEX",					0x03030267,
	"OID_FDDI_PORT_MAINT_LS",				0x03030268,
	"OID_FDDI_PORT_BS_FLAG",				0x03030269,
	"OID_FDDI_PORT_PC_LS",					0x0303026A,
	"OID_FDDI_PORT_EB_ERROR_CT",			0x0303026B,
	"OID_FDDI_PORT_LCT_FAIL_CT",			0x0303026C,
	"OID_FDDI_PORT_LER_ESTIMATE",			0x0303026D,
	"OID_FDDI_PORT_LEM_REJECT_CT",			0x0303026E,
	"OID_FDDI_PORT_LEM_CT",					0x0303026F,
	"OID_FDDI_PORT_LER_CUTOFF",				0x03030270,
	"OID_FDDI_PORT_LER_ALARM",				0x03030271,
	"OID_FDDI_PORT_CONNNECT_STATE",			0x03030272,
	"OID_FDDI_PORT_PCM_STATE",				0x03030273,
	"OID_FDDI_PORT_PC_WITHHOLD",			0x03030274,
	"OID_FDDI_PORT_LER_FLAG",				0x03030275,
	"OID_FDDI_PORT_HARDWARE_PRESENT",		0x03030276,
	"OID_FDDI_SMT_STATION_ACTION",			0x03030277,
	"OID_FDDI_PORT_ACTION",					0x03030278,
	"OID_FDDI_IF_DESCR",					0x03030279,
	"OID_FDDI_IF_TYPE",						0x0303027A,
	"OID_FDDI_IF_MTU",						0x0303027B,
	"OID_FDDI_IF_SPEED",					0x0303027C,
	"OID_FDDI_IF_PHYS_ADDRESS",				0x0303027D,
	"OID_FDDI_IF_ADMIN_STATUS",				0x0303027E,
	"OID_FDDI_IF_OPER_STATUS",				0x0303027F,
	"OID_FDDI_IF_LAST_CHANGE",				0x03030280,
	"OID_FDDI_IF_IN_OCTETS",				0x03030281,
	"OID_FDDI_IF_IN_UCAST_PKTS",			0x03030282,
	"OID_FDDI_IF_IN_NUCAST_PKTS",			0x03030283,
	"OID_FDDI_IF_IN_DISCARDS",				0x03030284,
	"OID_FDDI_IF_IN_ERRORS",				0x03030285,
	"OID_FDDI_IF_IN_UNKNOWN_PROTOS",		0x03030286,
	"OID_FDDI_IF_OUT_OCTETS",				0x03030287,
	"OID_FDDI_IF_OUT_UCAST_PKTS",			0x03030288,
	"OID_FDDI_IF_OUT_NUCAST_PKTS",			0x03030289,
	"OID_FDDI_IF_OUT_DISCARDS",				0x0303028A,
	"OID_FDDI_IF_OUT_ERRORS",				0x0303028B,
	"OID_FDDI_IF_OUT_QLEN",					0x0303028C,
	"OID_FDDI_IF_SPECIFIC",					0x0303028D,
	"OID_WAN_PERMANENT_ADDRESS",			0x04010101,
	"OID_WAN_CURRENT_ADDRESS",				0x04010102,
	"OID_WAN_QUALITY_OF_SERVICE",			0x04010103,
	"OID_WAN_PROTOCOL_TYPE",				0x04010104,
	"OID_WAN_MEDIUM_SUBTYPE",				0x04010105,
	"OID_WAN_HEADER_FORMAT",				0x04010106,
	"OID_WAN_GET_INFO",						0x04010107,
	"OID_WAN_SET_LINK_INFO",				0x04010108,
	"OID_WAN_GET_LINK_INFO",				0x04010109,
	"OID_WAN_LINE_COUNT",					0x0401010A,
	"OID_WAN_GET_BRIDGE_INFO",				0x0401020A,
	"OID_WAN_SET_BRIDGE_INFO",				0x0401020B,
	"OID_WAN_GET_COMP_INFO",				0x0401020C,
	"OID_WAN_SET_COMP_INFO",				0x0401020D,
	"OID_WAN_GET_STATS_INFO",				0x0401020E,
	"OID_PGP_OPERATION_MANDATORY",			0xFF010100,
	"OID_PGP_WIN32_IOCTL",					0xFF010101,
	"OID_PGP_OPERATION_OPTIONAL",			0xFF010200,
	"OID_PGP_EVENT_CREATE",					0xFF010201,
	"OID_PGP_EVENT_DESTROY",				0xFF010202,
	"OID_PGP_SHARED_MEM_ALLOC",				0xFF010203,
	"OID_PGP_SHARED_MEM_FREE",				0xFF010204,
	"OID_PGP_NEWCONFIG",					0xFF010205,
	"OID_PGP_NEWHOST",						0xFF010206,
	"OID_PGP_NEWSA",						0xFF010207,
	"OID_PGP_SAFAILED",						0xFF010208,
	"OID_PGP_SADIED",						0xFF010209,
	"OID_PGP_SAUPDATE",						0xFF010210,
	"OID_PGP_ALLHOSTS",						0xFF010211,
	"OID_PGP_SHUTDOWN",						0xFF010212,
	"OID_PGP_LOCALIP",						0xFF010213,
	"OID_PGP_QUEUE_INITIALIZE",				0xFF010220,
	"OID_PGP_QUEUE_RELEASE",				0xFF010221,
	"OID_PGP_NEW_DUMMY_HOST",				0xFF0102E0,
	"OID_PGP_NEW_DUMMY_SA",					0xFF0102E1,
	"OID_PGP_EVENT_SET",					0xFF0102FF,
	"OID_PGP_STATISTICS_MANDATORY",			0xFF020100,
	"OID_PGP_STATISTICS_OPTIONAL",			0xFF020200,
	"OID_PGP_GET_SEND_PACKETS",				0xFF020201,
	"OID_PGP_GET_RECEIVE_PACKETS",			0xFF020202
};


#define NUM_OID_ENTRIES (sizeof(SupportedOidArray) / sizeof(SupportedOidArray[0]))

/*
// This debug routine will lookup the printable name for the selected OID.
*/
char *
GetOidString(NDIS_OID Oid)
{
    UINT i;

    for (i = 0; i < NUM_OID_ENTRIES-1; i++)
    {
		if (SupportedOidArray[i].Oid == Oid) {
			break;
		}
    }
    return(SupportedOidArray[i].OidName);
}

void NdisRequestTrace(PNDIS_REQUEST NdisRequest)
{
	DBG_DISPLAY(("NdisRequest: %s\n", GetOidString(NdisRequest->DATA.QUERY_INFORMATION.Oid)););
}
#else
void NdisRequestTrace(PNDIS_REQUEST NdisRequest)
{
}
#endif // DBG_MESSAGE
