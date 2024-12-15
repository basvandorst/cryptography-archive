//
//	$Id: KMIEnumFormatEtc.h,v 1.1 1997/06/16 14:55:56 pbj Exp $
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

