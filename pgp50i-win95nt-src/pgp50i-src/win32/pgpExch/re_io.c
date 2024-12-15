/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
//: RE_io.c - RichEdit input/output routines
//. 14nov96 pbj

#include <windows.h>
#include <richedit.h>

#include "RE_io.h"

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

    if (psS->byteCount && psS->lpBuffer)
    {
        LPSTR lpNewBuffer;

        lpNewBuffer = HeapReAlloc(GetProcessHeap (), 0, psS->lpBuffer,
                                    cb+psS->byteCount);
        if (lpNewBuffer)
        {
            psS->lpBuffer = lpNewBuffer;
            MoveMemory(&(psS->lpBuffer[psS->byteCount-1]), pbBuff, cb);
            psS->lpBuffer[cb+psS->byteCount-1] = '\0';
            psS->byteCount += cb;
            *pcb = cb;
            return 0;
        }
    }

    else
    {
        psS->lpBuffer = HeapAlloc (GetProcessHeap (), 0, cb+1);

        if (psS->lpBuffer)
        {
            MoveMemory (psS->lpBuffer, pbBuff, cb);
            psS->lpBuffer[cb] = '\0';
            psS->byteCount = cb+1;
            *pcb = cb;
            return 0;
        }
    }

    return 1;
}

//----------------------------------------------------|
//  Get contents of RichEdit control

LPSTR GetRichEditContents (HWND hWndRichEditControl, LPLONG lpi,
                           BOOL bRTF, BOOL bSel) {
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

LONG SetRichEditContents (HWND hWndRichEditControl, LPSTR lpBuffer,
                          BOOL bRTF, BOOL bSel) {
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
