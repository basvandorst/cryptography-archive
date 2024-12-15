/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: GetPassphrase.h,v 1.2 1997/09/17 19:36:43 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include "pgpKeys.h"

Boolean	DoPassphraseDialog(Boolean firstTry, const char *userID,
							char *passphrase);

