/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: UPGPException.h,v 1.3.8.1 1997/12/05 22:15:27 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include <UException.h>
#include "PGPErrors.h"
#include "CComboError.h"


/* Defines the following macros
	PGPThrow_Err
	PGPThrowComboErr_
	PGPThrowIfPGPErr_
	PGPThrowPGPErr_
	PGPThrowIfOSErr_
	PGPThrowOSErr_
	PGPThrowIfNil_
	PGPThrowIfNULL_
	PGPThrowIfResError_
	PGPThrowIfMemError_
	PGPThrowIfResFail_
	PGPThrowIfMemFail_
	PGPThrowIf_
	PGPThrowIfNot_
*/

#define PGPThrow_Err(err)		throw(err)


#ifdef Debug_Throw
extern Str15	gErrorString;
#define	PGPThrowComboErr_(comboerr)										\
	do {																\
		if (! comboerr.IsCancelError()) {								\
			if (comboerr.HavePGPError()) {								\
				::NumToString(comboerr.pgpErr, gErrorString);			\
			} else {													\
				::NumToString(comboerr.err, gErrorString);				\
			}															\
			if (gDebugThrow == debugAction_Alert) {						\
				UDebugging::AlertThrowAt(	gErrorString,				\
											"\p" __FILE__, __LINE__);	\
			} else if (gDebugThrow == debugAction_LowLevelDebugger) {	\
				BreakStrToLowLevelDebugger_(gErrorString);				\
			} else if (gDebugThrow == debugAction_SourceDebugger) {		\
				BreakToSourceDebugger_();								\
			}															\
		}																\
		PGPThrow_Err(comboerr);											\
	} while (false)
#else
#define PGPThrowComboErr_(comboerr)		PGPThrow_Err(comboerr)
#endif


#define PGPThrowIfPGPErr_(pgpErr)										\
	if (pgpErr != kPGPError_NoErr) {									\
		PGPThrowComboErr_(CComboError(CComboError::kPGPError, pgpErr));	\
	}

#define PGPThrowPGPErr_(err)	PGPThrowComboErr_(							\
									CComboError(CComboError::kPGPError, err))



#define PGPThrowIfOSErr_(err)											\
	if (err != noErr) {													\
		PGPThrowComboErr_(CComboError(CComboError::kPlatformError, err));	\
	}



#define PGPThrowOSErr_(err)	PGPThrowComboErr_(								\
							CComboError(CComboError::kPlatformError, err))



#define	PGPThrowIfNil_(ptr)													\
		if ((ptr) == nil) PGPThrowComboErr_(								\
				CComboError(CComboError::kPGPError, kPGPError_BadMemAddress));



#define	PGPThrowIfNULL_(ptr)												\
		if ((ptr) == nil) PGPThrowComboErr_(								\
			CComboError(CComboError::kPGPError, kPGPError_BadMemAddress));

#define	PGPThrowIfResError_()	PGPThrowIfOSErr_(ResError())
#define	PGPThrowIfMemError_()	PGPThrowIfOSErr_(MemError())


#define	PGPThrowIfResFail_(h)												\
	if ((h) == nil) {														\
		OSErr	__theErr = ResError();										\
		if (__theErr == noErr) {											\
			__theErr = resNotFound;											\
		}																	\
	PGPThrowComboErr_(CComboError(CComboError::kPlatformError, __theErr));	\
	}


	
#define	PGPThrowIfMemFail_(p)												\
	if ((p) == nil) {														\
		OSErr	__theErr = MemError();										\
		if (__theErr == noErr) __theErr = memFullErr;						\
	PGPThrowComboErr_(CComboError(CComboError::kPlatformError, __theErr));	\
	}



#define	PGPThrowIf_(test)													\
		if (test) PGPThrowComboErr_(CComboError(CComboError::kPGPError,		\
										kPGPError_AssertFailed));



#define	PGPThrowIfNot_(test)												\
		if (!(test)) PGPThrowComboErr_(CComboError(CComboError::kPGPError,	\
					kPGPError_AssertFailed));
