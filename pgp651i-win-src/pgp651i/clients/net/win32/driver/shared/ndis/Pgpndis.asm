.LALL
;	This file is borrowed from the PPTP driver from the canceled NetCrypto
;	project with added functionality like Win95 version detection(Gold or OSR2).
;	More necessary features not provided by NDIS Lib will follow.
;	

    TITLE $PGPLNK
    .386P

INCLUDE VMM.INC
INCLUDE NDIS.INC
INCLUDE NETVXD.INC			; Net VxD initialization oredr


Declare_Virtual_Device PGPMAC, 3, 10, PGPMAC_Control, UNDEFINED_DEVICE_ID, \
    MAC_INIT_ORDER, 0, 0, 0

IFDEF CHICAGO
VxD_LOCKED_DATA_SEG

Public Initialized	
	Initialized	 DB 0

VxD_LOCKED_DATA_ENDS
ENDIF

VxD_LOCKED_CODE_SEG


BeginProc C_Device_Init

extern _DriverEntry@8:NEAR

IFDEF CHICAGO
    mov  al, Initialized
	cmp  al, 0					
	jnz  Succeed
	inc  Initialized			
ENDIF

    push 0
    push 0

    call _DriverEntry@8

    cmp  eax,NDIS_STATUS_SUCCESS

    jne  Failed
Succeed:
    clc
    ret

Failed:
    stc
    ret
EndProc C_Device_Init

Begin_Control_Dispatch PGPMAC
    Control_Dispatch Device_Init,       			C_Device_Init
IFDEF CHICAGO
    Control_Dispatch Sys_Dynamic_Device_Init,       C_Device_Init
	Control_Dispatch W32_DeviceIoControl, PGPnet_W32_DeviceIOControl, \
        sCall, <ecx, ebx, edx, esi>
ENDIF
End_Control_Dispatch PGPMAC

VxD_CODE_SEG

BeginProc _PriorityEventThunk

   sCall PriorityEventHandler, <ebx,edx,ebp>
	ret

EndProc _PriorityEventThunk

PgpKernelGetSystemTime@0 PROC C PUBLIC

    VMMCall Get_System_Time
    ret

PgpKernelGetSystemTime@0 ENDP


VxD_CODE_ENDS



VxD_LOCKED_CODE_ENDS

	END