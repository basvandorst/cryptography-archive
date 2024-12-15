/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: pgpSocketsLDAP.h,v 1.7 1999/03/25 17:59:20 melkins Exp $
____________________________________________________________________________*/

#ifndef Included_pgpSocketsLDAP_h
#define Included_pgpSocketsLDAP_h

#if PGP_WIN32
#include <winsock.h>
#endif

#if PGP_MACINTOSH
#include <stdlib.h>

#include "PGPldapLibUtils.h"

#include <TextUtils.h>

#define strcasecmp( s1, s2 )	IdenticalText( s1, s2, strlen( s1 ), \
					strlen( s2 ), nil )


#include <Memory.h>	/* to get BlockMove() */

#ifndef isascii
#define isascii(c)	((unsigned)(c)<=0177)	/* for those who don't have this in ctype.h */
#endif isascii

#define ntohl( l )	PGPNetToHostLong(l)
#define htonl( l )	PGPHostToNetLong(l)
#define ntohs( s )	PGPNetToHostShort(s)
#define htons( s )	PGPHostToNetShort(s)

struct timeval {
	PGPInt32	tv_sec;
	PGPInt32	tv_usec;
};
#endif


#endif
