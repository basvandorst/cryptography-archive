/*____________________________________________________________________________
	pgpDEBUGStartup.h
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpDEBUGStartup.h,v 1.3 1999/03/10 02:49:28 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpDEBUGStartup_h	/* [ */
#define Included_pgpDEBUGStartup_h



#if PGP_DEBUG	/* [ */

PGP_BEGIN_C_DECLARATIONS

void	PGPDebugStartup( void );


PGP_END_C_DECLARATIONS

#else	/* ] [ */

#define PGPDebugStartup()	{/*nothing*/}

#endif	/* ] */

#endif /* ] Included_pgpDEBUGStartup_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
