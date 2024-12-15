/*
 * Copyright (c) 1998, Network Associates, Inc.
 * All rights reserved.
 *
 * $Id: pgpNetPMHost.c,v 1.16 1999/03/29 01:07:39 elowe Exp $
 *
 */
#include <memory.h>
#include <ndis.h>

#include "vpn.h"
#include "vpndbg.h"
#include "pgpBase.h"
#include "pgpMem.h"

#include "pgpNetPMHost.h"

static NDIS_PHYSICAL_ADDRESS HighestAcceptableAddress = NDIS_PHYSICAL_ADDRESS_CONST(-1, -1);

/*
 * As far as this function is concerned, A secure host is one where
 * an SA is needed.
 *
 * There are two ways a host can be secure. It can either explicitly
 * be a secure host, or it can reside behind a gateway. Hosts marked
 * insecure and are behind a gateway are still protected by TUNNEL
 * mode to the gateway.
 */
PGPBoolean
PMIsSecureHost(NDIS_HANDLE handle, PGPUInt32 ipAddress)
{
	PGPnetPMContext *pContext = (PGPnetPMContext*)handle;
	PGPNetHostEntry *host = PMFindHost(handle, ipAddress);
	
	if (!host)
		return FALSE;

	if (host->hostType == kPGPnetSecureHost || host->childOf != -1)
		return TRUE;
		
	return FALSE;
}

/*
 * As far as this function is concerned, A secure host is one where
 * an SA is not needed.
 * 
 * An host is only insecure if it is marked as an insecure host, and it
 * does not live behind a gateway.
 */
PGPBoolean
PMIsInsecureHost(NDIS_HANDLE handle, PGPUInt32 ipAddress)
{
	PGPNetHostEntry *host = PMFindHost(handle, ipAddress);
	PGPnetPMContext *pContext = (PGPnetPMContext*)handle;

	if (!host)
		return FALSE;
	
	if (host->hostType == kPGPnetInsecureHost && host->childOf == -1) 
		return TRUE;
	
	return FALSE;
}

/* XXX this is exceedingly inefficient right now */
PGPNetHostEntry *
PMFindHost(NDIS_HANDLE handle, PGPUInt32 ipAddress)
{
	PGPUInt32 i = 0;
	PGPNetHostEntry *gwHost = 0;
	PGPnetPMContext *pContext = (PGPnetPMContext*)handle;
	
	DBG_FUNC("PMFindHost")

    DBG_ENTER();

	for (i = 0; i < pContext->numHosts; i++) {
		PGPNetHostEntry *host = &pContext->pKernelHostList[i];
		if (ipAddress == host->ipAddress) {
			if (host->hostType == kPGPnetSecureGateway) {
				/* found a secure gateway. Now try to find
				 * secure host, which is preferred
				 */
				gwHost = host;
				continue;
			}
			DBG_LEAVE(host);
			return host;
		}
	}
	if (gwHost) {
		DBG_LEAVE(gwHost);
		return gwHost;
	}

	for (i = 0; i < pContext->numHosts; i++) {
		PGPNetHostEntry *host = &pContext->pKernelHostList[i];
		if ((ipAddress & host->ipMask) == (host->ipAddress & host->ipMask)) {
			DBG_LEAVE(host);
			return host;
		}
	}

	DBG_LEAVE(0);
	return 0;
}

PGPNetHostEntry *
PMAddHost(NDIS_HANDLE handle, PGPNetHostEntry *host)
{
	PGPNetHostEntry *newHost = 0;
	PGPnetPMContext *pContext = (PGPnetPMContext*)handle;
	NDIS_STATUS status;
	
	DBG_FUNC("PMAddHost")

    DBG_ENTER();

	if (pContext->numHosts == pContext->maxHosts) {
		if (!pContext->pKernelHostList) {
			pContext->maxHosts = 16;

#ifndef USERLAND_TEST
			status = NdisAllocateMemory(&(pContext->pKernelHostList), 
				(pContext->maxHosts) * sizeof(PGPNetHostEntry),
				0, 
				HighestAcceptableAddress);

			if (status != NDIS_STATUS_SUCCESS)
				return 0;
#else
			pContext->pKernelHostList = (PGPNetHostEntry*) calloc(pContext->maxHosts,
								sizeof(PGPNetHostEntry));
			if (!pContext->pKernelHostList)
				return 0;
#endif	/* USERLAND_TEST */

			pgpClearMemory(pContext->pKernelHostList, 
				(pContext->maxHosts) * sizeof(PGPNetHostEntry));
		} else {
			PGPNetHostEntry *tmp;
			PGPUInt32 newSize = (pContext->maxHosts+16) * sizeof(PGPNetHostEntry);
			PGPUInt32 oldSize = pContext->maxHosts * sizeof(PGPNetHostEntry);

#ifndef USERLAND_TEST
			status = NdisAllocateMemory(&tmp, 
				newSize,
				0,
				HighestAcceptableAddress);
			if (status != NDIS_STATUS_SUCCESS) {
				return 0;
			} else {
				// zero out new block
				pgpClearMemory(tmp, newSize);
				// copy old data to new block
				pgpCopyMemory(pContext->pKernelHostList /*src*/,
					tmp /*dest*/,
					oldSize);
				// free old block
				NdisFreeMemory(pContext->pKernelHostList, oldSize, 0);
				// point to new
				pContext->pKernelHostList = tmp;
			}
#else
			tmp = (PGPNetHostEntry*) realloc(pContext->pKernelHostList,
								(pContext->maxHosts+16) * sizeof(PGPNetHostEntry));
#endif /* USERLAND_TEST */

			pContext->pKernelHostList = tmp;
			pContext->maxHosts += 16;
		}
	}

	newHost = &pContext->pKernelHostList[pContext->numHosts];
	pgpCopyMemory(host /*src*/, 
		newHost /*dest*/,
		sizeof(PGPNetHostEntry));
	pContext->numHosts++;

	DBG_LEAVE(newHost);

	return newHost;
}

PGPUInt32 
PMParentHostIP(NDIS_HANDLE handle, PGPNetHostEntry *host)
{
	PGPnetPMContext *pContext = (PGPnetPMContext*)handle;

	return (pContext->pKernelHostList[host->childOf].ipAddress);
}

void
PMClearHosts(NDIS_HANDLE handle)
{
	PGPnetPMContext *pContext = (PGPnetPMContext*)handle;

	if (pContext->pKernelHostList) 
		NdisFreeMemory(pContext->pKernelHostList, 
			pContext->maxHosts * sizeof(PGPNetHostEntry),
			0);

	pContext->pKernelHostList = 0;
	pContext->maxHosts = 0;
	pContext->numHosts = 0;
}

