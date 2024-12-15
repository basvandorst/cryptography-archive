/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPphras.c - passphrase caching routines
	
	Passphrase caching is implemented as user convenience.
	The pgplib preferences control whether caching is 
	enabled and for how long.  The code in this file implements
	the passphrase cache by keeping a global counter and a
	global pointer variable to the phrase.  When the
	phrase is entered or accessed, the counter is set to the
	current timeout value specified by the library preferences.
	A separate thread is created which awakes every second and
	decrements the counter.  When the counter reaches zero, the
	cache (string) is wiped, the string freed and the thread 
	terminates. 

	The caller to PGPGetCachedPhrase is responsible for freeing
	the returned string buffer with PGPFreePhrase.
	

	$Id: PGPphras.c,v 1.20 1997/10/21 13:36:41 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"	

#include "pgpphrsx.h"

#define CACHECLOCKPERIOD	500				// in milliseconds

// local globals
static BOOL	bEncryptCacheActive = FALSE;	// TRUE => cache thread is active
static INT iEncryptCacheCounter = 0;		// seconds left before purge
static LPSTR pszEncryptCachedPhrase = NULL;	// pointer to phrase

static BOOL	bSignCacheActive = FALSE;		// TRUE => cache thread is active
static INT iSignCacheCounter = 0;			// seconds left before purge
static LPSTR pszSignCachedPhrase = NULL;		// pointer to phrase
static PGPByte SignCachedKeyID[kPGPMaxExportedKeyIDSize];	// cached keyid
static PGPSize CachedKeyIDSize = 0;			// size of cached keyid
static PGPHashAlgorithm ulSignCachedHashAlg = 0;	// cached key hash alg
static BOOL bSignCachedDSAKey = FALSE;		// TRUE => cached key is DSA
static UINT uSignCachedOptions = 0;


//___________________________
//
// Memory allocation routines
//

VOID* 
ppAlloc (PGPContextRef context, UINT uBytes) 
{
	return (PGPNewSecureData (context, uBytes, NULL));
}


VOID 
ppFree (VOID* p) 
{
	PGPFreeData (p);
}

//_____________________________________________
//
// check error value.  If no error return FALSE, 
// otherwise return TRUE.
//

BOOL 
PGPphraseError (PGPError err) 
{
	if (IsntPGPError (err)) return FALSE;
	return TRUE;
}

//________________________________________
//
// Convert entire KeyID from big endian to 
// character string format. 
// NB: sz must be at least 19 bytes long

VOID 
ConvertStringKeyID64 (LPSTR sz) 
{
	INT		i;
	ULONG	ulFirst, ulSecond;

	// convert first half of keyid to little endian
	ulFirst = 0;
	for (i=0; i<4; i++) {
		ulFirst <<= 8;
		ulFirst |= ((ULONG)sz[i] & 0xFF);
	}

	// convert first half of keyid to little endian
	ulSecond = 0;
	for (i=0; i<4; i++) {
		ulSecond <<= 8;
		ulSecond |= ((ULONG)sz[i+4] & 0xFF);
	}

	// convert to string
	wsprintf (sz, "0x%08lX%08lX", ulFirst, ulSecond);
}

//_____________________________________________________
//
//  thread to clear cache after delay period
//	This routine is executed as a separate thread.  The
//	routine wakes up every second and decrements the
//	counter.  Whenthe counter reaches zero, the cache 
//	is purged and the flag is set to indicate cache is 
//	not active.

VOID
ClearEncryptCacheThread (PVOID pvoid) 
{
	while (iEncryptCacheCounter > 0) {
		Sleep (CACHECLOCKPERIOD);
		iEncryptCacheCounter--;
	}
	if (pszEncryptCachedPhrase) {
		PGPcomdlgFreePhrase (pszEncryptCachedPhrase);
		pszEncryptCachedPhrase = NULL;
	}
	bEncryptCacheActive = FALSE;
}

	
//_____________________________________________________
//
//  thread to clear signing cache after delay period
//	This routine is executed as a separate thread.  The
//	routine wakes up every second and decrements the
//	counter.  Whenthe counter reaches zero, the cache 
//	is purged and the flag is set to indicate cache is 
//	not active.

VOID 
ClearSignCacheThread (PVOID pvoid) 
{
	while (iSignCacheCounter > 0) {
		Sleep (CACHECLOCKPERIOD);
		iSignCacheCounter--;
	}
	if (pszSignCachedPhrase) {
		PGPcomdlgFreePhrase (pszSignCachedPhrase);
		pszSignCachedPhrase = NULL;
	}
	FillMemory (SignCachedKeyID, kPGPMaxExportedKeyIDSize, 0);
	bSignCacheActive = FALSE;
}

	
//_____________________________________________________
//
//	PGPGetCachedPhrase
//	This routine is called to get either cached phrase 
//	(if available) or prompt user for phrase.
//
//

PGPError 
PGPGetCachedPhrase (
		PGPContextRef	Context, 
		HWND			hWnd, 
		LPSTR			szPrompt, 
		BOOL			bForceUserInput,
		LPSTR*			pszBuffer,
		PGPKeySetRef	KeySet,
		PGPKeyID*		pKeyIDs,
		UINT			uKeyCount) 
{
	UINT			u;
	PGPError		Retval;
	DWORD			dw;
	PGPBoolean		bCacheEnabled = FALSE;
	PGPPrefRef		PrefRef;

	Retval = kPGPError_NoErr;

	// we rely on pgplib to supply cache parameters
	PGPcomdlgOpenClientPrefs (&PrefRef);
	PGPGetPrefBoolean (PrefRef, kPGPPrefDecryptCacheEnable, &bCacheEnabled);
	
	// use phrase from cache
	if (!bForceUserInput && bCacheEnabled && 
			bEncryptCacheActive && pszEncryptCachedPhrase) {
		u = 0;
		PGPGetPrefNumber (PrefRef, kPGPPrefDecryptCacheSeconds, &u);
		if (u > 0) {
			iEncryptCacheCounter = (u * 1000) / CACHECLOCKPERIOD;
		}
		*pszBuffer = ppAlloc (Context, lstrlen (pszEncryptCachedPhrase) +1);
		if (*pszBuffer) lstrcpy (*pszBuffer, pszEncryptCachedPhrase);
		else Retval = kPGPError_OutOfMemory;
	}

	// otherwise, get phrase from user
	else {
		if (pszEncryptCachedPhrase) {
			PGPcomdlgFreePhrase (pszEncryptCachedPhrase);
			pszEncryptCachedPhrase = NULL;
		}
		u = PGPcomdlgGetPhrase (Context, hWnd, szPrompt, 
								&pszEncryptCachedPhrase, 
								KeySet, pKeyIDs, uKeyCount,
								NULL, NULL, PGPCOMDLG_DECRYPTION);
		// user entered phrase -- setup cache
		if ((u == kPGPError_NoErr) && pszEncryptCachedPhrase) {
			if (bCacheEnabled) {
				u = 0;
				PGPGetPrefNumber (PrefRef,kPGPPrefDecryptCacheSeconds, &u);
				if (u > 0) {
					iEncryptCacheCounter = (u * 1000) / CACHECLOCKPERIOD;
				}
				if (!bEncryptCacheActive) {
					bEncryptCacheActive = TRUE;
					_beginthreadex (NULL, 0, 
						(LPTHREAD_START_ROUTINE)ClearEncryptCacheThread, 
						0, 0, &dw);
				}
			}
			*pszBuffer = ppAlloc (Context, 
								lstrlen (pszEncryptCachedPhrase) +1);
			if (*pszBuffer) lstrcpy (*pszBuffer, pszEncryptCachedPhrase);
			else Retval = kPGPError_OutOfMemory;
			if (!bCacheEnabled) {
				PGPcomdlgFreePhrase (pszEncryptCachedPhrase);
				pszEncryptCachedPhrase = NULL;
			}
		}
		// problem (cancel or other error) -- flush cache
		else {
			iEncryptCacheCounter = 0;
			if (pszEncryptCachedPhrase) {
				PGPcomdlgFreePhrase (pszEncryptCachedPhrase);
				pszEncryptCachedPhrase = NULL;
			}
			if ((u == kPGPError_NoErr) || (u == kPGPError_UserAbort)) 
				Retval = kPGPError_UserAbort;
			else if (u == kPGPError_OutOfMemory) 
				Retval = kPGPError_OutOfMemory;
			else 
				Retval = u;
		}
	}
	PGPcomdlgCloseClientPrefs (PrefRef, FALSE);
	return Retval;
}

//_____________________________________________________
//
//	PGPGetSignCachedPhrase
//	This routine is called to get either signing cached phrase 
//	(if available) or prompt user for phrase.
//

PGPError 
PGPGetSignCachedPhrase (
		PGPContextRef		Context, 
		HWND				hWnd,
		LPSTR				szPrompt,
		BOOL				bForceUserInput, 
		LPSTR*				pszBuffer, 
		PGPKeySetRef		KeySet,
		PGPKeyRef*			pKey,
		PGPHashAlgorithm*	pulHashAlg,
		UINT*				puOptions, 
		UINT				uFlags) 
{
	UINT			u				= 0;
	PGPError		Retval			= kPGPError_NoErr;
	PGPError		err				= kPGPError_NoErr;
	PGPBoolean		bCacheEnabled	= FALSE;
	BOOL			bAskUser		= TRUE;
	PGPPrefRef		PrefRef;
	PGPKeyID		KeyID;
	DWORD			dw;

	// we rely on preferences to supply cache parameters
	PGPcomdlgOpenClientPrefs (&PrefRef);
	PGPGetPrefBoolean (PrefRef, kPGPPrefSignCacheEnable, &bCacheEnabled);
	
	// use phrase from cache
	if (!bForceUserInput && bCacheEnabled && 
			bSignCacheActive && pszSignCachedPhrase) {
		if (bSignCachedDSAKey && (uFlags & PGPCOMDLG_RSAENCRYPT)) {
			Retval = PGPcomdlgRSADSAMixWarning (hWnd);
			switch (Retval) {
			case kPGPError_NoErr :
				bAskUser = TRUE;
				break;

			case kPGPError_UserAbort :
				bAskUser = FALSE;
				break;

			case kPGPError_Win32_NegativeResponse :
				bAskUser = TRUE;
				break;
			}
		}
		if (Retval == kPGPError_NoErr) {
			u = 0;

			PGPGetPrefNumber (PrefRef, kPGPPrefSignCacheSeconds, &u);
			if (u > 0) {
				iSignCacheCounter =  (u * 1000) / CACHECLOCKPERIOD;
			}

			// see if cached key is in keyset
			if (pKey) {
				err = PGPImportKeyID (SignCachedKeyID, &KeyID);
				if (IsntPGPError (err)) {

					// if so, get keyref for this keyid
					err = PGPGetKeyByKeyID (KeySet, &KeyID, 
							kPGPPublicKeyAlgorithm_Invalid, pKey);
					if (IsPGPError (err)) *pKey = NULL;
				}

				*pszBuffer = ppAlloc (Context, 
									lstrlen (pszSignCachedPhrase) +1);
				if (*pszBuffer) {
					lstrcpy (*pszBuffer, pszSignCachedPhrase);
					if (pulHashAlg) *pulHashAlg = ulSignCachedHashAlg;
					if (puOptions) *puOptions = uSignCachedOptions;
				}
				else Retval = kPGPError_OutOfMemory;

				bAskUser = FALSE;
			}
		}
	}

	// otherwise, get phrase from user
	if (bAskUser) {
		if (pszSignCachedPhrase) {
			PGPcomdlgFreePhrase (pszSignCachedPhrase);
			pszSignCachedPhrase = NULL;
		}
		FillMemory (SignCachedKeyID, kPGPMaxExportedKeyIDSize, 0);
		// call PGPcomdlg routine to post passphrase dialog
		err = PGPcomdlgGetPhrase (Context, hWnd, szPrompt, 
									 &pszSignCachedPhrase, 
									 KeySet, NULL, 0, pKey, puOptions, 
									 uFlags);
		// user entered phrase -- setup cache
		if ((err == kPGPError_NoErr) && pszSignCachedPhrase) {
			if (bCacheEnabled) {
				u = 0;
				PGPGetPrefNumber (PrefRef, kPGPPrefSignCacheSeconds, &u);
				if (u > 0) {
					iSignCacheCounter =  (u * 1000) / CACHECLOCKPERIOD;
				}
				if (!bSignCacheActive) {
					bSignCacheActive = TRUE;
					_beginthreadex (NULL, 0, 
						(LPTHREAD_START_ROUTINE)ClearSignCacheThread, 
						0, 0, &dw);
				}
			}
			if (puOptions) uSignCachedOptions = *puOptions;
			else uSignCachedOptions = 0;
			*pszBuffer = ppAlloc (Context, lstrlen (pszSignCachedPhrase) +1);
			if (*pszBuffer) {
				lstrcpy (*pszBuffer, pszSignCachedPhrase);
				if (pulHashAlg) *pulHashAlg = ulSignCachedHashAlg;
			}
			else Retval = kPGPError_OutOfMemory;

			if (bCacheEnabled) {
				PGPGetHashAlgUsed (*pKey, &ulSignCachedHashAlg);
				PGPGetKeyIDFromKey (*pKey, &KeyID);
				PGPExportKeyID (&KeyID, SignCachedKeyID, &CachedKeyIDSize);
			}
			else {
				PGPcomdlgFreePhrase (pszSignCachedPhrase);
				pszSignCachedPhrase = NULL;
			}
		}
		// problem (cancel or other error) -- flush cache
		else {
			iSignCacheCounter = 0;
			if (pszSignCachedPhrase) {
				PGPcomdlgFreePhrase (pszSignCachedPhrase);
				pszSignCachedPhrase = NULL;
			}
			if ((err == kPGPError_NoErr)||(err == kPGPError_UserAbort))
				Retval = kPGPError_UserAbort;
			else if (err == kPGPError_OutOfMemory) 
				Retval = kPGPError_OutOfMemory;
			else 
				Retval = err;
		}
	}

	PGPcomdlgCloseClientPrefs (PrefRef, FALSE);
	return Retval;
}

//___________________________________
//
//	PGPPurgeCachedPhrase
//	Called to purge phrase from cache.

BOOL 
PGPPurgeCachedPhrase (void) 
{
	iEncryptCacheCounter = 0;
	if (pszEncryptCachedPhrase) {
		PGPcomdlgFreePhrase (pszEncryptCachedPhrase);
		pszEncryptCachedPhrase = NULL;
	}
	return bEncryptCacheActive;
}

//____________________________________
//
//	PGPPurgeSignCachedPhrase
//	Called to purge phrase from cache.

BOOL 
PGPPurgeSignCachedPhrase (void) 
{
	iSignCacheCounter = 0;
	if (pszSignCachedPhrase) {
		PGPcomdlgFreePhrase (pszSignCachedPhrase);
		pszSignCachedPhrase = NULL;
	}
	FillMemory (SignCachedKeyID, kPGPMaxExportedKeyIDSize, 0);
	return bSignCacheActive;
}

//________________________________________________
//
//	PGPFreePhrase
//	Called to wipe and free the phrase returned by
//	PGPGetCachedPhrase.

VOID 
PGPFreePhrase (LPSTR szPhrase) 
{
	FillMemory (szPhrase, lstrlen (szPhrase), '\0');
	ppFree (szPhrase);
}

//_________________________________________________________
//
//	PGPQueryCacheSecsRemaining
//	Returns number of seconds remaining until cache expires

INT 
PGPQueryCacheSecsRemaining (VOID) 
{
	return (iEncryptCacheCounter);	
}

//_________________________________________________________
//
//	PGPQuerySignCacheSecsRemaining
//	Returns number of seconds remaining until cache expires

INT 
PGPQuerySignCacheSecsRemaining (VOID) 
{
	return (iSignCacheCounter);	
}


