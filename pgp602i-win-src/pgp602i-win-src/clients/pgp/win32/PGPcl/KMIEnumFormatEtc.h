/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMIEnumFormatEtc.cpp - implements OLE IEnumFORMATETC for drag and drop
	to the shell.

	$Id: KMIEnumFormatEtc.h,v 1.3 1998/08/11 14:43:30 pbj Exp $
____________________________________________________________________________*/

class FAR CEnumFormatEtc : public IEnumFORMATETC {
public:    
    CEnumFormatEtc (ULONG cFE, LPFORMATETC prgFE);
	~CEnumFormatEtc (void);

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    /* IEnumFORMATETC methods */
	STDMETHODIMP Next (ULONG, LPFORMATETC, ULONG *);
	STDMETHODIMP Skip (ULONG);
	STDMETHODIMP Reset (void);
	STDMETHODIMP Clone (IEnumFORMATETC **);

private:
    ULONG			m_refs;     
    ULONG           m_iCur;         //Current element.
    ULONG           m_cfe;          //Number of FORMATETCs in us
    LPFORMATETC     m_prgfe;        //Source of FORMATETCs
};  

