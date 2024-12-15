/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetKernelXChng.h,v 1.12 1999/05/04 22:34:19 elowe Exp $
____________________________________________________________________________*/

#ifndef Included_pgpNetKernelXChng_h
#define Included_pgpNetKernelXChng_h

#include "pgpIKE.h"
#include "cfgdef.h"

/*
 * These are the messages that the kernel sends to the service
 */
#pragma pack(push, 8)

typedef enum _PGPNET_KERNEL_MESSAGE_TYPE
{
	PGPnetMessageHeaderType = 0,
	PGPnetMessageICMPType,
	PGPnetMessageSARequest,
	PGPnetMessageSARekey,
	PGPnetMessageSADied,
	PGPnetMessageError,
	PGPnetMessageRASdisconnect,
	PGPnetMessageReserved

} PGPNET_KERNEL_MESSAGE_TYPE;

typedef struct _PGPnetSARequestMsg {
	UINT	ipAddress;
	UINT	ipAddrStart;
	UINT	ipMaskEnd;
	UCHAR	reserved[1];
} PGPnetSARequestMsg;

typedef struct _PGPnetSADiedMsg {
	UCHAR	spi[4];
	UCHAR	reserved[1];
} PGPnetSADiedMsg;

typedef struct _PGPnetSARekeyMsg {
	UCHAR	spi[4];
	UCHAR	reserved[1];
} PGPnetSARekeyMsg;

typedef struct _PGPnetErrorMsg {
	UINT	errorCode;
	UINT	ipAddress;
	UCHAR	reserved[1];
} PGPnetErrorMsg;

typedef struct _PGPnet_ICMP_Message {
	UINT	ip_packets_received;
	UCHAR	reserved[1];
} PGPnet_ICMP_Message;

#define MaxSlotSize 64
#define MaxSlots	16

typedef struct _PGPnetMessageHeader {

	ULONG               head;
	ULONG               tail;

	ULONG               maxSlots;
	ULONG               maxSlotSize;

	UCHAR				reserved[1];
} PGPnetMessageHeader;

typedef struct _PGPnetMessage
{
	UCHAR reserved[MaxSlotSize - 1];
} PGPnetMessage;

typedef struct _PGPMESSAGE_CONTEXT
{
	PGPNET_KERNEL_MESSAGE_TYPE	messageType;

	union {
		PGPnetMessageHeader header;

		PGPnetMessage		message;

	};

} PGPMESSAGE_CONTEXT, *PPGPMESSAGE_CONTEXT;

/*
 * These are the messages that the service sends to the kernel.
 * This isn't a complete list, since come messages send "simple"
 * types (like complete ikeSA structures, or PGPUInts)
 */
typedef struct _PGPnetKernelConfig {
	PGPBoolean				bPGPnetEnabled;
	PGPBoolean				bAllowUnconfigHost;
	PGPBoolean				bAttemptUnconfigHost;
} PGPnetKernelConfig;

typedef struct _PGPnetStat {
	PGPUInt32 spi;
	PGPUInt32 packets;
	PGPUInt32 bytes;
} PGPnetStat;

#pragma pack(pop)

#endif /* Included_pgpNetKernelXChng_h */
