/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMIDropSource.cpp - implements OLE IDropSource for drag and drop

	$Id: KMIDropSource.h,v 1.6 1998/08/11 14:43:27 pbj Exp $
____________________________________________________________________________*/

class FAR CDropSource : public IDropSource
{
public:    
    CDropSource(VOID* pKeyMan, HWND hwnd, HWND hwndTree);
	~CDropSource ();

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    /* IDropSource methods */
    STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed, DWORD grfKeyState);
    STDMETHOD(GiveFeedback)(DWORD dwEffect);
 
private:
    ULONG		m_refs;
	HWND		m_hwnd;
	ULONG		m_ncyoffset;
	ULONG		m_ncxoffset;
	VOID*		m_pKeyMan;
};  

