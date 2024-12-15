/*
 * Copyright (C) 1995, 1996, 1997 Pretty Good Privacy, Inc.
 * All rights reserved.
 *
 * $Id$
 */

// pgpwctx.hpp		Context Menu Handler header file

//
// The class ID of this Shell extension class.
//
// class id:  969223c0-26aa-11d0-90ee-444553540000
//
//
// NOTE!!!  If you use this shell extension as a starting point,
//          you MUST change the GUID below.  Simply run UUIDGEN.EXE
//          to generate a new GUID.
//

//#define ODS(sz) OutputDebugString(sz)
//#define ODS(sz) MessageBox(GetFocus(), sz, "PGP Shell Extension", MB_OK)
#ifdef _DEBUG
void ODS(LPCSTR msg);
#else
#define ODS(sz)
#endif

#ifndef _PGPWCTX_HPP
#define _PGPWCTX_HPP

// this class factory object creates context menu handlers for Windows 95 shell
class CPGPClassFactory : public IClassFactory
{
protected:
	ULONG	m_cRef;

public:
	CPGPClassFactory();
	~CPGPClassFactory();

	//IUnknown members
	STDMETHODIMP			QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	//IClassFactory members
	STDMETHODIMP		CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
	STDMETHODIMP		LockServer(BOOL);

};
typedef CPGPClassFactory *LPCPGPCLASSFACTORY;

// this is the actual OLE Shell context menu handler
class CShellExt : public IContextMenu,
                         IShellExtInit
{
public:

protected:
	ULONG        m_cRef;
	LPDATAOBJECT m_pDataObj;
    STGMEDIUM	 medium;
    char         m_szFileUserClickedOn[MAX_PATH];

	HMENU		 hParentMenu;
	HMENU		 hSubMenu;
	UINT		 SubMenuItems;
	char		 SubMenuType[64];
	void CreateEncryptedMenu(UINT idCmdFirst);
	void CreateKeyfileMenu(UINT idCmdFirst);
	void CreateViewKeyringMenu(UINT idCmdFirst);
	void CreatePlaintextMenu(UINT idCmdFirst);
	void CreateVerifyMenu(UINT idCmdFirst);

public:
	CShellExt();
	~CShellExt();

	//IUnknown members
	STDMETHODIMP			QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	//IShell members
	STDMETHODIMP			QueryContextMenu(HMENU hMenu,
	                                         UINT indexMenu,
	                                         UINT idCmdFirst,
                                             UINT idCmdLast,
                                             UINT uFlags);

	STDMETHODIMP			InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi);

	STDMETHODIMP			GetCommandString(UINT idCmd,
	                                         UINT uFlags,
	                                         UINT FAR *reserved,
                                             LPSTR pszName,
                                             UINT cchMax);

	//IShellExtInit methods
	STDMETHODIMP		    Initialize(LPCITEMIDLIST pIDFolder,
	                                   LPDATAOBJECT pDataObj,
	                                   HKEY hKeyID);

private:
	BOOL AddMenuSeparator(HMENU hMenu, int iPosition);
};
typedef CShellExt *LPCSHELLEXT;

#endif // _PGPWCTX_HPP
