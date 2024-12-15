/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//:KMIDropSource.cpp - implements OLE IDropSource for drag and drop
//
//	$Id: KMIDropSource.cpp,v 1.2 1997/05/09 19:29:15 pbj Exp $
//

#include <windows.h>
#include "KMIDropSource.h"


//----------------------------------------------------|
// IUnknown Methods

STDMETHODIMP CDropSource::QueryInterface (REFIID iid, void FAR* FAR* ppv) {
    if ((iid == IID_IUnknown) || (iid == IID_IDropSource)) {
      *ppv = this;
      ++m_refs;
      return NOERROR;
    }
    *ppv = NULL;

    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CDropSource::AddRef (void) {
    return ++m_refs;
}


STDMETHODIMP_(ULONG) CDropSource::Release (void) {
    if (--m_refs == 0) {
      delete this;
      return 0;
    }
    return m_refs;
}


//----------------------------------------------------|
// CDropSource Constructor

CDropSource::CDropSource() {
     m_refs = 1;
}


//----------------------------------------------------|
// IDropSource Methods

STDMETHODIMP CDropSource::QueryContinueDrag (BOOL fEscapePressed,
                                            DWORD grfKeyState) {

    if (fEscapePressed)
        return DRAGDROP_S_CANCEL;
    else if (!(grfKeyState & MK_LBUTTON) && !(grfKeyState & MK_RBUTTON))
        return DRAGDROP_S_DROP;
    else
        return NOERROR;
}

STDMETHODIMP CDropSource::GiveFeedback (DWORD dwEffect) {
     return DRAGDROP_S_USEDEFAULTCURSORS;
}


//----------------------------------------------------|
// Interface to C code

extern "C" {
LPDROPSOURCE KMCreateDropSource(void) {
   return ((LPDROPSOURCE) new CDropSource);
}
}
