/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPpk.h - include file for PGPkeys application
	

	$Id: PGPpk.h,v 1.1 1997/08/29 19:07:22 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_PGPpk_h	/* [ */
#define Included_PGPpk_h

#define WINCLASSNAME	("PGPkeysMainWinClass")
#define WINDOWTITLE		("PGPkeys")
#define SEMAPHORENAME	("PGPkeysInstSem")

#define	PGPPK_SELECTIVEIMPORT		0x00000001

#define	PGPPK_IMPORTKEYMASK			0xFFFF0000
#define PGPPK_IMPORTKEYBUFFER		0x00010000
#define PGPPK_IMPORTKEYFILELIST		0x00020000
#define PGPPK_IMPORTKEYCOMMANDLINE	0x00040000

#endif /* ] Included_PGPkm_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
