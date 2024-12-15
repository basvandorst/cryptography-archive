/*____________________________________________________________________________
	pgpPFLConfig.h  (Win32 version)
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	This file contains the Win32 version of the configuration file
	normally generated by the automatic configure script on Unix.

	$Id: pgpPFLConfig.h,v 1.2 1999/03/10 02:53:57 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpPFLConfig_h	/* [ */
#define Included_pgpPFLConfig_h

#define HAVE_STDARG_H	1
#define HAVE_STDLIB_H	1
#define HAVE_SYS_STAT_H	1
#define HAVE_UNISTD_H	0
#define HAVE_USHORT		0
#define HAVE_UINT		0
#define HAVE_ULONG		0
#define NO_LIMITS_H		0
#define NO_POPEN		1

#if  defined( __MWERKS__ )

	#define PGP_HAVE64		0
	
	
#elif defined( _MSC_VER )
	#define PGP_HAVE64		1
	typedef __int64				PGPInt64;
	typedef unsigned __int64	PGPUInt64;
	
	
	
#endif




#endif /* ] Included_pgpPFLConfig_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
