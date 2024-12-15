/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include <LArray.h>

typedef struct PassphraseListItem
{
	char	*passphrase;
	
} PassphraseListItem;

class CPGPtoolsPassphraseList : public LArray
{
public:
				CPGPtoolsPassphraseList(void);
				~CPGPtoolsPassphraseList(void);
				
	void		RememberPassphrase(PGPContextRef context,
						const char *passphrase);
};