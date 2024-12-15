/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//:KMIDropTarget.cpp - implements OLE IDropTarget for drag and drop
//
//	$Id: KMIDropTarget.cpp,v 1.2 1997/05/09 19:29:16 pbj Exp $
//

#include <windows.h>
#include "KMIDropTarget.h"

extern "C" {
BOOL KMDropKeys (HANDLE hMem);
}

//----------------------------------------------------|
// IUnknown Methods

STDMETHODIMP CDropTarget::QueryInterface (REFIID iid, void FAR* FAR* ppv) {
    if ((iid == IID_IUnknown) || (iid == IID_IDropTarget)) {
      *ppv = this;
      ++m_refs;
      return NOERROR;
    }
    *ppv = NULL;

    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CDropTarget::AddRef (void) {
     return ++m_refs;
}


STDMETHODIMP_(ULONG) CDropTarget::Release (void) {
    if (--m_refs == 0) {
      delete this;
      return 0;
    }
    return m_refs;
}


//----------------------------------------------------|
// CDropTarget Constructor

CDropTarget::CDropTarget () {
    m_refs = 1;
	m_bAcceptFmt = FALSE;
	m_bEnabled = FALSE;
}


//----------------------------------------------------|
// Enable/disable dropping.  Used when dragging a key out
// of key manager.

void CDropTarget::Enable (BOOL bEnable) {
    m_bEnabled = bEnable;
}


//----------------------------------------------------|
// IDropTarget Methods

STDMETHODIMP CDropTarget::DragEnter (LPDATAOBJECT pDataObj, DWORD grfKeyState,
									POINTL pt, LPDWORD pdwEffect) {
	FORMATETC fmtetc;

	if (!m_bEnabled) {
		*pdwEffect = DROPEFFECT_NONE;
		return NOERROR;
	}

	fmtetc.cfFormat = CF_TEXT;
	fmtetc.ptd      = NULL;
	fmtetc.dwAspect = DVASPECT_CONTENT;
	fmtetc.lindex   = -1;
	fmtetc.tymed    = TYMED_HGLOBAL;

	// Does the drag source provide CF_TEXT? -- the only format we accept.
	if (pDataObj->QueryGetData (&fmtetc) == NOERROR) {
		m_bAcceptFmt = TRUE;
		*pdwEffect = DROPEFFECT_COPY;
	}
	else {
		m_bAcceptFmt = FALSE;
 		*pdwEffect = DROPEFFECT_NONE;
	}

	return NOERROR;
}


STDMETHODIMP CDropTarget::DragOver (DWORD grfKeyState, POINTL pt,
									LPDWORD pdwEffect) {
	if (m_bAcceptFmt && m_bEnabled) *pdwEffect = DROPEFFECT_COPY;
	else *pdwEffect = DROPEFFECT_NONE;
	return NOERROR;
}


STDMETHODIMP CDropTarget::DragLeave () {
	m_bAcceptFmt = FALSE;
	return NOERROR;
}


STDMETHODIMP CDropTarget::Drop(LPDATAOBJECT pDataObj, DWORD grfKeyState,
							   POINTL pt, LPDWORD pdwEffect) {
	FORMATETC fmtetc;
	STGMEDIUM medium;
	HGLOBAL hText;
	HRESULT hr;

	if (m_bAcceptFmt && m_bEnabled) {
		fmtetc.cfFormat = CF_TEXT;
		fmtetc.ptd = NULL;
		fmtetc.dwAspect = DVASPECT_CONTENT;
		fmtetc.lindex = -1;
		fmtetc.tymed = TYMED_HGLOBAL;

		// User has dropped on us. Get the CF_TEXT data from drag source
		hr = pDataObj->GetData (&fmtetc, &medium);
		if (FAILED(hr)) {
			*pdwEffect = DROPEFFECT_NONE;
			return hr;
		}

		// Import the data and release it.
		hText = medium.hGlobal;
		KMDropKeys (hText);
		ReleaseStgMedium (&medium);
		*pdwEffect = DROPEFFECT_COPY;
	}
	return NOERROR;
}


//----------------------------------------------------|
// Interface to C code

extern "C" {
LPDROPTARGET KMCreateDropTarget (void) {
	return ((LPDROPTARGET) new CDropTarget);
}

void KMReleaseDropTarget (LPDROPTARGET pDropTarget) {
	pDropTarget->Release ();
}

void KMEnableDropTarget (CDropTarget* pDropTarget, BOOL bEnable) {
	pDropTarget->Enable (bEnable);
}

}
