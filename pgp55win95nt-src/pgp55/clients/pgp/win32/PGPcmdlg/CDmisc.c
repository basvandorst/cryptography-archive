/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPcomdlg.c - PGP common dialogs DLL utility routines
	

	$Id: CDmisc.c,v 1.6.4.1 1997/11/11 02:15:19 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"	

#include "pgpcdlgx.h"
#include <math.h>

#define YTEXTINC	16

// DLL globals
HINSTANCE g_hInst;

//	__________________________________________________
//
//	save user information to preferences file 

PGPError PGPcdExport 
PGPcomdlgSetUserInfo (
		LPSTR	szOwnerName,
		LPSTR	szCompanyName,
		LPSTR	szLicenseNumber)
{
	PGPError	err;
	PGPPrefRef	prefref;

	err = PGPcomdlgOpenClientPrefs (&prefref);
	if (IsPGPError (err)) return err;

	if (szOwnerName) {
		PGPSetPrefString (prefref, kPGPPrefOwnerName, szOwnerName);
	}

	if (szCompanyName) {
		PGPSetPrefString (prefref, kPGPPrefCompanyName, szCompanyName);
	}

#if (PGP_NO_LICENSE_NUMBER == 0)
	if (szLicenseNumber) {
		PGPSetPrefString (prefref, kPGPPrefLicenseNumber, szLicenseNumber);
	}
#endif //PGP_NO_LICENSE_NUMBER

	err = PGPcomdlgCloseClientPrefs (prefref, TRUE);

	return err;
}


//	_________________________________________________
//
//	Wrapper routine for platform independent 
//	word wrap code.

PGPError PGPcdExport 
PGPcomdlgWrapBuffer (LPSTR szInText,
					 PGPUInt16 wrapColumn,
					 LPSTR* pszOutText)
{
	PGPError		err				= kPGPError_NoErr;
	PGPIORef		inRef, outRef;
	PFLContextRef	pflcontext;
	PGPFileOffset	outSize;
	PGPSize			outRead;
	INT				InSize;

	outRead = 0;

	err = PFLNewContext (&pflcontext);

	if (IsntPGPError (err))
	{
		PGPNewMemoryIO (
			pflcontext,
			(PGPMemoryIORef *)(&inRef));

		InSize = strlen (szInText);
		PGPIOWrite (inRef,
			InSize,
			szInText);
		PGPIOFlush (inRef);
		PGPIOSetPos (inRef,0);

		PGPNewMemoryIO (
			pflcontext,
			(PGPMemoryIORef *)(&outRef));

		err = pgpWordWrapIO(
			  inRef,outRef,
			  wrapColumn,
			  "\r\n");

		if (IsntPGPError (err))
		{
			INT memamt;

			PGPIOGetPos (outRef, &outSize);

			memamt = (INT)outSize+1;
			*pszOutText = (CHAR *)malloc (memamt);

			if (*pszOutText)
			{
				memset (*pszOutText, 0x00, memamt);

				PGPIOSetPos (outRef,0);
				PGPIORead (outRef,
					(INT)outSize,
					*pszOutText, 
					&outRead);
				PGPIOFlush (outRef);
			}
			else err = kPGPError_OutOfMemory;
		}
		PGPFreeIO (inRef); 
		PGPFreeIO (outRef);
	}
	PFLFreeContext (pflcontext);

	return err;
}

//	__________________________________________________
//
//	free previously-wrapped buffer

PGPError PGPcdExport 
PGPcomdlgFreeWrapBuffer (LPSTR textBuffer)
{
	memset (textBuffer, 0x00, lstrlen (textBuffer));
	free (textBuffer);

	return kPGPError_NoErr;
}


//	________________________
//
//	Get keyid string from key

PGPError PGPcdExport 
PGPcomdlgGetKeyIDFromKey (PGPKeyRef Key, 
						  BOOL bFull,
						  LPSTR sz, 
						  UINT uLen) 
{
	PGPKeyID			KeyID;
	CHAR				szID[kPGPMaxKeyIDStringSize];

	if (bFull) {
		if (uLen < 19) return kPGPError_BadParams;
	}
	else {
		if (uLen < 11) return kPGPError_BadParams;
	}

	PGPGetKeyIDFromKey (Key, &KeyID);
	PGPGetKeyIDString (&KeyID, kPGPKeyIDString_Full, szID);
	lstrcpy (sz, "0x");
	if (bFull) lstrcat (sz, &szID[2]);
	else lstrcat (sz, &szID[10]);

	return kPGPError_NoErr;
}


//	________________________
//
//	Get keyid string from key

PGPError PGPcdExport 
PGPcomdlgGetKeyFromKeyID (PGPContextRef context,
						  PGPKeySetRef keyset,
						  LPSTR szID,
						  UINT alg,
						  PGPKeyRef* Key) 
{
	PGPError		err; 
	PGPKeyID		keyID;

	err = PGPGetKeyIDFromString (szID, &keyID);
	if (IsntPGPError (err)) {
		err = PGPGetKeyByKeyID (keyset, &keyID, alg, Key);
		if (!(*Key)) 
			err = kPGPError_ItemNotFound;
	}

	return err;
}

//	________________________
//
//	find out if SDK supports the specified public key algorithm

BOOL PGPcdExport 
PGPcomdlgCheckSDKSupportForPKAlg (
		PGPPublicKeyAlgorithm alg,
		PGPBoolean mustEncrypt,
		PGPBoolean mustSign)
{
	PGPError	err			= kPGPError_FeatureNotAvailable;

	PGPUInt32					i, iNumAlgs;
	PGPPublicKeyAlgorithmInfo	alginfo;

	err = PGPCountPublicKeyAlgorithms (&iNumAlgs);
	if (IsPGPError (err)) return kPGPError_FeatureNotAvailable;

	for (i=0; i<iNumAlgs; i++) {
		err = PGPGetIndexedPublicKeyAlgorithmInfo (i, &alginfo);
		if (IsntPGPError (err)) {
			if (alginfo.algID == alg) {
				err = kPGPError_NoErr;
				if (mustEncrypt && !(alginfo.canEncrypt))
					err = kPGPError_FeatureNotAvailable;
				if (mustSign && !(alginfo.canSign))
					err = kPGPError_FeatureNotAvailable;
				return err;
			}
		}
	}

	return kPGPError_FeatureNotAvailable;
}

//	________________________
//
//	find out if SDK supports the specified cipher algorithm

BOOL PGPcdExport 
PGPcomdlgCheckSDKSupportForCipherAlg (
		PGPCipherAlgorithm alg)
{
	PGPError	err			= kPGPError_FeatureNotAvailable;

	PGPUInt32					i, iNumAlgs;
	PGPSymmetricCipherInfo		cipherinfo;

	err = PGPCountSymmetricCiphers (&iNumAlgs);
	if (IsPGPError (err)) return kPGPError_FeatureNotAvailable;

	for (i=0; i<iNumAlgs; i++) {
		err = PGPGetIndexedSymmetricCipherInfo (i, &cipherinfo);
		if (IsntPGPError (err)) {
			if (cipherinfo.algID == alg) {
				err = kPGPError_NoErr;
				return err;
			}
		}
	}

	return kPGPError_FeatureNotAvailable;
}

//	________________________
//
//	sync up keysets to resolve library implementation problems

PGPError PGPcdExport 
PGPcomdlgSyncKeySets (
		PGPContextRef context,
		PGPKeySetRef keysetMain,
		PGPKeySetRef keysetNew)
{

	PGPKeySetRef	keysetCombined;
	PGPKeySetRef	keysetSync;
	PGPError		err;

	if (!PGPRefIsValid (keysetNew)) return kPGPError_NoErr;

	if (!PGPRefIsValid (keysetMain)) {
		PGPsdkLoadDefaultPrefs (context);
		err = PGPOpenDefaultKeyRings (context, 0, &keysetSync);
		if (IsPGPError (err) || !PGPRefIsValid (keysetSync)) return err;
	}
	else keysetSync = keysetMain;

	err = PGPNewKeySet (context, &keysetCombined);
	if (IsntPGPError (err)) {

		err = PGPAddKeys (keysetNew, keysetCombined);
		if (IsntPGPError (err)) {

			err = PGPAddKeys (keysetSync, keysetCombined);
			if (IsntPGPError (err)) {

				err = PGPCheckKeyRingSigs (keysetNew, 
									keysetCombined, FALSE, NULL, 0);

				if (IsntPGPError (err)) {
					err = PGPPropagateTrust (keysetCombined);
				}
			}
		}

		PGPFreeKeySet (keysetCombined);
	}

	if (!PGPRefIsValid (keysetMain)) PGPFreeKeySet (keysetSync);

	return err;
}

//	________________________
//
//	Calculate passphrase quality

#define NUMRANGES	8

PGPError PGPcdExport
PGPcomdlgPassphraseQuality (LPSTR szPhrase)
{
	UINT	quality;
	USHORT	charIndex, rangeIndex, span;
	typedef struct ScoreStruct
	{
		uchar	firstChar;
		uchar	lastChar;
	} Range;
	static Range sRanges[] =
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
	BOOL haveRange[NUMRANGES];
	
	pgpClearMemory (haveRange, sizeof (haveRange));
	for (charIndex = 1; charIndex < lstrlen (szPhrase); ++charIndex) {
		for (rangeIndex = 0; rangeIndex < NUMRANGES; ++rangeIndex)	{
			UCHAR			theChar;
			Range *			theRange;
			
			theChar		= szPhrase[charIndex];
			theRange	= &sRanges[rangeIndex];
			
			if (theChar >= theRange->firstChar && 
				theChar <= theRange->lastChar) 
			{
				haveRange[rangeIndex] 	= TRUE;
				break;
			}
		}
	}
	
	// determine the total span
	span	= 0;
	for (rangeIndex = 0; rangeIndex < NUMRANGES; ++rangeIndex) {
		if (haveRange[rangeIndex]) {
			span += 1 + (sRanges[rangeIndex].lastChar -
				sRanges[rangeIndex].firstChar);
		}
	}
	
	if (span != 0) {
		double	bitsPerChar	= log (span) / log (2.0);  // base 2 log of span
		ULONG	totalBits	= (ULONG)(bitsPerChar * lstrlen (szPhrase));
		
		// assume a maximum quality of 128 bits
		quality = (totalBits * 100) / 128;
		if (quality > 100) quality = 100;
	}
	else {
		quality	= 0;
	}
		
	return (quality);
}

	