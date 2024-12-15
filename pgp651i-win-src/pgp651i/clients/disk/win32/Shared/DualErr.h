//////////////////////////////////////////////////////////////////////////////
// DualErr.h
//
// Declaration and implementation of class DualErr.
//////////////////////////////////////////////////////////////////////////////

// $Id: DualErr.h,v 1.6 1999/02/27 06:15:54 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_DualErr_h	// [
#define Included_DualErr_h

#if !defined(PGPDISK_NOSDK)
#include "pgpErrors.h"
#endif	// !PGPDISK_NOSDK

#include "Errors.h"

// Align to 1.
#pragma pack(push, 1)


////////
// Types
////////

// ExternalErrorType defines what type of external error a DualErr holds.

enum ExternalErrorType {kEET_Win32Error, kEET_KernelModeError, 
	kEET_PGPSdkError};

#if defined(PGPDISK_NTDRIVER)
const ExternalErrorType kDefaultExErrorType = kEET_KernelModeError;
#else
const ExternalErrorType kDefaultExErrorType = kEET_Win32Error;
#endif // PGPDISK_NTDRIVER


////////////////
// Class DualErr
////////////////

// Note: This is code adapted from the CComboError class of the pfl. Instead
// of including a PGPError, the class includes a PGDMinorError and an
// 'external error' type.

class DualErr
{
public:
	static DualErr		NoError;

	PGDMinorError		mMinorError;
	ExternalErrorType	mExErrorType;
	ExternalError		mExternalError;
	
	inline				DualErr(PGDMinorError merr = kPGDMinorError_NoErr, 
							ExternalError exerr = kExternalError_NoErr, 
							ExternalErrorType extype = kDefaultExErrorType)
						{
								mMinorError		= merr;
								mExErrorType	= extype;
								mExternalError	= exerr;
						}

#if !defined(PGPDISK_NOSDK)

	inline				DualErr(PGPError pgpErr)
						{
							mExErrorType = kEET_PGPSdkError;
							mExternalError = (PGPUInt32) pgpErr;

							if (pgpErr == kPGPError_UserAbort)
								mMinorError = kPGDMinorError_UserAbort;
							else if (IsPGPError(pgpErr))
								mMinorError = kPGDMinorError_SDKFuncFailed;
							else
								mMinorError = kPGDMinorError_NoErr;
						}

#endif	// !PGPDISK_NOSDK

	inline PGPBoolean	IsError()
						{
							return (HasPGDMinorError() || HasExternalError());
						}
				
	inline PGPBoolean	IsntError()
						{
							return !IsError();
						}
				
	inline PGPBoolean	HasPGDMinorError()
						{
							return IsPGDMinorError(mMinorError);
						}

	inline PGPBoolean	HasExternalError()
						{
							return IsExternalError(mExternalError);
						}
};

// Restore alignment.
#pragma pack(pop)

#endif	// ] Included_DualErr_h
