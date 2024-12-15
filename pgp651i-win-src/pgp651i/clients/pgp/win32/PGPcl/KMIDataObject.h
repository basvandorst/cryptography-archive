/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMIDataObject.h - implements OLE IDataObject for drag and drop
	to the shell.

	$Id: KMIDataObject.h,v 1.5 1998/08/13 02:46:08 pbj Exp $
____________________________________________________________________________*/

#define CFSTR_PGPCOMPLETEFORMAT		("PGP_COMPLETE60KEYFORMAT")

class FAR CDataObject : public IDataObject
{
public:    
    CDataObject(LPVOID, LPSTR);
	~CDataObject ();
	BOOL OKToDelete (void);

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
	LPSTR m_szData;
    ULONG m_refs;  
	ULONG m_cfe;			// number of FORMATETC structs
	FORMATETC m_fe[4];		// pointer to FORMATETC structs
	UINT  m_uFmtUsed;		// format used to actually render object
	VOID* m_pKeyMan;
};  

