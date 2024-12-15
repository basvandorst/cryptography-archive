//////////////////////////////////////////////////////////////////////////////
// PGPdiskRegistry.cpp
//
// Functions for accessing the system registry.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskRegistry.cpp,v 1.5 1998/12/15 01:25:04 nryan Exp $

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
