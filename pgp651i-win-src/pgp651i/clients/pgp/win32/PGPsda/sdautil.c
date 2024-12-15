/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPsda.c - Code for decoding SDA's
	

	$Id: sdautil.c,v 1.4 1999/02/04 05:40:20 wjb Exp $
____________________________________________________________________________*/

#include "DecodeStub.h"

void GetPrefixPath(GETPUTINFO *gpi)
{
	char *p;

	GetModuleFileName(NULL,gpi->szPrefixPath, MAX_PATH);

	p=strrchr(gpi->szPrefixPath,'\\');

	if(p!=NULL)
	{
		p++;
		*p=0;
	}
	else
	{
		strcpy(gpi->szPrefixPath,"");
	}
}

BOOL  SaveOutputFile(HWND hwnd, 
					 char *Title,
					 char *InputFile, 
					 char *OutputFile)
{
	char *p;
	BOOL UserCancel = FALSE;
	OPENFILENAME SaveFileName;
	FILE *ftest;
	char StrRes[500];

	char DefaultExtension[MAX_PATH] = "\0";
	int FileStart = 0, FileExtensionStart = 0;

	strcpy(OutputFile, InputFile);

	if((p = strrchr(OutputFile, '\\')))
		FileStart = p - OutputFile + 1;

	if((p = strrchr(OutputFile, '.')))
		FileExtensionStart = p - OutputFile + 1;
	
	ftest=fopen(OutputFile,"rb");

	// If we could open the file, we need to ask the user
	if(ftest!=0)
	{
		fclose(ftest);
		
		LoadString (g_hinst, IDS_SAVEFILTER, StrRes, sizeof(StrRes));
		while (p = strrchr (StrRes, '@')) *p = '\0';

		memset(&SaveFileName,0x00,sizeof(OPENFILENAME));

		SaveFileName.lStructSize=sizeof(SaveFileName); 
		SaveFileName.hwndOwner=hwnd; 
	    SaveFileName.hInstance=NULL; 
	    SaveFileName.lpstrFilter=StrRes;
		SaveFileName.lpstrCustomFilter=NULL; 
	    SaveFileName.nMaxCustFilter=0; 
		SaveFileName.nFilterIndex=1; 
  	    SaveFileName.lpstrFile=OutputFile; 
	    SaveFileName.nMaxFile=MAX_PATH; 
	    SaveFileName.lpstrFileTitle=NULL; 
		SaveFileName.nMaxFileTitle=0; 
		SaveFileName.lpstrInitialDir=NULL; 
		SaveFileName.lpstrTitle=Title; 
		SaveFileName.Flags= OFN_OVERWRITEPROMPT | 
							OFN_HIDEREADONLY | 
							OFN_NOREADONLYRETURN |
							OFN_EXPLORER;
		SaveFileName.nFileOffset=FileStart; 
		SaveFileName.nFileExtension=FileExtensionStart; 
		SaveFileName.lpstrDefExt=DefaultExtension; 
		SaveFileName.lCustData=(long)NULL; 
		SaveFileName.lpfnHook=NULL;
		SaveFileName.lpTemplateName=NULL; 

		UserCancel = !GetSaveFileName(&SaveFileName);
	}
	
	return(UserCancel);
}

PGPError SDAGetPassphrase(SDAWORK *SDAWork,SDAHEADER *SDAHeader,PGPUInt32 **ExpandedKey)
{
	BOOL PassphraseOK;
	char szPrompt[100];

	PassphraseOK=FALSE;
	LoadString (g_hinst, IDS_PASSPROMPT, szPrompt, sizeof(szPrompt));

	// GET PASSPHRASE
	while(!PassphraseOK)
	{
		PGPError err;
		SHAContext sha;
		byte const *HashedPassphrase;
		char PassCheckBytes[8];
		PGPUInt8		j;
		PGPUInt16		i, k;
		char *pszPassword;

		err=SDAPassphraseDialog(SDAWork->hwndWorking,szPrompt,&pszPassword);

		if(err!=kPGPError_NoErr)
			return kPGPError_UserAbort;
		
		// Hash passphrase w/ SHA
		memset(&sha,0x00,sizeof(SHAContext));

		(HashSHA.init)(&sha);
		(HashSHA.update)(&sha,
			SDAHeader->Salt.saltBytes,
			sizeof(SDAHeader->Salt.saltBytes));

		// Hash the passphrase and a rotating byte counter the specified
		// number of times into the hash field with the 8 bytes of salt
		// from above.

		k = (SDAHeader->hashReps);

		for (i=0, j=0; i<k; i++, j++)
		{
			(HashSHA.update)(&sha,pszPassword,strlen(pszPassword));	
			(HashSHA.update)(&sha,&j, 1);
		}

		HashedPassphrase=(HashSHA.final)(&sha);

		memset(pszPassword,0x00,strlen(pszPassword));
		free(pszPassword);

		*ExpandedKey=(PGPUInt32 *)malloc(sizeof(PGPUInt32)*32);
		memset(*ExpandedKey,0x00,sizeof(PGPUInt32)*32);	

		// Expand hashed passphrase 
		(cipherCAST5.initKey)(*ExpandedKey,
			HashedPassphrase);

		memcpy(PassCheckBytes,HashedPassphrase,8);

		// Check if correct by decrypting checkbytes
		(cipherCAST5.encrypt)(*ExpandedKey,
			(BYTE *)&PassCheckBytes,(BYTE *)PassCheckBytes);

		// Compare the check bytes against the key
		if (memcmp(PassCheckBytes,&(SDAHeader->CheckBytes),8)==0)
		{
			PassphraseOK=TRUE;
		}
		else
		{
			LoadString (g_hinst, IDS_WRONGPASS, szPrompt, sizeof(szPrompt));

			memset(*ExpandedKey,0x00,sizeof(PGPUInt32)*32);	
			free(*ExpandedKey);
			*ExpandedKey=NULL;
		}
	}

	return kPGPError_NoErr;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/