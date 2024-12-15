/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: Recipients.h,v 1.6 1999/04/09 15:26:26 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_Recipients_h	/* [ */
#define Included_Recipients_h

#include <windows.h>
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpcl.h"

PGPError GetRecipients(HWND hwnd,
					   char *szNames, 
					   PGPContextRef pgpContext, 
					   PGPtlsContextRef tlsContext,
					   RECIPIENTDIALOGSTRUCT *prds);

void FreeRecipients(RECIPIENTDIALOGSTRUCT *prds);

#endif /* ] Included_Recipients_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
