//////////////////////////////////////////////////////////////////////////////
// PGPdiskRegistry.cpp
//
// Functions for accessing the system registry.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskRegistry.cpp,v 1.1.2.6.2.1 1998/10/05 18:28:26 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"

#else
#error Define PGPDISK_MFC.
#endif	// PGPDISK_MFC

#include "Required.h"
#include "PGPdiskRegistry.h"
#include "UtilityFunctions.h"


////////////
// Constants
////////////

// The global path is under "HKEY_LOCAL_MACHINE" and contains preferences
// specific to all users.

static LPCSTR kRegistryGlobalPath = 
	"SOFTWARE\\Network Associates\\PGPdisk";

static LPCSTR kRegistryLocalPath = 
	"Software\\Network Associates\\PGPdisk";

static LPCSTR kRegistryLocalPGP60Path = 
	"Software\\Network Associates\\PGP60";


////////////////////////////
// Low-Level Registry Access
////////////////////////////

// PGPdiskEnumSubkeys enumerates subkeys of a given key. Start with 'index'
// as 0 and then increment until 'isIndexValid' is set to FALSE.

DualErr 
PGPdiskEnumSubkeys(
	HKEY			root, 
	LPCSTR			section, 
	PGPUInt32		index, 
	LPSTR			keyName, 
	unsigned long	valueSize, 
	PGPBoolean		*isIndexValid)
{
	DualErr		derr;
	HKEY		regHandle;
	PGPBoolean	openedKey	= FALSE;
	PGPUInt32	result;

	pgpAssertStrValid(section);
	pgpAssertAddrValid(keyName, char);
	pgpAssertAddrValid(isIndexValid, PGPBoolean);

	result = RegOpenKeyEx(root, section, NULL, KEY_READ, &regHandle);

	if (result != ERROR_SUCCESS)
	{
		derr = DualErr(kPGDMinorError_RegOpenKeyExFailed, result);
		openedKey = derr.IsntError();
	}

	if (derr.IsntError())
	{
		result = RegEnumKeyEx(regHandle, index, keyName, &valueSize, NULL, 
			NULL, NULL, NULL);

		if (result == ERROR_NO_MORE_ITEMS)
		{
			(* isIndexValid) = FALSE;
		}
		else if (result != ERROR_SUCCESS)
		{
			derr = DualErr(kPGDMinorError_RegEnumKeyExFailed, result);
		}
		else
		{
			(* isIndexValid) = TRUE;
		}
	}

	if (openedKey)
		RegCloseKey(regHandle);

	return derr;
}

// PGPdiskEnumValues enumerates the names and types of the values under a
// given key. Start with 'index'  as 0 and then increment until
// 'isIndexValid' is set to FALSE.

DualErr 
PGPdiskEnumValues(
	HKEY			root, 
	LPCSTR			section, 
	PGPUInt32		index, 
	LPSTR			valueName, 
	unsigned long	valueNameSize, 
	unsigned long	*valueType, 
	PGPBoolean		*isIndexValid)
{
	DualErr		derr;
	HKEY		regHandle;
	PGPBoolean	openedKey	= FALSE;
	PGPUInt32	result;

	pgpAssertStrValid(section);
	pgpAssertAddrValid(valueName, char);
	pgpAssertAddrValid(isIndexValid, PGPBoolean);

	if (IsntNull(valueType))
		pgpAssertAddrValid(valueType, unsigned long);

	result = RegOpenKeyEx(root, section, NULL, KEY_READ, &regHandle);

	if (result != ERROR_SUCCESS)
	{
		derr = DualErr(kPGDMinorError_RegOpenKeyExFailed, result);
		openedKey = derr.IsntError();
	}

	if (derr.IsntError())
	{
		result = RegEnumValue(regHandle, index, valueName, &valueNameSize, 
			NULL, valueType, NULL, NULL);

		if (result == ERROR_NO_MORE_ITEMS)
		{
			(* isIndexValid) = FALSE;
		}
		else if (result != ERROR_SUCCESS)
		{
			derr = DualErr(kPGDMinorError_RegEnumValueFailed, result);
		}
		else
		{
			(* isIndexValid) = TRUE;
		}
	}

	if (openedKey)
		RegCloseKey(regHandle);

	return derr;
}

// PGPdiskGetRawKey returns the data associated with the specified registry
// key in the specified buffer.

DualErr 
PGPdiskGetRawKey(
	HKEY			root, 
	LPCSTR			section, 
	LPCSTR			keyName, 
	PGPUInt8		*buf, 
	unsigned long	bufSize)
{
	DualErr			derr;
	HKEY			regHandle;
	PGPBoolean		openedKey	= FALSE;
	PGPUInt32		result;
	unsigned long	valueType;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssertStrValid(section);
	pgpAssertStrValid(keyName);

	result = RegOpenKeyEx(root, section, NULL, KEY_READ, &regHandle);

	if (result != ERROR_SUCCESS)
	{
		derr = DualErr(kPGDMinorError_RegOpenKeyExFailed, result);
	}

	openedKey = derr.IsntError();

	if (derr.IsntError())
	{
		result = RegQueryValueEx(regHandle, keyName, NULL, &valueType, 
			buf, &bufSize);

		if (result != ERROR_SUCCESS)
			derr = DualErr(kPGDMinorError_RegQueryValueExFailed, result);
	}

	if (openedKey)
		RegCloseKey(regHandle);

	return derr;
}

// PGPdiskCreateAndSetRawKey creats and sets the value of any key in the
// registry, according to the parameters. It is a wrapper around
// RegCreateKeyEx and RegSetValueEx.

DualErr 
PGPdiskCreateAndSetRawKey(
	HKEY			root, 
	LPCSTR			section, 
	LPCSTR			keyName, 
	const PGPUInt8	*value, 
	PGPUInt32		valueSize, 
	PGPUInt32		dataType)
{
	DualErr			derr;
	HKEY			regHandle;
	PGPBoolean		openedKey	= FALSE;
	PGPUInt32		result;
	unsigned long	disp;

	pgpAssertStrValid(section);
	pgpAssertAddrValid(value, PGPUInt8);

	result = RegCreateKeyEx(root, section, 0, NULL, NULL, KEY_ALL_ACCESS, 
		NULL, &regHandle, &disp);

	if (result != ERROR_SUCCESS)
	{
		derr = DualErr(kPGDMinorError_RegCreateKeyExFailed, result);
	}

	openedKey = derr.IsntError();

	if (derr.IsntError())
	{
		result = RegSetValueEx(regHandle, keyName, 0, dataType, value, 
			valueSize);

		if (result != ERROR_SUCCESS)
			derr = DualErr(kPGDMinorError_RegSetValueExFailed, result);
	}

	if (openedKey)
		RegCloseKey(regHandle);

	return derr;
}

// PGPdiskDeleteSubkey deletes the specified subkey.

DualErr 
PGPdiskDeleteSubkey(HKEY root, LPCSTR section, LPCSTR keyName)
{
	DualErr		derr;
	HKEY		regHandle;
	PGPBoolean	openedKey	= FALSE;
	PGPUInt32	result;

	pgpAssertStrValid(section);
	pgpAssertStrValid(keyName);

	result = RegOpenKeyEx(root, section, NULL, KEY_ALL_ACCESS, &regHandle);

	if (result != ERROR_SUCCESS)
	{
		derr = DualErr(kPGDMinorError_RegOpenKeyExFailed, result);
		openedKey = derr.IsntError();
	}

	if (derr.IsntError())
	{
		result = RegDeleteKey(regHandle, keyName);

		if (result != ERROR_SUCCESS)
			derr = DualErr(kPGDMinorError_RegDeleteKeyFailed, result);
	}


	if (openedKey)
		RegCloseKey(regHandle);

	return derr;
}

// PGPdiskDeleteSubkey deletes the specified value.

DualErr 
PGPdiskDeleteValue(HKEY root, LPCSTR section, LPCSTR valueName)
{
	DualErr		derr;
	HKEY		regHandle;
	PGPBoolean	openedKey	= FALSE;
	PGPUInt32	result;

	pgpAssertStrValid(section);
	pgpAssertStrValid(valueName);

	result = RegOpenKeyEx(root, section, NULL, KEY_ALL_ACCESS, &regHandle);

	if (result != ERROR_SUCCESS)
	{
		derr = DualErr(kPGDMinorError_RegOpenKeyExFailed, result);
		openedKey = derr.IsntError();
	}

	if (derr.IsntError())
	{
		result = RegDeleteValue(regHandle, valueName);

		if (result != ERROR_SUCCESS)
			derr = DualErr(kPGDMinorError_RegDeleteValueFailed, result);
	}


	if (openedKey)
		RegCloseKey(regHandle);

	return derr;
}


////////////////////////////
// Mid-Level Registry Access
////////////////////////////

// PGPdiskGetPrefDWord gets the DWORD value of a key from the PGPdisk
// preferences. If there is an error, 'defaultDWord' is returned.

PGPUInt32 
PGPdiskGetPrefDWord(
	LPCSTR				regKey, 
	PGPUInt32			defaultDWord, 
	PGPdiskRegLocation	regLoc)
{
	CString		regSection;
	DualErr		derr;
	HKEY		regRoot;
	PGPUInt32	theDWord;

	try
	{
		switch (regLoc)
		{
		case kPRL_CurrentUser:
			regRoot = HKEY_CURRENT_USER;
			regSection = kRegistryLocalPath;
			break;

		case kPRL_LocalMachine:
			regRoot = HKEY_LOCAL_MACHINE;
			regSection = kRegistryGlobalPath;
			break;

		case kPRL_PGP60CurrentUser:
			regRoot = HKEY_CURRENT_USER;
			regSection = kRegistryLocalPGP60Path;
			break;

		default:
			pgpAssert(FALSE);
			break;
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	if (derr.IsntError())
	{
		derr = PGPdiskGetRawKey(regRoot, regSection, regKey, 
			(PGPUInt8 *) &theDWord, sizeof(theDWord));
	}

	if (derr.IsError())
	{
		return defaultDWord;
	}
	else
	{
		return theDWord;
	}
}

// PGPdiskSetPrefDWord sets the DWORD value of a key from the PGPdisk
// preferences.

DualErr 
PGPdiskSetPrefDWord(
	LPCSTR				regKey, 
	PGPUInt32			newDWord, 
	PGPdiskRegLocation	regLoc)
{
	CString	regSection;
	HKEY	regRoot;
	DualErr	derr;

	try
	{
		switch (regLoc)
		{
		case kPRL_CurrentUser:
			regRoot = HKEY_CURRENT_USER;
			regSection = kRegistryLocalPath;
			break;

		case kPRL_LocalMachine:
			regRoot = HKEY_LOCAL_MACHINE;
			regSection = kRegistryGlobalPath;
			break;

		case kPRL_PGP60CurrentUser:
			regRoot = HKEY_CURRENT_USER;
			regSection = kRegistryLocalPGP60Path;
			break;

		default:
			pgpAssert(FALSE);
			break;
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	if (derr.IsntError())
	{
		derr = PGPdiskCreateAndSetRawKey(regRoot, regSection, regKey, 
			(PGPUInt8 *) &newDWord, sizeof(newDWord), REG_DWORD);
	}

	return derr;
}

// PGPdiskGetPrefQWord gets the 64-bit QWord value of a key from the PGPdisk
// preferences. If there is an error, 'defaultQWord' is returned.

PGPUInt64 
PGPdiskGetPrefQWord(
	LPCSTR				regKey, 
	PGPUInt64			defaultQWord, 
	PGPdiskRegLocation	regLoc)
{
	CString		regSection;
	DualErr		derr;
	HKEY		regRoot;
	PGPUInt64	theQWord;

	try
	{
		switch (regLoc)
		{
		case kPRL_CurrentUser:
			regRoot = HKEY_CURRENT_USER;
			regSection = kRegistryLocalPath;
			break;

		case kPRL_LocalMachine:
			regRoot = HKEY_LOCAL_MACHINE;
			regSection = kRegistryGlobalPath;
			break;

		case kPRL_PGP60CurrentUser:
			regRoot = HKEY_CURRENT_USER;
			regSection = kRegistryLocalPGP60Path;
			break;

		default:
			pgpAssert(FALSE);
			break;
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	if (derr.IsntError())
	{
		derr = PGPdiskGetRawKey(regRoot, regSection, regKey, 
			(PGPUInt8 *) &theQWord, sizeof(theQWord));
	}

	if (derr.IsError())
	{
		return defaultQWord;
	}
	else
	{
		return theQWord;
	}
}

// PGPdiskSetPrefQWord sets the QWORD value of a key from the PGPdisk
// preferences.

DualErr 
PGPdiskSetPrefQWord(
	LPCSTR				regKey, 
	PGPUInt64			newQWord, 
	PGPdiskRegLocation	regLoc)
{
	CString	regSection;
	HKEY	regRoot;
	DualErr	derr;

	try
	{
		switch (regLoc)
		{
		case kPRL_CurrentUser:
			regRoot = HKEY_CURRENT_USER;
			regSection = kRegistryLocalPath;
			break;

		case kPRL_LocalMachine:
			regRoot = HKEY_LOCAL_MACHINE;
			regSection = kRegistryGlobalPath;
			break;

		case kPRL_PGP60CurrentUser:
			regRoot = HKEY_CURRENT_USER;
			regSection = kRegistryLocalPGP60Path;
			break;

		default:
			pgpAssert(FALSE);
			break;
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	if (derr.IsntError())
	{
		derr = PGPdiskCreateAndSetRawKey(regRoot, regSection, regKey, 
			(PGPUInt8 *) &newQWord, sizeof(newQWord), REG_BINARY);
	}

	return derr;
}

// PGPdiskGetPrefString gets the string value of a key from the PGPdisk
// preferences.

void 
PGPdiskGetPrefString(
	LPCSTR				regKey, 
	LPCSTR				defaultString, 
	CString				*outputString,  
	PGPdiskRegLocation	regLoc)
{
	CString	regSection;
	HKEY	regRoot;
	DualErr	derr;

	pgpAssertAddrValid(outputString, CString);

	try
	{
		switch (regLoc)
		{
		case kPRL_CurrentUser:
			regRoot = HKEY_CURRENT_USER;
			regSection = kRegistryLocalPath;
			break;

		case kPRL_LocalMachine:
			regRoot = HKEY_LOCAL_MACHINE;
			regSection = kRegistryGlobalPath;
			break;

		case kPRL_PGP60CurrentUser:
			regRoot = HKEY_CURRENT_USER;
			regSection = kRegistryLocalPGP60Path;
			break;

		default:
			pgpAssert(FALSE);
			break;
		}

		if (derr.IsntError())
		{
			derr = PGPdiskGetRawKey(regRoot, regSection, regKey, 
				(PGPUInt8 *) outputString->GetBuffer(kMaxStringSize), 
				kMaxStringSize);

			outputString->ReleaseBuffer();
		}

		if (derr.IsError())
		{
			(* outputString) = defaultString;
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}
}

// PGPdiskSetPrefString sets the string value of a key from the PGPdisk
// preferences.

DualErr 
PGPdiskSetPrefString(
	LPCSTR				regKey, 
	LPCSTR				newString, 
	PGPdiskRegLocation	regLoc)
{
	CString	regSection;
	HKEY	regRoot;
	DualErr	derr;

	pgpAssertStrValid(newString);

	try
	{
		switch (regLoc)
		{
		case kPRL_CurrentUser:
			regRoot = HKEY_CURRENT_USER;
			regSection = kRegistryLocalPath;
			break;

		case kPRL_LocalMachine:
			regRoot = HKEY_LOCAL_MACHINE;
			regSection = kRegistryGlobalPath;
			break;

		case kPRL_PGP60CurrentUser:
			regRoot = HKEY_CURRENT_USER;
			regSection = kRegistryLocalPGP60Path;
			break;

		default:
			pgpAssert(FALSE);
			break;
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	if (derr.IsntError())
	{
		derr = PGPdiskCreateAndSetRawKey(regRoot, regSection, regKey, 
			(PGPUInt8 *) newString, strlen(newString), REG_SZ);
	}

	return derr;
}

/////////////////////////////
// High-Level Registry Access
/////////////////////////////

// RecallWindowPos recalls the position of a given window from the given
// registry key.

void 
RecallWindowPos(CWnd *pWnd, LPCSTR wndCoordsKey)
{
	PGPUInt32 coords;

	pgpAssertAddrValid(pWnd, CWnd);
	pgpAssertStrValid(wndCoordsKey);

	coords = PGPdiskGetPrefDWord(wndCoordsKey, kInvalidRegDWordValue);

	// Move the window if the key exists.
	if (coords != kInvalidRegDWordValue)
	{
		PGPUInt16 x = GetLowWord(coords);
		PGPUInt16 y = GetHighWord(coords);

		if ((x >= 0) && (y >= 0) &&
			(x < GetSystemMetrics(SM_CXSCREEN)) &&
			(y < GetSystemMetrics(SM_CYSCREEN)))
		{
			pWnd->SetWindowPos(&CWnd::wndTop, x, y, 0, 0, 
				SWP_NOSIZE | SWP_NOZORDER);
		}
	}
}

// SaveWindowPos saves the position of a given window in the given registry
// key.

void 
SaveWindowPos(CWnd *pWnd, LPCSTR wndCoordsKey)
{
	PGPUInt32	coords;
	RECT		rect;

	pgpAssertAddrValid(pWnd, Wnd);
	pgpAssertStrValid(wndCoordsKey);

	pWnd->GetWindowRect(&rect);
	coords = MakeLong((PGPUInt16) rect.left, (PGPUInt16) rect.top);

	PGPdiskSetPrefDWord(wndCoordsKey, coords);
}

#if PGPDISK_BETAVERSION

// HasBetaTimedOut returns TRUE if this beta version of PGPdisk has timed
// out, FALSE otherwise.

PGPBoolean 
HasBetaTimedOut()
{
	PGPUInt64 currentTime, timeoutTime;

	// Get the current time.
	currentTime = GetSecondsSince1970();

	// Get the timeout time.
	timeoutTime = PGPdiskGetPrefQWord(kRegistryBetaTimeoutKey, 
		kInvalidRegQWordValue);

	// If we've exceeded the timeout data, return TRUE.
	if (timeoutTime != kInvalidRegQWordValue)
		return (currentTime >= timeoutTime);
	else
		return FALSE;
}

// SetPGPdiskBetaTimeout burns a beta timeout date into the registry, but
// only if no date is there already.

DualErr 
SetPGPdiskBetaTimeout()
{
	DualErr		derr;
	PGPUInt64	timeoutTime;

	timeoutTime = PGPdiskGetPrefQWord(kRegistryBetaTimeoutKey, 
		kInvalidRegQWordValue);

	// If no demo timeout has been set, set it now.
	if (timeoutTime == kInvalidRegQWordValue)
	{
		FILETIME	fileTimeoutTime;
		SYSTEMTIME	sysTimeoutTime;

		// Prepare data structure with target timeout date.
		pgpClearMemory(&sysTimeoutTime, sizeof(sysTimeoutTime));

		sysTimeoutTime.wYear	= kPGPdiskBetaTimeoutYear;
		sysTimeoutTime.wMonth	= kPGPdiskBetaTimeoutMonth;
		sysTimeoutTime.wDay		= kPGPdiskBetaTimeoutDay;

		// Convert it to a 64-bit value.
		SystemTimeToFileTime(&sysTimeoutTime, &fileTimeoutTime);

		// Convert it to standard form.
		timeoutTime = ConvertFileTimeToSecondsSince1970(&fileTimeoutTime);

		// Set the timeout time.
		derr = PGPdiskSetPrefQWord(kRegistryBetaTimeoutKey, timeoutTime);
	}

	return derr;
}

#elif PGPDISK_DEMOVERSION

// HasDemoTimedOut returns TRUE if this demo version of PGPdisk has timed
// out, FALSE otherwise.

PGPBoolean 
HasDemoTimedOut()
{
	PGPUInt64 currentTime, timeoutTime;

	// Get the current time.
	currentTime = GetSecondsSince1970();

	// Get the timeout time.
	timeoutTime = PGPdiskGetPrefQWord(kRegistryDemoTimeoutKey, 
		kInvalidRegQWordValue);

	// If we've exceeded the timeout data, return TRUE.
	if (timeoutTime != kInvalidRegQWordValue)
		return (currentTime >= timeoutTime);
	else
		return FALSE;
}

// SetPGPdiskDemoTimeout burns a demo timeout date into the registry, but
// only if no date is there already.

DualErr 
SetPGPdiskDemoTimeout()
{
	DualErr		derr;
	PGPUInt64	timeoutTime;

	timeoutTime = PGPdiskGetPrefQWord(kRegistryDemoTimeoutKey, 
		kInvalidRegQWordValue);

	// If no demo timeout has been set, set it now.
	if (timeoutTime == kInvalidRegQWordValue)
	{
		// Determine the timeout time.
		timeoutTime = GetSecondsSince1970() + kDemoTimeoutDays*kSecondsInADay;

		// Set the timeout time.
		derr = PGPdiskSetPrefQWord(kRegistryDemoTimeoutKey, timeoutTime);
	}

	return derr;
}

#endif // PGPDISK_BETAVERSION
