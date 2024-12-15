//
//	$Id: KMIDropTarget.h,v 1.1 1997/06/16 14:55:56 pbj Exp $
//
class FAR CDropTarget : public IDropTarget
{
public:    
    CDropTarget(VOID* pKeyMan);
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
	VOID* m_pKeyMan;
};  

