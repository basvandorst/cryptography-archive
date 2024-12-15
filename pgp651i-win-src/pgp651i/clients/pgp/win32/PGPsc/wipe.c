/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: wipe.c,v 1.59 1999/03/23 17:34:00 wjb Exp $
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

BOOL WINAPI DeleteDlgProc(HWND hdlg, UINT uMsg, 
                             WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
		{
			FILELIST *flist;
			int maxwidth,namelen;
			SIZE sizeString;
			HDC hdc;
			HWND hwndLB;
			int retval;
			HBITMAP hBmp;
			DWORD bits,bmp;
			HFONT hFont,hOldFont;

			flist=(FILELIST *)lParam;
			maxwidth=0;

			hwndLB=GetDlgItem(hdlg,IDC_FILELIST);

			hFont = (HFONT) GetStockObject(ANSI_VAR_FONT);
			SendMessage(hwndLB, WM_SETFONT, (WPARAM) hFont, 
				MAKELPARAM(TRUE, 0));

			hdc=GetDC(hwndLB);

			hOldFont=SelectObject(hdc,hFont);

			// Determine which bitmap will be displayed 
			bits = GetDeviceCaps (hdc, BITSPIXEL) * GetDeviceCaps (hdc, PLANES);

			if (bits <= 1)
				bmp = IDB_WIPEWIZ1;
			else if (bits <= 8) 
				bmp = IDB_WIPEWIZ4;
			else 
				bmp = IDB_WIPEWIZ8; 

			hBmp = LoadBitmap(g_hinst, MAKEINTRESOURCE(bmp));

			SendDlgItemMessage(	hdlg, 
				IDC_SECUREPIC, 
				STM_SETIMAGE, 
				IMAGE_BITMAP, 
				(LPARAM) hBmp);

			while(flist!=0)
			{
				SendMessage(hwndLB,
					LB_ADDSTRING,0,(LPARAM)flist->name);

				namelen=strlen(flist->name);
				retval=GetTextExtentPoint32(hdc, flist->name, namelen, &sizeString);
				
				sizeString.cx+=GetSystemMetrics(SM_CXVSCROLL);

				if(sizeString.cx>maxwidth)
					maxwidth=sizeString.cx;

				flist=flist->next;
			}

			SelectObject(hdc,hOldFont);

			ReleaseDC(hwndLB,hdc);

			SendMessage(hwndLB,
				LB_SETHORIZONTALEXTENT,(WPARAM)maxwidth,0);

			ShowWindow(hdlg,SW_SHOW);
			SetForegroundWindow(hdlg);

			return TRUE;
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

	while(!(SCGetProgressCancel(ms->hPrgDlg))&&(FileCurrent!=0)&&(IsntPGPError(err)))
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
			ms->hwndWorking,FileCurrent->name,ms->bDelFilePostWipe);

		if(WipeReturn!=WIPE_OK)
		{
			if (WipeError(ms->hwndWorking,FileCurrent,WipeReturn))
				err=kPGPError_UnknownError;
		}

		FileCurrent=FileCurrent->next;				
	}

	return err;
}


BOOL WipeFileList(HWND hwnd,void *PGPsc,FILELIST *ListHead,BOOL bDelFilePostWipe)
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

		ms->context=context;
		ms->ListHead=ListHead;
		ms->Operation=MS_WIPEFILELIST;
		ms->bDelFilePostWipe=bDelFilePostWipe;
	
		err=SCProgressDialog(hwnd,DoWorkThread,ms,
			  0,"Wiping File(s)...",
			  "","",IDR_ERASEAVI);

		free(ms);
	}
	
	FreeFileList(ListHead);

	if(IsPGPError(err))
		return FALSE;

	return TRUE;
}

UINT InternalFileWipe(PGPContextRef context,HWND hwnd,
					  char *FileName,BOOL bDelFilePostWipe)
{

	DWORD dwSize;
	DWORD dwPageSize;
	DWORD dwNumPages;
	SYSTEM_INFO sysinfo;
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
	char NewFileName[MAX_PATH+1];
	int retval;
	DWORD dwFileAttributes;
	char *name;
	HPRGDLG hPrgDlg;

	PGPError		error			= kPGPError_NoErr;
	PGPDiskWipeRef	wipeRef			= kPGPInvalidRef;
	PGPInt32		buffer[256];
	PGPInt32		passes			= kPGPNumPatterns;
	PGPPrefRef		prefRef			= kPGPInvalidRef;
	PGPMemoryMgrRef memMgr			= kPGPInvalidRef;

	CancelOperation=FALSE;

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
	{
		hPrgDlg=(HPRGDLG)GetWindowLong(hwnd, GWL_USERDATA);
		ms=(MYSTATE *)SCGetProgressUserValue(hPrgDlg);
	}
	
	if(hwnd!=NULL)
	{
		SCSetProgressNewFilename(ms->hPrgDlg,"From '%s'",FileName,TRUE);
	}
	
	// Movefile can't handle file names more than MAX_PATH
	if(strlen(FileName)>=MAX_PATH-11)
		return WIPE_NOTOPEN;

	strcpy(NewFileName,FileName);

	name=strrchr(NewFileName,'\\');

	if(name==NULL)
		name=NewFileName;
	else
		name++;

	// Set the filename to a's as much as MAX_PATH
	// will allow to write over old filename entries
	memset(name,'a',&(NewFileName[MAX_PATH])-name);
	NewFileName[MAX_PATH-10]=0;

	// Try to make writable if readonly
	dwFileAttributes=GetFileAttributes(FileName);
	dwFileAttributes&=~FILE_ATTRIBUTE_READONLY;
	retval=SetFileAttributes(FileName,dwFileAttributes);

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

	dwSize = GetFileSize( hFile, NULL );        //  Get the file's size

	if( dwSize==0 ) 
	{
		CloseHandle( hFile );
		if(bDelFilePostWipe)
			DeleteFile( NewFileName );		//  If nothing to erase, just kill it

		return WIPE_OK;
	}

	/* we want to wipe out to next page boundary */

	// Get up to one more page to make fit
	dwNumPages = (dwSize+dwPageSize-1)/dwPageSize;
			
	dwSize = dwNumPages * dwPageSize;

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

	// This is their last chance before it goes FUBAR!!!!!
	if(hwnd!=NULL)
	{
		if(SCGetProgressCancel(hPrgDlg))
			CancelOperation=TRUE;
		
		if(CancelOperation)
		{
			CloseHandle( hFile );
			CloseHandle( hMap );
			// Back from deaths door
			MoveFile(NewFileName,FileName);
			return WIPE_USERABORT;
		}
	}

	// we are now using Colin's patterns

	error = PGPCreateDiskWiper(	context, &wipeRef, passes);
	
	if( IsntPGPError(error) )
	{
		DWORD reps;

		reps=0;

		while( IsntPGPError( PGPGetDiskWipeBuffer(wipeRef, buffer) ) )
		{	
			DWORD count;

			for( count = 0; count < dwSize; count += 1024)
			{
				memcpy(lpFile + count, buffer, 1024);
			}

			FlushViewOfFile( lpFile, dwSize );  //  Flush it to disk*/
			FlushFileBuffers( hFile );

			reps++;

			if(IsPGPError(SCSetProgressBar(hPrgDlg,
				reps*100/passes,FALSE)))
			{
				CancelOperation=TRUE;
				break;
			}
		}

		error = PGPDestroyDiskWiper(wipeRef);
	}

	UnmapViewOfFile( lpFile );			//  Kill mapped area

	CloseHandle( hMap );                // Close all the handles and...
	FlushFileBuffers( hFile );
	CloseHandle( hFile );

	if(bDelFilePostWipe)
		DeleteFile( NewFileName );          //  ...(finally) delete the file
	else
		MoveFile(NewFileName,FileName);

	if(CancelOperation)
		return WIPE_USERABORT;

	if(hwnd!=NULL)
	{
		SCSetProgressBar(hPrgDlg,100,TRUE);
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
