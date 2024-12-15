/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: working.h,v 1.25 1999/03/23 17:34:00 wjb Exp $
____________________________________________________________________________*/
#ifndef Included_WORKING_h	/* [ */
#define Included_WORKING_h

#ifdef __cplusplus
extern "C" {
#endif

#include "events.h"

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

PGPError DoWorkThread(HPRGDLG hPrgDlg,void *pUserValue);

PGPError SDA(MYSTATE *ms);

#define kBlockSize			512
#define	kBlockSizeInUInt32	(kBlockSize / sizeof( PGPUInt32 ))

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
UINT InternalFileWipe(PGPContextRef context,HWND hwnd,
					  char *FileName,BOOL bDelFilePostWipe);
BOOL OpenRings(HWND hwnd,PGPContextRef context,PGPKeySetRef *pkeyset);
BOOL FileHasThisExtension(char *filename,char *extension);

LRESULT 
PGPscMessageBox (
		 HWND	hWnd, 
		 INT	iCaption, 
		 INT	iMessage,
		 ULONG	ulFlags) ;

#ifdef __cplusplus
}
#endif

#endif /* ] Included_WORKING_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
