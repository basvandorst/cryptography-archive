/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMIDropTarget.cpp - implements OLE IDropTarget for drag and drop

	$Id: KMIDropTarget.h,v 1.5 1998/08/11 14:43:28 pbj Exp $
____________________________________________________________________________*/

class FAR CDropTarget : public IDropTarget
{
public:    
    CDropTarget(HWND hwnd, VOID* pKeyMan, VOID* pSplitStruct);
	void Enable(BOOL bEnable);

    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    // IDropTarget methods
    STDMETHOD(DragEnter)(LPDATAOBJECT pDataObj, DWORD grfKeyState, 
		POINTL pt, LPDWORD pdwEffect);
    STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
    STDMETHOD(DragLeave)();
    STDMETHOD(Drop)(LPDATAOBJECT pDataObj, DWORD grfKeyState, 
		POINTL pt, LPDWORD pdwEffect); 
    
private:
    ULONG m_refs;  
	HWND m_hwnd;
    BOOL m_bAcceptFmt;
	BOOL m_bEnabled;
	VOID* m_pKeyMan;
	VOID* m_pSplitStruct;
	POINT m_ptPrevious;
	ULONG m_uDEPrevious;
};  

