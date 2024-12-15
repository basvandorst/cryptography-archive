/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMIDataObject.cpp - implements OLE IDataObject for drag and drop
	to the shell.

	$Id: KMIDataObject.cpp,v 1.7 1998/08/13 02:46:08 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// system header files
#include <windows.h> 
#include <shlobj.h>

// project header files
#include "KMIDataObject.h"
#include "KMIEnumFormatEtc.h"
#include "resource.h"

extern "C" {
typedef struct _KM *PKEYMAN;
LPSTR KMDropKeysGetData (PKEYMAN pKM, BOOL bForceComplete);
VOID KMDropKeysFreeData (LPSTR psz);
}

//	________________________________________
//
//	IUnknown Methods

STDMETHODIMP CDataObject::QueryInterface (
		REFIID			iid, 
		void FAR* FAR*	ppv) 
{
    if ((iid == IID_IUnknown) || (iid == IID_IDataObject)) {
      *ppv = this;
      ++m_refs;
      return NOERROR;
    }
    *ppv = NULL;

    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CDataObject::AddRef (void) 
{
    return ++m_refs;
}


STDMETHODIMP_(ULONG) CDataObject::Release (void) 
{
	if(--m_refs == 0) {
      delete this;
      return 0;
    }
    return m_refs;
}  


//	________________________________________
//
//	CDataObject Constructor
 
CDataObject::CDataObject (
		VOID*	pKeyMan, 
		LPSTR	szName) 
{
	m_szName = szName;
	m_szData = NULL;
	m_pKeyMan = pKeyMan;
    m_refs = 1;  
	m_cfe = 4;

	m_fe[0].cfFormat = RegisterClipboardFormat (CFSTR_FILEDESCRIPTOR);
	m_fe[0].ptd = NULL;
	m_fe[0].dwAspect = DVASPECT_CONTENT;
	m_fe[0].lindex = -1;
	m_fe[0].tymed = TYMED_HGLOBAL;

	m_fe[1].cfFormat = RegisterClipboardFormat (CFSTR_FILECONTENTS);
	m_fe[1].ptd = NULL;
	m_fe[1].dwAspect = DVASPECT_CONTENT;
	m_fe[1].lindex = -1;
	m_fe[1].tymed = TYMED_HGLOBAL;

	m_fe[2].cfFormat = RegisterClipboardFormat (CFSTR_PGPCOMPLETEFORMAT);
	m_fe[2].ptd = NULL;
	m_fe[2].dwAspect = DVASPECT_CONTENT;
	m_fe[2].lindex = -1;
	m_fe[2].tymed = TYMED_HGLOBAL;

	m_fe[3].cfFormat = CF_TEXT;
	m_fe[3].ptd = NULL;
	m_fe[3].dwAspect = DVASPECT_CONTENT;
	m_fe[3].lindex = -1;
	m_fe[3].tymed = TYMED_HGLOBAL;
}

//	________________________________________
//
//	CDataObject Destructor
 
CDataObject::~CDataObject () 
{
	KMDropKeysFreeData (m_szData);
}

//	________________________________________
//
//	IDataObject Methods

//	________________________________________
//
//	CDataObject::GetData
// 
//	Purpose:
//		Retrieves data described by a specific FormatEtc into a StgMedium
//		allocated by this function.  Used like GetClipboardData.
//
//	Parameters:
//		pFE             LPFORMATETC describing the desired data.
//		pSTM            LPSTGMEDIUM in which to return the data.
//
//	Return Value:
//		HRESULT         NOERROR or a general error value.
//

STDMETHODIMP CDataObject::GetData (
		LPFORMATETC		pFE, 
		LPSTGMEDIUM 	pSTM) 
{
    HGLOBAL hMem;
	LPFILEGROUPDESCRIPTOR pfgd;
    UINT uFormat;
	CHAR* p;

	m_uFmtUsed = 0;
	uFormat = pFE->cfFormat;

	// FILEDESCRIPTOR format
	if (uFormat == m_fe[0].cfFormat) {	
		if (!(pFE->dwAspect & DVASPECT_CONTENT))
			return DV_E_DVASPECT;
        if (!(pFE->tymed & TYMED_HGLOBAL))
			return DV_E_FORMATETC;

	    hMem=GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, 
							sizeof(FILEGROUPDESCRIPTOR));

		if (hMem == NULL) return STG_E_MEDIUMFULL;

		pfgd = (LPFILEGROUPDESCRIPTOR)GlobalLock (hMem);
		pfgd->cItems = 1;
		pfgd->fgd[0].dwFlags = FD_ATTRIBUTES;
		pfgd->fgd[0].dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
		lstrcpy (pfgd->fgd[0].cFileName, m_szName);

		GlobalUnlock (hMem);

		pSTM->hGlobal = hMem;
		pSTM->tymed = TYMED_HGLOBAL;
		pSTM->pUnkForRelease = NULL;
		m_uFmtUsed = uFormat;
		return S_OK;	
	}

	// FILECONTENTS clipboard format
	else if (uFormat == m_fe[1].cfFormat) {	
		if (!(pFE->dwAspect & DVASPECT_CONTENT))
			return DV_E_DVASPECT;
        if (!(pFE->tymed & TYMED_HGLOBAL))
			return DV_E_FORMATETC;

		if (!m_szData) 
			m_szData = KMDropKeysGetData ((PKEYMAN)m_pKeyMan, FALSE);

	    hMem=GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, lstrlen(m_szData)+1);

		if (hMem == NULL) return STG_E_MEDIUMFULL;

		p = (LPSTR) GlobalLock (hMem);
		lstrcpy (p, m_szData);
		GlobalUnlock (hMem);

		pSTM->hGlobal = hMem;
		pSTM->tymed = TYMED_HGLOBAL;
		pSTM->pUnkForRelease = NULL;
		m_uFmtUsed = uFormat;
		return S_OK;	
	}

	// CFSTR_PGPCOMPLETEFORMAT clipboard format
	else if (uFormat == m_fe[2].cfFormat) {
		if (pFE->lindex != -1) return DV_E_LINDEX;
		if (!(pFE->dwAspect & DVASPECT_CONTENT))
			return DV_E_DVASPECT;
        if (!(pFE->tymed & TYMED_HGLOBAL))
			return DV_E_FORMATETC;

		if (!m_szData) 
			m_szData = KMDropKeysGetData ((PKEYMAN)m_pKeyMan, TRUE);

	    hMem=GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, lstrlen(m_szData)+1);

		if (hMem == NULL) return STG_E_MEDIUMFULL;

		p = (LPSTR) GlobalLock (hMem);
		lstrcpy (p, m_szData);
		GlobalUnlock (hMem);

		pSTM->hGlobal = hMem;
		pSTM->tymed = TYMED_HGLOBAL;
		pSTM->pUnkForRelease = NULL;
		m_uFmtUsed = uFormat;
		return S_OK;	
	}

	// TEXT clipboard format
	else if (uFormat == CF_TEXT) {
		if (pFE->lindex != -1) return DV_E_LINDEX;
		if (!(pFE->dwAspect & DVASPECT_CONTENT))
			return DV_E_DVASPECT;
        if (!(pFE->tymed & TYMED_HGLOBAL))
			return DV_E_FORMATETC;

		if (!m_szData) 
			m_szData = KMDropKeysGetData ((PKEYMAN)m_pKeyMan, FALSE);

	    hMem=GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, lstrlen(m_szData)+1);

		if (hMem == NULL) return STG_E_MEDIUMFULL;

		p = (LPSTR) GlobalLock (hMem);
		lstrcpy (p, m_szData);
		GlobalUnlock (hMem);

		pSTM->hGlobal = hMem;
		pSTM->tymed = TYMED_HGLOBAL;
		pSTM->pUnkForRelease = NULL;
		m_uFmtUsed = uFormat;
		return S_OK;	
	}

    return DV_E_FORMATETC;
}


//	________________________________________
//
//	CDataObject::GetDataHere
//
//	Purpose:
//		Renders the specific FormatEtc into caller-allocated medium
//		provided in pSTM.
//
//	Parameters:
//		pFE             LPFORMATETC describing the desired data.
//		pSTM            LPSTGMEDIUM providing the medium into which
//						we render the data.
//
//	Return Value:
//		HRESULT         NOERROR or a general error value.
//

STDMETHODIMP CDataObject::GetDataHere (
		LPFORMATETC 	pFE, 
		LPSTGMEDIUM 	pSTM) 
{
    //Check the aspects we support.
    if (!(pFE->dwAspect & DVASPECT_CONTENT))
        return DV_E_DVASPECT;

	if (pFE->lindex != -1) return DV_E_LINDEX;

    return DV_E_FORMATETC;
}


//	________________________________________
//
//	CDataObject::QueryGetData
//
//	Purpose:
//		Tests if a call to GetData with this FormatEtc will provide
//		any rendering; used like IsClipboardFormatAvailable.
//
//	Parameters:
//		pFE             LPFORMATETC describing the desired data.
//
//	Return Value:
//		HRESULT         NOERROR or a general error value.
//

STDMETHODIMP CDataObject::QueryGetData (LPFORMATETC pFE) 
{
    UINT uFormat = pFE->cfFormat;
    BOOL bRet = FALSE;

    //Check the aspects we support.
    if (!(pFE->dwAspect & DVASPECT_CONTENT))
        return DV_E_DVASPECT;

	if (uFormat == m_fe[0].cfFormat) {			// filegroupdescriptor
		bRet = (BOOL)(pFE->tymed & TYMED_HGLOBAL);
	}
	else if (uFormat == m_fe[1].cfFormat) {		// filecontents
		bRet = (BOOL)(pFE->tymed & TYMED_HGLOBAL);
	}
	else if (uFormat == m_fe[2].cfFormat) {		// private PGP format
		if (pFE->lindex != -1) return DV_E_LINDEX;
		bRet = (BOOL)(pFE->tymed & TYMED_HGLOBAL);
	}
	else if (uFormat == CF_TEXT) {
		if (pFE->lindex != -1) return DV_E_LINDEX;
		bRet = (BOOL)(pFE->tymed & TYMED_HGLOBAL);
	}

	else bRet = FALSE;

    return (bRet ? S_OK : DV_E_FORMATETC);
}


//	________________________________________
//
//	CDataObject::EnumFormatEtc
//
//	Purpose:
//		Returns an IEnumFORMATETC object through which the caller can
//		iterate to learn about all the data formats this object can
//		provide through either GetData[Here] or SetData.
//
//	Parameters:
//		dwDir           DWORD describing a data direction, either
//						DATADIR_SET or DATADIR_GET.
//		ppEnum          LPENUMFORMATETC * in which to return the
//						pointer to the enumerator.
//
//	Return Value:
//		HRESULT         NOERROR or a general error value.
//

STDMETHODIMP CDataObject::EnumFormatEtc (
		DWORD				dwDir, 
		LPENUMFORMATETC*	ppEnum) 
{
    switch (dwDir) {
    case DATADIR_GET:
        *ppEnum = new CEnumFormatEtc (m_cfe, &m_fe[0]);
        break;

    case DATADIR_SET:
        *ppEnum = NULL;
        break;

    default:
        *ppEnum = NULL;
        break;
    }

    if (*ppEnum == NULL) return E_NOTIMPL;

    return S_OK;
}


//	________________________________________
//
//	Unimplemented methods 

STDMETHODIMP CDataObject::DAdvise (
		LPFORMATETC		pFE, 
		DWORD 			dwFlags, 
		LPADVISESINK 	pIAdviseSink, 
		LPDWORD 		pdwConn) 
{
    return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP CDataObject::DUnadvise (DWORD dwConn) {
    return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP CDataObject::EnumDAdvise (LPENUMSTATDATA *ppEnum) {
    return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP CDataObject::SetData (LPFORMATETC pFE, LPSTGMEDIUM pSTM, 
								   BOOL fRelease) {
    return E_NOTIMPL;
}

STDMETHODIMP CDataObject::GetCanonicalFormatEtc (LPFORMATETC pFEIn, 
												 LPFORMATETC pFEOut) {
	pFEOut->ptd = NULL;
    return E_NOTIMPL;
}


//	________________________________________
//
//	get format used to render data  

BOOL CDataObject::OKToDelete (void) 
{
	if ((m_uFmtUsed == CF_TEXT) ||
		(m_uFmtUsed == m_fe[2].cfFormat))
		return FALSE;
	else 
		return TRUE;
}


//	________________________________________
//
//	Interface to C code 

extern "C" {
LPDATAOBJECT KMCreateDataObject (
		VOID* 	pKeyMan, 
		LPSTR 	szName) 
{
   return ((LPDATAOBJECT) new CDataObject (pKeyMan, szName));
}

BOOL KMOKToDeleteDataObject (CDataObject* pDataObject) 
{
	return (pDataObject->OKToDelete ());
}

}