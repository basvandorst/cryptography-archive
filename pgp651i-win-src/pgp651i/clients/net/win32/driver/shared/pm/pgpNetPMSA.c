/*
 * Copyright (c) 1998, Network Associates, Inc.
 * All rights reserved.
 *
 * $Id: pgpNetPMSA.c,v 1.24 1999/03/02 22:49:47 elowe Exp $
 *
 */
#include <ndis.h>

#include "vpn.h"
#include "vpndbg.h"
#include "pgpMem.h"
#include "pgpNetKernel.h"
#include "pgpEndianConversion.h"

#include "pgpNetPMSA.h"

static PGPnetKernelSA * sCreateSA(NDIS_HANDLE handle);

static PGPnetKernelSA * sFindPendingSA(NDIS_HANDLE handle, 
									   PGPUInt32 ipAddress,
									   PGPUInt32 ipAddrStart,
									   PGPUInt32 ipMaskEnd);

static PGPBoolean		sCheckSPI(PGPnetKernelSA *pSA, PGPUInt32 spi);

static NDIS_PHYSICAL_ADDRESS HighestAcceptableAddress = NDIS_PHYSICAL_ADDRESS_CONST(-1, -1);

PGPnetKernelSA *
PMFindSA(NDIS_HANDLE handle, 
		 PGPUInt32 ipAddress, 
		 PGPUInt32 ipAddrStart,
		 PGPBoolean incoming)
{
	PGPnetPMContext *pContext = (PGPnetPMContext*)handle;
	PGPUInt32 i = 0;
	PGPnetKernelSA *pSA = 0;

	for (i = 0; i < pContext->numSAs; i++) {
		pSA = &(pContext->pKernelSAList[i]);
		if (!pSA->pending && !incoming && !pSA->ikeSA.activeOut)
			continue;
		if (pSA->ikeSA.ipAddress == ipAddress) {
			if (ipAddrStart) {
				/*
				 * Ok, we found an SA for the tunnel machine, but
				 * this SA needs to match the destination machine
				 * (ipAddrStart) as well
				 *
				 */
				if (pSA->ikeSA.destIsRange) {
					/* dealing with a range */
					if ((ipAddrStart >= pSA->ikeSA.ipAddrStart) &&
						(ipAddrStart <= pSA->ikeSA.ipMaskEnd))
						return pSA;
				} else {
					/* dealing with a mask */
					if ((ipAddrStart & pSA->ikeSA.ipMaskEnd) == 
						(pSA->ikeSA.ipAddrStart & pSA->ikeSA.ipMaskEnd))
						return pSA;
				}
			} else 
				return pSA;
		}
	}
	return 0;
}

PGPnetKernelSA *
PMFindSAspi(NDIS_HANDLE handle, PGPUInt32 ipAddress, PGPUInt32 spi)
{
	PGPnetPMContext *pContext = (PGPnetPMContext*)handle;
	PGPUInt32 i = 0;

	DBG_FUNC("PMFindSAspi")

	DBG_ENTER();

	for (i = 0; i < pContext->numSAs; i++) {
		if (pContext->pKernelSAList[i].ikeSA.ipAddress == ipAddress) {
			// found matching IP address, now need to look for matching spi.
			// look in all transforms
			if (sCheckSPI(&(pContext->pKernelSAList[i]), spi)) {
				DBG_LEAVE(&(pContext->pKernelSAList[i]));
				return &(pContext->pKernelSAList[i]);
			}
		}
	}

	DBG_LEAVE(0);
	return 0;
}

void
PMRemoveSA(NDIS_HANDLE handle, PGPipsecSPI spi)
{
	PGPnetPMContext *pContext = (PGPnetPMContext*)handle;
	PGPnetKernelSA *saList;
	PGPUInt32 i = 0;

	saList = pContext->pKernelSAList;

	for (i = 0; i < pContext->numSAs; i++) {
		if (pgpMemoryEqual(saList[i].ikeSA.transform[0].u.ipsec.inSPI,
					spi,
					sizeof(PGPipsecSPI))) {
			pgpCopyMemory(&saList[i+1] /*src*/,
				&saList[i] /*dest*/,
				(pContext->numSAs-i-1)*sizeof(PGPnetKernelSA));
			pContext->numSAs--;
			return;
		}
	}
	return;
}

PGPnetKernelSA *
PMAddSA(NDIS_HANDLE handle, PGPikeSA *ikeSA)
{
	PGPnetPMContext *pContext = (PGPnetPMContext*)handle;
	PGPnetKernelSA *newSA = 0;
	LARGE_INTEGER birthTime;
	
	DBG_FUNC("PMAddSA")

    DBG_ENTER();

	if (!ikeSA)
		return 0;

	newSA = sFindPendingSA(handle, 
		ikeSA->ipAddress, 
		ikeSA->ipAddrStart,
		ikeSA->ipMaskEnd);

	if (!newSA)
		newSA = sCreateSA(handle);

	if (newSA) {
		/* copy all the data from the ikeSA into the kernelSA */
		pgpCopyMemory(ikeSA /*src*/,
			&(newSA->ikeSA) /*dest*/,
			sizeof(PGPikeSA));

		newSA->pending = FALSE;
		newSA->reKeyInProgress = FALSE;

#ifdef CHICAGO
		/* XXX do something here */
//		NdisGetCurrentSystemTime(&birthTime);
		pgpCopyMemory(&(birthTime.HighPart) /*src*/,
			&(newSA->birthTime) /*dest*/,
			4);
		pgpCopyMemory(&(birthTime.LowPart) /*src*/,
			&(newSA->birthTime) + 4 /*dest*/,
			4);
#else
		KeQuerySystemTime(&birthTime);
		newSA->birthTime = birthTime.QuadPart;
#endif
	}

	DBG_LEAVE(newSA);

	return newSA;
}

PGPnetKernelSA *
PMAddPendingSA(NDIS_HANDLE handle, 
			   PGPUInt32 ipAddress,
			   PGPUInt32 ipAddrStart,
			   PGPUInt32 ipMaskEnd)
{
	PGPnetKernelSA *newSA = 0;

	DBG_FUNC("PMAddPendingSA")

    DBG_ENTER();

	newSA = sCreateSA(handle);
	if (newSA) {
		newSA->pending = TRUE;
		newSA->reKeyInProgress = FALSE;
		newSA->ikeSA.ipAddress = ipAddress;
		newSA->ikeSA.ipAddrStart = ipAddrStart;
		newSA->ikeSA.ipMaskEnd = ipMaskEnd;
	}

	DBG_LEAVE(newSA);

	return newSA;
}

void
PMRemovePendingSA(NDIS_HANDLE handle, 
				  PGPUInt32 ipAddress,
				  PGPUInt32 ipAddrStart,
				  PGPUInt32 ipMaskEnd)
{
	PGPnetPMContext *pContext = (PGPnetPMContext*)handle;
	PGPnetKernelSA *saList;
	PGPUInt32 i = 0;

	saList = pContext->pKernelSAList;

	for (i = 0; i < pContext->numSAs; i++) {
		if (saList[i].ikeSA.ipAddress == ipAddress && saList[i].pending) {
			/* need to check ipAddrStart */
			if (ipAddrStart) {
				/*
				 * Ok, we found an SA for the tunnel machine, but
				 * this SA needs to match the destination machine
				 * (ipAddrStart) as well. At this point we always
				 * have a mask, since this is our SARequest that failed,
				 * and we always use masks.
				 *
				 */
				if ((ipAddrStart & saList[i].ikeSA.ipMaskEnd) != 
					(saList[i].ikeSA.ipAddrStart & saList[i].ikeSA.ipMaskEnd)) {
					/* No match, continue searching */
					continue;
				}
			}
			pgpCopyMemory(&saList[i+1] /*src*/,
				&saList[i] /*dest*/,
				(pContext->numSAs-i-1)*sizeof(PGPnetKernelSA));
			pContext->numSAs--;
			return;
		}
	}
	return;
}

void
PMUpdateSA(NDIS_HANDLE handle, PGPipsecSPI spi)
{
	PGPnetPMContext *pContext = (PGPnetPMContext*)handle;
	PGPnetKernelSA *saList;
	PGPUInt32 i = 0;

	saList = pContext->pKernelSAList;

	for (i = 0; i < pContext->numSAs; i++) {
		if (pgpMemoryEqual(saList[i].ikeSA.transform[0].u.ipsec.inSPI,
					spi,
					sizeof(PGPipsecSPI))) {
			saList[i].ikeSA.activeOut = FALSE;
			return;
		}
	}
	return;
}

PGPnetKernelSA *
sCreateSA(NDIS_HANDLE handle)
{
	PGPnetPMContext *pContext = (PGPnetPMContext*)handle;
	PGPnetKernelSA *newSA = 0;
	NDIS_STATUS status;

	DBG_FUNC("sCreateSA")

    DBG_ENTER();
	
	if (pContext->numSAs == pContext->maxSAs) {
		if (!pContext->pKernelSAList) {
			pContext->maxSAs = 16;

#ifndef USERLAND_TEST
			status = NdisAllocateMemory(&(pContext->pKernelSAList), 
				pContext->maxSAs * sizeof(PGPnetKernelSA),
				0, 
				HighestAcceptableAddress);

			if (status != NDIS_STATUS_SUCCESS) {
				return 0;
			} else {
				pgpClearMemory(pContext->pKernelSAList,
					pContext->maxSAs * sizeof(PGPnetKernelSA));
			}
#else
			pContext->pKernelSAList = (PGPnetKernelSA*) calloc(pContext->maxSAs,
								sizeof(PGPnetKernelSA));
#endif

		} else {
			PGPnetKernelSA *tmp;
			PGPUInt32 newSize = 
				(pContext->maxSAs+16) * sizeof(PGPnetKernelSA);
			PGPUInt32 oldSize = 
				pContext->maxSAs * sizeof(PGPnetKernelSA);

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
				pgpCopyMemory(pContext->pKernelSAList /*src*/,
					tmp /*dest*/,
					oldSize);
				// free old block
				NdisFreeMemory(pContext->pKernelSAList, oldSize, 0);
				// point to new
				pContext->pKernelSAList = tmp;
			}
#else
			tmp = (PGPnetKernelSA*) realloc(pContext->pKernelSAList,
				(pContext->maxSAs+16) * sizeof(PGPnetKernelSA));
#endif

			pContext->pKernelSAList = tmp;
			pContext->maxSAs += 16;
		}
	}
	newSA = &(pContext->pKernelSAList[pContext->numSAs]);
	pgpClearMemory(newSA, sizeof(PGPnetKernelSA)); /* initialize to 0 */
	
	pContext->numSAs++; /* add one to the base SA counter */

	DBG_LEAVE(newSA);

	return newSA;
}

PGPnetKernelSA *
sFindPendingSA(NDIS_HANDLE handle,
			   PGPUInt32 ipAddress,
			   PGPUInt32 ipAddrStart,
			   PGPUInt32 ipMaskEnd)
{
	DBG_FUNC("sFindPendingSA")

	PGPnetPMContext *pContext = (PGPnetPMContext*)handle;
	PGPUInt32 i = 0;
	PGPnetKernelSA *pSA = 0;

	DBG_ENTER();

	for (i = 0; i < pContext->numSAs; i++) {
		pSA = &(pContext->pKernelSAList[i]);
		if (pSA->ikeSA.ipAddress == ipAddress && pSA->pending) {
			if (ipAddrStart) {
				if ((ipAddrStart & pSA->ikeSA.ipMaskEnd) != 
					(pSA->ikeSA.ipAddrStart & pSA->ikeSA.ipMaskEnd)) {
					/* No match, continue searching */
					continue;
				}
			}
			DBG_LEAVE(pSA);
			return pSA;
		}
	}
	DBG_LEAVE(0);
	return 0;
}

PGPBoolean
sCheckSPI(PGPnetKernelSA *pSA, PGPUInt32 spi)
{
	PGPUInt16 i = 0;
	DBG_FUNC("sCheckSPI")

	DBG_ENTER()
	for (i = 0; i < pSA->ikeSA.numTransforms; i++) {
		PGPUInt32 saSPI = PGPEndianToUInt32(kPGPBigEndian, 
			pSA->ikeSA.transform[i].u.ipsec.inSPI);

		if (saSPI == spi) {
			DBG_LEAVE(TRUE);
			return TRUE;
		}
	}

	DBG_LEAVE(FALSE);
	return FALSE;
}

void
PMClearSAs(NDIS_HANDLE handle)
{
	PGPnetPMContext *pContext = (PGPnetPMContext*)handle;

	if (pContext->pKernelSAList) 
		NdisFreeMemory(pContext->pKernelSAList, 
			pContext->maxSAs * sizeof(PGPnetKernelSA),
			0);

	pContext->pKernelSAList = 0;
	pContext->maxSAs = 0;
	pContext->numSAs = 0;
}