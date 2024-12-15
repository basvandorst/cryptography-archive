/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: Recipients.h,v 1.2 1997/07/25 23:50:48 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_Recipients_h	/* [ */
#define Included_Recipients_h


#include "stdinc.h"
#include "pgpPubTypes.h"
#include "PGPRecip.h"

PGPError GetRecipients(IExchExtCallback *pmecb, 
					   PGPContextRef context,
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
