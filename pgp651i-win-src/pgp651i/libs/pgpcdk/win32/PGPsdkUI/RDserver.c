/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: RDserver.c,v 1.12 1999/03/10 03:01:31 heller Exp $
____________________________________________________________________________*/

#include "RDprecmp.h"

// LookUpUnknownKeys
//
// Use common code to do key lookup, then rebuild the tables

PGPError LookUpUnknownKeys(HWND hdlg,PRECGBL prg)
{ 
	PGPError err;
	PGPBoolean haveNewKeys;

	err = PGPUpdateMissingRecipients( hdlg, &(prg->mRecipients), &haveNewKeys );

	PGPsdkUIErrorBox (hdlg,err);

	if( IsntPGPError( err ) && haveNewKeys )
	{
		BuildTables(prg);
	}

	return err;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
