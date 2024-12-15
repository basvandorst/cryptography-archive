/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: PTwipedl.c,v 1.8 1999/05/11 04:53:40 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"
#include <process.h>
#include "PGPwd.h"               //defines and structs for VxD conversations

// Global Variables
HANDLE      exist_sem;
HANDLE      stop_flag;
HANDLE      thread_handle;
HANDLE	    vxd_handle		= INVALID_HANDLE_VALUE;
DWORD       thread_id;
CRITICAL_SECTION   crit_sec; 
BOOL bWDIsNT;

struct  trap_criteria   trap_cond;

void wipe_file (struct trap_record * tr_ptr)
{
	FILELIST *fl;
	char *name;
	BOOL bSuccess;

	name=strrchr(tr_ptr->tr_file1,'\\');

	if(name==NULL)
		name=tr_ptr->tr_file1;
	else
		name++;

	// This must be one of our wiping temp files... skip.
	if(strnicmp(name,"aaaaaaaa",8)!=0)
	{
		fl=NULL;
		AddToFileList(&fl,tr_ptr->tr_file1,NULL);

		if(fl!=NULL)
		{
			bSuccess=WipeFileList(g_hwnd,PGPsc,fl,FALSE);
		}
	}
}

void release_rec(DWORD rec_address)
{
	BOOL brst;

	if(bWDIsNT)
	{
		DWORD dwLen;

	    brst=DeviceIoControl(vxd_handle, PGPWDNT_REC_DONE, 
			(void *) &rec_address,4,NULL, 0, &dwLen, NULL );
	}
	else
	{
		DeviceIoControl(vxd_handle, VXD_REC_DONE,
			(void *) &rec_address, 4, NULL, 0, NULL, NULL );
	}
}

void __stdcall our_callback(DWORD trap_rec_ptr)
{

	EnterCriticalSection(&crit_sec);
    wipe_file( (struct trap_record *)  trap_rec_ptr);
    release_rec(trap_rec_ptr);
	LeaveCriticalSection(&crit_sec);
}

void __stdcall simple_callback(DWORD Buffer)
{
	struct trap_record tr;

	tr.keFileName=(void *)Buffer;

	{
		DWORD dwLen;
		HANDLE driverHandle;

		// tell driver about the mutex (if the driver is there)
		if( (driverHandle = CreateFile( "\\\\.\\PGPWDNT",
						  GENERIC_READ | GENERIC_WRITE,
						  0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
						  NULL )) != ((HANDLE)-1) ) 
		{
			if ( ! DeviceIoControl(	driverHandle, PGPWDNT_MAP_MEM,
								&tr, sizeof(struct trap_record), 
								NULL, 0, &dwLen, NULL ) ) 
			{

				MessageBox(NULL,"Could not update Driver with synchronization information", 
					"PGPWDNT Error", MB_OK );
			}
			CloseHandle( driverHandle );
		}
	}

	MessageBox(NULL,tr.tr_file1,"HI!",MB_OK);

	{
		DWORD dwLen;
		HANDLE driverHandle;

		// tell driver about the mutex (if the driver is there)
		if( (driverHandle = CreateFile( "\\\\.\\PGPWDNT",
						  GENERIC_READ | GENERIC_WRITE,
						  0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
						  NULL )) != ((HANDLE)-1) ) 
		{
			if ( ! DeviceIoControl(	driverHandle, PGPWDNT_REC_DONE,
								&tr, sizeof(struct trap_record), 
								NULL, 0, &dwLen, NULL ) ) 
			{

				MessageBox(NULL,"Could not update Driver with synchronization information", 
					"PGPWDNT Error", MB_OK );
			}
			CloseHandle( driverHandle );
		}
	}

}

DWORD  __stdcall do_getcalls()
{
	BOOL brst;

    //Set up the callback with the VxD (note: VxD will get the
    //ID for this thread and use it and the callback address to
    //inform us about file deletes).

	if(bWDIsNT)
	{
		DWORD dwLen;
		HANDLE driverHandle;

		// tell driver about the mutex (if the driver is there)
		if( (driverHandle = CreateFile( "\\\\.\\PGPWDNT",
						  GENERIC_READ | GENERIC_WRITE,
						  0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
						  NULL )) != ((HANDLE)-1) ) 
		{
			if ( ! DeviceIoControl(	driverHandle, PGPWDNT_REGISTER_CB,
								&trap_cond, sizeof(struct trap_criteria), 
								NULL, 0, &dwLen, NULL ) ) 
			{

				MessageBox(NULL,"Could not update Driver with synchronization information", 
					"PGPWDNT Error", MB_OK );
			}
			CloseHandle( driverHandle );
		}
	}
	else
	{
		brst=DeviceIoControl(vxd_handle, VXD_REGISTER_CB, &trap_cond,
                  (sizeof(struct trap_criteria)),NULL, 0, NULL, NULL );

		if ( ! brst ) 
		{
			MessageBox(NULL,"Could not update Driver", 
					"PGPWDNT Error", MB_OK|MB_ICONERROR );
		}
	}

    while( WaitForSingleObjectEx(stop_flag, INFINITE, TRUE) == WAIT_IO_COMPLETION)
    ;

    return 0;
}

// Wipe on delete to be added for 7.0
// Until then this code is commented out wjb
BOOL InitializeWipeOnDelete(HWND hwnd)
{
	/*
	OSVERSIONINFO osid;

	osid.dwOSVersionInfoSize = sizeof (osid);
	GetVersionEx (&osid);   
	bWDIsNT=(osid.dwPlatformId == VER_PLATFORM_WIN32_NT);

	//First, check to see if we're already loaded (we don't need more
	//than one program snooping the file system.)

	exist_sem=CreateSemaphore(NULL, 0, 1, "WipeOnDeleteSemaphore");

	if (exist_sem != NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(exist_sem);
		MessageBox(hwnd, "WipeOnDelete is already loaded.", "PGP Error",
			MB_ICONINFORMATION|MB_SETFOREGROUND);
		return FALSE;
	}

	// open the handle to the VXD
	if(bWDIsNT)
	{
//		vxd_handle = CreateFile( "\\\\.\\PGPWDNT",
//						  GENERIC_READ | GENERIC_WRITE,
//						  0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
//						  NULL );
	}
	else
	{
		vxd_handle = CreateFile( VXD_NAME, 0, 0, NULL,
			0, FILE_FLAG_DELETE_ON_CLOSE, NULL );

		if ( vxd_handle == INVALID_HANDLE_VALUE )
		{
			MessageBox(hwnd,"Can't access PGPwd9x.vxd. Files will\n"
						"not be wiped on delete.", "PGP Error",
				MB_ICONINFORMATION|MB_SETFOREGROUND);
				return FALSE;
		}
	}

	if(bWDIsNT)
	{
		trap_cond.tc_callback=(DWORD) &simple_callback;
	}
	else
	{
		trap_cond.tc_callback=(DWORD) &our_callback;
	}

	InitializeCriticalSection(&crit_sec);
	stop_flag = CreateEvent(NULL, FALSE, FALSE, NULL); 

	ResetEvent(stop_flag);	

	if (!(thread_handle=(HANDLE) _beginthreadex(NULL, 4096,
		(unsigned int  (__stdcall *) (void *) ) do_getcalls,
		NULL, 0,(unsigned int *) &thread_id)))
	{
		MessageBox(hwnd, "Unable to create thread", "PGP Error",
			MB_ICONINFORMATION|MB_SETFOREGROUND);
		return FALSE;
	}
	*/
	return TRUE;
}

void UninitializeWipeOnDelete()
{
	/*
	if(bWDIsNT)
	{
		DWORD dwLen;
		HANDLE driverHandle;

		trap_cond.tc_callback=0;

		// tell driver about the mutex (if the driver is there)
		if( (driverHandle = CreateFile( "\\\\.\\PGPWDNT",
						  GENERIC_READ | GENERIC_WRITE,
						  0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
						  NULL )) != ((HANDLE)-1) ) 
		{
			if ( ! DeviceIoControl(	driverHandle, PGPWDNT_REGISTER_CB,
								&trap_cond, sizeof(struct trap_criteria), 
								NULL, 0, &dwLen, NULL ) ) 
			{

				MessageBox(NULL, "Could not update Driver with synchronization information", 
					"PGPWDNT Error", MB_ICONERROR );
			}
			CloseHandle( driverHandle );
		}
	}

    //Now, tell VxD not to call us back anymore, because we're 
    //leaving.  This is necessary because the VxD
    //will not be unloaded if another utility is using it.

 	if ( vxd_handle != INVALID_HANDLE_VALUE )
    {
        CloseHandle(vxd_handle);

        SetEvent(stop_flag);            //tell logging thread to stop
    }

    if (thread_handle)
    {
        CloseHandle(thread_handle);
        CloseHandle(exist_sem);
	    DeleteCriticalSection(&crit_sec);
    }
	*/
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
