;	Copyright (C) 1998 Network Associates, Inc.
;	All rights reserved.
;	
;	pgpMLdevdcl.asm - necessary dispatch declarations
;
;	$Id: pgpMLdevdcl.asm,v 1.2 1998/08/11 13:43:51 pbj Exp $
;
    .386p
    include vmm.inc
    include vdmad.inc
    include debug.inc

Declare_Virtual_Device PGPMLOCK, 0, 1, PGPMEMLOCK_control,\
    Undefined_Device_ID, Undefined_Init_Order

Begin_Control_Dispatch PGPMEMLOCK
    Control_Dispatch Sys_Dynamic_Device_Init, _OnSysDynamicDeviceInit, cCall
    Control_Dispatch Sys_Dynamic_Device_Exit, _OnSysDynamicDeviceExit, cCall
    Control_Dispatch W32_DeviceIOControl,     _OnDeviceIoControl, cCall, <esi>
End_Control_Dispatch   PGPMEMLOCK

    end
