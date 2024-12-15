#include "version.h"
#include "vpn.h"
#include "vpndbg.h"

#include "pgpNetKernel.h"

NDIS_STATUS PolicyManagerInitialize(
	IN PDRIVER_OBJECT	DriverObject,
	IN PUNICODE_STRING	RegistryPath,
	OUT PNDIS_HANDLE	PolicyManagerHandle
)
{
	DBG_FUNC("PolicyManagerInitialize")
    NDIS_STATUS         status;
	PGPnetPMContext *pContext = 0;
	PGPError err = kPGPError_NoErr;

	NDIS_PHYSICAL_ADDRESS HighestAcceptableAddress = NDIS_PHYSICAL_ADDRESS_CONST(-1, -1);

#ifdef PM_EMULATION
	PDummySA dummySA;
#endif

	DBG_ENTER();

    status = NdisAllocateMemory(PolicyManagerHandle,
                                sizeof(PGPnetPMContext),
                                0,
                                HighestAcceptableAddress
                                );

#ifndef PM_EMULATION

    if (status != NDIS_STATUS_SUCCESS)
    {
        DBG_PRINT(("!!!!! NdisAllocateMemory failed status=%Xh\n", status););
        *PolicyManagerHandle = NULL;
		goto failout;
    }
    else
    {
        NdisZeroMemory(*PolicyManagerHandle, sizeof(PGPnetPMContext));

		pContext = (PGPnetPMContext*)*PolicyManagerHandle;
		err = PGPNewIPsecContext(&(pContext->ipsecContext));

		if (IsPGPError(err)) {
			DBG_PRINT(("PGPNewIPsecContext error: %d\n", err););
			status = NDIS_STATUS_FAILURE;
			goto failout;
		}
    }
#endif

#ifdef PM_EMULATION
	dummySA = &((PGPnetPMContextRef)*PolicyManagerHandle)->dummySA;

	dummySA->ipAddress =		0;
	dummySA->outgoingKey = 'O';
	dummySA->incomingKey = 'I';
	dummySA->validSA = TRUE;
	dummySA->pendingSA = FALSE;
#endif

failout:

	DBG_LEAVE(status);
    return (status);
};

VOID PolicyManagerUninitialize(
	IN NDIS_HANDLE		PolicyManagerHandle)
{
	DBG_FUNC("PolicyManagerUnintialize")

	PGPnetPMContext *pContext = 0;

	DBG_ENTER();

	pContext = (PGPnetPMContext*)PolicyManagerHandle;

#ifndef PM_EMULATION
	if (pContext && pContext->ipsecContext)
		PGPFreeIPsecContext(pContext->ipsecContext);
#endif
	
	if (PolicyManagerHandle != NULL)
		NdisFreeMemory(PolicyManagerHandle,
					sizeof(PGPnetPMContext),
					0);

	DBG_LEAVE(0);
};
