/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: recpass.c,v 1.21 1999/04/13 17:29:54 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"

void FreePhrases(MYSTATE *ms)
{
	if(ms->PassPhrase)
	{
		PGPclFreeCachedPhrase(ms->PassPhrase);
		ms->PassPhrase=NULL;
	}

	if(ms->PassKey)
	{
		memset(ms->PassKey,0x00,ms->PassKeyLen);
		PGPFreeData(ms->PassKey);
		ms->PassKey=NULL;
	}

	if(ms->ConvPassPhrase)
	{
		PGPclFreePhrase(ms->ConvPassPhrase);
		ms->ConvPassPhrase=NULL;
	}
}

/*Returns TRUE if the user hit cancel*/

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
					  PGPKeySetRef	*AddedKeys,
					  char *szTitle)
{
	BOOL UserCancel = FALSE;
	BOOL ForceEntry = FALSE;
	char PhraseTitle[256];

	assert(EnteredOrCachedPhrase);

	if(*EnteredOrCachedPhrase)
	{
		PGPclFreeCachedPhrase(*EnteredOrCachedPhrase);
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


	if(PGPclGetCachedDecryptionPhrase (context,
					tlsContext,
					MainKeySet,
					hwnd, 
					PhraseTitle, 
					ForceEntry,
					EnteredOrCachedPhrase,
					KeySet,
					RecipientKeyIDArray,
					dwKeyIDCount,
					PassKey,PassKeyLen,
					AddedKeys,szTitle)!= kPGPError_NoErr)
	{
		UserCancel = TRUE;
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
