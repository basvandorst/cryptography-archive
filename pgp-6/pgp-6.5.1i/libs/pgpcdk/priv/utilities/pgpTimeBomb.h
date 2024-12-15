/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpTimeBomb.h,v 1.10 1999/03/10 02:59:06 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpTimeBomb_h	/* [ */
#define Included_pgpTimeBomb_h

#include "pgpPubTypes.h"	/* for PGP_BEGIN_C_DECLARATIONS */

/*____________________________________________________________________________
	set PGP_TIME_BOMB to 0 to disable time bomb, 1 to enable
	
	If you enable it, set the year, month, day appropriately.
____________________________________________________________________________*/
#define PGP_TIME_BOMB		0
#define kPGPTimeBombYear	1997
#define kPGPTimeBombMonth	9	/* 1-12 */
#define kPGPTimeBombDay		30	/* 1-31 */



#if PGP_TIME_BOMB

PGP_BEGIN_C_DECLARATIONS


PGPBoolean		pgpTimeBombHasExpired( void );



PGP_END_C_DECLARATIONS

#endif


#endif /* ] Included_pgpTimeBomb_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
