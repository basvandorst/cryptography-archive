; Source: Writing Windows VxDs and Device Drivers, 2nd Edition 
; by Karen Hazzah, R&D Books, Miller Freeman, Inc. 1996

include local.inc
include ifsmgr.inc
include vdmad.inc
include vpicd.inc
include vwin32.inc
include shell.inc

R0_OPENCREATFILE        equ 0D500h  ; Open/Create a file
R0_OPENCREAT_IN_CONTEXT equ 0D501h  ; Open/Create file in current context
R0_READFILE             equ 0D600h  ; Read a file, no context
R0_WRITEFILE            equ 0D601h  ; Write to a file, no context
R0_READFILE_IN_CONTEXT  equ 0D602h  ; Read a file, in thread context
R0_WRITEFILE_IN_CONTEXT equ 0D603h  ; Write to a file, in thread context
R0_CLOSEFILE            equ 0D700h  ; Close a file
R0_GETFILESIZE          equ 0D800h  ; Get size of a file

StartStdCall    MACRO   Name, Param
StartCDecl      Name&@&Param
ENDM

EndStdCall      MACRO   Name, Param
EndCDecl        Name&@&Param
ENDM

MakeCDecl   _ModifyPageBits

MakeCDecl   _Assign_Device_V86_Pages

MakeCDecl   _DeAssign_Device_V86_Pages

MakeCDecl   _PhysIntoV86

MakeCDecl   _MapIntoV86

MakeCDecl   _GetNulPageHandle

MakeCDecl   _PageReserve

MakeCDecl   _PageCommitPhys

MakeCDecl   _PageDecommit

;  void cdecl _Deallocate_Device_CB_Area( DWORD NumBytes, 
;                                         DWORD Flags ))
;
MakeCDecl   _Deallocate_Device_CB_Area 

;  EVENTHANDLE Call_Priority_VM_Event(DWORD PriorityBoost, 
;              VMHANDLE hVM, DWORD Flags, 
;              void * Refdata, 
;              CALLBACK EventCallback, DWORD Timeout );

StartCDecl  Call_Priority_VM_Event

PriorityBoost EQU [ebp+8]
hVM EQU [ebp+12]
Flags EQU [ebp+16]
Refdata EQU [ebp+20]
EventCallback EQU[ebp+24]
Timeout EQU [ebp+28]

      push  ebp
      mov   ebp, esp

      mov   eax, DWORD PTR PriorityBoost
      mov   ebx, DWORD PTR hVM
      mov   ecx, DWORD PTR Flags
      mov   edx, DWORD PTR Refdata
      mov   esi, DWORD PTR EventCallback
      mov   edi, DWORD PTR Timeout
      VMMCall Call_Priority_VM_Event
      mov   eax, esi    ; eax=event handle

      pop ebp
      ret

EndCDecl    Call_Priority_VM_Event

;  void * _stdcall Map_Flat( BYTE SegOffset, BYTE OffOffset )
;
;
StartStdCall   Map_Flat, 8

      pop   edx   ; save ret addr in unused reg
      pop   ebx   ; segment
      xor   bh,bh ; BL=segment
      mov   ah,bl ; AH=segment
      pop   ebx   ; offset
      xor   bh,bh ; BL=offset
      mov   al,bl ; AL=offset 
      VMMcall Map_Flat  ; AH=seg AL=off
      jmp   edx   ; jump to caller's ret addr

EndStdCall     Map_Flat, 8

;  BOOL _stdcall Hook_V86_Page( DWORD PageNum, 
;                                CALLBACK Callback )
;
StartStdCall   Hook_V86_Page, 8

      pop edx        ; save ret addr in unused reg
      pop eax        ; PageNum
      pop esi        ; Callback
      VMMCall Hook_V86_Page
      mov eax, 1     ; assume TRUE ret val
   	jnc @f
   	xor eax, eax   ; carry set, error, so FALSE ret val
@@:
   	jmp edx			; jump to caller's ret addr

EndStdCall     Hook_V86_Page, 8

;  BOOL _stdcall Unhook_V86_Page( DWORD PageNum, 
;                                CALLBACK Callback )
;
StartStdCall   Unhook_V86_Page, 8

      pop edx        ; save ret addr in unused reg
      pop eax        ; PageNum
      pop esi        ; Callback
      VMMCall Unhook_V86_Page
      mov eax, 1     ; assume TRUE ret val
   	jnc @f
   	xor eax, eax   ; carry set, error, so FALSE ret val
@@:
   	jmp edx			; jump to caller's ret addr

EndStdCall     Unhook_V86_Page, 8

;  PTCB _stdcall Get_Initial_Thread_Handle( VMHANDLE hVM )
StartStdCall   Get_Initial_Thread_Handle, 4

      pop edx     ; Get return address
      pop ebx     ; Get VMHandle
      VxDCall Get_Initial_Thread_Handle
      mov eax, edi ; move thread hnd into return 
      jmp edx     ; return addr still in edx

EndStdCall      Get_Initial_Thread_Handle, 4

;  BOOL _stdcall Install_IO_Handler( PortNum, Callback  )
StartStdCall   Install_IO_Handler, 8

      pop ebx        ; save ret addr in unused reg
      pop edx        ; PortNum
      pop esi        ; Callback
      VMMCall Install_IO_Handler
      mov eax, 1     ; assume TRUE ret val
   	jnc @f
   	xor eax, eax   ; carry set, error, so FALSE ret val
   @@:
   	jmp ebx			; jump to caller's ret addr

EndStdCall     Install_IO_Handler, 8

;  BOOL _stdcall Remove_IO_Handler( PortNum )
;
StartStdCall   Remove_IO_Handler, 4

      pop ebx        ; save ret addr in unused reg
      pop edx        ; PortNum
      VMMCall Remove_IO_Handler
      mov eax, 1     ; assume TRUE ret val
   	jnc @f
   	xor eax, eax   ; carry set, error, so FALSE ret val
   @@:
   	jmp ebx			; jump to caller's ret addr

EndStdCall     Remove_IO_Handler, 4

;  void _stdcall Enable_Local_Trapping( VMHANDLE hVM, DWORD PortNum )
;
StartStdCall   Enable_Local_Trapping, 8

      pop ecx        ; save ret addr in unused reg
      pop ebx        ; hVM
      pop edx        ; PortNum
      VMMcall Enable_Local_Trapping
      jmp ecx

EndStdCall     Enable_Local_Trapping, 8

;  void _stdcall Disable_Local_Trapping( VMHANDLE hVM, DWORD PortNum )
;
StartStdCall   Disable_Local_Trapping, 8

      pop ecx        ; save ret addr in unused reg
      pop ebx        ; hVM
      pop edx        ; PortNum
      VMMcall Disable_Local_Trapping
      jmp ecx

EndStdCall     Disable_Local_Trapping, 8

;  void cdecl Save_Client_State( CLIENT_STRUCT * pSavedRegs
;
StartCdecl     Save_Client_State

pSavedRegs EQU [ebp+8]

      push ebp
      mov ebp, esp

	  pushad		 ; service doesn't claim to save any regs
	  mov edi, pSavedRegs
      VMMcall Save_Client_State
	  popad

      pop ebp
      ret

EndCdecl	   Save_Client_State

;  void cdecl Restore_Client_State( CLIENT_STRUCT * pRestoredRegs
;
StartCdecl     Restore_Client_State

pSavedRegs EQU [ebp+8]

      push ebp
      mov ebp, esp

	  pushad		 ; service doesn't claim to save any regs
	  mov edi, pSavedRegs
      VMMcall Restore_Client_State
	  popad

      pop ebp
      ret

EndCdecl	   Restore_Client_State

;  void cdecl Begin_Nest_Exec( void )
;
StartCdecl     Begin_Nest_Exec

      push ebp
      mov ebp, esp

	  pushad		 ; service doesn't claim to save any regs
      VMMcall Begin_Nest_Exec
	  popad

      pop ebp
      ret

EndCdecl	   Begin_Nest_Exec

;  void cdecl End_Nest_Exec( void )
;
StartCdecl     End_Nest_Exec

      push ebp
      mov ebp, esp

	  pushad		 ; service doesn't claim to save any regs
      VMMcall End_Nest_Exec
	  popad

      pop ebp
      ret

EndCdecl	   End_Nest_Exec

;  void _stdcall Simulate_Far_Call( WORD seg, WORD off )                  
;
StartStdCall   Simulate_Far_Call, 8     

      pop eax        ; save ret addr in unused reg
      pop ecx        ; segment
      pop edx        ; offset 
      VMMcall Simulate_Far_Call      
      jmp eax

EndStdCall     Simulate_Far_Call, 8      

;  void _stdcall Simulate_Push( DWORD val )                               
;
StartStdCall   Simulate_Push, 4

      pop edx        ; save ret addr in unused reg
      pop eax        ; val    
      VMMcall Simulate_Push      
      jmp edx

EndStdCall     Simulate_Push, 4

; HANDLE cdecl IFSMgr_Ring0_OpenCreateFile( BOOL bInContext, PCHAR filename, 
;                               WORD mode, WORD attrib, 
;                               BYTE action, BYTE flags, 
;                               WORD *pError, BYTE *pAction)
StartCdecl      IFSMgr_Ring0_OpenCreateFile

bInContext EQU [ebp+8]
filename EQU [ebp+12]
mode EQU [ebp+16]
attrib EQU [ebp+20]
action EQU[ebp+24]
flags EQU [ebp+28]
pError EQU [ebp+32]
pAction EQU [ebp+36]

        push ebp
        mov  ebp, esp

        mov dl, action          
        mov dh, flags           
        mov cx, attrib          
        mov bx, mode            
        mov esi, filename       
        mov eax, R0_OPENCREATFILE
        cmp WORD PTR bInContext, 0          
        je  @f
        mov eax, R0_OPENCREAT_IN_CONTEXT
@@:
        
        VxDCall IFSMgr_Ring0_FileIO
        mov esi, pError         ; esi->error code
        jnc @f
        mov WORD PTR [esi], ax  ;give caller error code
        xor eax, eax    ;return failure to caller
@@: 
        mov esi, pAction
        mov DWORD PTR [esi], ecx    ; action performed
        ;returning with handle in eax 

        pop ebp
        ret

EndCdecl     IFSMgr_Ring0_OpenCreateFile

; BOOL cdecl IFSMgr_Ring0_CloseFile(HANDLE filehandle, WORD *pError)
StartCdecl  IFSMgr_Ring0_CloseFile

filehandle EQU [ebp+8]
pError EQU [ebp+12]

        push ebp
        mov  ebp, esp

        mov ebx, filehandle         
        mov eax, R0_CLOSEFILE   ; func code
        VxDCall IFSMgr_Ring0_FileIO
        mov ecx, 1      ; assume returning true
        jnc @f
        mov esi, pError
        mov WORD PTR [esi], ax
        xor ecx, ecx    ; returning false
@@:
        mov eax, ecx  ;error code or zero

        pop ebp
        ret

    
EndCdecl     IFSMgr_Ring0_CloseFile

; DWORD cdecl IFSMgr_Ring0_WriteFile(BOOL bInContext, HANDLE filehandle, 
;                           PVOID buf, DWORD count, DWORD filepos, 
;                           WORD  *perr))
StartCdecl  IFSMgr_Ring0_WriteFile

bInContext EQU [ebp+8]
filehandle EQU [ebp+12]
buf EQU [ebp+16]
count EQU [ebp+20]
filepos EQU [ebp+24]
pError EQU [ebp+28]

        push ebp
        mov  ebp, esp

        mov ebx, filehandle          
        mov esi, buf           
        mov ecx, count        
        mov edx, filepos             
        mov eax, R0_WRITEFILE
        cmp WORD PTR bInContext, 0          
        je  @f
        mov eax, R0_WRITEFILE_IN_CONTEXT
@@:
        VxDCall IFSMgr_Ring0_FileIO
        jnc @f
        mov esi, pError              
        mov WORD PTR [esi], ax  ;give caller error code
        xor ecx, ecx            ;set byte count to zero on error
@@: 
        ; ecx contains count
        mov eax, ecx

        pop ebp
        ret 

EndCdecl     IFSMgr_Ring0_WriteFile

;  IRQHANDLE _stdcall VPICD_Virtualize_IRQ (VPICD_IRQ_DESCRIPTOR *pIrqDesc)
;
StartStdCall VPICD_Virtualize_IRQ, 4

	pop	edx			; save ret addr in unused reg
	pop	edi			; pIrqDesc
	VxDcall VPICD_Virtualize_IRQ
	jnc @f
	xor eax, eax	; carry set, error, so zero return code
@@:
	jmp edx			; jump to caller's ret addr

EndStdCall   VPICD_Virtualize_IRQ, 4

;  void _stdcall VPICD_Physically_Mask(IRQHANDLE hndIrq))
;
StartStdCall VPICD_Physically_Mask, 4

	pop	edx			; save ret addr in unused reg
	pop	eax			; hndIrq  
	VxDcall VPICD_Physically_Mask
	jmp edx			; jump to caller's ret addr

EndStdCall   VPICD_Physically_Mask, 4

;  void _stdcall VPICD_Physically_Unmask(IRQHANDLE hndIrq))
; 
StartStdCall VPICD_Physically_Unmask, 4

	pop	edx			; save ret addr in unused reg
	pop	eax			; hndIrq  
	VxDcall VPICD_Physically_Unmask
	jmp edx			; jump to caller's ret addr

EndStdCall   VPICD_Physically_Unmask, 4

;  void _stdcall VPICD_Force_Default_Behavior(IRQHANDLE hndIrq));
; y
StartStdCall VPICD_Force_Default_Behavior, 4

	pop	edx			; save ret addr in unused reg
	pop	eax			; hndIrq  
	VxDcall VPICD_Force_Default_Behavior
	jmp edx			; jump to caller's ret addr

EndStdCall   VPICD_Force_Default_Behavior, 4

;  void _stdcall VPICD_Phys_EOI(IRQHANDLE hndIrq))
;
StartStdCall VPICD_Phys_EOI, 4

	pop	edx			; save ret addr in unused reg
	pop	eax			; hndIrq  
	VxDcall VPICD_Phys_EOI
	jmp edx			; jump to caller's ret addr

EndStdCall   VPICD_Phys_EOI, 4

;  void _stdcall VPICD_Set_Int_Request(VMHANDLE hVM,, 
;                                      IRQHANDLE hndIrq
;
StartStdCall VPICD_Set_Int_Request, 8

	pop	edx			; save ret addr in unused reg
	pop   ebx         ; hVM  
	pop	eax			; hndIrq
	VxDcall VPICD_Set_Int_Request
	jmp edx			; jump to caller's ret addr

EndStdCall   VPICD_Set_Int_Request, 8

;  void _stdcall VPICD_Clear_Int_Request(VMHANDLE hVM, 
;                                      IRQHANDLE hIrq
;
StartStdCall VPICD_Clear_Int_Request, 8

	pop	edx			; save ret addr in unused reg
	pop   ebx         ; hVM  
	pop	eax			; hndIrq
	VxDcall VPICD_Clear_Int_Request
	jmp edx			; jump to caller's ret addr

EndStdCall   VPICD_Clear_Int_Request, 8


; DMAHANDLE cdecl VDMAD_Virtualize_Channel(BYTE ch  DMACALLBACK pfCallback )
StartCdecl  VDMAD_Virtualize_Channel

chan EQU [ebp+8]
pfCallback EQU [ebp+12]

        push ebp
        mov  ebp, esp

        movzx eax, BYTE PTR chan
        mov esi, pfCallback
        VxDCall VDMAD_Virtualize_Channel
        jnc @f
		xor eax, eax	; carry set, error so zero return code
@@:
        pop ebp
        ret

EndCdecl VDMAD_Virtualize_Channel

; void _stdcall VDMAD_Set_Region_Info( DMAHANDLE DMAHandle, BYTE BufferId,
;									BOOL LockStatus, 
; 									DWORD Region, DWORD RegionSize, DWORD PhysAddr
;
StartStdCall VDMAD_Set_Region_Info, 24

		pop edi		; save ret addr in unused reg
		pop	eax		; DMAHandle
		pop ebx		; BufferId
		xor bh, bh  ; BL=BufferId
		pop ecx		; LockStatus
		shl ecx, 4  ; CX=LockStatus
		xor cl, cl  ; CH=LockStatus
		or	bx, cx  ; BX=LockStatus|BufferId
		pop esi		; Region
		pop	ecx		; RegionSize
		pop	edx		; PhysAddr
        VxDCall VDMAD_Set_Region_Info
		jmp	edi		; jump to caller's ret addr
        ret

EndStdCall VDMAD_Set_Region_Info, 24

; void _stdcall VDMAD_Set_Phys_State( DMAHANDLE DMAHandle, VMHANDLE hVM,
; 									BYTE Mode, BYTE ExtMode
;
StartStdCall VDMAD_Set_Phys_State, 16 

		pop	esi		; save ret addr in unused reg
		pop	eax		; DMAHandle
		pop	ebx		; hVM
		pop	edx		; Mode
		xor	dh, dh	; DL=Mode
		pop	ecx		; ExtMode
		shl ecx, 4  ; CH=ExtMode
		xor cl, cl	; CH=ExtMode
		or	dx, cx	; DX=ExtMode|Mode
		VxDcall VDMAD_Set_Phys_State
		jmp	esi		; jump to caller's ret addr
		ret

EndStdCall    VDMAD_Set_Phys_State, 16

; void _stdcall VDMAD_Phys_Unmask_Channel( DMAHANDLE DMAHandle, VMHANDLE hVM)
;
StartStdCall  VDMAD_Phys_Unmask_Channel, 8
		
		pop	esi	; save ret addr in unused reg
		pop	edx	; DMAHandle
		pop ebx	; hVM
		VxDcall VDMAD_Phys_Unmask_Channel
		jmp esi	; jump to caller's ret addr
		ret

EndStdCall    VDMAD_Phys_Unmask_Channel, 8

; void _stdcall VDMAD_Phys_Mask_Channel( DMAHANDLE DMAHandle )
;
StartStdCall  VDMAD_Phys_Mask_Channel, 4
		
		pop	esi	; save ret addr in unused reg
		pop	eax	; DMAHandle
		VxDcall VDMAD_Phys_Mask_Channel
		jmp esi	; jump to caller's ret addr
		ret

EndStdCall    VDMAD_Phys_Mask_Channel, 4

; DWORD cdecl VDMAD_Scatter_Lock( VMHANDLE hVM, DWORD Flags,       
;                               PDDW pDDS, PDWORD pPteOffset )
StartCdecl  VDMAD_Scatter_Lock

hVM EQU [ebp+8]
Flags EQU BYTE PTR [ebp+12]
pDDS EQU [ebp+16]
pPteOffset EQU[ebp+20]

RET_NO_LOCKED EQU 0
RET_ALL_LOCKED EQU 1
RET_PART_LOCKED EQU 2
        push ebp
        mov  ebp, esp

        mov al,  BYTE PTR Flags          
        mov ebx, hVM
        mov edi, pDDS
        VxDCall VDMAD_Scatter_Lock
        jc no_lock
        jz all_lock
        mov eax, RET_PART_LOCKED
        jmp flags_checked
no_lock:
        mov eax, RET_NO_LOCKED
        jmp flags_checked
all_lock:
        mov eax, RET_ALL_LOCKED
flags_checked:
        mov ebx, pPteOffset
        mov DWORD PTR [ebx], esi        

        pop ebp
        ret

EndCdecl     VDMAD_Scatter_Lock

; VMHANDLE cdecl SHELL_Resolve_Contention( VMHANDLE hndOwner, 
;                                         VMHANDLE hndContender, 
;                                         char *DeviceName )
;
StartStdCall  SHELL_Resolve_Contention, 12
		
		pop	edx	   ; save ret addr in unused reg
		pop	eax	   ; hndOwner
      pop   ebx      ; hndContender
      pop   esi      ; DeviceName
		VxDcall SHELL_Resolve_Contention
      mov   eax, edi ; move VM handle into return 
      jnc   @f
      xor   eax, eax ; carry set, error, so zero return code
@@:
		jmp   edx	; jump to caller's ret addr
		ret

EndStdCall    SHELL_Resolve_Contention, 12


; BOOL _SHELL_PostMessage( DWORD hWnd, WORD uMsg, 
;                          WORD wParam, DWORD lParam,
;                          CALLBACK pfnCallback, DWORD dwRefData );
;

MakeCDecl   _SHELL_PostMessage

; void VWIN32_QueueUserApc( void * pfnRing3APC, 
;                          DWORD dwParam, 
;                          PTCB hThread);
;

MakeCDecl   _VWIN32_QueueUserApc

;  BOOL VWIN32_SetWin32Event(HANDLE hEvent);
;

MakeCDecl   _VWIN32_SetWin32Event

END


