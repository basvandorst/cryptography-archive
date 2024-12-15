/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpAdminPrefs.c,v 1.6 1997/10/15 21:36:53 heller Exp $
____________________________________________________________________________*/
#include "pgpAdminPrefs.h"
#include "pflPrefTypes.h"
#include "pgpDebug.h"
#include "pgpEncode.h"


/*
	Default Admin Prefs are *never* set by the client apps.  Defaults
	are set only by the Admin Wizard which calls this function.
	Clients refuse to run if no Admin Prefs file is available.
	
	This file could also be used for testing purposes to create admin
	prefs behavior.
*/

	PGPError
PGPLoadAdminDefaults(
	PGPPrefRef	prefRef)
{
	PGPError	err;
	PGPUInt8	nullKeyID[8] = {13,'\0','\0','\0','\0'};
	
	// Boolean Prefs
	err = PGPSetPrefBoolean(prefRef, kPGPPrefUseOutgoingADK, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefUseDHADK, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefEnforceIncomingADK, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefEnforceMinChars, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefEnforceMinQuality, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefWarnNotCertByCorp, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefAutoSignTrustCorp, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefMetaIntroducerCorp, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefAllowKeyGen, TRUE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefAllowRSAKeyGen, TRUE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefAllowConventionalEncryption,
							TRUE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefEnforceOutgoingADK, FALSE);
	pgpAssertNoErr(err);
	err = PGPSetPrefBoolean(prefRef, kPGPPrefEnforceRemoteADKClass, TRUE);
	pgpAssertNoErr(err);
	
	// Number Prefs
	err = PGPSetPrefNumber(prefRef, kPGPPrefMinChars, 8);
	pgpAssertNoErr(err);
	err = PGPSetPrefNumber(prefRef, kPGPPrefMinQuality, 60);
	pgpAssertNoErr(err);
	err = PGPSetPrefNumber(prefRef, kPGPPrefCorpKeyPublicKeyAlgorithm, 
							kPGPPublicKeyAlgorithm_RSA);
	pgpAssertNoErr(err);
	err = PGPSetPrefNumber(prefRef, kPGPPrefOutADKPublicKeyAlgorithm, 
							kPGPPublicKeyAlgorithm_RSA);
	pgpAssertNoErr(err);
	err = PGPSetPrefNumber(prefRef, kPGPPrefMinimumKeySize, 768);
	pgpAssertNoErr(err);
	
	// String Prefs
	err = PGPSetPrefString(prefRef, kPGPPrefComments, "");
	pgpAssertNoErr(err);
	err = PGPSetPrefString(prefRef, kPGPPrefSiteLicenseNumber, "");
	pgpAssertNoErr(err);
	
	// Data Prefs
	err = PGPSetPrefData(prefRef, kPGPPrefOutgoingADKID, sizeof(nullKeyID),
							nullKeyID);
	pgpAssertNoErr(err);

	//PGPUInt8	realKeyID[9] = {14,0xAC,0xBB,0x16,0x4B,0xCF,0x73,0xEC,0x4C};
	//The above can be used for testing purposes but should otherwise
	//remain commented out
	
	err = PGPSetPrefData(prefRef, kPGPPrefDHADKID, sizeof(nullKeyID),
							nullKeyID);
	pgpAssertNoErr(err);
	err = PGPSetPrefData(prefRef, kPGPPrefCorpKeyID, sizeof(nullKeyID),
							nullKeyID);
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
