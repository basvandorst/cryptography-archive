//////////////////////////////////////////////////////////////////////////////
// CShellExt.cpp
//
// Definition of class CShellExt.
//////////////////////////////////////////////////////////////////////////////

// $Id: CShellExt.cpp,v 1.3 1998/12/14 19:00:43 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "DriverComm.h"
#include "DualErr.h"
#include "PGPdiskRegistry.h"
#include "StringAssociation.h"
#include "UtilityFunctions.h"
#include "Win32Utils.h"
#include "WindowsVersion.h"

#include "CShellExt.h"
#include "Globals.h"


////////////
// Constants
////////////

static LPCSTR	kPGPdiskShellExtSendToStringNT	= "Send &To";
static LPCSTR	kPGPdiskShellExtSendToStringWin95	= "Se&nd To";


/////////
// Macros
/////////

#define ResultFromShort(i) ResultFromScode(MAKE_SCODE(SEVERITY_SUCCESS, \
							0, (USHORT)(i)))


//////////////////////////////////////////
// Class CShellExt public member functions
//////////////////////////////////////////

// The CShellExt default constructor.

CShellExt::CShellExt()
{
	mPDataObj	= NULL;
	mRefCount	= 0;

	mTargets	= NULL;
	mNumDropped	= 0;

	mNumTotalPGPdisks	= 0;
	mNumMountedPGPdisks	= 0;

	DLL->mGCRefThisDll++;
}

// The CShellExt destructor.

CShellExt::~CShellExt()
{
	DeletePathsAndCommandLine();

	DLL->mGCRefThisDll--;
}

// QueryInterface returns a pointer to this class object's interface.

STDMETHODIMP 
CShellExt::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	(* ppv) = NULL;

	if (IsEqualIID(riid, IID_IShellExtInit) || 
		IsEqualIID(riid, IID_IUnknown))
	{
		(* ppv) = (LPSHELLEXTINIT) this;
	}
	else if (IsEqualIID(riid, IID_IContextMenu))
	{
		(* ppv) = (LPCONTEXTMENU) this;
	}

	if (* ppv)
	{
		AddRef();
		return NOERROR;
	}
	else
	{
		return E_NOINTERFACE;
	}
}

// QueryContextMenu is called to construct our context menu.

STDMETHODIMP 
CShellExt::QueryContextMenu(
	HMENU	hMenu, 
	UINT	indexMenu, 
	UINT	idCmdFirst, 					
	UINT	idCmdLast, 
	UINT	uFlags)
{
	DualErr		derr;
	PGPBoolean	hasTimedOut, success;
	PGPUInt32	itemPosition, menuId, subMenuPosition;
	
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	menuId = idCmdFirst;
	itemPosition = 0;

	hasTimedOut = FALSE;

	// Shift key down?
	mShiftKeyDown = (::GetAsyncKeyState(VK_SHIFT) & 0x8000 ? TRUE : FALSE);

	// Beta or demo timed out?
	#if PGPDISK_BETAVERSION
		hasTimedOut = HasBetaTimedOut();
	#elif PGPDISK_DEMOVERSION
		hasTimedOut = HasDemoTimedOut();
	#endif // PGPDISK_BETAVERSION

	// Only show a menu if we found PGPdisks.
	if (mNumTotalPGPdisks == 0)
		derr = DualErr(kPGDMinorError_FailSilently);

	// There are also other special cases in which we don't show the menu.
	if (derr.IsntError())
	{
		if (((uFlags & 0x000F) != CMF_NORMAL) && 
			!(uFlags & CMF_EXPLORE) && !(uFlags & CMF_VERBSONLY))
		{
			derr = DualErr(kPGDMinorError_FailSilently);
		}
	}

	// Save parent menu handle for later.
	if (derr.IsntError())
	{
		if (!mParentMenu.Attach(hMenu))
			derr = DualErr(kPGDMinorError_AttachMenuFailed);
	}

	// Create the submenu.
	if (derr.IsntError())
	{
		if (!mSubMenu.CreatePopupMenu())
			derr = DualErr(kPGDMinorError_CreatePopupMenuFailed);
	}

	// Add items to the menu.
	if (derr.IsntError())
	{
		PGPUInt32 flags, sendToPosition;

		flags = MF_BYPOSITION;

		// Insert open/mount/unmount item.
		if (mNumTotalPGPdisks > 1)
		{
			mSubMenu.InsertMenu(itemPosition++, flags, menuId++, 
				GetCommonString(kPGPdiskShellExtOpenMenuString));

		}
		else if (mNumMountedPGPdisks == 1)
		{
			mSubMenu.InsertMenu(itemPosition++, flags, menuId++, 
				GetCommonString(kPGPdiskShellExtUnmountMenuString));
		}
		else
		{
			mSubMenu.InsertMenu(itemPosition++, flags, menuId++, 
				GetCommonString(kPGPdiskShellExtMountMenuString));
		}

		// Insert separator.
		mSubMenu.InsertMenu(itemPosition++, flags | MF_SEPARATOR, 0);

		// Insert passphrase items.
		flags = MF_BYPOSITION | (mNumMountedPGPdisks > 0 ? MF_GRAYED : NULL);

		mSubMenu.InsertMenu(itemPosition++, flags, menuId++, 
			GetCommonString(kPGPdiskShellExtAddPassMenuString));

		mSubMenu.InsertMenu(itemPosition++, flags, menuId++, 
			GetCommonString(kPGPdiskShellExtChangePassMenuString));

		if (mShiftKeyDown)
		{
			mSubMenu.InsertMenu(itemPosition++, flags, menuId++, 
				GetCommonString(kPGPdiskShellExtRemoveAltsMenuString));
		}
		else
		{
			mSubMenu.InsertMenu(itemPosition++, flags, menuId++, 
				GetCommonString(kPGPdiskShellExtRemovePassMenuString));
		}

		mSubMenu.InsertMenu(itemPosition++, flags, menuId++, 
			GetCommonString(kPGPdiskShellExtPubKeysMenuString));

		// Insert separator.
		flags = MF_BYPOSITION | MF_SEPARATOR;
		mSubMenu.InsertMenu(itemPosition++, flags | MF_SEPARATOR, 0);

		// Insert help item.
		flags = MF_BYPOSITION;
		mSubMenu.InsertMenu(itemPosition++, flags, menuId++, 
			GetCommonString(kPGPdiskShellExtHelpMenuString));

		// Find position of "Send To".
		if (IsWin95CompatibleMachine())
		{
			sendToPosition = FindMenuItemPosition(&mParentMenu, 
				kPGPdiskShellExtSendToStringWin95);
		}
		else
		{
			sendToPosition = FindMenuItemPosition(&mParentMenu, 
				kPGPdiskShellExtSendToStringNT);
		}

		if (sendToPosition == -1)
			subMenuPosition = indexMenu;
		else
			subMenuPosition = sendToPosition + 1;

		// Insert another separator.
		mParentMenu.InsertMenu(subMenuPosition++, 
			MF_BYPOSITION | MF_SEPARATOR, 0);

		// Insert the submenu menu.
		success = mParentMenu.InsertMenu(subMenuPosition, 
			MF_BYPOSITION | MF_POPUP, (PGPUInt32) mSubMenu.m_hMenu, 
			"&PGPdisk");

		if (!success)
			derr = DualErr(kPGDMinorError_InsertMenuFailed);
	}

	return ResultFromShort(itemPosition);
}

// InvokeCommand is called after the user has selected a menu item.

STDMETHODIMP 
CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
	HRESULT hr = E_INVALIDARG;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	try
	{
		// If HIWORD(lpcmi->lpVerb) then we have been called programmatically
		// and lpVerb is a command that should be invoked. Otherwise, the
		// shell has called us, and LOWORD(lpcmi->lpVerb) is the menu ID the
		// user has selected. Actually, it's (menu ID - idCmdFirst) from
		// QueryContextMenu().

		if (!HIWORD(lpcmi->lpVerb))
		{
			CString		command;
			PGPBoolean	isNonTargeted, sendRequest;
			UINT		idCmd	= LOWORD(lpcmi->lpVerb);

			hr = NOERROR;

			isNonTargeted	= FALSE;
			sendRequest		= TRUE;

			switch (idCmd)
			{
			case 0:
				if (mNumTotalPGPdisks > 1)
					command = GetCommonString(kPGPdiskOpenCmdString);
				else if (mNumMountedPGPdisks == 1)
					command = GetCommonString(kPGPdiskUnmountCmdString);
				else
					command = GetCommonString(kPGPdiskMountCmdString);
				break;

			case 1:
				command = GetCommonString(kPGPdiskAddPassCmdString);
				break;

			case 2:
				command = GetCommonString(kPGPdiskChangePassCmdString);
				break;

			case 3:
				if (mShiftKeyDown)
					command = GetCommonString(kPGPdiskRemoveAltsCmdString);
				else
					command = GetCommonString(kPGPdiskRemovePassCmdString);
				break;

			case 4:
				command = GetCommonString(kPGPdiskPubKeysCmdString);
				break;

			case 5:
				command = GetCommonString(kPGPdiskHelpCmdString);
				isNonTargeted = TRUE;
				break;

			default:
				hr = E_INVALIDARG;
				sendRequest = FALSE;
				break;
			}

			if (sendRequest)
				SendRequestToApp(command, isNonTargeted);
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}

	return hr;
}

// GetCommandString returns the help string associated with a command.

STDMETHODIMP 
CShellExt::GetCommandString(
	UINT		idCmd, 
	UINT		uFlags, 
	UINT FAR	*reserved, 
	LPSTR		pszName, 
	UINT		cchMax)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	try
	{
		CString helpString;

		switch (idCmd)
		{
		case 0:
			if (mNumTotalPGPdisks > 1)
			{
				helpString = 
					GetCommonString(kPGPdiskShellExtDescribeOpenString);
			}
			else if (mNumMountedPGPdisks == 1)
			{
				helpString = 
					GetCommonString(kPGPdiskShellExtDescribeUnmountString);
			}
			else
			{
				helpString = 
					GetCommonString(kPGPdiskShellExtDescribeMountString);
			}
			break;

		case 1:
			helpString = 
				GetCommonString(kPGPdiskShellExtDescribeAddPassString);
			break;

		case 2:
			helpString = 
				GetCommonString(kPGPdiskShellExtDescribeChangePassString);
			break;

		case 3:
			if (mShiftKeyDown)
			{
				helpString = 
					GetCommonString(kPGPdiskShellExtDescribeRemoveAltsString);
			}
			else
			{
				helpString = 
					GetCommonString(kPGPdiskShellExtDescribeRemovePassString);
			}
			break;

		case 4:
			helpString = 
				GetCommonString(kPGPdiskShellExtDescribePubKeysString);
			break;

		case 5:
			helpString = 
				GetCommonString(kPGPdiskShellExtDescribeHelpString);
			break;
		}

		// Convert to Unicode under NT.
		if (IsWinNT4CompatibleMachine())
		{
			if (!(MultiByteToWideChar(CP_ACP, 0, helpString, -1, 
				(LPWSTR) pszName, cchMax / 2)))
			{
				((LPWSTR) pszName)[0] = NULL;
			}
		}
		else
		{
			SmartStringCopy(pszName, helpString, cchMax);
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}

	return NOERROR;
}

// AddRef increases the reference count for this class object.

STDMETHODIMP_(ULONG) 
CShellExt::AddRef()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return ++mRefCount;
}

// Release decreases the reference count for this class object.

STDMETHODIMP_(ULONG) 
CShellExt::Release()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (--mRefCount)
		return mRefCount;

    if (mPDataObj)
        mPDataObj->Release();

	delete this;

	return 0;
}

// Initialize is called before initializing the context menu.

STDMETHODIMP 
CShellExt::Initialize(
	LPCITEMIDLIST	pIDFolder, 
	LPDATAOBJECT	pDataObj, 
	HKEY			hRegKey)
{
	DualErr derr;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Initialize can be called more than once.
	if (mPDataObj)
		mPDataObj->Release();

	// Duplicate the object pointer and registry handle, and extract the path
	// of the file or drive being operated on.

	if (IsntNull(pDataObj))
	{
		mPDataObj = pDataObj;
		pDataObj->AddRef();

		derr = PreparePathsAndCommandLine();
	}

	if (derr.IsntError())
		return NOERROR;
	else
		return E_OUTOFMEMORY;
}


/////////////////////////////////////////////
// Class CShellExt protected member functions
/////////////////////////////////////////////

// DeletePathsAndCommandLine releases memory allocated by
// PreparePathsAndCommandLine.

void 
CShellExt::DeletePathsAndCommandLine()
{
	if (IsntNull(mTargets))
	{
		delete[] mTargets;
		mTargets = NULL;
	}
}

// ProcessTarget is a helper function for PreparePathsAndCommandLine.

DualErr 
CShellExt::ProcessTarget(HDROP hDrop, PGPUInt32 i)
{
	DualErr derr;

	pgpAssert(IsntNull(hDrop));
	pgpAssert(i <= mNumDropped);

	try
	{
		char		ext[_MAX_EXT];
		PGPBoolean	isAPGPdisk;

		mTargets[i].isVolume			= FALSE;
		mTargets[i].isPGPdisk			= FALSE;
		mTargets[i].isMountedPGPdisk	= FALSE;

		// Get the path.
		DragQueryFile((HDROP) hDrop, i, 
			mTargets[i].path.GetBuffer(kMaxStringSize), kMaxStringSize);

		mTargets[i].path.ReleaseBuffer();

		// Is this a volume being pointed at?
		mTargets[i].isVolume = IsPlainLocalRoot(mTargets[i].path);

		// Check if it is a mounted PGPdisk.
		if (mTargets[i].isVolume)
		{
			PGPUInt8 drive;

			// It's a volume, but is a PGPdisk volume (mounted)?
			drive = DriveLetToNum(mTargets[i].path[0]);
			derr = IsVolumeAPGPdisk(drive, &isAPGPdisk);

			if (derr.IsntError())
			{
				if (isAPGPdisk)
				{
					mNumTotalPGPdisks++;
					mNumMountedPGPdisks++;

					mTargets[i].isPGPdisk			= TRUE;
					mTargets[i].isMountedPGPdisk	= TRUE;
				}
			}
		}
		else
		{
			// It's a file, but is it a PGPdisk file (mounted or unmounted)?
			_splitpath(mTargets[i].path, NULL, NULL, NULL, ext);
			mTargets[i].isPGPdisk = !_stricmp(ext, kPGPdiskFileExtension);

			if (mTargets[i].isPGPdisk)
			{
				PGPBoolean isLoopedBack;

				mNumTotalPGPdisks++;

				// Resolve looped UNC paths.
				if (IsUNCPath(mTargets[i].path))
				{
					derr = CheckIfLoopedBack(mTargets[i].path, 
						&mTargets[i].path, &isLoopedBack);
				}

				// Is this a mounted PGPdisk?
				derr = IsFileAPGPdisk(mTargets[i].path, &isAPGPdisk);

				if (derr.IsntError())
				{
					mTargets[i].isMountedPGPdisk = isAPGPdisk;

					if (isAPGPdisk)
						mNumMountedPGPdisks++;
				}
			}
		}

	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	return derr;
}

// PreparePathsAndCommandLine prepares the list of targets and space for the
// command line for the app.

DualErr 
CShellExt::PreparePathsAndCommandLine()
{
	DualErr		derr;
	PGPBoolean	createdTargets	= FALSE;

	try
	{
		FORMATETC	fmte = {CF_HDROP, (DVTARGETDEVICE FAR *) NULL, 
			DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		PGPBoolean	createdTargets	= FALSE;
		PGPUInt32	i;
		STGMEDIUM	medium;

		mPDataObj->GetData(&fmte, &medium);

		// Get the number of targets.
		mNumDropped = DragQueryFile((HDROP) medium.hGlobal, 0xFFFFFFFF, NULL, 
			NULL);

		if (mNumDropped == 0)
			derr = DualErr(kPGDMinorError_FailSilently);

		if (derr.IsntError())
		{
			mNumTotalPGPdisks = mNumMountedPGPdisks = 0;

			// Get space for the target array.
			mTargets = new TargetInfo[mNumDropped];
			createdTargets = TRUE;

			// Get info about each of them.
			for (i = 0; i < mNumDropped; i++)
			{
				derr = ProcessTarget((HDROP) medium.hGlobal, i);

				if (derr.IsError())
					break;
			}
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	// Cleanup if error.
	if (derr.IsError())
	{
		if (createdTargets)
		{
			delete[] mTargets;

			mTargets = NULL;
		}

		mNumDropped = mNumTotalPGPdisks = mNumMountedPGPdisks = 0;
	}

	return derr;
}

// SendRequestToApp asks the PGPdisk application to perform a command on the
// PGPdisk targets. A "non-targeted" command does not take a pathname
// argument.

void 
CShellExt::SendRequestToApp(LPCSTR command, PGPBoolean isNonTargeted)
{
	try
	{
		CString		appName, temp;
		PGPUInt32	result;

		pgpAssertStrValid(command);

		// For each PGPdisk target, add a command to the command line
		// we are building.

		if (isNonTargeted)
		{
			mCommandLine += command;
		}
		else
		{
			for (PGPUInt32 i=0; i < mNumDropped; i++)
			{
				if (mTargets[i].isPGPdisk)
				{
					PGPInt32 length;
					
					length = mTargets[i].path.GetLength();

					// If slash on end, double it.
					if (mTargets[i].path[length - 1] == '\\')
						mTargets[i].path += "\\";

					temp.Format(" %s \"%s\"", command, mTargets[i].path);
					mCommandLine += temp;
				}
			}
		}

		// Execute the command line.
		appName = kPGPdiskAppName;
		appName += ".exe";

		result = (PGPUInt32) ShellExecute(NULL, "open", appName, 
			mCommandLine, NULL, SW_SHOWNORMAL);

		if (result < 32)
			ReportError(kPGDMajorError_CouldntFindAppString);
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}
