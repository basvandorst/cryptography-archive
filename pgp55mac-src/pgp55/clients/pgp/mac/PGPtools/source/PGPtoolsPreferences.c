/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include "pgpMem.h"
#include "pgpKeys.h"
#include "pgpSDKPrefs.h"
#include "pgpClientPrefs.h"
#include "PGPOpenPrefs.h"
#include "pflContext.h"

#include "PGPtoolsPreferences.h"

#define	kPreferencesPadSize		512L

typedef union PaddedPreferences
{
	PGPtoolsPreferences	prefs;
	uchar				padding[kPreferencesPadSize];

} PaddedPreferences;

PGPtoolsPreferences	gPreferences;

	static void
InitDefaultPreferences(PGPtoolsPreferences *prefs)
{
	pgpClearMemory( prefs, sizeof( prefs ) );

	prefs->version		= kPGPtoolsPreferencesVersion;
}

	PGPError
LoadPreferences(PGPContextRef context)
{
	PGPError	err;
	
	err = PGPsdkLoadDefaultPrefs( context );
	if( IsntPGPError( err ) )
	{
		PFLContextRef	pflContext;
		
		err	= PFLNewContext( &pflContext );
		if ( IsntPGPError( err ) )
		{
			PGPPrefRef		prefRef;
			
			err = PGPOpenClientPrefs( pflContext, &prefRef );
			if( IsntPGPError( err ) )
			{
				PGPSize				prefsSize;
				PaddedPreferences	*paddedPrefs;
				
				pgpAssert( sizeof( PGPtoolsPreferences ) <=
						kPreferencesPadSize );

				prefsSize = sizeof( paddedPrefs );
				
				err = PGPGetPrefData( prefRef,
							kPGPPrefPGPtoolsMacPrivateData, &prefsSize,
							&paddedPrefs );
				if( IsntPGPError( err ) )
				{
					gPreferences = paddedPrefs->prefs;
					
					PGPDisposePrefData( prefRef, paddedPrefs );
				}
				
				(void) PGPClosePrefFile( prefRef );
			}
			PFLFreeContext( pflContext );
		}
	}
	
	if( IsPGPError( err ) )
	{
		InitDefaultPreferences( &gPreferences );
		if( err == kPGPError_ItemNotFound )
			err = kPGPError_NoErr;
	}
	return( err );
}

	PGPError
SavePreferences()
{
	PGPError		err;
	PFLContextRef	pflContext;
	
	err	= PFLNewContext( &pflContext );
	if ( IsntPGPError( err ) )
	{
		PGPPrefRef	prefRef;
	
		err = PGPOpenClientPrefs( pflContext, &prefRef );
		if( IsntPGPError( err ) )
		{
			PaddedPreferences	paddedPrefs;
			
			pgpClearMemory( &paddedPrefs, sizeof( paddedPrefs ) );
			paddedPrefs.prefs = gPreferences;
			
			pgpAssert( sizeof( PGPtoolsPreferences ) <= kPreferencesPadSize );

			err = PGPSetPrefData( prefRef,
						kPGPPrefPGPtoolsMacPrivateData, sizeof( paddedPrefs ),
						&paddedPrefs );

			(void) PGPSavePrefFile( prefRef );
			(void) PGPClosePrefFile( prefRef );
		}
		
		PFLFreeContext( pflContext );
	}
	return( err );
}





