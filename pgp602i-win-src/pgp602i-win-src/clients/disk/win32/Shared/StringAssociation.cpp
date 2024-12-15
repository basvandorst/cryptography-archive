//////////////////////////////////////////////////////////////////////////////
// StringAssociation.cpp
//
// Functions for associating string identifiers with strings.
//////////////////////////////////////////////////////////////////////////////

// $Id: StringAssociation.cpp,v 1.1.2.10 1998/08/04 02:10:33 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"

#elif defined(PGPDISK_95DRIVER)

#include <vtoolscp.h>
#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT

#elif defined(PGPDISK_NTDRIVER)

#include <vdw.h>

#else
#error Define PGPDISK_MFC, PGPDISK_95DRIVER, or PGPDISK_NTDRIVER.
#endif	// PGPDISK_MFC

#include "Required.h"

#if PGPDISK_PUBLIC_KEY
#include "pgpErrors.h"
#endif	// PGPDISK_PUBLIC_KEY

#include "CommonStrings.h"
#include "Errors.h"
#include "StringAssociation.h"
#include "UtilityFunctions.h"


////////////
// Functions
////////////

// GetCommonString returns a pointer to the string associated with the given
// string ID, or a default string if none can be found.

LPCSTR 
GetCommonString(PGPUInt32 stringId, LPCSTR defaultString)
{
	LPSTR stringFound = NULL;

	if (stringId < kPGPdiskMaxCommonStrings)
		stringFound = (LPSTR) kPGPdiskCommonStrings[stringId];

	if (IsNull(stringFound))
		stringFound = (LPSTR) defaultString;

	return stringFound;
}

// GetMajorErrorString returns a pointer to the string associated with the
// given PGDMajorError.

LPCSTR 
GetMajorErrorString(
	PGDMajorError	perr, 
	LPCSTR			defaultString = kPGPdiskUnknownMajorErrorString)
{
	LPSTR stringFound = NULL;

	if (perr < kPGPdiskMaxMajorErrorStrings)
		stringFound = (LPSTR) kPGPdiskMajorErrorStrings[perr];

	if (IsNull(stringFound))
		stringFound = (LPSTR) defaultString;

	return stringFound;
}

// GetMinorErrorString returns a pointer to the string associated with the
// given PGDMinorError.

LPCSTR 
GetMinorErrorString(
	PGDMinorError	merr, 
	LPCSTR			defaultString = kPGPdiskUnknownMinorErrorString)
{
	LPSTR stringFound = NULL;

	if (merr < kPGPdiskMaxMinorErrorStrings)
		stringFound = (LPSTR) kPGPdiskMinorErrorStrings[merr];

	if (IsNull(stringFound))
		stringFound = (LPSTR) defaultString;

	return stringFound;
}

// GetExternalErrorString returns a string describing the given external
// error.

void 
GetExternalErrorString(
	ExternalErrorType	exErrType, 
	PGPUInt32			exErr, 
	PGPUInt32			sizeErrString, 
	LPSTR				errString)
{
	static char	tempString[kMaxStringSize];

	pgpAssertAddrValid(errString, char);

	switch (exErrType)
	{

	case kEET_Win32Error:

#if defined(PGPDISK_MFC)

		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, exErr, 
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errString, 
			sizeErrString, NULL);

#else	// !PGPDISK_MFC
		strcpy(errString, kEmptyString);
#endif	// PGPDISK_MFC

		break;

	case kEET_PGPSdkError:

#if PGPDISK_PUBLIC_KEY

		PGPGetErrorString((PGPError) exErr, sizeErrString, errString);

		sprintf(tempString, GetCommonString(kPGPdiskErrorWasString), 
			errString);
		SmartStringCopy(errString, tempString, sizeErrString);

#else	// !PGPDISK_PUBLIC_KEY
		strcpy(errString, kEmptyString);
#endif	// PGPDISK_PUBLIC_KEY

		break;

	default:
		strcpy(errString, kEmptyString);
		break;
	}
}

// FormatErrorString creates a single formatted error string from multiple
// sources of information.

void 
FormatErrorString(
	PGDMajorError	perr, 
	DualErr			derr, 
	PGPUInt8		drive, 
	LPSTR			outString, 
	PGPUInt32		outStringSize)
{
	static char	majorErrorString[kMaxStringSize];
	static char	minorErrorString[kMaxStringSize];
	static char	systemMessage[kMaxStringSize];
	static char	temp[kMaxStringSize];
	PGPBoolean	isQuestion;

	pgpAssertAddrValid(outString, char);

	// There are four possibilities. We can either have both a PGDMinorError
	// and an external error, a PGDMinorError alone, an external error alone, 
	// or neither.
	//
	// Each major error has the form of "text%s.text" or "text %c:\\ text%s."
	// %c represents an optional drive parameter, and %s is the place where 
	// the PGDMinorError/external error explanation string goes.

	strcpy(majorErrorString, GetMajorErrorString(perr));

	// Is this a question?
	isQuestion = majorErrorString[strlen(majorErrorString) - 1] == '?';

	// Don't put stuff after question mark.
	if (isQuestion)
		derr.mExternalError = kExternalError_NoErr;

	if (DoesMajorErrTakeDrive(perr))		// using drive parameter?
	{
		sprintf(temp, majorErrorString, DriveNumToLet(drive), "%s");
		strcpy(majorErrorString, temp);
	}

	if (derr.HasPGDMinorError() && derr.HasExternalError())	// both errs?
	{
		strcpy(minorErrorString, "");
		strcat(minorErrorString, GetCommonString(kPGPdiskBecauseString));
		strcat(minorErrorString, GetMinorErrorString(derr.mMinorError));

		sprintf(temp, majorErrorString, minorErrorString);
		strcpy(majorErrorString, temp);

		GetExternalErrorString(derr.mExErrorType, derr.mExternalError, 
			kMaxStringSize, systemMessage);

		strcat(majorErrorString, " ");
		strcat(majorErrorString, systemMessage);
	}
	else if (derr.HasPGDMinorError())		// just a PGDMinorError?
	{
		strcpy(minorErrorString, "");
		strcat(minorErrorString, GetCommonString(kPGPdiskBecauseString));
		strcat(minorErrorString, GetMinorErrorString(derr.mMinorError));

		sprintf(temp, majorErrorString, minorErrorString);
		strcpy(majorErrorString, temp);
	}
	else if (derr.HasExternalError())		// just an external Error?
	{
		GetExternalErrorString(derr.mExErrorType, derr.mExternalError, 
			kMaxStringSize, systemMessage);

		sprintf(temp, majorErrorString, kEmptyString);
		strcpy(majorErrorString, temp);

		strcat(majorErrorString, " ");
		strcat(majorErrorString, systemMessage);
	}

	// Stick a numeric error ID on the end.
	if (derr.HasPGDMinorError())
	{
		if (derr.HasExternalError())
		{
			sprintf(temp, " (%d, %X)", (PGPUInt32) derr.mMinorError, 
				derr.mExternalError);
		}
		else
		{
			sprintf(temp, " (%d)", (PGPUInt32) derr.mMinorError);
		}

		// Tack extra space onto end if last char isn't already space.
		if (majorErrorString[strlen(majorErrorString) - 1] != ' ')
			strcat(majorErrorString, " ");

		strcat(majorErrorString, temp);
	}

	SmartStringCopy(outString, majorErrorString, outStringSize);
}
