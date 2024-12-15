/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: wipe.c,v 1.27.2.1 1997/11/20 21:17:49 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

BOOL WipeError(HWND hwnd,char *FileName,UINT error)
{
	char Msg[256];
	int nMsgBoxResult;
	BOOL bCancel = FALSE;

	switch(error)
	{
		case WIPE_NOTOPEN:
			strcpy(Msg,"File was not wiped because it could not be opened\n"
				       "for write access. Please check file permissions,\n"
					   "and make sure that the file isn't opened by\n"
					   "another program.");
			break;
		case WIPE_USERABORT:
			strcpy(Msg,"File was not wiped completely because of a user\n"
				       "abort");
			bCancel = TRUE;
			break;
		case WIPE_NOSTATS:
			strcpy(Msg,"File was not wiped because _stat information about\n"
				       "it could not be obtained.");
			break;

		case WIPE_NOREMOVE:
			strcpy(Msg,"File was overwritten with random data, but could not\n"
				       "be deleted.");

		case WIPE_NOFLUSH:
			strcpy(Msg,"Stream was not flushed. File data may not have been\n"
				       "properly overwritten.");
			break;

		case WIPE_NOFILEMAP:
			strcpy(Msg,"File mapping could not be created.");
			break;

		case WIPE_NOMEMMAP:
			strcpy(Msg,"File could not be mapped to memory.");
			break;

		case WIPE_ALREADYOPEN:
			strcpy(Msg,"File was not wiped because it has been opened\n"
				       "by another program. Close any programs which might\n"
					   "have this file open before attempting to wipe it.");
			break;

		case WIPE_WRITEPROTECT:
			strcpy(Msg,"The media on which the file is located is\n"
					   "write-proctected. The file cannot be wiped\n"
					   "until the write protection is removed.");
			break;
	}

	nMsgBoxResult = MessageBox(hwnd,Msg,FileName,
						MB_OKCANCEL|MB_ICONSTOP|MB_SETFOREGROUND);

	if (nMsgBoxResult == IDCANCEL)
		bCancel = TRUE;

	return bCancel;
}

#if 0
UINT InternalFileWipe(HWND hwnd,char *FileName)
{
   struct _stat buf;
   int result;
   FILE *fwipe;
   long i;
   char randomjunk;
   WORKSTRUCT *ws;
   BOOL CancelOperation;
 //  clock_t ticks;

   if(hwnd!=NULL)
	   ws=(WORKSTRUCT *)GetWindowLong(hwnd, DWL_USER);

   /* Get data associated with "stat.c": */
   result = _stat(FileName, &buf );

   /* Check if statistics are valid: */
   if( result != 0 )
      return WIPE_NOSTATS;

   srand( (unsigned)time( NULL ) );

   fwipe=fopen(FileName,"r+b");

   if(fwipe==0)
	   return WIPE_NOTOPEN;

   if(hwnd!=NULL)
   {
	   	sprintf(ws->TitleText,"Wiping %s",JustFile(FileName));

		SendMessage(hwnd, PGPM_PROGRESS_UPDATE,
			(WPARAM)PGPSC_WORK_NEWFILE,0);

		ProcessWorkingDlg(hwnd);
   }

 //  ticks=clock();

   for(i=0;i<buf.st_size;i++)
   {
	  randomjunk=(char)rand();
	  fputc((int)randomjunk,fwipe);

	  if((hwnd!=NULL)&&((i%1024)==0))
	  {
		  CancelOperation=WorkingCallback (hwnd, 
				(unsigned long)i, (unsigned long)buf.st_size) ;

		  	if(CancelOperation)
			{
				fclose(fwipe);
				return WIPE_USERABORT;
			}
//			ticks=clock();
	  }
   }

	if(hwnd!=NULL)
	{
		CancelOperation=WorkingCallback (hwnd, 
				(unsigned long)buf.st_size, (unsigned long)buf.st_size);
	}

	result=fflush(fwipe);

	if(result!=0)
	{
		fclose(fwipe);
		return WIPE_NOFLUSH;
	}

   fclose(fwipe);

   result=remove(FileName);

   if(result!=0)
	   return WIPE_NOREMOVE;

   return WIPE_OK;
}
#endif	/* 0 */

BOOL FileWipe(char *FileName)
{
	return InternalFileWipe(NULL,FileName);
}

BOOL ReadWipePref(PGPBoolean *pbWarnOnWipe)
{
	PGPPrefRef	PrefRefClient=NULL;
	PGPError	err;

	*pbWarnOnWipe=TRUE;

	err=PGPcomdlgOpenClientPrefs (&PrefRefClient);
	if(IsntPGPError(err))
	{
		PGPGetPrefBoolean (PrefRefClient,
			kPGPPrefWarnOnWipe, pbWarnOnWipe);
		PGPcomdlgCloseClientPrefs (PrefRefClient, FALSE);

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
/*				if(flist->IsDirectory)
				{
					flist=flist->next;
					continue;
				}*/

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

BOOL FreewareWipeCheck(HWND hwnd)
{
/*	// We don't support Freeware file wiping
#if PGP_FREEWARE
	char sz[256];
	LoadString (g_hinst, IDS_WIPINGINFREEWARE, sz, sizeof(sz));
	PGPcomdlgHardSell (hwnd, sz);
	
	return FALSE;
#endif
*/
	return TRUE;
}

UINT WipeFileList(HWND hwnd,FILELIST *ListHead)
{
	BOOL ReturnCode;
	FILELIST *FileCurrent;
	int NumDone;
	WORKSTRUCT *ws;
	HWND hwndWorking;
	PGPBoolean bWarnOnWipe;
	BOOL DeleteThem;
	UINT WipeReturn;

	ReadWipePref(&bWarnOnWipe);

	if(bWarnOnWipe)
	{
		DeleteThem=DialogBoxParam(g_hinst, 
			MAKEINTRESOURCE( IDD_DELETEDLG),
            hwnd, 
            (DLGPROC) DeleteDlgProc, 
            (LPARAM) ListHead);

		if(!DeleteThem)
			return FALSE;
	}

	ws=(WORKSTRUCT *)malloc(sizeof(WORKSTRUCT));
	memset(ws, 0x00, sizeof(WORKSTRUCT) );

	strcpy(ws->StartText,"");
	strcpy(ws->EndText,"");

	hwndWorking=CreateDialogParam(g_hinst, 
		MAKEINTRESOURCE(IDD_WORKING), 
		hwnd,
		(DLGPROC)WorkingDlgProc,
		(LPARAM)ws);

	ProcessWorkingDlg(hwndWorking);

	ReturnCode=TRUE;
	FileCurrent=ListHead;
	NumDone=0;

	while((FileCurrent!=0)&&(ReturnCode==TRUE))
	{    
		if(FileCurrent->IsDirectory)
		{
			int result;

			result=_rmdir(FileCurrent->name);

			if(result!=0)
			{
				MessageBox(hwndWorking,FileCurrent->name,
					"Directory could not be removed. The directory path may\n"
					"be invalid, or the directory may not be empty.",
					MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
				ReturnCode=FALSE;
			}

			FileCurrent=FileCurrent->next;
			continue;
		}

		WipeReturn=InternalFileWipe(hwndWorking,FileCurrent->name);

		if(WipeReturn!=WIPE_OK)
		{
			if (WipeError(hwndWorking,FileCurrent->name,WipeReturn))
				ReturnCode=FALSE;
		}

		NumDone++;			
		FileCurrent=FileCurrent->next;				
	}

	DestroyWindow(hwndWorking);
	ProcessWorkingDlg(hwndWorking);
	
	free(ws);
	FreeFileList(ListHead);

	return ReturnCode;
}


/* Formerly ElrodsFileWipe */

UINT InternalFileWipe(HWND hwnd,char *FileName)
{

	DWORD dwSize;
	char NewFileName[200];
	char szDrive[_MAX_DRIVE];
	char szNewFile[200 - _MAX_DRIVE];
	HANDLE hFile;
	HANDLE hMap;
	LPSTR lpFile;
	int count = 0;
    WORKSTRUCT *ws;
    BOOL CancelOperation;
	BOOL bMoveFileSuccess;
    struct _stat buf;

	_stat(FileName, &buf );

    if(hwnd!=NULL)
		ws=(WORKSTRUCT *)GetWindowLong(hwnd, DWL_USER);
	
	if(hwnd!=NULL)
	{
		sprintf(ws->TitleText,"Wiping %s",JustFile(FileName));
		
		SendMessage(hwnd, PGPM_PROGRESS_UPDATE,
			(WPARAM)PGPSC_WORK_NEWFILE,0);
		
		ProcessWorkingDlg(hwnd);
	}
	
	_splitpath(FileName, szDrive, NULL, NULL, NULL);

	do{
		memset(szNewFile, 'a' + count, sizeof(szNewFile)-1);
		szNewFile[sizeof(szNewFile)-1] = 0;
		strcpy(NewFileName, szDrive);
		strcat(NewFileName, "\\");
		strcat(NewFileName, szNewFile);
		count++;
		bMoveFileSuccess = MoveFile(FileName, NewFileName);
		if (!bMoveFileSuccess)
		{
			DWORD dwLastError;

			dwLastError = GetLastError();

			switch (dwLastError)
			{
			case ERROR_SHARING_VIOLATION:
				return WIPE_ALREADYOPEN;

			case ERROR_ACCESS_DENIED:
				return WIPE_NOTOPEN;

			case ERROR_WRITE_PROTECT:
				return WIPE_WRITEPROTECT;

			case ERROR_REQUEST_ABORTED:
				return WIPE_USERABORT;
			}
		}
	}while(!bMoveFileSuccess);

	if(hwnd!=NULL)
	{
		CancelOperation=WorkingCallback(hwnd,
			(int)(1.0*0.16*(float)buf.st_size),buf.st_size) ;
		
		if(CancelOperation)
		{
			return WIPE_USERABORT;
		}
	}

	hFile = CreateFile(	NewFileName, GENERIC_READ | GENERIC_WRITE,
						0, NULL, OPEN_EXISTING, 
						FILE_ATTRIBUTE_NORMAL, NULL );

	if( hFile == INVALID_HANDLE_VALUE )  //  If unable to open, skip it
	{
		dwSize = GetLastError();       //  These calls are for debugging
		return WIPE_NOTOPEN;
	}

	if(hwnd!=NULL)
	{
		CancelOperation=WorkingCallback (hwnd,
			(int)(2.0*0.16*(float)buf.st_size),buf.st_size) ;
		
		if(CancelOperation)
		{
			return WIPE_USERABORT;
		}
	}

	dwSize = GetFileSize( hFile, NULL );        //  Get the file's size
	if( ! dwSize )
	{
		CloseHandle( hFile );
		DeleteFile( NewFileName );		//  If nothing to erase, just kill it
		return WIPE_OK;
	}

	if(hwnd!=NULL)
	{
		CancelOperation=WorkingCallback (hwnd,
			(int)(3.0*0.16*(float)buf.st_size),buf.st_size) ;
		
		if(CancelOperation)
		{
			CloseHandle( hFile );
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
		return WIPE_NOFILEMAP;
	}

	if(hwnd!=NULL)
	{
		CancelOperation=WorkingCallback (hwnd,
			(int)(4.0*0.16*(float)buf.st_size),buf.st_size) ;
		
		if(CancelOperation)
		{
			CloseHandle( hFile );
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
			return WIPE_USERABORT;
		}
	}

	// we are not using Colin's patterns yet

	/*for( count = 0; count < passes; count++)
	{
		char pattern[3] = {0x00, 0x00, 0x00};
		DWORD index = 0;

		//GetPattern(pattern, count);

		while((index + 3) <= dwSize)
		{
			//  Set the whole thing to pattern
			memcpy( lpFile + index, pattern, 3 );	
			index+=3;
		}

		// size of file not aligned to 3
		if(index < dwSize)
		{
			memcpy( lpFile + index, pattern, dwSize - index);	
		}

		FlushViewOfFile( lpFile, dwSize );    //  Flush it to disk
	}*/

	memset( lpFile, 0x00, dwSize );

	FlushViewOfFile( lpFile, dwSize );  //  Flush it to disk

	UnmapViewOfFile( lpFile );			//  Kill mapped area

	CloseHandle( hMap );                // Close all the handles and...
	CloseHandle( hFile );
	DeleteFile( NewFileName );          //  ...(finally) delete the file

	if(hwnd!=NULL)
	{
		CancelOperation=WorkingCallback (hwnd,
			buf.st_size,buf.st_size) ;
	}

	return WIPE_OK;
}

BOOL ClipboardWipe(HWND hwnd)
{
	int result;
	PGPBoolean bWarnOnWipe;

	ReadWipePref(&bWarnOnWipe);

	if(bWarnOnWipe)
	{
		if(MessageBox(hwnd,
		"Are you really sure you want to erase the clipboard?",
		"Last Chance", 
		MB_YESNO|MB_DEFBUTTON2|MB_ICONEXCLAMATION|MB_SETFOREGROUND)==IDNO)
		return FALSE;
	}

	OpenClipboard(NULL);

	result=EmptyClipboard();

	CloseClipboard();

	return result;
}


/*void GetPattern(char* PatternBuffer, int PatternIndex)
{
	static int Seeded = 0;

	// temporary random stuff for now
	// need Cryptographically strong 
	// RNG that is fast...
	if(!Seeded)
	{
		srand( (unsigned)time( NULL ) );
		Seeded = 1;
	}

	if( (PatternIndex <= 5 && PatternIndex > 0) ||
		(PatternIndex > 31) )
	{
		PatternBuffer[0] = rand();
		PatternBuffer[1] = rand();
		PatternBuffer[2] = rand();
	}
	else
	{
		memcpy(PatternBuffer, PatternArray[PatternIndex], 3);
	}
}*/


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
