/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: PGPUILibUtils.cp,v 1.10 1997/10/10 18:48:28 heller Exp $
____________________________________________________________________________*/

#include <LowMem.h>
#include <fp.h>
#include <string.h>

#include "MacBasics.h"
#include "MacDebug.h"
#include "MacErrors.h"
#include "MacFiles.h"

#include "pflPrefs.h"
#include "pgpBase.h"
#include "pgpDebug.h"
#include "pgpMem.h"

#include "PGPOpenPrefs.h"
#include "PGPUILibUtils.h"
#include "pgpUserInterface.h"
#include "pgpKeys.h"
#include "CComboError.h"

static FSSpec	sLibraryFileSpec;
static long		sLibraryFileID;

	OSStatus
SetLibraryFSSpec(const FSSpec *fileSpec)
{
	OSStatus	status;
	CInfoPBRec	cpb;
	
	// This routine should be called once ONLY from the library INIT routine.
	pgpAssert( sLibraryFileSpec.name[0] == 0 );
	pgpAssert( sLibraryFileID == 0 );
	
	status = FSpGetCatInfo( fileSpec, &cpb );
	if( IsntErr( status ) )
	{
		HParamBlockRec	pb;
		
		sLibraryFileSpec = *fileSpec;
		
		// Create a file ID so we can track this file later.
		
		pgpClearMemory( &pb, sizeof( pb ) );
		
		pb.fidParam.ioVRefNum	= sLibraryFileSpec.vRefNum;
		pb.fidParam.ioNamePtr	= sLibraryFileSpec.name;
		pb.fidParam.ioSrcDirID	= sLibraryFileSpec.parID;
		
		if( IsntErr( PBCreateFileIDRefSync( &pb ) ) )
		{
			sLibraryFileID = pb.fidParam.ioFileID;
		}
	}
	
	return( status );
}

	OSStatus
PGPGetUILibFSSpec(FSSpec *fileSpec)
{
	OSStatus	status;
	CInfoPBRec	cpb;
	
	pgpAssertAddrValid( fileSpec, FSSpec );
	AssertSpecIsValid( &sLibraryFileSpec, "PGPGetPGPLibraryFSSpec" );
	
	status = FSpGetCatInfo( &sLibraryFileSpec, &cpb );
	if( IsErr( status ) && sLibraryFileID != 0 )
	{
		// Find the library using the file ID.
		
		HParamBlockRec	pb;
		FSSpec			specCopy;
		
		pgpClearMemory( &pb, sizeof( pb ) );
		
		specCopy = sLibraryFileSpec;
		
		pb.fidParam.ioVRefNum	= specCopy.vRefNum;
		pb.fidParam.ioNamePtr	= specCopy.name;
		pb.fidParam.ioFileID	= sLibraryFileID;
		
		status = PBResolveFileIDRefSync( &pb );
		if( IsntErr( status ) )
		{
			specCopy.parID = pb.fidParam.ioSrcDirID;
			
			status = FSpGetCatInfo( &specCopy, &cpb );
			if( IsntErr( status ) )
			{
				sLibraryFileSpec = specCopy;
			}
		}
	}

	AssertNoErr( status, "GetPGPLibFileSpec" );
	
	*fileSpec = sLibraryFileSpec;
	
	return( status );
}

	CComboError
EnterPGPUILib(
	PGPContextRef context,
	PGPUILibState *state)
{
	CComboError	err;
	FSSpec		libraryFileSpec;
	
	pgpAssertAddrValid( state, PGPUILibState );
	
	pgpClearMemory( state, sizeof( *state ) );
	
	GetPort( &state->savedPort );
	
	state->savedResLoad = LMGetResLoad();
	state->savedZone	= GetZone();
	state->savedResFile	= CurResFile();
	
	SetResLoad( TRUE );
	
	err.err	= PGPGetUILibFSSpec( &libraryFileSpec );
	if( err.IsntError() )
	{
		state->libResFileRefNum = FSpOpenResFile( &libraryFileSpec, fsRdPerm );
		if( state->libResFileRefNum > 0 )
		{
			UseResFile( state->libResFileRefNum );
			
			err.pgpErr = PGPOpenClientPrefs( (PFLContextRef) context,
					&state->clientPrefsRef );

		#if PGP_BUSINESS_SECURITY	// [
		
			if( err.IsntError() )
			{
			#if PGP_ADMIN_BUILD	// [

				err.pgpErr = PGPOpenAdminPrefs( (PFLContextRef) context,
							&state->adminPrefsRef );
				if( err.IsError() )
				{
					// Allow failure to open admin prefs file in admin builds
				
					state->adminPrefsRef 	= kInvalidPGPPrefRef;
					err	.pgpErr				= kPGPError_NoErr;
				}
			#else	// ] PGP_ADMIN_BUILD [
			
				err.pgpErr	= PGPOpenAdminPrefs( (PFLContextRef) context,
							&state->adminPrefsRef );
							
			#endif	// ] PGP_ADMIN_BUILD
			
			}
		#endif	// ] PGP_BUSINESS_SECURITY
		
		}
		else
		{
			err.pgpErr	= kPGPError_FileNotFound;
		}
	}

	return( err );
}

	void
ExitPGPUILib(const PGPUILibState *state)
{
	pgpAssertAddrValid( state, PGPUILibState );

	if( state->libResFileRefNum > 0 )
		CloseResFile( state->libResFileRefNum );

	if( PGPPrefRefIsValid( state->clientPrefsRef ) )
		(void) PGPClosePrefFile( state->clientPrefsRef );
		
#if PGP_BUSINESS_SECURITY
	if( PGPPrefRefIsValid( state->adminPrefsRef ) )
		(void) PGPClosePrefFile( state->adminPrefsRef );
#endif

	SetPort( state->savedPort );
	SetResLoad( state->savedResLoad );
	SetZone( state->savedZone );
	UseResFile( state->savedResFile );
}

	PGPUInt16
PGPCalcPassphraseQuality(
	const char *passphrase)
{
	PGPUInt16	quality;
	PGPUInt16	passphraseLength;
	PGPUInt16 	charIndex;
	PGPUInt16 	rangeIndex;
	PGPUInt16	span = 0;
	
	typedef struct ScoreStruct
	{
		uchar	firstChar;
		uchar	lastChar;
	} Range;
	
	static const Range	sRanges[] =
	{
		{ 0,		' ' - 1},
		{ ' ',		'A' - 1},
		{ 'A',		'Z'},
		{ 'Z' + 1,	'a' - 1},
		{ 'a',		'z'},
		{ '0',		'9'},
		{ 'z' + 1,	127},
		{ 128,		255},
	};
	
	const PGPUInt16	kNumRanges	= sizeof( sRanges ) / sizeof( sRanges[ 0 ] );
	Boolean			haveRange[ kNumRanges ];
	
	pgpAssertAddrValid( passphrase, char );
	pgpClearMemory( haveRange, sizeof( haveRange ) );
	
	passphraseLength = strlen( passphrase );
	for( charIndex = 0; charIndex < passphraseLength; ++charIndex )
	{
		for( rangeIndex = 0; rangeIndex < kNumRanges; ++rangeIndex )
		{
			uchar			theChar;
			const Range *	theRange;
			
			theChar		= passphrase[ charIndex ];
			theRange	= &sRanges[ rangeIndex ];
			
			if ( theChar >= theRange->firstChar && 
				theChar <= theRange->lastChar )
			{
				haveRange[ rangeIndex ] 	= true;
				break;
			}
		}
	}
	
	// determine the total span
	for( rangeIndex = 0; rangeIndex < kNumRanges; ++rangeIndex )
	{
		if ( haveRange[ rangeIndex ] )
		{
			span	+= 1 + ( sRanges[ rangeIndex ].lastChar -
				sRanges[ rangeIndex ].firstChar );
		}
	}
	pgpAssert( span <= 256 );
	
	if ( span != 0 )
	{
		double		bitsPerChar	= log2( span );
		PGPUInt32	totalBits	= bitsPerChar * passphraseLength;
		
		// assume a maximum quality of 128 bits
		quality = ( totalBits * 100 ) / 128;
		if( quality > 100 )
			quality = 100;
	}
	else
	{
		quality	= 0;
	}
		
	return( quality );
}

	PGPError
PGPKeyCanSign(
	PGPKeyRef 	theKey,
	PGPBoolean 	*keyCanSign)
{
	PGPError	err;
	PGPBoolean	isSecretKey;
	PGPBoolean	canSign = FALSE;
	
	PGPValidateParam( PGPKeyRefIsValid( theKey ) );
	PGPValidateParam( IsntNull( keyCanSign ) );
	
	err = PGPGetKeyBoolean( theKey, kPGPKeyPropIsSecret, &isSecretKey );
	if( IsntPGPError( err ) && isSecretKey )
	{
		PGPBoolean	oldCanSign;
		
		err = PGPGetKeyBoolean( theKey, kPGPKeyPropCanSign, &oldCanSign );
		if( IsntPGPError( err ) && oldCanSign )
		{
			PGPBoolean	isRevoked;
			
			err = PGPGetKeyBoolean( theKey, kPGPKeyPropIsRevoked,
						&isRevoked );
			if( IsntPGPError( err ) && ! isRevoked )
			{
				PGPBoolean	isDisabled;
				
				err = PGPGetKeyBoolean( theKey, kPGPKeyPropIsDisabled,
							&isDisabled );
				if( IsntPGPError( err ) && ! isDisabled )
				{
					PGPBoolean	isValid;
					
					err = PGPGetKeyBoolean( theKey, kPGPKeyPropIsNotCorrupt,
								&isValid );
					if( IsntPGPError( err ) && isValid )
					{
						PGPBoolean	isExpired;
						
						err = PGPGetKeyBoolean( theKey,
									kPGPKeyPropIsExpired, &isExpired );
						if( IsntPGPError( err ) && ! isExpired )
						{
							canSign = TRUE;
						}
					}
				}
			}
		}
	}
						
	*keyCanSign = canSign;

	return( err );
}

	PGPError
PGPKeyCanDecrypt(
	PGPKeyRef 	theKey,
	PGPBoolean 	*keyCanDecrypt)
{
	return( PGPKeyCanSign( theKey, keyCanDecrypt ) );
}

	PGPError
PGPKeyCanEncrypt(
	PGPKeyRef 	theKey,
	PGPBoolean 	*keyCanEncrypt)
{
	PGPError		err;
	PGPBoolean		canEncrypt = FALSE;
	PGPBoolean		isRevoked;
			
	PGPValidateParam( PGPKeyRefIsValid( theKey ) );
	PGPValidateParam( IsntNull( keyCanEncrypt ) );

	err = PGPGetKeyBoolean( theKey, kPGPKeyPropIsRevoked, &isRevoked );
	if( IsntPGPError( err ) && ! isRevoked )
	{
		PGPBoolean	oldCanEncrypt;
		
		err = PGPGetKeyBoolean( theKey, kPGPKeyPropCanEncrypt,
					&oldCanEncrypt );
		if( IsntPGPError( err ) && oldCanEncrypt )
		{
			PGPBoolean	isDisabled;
			
			err = PGPGetKeyBoolean( theKey, kPGPKeyPropIsDisabled,
						&isDisabled );
			if( IsntPGPError( err ) && ! isDisabled )
			{
				PGPBoolean	isValid;
				
				err = PGPGetKeyBoolean( theKey, kPGPKeyPropIsNotCorrupt,
							&isValid );
				if( IsntPGPError( err ) && isValid )
				{
					PGPBoolean	isExpired;
					
					err = PGPGetKeyBoolean( theKey, kPGPKeyPropIsExpired,
								&isExpired );
					if( IsntPGPError( err ) && ! isExpired )
					{
						canEncrypt = TRUE;
					}
				}
			}
		}
	}
	
	*keyCanEncrypt = canEncrypt;
				
	return( err );
}

