/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: recpass.h,v 1.14 1999/04/13 17:29:54 wjb Exp $
____________________________________________________________________________*/
#ifndef Included_RECPASS_h	/* [ */
#define Included_RECPASS_h


BOOL GetDecryptPhrase(PGPContextRef context,
					  PGPKeySetRef MainKeySet,
					  HWND hwnd, 
					  char **EnteredOrCachedPhrase,
					  unsigned short *PassPhraseCount,
					  char *KeyForPhrase,
					  PGPKeySetRef KeySet,
					  PGPKeyID *RecipientKeyIDArray,
					  DWORD dwKeyIDCount,
					  PGPByte **PassKey,
					  PGPUInt32 *PassKeyLen,
					  PGPtlsContextRef tlsContext,
					  PGPKeySetRef *AddedKeys,
					  char *szTitle);

void FreePhrases(MYSTATE *myState);

#endif /* ] Included_RECPASS_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
