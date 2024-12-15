#define WANTVXDWRAPS

#include <basedef.h>
#include <vmm.h>
//#include <debug.h>

#include "wrappers.h"

#include <vxdwraps.h>
#include <vwin32.h>
#ifndef DBG_MESSAGE
#define DBG_MESSAGE 1
#endif
#include "vpndbg.h"

#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG

extern void PriorityEventThunk( void );

VOID __stdcall PriorityEventHandler(VMHANDLE hVM, PVOID Refdata, CRS *pRegs)
{
	DBG_FUNC("PriorityEventHandler")

	HANDLE hWin32Event = Refdata;

	DBG_ENTER();

	_VWIN32_SetWin32Event( hWin32Event );
	
	DBG_LEAVE(0);
}

VOID __stdcall CallWin32Event(
	ULONG		hWin32Event
	)
{
	DBG_FUNC("CallWin32Event")
	HVM		hSysVM;

	DBG_ENTER();

	hSysVM = Get_Sys_VM_Handle();
			 
	if (hSysVM == 0)
	{
		DBG_PRINT(("Get_Sys_VM_Handle FAILED\n"););
	}


	Call_Priority_VM_Event(LOW_PRI_DEVICE_BOOST,
				hSysVM,
				PEF_WAIT_FOR_STI+PEF_WAIT_NOT_CRIT,
				hWin32Event,
				PriorityEventThunk,
				0);

	DBG_LEAVE(0);
}
