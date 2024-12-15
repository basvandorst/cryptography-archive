#ifndef _VPN_H_
#define _VPN_H_
/*
 This header file is intented to define the defines and structures used in the
 Ndis part of the driver.
 The Engine, Policy and Configuration Managers etc should have their own header
 files.
 */

#include <ndis.h>
#include "cfgdef.h"
#define	NDIS_MAJOR_VERSION 0x03
#ifndef CHICAGO
#define NDIS_MINOR_VERSION 0
#endif

#define PACKET_HEAD_PENDING_MAXIMUM		128*2
#define PACKET_PENDING_MAXIMUM			(PACKET_HEAD_PENDING_MAXIMUM * 2)
#define PACKET_POOL_SIZE				PACKET_PENDING_MAXIMUM
#define BUFFER_POOL_SIZE				PACKET_PENDING_MAXIMUM

#define REQUEST_PENDING_MAXIMUM			16

#define	PROTOCOL_RESERVED_LENGTH	4	

#define BLOCK_SIZE			2000
#define MAX_DATASIZE		1480

/*
 One thing is interested about the PGPnet VPN driver is, although we call it a 
 NDIS Driver. The majority of the information contained in the driver object 
 itself is not related to NDIS at all.
 Besides the NdisWrapper Handle, Protocol Handle and Mac Handle, we put the 
 driver wide globals into this structure.
 */

typedef struct tagPGPNET_DRIVER
{
	// The three handles for all vpn drivers.
    NDIS_HANDLE NdisWrapperHandle;                          
	// This NdisWrapperHandle will be passed to us whenever the NdisWrapper 
	// want talk to the driver itself(not any adapter or protocol).

    NDIS_HANDLE NdisMacHandle;                              

    NDIS_HANDLE NdisProtocolHandle;
	// The VPN protocol handle. Get it when we register the VPN protocol to the
	// NdisWrapper. Needed when we open the adapter below on behalf of the
	// protocol. There would be only one protocol binding to the adapter from
	// now on, although there were maybe multiple protocols binding to the 
	// real adpater. The NdisProtocolHandle will relate the real adapter mac
	// driver to the various protocol characteristics registered while we 
	// register the VPN protocol itself. This makes the real adapter mac(or
	// miniport) drivers call the right functions while something happens. And 
	// with the appropriate context parameter while they do call. Since we have
	// multiple virtual adapters which accomodate multiple real macs, we can 
	// allways make sure the right context is passed on to us even there is only
	// one unique NdisProtocolHandle out there.

    NDIS_HANDLE PolicyManagerHandle;                            
	// Holds the handle to the entity that do real stuff.
	// The place for all the global info which are not adapter dependent.

} PGPNET_DRIVER, *PPGPNET_DRIVER;

extern PGPNET_DRIVER  PGPnetDriver;

typedef struct tagPGPNDIS_PACKET {
	struct tagPGPNDIS_PACKET *link;

	PNDIS_PACKET	srcPacket;
	PNDIS_BUFFER	srcBuffer;
	PUCHAR			srcBlock;
	UINT			srcBlockLen;

	UINT			ipAddress;
	USHORT			port;
	UINT			offset;

	PNDIS_PACKET	xformPacket;
	PNDIS_BUFFER	xformBuffer;
	PUCHAR			xformBlock;
	UINT			xformBlockLen;

    PUCHAR			HeaderBuffer;
    UINT			HeaderBufferSize;
    PUCHAR			LookaheadBuffer;

    struct BINDING_CONTEXT    *Binding;

    NDIS_HANDLE         MacReceiveContext;

	PNDIS_PACKET	originalPacket;
	
	UINT			fragmentNumber;

	BOOLEAN			firstSrcBlock;
	BOOLEAN			firstXformBlock;
	BOOLEAN			lastSrcBlock;
	BOOLEAN			lastXformBlock;

} PGPNDIS_PACKET, *PPGPNDIS_PACKET;

typedef struct tagPGPNDIS_PACKET_HEAD {
	struct tagPGPNDIS_PACKET_HEAD *next;

	struct tagPGPNDIS_PACKET *link;

	UINT	ipAddress;
	USHORT	id;
	USHORT	numFragments;

	ULONG	timeStamp;

	UINT	accumulatedLength;
	UINT	totalLength;

	USHORT	timeOut;

} PGPNDIS_PACKET_HEAD, *PPGPNDIS_PACKET_HEAD;

typedef struct tagPGPNDIS_REQUEST {
	struct tagPGPNDIS_REQUEST *link;

	PNDIS_REQUEST	NdisRequest;

    struct BINDING_CONTEXT    *Binding;

} PGPNDIS_REQUEST, *PPGPNDIS_REQUEST;

typedef struct tagVPN_ADAPTER
{
	/*
	 *	Generally speaking there are 6 different groups of fileds go into this
	 *	VPN_ADAPTER structure.
	 *	This NDIS VPN Mac adapter interacts with NDIS Wrapper(as a NDIS driver),
	 *	upper layer	protocol driver(as a mac driver), lower layer real mac 
	 *	driver(as a protocol driver), vpn driver globals(as an instance),
	 *	Group1: Stuff from or to Ndis Wrapper
	 *	Group2: Stuff from or to upper layer protocol
	 *	Group3: Stuff from or to lower layer real mac
	 *	Group4: Stuff as a mac driver should possess
	 *	Group5: Stuff as an instance of the VPN driver
	 *	Group6:	Stuff as system resources allocated
	 */
 
	// Place this Group5 member here for better view while debugging

	/*
	 *	Group1
	 */
	NDIS_HANDLE         NdisAdapterRegistrationHandle;      
	// Handle returned by register the adapter, used in all following NdisReq*

	/*
	 *	Group2
	 */
	NDIS_HANDLE			NdisBindingContextFromProtocol;
	// Handle given by the MacOpenAdapter call. Used when indicate packet or
	// status to the protocol. If we would ever want to support multiple
	// protocols instead of just TCPIP. We need come up with a list of this
	// BindingContext.

	LIST_ENTRY          Bindings;
	// Now it's time to support multiple protocols. Only one of this one and the
	// one above this one will survive.
	struct BINDING_CONTEXT *ResetBinding;
	// Hold the current outstanding reset binding.

	UINT				BindingNumber;

	/*
	 *	Group3
	 */
	NDIS_HANDLE			NdisBindingHandleToRealMac;
	// Handle returned by NdisOpenAdapter. Used when send packet or set status
	// to the real mac device. We can only bind to one adapter, so one 
	// BindingHandle will be enough.
	NDIS_STRING			RealMacName;
	// Name of the mac device we are binding to. Got from registry. Used when 
	// call NdisOpenAdapter.

	/*
	 *	Group4
	 */
	NDIS_HANDLE			MacContext;
	// The VpnAdapter itself. ????
	NDIS_MEDIUM			media;             
	// Media type reported by this adapter. Depends on the adapter this driver
	// is binding to
    UINT                NumSupportedMediums;                
    // Number of entries in the SupportedMediums array depends on the
    // MacMediaType read from the registry.
    PNDIS_MEDIUM        SupportedMediums;                   
    // Array of media types supported by this VpnAdapter.

	NDIS_STRING			adapter_name;
	// NDIS name(possibly UNICODE) provided while asked by other components.

	// Datalink layer header length
	UINT				eth_hdr_len;

	ULONG				ip_address;
	/*
	 *	Group5
	 */
	BOOLEAN				open;
	
    PPGPNDIS_PACKET		free_packet_list;
	PPGPNDIS_PACKET		sent_plainpacket_list;
	PPGPNDIS_PACKET		sent_ipsecpacket_list;
	PPGPNDIS_PACKET		incoming_plaintransferComplete_wait_list;
	PPGPNDIS_PACKET		incoming_ipsectransferComplete_wait_list;
	PPGPNDIS_PACKET		incoming_indicateComplete_wait_list;
	PPGPNDIS_PACKET		incoming_fragment_indicateComplete_wait_list;

	PPGPNDIS_PACKET		outgoing_multiple_ipsecpacket_list;
	PPGPNDIS_PACKET_HEAD	free_packet_head_list;
	PPGPNDIS_PACKET_HEAD	outgoing_packet_head_list;
	PPGPNDIS_PACKET_HEAD	incoming_packet_head_list;
	PPGPNDIS_PACKET_HEAD	sent_packet_head_list;

    PPGPNDIS_REQUEST	free_request_list;
	PPGPNDIS_REQUEST	send_request_list;

	NDIS_SPIN_LOCK		general_lock;

	BOOLEAN				unsecuredPacketIndicated;
	BOOLEAN				securedPacketIndicated;

	BOOLEAN				indicate_busy;
	BOOLEAN				receive_fragmented;

	PGPEVENT_CONTEXT	pgpEvent;
	struct _PGPMESSAGE_CONTEXT *	pgpMessage;

	NDIS_TIMER			collection_timer;
#ifdef CHICAGO
	NDIS_TIMER			event_timer;
#endif
	NDIS_TIMER			request_timer;
	PNDIS_REQUEST		ndis_request;
	PPGPNDIS_REQUEST	wait_request_list;
	/*
	 *	Group6
	 */
	NDIS_HANDLE			packet_pool;
	NDIS_HANDLE			buffer_pool;

	ULONG				SendPackets;
	ULONG				ReceivePackets;

    ULONG               SharedMemorySize;
    PVOID               SharedMemoryPtr;
    PVOID               SharedMemoryWin32Ptr;
    NDIS_PHYSICAL_ADDRESS SharedMemoryPhysicalAddress;

    BOOLEAN				WanIPAddressDetected;
	BOOLEAN				DescriptionQueried;
} VPN_ADAPTER, *PVPN_ADAPTER;

typedef struct BINDING_CONTEXT
{
    LIST_ENTRY      Next;

    PVPN_ADAPTER	adapter;

    UINT            InstanceNumber;

	NDIS_HANDLE     NdisBindingContextFromProtocol;

    BOOLEAN         BindingOpen;
    
} BINDING_CONTEXT, *PBINDING_CONTEXT;

#ifdef CHICAGO
#define NdisAdjustBufferLength(NdisBuffer, NewLength) ((NdisBuffer)->Length = NewLength)
#endif

NDIS_STATUS ProtocolDriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath 
    );

NDIS_STATUS MacDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID DriverUnload(
    NDIS_HANDLE MacMacContext
    );

VOID MacUnload(
	IN NDIS_HANDLE MacContext
	);

VOID ProtocolUnload(
    VOID
    );

NDIS_STATUS PolicyManagerInitialize(
	IN PDRIVER_OBJECT	DriverObject,
	IN PUNICODE_STRING	RegistryPath,
	OUT PNDIS_HANDLE	PolicyManagerHandle
);

VOID PolicyManagerUninitialize(
	IN NDIS_HANDLE		PolicyManagerHandle);


NDIS_STATUS  ConfigurationIoctl(
    IN PVPN_ADAPTER	adapter,
    IN NDIS_OID     Oid,         
    IN OUT PVOID    InformationBuffer,
    IN UINT         InformationBufferLength,
    IN OUT PUINT    BytesWritten,           
    IN OUT PUINT    BytesNeeded      
    );

PVOID PgpMemoryMapWin32Address(
    IN NDIS_PHYSICAL_ADDRESS    PhysicalAddress,
    IN PVOID                    LinearAddress,
    IN ULONG                    NumBytes,
    IN HANDLE                   ProcessHandle
    );

VOID PgpMemoryUnmapWin32Address(
    IN PVOID                    VirtualAddress,
    IN ULONG                    NumBytes,
    IN HANDLE                   ProcessHandle
    );

VOID ProtocolIndicateReceive(
    IN PVPN_ADAPTER		adapter,
    IN NDIS_HANDLE		MacContext,
    IN PUCHAR           HeaderBuffer,
    IN UINT             HeaderBufferSize,
    IN PVOID            LookAheadBuffer,
    IN UINT             LookAheadBufferSize,
    IN UINT             PacketSize
    );

VOID ProtocolIndicateReceiveComplete(
    IN PVPN_ADAPTER		adapter
    );


/*
 *	Fragmentation and Reassembly support routines.
 */

void PacketEnqueue(
    PVPN_ADAPTER adapter,
    PPGPNDIS_PACKET *packet_list,
    PPGPNDIS_PACKET packet);

PPGPNDIS_PACKET PacketDequeue(
    PVPN_ADAPTER adapter,
    PPGPNDIS_PACKET *packet_list);

PPGPNDIS_PACKET PacketRemoveByXformPacket(
		PVPN_ADAPTER		adapter,
		PPGPNDIS_PACKET		*packet_list,
		PNDIS_PACKET		packet);

PPGPNDIS_PACKET PacketRemoveBySrcPacket(
		PVPN_ADAPTER		adapter,
		PPGPNDIS_PACKET		*packet_list,
		PNDIS_PACKET		packet);

NDIS_STATUS AllocatePGPnetPacketPool(PVPN_ADAPTER adapter);

PPGPNDIS_PACKET PGPNdisPacketAllocWithBindingContext(
    PNDIS_STATUS status,
    PVPN_ADAPTER adapter);

PPGPNDIS_PACKET PGPNdisPacketAllocWithXformPacket(
    PNDIS_STATUS status,
    PVPN_ADAPTER adapter);

PPGPNDIS_PACKET PGPNdisPacketAllocWithSrcPacket(
    PNDIS_STATUS status,
    PVPN_ADAPTER adapter);

VOID PGPNdisPacketFree(
		PVPN_ADAPTER adapter,
		PPGPNDIS_PACKET packet);

VOID PGPNdisPacketFreeWithBindingContext(
		PVPN_ADAPTER adapter,
		PPGPNDIS_PACKET packet);

VOID PGPNdisPacketFreeSrcPacket(
		PVPN_ADAPTER adapter,
		PPGPNDIS_PACKET packet);

VOID PGPCopyPacketToBlock(
		PNDIS_PACKET	packet,
		PUCHAR			block,
		PUINT			bytesCopied);

VOID PGPnetAdjustTransferCompletePacket(PPGPNDIS_PACKET pgpPacket);

VOID PGPnetAdjustPacket(PPGPNDIS_PACKET packet);

void RequestEnqueue(
    PVPN_ADAPTER adapter,
    PPGPNDIS_REQUEST *request_list,
    PPGPNDIS_REQUEST request);

PPGPNDIS_REQUEST RequestDequeue(
    PVPN_ADAPTER adapter,
    PPGPNDIS_REQUEST *reqeust_list);

PPGPNDIS_REQUEST RequestRemoveByNdisRequest(
		PVPN_ADAPTER		adapter,
		PPGPNDIS_REQUEST		*reqeust_list,
		PNDIS_REQUEST		packet);

PPGPNDIS_REQUEST PGPNdisRequestAlloc(
    PNDIS_STATUS status,
    PVPN_ADAPTER adapter);

VOID PGPNdisRequestFree(
		PVPN_ADAPTER adapter,
		PPGPNDIS_REQUEST request);

NDIS_STATUS QueueForTransferComplete(
		PVPN_ADAPTER	adapter,
		NDIS_HANDLE		MacReceiveContext,
		PUCHAR			HeaderBuffer,
		UINT			HeaderBufferLength,
		PUCHAR			LookAheadBuffer,
		UINT			LookAheadBufferLength,
		UINT			PacketSize);

NDIS_STATUS TransformAndIndicate(
		PVPN_ADAPTER	adapter,
		PPGPNDIS_PACKET	pgpPacket);

USHORT iphdr_cksum(USHORT *iph);

void CopySrcBlockToXformBlock(PGPNDIS_PACKET *packet);

void GetIPAddressFromARP(PVPN_ADAPTER adapter, PVOID block);

VOID FragmentCollection(
    PVOID SystemArg1,
    PVOID Context,
    PVOID SystemArg2,
    PVOID SystemArg3);

#ifdef CHICAGO
VOID EventTimerRoutine(
    PVOID SystemArg1,
    PVOID Context,
    PVOID SystemArg2,
    PVOID SystemArg3);
#endif

VOID RequestTimerRoutine(
    PVOID SystemArg1,
    PVOID Context,
    PVOID SystemArg2,
    PVOID SystemArg3);

void NdisRequestTrace(
	PNDIS_REQUEST NdisRequest);


VOID PgpEventSet(
    IN PPGPEVENT_CONTEXT    pPgpEvent
    );

PVPN_ADAPTER AllocateVpnAdapter();

VOID FreeVpnAdapter(
    IN PVPN_ADAPTER    VpnAdapter
    );

VOID ProtocolCloseAdapterComplete(
    IN NDIS_HANDLE    ProtocolBindingContext,
    IN NDIS_STATUS    status
    );


VOID ProtocolOpenAdapterComplete(
    IN NDIS_HANDLE		ProtocolBindingContext,
    IN NDIS_STATUS      status,
    IN NDIS_STATUS      OpenErrorStatus
    );


NDIS_STATUS ProtocolReceive(
    IN NDIS_HANDLE		ProtocolBindingContext,
    IN NDIS_HANDLE      MacReceiveContext,
    IN PVOID            HeaderBuffer,
    IN UINT             HeaderBufferSize,
    IN PVOID            LookAheadBuffer,
    IN UINT             LookAheadBufferSize,
    IN UINT             PacketSize
    );


VOID ProtocolIndicateReceive(
    IN PVPN_ADAPTER		adapter,
    IN NDIS_HANDLE		MacContext,
    IN PUCHAR           HeaderBuffer,
    IN UINT             HeaderBufferSize,
    IN PVOID            LookAheadBuffer,
    IN UINT             LookAheadBufferSize,
    IN UINT             PacketSize
    );



VOID ProtocolReceiveComplete(
    IN NDIS_HANDLE		ProtocolBindingContext
    );


VOID ProtocolIndicateReceiveComplete(
    IN PVPN_ADAPTER		adapter
    );


VOID ProtocolRequestComplete(
    IN NDIS_HANDLE		ProtocolBindingContext,
    IN PNDIS_REQUEST    NdisRequest,
    IN NDIS_STATUS      Status
    );


VOID ProtocolResetComplete(
    IN NDIS_HANDLE		ProtocolBindingContext,
    IN NDIS_STATUS      status
    );


VOID ProtocolSendComplete(
    IN NDIS_HANDLE		ProtocolBindingContext,
    IN PNDIS_PACKET     Packet,
    IN NDIS_STATUS      Status
    );


VOID ProtocolStatus(
    IN NDIS_HANDLE		ProtocolBindingContext,
    IN NDIS_STATUS      Status,
    IN PVOID            StatusBuffer,
    IN UINT             StatusBufferSize
    );


VOID ProtocolStatusComplete(
    IN NDIS_HANDLE		ProtocolBindingContext
    );


VOID ProtocolTransferDataComplete(
    IN NDIS_HANDLE		ProtocolBindingContext,
    IN PNDIS_PACKET     Packet,
    IN NDIS_STATUS      Status,
    IN UINT             BytesTransferred
    );


VOID ProtocolBindAdapter(
    OUT PNDIS_STATUS    Status,
    IN NDIS_HANDLE      BindContext,
    IN PNDIS_STRING     DeviceName,
    IN PVOID            SystemSpecific1,
    IN PVOID            SystemSpecific2
    );


VOID ProtocolUnbindAdapter(
    OUT PNDIS_STATUS    Status,
    IN NDIS_HANDLE      ProtocolBindingContext,
    IN NDIS_HANDLE      UnbindContext
    );



VOID ProtocolUnload(
    VOID
    );

BOOLEAN BroadcastEthernetAddress(
	UCHAR* eth_dstAddress);

ULONG PgpKernelGetSystemTime();

void PacketHeadEnqueue(
    PVPN_ADAPTER adapter,
    PPGPNDIS_PACKET_HEAD *packet_head_list,
    PPGPNDIS_PACKET_HEAD packetHead);

PPGPNDIS_PACKET_HEAD PacketHeadDequeue(
    PVPN_ADAPTER adapter,
    PPGPNDIS_PACKET_HEAD *packet_head_list);

VOID PGPNdisPacketHeadFreeList(
		PVPN_ADAPTER adapter,
		PPGPNDIS_PACKET_HEAD packetHead,
		BOOLEAN outgoing);

VOID PGPNdisPacketHeadFree(
		PVPN_ADAPTER adapter,
		PPGPNDIS_PACKET_HEAD packetHead);

VOID PacketHeadListRemove(
		PVPN_ADAPTER			adapter,
		PPGPNDIS_PACKET_HEAD	*packet_list,
		PPGPNDIS_PACKET_HEAD	packetHead);

PPGPNDIS_PACKET_HEAD PacketHeadListQuery(
	PVPN_ADAPTER adapter,
	PPGPNDIS_PACKET_HEAD *packet_head_list,
	USHORT		 id,
	ULONG		 ipAddress);

PPGPNDIS_PACKET_HEAD PGPNdisPacketHeadAlloc(
    PNDIS_STATUS status,
    PVPN_ADAPTER adapter);

VOID InsertPGPNdisPacket(
	PVPN_ADAPTER adapter,
	PPGPNDIS_PACKET_HEAD packetHead,
	PPGPNDIS_PACKET pgpPacket);

VOID AppendPGPNdisPacket(
	PVPN_ADAPTER adapter,
	PPGPNDIS_PACKET_HEAD packetHead,
	PPGPNDIS_PACKET pgpPacket);

VOID IndicateReceiveClear(
		PVPN_ADAPTER adapter,
		PPGPNDIS_PACKET pgpPacket);

NDIS_STATUS MacSendPackets(
		PVPN_ADAPTER adapter,
		PPGPNDIS_PACKET_HEAD packetHead);

VOID ProtocolIndicatePackets(
		PVPN_ADAPTER adapter,
		PPGPNDIS_PACKET_HEAD packetHead);
#endif