/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.
	
	pgpwdel.c - main C routines for PGPwdel VxD (Win95/98)
	

	$Id: PGPwd9x.c,v 1.2 1999/05/11 02:41:55 wjb Exp $
____________________________________________________________________________*/

#define WANTVXDWRAPS
#include <basedef.h>
#include <vmm.h>
#include <vwin32.h>
#include <ifs.h>
#include <debug.h>
#include <winerror.h>
#include <regdef.h>
#include <vxdwraps.h>
#include "PGPwd.h"

// Wrappers for some VXD calls (which are in the asm file)
MAKE_HEADER(ppIFSFileHookFunc,_cdecl,IFSMgr_InstallFileSystemApiHook, (pIFSFileHookFunc HookFunc))
MAKE_HEADER(int,_cdecl,IFSMgr_RemoveFileSystemApiHook, (pIFSFileHookFunc HookFunc))
MAKE_HEADER(_QWORD,_cdecl,UniToBCSPath, (unsigned char* pBCSPath, PathElement* pUniPath, unsigned int maxlen, int charSet))
MAKE_HEADER(BOOL,_cdecl,_VWIN32_QueueUserApc, (DWORD userproc,DWORD refdata,PTCB thread))
MAKE_HEADER(VOID,_cdecl,IFSMgr_Block, (unsigned long key))
MAKE_HEADER(VOID,_cdecl,IFSMgr_Wakeup, (unsigned long key))

#define IFSMgr_InstallFileSystemApiHook     PREPEND(IFSMgr_InstallFileSystemApiHook)
#define IFSMgr_RemoveFileSystemApiHook      PREPEND(IFSMgr_RemoveFileSystemApiHook)
#define	UniToBCSPath					 	PREPEND(UniToBCSPath)
#define	_VWIN32_QueueUserApc				PREPEND(_VWIN32_QueueUserApc)
#define IFSMgr_Block						PREPEND(IFSMgr_Block)
#define IFSMgr_Wakeup						PREPEND(IFSMgr_Wakeup)

#define PGP_WIN32	1

// global data 
static ppIFSFileHookFunc ppPrevHook=0;   // the previous hooker
static VMMLIST list_handle=0;
static PTCB tc_thread=0;
static DWORD tc_callback=0;
static DWORD log_flag=0;

void __stdcall FillName(unsigned char *pszName, 
					   int drv, int res, int cp, pioreq pir ) 
{
    if ( (DWORD)(pir->ir_ppath) != 0 &&
         (DWORD)(pir->ir_ppath) != 0xfffffbbb ) 
	{
        int istart=0;
     
        if ( pszName != NULL ) 
		{
			if ( ((res & IFSFH_RES_CFSD)==0) && 
				drv != 0 && drv != 0xff ) 
			{
				pszName[0] = (char)('@' + drv);
				pszName[1] = (char)':';
				istart = 2;
			}

			UniToBCSPath( pszName+istart, 
				pir->ir_ppath->pp_elements, MAX_PATH, cp );
        }
    }
}

/* The FileSystem Hook Function */
int __cdecl FileHook( pIFSFunc pfn, int fn, int drv, 
					 int res, int cp, pioreq pir ) 
{
	switch( fn ) 
	{
		case IFSFN_DELETE :
		{
			if(tc_callback!=0)
			{
				VMMLISTNODE rec_buffer;
				struct trap_record *tr;

				rec_buffer=List_Allocate(list_handle);
				tr=(struct trap_record *)rec_buffer;

				FillName((unsigned char *)tr->tr_file1,drv,res,cp,pir);

				_VWIN32_QueueUserApc(tc_callback,(DWORD)rec_buffer,tc_thread);

				IFSMgr_Block((unsigned long)&(log_flag));
			}
			break;
		}
	}

	return (*(*ppPrevHook))( pfn, fn, drv, res, cp, pir );
}


//----------------------------------------------------|
//  called upon initialization of VxD

BOOL 
OnSysDynamicDeviceInit () 
{
	list_handle=List_Create(LF_ASYNC | LF_ALLOC_ERROR,sizeof(struct trap_record));

    ppPrevHook = IFSMgr_InstallFileSystemApiHook( FileHook );
    if ( ppPrevHook == NULL ) 
		return VXD_FAILURE;

	return VXD_SUCCESS;    	// success
}


//----------------------------------------------------|
//  called upon shutdown of VxD

BOOL 
OnSysDynamicDeviceExit () 
{
	IFSMgr_RemoveFileSystemApiHook( FileHook );
	tc_callback=0;
	tc_thread=0;

    return VXD_SUCCESS;    // success
}   


//----------------------------------------------------|
//  called upon as result of call to DeviceIoControl

DWORD 
OnDeviceIoControl (PDIOCPARAMETERS p) 
{
	DWORD	dwReturn;

	dwReturn=0;

    switch (p->dwIoControlCode) 
	{
		case DIOC_GETVERSION :
			dwReturn = 0;	// no error
			break;

		case VXD_REGISTER_CB :
		{
			struct  trap_criteria *trap_cond;

			// get current thread
			tc_thread=Get_Cur_Thread_Handle();

			// Get the callback
			trap_cond=(struct trap_criteria *)p->lpvInBuffer;
			tc_callback=trap_cond->tc_callback;
			break;
		}

		case VXD_REC_DONE :
		{
			VMMLISTNODE *rec_buffer;

			rec_buffer=(VMMLISTNODE *)p->lpvInBuffer;

			IFSMgr_Wakeup((unsigned long)&(log_flag));

			List_Deallocate(list_handle,*rec_buffer);
			break;
		}

		default :
			dwReturn = ERROR_BAD_COMMAND;
    }

	return dwReturn;
}

