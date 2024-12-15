#ifndef _PGPVM_H_
#define _PGPVM_H_


#pragma warning(disable:4035)
#pragma warning(disable:4005)
#include <basedef.h>

#include <vmm.h>
#define PAGEMAPGLOBAL       0x40000000
#ifdef WANTVXDWRAPS
#  include <vxdwraps.h>
#else
#  define VMM_MAP_PHYS_TO_LINEAR_INVALID  0xFFFFFFFF
#endif
#include <vwin32.h>

#pragma warning(disable:4035)
#pragma warning(disable:4005)

PVOID _stdcall MemPageLock(ULONG lpMem, ULONG cbSize);
VOID  _stdcall MemPageUnlock(ULONG lpMem, ULONG cbSize);
ULONG _stdcall SetWin32Event(ULONG hWin32Event);
VOID  _stdcall CloseVxdHandle(ULONG hWin32Event);

#if 0
ULONG VXDINLINE
_VWIN32_SetWin32Event(ULONG hEvent)
{
    _asm push [hEvent]
    VMMCall(_VWIN32_SetWin32Event)
	_asm mov eax, 1
	_asm or eax, eax
    _asm add esp, 4*3
}
#endif

ULONG VXDINLINE
_VWIN32_CloseVxDHandle(ULONG hEvent)
{
    _asm push [hEvent]
    VMMCall(_VWIN32_CloseVxDHandle)
    _asm add esp, 4*1
}

HEVENT VXDINLINE
Call_VM_Event(HVM hvm, void (__cdecl *pfnEvent)(), ULONG ulRefData)
{
    HEVENT hevent;
    _asm mov ebx, [hvm]
    _asm mov edx, [ulRefData]
    _asm mov esi, [pfnEvent]
    VMMCall(Call_VM_Event)
    _asm mov [hevent], esi
    return(hevent);
}

PVOID VXDINLINE
_LinPageLock(ULONG Page, ULONG nPages, ULONG Flags)
{
    _asm push [Flags]
    _asm push [nPages]
    _asm push [Page]
    VMMCall(_LinPageLock);
    _asm add esp, 4*3
}    

PVOID VXDINLINE
_LinPageUnLock(ULONG Page, ULONG nPages, ULONG Flags)
{
    _asm push [Flags]
    _asm push [nPages]
    _asm push [Page]
    VMMCall(_LinPageUnLock);
    _asm add esp, 4*3
}    

#pragma warning(default:4035)
#pragma warning(default:4005)

#endif // _PGPVM_H_
