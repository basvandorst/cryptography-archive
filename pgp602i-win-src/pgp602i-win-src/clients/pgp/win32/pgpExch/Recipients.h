/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: Recipients.h,v 1.4.8.1 1998/11/12 03:14:01 heller Exp $
____________________________________________________________________________*/
#ifndef Included_Recipients_h	/* [ */
#define Included_Recipients_h


#include "stdinc.h"
#include "pgpPubTypes.h"
#include "pgpcl.h"

PGPError GetRecipients(IExchExtCallback *pmecb, 
					   PGPContextRef context,
					   PGPtlsContextRef tlsContext,
					   RECIPIENTDIALOGSTRUCT *prds);

void FreeRecipients(RECIPIENTDIALOGSTRUCT *prds);


#endif /* ] Included_Exchange_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
