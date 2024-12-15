/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpSDKPrefs.h,v 1.2 1997/09/22 17:42:48 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_pgpCDKPrefs_h	/* [ */
#define Included_pgpCDKPrefs_h

#include "pgpConfig.h"
#include "pgpBase.h"


enum PGPsdkPrefSelector_
{
	kPGPsdkPref_First = 0,
	
	kPGPsdkPref_PublicKeyring	= 0,
	kPGPsdkPref_PrivateKeyring	= 1,
	kPGPsdkPref_RandomSeedFile	= 2,
	
	kPGPsdkPref_DefaultKeyID	= 3,
	
	kPGPsdkPref_LastPlusOne,
	
	PGP_ENUM_FORCE( PGPsdkPrefSelector_ )
};
PGPENUM_TYPEDEF( PGPsdkPrefSelector_, PGPsdkPrefSelector );



PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

/* call this if you want the CDK to use prefs stored in the prefs file */
PGPError PGPsdkLoadPrefs( PGPContextRef context,
					PGPFileSpecRef prefSpec );
PGPError PGPsdkLoadDefaultPrefs( PGPContextRef context );

/* save any changed preferences to disk */
PGPError PGPsdkSavePrefs( PGPContextRef context );


PGPError PGPsdkPrefSetFileSpec( PGPContextRef context,
					PGPsdkPrefSelector selector, PGPConstFileSpecRef ref );
					
/* caller must deallocate *outRef with PGPFreeFileSpec */
PGPError PGPsdkPrefGetFileSpec( PGPContextRef context,
					PGPsdkPrefSelector selector, PGPFileSpecRef * outRef );
					
					
PGPError PGPsdkPrefSetData( PGPContextRef context,
					PGPsdkPrefSelector selector,
					void const *data, PGPSize size );

/* caller must deallocate *dataOut with PGPFreeData */
PGPError PGPsdkPrefGetData( PGPContextRef context,
					PGPsdkPrefSelector selector,
					void **dataOut, PGPSize *sizeOut );

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpCDKPrefs_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
