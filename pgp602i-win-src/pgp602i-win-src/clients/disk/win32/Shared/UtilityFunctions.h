//////////////////////////////////////////////////////////////////////////////
// UtilityFunctions.h
//
// Small inline utility functions used by the application and the driver.
//////////////////////////////////////////////////////////////////////////////

// $Id: UtilityFunctions.h,v 1.1.2.31 1998/08/10 23:31:57 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_UtilityFunctions_h	// [
#define Included_UtilityFunctions_h

#include "DualErr.h"
#include "Packets.h"

#if defined(PGPDISK_NTDRIVER)
#include <stdio.h>
#include <stdlib.h>
#endif	// PGPDISK_NTDRIVER


/////////
// Macros
/////////

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

// Debugging macros.

#if !PGP_DEBUG

#define DebugOut NoOpPrintf

#elif defined(PGPDISK_MFC)

#define DebugOut TRACE

#elif defined(PGPDISK_95DRIVER)

#define DebugOut dprintf

#elif defined(PGPDISK_NTDRIVER)

#define DebugOut DbgPrint

#endif // PGPDISK_95DRIVER

// Helper function for the above macros.

inline 
int 
__cdecl 
NoOpPrintf(const char *, ...)
{
	return 0;
}


/////////////////
// Math functions
/////////////////

// CeilDiv divides 'a' by 'b' and rounds the result up to the nearest integer.

inline 
PGPUInt64 
CeilDiv(PGPUInt64 a, PGPUInt64 b)
{
	if (a % b)
		return ((a / b) + 1);
	else
		return (a / b);
}

// GetHighByte extracts and returns the high byte of the given word.

inline 
PGPUInt8 
GetHighByte(PGPUInt16 w)
{
	return (PGPUInt8) ((w & 0xFF00) >> 8);
}

// GetLowByte extracts and returns the low byte of the given word.

inline 
PGPUInt8 
GetLowByte(PGPUInt16 w)
{
	return (PGPUInt8) (w & 0x00FF);
}

// GetHighWord extracts and returns the high word of the given double word.

inline 
PGPUInt16 
GetHighWord(PGPUInt32 dw)
{
	return (PGPUInt16) ((dw & 0xFFFF0000) >> 16);
}

// GetLowWord extracts and returns the low word of the given double word.

inline 
PGPUInt16 
GetLowWord(PGPUInt32 dw)
{
	return (PGPUInt16) (dw & 0x0000FFFF);
}

// GetHighDWord extracts and returns the high dword of the given quad word.

inline 
PGPUInt32 
GetHighDWord(PGPUInt64 qw)
{
	return (PGPUInt32) ((qw & 0xFFFFFFFF00000000) >> 32);
}

// GetLowDWord extracts and returns the low dword of the given quad word.

inline 
PGPUInt32 
GetLowDWord(PGPUInt64 qw)
{
	return (PGPUInt32) (qw & 0x00000000FFFFFFFF);
}

// MakeWord makes a WORD out of a low byte and a high byte

inline 
PGPUInt16 
MakeWord(PGPUInt8 low, PGPUInt8 high)
{
	return (PGPUInt16) (((PGPUInt16) high) << 8 | low);
}

// MakeLong makes a DWORD out of a low word and a high word.

inline 
PGPUInt32 
MakeLong(PGPUInt16 low, PGPUInt16 high)
{
	return (PGPUInt32) (((PGPUInt32) high) << 16 | low);
}

// MakeQWord makes a QWORD out of a low DWORD and a high bDWORDyte

inline 
PGPUInt64 
MakeQWord(PGPUInt32 low, PGPUInt32 high)
{
	return (PGPUInt64) (((PGPUInt64) high) << 32 | low);
}

// IsValidHexChar returns TRUE if the specified char is 0-1 or A-F.

inline 
PGPBoolean 
IsValidHexChar(char c)
{
	c = toupper(c);

	return (((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'F')));
}

// SimpleLog2 returns the base 2 logarithm of a, assuming a is a perfect power
// of two.

inline 
PGPUInt8 
SimpleLog2(PGPUInt32 a)
{
	PGPUInt8 i = 0;

	while (a > 1)
	{
		a = a >> 1;
		i++;
	}

	return i;
}


///////////////////
// Memory functions
///////////////////

// DupeString duplicates a string.

inline 
DualErr 
DupeString(LPCSTR in, LPSTR *out)
{
	DualErr	derr;
	LPSTR	copy;

	pgpAssertAddrValid(out, LPCSTR);
	pgpAssertStrValid(in);

#if defined(PGPDISK_MFC)

	try
	{
		copy = new char[strlen(in) + 1];
		strcpy(copy, in);
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

#elif defined(PGPDISK_95DRIVER) || defined(PGPDISK_NTDRIVER)

	copy = new char[strlen(in) + 1];

	if (IsNull(copy))
		derr = DualErr(kPGDMinorError_OutOfMemory);
	else
		strcpy(copy, in);

#else
#error Define PGPDISK_MFC, PGPDISK_95DRIVER, or PGPDISK_NTDRIVER.
#endif	// PGPDISK_MFC

	if (derr.IsntError())
	{
		(* out) = copy;
	}

	return derr;
}

// GetByteBuffer returns a pointer to a buffer of bytes of the specified
// size.

inline 
DualErr 
GetByteBuffer(PGPUInt32 size, PGPUInt8 **buf)
{
	DualErr		derr;
	PGPUInt8	*pMem;

	pgpAssertAddrValid(buf, PGPUInt8 *);

#if defined(PGPDISK_MFC)

	try
	{
		pMem = new PGPUInt8[size];
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

#else // !PGPDISK_MFC

	pMem = new PGPUInt8[size];

	if (IsNull(pMem))
		derr = DualErr(kPGDMinorError_OutOfMemory);

#endif // PGPDISK_MFC
	
	if (derr.IsntError())
	{
		(* buf) = pMem;
	}

	return derr;
}

// FreeByteBuffer frees a byte buffer allocated by GetByteBuffer.

inline 
void 
FreeByteBuffer(PGPUInt8 *buf)
{
	delete[] buf;
}

// FlipBytes flips the bits in a chunk of memory.

inline 
void 
FlipBytes(PGPUInt8 *pMem, PGPUInt32 size)
{
	pgpAssertAddrValid(pMem, PGPUInt8);

	for (PGPUInt32 i=0; i < size; i++)
	{
		pMem[i] = ~pMem[i];
	}
}

// SmartStringCopy copies either 'inString' in its entirety to 'outString', 
// or 'size' chars of 'inString', whichever is smaller, and appends a NULL.

inline 
void 
SmartStringCopy(LPSTR outString, LPCSTR inString, PGPUInt32 size)
{
	PGPUInt32 numToCopy;

	pgpAssertStrValid(inString);
	pgpAssertAddrValid(outString, char);

	numToCopy = min(strlen(inString), size - 1);

	strncpy(outString, inString, numToCopy);
	outString[numToCopy] = kNullChar;
}


///////////////////////////////////////
// Data conversion/extraction functions
///////////////////////////////////////

// DriveLetToNum converts a drive letter to a 0-based drive number.

inline 
PGPUInt8 
DriveLetToNum(char drivelet)
{
	return (toupper(drivelet) - 'A');
}

// DriveNumToLet converts a 0-based drive number to an uppercase drive letter.

inline 
char 
DriveNumToLet(PGPUInt8 drive)
{
	pgpAssert(drive <= kMaxDrives);

	return (drive + 'A');
}

// DriveFromPath returns the drive number from a path.

inline 
PGPUInt8 
DriveFromPath(LPCSTR path)
{
	pgpAssertStrValid(path);

	return DriveLetToNum(path[0]);
}

// IsLegalDriveLetter returns TRUE if a legal drive letter was specified.

inline 
PGPBoolean 
IsLegalDriveLetter(char drivelet)
{
	return (DriveLetToNum(drivelet) < kMaxDrives);
}

// IsLegalDriveNumber returns TRUE if a legal drive number was specified.

inline 
PGPBoolean 
IsLegalDriveNumber(PGPUInt8 drive)
{
	return (drive < kMaxDrives);
}

// FormatSizeString takes a disk size and returns a string describing the
// size in English.

inline 
void 
FormatSizeString(PGPUInt32 kb, LPSTR sbuf, PGPUInt32 size)
{
	char kbStr[20], mbStr[20];

	pgpAssertAddrValid(sbuf, PGPUInt8);

	if (size < 25)
	{
		sbuf[0] = NULL;
		return;
	}

#if defined(PGPDISK_SHELLEXT)

	if (kb < kKbPerMeg*2)
	{
		_itoa(kb, kbStr, 10);
		wsprintf(sbuf, "%s KB", kbStr);
	}
	else if (kb < kKbPerGig*2)
	{
		_itoa(kb/kKbPerMeg, mbStr, 10);
		wsprintf(sbuf, "%s MB", mbStr);
	}
	else
	{
		_itoa(kb/kKbPerGig, mbStr, 10);
		wsprintf(sbuf, "%s GB", mbStr);
	}

#else // !PGPDISK_SHELLEXT

	if (kb < kKbPerMeg*2)
	{
		_itoa(kb, kbStr, 10);
		sprintf(sbuf, "%s KB", kbStr);
	}
	else if (kb < kKbPerGig*2)
	{
		_itoa(kb/kKbPerMeg, mbStr, 10);
		sprintf(sbuf, "%s MB", mbStr);
	}
	else
	{
		_itoa(kb/kKbPerGig, mbStr, 10);
		sprintf(sbuf, "%s GB", mbStr);
	}

#endif	// PGPDISK_SHELLEXT
}


//////////////////////////
// File and path functions
//////////////////////////

// HasPlainLocalRoot returns TRUE if the specified path begins with a form
// "C:\", FALSE otherwise.

inline 
PGPBoolean 
HasPlainLocalRoot(LPCSTR path)
{
	pgpAssertStrValid(path);

	return ((path[1] == ':') && (path[2] == '\\'));
}

// IsPlainLocalRoot returns TRUE if the specified path is of the form "C:\",
// FALSE otherwise.

inline 
PGPBoolean 
IsPlainLocalRoot(LPCSTR path)
{
	pgpAssertStrValid(path);

	return ((strlen(path) == 3) && HasPlainLocalRoot(path));
}


// IsShortCut returns TRUE if the specified path is of the form "*.lnk", 
// FALSE otherwise.

inline 
PGPBoolean 
IsShortCut(LPCSTR path)
{
	char		ext[4];
	PGPUInt32	length;

	pgpAssertStrValid(path);

	length = strlen(path);

	if (strlen(path) < 4)
	{
		return FALSE;
	}
	else
	{
		// There is no string 'right' command in Ring-0.
		ext[0] = path[length - 4];
		ext[1] = path[length - 3];
		ext[2] = path[length - 2];
		ext[3] = path[length - 1];

		return (_strnicmp(ext, ".lnk", 4) == 0);
	}
}

// IsUNCPath returns TRUE if the specified path is in UNC format, FALSE
// otherwise.

inline 
PGPBoolean 
IsUNCPath(LPCSTR path)
{
	PGPUInt32 i, length, numSlashes;

	pgpAssertStrValid(path);

	if (strncmp(path, "\\\\", 2) != 0)
		return FALSE;

	length = strlen(path);
	numSlashes = 0;

	for (i = 0; i < length; i++)
	{
		if ((path[i] == '\\') || (path[i] == '/'))
			numSlashes++;

		if (numSlashes == 4)
			return TRUE;
	}

	return FALSE;
}

// IsNetworkedPath returns TRUE if the specified path resides on a networked
// drive, FALSE otherwise.

inline 
PGPBoolean 
IsNetworkedPath(LPCSTR path)
{
	pgpAssertStrValid(path);

	if (strlen(path) < 3)
		return FALSE;

	if (IsUNCPath(path))
		return TRUE;

#if defined(PGPDISK_MFC)

	char head[4];

	strncpy(head, path, 3);
	head[3] = NULL;

	return (GetDriveType(head) == DRIVE_REMOTE);

#else // !PGPDISK_MFC

	return FALSE;

#endif	// PGPDISK_MFC
}


///////////////////////
// System call wrappers
///////////////////////

// PGPdiskGetTicks returns the 'system tick' count.

inline 
PGPUInt64 
PGPdiskGetTicks()
{
#if defined(PGPDISK_MFC)

	return GetTickCount();

#elif defined(PGPDISK_95DRIVER)

	return Get_System_Time();

#elif defined(PGPDISK_NTDRIVER)

	LARGE_INTEGER tickCount;

	KeQueryTickCount(&tickCount);

	return tickCount.QuadPart;

#endif	// PGPDISK_MFC
}

// ConvertSystemTimeToSecondsSince1970 converts a system time to the number
// of seconds since 1970.

inline 
PGPUInt64 
ConvertSystemTimeToSecondsSince1970(PGPUInt64 HundredNsSince1601)
{
#if defined(PGPDISK_MFC)

	PGPUInt64 secsSince1601, secsSince1970;

	secsSince1601	= HundredNsSince1601 / k100nsPerSecond;
	secsSince1970	= secsSince1601 - kSecsBetween1601And1970;

	return secsSince1970;

#elif defined(PGPDISK_95DRIVER) || defined(PGPDISK_NTDRIVER)

	pgpDebugMsg("This function is not available in driver mode.");

	return 0xFFFFFFFFFFFFFFFF;

#endif	// PGPDISK_MFC
}

// GetSecondsSince1970 returns the number of seconds elapsed since 1970.
// This is according to UTC time.

inline 
PGPUInt64 
GetSecondsSince1970()
{
#if defined(PGPDISK_MFC)

	FILETIME	fileTime;
	PGPUInt64	HundredNsSince1601;

	GetSystemTimeAsFileTime(&fileTime);

	HundredNsSince1601 = MakeQWord(fileTime.dwLowDateTime, 
		fileTime.dwHighDateTime);

	return ConvertSystemTimeToSecondsSince1970(HundredNsSince1601);

#elif defined(PGPDISK_95DRIVER)

	PGPUInt32 moreMs;

	return IFSMgr_Get_NetTime(&moreMs);

#elif defined(PGPDISK_NTDRIVER)

	LARGE_INTEGER HundredNsSince1601;

	KeQuerySystemTime(&HundredNsSince1601);

	return ConvertSystemTimeToSecondsSince1970(HundredNsSince1601.QuadPart);

#endif	// PGPDISK_MFC
}

#endif // Included_UtilityFunctions_h
