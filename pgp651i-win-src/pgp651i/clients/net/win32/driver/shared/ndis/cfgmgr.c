#include "version.h"

#include "vpn.h"
#ifndef DBG_MESSAGE
#define DBG_MESSAGE 1
#endif
#include "vpndbg.h"

#include "pgpNetKernel.h"
#include "pgpNetKernelXChng.h"

#ifdef CHICAGO
#include "pgpvm.h"
#include "vxdevent.h"
#endif

#ifdef PM_EMULATION
#include "dummypm.h"
#endif

extern PVPN_ADAPTER VpnAdapterGlobal;

const NDIS_OID ConfigManagerSupportedOidList[] =
{
	OID_PGP_WIN32_IOCTL,
	OID_PGP_EVENT_CREATE,
	OID_PGP_EVENT_DESTROY,

	OID_PGP_SHARED_MEM_ALLOC,
	OID_PGP_SHARED_MEM_FREE,

	OID_PGP_NEWCONFIG,
	OID_PGP_NEWHOST,
	OID_PGP_NEWSA,
	OID_PGP_SADIED,
	OID_PGP_SAFAILED,
	OID_PGP_SAUPDATE,
	OID_PGP_ALLHOSTS,
	OID_PGP_SHUTDOWN,
	OID_PGP_LOCALIP,

	OID_PGP_QUEUE_INITIALIZE,
	OID_PGP_QUEUE_RELEASE,


#ifdef PM_EMULATION
	OID_PGP_NEW_DUMMY_HOST,
	OID_PGP_NEW_DUMMY_SA,
#endif PM_EMULATION

	OID_PGP_EVENT_SET,

	OID_PGP_GET_SEND_PACKETS,
	OID_PGP_GET_RECEIVE_PACKETS
};

VOID PgpEventCreate(
    IN PPGPEVENT_CONTEXT    pPgpEvent
    )
{
    DBG_FUNC("PgpEventOpen")

#ifdef CHICAGO

	DBG_ENTER();

#else

    UNICODE_STRING          UnicodeName;
    WCHAR                   UnicodeBuffer[128];
    
    DBG_ENTER();



    UnicodeName.MaximumLength = sizeof(UnicodeBuffer);
    UnicodeName.Length = 0;
    UnicodeName.Buffer = UnicodeBuffer;
    memset(UnicodeBuffer, 0, sizeof(UnicodeBuffer));
    RtlAppendUnicodeToString(&UnicodeName, L"\\BaseNamedObjects\\");
    RtlAppendUnicodeToString(&UnicodeName, (PWCHAR) &pPgpEvent->EventName[0]);

    pPgpEvent->DriverEventObject = IoCreateSynchronizationEvent(
                                        &UnicodeName,
                                        &pPgpEvent->DriverEventHandle);
    if (pPgpEvent->DriverEventObject == NULL)
    { 
        DBG_PRINT(("IoCreateSynchronizationEvent FAILED\n"););
        pPgpEvent->DriverEventHandle = NULL;
    } 
    else
    {
        DBG_PRINT(("SUCCESS Name=%ls\n",pPgpEvent->EventName););
        KeClearEvent(pPgpEvent->DriverEventObject); 
    }
#endif

    DBG_LEAVE(0);
}

VOID PgpEventClose(
    IN PPGPEVENT_CONTEXT    pPgpEvent
    )
{
    DBG_FUNC("PgpEventClose")

    DBG_ENTER();

#ifdef CHICAGO
	if (pPgpEvent->DriverEventHandle)
	{
		_VWIN32_CloseVxDHandle((ULONG)pPgpEvent->DriverEventHandle);
		pPgpEvent->DriverEventHandle = NULL;

	}
#else
    if (pPgpEvent->DriverEventHandle)
    {
        ZwClose(pPgpEvent->DriverEventHandle);
        pPgpEvent->DriverEventObject = NULL;
        pPgpEvent->DriverEventHandle = NULL;
    }
#endif

    DBG_LEAVE(0);
}

VOID PgpEventSet(
    IN PPGPEVENT_CONTEXT    pPgpEvent
    )
{
    DBG_FUNC("PgpEventSet")

    DBG_ENTER();

#ifdef CHICAGO
	if (pPgpEvent->DriverEventHandle != NULL)
	{
		PVPN_ADAPTER adapter = VpnAdapterGlobal;

		NdisUpdateSharedMemory(adapter->NdisAdapterRegistrationHandle,
								adapter->SharedMemorySize,
								&adapter->SharedMemoryPtr,
								&adapter->SharedMemoryPhysicalAddress);
					
		//CallWin32Event((ULONG)pPgpEvent->DriverEventHandle);
		NdisSetTimer(&VpnAdapterGlobal->event_timer, 1000 /* 1 second? */);
	}
#else
    if (pPgpEvent->DriverEventObject != NULL)
    {
        KeSetEvent(pPgpEvent->DriverEventObject, 0, FALSE);
    }
#endif

    DBG_LEAVE(0);
}    

PVOID PgpMemoryMapWin32Address(
    IN NDIS_PHYSICAL_ADDRESS    PhysicalAddress,
    IN PVOID                    LinearAddress,
    IN ULONG                    NumBytes,
    IN HANDLE                   ProcessHandle
    )
{
    DBG_FUNC("PgpMemoryMapWin32Address")

    PVOID                       virtualAddress = NULL;
	NTSTATUS                    ntStatus;
#ifdef CHICAGO
	virtualAddress = MemPageLock((ULONG)LinearAddress, NumBytes);
	ntStatus	=	STATUS_SUCCESS;
#else // !CHICAGO
    NDIS_STRING                 physicalMemoryUnicodeString = NDIS_STRING_CONST("\\Device\\PhysicalMemory");
    OBJECT_ATTRIBUTES           objectAttributes;
    HANDLE                      physicalMemoryHandle  = NULL;
    PVOID                       PhysicalMemorySection = NULL;
    NDIS_PHYSICAL_ADDRESS       viewBase;

    DBG_ENTER();

    InitializeObjectAttributes(&objectAttributes,
                               &physicalMemoryUnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               (HANDLE) NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

    ntStatus = ZwOpenSection(&physicalMemoryHandle,
                             SECTION_ALL_ACCESS,
                             &objectAttributes);

    if (!NT_SUCCESS(ntStatus))
    {
        DBG_PRINT(("!!!!! ZwOpenSection failed\n"););
        goto error_out;
    }

    ntStatus = ObReferenceObjectByHandle(physicalMemoryHandle,
                                         SECTION_ALL_ACCESS,
                                         (POBJECT_TYPE) NULL,
                                         KernelMode,
                                         &PhysicalMemorySection,
                                         (POBJECT_HANDLE_INFORMATION) NULL);

    if (!NT_SUCCESS(ntStatus))
    {
        DBG_PRINT(("!!!!!ObReferenceObjectByHandle failed\n"););
        goto close_handle;
    }

    viewBase = PhysicalAddress;

    ntStatus = ZwMapViewOfSection(physicalMemoryHandle,
                                  ProcessHandle,
                                  &virtualAddress,
                                  0L,
                                  NumBytes,
                                  &viewBase,
                                  &NumBytes,
                                  ViewShare,
                                  0,
                                  PAGE_READWRITE | PAGE_NOCACHE);

    if (!NT_SUCCESS(ntStatus))
    {
        DBG_PRINT(("!!!!! ZwMapViewOfSection failed\n"););
        goto close_handle;
    }

    (ULONG) virtualAddress += (ULONG)PhysicalAddress.LowPart -
                              (ULONG)viewBase.LowPart;

close_handle:

    ZwClose(physicalMemoryHandle);

error_out:
#endif // CHICAGO

    DBG_LEAVE(ntStatus);

    return (virtualAddress);
}

VOID PgpMemoryUnmapWin32Address(
    IN PVOID                    VirtualAddress,
    IN ULONG                    NumBytes,
    IN HANDLE                   ProcessHandle
    )
{
    DBG_FUNC("PgpMemoryUnmapWin32Address")

    DBG_ENTER();
#ifdef CHICAGO
	MemPageUnlock((ULONG)VirtualAddress, NumBytes);
#else // !CHICAGO
    ZwUnmapViewOfSection(ProcessHandle, VirtualAddress);
#endif // CHICAGO

    DBG_LEAVE(0);
}

#ifdef PM_EMULATION
#pragma warning(disable:4035)
#pragma warning(disable:4005)

PGPError
PGPnetPMNewSA(struct _PGPnetPMContext *pContext,
			  void *data,
			  UINT dataLen,
			  ULONG *pSrcBufferLen){}

PGPError
PGPnetPMRemoveSA(struct _PGPnetPMContext *pContext,
				 void *data,
				 UINT dataLen,
				 ULONG *pSrcBufferLen){}

PGPError
PGPnetPMFailedSA(struct _PGPnetPMContext *pContext,
				 void *data,
				 UINT dataLen,
				 ULONG *pSrcBufferLen){}

PGPError
PGPnetPMUpdateSA(struct _PGPnetPMContext *pContext,
				 void *data,
				 UINT dataLen,
				 ULONG *pSrcBufferLen){}

PGPError
PGPnetPMNewHost(struct _PGPnetPMContext *pContext,
				void *data,
				UINT dataLen,
				ULONG *pSrcBufferLen){}

NDIS_STATUS
PGPnetPMNewConfig(struct _PGPnetPMContext *pContext,
				  void *data,
				  UINT dataLen,
				  ULONG *pSrcBufferLen){}
#pragma warning(default:4035)
#pragma warning(default:4005)

#endif

NDIS_STATUS  ConfigurationIoctl(
    IN PVPN_ADAPTER	adapter,
    IN NDIS_OID     Oid,         
    IN OUT PVOID    InformationBuffer,
    IN UINT         InformationBufferLength,
    IN OUT PUINT    BytesWritten,           
    IN OUT PUINT    BytesNeeded             
    )
{
    DBG_FUNC("ConfigurationIoctl")
    
    
    NDIS_STATUS  status;

    PVOID        SourceBuffer;
    ULONG        SourceBufferLength;
    ULONG        GenericULong;

	DBG_ENTER();

    SourceBuffer = &GenericULong;
    SourceBufferLength = sizeof(GenericULong);

    switch (Oid)
    {
    case OID_GEN_SUPPORTED_LIST:

        SourceBuffer =  (PVOID)ConfigManagerSupportedOidList;
        SourceBufferLength = sizeof(ConfigManagerSupportedOidList);
		status = NDIS_STATUS_SUCCESS;

        break;
    case OID_GEN_MAC_OPTIONS:

		status = NDIS_STATUS_NOT_SUPPORTED;

        break;
	case OID_GEN_MEDIA_CAPABILITIES:

		status = NDIS_STATUS_NOT_SUPPORTED;

		break;
    case OID_GEN_MEDIA_SUPPORTED:

		status = NDIS_STATUS_NOT_SUPPORTED;

        break;

    case OID_GEN_MEDIA_IN_USE:

		status = NDIS_STATUS_NOT_SUPPORTED;

        break;

    case OID_PGP_WIN32_IOCTL:

		SourceBuffer = (PVOID)NULL;
		SourceBufferLength = 0;
		status = NDIS_STATUS_SUCCESS;

        break;

	case OID_PGP_GET_SEND_PACKETS:

		GenericULong = adapter->SendPackets;

		status = NDIS_STATUS_SUCCESS;

		break;

	case OID_PGP_GET_RECEIVE_PACKETS:

		GenericULong = adapter->ReceivePackets;

		status = NDIS_STATUS_SUCCESS;

		break;

    case OID_PGP_EVENT_CREATE:
        SourceBufferLength = sizeof(PGPEVENT_CONTEXT);
        if (InformationBufferLength >= SourceBufferLength)
        {
            PPGPEVENT_CONTEXT pPgpEvent = (PPGPEVENT_CONTEXT)InformationBuffer;

            PgpEventCreate(pPgpEvent);

			//adapter->pgpEvent = *pPgpEvent;
			NdisMoveMemory(&adapter->pgpEvent, pPgpEvent, sizeof(PGPEVENT_CONTEXT));
            SourceBuffer = InformationBuffer;
			status = NDIS_STATUS_SUCCESS;
        }
        else
        {
            DBG_PRINT(("!!!!! OID_PGP_EVENT_CREATE: Invalid size=%d expected=%d\n",
                      InformationBufferLength, SourceBufferLength););
            status = NDIS_STATUS_INVALID_LENGTH;
        }
        break;

    case OID_PGP_EVENT_DESTROY:
        SourceBufferLength = sizeof(PGPEVENT_CONTEXT);
        if (InformationBufferLength >= SourceBufferLength)
        {
            PPGPEVENT_CONTEXT pPgpEvent = (PPGPEVENT_CONTEXT)InformationBuffer;

            PgpEventClose(pPgpEvent);

			adapter->pgpEvent = *pPgpEvent;
            SourceBuffer = InformationBuffer;
			status = NDIS_STATUS_SUCCESS;
        }
        else
        {
            DBG_PRINT(("!!!!! OID_PGP_EVENT_DESTROY: Invalid size=%d expected=%d\n",
                      InformationBufferLength, SourceBufferLength););
            status = NDIS_STATUS_INVALID_LENGTH;
        }
        break;

	case OID_PGP_EVENT_SET:
        SourceBufferLength = sizeof(PGPEVENT_CONTEXT);
        if (InformationBufferLength >= SourceBufferLength)
        {
            PPGPEVENT_CONTEXT pPgpEvent = (PPGPEVENT_CONTEXT)InformationBuffer;
			if (adapter->SharedMemoryWin32Ptr != NULL)
			{
				ULONG *pData = (ULONG*)(adapter->SharedMemoryPtr);
				*pData = *pData + 1;
			}

            PgpEventSet(&adapter->pgpEvent);

			adapter->pgpEvent = *pPgpEvent;
            SourceBuffer = InformationBuffer;
			status = NDIS_STATUS_SUCCESS;
        }
        else
        {
            DBG_PRINT(("!!!!! OID_PGP_EVENT_Set: Invalid size=%d expected=%d\n",
                      InformationBufferLength, SourceBufferLength););
            status = NDIS_STATUS_INVALID_LENGTH;
        }

		break;

	case OID_PGP_SHARED_MEM_ALLOC:
        SourceBufferLength = sizeof(HANDLE);
		if( adapter->SharedMemoryPtr == NULL)
		{
            DBG_PRINT(("!!!!! OID_PGP_SHARED_MEM_ALLOC: Shared Memory not available!\n"););
			adapter->SharedMemoryWin32Ptr = NULL;
			GenericULong = (ULONG) adapter->SharedMemoryWin32Ptr;
            status = NDIS_STATUS_RESOURCES;
			break;
		}
        if (InformationBufferLength >= SourceBufferLength &&
            adapter->SharedMemoryWin32Ptr == NULL)
        {
            HANDLE ProcessHandle = *(HANDLE *)InformationBuffer;
            adapter->SharedMemoryWin32Ptr = PgpMemoryMapWin32Address(
                                            adapter->SharedMemoryPhysicalAddress,
                                            adapter->SharedMemoryPtr,
                                            adapter->SharedMemorySize,
                                            ProcessHandle
                                            );
#ifdef CHICAGO
			adapter->SharedMemoryWin32Ptr = adapter->SharedMemoryPtr;
#endif
            GenericULong = (ULONG) adapter->SharedMemoryWin32Ptr;
			DBG_PRINT(("kernelSharedMemory Address: %xh\n", adapter->SharedMemoryPtr););
			DBG_PRINT(("Win32SharedMemory Address: %xh\n", adapter->SharedMemoryWin32Ptr););

			if (adapter->SharedMemoryWin32Ptr)
				status = NDIS_STATUS_SUCCESS;
			else
				status = NDIS_STATUS_RESOURCES;
        }
        else
        {
            DBG_PRINT(("!!!!! OID_PGP_SHARED_MEM_ALLOC: Invalid size=%d expected=%d\n",
                      InformationBufferLength, SourceBufferLength););
            status = NDIS_STATUS_INVALID_LENGTH;
        }
        break;

	case OID_PGP_SHARED_MEM_FREE:
        SourceBufferLength = sizeof(HANDLE);
        if (InformationBufferLength >= SourceBufferLength &&
            adapter->SharedMemoryWin32Ptr)
        {
            HANDLE ProcessHandle = *(HANDLE *)InformationBuffer;
            PgpMemoryUnmapWin32Address(
                                        adapter->SharedMemoryWin32Ptr,
                                        adapter->SharedMemorySize,
                                        ProcessHandle
                                        );
            adapter->SharedMemoryWin32Ptr = NULL;
			status = NDIS_STATUS_SUCCESS;
        }
        else
        {
            DBG_PRINT(("!!!!! OID_PGP_SHARED_MEM_FREE: Invalid size=%d expected=%d\n",
                      InformationBufferLength, SourceBufferLength););
            status = NDIS_STATUS_INVALID_LENGTH;
        }
        SourceBufferLength = 0;
        break;

	case OID_PGP_NEWCONFIG:

		status = PGPnetPMNewConfig(PGPnetDriver.PolicyManagerHandle,
			(PVOID)InformationBuffer,
			InformationBufferLength,
			&SourceBufferLength);

		break;

	case OID_PGP_NEWHOST:

		status = PGPnetPMNewHost(
			PGPnetDriver.PolicyManagerHandle,
			(PVOID)InformationBuffer,
			InformationBufferLength,
			&SourceBufferLength);

		break;

	case OID_PGP_ALLHOSTS:

		status = PGPnetPMAllHosts(
			PGPnetDriver.PolicyManagerHandle,
			(PVOID)InformationBuffer,
			InformationBufferLength,
			&SourceBufferLength);

		break;

	case OID_PGP_SHUTDOWN:

		status = PGPnetPMShutdown(
			PGPnetDriver.PolicyManagerHandle,
			(PVOID)InformationBuffer,
			InformationBufferLength,
			&SourceBufferLength);

		break;

	case OID_PGP_LOCALIP:

		GenericULong = adapter->ip_address;
		status = NDIS_STATUS_SUCCESS;

		break;

	case OID_PGP_NEWSA:

		status = PGPnetPMNewSA(
			PGPnetDriver.PolicyManagerHandle,
			(PVOID)InformationBuffer,
			InformationBufferLength,
			&SourceBufferLength);

		break;

	case OID_PGP_SADIED:

		status = PGPnetPMRemoveSA(PGPnetDriver.PolicyManagerHandle,
			(PVOID)InformationBuffer,
			InformationBufferLength,
			&SourceBufferLength);

		break;

	case OID_PGP_SAFAILED:

		status = PGPnetPMFailedSA(PGPnetDriver.PolicyManagerHandle,
			(PVOID)InformationBuffer,
			InformationBufferLength,
			&SourceBufferLength);

		break;
	case OID_PGP_SAUPDATE:

		status = PGPnetPMUpdateSA(PGPnetDriver.PolicyManagerHandle,
			(PVOID)InformationBuffer,
			InformationBufferLength,
			&SourceBufferLength);

		break;
	case OID_PGP_NEW_DUMMY_HOST:

#ifdef PM_EMULATION
		PMAddDummyHost(PGPnetDriver.PolicyManagerHandle,
			(ULONG*)InformationBuffer);
#endif
		status = NDIS_STATUS_SUCCESS;
		SourceBufferLength = 0;

		break;

	case OID_PGP_NEW_DUMMY_SA:
#ifdef PM_EMULATION
		PMAddDummySA(PGPnetDriver.PolicyManagerHandle,
			(PVOID)InformationBuffer);
#endif
		status = NDIS_STATUS_SUCCESS;
		SourceBufferLength = 0;

		break;

	case OID_PGP_QUEUE_INITIALIZE:

		if (adapter->pgpMessage == NULL)
		{

			PPGPMESSAGE_CONTEXT userMessageContext = (PPGPMESSAGE_CONTEXT)InformationBuffer;
			PPGPMESSAGE_CONTEXT kernelMessageContext;

			kernelMessageContext = (PPGPMESSAGE_CONTEXT)adapter->SharedMemoryPtr;

			kernelMessageContext->messageType = userMessageContext->messageType;
			//kernelMessageContext->header.event = 0; // ?? fix
			kernelMessageContext->header.head = userMessageContext->header.head;
			kernelMessageContext->header.tail = userMessageContext->header.tail;
			kernelMessageContext->header.maxSlots = userMessageContext->header.maxSlots;
			kernelMessageContext->header.maxSlotSize = userMessageContext->header.maxSlotSize;

			if ((kernelMessageContext->header.head != 0) |
				(kernelMessageContext->header.tail != 0) |
				((kernelMessageContext->header.maxSlots * kernelMessageContext->header.maxSlotSize) > adapter->SharedMemorySize))
			{
				status = NDIS_STATUS_FAILURE;
			}
			else
			{
				DBG_PRINT(("kernelMessageContext Address: %xh\n", kernelMessageContext););
				adapter->pgpMessage = kernelMessageContext;

				status = NDIS_STATUS_SUCCESS;
			}
		}

	
        SourceBufferLength = 0;

        break;


	case OID_PGP_QUEUE_RELEASE:

		if (adapter->pgpMessage != NULL)
			adapter->pgpMessage = NULL;

		status = NDIS_STATUS_SUCCESS;

		SourceBufferLength = 0;

		break;

    default:

        status = NDIS_STATUS_INVALID_OID;
        SourceBufferLength = 0;

        break;
    }

    if (status == NDIS_STATUS_SUCCESS)
    {
        if (SourceBufferLength > InformationBufferLength)
        {
            *BytesNeeded  = SourceBufferLength;
            *BytesWritten = 0;
            status = NDIS_STATUS_INVALID_LENGTH;
        }
        else if (SourceBufferLength)
        {
            NdisMoveMemory(InformationBuffer, SourceBuffer, SourceBufferLength);
            *BytesWritten = SourceBufferLength;
        }
        else
        {
            *BytesNeeded = *BytesWritten = 0;
        }
    }
    else
    {
        *BytesNeeded = SourceBufferLength;
        *BytesWritten = 0;
    }

    DBG_LEAVE(status);

    return (status);

}

#ifdef CHICAGO
VOID EventTimerRoutine(
    PVOID SystemArg1,
    PVOID Context,
    PVOID SystemArg2,
    PVOID SystemArg3)
{

	DBG_FUNC("EventTimerRoutine")

	PVPN_ADAPTER adapter = (PVPN_ADAPTER)Context;

	DBG_ENTER();

	CallWin32Event( (ULONG) adapter->pgpEvent.DriverEventHandle);

	DBG_LEAVE(0);
}
#endif

VOID RequestTimerRoutine(
	PVOID SystemArg1,
	PVOID Context,
	PVOID SystemArg2,
	PVOID SystemArg3)
{
	DBG_FUNC("RequestTimerRoutine")

	NDIS_STATUS			status;
	PPGPNDIS_REQUEST	pgpRequest;
	PVPN_ADAPTER		adapter = (PVPN_ADAPTER)Context;

	DBG_ENTER();

	pgpRequest = RequestDequeue(adapter, &adapter->wait_request_list);

	RequestEnqueue(adapter, &adapter->send_request_list, pgpRequest);

	NdisRequestTrace(pgpRequest->NdisRequest);

	NdisRequest(
		&status,
		adapter->NdisBindingHandleToRealMac,
		pgpRequest->NdisRequest);

	if (status != NDIS_STATUS_PENDING)
	{
		pgpRequest = RequestRemoveByNdisRequest(adapter, &adapter->send_request_list, pgpRequest->NdisRequest);
		PGPNdisRequestFree(adapter, pgpRequest);
		NdisCompleteRequest (
					pgpRequest->Binding->NdisBindingContextFromProtocol,
					pgpRequest->NdisRequest,
					status
					);


	}
	else
	{
	}

	DBG_LEAVE(status);
}
