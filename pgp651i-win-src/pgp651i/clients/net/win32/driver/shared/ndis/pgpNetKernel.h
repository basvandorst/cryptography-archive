/*
 * Copyright (c) 1998 Network Associates, Inc.
 * All rights reserved.
 *
 * Various public interfaces and structure for the kernel driver
 *
 * $Id: pgpNetKernel.h,v 1.23 1999/05/20 03:05:57 elowe Exp $
 */

#ifndef Included_pgpNetKernel_h
#define Included_pgpNetKernel_h

#include "vpn.h"
#ifdef PM_EMULATION
#include "dummypm.h"
#endif

#pragma pack(push, 8)

#define MAX_IPSEC_PACKETSIZE_INCREASE	672

#include "pgpIKE.h"
#include "pgpTypes.h"
#include "pgpIPsecErrors.h"
#include "pgpIPsecContext.h"

typedef struct _PGPnetKernelSA {
	PGPikeSA 		ikeSA;				/* base SA w/ transforms */
	PGPUInt32		packetsSent; 		/* xfer packet count for this SA */
	PGPUInt32		bytesSent;			/* xfer byte count for this SA */
	PGPUInt32		sequenceWindowUpper;/* replay attacks */
	PGPUInt32		sequenceWindowLower;/* replay attacks */
	PGPUInt32		sequenceWindow;		/* replay attacks */
	PGPBoolean		pending;			/* to prevent multiple SA requests */
	PGPBoolean		reKeyInProgress;
	PGPInt64		birthTime;
} PGPnetKernelSA;

#define	kMaxNetHostNameLength			255
#define	kMaxNetHostSharedSecretLength	255
#define kMaxNetHostIdentityLength		255
#define kMaxNetHostIASNLength			192
#define kMaxNetHostKeyIDSize			34

typedef struct _PGPNetHostEntry
{
	PGPUInt32				hostType;
	PGPUInt32				ipAddress;
	PGPUInt32				ipMask;
	PGPInt32				childOf;
	char					hostName[kMaxNetHostNameLength + 1];
	PGPUInt32				identityType;
	PGPUInt32				identityIPAddress;
	char					identity[kMaxNetHostIdentityLength + 1];
	char					sharedSecret[kMaxNetHostSharedSecretLength + 1];

	PGPPublicKeyAlgorithm	authKeyAlg;
	PGPByte					authKeyExpKeyID[kMaxNetHostKeyIDSize];
	PGPByte					authCertIASN[kMaxNetHostIASNLength];
	PGPUInt32				authCertIASNLength;
} PGPNetHostEntry;

typedef enum PGPnetPMStatus_ {
	kPGPNetPMPacketSent 	= 0,
	kPGPNetPMPacketWaiting,
	kPGPNetPMPacketDrop,
	kPGPNetPMPacketEncrypt,
	kPGPNetPMPacketClear
} PGPnetPMStatus;

typedef struct _PGPnetPMContext
{
	/* global config info */
	struct _PGPnetKernelConfig *pConfig;

	/* host list */
	unsigned int		maxHosts;
	unsigned int		numHosts;
	PGPNetHostEntry *	pKernelHostList;

	/* SA list */
	unsigned int		maxSAs;
	unsigned int		numSAs;
	PGPnetKernelSA *	pKernelSAList;

	/* IPsec Context */
	PGPIPsecContextRef	ipsecContext;
#ifdef PM_EMULATION
	DummySA	dummySA;
#endif

} PGPnetPMContext, *PGPnetPMContextRef;

#pragma pack(pop)

/*
 * Called by the NDIS driver when a RAS disconnect event occurs
 */
void
PGPnetRASdisconnect(PVPN_ADAPTER adaptor);

void
PGPnetRASconnect(PVPN_ADAPTER adaptor, ULONG ipAddress);

/*
 * Called by the NDIS driver to quickly determine whether the
 * packet will be encrypted. Helps to avoid a buffer copy in the
 * unencrypted case. Copying is bad. This is the Heavy version that
 * starts the SA process
 */
PGPnetPMStatus
PGPnetPMNeedTransform(struct _PGPnetPMContext *pContext, 
					  PGPUInt32 ipAddress,
					  PGPUInt16 port,
					  PGPBoolean incoming,
					  void * ipBuffer,
					  PGPUInt32 ipBufferSize,
					  void *reserved);

/*
 * Called by the NDIS driver to quickly determine whether the
 * packet will be encrypted. Helps to avoid a buffer copy in the
 * unencrypted case. Copying is bad. This is the lightweight version
 * that is used for fragments. It does not start the SA process.
 */
PGPnetPMStatus
PGPnetPMNeedTransformLight(struct _PGPnetPMContext *pContext, 
					  PGPUInt32 ipAddress,
					  PGPBoolean incoming,
					  void *reserved);

/*
 * Called by the NDIS driver. The encrypted packet is placed in
 * the dest pointer. Memory is allocated by the NDIS driver.
 */
PGPnetPMStatus
PGPnetPMDoTransform(struct _PGPnetPMContext *pContext, 
					PGPNDIS_PACKET *packet,
					PGPBoolean incoming,
					PVPN_ADAPTER adapter);

NDIS_STATUS
PGPnetPMNewSA(struct _PGPnetPMContext *pContext,
			  void *data,
			  UINT dataLen,
			  ULONG *pSrcBufferLen);

NDIS_STATUS
PGPnetPMRemoveSA(struct _PGPnetPMContext *pContext,
				 void *data,
				 UINT dataLen,
				 ULONG *pSrcBufferLen);

NDIS_STATUS
PGPnetPMFailedSA(struct _PGPnetPMContext *pContext,
				 void *data,
				 UINT dataLen,
				 ULONG *pSrcBufferLen);

NDIS_STATUS
PGPnetPMUpdateSA(struct _PGPnetPMContext *pContext,
				 void *data,
				 UINT dataLen,
				 ULONG *pSrcBufferLen);

NDIS_STATUS
PGPnetPMNewHost(struct _PGPnetPMContext *pContext,
				void *data,
				UINT dataLen,
				ULONG *pSrcBufferLen);

NDIS_STATUS
PGPnetPMAllHosts(struct _PGPnetPMContext *pContext,
				void *data,
				UINT dataLen,
				ULONG *pSrcBufferLen);

NDIS_STATUS
PGPnetPMShutdown(struct _PGPnetPMContext *pContext,
				void *data,
				UINT dataLen,
				ULONG *pSrcBufferLen);

NDIS_STATUS
PGPnetPMNewConfig(struct _PGPnetPMContext *pContext,
				  void *data,
				  UINT dataLen,
				  ULONG *pSrcBufferLen);

/* spi = 0 if packet is not an IPsec packet */

PGPError
PGPnetIPsecGetAddrAndSPI(struct _PGPnetPMContext *pContext,
						 PGPNDIS_PACKET *packet, 
						 PGPUInt32 *ipAddress,
						 PGPUInt32 *spi);

/*
 * Called by the Policy Manager in PGPnetPMDoTransform().
 * Memory is already allocated by the NDIS driver.
 */

PGPError
PGPnetIPsecApplySA(PVPN_ADAPTER adapter,
				   struct _PGPnetPMContext *pContext,
				   PGPNDIS_PACKET *packet,
				   PGPnetKernelSA *sa,
				   PGPBoolean incoming,
				   PGPBoolean tunnel);

#endif /* Included_pgpNetKernel_h */
