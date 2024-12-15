/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: PGPOpenPrefs.cp,v 1.6 1997/09/29 10:24:37 wprice Exp $
____________________________________________________________________________*/
#include "PGPOpenPrefs.h"

#include "MacErrors.h"
#include "MacFiles.h"
#include "pgpClientPrefs.h"
#include "pgpAdminPrefs.h"
#include "pgpFileSpec.h"
#include "CComboError.h"
#include "pgpMacClientErrors.h"

const ResID		kPrefsNameListID			= 14444;
enum
{
	kClientPrefsFilenameStrIndex	= 1,
	kAdminPrefsFilenameStrIndex,
	kGroupsFilenameStrIndex
};


	PGPError
PGPOpenClientPrefs(
	PFLContextRef	context,
	PGPPrefRef		*prefRef)
{
	CComboError		err;
	FSSpec			fsSpec;
	Boolean			created = FALSE;
	
	PGPValidatePtr( prefRef );
	*prefRef	= NULL;
	
	err.err = FindPGPPreferencesFolder(-1, &fsSpec.vRefNum, &fsSpec.parID);
	if ( err.IsntError() )
	{
		PFLFileSpecRef	fsRef	= NULL;
		
		GetIndString( fsSpec.name, kPrefsNameListID,
					kClientPrefsFilenameStrIndex);
		pgpAssert( fsSpec.name[0] != 0 );
		
		/* create it if it doesn't already exist */
		if ( ! FSpExists( &fsSpec ) )
		{
			err.err	= FSpCreate( &fsSpec, 'pgpK', 'pref', smSystemScript );
			created = TRUE;
		}
		
		if ( err.IsntError() )
			err.pgpErr	= PFLNewFileSpecFromFSSpec( context, &fsSpec, &fsRef);
			
		if ( err.IsntError() )
		{
			err.pgpErr = PGPOpenPrefFile(fsRef, prefRef);
			if( err.IsntError()  && created )
			{
				PGPLoadClientDefaults(*prefRef);
			}
			
			PFLFreeFileSpec( fsRef );
		}
	}
	
	return( err.ConvertToPGPError() );
}

/*____________________________________________________________________________
	Acts differently, depending on whether this is an administrator build
	or not.  If it's an admin build, then the admin prefs will be created
	if they don't exist.  If it's not an admin build, then they won't be
	created and a kPGPError_AdminPrefsNotFound error will be returned.
____________________________________________________________________________*/

	PGPError
PGPOpenAdminPrefs(
	PFLContextRef	context,
	PGPPrefRef		*prefRef)
{
#if PGP_BUSINESS_SECURITY	// [
	CComboError		err;
	FSSpec			fsSpec;
	PGPBoolean		created = FALSE;
	
	PGPValidatePtr( prefRef );
	*prefRef	= NULL;
	
	err.err = FindPGPPreferencesFolder(-1, &fsSpec.vRefNum, &fsSpec.parID);
	if ( err.IsntError() )
	{
		GetIndString( fsSpec.name, kPrefsNameListID,
				kAdminPrefsFilenameStrIndex);
		pgpAssert( fsSpec.name[0] != 0 );
		
		/* create it if it doesn't already exist */
		if ( ! FSpExists( &fsSpec ) )
		{
		#if PGP_ADMIN_BUILD
			// only an administrator build is allowed to create these prefs
			err.err	= FSpCreate( &fsSpec,
					kPGPMacFileCreator_Keys, 'pref', smSystemScript );
			created = TRUE;
		#else
			err.pgpErr	= kPGPError_AdminPrefsNotFound;
		#endif
		}
		
		if ( err.IsntError() )
		{
			PFLFileSpecRef	fsRef	= NULL;
		
			err.pgpErr	= PFLNewFileSpecFromFSSpec( context, &fsSpec, &fsRef);
			if ( err.IsntError() )
			{
				err.pgpErr = PGPOpenPrefFile(fsRef, prefRef);
				if( err.IsError() || created )
				{
					PGPLoadAdminDefaults(*prefRef);
				}
				PFLFreeFileSpec( fsRef );
			}
		}
	}
	
	return( err.ConvertToPGPError() );
#else
	context;
	prefRef;
	pgpAssert(0);	// This function is *only* called in non-BS builds
	return kPGPError_FeatureNotAvailable;
#endif	// ] PGP_BUSINESS_SECURITY
}



	PGPError
PGPGetGroupsFileFSSpec(FSSpec *fileSpec)
{
	CComboError	err;
	
	PGPValidatePtr( fileSpec );
	
	err.err	= FindPGPPreferencesFolder( -1,
				&fileSpec->vRefNum, &fileSpec->parID );
	if( err.IsntError() )
	{
		GetIndString( fileSpec->name, kPrefsNameListID,
					kGroupsFilenameStrIndex );
		pgpAssert( fileSpec->name[0] != 0 );
	}
	
	return( err.ConvertToPGPError() );
}






