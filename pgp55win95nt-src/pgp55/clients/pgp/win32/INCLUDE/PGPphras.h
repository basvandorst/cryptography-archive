/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPphras.h - header for PGP passphrase caching routines
	

	$Id: PGPphras.h,v 1.10 1997/10/08 16:00:46 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_PGPPhras_h	/* [ */
#define Included_PGPPhras_h

#include "..\include\pgpWerr.h"

#ifdef __cplusplus
extern "C" {
#endif

//____________________________________
//
//	PGPGetCachedPhrase -
//	get passphrase from cache or user.
//
//	Entry parameters :
//		Context			- PGP library context
//		hWnd			- handle of parent window
//		szPrompt		- message string to be displayed to user
//		bForceUserInput - TRUE => force dialog box regardless of cache
//		pszBuffer		- variable to receive pointer to buffer with phrase
//		KeySet			- key set containing keys to which this message
//						  has been encrypted
//		pKeyIDs			- list of keyids to append to key list
//		uKeyCount		- total number of keys (if this parameter
//						  is less than number of keys in keyset,
//						  no keyids are added to list)
//
//	This function returns one of the above-defined values.
//	If kPGPError_NoErr, PGPGetCachedPhrase has allocated a buffer,
//  filled it with the passphrase, and returned a pointer to the
//	buffer in pszBuffer.
//
//  Important: the caller must call PGPFreePhrase when the phrase
//  is no longer needed!

INT 
PGPGetCachedPhrase (PGPContextRef Context, 
					HWND hWnd, 
					LPSTR szPrompt, 
					BOOL bForceUserInput,
					LPSTR* pszBuffer,
					PGPKeySetRef KeySet,
					PGPKeyID* pKeyIDs,
					UINT uKeyCount);

//____________________________________
//
//	PGPPurgeCachedPhrase -
//	purge passphrase cache.
//
//	returns TRUE if cache thread is still active
//			FALSE if cache thread has terminated
//
//	(can be called repeatedly to poll for thread termination)
//

BOOL 
PGPPurgeCachedPhrase (void);

//____________________________________
//
//	PGPQueryCacheSecsRemaining
//	Returns number of seconds remaining until cache expires

INT 
PGPQueryCacheSecsRemaining (void);

//____________________________________
//
//	PGPGetSignCachedPhrase -
//	get signing key passphrase from cache or user.
//
//	Entry parameters :
//		Context			- PGP library context
//		hWnd			- handle of parent window
//		szPrompt		- message string to be displayed to user
//		bForceUserInput - TRUE => force dialog box regardless of cache
//		pszBuffer		- variable to receive pointer to buffer with phrase
//		KeySet			- set of keys to display in combobox
//		pKey			- pointer to buffer to receive ref of selected key.
//						  if pKey contains key on entry, that key is used
//						  as default.  Otherwise default key is used.
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
//	If kPGPError_NoErr, PGPGetCachedPhrase has allocated a buffer,
//  filled it with the passphrase, and returned a pointer to the
//	buffer in pszBuffer.  
//
//  Important: the caller must call PGPFreePhrase when the phrase
//  is no longer needed!
//
INT
PGPGetSignCachedPhrase (PGPContextRef	Context, 
						HWND			hWnd,
						LPSTR			szPrompt,
						BOOL			bForceUserInput, 
						LPSTR*			pszBuffer, 
						PGPKeySetRef	KeySet,
						PGPKeyRef*		pKey,
						PGPHashAlgorithm*			pulHashAlg,
						UINT*			puOptions, 
						UINT			uFlags); 

//____________________________________
//
//	PGPPurgeSignCachedPhrase -
//	purge signing key passphrase cache.
//
//	returns TRUE if cache thread is still active
//			FALSE if cache thread has terminated
//
//	(can be called repeatedly to poll for thread termination)
//

BOOL 
PGPPurgeSignCachedPhrase (void);

//____________________________________
//
//	PGPQuerySignCacheSecsRemaining
//	Returns number of seconds remaining until signing key cache expires

INT 
PGPQuerySignCacheSecsRemaining (void);

//____________________________________
//
//	PGPFreePhrase -
//	wipe and deallocate the passphrase buffer.

VOID 
PGPFreePhrase (LPSTR szPhrase);

#ifdef __cplusplus
}
#endif

#endif /* ] Included_PGPPhras_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
