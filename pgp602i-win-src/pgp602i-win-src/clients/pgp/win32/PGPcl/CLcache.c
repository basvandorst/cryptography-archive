/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLcache.c - passphrase caching routines
	
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

	The caller to PGPclGetCachedPhrase is responsible for freeing
	the returned string buffer with PGPFreePhrase.
	

	$Id: CLcache.c,v 1.5.2.1 1998/09/15 20:35:47 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"	

#include "pgpclx.h"
#include <process.h>

#define SCRAMBLERLENGTH		256
#define CACHECLOCKPERIOD	500				// in milliseconds

// typedefs
typedef struct {
	BOOL				bCacheActive;
	INT					iCacheCounter;
	LPSTR				pszCachedPassphrase;
	PGPUInt32			iCachedPassphraseLength;
	PGPByte*			pCachedPasskey;
	PGPUInt32			iCachedPasskeyLength;

	PGPByte				CachedKeyID[kPGPMaxExportedKeyIDSize];
	PGPInt32			CachedKeyAlg;
	PGPHashAlgorithm	CachedHashAlg;
	UINT				uCachedOptions;
} CACHE, *PCACHE;

static CACHE	cacheDecryption;
static CACHE	cacheSigning;

static CHAR		szScrambler[SCRAMBLERLENGTH];
static BOOL		bScramblerInitialized = FALSE;

CRITICAL_SECTION	criticalSectionCache;

//___________________________
//
// Memory allocation routines
//

static VOID* 
secAlloc (PGPContextRef context, UINT uBytes) 
{
	return (PGPNewSecureData (PGPGetContextMemoryMgr (context), uBytes, 0));
}


static VOID 
secFree (VOID* p) 
{
	PGPFreeData (p);
}


//_____________________________________________________
//

static VOID
sEmptyCache (PCACHE pcache)
{
	if (pcache->pszCachedPassphrase) 
	{
		secFree (pcache->pszCachedPassphrase);
		pcache->pszCachedPassphrase = NULL;
		pcache->iCachedPassphraseLength = 0;
	}

	if (pcache->pCachedPasskey) 
	{
		secFree (pcache->pCachedPasskey);

		pcache->pCachedPasskey = NULL;
		pcache->iCachedPasskeyLength = 0;
	}
	
	pgpClearMemory (pcache->CachedKeyID, kPGPMaxExportedKeyIDSize);
	pcache->CachedKeyAlg = 0;
	pcache->CachedHashAlg = 0;
	pcache->uCachedOptions = 0;
}

//_____________________________________________________
//

static VOID 
sSetCacheDuration (PCACHE pcache, UINT uSecs)
{
	pcache->iCacheCounter = (uSecs * 1000) / CACHECLOCKPERIOD;
}

//_____________________________________________________
//

static BOOL 
sIsCacheValid (PCACHE pcache)
{
	if (pcache->bCacheActive) {
		if (pcache->pszCachedPassphrase || 
			pcache->pCachedPasskey) 
		{
			return TRUE;
		}
	}

	return FALSE;
}

//_____________________________________________________
//

static VOID 
sScrambleCache (PCACHE pcache)
{
	INT i;
	INT iLen;

	iLen = lstrlen (pcache->pszCachedPassphrase);
	pcache->iCachedPassphraseLength = iLen;
	for (i=0; i<iLen; i++) 
		pcache->pszCachedPassphrase[i] ^= szScrambler[i%SCRAMBLERLENGTH];

	iLen = pcache->iCachedPasskeyLength;
	for (i=0; i<iLen; i++) 
		pcache->pCachedPasskey[i] ^= szScrambler[i%SCRAMBLERLENGTH];
}


//_____________________________________________________
//

static PGPError 
sCopyCache (
		PCACHE			pcache,
		PGPContextRef	context,
		LPSTR*			pszPassphraseBuffer,
		PGPByte**		ppPasskeyBuffer,
		PGPUInt32*		piPasskeyLength)
{
	PGPError	err		= kPGPError_NoErr;
	INT			i;
	INT			iLen;

	if ((pcache->pCachedPasskey) && (ppPasskeyBuffer)) 
	{
		iLen = pcache->iCachedPasskeyLength;
		*ppPasskeyBuffer = secAlloc (context, iLen);

		if(*ppPasskeyBuffer) {
			for (i=0; i<iLen; i++) 
			{
				(*ppPasskeyBuffer)[i] = 
						pcache->pCachedPasskey[i] ^ 
							szScrambler[i%SCRAMBLERLENGTH];
			}
			*piPasskeyLength = iLen;
		}
		else 
			err = kPGPError_OutOfMemory;
	}

	if ((pcache->pszCachedPassphrase) && (pszPassphraseBuffer)) 
	{
		iLen = pcache->iCachedPassphraseLength;
		*pszPassphraseBuffer = secAlloc (context, iLen+1);

		if (*pszPassphraseBuffer) 
		{
			for (i=0; i<iLen; i++) 
			{
				(*pszPassphraseBuffer)[i] = 
						pcache->pszCachedPassphrase[i] ^ 
							szScrambler[i%SCRAMBLERLENGTH];
			}
			(*pszPassphraseBuffer)[iLen] = '\0';
		}
		else
			err = kPGPError_OutOfMemory;
	}

	return err;
}


//_____________________________________________________
//
//  thread to clear cache after delay period
//	This routine is executed as a separate thread.  The
//	routine wakes up every second and decrements the
//	counter.  Whenthe counter reaches zero, the cache 
//	is purged and the flag is set to indicate cache is 
//	not active.

static VOID
sClearDecryptionCacheThread (PVOID pvoid) 
{
	while (cacheDecryption.iCacheCounter > 0) {
		Sleep (CACHECLOCKPERIOD);
		cacheDecryption.iCacheCounter--;
	}

	EnterCriticalSection (&criticalSectionCache);

	sEmptyCache (&cacheDecryption);
	cacheDecryption.bCacheActive = FALSE;

	LeaveCriticalSection (&criticalSectionCache);
}

	
//_____________________________________________________
//
//  thread to clear signing cache after delay period
//	This routine is executed as a separate thread.  The
//	routine wakes up every second and decrements the
//	counter.  Whenthe counter reaches zero, the cache 
//	is purged and the flag is set to indicate cache is 
//	not active.

static VOID 
sClearSigningCacheThread (PVOID pvoid) 
{
	while (cacheSigning.iCacheCounter > 0) {
		Sleep (CACHECLOCKPERIOD);
		cacheSigning.iCacheCounter--;
	}

	EnterCriticalSection (&criticalSectionCache);

	sEmptyCache (&cacheSigning);
	cacheSigning.bCacheActive = FALSE;

	LeaveCriticalSection (&criticalSectionCache);
}

	
//_____________________________________________________
//
//	PGPGetCachedDecryptionPhrase
//	This routine is called to get either cached phrase 
//	(if available) or prompt user for phrase.
//
//

PGPError PGPclExport
PGPclGetCachedDecryptionPhrase (
		PGPContextRef		context, 
		PGPtlsContextRef	tlsContext,
		PGPKeySetRef		keysetMain,
		HWND				hwnd, 
		LPSTR				szPrompt, 
		BOOL				bForceUserInput,
		LPSTR*				pszBuffer,
		PGPKeySetRef		keysetEncryptedTo,
		PGPKeyID*			pkeyidEncryptedTo,
		UINT				uKeyIDCount,
		PGPByte**			ppPasskeyBuffer,
		PGPUInt32*			piPasskeyLength,
		PGPKeySetRef*		pkeysetAdded) 
{
	PGPBoolean		bCacheEnabled		= FALSE;
	PGPUInt32		uCacheSecs			= 0;
	PGPError		err					= kPGPError_NoErr;

	DWORD			dw;
	PGPPrefRef		prefref;

	// Get needed preferences
	err = PGPclOpenClientPrefs (PGPGetContextMemoryMgr (context), &prefref);
	if (IsntPGPError (err))
	{
		PGPGetPrefBoolean (prefref, 
							kPGPPrefDecryptCacheEnable, &bCacheEnabled);
		PGPGetPrefNumber (prefref, 
							kPGPPrefDecryptCacheSeconds, &uCacheSecs);
		PGPclCloseClientPrefs (prefref, FALSE);
	}

	EnterCriticalSection (&criticalSectionCache);

	// use phrase from cache
	if (!bForceUserInput && 
		bCacheEnabled && 
		sIsCacheValid (&cacheDecryption))
	{
		// copy passphrase/passkey to caller's buffers
		err = sCopyCache (&cacheDecryption, context, pszBuffer, 
									ppPasskeyBuffer, piPasskeyLength);
		// reset cache duration
		sSetCacheDuration (&cacheDecryption, uCacheSecs);
	}

	// cache is disabled or empty, get phrase from user
	else 
	{
		sEmptyCache (&cacheDecryption);

		err = PGPclGetPhrase (context, keysetMain, hwnd, szPrompt, 
								&cacheDecryption.pszCachedPassphrase, 
								keysetEncryptedTo, 
								pkeyidEncryptedTo, uKeyIDCount,
								NULL, NULL, PGPCL_DECRYPTION,
								&cacheDecryption.pCachedPasskey,
								&cacheDecryption.iCachedPasskeyLength,
								0, 0, tlsContext, pkeysetAdded);

		// user entered phrase -- setup cache
		if ((IsntPGPError (err)) && 
			(cacheDecryption.pszCachedPassphrase || 
							cacheDecryption.pCachedPasskey)) 
		{
			// scramble cache contents
			sScrambleCache (&cacheDecryption);

			// copy data to caller's buffers
			err = sCopyCache (&cacheDecryption, context, pszBuffer, 
										ppPasskeyBuffer, piPasskeyLength);

			if (bCacheEnabled) {
				// set cache duration
				sSetCacheDuration (&cacheDecryption, uCacheSecs);

				// start thread to purge cache after delay
				if (!cacheDecryption.bCacheActive) {
					cacheDecryption.bCacheActive = TRUE;
					_beginthreadex (NULL, 0, 
						(LPTHREAD_START_ROUTINE)sClearDecryptionCacheThread, 
						0, 0, &dw);
				}
			}
			else
				sEmptyCache (&cacheDecryption);
		}

		// problem (cancel or other error) -- flush cache
		else {
			sEmptyCache (&cacheDecryption);
			sSetCacheDuration (&cacheDecryption, 0);

			if (err == kPGPError_NoErr) 
				err = kPGPError_UserAbort;
		}
	}

	LeaveCriticalSection (&criticalSectionCache);

//	MessageBox(hwnd,*pszBuffer,"Passphrase",MB_OK);

	return err;
}

//_____________________________________________________
//
//	PGPGetCachedSigningPhrase
//	This routine is called to get either signing cached phrase 
//	(if available) or prompt user for phrase.
//

PGPError PGPclExport
PGPclGetCachedSigningPhrase (
		PGPContextRef		context, 
		PGPtlsContextRef	tlsContext,
		HWND				hwnd,
		LPSTR				szPrompt,
		BOOL				bForceUserInput, 
		LPSTR*				pszBuffer, 
		PGPKeySetRef		keysetSigning,
		PGPKeyRef*			pkeySigning,
		PGPHashAlgorithm*	pulHashAlg,
		UINT*				puOptions, 
		UINT				uFlags,
		PGPByte**			ppPasskeyBuffer,
		PGPUInt32*			piPasskeyLength,
		PGPKeySetRef*		pkeysetAdded)
{
	PGPBoolean		bCacheEnabled	= FALSE;
	PGPUInt32		uCacheSecs		= 0;
	PGPError		err				= kPGPError_NoErr;
	PGPBoolean		bWarnRSADSA		= FALSE;
	BOOL			bAskUser		= TRUE;
	PGPSize			size;
	PGPPrefRef		prefref;
	PGPKeyID		keyid;
	DWORD			dw;

	// Get needed preferences
	err = PGPclOpenClientPrefs (PGPGetContextMemoryMgr (context), &prefref);
	if (IsntPGPError (err))
	{
		PGPGetPrefBoolean (prefref, kPGPPrefSignCacheEnable, &bCacheEnabled);
		PGPGetPrefNumber (prefref, kPGPPrefSignCacheSeconds, &uCacheSecs);
		// Need a never show again box?
		PGPGetPrefBoolean (prefref, 
					kPGPPrefWarnOnRSARecipAndNonRSASigner, &bWarnRSADSA);
		PGPclCloseClientPrefs (prefref, FALSE);
	}

	EnterCriticalSection (&criticalSectionCache);

	// use phrase from cache
	if (!bForceUserInput && 
		bCacheEnabled && 
		sIsCacheValid (&cacheSigning)) 
	{
		err = kPGPError_NoErr;

		if (bWarnRSADSA && 
			(cacheSigning.CachedKeyAlg != kPGPPublicKeyAlgorithm_RSA) && 
			(uFlags & PGPCL_RSAENCRYPT)) 
		{
			PGPBoolean bNeverShowAgain	= FALSE;

			err = PGPclRSADSAMixWarning (hwnd, &bNeverShowAgain);

			switch (err) {
			case kPGPError_NoErr :
				bAskUser = TRUE;
				if (bNeverShowAgain) {
					err = PGPclOpenClientPrefs (
								PGPGetContextMemoryMgr (context), &prefref);
					if (IsntPGPError (err))
					{
						PGPSetPrefBoolean (prefref, 
							kPGPPrefWarnOnRSARecipAndNonRSASigner, FALSE);
						PGPclCloseClientPrefs (prefref, TRUE);
					}
				}
				break;

			case kPGPError_UserAbort :
				bAskUser = FALSE;
				break;
			}
		}

		if (err == kPGPError_NoErr) {
			// reset cache duration
			sSetCacheDuration (&cacheSigning, uCacheSecs);

			// see if cached key is in keyset
			if (pkeySigning) {
				err = PGPImportKeyID (&cacheSigning.CachedKeyID, &keyid);
				if (IsntPGPError (err)) {

					// if so, get keyref for this keyid
					err = PGPGetKeyByKeyID (keysetSigning, &keyid, 
								kPGPPublicKeyAlgorithm_Invalid, pkeySigning);
					if (IsPGPError (err)) 
						*pkeySigning = NULL;
				}

				err = sCopyCache (&cacheSigning, context, pszBuffer, 
										ppPasskeyBuffer, piPasskeyLength);

				if (pulHashAlg) 
					*pulHashAlg = cacheSigning.CachedHashAlg;
				if (puOptions) 
					*puOptions = cacheSigning.uCachedOptions;

				bAskUser = FALSE;
			}
		}
	}

	// otherwise, get phrase from user
	if (bAskUser) 
	{
		sEmptyCache (&cacheSigning);

		// call PGPcl routine to post passphrase dialog
		err = PGPclGetPhrase (context, keysetSigning, hwnd, szPrompt, 
									 &cacheSigning.pszCachedPassphrase, 
									 NULL, NULL, 0, pkeySigning, puOptions, 
									 uFlags,
									 &cacheSigning.pCachedPasskey,
									 &cacheSigning.iCachedPasskeyLength,
									 0, 0, tlsContext, pkeysetAdded);

		// user entered phrase -- setup cache
		if ((err == kPGPError_NoErr) && 
			(cacheSigning.pszCachedPassphrase || 
						cacheSigning.pCachedPasskey)) 
		{
			// scramble cache contents
			sScrambleCache (&cacheSigning);

			if (bCacheEnabled) {
				sSetCacheDuration (&cacheSigning, uCacheSecs);

				if (!cacheSigning.bCacheActive) {
					cacheSigning.bCacheActive = TRUE;
					_beginthreadex (NULL, 0, 
						(LPTHREAD_START_ROUTINE)sClearSigningCacheThread, 
						0, 0, &dw);
				}
			}

			// copy to caller's buffers
			err = sCopyCache (&cacheSigning, context, pszBuffer, 
								ppPasskeyBuffer, piPasskeyLength);

			PGPGetHashAlgUsed (*pkeySigning, &cacheSigning.CachedHashAlg);
			if (pulHashAlg) 
				*pulHashAlg = cacheSigning.CachedHashAlg;

			if (bCacheEnabled) {
				if (puOptions) 
					cacheSigning.uCachedOptions = *puOptions;
				else 
					cacheSigning.uCachedOptions = 0;

				PGPGetKeyIDFromKey (*pkeySigning, &keyid);
				PGPExportKeyID (&keyid, 
									cacheSigning.CachedKeyID, &size);

				PGPGetKeyNumber (*pkeySigning, 
					kPGPKeyPropAlgID, &cacheSigning.CachedKeyAlg);
			}
			else 
			{
				sEmptyCache (&cacheSigning);
			}
		}

		// problem (cancel or other error) -- flush cache
		else {
			sEmptyCache (&cacheSigning);
			sSetCacheDuration (&cacheSigning, 0);

			if (err == kPGPError_NoErr)
				err = kPGPError_UserAbort;
		}
	}

	LeaveCriticalSection (&criticalSectionCache);

	return err;
}

//	___________________________________________________
//
//	Broadcast purge message

VOID PGPclExport 
PGPclNotifyPurgePassphraseCache (
		WPARAM wParam,
		LPARAM lParam)
{
	UINT uMessageID;

	uMessageID = RegisterWindowMessage (PURGEPASSPHRASECACEHMSG);
	PostMessage (HWND_BROADCAST, uMessageID, wParam, lParam);
}

//___________________________________
//
//	PGPPurgeCachedDecryptionPhrase
//	Called to purge phrase from cache.

BOOL PGPclExport
PGPclPurgeCachedPassphrase (DWORD dwFlags) 
{
	BOOL	bReturn		= FALSE;

	EnterCriticalSection (&criticalSectionCache);

	if (dwFlags & PGPCL_DECRYPTIONCACHE) {
		sEmptyCache (&cacheDecryption);
		sSetCacheDuration (&cacheDecryption, 0);
		bReturn |= cacheDecryption.bCacheActive;
	}

	if (dwFlags & PGPCL_SIGNINGCACHE) {
		sEmptyCache (&cacheSigning);
		sSetCacheDuration (&cacheSigning, 0);
		bReturn |= cacheSigning.bCacheActive;
	}

	LeaveCriticalSection (&criticalSectionCache);

	return bReturn;
}

//________________________________________________
//
//	PGPFreeCachedPhrase
//	Called to wipe and free the phrase returned by
//	PGPGetCachedPhrase.

VOID PGPclExport
PGPclFreeCachedPhrase (LPSTR szPhrase) 
{
	pgpClearMemory (szPhrase, lstrlen (szPhrase));
	secFree (szPhrase);
}

//_________________________________________________________
//
//	PGPQueryDecryptCacheSecsRemaining
//	Returns number of seconds remaining until cache expires

INT PGPclExport 
PGPclQueryDecryptionCacheSecsRemaining (VOID) 
{
	return (cacheDecryption.iCacheCounter);	
}

//_________________________________________________________
//
//	PGPQuerySignCacheSecsRemaining
//	Returns number of seconds remaining until cache expires

INT PGPclExport
PGPclQuerySigningCacheSecsRemaining (VOID) 
{
	return (cacheSigning.iCacheCounter);	
}


//_____________________________________________________
//

VOID
PGPInitPassphraseCaches (PGPContextRef context)
{
	if (!bScramblerInitialized) {
		PGPContextGetRandomBytes (context, 
							szScrambler, sizeof(szScrambler));
		bScramblerInitialized = TRUE;
	}

	pgpClearMemory (&cacheDecryption, sizeof(CACHE));
	pgpClearMemory (&cacheSigning, sizeof(CACHE));


	InitializeCriticalSection (&criticalSectionCache);
}

//_____________________________________________________
//

VOID
PGPClosePassphraseCaches (VOID)
{
	while (
		PGPclPurgeCachedPassphrase (
			PGPCL_DECRYPTIONCACHE|PGPCL_SIGNINGCACHE)) 
	{
		Sleep (200);
	}

	pgpClearMemory (&cacheDecryption, sizeof(CACHE));
	pgpClearMemory (&cacheSigning, sizeof(CACHE));
	DeleteCriticalSection (&criticalSectionCache);
}


