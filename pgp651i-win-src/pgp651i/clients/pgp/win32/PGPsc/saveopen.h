/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: saveopen.h,v 1.9 1999/03/10 03:02:30 heller Exp $
____________________________________________________________________________*/
#ifndef Included_SAVEOPEN_h	/* [ */
#define Included_SAVEOPEN_h

BOOL AlterEncryptedFileName(char *FileName,DWORD Actions);

BOOL AlterDecryptedFileName(char *FileName,char *SuggestedName);

BOOL  SaveOutputFile(PGPContextRef Context,
					 HWND hwnd, 
					 char *Title,
					 char *InputFile, 
					 PGPFileSpecRef *pOutputFileRef,
					 BOOL Force);

BOOL GetOriginalFileRef(HWND hwnd,PGPContextRef context,
						char *InputFile,
						char *OutputFile,
						PGPFileSpecRef *OriginalFileRef,
						HWND hwndWorking);


#endif /* ] Included_SAVEOPEN_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
