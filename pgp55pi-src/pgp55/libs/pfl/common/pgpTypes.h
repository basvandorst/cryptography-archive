/*____________________________________________________________________________
	pgpTypes.h
	
	Copyright (C) 1996,1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Basic type definitions

	$Id: pgpTypes.h,v 1.5 1997/06/17 21:44:33 hal Exp $
____________________________________________________________________________*/
#ifndef Included_pgpTypes_h	/* [ */
#define Included_pgpTypes_h

#if PGP_MACINTOSH
#include <types.h>
#else
#include <sys/types.h>
#endif

#include "pgpBase.h"

#if ! HAVE_UCHAR
typedef unsigned char	uchar;
#endif

#if ! HAVE_USHORT
typedef unsigned short	ushort;
#endif

#if ! HAVE_UINT
typedef unsigned int	uint;
#endif

#if ! HAVE_ULONG
typedef unsigned long	ulong;
#endif

typedef char			VoidAlign;

#if ! PGP_MACINTOSH
typedef PGPBoolean		Boolean;
#endif


/* Lookie here!  An ANSI-compliant alignment finder! */
#ifndef 	alignof
#define 	alignof(type) (sizeof(struct{type _x; char _y;}) - sizeof(type))
#if PGP_WIN32
/* Causes "unnamed type definition in parentheses" warning" */
#pragma warning ( disable : 4116 )
#endif
#endif


#endif /* ] Included_pgpTypes_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
