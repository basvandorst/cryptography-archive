/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMIDropTarget.cpp - implements OLE IDropTarget for drag and drop

	$Id: KMIDropTarget.cpp,v 1.8 1998/08/13 02:46:09 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// system header files
#include <windows.h>
#include <commctrl.h>

// project header files
#include "KMIDropTarget.h"
#include "KMIDataObject.h"

extern "C" {
typedef struct _KM *PKEYMAN;
typedef struct _PSKS *PSKS;
BOOL KMDropKeys (PKEYMAN pKM, HANDLE hMem);
BOOL KMImportKey (PKEYMAN pKM, HANDLE hMem);
BOOL KMSplitDropKeys (PSKS psks, HANDLE hMem);
}

//	________________________________________
//
//	IUnknown Methods

STDMETHODIMP CDropTarget::QueryInterface (
		REFIID			iid, 
		void FAR* FAR*	ppv) 
{
    if ((iid == IID_IUnknown) || (iid == IID_IDropTarget)) {
      *ppv = this;
      ++m_refs;
      return NOERROR;
    }
    *ppv = NULL;

    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CDropTarget::AddRef (void) 
{
    return ++m_refs;
}


STDMETHODIMP_(ULONG) CDropTarget::Release (void) 
{
    if (--m_refs == 0) {
      delete this;
      return 0;
    }
    return m_refs;
}  


//	________________________________________
//
//	CDropTarget Constructor
 
CDropTarget::CDropTarget (
		HWND 	hwnd, 
		VOID* 	pKeyMan, 
		VOID* 	pSplitStruct) 
{
    m_refs = 1;  
	m_hwnd = hwnd;
	m_bAcceptFmt = FALSE;
	m_bEnabled = FALSE;
	m_pKeyMan = pKeyMan;
	m_pSplitStruct = pSplitStruct;
}


//	________________________________________
//
//	Enable/disable dropping.  Used when dragging a key out
//	of key manager.
 
void CDropTarget::Enable (BOOL bEnable) 
{
    m_bEnabled = bEnable;  
}


//	________________________________________
//
//	IDropTarget Methods

STDMETHODIMP CDropTarget::DragEnter (
		LPDATAOBJECT	pDataObj, 
		DWORD 			grfKeyState, 
		POINTL 			pt, 
		LPDWORD 		pdwEffect) 
{  
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
   
	if (m_pKeyMan) {
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
	}
   
	return NOERROR;
}


STDMETHODIMP CDropTarget::DragOver (
		DWORD	grfKeyState, 
		POINTL	pt, 
		LPDWORD	pdwEffect) 
{
	if (m_bAcceptFmt && m_bEnabled) *pdwEffect = DROPEFFECT_COPY; 
	else *pdwEffect = DROPEFFECT_NONE; 
	return NOERROR;
}


STDMETHODIMP CDropTarget::DragLeave () 
{   
	m_bAcceptFmt = FALSE;   
	ImageList_DragLeave (m_hwnd);
	return NOERROR;
}


STDMETHODIMP CDropTarget::Drop (
		LPDATAOBJECT	pDataObj, 
		DWORD 			grfKeyState, 
		POINTL 			pt, 
		LPDWORD 		pdwEffect) 
{   
	FORMATETC fmtetc;   
	STGMEDIUM medium;   
	HGLOBAL hGlobal;
	HRESULT hr;
     
	*pdwEffect = DROPEFFECT_NONE;
	hr = NOERROR;

	if (m_bAcceptFmt && m_bEnabled) {      
		// User has dropped on us. First, try getting data in the 
		// private PGP format which is always a complete representation 
		// of the key(s)
		fmtetc.cfFormat = RegisterClipboardFormat (CFSTR_PGPCOMPLETEFORMAT);
		fmtetc.ptd = NULL;
		fmtetc.dwAspect = DVASPECT_CONTENT;  
		fmtetc.lindex = -1;
		fmtetc.tymed = TYMED_HGLOBAL;       
        
		hr = pDataObj->GetData (&fmtetc, &medium);
		if (!FAILED(hr)) {
		// Import the data and release it.
			hGlobal = medium.hGlobal;
			if (m_pKeyMan)
				KMDropKeys ((PKEYMAN)m_pKeyMan, hGlobal);
			else
				KMSplitDropKeys ((PSKS)m_pSplitStruct, hGlobal);
			ReleaseStgMedium (&medium);
			*pdwEffect = DROPEFFECT_COPY;
			return NOERROR;
		}

		// Next try getting CF_TEXT data from drag source
		fmtetc.cfFormat = CF_TEXT;
		fmtetc.ptd = NULL;
		fmtetc.dwAspect = DVASPECT_CONTENT;  
		fmtetc.lindex = -1;
		fmtetc.tymed = TYMED_HGLOBAL;       
        
		hr = pDataObj->GetData (&fmtetc, &medium);
		if (!FAILED(hr)) {
		// Import the data and release it.
			hGlobal = medium.hGlobal;
			if (m_pKeyMan)
				KMDropKeys ((PKEYMAN)m_pKeyMan, hGlobal);
			else
				KMSplitDropKeys ((PSKS)m_pSplitStruct, hGlobal);
			ReleaseStgMedium (&medium);
			*pdwEffect = DROPEFFECT_COPY;
			return NOERROR;
		}

		if (m_pKeyMan) {
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

	}
	return hr;      
}   


//	________________________________________
//
//	Interface to C code 

extern "C" {
LPDROPTARGET KMCreateDropTarget (
		HWND	hwnd, 
		VOID* 	pKeyMan, 
		VOID* 	pSplitStruct) 
{
	return ((LPDROPTARGET) new CDropTarget (hwnd, pKeyMan, pSplitStruct));
}

void KMReleaseDropTarget (LPDROPTARGET pDropTarget) 
{
	pDropTarget->Release ();
}

void KMEnableDropTarget (CDropTarget* pDropTarget, BOOL bEnable) 
{
	pDropTarget->Enable (bEnable);
}

}