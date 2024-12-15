//////////////////////////////////////////////////////////////////////////////
// KernelModeUtils.cpp
//
// Kernel-Mode utility functions.
//////////////////////////////////////////////////////////////////////////////

// $Id: KernelModeUtils.cpp,v 1.4 1999/03/31 23:51:08 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#define	__w64
#include <vdw.h>
#include <Devioctl.h>

#include "Required.h"
#include "UtilityFunctions.h"

#include "KernelModeUtils.h"
#include "File.h"
#include "Globals.h"
#include "Volume.h"


///////////////////////////
// String utility functions
///////////////////////////

// UniToAnsi converts a Unicode string to an ANSI string.

DualErr 
UniToAnsi(KUstring *uniString, LPSTR *string)
{
	ANSI_STRING	ansiString;
	DualErr		derr;
	NTSTATUS	status;
	PGPBoolean	allocedString;
	PGPUInt16	length;

	pgpAssertAddrValid(uniString, KUstring);
	pgpAssertAddrValid(string, char);

	length = uniString->Length() + 1;

	// Get space for new string.
	ansiString.Buffer = new char[length];

	if (IsNull(ansiString.Buffer))
		derr = DualErr(kPGDMinorError_OutOfMemory);

	allocedString = derr.IsntError();

	// Perform the conversion.
	if (derr.IsntError())
	{
		ansiString.MaximumLength	= length;
		ansiString.Length			= length - 1;

		status = RtlUnicodeStringToAnsiString(&ansiString, (* uniString), 
			FALSE);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_RtlUniToAnsiFailed, status);
	}

	// Output the string.
	if (derr.IsntError())
	{
		(* string) = ansiString.Buffer;
	}

	if (derr.IsError())
	{
		if (allocedString)
			delete[] ansiString.Buffer;
	}

	return derr;
}

// AssignToUni (... PUNICODE_STRING inUniString) assigns one unicode string
// to another.

DualErr 
AssignToUni(
	KUstring		*outUniString, 
	PUNICODE_STRING	inUniString)
{
	DualErr		derr;
	NTSTATUS	status;
	PGPUInt16	length;

	pgpAssertAddrValid(outUniString, KUstring);
	pgpAssertAddrValid(inUniString, UNICODE_STRING);

	length = inUniString->Length/sizeof(WCHAR) + 1;

	// Allocate more space if necessary.
	if (length > outUniString->MaximumLength())
	{
		status = 
			outUniString->GrowMaxBy(length - outUniString->MaximumLength(), 
				NonPagedPool);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_OutOfMemory, status);
	}

	// Perform the assignment.
	if (derr.IsntError())
	{
		status = outUniString->Assign(KUstring(inUniString, FALSE));

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_UniStringOpFailed, status);
	}

	return derr;
}

// AssignToUni (... LPCWSTR inUniString) assigns one unicode string to
// another.

DualErr 
AssignToUni(
	KUstring	*outUniString, 
	LPCWSTR		inUniString)
{
	DualErr		derr;
	NTSTATUS	status;
	PGPUInt16	length;

	pgpAssertAddrValid(outUniString, KUstring);
	pgpAssertAddrValid(inUniString, WCHAR);

	length = wcslen(inUniString) + 1;

	// Allocate more space if necessary.
	if (length > outUniString->MaximumLength())
	{
		status = 
			outUniString->GrowMaxBy(length - outUniString->MaximumLength(), 
			NonPagedPool);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_OutOfMemory, status);
	}

	// Perform the assignment.
	if (derr.IsntError())
	{
		status = outUniString->Assign(inUniString);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_UniStringOpFailed, status);
	}

	return derr;
}

// AssignToUni (... LPCSTR inString) assigns an ANSI string to a unicode
// string.

DualErr 
AssignToUni(
	KUstring	*outUniString, 
	LPCSTR		inString)
{
	ANSI_STRING	ansiString;
	DualErr		derr;
	NTSTATUS	status;
	PGPUInt16	length;

	pgpAssertStrValid(inString);
	pgpAssertAddrValid(outUniString, KUstring);

	length = strlen(inString) + 1;

	// Allocate more space if necessary.
	if (length > outUniString->MaximumLength())
	{
		status = 
			outUniString->GrowMaxBy(length - outUniString->MaximumLength(), 
				NonPagedPool);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_OutOfMemory, status);
	}

	// Perform the conversion.
	if (derr.IsntError())
	{
		RtlInitAnsiString(&ansiString, inString);

		status = RtlAnsiStringToUnicodeString((* outUniString), &ansiString, 
			FALSE);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_RtlAnsiToUniFailed, status);
	}

	return derr;
}

// AppendToUni (... PUNICODE_STRING inUniString) appends one unicode string
// to another.

DualErr 
AppendToUni(
	KUstring		*outUniString, 
	PUNICODE_STRING	inUniString)
{
	DualErr		derr;
	NTSTATUS	status;
	PGPUInt16	length;

	pgpAssertAddrValid(outUniString, KUstring);
	pgpAssertAddrValid(inUniString, UNICODE_STRING);

	length = outUniString->Length() + inUniString->Length/sizeof(WCHAR) + 1;

	// Allocate more space if necessary.
	if (length > outUniString->MaximumLength())
	{
		status = 
			outUniString->GrowMaxBy(length - outUniString->MaximumLength(), 
				NonPagedPool);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_OutOfMemory, status);
	}

	// Perform the append.
	if (derr.IsntError())
	{
		status = outUniString->Append(KUstring(inUniString, FALSE));

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_UniStringOpFailed, status);
	}

	return derr;
}

// AppendToUni (... LPCWSTR inUniString) appends one unicode string to
// another.

DualErr 
AppendToUni(
	KUstring	*outUniString, 
	LPCWSTR		inUniString)
{
	DualErr		derr;
	NTSTATUS	status;
	PGPUInt16	length;

	pgpAssertAddrValid(outUniString, KUstring);
	pgpAssertAddrValid(inUniString, WCHAR);

	length = outUniString->Length() + wcslen(inUniString) + 1;

	// Allocate more space if necessary.
	if (length > outUniString->MaximumLength())
	{
		status = 
			outUniString->GrowMaxBy(length - outUniString->MaximumLength(), 
				NonPagedPool);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_OutOfMemory, status);
	}

	// Perform the append.
	if (derr.IsntError())
	{
		status = outUniString->Append(inUniString);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_UniStringOpFailed, status);
	}

	return derr;
}

// AppendToUni (... LPCSTR inString) appends an ANSI string to a unicode
// string.

DualErr 
AppendToUni(
	KUstring	*outUniString, 
	LPCSTR		inString)
{
	DualErr		derr;
	KUstring	tempUniString;
	NTSTATUS	status;
	PGPUInt16	length;

	pgpAssertStrValid(inString);
	pgpAssertAddrValid(outUniString, KUstring);

	length = outUniString->Length() + strlen(inString) + 1;

	// Allocate more space if necessary.
	if (length > outUniString->MaximumLength())
	{
		status = 
			outUniString->GrowMaxBy(length - outUniString->MaximumLength(), 
				NonPagedPool);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_OutOfMemory, status);
	}

	// Perform the conversion.
	if (derr.IsntError())
	{
		derr = AssignToUni(&tempUniString, inString);
	}

	// Perform the append
	if (derr.IsntError())
	{
		status = outUniString->Append(tempUniString);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_UniStringOpFailed, status);
	}

	return derr;
}

// PrependToUni (... PUNICODE_STRING inUniString) prepends one unicode string
// to another.

DualErr 
PrependToUni(
	KUstring		*outUniString, 
	PUNICODE_STRING	inUniString)
{
	DualErr		derr;
	KUstring	tempUniString;
	NTSTATUS	status;
	PGPUInt16	length;

	pgpAssertAddrValid(outUniString, KUstring);
	pgpAssertAddrValid(inUniString, UNICODE_STRING);

	length = outUniString->Length() + inUniString->Length/sizeof(WCHAR) + 1;

	// Allocate more space if necessary.
	if (length > outUniString->MaximumLength())
	{
		status = 
			outUniString->GrowMaxBy(length - outUniString->MaximumLength(), 
				NonPagedPool);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_OutOfMemory, status);
	}

	// Prepare temp variable.
	if (derr.IsntError())
	{
		derr = AssignToUni(&tempUniString, (* outUniString));
	}

	// Prepare output variable.
	if (derr.IsntError())
	{
		derr = AssignToUni(outUniString, inUniString);
	}

	// Perform the prepend.
	if (derr.IsntError())
	{
		status = outUniString->Append(KUstring(tempUniString, FALSE));

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_UniStringOpFailed, status);
	}

	return derr;
}

// PrependToUni (... LPCWSTR inUniString) prepends one unicode string to
// another.

DualErr 
PrependToUni(
	KUstring	*outUniString, 
	LPCWSTR		inUniString)
{
	DualErr		derr;
	KUstring	tempUniString;
	NTSTATUS	status;
	PGPUInt16	length;

	pgpAssertAddrValid(outUniString, KUstring);
	pgpAssertAddrValid(inUniString, WCHAR);

	length = outUniString->Length() + wcslen(inUniString) + 1;

	// Allocate more space if necessary.
	if (length > outUniString->MaximumLength())
	{
		status = outUniString->GrowMaxBy(length - 
			outUniString->MaximumLength(), NonPagedPool);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_OutOfMemory, status);
	}

	// Prepare temp variable.
	if (derr.IsntError())
	{
		derr = AssignToUni(&tempUniString, (* outUniString));
	}

	// Prepare output variable.
	if (derr.IsntError())
	{
		derr = AssignToUni(outUniString, inUniString);
	}

	// Perform the prepend.
	if (derr.IsntError())
	{
		status = outUniString->Append(tempUniString);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_UniStringOpFailed, status);
	}

	return derr;
}

// PrependToUni (... LPCSTR inString) prepends an ANSI string to a unicode
// string.

DualErr 
PrependToUni(
	KUstring	*outUniString, 
	LPCSTR		inString)
{
	DualErr		derr;
	KUstring	tempUniString;
	NTSTATUS	status;
	PGPUInt16	length;

	pgpAssertStrValid(inString);
	pgpAssertAddrValid(outUniString, KUstring);

	length = outUniString->Length() + strlen(inString) + 1;

	// Allocate more space if necessary.
	if (length > outUniString->MaximumLength())
	{
		status = 
			outUniString->GrowMaxBy(length - outUniString->MaximumLength(), 
				NonPagedPool);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_OutOfMemory, status);
	}

	// Prepare output variable.
	if (derr.IsntError())
	{
		derr = AssignToUni(outUniString, inString);
	}

	// Prepare temp variable.
	if (derr.IsntError())
	{
		derr = AssignToUni(&tempUniString, (* outUniString));
	}

	// Perform the append
	if (derr.IsntError())
	{
		status = outUniString->Append(tempUniString);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_UniStringOpFailed, status);
	}

	return derr;
}


/////////////////////////////////
// Device/drive utility functions
/////////////////////////////////

// MakePathToDrive returns a link pathname corresponding to the given
// drive.

DualErr 
MakePathToDrive(PGPUInt8 drive, KUstring *outPath)
{
	DualErr	derr;
	char	driveLet[3];

	pgpAssert(IsLegalDriveNumber(drive));
	pgpAssertAddrValid(outPath, KUstring);

	// Initialize the pathname.
	driveLet[0] = DriveNumToLet(drive);
	driveLet[1] = ':';
	driveLet[2] = '\0';

	derr = AssignToUni(outPath, driveLet);

	// Prepend the link qualifier.
	if (derr.IsntError())
	{
		derr = PrependToUni(outPath, kNTLinkPathPrefix);
	}

	return derr;
}

// IsValidDeviceName returns TRUE if the specified string refers to a valid
// device, FALSE otherwise.

PGPBoolean 
IsValidDeviceName(PUNICODE_STRING deviceName)
{
	PDEVICE_OBJECT	deviceObject;
	PFILE_OBJECT	fileObject;
	NTSTATUS		status;

	pgpAssertAddrValid(deviceName, UNICODE_STRING);

	status = IoGetDeviceObjectPointer(deviceName, FILE_READ_DATA, 
		&fileObject, &deviceObject);

	if (NT_SUCCESS(status))
	{
		ObDereferenceObject(fileObject);
		return TRUE;
	}
	else
	{
		return (status != STATUS_OBJECT_NAME_NOT_FOUND);
	}
}

// IsFileInUseByReader returns TRUE if someone has opened the specified file
// with read access, FALSE otherwise.

PGPBoolean 
IsFileInUseByReader(LPCSTR path)
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
IsFileInUseByWriter(LPCSTR path)
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
IsFileInUse(LPCSTR path)
{
	pgpAssertStrValid(path);

	return (IsFileInUseByReader(path) || IsFileInUseByWriter(path));
}


////////////////////////////
// Error/Debugging functions
////////////////////////////

// GetName parses a nametable and determines which string is associated with
// which constant, and returns it.

LPCSTR 
GetName(NameAssoc nameTable[], PGPUInt32 n, PGPUInt32 func)
{
	PGPUInt32 i;

	for (i = 0; i < n; i++)
	{
		if (func == nameTable[i].func)
			return nameTable[i].name;
	}

	return "<Unknown>";
}

// GetADPacketName returns a string with the name of the AD packet passed in
// 'code'.

LPCSTR 
GetADPacketName(PGPUInt32 code)
{
	static NameAssoc nameTable[] =
	{
		{kAD_Mount,					"kAD_Mount"},
		{kAD_Unmount,				"kAD_Unmount"},
		{kAD_QueryVersion,			"kAD_QueryVersion"},
		{kAD_QueryMounted,			"kAD_QueryMounted"},
		{kAD_QueryOpenFiles,		"kAD_QueryOpenFiles"},
		{kAD_ChangePrefs,			"kAD_ChangePrefs"},
		{kAD_LockUnlockMem,			"kAD_LockUnlockMem"}, 
		{kAD_GetPGPdiskInfo,		"kAD_GetPGPdiskInfo"}, 
		{kAD_LockUnlockVol,			"kAD_LockUnlockVol"}, 
		{kAD_ReadWriteVol,			"kAD_ReadWriteVol"}, 
		{kAD_QueryVolInfo,			"kAD_QueryVolInfo"}, 
		{kAD_NotifyUserLogoff,		"kAD_NotifyUserLogoff"}

	};

	return GetName(nameTable, (sizeof(nameTable) / sizeof(NameAssoc)), code);
}

// GetIOCTLFunctionName returns a string with the name of the IOCTL function
// passed in 'ioctlCode'.

LPCSTR 
GetIOCTLFunctionName(PGPUInt32 ioctlCode)
{
	static NameAssoc nameTable[] =
	{
		{IOCTL_DISK_FORMAT_TRACKS,			"IOCTL_DISK_FORMAT_TRACKS"}, 
		{IOCTL_DISK_FORMAT_TRACKS_EX,		"IOCTL_DISK_FORMAT_TRACKS_EX"}, 
		{IOCTL_DISK_VERIFY,					"IOCTL_DISK_VERIFY"}, 
		{IOCTL_DISK_GET_DRIVE_GEOMETRY,		"IOCTL_DISK_GET_DRIVE_GEOMETRY"}, 
		{IOCTL_DISK_GET_MEDIA_TYPES,		"IOCTL_DISK_GET_MEDIA_TYPES"}, 
		{IOCTL_DISK_CHECK_VERIFY,			"IOCTL_DISK_CHECK_VERIFY"}, 
		{IOCTL_STORAGE_CHECK_VERIFY,		"IOCTL_STORAGE_CHECK_VERIFY"}, 
		{IOCTL_DISK_GET_PARTITION_INFO,		"IOCTL_DISK_GET_PARTITION_INFO"}, 
		{IOCTL_DISK_SET_PARTITION_INFO,		"IOCTL_DISK_SET_PARTITION_INFO"}, 
		{IOCTL_DISK_GET_DRIVE_LAYOUT,		"IOCTL_DISK_GET_DRIVE_LAYOUT"}, 
		{IOCTL_DISK_SET_DRIVE_LAYOUT,		"IOCTL_DISK_SET_DRIVE_LAYOUT"}, 
		{IOCTL_DISK_IS_WRITABLE,			"IOCTL_DISK_IS_WRITABLE"}, 
		{IOCTL_DISK_REASSIGN_BLOCKS,		"IOCTL_DISK_REASSIGN_BLOCKS"}, 
		{IOCTL_DISK_FIND_NEW_DEVICES,		"IOCTL_DISK_FIND_NEW_DEVICES"}, 
		{IOCTL_DISK_MEDIA_REMOVAL,			"IOCTL_DISK_MEDIA_REMOVAL"}, 

		{IOCTL_STORAGE_FIND_NEW_DEVICES, 
			"IOCTL_STORAGE_FIND_NEW_DEVICES"}, 

		{IOCTL_DISK_PERFORMANCE,			"IOCTL_DISK_PERFORMANCE"}, 
		{SMART_GET_VERSION,					"SMART_GET_VERSION"}, 
		{SMART_RCV_DRIVE_DATA,				"SMART_RCV_DRIVE_DATA"}, 
		{SMART_SEND_DRIVE_COMMAND,			"SMART_SEND_DRIVE_COMMAND"}, 

		{IOCTL_DISK_INTERNAL_SET_VERIFY, 
			"IOCTL_DISK_INTERNAL_SET_VERIFY"}, 

		{IOCTL_DISK_INTERNAL_CLEAR_VERIFY, 
			"IOCTL_DISK_INTERNAL_CLEAR_VERIFY"}

	};

	return GetName(nameTable, (sizeof(nameTable) / sizeof(NameAssoc)), 
		ioctlCode);
}

// GetIRPMajorFunctionName returns a string with the name of the IRP major
// function passed in 'majorFunc'.

LPCSTR 
GetIRPMajorFunctionName(PGPUInt8 majorFunc)
{
	static NameAssoc nameTable[] =
	{
		{IRP_MJ_CREATE,						"IRP_MJ_CREATE"}, 
		{IRP_MJ_CLEANUP,					"IRP_MJ_CLEANUP"}, 
		{IRP_MJ_CLOSE,						"IRP_MJ_CLOSE"}, 
		{IRP_MJ_READ,						"IRP_MJ_READ"}, 
		{IRP_MJ_WRITE,						"IRP_MJ_WRITE"}, 
		{IRP_MJ_FLUSH_BUFFERS,				"IRP_MJ_FLUSH_BUFFERS"}, 
		{IRP_MJ_DEVICE_CONTROL,				"IRP_MJ_DEVICE_CONTROL"}, 
		{IRP_MJ_INTERNAL_DEVICE_CONTROL,	"IRP_MJ_INTERNAL_DEVICE_CONTROL"}

	};

	return GetName(nameTable, (sizeof(nameTable) / sizeof(NameAssoc)), 
		majorFunc);
}

PGPBoolean
IsThisAnNT5Machine()
{
	KRegistryKey	verKey(REGISTRY_WINDOWS_NT, L"");
	LPWSTR			verString	= NULL;
	NTSTATUS		status		= STATUS_SUCCESS;
	ULONG			length		= 0;

	status = verKey.QueryValue(L"CurrentVersion", verString, length, 
		NonPagedPool);

	if (NT_SUCCESS(status))
	{
		delete[] verString;
		return (verString[0] == '5');
	}
	else
	{
		return FALSE;
	}
}
