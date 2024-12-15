//////////////////////////////////////////////////////////////////////////////
// Win32Utils.cpp
//
// Common Win32 utils.
//////////////////////////////////////////////////////////////////////////////

// $Id: Win32Utils.cpp,v 1.1.2.23.2.4 1998/10/23 02:19:00 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"
#include "AfxPriv.h"

#include <Objbase.h>
#include <Shlobj.h>
#include <Dbt.h>

#else
#error Define PGPDISK_MFC.
#endif	// PGPDISK_MFC

#include "Required.h"
#include "LinkResolution.h"
#include "PGPdiskRegistry.h"
#include "StringAssociation.h"
#include "UtilityFunctions.h"
#include "Win32Utils.h"
#include "WindowsVersion.h"


////////////
// Constants
////////////

const char kInvalidCharReplacement = '_';


//////////
// Globals
//////////

static CWnd *MessageBoxParent;


////////////////////////
// Message box functions
////////////////////////

// RegisterPGPdiskMsgBoxParent registers a window to be used as a message-box
// parent.

void 
RegisterPGPdiskMsgBoxParent(CWnd *pWnd)
{
	MessageBoxParent = pWnd;
}

// GetLastPGPdiskWindow returns the frontmost PGPdisk window.

CWnd * 
GetLastPGPdiskWindow()
{
	if (MessageBoxParent->GetSafeHwnd())
		return MessageBoxParent->GetLastActivePopup();
	else
		return NULL;
}

// PGPdiskMessageBox shows a message box with the specified options. It
// returns the button the user pressed.

UserResponse 
PGPdiskMessageBox(
	LPCSTR				message, 
	CWnd				*pParent, 
	LPCSTR				title, 
	PGDMessageBoxStyle	style, 
	PGDMessageBoxFocus	focus)
{
	MSGBOXPARAMS	mbParams;
	PGPUInt32		button, defaultButton, flags;

	pgpAssertStrValid(message);
	pgpAssertStrValid(title);

	mbParams.cbSize = sizeof(MSGBOXPARAMS);

	mbParams.hwndOwner			= pParent->GetSafeHwnd();
	mbParams.hInstance			= NULL;
	mbParams.lpszText			= message;
	mbParams.lpszCaption		= title;
	mbParams.lpszIcon			= NULL;
	mbParams.dwContextHelpId	= NULL;
	mbParams.lpfnMsgBoxCallback	= NULL;
	mbParams.dwLanguageId		= MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

	flags = MB_SETFOREGROUND;

	// If main window on top, always on top.
	if (PGPdiskGetPrefDWord(kRegistryMainStayOnTopKey, MF_UNCHECKED) == 
		MF_CHECKED)
	{
		flags |= MB_TOPMOST;
	}

	switch (style)
	{
	case kPMBS_Ok:
		flags |= MB_OK | MB_ICONERROR;

		mbParams.dwStyle = flags;

		button = MessageBoxIndirect(&mbParams);
		break;

	case kPMBS_OkCancel:
		switch (focus)
		{
		case kPMBF_OkButton:
			defaultButton = MB_DEFBUTTON1;
			break;

		case kPMBF_CancelButton:
			defaultButton = MB_DEFBUTTON2;
			break;

		default:
			pgpAssert(FALSE);
			break;
		}
		
		flags |= defaultButton | MB_OKCANCEL | MB_ICONERROR;

		mbParams.dwStyle = flags;

		button = MessageBoxIndirect(&mbParams);
		break;

	case kPMBS_YesNo:
		switch (focus)
		{
		case kPMBF_YesButton:
			defaultButton = MB_DEFBUTTON1;
			break;

		case kPMBF_NoButton:
			defaultButton = MB_DEFBUTTON2;
			break;

		default:
			pgpAssert(FALSE);
			break;
		}

		flags |= defaultButton | MB_YESNO | MB_ICONEXCLAMATION;

		mbParams.dwStyle = flags;

		button = MessageBoxIndirect(&mbParams);
		break;

	case kPMBS_YesNoCancel:
		switch (focus)
		{
		case kPMBF_YesButton:
			defaultButton = MB_DEFBUTTON1;
			break;

		case kPMBF_NoButton:
			defaultButton = MB_DEFBUTTON2;
			break;

		case kPMBF_CancelButton:
			defaultButton = MB_DEFBUTTON3;
			break;

		default:
			pgpAssert(FALSE);
			break;
		}

		flags |= defaultButton | MB_YESNOCANCEL | MB_ICONEXCLAMATION;

		mbParams.dwStyle = flags;

		button = MessageBoxIndirect(&mbParams);
		break;

	default:
		pgpAssert(FALSE);
		break;
	}

	// Finally return the choice the user made in the message box.
	switch (button)
	{
	case IDYES:
		return kUR_Yes;
	case IDNO:
		return kUR_No;
	case IDOK:
		return kUR_OK;
	default:
		return kUR_Cancel;
	}
}

// ReportError reports an error to the user.

UserResponse 
ReportError(
	PGDMajorError		perr, 
	DualErr				derr, 
	PGPUInt8			drive, 
	PGDMessageBoxStyle	style, 
	PGDMessageBoxFocus	focus)
{
	UserResponse button = kUR_Cancel;

	// Don't display dialog if we fail silently or if user canceled.
	if ((derr.mMinorError == kPGDMinorError_FailSilently) ||
		(derr.mMinorError == kPGDMinorError_UserAbort))
	{
		return kUR_Cancel;
	}

	MessageBeep(MB_ICONHAND);

	try
	{
		CString errorString;

		// Get the error string.
		FormatErrorString(perr, derr, drive, 
			errorString.GetBuffer(kMaxStringSize), kMaxStringSize);

		errorString.ReleaseBuffer();

		// Show the error message.
		button = PGPdiskMessageBox(errorString, GetLastPGPdiskWindow(), 
			kPGPdiskErrorMsgBoxTitle, style, focus);
	}
	catch (CMemoryException *ex)
	{
		// Guaranteed to succeed with these flags.
		MessageBox(NULL, kPGPdiskErrorMsgBoxTitle, 
			GetCommonString(kPGPdiskNoMemForErrorString), 
			MB_ICONHAND | MB_SYSTEMMODAL);

		ex->Delete();
	}

	return button;
}

// DisplayMessage displays a message to the user.

UserResponse 
DisplayMessage(
	PGPUInt32			stringId, 
	PGDMessageBoxStyle	style, 
	PGDMessageBoxFocus	focus)
{
	UserResponse button = kUR_Cancel;

	// Show the message.
	button = PGPdiskMessageBox(GetCommonString(stringId), 
		GetLastPGPdiskWindow(), kPGPdiskNormalMsgBoxTitle, style, focus);

	return button;
}


////////////////
// GUI functions
////////////////

// FindMenuItemPosition returns the position of the menu item with the given
// text in the given menu, or -1 otherwise.

PGPInt32 
FindMenuItemPosition(CMenu *pMenu, LPCSTR menuString)
{
	PGPInt32 position = -1;

	try
	{
		CString		itemText;
		PGPInt32	i, numItems;

		pgpAssertAddrValid(pMenu, CMenu);
		pgpAssertStrValid(menuString);

		numItems = pMenu->GetMenuItemCount();

		if (numItems == -1)
			return -1;

		for (i = 0; i < numItems; i++)
		{
			pMenu->GetMenuString(i, itemText, MF_BYPOSITION);
			
			if (itemText.Compare(menuString) == 0)
			{
				position = i;
				break;
			}
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}

	return position;
}


////////////////////////////////
// Pathname Processing Functions
////////////////////////////////

// ConvertPathToLong converts a short pathname to a long pathname.

DualErr 
ConvertPathToLong(CString *path)
{
	DualErr derr;

	try
	{
		CString			longPath, piece, prefix, temp;
		HANDLE			findHandle;
		PGPUInt32		length;
		WIN32_FIND_DATA	findData;

		pgpAssertAddrValid(path, CString);

		length = path->GetLength();

		// Get the prefix.
		if (path->GetAt(length - 1) == '\\')
		{
			(* path) = path->Left(length - 1);
			length--;
		}

		derr = GetRoot((* path), &prefix);

		// Take out the prefix and save it for later.
		if (derr.IsntError())
		{
			(* path) = path->Right(length - prefix.GetLength());
		}

		// Convert each component of the pathname to its long form.
		while (derr.IsntError() && !path->IsEmpty())
		{
			temp = prefix + (* path);
			findHandle = FindFirstFile(temp, &findData);

			if (findHandle == INVALID_HANDLE_VALUE)
			{
				derr = DualErr(kPGDMinorError_FindFirstFileFailed, 
					GetLastError());
			}
			else
			{
				FindClose(findHandle);

				(* path) = path->Left(path->ReverseFind('\\'));

				piece = findData.cFileName;
				longPath = piece + '\\' + longPath;
			}
		}

		// Prepare the output.
		if (derr.IsntError())
		{
			longPath	= longPath.Left(longPath.GetLength() - 1);
			longPath	= prefix + longPath;

			(* path) = longPath;
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	return derr;
}

// GetRoot takes any legal path and returns a string of the form "C:\" or
// "\\UncVolumeName\share\".

DualErr 
GetRoot(LPCSTR path, CString *root)
{
	DualErr derr;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(root, CString);

	try
	{
		CString tempString;

		tempString = path;
		root->Empty();

		if (HasPlainLocalRoot(path))
		{
			(* root) = tempString.Left(3);
		}
		else if (IsUNCPath(path))
		{
			PGPInt32	i;
			PGPUInt32	numSlashes;

			for (i = 0, numSlashes = 0; i < tempString.GetLength(); i++)
			{
				if (path[i] == '\\')
					numSlashes++;

				if (numSlashes == 4)
				{
					(* root) = tempString.Left(i + 1);
					break;
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

// MakeRoot makes a root out of a drive number.

DualErr 
MakeRoot(PGPUInt8 drive, CString *root)
{
	DualErr derr;

	pgpAssert(IsLegalDriveNumber(drive));
	pgpAssertAddrValid(root, CString);

	try
	{
		root->Format("%c:\\", DriveNumToLet(drive));
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	return derr;
}

// StripRoot removes the root from a pathname.

DualErr 
StripRoot(LPCSTR path, CString *nonRoot)
{
	DualErr derr;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(nonRoot, CString);

	try
	{
		CString csInPath, root;

		csInPath = path;

		derr = GetRoot(path, &root);

		if (derr.IsntError())
		{
			(* nonRoot) = csInPath.Right(csInPath.GetLength() - 
				root.GetLength());
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	return derr;
}

// GetServer takes a UNC path and returns the server component.

DualErr 
GetServer(LPCSTR path, CString *server)
{
	DualErr derr;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(server, CString);

	try
	{
		CString tempString;

		server->Empty();

		if (IsUNCPath(path))
		{
			tempString = path + 2;
			(* server) = tempString.Left(tempString.Find('\\'));
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	return derr;
}

// GetShare takes a UNC path and returns the share component.

DualErr 
GetShare(LPCSTR path, CString *share)
{
	DualErr derr;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(share, CString);

	try
	{
		CString tempString;

		tempString = path;
		share->Empty();

		if (IsUNCPath(path))
		{
			PGPInt32	i;
			PGPUInt32	numSlashes;

			for (i = 0, numSlashes = 0; i < tempString.GetLength(); i++)
			{
				if (path[i] == '\\')
					numSlashes++;

				if (numSlashes == 3)
				{
					tempString = path + i + 1;
					(* share) = tempString.Left(tempString.Find('\\'));

					break;
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

// GetCurrentDirectory gets the current working directory.

DualErr 
GetCurrentDirectory(CString *outDir)
{
	DualErr derr;

	pgpAssertAddrValid(outDir, CString);

	try 
	{
		LPSTR outDirBuf;

		outDirBuf = outDir->GetBuffer(kMaxStringSize);
		GetCurrentDirectory(kMaxStringSize, outDirBuf);
		outDir->ReleaseBuffer();
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	return derr;
}

// GetDirectory strips the file name off of a path.

DualErr
GetDirectory(LPCSTR path, CString *dir)
{
	DualErr derr;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(dir, CString);

	try
	{
		CString tempString;

		dir->Empty();
		tempString = path;

		if (tempString.GetAt(tempString.GetLength() - 1) != '\\')
		{
			PGPInt32 lastSlash;

			lastSlash = tempString.ReverseFind('\\');

			if (lastSlash != -1)
				(* dir) = tempString.Left(lastSlash + 1);
		}
		else
		{
			(* dir) = tempString;
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	return derr;
}

// GetDirectorySmart is like GetDirectory except it tacks on a working
// directory path to incomplete directories.

DualErr
GetDirectorySmart(LPCSTR path, CString *dir)
{
	DualErr derr;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(dir, CString);

	try
	{
		CString tempString;

		dir->Empty();
		tempString = path;

		if (tempString.GetAt(tempString.GetLength() - 1) != '\\')
		{
			PGPUInt32 lastSlash;

			lastSlash = tempString.ReverseFind('\\');

			if (lastSlash != -1)
			{
				(* dir) = tempString.Left(lastSlash);
				(* dir) += "\\";
			}
		}
		else
		{
			(* dir) = tempString;
		}

		if (!HasPlainLocalRoot((* dir)) && !IsUNCPath((* dir)))
		{
			CString workingDir;

			derr = GetCurrentDirectory(&workingDir);

			if (derr.IsntError())
			{
				tempString = (* dir);

				if (tempString.IsEmpty() || (tempString.GetAt(0) != '\\'))
					(* dir) = workingDir + "\\" + tempString;
				else
					(* dir) = workingDir + tempString;
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

// GetBareName takes a full pathname to a file and returns just the name of
// the file, without the path. It checks the Windows 'show extensions'
// preference and keeps or discards the extension as necessary.

DualErr
GetBareName(LPCSTR path, CString *bareName, BareNameExtFlag bneFlag)
{
	DualErr derr;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(bareName, CString);

	try
	{
		CString		displayName, tempString;
		PGPBoolean	stripExtension;
		PGPInt32	lastSlash;
		SHFILEINFO	SHFI;

		bareName->Empty();

		if (strlen(path) < 3)
			derr = DualErr(kPGDMinorError_InvalidPathNameFound);

		// Will we show or hide the extension?
		if (derr.IsntError())
		{
			switch (bneFlag)
			{
			case kBNE_Default:
				// Ask the system how to display this pathname.
				SHGetFileInfo(path, NULL, &SHFI, sizeof(SHFI), 
					SHGFI_DISPLAYNAME);

				displayName = SHFI.szDisplayName;

				stripExtension = (displayName.Right(4).CompareNoCase(
					kPGPdiskFileExtension) != 0);
				;
				break;

			case kBNE_HideExt:
				stripExtension = TRUE;
				break;

			case kBNE_ShowExt:
				stripExtension = FALSE;
				break;

			default:
				pgpAssert(FALSE);
				break;
			}

			tempString = path;

			// Strip the path component.
			lastSlash = tempString.ReverseFind('\\');

			if (lastSlash != -1)
			{
				(* bareName) = tempString.Right(tempString.GetLength() - 
					lastSlash - 1);
			}
			else
			{
				(* bareName) = tempString;
			}

			// Strip extension if ordered to.
			if (stripExtension)
			{
				PGPInt32 lastDot;

				tempString = (* bareName);
				lastDot = tempString.ReverseFind('.');

				if (lastDot != -1)
					(* bareName) = tempString.Left(lastDot);
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

// CanonicalizeVolumeName turns the input string into a legal volume name.

DualErr 
CanonicalizeVolumeName(LPCSTR inName, CString *outName)
{
	DualErr derr;

	pgpAssertStrValid(inName);
	pgpAssertAddrValid(outName, CString);

	try 
	{
		PGPUInt32 i;

		// Legalize the label.
		(* outName)	= inName;
		(* outName)	= outName->Left(kMaxVolumeLabelLength);

		while ((i = outName->FindOneOf(kInvalidVolumeNameChars)) != -1)
		{
			outName->SetAt(i, kInvalidCharReplacement);
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	return derr;
}

// VerifyAndCanonicalizePath takes a pathname or a filename, and returns the
// full path to the closest file it can find.

DualErr 
VerifyAndCanonicalizePath(LPCSTR inPath, CString *outPath)
{
	DualErr derr;

	try
	{
		CString		bareName, csInPath, dir, localPath;
		LPSTR		pFilePart	= NULL;
		PGPBoolean	isLoopedBack;

		pgpAssertStrValid(inPath);
		pgpAssertAddrValid(outPath, CString);

		csInPath = inPath;

		// If looped back, resolve it a to local path.
		if (derr.IsntError())
		{
			if (IsNetworkedPath(csInPath))
			{
				if (!IsUNCPath(csInPath))
					derr = TranslateDriveToUNC(csInPath, &csInPath);

				if (derr.IsntError())
				{
					CheckIfLoopedBack(csInPath, &localPath, 
						&isLoopedBack);

					if (derr.IsntError())
					{
						if (isLoopedBack)
							csInPath = localPath;
					}

					// This check WILL fail on non-admin accounts; don't
					// sweat it now, we will deal with it later.

					derr = DualErr::NoError;
				}
			}
		}

		// Get the bare file name.
		if (derr.IsntError())
		{
			derr = GetBareName(csInPath, &bareName, kBNE_ShowExt);
		}

		// Get the directory component.
		if (derr.IsntError())
		{
			derr = GetDirectorySmart(csInPath, &dir);
		}

		// Get full pathname to the file.
		if (derr.IsntError())
		{
			if (!::SearchPath(dir, bareName, kPGPdiskFileExtension, 
				kMaxStringSize, outPath->GetBuffer(kMaxStringSize), 
				&pFilePart))
			{
				derr = DualErr(kPGDMinorError_PGPdiskNotFound);
			}

			outPath->ReleaseBuffer();
		}

		// Convert to long pathname form.
		if (derr.IsntError())
		{
			derr = ConvertPathToLong(outPath);
		}	
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	return derr;
}

// ResolveShortcut takes a path to a shortcut and returns a path to the actual
// file. Ripped from the SDK.

DualErr 
ResolveShortcut(LPCSTR shortcutPath, CString *fullPath)
{
	CString			szGotPath;
	DualErr 		derr;
	HRESULT 		result;
	IPersistFile	*pIPF;
	IShellLink		*pShellLink;
	PGPBoolean		loadedCOM, loadedPersistFile, loadedShellLink;
	WIN32_FIND_DATA	WFD;
 
	pgpAssertStrValid(shortcutPath);
	pgpAssertAddrValid(fullPath, CString);

	loadedCOM = loadedPersistFile = loadedShellLink = FALSE;

	// Load COM.
	result = CoInitialize(NULL);

	if (result < 0)
		derr = DualErr(kPGDMinorError_CoInitializeExFailed);

	loadedCOM = derr.IsntError();

	// Get a pointer to the IShellLink interface.
	if (derr.IsntError())
	{
		result = CoCreateInstance(CLSID_ShellLink, NULL, 
			CLSCTX_INPROC_SERVER, IID_IShellLink, (void **) &pShellLink);

		if (result < 0)
			derr = DualErr(kPGDMinorError_CoCreateInstanceFailed);

		loadedShellLink = derr.IsntError();
	}

	// Get a pointer to the IPersistFile interface.
	if (derr.IsntError())
	{ 
		result = pShellLink->QueryInterface(IID_IPersistFile, 
			(void **) &pIPF);

		if (result < 0)
			derr = DualErr(kPGDMinorError_QueryInterfaceFailed);

		loadedPersistFile = derr.IsntError();
	}

	if (derr.IsntError())
	{
		PGPUInt16 uniSCPath[kMaxStringSize]; 
 
		// Ensure that the string is Unicode.
		MultiByteToWideChar(CP_ACP, 0, shortcutPath, -1, uniSCPath, 
			kMaxStringSize);
 
		// Load the shortcut.
		result = pIPF->Load(uniSCPath, STGM_READ);

		if (result < 0)
			derr = DualErr(kPGDMinorError_OLELoadCommandFailed);
	}

	// Resolve the link.
	if (derr.IsntError())
	{
		result = pShellLink->Resolve(MessageBoxParent->GetSafeHwnd(), 
			SLR_ANY_MATCH);

		if (result < 0)
			derr = DualErr(kPGDMinorError_ResolveShortcutFailed);
	}

	// Get the path to the link target.
	if (derr.IsntError())
	{
		try
		{
			result = pShellLink->GetPath(fullPath->GetBuffer(kMaxStringSize), 
				kMaxStringSize, (WIN32_FIND_DATA *) &WFD, SLGP_SHORTPATH);

			fullPath->ReleaseBuffer();

			if (result < 0)
				derr = DualErr(kPGDMinorError_OLEGetPathFailed);
		}
		catch (CMemoryException *ex)
		{
			derr = DualErr(kPGDMinorError_OutOfMemory);
			ex->Delete();
		}
	}

	// Release the pointer to the IPersistFile interface.
	if (loadedPersistFile)
		pIPF->Release(); 

	// Release the pointer to the IShellLink interface.
	if (loadedShellLink)
		pShellLink->Release(); 

	if (loadedCOM)
		CoUninitialize();

	return derr;
}


/////////////////////////
// Drive/volume functions
/////////////////////////

// BroadcastDriveMessage broadcasts a system message concerning the volume.

DualErr 
BroadcastDriveMessage(PGPUInt8 drive, WPARAM msg)
{
	DEV_BROADCAST_VOLUME	DBV;
	DualErr					derr;
	PGPInt32				result;

	pgpAssert(IsLegalDriveNumber(drive));

	DBV.dbcv_size		= sizeof(DBV); 
	DBV.dbcv_devicetype = DBT_DEVTYP_VOLUME; 
	DBV.dbcv_reserved	= 0;
	DBV.dbcv_unitmask	= 1 << drive; 
	DBV.dbcv_flags		= DBTF_MEDIA;

	result = BroadcastSystemMessage(BSF_NOHANG | BSF_POSTMESSAGE, NULL, 
		WM_DEVICECHANGE, msg, (LPARAM) &DBV);

	if (result < 1)
		derr = DualErr(kPGDMinorError_BroadcastSystemMessageFailed);
	
	return derr;
}

// GetLocalComputerName returns the name of the local computer.

DualErr 
GetLocalComputerName(CString *compName)
{
	DualErr derr;

	try
	{
		PGPUInt32		result;
		WKSTA_INFO_100	*pWI100;

		USES_CONVERSION;

		pgpAssertAddrValid(compName, CString);
		pgpAssert(IsWinNT4CompatibleMachine());

		result = WinNT_NetWkstaGetInfo(NULL, 100, (LPBYTE *) &pWI100);

		if (result != NERR_Success)
		{
			derr = DualErr(kPGDMinorError_NetWkstaGetInfoFailed, 
				result);
		}

		if (derr.IsntError())
		{
			(* compName) = W2A((LPWSTR) pWI100->wki100_computername);
			WinNT_NetApiBufferFree(pWI100);
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
		derr = DualErr(kPGDMinorError_OutOfMemory);
	}

	return derr;
}

// Win95GetUniversalName implements a hack under Windows95 that simulates
// the non-working WNetGetUniversalName (from KB Article ID: Q131416).

static BOOL
Win95GetUniversalName( LPCTSTR szDrive, LPTSTR szUniv  )   
{
	// get the local drive letter
	char chLocal = toupper( szDrive[0] );

	// cursory validation
	if ( chLocal < 'A' || chLocal > 'Z' )
		return FALSE;

	if ( szDrive[1] != ':' || szDrive[2] != '\\' )
		return FALSE;

	HANDLE hEnum;
	DWORD dwResult = WNetOpenEnum( RESOURCE_CONNECTED, RESOURCETYPE_DISK,
									 0, NULL, &hEnum );

	if ( dwResult != NO_ERROR )
		return FALSE;

	// request all available entries
	const int    c_cEntries   = 0xFFFFFFFF;
	// start with a reasonable buffer size
	DWORD        cbBuffer     = 50 * sizeof( NETRESOURCE );
	NETRESOURCE *pNetResource = (NETRESOURCE*) malloc( cbBuffer );

	BOOL fResult = FALSE;

	while ( TRUE )
	{
		DWORD dwSize   = cbBuffer,
		cEntries = c_cEntries;

		dwResult = WNetEnumResource( hEnum, &cEntries, pNetResource,
									  &dwSize );

		if ( dwResult == ERROR_MORE_DATA )
		{
			// the buffer was too small, enlarge
			cbBuffer = dwSize;
			pNetResource = (NETRESOURCE*) realloc(pNetResource, cbBuffer);
			continue;
		}

		if ( dwResult != NO_ERROR )
			goto done;

		// search for the specified drive letter
		for ( int i = 0; i < (int) cEntries; i++ )
			if ( pNetResource[i].lpLocalName &&
				chLocal == toupper(pNetResource[i].lpLocalName[0]) )
			{
				// match
				fResult = TRUE;

				// build a UNC name
				strcpy( szUniv, pNetResource[i].lpRemoteName );
				strcat( szUniv, szDrive + 2 );
				_strupr( szUniv );
				goto done;
			}
		}

done:
	// cleanup
	WNetCloseEnum( hEnum );
	free( pNetResource );

	return fResult;
}


// TranslateDriveToUNC translates a mapped networked drive-based pathname to
// its UNC form.

DualErr 
TranslateDriveToUNC(LPCSTR inPath, CString *outPath)
{
	DualErr	derr;

	try
	{
		PGPUInt32 result;

		pgpAssertStrValid(inPath);
		pgpAssertAddrValid(outPath, CString);

		if (IsWin95CompatibleMachine() && !IsWin98CompatibleMachine())
		{
			PGPBoolean resultWin95;
			char	   tempOutPath[kMaxStringSize];

			// Special hack function for Windows95.
			resultWin95 = Win95GetUniversalName(inPath, tempOutPath);

			if (!resultWin95)
			{
				derr = DualErr(kPGDMinorError_WNetGetUniNameFailed, 
					GetLastError());
			}
			else
			{
				(* outPath) = tempOutPath;
			}
		}
		else
		{
			UNIVERSAL_NAME_INFO	*pUNI;
			unsigned long		bufSize;

			// Handle normally for Win98 and WinNT.
			bufSize = sizeof(UNIVERSAL_NAME_INFO) + 
				kMaxStringSize * sizeof(WCHAR);

			pUNI = (UNIVERSAL_NAME_INFO *) new PGPUInt8[bufSize];

			if (derr.IsntError())
			{
				result = WNetGetUniversalName(inPath, 
					UNIVERSAL_NAME_INFO_LEVEL, pUNI, &bufSize);

				if (result != NO_ERROR)
				{
					derr = DualErr(kPGDMinorError_WNetGetUniNameFailed,
						result);
				}
			}

			if (derr.IsntError())
			{
				(* outPath) = pUNI->lpUniversalName;
			}

			delete[] (PGPUInt8 *) pUNI;
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
		derr = DualErr(kPGDMinorError_OutOfMemory);
	}

	return derr;
}

// TranslateUNCToLocal translates a 'looped-back' path to its local form.

DualErr 
TranslateUNCToLocal(
	LPCSTR		inPath, 
	CString		*outPath, 
	PGPBoolean	*isLoopedBack)
{
	DualErr	derr;

	pgpAssertStrValid(inPath);
	pgpAssertAddrValid(outPath, CString);
	pgpAssertAddrValid(isLoopedBack, PGPBoolean);

	pgpAssert(IsWin95CompatibleMachine() || IsWinNT4CompatibleMachine());

	try
	{
		CString newRoot, nonRoot, share;

		derr = GetShare(inPath, &share);

		if (derr.IsntError())
		{
			derr = StripRoot(inPath, &nonRoot);
		}

		if (derr.IsntError())
		{
			if (IsWin95CompatibleMachine())
			{
				PGPUInt32		bufSize, result;
				share_info_50	*pSI50;
				unsigned short	cbTotalAvail;

				bufSize = sizeof(share_info_50) + MAX_PATH + MAXCOMMENTSZ + 2;
				pSI50 = (share_info_50 *) new PGPUInt8[bufSize];

				result = Win95_NetShareGetInfo(NULL, share, 50, 
					(char *) pSI50, bufSize, &cbTotalAvail);

				if (result != NERR_Success)
				{
					derr = DualErr(kPGDMinorError_NetShareGetInfoFailed, 
						result);
				}

				if (derr.IsntError())
				{
					newRoot = pSI50->shi50_path;
				}

				delete[] (PGPUInt8 *) pSI50;
			}
			else if (IsWinNT4CompatibleMachine())
			{
				PGPUInt32		result;
				SHARE_INFO_2	*pSI2;

				USES_CONVERSION;

				result = WinNT_NetShareGetInfo(NULL, A2W(share), 2, 
					(LPBYTE *) &pSI2);

				if (result != NERR_Success)
				{
					derr = DualErr(kPGDMinorError_NetShareGetInfoFailed, 
						result);
				}

				if (derr.IsntError())
				{
					newRoot = W2A((LPWSTR) pSI2->shi2_path);
					WinNT_NetApiBufferFree(pSI2);
				}
			}
		}

		if (derr.IsntError())
		{
			(* outPath) = newRoot + "\\" + nonRoot;
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
		derr = DualErr(kPGDMinorError_OutOfMemory);
	}

	(* isLoopedBack) = derr.IsntError();

	return derr;
}

// CheckIfLoopedBack checks if the given networked path is looped back to a
// local drive.

DualErr 
CheckIfLoopedBack(
	LPCSTR		inPath, 
	CString		*outPath, 
	PGPBoolean	*isLoopedBack)
{
	DualErr derr;

	pgpAssertStrValid(inPath);
	pgpAssertAddrValid(outPath, CString);
	pgpAssertAddrValid(isLoopedBack, PGPBoolean);

	if (!IsNetworkedPath(inPath))
	{
		(* isLoopedBack) = FALSE;
	}
	else if (IsUNCPath(inPath))
	{
		derr = TranslateUNCToLocal(inPath, outPath, isLoopedBack);
	}
	else
	{
		CString uncPath, share;

		derr = TranslateDriveToUNC(inPath, &uncPath);
		
		if (derr.IsntError())
		{
			derr = TranslateUNCToLocal(uncPath, outPath, isLoopedBack);
		}
	}

	return derr;
}
