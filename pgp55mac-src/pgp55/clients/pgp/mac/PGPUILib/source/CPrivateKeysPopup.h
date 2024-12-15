/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: CPrivateKeysPopup.h,v 1.6 1997/10/02 00:14:33 heller Exp $
____________________________________________________________________________*/

#pragma once

#include "CGAFix.h"
#include "pgpKeys.h"

struct PGPKeySpec;

class CPrivateKeysPopup : public CGAPopupFix
{
public:
	enum { class_ID = 'PKPP' };
							CPrivateKeysPopup(LStream *inStream);
	virtual					~CPrivateKeysPopup();

	virtual	void			GetCurrentItemTitle(Str255 outItemTitle);	
	PGPKeyRef				GetSigningKey(void);
	void					SetSigningKey(PGPKeyRef signingKey);
	Boolean					BuildKeyList(PGPKeySetRef allKeys);
	
private:

	LArray					*mKeyItemList;
};
