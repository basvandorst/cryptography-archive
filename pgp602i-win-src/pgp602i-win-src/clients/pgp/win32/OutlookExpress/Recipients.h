/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: Recipients.h,v 1.4.8.1 1998/11/12 03:12:48 heller Exp $
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
