//////////////////////////////////////////////////////////////////////////////
// CPGPdiskAppWinutils.cpp
//
// Wrapper functions around useful system calls.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskAppWinutils.cpp,v 1.2.2.29 1998/08/12 01:50:28 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "DriverComm.h"
#include "PGPdiskRegistry.h"
#include "PGPdiskVersion.h"
#include "StringAssociation.h"
#include "UtilityFunctions.h"
#include "Win32Utils.h"

#include "CMainDialog.h"
#include "CPGPdiskApp.h"
#include "CPGPdiskAppWinutils.h"
#include "Globals.h"
#include "PGPdisk.h"
#include "Resource.h"


////////////
// Constants
////////////

const	PGPUInt32	kRegTextInc		= 16;
const	PGPUInt32	kLicenseToGap	= 8;


////////////////////////////////////
// File and Volume Utility Functions
////////////////////////////////////

// AreAnyDriveLettersFree returns TRUE if there are any drive letters free
// for mounting PGPdisks, otherwise FALSE.

	PGPBoolean 
CPGPdiskApp::AreAnyDriveLettersFree()
{
	PGPUInt32 drives = GetLogicalDrives();

	// Mask off drives A and B and anything over Z.
	return ((drives &= 0x3FFFFFC) != 0x3FFFFFC);
}

// IsDirectoryValid returns TRUE if the specified directory exists, FALSE
// otherwise.

PGPBoolean 
CPGPdiskApp::IsDirectoryValid(LPCSTR path)
{
	PGPUInt32 attribs;

	pgpAssertStrValid(path);

	attribs = GetFileAttributes(path);

	if (attribs == 0xFFFFFFFF)
		return FALSE;
	else
		return (attribs & FILE_ATTRIBUTE_DIRECTORY ? TRUE : FALSE);
}

// IsFileValid returns TRUE if the specified file exists, FALSE otherwise.

PGPBoolean 
CPGPdiskApp::IsFileValid(LPCSTR path)
{
	PGPUInt32 attribs;

	pgpAssertStrValid(path);

	attribs = GetFileAttributes(path);

	if (attribs == 0xFFFFFFFF)
		return FALSE;
	else
		return (attribs & FILE_ATTRIBUTE_DIRECTORY ? FALSE : TRUE);
}

// IsPathValid returns TRUE if the specified path exists, FALSE otherwise.

PGPBoolean 
CPGPdiskApp::IsPathValid(LPCSTR path)
{
	return (IsFileValid(path) || IsDirectoryValid(path));
}

// IsVolumeValid returns TRUE if the specified volume is valid, FALSE
// otherwise.

PGPBoolean 
CPGPdiskApp::IsVolumeValid(PGPUInt8 drive)
{
	CString root;

	pgpAssert(IsLegalDriveNumber(drive));

	return ((GetLogicalDrives() & (1 << drive)) ? TRUE : FALSE);
}

// IsVolumeFormatted returns TRUE if the specified volume is formatted, FALSE
// otherwise.

PGPBoolean 
CPGPdiskApp::IsVolumeFormatted(PGPUInt8 drive)
{
	CString root;

	pgpAssert(IsLegalDriveNumber(drive));

	if (MakeRoot(drive, &root).IsntError())
	{
		return GetVolumeInformation(root, NULL, 0, NULL, NULL, NULL, NULL, 0);
	}
	else
	{
		return TRUE;
	}
}

// IsDirectoryReadOnly returns TRUE if the directory specified by path is
// read-only, FALSE otherwise.

PGPBoolean 
CPGPdiskApp::IsDirectoryReadOnly(LPCSTR path)
{
	CString		dir, csPath, temp;
	DualErr		derr;
	PGPBoolean	isDirReadOnly;
	PGPUInt32	result;

	pgpAssertStrValid(path);

	// Attempt to create a temporary file.
	try
	{
		csPath = path;
		dir = csPath.Left(csPath.ReverseFind('\\') + 1);

		result = GetTempFileName(dir, "pgp", 0, 
			temp.GetBuffer(kMaxStringSize));

		temp.ReleaseBuffer();
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	// If failure, we assume read-only.
	if (derr.IsntError())
	{
		if (result == 0)
		{
			isDirReadOnly = TRUE;
		}
		else
		{
			isDirReadOnly = FALSE;
			DeleteFile(temp);
		}
	}

	if (derr.IsntError())
		return isDirReadOnly;
	else
		return TRUE;
}

// IsFileReadOnly returns TRUE if the file specified by path is read-only,
// FALSE otherwise.

PGPBoolean 
CPGPdiskApp::IsFileReadOnly(LPCSTR path)
{
	DualErr		derr;
	File		theFile;
	PGPUInt8	firstByte	= 0;
	PGPUInt64	length;

	// Open a read/write handle to the file and try to write a byte. If
	// anything fails, we aren't going to be doing any writing. Handle
	// 0-length files as a special case.

	derr = theFile.Open(path, kOF_MustExist);

	if (derr.IsntError())
	{
		derr = theFile.GetLength(&length);
	}

	if (derr.IsntError())
	{
		if (length > 0)
			derr = theFile.Read((PGPUInt8 *) &firstByte, 0, 1);
	}

	if (derr.IsntError())
	{
		derr = theFile.Write((PGPUInt8 *) &firstByte, 0, 1);
	}

	if (derr.IsntError())
	{
		if (length == 0)
			theFile.SetLength(0);
	}

	if (theFile.Opened())
		theFile.Close();

	return derr.IsError();
}

// IsFileInUseByReader returns TRUE if someone has opened the specified file
// with read access, FALSE otherwise.

PGPBoolean 
CPGPdiskApp::IsFileInUseByReader(LPCSTR path)
{
	File	existingFile;
	DualErr	derr;

	pgpAssertStrValid(path);

	derr = existingFile.Open(path, 
		kOF_ReadOnly | kOF_DenyRead | kOF_MustExist);

	if (existingFile.Opened())
	{
		existingFile.Close();
	}

	return derr.IsError();
}

// IsFileInUseByWriter returns TRUE if someone has opened the specified file
// with write access, FALSE otherwise.

PGPBoolean 
CPGPdiskApp::IsFileInUseByWriter(LPCSTR path)
{
	File	existingFile;
	DualErr	derr;

	pgpAssertStrValid(path);

	derr = existingFile.Open(path, 
		kOF_ReadOnly | kOF_DenyWrite | kOF_MustExist);

	if (existingFile.Opened())
	{
		existingFile.Close();
	}

	return derr.IsError();
}

// IsFileInUseByWriter returns TRUE if someone has opened the specified file,
// FALSE otherwise.

PGPBoolean 
CPGPdiskApp::IsFileInUse(LPCSTR path)
{
	return (IsFileInUseByReader(path) || IsFileInUseByWriter(path));
}

// BruteForceTestIfEnoughSpace tests if the desired directory has enough
// space to hold a file of the specified size. It does this by creating a
// temporary file of the desired size.

DualErr 		
CPGPdiskApp::BruteForceTestIfEnoughSpace(
	LPCSTR		dir, 
	PGPUInt32	kbLength, 
	PGPBoolean	*isEnoughSpace)
{
	DualErr derr;

	try
	{
		File		tempFile;
		CString		tempPath;
		DualErr		setLengthErr;
		PGPBoolean	createdTemp	= FALSE;
		PGPUInt64	tempSize;

		pgpAssertStrValid(dir);
		pgpAssertAddrValid(isEnoughSpace, PGPBoolean);

		// Open a temporary file in the desired directory.
		GetTempFileName(dir, "pgp", 0, tempPath.GetBuffer(kMaxStringSize));
		tempPath.ReleaseBuffer();

		derr = tempFile.Open(tempPath, kOF_Trunc);
		createdTemp = derr.IsntError();

		// Set the file to the desired length.
		if (derr.IsntError())
		{
			setLengthErr = tempFile.SetLength(kbLength * kBytesPerKb);

			// Did we fail to set the size of the file?
			if (setLengthErr.IsError())
			{
				(* isEnoughSpace) = FALSE;
			}
		}

		// Verify the set succeeded by closing and re-opening the file.
		if (derr.IsntError() && setLengthErr.IsntError())
		{
			tempFile.Close();
			derr = tempFile.Open(tempPath, kOF_MustExist);
		}

		if (derr.IsntError() && setLengthErr.IsntError())
		{
			derr = tempFile.GetLength(&tempSize);
		}

		// Do the sizes match?
		if (derr.IsntError() && setLengthErr.IsntError())
		{
			(* isEnoughSpace) = (tempSize == kbLength*kBytesPerKb);
		}

		if (tempFile.Opened())
		{
			tempFile.Close();
		}

		if (createdTemp)
		{
			DeleteFile(tempPath);
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
		derr = DualErr(kPGDMinorError_OutOfMemory);
	}

	return derr;
}

// HowMuchFreeSpace returns the amount of free space on the specified drive,
// in bytes.

DualErr 
CPGPdiskApp::HowMuchFreeSpace(LPCSTR path, PGPUInt64 *bytesFree)
{
	CString			root;
	DualErr			derr;
	ULARGE_INTEGER	freeAvail;

	pgpAssertAddrValid(bytesFree, PGPUInt64);

	// We must pass the root, not the path, to the system.
	derr = GetRoot(path, &root);

	// There are two different functions for OSR1 and OSR2.
	if (derr.IsntError())
	{
		if (IsWin95OSR2CompatibleMachine() || IsWinNT4CompatibleMachine())
		{
			PGPUInt32		result;
			ULARGE_INTEGER	freeBytes, totalBytes;

			result = GetDiskFreeSpaceEx(root, &freeAvail, &totalBytes, 
				&freeBytes);

			if (!result)
			{
				derr = DualErr(kPGDMinorError_GetFreeSpaceFailed, 
					GetLastError());
			}
		}
		else
		{
			unsigned long spc, bps, freeClust, totalClust;

			if (!::GetDiskFreeSpace(root, &spc, &bps, &freeClust, 
				&totalClust))
			{

				derr = DualErr(kPGDMinorError_GetFreeSpaceFailed, 
					GetLastError());
			}

			if (derr.IsntError())
			{
				freeAvail.QuadPart = (bps*spc*freeClust);
			}
		}
	}

	if (derr.IsntError())
	{
		(* bytesFree) = freeAvail.QuadPart;
	}

	return derr;
}

// ExceptionFilter determines whether to handle the Windows exception passed
// to it.

int
CPGPdiskApp::ExceptionFilter(int exception)
{
	switch (exception)
	{
	case STATUS_BREAKPOINT:
	case STATUS_DATATYPE_MISALIGNMENT:
	case STATUS_SINGLE_STEP:
		return EXCEPTION_CONTINUE_SEARCH;

	default:
		return EXCEPTION_EXECUTE_HANDLER;
	}
}

// ShowWindowsFormatDialog brings up the Windows formatting dialog for the
// specified drive letter.

DualErr 
CPGPdiskApp::ShowWindowsFormatDialog(PGPUInt8 drive)
{
	DualErr		derr;
	HINSTANCE	shell32Handle;
	PGPBoolean	loadedShell32	= FALSE;
	PVOID		SHFormatDriveFunc;

	pgpAssert(IsWin95CompatibleMachine() || IsWinNT4CompatibleMachine());

	// Open handle to shell32.dll
	shell32Handle = LoadLibrary("shell32.dll");
	loadedShell32 = IsntNull(shell32Handle);

	if (!loadedShell32)
		derr = DualErr(kPGDMinorError_LoadLibraryFailed, GetLastError());

	// Get procedure address for the formatting function.
	if (derr.IsntError())
	{
		SHFormatDriveFunc = GetProcAddress(shell32Handle, 
			"SHFormatDrive");

		if (IsNull(SHFormatDriveFunc))
		{
			derr = DualErr(kPGDMinorError_GetProcAddrFailed, 
				GetLastError());
		}
	}

	// Attempt to call the function, catching all crashes in the process.
	if (derr.IsntError())
	{
		HWND		parentHwnd	= mMainDialog->GetSafeHwnd();
		PGPUInt32	largeDrive	= drive;

		__try
		{
			__asm
			{
				push 1
				push 0x0000FFFF
				push largeDrive
				push parentHwnd
				call [SHFormatDriveFunc]
				sub esp, 0x10
			}
		}
		__except(ExceptionFilter(GetExceptionCode()))
		{
			derr = DualErr(kPGDMinorError_SHFormatDriveFailed);
		}
	}

	if (loadedShell32)
		FreeLibrary(shell32Handle);

	return derr;
}


//////////////////////////////
// Interface Utility Functions
//////////////////////////////

// TweakOnTopAttribute makes the selected window "on top" if the main window's
// "on-top" preference is set.

void 
CPGPdiskApp::TweakOnTopAttribute(CWnd *pWnd)
{
	PGPUInt32 checkState;

	pgpAssertAddrValid(pWnd, CWnd);

	checkState = PGPdiskGetPrefDWord(kRegistryMainStayOnTopKey, MF_UNCHECKED);

	if (checkState == MF_CHECKED)
	{
		pWnd->SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, 
			SWP_NOMOVE | SWP_NOSIZE);
	}
}

// CreateDIBPalette is a helper function for GetBitmapAndPalette.

HPALETTE 
CPGPdiskApp::CreateDIBPalette(LPBITMAPINFO lpbmi, LPINT lpiNumColors) 
{
	LPBITMAPINFOHEADER	lpbi;
	LPLOGPALETTE		lpPal;
	HANDLE				hLogPal;
	HPALETTE			hPal	= NULL;
	PGPInt32			i;
 
	lpbi = (LPBITMAPINFOHEADER) lpbmi;

	if (lpbi->biBitCount <= 8)
	{
		(* lpiNumColors) = (1 << lpbi->biBitCount);
	}
	else
	{
		(* lpiNumColors = 0);  // No palette needed for 24 BPP DIB
	}

	if (* lpiNumColors)
	{
		hLogPal = GlobalAlloc(GHND, sizeof(LOGPALETTE) + 
			sizeof(PALETTEENTRY) * (* lpiNumColors));

		lpPal = (LPLOGPALETTE) GlobalLock(hLogPal);
		lpPal->palVersion = 0x300;
		lpPal->palNumEntries = *lpiNumColors;
 
		for (i = 0;  i < (* lpiNumColors);  i++)
		{
			lpPal->palPalEntry[i].peRed   = lpbmi->bmiColors[i].rgbRed;
			lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
			lpPal->palPalEntry[i].peBlue  = lpbmi->bmiColors[i].rgbBlue;
			lpPal->palPalEntry[i].peFlags = 0;
		}

		hPal = CreatePalette(lpPal);

		GlobalUnlock(hLogPal);
		GlobalFree(hLogPal);
   }

   return hPal;
}

// GetBitmapAndPalette loads a 256-color bitmap as a resource and constructs
// the necessary palette to draw it with.

BOOL 
CPGPdiskApp::GetBitmapAndPalette(
	UINT		nIDResource, 
	CBitmap		*bitmap, 
	CPalette	*pal)
{
	HRSRC				hRsrc;
	HGLOBAL				hGlobal;
	HBITMAP				hBitmapFinal	= NULL;
	LPBITMAPINFOHEADER	lpbi;
	HDC					hdc;
    INT					iNumColors;
 
	hRsrc = FindResource(AfxGetInstanceHandle(), 
		MAKEINTRESOURCE(nIDResource), RT_BITMAP);

	if (IsntNull(hRsrc))
	{
		hGlobal = LoadResource(AfxGetInstanceHandle(), hRsrc);
		lpbi = (LPBITMAPINFOHEADER) LockResource(hGlobal);

		hdc = GetDC(NULL);

		if (pal->Attach(CreateDIBPalette((LPBITMAPINFO) lpbi, &iNumColors)))
		{
			SelectPalette(hdc, (* pal), FALSE);
			RealizePalette(hdc);
		}
 
		hBitmapFinal = CreateDIBitmap(hdc, (LPBITMAPINFOHEADER)lpbi,
			(LONG) CBM_INIT,
			(LPSTR) lpbi + lpbi->biSize + iNumColors * sizeof(RGBQUAD),
			(LPBITMAPINFO) lpbi, DIB_RGB_COLORS);

		bitmap->Attach(hBitmapFinal);

		ReleaseDC(NULL,hdc);
		UnlockResource(hGlobal);
		FreeResource(hGlobal);

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

// PaintRegistrationInfo paints the user registration info on an about box or
// splash screen of the given size using the given DC in the given window.

void 
CPGPdiskApp::PaintRegistrationInfo(
	CDC			*pDC, 
	PGPUInt32	x, 
	PGPUInt32	y, 
	COLORREF	textColor, 
	CWnd		*pWnd)
{
	try
	{
		CSize		licenseToSize;
		CString		orgText, userText;
		HFONT		oldFont;
		LPSTR		licenseToText;
		PGPInt32	maxPixelLength, widthLicenseTo;
		RECT		windowRect;

		pgpAssertAddrValid(pDC, CDC);
		pgpAssertAddrValid(pWnd, CWnd);

		pDC->SetTextColor(textColor);
		pDC->SetBkMode(TRANSPARENT);

		// MFC bug - CDC::SelectObject returns incorrect pointers/handles so
		// don't fool with it.

		oldFont = (HFONT) SelectObject(pDC->GetSafeHdc(), 
			GetStockObject(DEFAULT_GUI_FONT));

		// Determine the maximum pixel string width we can draw.
		pWnd->GetClientRect(&windowRect);

		maxPixelLength = windowRect.right - x - 5;
		pgpAssert(maxPixelLength > 0);

		// Truncate the strings with periods if they are too long.
		TruncateDisplayString(App->mOrgText, &orgText, pDC, maxPixelLength);

		TruncateDisplayString(App->mUserText, &userText, pDC, maxPixelLength);

		licenseToText = (LPSTR) GetCommonString(kPGPdiskLicenseToText);

		// Draw the static string.
		licenseToSize = pDC->GetOutputTextExtent(licenseToText, 
			strlen(licenseToText));

		pDC->LPtoDP(&licenseToSize);

		widthLicenseTo = licenseToSize.cx;
		pgpAssert(x > widthLicenseTo + kLicenseToGap);

		pDC->TextOut(x - widthLicenseTo - kLicenseToGap, y, licenseToText, 
			strlen(licenseToText));

		// Draw the variable strings.
		if (!userText.IsEmpty())
		{
			pDC->TextOut(x, y, userText, userText.GetLength());
			y += kRegTextInc;
		}

		if (!orgText.IsEmpty())
		{
			pDC->TextOut(x, y, orgText, orgText.GetLength());
			y += kRegTextInc;
		}

		pDC->SelectObject(oldFont);
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// PaintVersionInfo paints the PGPdisk version info on an about box or splash
// screen of the given size using the given DC in the given window at the
// given location.

void 
CPGPdiskApp::PaintVersionInfo(
	CDC			*pDC, 
	PGPUInt32	x, 
	PGPUInt32	y, 
	COLORREF	textColor, 
	CWnd		*pWnd)
{
	HFONT oldFont;

	pgpAssertAddrValid(pDC, CDC);
	pgpAssertAddrValid(pWnd, CWnd);

	pDC->SetTextColor(textColor);
	pDC->SetBkMode(TRANSPARENT);

	// MFC bug - CDC::SelectObject returns incorrect pointers/handles so don't
	// fool with it.

	oldFont = (HFONT) SelectObject(pDC->GetSafeHdc(), 
		GetStockObject(DEFAULT_GUI_FONT));

	pDC->TextOut(x, y, kVersionTextString, strlen(kVersionTextString));
	pDC->SelectObject(oldFont);
}

// GetTextRect returns the size of the rectangle the given text would occupy
// if drawn in the given window.

void 
CPGPdiskApp::GetTextRect(LPCSTR text, CWnd *pWnd, RECT *pRect)
{
	CDC		memDC;
	CDC		*pDC;
	CSize	textSize;
	HFONT	oldFont;

	pgpAssertStrValid(text);
	pgpAssertAddrValid(pWnd, CWnd);
	pgpAssertAddrValid(pRect, RECT);
	
	pDC = pWnd->GetDC();
	memDC.CreateCompatibleDC(pDC);

	// MFC bug - CDC::SelectObject returns incorrect pointers/handles so
	// don't fool with it.

	oldFont = (HFONT) SelectObject(memDC.GetSafeHdc(), 
		GetStockObject(DEFAULT_GUI_FONT));

	// Get dimensions of text.
	textSize = memDC.GetOutputTextExtent(text, strlen(text));
	memDC.LPtoDP(&textSize);

	pRect->left		= 0;
	pRect->right	= textSize.cx;
	pRect->top		= 0;
	pRect->bottom	= textSize.cy;

	memDC.SelectObject(oldFont);
	memDC.DeleteDC();
	pWnd->ReleaseDC(pDC);
}

// TruncateDisplayString will, given a string, a DC, and a maximum length
// (in pixels), return a string truncated with dots that will fit within that
// length.

void 
CPGPdiskApp::TruncateDisplayString(
	LPCSTR		inString, 
	CString		*outString, 
	CDC			*pDC, 
	PGPInt32	maxPixelLength)
{
	try
	{
		PGPBoolean	didWeTruncateString	= FALSE;
		PGPUInt32	i, numInputChars;

		pgpAssertStrValid(inString);
		pgpAssertAddrValid(outString, CString);
		pgpAssertAddrValid(pDC, CDC);
		pgpAssert(maxPixelLength > 0);

		numInputChars = strlen(inString);

		// Calculate the length of each substring that begins at position 0.
		// Once we reach a substring that exceeds our maximum pixel length,
		// we truncate it with dots.

		for (i=0; i < numInputChars; i++)
		{
			CSize	stringRect;
			CString	csInString;

			csInString = inString;

			// Calculate pixel width of string.
			stringRect = pDC->GetOutputTextExtent(inString, i);
			pDC->LPtoDP(&stringRect);

			// If too large, ditch last two chars and tack on 3 dots.
			if (stringRect.cx > maxPixelLength)
			{
				didWeTruncateString = TRUE;

				if (i > 3)
				{
					(* outString)	= csInString.Left(i - 3);
					(* outString)	+= "...";
				}
				else
				{
					(* outString) = csInString.Left(i - 1);
				}

				break;
			}
		}

		if (!didWeTruncateString)
		{
			(* outString) = inString;
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// FitStringToWindow truncates the string so it fits within the given window
// using the default display font.

void 
CPGPdiskApp::FitStringToWindow(
	LPCSTR	inString, 
	CString	*outString, 
	CWnd	*pWnd)
{
	CDC			memDC;
	CDC			*pDC;
	CSize		messageSize;
	HFONT		oldFont;
	PGPUInt32	widthMessage, xWindow;
	RECT		windowRect;

	pgpAssertStrValid(inString);
	pgpAssertAddrValid(outString, CString);
	pgpAssertAddrValid(pWnd, CWnd);

	// Get dimensions of window.
	pWnd->GetWindowRect(&windowRect);
	pgpAssert(windowRect.right - windowRect.left > 3);

	xWindow = windowRect.right - windowRect.left - 3;
	
	pDC = pWnd->GetDC();
	memDC.CreateCompatibleDC(pDC);

	// MFC bug - CDC::SelectObject returns incorrect pointers/handles so
	// don't fool with it.

	oldFont = (HFONT) SelectObject(memDC.GetSafeHdc(), 
		GetStockObject(DEFAULT_GUI_FONT));

	// Get dimensions of text to be written.
	messageSize = memDC.GetOutputTextExtent(inString, strlen(inString));
	memDC.LPtoDP(&messageSize);

	widthMessage = messageSize.cx;

	// If too big truncate string.
	if (widthMessage > xWindow)
		TruncateDisplayString(inString, outString, &memDC, xWindow);

	memDC.SelectObject(oldFont);
	memDC.DeleteDC();
	pWnd->ReleaseDC(pDC);
}

// FormatFitStringToWindow takes a string of the form "...%s..." and a
// substring and truncates the substring so the entire formatted message will
// fit inside the specified window.

void 
CPGPdiskApp::FormatFitStringToWindow(
	LPCSTR	inString, 
	LPCSTR	subString, 
	CString	*outString, 
	CWnd	*pWnd)
{
	CDC			memDC;
	CDC			*pDC;
	CSize		messageSizeWoSub, substringSize;
	CString		temp;
	HFONT		oldFont;
	PGPUInt32	widthSubString, widthMessageWoSub, xForSubstring, xWindow;
	RECT		windowRect;

	pgpAssertStrValid(inString);
	pgpAssertStrValid(subString);
	pgpAssertAddrValid(outString, CString);
	pgpAssertAddrValid(pWnd, CWnd);

	// Get dimensions of window.
	pWnd->GetWindowRect(&windowRect);
	pgpAssert(windowRect.right - windowRect.left > 3);

	xWindow = windowRect.right - windowRect.left - 3;
	
	pDC = pWnd->GetDC();
	memDC.CreateCompatibleDC(pDC);

	// MFC bug - CDC::SelectObject returns incorrect pointers/handles so
	// don't fool with it.

	oldFont = (HFONT) SelectObject(memDC.GetSafeHdc(), 
		GetStockObject(DEFAULT_GUI_FONT));

	// Get dimensions of text without the substring.
	try
	{
		temp.Format(inString, "");
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}

	messageSizeWoSub = memDC.GetOutputTextExtent(temp, temp.GetLength());
	memDC.LPtoDP(&messageSizeWoSub);

	widthMessageWoSub = messageSizeWoSub.cx;

	if (widthMessageWoSub < xWindow)
	{
		// Get dimensions of substring.
		substringSize = memDC.GetOutputTextExtent(subString, 
			strlen(subString));
		memDC.LPtoDP(&substringSize);

		widthSubString = substringSize.cx;
		xForSubstring = xWindow - widthMessageWoSub;

		TruncateDisplayString(subString, &temp, &memDC, xForSubstring);
		outString->Format(inString, temp);
	}
	else
	{
		TruncateDisplayString(temp, outString, &memDC, xWindow);
	}

	memDC.SelectObject(oldFont);
	memDC.DeleteDC();
	pWnd->ReleaseDC(pDC);
}
