//////////////////////////////////////////////////////////////////////////////
// CPGPdiskInstallHelperDll.cpp
//
// Helper functions called by the PGPdisk installer.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskInstallHelperDll.cpp,v 1.4 1998/12/14 19:00:33 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"

#include "DriverComm.h"
#include "LinkResolution.h"
#include "Packets.h"
#include "PGPdiskRegistry.h"
#include "PGPdiskResidentDefines.h"
#include "SharedMemory.h"
#include "UtilityFunctions.h"
#include "WindowsVersion.h"

#include "CPGPdiskInstallHelperDll.h"


////////////
// Constants
////////////

// Data for spurious keys and values that need cleaning up.

const HKEY		kRegistrySpurious1Root			= HKEY_LOCAL_MACHINE;
static LPCSTR	kRegistrySpurious1Section		= "Enum\\ESDI";
static LPCSTR	kRegistrySpurious1SubkeyMatch	= "PGPDISK";

const HKEY		kRegistrySpurious2Root			= HKEY_LOCAL_MACHINE;
static LPCSTR	kRegistrySpurious2Section		= 
	"System\\CurrentControlSet\\Control\\ASD\\Prob";
static LPCSTR	kRegistrySpurious2ValueMatch	= "PGPDISK";

const HKEY		kRegistrySpurious3Root			= HKEY_DYN_DATA;
static LPCSTR	kRegistrySpurious3Section		= "Config Manager\\Enum";
static LPCSTR	kRegistrySpurious3Value			= "HardWareKey";
static LPCSTR	kRegistrySpurious3StringMatch	= "PGPDISK";


///////////////////////////
// Global variables for MFC
///////////////////////////

CPGPdiskInstallHelperDll theApp;


///////////////////////////
// MFC specific definitions
///////////////////////////

// MFC message map

BEGIN_MESSAGE_MAP(CPGPdiskInstallHelperDll, CWinApp)
	//{{AFX_MSG_MAP(CPGPdiskInstallHelperDll)
	//}}AFX_MSG
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////
// CPGPdiskInstallHelperDll public custom functions and non-default handlers
////////////////////////////////////////////////////////////////////////////

// The CPGPdiskInstallHelperDll constructor.

CPGPdiskInstallHelperDll::CPGPdiskInstallHelperDll()
{
	ExplicitlyLinkAPIsForPlatform();
}

// The CPGPdiskInstallHelperDll destructor.

CPGPdiskInstallHelperDll::~CPGPdiskInstallHelperDll()
{
	CleanupExplicitLinkage();
}

// ExitInstance is called when the DLL is unloading.

int 
CPGPdiskInstallHelperDll::ExitInstance()
{
	DualErr derr;

	CleanupExplicitLinkage();

	return CWinApp::ExitInstance();
}


// InitInstance is called during DLL initialization.

BOOL 
CPGPdiskInstallHelperDll::InitInstance()
{
	DualErr derr;

	derr = CheckWindowsVersion();

	if (derr.IsntError())
	{
		derr = ExplicitlyLinkAPIsForPlatform();
	}

	// DON'T bug user on error, just fail to load.
	return derr.IsntError();
}


/////////////////////////////////////////////////////////////////////////////
// CPGPdiskInstallHelperDll private custom functions and non-default handlers
/////////////////////////////////////////////////////////////////////////////

// CheckWindowsVersion checks if we are running an incompatible version of
// Windows.

DualErr 
CPGPdiskInstallHelperDll::CheckWindowsVersion()
{
	DualErr derr;

	if (!IsWin95CompatibleMachine() && !IsWinNT4CompatibleMachine())
	{
		derr = DualErr(kPGDMinorError_NotCompatibleWindows);
	}

	return derr;
}


/////////////////////
// Exported functions
/////////////////////

// KillPGPdiskResident cleans up extraneous PGPdisk registry entries.

void 
__stdcall 
KillPGPdiskResident()
{
	DualErr			derr;
	PGPBoolean		attachedMem	= FALSE;
	PGPdiskResInfo	*pgpDiskResInfo;
	SharedMemory	pgpDiskResInfoMem;

	// Try to find an existing PGPdiskResident application.
	derr = pgpDiskResInfoMem.Attach(kPGPdiskResSharedMemName);
	attachedMem = derr.IsntError();

	// If found, kill it.
	if (derr.IsntError())
	{
		pgpDiskResInfo = (PGPdiskResInfo *) 
			pgpDiskResInfoMem.GetMemPointer();

		PostMessage(pgpDiskResInfo->hiddenWindowHwnd, WM_QUIT, NULL, NULL);
	}

	if (attachedMem)
		pgpDiskResInfoMem.Detach();
}

// PGPdiskCleanup cleans up extraneous PGPdisk registry entries.

void 
__stdcall 
PGPdiskCleanup()
{
	try
	{
		CString		keyName, keyPath, valueString;
		DualErr		derr;
		PGPBoolean	isIndexValid;
		PGPUInt32	i, j;

		// Wipe the "PGPDISK" subtree under "HKLM\Enum\ESDI".
		i = 0;

		while (derr.IsntError())
		{
			derr = PGPdiskEnumSubkeys(kRegistrySpurious1Root, 
				kRegistrySpurious1Section, i, 
				keyName.GetBuffer(kMaxStringSize), kMaxStringSize, 
				&isIndexValid);

			keyName.ReleaseBuffer();

			if (derr.IsntError())
			{
				if (!isIndexValid)
					break;

				if (keyName.Find(kRegistrySpurious1SubkeyMatch) == -1)
				{
					i++;
					continue;
				}
				else
				{
					derr = PGPdiskDeleteSubkey(kRegistrySpurious1Root, 
						kRegistrySpurious1Section, keyName);
					break;
				}
			}
		}

		derr = DualErr::NoError;

		// Wipe all values containing "PGPDISK" in all keys that reside under
		// "HKLM\System\CurrentControlSet\control\ASD\Prob".

		i = 0;

		while (derr.IsntError())
		{
			derr = PGPdiskEnumSubkeys(kRegistrySpurious2Root, 
				kRegistrySpurious2Section, i, 
				keyName.GetBuffer(kMaxStringSize), kMaxStringSize, 
				&isIndexValid);

			keyName.ReleaseBuffer();

			if (derr.IsntError())
			{
				CString valueName;

				j = 0;

				if (!isIndexValid)
					break;

				keyPath	= kRegistrySpurious2Section;
				keyPath += "\\";
				keyPath	+= keyName;

				// Enumerate all values under this key.
				while (derr.IsntError())
				{
					derr = PGPdiskEnumValues(kRegistrySpurious2Root, 
						keyPath, j, valueName.GetBuffer(kMaxStringSize), 
						kMaxStringSize, NULL, &isIndexValid);

					valueName.ReleaseBuffer();
					
					if (derr.IsntError())
					{
						if (!isIndexValid)
							break;

						if (valueName.Find(kRegistrySpurious2ValueMatch) == 
							-1)
						{
							j++;
							continue;
						}
						else
						{
							derr = PGPdiskDeleteValue(kRegistrySpurious2Root, 
								keyPath, valueName);
							j = 0;
						}
					}
				}
			}

			i++;
		}

		derr = DualErr::NoError;

		// Wipe all keys with a "HardWareKey" value containing "PGPDISK"
		// under "HDD\Config Manager\Enum".

		i = 0;

		while (derr.IsntError())
		{
			derr = PGPdiskEnumSubkeys(kRegistrySpurious3Root, 
				kRegistrySpurious3Section, i, 
				keyName.GetBuffer(kMaxStringSize), kMaxStringSize, 
				&isIndexValid);

			keyName.ReleaseBuffer();

			if (derr.IsntError())
			{
				if (!isIndexValid)
					break;

				keyPath	= kRegistrySpurious3Section;
				keyPath += "\\";
				keyPath	+= keyName;

				derr = PGPdiskGetRawKey(kRegistrySpurious3Root, keyPath, 
					kRegistrySpurious3Value, 
					(PGPUInt8 *) valueString.GetBuffer(kMaxStringSize), 
					kMaxStringSize);

				valueString.ReleaseBuffer();
			}

			if (derr.IsntError())
			{
				if (valueString.Find(kRegistrySpurious3StringMatch) == -1)
				{
					i++;
					continue;
				}
				else
				{
					derr = PGPdiskDeleteSubkey(kRegistrySpurious3Root, 
						kRegistrySpurious3Section, keyName);
					i = 0;
				}
			}
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// AreAnyPGPdisksMounted returns TRUE if any PGPdisks are mounted, FALSE
// otherwise.

BOOLEAN 
__stdcall 
AreAnyPGPdisksMounted()
{
	DualErr		derr;
	PGPBoolean	anyMounted	= FALSE;

	// Open the driver.
	derr = OpenPGPdiskDriver();

	if (derr.IsntError())
	{
		PGPUInt8 i;

		// Check if any drive letter is a mounted PGPdisk.
		for (i = 0; i < kMaxDrives; i++)
		{
			derr = IsVolumeAPGPdisk(i, &anyMounted);

			if (derr.IsError())
				break;

			if (anyMounted)
				break;
		}
	}

	// Close the driver.
	if (IsDriverOpen())
		ClosePGPdiskDriver();

	if (derr.IsntError())
		return anyMounted;
	else
		return FALSE;
}

// IsVolumeAPGPdisk returns TRUE if the specified volume is a mounted
// PGPdisk.

BOOLEAN 
__stdcall 
IsVolumeAPGPdisk(int drive)
{
	DualErr		derr;
	PGPBoolean	isVolumeAPGPdisk;

	pgpAssert(IsLegalDriveNumber(drive));

	// Open the driver.
	derr = OpenPGPdiskDriver();

	// Check if drive is PGPdisk.
	if (derr.IsntError())
	{
		derr = IsVolumeAPGPdisk(drive, &isVolumeAPGPdisk);
	}

	// Close the driver.
	if (IsDriverOpen())
		ClosePGPdiskDriver();

	if (derr.IsntError())
		return isVolumeAPGPdisk;
	else
		return FALSE;
}

// UnmountAllPGPdisks returns TRUE if all mounted PGPdisks were successfully
// unmounted, FALSE otherwise.

BOOLEAN 
__stdcall 
UnmountAllPGPdisks()
{
	DualErr derr;

	if (!AreAnyPGPdisksMounted())
		return TRUE;

	// Open the driver.
	derr = OpenPGPdiskDriver();

	if (derr.IsntError())
	{
		PGPUInt8	i;
		PGPUInt32	drives;

		drives = GetLogicalDrives();

		// For every drive...
		for (i = 0; i < kMaxDrives; i++)
		{
			if (drives & (1 << i))
			{
				PGPBoolean isVolumeAPGPdisk = FALSE;

				//... ask if it's a mounted PGPdisk.
				IsVolumeAPGPdisk(i, &isVolumeAPGPdisk);

				// If so, unmount it.
				if (isVolumeAPGPdisk)
				{
					AD_Unmount UNMNT;

					UNMNT.drive = i;
					UNMNT.isThisEmergency = FALSE;

					derr = SendUnmountRequest(&UNMNT);
				}
			}

			if (derr.IsError())
				break;
		}
	}

	// Close the driver.
	if (IsDriverOpen())
		ClosePGPdiskDriver();

	return derr.IsntError();
}
