/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//
//	$Id: KMIDataObject.h,v 1.2 1997/05/09 19:29:14 pbj Exp $
//
class FAR CDataObject : public IDataObject
{
public:
    CDataObject(LPSTR, LPSTR);

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    /* IDataObject methods */
	/* NB: these are the subset of IDataObject methods required for D'n'D */
	STDMETHODIMP GetData (LPFORMATETC, LPSTGMEDIUM);
    STDMETHODIMP GetDataHere (LPFORMATETC, LPSTGMEDIUM);
    STDMETHODIMP QueryGetData (LPFORMATETC);
    STDMETHODIMP EnumFormatEtc (DWORD, LPENUMFORMATETC *);

	/*  IDataObject methods not required for Drag and Drop */
    STDMETHODIMP GetCanonicalFormatEtc (LPFORMATETC,LPFORMATETC);
    STDMETHODIMP SetData (LPFORMATETC, LPSTGMEDIUM, BOOL);
    STDMETHODIMP DAdvise (LPFORMATETC, DWORD,  LPADVISESINK, DWORD *);
    STDMETHODIMP DUnadvise (DWORD);
    STDMETHODIMP EnumDAdvise (LPENUMSTATDATA *);

private:
	LPSTR m_szName;
	LPSTR m_szExport;
    ULONG m_refs;
	ULONG m_cfe;			// number of FORMATETC structs
	FORMATETC m_fe[3];		// pointer to FORMATETC structs
};
