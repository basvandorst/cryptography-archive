/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: openfile.c,v 1.7.4.1 1997/11/07 17:53:04 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"

#define OFNBUFFSIZE 25000

BOOL 
#ifndef WIN32
FAR __export
#endif // WIN32
PASCAL FileDlgHookProc(HWND hDlg, unsigned message,
                           WORD wParam, LONG lParam)
{ 
    static BOOL *ClipBoard;
    
    switch (message)
    {
        case WM_INITDIALOG:
                 ClipBoard=(BOOL *)(((OPENFILENAME *)lParam)->lCustData);
                 *ClipBoard=FALSE;
             return TRUE;
     
        case WM_COMMAND:
             switch(wParam)
             {
                 case IDC_CLIPBOARD:
                     *ClipBoard=TRUE;
#ifdef WIN32
					 EndDialog(GetParent(hDlg),TRUE);
#else
            		 EndDialog(hDlg, TRUE);
#endif
             }
             break;
                           
    }
      
    return FALSE;
}

#ifdef WIN32 // Translate Multi for Win95 and WinNT

FILELIST *TranslateMulti(char *FileString)
{
	char *FileName;
	char *FilePath;
	FILELIST *ListHead,*Current;
	int FirstFile;
	char name[MAX_PATH];

	FirstFile=0;

	while(FileString[FirstFile]!=0)
	{
		FirstFile++;
	}

	FirstFile++;

	ListHead=Current=0;
	FilePath=FileString;
	FileName=&FileString[FirstFile];

	while(TRUE)
	{
		strcpy(name,FilePath);

		if((*FileName)!=0)
		{
			strcat(name,"\\");
			strcat(name,FileName);
		}

		AddToFileList(&ListHead,name);

		while((*FileName)!=0)
		{
			FileName++;
		}

		FileName++;

		if((*FileName)==0)
			break;
	}

	return ListHead;
}

#else // Translate Multi For Win3.1

FILELIST *TranslateMulti(char *FileString)
{
	char *FileName;
	char *FilePath;  
	char *FileStart;
	FILELIST *ListHead;
	int FirstFile;
    BOOL OneMore;
	char name[MAX_PATH];
    
	FirstFile=0;

	while((FileString[FirstFile]!=' ')&&(FileString[FirstFile]!=0))
	{ 
		FirstFile++;
	}
    
    FileString[FirstFile]=0;
    
	FirstFile++;

	ListHead=0;
	FilePath=FileString;
	FileName=&FileString[FirstFile];
    
    OneMore=TRUE;
    
	while(OneMore)
	{
	FileStart=FileName;
	
	while(((*FileName)!=' ')&&((*FileName)!=0))
	{
		FileName++;
	}
 
 	if((*FileName)==0)
		OneMore=FALSE;
	
	*FileName=0;
	                                
//    MessageBox(NULL,FilePath,FileStart,MB_OK);
    
	strcpy(name,FilePath);
	if((*FileStart)!=0)
	{
		strcat(name,"\\");
		strcat(name,FileStart);
    }
    
    AddToFileList(&ListHead,name);
   
	FileName++;
	}

	return ListHead;
}

#endif // WIN32

BOOL GetFileName(HWND hwnd,
				 char *lpszMessage,
				 FILELIST **FileList,
				 BOOL Clipboard,
				 char *FilterString)
{
	char *OriginalFile;
	BOOL ReturnValue;
    OPENFILENAME OriginalFileName;    
    BOOL ClipBoard;
	DWORD OFN_Error,Flags;

	// Get 40K for file selector
	OriginalFile=(char *)malloc(OFNBUFFSIZE); 
    
	memset(OriginalFile,0x00,OFNBUFFSIZE);

	OriginalFileName.lStructSize=sizeof(OPENFILENAME);
    OriginalFileName.hwndOwner=hwnd;
    OriginalFileName.hInstance=g_hinst;
    OriginalFileName.lpstrFilter=FilterString;
    OriginalFileName.lpstrCustomFilter=NULL;
    OriginalFileName.nMaxCustFilter=0;
    OriginalFileName.nFilterIndex=1;
    OriginalFileName.lpstrFile=OriginalFile;
    OriginalFileName.nMaxFile=OFNBUFFSIZE;
    OriginalFileName.lpstrFileTitle=NULL;
    OriginalFileName.nMaxFileTitle=0;
    OriginalFileName.lpstrInitialDir=NULL;
    OriginalFileName.lpstrTitle=lpszMessage;
	Flags=OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY |
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | 
		OFN_ALLOWMULTISELECT;
	if(Clipboard)
		Flags|=OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;
    OriginalFileName.Flags=Flags;
#ifdef WIN32
    OriginalFileName.Flags=OriginalFileName.Flags | OFN_EXPLORER;
#endif
    OriginalFileName.nFileOffset=0;
    OriginalFileName.nFileExtension=strlen(OriginalFile);
    OriginalFileName.lpstrDefExt=0;
    OriginalFileName.lCustData=(LPARAM)(&ClipBoard);
	if(Clipboard)
	{
#ifdef WIN32
		OriginalFileName.lpfnHook=
			(LPOFNHOOKPROC)FileDlgHookProc;
		OriginalFileName.lpTemplateName=
			MAKEINTRESOURCE(IDD_CUST32DLG);
#else
		OriginalFileName.lpfnHook=
			MakeProcInstance(FileDlgHookProc,g_hinst);
		OriginalFileName.lpTemplateName=
			MAKEINTRESOURCE(IDD_CUST16DLG);
#endif
	}
	else
	{
		OriginalFileName.lpfnHook=NULL;
		OriginalFileName.lpTemplateName=NULL;
	}

	if(GetOpenFileName(&OriginalFileName))
	{
		ReturnValue = TRUE;

		if(ClipBoard==TRUE)
			*FileList=0;
		else
		{
			*FileList=TranslateMulti(OriginalFileName.lpstrFile);
		}
	}
	else
	{
		OFN_Error=CommDlgExtendedError();
		ReturnValue = FALSE;
	}
    
#ifndef _WIN32
    FreeProcInstance(OriginalFileName.lpfnHook);
#endif // !_WIN32

	memset(OriginalFile,0x00,OFNBUFFSIZE);
	free(OriginalFile);
    
	return(ReturnValue);
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

