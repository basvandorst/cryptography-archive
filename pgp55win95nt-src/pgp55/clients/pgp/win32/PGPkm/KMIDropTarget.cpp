//:KMIDropTarget.cpp - implements OLE IDropTarget for drag and drop
//
//	$Id: KMIDropTarget.cpp,v 1.1 1997/06/16 14:55:55 pbj Exp $
//

#include <windows.h>
#include "KMIDropTarget.h"

extern "C" {
typedef struct _KM *PKEYMAN;
BOOL KMDropKeys (PKEYMAN pKM, HANDLE hMem);
BOOL KMImportKey (PKEYMAN pKM, HANDLE hMem);
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
 
CDropTarget::CDropTarget (VOID* pKeyMan) {
    m_refs = 1;  
	m_bAcceptFmt = FALSE;
	m_bEnabled = FALSE;
	m_pKeyMan = pKeyMan;
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
    
	// default to not allowing drop
	m_bAcceptFmt = FALSE;
 	*pdwEffect = DROPEFFECT_NONE;

	if (!m_bEnabled) {
		return NOERROR;
	}

	// Does the drag source provide CF_TEXT?     
	fmtetc.cfFormat = CF_TEXT;
	fmtetc.ptd      = NULL;
	fmtetc.dwAspect = DVASPECT_CONTENT;  
	fmtetc.lindex   = -1;
	fmtetc.tymed    = TYMED_HGLOBAL; 
    
	if (pDataObj->QueryGetData (&fmtetc) == NOERROR) {
		m_bAcceptFmt = TRUE;
		*pdwEffect = DROPEFFECT_COPY;
	}
   
	// Does the drag source provide CF_HDROP?     
	fmtetc.cfFormat = CF_HDROP;
	fmtetc.ptd      = NULL;
	fmtetc.dwAspect = DVASPECT_CONTENT;  
	fmtetc.lindex   = -1;
	fmtetc.tymed    = TYMED_HGLOBAL; 
    
	if (pDataObj->QueryGetData (&fmtetc) == NOERROR) {
		m_bAcceptFmt = TRUE;
		*pdwEffect = DROPEFFECT_COPY;
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
	HGLOBAL hGlobal;
	HRESULT hr;
     
	*pdwEffect = DROPEFFECT_NONE;
	hr = NOERROR;

	if (m_bAcceptFmt && m_bEnabled) {      
		// User has dropped on us. Try getting CF_TEXT data from drag source
		fmtetc.cfFormat = CF_TEXT;
		fmtetc.ptd = NULL;
		fmtetc.dwAspect = DVASPECT_CONTENT;  
		fmtetc.lindex = -1;
		fmtetc.tymed = TYMED_HGLOBAL;       
        
		hr = pDataObj->GetData (&fmtetc, &medium);
		if (!FAILED(hr)) {
		// Import the data and release it.
			hGlobal = medium.hGlobal;
			KMDropKeys ((PKEYMAN)m_pKeyMan, hGlobal);
			ReleaseStgMedium (&medium);
			*pdwEffect = DROPEFFECT_COPY;
			return NOERROR;
		}

		// no CF_TEXT, try getting CF_HDROP data from drag source
		fmtetc.cfFormat = CF_HDROP;
		fmtetc.ptd = NULL;
		fmtetc.dwAspect = DVASPECT_CONTENT;  
		fmtetc.lindex = -1;
		fmtetc.tymed = TYMED_HGLOBAL;       
        
		hr = pDataObj->GetData (&fmtetc, &medium);
		if (!FAILED(hr)) {
		// Import the data and release it.
			hGlobal = medium.hGlobal;
			KMImportKey ((PKEYMAN)m_pKeyMan, hGlobal);
			ReleaseStgMedium (&medium);
			*pdwEffect = DROPEFFECT_COPY;
			return NOERROR;
		}

	}
	return hr;      
}   


//----------------------------------------------------|
// Interface to C code 

extern "C" {
LPDROPTARGET KMCreateDropTarget (VOID* pKeyMan) {
	return ((LPDROPTARGET) new CDropTarget (pKeyMan));
}

void KMReleaseDropTarget (LPDROPTARGET pDropTarget) {
	pDropTarget->Release ();
}

void KMEnableDropTarget (CDropTarget* pDropTarget, BOOL bEnable) {
	pDropTarget->Enable (bEnable);
}

}