//: PGPphrase.h - include for PGP common passphrase routines
//
//	$Id: PGPphrase.h,v 1.5 1997/05/28 15:45:20 pbj Exp $
//
/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */

#ifndef _PGPPHRASEH
#define _PGPPHRASEH

#define	PGPPHRASE_OK				0
#define PGPPHRASE_USERCANCEL		5501
#define PGPPHRASE_MEMERROR			5502
#define PGPPHRASE_LIBERROR			5503
#define PGPPHRASE_KEYRINGERROR		5504

#ifdef __cplusplus
extern "C" {
#endif

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

INT PGPGetCachedPhrase (HWND hWnd, LPSTR szPrompt, BOOL bForceUserInput,
						 LPSTR* pszBuffer);


//-------------------------------------------------|
//	PGPPurgeCachedPhrase -
//	purge passphrase cache.
//
//	returns TRUE if cache thread is still active
//			FALSE if cache thread has terminated
//
//	(can be called repeatedly to poll for thread termination)
//

BOOL PGPPurgeCachedPhrase (void);


//-------------------------------------------------|
//	PGPQueryCacheSecsRemaining
//	Returns number of seconds remaining until cache expires

INT PGPQueryCacheSecsRemaining (void);


//-------------------------------------------------|
//	PGPGetSignCachedPhrase -
//	get signing key passphrase from cache or user.
//
//	Entry parameters :
//		hwnd			- handle of parent window
//		szPrompt		- message string to be displayed to user
//		bForceUserInput - TRUE => force dialog box regardless of cache
//		pszBuffer		- variable to receive pointer to buffer with phrase
//		szKeyID			- string buffer to receive KeyID, minimum 19 chars
//						  NULL => do not display key selection combo box
//		pulHashAlg		- points to buffer that receives hash algorithm value.
//						  NULL is OK.
//		uOptions		- pointer to buffer which contains and will receive
//						  options bits.
//						  NULL => hide option checkboxes
//		uFlags			- flags bits.
//						  PGPCOMDLG_RSAENCRYPT => encrypting to RSA key(s).		
//
//
//	This function returns one of the above-defined values.
//	If PGPPHRASE_OK, PGPGetCachedPhrase has allocated a buffer,
//  filled it with the passphrase, and returned a pointer to the
//	buffer in pszBuffer.
//
//  Important: the caller must call PGPFreePhrase when the phrase
//  is no longer needed!

INT PGPGetSignCachedPhrase (HWND hWnd, LPSTR szPrompt, BOOL bForceUserInput,
						 LPSTR* pszBuffer, LPSTR szKeyID, ULONG* pulHashAlg,
						 UINT* uOptions, UINT uFlags);


//-------------------------------------------------|
//	PGPPurgeSignCachedPhrase -
//	purge signing key passphrase cache.
//
//	returns TRUE if cache thread is still active
//			FALSE if cache thread has terminated
//
//	(can be called repeatedly to poll for thread termination)
//

BOOL PGPPurgeSignCachedPhrase (void);


//-------------------------------------------------|
//	PGPQuerySignCacheSecsRemaining
//	Returns number of seconds remaining until signing key cache expires

INT PGPQuerySignCacheSecsRemaining (void);


//-------------------------------------------------|
//	PGPSetCachedPhrase
//	Called to set phrase in cache.

INT PGPSetCachedPhrase (LPSTR szPhrase);


//-------------------------------------------------|
//	PGPSetSignCachedPhrase
//	Called to set phrase in cache.

//INT PGPSetSignCachedPhrase (LPSTR szPhrase);


//-------------------------------------------------|
//	PGPFreePhrase -
//	wipe and deallocate the passphrase buffer.

VOID PGPFreePhrase (LPSTR szPhrase);

#ifdef __cplusplus
}
#endif

#endif	//_PGPPHRASEH
