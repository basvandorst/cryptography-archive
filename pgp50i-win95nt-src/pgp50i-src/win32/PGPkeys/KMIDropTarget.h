/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//
//	$Id: KMIDropTarget.h,v 1.2 1997/05/09 19:29:16 pbj Exp $
//
class FAR CDropTarget : public IDropTarget
{
public:
    CDropTarget();
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
    BOOL m_bAcceptFmt;
	BOOL m_bEnabled;
};
