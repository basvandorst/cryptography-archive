/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//
//	$Id: KMIEnumFormatEtc.h,v 1.2 1997/05/09 19:29:17 pbj Exp $
//
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
