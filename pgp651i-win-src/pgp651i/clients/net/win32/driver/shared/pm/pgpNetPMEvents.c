/*
 * Copyright (c) 1998, Network Associates, Inc.
 * All rights reserved.
 *
 * $Id: pgpNetPMEvents.c,v 1.1 1999/05/21 21:29:08 elowe Exp $
 *
 */
#include <ndis.h>

#include "ipdef.h"
#include "vpn.h"
#include "vpndbg.h"

#include "pgpMem.h"
#include "pgpNetKernel.h"
#include "pgpNetKernelXChng.h"
#include "pgpNetPMEvents.h"

PGPnetPMStatus
PMfireSARequestEvent(PVPN_ADAPTER adapter, 
					PGPUInt32 ipAddress,
					PGPUInt32 ipAddrStart,
					PGPUInt32 ipMaskEnd)
{
	DBG_FUNC("sFireSARequestEvent")

	PGPMESSAGE_CONTEXT	*	kernelMessageContext;
	PGPnetMessageHeader *	kernelMessageHeader;
	PGPnetSARequestMsg *	pSARequestMsg;
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
		kernelMessageContext->messageType = PGPnetMessageSARequest;

		pSARequestMsg = (PGPnetSARequestMsg*)&kernelMessageContext->message;
		pSARequestMsg->ipAddress = ipAddress;
		pSARequestMsg->ipAddrStart = ipAddrStart;
		pSARequestMsg->ipMaskEnd = ipMaskEnd;

		kernelMessageHeader->head = head;

		NdisReleaseSpinLock(&adapter->general_lock);

		PgpEventSet(&adapter->pgpEvent);

		DBG_LEAVE(kPGPNetPMPacketWaiting);
		return kPGPNetPMPacketWaiting;
	} else {
		DBG_LEAVE(kPGPNetPMPacketDrop);
		return kPGPNetPMPacketDrop;
	}
}

void
PMfireSADiedEvent(PVPN_ADAPTER adapter, PGPipsecSPI spi)
{
	DBG_FUNC("sFireSADiedEvent")

	PGPMESSAGE_CONTEXT	*	kernelMessageContext;
	PGPnetMessageHeader *	kernelMessageHeader;
	PGPnetSADiedMsg *		pSADiedMsg;
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
		kernelMessageContext->messageType = PGPnetMessageSADied;

		pSADiedMsg = (PGPnetSADiedMsg*)&kernelMessageContext->message;
		pgpCopyMemory(spi, /*src*/
			pSADiedMsg->spi, /*dest*/
			sizeof(PGPipsecSPI));

		kernelMessageHeader->head = head;

		NdisReleaseSpinLock(&adapter->general_lock);

		PgpEventSet(&adapter->pgpEvent);
	}

	DBG_LEAVE(0);
}

void
PMfireSARekeyEvent(PVPN_ADAPTER adapter, PGPipsecSPI spi)
{
	DBG_FUNC("sFireSARekeyEvent")

	PGPMESSAGE_CONTEXT	*	kernelMessageContext;
	PGPnetMessageHeader *	kernelMessageHeader;
	PGPnetSARekeyMsg *		pSARekeyMsg;
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
		kernelMessageContext->messageType = PGPnetMessageSARekey;

		pSARekeyMsg = (PGPnetSARekeyMsg*)&kernelMessageContext->message;
		pgpCopyMemory(spi, /*src*/
			pSARekeyMsg->spi, /*dest*/
			sizeof(PGPipsecSPI));

		kernelMessageHeader->head = head;

		NdisReleaseSpinLock(&adapter->general_lock);

		PgpEventSet(&adapter->pgpEvent);
	}

	DBG_LEAVE(0);
}

void
PMfireErrorEvent(PVPN_ADAPTER adapter, 
				PGPUInt32 ipAddress,
				PGPUInt32 errorCode)
{
	DBG_FUNC("sFireErrorEvent")

	PGPMESSAGE_CONTEXT	*	kernelMessageContext;
	PGPnetMessageHeader *	kernelMessageHeader;
	PGPnetErrorMsg		*	pErrorMsg;
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
		kernelMessageContext->messageType = PGPnetMessageError;

		pErrorMsg = (PGPnetErrorMsg*)&kernelMessageContext->message;
		pErrorMsg->errorCode = errorCode;
		pErrorMsg->ipAddress = ipAddress;

		kernelMessageHeader->head = head;

		NdisReleaseSpinLock(&adapter->general_lock);

		PgpEventSet(&adapter->pgpEvent);
	}

	DBG_LEAVE(0);
}