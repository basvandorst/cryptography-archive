/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
//: PGPphrase.h - include for PGP common passphrase routines

#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C	extern "C"
#else
#define EXTERN_C
#endif
#endif


#define	PGPPHRASE_OK				0
#define PGPPHRASE_USERCANCEL		-1
#define PGPPHRASE_MEMERROR			-2
#define PGPPHRASE_LIBERROR			-3


//-------------------------------------------------|
//	PGPGetCachedPhrase -
//	get passphrase from cache or user.
//
//	Entry parameters :
//		hwnd			- handle of parent window
//		szPrompt		- message string to be displayed to user
//		bForceUserInput - TRUE => force dialog box regardless of cache
//		pszBuffer		- variable to receive pointer to buffer with phrase
//
//	This function returns one of the above-defined values.
//	If PGPPHRASE_OK, PGPGetCachedPhrase has allocated a buffer,
//  filled it with the passphrase, and returned a pointer to the
//	buffer in pszBuffer.
//
//  Important: the caller must call PGPFreePhrase when the phrase
//  is no longer needed!

EXTERN_C INT PGPGetCachedPhrase (HWND hWnd, LPSTR szPrompt,
						BOOL bForceUserInput, LPSTR* pszBuffer);


//-------------------------------------------------|
//	PGPPurgeCachedPhrase -
//	purge passphrase cache.

EXTERN_C void PGPPurgeCachedPhrase (void);


//-------------------------------------------------|
//	PGPQueryCacheSecsRemaining
//	Returns number of seconds remaining until cache expires

EXTERN_C INT PGPQueryCacheSecsRemaining (void);


//-------------------------------------------------|
//	PGPGetSignCachedPhrase -
//	get signing key passphrase from cache or user.
//
//	Entry parameters :
//		hwnd			- handle of parent window
//		szPrompt		- message string to be displayed to user
//		bForceUserInput - TRUE => force dialog box regardless of cache
//		pszBuffer		- variable to receive pointer to buffer with phrase
//
//	This function returns one of the above-defined values.
//	If PGPPHRASE_OK, PGPGetCachedPhrase has allocated a buffer,
//  filled it with the passphrase, and returned a pointer to the
//	buffer in pszBuffer.
//
//  Important: the caller must call PGPFreePhrase when the phrase
//  is no longer needed!

EXTERN_C INT PGPGetSignCachedPhrase (HWND hWnd, LPSTR szPrompt,
						BOOL bForceUserInput, LPSTR* pszBuffer);


//-------------------------------------------------|
//	PGPPurgeSignCachedPhrase -
//	purge signing key passphrase cache.

EXTERN_C void PGPPurgeSignCachedPhrase (void);


//-------------------------------------------------|
//	PGPQuerySignCacheSecsRemaining
//	Returns number of seconds remaining until signing key cache expires

EXTERN_C INT PGPQuerySignCacheSecsRemaining (void);


//-------------------------------------------------|
//	PGPSetCachedPhrase
//	Called to set phrase in cache.

EXTERN_C INT PGPSetCachedPhrase (LPSTR szPhrase);


//-------------------------------------------------|
//	PGPSetSignCachedPhrase
//	Called to set phrase in cache.

EXTERN_C INT PGPSetSignCachedPhrase (LPSTR szPhrase);


//-------------------------------------------------|
//	PGPFreePhrase -
//	wipe and deallocate the passphrase buffer.

EXTERN_C void PGPFreePhrase (LPSTR szPhrase);
