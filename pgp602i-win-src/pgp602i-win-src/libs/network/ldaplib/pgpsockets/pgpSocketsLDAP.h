/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	
	
	$Id: pgpSocketsLDAP.h,v 1.4.10.1 1998/11/12 03:17:47 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpSocketsLDAP_h
#define Included_pgpSocketsLDAP_h

#ifdef PGP_WIN32
#include <winsock.h>
#endif

#ifdef PGP_MACINTOSH
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