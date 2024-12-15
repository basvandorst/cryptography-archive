/*
* pgpTypes.h -- Basic type definitions
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpTypes.h,v 1.6.2.1 1997/06/07 09:50:14 mhw Exp $
*/

#ifndef PGPTYPES_H		/* [ */
#define PGPTYPES_H

#if !MACINTOSH
#include <sys/types.h>
#endif

#if !HAVE_UCHAR
typedef unsigned char	uchar;
#endif

#if !HAVE_USHORT
typedef unsigned short	ushort;
#endif

#if !HAVE_UINT
typedef unsigned int	uint;
#endif

#if !HAVE_ULONG
typedef unsigned long	ulong;
#endif

typedef char			VoidAlign;

#if ! MACINTOSH
typedef uchar		 	Boolean;
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

/* Lookie here! An ANSI-compliant alignment finder! */
#ifndef 	alignof
#define 	alignof(type) (sizeof(struct{type _x; char _y;}) - sizeof(type))
#endif

#endif /* ] PGPTYPES_H */

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
