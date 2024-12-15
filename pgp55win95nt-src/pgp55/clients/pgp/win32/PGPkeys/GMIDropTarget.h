//
//	$Id: GMIDropTarget.h,v 1.2 1997/09/10 15:33:26 pbj Exp $
//
class FAR CDropTarget : public IDropTarget
{
public:    
    CDropTarget(VOID* pGroupMan);
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
	VOID* m_pGroupMan;
};  
