/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: RichEdit_IO.c,v 1.2 1999/03/10 03:01:57 heller Exp $
____________________________________________________________________________*/

// System Headers
#include <windows.h>
#include <richedit.h>

// Project Headers
#include "RichEdit_IO.h"

typedef struct {
	LPSTR lpBuffer;
	LONG byteCount;
} STREAMSTRUCT;


//----------------------------------------------------|
//  Get contents of RichEdit control callback routine

DWORD CALLBACK GetRichEditCallback (DWORD dwCookie, LPBYTE pbBuff,
									LONG cb, LONG FAR *pcb) {

	STREAMSTRUCT* psS;

	psS = (STREAMSTRUCT*)dwCookie;

	if (psS->lpBuffer) psS->lpBuffer = HeapReAlloc (GetProcessHeap (), 0, psS->lpBuffer, psS->byteCount + cb + 1);
	else psS->lpBuffer = HeapAlloc (GetProcessHeap (), 0, cb+1);

	//MessageBox(NULL, "GetRichEditCallback", "Here", MB_OK);

	if (psS->lpBuffer) {
		MoveMemory (psS->lpBuffer + psS->byteCount, pbBuff, cb);
		psS->lpBuffer[psS->byteCount + cb] = '\0';
		psS->byteCount += cb;
		*pcb = cb;
		return 0;
	}

	return 1;
}

//----------------------------------------------------|
//  Get contents of RichEdit control

LPSTR GetRichEditContents (HWND hWndRichEditControl, LPLONG lpi, BOOL bRTF, BOOL bSel) {
	int iAttrib;
	EDITSTREAM eStream;
	STREAMSTRUCT sStruct;

	if (bRTF) iAttrib = SF_RTF;
	else iAttrib = SF_TEXT;

	if(bSel)
	{
		iAttrib |= SFF_SELECTION;
	}

	sStruct.lpBuffer = NULL;
	sStruct.byteCount = 0;

	eStream.dwCookie = (DWORD)&sStruct;
	eStream.dwError = 0;
	eStream.pfnCallback = GetRichEditCallback;

	SendMessage (hWndRichEditControl, EM_STREAMOUT, (WPARAM)iAttrib,
		(LPARAM)&eStream);

	if (eStream.dwError) {
		if (sStruct.lpBuffer) HeapFree (GetProcessHeap (), 0, sStruct.lpBuffer);
		return NULL;
	}

	if (lpi) *lpi = sStruct.byteCount;
	return (LPSTR)sStruct.lpBuffer;
}


//----------------------------------------------------|
//  Set contents of RichEdit control callback routine

DWORD CALLBACK SetRichEditCallback (DWORD dwCookie, LPBYTE pbBuff,
									LONG cb, LONG FAR *pcb) {
	STREAMSTRUCT* psS;

	psS = (STREAMSTRUCT*)dwCookie;

	if (cb >= psS->byteCount) {
		MoveMemory (pbBuff, psS->lpBuffer, psS->byteCount);
		*pcb = psS->byteCount;
		psS->byteCount = 0;
		return 0;
	}
	else {
		MoveMemory (pbBuff, psS->lpBuffer, cb);
		*pcb = cb;
		psS->byteCount -= cb;
		psS->lpBuffer += cb;
		return 0;
	}
}

//----------------------------------------------------|
//  Set contents of RichEdit control

LONG SetRichEditContents (HWND hWndRichEditControl, LPSTR lpBuffer, BOOL bRTF, BOOL bSel) {
	int iAttrib;
	EDITSTREAM eStream;
	STREAMSTRUCT sStruct;

	if (bRTF) iAttrib = SF_RTF;
	else iAttrib = SF_TEXT;

	if(bSel)
	{
		iAttrib |= SFF_SELECTION;
	}

	sStruct.lpBuffer = lpBuffer;
	sStruct.byteCount = lstrlen (lpBuffer);

	eStream.dwCookie = (DWORD)&sStruct;
	eStream.dwError = 0;
	eStream.pfnCallback = SetRichEditCallback;

	SendMessage (hWndRichEditControl, EM_STREAMIN, (WPARAM)iAttrib,
		(LPARAM)&eStream);

	SendMessage (hWndRichEditControl, EM_SETMODIFY, (WPARAM)TRUE, 0L);

	if (eStream.dwError) return 1;
	return 0;
}


