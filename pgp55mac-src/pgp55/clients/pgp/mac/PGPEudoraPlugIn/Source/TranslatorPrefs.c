/*____________________________________________________________________________
	TranslatorPrefs.c
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: TranslatorPrefs.c,v 1.15 1997/10/21 00:22:32 lloyd Exp $
____________________________________________________________________________*/
#include <string.h>

#include "PGPOpenPrefs.h"
#include "pflPrefTypes.h"
#include "pgpClientPrefs.h"
#include "pgpAdminPrefs.h"
#include "pflContext.h"

#include "TranslatorPrefs.h"

typedef enum  { kDummy	= 0 } Dummy;

typedef struct PrefsStruct
{
	Boolean		usePGPMimeEncrypt;
	Boolean		usePGPMimeSign;
	Boolean		doBreakLines;
	ulong		lineLength;
	
#define kMaxCommentLength	69
	char		comment[ kMaxCommentLength + 1];
	
	ulong		encryptionPassphraseCacheSeconds;	// 0 means none
	ulong		signingPassphraseCacheSeconds;		// 0 means none
	
	PGPCipherAlgorithm			conventionalCipher;
	
	PreferredAlgorithmsStruct	preferredAlgs;
	
	// used to ensure we initialize right number of members
	enum Dummy	dummy;
} PrefsStruct;
static PrefsStruct	sPrefs	=
{
	true,
	false,
	true,
	78,
	"",
	2 * 60,
	0,
	kPGPCipherAlgorithm_CAST5,
	
	kDummy
};

	static PGPError
GetAdminComment(
	PFLContextRef	pflContext,
	PGPSize			commentMax,
	char 			comment[ kMaxCommentLength ] )
{
	PGPError	err	= kPGPError_UnknownError;
	
	comment[ 0 ]	= '\0';
#if PGP_BUSINESS_SECURITY	// [
	{
		PGPPrefRef	prefRef;
		
		err = PGPOpenAdminPrefs( pflContext, &prefRef );
		if( IsntPGPError( err ) )
		{
			// Comments are not always present. Don't check for errors.
			(void) PGPGetPrefStringBuffer( prefRef, kPGPPrefComments,
					commentMax, comment );

			(void) PGPClosePrefFile( prefRef );
		}
	}
#else
	(void)pflContext;
	(void)commentMax;
#endif	// ] PGP_BUSINESS_SECURITY

	return( err );
}


	void
RefreshPrefs( void )
{
	PGPError	pgpErr;
	PrefsStruct	prefs	= sPrefs;
	PGPPrefRef	prefRef;
	PFLContextRef	context	= NULL;
	
	pgpErr	= PFLNewContext( &context );
	
	if( IsntPGPError( pgpErr ) )
		pgpErr = PGPOpenClientPrefs( context, &prefRef );
	if( IsntPGPError( pgpErr ) )
	{
		PGPBoolean		useCache;
		PGPBoolean		usePGPMimeEncrypt;
		PGPBoolean		usePGPMimeSign;
		PGPBoolean		doBreakLines;
		void *			prefData	= NULL;
		PGPSize			prefSize;
		PGPUInt32		number;
		
		pgpErr	= PGPGetPrefBoolean( prefRef,
					kPGPPrefMailEncryptPGPMIME, &usePGPMimeEncrypt );
		AssertNoErr( pgpErr, "RefreshPrefs" );
		prefs.usePGPMimeEncrypt = usePGPMimeEncrypt;
		
		pgpErr	= PGPGetPrefBoolean( prefRef,
					kPGPPrefMailSignPGPMIME, &usePGPMimeSign );
		AssertNoErr( pgpErr, "RefreshPrefs" );
		prefs.usePGPMimeSign = usePGPMimeSign;
		
		pgpErr	= PGPGetPrefBoolean( prefRef, 
					kPGPPrefDecryptCacheEnable, &useCache );
		AssertNoErr( pgpErr, "RefreshPrefs" );
		if ( useCache )
		{
			PGPUInt32	cacheSeconds;
			
			pgpErr	= PGPGetPrefNumber( prefRef,
						kPGPPrefDecryptCacheSeconds, &cacheSeconds );
			AssertNoErr( pgpErr, "RefreshPrefs" );
			prefs.encryptionPassphraseCacheSeconds	= cacheSeconds;
		}
		else
		{
			prefs.encryptionPassphraseCacheSeconds	= 0;
		}
		
		
		pgpErr	= PGPGetPrefBoolean( prefRef,
					kPGPPrefSignCacheEnable, &useCache );
		AssertNoErr( pgpErr, "RefreshPrefs" );
		if ( useCache )
		{
			PGPUInt32	cacheSeconds;
			
			pgpErr	= PGPGetPrefNumber( prefRef, kPGPPrefSignCacheSeconds,
						&cacheSeconds );
			AssertNoErr( pgpErr, "RefreshPrefs" );
			prefs.signingPassphraseCacheSeconds	= cacheSeconds;
		}
		else
		{
			prefs.signingPassphraseCacheSeconds	= 0;
		}
		
		
		pgpErr	= PGPGetPrefBoolean( prefRef, kPGPPrefWordWrapEnable,
						&doBreakLines );
		AssertNoErr( pgpErr, "RefreshPrefs" );
		prefs.doBreakLines = doBreakLines;
		
		if ( prefs.doBreakLines )
		{
			PGPUInt32	lineLength;
			
			pgpErr	= PGPGetPrefNumber( prefRef, kPGPPrefWordWrapWidth,
						&lineLength );
			AssertNoErr( pgpErr, "RefreshPrefs" );
			prefs.lineLength	= lineLength;
		}
		else
		{
			prefs.doBreakLines	= true;
			prefs.lineLength	= 78;
		}

		pgpErr	= GetAdminComment( context, sizeof(prefs.comment),
					prefs.comment );
		/* if we couldn't get an admin comment, use our own */
		if( IsPGPError( pgpErr ) || strlen( prefs.comment ) == 0 )
		{
			// Comments are not always present. Don't check for errors.
			(void) PGPGetPrefStringBuffer( prefRef, kPGPPrefComment,
					sizeof( prefs.comment ), prefs.comment );
		}
		/* not an error if we don't have a comment */
		pgpErr	= kPGPError_NoErr;
		
		pgpErr	= PGPGetPrefNumber( prefRef,
					kPGPPrefPreferredAlgorithm, &number );
		if ( IsntPGPError( pgpErr ) )
		{
			prefs.conventionalCipher	= (PGPCipherAlgorithm)number;
		}
		else
		{
			prefs.conventionalCipher	= kPGPCipherAlgorithm_CAST5;
		}
		
		pgpErr	= PGPGetPrefData( prefRef, kPGPPrefAllowedAlgorithmsList,
						&prefSize, &prefData);
		if ( IsntPGPError( pgpErr ) )
		{
			PGPUInt32			numAlgs	= 0;
			
			#define kAlgSize	sizeof( prefs.preferredAlgs.algs[ 0 ] )
			#define kMaxAlgs	sizeof( prefs.preferredAlgs.algs ) /\
									sizeof( prefs.preferredAlgs.algs[ 0 ] )
			
			numAlgs	= prefSize / kAlgSize;
			if ( numAlgs > kMaxAlgs )
			{
				numAlgs	= kMaxAlgs;
			}
			prefs.preferredAlgs.numAlgs	= numAlgs;
			
			pgpCopyMemory( prefData,
				prefs.preferredAlgs.algs, numAlgs * kAlgSize );
			
			PGPDisposePrefData( prefRef, prefData );
			prefData	= NULL;
		}
		else
		{
			prefs.preferredAlgs.numAlgs	= 0;
		}

		PGPClosePrefFile( prefRef );
	}
	
	if ( IsntNull( context ) )
		PFLFreeContext( context );
	
	sPrefs	= prefs;
		
	AssertNoErr( pgpErr, "RefreshPrefs" );
}


	static void
RefreshPeriodic(  )
{
	ulong			now;
	static ulong	sLastTime	= 0;
	const ulong		kRefreshSeconds	= 1;
	
	GetDateTime( &now );
	if ( now - sLastTime >= kRefreshSeconds )
	{
		RefreshPrefs( );
		GetDateTime( &sLastTime );
	}
}


	Boolean
PrefShouldUsePGPMimeEncrypt(  )
{
	RefreshPeriodic( );
	
	return( sPrefs.usePGPMimeEncrypt );
}

	Boolean
PrefShouldUsePGPMimeSign(  )
{
	RefreshPeriodic( );
	
	return( sPrefs.usePGPMimeSign );
}

	ulong
PrefGetSigningPassphraseCacheSeconds(  )
{
	RefreshPeriodic(  );
	
	return( sPrefs.signingPassphraseCacheSeconds );
}

	ulong
PrefGetEncryptionPassphraseCacheSeconds(  )
{
	RefreshPeriodic(  );
	
	return( sPrefs.encryptionPassphraseCacheSeconds );
}


	Boolean
PrefShouldBreakLines(
	ulong *			lineLength )
{
	RefreshPeriodic( );
	
	*lineLength	= sPrefs.lineLength;
	return( sPrefs.doBreakLines );
}


	Boolean
PrefGetComment( char comment[ 256 ] )
{
	strcpy( comment, sPrefs.comment );
	
	return( comment[ 0 ] != '\0' );
}



	PGPCipherAlgorithm
PrefGetConventionalCipher( void )
{
	RefreshPeriodic( );

	return( sPrefs.conventionalCipher );
}


	void
PrefGetPreferredAlgorithms( PreferredAlgorithmsStruct *algs )
{
	*algs	= sPrefs.preferredAlgs;
}









