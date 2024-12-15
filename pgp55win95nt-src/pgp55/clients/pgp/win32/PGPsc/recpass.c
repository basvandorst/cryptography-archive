/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: recpass.c,v 1.13 1997/10/08 19:04:56 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"

/*Returns TRUE if the user hit cancel*/

BOOL GetDecryptPhrase(PGPContextRef context,HWND hwnd, 
					  char **EnteredOrCachedPhrase,
					  unsigned long *PassPhraseLen,
					  unsigned short *PassPhraseCount,
					  char *KeyForPhrase,
					  PGPKeySetRef KeySet,
					  PGPKeyID *RecipientKeyIDArray,
					  DWORD dwKeyIDCount)
{
	BOOL UserCancel = FALSE;
	BOOL ForceEntry = FALSE;
	char PhraseTitle[256];

	assert(EnteredOrCachedPhrase);
	assert(PassPhraseLen);

	if(*EnteredOrCachedPhrase)
	{
		PGPFreePhrase(*EnteredOrCachedPhrase);
		*EnteredOrCachedPhrase = NULL;
	}

	if(PassPhraseCount && *PassPhraseCount > 0)
	{
		LoadString(g_hinst, IDS_WRONG_PHRASE, PhraseTitle, sizeof(PhraseTitle));
		ForceEntry = TRUE;
	}
	else 
	{
		LoadString(g_hinst, IDS_ENTER_PHRASE, PhraseTitle, sizeof(PhraseTitle));
		lstrcat (PhraseTitle, KeyForPhrase);
	}


	if(PGPGetCachedPhrase (context, 
					hwnd, 
					PhraseTitle, 
					ForceEntry,
					EnteredOrCachedPhrase,
					KeySet,
					RecipientKeyIDArray,
					dwKeyIDCount)!= kPGPError_NoErr)
	{
		UserCancel = TRUE;
	}
	else
	{
		*PassPhraseLen = (unsigned long) strlen(*EnteredOrCachedPhrase);
	}
	if(PassPhraseCount)
		++(*PassPhraseCount);

	return(UserCancel);
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
