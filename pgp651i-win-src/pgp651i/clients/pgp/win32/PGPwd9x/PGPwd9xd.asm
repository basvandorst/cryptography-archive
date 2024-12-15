;	Copyright (C) 1998 Network Associates, Inc.
;	All rights reserved.
;	
;	pgpwd9xd.asm - necessary dispatch declarations
;
;	$Id: PGPwd9xd.asm,v 1.1 1999/04/09 02:37:20 wjb Exp $
;
    .386p

SEGNUM equ 1

    include vmm.inc
    include vdmad.inc
	include local.inc

Declare_Virtual_Device PGPWD9X, 0, 1, PGPWDEL_control,\
    Undefined_Device_ID, Undefined_Init_Order

Begin_Control_Dispatch PGPWDEL
    Control_Dispatch Sys_Dynamic_Device_Init, _OnSysDynamicDeviceInit, cCall
    Control_Dispatch Sys_Dynamic_Device_Exit, _OnSysDynamicDeviceExit, cCall
    Control_Dispatch W32_DeviceIOControl,     _OnDeviceIoControl, cCall, <esi>
End_Control_Dispatch   PGPWDEL

MakeCDecl      IFSMgr_InstallFileSystemApiHook
MakeCDecl      IFSMgr_RemoveFileSystemApiHook
MakeCDecl      UniToBCSPath
MakeCDecl      _VWIN32_QueueUserApc
MakeCDecl      IFSMgr_Block
MakeCDecl      IFSMgr_Wakeup

    end
