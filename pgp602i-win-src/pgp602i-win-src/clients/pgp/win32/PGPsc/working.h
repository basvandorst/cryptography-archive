/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: working.h,v 1.19.8.1 1998/11/12 03:13:41 heller Exp $
____________________________________________________________________________*/
#ifndef Included_WORKING_h	/* [ */
#define Included_WORKING_h

#include "events.h"

#define PGPSC_WORK_PROGRESS 0
#define PGPSC_WORK_NEWFILE 1

#define PGPM_PROGRESS_UPDATE  WM_USER + 100

#define MS_ENCRYPTFILELIST 1
#define MS_ENCRYPTCLIPBOARD 2
#define MS_DECRYPTFILELIST 3
#define MS_DECRYPTCLIPBOARD 4
#define MS_ADDKEYFILELIST 5
#define MS_ADDKEYCLIPBOARD 6
#define MS_WIPEFILELIST 7

PGPError DecryptFileListStub (MYSTATE *ms) ;
PGPError DecryptClipboardStub (MYSTATE *ms) ;
PGPError EncryptFileListStub (MYSTATE *ms) ;
PGPError EncryptClipboardStub (MYSTATE *ms) ;
PGPError AddKeyFileListStub (MYSTATE *ms) ;
PGPError AddKeyClipboardStub (MYSTATE *ms) ;
PGPError WipeFileListStub (MYSTATE *ms) ;

PGPError DoWork(HWND hwndParent,MYSTATE *myState);

int WorkingCallback (void* arg, unsigned long SoFar, unsigned long Total) ;

#define WIPE_OK 1
#define WIPE_NOTOPEN 2
#define WIPE_USERABORT 3
#define WIPE_NOSTATS 4
#define WIPE_NOREMOVE 5
#define WIPE_NOFLUSH 6
#define WIPE_NOFILEMAP 7
#define WIPE_NOMEMMAP 8
#define WIPE_ALREADYOPEN 9
#define WIPE_WRITEPROTECT 10

BOOL WipeError(HWND hwnd,FILELIST *FileEntry,UINT error);
UINT InternalFileWipe(PGPContextRef context,HWND hwnd,char *FileName);
BOOL OpenRings(HWND hwnd,PGPContextRef context,PGPKeySetRef *pkeyset);
BOOL FileHasThisExtension(char *filename,char *extension);

LRESULT 
PGPscMessageBox (
		 HWND	hWnd, 
		 INT	iCaption, 
		 INT	iMessage,
		 ULONG	ulFlags) ;

#endif /* ] Included_WORKING_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
