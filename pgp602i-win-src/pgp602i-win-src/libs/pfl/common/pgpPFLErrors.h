/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	Error codes.

	$Id: pgpPFLErrors.h,v 1.19.16.1 1998/11/12 03:18:01 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpPFLErrors_h	/* [ */
#define Included_pgpPFLErrors_h

#include "pgpDebug.h"



#define kPGPPFLErrorBase	-12000
#define kPGPPFLErrorRange	500

enum
{
	/*
	NOTE: error numbers must not be changed as compile clients depend on them.
	*/
	
	kPGPError_NoErr				= 0,
	
	kPGPError_BadParams			= -12000,
	kPGPError_OutOfMemory		= -11999,
	kPGPError_BufferTooSmall	= -11998,
	
	kPGPError_FileNotFound		= -11997,
	kPGPError_CantOpenFile		= -11996,
	kPGPError_FilePermissions	= -11995,
	kPGPError_FileLocked		= -11994,
	kPGPError_DiskFull			= -11993,
	kPGPError_IllegalFileOp		= -11992,
	kPGPError_FileOpFailed		= -11991,
	kPGPError_ReadFailed		= -11990,
	kPGPError_WriteFailed		= -11989,
	kPGPError_EOF				= -11988,
	
	kPGPError_UserAbort			= -11987,
	kPGPError_UnknownRequest	= -11986,
	kPGPError_LazyProgrammer	= -11985,
	kPGPError_ItemNotFound		= -11984,
	kPGPError_ItemAlreadyExists	= -11983,
	kPGPError_AssertFailed		= -11982,
	kPGPError_BadMemAddress		= -11981,
	kPGPError_UnknownError		= -11980,
	
	kPGPError_PrefNotFound		= -11979,
	kPGPError_EndOfIteration	= -11978,
	kPGPError_ImproperInitialization	= -11977,
	kPGPError_CorruptData				= -11976,
	kPGPError_FeatureNotAvailable		= -11975,
	
	kPGPError_LastPFLError = kPGPPFLErrorBase + kPGPPFLErrorRange
};

#define IsPGPError( err )		( (err) != kPGPError_NoErr )
#define IsntPGPError( err )		( (err) == kPGPError_NoErr )



#define PGPValidateParam( expr )	\
	if ( ! (expr ) )	\
	{\
		pgpAssert( expr );\
		return( kPGPError_BadParams );\
	}

#define PGPValidatePtr( ptr )	\
			PGPValidateParam( (ptr) != NULL )


#include "pgpBase.h"
			
PGP_BEGIN_C_DECLARATIONS


PGPError 	PFLGetErrorString( PGPError theError,
				PGPSize bufferSize, char * theString );


PGP_END_C_DECLARATIONS


#endif /* ] Included_pgpPFLErrors_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
