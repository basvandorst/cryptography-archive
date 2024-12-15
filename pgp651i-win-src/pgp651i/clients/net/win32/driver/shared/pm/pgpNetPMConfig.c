/*
 * Copyright (c) 1998, Network Associates, Inc.
 * All rights reserved.
 *
 * $Id: pgpNetPMConfig.c,v 1.9.6.1 1999/06/14 03:23:06 elowe Exp $
 *
 */
#ifndef USERLAND_TEST
# include <ndis.h>
# include "vpn.h"
# include "vpndbg.h"
#endif

#include "pgpMem.h"
#include "pgpNetKernelXChng.h"
#include "pgpNetKernel.h"
#include "pgpNetPMConfig.h"

static NDIS_PHYSICAL_ADDRESS HighestAcceptableAddress = NDIS_PHYSICAL_ADDRESS_CONST(-1, -1);

PGPError
PMAddConfig(NDIS_HANDLE handle, PGPnetKernelConfig *pConfig)
{
	PGPnetPMContext		*pContext = (PGPnetPMContext*) handle;
	NDIS_STATUS			status;

	DBG_FUNC("PMAddConfig")

    DBG_ENTER();

	/* if config doesn't exist, create memory for it */
	if (!pContext->pConfig) {

#ifndef USERLAND_TEST
		status = NdisAllocateMemory(&(pContext->pConfig), 
			sizeof(PGPnetKernelConfig),
			0, 
			HighestAcceptableAddress);

			if (status != NDIS_STATUS_SUCCESS)
				return kPGPError_OutOfMemory;
#else
		pContext->pConfig = malloc(sizeof(PGPnetKernelConfig));
		if (!pContext->pConfig)
			return kPGPError_OutOfMemory;
#endif /* USERLAND_TEST */

	}

	/* clear out anything there */
	pgpClearMemory(pContext->pConfig, sizeof(PGPnetKernelConfig));

	/* copy configuration values */
	pContext->pConfig->bPGPnetEnabled = pConfig->bPGPnetEnabled;
	pContext->pConfig->bAllowUnconfigHost = pConfig->bAllowUnconfigHost;
	pContext->pConfig->bAttemptUnconfigHost = pConfig->bAttemptUnconfigHost;

	DBG_LEAVE(kPGPError_NoErr);

	return kPGPError_NoErr;
}

void
PMClearConfig(NDIS_HANDLE handle)
{
	PGPnetPMContext		*pContext = (PGPnetPMContext*) handle;

	if (pContext->pConfig)
		NdisFreeMemory(pContext->pConfig, 
			sizeof(PGPnetKernelConfig),
			0);

	pContext->pConfig = 0;
}
