/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: SaveOpen.c,v 1.16.6.1 1997/11/07 17:53:03 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"

BOOL AlterEncryptedFileName(char *FileName,DWORD Actions)
{
	char DefaultExtension[5] = ".pgp";

	if((Actions & PGPCOMDLG_DETACHEDSIG) == PGPCOMDLG_DETACHEDSIG)
	{
		strcpy(DefaultExtension, ".sig");
	}
	else
	{
		if((Actions & PGPCOMDLG_ASCIIARMOR) == PGPCOMDLG_ASCIIARMOR)
		{
			strcpy(DefaultExtension, ".asc");
		}
	}

	strcat(FileName, DefaultExtension);

	return TRUE;
}

BOOL AlterDecryptedFileName(char *FileName,char *SuggestedName)
{
	char *p;

	p = strrchr(FileName, '\\');

	if(p!=0)
		*(p+1)=0;
	else
		*FileName=0;

	strcat(FileName,SuggestedName);

	return TRUE;
}

BOOL  SaveOutputFile(PGPContextRef Context,
					 HWND hwnd, 
					 char *Title,
					 char *InputFile, 
					 PGPFileSpecRef *pOutputFileRef,
					 BOOL Force)
{
	char *p;
	BOOL UserCancel = FALSE;
	OPENFILENAME SaveFileName;
	FILE *ftest;

	char FinalFile[MAX_PATH]="\0";
	char DefaultExtension[MAX_PATH] = "\0";
	int FileStart = 0, FileExtensionStart = 0;

	strcpy(FinalFile, InputFile);

	if((p = strrchr(FinalFile, '\\')))
		FileStart = p - FinalFile + 1;

	if((p = strrchr(FinalFile, '.')))
		FileExtensionStart = p - FinalFile + 1;
	
	ftest=fopen(FinalFile,"rb");

	// If we could open the file, we need to ask the user
	if((ftest!=0)||(Force))
	{
		if(ftest!=0)
			fclose(ftest);
		
		SaveFileName.lStructSize=sizeof(SaveFileName); 
		SaveFileName.hwndOwner=hwnd; 
	    SaveFileName.hInstance=NULL; 
	    SaveFileName.lpstrFilter="All Files (*.*)\0*.*\0\0";
		SaveFileName.lpstrCustomFilter=NULL; 
	    SaveFileName.nMaxCustFilter=0; 
		SaveFileName.nFilterIndex=1; 
  	    SaveFileName.lpstrFile=FinalFile; 
	    SaveFileName.nMaxFile=MAX_PATH; 
	    SaveFileName.lpstrFileTitle=NULL; 
		SaveFileName.nMaxFileTitle=0; 
		SaveFileName.lpstrInitialDir=NULL; 
		SaveFileName.lpstrTitle=Title; 
		SaveFileName.Flags= OFN_OVERWRITEPROMPT | 
							OFN_HIDEREADONLY | 
							OFN_NOREADONLYRETURN;
#ifdef WIN32
		SaveFileName.Flags=SaveFileName.Flags | OFN_EXPLORER;
#endif
		SaveFileName.nFileOffset=FileStart; 
		SaveFileName.nFileExtension=FileExtensionStart; 
		SaveFileName.lpstrDefExt=DefaultExtension; 
		SaveFileName.lCustData=(long)NULL; 
		SaveFileName.lpfnHook=NULL;
		SaveFileName.lpTemplateName=NULL; 

		UserCancel = !GetSaveFileName(&SaveFileName);
	}
	
	if(!UserCancel)
	{
		PGPNewFileSpecFromFullPath(Context,FinalFile,pOutputFileRef);
	}

	return(UserCancel);
}

BOOL GetOriginalFileRef(HWND hwnd,PGPContextRef context,
						char *InputFile,
						PGPFileSpecRef *OriginalFileRef,
						HWND hwndWorking)
{
	char OriginalFile[MAX_PATH + 1], *p;
	char *pDefaultExtension;
	unsigned short FileStart, FileExtensionStart;
	BOOL UserCancel = FALSE;
	OPENFILENAME OriginalFileName;
	FILE *ftest;

	assert(InputFile);
	assert(OriginalFileRef);

	strcpy(OriginalFile, InputFile);

	if((p = strrchr(OriginalFile, '\\')))
		FileStart = p - OriginalFile + 1;
	else
		FileStart = 0;

	// Get rid of the .sig extension

	if((p = strrchr(OriginalFile, '.')))
	{
		*p = '\0';
	}

	ftest=fopen(OriginalFile,"rb");
	if(ftest!=0)
	{
		fclose(ftest);
		PGPNewFileSpecFromFullPath( context,OriginalFile, OriginalFileRef);
		return FALSE;
	}

	// Don't add any default extensions to the file

	pDefaultExtension=0;
	FileExtensionStart=strlen(OriginalFile);

	OriginalFileName.lStructSize=sizeof(OriginalFileName); 
    OriginalFileName.hwndOwner=hwnd; 
    OriginalFileName.hInstance=NULL; 
    OriginalFileName.lpstrFilter="All Files (*.*)\0*.*\0\0"; 
    OriginalFileName.lpstrCustomFilter=NULL; 
    OriginalFileName.nMaxCustFilter=0; 
    OriginalFileName.nFilterIndex=1; 
    OriginalFileName.lpstrFile=OriginalFile; 
    OriginalFileName.nMaxFile=MAX_PATH; 
    OriginalFileName.lpstrFileTitle=NULL; 
    OriginalFileName.nMaxFileTitle=0; 
    OriginalFileName.lpstrInitialDir=NULL; 
    OriginalFileName.lpstrTitle="Select Signed File"; 
    OriginalFileName.Flags=OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY |
						   OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
#ifdef WIN32
	OriginalFileName.Flags=OriginalFileName.Flags | OFN_EXPLORER;
#endif
    OriginalFileName.nFileOffset=FileStart; 
    OriginalFileName.nFileExtension=FileExtensionStart; 
    OriginalFileName.lpstrDefExt=pDefaultExtension; 
    OriginalFileName.lCustData=(long)NULL; 
    OriginalFileName.lpfnHook=NULL;
    OriginalFileName.lpTemplateName=NULL; 

	if(GetOpenFileName(&OriginalFileName))
	{
		PGPNewFileSpecFromFullPath( context,OriginalFile, OriginalFileRef);
	}
	else
		UserCancel = TRUE;

	return(UserCancel);
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

