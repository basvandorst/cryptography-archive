/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	GMIDropTarget.cpp - implements OLE IDropTarget for drag and drop
	
	$Id: GMIDropTarget.cpp,v 1.6 1998/08/11 15:20:00 pbj Exp $
____________________________________________________________________________*/

// system header files
#include <windows.h>
#include <commctrl.h>

// project header files
#include "GMIDropTarget.h"
#include "..\include\treelist.h"

extern "C" {
typedef struct _GM *PGROUPMAN;
BOOL GMDropKeys (PGROUPMAN pGM, HANDLE hMem);
BOOL GMImportGroups (PGROUPMAN pGM, HDROP hDrop);
BOOL GMSelectGroup (PGROUPMAN pGM, POINTL pt);
}

//	___________________________________________________
//
//	IUnknown Methods

STDMETHODIMP CDropTarget::QueryInterface (
		REFIID 			iid, 
		void FAR* FAR* 	ppv) 
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


//	___________________________________________________
//
//	CDropTarget Constructor
 
CDropTarget::CDropTarget (
		HWND 	hwnd, 
		VOID* 	pGroupMan) 
{
    m_refs = 1;  
	m_bAcceptFmt = FALSE;
	m_bEnabled = FALSE;
	m_pGroupMan = pGroupMan;
	m_hwnd = hwnd;
	m_fmtDrop = 0;
}


//	___________________________________________________
//
//	Enable/disable dropping.  Used when dragging a key out
//	of key manager.
 
void CDropTarget::Enable (BOOL bEnable) 
{
    m_bEnabled = bEnable;  
}


//	___________________________________________________
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
		m_fmtDrop = CF_TEXT;
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
		m_fmtDrop = CF_HDROP;
		*pdwEffect = DROPEFFECT_COPY;
	}
   
	return NOERROR;
}


STDMETHODIMP CDropTarget::DragOver (
		DWORD 	grfKeyState, 
		POINTL 	pt, 
		LPDWORD pdwEffect) 
{
	*pdwEffect = DROPEFFECT_NONE; 
	if (m_bAcceptFmt && m_bEnabled) {
		if (m_fmtDrop == CF_HDROP)
			*pdwEffect = DROPEFFECT_COPY; 
		else {
			if (GMSelectGroup ((PGROUPMAN)m_pGroupMan, pt)) 
				*pdwEffect = DROPEFFECT_COPY; 
		}
	}
	return NOERROR;
}


STDMETHODIMP CDropTarget::DragLeave () 
{   
	m_bAcceptFmt = FALSE;   
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
			GMDropKeys ((PGROUPMAN)m_pGroupMan, hGlobal);
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
			GMImportGroups ((PGROUPMAN)m_pGroupMan, hGlobal);
			ReleaseStgMedium (&medium);
			*pdwEffect = DROPEFFECT_COPY;
			return NOERROR;
		}
	}
	return hr;      
}   


//	___________________________________________________
//
//	Interface to C code 

extern "C" {
LPDROPTARGET GMCreateDropTarget (
		HWND	hwnd, 
		VOID* 	pGroupMan) 
{
	return ((LPDROPTARGET) new CDropTarget (hwnd, pGroupMan));
}

void GMReleaseDropTarget (LPDROPTARGET pDropTarget) 
{
	pDropTarget->Release ();
}

void GMEnableDropTarget (
		CDropTarget*	pDropTarget, 
		BOOL 			bEnable) 
{
	pDropTarget->Enable (bEnable);
}

}