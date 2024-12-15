/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpCodes.hpp,v 1.5.16.1 1998/11/12 03:12:59 heller Exp $
____________________________________________________________________________*/
#ifndef Included_PGPCODES_h	/* [ */
#define Included_PGPCODES_h


#define PGP_CODE_ENCRYPT		2
#define PGP_CODE_SIGN			4
#define PGP_CODE_ENCRYPT_SIGN	8
#define PGP_CODE_DECRYPT		16
#define PGP_CODE_VERIFY			32
#define PGP_CODE_ADD_KEYS		64
#define PGP_CODE_VIEW_KEYS		128
#define PGP_CODE_ENCRYPT_SEND	256
#define PGP_CODE_VERIFY_SIG     512
#define PGP_CODE_WIPE			1024


#endif /* ] Included_PGPCODES_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
