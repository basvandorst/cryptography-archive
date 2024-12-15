#ifndef Included_vxdevent_h
#define Included_vxdevent_h

#ifndef VMHANDLE
typedef DWORD VMHANDLE;
#endif
VOID __stdcall PriorityEventHandler(VMHANDLE hVM, PVOID Refdata, CRS *pRegs);

VOID __stdcall CallWin32Event(
	ULONG		hWin32Event
	);

#endif