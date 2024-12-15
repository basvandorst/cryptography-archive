//////////////////////////////////////////////////////////////////////////////
// CShellExt.h
//
// Declarations for CShellExt.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: CShellExt.h,v 1.3 1998/12/14 19:00:44 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CShellExt_h // [
#define Included_CShellExt_h

#include <Shlobj.h>
#include "DualErr.h"


////////
// Types
////////

// A TargetInfo holds info about the specified shell target.

typedef struct TargetInfo
{
	CString		path;

	PGPBoolean	isVolume;
	PGPBoolean	isPGPdisk;
	PGPBoolean	isMountedPGPdisk;

} TargetInfo;


//////////////////
// Class CShellExt
//////////////////

// This is the actual OLE Shell context menu handler.

class CShellExt : public IContextMenu, IShellExtInit
{
public:
							CShellExt();
							~CShellExt();

	// IUnknown members.
	STDMETHODIMP			QueryInterface(REFIID riid, LPVOID FAR *ppv);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	// IShell members.
	STDMETHODIMP			QueryContextMenu(HMENU hMenu, UINT indexMenu, 
								UINT idCmdFirst, UINT idCmdLast, UINT uFlags);

	STDMETHODIMP			InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi);
	STDMETHODIMP			GetCommandString(UINT idCmd, UINT uFlags, 
								UINT FAR *reserved, LPSTR pszName, 
								UINT cchMax);

	// IShellExtInit methods.
	STDMETHODIMP			Initialize(LPCITEMIDLIST pIDFolder, 
								LPDATAOBJECT pDataObj, HKEY hKeyID);

	// IExtractIcon methods.
	STDMETHODIMP			GetIconLocation(UINT uFlags, LPSTR szIconFile, 
								UINT cchMax, int *piIndex, UINT *pwFlags);

	STDMETHODIMP			Extract(LPCSTR pszFile, UINT nIconIndex, 
								HICON *phiconLarge, HICON *phiconSmall, 
								UINT nIconSize);

protected:
	LPDATAOBJECT	mPDataObj;				// data object passed to us
	PGPUInt32		mRefCount;				// ref count for the object

	CMenu			mParentMenu;			// parent menu of PGPdisk menu
	CMenu			mSubMenu;				// the PGPdisk menu

	CString			mCommandLine;			// command line
	TargetInfo		*mTargets;				// array of targets

	PGPUInt32		mNumDropped;			// number of targets
	PGPUInt32		mNumTotalPGPdisks;		// # that are PGPdisks
	PGPUInt32		mNumMountedPGPdisks;	// # that are mounted PGPdisks

	PGPBoolean		mShiftKeyDown;			// shift key down?

	void			DeletePathsAndCommandLine();
	DualErr			ProcessTarget(HDROP hDrop, PGPUInt32 i);
	DualErr			PreparePathsAndCommandLine();

	void			SendRequestToApp(LPCSTR request, 
						PGPBoolean isNonTargeted = FALSE);
};

typedef CShellExt *LPCSHELLEXT;

#endif	// ] Included_CShellExt_h
