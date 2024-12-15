/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpIPsecContextPriv.h,v 1.5 1999/03/10 02:51:59 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpIPsecContextPriv_h	/* [ */
#define Included_pgpIPsecContextPriv_h

#include "pgpMemoryMgr.h"
#include "pgpSymmetricCipher.h"
#include "pgpCBC.h"
#include "pgpCompress.h"
#include "pgpIPsecContext.h"

struct PGPIPsecContext
{
	PGPMemoryMgrRef		memory;
};


#endif /* ] Included_pgpIPsecContextPriv_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
