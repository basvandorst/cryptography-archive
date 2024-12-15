/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	$Id: GMIDropTarget.h,v 1.5 1998/08/11 15:20:01 pbj Exp $
____________________________________________________________________________*/

class FAR CDropTarget : public IDropTarget
{
public:    
    CDropTarget(HWND hwnd, VOID* pGroupMan);
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
    ULONG m_fmtDrop;  
    BOOL m_bAcceptFmt;
	BOOL m_bEnabled;
	HWND m_hwnd;
	VOID* m_pGroupMan;
};  

