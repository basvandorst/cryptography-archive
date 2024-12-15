/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: working.h,v 1.15 1997/10/16 22:34:47 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_WORKING_h	/* [ */
#define Included_WORKING_h

#include "events.h"

#define PGPSC_WORK_PROGRESS 0
#define PGPSC_WORK_NEWFILE 1

#define PGPM_PROGRESS_UPDATE  WM_USER + 100

BOOL CALLBACK WorkingDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
							LPARAM lParam);

int WorkingCallback (void* arg, unsigned long SoFar, 
					 unsigned long Total) ;

void UpdateProgValBar(HWND hwnd);

void ProcessWorkingDlg(HWND hwnd);

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


UINT InternalFileWipe(HWND hwnd,char *FileName);
BOOL WipeError(HWND hwnd,char *FileName,UINT error);
BOOL OpenRings(HWND hwnd,PGPContextRef context,PGPKeySetRef *pkeyset);

#endif /* ] Included_WORKING_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
