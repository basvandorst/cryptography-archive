/****************************************************************************
*                                                                           *
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY     *
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE       *
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR     *
* PURPOSE.                                                                  *
*                                                                           *
* Copyright 1993-95  Microsoft Corporation.  All Rights Reserved.           *
*                                                                           *
****************************************************************************/

#define WANTVXDWRAPS

#include <basedef.h>

#include <vmm.h>
#include <debug.h>
#include <vxdwraps.h>
#include <vwin32.h>
#include <winerror.h>
#include <ndis.h>

#include "vpn.h"

#pragma LCODE

typedef DIOCPARAMETERS *LPDIOC;

DWORD _stdcall PGPnet_W32_DeviceIOControl(DWORD, DWORD, DWORD, LPDIOC);
DWORD _stdcall PGPnetCleanUp(void);

extern PVPN_ADAPTER VpnAdapterGlobal;

DWORD _stdcall PGPnet_W32_DeviceIOControl(
    DWORD dwService,
    DWORD dwDDB,
    DWORD hDevice,
    LPDIOC lpDIOCParms)
{
    DWORD dwRetVal;

    if (dwService == DIOC_OPEN) {
        // DIOC_OPEN is sent when VxD is loaded w/ CreateFile (this happens just after
        // SYS_DYNAMIC_INIT). Must return 0 to tell WIN32 that this VxD supports DEVIOCTL
        dwRetVal = 0;
    } else if (dwService == DIOC_CLOSEHANDLE) {
        // DIOC_CLOSEHANDLE is sent when VxD is unloaded w/ CloseHandle (this happens just
        // before SYS_DYNAMIC_EXIT). Dispatch to cleanup proc
        dwRetVal = PGPnetCleanUp();
    } else if ( 1 /*dwService == IOCTL_NDIS_QUERY_GLOBAL_STATS*/) {

		DWORD   BytesNeeded = 0;

		dwRetVal = ConfigurationIoctl(VpnAdapterGlobal,
									*(ULONG*)lpDIOCParms->lpvInBuffer,
									(PVOID)lpDIOCParms->lpvOutBuffer,
									lpDIOCParms->cbOutBuffer,
									(PDWORD)lpDIOCParms->lpcbBytesReturned,
									&BytesNeeded
									);
    } else {
        dwRetVal = NDIS_STATUS_NOT_RECOGNIZED;
    }
    return dwRetVal;
}

DWORD _stdcall PGPnetCleanUp(void)
{
    return VXD_SUCCESS;
}

