/*____________________________________________________________________________
	pgpDEBUGStartup.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpDEBUGStartup.h,v 1.2 1997/06/16 22:21:21 lloyd Exp $
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
