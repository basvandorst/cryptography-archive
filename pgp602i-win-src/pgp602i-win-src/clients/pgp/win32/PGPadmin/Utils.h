/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: Utils.h,v 1.14.8.1 1998/11/12 03:13:15 heller Exp $
____________________________________________________________________________*/
#ifndef Included_Utils_h	/* [ */
#define Included_Utils_h

#include "pgpBase.h"
#include "pgpKeys.h"

void ShowKeyring(HWND hWnd, 
				 int nListCtrl,
				 PGPContextRef pgpContext,
				 PGPKeySetRef onlyThisKeySet,
				 PGPBoolean bShowRSA, 
				 PGPBoolean bShowDH,
				 PGPBoolean bMustEncrypt,
				 PGPBoolean bMustSign);

BOOL SelectKeys(HWND hWnd,
				int nListCtrl,
				pgpConfigInfo *pConfig);

void AddSelectedKeys(HWND hWnd, 
					 int nListCtrl, 
					 PGPContextRef pgpContext,
					 PGPKeySetRef *pKeySet);

void FreeKeyring(HWND hWnd, 
				 int nListCtrl);

void PaintWizBitmap(HWND hWnd, HBITMAP hBitmap);

BOOL GetCommentString(PGPMemoryMgrRef memoryMgr, 
					  char *szComment, 
					  int nLength);

#endif /* ] Included_Utils_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
