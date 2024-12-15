/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpSDKPrefs.c,v 1.12 1997/10/09 23:54:21 lloyd Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "pgpFileSpec.h"
#include "pflPrefs.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpEnv.h"
#include "pgpContext.h"
#include "pgpUtilitiesPriv.h"

#if PGP_MACINTOSH
#include "pgpFileSpecMac.h"
#elif PGP_WIN32
#include "windows.h"
#endif

#include "pgpSDKPrefs.h"




/*____________________________________________________________________________
	If prefs for the various file locations do not exist, then create a
	file spec for the location and set it in the prefs.
____________________________________________________________________________*/
typedef struct DefaultLocInfo
{
	PGPsdkPrefSelector	selector;
	const char *		defaultName;
} DefaultLocInfo;

	static PGPError
sSetDefaultFileLocations(
	PGPContextRef	context,
	PGPBoolean *	didAddPrefs )
{
	static const DefaultLocInfo	sInfo[] =
	{
#if PGP_MACINTOSH		/* [ */
		{ kPGPsdkPref_PublicKeyring,	"PGP Public Keys" },
		{ kPGPsdkPref_PrivateKeyring,	"PGP Private Keys" },
		{ kPGPsdkPref_RandomSeedFile,	"PGP Random Seed" }
#else					/* ] PGP_MACINTOSH [ */
		{ kPGPsdkPref_PublicKeyring,	"pubring.pkr" },
		{ kPGPsdkPref_PrivateKeyring,	"secring.skr" },
		{ kPGPsdkPref_RandomSeedFile,	"randseed.bin" }
#endif					/* ] PGP_MACINTOSH */
	};
	#define kNumLocations	sizeof( sInfo ) / sizeof( sInfo[ 0 ] )
	PFLFileSpecRef	locSpec;
	PGPError		err	= kPGPError_NoErr;
	
	*didAddPrefs	= FALSE;
	
	/* by default, we'll place the files in the same directory
	as the prefs file itself */
	err	= pgpGetPrefsSpec( pgpContextGetPFLContext( context ), &locSpec );
	if ( IsntPGPError( err ) )
	{
		PGPUInt32		infoIndex;
		
		for( infoIndex = 0; infoIndex < kNumLocations; ++infoIndex )
		{
			PGPFileSpecRef			tempRef;
			const DefaultLocInfo *	info;
			
			info	= &sInfo[ infoIndex ];
			
			err	= PGPsdkPrefGetFileSpec( context, info->selector, &tempRef );
			if ( err == kPGPError_ItemNotFound )
			{
				/* one doesn't exist yet */
				pgpAssert( IsNull( tempRef ) );
				err	= PFLSetFileSpecName( locSpec, info->defaultName );
				if ( IsntPGPError( err ) )
				{
					PGPsdkPrefSetFileSpec( context,
						info->selector, (PGPFileSpecRef)locSpec );
					*didAddPrefs	= TRUE;
				}
			}
			else if ( IsntPGPError( err ) )
			{
				/* no error; there is one already set */
				PFLFreeFileSpec( (PFLFileSpecRef)tempRef );
			}
		}
		
		PFLFreeFileSpec( locSpec );
	}
	
	return( err );
}



	PGPError
PGPsdkLoadPrefs(
	PGPContextRef	context,
	PGPFileSpecRef	prefsSpecIn )
{
	PGPError		err	= kPGPError_NoErr;
	PGPPrefRef		prefsRef	= NULL;
	PFLFileSpecRef	prefsSpec	= (PFLFileSpecRef)prefsSpecIn;
	
	PGPValidateContext( context );
	PFLValidateFileSpec( prefsSpec );
	
	/* create it, if it doesn't already exist */
	(void)PFLFileSpecCreate( prefsSpec );
	
	err	= PGPOpenPrefFile( prefsSpec, &prefsRef );
	if ( IsntPGPError( err ) )
	{
		PGPPrefRef		oldPrefs	= NULL;
		PGPBoolean		didAddPrefs;
	
		oldPrefs	= pgpContextGetPrefs( context );
		if ( IsntNull( oldPrefs ) )
		{
			PGPClosePrefFile( oldPrefs );
			pgpContextSetPrefs( context, NULL );
		}
	
		pgpContextSetPrefs( context, prefsRef );
		
		(void)sSetDefaultFileLocations( context, &didAddPrefs );
		if ( didAddPrefs )
		{
			/* prefs were added so save them now */
			PGPsdkSavePrefs( context );
		}
	}
	
	return( err );
}


	PGPError
PGPsdkLoadDefaultPrefs( PGPContextRef context )
{
	PGPError		err	= kPGPError_NoErr;
	PFLFileSpecRef	prefsSpec	= NULL;
	
	PGPValidateContext( context );
	
	err	= pgpGetPrefsSpec( pgpContextGetPFLContext( context ), &prefsSpec);
	if ( IsntPGPError( err ) )
	{
		err	= PGPsdkLoadPrefs( context, (PGPFileSpecRef)prefsSpec );
		
		PFLFreeFileSpec( prefsSpec );
	}
	return( err );
}

	PGPError
PGPsdkSavePrefs( PGPContextRef context )
{
	PGPError	err	= kPGPError_NoErr;
	PGPPrefRef	prefsRef	= NULL;
	
	PGPValidateContext( context );
	
	prefsRef	= pgpContextGetPrefs( context );
	if ( IsNull( prefsRef ) )
		err	= kPGPError_ImproperInitialization;
	else
		err	= PGPSavePrefFile( prefsRef );
	
	return( err );
}


/*____________________________________________________________________________
	Set the prefs to contain data representing a PFLFileSpecRef
____________________________________________________________________________*/
	PGPError
PGPsdkPrefSetFileSpec(
	PGPContextRef			context,
	PGPsdkPrefSelector		selector,
	PGPConstFileSpecRef		fileSpecIn )
{
	PGPError		err	= kPGPError_NoErr;
	PGPPrefRef		prefsRef	= NULL;
	PGPByte *		data	= NULL;
	PGPSize			dataLength;
	PFLContextRef	pflContext;
	PFLConstFileSpecRef	fileSpec	= (PFLConstFileSpecRef)fileSpecIn;
	
	PGPValidateContext( context );
	PGPValidateParam(
		selector == kPGPsdkPref_PublicKeyring ||
		selector == kPGPsdkPref_PrivateKeyring ||
		selector == kPGPsdkPref_RandomSeedFile );
	PFLValidateFileSpec( fileSpec );
	
	prefsRef	= pgpContextGetPrefs( context );
	if ( IsNull( prefsRef ) )
		return( kPGPError_ImproperInitialization );
	
	pflContext	= pgpContextGetPFLContext( context );
	
	err	= PFLExportFileSpec( fileSpec, &data, &dataLength );
	if ( IsntPGPError( err ) )
	{
		err	= PGPSetPrefData( prefsRef,
				(PGPPrefIndex)selector, dataLength, data );
		
		PFLContextMemFree( pflContext, data );
	}
	
	return( err );
}


/*____________________________________________________________________________
	Construct a PFLFileSpecRef from prefs data and return it.
____________________________________________________________________________*/
	PGPError
PGPsdkPrefGetFileSpec(
	PGPContextRef		context,
	PGPsdkPrefSelector	selector,
	PGPFileSpecRef *	outSpec )
{
	PGPError		err	= kPGPError_NoErr;
	PGPPrefRef		prefsRef	= NULL;
	PGPByte *		data	= NULL;
	PGPSize			dataLength;
	PFLContextRef	pflContext;
	
	PGPValidatePtr( outSpec );
	*outSpec	= NULL;
	PGPValidateContext( context );
	PGPValidateParam(
		selector == kPGPsdkPref_PublicKeyring ||
		selector == kPGPsdkPref_PrivateKeyring ||
		selector == kPGPsdkPref_RandomSeedFile );
	
	prefsRef	= pgpContextGetPrefs( context );
	if ( IsNull( prefsRef ) )
		return( kPGPError_ImproperInitialization );
	
	pflContext	= pgpContextGetPFLContext( context );
	
	err	= PGPGetPrefData( prefsRef,
			(PGPPrefIndex)selector, &dataLength, (void**)&data );
		
	if ( IsntPGPError( err ) )
	{
		PFLFileSpecRef	tempRef;
		
		err	= PFLImportFileSpec( pflContext, data, dataLength, &tempRef );
		*outSpec	= (PGPFileSpecRef)tempRef;
		
		PGPDisposePrefData( prefsRef, data );
	}
	
	pgpAssertErrWithPtr( err, *outSpec );
	return( err );
}


/*____________________________________________________________________________
	Get raw pref data.
	
	Caller must dispose of data with PGPFreeData.
____________________________________________________________________________*/
	PGPError
PGPsdkPrefGetData(
	PGPContextRef		context,
	PGPsdkPrefSelector	selector,
	void **				outData,
	PGPSize *			outSize )
{
	PGPError		err	= kPGPError_NoErr;
	PGPPrefRef		prefsRef	= NULL;
	void *			data	= NULL;
	void *			prefData = NULL;
	PGPSize			dataLength;
	PFLContextRef	pflContext;
	
	if ( IsntNull( outSize ) )
		*outSize	= 0;
	if ( IsntNull( outData ) )
		*outData	= NULL;
	PGPValidatePtr( outData );
	PGPValidatePtr( outSize );
	PGPValidateContext( context );
	PGPValidateParam( selector == kPGPsdkPref_DefaultKeyID );
	
	prefsRef	= pgpContextGetPrefs( context );
	if ( IsNull( prefsRef ) )
		return( kPGPError_ImproperInitialization );
	
	pflContext	= pgpContextGetPFLContext( context );
	
	err	= PGPGetPrefData( prefsRef,
			(PGPPrefIndex)selector, &dataLength, &prefData );
		
	if ( IsntPGPError( err ) )
	{
		/* we must allocate data with PGPNewData */
		data	= pgpNewDataCopy( context, dataLength, 0, prefData );
		if ( IsNull( data ) )
		{
			err	= kPGPError_OutOfMemory;
			dataLength	= 0;
		}
			
		PGPDisposePrefData( prefsRef, prefData );
	}
	
	*outData	= data;
	*outSize	= dataLength;
	
	pgpAssertErrWithPtr( err, *outData );
	return( err );
}



/*____________________________________________________________________________
	Get raw pref data.
	
	Caller must dispose of data with PGPFreeData.
____________________________________________________________________________*/
	PGPError
PGPsdkPrefSetData(
	PGPContextRef		context,
	PGPsdkPrefSelector	selector,
	void const *		data,
	PGPSize				dataSize )
{
	PGPError		err	= kPGPError_NoErr;
	PGPPrefRef		prefsRef	= NULL;
	
	PGPValidatePtr( data );
	PGPValidateParam( dataSize != 0 );
	PGPValidateContext( context );
	PGPValidateParam( selector == kPGPsdkPref_DefaultKeyID );
	
	prefsRef	= pgpContextGetPrefs( context );
	if ( IsNull( prefsRef ) )
		return( kPGPError_ImproperInitialization );
	
	err	= PGPSetPrefData( prefsRef,
			(PGPPrefIndex)selector, dataSize, data );
	
	return( err );
}






























/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
