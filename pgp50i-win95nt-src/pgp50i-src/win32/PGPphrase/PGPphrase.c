/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//:PGPphrase - get cached passphrase
//
//	$Id: PGPphrase.c,v 1.5 1997/05/28 15:45:21 pbj Exp $
//
//	Passphrase caching is implemented as user convenience.
//	The pgplib preferences control whether caching is
//	enabled and for how long.  The code in this file implements
//	the passphrase cache by keeping a global counter and a
//	global pointer variable to the phrase.  When the
//	phrase is entered or accessed, the counter is set to the
//	current timeout value specified by the library preferences.
//	A separate thread is created which awakes every second and
//	decrements the counter.  When the counter reaches zero, the
//	cache (string) is wiped, the string freed and the thread
//  terminates.
//
//	The caller to PGPGetCachedPhrase is responsible for freeing
//	the returned string buffer with PGPFreePhrase.
//

#include "pgpphrasex.h"

#define CACHECLOCKPERIOD	500				// in milliseconds

// local globals
static BOOL bEncryptCacheActive = FALSE;	// TRUE => cache thread is active
static INT iEncryptCacheCounter = 0;		// seconds left before purge
static LPSTR szEncryptCachedPhrase = NULL;	// pointer to phrase

static BOOL bSignCacheActive = FALSE;		// TRUE => cache thread is active
static INT iSignCacheCounter = 0;			// seconds left before purge
static LPSTR szSignCachedPhrase = NULL;		// pointer to phrase
static CHAR szSignCachedKeyID[20];			// cached key keyid
static ULONG ulSignCachedHashAlg;			// cached key hash algorithm
static BOOL bSignCachedDSAKey = FALSE;		// TRUE => cached key is DSA
static UINT uSignCachedOptions = 0;


//----------------------------------------------------|
// check error value.  If no error return FALSE,
// otherwise return TRUE.
//

BOOL PGPphraseError (PGPError err) {
	if (err == PGPERR_OK) return FALSE;
	return TRUE;
}


//----------------------------------------------------|
// Convert entire KeyID from big endian to
// character string format.
// NB: sz must be at least 19 bytes long

VOID ConvertStringKeyID64 (LPSTR sz) {
	INT i;
	ULONG ulFirst, ulSecond;

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


//----------------------------------------------------|
//  thread to clear cache after delay period
//	This routine is executed as a separate thread.  The
//	routine wakes up every second and decrements the
//	counter.  Whenthe counter reaches zero, the cache
//	is purged and the flag is set to indicate cache is
//	not active.

void ClearEncryptCacheThread (PVOID pvoid) {
	while (iEncryptCacheCounter > 0) {
		Sleep (CACHECLOCKPERIOD);
		iEncryptCacheCounter--;
	}
	if (szEncryptCachedPhrase) {
		PGPcomdlgFreePhrase (szEncryptCachedPhrase);
		szEncryptCachedPhrase = NULL;
	}
	bEncryptCacheActive = FALSE;
}

	
//----------------------------------------------------|
//  thread to clear signing cache after delay period
//	This routine is executed as a separate thread.  The
//	routine wakes up every second and decrements the
//	counter.  Whenthe counter reaches zero, the cache
//	is purged and the flag is set to indicate cache is
//	not active.

void ClearSignCacheThread (PVOID pvoid) {
	while (iSignCacheCounter > 0) {
		Sleep (CACHECLOCKPERIOD);
		iSignCacheCounter--;
	}
	if (szSignCachedPhrase) {
		PGPcomdlgFreePhrase (szSignCachedPhrase);
		szSignCachedPhrase = NULL;
	}
	bSignCacheActive = FALSE;
}

	
//---------------------------------------------------|
//	PGPGetCachedPhrase
//	This routine is called to get either cached phrase
//	(if available) or prompt user for phrase.
//
//	hWnd			- parent window
//	szPrompt		- prompt string to be displayed in dialog
//	bForceUserInput	- TRUE => post dialog regardless of cache
//	pszBuffer		- pointer to buffer to hold phrase.
//					  PGPGetCachedPhrase will allocate enough
//					  memory to hold phrase and return pointer.
//					  Caller must call PGPFreePhrase when done, to
//					  deallocate memory.
//

INT PGPGetCachedPhrase (HWND hWnd, LPSTR szPrompt, BOOL bForceUserInput,
						 LPSTR* pszBuffer) {
	UINT u;
	INT iRetval;
	DWORD dw;
	Boolean bCacheEnabled = FALSE;

	iRetval = PGPPHRASE_OK;

	// we rely on pgplib to supply cache parameters
	if (PGPphraseError (pgpLibInit ())) return PGPPHRASE_LIBERROR;
	pgpLoadPrefs ();
	pgpGetPrefBoolean (kPGPPrefMailPassCacheEnable, &bCacheEnabled);
	
	// use phrase from cache
	if (!bForceUserInput && bCacheEnabled &&
			bEncryptCacheActive && szEncryptCachedPhrase) {
		if (!PGPphraseError (pgpGetPrefNumber (
					kPGPPrefMailPassCacheDuration, &u))) {
			iEncryptCacheCounter = (u * 1000) / CACHECLOCKPERIOD;
		}
		*pszBuffer = pgpMemAlloc (lstrlen (szEncryptCachedPhrase) +1);
		if (*pszBuffer) lstrcpy (*pszBuffer, szEncryptCachedPhrase);
		else iRetval = PGPPHRASE_MEMERROR;
	}

	// otherwise, get phrase from user
	else {
		if (szEncryptCachedPhrase) {
			PGPcomdlgFreePhrase (szEncryptCachedPhrase);
			szEncryptCachedPhrase = NULL;
		}
		u = PGPcomdlgGetPhrase (hWnd, szPrompt, &szEncryptCachedPhrase,
									NULL, NULL, NULL, NULL, 0);
		// user entered phrase -- setup cache
		if ((u == PGPCOMDLG_OK) && szEncryptCachedPhrase) {
			if (bCacheEnabled) {
				if (!PGPphraseError (pgpGetPrefNumber (
						kPGPPrefMailPassCacheDuration, &u))) {
					iEncryptCacheCounter = (u * 1000) / CACHECLOCKPERIOD;
				}
				if (!bEncryptCacheActive) {
					bEncryptCacheActive = TRUE;
					_beginthreadex (NULL, 0,
						(LPTHREAD_START_ROUTINE)ClearEncryptCacheThread,
						0, 0, &dw);
				}
			}
			*pszBuffer = pgpMemAlloc (lstrlen (szEncryptCachedPhrase) +1);
			if (*pszBuffer) lstrcpy (*pszBuffer, szEncryptCachedPhrase);
			else iRetval = PGPPHRASE_MEMERROR;
		}
		// problem (cancel or other error) -- flush cache
		else {
			iEncryptCacheCounter = 0;
			if (szEncryptCachedPhrase) {
				PGPcomdlgFreePhrase (szEncryptCachedPhrase);
				szEncryptCachedPhrase = NULL;
			}
			if ((u == PGPCOMDLG_OK) || (u == PGPCOMDLG_CANCEL))
				iRetval = PGPPHRASE_USERCANCEL;
			else if (u == PGPCOMDLG_MEMERROR)
				iRetval = PGPPHRASE_MEMERROR;
			else
				iRetval = u;
		}
	}

	pgpLibCleanup ();
	return iRetval;
}


//---------------------------------------------------|
//	PGPGetSignCachedPhrase
//	This routine is called to get either signing cached phrase
//	(if available) or prompt user for phrase.
//

INT PGPGetSignCachedPhrase (HWND hWnd, LPSTR szPrompt, BOOL bForceUserInput,
						 LPSTR* pszBuffer, LPSTR szKeyID, ULONG* pulHashAlg,
						 UINT* puOptions, UINT uFlags) {
	UINT u, uError;
	INT iRetval;
	DWORD dw;
	Boolean bCacheEnabled = FALSE;
	PGPKey* pKey;
	PGPKeySet* pKeySet;
	PGPKeySet* pKeySetToUse;
	BOOL bAskUser = TRUE;

	iRetval = PGPPHRASE_OK;

	// we rely on pgplib to supply cache parameters
	if (PGPphraseError (pgpLibInit ())) return PGPPHRASE_LIBERROR;
	pgpLoadPrefs ();
	pgpGetPrefBoolean (kPGPPrefSignCacheEnable, &bCacheEnabled);
	
	// use phrase from cache
	if (!bForceUserInput && bCacheEnabled &&
			bSignCacheActive && szSignCachedPhrase) {
		if (bSignCachedDSAKey && (uFlags & PGPCOMDLG_RSAENCRYPT)) {
			iRetval = PGPcomdlgRSADSAMixWarning (hWnd);
			switch (iRetval) {
			case PGPCOMDLG_OK :
				iRetval = PGPPHRASE_OK;
				break;

			case PGPCOMDLG_CANCEL :
				iRetval = PGPPHRASE_USERCANCEL;
				bAskUser = FALSE;
				break;

			case PGPCOMDLG_NO :
				bAskUser = TRUE;
				break;
			}
		}
		if (iRetval == PGPPHRASE_OK) {
			if (!PGPphraseError (pgpGetPrefNumber (
					kPGPPrefSignCacheDuration, &u))) {
				iSignCacheCounter =  (u * 1000) / CACHECLOCKPERIOD;
			}
			*pszBuffer = pgpMemAlloc (lstrlen (szSignCachedPhrase) +1);
			if (*pszBuffer) {
				lstrcpy (*pszBuffer, szSignCachedPhrase);
				if (szKeyID) lstrcpy (szKeyID, szSignCachedKeyID);
				if (pulHashAlg) *pulHashAlg = ulSignCachedHashAlg;
				if (puOptions) *puOptions = uSignCachedOptions;
			}
			else iRetval = PGPPHRASE_MEMERROR;
			bAskUser = FALSE;
		}
	}

	// otherwise, get phrase from user
	if (bAskUser) {
		// open keyring and get keyset
		pKeySet = pgpOpenDefaultKeyRings (FALSE, NULL);
		if (!pKeySet) iRetval = PGPPHRASE_KEYRINGERROR;
		else {
			if (szSignCachedPhrase) {
				PGPcomdlgFreePhrase (szSignCachedPhrase);
				szSignCachedPhrase = NULL;
			}
			// call PGPcomdlg routine to post passphrase dialog
			if (szKeyID) pKeySetToUse = pKeySet;
			else pKeySetToUse = NULL;
			pKey = NULL;
			uError = PGPcomdlgGetPhrase (hWnd, szPrompt, &szSignCachedPhrase,
										 pKeySet, &pKey, szKeyID, puOptions,
										 uFlags);
			// user entered phrase -- setup cache
			if ((uError == PGPCOMDLG_OK) && szSignCachedPhrase) {
				if (bCacheEnabled) {
					if (!PGPphraseError (pgpGetPrefNumber (
						kPGPPrefSignCacheDuration, &u))) {
						iSignCacheCounter =  (u * 1000) / CACHECLOCKPERIOD;
					}
					if (!bSignCacheActive) {
						bSignCacheActive = TRUE;
						_beginthreadex (NULL, 0,
							(LPTHREAD_START_ROUTINE)ClearSignCacheThread,
							0, 0, &dw);
					}
				}
				pgpGetHashAlgUsed (pKey, &ulSignCachedHashAlg);
				u = sizeof (szSignCachedKeyID);
				pgpGetKeyString (pKey, kPGPKeyPropKeyId,
									szSignCachedKeyID, &u);
				szSignCachedKeyID[u] = '\0';
				ConvertStringKeyID64 (szSignCachedKeyID);
				if (puOptions) uSignCachedOptions = *puOptions;
				else uSignCachedOptions = 0;
				*pszBuffer = pgpMemAlloc (lstrlen (szSignCachedPhrase) +1);
				if (*pszBuffer) {
					lstrcpy (*pszBuffer, szSignCachedPhrase);
					if (szKeyID) lstrcpy (szKeyID, szSignCachedKeyID);
					if (pulHashAlg) *pulHashAlg = ulSignCachedHashAlg;
				}
				else iRetval = PGPPHRASE_MEMERROR;
			}
			// problem (cancel or other error) -- flush cache
			else {
				iSignCacheCounter = 0;
				if (szSignCachedPhrase) {
					PGPcomdlgFreePhrase (szSignCachedPhrase);
					szSignCachedPhrase = NULL;
				}
				if ((uError == PGPCOMDLG_OK) || (uError == PGPCOMDLG_CANCEL))
					iRetval = PGPPHRASE_USERCANCEL;
				else if (uError == PGPCOMDLG_MEMERROR)
					iRetval = PGPPHRASE_MEMERROR;
				else
					iRetval = u;
			}
			pgpFreeKeySet (pKeySet);
		}
	}

	pgpLibCleanup ();
	return iRetval;
}


//---------------------------------------------------|
//	PGPPurgeCachedPhrase
//	Called to purge phrase from cache.

BOOL PGPPurgeCachedPhrase (void) {
	iEncryptCacheCounter = 0;
	if (szEncryptCachedPhrase) {
		PGPcomdlgFreePhrase (szEncryptCachedPhrase);
		szEncryptCachedPhrase = NULL;
	}
	return bEncryptCacheActive;
}


//---------------------------------------------------|
//	PGPPurgeSignCachedPhrase
//	Called to purge phrase from cache.

BOOL PGPPurgeSignCachedPhrase (void) {
	iSignCacheCounter = 0;
	if (szSignCachedPhrase) {
		PGPcomdlgFreePhrase (szSignCachedPhrase);
		szSignCachedPhrase = NULL;
	}
	return bSignCacheActive;
}


//---------------------------------------------------|
//	PGPSetCachedPhrase
//	Called to set phrase in cache.

INT PGPSetCachedPhrase (LPSTR szPhrase) {
	UINT u;
	INT iRetval;
	DWORD dw;
	Boolean bCacheEnabled = FALSE;

	// we rely on pgplib to supply cache parameters
	if (PGPphraseError (pgpLibInit ())) return PGPPHRASE_LIBERROR;
	pgpLoadPrefs ();
	pgpGetPrefBoolean (kPGPPrefMailPassCacheEnable, &bCacheEnabled);

	if (szEncryptCachedPhrase) {
		PGPcomdlgFreePhrase (szEncryptCachedPhrase);
		szEncryptCachedPhrase = NULL;
	}

	szEncryptCachedPhrase = pgpMemAlloc (lstrlen (szPhrase) +1);
	if (szEncryptCachedPhrase) {
		lstrcpy (szEncryptCachedPhrase, szPhrase);
		if (bCacheEnabled) {
			if (!PGPphraseError (pgpGetPrefNumber (
					kPGPPrefMailPassCacheDuration, &u))) {
				iEncryptCacheCounter =  (u * 1000) / CACHECLOCKPERIOD;
			}
			if (!bEncryptCacheActive) {
				bEncryptCacheActive = TRUE;
				_beginthreadex (NULL, 0,
					(LPTHREAD_START_ROUTINE)ClearEncryptCacheThread,
					0, 0, &dw);
			}
		}
		iRetval = PGPPHRASE_OK;
	}
	else iRetval = PGPPHRASE_MEMERROR;

	pgpLibCleanup ();
	return iRetval;
}

/*
//---------------------------------------------------|
//	PGPSetSignCachedPhrase
//	Called to set phrase in cache.

INT PGPSetSignCachedPhrase (LPSTR szPhrase) {
	UINT u;
	INT iRetval;
	DWORD dw;
	Boolean bCacheEnabled = FALSE;

	// we rely on pgplib to supply cache parameters
	if (PGPphraseError (pgpLibInit ())) return PGPPHRASE_LIBERROR;
	pgpLoadPrefs ();
	pgpGetPrefBoolean (kPGPPrefSignCacheEnable, &bCacheEnabled);

	if (szSignCachedPhrase) {
		PGPcomdlgFreePhrase (szSignCachedPhrase);
		szSignCachedPhrase = NULL;

	}

	szSignCachedPhrase = pgpMemAlloc (lstrlen (szPhrase) +1);
	if (szSignCachedPhrase) {
		lstrcpy (szSignCachedPhrase, szPhrase);
		if (bCacheEnabled) {
			if (!PGPphraseError (pgpGetPrefNumber (
					kPGPPrefSignCacheDuration, &u))) {
				iSignCacheCounter =  (u * 1000) / CACHECLOCKPERIOD;
			}
			if (!bSignCacheActive) {
				bSignCacheActive = TRUE;
				_beginthreadex (NULL, 0,
					(LPTHREAD_START_ROUTINE)ClearSignCacheThread,
					0, 0, &dw);
			}
		}
		iRetval = PGPPHRASE_OK;
	}
	else iRetval = PGPPHRASE_MEMERROR;

	pgpLibCleanup ();
	return iRetval;
}
*/

//---------------------------------------------------|
//	PGPFreeCachedPhrase
//	Called to wipe and free the phrase returned by
//	PGPGetCachedPhrase.

void PGPFreePhrase (LPSTR szPhrase) {
	FillMemory (szPhrase, lstrlen (szPhrase), '\0');
	pgpMemFree (szPhrase);
}


//---------------------------------------------------|
//	PGPQueryCacheSecsRemaining
//	Returns number of seconds remaining until cache expires

INT PGPQueryCacheSecsRemaining (void) {
	return (iEncryptCacheCounter);	
}


//---------------------------------------------------|
//	PGPQuerySignCacheSecsRemaining
//	Returns number of seconds remaining until cache expires

INT PGPQuerySignCacheSecsRemaining (void) {
	return (iSignCacheCounter);	
}
