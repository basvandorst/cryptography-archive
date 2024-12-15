/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMIEnumFormatEtc.cpp - implements OLE IEnumFORMATETC for drag and drop
	to the shell.

	$Id: KMIEnumFormatEtc.cpp,v 1.4 1998/08/11 14:43:29 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// system header files
#include <windows.h>

// project header files
#include "KMIEnumFormatEtc.h"


//	________________________________________
//
//	IUnknown Methods

STDMETHODIMP CEnumFormatEtc::QueryInterface (
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


STDMETHODIMP_(ULONG) CEnumFormatEtc::AddRef (void) 
{
    return ++m_refs;
}


STDMETHODIMP_(ULONG) CEnumFormatEtc::Release (void) 
{
	if(--m_refs == 0) {
		if (m_prgfe != NULL) delete [] m_prgfe;	
		m_prgfe = NULL;							
		delete this;						
		return 0;
    }
    return m_refs;
}  


//	________________________________________
//
//	CEnumFormatEtc Constructor
//		cFE             ULONG number of FORMATETCs in pFE
//		prgFE           LPFORMATETC to the array to enumerate.
// 

CEnumFormatEtc::CEnumFormatEtc (
		ULONG		cFE, 
		LPFORMATETC	prgFE) 
{
    UINT i;

    m_refs = 1;

    m_iCur = 0;
    m_cfe = cFE;
    m_prgfe = new FORMATETC[(UINT)cFE];

    if (m_prgfe != NULL) {
        for (i = 0; i < cFE; i++)
            m_prgfe[i] = prgFE[i];
    }

    return;
}


//	________________________________________
//
//	CEnumFormatEtc Destructor

CEnumFormatEtc::~CEnumFormatEtc(void) 
{
    if (m_prgfe != NULL)
        delete [] m_prgfe;
    return;
}


//	________________________________________
//
//	IEnumFORMATETC Methods


//	________________________________________
//
//	CEnumFormatEtc::Next
//
//	Purpose:
//		Returns the next element in the enumeration.
//
//	Parameters:
//		cFE             ULONG number of FORMATETCs to return.
//		pFE             LPFORMATETC in which to store the returned
//					    structures.
//		pulFE           ULONG * in which to return how many we
//						enumerated.
// 
//	Return Value:
//		HRESULT         NOERROR if successful, S_FALSE otherwise,
// 

STDMETHODIMP CEnumFormatEtc::Next (
		ULONG		cFE, 
		LPFORMATETC	pFE, 
		ULONG*		pulFE) 
{
    ULONG               cReturn=0L;

    if (m_prgfe == NULL)
        return S_FALSE;

    if (pulFE == NULL) {
        if (cFE != 1L) return S_FALSE;
    }
    else *pulFE=0L;

    if ((pFE == NULL) || (m_iCur >= m_cfe))
        return S_FALSE;

    while ((m_iCur < m_cfe) && (cFE > 0)) {
        *pFE++=m_prgfe[m_iCur++];
        cReturn++;
        cFE--;
    }

    if (pulFE != NULL) *pulFE=cReturn;

    return S_OK;
}


//	________________________________________
//
//	CEnumFormatEtc::Skip
//
//	Purpose:
//		Skips the next n elements in the enumeration.
//
//	Parameters:
//		cSkip           ULONG number of elements to skip.
//
//	Return Value:
//		HRESULT         NOERROR if successful, S_FALSE if we could not
//						skip the requested number.
//

STDMETHODIMP CEnumFormatEtc::Skip (ULONG cSkip) 
{
    if (((m_iCur+cSkip) >= m_cfe) || (m_prgfe == NULL))
        return S_FALSE;

    m_iCur+=cSkip;
    return S_OK;
}


//	________________________________________
//
//	CEnumFormatEtc::Reset
//
//	Purpose:
//		Resets the current element index in the enumeration to zero.
//
//	Parameters:
//		None
//
//	Return Value:
//		HRESULT         NOERROR
//

STDMETHODIMP CEnumFormatEtc::Reset (void) 
{
    m_iCur=0;
    return S_OK;
}


//	________________________________________
//
//	CEnumFormatEtc::Clone
//
//	Purpose:
//		Returns another IEnumFORMATETC with the same state as ourselves.
//
//	Parameters:
//		ppEnum          LPENUMFORMATETC * in which to return the
//						new object.
//
//	Return Value:
//		HRESULT         NOERROR or a general error value.
//

STDMETHODIMP CEnumFormatEtc::Clone (LPENUMFORMATETC *ppEnum) 
{
    CEnumFormatEtc*     pNew;

    *ppEnum=NULL;

    //Create the clone
    pNew = new CEnumFormatEtc (m_cfe, m_prgfe);

    if (pNew == NULL) return E_OUTOFMEMORY;

    pNew->m_iCur = m_iCur;

    *ppEnum = pNew;
    return S_OK;
}


