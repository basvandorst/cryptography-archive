/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpIPsecContext.h,v 1.4 1999/03/10 02:51:58 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpIPsecContext_h	/* [ */
#define Included_pgpIPsecContext_h

#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS

typedef struct PGPIPsecContext			PGPIPsecContext;
typedef PGPIPsecContext *				PGPIPsecContextRef;

#define kInvalidPGPIPsecContextRef		((PGPIPsecContextRef) NULL)
#define PGPIPsecContextRefIsValid(ref)	((ref) != kInvalidPGPIPsecContextRef)
/*

PGPNewIPsecContext

Creates a new IPsec context. This context should be created when the driver
loads, and kept around until the driver unloads.

	ipsec - Pointer to the new context ref.

*/

PGPError PGPNewIPsecContext(PGPIPsecContextRef *ipsec);

/*

PGPFreeIPsecContext

Frees the IPsec context. Do this before the driver unloads.

	ipsec - Context ref to be freed.

*/

PGPError PGPFreeIPsecContext(PGPIPsecContextRef ipsec);

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpIPsecContext_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
