/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: recpass.h,v 1.8 1997/10/08 19:04:58 wjb Exp $
____________________________________________________________________________*/
#ifndef Included_RECPASS_h	/* [ */
#define Included_RECPASS_h


BOOL GetDecryptPhrase(PGPContextRef context,HWND hwnd, 
					  char **EnteredOrCachedPhrase,
					  unsigned long *PassPhraseLen,
					  unsigned short *PassPhraseCount,
					  char *KeyForPhrase,
					  PGPKeySetRef KeySet,
					  PGPKeyID *RecipientKeyIDArray,
					  DWORD dwKeyIDCount);


#endif /* ] Included_RECPASS_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
