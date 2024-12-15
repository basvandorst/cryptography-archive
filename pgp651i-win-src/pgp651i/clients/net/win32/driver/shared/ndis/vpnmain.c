#include "version.h"
#include "vpn.h"
#include "vpndbg.h"

//#if ( defined(DBG) && (DBG != 0)) || ( defined(DBG_MESSAGE) )

ULONG VpnDbgMask = DBG_DEBUG_ALL;

//#endif


NDIS_STATUS DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NDIS_STRING DriverName = NDIS_STRING_CONST("PGPmac");
NDIS_STRING MSTCPName  = NDIS_STRING_CONST("MSTCP");

/*
 * Global handles.
 */
PGPNET_DRIVER PGPnetDriver = {0};


/*
 DriverEntry is the entrypoint when system load this driver. Besides it's 
 responsibility to register the various NDIS entrypoints, it's the right 
 place to do the initialization job of the Policy Manager.
 It's the Policy Manager's job to initialize the rest of the driver like
 IPSEC Engine (Encryption Engine).
 We might have to initialize the Configuration Manager here also.
 */

NDIS_STATUS DriverEntry(
    IN PDRIVER_OBJECT DriverObject,       
    IN PUNICODE_STRING RegistryPath
    )
{
    DBG_FUNC("DriverEntry")
    NDIS_STATUS status;
	
	DBG_ENTER();

#ifdef CHICAGO
	DBG_BREAK();
#endif


	DBG_PRINT(("pgpmac: Build Date: "__DATE__" Time: "__TIME__"\n"););	

    if (PGPnetDriver.NdisMacHandle)
    {
		// How could this happen? Would we ever come into this point?
		DBG_BREAK();
		DBG_LEAVE(NDIS_STATUS_SUCCESS);
        return (NDIS_STATUS_SUCCESS);
    }
    
#if ( defined(CHICAGO) && !defined(MEMPHIS) )
	if (PGPnetDriver.NdisProtocolHandle)
	{
		// OK, we have the protocolHandle but not the MacHandle
		// This means we are given a second chance to register
		// MacEntery

		status = MacDriverEntry(DriverObject, RegistryPath);
		DBG_LEAVE(status);
		return (status);

	}
#endif

    NdisZeroMemory(&PGPnetDriver, sizeof(PGPnetDriver));

    /*
     * Initialize NDIS wrapper.
     */
    NdisInitializeWrapper(&PGPnetDriver.NdisWrapperHandle,
                          DriverObject,
                          RegistryPath,
                          NULL
                          );

    /*
     * Initialize the protocol driver
     */
    status = ProtocolDriverEntry(DriverObject, RegistryPath);
    
#if ( !defined(CHICAGO) || defined(MEMPHIS) )
    if (status == NDIS_STATUS_SUCCESS)
    {
        /*
         * Initialize the MAC driver.
         */
        status = MacDriverEntry(DriverObject, RegistryPath);
    }
#endif

    if (status == NDIS_STATUS_SUCCESS)
    {
        /*
         * Intialize the rest of the VPN driver.
         */
        status = PolicyManagerInitialize(DriverObject,
                               RegistryPath,
                               &PGPnetDriver.PolicyManagerHandle
                               );
    }

    if (status == NDIS_STATUS_SUCCESS)
    {
        DBG_PRINT(("||||| Driver loaded successfully!\n"););
    }
    else
    {
        DriverUnload(&PGPnetDriver);
		NdisZeroMemory(&PGPnetDriver, sizeof(PGPnetDriver));

        status = STATUS_UNSUCCESSFUL;
    }

	DBG_LEAVE(status);
    return (status);
}


/*
 */

VOID DriverUnload(
    NDIS_HANDLE MacMacContext
    )
{
    DBG_FUNC("DriverUnload")

	DBG_ENTER();
    ASSERT(MacMacContext == &PGPnetDriver);

    if (PGPnetDriver.PolicyManagerHandle)
    {
        PolicyManagerUninitialize(PGPnetDriver.PolicyManagerHandle);
        PGPnetDriver.PolicyManagerHandle = NULL;
    }

    MacUnload(PGPnetDriver.NdisMacHandle);

    ProtocolUnload();
    
    if (PGPnetDriver.NdisWrapperHandle)
    {
        NdisTerminateWrapper(PGPnetDriver.NdisWrapperHandle, NULL);
        PGPnetDriver.NdisWrapperHandle = NULL;
    }

    DBG_LEAVE(0);
}
