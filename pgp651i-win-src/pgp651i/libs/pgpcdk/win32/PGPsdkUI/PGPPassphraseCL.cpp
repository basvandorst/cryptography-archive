/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: PGPPassphraseCL.cpp,v 1.4 1999/03/10 02:54:01 heller Exp $
____________________________________________________________________________*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pgpDialogs.h"
#include "pgpPassphraseUtils.h"
#include "pgpErrors.h"
#include "pgpCLUtils.h"
#include "pgpKeys.h"

#define MAXDECRYPTIONNAMECHAR		36

// global variable structure for re-entrancy
typedef struct _GPP
{
	char *				pszPassPhrase;
	char *				pszPassPhraseConf;
	PGPInt32 iNextTabControl;
	PGPBoolean				bHideText;
	PGPContextRef		context;
	const CPGPPassphraseDialogOptions *options;
} GPP;

// internal prototypes
static PGPError PGPsdkCLError(PGPError);
static void *secAlloc(PGPContextRef, PGPUInt32);
static void secFree(void*);
static void FreePassphrases(GPP *gpp);
static void ClearPassphrases(GPP *gpp);

static PGPError
PGPsdkUIErrorBox(PGPError error) 
{
	PGPError	err				= kPGPError_NoErr;
	char		szMessage[512];
	
	if (IsPGPError (error) && (error!=kPGPError_UserAbort)) {
		PGPGetErrorString (error, sizeof(szMessage), szMessage);
		printf("%s: PGP Error", szMessage);
	}

	return err;
}

//___________________________
//
// Secure memory allocation routines
//

static void * 
secAlloc (PGPContextRef context, PGPUInt32 uBytes) 
{
	PGPMemoryMgrRef	memmgr;
	
	memmgr = PGPGetContextMemoryMgr (context);
	return (PGPNewSecureData (memmgr, uBytes, 0));
}

static void 
secFree (void* p) 
{
	if (p) {
		memset ((char *)p, '\0', strlen((char *)p));
		PGPFreeData ((char *)p);
	}
}

static void
FreePassphrases(GPP *gpp)
{
	if (gpp->pszPassPhrase) {
		secFree(gpp->pszPassPhrase);
		gpp->pszPassPhrase=NULL;
	}

	if (gpp->pszPassPhraseConf) {
		secFree(gpp->pszPassPhraseConf);
		gpp->pszPassPhraseConf=NULL;
	}

}

static void
ClearPassphrases(GPP *gpp)
{
	if(gpp->pszPassPhraseConf) {
		secFree(gpp->pszPassPhraseConf);
		gpp->pszPassPhraseConf=NULL;
	}

}

static void
GetKeyString(PGPKeyRef Key,char *szNameFinal)
{
	char			sz1[32],sz2[32];
	char			szName[kPGPMaxUserIDSize];
	PGPUInt32		uAlgorithm,uKeyBits;
	PGPUInt32			u;

	PGPGetKeyNumber (Key, kPGPKeyPropAlgID, (int *)&uAlgorithm);

	// get key type / size info to append to name
	strcpy (sz2, "   (");
	switch (uAlgorithm) 
	{
	case kPGPPublicKeyAlgorithm_RSA :
		strcat (sz2, "RSA/");
		PGPGetKeyNumber (Key, kPGPKeyPropBits, (int *)&uKeyBits);
		sprintf (sz1, "%i", uKeyBits);
		strcat (sz2, sz1);
		break;
		
	case kPGPPublicKeyAlgorithm_DSA :
		strcat (sz2, "DSS/");
		PGPGetKeyNumber (Key, kPGPKeyPropBits, (int *)&uKeyBits);
		sprintf (sz1, "%i", uKeyBits);
		strcat (sz2, sz1);
		break;
		
	default :
		strcat (sz2, "Unknown/Unknown");
		break;
	}
	
	strcat (sz2, ")");

	// get name on key
	PGPGetPrimaryUserIDNameBuffer (Key, sizeof(szName),szName, &u);

	strcpy(szNameFinal, szName);
	strcat(szNameFinal, sz2);
	
	//TruncateKeyText (hdc, szName, sz2, iComboWidth, szNameFinal);
}


//	____________________________
//
//  setup keyselection list o' keys
//

PGPBoolean 
InitSigningKeyComboBox (CPGPKeySetPassphraseDialogOptions *options) 
{
	PGPKeyListRef	KeyList;
	PGPKeyIterRef	KeyIter;
	PGPKeyRef		Key;
	PGPBoolean		bSecret, bRevoked, bExpired, bCanSign;
	PGPBoolean		bAtLeastOneSecretKey;
	char			szNameFinal[kPGPMaxUserIDSize];

	PGPOrderKeySet (options->mKeySet, kPGPValidityOrdering, &KeyList);
	PGPNewKeyIter (KeyList, &KeyIter);

	bAtLeastOneSecretKey = FALSE;

	PGPKeyIterNext (KeyIter, &Key);
	while (Key) {
		PGPGetKeyBoolean (Key, kPGPKeyPropIsSecret, &bSecret);
		if (bSecret) {
			PGPGetKeyBoolean (Key, kPGPKeyPropIsRevoked,
							  (unsigned char *)&bRevoked);
			PGPGetKeyBoolean (Key, kPGPKeyPropIsExpired,
							  (unsigned char *)&bExpired);
			PGPGetKeyBoolean (Key, kPGPKeyPropCanSign,
							  (unsigned char *)&bCanSign);
			if (!bRevoked && !bExpired && bCanSign) {
				bAtLeastOneSecretKey = TRUE;

				GetKeyString(Key,szNameFinal);

				fprintf(stdout, "\n%s\n", szNameFinal);
			}
		}
		PGPKeyIterNext (KeyIter, &Key);
	}
	PGPFreeKeyIter (KeyIter);
	PGPFreeKeyList (KeyList);

	return (bAtLeastOneSecretKey);

}

PGPBoolean
PassphraseLengthAndQualityOK(
	CPGPPassphraseDialogOptions	*options,
	char 						*Passphrase)
{
	if (options->mMinPassphraseLength != 0) {
		if(strlen(Passphrase) < options->mMinPassphraseLength) {
			printf("Passphrase is not of sufficient length. Please choose another.");
			return FALSE;
		}
	}
	
	if (options->mMinPassphraseQuality != 0) {
		if(PGPEstimatePassphraseQuality(Passphrase) < options->mMinPassphraseQuality) {
			printf("Passphrase is not of sufficient quality. Please choose another.");
			return FALSE;
		}
	}

	return TRUE;
}

//	____________________________
//
//  search keys for matching phrase

PGPError 
ValidateSigningPhrase (GPP *gpp, char * pszPhrase, PGPKeyRef key) 
{
	char	szName[kPGPMaxUserIDSize];
	char	sz[128];
	char	sz2[kPGPMaxUserIDSize + 128];
	PGPSize	size;
	CPGPSigningPassphraseDialogOptions *options;

	options = (CPGPSigningPassphraseDialogOptions *)gpp->options;

	// does phrase match selected key ?
	if (PGPPassphraseIsValid (key, 
			PGPOPassphrase (gpp->context, pszPhrase),
			PGPOLastOption (gpp->context))) {
		*(options->mPassphraseKeyPtr) = key;
		return kPGPError_NoErr;
	}

	if (options->mFindMatchingKey) {
		// does phrase match any private key ?
		key=GetKeyForPassphrase(options->mKeySet,pszPhrase,TRUE);

		if (key != NULL) {
			// ask user to use other key
			PGPGetPrimaryUserIDNameBuffer (key, sizeof(szName), szName, &size);
			sprintf (sz2, sz, szName);
			return kPGPError_BadPassphrase;
		}
	}

	// phrase doesn't match any key
	printf("Bad Passphrase: Please re-enter\n");
	
	return kPGPError_BadPassphrase;

}

// ****************************************************************************
// ****************************************************************************

#define KEYSIZE 256		// hard-coded key size XXX

// Signer combo box
PGPError
pgpSigningPassphraseCL(
	PGPContextRef						context,
	CPGPSigningPassphraseDialogOptions 	*options)
{
	PGPError err = 0;
	GPP	gpp;

	memset(&gpp,0x00,sizeof(GPP));
	gpp.context=context;
	gpp.options=options;

	/*
	 * XXX
	 * Currently expects to display only *ONE* key.
	 * The caller must set the keyset in the options to consist
	 * of the signing key that the caller wants this routine
	 * to verify
	 */
	
	// Initialize stuff
	if (!InitSigningKeyComboBox (options)) {
		return kPGPError_UserAbort; // kPGPError_Win32_NoSecret_Key
	}
	
	if (options->mPrompt)
		puts(options->mPrompt);
	else
		printf("Need a pass phrase to use this key\n");
	
	// Need to ask and get Passphrase
	
	PGPKeyListRef	KeyList;
	PGPKeyIterRef	KeyIter;
	PGPKeyRef		key;
	PGPOrderKeySet (options->mKeySet,kPGPValidityOrdering,&KeyList);
	PGPNewKeyIter (KeyList, &KeyIter);
	PGPKeyIterNext (KeyIter, &key);

	while (1) {
		FreePassphrases(&gpp);
		gpp.pszPassPhrase = (char *)secAlloc (gpp.context, KEYSIZE);
		if (gpp.pszPassPhrase) {
			PGPBoolean  bShared;
			
			PGPInt32 len = pgpCLGetPass(stdout, gpp.pszPassPhrase, KEYSIZE);
			if (len < 0) {
				err = kPGPError_UserAbort;
				break;
			}
			
			*(options->mPassphrasePtr) = gpp.pszPassPhrase;
			
			// Check Shared status
			err = PGPGetKeyBoolean(key, kPGPKeyPropIsSecretShared, &bShared);
			
			if (IsntPGPError(err) && bShared) {
				// So, they want to do a shared key
				*(options->mPassphraseKeyPtr) = key;
				err = kPGPError_KeyUnusableForSignature;
				break;
			}
			
			if (PassphraseLengthAndQualityOK(options,gpp.pszPassPhrase)) {
				if (!options->mVerifyPassphrase) {
					err = kPGPError_NoErr;
					break;
				}
				
				err = ValidateSigningPhrase(&gpp,gpp.pszPassPhrase,key);
				
				if (IsntPGPError(err)) {
					err = kPGPError_NoErr;
					break;
				}
			} else {
				ClearPassphrases(&gpp);
				FreePassphrases(&gpp);
				break;
			}
		} else {
			err = kPGPError_OutOfMemory;
			break;
		}
	}
	
	PGPFreeKeyIter(KeyIter);
	PGPFreeKeyList(KeyList);
	ClearPassphrases(&gpp);
	if (err != kPGPError_NoErr)
		FreePassphrases(&gpp);
	return(err);
}

PGPError
pgpPassphraseCL(
	PGPContextRef 				context,
	CPGPPassphraseDialogOptions	*options)
{
	PGPError	err = 0;
	GPP			gpp;

	if (options->mPrompt)
		puts(options->mPrompt);
	else
		printf("Please enter pass phrase for secret key\n");

	memset(&gpp,0x00,sizeof(GPP));
	gpp.context=context;
	gpp.options=options;
	
	while (1) {
		FreePassphrases(&gpp);
		gpp.pszPassPhrase = (char *)secAlloc (gpp.context, KEYSIZE);
		if (gpp.pszPassPhrase) {
			PGPInt32 len = pgpCLGetPass(stdout, gpp.pszPassPhrase, KEYSIZE);
			if (len < 0) {
				err = kPGPError_UserAbort;
				break;
			}
			
			*(options->mPassphrasePtr) = gpp.pszPassPhrase;
			
			if (PassphraseLengthAndQualityOK(options,gpp.pszPassPhrase)) {
				err = kPGPError_NoErr;
				break;
			} else {
				ClearPassphrases(&gpp);
				FreePassphrases(&gpp);
			}
		} else {
			err = kPGPError_OutOfMemory;
			break;
		}
	}

	ClearPassphrases(&gpp);
	if (err != kPGPError_NoErr)
		FreePassphrases(&gpp);
	return(err);
}

PGPError
pgpKeyPassphraseCL(
	PGPContextRef					context,
	CPGPKeyPassphraseDialogOptions	*options)
{
	PGPError err;
	GPP	gpp;
	
	memset(&gpp,0x00,sizeof(GPP));
	gpp.context=context;
	gpp.options=options;

	char szNameFinal[kPGPMaxUserIDSize];
	GetKeyString(options->mDefaultKey, szNameFinal);
	fprintf(stdout, "\n%s\n", szNameFinal);

	while (1) {
		FreePassphrases(&gpp);
		gpp.pszPassPhrase = (char *)secAlloc (gpp.context, KEYSIZE);
		if (gpp.pszPassPhrase) {
			PGPInt32 len = pgpCLGetPass(stdout, gpp.pszPassPhrase, KEYSIZE);
			if (len < 0) {
				err = kPGPError_UserAbort;
				break;
			}
			
			*(options->mPassphrasePtr) = gpp.pszPassPhrase;
			
			if (PassphraseLengthAndQualityOK(options,gpp.pszPassPhrase)) {
				PGPBoolean PassValid;
				PassValid = PGPPassphraseIsValid(options->mDefaultKey,
									PGPOPassphrase(context, gpp.pszPassPhrase),
									PGPOLastOption(context));
				if (PassValid) {
					err = kPGPError_NoErr;
					break;
				} else {
					fprintf(stdout, "Wrong passphrase, reenter\n");
				}
			} else {
				ClearPassphrases(&gpp);
				FreePassphrases(&gpp);
			}
		} else {
			err = kPGPError_OutOfMemory;
			break;
		}
	}

	ClearPassphrases(&gpp);
	if (err != kPGPError_NoErr)
		FreePassphrases(&gpp);
	return(err);
}

