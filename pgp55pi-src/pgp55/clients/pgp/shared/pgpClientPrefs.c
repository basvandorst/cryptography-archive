/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpClientPrefs.c,v 1.11.2.2 1997/11/14 22:58:58 build Exp $
____________________________________________________________________________*/
#include "pgpClientPrefs.h"
#include "pflPrefTypes.h"
#include "pgpSymmetricCipher.h"
#include "pgpPubTypes.h"
#include "pgpKeyServerPrefs.h"
#include "pgpDebug.h"



static const PGPKeyServerEntry sDefaultKeyServer[] =
{
	{"mit.edu","http://pgpkeys.mit.edu:11371",
		(kKeyServerListed)},
	{"pgp.com","ldap://certserver.pgp.com",
		(kKeyServerListed|kKeyServerDefault)}
};

static const PGPCipherAlgorithm sAllowedAlgorithms[] =
{
	kPGPCipherAlgorithm_CAST5,
	kPGPCipherAlgorithm_3DES,
	kPGPCipherAlgorithm_IDEA
};



#if	PGP_MACINTOSH	/* [ */

typedef struct PGPmenuApps
{
	PGPBoolean	privateScrap;
	PGPBoolean	useOutputDialog;
	OSType		creator;
	Str31		appName;
} PGPmenuApps;

static const PGPmenuApps	sInitPGPmenuApps[] =
{
	{FALSE,	FALSE,	'ttxt', "\pSimpleText"},
	{TRUE,	TRUE,	'MOSS', "\pNetscape"},			/* Verified -> 4.01 */
	{TRUE,	TRUE,	'AOp3', "\pAmerica Online"},	/* Verified -> 3.01 */
	{TRUE,	FALSE,	'R*ch', "\pBBEdit"},			/* Verified -> 4.5	*/
	{TRUE,	TRUE,	'MSNM', "\pMS Internet Mail"}
};	

	static PGPError
PGPLoadClientDefaultsPlatform(
	PGPPrefRef	prefRef)
{
	PGPError	err;
	
	err = PGPSetPrefData(prefRef, kPGPPrefPGPmenuMacAppSignatures,
							sizeof(sInitPGPmenuApps),
							sInitPGPmenuApps);
	pgpAssertNoErr(err);
	return( err );
}

#else	/* ][ */

	static PGPError
PGPLoadClientDefaultsPlatform(
	PGPPrefRef	prefRef)
{
	(void)prefRef;
	return( kPGPError_NoErr );
}

#endif	/* ] */



	PGPError
PGPLoadClientDefaults(
	PGPPrefRef	prefRef)
{
	PGPError	err;
	
	// Boolean Prefs
	err = PGPSetPrefBoolean(prefRef, kPGPPrefEncryptToSelf, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefWordWrapEnable, TRUE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefDecryptCacheEnable, TRUE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefSignCacheEnable, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefMailEncryptPGPMIME, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefMailSignPGPMIME, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefFastKeyGen, TRUE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefWarnOnMixRSAElGamal, TRUE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefMailEncryptDefault, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefMailSignDefault, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefMarginalIsInvalid, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefDisplayMarginalValidity, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefFirstKeyGenerated, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefWarnOnRSARecipAndNonRSASigner,
										TRUE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefKeyServerSyncUnknownKeys, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefKeyServerSyncOnAddName, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefKeyServerSyncOnKeySign, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefKeyServerSyncOnRevocation,
		FALSE);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefWarnOnReadOnlyKeyRings, TRUE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefWarnOnWipe, TRUE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefAutoDecrypt, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefWarnOnADK, TRUE);
	pgpAssertNoErr(err);
	
	// Number Prefs
	err = PGPSetPrefNumber(prefRef, kPGPPrefWordWrapWidth, 70);
	pgpAssertNoErr(err);
	err = PGPSetPrefNumber(prefRef, kPGPPrefDecryptCacheSeconds, 120);
	pgpAssertNoErr(err);
	err = PGPSetPrefNumber(prefRef, kPGPPrefSignCacheSeconds, 120);
	pgpAssertNoErr(err);
	err = PGPSetPrefNumber(prefRef, kPGPPrefDateOfLastSplashScreen, 0);
	pgpAssertNoErr(err);
	err = PGPSetPrefNumber(prefRef, kPGPPrefPreferredAlgorithm,
										kPGPCipherAlgorithm_CAST5);
	pgpAssertNoErr(err);
	err = PGPSetPrefNumber(prefRef, kPGPPrefAllowedAlgorithms, 
										0xE0000000);	// all 3
	pgpAssertNoErr(err);
	err = PGPSetPrefNumber(prefRef, kPGPPrefMacBinaryDefault,
										kPGPPrefMacBinaryOn );
	pgpAssertNoErr(err);
	err = PGPSetPrefNumber(prefRef, kPGPPrefDateOfLastFreewareNotice, 0);
	pgpAssertNoErr(err);
	
	// String Prefs
	err = PGPSetPrefString(prefRef, kPGPPrefOwnerName, "");
	pgpAssertNoErr(err);	
	err = PGPSetPrefString(prefRef, kPGPPrefCompanyName, "");
	pgpAssertNoErr(err);	
	err = PGPSetPrefString(prefRef, kPGPPrefLicenseNumber, "");
	pgpAssertNoErr(err);	
	err = PGPSetPrefString(prefRef, kPGPPrefComment, "");
	pgpAssertNoErr(err);	
	
	/* platform-specific prefs should go into PGPLoadClientDefaultsPlatform() */
#if	PGP_MACINTOSH
	// Data Prefs
	err	= PGPLoadClientDefaultsPlatform( prefRef );
	pgpAssertNoErr(err);
#endif
	
	err = PGPSetPrefData(prefRef, kPGPPrefKeyServerList,
							sizeof(sDefaultKeyServer),
							sDefaultKeyServer);
	pgpAssertNoErr(err);
	err = PGPSetPrefData(prefRef, kPGPPrefAllowedAlgorithmsList,
							sizeof(sAllowedAlgorithms),
							sAllowedAlgorithms);
	pgpAssertNoErr(err);

	err= PGPSavePrefFile(prefRef);
	pgpAssertNoErr(err);
	
	return err;
}




/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
