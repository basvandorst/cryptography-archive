/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: wipe.c,v 1.47.8.1 1998/11/12 03:13:39 heller Exp $
____________________________________________________________________________*/
#include "precomp.h"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

BOOL WipeError(HWND hwnd,FILELIST *FileEntry,UINT error)
{
	int ErrMsg;
	char StrRes[500];
	int nMsgBoxResult;
	BOOL bCancel = FALSE;
	UINT MBoxFlag;

	MBoxFlag=MB_OKCANCEL;

	switch(error)
	{
		case WIPE_NOTOPEN:
			ErrMsg=IDS_WIPENOTOPEN;
			break;
		case WIPE_USERABORT:
			ErrMsg=IDS_WIPEUSERABORT;
			bCancel = TRUE;
			break;
		case WIPE_NOSTATS:
			ErrMsg=IDS_WIPENOSTATS;
			break;

		case WIPE_NOREMOVE:
			ErrMsg=IDS_WIPENOREMOVE;
			break;
	
		case WIPE_NOFLUSH:
			ErrMsg=IDS_WIPENOFLUSH;
			break;

		case WIPE_NOFILEMAP:
			ErrMsg=IDS_WIPENOFILEMAP;
			break;

		case WIPE_NOMEMMAP:
			ErrMsg=IDS_WIPENOMEMMAP;
			break;

		case WIPE_ALREADYOPEN:
			ErrMsg=IDS_WIPEALREADYOPEN;
			break;

		case WIPE_WRITEPROTECT:
			ErrMsg=IDS_WIPEPROTECT;
			break;
	}

	LoadString (g_hinst, ErrMsg, StrRes, sizeof(StrRes));

	// There is no next file, so we don't put up both...
	if(FileEntry->next==NULL)
		MBoxFlag=MB_OK;

	nMsgBoxResult = MessageBox(hwnd,StrRes,JustFile(FileEntry->name),
						MBoxFlag|MB_ICONSTOP|MB_SETFOREGROUND);

	if (nMsgBoxResult == IDCANCEL)
		bCancel = TRUE;

	return bCancel;
}


BOOL ReadWipePref(PGPContextRef context,
				  PGPBoolean *pbWarnOnWipe)
{
	PGPPrefRef	PrefRefClient=NULL;
	PGPError	err;
	PGPMemoryMgrRef memMgr;

	memMgr=PGPGetContextMemoryMgr(context);

	*pbWarnOnWipe=TRUE;
	
	err=PGPclOpenClientPrefs (memMgr,&PrefRefClient);

	if(IsntPGPError(err))
	{
		PGPGetPrefBoolean (PrefRefClient,
			kPGPPrefWarnOnWipe, pbWarnOnWipe);

		PGPclCloseClientPrefs (PrefRefClient, FALSE);

		return TRUE;
	}
	return FALSE;
}

#define MINDLGX 400
#define MINDLGY 200
#define DLGMARGIN 5

int ResizeEm(HWND hdlg)
{
    RECT dlgRect;
	int buttx;
	HWND hwndText,hwndList,hwndYes,hwndNo;

    GetClientRect(hdlg, &dlgRect);

    dlgRect.top+=DLGMARGIN;
    dlgRect.bottom-=DLGMARGIN;
    dlgRect.left+=DLGMARGIN;
    dlgRect.right-=DLGMARGIN;
	
	hwndText=GetDlgItem(hdlg,IDC_WIPEQUERY);

	InvalidateRect(hwndText,NULL,TRUE);

	MoveWindow(hwndText,
	    dlgRect.left,dlgRect.top,
		dlgRect.right-dlgRect.left,30,TRUE);

	hwndList=GetDlgItem(hdlg,IDC_FILELIST);

	InvalidateRect(hwndList,NULL,TRUE);

	MoveWindow(hwndList,
		dlgRect.left,40+dlgRect.top,
		dlgRect.right-dlgRect.left,dlgRect.bottom-dlgRect.top-80,TRUE);

	buttx=(dlgRect.right-dlgRect.left)/2-80+dlgRect.left;

	hwndYes=GetDlgItem(hdlg,IDYES);

	InvalidateRect(hwndYes,NULL,TRUE);

	MoveWindow(hwndYes,
	    buttx,dlgRect.bottom-25,
		75,25,TRUE);

	hwndNo=GetDlgItem(hdlg,IDNO);

	InvalidateRect(hwndNo,NULL,TRUE);

	MoveWindow(hwndNo,
	    buttx+85,dlgRect.bottom-25,
		75,25,TRUE);

	return TRUE;
}

BOOL WINAPI DeleteDlgProc(HWND hdlg, UINT uMsg, 
                             WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
		{
			FILELIST *flist;

			flist=(FILELIST *)lParam;

			while(flist!=0)
			{
				SendMessage(GetDlgItem(hdlg,IDC_FILELIST),
					LB_ADDSTRING,0,(LPARAM)flist->name);
				flist=flist->next;
			}
	
			ResizeEm(hdlg);

			ShowWindow(hdlg,SW_SHOW);
			SetForegroundWindow(hdlg);

			return TRUE;
		}

		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* lpmmi;

		    lpmmi = (MINMAXINFO*) lParam;
    		lpmmi->ptMinTrackSize.x = MINDLGX;
    		lpmmi->ptMinTrackSize.y = MINDLGY;
            break;
		}

        case WM_SIZE:
        {
            ResizeEm(hdlg);
            break;
        }

		case WM_CLOSE:
		case WM_QUIT:
		case WM_DESTROY:
		{
			EndDialog(hdlg, FALSE);
			break;
		}

        case WM_COMMAND:
        {
            switch(wParam)
            {
				case IDYES:
				{
					EndDialog(hdlg, TRUE);
					break;
				}

				case IDNO:
				{
					EndDialog(hdlg, FALSE);
					break;
				}
			}
			break;
		}
	}
	return FALSE;
}

PGPError WipeFileListStub (MYSTATE *ms) 
{
	PGPContextRef context;
	FILELIST *FileCurrent;
	UINT WipeReturn;
	char StrRes[500];
	PGPError err;

	err=kPGPError_NoErr;

	context=ms->context;

	FileCurrent=ms->ListHead;

	while(!(ms->ws.CancelPending)&&(FileCurrent!=0)&&(IsntPGPError(err)))
	{    
		if(FileCurrent->IsDirectory)
		{
			int result;

			result=_rmdir(FileCurrent->name);

			if(result!=0)
			{
				LoadString (g_hinst, IDS_DIRNOTREMOVED, StrRes, sizeof(StrRes));

				MessageBox(ms->hwndWorking,FileCurrent->name,StrRes,
					MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
				err=kPGPError_UnknownError;
			}

			FileCurrent=FileCurrent->next;
			continue;
		}

		WipeReturn=InternalFileWipe(ms->context,
			ms->hwndWorking,FileCurrent->name);

		if(WipeReturn!=WIPE_OK)
		{
			if (WipeError(ms->hwndWorking,FileCurrent,WipeReturn))
				err=kPGPError_UnknownError;
		}

		FileCurrent=FileCurrent->next;				
	}

	return err;
}


BOOL WipeFileList(HWND hwnd,void *PGPsc,FILELIST *ListHead)
{
	MYSTATE *ms;
	PGPBoolean bWarnOnWipe;
	BOOL DeleteThem;
	PGPContextRef context;
	PGPError err;

	err=kPGPError_NoErr;

	context=(PGPContextRef)PGPsc;

	ReadWipePref(context,&bWarnOnWipe);

	if(bWarnOnWipe)
	{
		DeleteThem=DialogBoxParam(g_hinst, 
			MAKEINTRESOURCE( IDD_DELETEDLG),
            hwnd, 
            (DLGPROC) DeleteDlgProc, 
            (LPARAM) ListHead);

		if(!DeleteThem)
		{
			return FALSE;
		}
	}

	ms=(MYSTATE *)malloc(sizeof(MYSTATE));

	if(ms)
	{
		memset(ms, 0x00, sizeof(MYSTATE) );

		LoadString (g_hinst, IDS_STARTINGWORK, ms->ws.StartText, sizeof(ms->ws.StartText));
		LoadString (g_hinst, IDS_FINISHINGUP, ms->ws.EndText, sizeof(ms->ws.EndText));

		ms->context=context;
		ms->ListHead=ListHead;
		ms->Operation=MS_WIPEFILELIST;
	
		err=DoWork(hwnd,ms);

		free(ms);
	}
	
	FreeFileList(ListHead);

	if(IsPGPError(err))
		return FALSE;

	return TRUE;
}

UINT InternalFileWipe(PGPContextRef context,HWND hwnd,char *FileName)
{

	DWORD dwSize;
	DWORD dwPageSize;
	DWORD dwNumPages;
	SYSTEM_INFO sysinfo;
	char NewFileName[200+_MAX_DIR];
	char szDrive[_MAX_DRIVE];
	char szDir[ _MAX_DIR];
	char szNewFile[200-_MAX_DRIVE];
	HANDLE hFile;
	HANDLE hMap;
	LPSTR lpFile;
	int count = 0;
    BOOL CancelOperation;
	BOOL bMoveFileSuccess;
    struct _stat buf;
	UINT wipe_err = 0;
	MYSTATE *ms;
	int staterror;
	char StrRes[500];

	PGPError		error			= kPGPError_NoErr;
	PGPDiskWipeRef	wipeRef			= kPGPInvalidRef;
	PGPInt32		buffer[256];
	PGPInt32		passes			= kPGPNumPatterns;
	PGPPrefRef		prefRef			= kPGPInvalidRef;
	PGPMemoryMgrRef memMgr			= kPGPInvalidRef;

	if( IsntPGPError(error) )
	{
		memMgr = PGPGetContextMemoryMgr(context);
	
		error = PGPclOpenClientPrefs (memMgr, &prefRef);

		if(IsntPGPError(error))
		{
			PGPGetPrefNumber (	prefRef,
								kPGPPrefFileWipePasses, 
								(PGPUInt32*)&passes);

			PGPclCloseClientPrefs (prefRef, FALSE);

		}
	}

	GetSystemInfo(&sysinfo);

	dwPageSize = sysinfo.dwPageSize;

	staterror=_stat(FileName, &buf );

	if(staterror!=0)
		return WIPE_NOTOPEN; // check to make sure still around

    if(hwnd!=NULL)
		ms=(MYSTATE *)GetWindowLong(hwnd, GWL_USERDATA);
	
	if(hwnd!=NULL)
	{
		LoadString (g_hinst, IDS_WIPINGFILE, StrRes, sizeof(StrRes));

		sprintf(ms->ws.TitleText,StrRes,JustFile(FileName));
		
		SendMessage(hwnd, PGPM_PROGRESS_UPDATE,
			(WPARAM)PGPSC_WORK_NEWFILE,0);
	}
	
	_splitpath(FileName, szDrive, szDir, NULL, NULL);

	memset(szNewFile, 'a', sizeof(szNewFile)-1);
	szNewFile[sizeof(szNewFile)-1] = 0;
	strcpy(NewFileName, szDrive);
	strcat(NewFileName, szDir); // do in same directory
	strcat(NewFileName, szNewFile);

	bMoveFileSuccess = MoveFile(FileName, NewFileName);
	if (!bMoveFileSuccess)
	{
		DWORD dwLastError;

		dwLastError = GetLastError();

		switch (dwLastError)
		{
			case ERROR_SHARING_VIOLATION:
				return WIPE_ALREADYOPEN;

			case ERROR_WRITE_PROTECT:
				return WIPE_WRITEPROTECT;

			case ERROR_REQUEST_ABORTED:
				return WIPE_USERABORT;

			case ERROR_ACCESS_DENIED:
			default:
				return WIPE_NOTOPEN;
		}
	}

	if(hwnd!=NULL)
	{
		CancelOperation=WorkingCallback(hwnd,
			(int)(1.0*0.16*(float)buf.st_size),buf.st_size) ;
		
		if(CancelOperation)
		{
			MoveFile(NewFileName,FileName);
			return WIPE_USERABORT;
		}
	}

	hFile = CreateFile(	NewFileName, 
						GENERIC_READ | GENERIC_WRITE,
						0, 
						NULL,
						OPEN_EXISTING, 
						FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH, 
						NULL );

	if( hFile == INVALID_HANDLE_VALUE )  //  If unable to open, skip it
	{
		dwSize = GetLastError();       //  These calls are for debugging
		MoveFile(NewFileName,FileName);
		return WIPE_NOTOPEN;
	}

	if(hwnd!=NULL)
	{
		CancelOperation=WorkingCallback (hwnd,
			(int)(2.0*0.16*(float)buf.st_size),buf.st_size) ;
		
		if(CancelOperation)
		{
			CloseHandle( hFile );
			MoveFile(NewFileName,FileName);
			return WIPE_USERABORT;
		}
	}

	dwSize = GetFileSize( hFile, NULL );        //  Get the file's size

	if( dwSize==0 ) 
	{
		CloseHandle( hFile );
		DeleteFile( NewFileName );		//  If nothing to erase, just kill it

		return WIPE_OK;
	}

	/* we want to wipe out to next page boundary */

	// Get up to one more page to make fit
	dwNumPages = (dwSize+dwPageSize-1)/dwPageSize;
			
	dwSize = dwNumPages * dwPageSize;

	if(hwnd!=NULL)
	{
		CancelOperation=WorkingCallback (hwnd,
			(int)(3.0*0.16*(float)buf.st_size),buf.st_size) ;
		
		if(CancelOperation)
		{
			CloseHandle( hFile );
			MoveFile(NewFileName,FileName);
			return WIPE_USERABORT;
		}
	}

	//  Create a file mapping object
	hMap = CreateFileMapping( hFile, NULL, PAGE_READWRITE, 
						  0, 0, NULL );
	if( ! hMap )
	{
		dwSize = GetLastError();        //  If it didn't work, next file
		CloseHandle( hFile );
		MoveFile(NewFileName,FileName);
		return WIPE_NOFILEMAP;
	}

	if(hwnd!=NULL)
	{
		CancelOperation=WorkingCallback (hwnd,
			(int)(4.0*0.16*(float)buf.st_size),buf.st_size) ;
		
		if(CancelOperation)
		{
			CloseHandle( hFile );
			MoveFile(NewFileName,FileName);
			return WIPE_USERABORT;
		}
	}

	//  Map the file into a memory space...
	lpFile = MapViewOfFile( hMap, FILE_MAP_WRITE, 0, 0, 0 );

	if( lpFile == NULL )
	{
		dwSize = GetLastError();              //  On error, back way out
		CloseHandle( hFile );
		CloseHandle( hMap );
		MoveFile(NewFileName,FileName);
		return WIPE_NOMEMMAP;
	}

	if(hwnd!=NULL)
	{
		CancelOperation=WorkingCallback (hwnd,
			(int)(5.0*0.16*(float)buf.st_size),buf.st_size) ;
		
		if(CancelOperation)
		{
			CloseHandle( hFile );
			CloseHandle( hMap );
			MoveFile(NewFileName,FileName);
			return WIPE_USERABORT;
		}
	}

	// we are now using Colin's patterns

	error = PGPCreateDiskWiper(	context, &wipeRef, passes);
	
	if( IsntPGPError(error) )
	{
		while( IsntPGPError( PGPGetDiskWipeBuffer(wipeRef, buffer) ) )
		{	
			DWORD count;

			for( count = 0; count < dwSize; count += 1024)
			{
				memcpy(lpFile + count, buffer, 1024);
			}

			FlushViewOfFile( lpFile, dwSize );  //  Flush it to disk*/
			FlushFileBuffers( hFile );
		}

		error = PGPDestroyDiskWiper(wipeRef);
	}

	UnmapViewOfFile( lpFile );			//  Kill mapped area

	CloseHandle( hMap );                // Close all the handles and...
	FlushFileBuffers( hFile );
	CloseHandle( hFile );
	DeleteFile( NewFileName );          //  ...(finally) delete the file

	if(hwnd!=NULL)
	{
		CancelOperation=WorkingCallback (hwnd,
			buf.st_size,buf.st_size) ;
	}

	return WIPE_OK;
}

BOOL ClipboardWipe(HWND hwnd,void *PGPsc)
{
	int result;
	PGPBoolean bWarnOnWipe;
	PGPContextRef context;
	int rst;

	context=(PGPContextRef)PGPsc;

	if(hwnd!=NULL)
	{
		ReadWipePref(context,&bWarnOnWipe);

		if(bWarnOnWipe)
		{
			rst=PGPscMessageBox (hwnd,IDS_LASTCHANCE,IDS_REALLYWIPECLIPBOARD,
					MB_YESNO|MB_DEFBUTTON2|MB_ICONEXCLAMATION);	

			if(rst==IDNO)
				return FALSE;
		}
	}

	OpenClipboard(NULL);

	result=EmptyClipboard();

	CloseClipboard();

	return result;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
