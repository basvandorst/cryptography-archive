#ifdef CHICAGO

#undef BINARY_COMPATIBLE
#undef NDIS_MINIPORT_DRIVER
#include <ndis.h>

#include "pgpvm.h"
#ifndef DBG_MESSAGE
#define DBG_MESSAGE 1
#endif
#include "vpndbg.h"

#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG

//void PriorityEventThunk( void );

PVOID _stdcall MemPageLock(
    ULONG       lpMem,
    ULONG       cbSize
    )
{
    DBG_FUNC("MemPageLock")

    ULONG       LinOffset;
    PCHAR       VirtPtr;

	DWORD cbExtra = lpMem & PAGEMASK;

	DBG_ENTER();

	DBG_PRINT(("lpMem Address: %xh\n", lpMem););

    LinOffset = lpMem & 0xfff; // page offset of memory to map

	VirtPtr = (PCHAR) _LinPageLock(PAGE(lpMem), NPAGES(cbSize + cbExtra), PAGEMAPGLOBAL);

	if (VirtPtr != NULL)
		VirtPtr += LinOffset;

    if (VirtPtr == NULL)
    {
        DBG_PRINT(("_LinPageLock FAILED!\n"););
    }
	DBG_PRINT(("VirtPtr Address: %xh\n", VirtPtr););
	DBG_PRINT(("lpMem Address: %xh\n", lpMem););
	DBG_LEAVE(0);
    //return ((PVOID) VirtPtr);
	return ((PVOID) lpMem);
}

//#######################################################################
//
VOID _stdcall MemPageUnlock(
    ULONG       lpMem,
    ULONG       cbSize
    )
{
    DBG_FUNC("MemPageUnlock")

	DWORD cbExtra = lpMem & PAGEMASK;
	DBG_ENTER();

	_LinPageUnLock(PAGE(lpMem), NPAGES(cbSize + cbExtra), PAGEMAPGLOBAL);

	DBG_LEAVE(0);
}

#if 0
// Signals sent to Win32 must be sent from the system VM, so this routine
// is scheduled as a callback in the system VM.
// ON ENTRY: EDX has the hWin32Event
STATIC VOID _cdecl SetWin32EventCallback( VOID )
{
    DBG_FUNC("SetWin32EventCallback")

    ULONG       dwRetVal;
    ULONG       hWin32Event;
    
    // WARNING: Don't trash EDX before you save it!
    _asm mov [hWin32Event], edx
    
    dwRetVal = _VWIN32_SetWin32Event(hWin32Event);

    if (dwRetVal == 0)
    {
        DBG_PRINT(("_VWIN32_SetWin32Event Error=%Xh\n",dwRetVal););
    }
}

// Signals sent to Win32 must be sent from the system VM, so this routine
// is called to schedule a callback in the system VM to do the signal.
ULONG _stdcall SetWin32Event(
    ULONG       hWin32Event
    )
{
    DBG_FUNC("SetWin32Event")

    HVM         hSysVM;
    ULONG       hVMEvent;

    hSysVM = Get_Sys_VM_Handle();
    if (hSysVM == 0)
    {
        DBG_PRINT(("Get_Sys_VM_Handle FAILED\n"););
    }
    hVMEvent = Call_VM_Event(hSysVM, SetWin32EventCallback, hWin32Event);

    if (hVMEvent == 0)
    {
        DBG_PRINT(("Call_VM_Event FAILED\n"););
    }
    return(hVMEvent);
}
#endif

VOID _stdcall CloseVxdHandle(
    ULONG       hWin32Event
    )
{
    DBG_FUNC("CloseVxdHandle")

    ULONG       dwRetVal;

    // Gotta close the handle here in the VxD even though the DLL opened it!
    dwRetVal = _VWIN32_CloseVxDHandle(hWin32Event);

    if (dwRetVal == 0)
    {
        DBG_PRINT(("_VWin32_CloseVxDHandle(%X) FAILED\n",hWin32Event););
    }
}
#endif // CHICAGO
