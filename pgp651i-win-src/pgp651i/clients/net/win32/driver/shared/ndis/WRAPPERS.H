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

#ifndef _VMMXWRAP_H
#define _VMMXWRAP_H

#include <vxdwraps.h>

/****************************************************************************
*                                                                           *
*   VMM services                                                            *
*                                                                           *
****************************************************************************/
typedef DWORD VMHANDLE;
typedef DWORD EVENTHANDLE;
typedef DWORD MEMHANDLE;
typedef DWORD PTHCB;  // pointer to thread control block
typedef void *CALLBACK(void);

MAKE_HEADER(PTHCB,_stdcall,Get_Initial_Thread_Handle, (HVM hVM))
MAKE_HEADER(BOOL, _stdcall,Install_IO_Handler, (DWORD PortNum, \
                                                CALLBACK Callback ))
MAKE_HEADER(BOOL, _stdcall,Remove_IO_Handler, (DWORD PortNum))
MAKE_HEADER(void, _stdcall,Enable_Local_Trapping, (VMHANDLE hVM, \
                                                   DWORD PortNum))
MAKE_HEADER(void, _stdcall,Disable_Local_Trapping, (VMHANDLE hVM, \
                                                   DWORD PortNum))
MAKE_HEADER(EVENTHANDLE,cdecl,Call_Priority_VM_Event, (DWORD PriorityBoost, \
                                       VMHANDLE hVM, \
                                       DWORD Flags, \
                                       void *Refdata, \
                                       CALLBACK EventCallback, \
                                       DWORD Timeout )) 
MAKE_HEADER(void, cdecl, _Deallocate_Device_CB_Area, (DWORD Offset, \
                                                      DWORD Flags ))
MAKE_HEADER(void, cdecl, Save_Client_State, (CLIENT_STRUCT * pSavedRegs))
MAKE_HEADER(void, cdecl, Restore_Client_State, (CLIENT_STRUCT * pSavedRegs))
MAKE_HEADER(void, cdecl, Begin_Nest_Exec, (void))
MAKE_HEADER(void, cdecl, End_Nest_Exec, (void))
MAKE_HEADER(void, _stdcall, Simulate_Far_Call, (WORD seg, WORD off ))
MAKE_HEADER(void, _stdcall, Simulate_Push, (DWORD val))
MAKE_HEADER(BOOL,cdecl,_ModifyPageBits, (VMHANDLE hVM, \
                                          DWORD VMLinPgNum, \
                                          DWORD nPages, \
                                          DWORD bitAnd, \
                                          DWORD bitOR, \
                                          DWORD pType, \
                                          DWORD Flags))
MAKE_HEADER(BOOL,_stdcall,Hook_V86_Page, (DWORD PageNum, \
                                          CALLBACK Callback))
MAKE_HEADER(BOOL,_stdcall,Unhook_V86_Page, (DWORD PageNum, \
                                          CALLBACK Callback))
MAKE_HEADER(BOOL,cdecl,_Assign_Device_V86_Pages, (DWORD VMLinrPage, \
                                                   DWORD nPages, \
                                                   VMHANDLE hVM, \
                                                   DWORD Flags))
MAKE_HEADER(BOOL,cdecl,_DeAssign_Device_V86_Pages, (DWORD VMLinrPage, \
                                                   DWORD nPages, \
                                                   VMHANDLE hVM, \
                                                   DWORD Flags))
MAKE_HEADER(BOOL,cdecl,_PhysIntoV86, (DWORD PhysPage, \
                                       VMHANDLE hVM, \
                                       DWORD VMLinPgNum, \
                                       DWORD nPages, \
                                       DWORD Flags))
MAKE_HEADER(BOOL,cdecl,_MapIntoV86, (MEMHANDLE hMem, \
                                    VMHANDLE hVM, \
                                    DWORD VMLinPageNumber, \
                                    DWORD nPages, \
                                    DWORD PageOff, \
                                    DWORD Flags))
MAKE_HEADER(MEMHANDLE,cdecl,_GetNulPageHandle,(void))
MAKE_HEADER(ULONG,cdecl,_PageReserve, (ULONG page, \
                                       ULONG npages, \
                                       ULONG flags))
MAKE_HEADER(ULONG,cdecl,_PageCommitPhys, (ULONG page, \
                                          ULONG npages, \
                                          ULONG physpg, \
                                          ULONG flags))
MAKE_HEADER(ULONG,cdecl,_PageDecommit, (ULONG page, \
                                       ULONG npages, \
                                       ULONG flags))
MAKE_HEADER(void *,_stdcall,Map_Flat, (BYTE SegOffset, BYTE OffOffset ))

#define MAPFLAT(sgmnt,offst) Map_Flat(((DWORD)(&((CRS *)0)->sgmnt)),(DWORD)(&((struct Client_Word_Reg_Struc *)0)->offst))
#define Map_Flat                    PREPEND(Map_Flat)
#define Call_Priority_VM_Event      PREPEND(Call_Priority_VM_Event)
#define Get_Initial_Thread_Handle   PREPEND(Get_Initial_Thread_Handle)
#define Install_IO_Handler          PREPEND(Install_IO_Handler)
#define Remove_IO_Handler           PREPEND(Remove_IO_Handler)
#define Enable_Local_Trapping       PREPEND(Enable_Local_Trapping)
#define Disable_Local_Trapping      PREPEND(Disable_Local_Trapping)
#define _Deallocate_Device_CB_Area  PREPEND(_Deallocate_Device_CB_Area)   
#define Save_Client_State           PREPEND(Save_Client_State)
#define Restore_Client_State        PREPEND(Restore_Client_State)
#define Begin_Nest_Exec             PREPEND(Begin_Nest_Exec)
#define End_Nest_Exec               PREPEND(End_Nest_Exec)
#define Simulate_Far_Call           PREPEND(Simulate_Far_Call)
#define Simulate_Push               PREPEND(Simulate_Push)
#define _ModifyPageBits             PREPEND(_ModifyPageBits)
#define Hook_V86_Page               PREPEND(Hook_V86_Page)
#define Unhook_V86_Page             PREPEND(Unhook_V86_Page)
#define _Assign_Device_V86_Pages    PREPEND(_Assign_Device_V86_Pages)
#define _DeAssign_Device_V86_Pages  PREPEND(_DeAssign_Device_V86_Pages)
#define _PhysIntoV86                PREPEND(_PhysIntoV86)
#define _MapIntoV86                 PREPEND(_MapIntoV86)
#define _GetNulPageHandle           PREPEND(_GetNulPageHandle)
#define _PageReserve                PREPEND(_PageReserve)
#define _PageCommitPhys             PREPEND(_PageCommitPhys)
#define _PageDecommit               PREPEND(_PageDecommit)

// the following functions are really in VXDWRAPS.CLB, but aren't 
// prototyped in VXDWRAPS.H
MAKE_HEADER(DWORD, cdecl, _Allocate_Device_CB_Area, (DWORD NumBytes, \
                                                      DWORD Flags ))
#define _Allocate_Device_CB_Area     PREPEND(_Allocate_Device_CB_Area)
/****************************************************************************
*                                                                           *
*   IFSMgr services                                                         *
*                                                                           *
****************************************************************************/

#define R0_OPENCREATFILE        0xD500  /* Open/Create a file */
#define R0_OPENCREAT_IN_CONTEXT 0xD501  /* Open/Create file in current context */
#define R0_CLOSEFILE            0xD700  /* Close file */                   
#define R0_WRITEFILE            0xD601  /* WRite to a file */
#define R0_WRITEFILE_IN_CONTEXT 0xD603  /* Write to a file in current context */

MAKE_HEADER(HANDLE,cdecl,IFSMgr_Ring0_OpenCreateFile, (BOOL bInContext, \
                                                PCHAR filename, \
                                                WORD mode, \
                                                WORD attrib, \
                                                BYTE action, \
                                                BYTE flags, \
                                                WORD *pError, \
                                                BYTE *pAction))
MAKE_HEADER(DWORD,cdecl,IFSMgr_Ring0_WriteFile, (BOOL bInContext, \
                                        HANDLE filehandle, \
                                        PVOID buf, \
                                        DWORD count, \
                                        DWORD filepos, \
                                        WORD  *perr))
MAKE_HEADER(BOOL,cdecl,IFSMgr_Ring0_CloseFile, (HANDLE filehandle, \
                                      WORD   *pError))
    
#define IFSMgr_Ring0_OpenCreateFile PREPEND(IFSMgr_Ring0_OpenCreateFile)
#define IFSMgr_Ring0_WriteFile PREPEND(IFSMgr_Ring0_WriteFile)
#define IFSMgr_Ring0_CloseFile PREPEND(IFSMgr_Ring0_CloseFile)

/****************************************************************************
*                                                                           *
*   VPICD services                                                          *
*                                                                           *
****************************************************************************/

typedef struct 
{
    WORD VID_IRQ_Number;    // IRQ to virtualize
    WORD VID_Options;   
    // VPICD_OPT_CAN_SHARE: allow other VxDs to virtualize IRQ also
    // VPICD_OPT_REF_DATA: pass VID_Hw_Int_Ref as param to Hw_Int_Handler
    DWORD VID_Hw_Int_Proc;  // callback for hardware interrupt
    DWORD VID_Virt_Int_Proc;  
    DWORD VID_EOI_Proc;         
    DWORD VID_Mask_Change_Proc;     
    DWORD VID_IRET_Proc;        
    DWORD VID_IRET_Time_Out;    
    PVOID VID_Hw_Int_Ref;   // pass this data to Hw_Int_Handler     
} VPICD_IRQ_DESCRIPTOR;

typedef DWORD IRQHANDLE;

MAKE_HEADER(IRQHANDLE,_stdcall,VPICD_Virtualize_IRQ, (VPICD_IRQ_DESCRIPTOR *pIrqDesc))
MAKE_HEADER(void,_stdcall,VPICD_Physically_Mask, (IRQHANDLE hndIrq))
MAKE_HEADER(void,_stdcall,VPICD_Physically_Unmask, (IRQHANDLE hndIrq))
MAKE_HEADER(void,_stdcall,VPICD_Force_Default_Behavior, (IRQHANDLE hndIrq))
MAKE_HEADER(void,_stdcall,VPICD_Phys_EOI, (IRQHANDLE hndIrq))
MAKE_HEADER(void,_stdcall,VPICD_Set_Int_Request, (IRQHANDLE hIRQ, \
                                                   VMHANDLE hVM))
MAKE_HEADER(void,_stdcall,VPICD_Clear_Int_Request, (IRQHANDLE hIRQ, \
                                                   VMHANDLE hVM))


#define VPICD_Virtualize_IRQ    PREPEND(VPICD_Virtualize_IRQ)
#define VPICD_Physically_Mask   PREPEND(VPICD_Physically_Mask)
#define VPICD_Physically_Unmask PREPEND(VPICD_Physically_Unmask)
#define VPICD_Force_Default_Behavior    PREPEND(VPICD_Force_Default_Behavior)
#define VPICD_Phys_EOI          PREPEND(VPICD_Phys_EOI)
#define VPICD_Set_Int_Request   PREPEND(VPICD_Set_Int_Request)
#define VPICD_Clear_Int_Request PREPEND(VPICD_Clear_Int_Request)


/****************************************************************************
*                                                                           *
*   VDMAD services                                                          *
*                                                                           *
****************************************************************************/

#define DMA_type_verify 0x00
#define DMA_type_write  0x04
#define DMA_type_read   0x08
#define DMA_AutoInit    0x10
#define DMA_AdrDec  0x20

#define DMA_demand_mode 0x00
#define DMA_single_mode 0x40
#define DMA_block_mode  0x80
#define DMA_cascade 0xc0

#define DMA_mode_mask   0xc0   // mask to isolate controller mode bits (above)

#define DMA_chan_sel    0x03

#define NONE_LOCKED 0
#define ALL_LOCKED  1
#define SOME_LOCKED 2

typedef struct
{
    DWORD   PhysAddr;
    DWORD   Size;
} REGION;

typedef struct Extended_DDS_Struc 
{
   DWORD    DDS_size;
   DWORD    DDS_linear;
   WORD DDS_seg;
   WORD RESERVED;
   WORD DDS_avail;
   WORD DDS_used;
} EXTENDED_DDS, *PEXTENDED_DDS;

typedef struct
{
    EXTENDED_DDS    dds;
    union
    {
        REGION          aRegionInfo[16];
        DWORD           aPte[16];
    };
} DDS, *PDDS;

typedef DWORD DMAHANDLE;

MAKE_HEADER(DWORD,cdecl,VDMAD_Scatter_Lock, ( VMHANDLE hVM, \
                                                DWORD Flags, \
                                                PDDS pDDS, \
                                                PDWORD pPteOffset )) 
MAKE_HEADER(DMAHANDLE, cdecl, VDMAD_Virtualize_Channel, (BYTE ch, CALLBACK pfCallback ))
MAKE_HEADER(void, _stdcall, VDMAD_Set_Region_Info, (DMAHANDLE DMAHandle, \
                                                BYTE BufferId, \
                                                BOOL LockStatus,
                                                DWORD Region, \
                                                DWORD RegionSize, \
                                                DWORD PhysAddr ))
MAKE_HEADER(void, _stdcall, VDMAD_Set_Phys_State, (DMAHANDLE DMAHandle, \
                                                VMHANDLE hVM, \
                                                WORD Mode, \
                                                WORD ExtMode ))
MAKE_HEADER(void, _stdcall, VDMAD_Phys_Unmask_Channel, (DMAHANDLE DMAHandle, \
                                                      VMHANDLE hVM ))   
MAKE_HEADER(void, _stdcall, VDMAD_Phys_Mask_Channel, (DMAHANDLE DMAHandle))   
#define VDMAD_Virtualize_Channel PREPEND(VDMAD_Virtualize_Channel)
#define VDMAD_Set_Region_Info PREPEND(VDMAD_Set_Region_Info)
#define VDMAD_Set_Phys_State PREPEND(VDMAD_Set_Phys_State)
#define VDMAD_Scatter_Lock  PREPEND(VDMAD_Scatter_Lock)
#define VDMAD_Phys_Unmask_Channel PREPEND(VDMAD_Phys_Unmask_Channel)
#define VDMAD_Phys_Mask_Channel PREPEND(VDMAD_Phys_Unmask_Channel)

/****************************************************************************
*                                                                           *
*   SHELL services                                                          *
*                                                                           *
****************************************************************************/

MAKE_HEADER(VMHANDLE, _stdcall, SHELL_Resolve_Contention, (VMHANDLE hndOwner, \
                                                           VMHANDLE hndContender, \
                                                            char *DeviceName ))
MAKE_HEADER(BOOL,cdecl,_SHELL_PostMessage, (HANDLE hWnd, \
                                          DWORD uMsg, \
                                          WORD wParam, \
                                          DWORD lParam, \
                                          CALLBACK pCallback, \
                                          void *dwRefData))

#define SHELL_Resolve_Contention PREPEND(SHELL_Resolve_Contention)
#define _SHELL_PostMessage       PREPEND(_SHELL_PostMessage)

/****************************************************************************
*                                                                           *
*   VWIN32 services                                                         *
*                                                                           *
****************************************************************************/

MAKE_HEADER(void,cdecl,_VWIN32_QueueUserApc, (void *pR3Proc, \
                                                DWORD Param, \
                                                PTHCB hThread))
MAKE_HEADER(BOOL,cdecl,_VWIN32_SetWin32Event, (EVENTHANDLE hEvent) )

#define _VWIN32_QueueUserApc     PREPEND(_VWIN32_QueueUserApc)
#define _VWIN32_SetWin32Event    PREPEND(_VWIN32_SetWin32Event)

#endif // _VMMXWRAP_H
