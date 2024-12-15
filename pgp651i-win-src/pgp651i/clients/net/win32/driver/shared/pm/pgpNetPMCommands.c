/*
 * Copyright (c) 1998, Network Associates, Inc.
 * All rights reserved.
 *
 * $Id: pgpNetPMCommands.c,v 1.5 1999/03/02 22:49:47 elowe Exp $
 *
 */
#include <ndis.h>

#include "vpndbg.h"

#include "pgpNetKernel.h"
#include "pgpNetKernelXChng.h"
#include "pgpNetPMConfig.h"
#include "pgpNetPMHost.h"
#include "pgpNetPMSA.h"

NDIS_STATUS
PGPnetPMNewSA(PGPnetPMContext *pContext, 
			  void *data, 
			  UINT dataLen, 
			  ULONG *pSrcBufferLen)
{
    DBG_FUNC("PGPnetPMNewSA")

    DBG_ENTER();

	*pSrcBufferLen = sizeof(PGPikeSA);

	if (dataLen != *pSrcBufferLen) {
		DBG_LEAVE(NDIS_STATUS_INVALID_LENGTH);
		return NDIS_STATUS_INVALID_LENGTH;
	}

	if (!PMAddSA(pContext, (PGPikeSA*) data)) {
		DBG_LEAVE(NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	*pSrcBufferLen = 0;
    DBG_LEAVE(NDIS_STATUS_SUCCESS);
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
PGPnetPMFailedSA(PGPnetPMContext *pContext, 
				 void *data,
				 UINT dataLen,
				 ULONG *pSrcBufferLen)
{
    DBG_FUNC("PGPnetPMFailedSA")

	PGPikeMTSAFailed * pSAFailed = (PGPikeMTSAFailed*)data;

    DBG_ENTER();

	*pSrcBufferLen = sizeof(PGPikeMTSAFailed);

	if (dataLen != *pSrcBufferLen) {
		DBG_LEAVE(NDIS_STATUS_INVALID_LENGTH);
		return NDIS_STATUS_INVALID_LENGTH;
	}

	PMRemovePendingSA(pContext, pSAFailed->ipAddress,
		pSAFailed->u.ipsec.ipAddrStart,
		pSAFailed->u.ipsec.ipMaskEnd);

	*pSrcBufferLen = 0;
    DBG_LEAVE(NDIS_STATUS_SUCCESS);
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
PGPnetPMUpdateSA(PGPnetPMContext *pContext,
				 void *data,
				 UINT dataLen,
				 ULONG *pSrcBufferLen)
{
    DBG_FUNC("PGPnetPMUpdateSA")

    DBG_ENTER();

	*pSrcBufferLen = sizeof(PGPipsecSPI);

	if (dataLen != *pSrcBufferLen) {
		DBG_LEAVE(NDIS_STATUS_INVALID_LENGTH);
		return NDIS_STATUS_INVALID_LENGTH;
	}

	PMUpdateSA(pContext, (PGPByte*)data);

	*pSrcBufferLen = 0;
    DBG_LEAVE(NDIS_STATUS_SUCCESS);
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
PGPnetPMRemoveSA(PGPnetPMContext *pContext, 
				 void *data,
				 UINT dataLen,
				 ULONG *pSrcBufferLen)
{
	DBG_FUNC("PGPnetPMRemoveSA")

	DBG_ENTER();

	*pSrcBufferLen = sizeof(PGPipsecSPI);

	if (dataLen != *pSrcBufferLen) {
		DBG_LEAVE(NDIS_STATUS_INVALID_LENGTH);
		return NDIS_STATUS_INVALID_LENGTH;
	}
	(void) PMRemoveSA(pContext, (PGPByte*) data);

	*pSrcBufferLen = 0;
	DBG_LEAVE(NDIS_STATUS_SUCCESS);
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
PGPnetPMNewHost(PGPnetPMContext *context, 
				void *data,
				UINT dataLen,
				ULONG *pSrcBufferLen)
{
    DBG_FUNC("PGPnetPMNewHost")

    DBG_ENTER();

	*pSrcBufferLen = sizeof(PGPNetHostEntry);

	if (dataLen != *pSrcBufferLen) {
		DBG_LEAVE(NDIS_STATUS_INVALID_LENGTH);
		return NDIS_STATUS_INVALID_LENGTH;
	}

	if (!PMAddHost(context, (PGPNetHostEntry*) data)) {
		DBG_LEAVE(NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	*pSrcBufferLen = 0;
	DBG_LEAVE(NDIS_STATUS_SUCCESS);
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
PGPnetPMAllHosts(PGPnetPMContext *context, 
				void *data,
				UINT dataLen,
				ULONG *pSrcBufferLen)
{
    DBG_FUNC("PGPnetPMAllHosts")

	int nItems = 0, i = 0;
	PGPNetHostEntry *host = data;

    DBG_ENTER();

	*pSrcBufferLen = sizeof(PGPNetHostEntry);

	nItems = dataLen / *pSrcBufferLen;

	if ((dataLen % *pSrcBufferLen) != 0) {
		DBG_LEAVE(NDIS_STATUS_INVALID_LENGTH);
		return NDIS_STATUS_INVALID_LENGTH;
	}

	for (i = 0; i < nItems; i++) {
		if (!PMAddHost(context, host++)) {
			DBG_LEAVE(NDIS_STATUS_FAILURE);
			return NDIS_STATUS_FAILURE;
		}
	}

	*pSrcBufferLen = 0;
	DBG_LEAVE(NDIS_STATUS_SUCCESS);
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
PGPnetPMNewConfig(PGPnetPMContext *pContext, void *data,
				  UINT dataLen,
				  ULONG *pSrcBufferLen)
{
	DBG_FUNC("PGPnetPMNewConfig")

	PGPError err = kPGPError_NoErr;

    DBG_ENTER();

	*pSrcBufferLen = sizeof(PGPnetKernelConfig);

	if (dataLen != *pSrcBufferLen) {
		DBG_LEAVE(NDIS_STATUS_INVALID_LENGTH);
		return NDIS_STATUS_INVALID_LENGTH;
	}

	/*
	* There is a new config coming, so we need to wipe
	* out everything in the hosts list. The host list
	* will be populated again by following AddHost messages
	*/
	PMClearHosts(pContext);

	err = PMAddConfig(pContext, (PGPnetKernelConfig*)data);
	if (IsPGPError(err)) {
		DBG_LEAVE(NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	*pSrcBufferLen = 0;
	DBG_LEAVE(NDIS_STATUS_SUCCESS);
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
PGPnetPMShutdown(PGPnetPMContext *pContext, 
				void *data,
				UINT dataLen,
				ULONG *pSrcBufferLen)
{
    DBG_FUNC("PGPnetPMShutdown")

	/* XXX delete all configuration
	 * all SAs
	 * all pending SAs
	 */
	 
	PMClearHosts(pContext);
	PMClearSAs(pContext);
	PMClearConfig(pContext);

	*pSrcBufferLen = 0;
	DBG_LEAVE(NDIS_STATUS_SUCCESS);
	return NDIS_STATUS_SUCCESS;
}
